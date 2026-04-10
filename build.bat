@echo off
setlocal

set TARGET=%1
if "%TARGET%"=="" set TARGET=extension

if "%TARGET%"=="extension" (
    echo Building GDExtension for Windows x64...
    scons arch=x64 compileTarget=extension target=template_release production=yes compiledb=yes
) else if "%TARGET%"=="web" (
    echo Building GDExtension for Web (WASM)...
    scons platform=web compileTarget=extension target=template_release production=yes
) else if "%TARGET%"=="module" (
    echo Preparing module dependencies...
    scons compileTarget=module buildCesium=yes
) else (
    echo Usage: build.bat [extension^|web^|module]
    echo   extension  - Build GDExtension for Windows x64 (default)
    echo   web        - Build GDExtension for Web/WASM
    echo   module     - Prepare dependencies for Godot engine module build
)

pause
