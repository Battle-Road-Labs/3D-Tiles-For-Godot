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
scons arch=x64 compileTarget=extension target=template_release precision=double production=yes compiledb=yes
goto :done

:build_web
echo Building GDExtension for Web/WASM...
rem Force emscripten-style longjmp to avoid conflict with godot-cpp's exception handling
set CFLAGS=-sSUPPORT_LONGJMP=emscripten
set CXXFLAGS=-sSUPPORT_LONGJMP=emscripten
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
