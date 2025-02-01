# This file contains utility functions to build CesiumForGodot in SCons
import subprocess
import os
import fnmatch
import sys

from SCons.Script import Dir

ROOT_DIR_MODULE = "#modules/cesium_godot"

ROOT_DIR_EXT = "#cesium_godot"

BINDINGS_DIR = "#godot-cpp"

CESIUM_MODULE_DEF = "CESIUM_GD_MODULE"

CESIUM_EXT_DEF = "CESIUM_GD_EXT"

CESIUM_NATIVE_DIR_EXT = "#cesium_godot/native"

CESIUM_NATIVE_DIR_MODULE = "#modules/cesium_godot/native"

OS_WIN = "nt"


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
        print("Cesium Native repo already exists, skipping clone phase")
        return
    repoUrl = "https://github.com/CesiumGS/cesium-native.git"
    subprocess.run(["git", "clone", repoUrl, "--recursive", repoDirectory])


def clone_bindings_repo_if_needed():
    repoDirectory = _scons_to_abs_path(BINDINGS_DIR)
    if (os.path.exists(repoDirectory)):
        return
    repoUrl = "https://github.com/godotengine/godot-cpp"
    branchTag = "4.1"
    subprocess.run(["git", "clone", "-b", branchTag,
                   repoUrl, "--recursive", repoDirectory])


# Configure with CMake
def configure_native(argumentsDict):
    isExt = is_extension_target(argumentsDict)
    repoDirectory = CESIUM_NATIVE_DIR_EXT if isExt else CESIUM_NATIVE_DIR_MODULE
    repoDirectory = _scons_to_abs_path(repoDirectory)
    os.chdir(repoDirectory)
    # Assume you already have the triplet (for now)
    triplet = "x64-windows-static"
    os.environ["VCPKG_TRIPLET"] = triplet
    # Run Cmake with the /MT flag on
    result = subprocess.run(
        ["cmake", "-DCESIUM_MSVC_STATIC_RUNTIME_ENABLED=ON", "-DVCPKG_TRIPLET=%s" % triplet, "."])

    # We pray this works haha
    if result.returncode != 0:
        errorMsg = "cmake return code: %s" % str(result.returncode)
        print('Error configuring Cesium native, please make sure you have CMake installed and up to date: ' + errorMsg)
        return
    print("Configuration completed without any errors!")


def compile_native(argumentsDict):
    shouldBuildResponse = input(
        "Do you wanna build Cesium Native (Choose yes if it's the first install)? [y/n]")

    if shouldBuildResponse.capitalize()[0] != 'Y':
        return

    print("Building Cesium Native, this might take a few minutes...")
    configure_native(argumentsDict)
    print("Compiling Cesium Native...")
    if os.name != OS_WIN:
        print("Compiling for platform %s is not yet supported!" %
              os.name, file=sys.stderr)

    # execute MSBuild
    buildConfig: str = "Release"
    solutionName: str = "cesium-native.sln"
    msbuildPath: str = find_ms_build()
    if msbuildPath == '':
        print(
            "Could not find MSBuild.exe, make sure to have Visual Studio installed", file=sys.stderr)
        return
    releaseConfig = "/property:Configuration=%s" % buildConfig
    result = subprocess.run([msbuildPath, solutionName, releaseConfig])
    if result.returncode != 0:
        print("Error building Cesium Native: %s" % str(result.stderr))
    print("Finished building Cesium Native!")


def find_ms_build() -> str:
    print("Searching for MS Build")
    # Try to search for an msbuild executable in the system
    try:
        testCmd = subprocess.run(
            ["msbuild", "-version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        # Yay, we found it... (not gonna happen lol)
        if testCmd.returncode == 0:
            return 'msbuild'
    except:
        # More likely we'll need to search for another path
        vsPath = "C:\\Program Files\\Microsoft Visual Studio"
        found, path = find_in_dir_recursive(vsPath, '*MSBuild.exe')

        if found:
            # Access the next latest directory (latest VS version)
            return path

        # Try with a .NET path
        print(".NET path is not yet supported!", sys.stderr)
        return ''


def find_in_dir_recursive(path: str, pattern: str) -> (bool, str):
    """
    Use only when there might be a few directories left to search
    as this function is recursive
    """

    if not os.path.exists(path):
        return False, ''

    foundFiles: list[str] = os.listdir(path)

    if len(foundFiles) == 0:
        return False, ''

    for root, dirnames, filenames in os.walk(path):
        for filename in fnmatch.filter(filenames, pattern):
            return True, os.path.join(root, filename)

    return False, ''


def clone_engine_repo_if_needed():
    pass


def _scons_to_abs_path(path: str) -> str:
    return Dir(path).get_abspath()


def get_root_dir() -> str:
    return currentRootDir


def get_root_dir_native() -> str:
    return currentRootDir + "/native"
