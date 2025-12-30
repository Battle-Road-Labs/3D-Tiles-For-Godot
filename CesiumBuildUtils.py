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

OS_LINUX = "posix"

OS_MACOS = "darwin"


def is_macos() -> bool:
    """Check if we're running on macOS using sys.platform (more reliable than os.name)"""
    return sys.platform == OS_MACOS


def is_windows() -> bool:
    """Check if we're running on Windows"""
    return os.name == OS_WIN


def is_linux() -> bool:
    """Check if we're running on Linux (but not macOS)"""
    return os.name == OS_LINUX and not is_macos()

STATIC_TRIPLET = "x64-windows-static"

RELEASE_CONFIG = "Release"

ezvcpkgFoundPath: str = ""


def get_compile_flags():
    if is_windows():
        return ["/std:c++20", "/Zc:__cplusplus", "/utf-8", "/bigobj"]
    elif is_macos():
        return ["-std=c++20", "-fexceptions", "-fPIC", "-mmacosx-version-min=11.0"]
    elif is_linux():
        return ["-std=c++20", "-fexceptions", "-fpermissive", "-fPIC"]
    return []


def get_linker_flags():
    if is_windows():
        return ["/IGNORE:4217"]
    elif is_macos():
        return ["-mmacosx-version-min=11.0"]
    return []


def is_extension_target(argsDict) -> bool:
    return get_compile_target_definition(argsDict) == CESIUM_EXT_DEF


def get_curl_lib_name() -> str:
    if is_windows():
        return "libcurl"
    return "curl"


def generate_precision_symbols(argsDict, env):
    print("Generating double precision compile symbols")
    desiredPrecision = argsDict.get("precision")
    if desiredPrecision == "double":
        env.Append(CPPDEFINES=["REAL_T_IS_DOUBLE"])


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


def link_macos_frameworks(env):
    """Link required macOS frameworks for networking and security"""
    frameworks = [
        "Security",
        "CoreFoundation",
        "SystemConfiguration",
    ]
    for framework in frameworks:
        env.Append(LINKFLAGS=["-framework", framework])


def link_abseil_libs_macos(env):
    """Link Abseil libraries on macOS (doesn't use --start-group/--end-group)"""
    foundLibs: list = env.Glob(
        f"{find_ezvcpkg_path()}/packages/abseil_{determine_triplet()}/lib/*absl*.a"
    )

    # Strip the lib prefix and the file extension
    foundLibs = [lib.name.replace("lib", "")[:-2] for lib in foundLibs]

    # macOS doesn't support --start-group/--end-group, but usually doesn't need them
    # due to different linker behavior
    env.Append(LIBS=foundLibs)


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


def clone_lite_html_if_needed():
    # clone_repo_if_needed(ROOT_DIR_EXT + "/third_party/lite-html", "Lite HTML",
    #                      "https://github.com/litehtml/litehtml.git", "v0.9", "6ca1ab0419e770e6d35a1ef690238773a1dafcee")
    pass


