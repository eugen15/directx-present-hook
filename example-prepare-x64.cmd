SET CMAKE_EXE="C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if not exist build mkdir build
cd build && %CMAKE_EXE% .. -A x64 -DINCLUDE_DIRS=C:\devel\github\vcpkg\installed\x64-windows\include -DLIBRARY_DIRS=C:\devel\github\vcpkg\installed\x64-windows\debug\lib