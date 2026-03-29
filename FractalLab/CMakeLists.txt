cmake_minimum_required(VERSION 3.24)

project(FractalLab LANGUAGES CXX CUDA)

find_package(CUDAToolkit REQUIRED)

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

target_link_libraries(FractalLab PRIVATE CUDA::cudart)

if(MSVC)
    target_compile_options(FractalLab PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W4 /EHsc>)
else()
    target_compile_options(
        FractalLab
        PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wpedantic>
            $<$<COMPILE_LANGUAGE:CUDA>:--compiler-options=-Wall,-Wextra,-Wpedantic>
    )
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${FRACTALLAB_SOURCES} ${FRACTALLAB_HEADERS})
