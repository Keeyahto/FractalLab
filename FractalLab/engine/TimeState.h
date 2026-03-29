#pragma once

namespace FractalLab
{
struct TimeState
{
    unsigned long long frameIndex = 0;
    double deltaTimeSeconds = 1.0 / 60.0;
    double elapsedSeconds = 0.0;
};
}
