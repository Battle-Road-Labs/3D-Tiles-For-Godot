# This file contains utility functions to build CesiumForGodot in SCons
import subprocess
import os
import fnmatch
import shutil
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

OS_LINUX = "posix"

# sys.platform value for macOS (os.name returns 'posix' for both Linux and macOS)
PLATFORM_MACOS = "darwin"

PLATFORM_WEB = "web"

STATIC_TRIPLET = "x64-windows-static"

RELEASE_CONFIG = "Release"

ezvcpkgFoundPath: str = ""

# Default to extension root dir; overwritten by get_compile_target_definition() or
# set_root_dir_for_module() when building as a module.
currentRootDir: str = ROOT_DIR_EXT


def is_web_platform(env=None):
    """Check if we are targeting the web/Emscripten platform."""
    if env is not None:
        return env.get("platform", "") == PLATFORM_WEB
    return False


def get_compile_flags(env=None):
    if is_web_platform(env):
        return ["-std=c++20", "-fexceptions", "-fPIC", "-pthread"]
    if os.name == OS_WIN:
        return ["/std:c++20", "/Zc:__cplusplus", "/utf-8", "/bigobj"]
    elif sys.platform == PLATFORM_MACOS:
        return ["-std=c++20", "-fexceptions", "-fPIC"]
    elif os.name == OS_LINUX:
        return ["-std=c++20", "-fexceptions", "-fpermissive", "-fPIC"]


def get_linker_flags(env=None):
    if is_web_platform(env):
        return [
            "-sSIDE_MODULE=1",
            "-pthread",
            "-sPTHREAD_POOL_SIZE=4",
            "-sALLOW_MEMORY_GROWTH=1",
        ]
    if os.name == OS_WIN:
        return ["/IGNORE:4217"]
    return []


def is_extension_target(argsDict) -> bool:
    return get_compile_target_definition(argsDict) == CESIUM_EXT_DEF


def get_curl_lib_name(env=None) -> str:
    if is_web_platform(env):
        # On web, curl is not available; networking goes through browser fetch.
        # Return empty string so callers can filter it out.
        return ""
    if os.name == OS_WIN:
        return "libcurl"
    return "curl"


def generate_precision_symbols(argsDict, env):
    print("Generating double precision compile symbols")
    desiredPrecision = argsDict.get("precision")
    if desiredPrecision == "double":
        env.Append(CPPDEFINES=["REAL_T_IS_DOUBLE"])


def set_module_context():
    """Explicitly configure CesiumBuildUtils for module builds.
    Called by the SCsub when it detects it's running inside the Godot engine
    source tree (i.e., not through our SConstruct.py)."""
    global currentRootDir
    currentRootDir = ROOT_DIR_MODULE
    print("[CESIUM] - Configured for engine module build (root: %s)" % currentRootDir)


def get_compile_target_definition(argsDict) -> str:
    # Get the format (default is extension)
    global currentRootDir
    compileTarget = argsDict.get("compileTarget", CESIUM_EXT_DEF)
    if compileTarget == "module":
        print("[CESIUM] - Compiling Cesium For Godot as an engine module...")
        currentRootDir = ROOT_DIR_MODULE
        return CESIUM_MODULE_DEF
    if compileTarget == "" or compileTarget == "extension":
        print("[CESIUM] - Compiling Cesium For Godot as a GDExtension")
        currentRootDir = ROOT_DIR_EXT
        return CESIUM_EXT_DEF

    print("[CESIUM] - Compile target not recognized, options are: module / extension")
    exit(1)


def link_abseil_libs(env):
    foundLibs: list[SCons.Node.FS.File] = env.Glob(
        f"{find_ezvcpkg_path()}/packages/abseil_{determine_triplet()}/lib/*absl*.a"
    )

    # Dark magic to strip the lib prefix and the file extension
    foundLibs = [lib.name.replace("lib", "")[:-2] for lib in foundLibs]

    env.Append(LINKFLAGS=["-Wl,--start-group"], LIBS=foundLibs)

    env.Append(LINKFLAGS=["-Wl,--end-group"])

    env.Append(
        LINKFLAGS=["-Wl,--start-group"],
        LIBS=[
            "absl_log_internal_log_sink_set",
            "absl_log_globals",
            "absl_leak_check",
            "absl_log_internal_globals",
            "absl_log_internal_format",
            "absl_base",
            "absl_hash",
            "absl_city",
            "absl_low_level_hash",
            "absl_examine_stack",
            "absl_stacktrace",
            "absl_debugging_internal",
            "absl_synchronization",
            "absl_base",
            "absl_malloc_internal",
            "absl_int128",
            "absl_symbolize",
            "absl_kernel_timeout_internal",
            "absl_debugging_internal",
            "absl_demangle_internal",
            "absl_log_sink",
            "absl_demangle_rust",
            "absl_decode_rust_punycode",
            "absl_utf8_for_code_point",
        ],
    )
    env.Append(LINKFLAGS=["-Wl,--end-group"])


