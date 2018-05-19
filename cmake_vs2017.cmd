@cd %~dp0
cmake -H. -Bbuild/vs2017 -G "Visual Studio 15 2017 Win64" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake