#!/usr/bin/env bash
set -euo pipefail

# Default ezvcpkg location if not already set. Used by vcpkg for any native
# build (Linux extension as well as the web SIDE_MODULE). Matches build.bat's
# C:\.ezvcpkg equivalent.
export EZVCPKG_BASEDIR="${EZVCPKG_BASEDIR:-$HOME/.ezvcpkg}"

# Default emsdk install location and version. Only used by the web target
# (see ensure_emsdk). Override EMSDK_DIR to point at an existing checkout;
# override EMSDK_VERSION to pin to a different toolchain.
#
# Version history:
#   3.1.56 -> 3.1.60: picked up the -sSUPPORT_LONGJMP=wasm compile-time lowering
#     fix. 3.1.56 accepted the flag but emitted saveSetjmp/testSetjmp imports in
#     SIDE_MODULE .o files anyway, which Godot's main wasm can't resolve.
#   3.1.60 -> 3.1.62: required for wasm64. Godot's platform/web/detect.py enforces
#     emcc >= 3.1.62, and the wasm64 SIDE_MODULE dyncall metadata format
#     stabilized in 3.1.62 — building the extension at 3.1.60 against a 3.1.62+
#     main module yields "Cannot set properties of undefined (setting 'sig')"
#     at runtime because function-table entries fail to wire up.
#   3.1.62 -> 3.1.74: 3.1.62 + MEMORY64 + SIDE_MODULE link is broken — wasm-opt's
#     --table64-lowering pass aborts with "i32 != i64: call-indirect call target
#     must match the table index type" because parts of the linker-generated
#     startup code emit table64 call_indirects while user TUs emit i32.wrap_i64
#     table32 indices, and the pass refuses to lower mixed inputs. 3.1.74
#     completes the MEMORY64 dlink work so every TU agrees on table64.
# If KTX or any other dep regresses at a higher emsdk, step up incrementally
# (3.1.76, 4.0.x) rather than reverting — older toolchains have known dlink-ABI
# gaps that re-surface as opaque runtime errors. Kept in a separate directory
# from any other emsdk (e.g. the Godot engine's) so both can coexist with
# independent configs.
export EMSDK_DIR="${EMSDK_DIR:-$HOME/emsdk-cesium}"
export EMSDK_VERSION="${EMSDK_VERSION:-3.1.74}"

# Always activate this repo's pinned emsdk, even if the parent shell has a
# different one active. Checking $EMSDK and skipping activation would
# silently compile against the wrong toolchain. install + activate are
# near-instant no-ops once applied, so cost on repeat builds is negligible.
ensure_emsdk() {
    if [ ! -f "$EMSDK_DIR/emsdk" ]; then
        echo "emsdk not found at $EMSDK_DIR - cloning..."
        if ! command -v git >/dev/null 2>&1; then
            echo "ERROR: git is required on PATH to clone emsdk. Install git, or pre-install emsdk and set EMSDK_DIR." >&2
            return 1
        fi
        git clone https://github.com/emscripten-core/emsdk.git "$EMSDK_DIR" || return 1
    fi
    ( cd "$EMSDK_DIR" && ./emsdk install "$EMSDK_VERSION" ) || {
        echo "ERROR: emsdk install $EMSDK_VERSION failed." >&2
        return 1
    }
    ( cd "$EMSDK_DIR" && ./emsdk activate "$EMSDK_VERSION" ) || {
        echo "ERROR: emsdk activate $EMSDK_VERSION failed." >&2
        return 1
    }
    echo "Activating emsdk $EMSDK_VERSION from $EMSDK_DIR..."
    # shellcheck disable=SC1091
    source "$EMSDK_DIR/emsdk_env.sh"
}

TARGET="${1:-extension}"

