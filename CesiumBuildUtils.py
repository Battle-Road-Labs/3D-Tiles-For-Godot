# This file contains utility functions to build CesiumForGodot in SCons
import subprocess
import os
import fnmatch
import sys

from SCons.Script import Dir

ROOT_DIR_MODULE = "#modules/cesium_godot/cesium_godot"

ROOT_DIR_EXT = "#cesium_godot"

BINDINGS_DIR = "#godot-cpp"

CESIUM_MODULE_DEF = "CESIUM_GD_MODULE"

CESIUM_EXT_DEF = "CESIUM_GD_EXT"

CESIUM_NATIVE_DIR_EXT = "#cesium_godot/native"

CESIUM_NATIVE_DIR_MODULE = "#modules/cesium_godot/native"

OS_WIN = "nt"

OS_LINUX = "posix"

STATIC_TRIPLET = "x64-windows-static"

RELEASE_CONFIG = "Release"

ezvcpkgFoundPath: str = ""


def get_compile_flags():
    if os.name == OS_WIN:
        return ["/std:c++20", "/Zc:__cplusplus", "/utf-8", "/bigobj"]
    elif os.name == OS_LINUX:
        return ["-std=c++20", "-fexceptions", "-fpermissive", "-fPIC"]


def get_compile_defs(argsDict):
    compilationTarget: str = get_compile_target_definition(
        argsDict)
    if os.name == OS_LINUX:
        return ["CURL_STATIC_LIB", "SQLITE_STATIC", compilationTarget]
    return [compilationTarget]


def get_include_paths() -> list[str]:
    return [get_root_dir(), get_root_dir() + "/cesium_auxiliars"]

def get_linker_flags():
    if os.name == OS_WIN:
        return ["/IGNORE:4217"]
    return []


def is_extension_target(argsDict) -> bool:
    return get_compile_target_definition(argsDict) == CESIUM_EXT_DEF


def get_curl_lib_name() -> str:
    if os.name == OS_WIN:
        return "libcurl"
    return "curl"


def add_compile_definitions(p_env, argsDict):
    generate_precision_symbols(argsDict, p_env)
    p_env.Append(CXXFLAGS=get_compile_flags())
    p_env.Append(LINKFLAGS=get_linker_flags())
    p_env.Append(CPPDEFINES=[get_compile_defs(argsDict)])


def generate_precision_symbols(argsDict, env):
    print("Generating double precision compile symbols")
    desiredPrecision = argsDict.get("precision")
    if (desiredPrecision == "double"):
        env.Append(CPPDEFINES=["REAL_T_IS_DOUBLE"])


def get_compile_target_definition(argsDict) -> str:
    # Get the format (default is extension)
    global currentRootDir
    compileTarget = argsDict.get("compileTarget", CESIUM_EXT_DEF)
    if (compileTarget == "module"):
        currentRootDir = ROOT_DIR_MODULE
        return CESIUM_MODULE_DEF
    if (compileTarget == "extension"):
        currentRootDir = ROOT_DIR_EXT
        return CESIUM_EXT_DEF

    print("[3D-Tiles-For-Godot] - compileTarget not recognized, options are: module / extension, defaulting to module")
    currentRootDir = ROOT_DIR_MODULE
    return CESIUM_MODULE_DEF


def link_abseil_libs(env):
    foundLibs: list[SCons.Node.FS.File] = env.Glob(
        f"{find_ezvcpkg_path()}/packages/abseil_{determine_triplet()}/lib/*absl*.a")

    # Dark magic to strip the lib prefix and the file extension
    foundLibs = [lib.name.replace("lib", "")[:-2] for lib in foundLibs]

    env.Append(LINKFLAGS=['-Wl,--start-group'], LIBS=foundLibs)

    env.Append(LINKFLAGS=['-Wl,--end-group'])

    env.Append(LINKFLAGS=['-Wl,--start-group'], LIBS=["absl_log_internal_log_sink_set", "absl_log_globals", "absl_leak_check", "absl_log_internal_globals", "absl_log_internal_format", "absl_base", "absl_hash", "absl_city", "absl_low_level_hash", "absl_examine_stack", "absl_stacktrace",
               "absl_debugging_internal", "absl_synchronization", "absl_base", "absl_malloc_internal", "absl_int128", "absl_symbolize", "absl_kernel_timeout_internal", "absl_debugging_internal", "absl_demangle_internal", "absl_log_sink", "absl_demangle_rust", "absl_decode_rust_punycode", "absl_utf8_for_code_point"])
    env.Append(LINKFLAGS=['-Wl,--end-group'])


