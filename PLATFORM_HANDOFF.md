# Platform support — handoff writeup

This document is for a colleague picking up the cross-platform build work after the initial wasm64 + Linux + Windows push. It covers:

- The current state of each target
- What's already done for **macOS** (the most likely next target) — and what's not
- A recipe for adding new platforms (Android, iOS, Mac if revived)
- The pitfalls we hit on each existing platform so they don't get re-discovered
- Toolchain bump policy

For day-to-day build commands, see `CLAUDE.md`. This doc is the strategic / archeological one.

## Platform status snapshot

| Platform | Build status | Test status | Notes |
|----------|--------------|-------------|-------|
| **Windows x64** | ✓ green | ✓ used in production | MSVC 2022, static link, libcurl HTTP |
| **Linux x64** | ✓ green | ✓ CI-tested | GCC/Clang, libcurl, requires `libcurl4-openssl-dev` |
| **Web wasm32** | ✓ legacy | ⚠ not in CI | Path still works for compat fallback. Stock emsdk-era code |
| **Web wasm64 (MEMORY64)** | ✓ green | ✓ QA-validated | The big work item of this cycle. emsdk 4.0.11, libpthread.js patched, ~40 boundary fixes |
| **macOS arm64** | ⚠ build-system scaffolded, never linked end-to-end | ✗ never run | See §"macOS handoff" below |
| **Android** | ✗ not started | ✗ | No NDK config, no triplet, no HTTP backend selected |
| **iOS** | ✗ not started | ✗ | No xcframework infrastructure |

## macOS handoff — the most likely next target

macOS support is **partially scaffolded but never validated**. Several pieces are in place because the project was originally designed to be cross-platform; they just haven't been wired together and tested.

### What's already in place

1. **vcpkg triplet selection** (`CesiumBuildUtils.py::determine_triplet`):
   ```python
   if sys.platform == PLATFORM_MACOS:
       return "arm64-osx"
   ```
   Reads `sys.platform == "darwin"` and picks `arm64-osx`. No `x64-osx` (Intel) path exists — adding one is straightforward but the project assumes Apple Silicon.

2. **cmake build directory** (`CesiumBuildUtils.py::get_native_build_dir_name`):
   ```python
   if sys.platform == PLATFORM_MACOS:
       return "build-macos"
   ```
   So cesium-native builds at `cesium_godot/native/build-macos/`.

3. **Native build entrypoint** (`CesiumBuildUtils.py::build_native_macos`):
   ```python
   def build_native_macos(build_path):
       return subprocess.run(["cmake", "--build", build_path])
   ```
   Plain `cmake --build` (no `--config` since macOS uses single-config generators by default). Should work as-is if cmake is wired up.

4. **HTTP client backend** (`cesium_godot/Utils/GodotHttpClient.h` — header-only, ready to use):
   ```cpp
   #elif defined(__APPLE__)
     #include "../Utils/GodotHttpClient.h"
   ```
   Uses Godot's `HTTPClient` instead of libcurl because **curl has DNS resolution and SSL/SNI issues with macOS's network stack**. This is a known cross-platform Cesium issue — there is even an upstream cesium-native note about it. The selection in `NetworkAssetAccessor.h` is already correct; this part is done.

5. **litehtml build path** (`CesiumBuildUtils.py::build_litehtml`):
   ```python
   def build_litehtml(arch="arm64"):
       output_dir = os.path.join(third_party_dir, "litehtml", "macos")
       ...
   ```
   Has a working macOS arm64 build. The output lands in `cesium_godot/third_party/litehtml/macos/`. SConstruct.py already calls `build_litehtml()` on macOS.

6. **C++ flags** (`CesiumBuildUtils.py::get_compile_flags`):
   ```python
   elif sys.platform == PLATFORM_MACOS:
       return ["-std=c++20", "-fexceptions", "-fPIC"]
   ```

7. **vcpkg binary cache directory** is correctly detected at `~/Library/Caches/vcpkg/archives` in two places in `CesiumBuildUtils.py`.

8. **SConstruct.py library naming** has a macOS special-case (line ~94):
   ```python
   if env["platform"] == "macos":
       library = env.SharedLibrary(
           "godot3dtiles/addons/cesium_godot/lib/lib{}{}{}".format(...)
       )
   ```
   Uses the `lib` prefix convention that Apple's loader expects.

### What needs to happen to ship macOS

1. **Add a `macos` target to `build.sh`**. Right now `build.sh` has `extension`, `web`, `web64`, `module`, `clean`, `clean-deep` — but `extension` on macOS hasn't been smoke-tested. Add an explicit:
   ```bash
   macos)
       scons platform=macos arch=arm64 compileTarget=extension target=template_release precision=double production=yes
       scons platform=macos arch=arm64 compileTarget=extension target=template_debug precision=double
       ;;
   ```
   (Note: `platform=macos` is what godot-cpp expects, not `darwin`. SConstruct.py reads `env["platform"]`, while CesiumBuildUtils.py reads `sys.platform == "darwin"`. Both are correct for their respective contexts but the dichotomy will confuse — add a comment.)

2. **First-build smoke test** on a real Apple Silicon machine. Expect cmake/vcpkg failures the first time. Likely friction points based on what we hit elsewhere:
   - **vcpkg port that ships .dylib by default needs `arm64-osx-static`**. The `_hide_conflicting_vcpkg_triplets` helper was written for Windows's dynamic/static collision but may need a macOS variant. **High-confidence area for first-build pain.**
   - **OpenSSL on macOS** can pick up the system's LibreSSL fork instead of vcpkg's. Watch for SSL-related link errors.
   - **`-fPIC` is implicit on macOS** but adding it explicitly (as we do) is harmless — keep.
   - **Codesigning the produced `.dylib`** is not handled by the build. Godot's macOS export pipeline expects this; coordinate with the consuming app.

3. **Verify the GDExtension `.gdextension` manifest lists macos correctly**. Check `godot3dtiles/addons/cesium_godot/cesium_godot.gdextension` for a `macos.debug` / `macos.release` entry mapping to the produced `.dylib` path.

4. **Confirm `GodotHttpClient` actually compiles + links** as a SIDE_MODULE-free static linkage. It depends on `godot_cpp/classes/http_client.hpp` — make sure godot-cpp is built for macos before this is touched (`scons platform=macos` in godot-cpp will produce the right `.a`).

5. **Test against a real tileset over HTTPS** — the whole reason we use `GodotHttpClient` on macOS is the curl/DNS/SNI issue. If it's silently falling back somehow, you'll know.

### What might bite that we haven't seen yet

- **Mach-O double-rpath** — Godot's macOS export wraps the GDExtension into an `.app` bundle. The `.dylib` may need explicit `@rpath` adjustments. Check with `otool -L` after the build.
- **macOS's pthread differences** — cesium-native uses std::thread heavily. We haven't validated the thread-pool work on macOS but there's no obvious reason it should fail.
- **Vulkan vs Metal** — Cesium's GPU resource path goes through Godot's RenderingDevice. On macOS this defaults to Metal. Texture format support differs from Vulkan; KTX2 textures decoded via our path may need fallbacks. Worth profiling first export.

## Adding a brand-new platform (Android / iOS / etc.)

If you're adding Android or iOS, the work is significantly larger than macOS. Here's the rough recipe — none of these steps exist yet:

1. **Pick a vcpkg triplet** for the target. Android NDK has `arm64-android`, `armv7-android`, etc. iOS has `arm64-ios`. Add `determine_triplet()` and `get_native_build_dir_name()` branches.

2. **Add a `build_native_<platform>` cmake invocation** to `CesiumBuildUtils.py` matching the toolchain. Android needs `-DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI=<abi> -DANDROID_PLATFORM=android-XX`. iOS needs the Xcode generator with the right SDK.

3. **Decide HTTP backend.** Android can probably use libcurl (matches Linux). iOS is more like macOS — use `GodotHttpClient` to dodge Apple's networking stack quirks.

4. **Compile flags** in `get_compile_flags()` — Android wants `-fPIC -fexceptions -std=c++20`, iOS adds `-fembed-bitcode` depending on era.

5. **Linker flags** in `get_linker_flags()` — Android Gradle's APK packaging expects `.so` files at `lib/<abi>/`. iOS needs `-fembed-bitcode` to match.

6. **godot-cpp platform support** — check `godot-cpp/tools/<platform>.py` exists and works. Android does; iOS does. Both are stock godot-cpp.

