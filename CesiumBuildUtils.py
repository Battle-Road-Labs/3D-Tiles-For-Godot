# This file contains utility functions to build CesiumForGodot in SCons
import subprocess
import os

from SCons.Script import Dir

ROOT_DIR_MODULE = "#modules/cesium_godot"

ROOT_DIR_EXT = "#cesium_godot"

BINDINGS_DIR = "#godot-cpp"

CESIUM_MODULE_DEF = "CESIUM_GD_MODULE"

CESIUM_EXT_DEF = "CESIUM_GD_EXT"


def is_extension_target(argsDict) -> bool:
    return get_compile_target_definition(argsDict) == CESIUM_EXT_DEF


def get_compile_target_definition(argsDict) -> str:
    # Get the format (default is extension)
    global currentRootDir
    compileTarget = argsDict.get("compileTarget", CESIUM_EXT_DEF)
    if (compileTarget == "module"):
        print("[CESIUM] - Compiling Cesium For Godot as an engine module...")
        currentRootDir = ROOT_DIR_MODULE
        return CESIUM_MODULE_DEF
    if (compileTarget == "" or compileTarget == "extension"):
        print("[CESIUM] - Compiling Cesium For Godot as a GDExtension")
        currentRootDir = ROOT_DIR_EXT
        return CESIUM_EXT_DEF

    print("[CESIUM] - Compile target not recognized, options are: module / extension")
    exit(1)


def clone_native_repo_if_needed():
    repoDirectory = _scons_to_abs_path(ROOT_DIR_EXT + "/native")
    if (os.path.exists(repoDirectory)):
        return
    repoUrl = "https://github.com/CesiumGS/cesium-native.git"
    subprocess.run(["git", "clone", repoUrl, "--recursive", repoDirectory])


def compile_native():
    # Check if the libs are present
    pass


def clone_bindings_repo_if_needed():
    repoDirectory = _scons_to_abs_path(BINDINGS_DIR)
    if (os.path.exists(repoDirectory)):
        return
    repoUrl = "https://github.com/godotengine/godot-cpp"
    branchTag = "4.1"
    subprocess.run(["git", "clone", "-b", branchTag,
                   repoUrl, "--recursive", repoDirectory])


def clone_engine_repo_if_needed():
    pass


def _scons_to_abs_path(path: str) -> str:
    return Dir(path).get_abspath()


def get_root_dir() -> str:
    return currentRootDir


def get_root_dir_native() -> str:
    return currentRootDir + "/native"
