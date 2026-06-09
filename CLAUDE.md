# CLAUDE.md

Guidance for Claude Code (claude.ai/code) when working in this repository.

## Project overview

GDExtension that integrates **Cesium Native 3D Tiles** into the Godot Engine. Streams real-world 3D content (photogrammetry, terrain, imagery) from Cesium Ion or any 3D Tiles–compatible source.

Two compile targets:
- **GDExtension** (`compileTarget=extension`) — shared library loaded at runtime via Godot's GDExtension API. The primary target. Output: `godot3dtiles/addons/cesium_godot/lib/Godot3DTiles.<platform>.<target>.<precision>.<arch>.{dll|so|wasm}`.
- **Engine module** (`compileTarget=module`) — same code linked statically into a custom Godot engine build. Used by the consuming project (AtomEngine) on its hot paths. Driven by `cesium_godot/SCsub` with `_is_module=True` and an outer Godot `scons` invocation.

The consuming app is **AtomEngine**, a peer repository that pulls this in either as a GDExtension addon or via the module path.

## Build entry points

Don't run `scons` directly unless you know what you're doing — both `build.bat` (Windows) and `build.sh` (Linux/Mac) drive scons with the right flag set, the right cesium-native CMake step, and the right per-platform vcpkg triplet. They also auto-clone `godot-cpp`, `cesium-native`, `litehtml`, and (where needed) `emsdk`.

```
build.bat <TARGET>      # Windows
./build.sh <TARGET>     # Linux / macOS
```

| TARGET | What it builds | Notes |
|--------|----------------|-------|
| `extension` (default) | Windows x64 GDExtension, release + debug | Stock path — what you want most of the time |
| `web` | wasm32 GDExtension, release + debug | Legacy; CI no longer covers this, but the path still works for fallback |
| `web64` | wasm64 (MEMORY64) GDExtension, release + debug | **Production web target.** Auto-installs/activates emsdk 4.0.11 at `%USERPROFILE%\emsdk-cesium` or `~/emsdk-cesium`. Applies `patch_emsdk_for_wasm64.py` to fix a known libpthread.js bug |
| `module` | Prepares cesium-native + ABI files for the engine module path | Doesn't produce the final binary — the consuming Godot engine build does |
| `clean` | Wipes `cesium_godot/native/build-*` cmake trees + `godot-cpp/bin/` + litehtml's `build-web*` | Use after vcpkg pin bumps or cesium-native CMakeLists edits |
| `clean-deep` | `clean` + strips stale wasm32/wasm64-emscripten vcpkg state | Recovery only — see "common failures" below |

The `build.bat web64` flow auto-installs **emsdk 4.0.11** at `%USERPROFILE%\emsdk-cesium` (a separate install from any Godot emsdk you may have). Variables are prefixed `_CESIUM_EMSDK_*` to avoid colliding with names that `emsdk activate` clears mid-script.

## Platform status

| Platform | Status | Notes |
|----------|--------|-------|
| Windows x64 | **✓ shipping** | MSVC 2022, `x64-windows-static` vcpkg triplet |
| Linux x64 | **✓ shipping** | GCC/Clang, `x64-linux` triplet, needs `libcurl4-openssl-dev` |
| Web (wasm32) | **✓ legacy** | emsdk 3.1.56-era path, still works but not CI-covered |
| Web (wasm64 / MEMORY64) | **✓ shipping** | emsdk 4.0.11. See [PLATFORM_HANDOFF.md](PLATFORM_HANDOFF.md) and "wasm64 patches" below for the patches that made this work |
| macOS arm64 | **⚠ scaffolded, unverified** | All build-system scaffolding exists (`arm64-osx` triplet, `GodotHttpClient` for the curl-DNS workaround, `build_litehtml` for macOS). Never validated end-to-end. See [PLATFORM_HANDOFF.md](PLATFORM_HANDOFF.md) |
| Android | ✗ not started | |
| iOS | ✗ not started | |

## Architecture

```
3D-Tiles-For-Godot/
├── SConstruct.py              # scons entry — dispatches to extension/module path
├── CesiumBuildUtils.py        # build orchestrator: clones, patches, cmake, triplets
├── build.bat / build.sh       # human-facing wrappers
├── patch_emsdk_for_wasm64.py  # legacy copy; vendored copy in godot/misc/scripts/
├── cesium_godot/              # GDExtension binding code (audited & supported)
│   ├── SCsub                  # source list (one place to add new .cpp files)
│   ├── register_types.cpp     # GDExtension class registration
│   ├── Models/                # Cesium3DTile, CesiumGDTileset, CesiumGlobe, etc.
│   ├── Implementations/       # NetworkAssetAccessor, GodotPrepareRenderResources
│   ├── Utils/                 # HTTP clients, asset builder, texture loader
│   │   ├── WebFetchClient.cpp       # web: browser fetch() via EM_JS
│   │   ├── GodotHttpClient.h        # macOS: Godot HTTPClient (curl has DNS/SNI issues on mac)
│   │   └── CurlHttpClient.h         # Windows/Linux: libcurl
│   ├── Shaders/, third_party/, native/ (gitignored)
│   └── config.py              # module build config
├── cesium_auxiliars/          # small C++ helpers compiled separately
├── godot-cpp/                 # submodule — Godot C++ bindings (patched, see below)
└── godot3dtiles/addons/cesium_godot/lib/  # build output target
```

