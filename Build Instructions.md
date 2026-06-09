# Build Instructions

Guide to building the 3D Tiles for Godot GDExtension from scratch on a fresh machine. Covers Windows, Linux, and Web (WASM) targets.

## 1. Install Prerequisites

### All platforms

- **Git** — <https://git-scm.com/downloads>
- **Python 3.8+** with `pip` — <https://www.python.org/downloads/>
- **SCons**: `pip install scons`
- **CMake 3.1+ and strictly less than 4.0** — <https://cmake.org/download/>
  - Versions 4.x have known compatibility issues with vcpkg ports in this project.

### Windows

- **Visual Studio 2022** with the "Desktop development with C++" workload — <https://visualstudio.microsoft.com/downloads/>
- Windows 10 or 11 (64-bit)

Run `build.bat` from a regular `cmd` prompt. You do NOT need to launch from a Developer Command Prompt — scons auto-detects the MSVC toolchain.

### Linux

- `libcurl4-openssl-dev` (Debian/Ubuntu) or equivalent
- x86_64 architecture
- A C++20-capable GCC or Clang

### Web (WASM) — additional prerequisites

- **Emscripten SDK (emsdk)** pinned to **3.1.56**. Newer versions break the KTX library included via vcpkg.
  ```bash
  git clone https://github.com/emscripten-core/emsdk.git
  cd emsdk
  ./emsdk install 3.1.56
  ./emsdk activate 3.1.56 --permanent
  ```
  Then open a **fresh terminal** before building — `activate` only affects newly launched shells.
- On Windows, the activate script is `emsdk.bat` and env setup is `emsdk_env.bat`.

Verify with `emcc --version` — it must report 3.1.56.

## 2. Clone the Repository

```bash
git clone https://github.com/Battle-Road/3D-Tiles-For-Godot.git
cd 3D-Tiles-For-Godot
```

The build will auto-clone `godot-cpp`, `cesium-native`, and `litehtml` into the correct locations on first run. It will also auto-clone `vcpkg` at a pinned commit into `C:\.ezvcpkg\` (Windows) or `~/.ezvcpkg/` (Linux/Mac) and install dependencies from source — budget **20–45 minutes** for the first build.

## 3. Build

The top-level `build.bat` (Windows) and `build.sh` (Linux/Mac) wrap scons with sensible defaults.

### Build GDExtension for the current platform

```cmd
build.bat extension
```
```bash
./build.sh extension
```

On the first run you'll be prompted:
```
Do you wanna build Cesium Native (Choose yes if it's the first install)? [y/n]
```
Answer **y**. Subsequent builds on the same platform can answer **n** to skip reconfiguration.

### Build GDExtension for Web (WASM)

Activate emsdk 3.1.56 in a fresh terminal, then:
```cmd
build.bat web
```
```bash
./build.sh web
```

### Other targets

- `build.bat module` / `./build.sh module` — prepare cesium-native for a custom Godot engine module build (advanced)
- `build.bat clean` / `./build.sh clean` — remove `cesium_godot/native/build-windows/` and `build-web/` cmake trees. Use after bumping the vcpkg pin or changing cesium-native's CMakeLists.
- `build.bat clean-deep` / `./build.sh clean-deep` — `clean` + wipes stale wasm32-emscripten vcpkg state. Use only for web-build recovery when emscripten or KTX state is corrupted.

## 4. Output

After a successful build:

- **Windows**: `godot3dtiles/addons/cesium_godot/lib/Godot3DTiles.windows.template_release.double.x86_64.dll`
- **Linux**: `godot3dtiles/addons/cesium_godot/lib/libGodot3DTiles.linux.template_release.double.x86_64.so`
- **Web**: `godot3dtiles/addons/cesium_godot/lib/Godot3DTiles.web.template_release.double.wasm32.wasm`

Debug-variant `.dll`/`.so`/`.wasm` files are emitted alongside the release ones.

To use the plugin in a Godot project, copy `godot3dtiles/addons/cesium_godot/` into your project's `addons/` folder. See `README.md` for editor setup.

## 5. Switching Platforms

You can build Windows and Web from the same checkout without cleaning between runs. Each platform has its own:

- cmake tree (`build-windows/` vs `build-web/`)
- vcpkg install tree (per-triplet)
- scons object files (platform-tagged)

Say **n** to "build Cesium Native" on repeat builds of the same platform.

## 6. Troubleshooting

### `Unknown CMake command "vcpkg_cmake_configure"`
A from-source vcpkg port build needs the `vcpkg-cmake` helper. The build script should handle this automatically. If you see this error, confirm `CesiumBuildUtils.py`'s `_hide_conflicting_vcpkg_triplets` is preserving `share/vcpkg-cmake*` subdirs.

### Web build fails with `val(dst, allow_raw_pointers())`
Your emscripten is too new. Downgrade to 3.1.56 (see prerequisites) and open a fresh terminal.

### Windows linker error `LNK1181: cannot open input file 'absl_<name>.lib'`
Abseil's library names occasionally change between versions. Compare `cesium_godot/SCsub`'s Windows abseil list against what's actually installed:
```cmd
dir /b C:\.ezvcpkg\<commit>\installed\x64-windows-static\lib\absl_*.lib
```
Remove missing entries and add new ones, then re-link with `build.bat extension` (answer **n**).

### Web build complains about zstd or another wasm32 dep being "not found" after it was just installed
Your vcpkg status DB is out of sync with the filesystem. Run:
```cmd
build.bat clean-deep
```
If that doesn't clear it, surgically uninstall all wasm32 packages:
```cmd
cd C:\.ezvcpkg\<commit>
for /f "tokens=1 delims=:" %i in ('vcpkg.exe list ^| findstr ":wasm32-emscripten"') do vcpkg.exe remove --recurse --triplet=wasm32-emscripten %i
```
Then rebuild.

### `cesium_godot/native/extern/vcpkg/ports/ktx/` exists and build complains about overlay port
A previous experimental patch left a stale overlay port. Delete it:
```cmd
rmdir /s /q cesium_godot\native\extern\vcpkg\ports\ktx
```

## 7. Double-Precision Builds

All `build.bat`/`build.sh` targets pass `precision=double` by default. To use the resulting `.dll` with Godot, the Godot editor and export templates must also be built with `precision=double`. See `README.md` for details.
