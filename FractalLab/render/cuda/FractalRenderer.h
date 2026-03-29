#pragma once

#include "engine/FrameState.h"

#include <cstdint>
#include <string>
#include <vector>

namespace FractalLab
{
bool RenderFrameCuda(
    const FrameState& frameState,
    std::vector<std::uint32_t>& outputPixels,
    std::string& errorMessage);
}
