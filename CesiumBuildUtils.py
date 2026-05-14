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


def is_web_memory64():
    """Returns True when the current web build should target wasm64 (Memory64).

    Driven by the CESIUM_WEB_MEMORY64 env var (set by build.bat/build.sh `web64`).
    Affects vcpkg triplet selection, native build dir name, EMCC flags,
    and the GDExtension output filename (wasm32 -> wasm64 suffix)."""
    return os.environ.get("CESIUM_WEB_MEMORY64", "").lower() in ("1", "yes", "true")


def get_compile_flags(env=None):
    if is_web_platform(env):
        flags = ["-std=c++20", "-fwasm-exceptions", "-fPIC", "-pthread"]
        if is_web_memory64():
            flags.append("-sMEMORY64=1")
        return flags
    if os.name == OS_WIN:
        return ["/std:c++20", "/Zc:__cplusplus", "/utf-8", "/bigobj"]
    elif sys.platform == PLATFORM_MACOS:
        return ["-std=c++20", "-fexceptions", "-fPIC"]
    elif os.name == OS_LINUX:
        return ["-std=c++20", "-fexceptions", "-fpermissive", "-fPIC"]


def get_linker_flags(env=None):
    if is_web_platform(env):
        flags = [
            "-sSIDE_MODULE=1",
            "-pthread",
            "-sPTHREAD_POOL_SIZE=4",
            "-sALLOW_MEMORY_GROWTH=1",
        ]
        if is_web_memory64():
            flags.append("-sMEMORY64=1")
        return flags
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
    patch_cesium_gltf_model_glm_include()


def patch_cesium_gltf_model_glm_include():
    """Add <glm/gtc/quaternion.hpp> to CesiumGltf/src/Model.cpp.

    Newer GLM (from recent vcpkg pins) requires mat4_cast to be visible at the
    translation unit level — ADL from glm/detail/type_quat.inl isn't enough.
    Cesium-native v0.52.1 doesn't include this header, causing
    'mat4_cast: identifier not found' when Model.cpp instantiates the
    quaternion → mat4 conversion operator. Idempotent."""
    model_cpp = os.path.join(
        scons_to_abs_path(CESIUM_NATIVE_DIR_EXT),
        "CesiumGltf", "src", "Model.cpp"
    )
    if not os.path.exists(model_cpp):
        return
    with open(model_cpp, "r") as f:
        content = f.read()
    if "<glm/gtc/quaternion.hpp>" in content:
        return  # already patched
    patched = content.replace(
        "#include <glm/geometric.hpp>",
        "#include <glm/geometric.hpp>\n#include <glm/gtc/quaternion.hpp>"
    )
    if patched != content:
        with open(model_cpp, "w") as f:
            f.write(patched)
        print("[CESIUM] Patched CesiumGltf/src/Model.cpp with glm/gtc/quaternion.hpp")


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
    """Clone litehtml at the exact commit matching the pre-built binaries."""
    target_dir = scons_to_abs_path(ROOT_DIR_EXT + "/third_party/litehtml-src")
    commit = "35ecd69d05e72b0148204a576db62c2148084193"
    print("Cloning Lite HTML repo")
    if os.path.exists(target_dir):
        return
    subprocess.run(["git", "clone", "--recursive",
                     "https://github.com/litehtml/litehtml.git", target_dir])
    prev_dir = os.getcwd()
    os.chdir(target_dir)
    subprocess.run(["git", "checkout", commit])
    os.chdir(prev_dir)


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


