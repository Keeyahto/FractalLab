#include "render/cuda/FractalRenderer.h"

#include "cuda_runtime.h"

#include <cmath>
#include <sstream>

namespace FractalLab
{
namespace
{
constexpr int kTileHeight = 64;

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

__device__ DeviceVec3 Sub(const DeviceVec3 a, const DeviceVec3 b)
{
    return MakeVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

__device__ float Dot(const DeviceVec3 a, const DeviceVec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

__device__ DeviceVec3 Cross(const DeviceVec3 a, const DeviceVec3 b)
{
    return MakeVec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x);
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

__device__ float Lerp(const float a, const float b, const float t)
{
    return a + (b - a) * t;
}

struct DistanceSample
{
    float distance;
    float orbitTrap;
};

__device__ DistanceSample EvaluateMandelbulbDistance(const DeviceVec3 position, const float power)
{
    DeviceVec3 z = position;
    float dr = 1.0f;
    float radius = 0.0f;
    float orbitTrap = 1.0e6f;

    for (int iteration = 0; iteration < 12; ++iteration)
    {
        radius = Length(z);
        orbitTrap = fminf(orbitTrap, radius);
        if (radius > 2.0f)
        {
            break;
        }

        const float safeRadius = fmaxf(radius, 1.0e-6f);
        const float theta = acosf(fminf(fmaxf(z.z / safeRadius, -1.0f), 1.0f));
        const float phi = atan2f(z.y, z.x);
        const float zr = powf(safeRadius, power);

        dr = powf(safeRadius, power - 1.0f) * power * dr + 1.0f;

        const float nextTheta = theta * power;
        const float nextPhi = phi * power;
        z = Add(
            Mul(
                MakeVec3(
                    sinf(nextTheta) * cosf(nextPhi),
                    sinf(nextTheta) * sinf(nextPhi),
                    cosf(nextTheta)),
                zr),
            position);
    }

    DistanceSample sample = {};
    sample.distance = 0.5f * logf(fmaxf(radius, 1.0e-6f)) * radius / fmaxf(dr, 1.0e-6f);
    sample.orbitTrap = orbitTrap;
    return sample;
}

__device__ DeviceVec3 EstimateNormal(const DeviceVec3 position, const float power)
{
    const float epsilon = 0.0015f;
    const DeviceVec3 offsetX = MakeVec3(epsilon, 0.0f, 0.0f);
    const DeviceVec3 offsetY = MakeVec3(0.0f, epsilon, 0.0f);
    const DeviceVec3 offsetZ = MakeVec3(0.0f, 0.0f, epsilon);

    const float dx =
        EvaluateMandelbulbDistance(Add(position, offsetX), power).distance -
        EvaluateMandelbulbDistance(Sub(position, offsetX), power).distance;
    const float dy =
        EvaluateMandelbulbDistance(Add(position, offsetY), power).distance -
        EvaluateMandelbulbDistance(Sub(position, offsetY), power).distance;
    const float dz =
        EvaluateMandelbulbDistance(Add(position, offsetZ), power).distance -
        EvaluateMandelbulbDistance(Sub(position, offsetZ), power).distance;

    return Normalize(MakeVec3(dx, dy, dz));
}

__device__ std::uint32_t PackRgba8(const float red, const float green, const float blue)
{
    const std::uint32_t r = static_cast<std::uint32_t>(Saturate(red) * 255.0f);
    const std::uint32_t g = static_cast<std::uint32_t>(Saturate(green) * 255.0f);
    const std::uint32_t b = static_cast<std::uint32_t>(Saturate(blue) * 255.0f);
    return 0xFF000000u | (b << 16U) | (g << 8U) | r;
}

__global__ void RenderKernel(
    cudaSurfaceObject_t outputSurface,
    std::uint32_t* centerPixel,
    const int startY,
    const int rowCount,
    const FrameState frameState)
{
    const int x = static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
    const int localY = static_cast<int>(blockIdx.y * blockDim.y + threadIdx.y);
    const int y = startY + localY;
    const int width = frameState.renderSettings.width;
    const int height = frameState.renderSettings.height;

    if (x >= width || localY >= rowCount || y >= height)
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
    const DeviceVec3 forward = Normalize(MakeVec3(
        frameState.camera.forward.x,
        frameState.camera.forward.y,
        frameState.camera.forward.z));
    const DeviceVec3 worldUp = fabsf(forward.y) > 0.98f ? MakeVec3(0.0f, 0.0f, 1.0f) : MakeVec3(0.0f, 1.0f, 0.0f);
    const DeviceVec3 right = Normalize(Cross(worldUp, forward));
    const DeviceVec3 up = Normalize(Cross(forward, right));
    const DeviceVec3 rayDirection = Normalize(Add(Add(Mul(right, uvX), Mul(up, uvY)), Mul(forward, 1.5f)));

    float distanceTravelled = 0.0f;
    float glow = 0.0f;
    bool hit = false;
    DeviceVec3 hitPosition = {};
    float orbitTrap = 1.0f;
    const float fractalPower = fmaxf(frameState.fractalPower, 2.0f);

    for (int step = 0; step < frameState.renderSettings.maxSteps; ++step)
    {
        const DeviceVec3 samplePosition = Add(rayOrigin, Mul(rayDirection, distanceTravelled));
        const DistanceSample sample = EvaluateMandelbulbDistance(samplePosition, fractalPower);
        const float distanceEstimate = fmaxf(sample.distance, frameState.renderSettings.hitEpsilon * 0.35f);

        glow += expf(-distanceEstimate * 12.0f) * 0.02f;

        if (distanceEstimate <= frameState.renderSettings.hitEpsilon)
        {
            hit = true;
            hitPosition = samplePosition;
            orbitTrap = sample.orbitTrap;
            break;
        }

        distanceTravelled += distanceEstimate;
        if (distanceTravelled > frameState.renderSettings.maxDistance)
        {
            break;
        }
    }

    const float vignette = 1.0f - Saturate((uvX * uvX + uvY * uvY) * 0.35f);
    const float fog = expf(-distanceTravelled * frameState.fogDensity);
    const float pulse = frameState.ambientPulse;

    float red = 0.015f + 0.015f * pulse;
    float green = 0.018f;
    float blue = 0.03f + 0.02f * pulse;

    if (hit)
    {
        const DeviceVec3 normal = EstimateNormal(hitPosition, fractalPower);
        const DeviceVec3 lightDirection = Normalize(MakeVec3(-0.45f, 0.7f, 0.55f));
        const DeviceVec3 viewDirection = Mul(rayDirection, -1.0f);
        const DeviceVec3 halfVector = Normalize(Add(lightDirection, viewDirection));

        const float diffuse = Saturate(Dot(normal, lightDirection));
        const float specular = powf(Saturate(Dot(normal, halfVector)), 28.0f);
        const float fresnel = powf(1.0f - Saturate(Dot(normal, viewDirection)), 3.0f);
        const float cavity = Saturate(1.0f - orbitTrap * 0.9f);

        const float surfaceR = Lerp(0.16f, 0.78f, cavity);
        const float surfaceG = Lerp(0.12f, 0.42f, cavity);
        const float surfaceB = Lerp(0.24f, 0.95f, 1.0f - cavity * 0.6f);

        const float lightMix = 0.14f + diffuse * 0.86f;
        red = surfaceR * lightMix + specular * 0.45f + fresnel * 0.08f;
        green = surfaceG * lightMix + specular * 0.35f + fresnel * 0.06f;
        blue = surfaceB * lightMix + specular * 0.60f + fresnel * 0.10f;
    }
    else
    {
        const float horizon = Saturate(0.5f + uvY * 0.5f);
        red = Lerp(0.015f, 0.06f, horizon) + glow * 0.08f;
        green = Lerp(0.018f, 0.04f, horizon) + glow * 0.05f;
        blue = Lerp(0.03f, 0.10f, horizon) + glow * 0.12f;
    }

    red = (red + glow * 0.10f) * vignette * fog;
    green = (green + glow * 0.07f) * vignette * fog;
    blue = (blue + glow * 0.14f) * vignette * (0.9f + 0.1f * fog);

    const std::uint32_t packed = PackRgba8(red, green, blue);

    const uchar4 rgba = make_uchar4(
        static_cast<unsigned char>(packed & 0xFFU),
        static_cast<unsigned char>((packed >> 8U) & 0xFFU),
        static_cast<unsigned char>((packed >> 16U) & 0xFFU),
        255U);
    surf2Dwrite(rgba, outputSurface, x * static_cast<int>(sizeof(uchar4)), y);

    if (x == width / 2 && y == height / 2)
    {
        *centerPixel = packed;
    }
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
    const cudaSurfaceObject_t outputSurface,
    std::uint32_t* deviceCenterPixel,
    std::uint32_t& centerPixel,
    std::string& errorMessage)
{
    errorMessage.clear();

    const dim3 blockSize(16, 16, 1);
    bool success = true;

    for (int startY = 0; startY < frameState.renderSettings.height && success; startY += kTileHeight)
    {
        const int rowCount = min(kTileHeight, frameState.renderSettings.height - startY);
        const dim3 gridSize(
            static_cast<unsigned int>((frameState.renderSettings.width + blockSize.x - 1) / blockSize.x),
            static_cast<unsigned int>((rowCount + blockSize.y - 1) / blockSize.y),
            1);

        RenderKernel<<<gridSize, blockSize>>>(outputSurface, deviceCenterPixel, startY, rowCount, frameState);

        success = CheckCudaStatus(cudaGetLastError(), errorMessage, "RenderKernel launch");
        if (success)
        {
            success = CheckCudaStatus(cudaDeviceSynchronize(), errorMessage, "cudaDeviceSynchronize");
        }
    }

    if (success)
    {
        success = CheckCudaStatus(
            cudaMemcpy(&centerPixel, deviceCenterPixel, sizeof(centerPixel), cudaMemcpyDeviceToHost),
            errorMessage,
            "cudaMemcpy(centerPixel)");
    }

    return success;
}
}
