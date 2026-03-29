#pragma once

namespace FractalLab
{
struct RenderSettings
{
    int width = 960;
    int height = 540;
    int maxSteps = 48;
    float hitEpsilon = 0.001f;
    float maxDistance = 24.0f;
    float resolutionScale = 0.5f;
};
}
