@echo off
setlocal

rem Default ezvcpkg location if not already set. The quoted form catches both
rem unset and empty-string cases; `if not defined` misses the empty case.
if "%EZVCPKG_BASEDIR%"=="" set EZVCPKG_BASEDIR=C:\.ezvcpkg

rem Default emsdk install location and version. Only used by the web target
rem (see :ensure_emsdk). Override EMSDK_DIR to point at an existing checkout;
rem override EMSDK_VERSION to pin to a different toolchain.
rem
rem 3.1.56 is pinned for this repo because newer emsdk breaks the KTX library
rem from vcpkg. The Godot engine itself uses a newer emsdk (4.0.11+) which
rem typically lives at %USERPROFILE%\emsdk — keep this repo's emsdk in a
rem separate directory so both installs can coexist and each project can
rem activate its own required version without fighting the other.
if "%EMSDK_DIR%"=="" set EMSDK_DIR=C:\emsdk-cesium
rem Version history:
rem   3.1.56 -> 3.1.60: picked up the -sSUPPORT_LONGJMP=wasm compile-time lowering
rem     fix. 3.1.56 accepted the flag but emitted saveSetjmp/testSetjmp imports in
rem     SIDE_MODULE .o files anyway, which Godot's main wasm can't resolve.
rem   3.1.60 -> 3.1.62: required for wasm64. Godot's platform/web/detect.py enforces
rem     emcc >= 3.1.62, and the wasm64 SIDE_MODULE dyncall metadata format stabilized
rem     in 3.1.62 — building the extension at 3.1.60 against a 3.1.62+ main module
rem     yields "Cannot set properties of undefined (setting 'sig')" at runtime
rem     because function-table entries fail to wire up.
rem   3.1.62 -> 3.1.74: 3.1.62 + MEMORY64 + SIDE_MODULE link is broken — wasm-opt's
rem     --table64-lowering pass aborts with "i32 != i64: call-indirect call target
rem     must match the table index type" because parts of the linker-generated
rem     startup code emit table64 call_indirects while user TUs emit i32.wrap_i64
rem     table32 indices, and the pass refuses to lower mixed inputs. 3.1.74
rem     completes the MEMORY64 dlink work so every TU agrees on table64.
rem   3.1.74 -> 4.0.11: 3.1.74 had a codegen bug somewhere in the
rem     MEMORY64 + SIDE_MODULE + pthreads combo (flagged at link time as
rem     `-sSIDE_MODULE + pthreads is experimental [-Wexperimental]`). The
rem     produced wasm passed wasm-opt validation but the browser rejected at
rem     instantiation: "call[3] expected type i32, found i64.add of type i64"
rem     — a direct-call argument mismatch deterministic at the same function
rem     index across rebuilds, so it's a toolchain bug rather than our build
rem     config. The 3.1.x line stopped at 3.1.74 (no 3.1.75-3.1.79 in emsdk's
rem     registry), so we jumped to 4.0.x where MEMORY64 + SIDE_MODULE +
rem     pthreads is production-ready. 4.0.11 was the floor at jump time and
rem     happened to already be installed locally.
rem If KTX or any other dep regresses at a higher emsdk, step up incrementally
rem (4.0.23, etc.) rather than reverting — older toolchains have known
rem dlink-ABI gaps that re-surface as opaque runtime errors.
if "%EMSDK_VERSION%"=="" set EMSDK_VERSION=4.0.11

set TARGET=%1
if "%TARGET%"=="" set TARGET=extension

if "%TARGET%"=="extension" goto :build_extension
if "%TARGET%"=="web" goto :build_web
if "%TARGET%"=="web64" goto :build_web64
if "%TARGET%"=="module" goto :build_module
if "%TARGET%"=="clean" goto :clean
if "%TARGET%"=="clean-deep" goto :clean_deep
goto :usage

:build_extension
echo Building GDExtension for Windows x64...
scons arch=x64 compileTarget=extension target=template_release precision=double production=yes compiledb=yes
scons arch=x64 compileTarget=extension target=template_debug precision=double compiledb=yes
goto :done

:build_web
echo Building GDExtension for Web/WASM (wasm32)...
set CESIUM_WEB_MEMORY64=
call :ensure_emsdk
if errorlevel 1 goto :done
rem Force emscripten-style longjmp to avoid conflict with godot-cpp's exception handling.
rem -pthread enables atomics/bulk-memory needed for --shared-memory at link time.
rem EMCC_CFLAGS is read by emcc/em++ for ALL compilations (vcpkg, cmake, scons).
rem -fwasm-exceptions: native wasm exception handling (no JS invoke_* wrappers)
rem -sSUPPORT_LONGJMP=wasm: native wasm longjmp (pairs with -fwasm-exceptions)
rem -pthread -fPIC: required for threaded SIDE_MODULE builds
rem -Wno-overriding-option: emsdk 3.1.74's clang flags `-ffp-model=precise` +
rem `-ffp-contract=off` as a redundant override, which KTX's astc-encoder
rem subbuild upgrades to fatal via -Werror -Wpedantic.
set EMCC_CFLAGS=-fwasm-exceptions -sSUPPORT_LONGJMP=wasm -pthread -fPIC -Wno-overriding-option
scons platform=web compileTarget=extension target=template_release precision=double production=yes disable_exceptions=no
scons platform=web compileTarget=extension target=template_debug precision=double disable_exceptions=no
goto :done