def build_litehtml_web():
    """Build litehtml from source for Web/WASM using Emscripten.

    Uses a parallel build-web64/ output dir when wasm64 mode is active so a
    wasm32 build can coexist with a wasm64 build without cross-contamination."""
    third_party_dir = scons_to_abs_path(ROOT_DIR_EXT + "/third_party")
    source_dir = os.path.join(third_party_dir, "litehtml-src")
    out_subdir = "web64" if is_web_memory64() else "web"
    output_dir = os.path.join(third_party_dir, "litehtml", out_subdir)

    # Check if already built
    if (os.path.exists(os.path.join(output_dir, "liblitehtml.a"))
            and os.path.exists(os.path.join(output_dir, "libgumbo.a"))):
        print("litehtml already built for Web/WASM (%s), skipping..." % out_subdir)
        return

    if not os.path.exists(source_dir):
        print("litehtml source not found at %s" % source_dir, file=sys.stderr)
        return

    emsdk = os.environ.get("EMSDK", "")
    if not emsdk:
        print(
            "Error: EMSDK environment variable not set. "
            "Please install and activate the Emscripten SDK first.",
            file=sys.stderr,
        )
        return

    print("Building litehtml from source for Web/WASM (%s)..." % out_subdir)

    toolchain = os.path.join(
        emsdk, "upstream", "emscripten", "cmake", "Modules", "Platform", "Emscripten.cmake"
    )

    build_dir = os.path.join(source_dir, "build-" + out_subdir)
    os.makedirs(build_dir, exist_ok=True)
    os.makedirs(output_dir, exist_ok=True)

    prev_dir = os.getcwd()
    os.chdir(build_dir)

    memory64_flag = " -sMEMORY64=1" if is_web_memory64() else ""
    # Configure with CMake using Emscripten toolchain
    result = subprocess.run([
        "cmake",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_TOOLCHAIN_FILE=%s" % toolchain,
        "-DLITEHTML_BUILD_TESTING=OFF",
        "-DCMAKE_CXX_FLAGS=-pthread -fPIC -fwasm-exceptions" + memory64_flag,
        "-DCMAKE_C_FLAGS=-pthread -fPIC" + memory64_flag,
        "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
        "-G", "Ninja",
        ".."
    ])

    if result.returncode != 0:
        print("Failed to configure litehtml for Web/WASM", file=sys.stderr)
        os.chdir(prev_dir)
        return

    # Build
    result = subprocess.run(["cmake", "--build", ".", "--config", "Release"])

    if result.returncode != 0:
        print("Failed to build litehtml for Web/WASM", file=sys.stderr)
        os.chdir(prev_dir)
        return

    # Copy output libraries — they may be in subdirectories
    for lib in ["liblitehtml.a", "libgumbo.a"]:
        src = os.path.join(build_dir, lib)
        if not os.path.exists(src):
            for walk_root, dirs, files in os.walk(build_dir):
                if lib in files:
                    src = os.path.join(walk_root, lib)
                    break
        if os.path.exists(src):
            shutil.copy2(src, output_dir)
            print("Copied %s to %s" % (lib, output_dir))
        else:
            print("Warning: %s not found after build" % lib, file=sys.stderr)

    os.chdir(prev_dir)
    print("litehtml Web/WASM build complete!")


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
        # For wasm64, add -sMEMORY64=1 to all vcpkg port builds via the triplet.
        web_extra_flags = "-sMEMORY64=1" if is_web_memory64() else ""
        patch_vcpkg_wasm_triplet_pthread(triplet, web_extra_flags)
        patch_libjpeg_turbo_port_no_setjmp()
        # KTX's vcpkg port applies 0001-Use-vcpkg-zstd.patch which replaces
        # KTX's vendored zstd with find_package(zstd). KTX's vcpkg.json
        # doesn't declare zstd as a dep (at least not for our feature set),
        # so ezvcpkg_fetch doesn't install it — KTX's configure then fails
        # with "Could not find a package configuration file provided by zstd".
        # Pre-install it so it's present in installed/<triplet>/share/zstd/
        # before ezvcpkg processes the KTX port.
        vcpkg_exe = os.path.join(find_ezvcpkg_path(), "vcpkg" + (".exe" if os.name == OS_WIN else ""))
        if os.path.exists(vcpkg_exe):
            subprocess.run([vcpkg_exe, "install", "--allow-unsupported", "zstd:%s" % triplet])

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

    # wasm64: lift MEMORY64=1 into cesium-native's own cmake build (not just
    # the vcpkg ports). All compile units in the cesium-native libs need the
    # same flag or the final link mixes wasm32/wasm64 calling conventions.
    if is_web and is_web_memory64():
        cmake_args.extend([
            "-DCMAKE_C_FLAGS=-sMEMORY64=1",
            "-DCMAKE_CXX_FLAGS=-sMEMORY64=1",
            "-DCMAKE_EXE_LINKER_FLAGS=-sMEMORY64=1",
        ])

    # Workaround: cmake's find_package picks up configs from the x64-windows
    # (shared/DLL) triplet even when VCPKG_TARGET_TRIPLET is x64-windows-static.
    # Temporarily hide conflicting triplet directories during configure so cmake
    # can only discover the correct one.
    _hidden_triplet_dirs = []
    if not is_web:
        _hidden_triplet_dirs = _hide_conflicting_vcpkg_triplets(triplet)

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
            # wasm32 has 32-bit size_t/ptrdiff_t; cesium-native code assumes
            # 64-bit and triggers -Werror. Downgrade these to warnings.
            # -pthread enables atomics+bulk-memory needed for --shared-memory at link time.
            "-DCMAKE_CXX_FLAGS=-pthread -fPIC -fwasm-exceptions -Wno-error=constant-conversion -Wno-error=shift-count-overflow -Wno-error=shorten-64-to-32 -Wno-error=sign-conversion",
            "-DCMAKE_C_FLAGS=-pthread -fPIC",
        ])

    print(f"[CESIUM] Build directory: {buildDir}")
    result = subprocess.run(cmake_args)

    # Restore any hidden triplet directories
    _restore_hidden_vcpkg_triplets(_hidden_triplet_dirs)

    if result.returncode != 0:
        errorMsg = "cmake return code: %s" % str(result.returncode)
        print(
            "Error configuring Cesium native, please make sure you have CMake installed and up to date: "
            + errorMsg
        )
        exit(1)

    print("Configuration completed without any errors!")


