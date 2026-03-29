#pragma once

#include "engine/TimeState.h"
#include "render/RenderSettings.h"
#include "scene/SceneState.h"

namespace FractalLab
{
struct EngineState
{
    bool isRunning = false;
    unsigned int maxFrames = 180;
    bool animateScene = true;
    TimeState time;
    SceneState scene;
    RenderSettings renderSettings;
};
}