:build_web64
echo Building GDExtension for Web/WASM (wasm64 / Memory64)...
echo NOTE: requires a Godot engine built with MEMORY64=1; without it the .wasm
echo       will compile but Godot's web export will refuse to load it.
rem Signal to CesiumBuildUtils.py to select the wasm64-emscripten vcpkg triplet,
rem the build-web64/ cmake tree, and the wasm64 output filename suffix.
set CESIUM_WEB_MEMORY64=1
call :ensure_emsdk
if errorlevel 1 goto :done
rem Same SIDE_MODULE prerequisites as wasm32, plus -sMEMORY64=1 which switches
rem clang/emcc into Memory64 codegen (i64 wasm pointers, 64-bit size_t).
rem -Wno-experimental: emsdk emits a -Wexperimental warning on every MEMORY64
rem compile; some deps (KTX/astc-encoder) build with -Werror -Wpedantic which
rem upgrades it to a fatal error without this suppress.
rem -Wno-overriding-option: emsdk 3.1.74's clang flags `-ffp-model=precise` +
rem `-ffp-contract=off` (passed together by KTX's astc-encoder) as a redundant
rem override, which becomes fatal via -Werror -Wpedantic without this suppress.
set EMCC_CFLAGS=-fwasm-exceptions -sSUPPORT_LONGJMP=wasm -pthread -fPIC -sMEMORY64=1 -Wno-experimental -Wno-overriding-option
rem arch=wasm64 routes through godot-cpp's tools/web.py (CESIUM-patched to accept
rem wasm64 and emit -sMEMORY64=1). Without it godot-cpp defaults to wasm32 and
rem builds its library with i32 table indices, which wasm-opt --table64-lowering
rem rejects against the i64-indexed link from cesium-native + the extension.
scons platform=web arch=wasm64 compileTarget=extension target=template_release precision=double production=yes disable_exceptions=no
scons platform=web arch=wasm64 compileTarget=extension target=template_debug precision=double disable_exceptions=no
goto :done


:build_module
echo Preparing module dependencies...
scons compileTarget=module buildCesium=yes
goto :done

:clean
echo Cleaning cesium-native build trees...
if exist cesium_godot\native\build-windows rmdir /s /q cesium_godot\native\build-windows
if exist cesium_godot\native\build-web rmdir /s /q cesium_godot\native\build-web
if exist cesium_godot\native\build-web64 rmdir /s /q cesium_godot\native\build-web64
rem Wipe godot-cpp's library cache. SCons treats the lib as "up to date" once
rem the .a file exists, so flag changes (e.g. MEMORY64 added on emsdk bump) or
rem ABI shifts between emsdk versions don't trigger a rebuild. The lib then
rem ships stale wasm32-style call_indirects into the wasm64 link, which the
rem browser rejects at instantiation: "call_indirect[0] expected type i64,
rem found i32.wrap_i64".
if exist godot-cpp\bin rmdir /s /q godot-cpp\bin
rem Same reasoning for pre-built litehtml/gumbo. `build_litehtml_web()` skips
rem when the .a files exist, so an old emsdk's table-convention .a's persist.
if exist cesium_godot\third_party\litehtml\web rmdir /s /q cesium_godot\third_party\litehtml\web
if exist cesium_godot\third_party\litehtml\web64 rmdir /s /q cesium_godot\third_party\litehtml\web64
if exist cesium_godot\third_party\litehtml-src\build-web rmdir /s /q cesium_godot\third_party\litehtml-src\build-web
if exist cesium_godot\third_party\litehtml-src\build-web64 rmdir /s /q cesium_godot\third_party\litehtml-src\build-web64
echo Done.
goto :done

