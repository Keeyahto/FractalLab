#pragma once

#include <cstdint>

struct GLFWwindow;

namespace FractalLab
{
class Engine;

class App
{
public:
    int Run();

private:
    bool InitializeWindow();
    bool InitializeImGui();
    void Shutdown();
    void DrawFrameTexture(unsigned int texture, int displayWidth, int displayHeight) const;
    void DrawUi(Engine& engine, double deltaTimeSeconds) const;

    GLFWwindow* window_ = nullptr;
    mutable bool showImGuiDemo_ = false;
};
}
