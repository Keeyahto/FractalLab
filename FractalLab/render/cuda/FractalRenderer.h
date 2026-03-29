#pragma once

#include "engine/FrameState.h"

#include <cuda_runtime.h>

#include <cstdint>
#include <string>

namespace FractalLab
{
bool RenderFrameCuda(
    const FrameState& frameState,
    cudaSurfaceObject_t outputSurface,
    std::uint32_t* deviceCenterPixel,
    std::uint32_t& centerPixel,
    std::string& errorMessage);
}
