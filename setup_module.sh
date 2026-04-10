#!/usr/bin/env bash
set -euo pipefail

# Sets up 3D-Tiles-For-Godot as a Godot engine module by creating symlinks
# from a Godot source tree into this repository.
#
# Usage:
#   ./setup_module.sh /path/to/godot-source
#
# After running this script:
#   1. Build Cesium Native dependencies (if not already done):
#        ./build.sh module        (or:  scons compileTarget=module buildCesium=yes)
#   2. Build the engine from the Godot source root:
#        cd /path/to/godot-source
#        scons platform=<platform> target=<target>

if [ $# -lt 1 ]; then
    echo "Usage: $0 <path-to-godot-source>"
    echo ""
    echo "Creates symlinks in <godot-source>/modules/cesium_godot/ pointing"
    echo "back to this repository so the module is compiled into the engine."
    exit 1
fi

GODOT_SRC="$(cd "$1" && pwd)"
MODULE_DIR="$GODOT_SRC/modules/cesium_godot"
REPO_DIR="$(cd "$(dirname "$0")" && pwd)"

if [ ! -f "$GODOT_SRC/SConstruct" ]; then
    echo "Error: $GODOT_SRC does not look like a Godot source tree (no SConstruct found)."
    exit 1
fi

echo "Godot source: $GODOT_SRC"
echo "Plugin repo:  $REPO_DIR"
echo ""

# Create the module directory
mkdir -p "$MODULE_DIR"

# Symlink the core module files
for item in SCsub config.py register_types.h register_types.cpp \
            Models Implementations Utils Shaders \
            CesiumGDModelLoader.h CesiumGDModelLoader.cpp \
            native third_party; do
    src="$REPO_DIR/cesium_godot/$item"
    dst="$MODULE_DIR/$item"
    if [ -e "$src" ]; then
        if [ -e "$dst" ] || [ -L "$dst" ]; then
            echo "  (exists) $item"
        else
            ln -s "$src" "$dst"
            echo "  (linked) $item -> $src"
        fi
    else
        echo "  (skip)   $item  (not found in repo)"
    fi
done

# Symlink CesiumBuildUtils.py into the module dir so the SCsub can import it
for util in CesiumBuildUtils.py; do
    src="$REPO_DIR/$util"
    dst="$MODULE_DIR/$util"
    if [ -e "$dst" ] || [ -L "$dst" ]; then
        echo "  (exists) $util"
    else
        ln -s "$src" "$dst"
        echo "  (linked) $util -> $src"
    fi
done

# Symlink cesium_auxiliars into the module dir
src="$REPO_DIR/cesium_auxiliars"
dst="$MODULE_DIR/cesium_auxiliars"
if [ -e "$dst" ] || [ -L "$dst" ]; then
    echo "  (exists) cesium_auxiliars"
else
    ln -s "$src" "$dst"
    echo "  (linked) cesium_auxiliars -> $src"
fi

echo ""
echo "Module setup complete."
echo ""
echo "Next steps:"
echo "  1. Build Cesium Native (if first time):"
echo "       cd $REPO_DIR"
echo "       ./build.sh module"
echo ""
echo "  2. Build the Godot engine:"
echo "       cd $GODOT_SRC"
echo "       scons platform=<platform> target=editor"
echo ""
echo "  The cesium_godot module will be automatically detected and compiled"
echo "  into the engine binary."