case "$TARGET" in
    extension)
        echo "Building GDExtension for $(uname -s) x64..."
        scons arch=x86_64 compileTarget=extension target=template_release precision=double production=yes compiledb=yes
		scons arch=x86_64 compileTarget=extension target=template_debug precision=double compiledb=yes
        ;;
    web)
        echo "Building GDExtension for Web (WASM, wasm32)..."
        unset CESIUM_WEB_MEMORY64
        ensure_emsdk
        # Force emscripten-style longjmp to avoid conflict with godot-cpp's exception handling.
		# -pthread enables atomics/bulk-memory needed for --shared-memory at link time.
		# EMCC_CFLAGS is read by emcc/em++ for ALL compilations (vcpkg, cmake, scons).
		# -fwasm-exceptions: native wasm exception handling (no JS invoke_* wrappers)
		# -sSUPPORT_LONGJMP=wasm: native wasm longjmp (pairs with -fwasm-exceptions)
		# -pthread -fPIC: required for threaded SIDE_MODULE builds
		# -Wno-overriding-option: emsdk 3.1.74's clang flags `-ffp-model=precise`
		# + `-ffp-contract=off` as a redundant override, which KTX's astc-encoder
		# subbuild upgrades to fatal via -Werror -Wpedantic.
        export EMCC_CFLAGS="-fwasm-exceptions -sSUPPORT_LONGJMP=wasm -pthread -fPIC -Wno-overriding-option"
		scons platform=web compileTarget=extension target=template_release precision=double production=yes disable_exceptions=no
		scons platform=web compileTarget=extension target=template_debug precision=double disable_exceptions=no
        ;;
    web64)
        echo "Building GDExtension for Web (WASM, wasm64 / Memory64)..."
        echo "NOTE: requires a Godot engine built with MEMORY64=1; without it the .wasm"
        echo "      will compile but Godot's web export will refuse to load it."
        # Signal to CesiumBuildUtils.py: use wasm64-emscripten triplet, build-web64/
        # cmake tree, and the wasm64 output filename suffix.
        export CESIUM_WEB_MEMORY64=1
        ensure_emsdk
        # Same SIDE_MODULE flags as wasm32 plus -sMEMORY64=1 which switches clang/emcc
        # into Memory64 codegen (i64 wasm pointers, 64-bit size_t).
        # -Wno-experimental: emsdk emits a -Wexperimental warning on every MEMORY64
        # compile; some deps (KTX/astc-encoder) build with -Werror -Wpedantic which
        # upgrades it to a fatal error without this suppress.
        # -Wno-overriding-option: emsdk 3.1.74's clang flags `-ffp-model=precise`
        # + `-ffp-contract=off` (passed together by KTX's astc-encoder) as a
        # redundant override, which becomes fatal via -Werror -Wpedantic without
        # this suppress.
        export EMCC_CFLAGS="-fwasm-exceptions -sSUPPORT_LONGJMP=wasm -pthread -fPIC -sMEMORY64=1 -Wno-experimental -Wno-overriding-option"
		# arch=wasm64 routes through godot-cpp's tools/web.py (CESIUM-patched to
		# accept wasm64 and emit -sMEMORY64=1). Without it godot-cpp defaults to
		# wasm32 and builds its library with i32 table indices, which wasm-opt
		# --table64-lowering rejects against the i64-indexed link from
		# cesium-native + the extension.
		scons platform=web arch=wasm64 compileTarget=extension target=template_release precision=double production=yes disable_exceptions=no
		scons platform=web arch=wasm64 compileTarget=extension target=template_debug precision=double disable_exceptions=no
        ;;
    module)
        echo "Preparing module dependencies..."
        scons compileTarget=module buildCesium=yes
        ;;
    clean)
        echo "Cleaning cesium-native build trees..."
        rm -rf cesium_godot/native/build-windows
        rm -rf cesium_godot/native/build-web
        rm -rf cesium_godot/native/build-web64
        rm -rf cesium_godot/native/build-linux
        # SCons treats the godot-cpp lib as "up to date" once the .a file
        # exists, so flag changes (e.g. MEMORY64 added on emsdk bump) or ABI
        # shifts between emsdk versions don't trigger a rebuild. The lib then
        # ships stale wasm32-style call_indirects into the wasm64 link, which
        # the browser rejects at instantiation:
        #   "call_indirect[0] expected type i64, found i32.wrap_i64".
        # Same reasoning for pre-built litehtml/gumbo.
        rm -rf godot-cpp/bin
        rm -rf cesium_godot/third_party/litehtml/web
        rm -rf cesium_godot/third_party/litehtml/web64
        rm -rf cesium_godot/third_party/litehtml-src/build-web
        rm -rf cesium_godot/third_party/litehtml-src/build-web64
        echo "Done."
        ;;
    clean-deep)
        echo "Cleaning cesium-native build trees..."
        rm -rf cesium_godot/native/build-windows
        rm -rf cesium_godot/native/build-web
        rm -rf cesium_godot/native/build-web64
        rm -rf cesium_godot/native/build-linux
        # Wipe godot-cpp + pre-built litehtml caches — see clean for why.
        rm -rf godot-cpp/bin
        rm -rf cesium_godot/third_party/litehtml/web
        rm -rf cesium_godot/third_party/litehtml/web64
        rm -rf cesium_godot/third_party/litehtml-src/build-web
        rm -rf cesium_godot/third_party/litehtml-src/build-web64
        echo "Cleaning stale wasm32-emscripten and wasm64-emscripten vcpkg state..."
        if [ -n "${EZVCPKG_BASEDIR:-}" ] && [ -d "$EZVCPKG_BASEDIR" ]; then
            for d in "$EZVCPKG_BASEDIR"/*/; do
                [ -d "$d" ] || continue
                rm -rf "$d/installed/wasm32-emscripten"
                rm -rf "$d/installed/wasm64-emscripten"
                rm -f "$d/installed/vcpkg/info/"*_wasm32-emscripten.list 2>/dev/null
                rm -f "$d/installed/vcpkg/info/"*_wasm64-emscripten.list 2>/dev/null
                rm -rf "$d/buildtrees/ktx"
                # Strip orphaned wasm32/wasm64 stanzas from vcpkg's status file.
                # Without this, any subsequent vcpkg op fails with
                # "read_lines(...wasm*-emscripten.list): no such file or directory"
                # because status still references entries whose .list files we
                # just deleted.
                if [ -f "$d/installed/vcpkg/status" ]; then
                    python3 -c "import re,sys; p=sys.argv[1]; c=open(p,'r',encoding='utf-8').read(); k=[s for s in re.split(r'\n\n+',c) if not re.search(r'^Architecture:\s*wasm\d+-emscripten\s*$',s,re.MULTILINE)]; open(p,'w',encoding='utf-8').write('\n\n'.join(k))" "$d/installed/vcpkg/status" || true
                fi
            done
        fi
        echo "Done."
        ;;
    *)
        echo "Usage: ./build.sh [extension|web|web64|module|clean|clean-deep]"
        echo "  extension  - Build GDExtension for current platform (default)"
        echo "  web        - Build GDExtension for Web/WASM (wasm32, universal compat)"
        echo "  web64      - Build GDExtension for Web/WASM (wasm64 / Memory64, experimental)"
        echo "  module     - Prepare dependencies for Godot engine module build"
        echo "  clean      - Remove cesium-native build-* directories"
        echo "  clean-deep - clean + nuke stale wasm32/wasm64-emscripten vcpkg state (recovery)"
        exit 1
        ;;
esac
