cmake_minimum_required(VERSION 3.15)

project(directx-present-hook)

message(STATUS "Use -DINCLUDE_DIRS=\"path1;path2;path3\" to specify additional include directories.")
message(STATUS "Use -DLIBRARY_DIRS=\"path1;path2;path3\" to specify additional library directories.")

foreach(INCLUDE_DIR IN LISTS INCLUDE_DIRS)
  include_directories(${INCLUDE_DIR})
endforeach()

foreach(LIBRARY_DIR IN LISTS LIBRARY_DIRS)
  link_directories(${LIBRARY_DIR})
endforeach()

if (WIN32)
  if((NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR MSVC_VERSION LESS 1929)
    message(
      "-------------------------------------\n"
      "VS 2019 v16.10 & v16.11 or later is required because the project uses std::format. "
      "See: https://devblogs.microsoft.com/cppblog/msvc-cpp20-and-the-std-cpp20-switch.\n"
      "-------------------------------------")
    message(FATAL_ERROR "See above for details.")
  endif()

  add_compile_options("/std:c++latest")

  option(USE_STATIC_RUNTIMES "Use /MT instead of /MD" OFF)
  if(USE_STATIC_RUNTIMES)
    add_compile_options(
      $<$<CONFIG:>:/MT>
      $<$<CONFIG:Debug>:/MTd>
      $<$<CONFIG:Release>:/MT>
    )
  endif()
else()
    message(
      "-------------------------------------\n"
      "This project is only for Windows.\n"
      "-------------------------------------")
    message(FATAL_ERROR "See above for details.")
endif()

add_definitions(-DUNICODE -D_UNICODE)

set(PIXELSHADER src/pixel-shader.hlsl)
set(VERTEXSHADER src/vertex-shader.hlsl)

set(SOURCES
  ${SOURCES}
  src/misc-helpers.cpp
  src/base-window.cpp
  src/d3d11-base-helper.cpp
  src/d3d11-present-hook.cpp
  src/d3d11-renderer.cpp
  src/d3d12-base-helper.cpp
  src/d3d12-present-hook.cpp
  src/d3d12-renderer.cpp
  src/main.cpp
  ${PIXELSHADER}
  ${VERTEXSHADER}
)

set(HEADERS
  ${HEADERS}
  src/misc-helpers.h
  src/base-window.h
  src/black-box-dx-window.h
  src/d3d11-base-helper.h
  src/d3d11-present-hook.h
  src/d3d11-renderer.h
  src/d3d12-base-helper.h
  src/d3d12-present-hook.h
  src/d3d12-renderer.h
  src/d3dx12.h
)

set_property(SOURCE ${PIXELSHADER} PROPERTY VS_SHADER_TYPE Pixel)
set_property(SOURCE ${PIXELSHADER} PROPERTY VS_SHADER_ENTRYPOINT PS)
set_property(SOURCE ${PIXELSHADER} PROPERTY VS_SHADER_MODEL 4.0_level_9_3)
set_property(SOURCE ${PIXELSHADER} PROPERTY VS_SHADER_OUTPUT_HEADER_FILE "$(ProjectDir)%(Filename).h")
set_property(SOURCE ${PIXELSHADER} PROPERTY VS_SHADER_OBJECT_FILE_NAME "")

set_property(SOURCE ${VERTEXSHADER} PROPERTY VS_SHADER_TYPE Vertex)
set_property(SOURCE ${VERTEXSHADER} PROPERTY VS_SHADER_ENTRYPOINT VS)
set_property(SOURCE ${VERTEXSHADER} PROPERTY VS_SHADER_MODEL 4.0_level_9_3)
set_property(SOURCE ${VERTEXSHADER} PROPERTY VS_SHADER_OUTPUT_HEADER_FILE "$(ProjectDir)%(Filename).h")
set_property(SOURCE ${VERTEXSHADER} PROPERTY VS_SHADER_OBJECT_FILE_NAME "")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(${CMAKE_PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

target_link_libraries(${CMAKE_PROJECT_NAME} D3D11 D3D12 Dxgi dxguid PolyHook_2 capstone_dll)
