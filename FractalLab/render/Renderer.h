#pragma once

#include "engine/FrameState.h"

#include <cstdint>
#include <string>
#include <vector>

namespace FractalLab
{
class Renderer
{
public:
    bool Initialize(const RenderSettings& settings);
    bool Render(const FrameState& frameState);

    const std::string& GetLastError() const;
    std::uint32_t GetCenterPixel() const;
    const std::vector<std::uint32_t>& GetPixels() const;

private:
    void ResizeIfNeeded(const RenderSettings& settings);

    int width_ = 0;
    int height_ = 0;
    std::vector<std::uint32_t> pixelBuffer_;
    std::string lastError_;
};
}