def build_litehtml_macos():
    """Build litehtml from source for macOS with utf32 support.

    The vcpkg version (0.9.0) doesn't have the utf32_to_utf8/utf8_to_utf32 classes
    that the codebase requires. We need to build from a newer commit.
    """
    if not is_macos():
        return

    import multiprocessing
    import shutil
    from pathlib import Path

    third_party_dir = scons_to_abs_path(ROOT_DIR_EXT + "/third_party")
    litehtml_dir = os.path.join(third_party_dir, "litehtml-src")
    build_dir = os.path.join(litehtml_dir, "build")
    # Put macOS libraries in a separate directory to avoid overwriting Linux precompiled binaries
    macos_lib_dir = os.path.join(third_party_dir, "litehtml", "macos")
    os.makedirs(macos_lib_dir, exist_ok=True)
    output_lib = os.path.join(macos_lib_dir, "liblitehtml.a")
    output_gumbo = os.path.join(macos_lib_dir, "libgumbo.a")

    # Check if already built
    if os.path.exists(output_lib):
        # Verify it's a Mach-O file
        result = subprocess.run(["file", output_lib], capture_output=True, text=True)
        if "Mach-O" in result.stdout:
            print("[litehtml] macOS library already exists, skipping build")
            return

    print("[litehtml] Building litehtml from source for macOS...")

    # Clone litehtml at the specific commit that matches the third_party headers
    # Commit 35ecd69d has the utf32 functions and uses int parameters (not float)
    # The vcpkg version (0.9.0) uses wchar instead of utf32, which is incompatible
    litehtml_commit = "35ecd69d05e72b0148204a576db62c2148084193"
    if not os.path.exists(litehtml_dir):
        print(f"[litehtml] Cloning litehtml at commit {litehtml_commit[:8]}...")
        subprocess.run([
            "git", "clone",
            "https://github.com/litehtml/litehtml.git",
            litehtml_dir
        ])
        # Fetch all history to get the specific commit
        subprocess.run(["git", "-C", litehtml_dir, "fetch", "--unshallow"], check=False)
        subprocess.run(["git", "-C", litehtml_dir, "checkout", litehtml_commit])
        subprocess.run(["git", "-C", litehtml_dir, "submodule", "update", "--init", "--recursive"])

    # Create build directory
    os.makedirs(build_dir, exist_ok=True)
    prev_dir = os.getcwd()
    os.chdir(build_dir)

    try:
        # Configure with CMake
        cmake_args = [
            "cmake",
            "-DCMAKE_BUILD_TYPE=Release",
            "-DCMAKE_OSX_DEPLOYMENT_TARGET=11.0",
            "-DLITEHTML_BUILD_TESTING=OFF",
            ".."
        ]

        # Set architecture for cross-compilation if needed
        import platform
        arch = platform.machine()
        if arch == "arm64":
            cmake_args.insert(-1, "-DCMAKE_OSX_ARCHITECTURES=arm64")
        elif arch == "x86_64":
            cmake_args.insert(-1, "-DCMAKE_OSX_ARCHITECTURES=x86_64")

        print("[litehtml] Configuring with CMake...")
        result = subprocess.run(cmake_args)
        if result.returncode != 0:
            print("[litehtml] CMake configuration failed!")
            return

        # Build
        num_jobs = multiprocessing.cpu_count()
        print(f"[litehtml] Building with {num_jobs} parallel jobs...")
        result = subprocess.run(["cmake", "--build", ".", "--parallel", str(num_jobs)])
        if result.returncode != 0:
            print("[litehtml] Build failed!")
            return

        # Copy built libraries to third_party/litehtml
        built_litehtml = os.path.join(build_dir, "liblitehtml.a")
        # gumbo is a submodule built in src/gumbo/
        built_gumbo = os.path.join(build_dir, "src", "gumbo", "libgumbo.a")

        if os.path.exists(built_litehtml):
            shutil.copy2(built_litehtml, output_lib)
            print(f"[litehtml] Copied liblitehtml.a to {output_lib}")

        if os.path.exists(built_gumbo):
            shutil.copy2(built_gumbo, output_gumbo)
            print(f"[litehtml] Copied libgumbo.a to {output_gumbo}")

        print("[litehtml] Build complete!")

    finally:
        os.chdir(prev_dir)


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


