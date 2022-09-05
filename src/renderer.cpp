#include "main.hpp"

#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

// util functions
SDL_Texture* getMapTexture(char c) {
    for (Assets::Tile& tx : selectedTerrainVariant->terrainTextures)
        if (tx.name[0] == c) return tx.texture;
    return missingTextureTexture;
}

void renderTexture(SDL_Texture *t, int w, int h, int x, int y) {
    SDL_Rect rect;
    rect.h = h; rect.w = w; rect.x = x; rect.y = y;
    SDL_RenderCopy(renderer, t, NULL, &rect);
}

#define TEXT_CENTERX    (unsigned int)1
#define TEXT_CENTERY    (unsigned int)2

int renderText(std::string str, TTF_Font* font, int x, int y, unsigned int flags, SDL_Color color) {
    SDL_Surface* surfaceText = TTF_RenderText_Solid(font, str.c_str(), color);
    SDL_Texture* textureText = SDL_CreateTextureFromSurface(renderer, surfaceText);

    SDL_Rect rectText;  // create a rect
    rectText.x = x;     // controls the rect's x coordinate 
    rectText.y = y;     // controls the rect's y coordinte
    rectText.w = 0;     // controls the width of the rect
    rectText.h = 0;     // controls the height of the rect

    TTF_SizeText(font, str.c_str(), &rectText.w, &rectText.h);

    if (flags & TEXT_CENTERX) rectText.x -= rectText.w / 2;
    if (flags & TEXT_CENTERY) rectText.y -= rectText.h / 2;

    SDL_RenderCopy(renderer, textureText, nullptr, &rectText);

    // I had to run valgrind to find this, I'm such a terrible programmer
    SDL_FreeSurface(surfaceText);
    SDL_DestroyTexture(textureText);

    return 0;
}


// globals owned by renderer
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

SDL_Texture* missingTextureTexture;

std::vector<Assets::TerrainVariant>::iterator selectedTerrainVariant;
std::vector<Assets::Map>::iterator selectedMap;
std::vector<Assets::Font>::iterator defaultFont;

// local stuff
#define TILE_SIZE   32

float fps = 0.0f;

int screenWidth = 1280;
int screenHeight = 720;

int worldOrgX = 0, mapOrgY = 0;

void renderMap() {
    for (int y = 0; y < selectedMap->height; y++) {
        for (int x = 0; x < selectedMap->width; x++) {
            if (selectedMap->map[y][x] == ' ') continue;
            renderTexture(getMapTexture(selectedMap->map[y][x]), TILE_SIZE, TILE_SIZE, worldOrgX + (TILE_SIZE * x), mapOrgY + (TILE_SIZE * y));
        }
    }
}

void renderSetup() {
    
}

void render() {
    mapOrgY = screenHeight - (TILE_SIZE * selectedMap->height);
    renderMap();

    renderText(std::string("fps: ") + std::to_string(fps), defaultFont->font, 10, 10, 0, { 255, 255, 255, 255 });
}

// public functions
void renderLoop() {
    bool run = true;
    SDL_Event event;
    while (run) {
        Uint32 start_time = SDL_GetTicks();
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN: {
                    switch (event.key.keysym.sym) {
                        case SDLK_a: {
                            worldOrgX += 10;
                        } break;
                        case SDLK_d: {
                            worldOrgX -= 10;
                        } break;
                    }
                } break;
                case SDL_QUIT: {
                    run = false;
                } break;
            }
        }

        SDL_GetWindowSize(window, &screenWidth, &screenHeight);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        render();

        SDL_RenderPresent(renderer);
        Uint32 frame_time = SDL_GetTicks() - start_time;
        fps = (frame_time > 0) ? 1000.0f / frame_time : 0.0f;
    }
}

void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        exit_error_sdl("SDL_Init failed");

    if ((window = SDL_CreateWindow("www1game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)) == NULL)
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
