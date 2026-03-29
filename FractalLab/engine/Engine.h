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
    int Run();

    const std::string& GetLastError() const;

private:
    void AdvanceTime();
    void UpdateScene();
    FrameState BuildFrameState() const;
    void LogFrame(const FrameState& frameState) const;

    EngineState state_;
    Renderer renderer_;
    std::string lastError_;
};
}
