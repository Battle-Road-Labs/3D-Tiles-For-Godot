#!/usr/bin/env bash
set -euo pipefail

TARGET="${1:-extension}"

case "$TARGET" in
    extension)
        echo "Building GDExtension for $(uname -s) x64..."
        scons arch=x86_64 compileTarget=extension target=template_release precision=double production=yes compiledb=yes
		scons arch=x86_64 compileTarget=extension target=template_debug precision=double compiledb=yes
        ;;
    web)
        echo "Building GDExtension for Web (WASM)..."
        if [ -z "${EMSDK:-}" ]; then
            echo "Error: EMSDK environment variable not set."
            echo "Please install and activate the Emscripten SDK first:"
            echo "  source <emsdk-path>/emsdk_env.sh"
            exit 1
        fi
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
