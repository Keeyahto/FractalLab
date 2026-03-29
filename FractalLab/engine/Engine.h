#pragma once

#include "engine/EngineState.h"
#include "render/Renderer.h"

#include <string>

namespace FractalLab
{
class Engine
{
public:
    bool Initialize();
    void Shutdown();
    bool Tick(double deltaTimeSeconds);
    void Resize(int width, int height);

    const std::string& GetLastError() const;
    EngineState& GetState();
    const EngineState& GetState() const;
    Renderer& GetRenderer();
    const Renderer& GetRenderer() const;

private:
    void AdvanceTime();
    void UpdateScene();
    FrameState BuildFrameState() const;

    EngineState state_;
    Renderer renderer_;
    std::string lastError_;
};
}
