#pragma once

#include "render/RenderSettings.h"
#include "scene/SceneState.h"

namespace FractalLab
{
struct FrameState
{
    unsigned long long frameIndex = 0;
    double elapsedSeconds = 0.0;
    CameraState camera;
    FractalType activeFractal = FractalType::PlaceholderField;
    RenderSettings renderSettings;
    float fractalPower = 8.0f;
    float glowIntensity = 0.65f;
    float fogDensity = 0.02f;
    float ambientPulse = 0.0f;
};
}
