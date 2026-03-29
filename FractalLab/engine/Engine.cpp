#include "engine/Engine.h"

#include <cmath>

namespace FractalLab
{
bool Engine::Initialize()
{
    state_.isRunning = true;
    state_.scene.activeFractal = FractalType::Mandelbulb;
    state_.scene.camera.position = { 0.0f, 0.0f, -4.0f };
    state_.scene.camera.yawRadians = 0.0f;
    state_.scene.camera.pitchRadians = 0.0f;
    state_.scene.camera.forward = { 0.0f, 0.0f, 1.0f };

    if (!renderer_.Initialize(state_.renderSettings))
    {
        lastError_ = renderer_.GetLastError();
        return false;
    }

    return true;
}

void Engine::Shutdown()
{
    renderer_.Shutdown();
    state_.isRunning = false;
}

bool Engine::Tick(const double deltaTimeSeconds)
{
    if (!state_.isRunning)
    {
        lastError_ = "Engine is not initialized.";
        return false;
    }

    state_.time.deltaTimeSeconds = deltaTimeSeconds > 0.0 ? deltaTimeSeconds : 1.0 / 60.0;
    UpdateScene();

    const FrameState frameState = BuildFrameState();
    if (!renderer_.Render(frameState))
    {
        lastError_ = renderer_.GetLastError();
        return false;
    }

    AdvanceTime();
    return true;
}

void Engine::Resize(const int width, const int height)
{
    if (width > 0)
    {
        state_.renderSettings.width =
            static_cast<int>(std::max(1.0f, std::round(static_cast<float>(width) * state_.renderSettings.resolutionScale)));
    }

    if (height > 0)
    {
        state_.renderSettings.height =
            static_cast<int>(std::max(1.0f, std::round(static_cast<float>(height) * state_.renderSettings.resolutionScale)));
    }
}

const std::string& Engine::GetLastError() const
{
    return lastError_;
}

EngineState& Engine::GetState()
{
    return state_;
}

const EngineState& Engine::GetState() const
{
    return state_;
}

Renderer& Engine::GetRenderer()
{
    return renderer_;
}

const Renderer& Engine::GetRenderer() const
{
    return renderer_;
}

void Engine::AdvanceTime()
{
    ++state_.time.frameIndex;
    state_.time.elapsedSeconds += state_.time.deltaTimeSeconds;
}

void Engine::UpdateScene()
{
    if (!state_.animateScene)
    {
        return;
    }

    const float time = static_cast<float>(state_.time.elapsedSeconds);

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
}