def clone_native_repo_if_needed():
    clone_repo_if_needed(
        ROOT_DIR_EXT + "/native",
        "Cesium Native",
        "https://github.com/CesiumGS/cesium-native.git",
        "v0.52.1",
        "9f6ae299e2709f866db52c4be29b6c31e10718c8",
    )


def clone_bindings_repo_if_needed():
    clone_repo_if_needed(
        BINDINGS_DIR,
        "Godot CPP Bindings",
        "https://github.com/godotengine/godot-cpp",
        "godot-4.1.4-stable",
        "4b0ee133274d67687b6003b8d5fdaf7b79cf4921",
    )
    # Always run the patch — clone_repo_if_needed skips if dir exists,
    # but the patch is idempotent and needs to be applied regardless.
    patch_godot_cpp_web_flags()


def patch_godot_cpp_web_flags():
    """Patch godot-cpp's web.py to use emscripten-style longjmp instead of wasm-style.

    Newer Emscripten (3.1.56+) defaults to emscripten-style C++ exception handling,
    which conflicts with wasm-style setjmp/longjmp. Force both to use the emscripten
    backend so they don't clash."""
    web_py_path = os.path.join(scons_to_abs_path(BINDINGS_DIR), "tools", "web.py")
    if not os.path.exists(web_py_path):
        return

    with open(web_py_path, "r") as f:
        content = f.read()

    patched = content.replace(
        "-sSUPPORT_LONGJMP='wasm'",
        "-sSUPPORT_LONGJMP='emscripten'"
    )

    if patched != content:
        with open(web_py_path, "w") as f:
            f.write(patched)
        print("[CESIUM] Patched godot-cpp web.py for Emscripten compatibility")


def clone_lite_html_if_needed():
    # clone_repo_if_needed(ROOT_DIR_EXT + "/third_party/lite-html", "Lite HTML",
    #                      "https://github.com/litehtml/litehtml.git", "v0.9", "6ca1ab0419e770e6d35a1ef690238773a1dafcee")
    pass


def build_litehtml(arch="arm64"):
    """Build litehtml from source for the given architecture."""
    third_party_dir = scons_to_abs_path(ROOT_DIR_EXT + "/third_party")
    source_dir = os.path.join(third_party_dir, "litehtml-src")
    output_dir = os.path.join(third_party_dir, "litehtml", "macos")

    # Check if already built
    if os.path.exists(os.path.join(output_dir, "liblitehtml.a")):
        print("litehtml already built for macOS, skipping...")
        return

    if not os.path.exists(source_dir):
        print("litehtml source not found at %s" % source_dir, file=sys.stderr)
        return

    print("Building litehtml from source for macOS...")

    build_dir = os.path.join(source_dir, "build-macos")
    os.makedirs(build_dir, exist_ok=True)
    os.makedirs(output_dir, exist_ok=True)

    prev_dir = os.getcwd()
    os.chdir(build_dir)

    # Configure with CMake
    result = subprocess.run([
        "cmake",
        "-DCMAKE_BUILD_TYPE=Release",
        f"-DCMAKE_OSX_ARCHITECTURES={arch}",
        "-DLITEHTML_BUILD_TESTING=OFF",
        ".."
    ])

    if result.returncode != 0:
        print("Failed to configure litehtml", file=sys.stderr)
        os.chdir(prev_dir)
        return

    # Build
    result = subprocess.run(["cmake", "--build", ".", "--config", "Release"])

    if result.returncode != 0:
        print("Failed to build litehtml", file=sys.stderr)
        os.chdir(prev_dir)
        return

    # Copy output libraries
    for lib in ["liblitehtml.a", "libgumbo.a"]:
        src = os.path.join(build_dir, lib)
        if not os.path.exists(src):
            # Try in subdirectories
            for root, dirs, files in os.walk(build_dir):
                if lib in files:
                    src = os.path.join(root, lib)
                    break
        if os.path.exists(src):
            shutil.copy2(src, output_dir)
            print(f"Copied {lib} to {output_dir}")

    os.chdir(prev_dir)
    print("litehtml build complete!")


def clone_repo_if_needed(
    targetDir: str, name: str, repoUrl: str, branch: str, acceptedCommitSHA: str
):
    print(f"Cloning {name} repo")
    repoDirectory = scons_to_abs_path(targetDir)
    if os.path.exists(repoDirectory):
        return
    subprocess.run(
        ["git", "clone", "--depth=1", "-b", branch, repoUrl, "--recursive", repoDirectory]
    )

    # Shouldn't we just rely on the repo tags?
    # prevDir: str = os.getcwd()
    # os.chdir(repoDirectory)
    # subprocess.run(["git", "reset", "--hard", acceptedCommitSHA])
    # os.chdir(prevDir)


# Configure with CMake (out-of-tree build per platform)
def configure_native(argumentsDict):
    print("Configuring Cesium Native")
    isExt = is_extension_target(argumentsDict)
    nativeDir = CESIUM_NATIVE_DIR_EXT if isExt else CESIUM_NATIVE_DIR_MODULE
    sourceDir = scons_to_abs_path(nativeDir)

    platform = argumentsDict.get("platform", "")
    is_web = platform == PLATFORM_WEB
    triplet: str = determine_triplet_for_args(argumentsDict)
    os.environ["VCPKG_TRIPLET"] = triplet

    # Create platform-specific build directory
    build_dir_name = get_native_build_dir_name(platform)
    buildDir = os.path.join(sourceDir, build_dir_name)
    os.makedirs(buildDir, exist_ok=True)

    if is_web:
        patch_ezvcpkg_allow_unsupported(sourceDir)

    cmake_args = [
        "cmake",
        "-S", sourceDir,
        "-B", buildDir,
        f"-DCMAKE_BUILD_TYPE={RELEASE_CONFIG}",
        "-DCESIUM_MSVC_STATIC_RUNTIME_ENABLED=ON",
        "-DCESIUM_TESTS_ENABLED=OFF",
        "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
        "-DGIT_LFS_SKIP_SMUDGE=1",
        "-DVCPKG_TRIPLET=%s" % triplet,
        "-DVCPKG_TARGET_TRIPLET=%s" % triplet,
    ]

    # If ezvcpkg has already been set up from a previous configure, pass the
    # vcpkg toolchain file on the command line so cmake loads it from the very
    # start. Without this, the first configure runs find_package() before the
    # toolchain is active, caching paths from the wrong triplet directory.
    try:
        vcpkg_base = find_ezvcpkg_path()
        vcpkg_toolchain = os.path.join(vcpkg_base, "scripts", "buildsystems", "vcpkg.cmake")
        if os.path.exists(vcpkg_toolchain):
            cmake_args.append(f"-DCMAKE_TOOLCHAIN_FILE={vcpkg_toolchain}")
            print(f"[CESIUM] Using vcpkg toolchain: {vcpkg_toolchain}")
    except Exception:
        pass  # Fresh build — ezvcpkg will set up the toolchain during configure

    if is_web:
        # Use Emscripten toolchain chainloaded through vcpkg
        emsdk = os.environ.get("EMSDK", "")
        if not emsdk:
            print(
                "Error: EMSDK environment variable not set. "
                "Please install and activate the Emscripten SDK first.",
                file=sys.stderr,
            )
            exit(1)
        toolchain = os.path.join(
            emsdk, "upstream", "emscripten", "cmake", "Modules", "Platform", "Emscripten.cmake"
        )
        node_path = find_emsdk_node(emsdk)
        cmake_args.extend([
            "-G", "Ninja",
            f"-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE={toolchain}",
            f"-DCMAKE_CROSSCOMPILING_EMULATOR={node_path}",
        ])

    print(f"[CESIUM] Build directory: {buildDir}")
    result = subprocess.run(cmake_args)

    if result.returncode != 0:
        errorMsg = "cmake return code: %s" % str(result.returncode)
        print(
            "Error configuring Cesium native, please make sure you have CMake installed and up to date: "
            + errorMsg
        )
        exit(1)
    print("Configuration completed without any errors!")


def determine_triplet(env=None):
    if is_web_platform(env):
        return "wasm32-emscripten"
    if os.name == OS_WIN:
        return "x64-windows-static"
    if sys.platform == PLATFORM_MACOS:
        return "arm64-osx"
    if os.name == OS_LINUX:
        return "x64-linux"


def determine_triplet_for_args(argumentsDict):
    """Determine the vcpkg triplet based on SCons arguments."""
    platform = argumentsDict.get("platform", "")
    if platform == PLATFORM_WEB:
        return "wasm32-emscripten"
    return determine_triplet()


