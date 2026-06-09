echo @echo off
echo setlocal
echo set VCPKG=C:\.ezvcpkg\dbe35ceb30c688bf72e952ab23778e009a578f18\vcpkg.exe
echo set ROOT=C:\.ezvcpkg\dbe35ceb30c688bf72e952ab23778e009a578f18
echo for /f "tokens=1" %%%%p in ('%VCPKG% --vcpkg-root %ROOT% list --triplet wasm32-emscripten 2^^^>nul ^^^| findstr
"wasm32-emscripten"'^) do %VCPKG% --vcpkg-root %ROOT% remove --recurse %%%%p 2^>nul
echo if exist "%ROOT%\installed\wasm32-emscripten" rmdir /s /q "%ROOT%\installed\wasm32-emscripten"
echo if exist "%USERPROFILE%\AppData\Local\vcpkg\archives" rmdir /s /q "%USERPROFILE%\AppData\Local\vcpkg\archives"
echo if exist "cesium_godot\native\build-web" rmdir /s /q "cesium_godot\native\build-web"
echo echo Done - now run: build.bat web
echo pause