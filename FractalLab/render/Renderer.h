#pragma once

#include "engine/FrameState.h"

#include <cstdint>
#include <string>

struct cudaGraphicsResource;

namespace FractalLab
{
class Renderer
{
public:
    bool Initialize(const RenderSettings& settings);
    void Shutdown();
    bool Render(const FrameState& frameState);

    const std::string& GetLastError() const;
    std::uint32_t GetCenterPixel() const;
    unsigned int GetOutputTexture() const;
    int GetWidth() const;
    int GetHeight() const;

private:
    bool EnsureResources(const RenderSettings& settings);
    bool CreateOrResizeOutputTexture(const RenderSettings& settings);
    void ReleaseInteropResource();
    void ReleaseTexture();

    int width_ = 0;
    int height_ = 0;
    unsigned int outputTexture_ = 0;
    cudaGraphicsResource* cudaTextureResource_ = nullptr;
    std::uint32_t* deviceCenterPixel_ = nullptr;
    std::uint32_t centerPixel_ = 0;
    bool cudaDeviceReady_ = false;
    std::string lastError_;
};
}
