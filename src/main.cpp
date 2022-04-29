
// Tell SDL not to mess with main()
#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#include <iostream>
#include <vector>

#include "renderer/renderer.h"
#include "system/config.h"

int main(int argc, char *argv[])
{
    Config::Instance()->ParseCommandLine(argc, argv);

    Renderer renderer;

    renderer.Init();

    // Poll for user input.
    bool stillRunning = true;
    while (stillRunning)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {

            case SDL_QUIT:
                stillRunning = false;
                break;

            default:
                // Do nothing.
                break;
            }
        }
        // TODO: Need a vsync system here
        SDL_Delay(10);
    }

    renderer.Shutdown();
    return 0;
}
