#!/usr/bin/env python
import os
import sys
import CesiumBuildUtils as cesium_build_utils

LIB_NAME = "Godot3DTiles"

# Glob source files
sources = Glob("cesium_auxiliars/*.cpp")


def add_source_files(self, p_sources):
    sources.extend(p_sources)


# Clone all the needed projects
if (cesium_build_utils.is_extension_target(ARGUMENTS)):
    cesium_build_utils.clone_bindings_repo_if_needed()

cesium_build_utils.clone_native_repo_if_needed()
cesium_build_utils.clone_lite_html_if_needed()

cesium_build_utils.compile_native(ARGUMENTS)
env = SConscript("godot-cpp/SConstruct")
cesium_build_utils.generate_precision_symbols(ARGUMENTS, env)
env.Append(CXXFLAGS=cesium_build_utils.get_compile_flags())
env.Append(LINKFLAGS=cesium_build_utils.get_linker_flags())

cesium_build_utils.install_additional_libs()

compilationTarget: str = cesium_build_utils.get_compile_target_definition(ARGUMENTS)

env.Append(CPPDEFINES=[compilationTarget])
if cesium_build_utils.is_linux() or cesium_build_utils.is_macos():
    env.Append(CPPDEFINES=["CURL_STATIC_LIB", "SQLITE_STATIC"])
env.__class__.add_source_files = add_source_files

# Append include paths
env.Append(CPPPATH=["testSrc/", "cesium_godot/", "cesium_auxiliars/"])

# Run the SCsub that is under cesium_godot/
SConscript("cesium_godot/SCsub", exports="env")


# Create shared library
# Output path matches the naming convention in Godot3DTiles.gdextension
library = env.SharedLibrary(
    "godot3dtiles/addons/cesium_godot/lib/{}{}{}".format(
        LIB_NAME, env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

# Set the default target
Default(library)
