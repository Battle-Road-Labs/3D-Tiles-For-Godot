@echo off
setlocal

set TARGET=%1
if "%TARGET%"=="" set TARGET=extension

if "%TARGET%"=="extension" goto :build_extension
if "%TARGET%"=="web" goto :build_web
if "%TARGET%"=="module" goto :build_module
goto :usage

:build_extension
echo Building GDExtension for Windows x64...
scons arch=x64 compileTarget=extension target=template_release precision=double compiledb=yes
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
goto :done

:build_module
echo Preparing module dependencies...
scons compileTarget=module buildCesium=yes
goto :done

:usage
echo Usage: build.bat [extension/web/module]
echo   extension  - Build GDExtension for Windows x64 (default)
echo   web        - Build GDExtension for Web/WASM
echo   module     - Prepare dependencies for Godot engine module build

:done
pause
