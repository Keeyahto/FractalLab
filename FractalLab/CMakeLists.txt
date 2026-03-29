cmake_minimum_required(VERSION 3.24)

project(FractalLab LANGUAGES CXX CUDA)

find_package(CUDAToolkit REQUIRED)
find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(third_party/glfw EXCLUDE_FROM_ALL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

set(FRACTALLAB_SOURCES
    app/main.cpp
    app/App.cpp
    engine/Engine.cpp
    render/Renderer.cpp
    render/cuda/FractalRenderer.cu
)

set(FRACTALLAB_HEADERS
    app/App.h
    engine/Engine.h
    engine/EngineState.h
    engine/FrameState.h
    engine/TimeState.h
    scene/Camera.h
    scene/SceneState.h
    render/RenderSettings.h
    render/Renderer.h
    render/cuda/FractalRenderer.h
)

add_executable(FractalLab ${FRACTALLAB_SOURCES} ${FRACTALLAB_HEADERS})

target_include_directories(FractalLab PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(FractalLab PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    CUDA_ARCHITECTURES native
    CUDA_RUNTIME_LIBRARY None
)

add_library(imgui STATIC
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_demo.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_glfw.cpp
    third_party/imgui/backends/imgui_impl_opengl2.cpp
)

target_include_directories(imgui PUBLIC third_party/imgui third_party/imgui/backends)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

target_link_libraries(FractalLab PRIVATE CUDA::cudart OpenGL::GL glfw imgui)

if(MSVC)
    target_compile_options(FractalLab PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W4 /EHsc>)
    target_compile_definitions(FractalLab PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
else()
    target_compile_options(
        FractalLab
        PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wpedantic>
            $<$<COMPILE_LANGUAGE:CUDA>:--compiler-options=-Wall,-Wextra,-Wpedantic>
    )
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${FRACTALLAB_SOURCES} ${FRACTALLAB_HEADERS})