def _hide_conflicting_vcpkg_triplets(target_triplet):
    """Temporarily rename find_package-visible content in conflicting vcpkg triplets.

    cmake's find_package otherwise picks up the wrong triplet (e.g. x64-windows
    instead of x64-windows-static) despite VCPKG_TARGET_TRIPLET being set.
    Hide lib/, include/, debug/, and share/<pkg>/ subdirs — but keep
    share/vcpkg-cmake*/ and tools/ visible so vcpkg-cmake's host helpers remain
    available when a port has a cache miss and needs to build from source."""
    # Keep these share/ subdirs visible — vcpkg's port builds include() them.
    preserve_share = {
        "vcpkg-cmake",
        "vcpkg-cmake-config",
        "vcpkg-cmake-get-vars",
        "vcpkg-get-python-packages",
        "vcpkg-tool-meson",
        "vcpkg-tool-ninja",
        "vcpkg-pkgconfig-get-modules",
    }
    hidden = []
    try:
        vcpkg_base = find_ezvcpkg_path()
        installed_dir = os.path.join(vcpkg_base, "installed")
        if not os.path.exists(installed_dir):
            return hidden
        for entry in os.listdir(installed_dir):
            entry_path = os.path.join(installed_dir, entry)
            if not os.path.isdir(entry_path):
                continue
            if entry == target_triplet or entry == "vcpkg" or entry.endswith(".bak"):
                continue
            if not target_triplet.startswith(entry.split("-")[0] + "-"):
                continue
            print(f"[CESIUM] Hiding find_package content in conflicting triplet: {entry}")
            # Hide lib/, include/, debug/ outright
            for sub in ("lib", "include", "debug"):
                sp = os.path.join(entry_path, sub)
                if os.path.isdir(sp):
                    bak = sp + ".bak"
                    try:
                        os.rename(sp, bak)
                        hidden.append((bak, sp))
                    except OSError:
                        pass
            # Hide share/<pkg>/ individually, preserving vcpkg helper dirs
            share_dir = os.path.join(entry_path, "share")
            if os.path.isdir(share_dir):
                for pkg in os.listdir(share_dir):
                    if pkg in preserve_share or pkg.endswith(".bak"):
                        continue
                    pkg_path = os.path.join(share_dir, pkg)
                    if not os.path.isdir(pkg_path):
                        continue
                    bak = pkg_path + ".bak"
                    try:
                        os.rename(pkg_path, bak)
                        hidden.append((bak, pkg_path))
                    except OSError:
                        pass
    except Exception:
        pass
    return hidden


