#!/usr/bin/env python
import os
import sys
import CesiumBuildUtils as cesium_build_utils

LIB_NAME = "Godot3DTiles"

# Glob source files
sources = Glob("cesium_auxiliars/*.cpp")


def add_source_files(self, p_sources):
    sources.extend(p_sources)


compilationTarget: str = cesium_build_utils.get_compile_target_definition(ARGUMENTS)
is_module = compilationTarget == cesium_build_utils.CESIUM_MODULE_DEF

# Clone all the needed projects
if not is_module:
    cesium_build_utils.clone_bindings_repo_if_needed()

cesium_build_utils.clone_native_repo_if_needed()
cesium_build_utils.clone_lite_html_if_needed()

cesium_build_utils.compile_native(ARGUMENTS)

# Build litehtml from source on macOS (no pre-built binaries available)
if sys.platform == cesium_build_utils.PLATFORM_MACOS:
    cesium_build_utils.build_litehtml()

if is_module:
    # Module build: the environment comes from the Godot engine build system.
    # This SConstruct is only used to prepare dependencies (clone, compile native).
    # The actual compilation happens via cesium_godot/SCsub when placed under
    # the engine's modules/ directory.
    #
    # To build as a module:
    #   1. Clone the Godot engine source
    #   2. Symlink or copy cesium_godot/ into <godot-source>/modules/cesium_godot/
    #   3. Copy cesium_auxiliars/ into <godot-source>/modules/cesium_godot/cesium_auxiliars/
    #   4. Run this SConstruct first with: scons compileTarget=module buildCesium=yes
    #      (this only clones and compiles Cesium Native dependencies)
    #   5. Then build the engine from the Godot source root:
    #      scons platform=<platform> target=<target> module_cesium_godot_enabled=yes
    print("")
    print("=" * 70)
    print("[CESIUM] Module dependencies prepared successfully.")
    print("[CESIUM] To complete the build:")
    print("[CESIUM]   1. Symlink cesium_godot/ to <godot-source>/modules/cesium_godot/")
    print("[CESIUM]   2. Build Godot from the engine source root with:")
    print("[CESIUM]      scons platform=<platform> target=<target>")
    print("=" * 70)
    print("")
else:
    # GDExtension build: use godot-cpp bindings
    env = SConscript("godot-cpp/SConstruct")
    cesium_build_utils.generate_precision_symbols(ARGUMENTS, env)
    env.Append(CXXFLAGS=cesium_build_utils.get_compile_flags(env))
    env.Append(LINKFLAGS=cesium_build_utils.get_linker_flags(env))

    cesium_build_utils.install_additional_libs()

    env.Append(CPPDEFINES=[compilationTarget])
    if env.get("platform", "") == cesium_build_utils.PLATFORM_WEB:
        # Web-specific defines
        env.Append(CPPDEFINES=["EMSCRIPTEN"])
    elif os.name == cesium_build_utils.OS_LINUX:
        env.Append(CPPDEFINES=["CURL_STATIC_LIB", "SQLITE_STATIC"])
    env.__class__.add_source_files = add_source_files

    # Append include paths
    env.Append(CPPPATH=["testSrc/", "cesium_godot/", "cesium_auxiliars/"])

    # Run the SCsub that is under cesium_godot/
    SConscript("cesium_godot/SCsub", exports="env")

    # Create shared library
    if env["platform"] == "macos":
        library = env.SharedLibrary(
            "godot3dtiles/addons/cesium_godot/lib/lib{}{}{}".format(
                LIB_NAME, env["suffix"], env["SHLIBSUFFIX"]
            ),
            source=sources,
        )
    else:
        library = env.SharedLibrary(
            "godot3dtiles/addons/cesium_godot/lib/{}{}{}".format(
                LIB_NAME, env["suffix"], env["SHLIBSUFFIX"]),
            source=sources,
        )

    # Set the default target
    Default(library)
