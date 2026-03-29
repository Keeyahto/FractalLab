#include "app/App.h"

#include "engine/Engine.h"

#include <exception>
#include <iostream>

namespace FractalLab
{
int App::Run()
{
    try
    {
        Engine engine;
        if (!engine.Initialize())
        {
            std::cerr << "Fractal Lab failed to initialize: " << engine.GetLastError() << '\n';
            return 1;
        }

        return engine.Run();
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Unhandled exception: " << exception.what() << '\n';
        return 1;
    }
}
}