:clean_deep
echo Cleaning cesium-native build trees...
if exist cesium_godot\native\build-windows rmdir /s /q cesium_godot\native\build-windows
if exist cesium_godot\native\build-web rmdir /s /q cesium_godot\native\build-web
if exist cesium_godot\native\build-web64 rmdir /s /q cesium_godot\native\build-web64
rem Wipe godot-cpp + pre-built litehtml caches — see :clean for why.
if exist godot-cpp\bin rmdir /s /q godot-cpp\bin
if exist cesium_godot\third_party\litehtml\web rmdir /s /q cesium_godot\third_party\litehtml\web
if exist cesium_godot\third_party\litehtml\web64 rmdir /s /q cesium_godot\third_party\litehtml\web64
if exist cesium_godot\third_party\litehtml-src\build-web rmdir /s /q cesium_godot\third_party\litehtml-src\build-web
if exist cesium_godot\third_party\litehtml-src\build-web64 rmdir /s /q cesium_godot\third_party\litehtml-src\build-web64
echo Cleaning stale wasm32-emscripten and wasm64-emscripten vcpkg state...
if exist "%EZVCPKG_BASEDIR%" (
    for /d %%d in ("%EZVCPKG_BASEDIR%\*") do (
        if exist "%%d\installed\wasm32-emscripten" rmdir /s /q "%%d\installed\wasm32-emscripten"
        if exist "%%d\installed\wasm64-emscripten" rmdir /s /q "%%d\installed\wasm64-emscripten"
        if exist "%%d\installed\vcpkg\info" del /q "%%d\installed\vcpkg\info\*_wasm32-emscripten.list" 2>nul
        if exist "%%d\installed\vcpkg\info" del /q "%%d\installed\vcpkg\info\*_wasm64-emscripten.list" 2>nul
        if exist "%%d\buildtrees\ktx" rmdir /s /q "%%d\buildtrees\ktx"
        rem Strip orphaned wasm32/wasm64 stanzas from vcpkg's status file. Without
        rem this, any subsequent vcpkg op fails with "read_lines(...wasm*-emscripten.list):
        rem no such file or directory" because status still references entries
        rem whose .list files we just deleted.
        if exist "%%d\installed\vcpkg\status" python -c "import re; p=r'%%d\installed\vcpkg\status'; c=open(p,'r',encoding='utf-8').read(); k=[s for s in re.split(r'\n\n+',c) if not re.search(r'^Architecture:\s*wasm\d+-emscripten\s*$',s,re.MULTILINE)]; open(p,'w',encoding='utf-8').write('\n\n'.join(k))"
    )
)
echo Done.
goto :done

:usage
echo Usage: build.bat [extension/web/web64/module/clean/clean-deep]
echo   extension  - Build GDExtension for Windows x64 (default)
echo   web        - Build GDExtension for Web/WASM (wasm32, universal compat)
echo   web64      - Build GDExtension for Web/WASM (wasm64 / Memory64, experimental)
echo   module     - Prepare dependencies for Godot engine module build
echo   clean      - Remove cesium-native build-windows / build-web / build-web64 dirs
echo   clean-deep - clean + nuke stale wasm32/wasm64-emscripten vcpkg state (recovery)
goto :done

rem --- Ensure Emscripten SDK is installed and activated in this shell --------
rem Always activate this repo's pinned emsdk (3.1.56 at %EMSDK_DIR%), even if
rem the parent shell has a different emsdk active (e.g. the user was just
rem working in the Godot repo with 4.0.11 at %USERPROFILE%\emsdk). Checking
rem `defined EMSDK` would skip activation and silently compile against the
rem wrong toolchain. Clone emsdk into %EMSDK_DIR% if missing, then run
rem install+activate for the pinned version — both are near-instant no-ops
rem when already applied, so the cost on repeat builds is negligible.
rem
rem Note: emsdk.bat install/activate internally uses setlocal/endlocal+set
rem patterns that can clobber a caller's EMSDK_DIR on first install. Stash
rem the path into an underscore-prefixed name that won't collide with
rem emsdk's internals, and use that for the final emsdk_env.bat call.
:ensure_emsdk
set "_CESIUM_EMSDK_DIR=%EMSDK_DIR%"
set "_CESIUM_EMSDK_VERSION=%EMSDK_VERSION%"
if exist "%_CESIUM_EMSDK_DIR%\emsdk.bat" goto :run_emsdk_install
echo emsdk not found at %_CESIUM_EMSDK_DIR% - cloning...
where git >nul 2>&1
if errorlevel 1 (
    echo ERROR: git is required on PATH to clone emsdk. Install git, or pre-install emsdk and set EMSDK_DIR.
    exit /b 1
)
git clone https://github.com/emscripten-core/emsdk.git "%_CESIUM_EMSDK_DIR%"
if errorlevel 1 exit /b 1
:run_emsdk_install
pushd "%_CESIUM_EMSDK_DIR%"
call emsdk.bat install %_CESIUM_EMSDK_VERSION%
if errorlevel 1 (
    popd
    echo ERROR: emsdk install %_CESIUM_EMSDK_VERSION% failed.
    exit /b 1
)
call emsdk.bat activate %_CESIUM_EMSDK_VERSION%
if errorlevel 1 (
    popd
    echo ERROR: emsdk activate %_CESIUM_EMSDK_VERSION% failed.
    exit /b 1
)
popd
echo Activating emsdk %_CESIUM_EMSDK_VERSION% from %_CESIUM_EMSDK_DIR%...
call "%_CESIUM_EMSDK_DIR%\emsdk_env.bat"
exit /b %ERRORLEVEL%

:done
pause