def get_native_build_dir_name(platform_str=""):
    """Return the build subdirectory name for the given target platform.

    Each platform gets its own out-of-tree CMake build directory so that
    multiple platform builds can coexist under cesium_godot/native/."""
    if platform_str == PLATFORM_WEB:
        return "build-web"
    if os.name == OS_WIN:
        return "build-windows"
    if sys.platform == PLATFORM_MACOS:
        return "build-macos"
    if os.name == OS_LINUX:
        return "build-linux"
    return "build"


def get_native_build_path(env=None):
    """Return the absolute path to the platform-specific cesium-native build directory."""
    platform_str = env.get("platform", "") if env is not None else ""
    build_dir_name = get_native_build_dir_name(platform_str)
    return os.path.join(scons_to_abs_path(currentRootDir + "/native"), build_dir_name)


def patch_ezvcpkg_allow_unsupported(native_source_dir):
    """Patch ezvcpkg.cmake to allow unsupported vcpkg triplets (needed for wasm32-emscripten)."""
    ezvcpkg_cmake = os.path.join(native_source_dir, "cmake", "ezvcpkg", "ezvcpkg.cmake")
    if not os.path.exists(ezvcpkg_cmake):
        return
    with open(ezvcpkg_cmake, "r") as f:
        content = f.read()
    if "--allow-unsupported" in content:
        return  # Already patched
    patched = content.replace("install --triplet", "install --allow-unsupported --triplet")
    if patched != content:
        with open(ezvcpkg_cmake, "w") as f:
            f.write(patched)
        print("[CESIUM] Patched ezvcpkg.cmake to allow unsupported triplets")


def find_emsdk_node(emsdk_path):
    """Find the node executable bundled with the Emscripten SDK."""
    node_dir = os.path.join(emsdk_path, "node")
    if os.path.exists(node_dir):
        for entry in sorted(os.listdir(node_dir), reverse=True):
            for candidate in ["bin/node.exe", "bin/node"]:
                full_path = os.path.join(node_dir, entry, candidate)
                if os.path.exists(full_path):
                    return full_path
    return "node"


def compile_native(argumentsDict):
    shouldBuildArg = argumentsDict.get("buildCesium", None)
    if shouldBuildArg is None:
        shouldBuildResponse = input(
            "Do you wanna build Cesium Native (Choose yes if it's the first install)? [y/n]"
        )
        shouldBuildArg = shouldBuildResponse.capitalize()[0] == "Y"
    else:
        shouldBuildArg = (
            shouldBuildArg.upper() == "YES" or shouldBuildArg.upper() == "TRUE"
        )

    if not shouldBuildArg:
        return

    print("Building Cesium Native, this might take a few minutes...")
    configure_native(argumentsDict)
    print("Compiling Cesium Native...")

    platform = argumentsDict.get("platform", "")
    isExt = is_extension_target(argumentsDict)
    nativeDir = CESIUM_NATIVE_DIR_EXT if isExt else CESIUM_NATIVE_DIR_MODULE
    sourceDir = scons_to_abs_path(nativeDir)
    buildDir = os.path.join(sourceDir, get_native_build_dir_name(platform))

    result = None
    if platform == PLATFORM_WEB:
        result = build_native_web(buildDir)
    elif os.name == OS_WIN:
        result = build_native_win(buildDir)
    elif sys.platform == PLATFORM_MACOS:
        result = build_native_macos(buildDir)
    elif os.name == OS_LINUX:
        result = build_native_linux(buildDir)
    else:
        print(
            "Compiling for platform %s is not yet supported!" % os.name, file=sys.stderr
        )
    if result.returncode != 0:
        print("Error building Cesium Native: %s" % str(result.stderr))
    print("Cleaning definitions on generated files...")
    clean_cesium_definitions()
    print("Finished building Cesium Native!")


def build_native_linux(build_path):
    return subprocess.run(["cmake", "--build", build_path])


def build_native_web(build_path):
    return subprocess.run(["cmake", "--build", build_path])


def build_native_macos(build_path):
    return subprocess.run(["cmake", "--build", build_path])


def build_native_win(build_path):
    # cmake --build dispatches to MSBuild for Visual Studio generators
    return subprocess.run(["cmake", "--build", build_path, "--config", RELEASE_CONFIG])


