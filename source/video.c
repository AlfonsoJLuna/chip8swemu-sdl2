#include "video.h"

#include "chip8.h"
#include "config.h"
#include "icon.h"

#include <SDL.h>
#include <stdio.h>


static SDL_Window* Window = NULL;
static SDL_Renderer* Renderer = NULL;
static SDL_Texture* Texture = NULL;

static double LastTime = 0;


static void Video_SetIcon()
{
    SDL_Surface* Surface = NULL;

    Surface = SDL_CreateRGBSurfaceFrom(
        (void*)icon, 96, 96, 32, 96*4,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000
    );

    SDL_SetWindowIcon(Window, Surface);
}

bool Video_Init()
{
    config_t Config = Config_Get();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    Window = SDL_CreateWindow(
        "chip8swemu",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        Config.WindowWidth,
        Config.WindowHeight,
        SDL_WINDOW_SHOWN |
        SDL_WINDOW_RESIZABLE
    );

    if (Window == NULL)
    {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    Video_SetIcon();

    if (Config.EnableFullscreen)
    {
        SDL_SetWindowFullscreen(Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }

    if (Config.EnableVSync)
    {
        Renderer = SDL_CreateRenderer(
            Window,
            -1,
            SDL_RENDERER_ACCELERATED |
            SDL_RENDERER_PRESENTVSYNC
        );
    }
    else
    {
        Renderer = SDL_CreateRenderer(
            Window,
            -1,
            SDL_RENDERER_ACCELERATED
        );
    }

    if (Renderer == NULL)
    {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    Texture = SDL_CreateTexture(
        Renderer,
        SDL_PIXELFORMAT_RGB888,
        SDL_TEXTUREACCESS_TARGET,
        128,
        64
    );

    if (Texture == NULL)
    {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    return 0;
}

void Video_ToggleFullscreen()
{
    config_t Config = Config_Get();

    if (Config.EnableFullscreen)
    {
        SDL_SetWindowFullscreen(Window, 0);
        Config.EnableFullscreen = false;
    }
    else
    {
        SDL_SetWindowFullscreen(Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        Config.EnableFullscreen = true;
    }

    Config_Set(Config);
}

static void Video_Wait()
{
    double FramePeriod = 1000.0 / 60.0; // ms

    double CurrentTime = SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency();

    double BusyTime = (CurrentTime - LastTime) * 1000.0;

    if (BusyTime < FramePeriod)
    {
        SDL_Delay(FramePeriod - BusyTime);
    }

    LastTime = SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency();
}

void Video_Render()
{
    config_t Config = Config_Get();

    // Render the CHIP-8 display into the texture
    SDL_SetRenderTarget(Renderer, Texture);
    SDL_SetRenderDrawColor(Renderer, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(Renderer);

    for (int x = 0; x < 128; x++)
    {
        for (int y = 0; y < 64; y++)
        {
            if (chip8GetPixel(y, x))
            {
                SDL_SetRenderDrawColor(
                    Renderer,
                    Config.ColorAccent.R,
                    Config.ColorAccent.G,
                    Config.ColorAccent.B,
                    SDL_ALPHA_OPAQUE
                );
            }
            else
            {
                SDL_SetRenderDrawColor(
                    Renderer,
                    Config.ColorBackground.R,
                    Config.ColorBackground.G,
                    Config.ColorBackground.B,
                    SDL_ALPHA_OPAQUE
                );
            }

            SDL_RenderDrawPoint(Renderer, x, y);
        }
    }

    // Render the texture into the window
    SDL_SetRenderTarget(Renderer, NULL);
    SDL_SetRenderDrawColor(Renderer, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(Renderer);
    SDL_RenderCopy(Renderer, Texture, NULL, NULL);
    SDL_RenderPresent(Renderer);

    // Limit the framerate if VSync is disabled
    if (Config.EnableVSync == false)
    {
        Video_Wait();
    }
}

void Video_Quit()
{
    if (Texture != NULL)
    {
        SDL_DestroyTexture(Texture);
        Texture = NULL;
    }

    if (Renderer != NULL)
    {
        SDL_DestroyRenderer(Renderer);
        Renderer = NULL;
    }

    if (Window != NULL)
    {
        SDL_DestroyWindow(Window);
        Window = NULL;
    }

    SDL_Quit();
}
