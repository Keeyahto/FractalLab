#include "render/cuda/FractalRenderer.h"

#include "cuda_runtime.h"

#include <cmath>
#include <sstream>

namespace FractalLab
{
namespace
{
struct DeviceVec3
{
    float x;
    float y;
    float z;
};

__device__ DeviceVec3 MakeVec3(const float x, const float y, const float z)
{
    DeviceVec3 value = { x, y, z };
    return value;
}

__device__ DeviceVec3 Add(const DeviceVec3 a, const DeviceVec3 b)
{
    return MakeVec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

__device__ DeviceVec3 Mul(const DeviceVec3 value, const float scalar)
{
    return MakeVec3(value.x * scalar, value.y * scalar, value.z * scalar);
}

__device__ DeviceVec3 AbsVec(const DeviceVec3 value)
{
    return MakeVec3(fabsf(value.x), fabsf(value.y), fabsf(value.z));
}

__device__ float Dot(const DeviceVec3 a, const DeviceVec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

__device__ float Length(const DeviceVec3 value)
{
    return sqrtf(Dot(value, value));
}

__device__ DeviceVec3 Normalize(const DeviceVec3 value)
{
    const float length = fmaxf(Length(value), 1.0e-5f);
    return Mul(value, 1.0f / length);
}

__device__ float Saturate(const float value)
{
    return fminf(fmaxf(value, 0.0f), 1.0f);
}

__device__ float FractalField(DeviceVec3 position, const float time, const float powerBoost)
{
    position.z += time * 0.35f;
    position = Mul(position, 0.8f + powerBoost * 0.03f);

    float orbitTrap = 1.0e6f;
    for (int iteration = 0; iteration < 6; ++iteration)
    {
        const float radiusSquared = fmaxf(Dot(position, position), 0.25f);
        position = Add(Mul(AbsVec(position), 1.0f / radiusSquared), MakeVec3(-0.82f, -0.78f, -0.74f));
        orbitTrap = fminf(orbitTrap, fabsf(Length(position) - 1.2f));
    }

    return expf(-4.0f * orbitTrap);
}

__device__ std::uint32_t PackRgba8(const float red, const float green, const float blue)
{
    const std::uint32_t r = static_cast<std::uint32_t>(Saturate(red) * 255.0f);
    const std::uint32_t g = static_cast<std::uint32_t>(Saturate(green) * 255.0f);
    const std::uint32_t b = static_cast<std::uint32_t>(Saturate(blue) * 255.0f);
    return 0xFF000000u | (b << 16U) | (g << 8U) | r;
}

__global__ void RenderKernel(std::uint32_t* output, const FrameState frameState)
{
    const int x = static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
    const int y = static_cast<int>(blockIdx.y * blockDim.y + threadIdx.y);
    const int width = frameState.renderSettings.width;
    const int height = frameState.renderSettings.height;

    if (x >= width || y >= height)
    {
        return;
    }

    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    const float uvX = ((static_cast<float>(x) + 0.5f) / static_cast<float>(width) * 2.0f - 1.0f) * aspect;
    const float uvY = 1.0f - ((static_cast<float>(y) + 0.5f) / static_cast<float>(height) * 2.0f);

    const DeviceVec3 rayOrigin = MakeVec3(
        frameState.camera.position.x,
        frameState.camera.position.y,
        frameState.camera.position.z);
    const DeviceVec3 rayDirection = Normalize(MakeVec3(uvX, uvY, 1.5f));

    float distanceTravelled = 0.0f;
    float density = 0.0f;
    float glow = 0.0f;

    for (int step = 0; step < frameState.renderSettings.maxSteps; ++step)
    {
        const DeviceVec3 samplePosition = Add(rayOrigin, Mul(rayDirection, distanceTravelled));
        const float field = FractalField(samplePosition, static_cast<float>(frameState.elapsedSeconds), frameState.fractalPower);

        density += field * 0.016f;
        glow += field * field * 0.006f;
        distanceTravelled += 0.055f + field * 0.025f;

        if (distanceTravelled > frameState.renderSettings.maxDistance)
        {
            break;
        }
    }

    const float pulse = frameState.ambientPulse;
    const float baseR = 0.03f + density * (0.22f + 0.18f * pulse);
    const float baseG = 0.02f + density * (0.15f + 0.12f * frameState.glowIntensity);
    const float baseB = 0.05f + density * 0.30f + glow * 0.20f;
    const float vignette = 1.0f - Saturate((uvX * uvX + uvY * uvY) * 0.45f);
    const float fog = expf(-distanceTravelled * frameState.fogDensity);

    const float red = baseR * vignette * fog;
    const float green = baseG * vignette * fog;
    const float blue = baseB * vignette * (0.85f + 0.15f * fog);

    output[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)] =
        PackRgba8(red, green, blue);
}

bool CheckCudaStatus(const cudaError_t status, std::string& errorMessage, const char* step)
{
    if (status == cudaSuccess)
    {
        return true;
    }

    std::ostringstream stream;
    stream << step << " failed: " << cudaGetErrorString(status);
    errorMessage = stream.str();
    return false;
}
}

bool RenderFrameCuda(
    const FrameState& frameState,
    std::vector<std::uint32_t>& outputPixels,
    std::string& errorMessage)
{
    errorMessage.clear();

    const std::size_t pixelCount =
        static_cast<std::size_t>(frameState.renderSettings.width) *
        static_cast<std::size_t>(frameState.renderSettings.height);
    if (outputPixels.size() != pixelCount)
    {
        outputPixels.resize(pixelCount);
    }

    std::uint32_t* devicePixels = nullptr;
    const std::size_t bytes = pixelCount * sizeof(std::uint32_t);

    if (!CheckCudaStatus(cudaSetDevice(0), errorMessage, "cudaSetDevice"))
    {
        return false;
    }

    if (!CheckCudaStatus(cudaMalloc(reinterpret_cast<void**>(&devicePixels), bytes), errorMessage, "cudaMalloc"))
    {
        return false;
    }

    const dim3 blockSize(16, 16, 1);
    const dim3 gridSize(
        static_cast<unsigned int>((frameState.renderSettings.width + blockSize.x - 1) / blockSize.x),
        static_cast<unsigned int>((frameState.renderSettings.height + blockSize.y - 1) / blockSize.y),
        1);

    RenderKernel<<<gridSize, blockSize>>>(devicePixels, frameState);

    bool success = CheckCudaStatus(cudaGetLastError(), errorMessage, "RenderKernel launch");
    success = success && CheckCudaStatus(cudaDeviceSynchronize(), errorMessage, "cudaDeviceSynchronize");
    success = success && CheckCudaStatus(
        cudaMemcpy(outputPixels.data(), devicePixels, bytes, cudaMemcpyDeviceToHost),
        errorMessage,
        "cudaMemcpy");

    cudaFree(devicePixels);
    return success;
}
}
