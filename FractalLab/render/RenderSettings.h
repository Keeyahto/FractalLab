#pragma once

namespace FractalLab
{
struct RenderSettings
{
    int width = 960;
    int height = 540;
    int maxSteps = 96;
    float hitEpsilon = 0.001f;
    float maxDistance = 32.0f;
    float resolutionScale = 1.0f;
};
}
