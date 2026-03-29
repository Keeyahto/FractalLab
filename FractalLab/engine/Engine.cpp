#include "engine/Engine.h"

#include <cmath>
#include <iomanip>
#include <iostream>

namespace FractalLab
{
bool Engine::Initialize()
{
    state_.isRunning = true;
    state_.scene.activeFractal = FractalType::PlaceholderField;
    state_.scene.camera.position = { 0.0f, 0.0f, -4.0f };
    state_.scene.camera.forward = { 0.0f, 0.0f, 1.0f };

    if (!renderer_.Initialize(state_.renderSettings))
    {
        lastError_ = renderer_.GetLastError();
        return false;
    }

    return true;
}

int Engine::Run()
{
    std::cout << "Fractal Lab skeleton booted.\n";
    std::cout << "Phase 0 runtime loop is active through App -> Engine -> Renderer -> CUDA.\n";

    while (state_.isRunning && state_.time.frameIndex < state_.maxFrames)
    {
        UpdateScene();

        const FrameState frameState = BuildFrameState();
        if (!renderer_.Render(frameState))
        {
            lastError_ = renderer_.GetLastError();
            std::cerr << "Render failed: " << lastError_ << '\n';
            return 1;
        }

        const bool shouldLog =
            frameState.frameIndex == 0 ||
            ((frameState.frameIndex + 1) % 30ULL) == 0ULL ||
            frameState.frameIndex + 1ULL == state_.maxFrames;

        if (shouldLog)
        {
            LogFrame(frameState);
        }

        AdvanceTime();
    }

    std::cout << "Runtime loop completed after " << state_.time.frameIndex << " frames.\n";
    return 0;
}

const std::string& Engine::GetLastError() const
{
    return lastError_;
}

void Engine::AdvanceTime()
{
    ++state_.time.frameIndex;
    state_.time.elapsedSeconds += state_.time.deltaTimeSeconds;
}

void Engine::UpdateScene()
{
    const float time = static_cast<float>(state_.time.elapsedSeconds);

    state_.scene.camera.position.x = std::sin(time * 0.35f) * 0.6f;
    state_.scene.camera.position.y = std::cos(time * 0.22f) * 0.35f;
    state_.scene.camera.position.z = -4.0f + std::sin(time * 0.18f) * 0.4f;

    state_.scene.ambientPulse = 0.5f + 0.5f * std::sin(time * 1.3f);
    state_.scene.glowIntensity = 0.45f + 0.35f * (0.5f + 0.5f * std::sin(time * 0.6f));
    state_.scene.fogDensity = 0.015f + 0.01f * (0.5f + 0.5f * std::cos(time * 0.4f));
}

FrameState Engine::BuildFrameState() const
{
    FrameState frameState;
    frameState.frameIndex = state_.time.frameIndex;
    frameState.elapsedSeconds = state_.time.elapsedSeconds;
    frameState.camera = state_.scene.camera;
    frameState.activeFractal = state_.scene.activeFractal;
    frameState.renderSettings = state_.renderSettings;
    frameState.fractalPower = state_.scene.fractalPower;
    frameState.glowIntensity = state_.scene.glowIntensity;
    frameState.fogDensity = state_.scene.fogDensity;
    frameState.ambientPulse = state_.scene.ambientPulse;
    return frameState;
}

void Engine::LogFrame(const FrameState& frameState) const
{
    std::cout
        << "frame=" << std::setw(3) << frameState.frameIndex
        << " time=" << std::fixed << std::setprecision(2) << frameState.elapsedSeconds
        << "s center=0x" << std::hex << std::setw(8) << std::setfill('0') << renderer_.GetCenterPixel()
        << std::dec << std::setfill(' ')
        << " res=" << frameState.renderSettings.width << 'x' << frameState.renderSettings.height
        << '\n';
}
}
