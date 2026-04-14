# Building on macOS (Apple Silicon)

The `SConstruct.py` already contains a `platform == "macos"` branch, so
building on macOS works out of the box — it is simply not documented or
listed in the README's supported platforms. This doc captures the exact
steps that succeeded on Apple Silicon (tested on M3 Pro, Godot 4.6.2).

## Prerequisites

```bash
brew install pkg-config cmake ninja openssl@3
pip3 install --user scons
```

- `pkg-config`: required by vcpkg (fmt fails to build without it).
- `openssl@3`: macOS's bundled LibreSSL breaks Cesium Native's SSL build.
- `cmake` + `ninja`: C++ build tools.
- `scons`: the build orchestrator used by this repo.

## Environment

Export OpenSSL root so Cesium Native picks up Homebrew's OpenSSL
instead of the system LibreSSL:

```bash
export OPENSSL_ROOT_DIR=$(brew --prefix openssl@3)
export OPENSSL_DIR=$(brew --prefix openssl@3)
```

## Build

First run will clone Cesium Native into `cesium_godot/native/`, then ask
interactively whether to build Cesium Native. Pipe `yes` so scons proceeds
non-interactively:

```bash
yes | scons platform=macos arch=arm64 compileTarget=extension \
            target=template_release production=yes
```

Build takes ~15–20 minutes on M3 Pro. Output:

```
godot3dtiles/addons/cesium_godot/lib/libGodot3DTiles.macos.template_release.arm64.dylib
```

(~16 MB, Mach-O 64-bit shared library for arm64.)

## Wiring into a Godot project

1. Copy `godot3dtiles/addons/cesium_godot` into your project's `addons/`.
2. The `Godot3DTiles.gdextension` already references the macOS arm64 paths
   for both `macos.debug.arm64` and `macos.release.arm64`. If you only
   built `template_release`, duplicate it as debug so the editor loads it:
   ```bash
   cp addons/cesium_godot/lib/libGodot3DTiles.macos.template_release.arm64.dylib \
      addons/cesium_godot/lib/libGodot3DTiles.macos.template_debug.arm64.dylib
   ```
3. Enable the plugin in `Project → Project Settings → Plugins`
   (or append `"res://addons/cesium_godot/plugin.cfg"` to
   `editor_plugins/enabled` in `project.godot`).

Verify the GDExtension loaded:

```bash
godot --headless -s /path/to/probe.gd
```

where `probe.gd` does:

```gdscript
extends SceneTree
func _init():
    for name in ("CesiumGeoreference", "Cesium3DTileset", "CesiumIonRasterOverlay"):
        print(name, " registered: ", ClassDB.class_exists(name))
    quit()
```

Expected: all three print `true`.

## Known quirks on macOS

- `CesiumDynamicCamera`'s default `move_speed = 100` m/s is designed for
  globe-scale navigation and is unusable at building scale. For
  Cartographic origin at city level, either lower `move_speed` or write
  a custom camera that calls `tileset.update_tileset(camera_xform)` each
  frame.
- The default shaders reference
  `res://Shaders/spatial_texture_rotation.gdshader` which does not ship in
  `addons/cesium_godot/`. Missing-shader errors appear on scene load but
  do not block tile streaming. (Tracking separately.)
- `Godot3DTiles.gdextension` lists both `macos.debug.arm64` and
  `macos.release.arm64`. Distributing pre-built binaries of both
  variants would unblock users who can't compile C++.

## Rationale

Several users on forums ask whether the plugin "works on Mac". The
answer is "yes, the code is ready" — only the build toolchain friction
and the README listing discourage them. This doc closes that gap without
changing any build code.
