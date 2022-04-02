# DirectX Present Hook

The project is an answer to this Stack Overflow question https://stackoverflow.com/questions/40538590/getting-dxgi-swapchain-by-hwnd. It shows how to hook IDXGISwapChain::Present (when it is used with DirectX 11/DirectX 12) to capture frames of a window which you do not control. For example, it can be used for some ActiveX controls which you integrate to your application. Probably, the project can be extended for more complicated tasks.

## Source code

See the following classes:
* ``D3D11PresentHook``: d3d11-present-hook.h, d3d11-present-hook.cpp.
* ``D3D12PresentHook``: d3d12-present-hook.h, d3d12-present-hook.cpp.

The classes above are well commented. So, I hope that even if they do not solve your task directly, they may give you some ideas at least. The other classes are auxiliary or used to test the hooks by creating a "black box" window with a moving square.

## Building

#### Visual Studio
Visual Studio 2019 v16.10 & v16.11 or later is required. Personally, I use Visual Studio 2022. The download link is here https://visualstudio.microsoft.com/vs/community/.

#### CMake
CMake is available as a part of Visual Studio 2022 installation. In my case it is located in the following folder: ``C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/``. Install CMake separately https://cmake.org/ if your Visual Studio does not include it. 

#### PolyHook 2
You need to compile this https://github.com/stevemk14ebr/PolyHook_2_0

#### DirectX Present Hook
The root folder contains ``example-prepare-x32.cmd`` and ``example-prepare-x64.cmd`` scripts.
* Change the include and library paths inside them to point to PolyHook 2.
* Change the CMAKE path if you do not use Visual Studio 2022.
* Depending on PolyHook 2 compilation parameters, you may have to change the ``target_link_libraries`` line in ``CMakeLists.txt``.
* Run one of the scripts to prepare the Visual Studio solution.
* Go to the ``build`` folder, open the solution. compile it.
* You might need ``asmjit.dll``, ``capstone.dll``, ``PolyHook_2.dll`` and ``Zydis.dll`` in the output folder. If you compiled PolyHook 2 via vcpkg, they will be there automatically.

## Testing
* ``directx-present-hook.exe`` will create a DirectX 11 window with a moving square, set the hook and save first ten frames into BMP files in the same output folder.
* ``directx-present-hook.exe 12``  will create a DirectX 12 window with a moving square, set the hook and save first ten frames into BMP files in the same output folder.
* ``directx-present-hook.exe 11 C:\Temp``  will create a DirectX 11 window with a moving square, set the hook and save first ten frames into BMP files in ``C:\Temp``.