7. **`build.bat` / `build.sh` target entry.** Probably easiest to add to `build.sh` only since neither mobile platform is Windows-buildable.

8. **`.gdextension` manifest entries** for the new platform/arch combos.

9. **Test rig.** Smoke-testing GDExtensions on mobile requires an actual device or simulator + a Godot test project. Budget more time for the test setup than the build itself.

Look at how the **web64** path was added as a reference: search `CesiumBuildUtils.py` for `is_web_memory64()` and trace every `if` branch. Mobile will need a similar fanout but with NDK-specific logic instead of emcc.

## Toolchain bump policy

Every emsdk version in our history either introduced or fixed a MEMORY64 / SIDE_MODULE / pthreads bug. **Bumping the toolchain is a high-risk action.** History (in `CesiumBuildUtils.py::ensure_emsdk` comments and `feedback_*` memory entries):

- **3.1.56**: had a `saveSetjmp` codegen bug in SIDE_MODULE .o files. The build accepted `-sSUPPORT_LONGJMP=wasm` but emitted unresolvable JS shims. **Avoid.**
- **3.1.60**: fixed the longjmp bug. Was usable for wasm32. Insufficient for wasm64 — Godot's `platform/web/detect.py` enforces `emcc >= 3.1.62`.
- **3.1.62**: minimum for wasm64 per Godot's detect.py. But the link is broken: `wasm-opt --table64-lowering` aborts with "i32 != i64: call-indirect call target must match the table index type" because parts of the linker-generated startup emit table64 indices while user TUs emit i32. **Avoid for wasm64.**
- **3.1.74**: completes MEMORY64 dlink work so every TU agrees on table64. But has a separate codegen bug for MEMORY64+SIDE_MODULE+pthreads: produces wasm that fails browser instantiation with "call[N] expected type i32, found i64.add". 3.1.x line ended at .74 (no .75–.79 published). **Avoid.**
- **4.0.11**: current pin. MEMORY64+SIDE_MODULE+pthreads is production-ready, but has its own `libpthread.js` dlsync_threads BigInt bug that we patch via `patch_emsdk_for_wasm64.py`. Stable for our use.
- **4.0.12+ / 4.1.x**: untested. Could fix the dlsync bug we patch, could introduce new ones.

**Bump checklist:**

1. Pin the new version in `CesiumBuildUtils.py::ensure_emsdk` AND in `.github/workflows/web_builds.yml` of the consuming Godot fork. Both must match.
2. Check whether the upstream `libpthread.js` still has the dlsync BigInt bug. If it's fixed upstream, delete `patch_emsdk_for_wasm64.py` and remove the call sites.
3. Re-verify every patch in `CesiumBuildUtils.py` that targets vcpkg ports (KTX, fmt, libjpeg-turbo). These patches' "before" strings have to still match the upstream source.
4. Build wasm64 from scratch (`build.bat clean-deep && build.bat web64`) — never trust an incremental build to surface toolchain incompatibilities.
5. Test the most failure-prone paths: tile streaming (camera move with backgrounded tab), websocket binary frames, HTTP failure recovery, large allocations.
6. Toggle `arch=wasm32` to confirm the wasm32 fallback path still works; that target hasn't been CI-covered since the wasm64 migration but losing it entirely is a step back.

## CI coverage

GitHub Actions for this repo: minimal. The consuming Godot fork's `.github/workflows/web_builds.yml` is the only CI that exercises wasm64 end-to-end (it builds Godot's web template with our GDExtension flags). This repo's own CI focuses on Windows/Linux.

**Adding macOS CI** when the platform is ready: the GitHub-hosted `macos-13` / `macos-14` runners are arm64 (M1) — match our triplet. Budget the first run for ~30–60 minutes because vcpkg builds everything from source.

## Where to read more

- `CLAUDE.md` — AI assistant guidance (commands, gotchas)
- `Build Instructions.md` — user-facing build prerequisites and steps (note: written in the wasm32 era; macOS section is sparse)
- `README.md` — feature overview, GDExtension installation guidance
- `CesiumBuildUtils.py` — the source of truth for all patches and platform branches; ~1500 lines, well-commented at the patch functions
- The `MEMORY.md` index for accumulated gotchas — read the relevant entries before debugging
