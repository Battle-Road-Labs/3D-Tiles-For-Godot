@echo off
setlocal
set VCPKG=C:\.ezvcpkg\dbe35ceb30c688bf72e952ab23778e009a578f18\vcpkg.exe
set ROOT=C:\.ezvcpkg\dbe35ceb30c688bf72e952ab23778e009a578f18

echo === Listing wasm32-emscripten packages ===
%VCPKG% --vcpkg-root %ROOT% list --triplet wasm32-emscripten 2>nul | findstr "wasm32-emscripten"
if errorlevel 1 (
    echo No wasm32-emscripten packages found in status DB
) else (
    echo === Removing all wasm32-emscripten packages ===
    for /f "tokens=1" %%p in ('%VCPKG% --vcpkg-root %ROOT% list --triplet wasm32-emscripten 2^>nul ^| findstr "wasm32-emscripten"') do (
        echo Removing %%p...
        %VCPKG% --vcpkg-root %ROOT% remove --recurse %%p 2>nul
    )
)

echo === Cleaning installed directory ===
if exist "%ROOT%\installed\wasm32-emscripten" (
    rmdir /s /q "%ROOT%\installed\wasm32-emscripten"
    echo Deleted installed\wasm32-emscripten
) else (
    echo installed\wasm32-emscripten already clean
)

echo === Cleaning binary cache ===
if exist "%USERPROFILE%\AppData\Local\vcpkg\archives" (
    rmdir /s /q "%USERPROFILE%\AppData\Local\vcpkg\archives"
    echo Deleted vcpkg binary cache
) else (
    echo Binary cache already clean
)

echo === Cleaning build-web ===
if exist "cesium_godot\native\build-web" (
    rmdir /s /q "cesium_godot\native\build-web"
    echo Deleted build-web
) else (
    echo build-web already clean
)

echo === Verifying no wasm32-emscripten packages remain ===
%VCPKG% --vcpkg-root %ROOT% list --triplet wasm32-emscripten 2>nul | findstr "wasm32-emscripten"
if errorlevel 1 (
    echo CLEAN - no wasm32-emscripten packages remain
) else (
    echo WARNING - some packages still listed, running second pass...
    for /f "tokens=1" %%p in ('%VCPKG% --vcpkg-root %ROOT% list --triplet wasm32-emscripten 2^>nul ^| findstr "wasm32-emscripten"') do (
        %VCPKG% --vcpkg-root %ROOT% remove --recurse %%p 2>nul
    )
)

echo.
echo === Done! Now run: build.bat web ===
pause