def _restore_hidden_vcpkg_triplets(hidden_dirs):
    """Restore triplet directories that were temporarily hidden."""
    for bak_path, orig_path in hidden_dirs:
        try:
            if os.path.exists(bak_path) and not os.path.exists(orig_path):
                os.rename(bak_path, orig_path)
        except OSError:
            print(f"[CESIUM] Warning: could not restore {orig_path}, rename manually from {bak_path}")


def determine_triplet(env=None):
    if is_web_platform(env):
        return "wasm64-emscripten" if is_web_memory64() else "wasm32-emscripten"
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
        return "wasm64-emscripten" if is_web_memory64() else "wasm32-emscripten"
    return determine_triplet()


def get_native_build_dir_name(platform_str=""):
    """Return the build subdirectory name for the given target platform.

    Each platform gets its own out-of-tree CMake build directory so that
    multiple platform builds can coexist under cesium_godot/native/."""
    if platform_str == PLATFORM_WEB:
        return "build-web64" if is_web_memory64() else "build-web"
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


def patch_vcpkg_wasm_triplet_pthread(triplet_name="wasm32-emscripten", extra_flags=""):
    """Patch a wasm-emscripten vcpkg triplet for SIDE_MODULE builds.

    Adds flags via VCPKG_CMAKE_CONFIGURE_OPTIONS (following cesium-native PR #1267 approach):
    - -pthread -fPIC: required for SIDE_MODULE shared library builds
    - -fwasm-exceptions: native wasm exception handling (no JS invoke_* wrappers)
    - -sSUPPORT_LONGJMP=wasm: native wasm longjmp (pairs with -fwasm-exceptions)
    - extra_flags: additional flags appended verbatim (e.g. "-sMEMORY64=1" for wasm64).

    For wasm64-emscripten the upstream vcpkg tree has no such triplet, so this
    function seeds one from the wasm32-emscripten community triplet on first
    call (vcpkg's VCPKG_TARGET_ARCHITECTURE field doesn't recognize "wasm64",
    so the seeded file keeps the wasm32 label — the actual MEMORY64=1 lift
    is purely a compile/link-flag concern handled here).

    Also passes EMCC_CFLAGS through for make-based builds (openssl)."""
    try:
        vcpkg_base = find_ezvcpkg_path()
        triplet_path = os.path.join(vcpkg_base, "triplets", "community", f"{triplet_name}.cmake")
        # Seed a wasm64-emscripten community triplet from wasm32-emscripten if missing.
        if not os.path.exists(triplet_path) and triplet_name == "wasm64-emscripten":
            src_path = os.path.join(vcpkg_base, "triplets", "community", "wasm32-emscripten.cmake")
            if os.path.exists(src_path):
                shutil.copy2(src_path, triplet_path)
                print(f"[CESIUM] Seeded {triplet_name}.cmake from wasm32-emscripten")
        if not os.path.exists(triplet_path):
            return
        with open(triplet_path, "r") as f:
            content = f.read()
        import re
        # Check if already patched with our full current configuration. The
        # `zstd_DIR` marker means the latest patch (with explicit zstd path for
        # KTX's find_package) is applied. Also detect the malformed `)set(`
        # state that a buggy earlier version of this function could produce
        # (two `set()` calls concatenated without a newline between them) —
        # always re-patch in that case.
        # Require the quoted `"-DCMAKE_C_FLAGS=` form — earlier versions of
        # this patch emitted unquoted `-DCMAKE_C_FLAGS=${_cesiumFlags}`, which
        # causes CMake's `set()` to tokenize the expanded flag string by
        # whitespace and only `-pthread` reaches CMAKE_C_FLAGS in the port
        # subprocess. The rest (including -sSUPPORT_LONGJMP=wasm) gets dropped.
        # Detecting the quoted form forces re-patch from the unquoted state.
        already_patched = (
            "zstd_DIR" in content
            and "-fwasm-exceptions" in content
            and '"-DCMAKE_C_FLAGS=${_cesiumFlags}"' in content
        )
        # Two `set()` calls concatenated with no whitespace between them —
        # CMake parses `)set(` as a syntax error. This shape only appears if
        # an earlier buggy version of this function collapsed lines during
        # strip operations. Always re-patch to repair.
        malformed = ")set(" in content
        if already_patched and not malformed:
            return
        # Strip any previous partial patches before re-applying. Use '\n' as
        # the replacement (not '') so we don't collapse the preceding and
        # following lines into a single line — that's exactly what produced
        # the malformed `)set(` state in earlier runs.
        content = content.replace('\nset(VCPKG_CXX_FLAGS "-pthread")\nset(VCPKG_C_FLAGS "-pthread")\n', '\n')
        content = content.replace('\nset(VCPKG_CXX_FLAGS "-pthread -fPIC")\nset(VCPKG_C_FLAGS "-pthread -fPIC")\n', '\n')
        # Repair any existing `)set(` concatenation in the file from prior buggy runs.
        content = re.sub(r'\)set\(', ')\nset(', content)
        # Remove old VCPKG_CMAKE_CONFIGURE_OPTIONS block if present (may span
        # multiple lines if we add `-Dzstd_DIR=...` — match non-greedy to `)`).
        content = re.sub(r'\nset\(VCPKG_CMAKE_CONFIGURE_OPTIONS[\s\S]*?\)\n', '\n', content)
        # Also strip the stale _cesiumFlags line so it can be re-emitted cleanly.
        content = re.sub(r'\nset\(_cesiumFlags [^\n]*\)\n', '\n', content)
        content = re.sub(r'\n# Cesium SIDE_MODULE flags \(patched by CesiumBuildUtils\.py\)\n', '\n', content)
        # Add EMCC_CFLAGS to the passthrough list so vcpkg propagates it to emcc
        # (needed for make-based builds like openssl that don't use cmake)
        if "EMCC_CFLAGS" not in content:
            content = content.replace(
                "set(VCPKG_ENV_PASSTHROUGH_UNTRACKED EMSCRIPTEN_ROOT EMSDK PATH)",
                "set(VCPKG_ENV_PASSTHROUGH_UNTRACKED EMSCRIPTEN_ROOT EMSDK PATH EMCC_CFLAGS)"
            )
        # Following cesium-native PR #1267: pass flags through VCPKG_CMAKE_CONFIGURE_OPTIONS
        # This sets CMAKE_C_FLAGS/CMAKE_CXX_FLAGS for all cmake-based vcpkg ports.
        #
        # -Dzstd_DIR: KTX's CMakeLists calls `find_package(zstd)`, and Emscripten's
        # CMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY (combined with Windows absolute
        # paths) collapses find_package search to the emscripten sysroot, so it
        # never checks the vcpkg installed dir. Pass zstd_DIR explicitly; CMake
        # uses it as a hint that bypasses the root-path restrictions. The path
        # is baked in at patch time from find_ezvcpkg_path() so it's absolute
        # and not subject to VCPKG_INSTALLED_DIR resolution timing.
        zstd_dir_cmake = os.path.join(
            vcpkg_base, "installed", triplet_name, "share", "zstd"
        ).replace("\\", "/")
        cesium_flags = "-pthread -fPIC -fwasm-exceptions -sSUPPORT_LONGJMP=wasm"
        if extra_flags:
            cesium_flags = cesium_flags + " " + extra_flags
        # Quote each -D...=... arg so CMake treats each full flag string as a
        # single list element. Without quotes, ${_cesiumFlags} expands and the
        # whitespace in it splits the flag assignment into multiple list items
        # (`-DCMAKE_C_FLAGS=-pthread`, `-fPIC`, `-fwasm-exceptions`,
        # `-sSUPPORT_LONGJMP=wasm`). Only `-pthread` actually reaches
        # CMAKE_C_FLAGS in the port subprocess; the rest become orphan cmake
        # args that get dropped. That silently loses -sSUPPORT_LONGJMP=wasm,
        # which is why libjpeg-turbo was still emitting saveSetjmp calls at
        # runtime despite a fresh rebuild.
        # Inline cesium_flags into the cmake set() so VCPKG_CMAKE_CONFIGURE_OPTIONS
        # gets the literal string with both substitutions resolved at Python time.
        vcpkg_short_flags = "-pthread -fPIC"
        if extra_flags:
            vcpkg_short_flags = vcpkg_short_flags + " " + extra_flags
        patch_block = f'''
# Cesium SIDE_MODULE flags (patched by CesiumBuildUtils.py)
set(_cesiumFlags "{cesium_flags}")
set(VCPKG_CXX_FLAGS "{vcpkg_short_flags}")
set(VCPKG_C_FLAGS "{vcpkg_short_flags}")
set(VCPKG_CMAKE_CONFIGURE_OPTIONS
    "-DCMAKE_C_FLAGS=${{_cesiumFlags}}"
    "-DCMAKE_CXX_FLAGS=${{_cesiumFlags}}"
    "-DCMAKE_EXE_LINKER_FLAGS=${{_cesiumFlags}}"
    "-Dzstd_DIR={zstd_dir_cmake}")
'''
        patched = content + patch_block
        with open(triplet_path, "w") as f:
            f.write(patched)
        print(f"[CESIUM] Patched {triplet_name} triplet (flags: {cesium_flags})")
        # Force vcpkg to rebuild all packages on this triplet with the new flags.
        # We must use vcpkg remove to properly clean the tracking metadata, not just
        # delete the installed directory.
        exec_ext = ".exe" if os.name == OS_WIN else ""
        vcpkg_exe = os.path.join(vcpkg_base, "vcpkg" + exec_ext)
        if os.path.exists(vcpkg_exe):
            print(f"[CESIUM] Removing {triplet_name} packages so vcpkg rebuilds with new flags...")
            subprocess.run(
                [vcpkg_exe, "--vcpkg-root", vcpkg_base, "remove", "--outdated", "--recurse",
                 "--triplet", triplet_name],
                cwd=vcpkg_base,
            )
            # Remove installed packages for this triplet — delete first, then clean status
            installed_dir = os.path.join(vcpkg_base, "installed", triplet_name)
            if os.path.exists(installed_dir):
                shutil.rmtree(installed_dir)
            # Clear the status database entries so vcpkg reinstalls everything.
            # The status file is a series of stanzas separated by blank lines;
            # each stanza has multiple `Key: value` fields. Split by blank-line
            # boundaries and drop any stanza whose Architecture is wasm32-
            # emscripten. This avoids the bug in the earlier non-greedy regex
            # that spanned across non-wasm stanzas and deleted valid entries
            # for other triplets (x64-windows-static, x64-windows, etc.),
            # corrupting the db with 'Package X installed, but dependency Y
            # is not' errors on subsequent vcpkg invocations.
            vcpkg_status = os.path.join(vcpkg_base, "installed", "vcpkg", "status")
            try:
                if os.path.exists(vcpkg_status):
                    with open(vcpkg_status, "r", encoding="utf-8", errors="replace") as f:
                        status_content = f.read()
                    stanzas = re.split(r'\n\n+', status_content)
                    arch_re = re.compile(
                        r'^Architecture:\s*' + re.escape(triplet_name) + r'\s*$',
                        re.MULTILINE,
                    )
                    kept = [s for s in stanzas if not arch_re.search(s)]
                    cleaned = '\n\n'.join(kept)
                    # Preserve trailing blank line if the original had one
                    if status_content.endswith('\n') and not cleaned.endswith('\n'):
                        cleaned += '\n'
                    with open(vcpkg_status, "w", encoding="utf-8") as f:
                        f.write(cleaned)
            except Exception as e:
                print(f"[CESIUM] Warning: could not clean vcpkg status file: {e}")
            print(f"[CESIUM] Cleared {triplet_name} packages (will rebuild with new flags)")
            # Layer 4: vcpkg binary archive cache. Keyed by a hash that includes
            # triplet content, so a patched triplet normally invalidates old
            # entries — but stale/partial entries from prior attempts can still
            # short-circuit a rebuild (the libjpeg-turbo saveSetjmp incident).
            # Archive files aren't labeled by triplet, so clearing the whole
            # dir is the only safe option. Respects VCPKG_DEFAULT_BINARY_CACHE.
            archives_override = os.environ.get("VCPKG_DEFAULT_BINARY_CACHE", "")
            if archives_override:
                archives_dir = archives_override
            elif os.name == OS_WIN:
                archives_dir = os.path.join(os.environ.get("LOCALAPPDATA", ""), "vcpkg", "archives")
            elif sys.platform == PLATFORM_MACOS:
                archives_dir = os.path.expanduser("~/Library/Caches/vcpkg/archives")
            else:
                archives_dir = os.path.expanduser("~/.cache/vcpkg/archives")
            if archives_dir and os.path.exists(archives_dir):
                try:
                    shutil.rmtree(archives_dir)
                    print(f"[CESIUM] Cleared vcpkg binary archive cache: {archives_dir}")
                except Exception as e:
                    print(f"[CESIUM] Warning: could not clear vcpkg binary cache at {archives_dir}: {e}")
    except Exception as e:
        print(f"[CESIUM] Warning: could not patch wasm triplet: {e}")


