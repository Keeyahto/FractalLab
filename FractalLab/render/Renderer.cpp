#include "render/Renderer.h"

#include "render/cuda/FractalRenderer.h"

#include <cstddef>

namespace FractalLab
{
bool Renderer::Initialize(const RenderSettings& settings)
{
    ResizeIfNeeded(settings);
    lastError_.clear();
    return !pixelBuffer_.empty();
}

bool Renderer::Render(const FrameState& frameState)
{
    ResizeIfNeeded(frameState.renderSettings);
    if (pixelBuffer_.empty())
    {
        lastError_ = "Renderer has no pixel buffer.";
        return false;
    }

    return RenderFrameCuda(frameState, pixelBuffer_, lastError_);
}

const std::string& Renderer::GetLastError() const
{
    return lastError_;
}

std::uint32_t Renderer::GetCenterPixel() const
{
    if (pixelBuffer_.empty() || width_ <= 0 || height_ <= 0)
    {
        return 0;
    }

    const std::size_t centerIndex =
        static_cast<std::size_t>(height_ / 2) * static_cast<std::size_t>(width_) +
        static_cast<std::size_t>(width_ / 2);

    return pixelBuffer_[centerIndex];
}

const std::vector<std::uint32_t>& Renderer::GetPixels() const
{
    return pixelBuffer_;
}

void Renderer::ResizeIfNeeded(const RenderSettings& settings)
{
    if (settings.width == width_ && settings.height == height_)
    {
        return;
    }

    width_ = settings.width;
    height_ = settings.height;
    pixelBuffer_.assign(static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_), 0U);
}
}
