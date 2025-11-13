#!/usr/bin/env python
import os
import sys
import CesiumBuildUtils as cesium_build_utils
import shutil

LIB_NAME = "Godot3DTiles"

# Glob source files
sources = Glob("cesium_auxiliars/*.cpp")


def add_source_files(self, p_sources):
    sources.extend(p_sources)


def clone_repositories():
    # Clone all the needed projects
    if (cesium_build_utils.is_extension_target(ARGUMENTS)):
        cesium_build_utils.clone_bindings_repo_if_needed()

    cesium_build_utils.clone_native_repo_if_needed()
    cesium_build_utils.clone_lite_html_if_needed()
    pass


def configure_scsub():
    if (cesium_build_utils.is_extension_target(ARGUMENTS)):
        SConscript("cesium_godot/SCsub", exports="env")
        return
    # Here we need to copy the SCsub that's in ./cesium_godot/SCsub to this directory
    if not os.path.exists("./SCsub"):
        shutil.move("./cesium_godot/SCsub", "./SCsub")
    if not os.path.exists("./config.py"):
        shutil.move("./cesium_godot/config.py", "./config.py")
    if not os.path.exists("./register_types.h"):
        shutil.move("./cesium_godot/register_types.h", "./register_types.h")


def configure_library(p_env):
    if (not cesium_build_utils.is_extension_target(ARGUMENTS)):
        return
    # Create shared library
    if p_env["platform"] == "macos":
        library = p_env.SharedLibrary(
            "godot3dtiles/bin/{}.{}.{}.framework/helloWorld.{}.{}".format(
                LIB_NAME, p_env["platform"], p_env["target"], p_env["platform"], p_env["target"]
            ),
            source=sources,
        )
    else:
        library = p_env.SharedLibrary(
            "godot3dtiles/bin/{}{}{}".format(
                LIB_NAME, p_env["suffix"], p_env["SHLIBSUFFIX"]),
            source=sources,
        )

    # Set the default target
    Default(library)


clone_repositories()

cesium_build_utils.compile_native(ARGUMENTS)

env = None
if (cesium_build_utils.is_extension_target(ARGUMENTS)):
    env = SConscript("godot-cpp/SConstruct")
    cesium_build_utils.add_compile_definitions(env)
    env.__class__.add_source_files = add_source_files
    # Append include paths
    env.Append(CPPPATH=["testSrc/", "cesium_godot/", "cesium_auxiliars/"])

cesium_build_utils.install_additional_libs()

# Run the SCsub that is under cesium_godot/
configure_scsub()

configure_library(env)