# Configure with CMake
def configure_native(argumentsDict):
    print("Configuring Cesium Native")
    isExt = is_extension_target(argumentsDict)
    repoDirectory = CESIUM_NATIVE_DIR_EXT if isExt else CESIUM_NATIVE_DIR_MODULE
    repoDirectory = scons_to_abs_path(repoDirectory)
    os.chdir(repoDirectory)

    # Get architecture from arguments if provided
    arch = argumentsDict.get("arch", None)
    triplet: str = determine_triplet(arch)

    if triplet is None:
        print(f"Error: Could not determine vcpkg triplet for platform {sys.platform}", file=sys.stderr)
        exit(1)

    os.environ["VCPKG_TRIPLET"] = triplet

    # Base CMake arguments
    cmake_args = [
        "cmake",
        f"-DCMAKE_BUILD_TYPE={RELEASE_CONFIG}",
        "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
        "-DGIT_LFS_SKIP_SMUDGE=1",
        "-DVCPKG_TRIPLET=%s" % triplet,
    ]

    # Platform-specific CMake arguments
    if is_windows():
        cmake_args.append("-DCESIUM_MSVC_STATIC_RUNTIME_ENABLED=ON")
    elif is_macos():
        cmake_args.extend([
            "-DCMAKE_OSX_DEPLOYMENT_TARGET=11.0",
        ])
        # Set architecture for cross-compilation if specified
        if arch in ("arm64", "aarch64"):
            cmake_args.append("-DCMAKE_OSX_ARCHITECTURES=arm64")
        elif arch in ("x86_64", "x64"):
            cmake_args.append("-DCMAKE_OSX_ARCHITECTURES=x86_64")

    cmake_args.append(".")

    result = subprocess.run(cmake_args)

    if result.returncode != 0:
        errorMsg = "cmake return code: %s" % str(result.returncode)
        print(
            "Error configuring Cesium native, please make sure you have CMake installed and up to date: "
            + errorMsg
        )
        exit(1)
    print("Configuration completed without any errors!")


def determine_triplet(arch: str = None):
    """Determine the vcpkg triplet for the current platform and architecture"""
    if is_windows():
        return "x64-windows-static"
    if is_macos():
        # Detect architecture - use provided arch or auto-detect
        if arch is None:
            import platform
            arch = platform.machine()
        if arch in ("arm64", "aarch64"):
            return "arm64-osx"
        else:
            return "x64-osx"
    if is_linux():
        return "x64-linux"
    return None


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

    # TODO: Test if we can just do cmake --build for all platforms
    result = None
    if is_windows():
        result = build_native_win()
    elif is_macos():
        result = build_native_macos()
    elif is_linux():
        result = build_native_linux()
    else:
        print(
            "Compiling for platform %s (sys.platform=%s) is not yet supported!" % (os.name, sys.platform), file=sys.stderr
        )
    if result.returncode != 0:
        print("Error building Cesium Native: %s" % str(result.stderr))
    print("Cleaning definitions on generated files...")
    clean_cesium_definitions()
    print("Finished building Cesium Native!")


def build_native_linux():
    return subprocess.run(["cmake", "--build", "."])


def build_native_macos():
    """Build Cesium Native on macOS using cmake --build with parallel jobs"""
    import multiprocessing
    num_jobs = multiprocessing.cpu_count()
    return subprocess.run(["cmake", "--build", ".", "--parallel", str(num_jobs)])


def build_native_win():
    # execute MSBuild
    buildConfig: str = RELEASE_CONFIG
    solutionName: str = "cesium-native.sln"
    msbuildPath: str = find_ms_build()
    if msbuildPath == "":
        print(
            "Could not find MSBuild.exe, make sure to have Visual Studio installed",
            file=sys.stderr,
        )
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


def install_additional_libs():
    print("Installing additional libraries")
    vcpkgPath = find_ezvcpkg_path()
    execExtension = ".exe" if is_windows() else ""
    executable = "%s/%s" % (vcpkgPath, "vcpkg" + execExtension)
    subprocess.run([executable, "install", "uriparser:%s" % (determine_triplet())])
    subprocess.run([executable, "install", "ada-url:%s" % (determine_triplet())])
    if is_windows():
        subprocess.run([executable, "install", "curl:%s" % (determine_triplet())])
    if is_macos():
        # Build litehtml from source for macOS (vcpkg version lacks utf32 support)
        build_litehtml_macos()


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


def find_ezvcpkg_include_path() -> str:
    return f"{find_ezvcpkg_path()}/installed/{determine_triplet()}/include"


def find_ezvcpkg_lib_path() -> str:
    return f"{find_ezvcpkg_path()}/installed/{determine_triplet()}/lib"


def get_root_dir() -> str:
    return currentRootDir


def get_root_dir_native() -> str:
    return scons_to_abs_path(currentRootDir + "/native")
