#include "main.hpp"

#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

// util functions
SDL_Texture* getMapTexture(char c) {
    for (Assets::Texture& tx : selectedTerrainVariant->terrainTextures)
        if (tx.name[0] == c) return tx.texture;
    return missingTextureTexture;
}

void renderTexture(SDL_Texture *t, int w, int h, int x, int y) {
    SDL_Rect rect;
    rect.h = h; rect.w = w; rect.x = x; rect.y = y;
    SDL_RenderCopy(renderer, t, NULL, &rect);
}


// globals owned by renderer
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

SDL_Texture* missingTextureTexture;

std::vector<Assets::TerrainVariant>::iterator selectedTerrainVariant;
std::vector<Assets::Map>::iterator selectedMap;

// local stuff
#define WIDTH       1280
#define HEIGHT      720
#define TILE_SIZE   32

int mapOrgX = 0, mapOrgY = 0;

// public functions
void renderLoop() {
    bool run = true;
    SDL_Event event;
    while (run) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        for (int y = 0; y < selectedMap->height; y++) {
            for (int x = 0; x < selectedMap->width; x++) {
                if (selectedMap->map[y][x] == ' ') continue;
                renderTexture(getMapTexture(selectedMap->map[y][x]), TILE_SIZE, TILE_SIZE, mapOrgX + (TILE_SIZE * x), mapOrgY + (TILE_SIZE * y));
            }
        }

        SDL_RenderPresent(renderer);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                run = false;
            }
        }
    }
}

void renderSetup() {
    mapOrgX = 0;
    mapOrgY = HEIGHT - (TILE_SIZE * selectedMap->height);
}

void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        exit_error_sdl("SDL_Init failed");

    if ((window = SDL_CreateWindow("www1game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN)) == NULL)
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
