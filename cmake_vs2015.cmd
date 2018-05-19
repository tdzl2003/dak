@cd %~dp0
cmake -H. -Bbuild/vs2015 -G "Visual Studio 14 2015 Win64" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake