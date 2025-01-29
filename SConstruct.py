#!/usr/bin/env python
import os
import sys
import CesiumBuildUtils as cesium_build_utils

LIB_NAME = "Godot3DTiles"

# Glob source files
sources = Glob("cesium_auxiliars/*.cpp")


def add_source_files(self, p_sources):
    sources.extend(p_sources)

env = SConscript("godot-cpp/SConstruct")
env.Append(CXXFLAGS=["/std:c++17"])
env.Append(LINKFLAGS=["/IGNORE:4098", "/FORCE", "/IGNORE:4217"])

compilationTarget: str = cesium_build_utils.get_compile_target_definition(ARGUMENTS)

env.Append(CPPDEFINES=[compilationTarget])
env.__class__.add_source_files = add_source_files

# Append include paths
env.Append(CPPPATH=["testSrc/", "cesium_godot/", "cesium_auxiliars/"])

# Run the SCsub that is under cesium_godot/
SConscript("cesium_godot/SCsub", exports="env")


# Create shared library
if env["platform"] == "macos":
    library = env.SharedLibrary(
        "godot3dtiles/bin/{}.{}.{}.framework/helloWorld.{}.{}".format(
            LIB_NAME, env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "godot3dtiles/bin/{}{}{}".format(
            LIB_NAME, env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

# Set the default target
Default(library)
