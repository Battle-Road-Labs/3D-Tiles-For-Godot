@echo off
setlocal enabledelayedexpansion

REM Sets up 3D-Tiles-For-Godot as a Godot engine module by creating symlinks
REM from a Godot source tree into this repository.
REM
REM Usage:
REM   setup_module.bat C:\path\to\godot-source
REM
REM NOTE: Must be run as Administrator (mklink requires elevated privileges).

if "%~1"=="" (
    echo Usage: setup_module.bat ^<path-to-godot-source^>
    echo.
    echo Creates symlinks in ^<godot-source^>\modules\cesium_godot\ pointing
    echo back to this repository so the module is compiled into the engine.
    echo.
    echo NOTE: Must be run as Administrator.
    exit /b 1
)

set GODOT_SRC=%~f1
set MODULE_DIR=%GODOT_SRC%\modules\cesium_godot
set REPO_DIR=%~dp0

if not exist "%GODOT_SRC%\SConstruct" (
    echo Error: %GODOT_SRC% does not look like a Godot source tree (no SConstruct found^).
    exit /b 1
)

echo Godot source: %GODOT_SRC%
echo Plugin repo:  %REPO_DIR%
echo.

if not exist "%MODULE_DIR%" mkdir "%MODULE_DIR%"

REM Symlink files from cesium_godot/
for %%F in (SCsub config.py register_types.h register_types.cpp CesiumGDModelLoader.h CesiumGDModelLoader.cpp) do (
    if exist "%REPO_DIR%cesium_godot\%%F" (
        if not exist "%MODULE_DIR%\%%F" (
            mklink "%MODULE_DIR%\%%F" "%REPO_DIR%cesium_godot\%%F"
            echo   (linked^) %%F
        ) else (
            echo   (exists^) %%F
        )
    )
)

REM Symlink directories from cesium_godot/
for %%D in (Models Implementations Utils Shaders native third_party) do (
    if exist "%REPO_DIR%cesium_godot\%%D" (
        if not exist "%MODULE_DIR%\%%D" (
            mklink /D "%MODULE_DIR%\%%D" "%REPO_DIR%cesium_godot\%%D"
            echo   (linked^) %%D
        ) else (
            echo   (exists^) %%D
        )
    )
)

REM Symlink CesiumBuildUtils.py from repo root
if not exist "%MODULE_DIR%\CesiumBuildUtils.py" (
    mklink "%MODULE_DIR%\CesiumBuildUtils.py" "%REPO_DIR%CesiumBuildUtils.py"
    echo   (linked^) CesiumBuildUtils.py
) else (
    echo   (exists^) CesiumBuildUtils.py
)

REM Symlink cesium_auxiliars from repo root
if not exist "%MODULE_DIR%\cesium_auxiliars" (
    mklink /D "%MODULE_DIR%\cesium_auxiliars" "%REPO_DIR%cesium_auxiliars"
    echo   (linked^) cesium_auxiliars
) else (
    echo   (exists^) cesium_auxiliars
)

echo.
echo Module setup complete.
echo.
echo Next steps:
echo   1. Build Cesium Native (if first time^):
echo        cd %REPO_DIR%
echo        build.bat module
echo.
echo   2. Build the Godot engine:
echo        cd %GODOT_SRC%
echo        scons platform=windows target=editor
echo.
pause