def clean_cesium_definitions():
    """
    This function modifies some of Cesium's header files to clean up
    definitions that conflict with the engine's
    """
    # Get the conflicting file (Material.h in our case)
    print("Cleaning native definitions")

    conflictFilePath: str = "%s/%s" % (
        CESIUM_NATIVE_DIR_EXT,
        "/CesiumGltf/generated/include/CesiumGltf",
    )
    conflictFilePath = scons_to_abs_path(conflictFilePath) + "/Material.h"
    # Load the file into memory

    # Read in the file
    fileData: str = ""
    with open(conflictFilePath, "r") as file:
        fileData = file.read()

    # Replace the target string
    fileData = fileData.replace("#pragma once", "#pragma once\n#undef OPAQUE")

    # Write the file out again
    with open(conflictFilePath, "w") as file:
        file.write(fileData)
    print("Finished cleaning native definitions")


def install_additional_libs(argumentsDict=None):
    print("Installing additional libraries")
    vcpkgPath = find_ezvcpkg_path()
    execExtension = ".exe" if os.name == OS_WIN else ""
    executable = "%s/%s" % (vcpkgPath, "vcpkg" + execExtension)
    triplet = determine_triplet_for_args(argumentsDict) if argumentsDict else determine_triplet()
    allow_unsupported = []
    platform = argumentsDict.get("platform", "") if argumentsDict else ""
    if platform == PLATFORM_WEB:
        allow_unsupported = ["--allow-unsupported"]
    subprocess.run([executable, "install"] + allow_unsupported + ["uriparser:%s" % triplet])
    subprocess.run([executable, "install"] + allow_unsupported + ["ada-url:%s" % triplet])
    if platform != PLATFORM_WEB and os.name == OS_WIN:
        subprocess.run([executable, "install", "curl:%s" % triplet])


def find_ms_build() -> str:
    print("Searching for MS Build")
    # Try to search for an msbuild executable in the system
    try:
        testCmd = subprocess.run(
            ["msbuild", "-version"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        # Yay, we found it... (not gonna happen lol)
        if testCmd.returncode == 0:
            return "msbuild"
    except:
        # More likely we'll need to search for another path
        vsPath = "C:\\Program Files\\Microsoft Visual Studio"
        found, path = find_in_dir_recursive(vsPath, "*MSBuild.exe")

        if found:
            # Access the next latest directory (latest VS version)
            return path

        # Try with a .NET path
        print(".NET path is not yet supported!", sys.stderr)
        return ""


def find_in_dir_recursive(path: str, pattern: str) -> (bool, str):
    """
    Use only when there might be a few directories left to search
    as this function is recursive
    """

    if not os.path.exists(path):
        return False, ""

    foundFiles: list[str] = os.listdir(path)

    if len(foundFiles) == 0:
        return False, ""

    for root, dirnames, filenames in os.walk(path):
        for filename in fnmatch.filter(filenames, pattern):
            return True, os.path.join(root, filename)

    return False, ""


def find_ezvcpkg_path() -> str:
    global ezvcpkgFoundPath
    if ezvcpkgFoundPath != "":
        return ezvcpkgFoundPath
    # Search the home directory
    assumedPath = "%s.ezvcpkg" % (os.path.abspath(os.sep))
    print(f"Searching vcpkg at: {assumedPath}")
    if not os.path.exists(assumedPath):
        from pathlib import Path

        assumedPath = (Path.home() / ".ezvcpkg").as_posix()
        print(f"Searching vcpkg at: {assumedPath}")
        if not os.path.exists(assumedPath):
            print(
                "EZVCPKG not found, please make sure that CesiumNative was compiled and configured properly!"
            )
            return ""
        # Assume it is in /home (C:/Users/currUser)
    # Then find the latest version (use the last created folder)
    subDirs = [x for x in next(os.walk(assumedPath))[1]]
    subDirs.sort(
        reverse=True, key=lambda x: os.stat("%s/%s" % (assumedPath, x)).st_ctime
    )
    latestDir = subDirs[0]
    ezvcpkgFoundPath = "%s/%s" % (assumedPath, latestDir)
    print(f"Found ezvcpkg at {ezvcpkgFoundPath}")
    return ezvcpkgFoundPath


def clone_engine_repo_if_needed():
    pass


def scons_to_abs_path(path: str) -> str:
    return Dir(path).get_abspath()


def find_ezvcpkg_include_path(env=None) -> str:
    return f"{find_ezvcpkg_path()}/installed/{determine_triplet(env)}/include"


def find_ezvcpkg_lib_path(env=None) -> str:
    return f"{find_ezvcpkg_path()}/installed/{determine_triplet(env)}/lib"


def get_root_dir() -> str:
    return currentRootDir


def get_root_dir_native() -> str:
    return scons_to_abs_path(currentRootDir + "/native")
