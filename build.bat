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
rem EMCC_CFLAGS provides base flags for ALL emcc invocations (vcpkg make-based builds
rem like openssl, scons, etc). Keep it minimal — only -pthread and -fPIC here.
rem Exception handling flags (-fwasm-exceptions -sSUPPORT_LONGJMP=wasm) are passed
rem through the vcpkg triplet and cmake flags instead, to avoid breaking make-based
rem builds like openssl that don't understand those flags.
set EMCC_CFLAGS=-pthread -fPIC
scons platform=web compileTarget=extension target=template_release precision=double production=yes
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