def patch_fmt_consteval(env=None):
    """Disable consteval in ezvcpkg-installed fmt's basic_format_string.

    Spdlog's SPDLOG_FMT_STRING("{:02}") path in details/fmt_helper.h instantiates
    fmt::basic_format_string<...> whose constructor is consteval in fmt 11+.
    Emscripten's clang rejects the instantiation as not a constant expression.
    Flipping FMT_USE_CONSTEVAL to 0 at the source makes fmt emit a plain
    constructor so the format-string check falls back to runtime. Command-line
    -DFMT_USE_CONSTEVAL=0 doesn't work because fmt/base.h redefines it.

    Idempotent: the second run finds `FMT_USE_CONSTEVAL 0` and is a no-op.
    """
    try:
        ezvcpkg = find_ezvcpkg_path()
        if not ezvcpkg:
            return
        triplet = determine_triplet(env)
        base_h = os.path.join(ezvcpkg, "installed", triplet, "include", "fmt", "base.h")
        if not os.path.exists(base_h):
            return
        with open(base_h, "r", encoding="utf-8", errors="replace") as f:
            content = f.read()
        needle = "#  define FMT_USE_CONSTEVAL 1"
        if needle not in content:
            return
        with open(base_h, "w", encoding="utf-8") as f:
            f.write(content.replace(needle, "#  define FMT_USE_CONSTEVAL 0"))
        print(f"[CESIUM] Patched {base_h}: FMT_USE_CONSTEVAL 1 -> 0")
    except Exception as e:
        print(f"[CESIUM] Warning: could not patch fmt/base.h: {e}")


