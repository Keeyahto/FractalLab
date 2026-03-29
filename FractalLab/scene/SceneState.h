#pragma once

#include "scene/Camera.h"

namespace FractalLab
{
enum class FractalType
{
    PlaceholderField = 0,
    Mandelbulb = 1,
};

struct SceneState
{
    CameraState camera;
    FractalType activeFractal = FractalType::PlaceholderField;
    float fractalPower = 8.0f;
    float glowIntensity = 0.65f;
    float fogDensity = 0.02f;
    float ambientPulse = 0.0f;
};
}