### Source list

`cesium_godot/SCsub` enumerates every `.cpp` explicitly (no `Glob`). Adding a new translation unit requires editing the `sources` list around line 75.

### HTTP client selection

`cesium_godot/Implementations/NetworkAssetAccessor.h` picks the backend at compile time:

```cpp
#if defined(__EMSCRIPTEN__)
  #include "../Utils/WebFetchClient.h"      // EM_JS browser fetch()
#elif defined(__APPLE__)
  #include "../Utils/GodotHttpClient.h"     // Godot's HTTPClient (curl has DNS/SNI issues on mac)
#else
  #include "../Utils/CurlHttpClient.h"      // libcurl (Windows + Linux)
#endif
```

Each one implements the same `WebFetchClient`-shaped interface (`init_client`, `send_request`, `add_default_header`, ...). When porting to a new platform, decide which backend fits and conditionally select it here.

## CesiumBuildUtils.py — the patch system

The cesium-native upstream isn't directly buildable against our toolchain on every platform. `CesiumBuildUtils.py` applies a set of **patches** at scons time, before cmake runs. Patches are idempotent (detect "already patched" markers) so re-running is safe.

| Function | What it does | When it runs |
|----------|--------------|--------------|
| `clone_native_repo_if_needed()` | Clones `cesium-native` into `cesium_godot/native/` (gitignored) at a pinned commit | First scons run on any platform |
| `patch_cesium_gltf_model_glm_include()` | Adds a missing `#include <glm/...>` to a cesium-native header | Every scons run; idempotent |
| `clone_bindings_repo_if_needed()` | Clones `godot-cpp` if missing, runs `patch_godot_cpp_web_flags()` and `patch_emsdk_libpthread_wasm64_bigint()` | First scons run, web only |
| `patch_godot_cpp_web_flags()` | Edits `godot-cpp/tools/web.py` to accept `arch=wasm64` and emit `-sMEMORY64=1` | Web only |
| `patch_emsdk_libpthread_wasm64_bigint(emsdk_dir)` | Wraps `pthread_ptr` calls in `emsdk/.../libpthread.js` with `to64()` so dlopen works under MEMORY64+pthreads | Web64 only |
| `patch_vcpkg_wasm_triplet_pthread()` | Injects `-pthread -fPIC` into the wasm vcpkg triplet so dep .a files match our SIDE_MODULE link | Web only |
| `patch_fmt_consteval()` | Strips `consteval` from a `fmt` header that emsdk 4.0.x's clang can't compile in our config | Web only |
| `patch_ktx_disable_js_bindings()` | Disables KTX's Emscripten JS bindings (they conflict with our build) | Web only |
| `patch_libjpeg_turbo_port_no_setjmp()` | Removes `-sSUPPORT_LONGJMP=emscripten` from libjpeg-turbo's port so the SIDE_MODULE link doesn't pull in unresolvable JS longjmp shims | Web only |
| `patch_ezvcpkg_allow_unsupported()` | Adds `--allow-unsupported` to ezvcpkg's call so wasm64-emscripten ports build despite the triplet being marked "unsupported" upstream | Web only |
| `_hide_conflicting_vcpkg_triplets()` / `_restore_*` | Temporarily renames vcpkg's auto-installed dynamic triplets so cmake picks our static one. **Critical — see the `feedback_vcpkg_triplet` memory entry** | Every Windows scons run |

**If a patch silently regresses, the symptom is usually a CMake/vcpkg config error during the cesium-native build step, not a runtime crash.** Re-read CesiumBuildUtils.py output carefully on any unexpected failure.

## Web (wasm64) — the big work

The web target was the dominant work item this cycle. Detailed runtime gotchas are documented in the consuming Godot fork's `CLAUDE.md` (the patched `library_godot_*.js` files live there, not in this repo). Summary of what's specific to **this** repo:

1. **`cesium_godot/Utils/WebFetchClient.cpp`** — the only file in this repo with substantial wasm64 surface area. Five separate bugs were fixed in this file:
   - Refresh `HEAPU8` (via `growMemViews()`) after `_malloc` so the subsequent `HEAPU8.set` doesn't write to a detached view (UTF-8 cascade fix)
   - Detect wasm64 at runtime (`typeof HEAPU64 !== 'undefined'`) and read the `char* const*` header array with 8-byte stride via HEAPU64 (not 4-byte HEAPU32)
   - `Number()`-wrap pointer offsets at every typed-array boundary
   - `BigInt()`-wrap the `dataPtr` when calling back into wasm (since `uint8_t*` is i64 under MEMORY64)
   - `BigInt()`-wrap the function-table index passed to `wasmTable.get()` (the table is i64-indexed because dlink+MEMORY64 forces table64)
   - Migrated from legacy `emscripten_async_run_in_main_runtime_thread` (opcode-based, can't dispatch pointer-arg signatures under MEMORY64) to modern `<emscripten/proxying.h>::emscripten_proxy_async`

2. **emsdk pin**: 4.0.11 for web64. The build's `ensure_emsdk` clones to `~/emsdk-cesium` / `%USERPROFILE%\emsdk-cesium` so it doesn't fight the Godot fork's emsdk (also 4.0.11, at `%USERPROFILE%\emsdk`).

3. **godot-cpp patched** (via `patch_godot_cpp_web_flags`) — its stock `tools/web.py` hardcodes `wasm32`. We need it to honor `arch=wasm64` and pass `-sMEMORY64=1` through to emcc.

## Common failure modes

### `vcpkg_cmake_configure` not found
The build temporarily hides conflicting auto-installed vcpkg triplets; if the rename/restore got interrupted, you may see stale state. Fix: `build.bat clean` then re-run.

### Web build complains about an `*-emscripten` port being missing after it was just installed
vcpkg's status DB is out of sync with the filesystem. `build.bat clean-deep` clears wasm32/wasm64 emscripten state. Sometimes also needs:
```cmd
cd %EZVCPKG_BASEDIR%\<commit>
vcpkg.exe remove --recurse --triplet=wasm32-emscripten <name>
```

### `LNK1181: cannot open input file 'absl_<name>.lib'` on Windows
Abseil renames `absl_*` libs between versions. `cesium_godot/SCsub`'s Windows abseil list is hand-curated; compare it against `dir %EZVCPKG_BASEDIR%\<commit>\installed\x64-windows-static\lib\absl_*.lib` and reconcile.

### `Cannot mix BigInt and other types` at JS↔wasm boundary on web64
Almost always a missed `Number()` wrap (input from wasm) or missed `BigInt()` wrap (going back to wasm). The patterns in `WebFetchClient.cpp`'s EM_JS body are the canonical fix templates.

### `Invalid Emscripten pthread _do_call opcode!` on web64
Someone added a `emscripten_async_run_in_main_runtime_thread` / `_sync_*` / `dispatch_to_thread` call. The legacy proxy API can't dispatch pointer-arg signatures under MEMORY64. Migrate to `<emscripten/proxying.h>::emscripten_proxy_async` — see `WebFetchClient.cpp` `start_fetch` else-branch for the template.

### `Invalid UTF-8 leading byte` cascade during tile streaming
A new code path is calling `_malloc` and then writing through `HEAPU8.set` without refreshing the view. Add `growMemViews()` between the two and `Number()`-wrap the offset.

### Existing memory entries worth checking first

The `MEMORY.md` index has accumulated specific gotchas across this work. When debugging any wasm/build-system failure, check these first — most failure modes have already been hit:

- `feedback_vcpkg_triplet` — Windows triplet hiding/restoring
- `feedback_wasm_build` — `-fPIC` requirements, vcpkg cache layers
- `project_cesium_native_patching` — patches go in CesiumBuildUtils, not committed to native/
- `project_emsdk_version_pin` — why 4.0.11 specifically
- `project_emsdk_wasm_longjmp_bug` — longjmp lowering history (3.1.56 → 3.1.60)
- `feedback_emsdk_reserved_vars` — don't name a var `EMSDK_DIR` in any bat that runs `emsdk activate`
- `feedback_em_js_memory_grow` — the malloc→HEAPU8 detachment pattern
- `feedback_em_func_sig_memory64` — why we use the modern proxying API
- `feedback_wasm_table64_bigint` — `wasmTable.get(BigInt)` requirement
- `project_web_http_client_paths` — the design history of `WebFetchClient`
- `project_web_tile_budget` — 512 MB cache + SSE ≤ 16 needed for deep-LOD
- `project_web_material_path` — web skips `BaseMaterial3D` for thread-safety reasons

## Code conventions

- C++20 throughout (`-std=c++20` on POSIX, `/std:c++20` on MSVC)
- Exceptions enabled (`-fexceptions`); cesium-native requires them
- `-fPIC` everywhere on POSIX, mandatory for the SIDE_MODULE link on web
- Always pass `precision=double` — the consuming Godot engine is built `precision=double`, mismatched precision causes silent ABI corruption
- Always pass `custom_modules=battle_road` when building the engine module path against this code (the consuming engine fork expects it)

## When you make a change

1. **Touching anything under `cesium_godot/Utils/WebFetchClient.cpp`** — verify the JS body still handles BigInt/Number correctly on both wasm32 and wasm64. The five bug categories listed above are the audit checklist.
2. **Adding a new `.cpp` to `cesium_godot/`** — add it to the `sources` list in `cesium_godot/SCsub` (~line 75). It will NOT be auto-picked up.
3. **Bumping cesium-native** — re-run `build.bat clean` first so the build-* cmake trees regenerate against the new sources.
4. **Bumping emsdk** — read [PLATFORM_HANDOFF.md](PLATFORM_HANDOFF.md) §"Toolchain bumps". Every emsdk version in our history has either introduced or fixed a MEMORY64 bug; nothing is safe by default.
5. **Adding a new platform** — see [PLATFORM_HANDOFF.md](PLATFORM_HANDOFF.md).