def clone_native_repo_if_needed():
    clone_repo_if_needed(ROOT_DIR_EXT + "/native", "Cesium Native",
                         "https://github.com/CesiumGS/cesium-native.git", "v0.48.0", "95498106dbac35de7ca87bc52c926a94b2091938")


def clone_bindings_repo_if_needed():
    clone_repo_if_needed(BINDINGS_DIR, "Godot CPP Bindings", "https://github.com/godotengine/godot-cpp",
                         "4.1", "6388e26dd8a42071f65f764a3ef3d9523dda3d6e")


def clone_lite_html_if_needed():
    # clone_repo_if_needed(ROOT_DIR_EXT + "/third_party/lite-html", "Lite HTML",
    #                      "https://github.com/litehtml/litehtml.git", "v0.9", "6ca1ab0419e770e6d35a1ef690238773a1dafcee")
    pass


def clone_repo_if_needed(targetDir: str, name: str, repoUrl: str, branch: str, acceptedCommitSHA: str):
    print(f"Cloning {name} repo")
    repoDirectory = scons_to_abs_path(targetDir)
    if (os.path.exists(repoDirectory)):
        return
    subprocess.run(["git", "clone", "-b", branch,
                   repoUrl, "--recursive", repoDirectory])

    prevDir: str = os.getcwd()
    os.chdir(repoDirectory)
    subprocess.run(["git", "reset", "--hard", acceptedCommitSHA])
    os.chdir(prevDir)


# Configure with CMake
def configure_native(argumentsDict):
    print("Configuring Cesium Native")
    isExt = is_extension_target(argumentsDict)
    repoDirectory = CESIUM_NATIVE_DIR_EXT if isExt else CESIUM_NATIVE_DIR_MODULE
    repoDirectory = scons_to_abs_path(repoDirectory)
    os.chdir(repoDirectory)
    # Assume you already have the triplet (for now)
    triplet: str = determine_triplet()
    os.environ["VCPKG_TRIPLET"] = triplet
    # Run Cmake with the /MT flag on
    result = subprocess.run(
        ["cmake", f"-DCMAKE_BUILD_TYPE={RELEASE_CONFIG}", "-DCESIUM_MSVC_STATIC_RUNTIME_ENABLED=ON", '-DCMAKE_POLICY_VERSION_MINIMUM="3.5"',  "-DGIT_LFS_SKIP_SMUDGE=1", "-DVCPKG_TRIPLET=%s" % triplet, "."])

    # We pray this works haha
    if result.returncode != 0:
        errorMsg = "cmake return code: %s" % str(result.returncode)
        print('Error configuring Cesium native, please make sure you have CMake installed and up to date: ' + errorMsg)
        exit(1)
    print("Configuration completed without any errors!")


def determine_triplet():
    if (os.name == OS_WIN):
        return "x64-windows-static"
    if (os.name == OS_LINUX):
        return "x64-linux"


def compile_native(argumentsDict):
    shouldBuildArg = argumentsDict.get("buildCesium", None)
    if shouldBuildArg is None:
        shouldBuildResponse = input(
            "Do you wanna build Cesium Native (Choose yes if it's the first install)? [y/n]")
        shouldBuildArg = shouldBuildResponse.capitalize()[0] == 'Y'
    else:
        shouldBuildArg = shouldBuildArg.upper() == "YES" or shouldBuildArg.upper() == "TRUE"

    if not shouldBuildArg:
        return

    print("Building Cesium Native, this might take a few minutes...")
    configure_native(argumentsDict)
    print("Compiling Cesium Native...")

    # TODO: Test if we can just do cmake --build for all platforms
    result = None
    if os.name == OS_WIN:
        result = build_native_win()
    elif os.name == OS_LINUX:
        result = build_native_linux()
    else:
        print("Compiling for platform %s is not yet supported!" %
              os.name, file=sys.stderr)
    if result.returncode != 0:
        print("Error building Cesium Native: %s" % str(result.stderr))
    print("Cleaning definitions on generated files...")
    clean_cesium_definitions()
    print("Finished building Cesium Native!")


