@echo off
setlocal

rem Default ezvcpkg location if not already set. The quoted form catches both
rem unset and empty-string cases; `if not defined` misses the empty case.
if "%EZVCPKG_BASEDIR%"=="" set EZVCPKG_BASEDIR=C:\.ezvcpkg

set TARGET=%1
if "%TARGET%"=="" set TARGET=extension

if "%TARGET%"=="extension" goto :build_extension
if "%TARGET%"=="web" goto :build_web
if "%TARGET%"=="module" goto :build_module
if "%TARGET%"=="clean" goto :clean
goto :usage

:build_extension
echo Building GDExtension for Windows x64...
scons arch=x64 compileTarget=extension target=template_release precision=double production=yes compiledb=yes
scons arch=x64 compileTarget=extension target=template_debug precision=double compiledb=yes
goto :done

:build_web
echo Building GDExtension for Web/WASM...
rem Force emscripten-style longjmp to avoid conflict with godot-cpp's exception handling.
rem -pthread enables atomics/bulk-memory needed for --shared-memory at link time.
rem EMCC_CFLAGS is read by emcc/em++ for ALL compilations (vcpkg, cmake, scons).
rem -fwasm-exceptions: native wasm exception handling (no JS invoke_* wrappers)
rem -sSUPPORT_LONGJMP=wasm: native wasm longjmp (pairs with -fwasm-exceptions)
rem -pthread -fPIC: required for threaded SIDE_MODULE builds
set EMCC_CFLAGS=-fwasm-exceptions -sSUPPORT_LONGJMP=wasm -pthread -fPIC
scons platform=web compileTarget=extension target=template_release precision=double production=yes disable_exceptions=no
scons platform=web compileTarget=extension target=template_debug precision=double disable_exceptions=no
goto :done


:build_module
echo Preparing module dependencies...
scons compileTarget=module buildCesium=yes
goto :done

:clean
echo Cleaning cesium-native build trees...
if exist cesium_godot\native\build-windows rmdir /s /q cesium_godot\native\build-windows
if exist cesium_godot\native\build-web rmdir /s /q cesium_godot\native\build-web
echo Done.
goto :done

:usage
echo Usage: build.bat [extension/web/module/clean]
echo   extension  - Build GDExtension for Windows x64 (default)
echo   web        - Build GDExtension for Web/WASM
echo   module     - Prepare dependencies for Godot engine module build
echo   clean      - Remove cesium-native build-windows and build-web directories

:done
pause
