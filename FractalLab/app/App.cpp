#include "app/App.h"

#include "engine/Engine.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl2.h>

#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace
{
constexpr int kInitialWindowWidth = 1280;
constexpr int kInitialWindowHeight = 720;
constexpr const char* kWindowTitle = "Fractal Lab";
}

namespace FractalLab
{
bool App::InitializeWindow()
{
    if (glfwInit() != GLFW_TRUE)
    {
        std::cerr << "GLFW initialization failed.\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    window_ = glfwCreateWindow(kInitialWindowWidth, kInitialWindowHeight, kWindowTitle, nullptr, nullptr);
    if (window_ == nullptr)
    {
        std::cerr << "Failed to create GLFW window.\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    return true;
}

bool App::InitializeImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    if (!ImGui_ImplGlfw_InitForOpenGL(window_, true))
    {
        std::cerr << "Failed to initialize ImGui GLFW backend.\n";
        return false;
    }

    if (!ImGui_ImplOpenGL2_Init())
    {
        std::cerr << "Failed to initialize ImGui OpenGL2 backend.\n";
        return false;
    }

    return true;
}

void App::Shutdown()
{
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window_ != nullptr)
    {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }

    glfwTerminate();
}

void App::DrawFrameTexture(const unsigned int texture, const int displayWidth, const int displayHeight) const
{
    glViewport(0, 0, displayWidth, displayHeight);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClearColor(0.015f, 0.015f, 0.025f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void App::DrawUi(Engine& engine, const double deltaTimeSeconds) const
{
    EngineState& state = engine.GetState();
    const Renderer& renderer = engine.GetRenderer();

    ImGui::Begin("Fractal Lab");
    ImGui::Text("Phase 0 windowed runtime");
    ImGui::Separator();
    ImGui::Text("Frame: %llu", state.time.frameIndex);
    ImGui::Text("Delta: %.2f ms", deltaTimeSeconds * 1000.0);
    ImGui::Text("Render resolution: %d x %d", renderer.GetWidth(), renderer.GetHeight());
    ImGui::Text("Center pixel: 0x%08X", renderer.GetCenterPixel());

    ImGui::Separator();
    ImGui::Checkbox("Animate scene", &state.animateScene);
    ImGui::SliderFloat("Resolution scale", &state.renderSettings.resolutionScale, 0.35f, 1.0f, "%.2f");
    ImGui::SliderFloat("Fractal power", &state.scene.fractalPower, 2.0f, 16.0f);
    ImGui::SliderFloat("Glow intensity", &state.scene.glowIntensity, 0.0f, 1.5f);
    ImGui::SliderFloat("Fog density", &state.scene.fogDensity, 0.0f, 0.08f);
    ImGui::SliderInt("Max steps", &state.renderSettings.maxSteps, 16, 192);
    ImGui::SliderFloat("Max distance", &state.renderSettings.maxDistance, 8.0f, 64.0f);

    ImGui::Separator();
    ImGui::Checkbox("Show ImGui demo", &showImGuiDemo_);
    ImGui::TextWrapped("Current display path uses CUDA/OpenGL interop, so the frame stays on the GPU all the way to presentation.");
    ImGui::End();

    if (showImGuiDemo_)
    {
        ImGui::ShowDemoWindow(&showImGuiDemo_);
    }
}

int App::Run()
{
    try
    {
        if (!InitializeWindow())
        {
            return 1;
        }

        if (!InitializeImGui())
        {
            Shutdown();
            return 1;
        }

        Engine engine;
        if (!engine.Initialize())
        {
            std::cerr << "Fractal Lab failed to initialize: " << engine.GetLastError() << '\n';
            Shutdown();
            return 1;
        }

        double previousFrameTime = glfwGetTime();
        while (glfwWindowShouldClose(window_) == GLFW_FALSE)
        {
            glfwPollEvents();

            int framebufferWidth = 0;
            int framebufferHeight = 0;
            glfwGetFramebufferSize(window_, &framebufferWidth, &framebufferHeight);
            if (framebufferWidth <= 0 || framebufferHeight <= 0)
            {
                glfwWaitEventsTimeout(0.05);
                continue;
            }

            const double currentFrameTime = glfwGetTime();
            const double deltaTimeSeconds = currentFrameTime - previousFrameTime;
            previousFrameTime = currentFrameTime;

            engine.Resize(framebufferWidth, framebufferHeight);
            if (!engine.Tick(deltaTimeSeconds))
            {
                std::cerr << "Fractal Lab frame failed: " << engine.GetLastError() << '\n';
                engine.Shutdown();
                Shutdown();
                return 1;
            }

            ImGui_ImplOpenGL2_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            DrawUi(engine, deltaTimeSeconds);

            DrawFrameTexture(engine.GetRenderer().GetOutputTexture(), framebufferWidth, framebufferHeight);

            ImGui::Render();
            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

            std::ostringstream title;
            title << kWindowTitle << " | "
                  << std::fixed << std::setprecision(1)
                  << (deltaTimeSeconds > 0.0 ? (1.0 / deltaTimeSeconds) : 0.0)
                  << " FPS";
            glfwSetWindowTitle(window_, title.str().c_str());

            glfwSwapBuffers(window_);
        }

        engine.Shutdown();
        Shutdown();
        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Unhandled exception: " << exception.what() << '\n';
        Shutdown();
        return 1;
    }
}
}
