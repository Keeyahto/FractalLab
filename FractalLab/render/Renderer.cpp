#include "render/Renderer.h"

#include "render/cuda/FractalRenderer.h"

#include <GLFW/glfw3.h>

#include <cuda_gl_interop.h>
#include <cuda_runtime.h>

namespace FractalLab
{
namespace
{
bool CheckCudaStatus(const cudaError_t status, std::string& errorMessage, const char* step)
{
    if (status == cudaSuccess)
    {
        return true;
    }

    errorMessage = std::string(step) + " failed: " + cudaGetErrorString(status);
    return false;
}
}

bool Renderer::Initialize(const RenderSettings& settings)
{
    lastError_.clear();

    if (!cudaDeviceReady_)
    {
        if (!CheckCudaStatus(cudaSetDevice(0), lastError_, "cudaSetDevice"))
        {
            return false;
        }

        if (!CheckCudaStatus(
                cudaMalloc(reinterpret_cast<void**>(&deviceCenterPixel_), sizeof(std::uint32_t)),
                lastError_,
                "cudaMalloc(deviceCenterPixel_)"))
        {
            return false;
        }

        cudaDeviceReady_ = true;
    }

    return EnsureResources(settings);
}

void Renderer::Shutdown()
{
    ReleaseInteropResource();
    ReleaseTexture();

    if (deviceCenterPixel_ != nullptr)
    {
        cudaFree(deviceCenterPixel_);
        deviceCenterPixel_ = nullptr;
    }

    width_ = 0;
    height_ = 0;
    centerPixel_ = 0;
    cudaDeviceReady_ = false;
}

bool Renderer::Render(const FrameState& frameState)
{
    if (!EnsureResources(frameState.renderSettings))
    {
        return false;
    }

    if (cudaTextureResource_ == nullptr)
    {
        lastError_ = "CUDA/OpenGL interop resource is not initialized.";
        return false;
    }

    cudaArray_t mappedArray = nullptr;
    cudaSurfaceObject_t surfaceObject = 0;

    if (!CheckCudaStatus(cudaGraphicsMapResources(1, &cudaTextureResource_, 0), lastError_, "cudaGraphicsMapResources"))
    {
        return false;
    }

    bool success = CheckCudaStatus(
        cudaGraphicsSubResourceGetMappedArray(&mappedArray, cudaTextureResource_, 0, 0),
        lastError_,
        "cudaGraphicsSubResourceGetMappedArray");

    if (success)
    {
        cudaResourceDesc resourceDesc = {};
        resourceDesc.resType = cudaResourceTypeArray;
        resourceDesc.res.array.array = mappedArray;

        success = CheckCudaStatus(
            cudaCreateSurfaceObject(&surfaceObject, &resourceDesc),
            lastError_,
            "cudaCreateSurfaceObject");
    }

    if (success)
    {
        success = RenderFrameCuda(frameState, surfaceObject, deviceCenterPixel_, centerPixel_, lastError_);
    }

    if (surfaceObject != 0)
    {
        cudaDestroySurfaceObject(surfaceObject);
    }

    const cudaError_t unmapStatus = cudaGraphicsUnmapResources(1, &cudaTextureResource_, 0);
    if (success)
    {
        success = CheckCudaStatus(unmapStatus, lastError_, "cudaGraphicsUnmapResources");
    }

    return success;
}

const std::string& Renderer::GetLastError() const
{
    return lastError_;
}

std::uint32_t Renderer::GetCenterPixel() const
{
    return centerPixel_;
}

unsigned int Renderer::GetOutputTexture() const
{
    return outputTexture_;
}

int Renderer::GetWidth() const
{
    return width_;
}

int Renderer::GetHeight() const
{
    return height_;
}

bool Renderer::EnsureResources(const RenderSettings& settings)
{
    if (settings.width <= 0 || settings.height <= 0)
    {
        lastError_ = "Render target dimensions must be positive.";
        return false;
    }

    if (settings.width == width_ && settings.height == height_ && outputTexture_ != 0 && cudaTextureResource_ != nullptr)
    {
        return true;
    }

    return CreateOrResizeOutputTexture(settings);
}

bool Renderer::CreateOrResizeOutputTexture(const RenderSettings& settings)
{
    ReleaseInteropResource();

    if (outputTexture_ == 0U)
    {
        glGenTextures(1, &outputTexture_);
    }

    if (outputTexture_ == 0U)
    {
        lastError_ = "glGenTextures failed.";
        return false;
    }

    width_ = settings.width;
    height_ = settings.height;

    glBindTexture(GL_TEXTURE_2D, outputTexture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (!CheckCudaStatus(
            cudaGraphicsGLRegisterImage(
                reinterpret_cast<cudaGraphicsResource_t*>(&cudaTextureResource_),
                outputTexture_,
                GL_TEXTURE_2D,
                cudaGraphicsRegisterFlagsWriteDiscard | cudaGraphicsRegisterFlagsSurfaceLoadStore),
            lastError_,
            "cudaGraphicsGLRegisterImage"))
    {
        ReleaseTexture();
        width_ = 0;
        height_ = 0;
        return false;
    }

    return true;
}

void Renderer::ReleaseInteropResource()
{
    if (cudaTextureResource_ != nullptr)
    {
        cudaGraphicsUnregisterResource(reinterpret_cast<cudaGraphicsResource_t>(cudaTextureResource_));
        cudaTextureResource_ = nullptr;
    }
}

void Renderer::ReleaseTexture()
{
    if (outputTexture_ != 0U)
    {
        glDeleteTextures(1, &outputTexture_);
        outputTexture_ = 0U;
    }
}
}