def build_native_linux():
    return subprocess.run(["cmake", "--build", "."])


def build_native_win():
    # execute MSBuild
    buildConfig: str = RELEASE_CONFIG
    solutionName: str = "cesium-native.sln"
    msbuildPath: str = find_ms_build()
    if msbuildPath == '':
        print(
            "Could not find MSBuild.exe, make sure to have Visual Studio installed", file=sys.stderr)
        return
    releaseConfig = "/property:Configuration=%s" % buildConfig
    return subprocess.run([msbuildPath, solutionName, releaseConfig])


def clean_cesium_definitions():
    """
    This function modifies some of Cesium's header files to clean up
    definitions that conflict with the engine's
    """
    # Get the conflicting file (Material.h in our case)
    print("Cleaning native definitions")

    conflictFilePath: str = "%s/%s" % (CESIUM_NATIVE_DIR_EXT,
                                       "/CesiumGltf/generated/include/CesiumGltf")
    conflictFilePath = scons_to_abs_path(conflictFilePath) + "/Material.h"
    # Load the file into memory

    # Read in the file
    fileData: str = ""
    with open(conflictFilePath, 'r') as file:
        fileData = file.read()

    # Replace the target string
    fileData = fileData.replace('#pragma once', '#pragma once\n#undef OPAQUE')

    # Write the file out again
    with open(conflictFilePath, 'w') as file:
        file.write(fileData)
    print("Finished cleaning native definitions")


def install_additional_libs():
    print("Installing additional libraries")
    vcpkgPath = find_ezvcpkg_path()
    execExtension = ".exe" if os.name == OS_WIN else ""
    executable = "%s/%s" % (vcpkgPath, "vcpkg" + execExtension)
    subprocess.run([executable, "install", "uriparser:%s" %
                   (determine_triplet())])
    subprocess.run([executable, "install", "ada-url:%s" %
                   (determine_triplet())])
    if os.name == OS_WIN:
        subprocess.run([executable, "install", "curl:%s" %
                       (determine_triplet())])


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


def find_ezvcpkg_path() -> str:
    global ezvcpkgFoundPath
    if (ezvcpkgFoundPath != ""):
        return ezvcpkgFoundPath
    # Search the home directory
    assumedPath = "%s.ezvcpkg" % (os.path.abspath(os.sep))
    print(f"Searching vcpkg at: {assumedPath}")
    if (not os.path.exists(assumedPath)):
        from pathlib import Path
        assumedPath = (Path.home() / ".ezvcpkg").as_posix()
        print(f"Searching vcpkg at: {assumedPath}")
        if (not os.path.exists(assumedPath)):
            print(
                "EZVCPKG not found, please make sure that CesiumNative was compiled and configured properly!")
            return ""
        # Assume it is in /home (C:/Users/currUser)
    # Then find the latest version (use the last created folder)
    subDirs = [x for x in next(os.walk(assumedPath))[1]]
    subDirs.sort(reverse=True, key=lambda x: os.stat(
        "%s/%s" % (assumedPath, x)).st_ctime)
    latestDir = subDirs[0]
    ezvcpkgFoundPath = "%s/%s" % (assumedPath, latestDir)
    print(f"Found ezvcpkg at {ezvcpkgFoundPath}")
    return ezvcpkgFoundPath


def clone_engine_repo_if_needed():
    pass


def scons_to_abs_path(path: str) -> str:
    return Dir(path).get_abspath()


def find_ezvcpkg_include_path() -> str:
    return f"{find_ezvcpkg_path()}/installed/{determine_triplet()}/include"


def find_ezvcpkg_lib_path() -> str:
    return f"{find_ezvcpkg_path()}/installed/{determine_triplet()}/lib"


def get_root_dir() -> str:
    return currentRootDir


def get_root_dir_native() -> str:
    return scons_to_abs_path(currentRootDir + "/native")
