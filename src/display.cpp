#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "SDL2\SDL.h"
#include "display.h"

struct Display
{
    size_t        width    = 0;
    size_t        height   = 0;
    SDL_Window*   window   = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture * texture  = NULL;
};

Display* newDisplay(size_t width, size_t height)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());

        return NULL;
    }
    
    Display* display = (Display*) calloc(1, sizeof(Display));
    assert(display != NULL);

    display->width  = width;
    display->height = height;

    display->window = SDL_CreateWindow("Software CPU display",
                                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       DISPLAY_PXL_SIZE * width, DISPLAY_PXL_SIZE * height,
                                       0);
    assert(display->window != NULL);

    display->renderer = SDL_CreateRenderer(display->window, -1, 
                                           SDL_RENDERER_ACCELERATED);
    assert(display->renderer != NULL);

    display->texture = SDL_CreateTexture(display->renderer,
                                         SDL_PIXELFORMAT_RGBA8888, 
                                         SDL_TEXTUREACCESS_STREAMING,
                                         width, height);
    assert(display->texture != NULL);

    return display;
}

Display* newDisplay()
{
    return newDisplay(DISPLAY_DEFAULT_WIDTH, DISPLAY_DEFAULT_HEIGHT);
}

void deleteDisplay(Display* display)
{
    free(display);
}

void updateDisplay(Display* display, unsigned char* buffer)
{
    assert(display != NULL);
    assert(buffer  != NULL);

    SDL_Event event = {};
    while (SDL_PollEvent(&event))
        if (event.type == SDL_QUIT) 
            exit(0);

    SDL_RenderClear(display->renderer);
    SDL_UpdateTexture(display->texture, 
                      NULL, 
                      (void*) buffer, 
                      display->width * 4); // 4 for ARGB

    SDL_RenderCopy(display->renderer, display->texture, NULL, NULL);
    SDL_RenderPresent(display->renderer);
}

size_t getDisplayBufferSize(Display* display)
{
    assert(display != NULL);

    return display->width * display->height * 4;
}

