#include "main.hpp"

#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

namespace Assets {
    std::vector<TerrainVariant> terrainVariants;
}

void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        exit_error_sdl("SDL_Init failed");

    if ((window = SDL_CreateWindow("www1game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN)) == NULL)
        exit_error_sdl("SDL_CreateWindow failed");

    if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)) == NULL)
        exit_error_sdl("SDL_CreateRenderer failed");

    int imgFlags = IMG_INIT_PNG;
    if(!(IMG_Init(imgFlags) & imgFlags))
        exit_error_img("IMG_Init failed");

    if (TTF_Init() < 0)
        exit_error_sdl("TTF_Init failed");
}

void destroySDL() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void renderLoop() {
    bool run = true;
    SDL_Event event;
    while (run) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        //SDL_RenderCopy(renderer, test_texture, NULL, NULL);

        SDL_RenderPresent(renderer);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                run = false;
            }
        }
    }
}