def patch_libjpeg_turbo_port_no_setjmp():
    """(self-revert stub) Strip any prior CESIUM_NO_SETJMP_PATCH block from
    the libjpeg-turbo port's portfile.cmake.

    Historical context: this function used to INJECT a vcpkg_replace_string
    that neutralized setjmp/longjmp in TurboJPEG's src/turbojpeg.c to work
    around an emsdk 3.1.56 bug where emcc ignores -sSUPPORT_LONGJMP=wasm at
    compile time for SIDE_MODULE relocatable objects. Problem discovered in
    use: libjpeg-turbo calls longjmp() on the normal "not a JPEG, try
    another decoder" fallback path during format-probe (not just on corrupt
    data). Our patched longjmp -> abort() was killing the tile-loader
    worker thread on pretty much every non-JPEG tile.

    Replacement strategy: vendor emscripten's own `emscripten_setjmp.c` and
    `emscripten_tempret.s` into cesium_godot/third_party/emscripten_sjlj/
    and compile them into the extension's SIDE_MODULE. That makes
    saveSetjmp / testSetjmp / __wasm_longjmp DEFINED symbols inside our
    wasm, which resolves libturbojpeg.a's undefined references without
    touching its semantics. Graceful longjmp-based unwind works again.

    This function remains only to auto-revert the old injection if it's
    still present in the user's ezvcpkg cache from an earlier build. Safe
    to call repeatedly; no-op once the marker is gone."""
    try:
        vcpkg_base = find_ezvcpkg_path()
        port_dir = os.path.join(vcpkg_base, "ports", "libjpeg-turbo")
        portfile = os.path.join(port_dir, "portfile.cmake")
        if not os.path.exists(portfile):
            return
        with open(portfile, "r", encoding="utf-8") as f:
            content = f.read()
        marker = "# CESIUM_NO_SETJMP_PATCH"
        if marker not in content:
            return  # nothing to clean up
        # Strip the injected block. Matches the block we wrote: marker line,
        # 2 comment lines, a vcpkg_replace_string(...) spanning to a closing
        # `)`, then a blank line. Use a non-greedy regex so we only consume
        # up to the first closing `)` after the marker.
        import re
        pattern = re.compile(
            re.escape(marker) + r"\n"
            r"# Neutralize setjmp/longjmp.*?\n"
            r"# See CesiumBuildUtils\.py.*?\n"
            r"vcpkg_replace_string\([\s\S]*?\)\n\n",
            re.MULTILINE,
        )
        cleaned, n = pattern.subn("", content, count=1)
        if n == 0:
            # Fallback: marker present but the surrounding shape doesn't
            # match — user may have hand-edited. Leave the file alone and
            # surface a warning so they know to clean it up manually.
            print("[CESIUM] Warning: CESIUM_NO_SETJMP_PATCH marker found in "
                  f"{portfile} but block shape did not match the regex. "
                  "Remove the marker + vcpkg_replace_string block manually.")
            return
        with open(portfile, "w", encoding="utf-8") as f:
            f.write(cleaned)
        print("[CESIUM] Reverted CESIUM_NO_SETJMP_PATCH from libjpeg-turbo portfile; "
              "forcing rebuild from clean source")
        # Same force-rebuild logic the inject path used: remove the
        # installed copy and nuke the binary archive cache so vcpkg
        # actually recompiles.
        exec_ext = ".exe" if os.name == OS_WIN else ""
        vcpkg_exe = os.path.join(vcpkg_base, "vcpkg" + exec_ext)
        if os.path.exists(vcpkg_exe):
            subprocess.run(
                [vcpkg_exe, "--vcpkg-root", vcpkg_base, "remove",
                 f"libjpeg-turbo:{determine_triplet()}", "--recurse"],
                cwd=vcpkg_base,
            )
        archives_override = os.environ.get("VCPKG_DEFAULT_BINARY_CACHE", "")
        if archives_override:
            archives_dir = archives_override
        elif os.name == OS_WIN:
            archives_dir = os.path.join(os.environ.get("LOCALAPPDATA", ""), "vcpkg", "archives")
        elif sys.platform == PLATFORM_MACOS:
            archives_dir = os.path.expanduser("~/Library/Caches/vcpkg/archives")
        else:
            archives_dir = os.path.expanduser("~/.cache/vcpkg/archives")
        if archives_dir and os.path.exists(archives_dir):
            try:
                shutil.rmtree(archives_dir)
                print(f"[CESIUM] Cleared vcpkg binary archive cache: {archives_dir}")
            except Exception as e:
                print(f"[CESIUM] Warning: could not clear vcpkg binary cache: {e}")
    except Exception as e:
        print(f"[CESIUM] Warning: could not revert libjpeg-turbo port patch: {e}")


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
    # KTX's vcpkg port applies 0001-Use-vcpkg-zstd.patch, which rewrites KTX
    # to find_package(zstd) instead of using its vendored copy. But KTX's
    # vcpkg.json doesn't declare zstd as a dep for our feature set, so ezvcpkg
    # doesn't pull it in when building cesium-native. Install it explicitly
    # for web so it's present in installed/wasm32-emscripten/share/zstd/ by
    # the time ezvcpkg_fetch processes ktx.
    if platform == PLATFORM_WEB:
        subprocess.run([executable, "install"] + allow_unsupported + ["zstd:%s" % triplet])
    # Runs after vcpkg install so the patch survives a fresh fmt reinstall.
    if platform == PLATFORM_WEB:
        patch_fmt_consteval({"platform": platform})


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
