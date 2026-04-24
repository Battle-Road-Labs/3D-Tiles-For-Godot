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
# 3.1.60 is pinned for this repo (bumped from 3.1.56 to pick up the
# -sSUPPORT_LONGJMP=wasm compile-time lowering fix). If 3.1.60 breaks KTX
# again (the original reason we pinned 3.1.56), step up gradually to 3.1.64 /
# 3.1.70 etc. Revert to 3.1.56 only as last resort — saveSetjmp workarounds
# there got messy. Kept in a separate directory from any other emsdk (e.g.
# the Godot engine's) so both can coexist with independent configs.
export EMSDK_DIR="${EMSDK_DIR:-$HOME/emsdk-cesium}"
export EMSDK_VERSION="${EMSDK_VERSION:-3.1.60}"

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
        echo "Building GDExtension for Web (WASM)..."
        ensure_emsdk
        # Force emscripten-style longjmp to avoid conflict with godot-cpp's exception handling.
		# -pthread enables atomics/bulk-memory needed for --shared-memory at link time.
		# EMCC_CFLAGS is read by emcc/em++ for ALL compilations (vcpkg, cmake, scons).
		# -fwasm-exceptions: native wasm exception handling (no JS invoke_* wrappers)
		# -sSUPPORT_LONGJMP=wasm: native wasm longjmp (pairs with -fwasm-exceptions)
		# -pthread -fPIC: required for threaded SIDE_MODULE builds
        export EMCC_CFLAGS="-fwasm-exceptions -sSUPPORT_LONGJMP=wasm -pthread -fPIC"
		scons platform=web compileTarget=extension target=template_release precision=double production=yes disable_exceptions=no
		scons platform=web compileTarget=extension target=template_debug precision=double disable_exceptions=no
        ;;
    module)
        echo "Preparing module dependencies..."
        scons compileTarget=module buildCesium=yes
        ;;
    clean)
        echo "Cleaning cesium-native build trees..."
        rm -rf cesium_godot/native/build-windows
        rm -rf cesium_godot/native/build-web
        rm -rf cesium_godot/native/build-linux
        echo "Done."
        ;;
    clean-deep)
        echo "Cleaning cesium-native build trees..."
        rm -rf cesium_godot/native/build-windows
        rm -rf cesium_godot/native/build-web
        rm -rf cesium_godot/native/build-linux
        echo "Cleaning stale wasm32-emscripten vcpkg state..."
        if [ -n "${EZVCPKG_BASEDIR:-}" ] && [ -d "$EZVCPKG_BASEDIR" ]; then
            for d in "$EZVCPKG_BASEDIR"/*/; do
                [ -d "$d" ] || continue
                rm -rf "$d/installed/wasm32-emscripten"
                rm -f "$d/installed/vcpkg/info/"*_wasm32-emscripten.list 2>/dev/null
                rm -rf "$d/buildtrees/ktx"
            done
        fi
        echo "Done."
        ;;
    *)
        echo "Usage: ./build.sh [extension|web|module|clean|clean-deep]"
        echo "  extension  - Build GDExtension for current platform (default)"
        echo "  web        - Build GDExtension for Web/WASM (requires EMSDK)"
        echo "  module     - Prepare dependencies for Godot engine module build"
        echo "  clean      - Remove cesium-native build-* directories"
        echo "  clean-deep - clean + nuke stale wasm32-emscripten vcpkg state (recovery)"
        exit 1
        ;;
esac
