#!/usr/bin/env bash
set -euo pipefail

TARGET="${1:-extension}"

case "$TARGET" in
    extension)
        echo "Building GDExtension for $(uname -s) x64..."
        scons arch=x86_64 compileTarget=extension target=template_release precision=double production=yes compiledb=yes
        ;;
    web)
        echo "Building GDExtension for Web (WASM)..."
        if [ -z "${EMSDK:-}" ]; then
            echo "Error: EMSDK environment variable not set."
            echo "Please install and activate the Emscripten SDK first:"
            echo "  source <emsdk-path>/emsdk_env.sh"
            exit 1
        fi
        scons platform=web compileTarget=extension target=template_release precision=double production=yes
        ;;
    module)
        echo "Preparing module dependencies..."
        scons compileTarget=module buildCesium=yes
        ;;
    *)
        echo "Usage: ./build.sh [extension|web|module]"
        echo "  extension  - Build GDExtension for current platform (default)"
        echo "  web        - Build GDExtension for Web/WASM (requires EMSDK)"
        echo "  module     - Prepare dependencies for Godot engine module build"
        exit 1
        ;;
esac
