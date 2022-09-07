#include "main.hpp"

#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

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
    SDL_Surface* surfaceText = TTF_RenderText_Shaded(font, str.c_str(), color, {0, 0, 0, 0});
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
Mix_Chunk* missingSoundSound;

std::vector<Assets::TerrainVariant>::iterator selectedTerrainVariant;
std::vector<Assets::Map>::iterator selectedMap;
std::vector<Assets::Font>::iterator defaultFont;

// local stuff
#define ANIM_FPS    7

float fps = 0.0f;
Uint32 time_prev = 0;
long frameCounter = 0;
int anim_div = 0;

int screenWidth = 1280;
int screenHeight = 720;

int worldOrgX = 0, worldOrgY = 0;

void renderMap() {
    for (int y = 0; y < selectedMap->height; y++) {
        for (int x = 0; x < selectedMap->width; x++) {
            if (selectedMap->map[y][x] == ' ') continue;
            renderTexture(getMapTexture(selectedMap->map[y][x]), TILE_SIZE, TILE_SIZE, worldOrgX + (TILE_SIZE * x), worldOrgY + (TILE_SIZE * y));
        }
    }
}

void renderSoldiers() {
    for (auto it = Game::soldiers.begin(); it != Game::soldiers.end(); it++) {
        auto& soldier = *it;
        switch (soldier.state) {
            case SoldierState::IDLE: {
                renderTexture(soldier.character->idle, soldier.character->width, soldier.character->height, worldOrgX + soldier.x, worldOrgY + soldier.y);
            } break;
            case SoldierState::MARCHING: {
                if (soldier.frameCounter == soldier.character->march.size()) soldier.frameCounter = 0;
                renderTexture(soldier.character->march[soldier.frameCounter], soldier.character->width, soldier.character->height, worldOrgX + soldier.x, worldOrgY + soldier.y);
                if ((frameCounter % anim_div) == 0) soldier.frameCounter++;
            } break;
            case SoldierState::FIRING: {
                if (soldier.frameCounter == soldier.character->fire.size()) { soldier.state = soldier.prevState; soldier.frameCounter = 0; continue; }
                renderTexture(soldier.character->fire[soldier.frameCounter], soldier.character->width, soldier.character->height, worldOrgX + soldier.x, worldOrgY + soldier.y);
                if ((frameCounter % anim_div) == 0) soldier.frameCounter++;
            } break;
            case SoldierState::DYING: {
                if (soldier.frameCounter == soldier.character->death.size()) { Game::soldiers.erase(it); continue; }
                renderTexture(soldier.character->death[soldier.frameCounter], soldier.character->width, soldier.character->height, worldOrgX + soldier.x, worldOrgY + soldier.y);
                if ((frameCounter % anim_div) == 0) soldier.frameCounter++;
            } break;
        }
    }
}

void renderSetup() {
    
}

void render() {
    worldOrgY = screenHeight - (TILE_SIZE * selectedMap->height);

    renderMap();
    renderSoldiers();

    renderText(std::string("fps: ") + std::to_string(fps), defaultFont->font, 10, 10, 0, { 255, 255, 255, 255 });
}

// public functions
void renderLoop() {
    bool run = true;
    SDL_Event event;
    while (run) {
        Uint32 time_now = SDL_GetTicks();
        Uint32 deltaTime = time_now - time_prev;
        fps = (deltaTime > 0) ? 1000.0f / deltaTime : 0.0f;
        if (fps == 0.0f) fps = 1.0;
        anim_div = std::roundl(fps / (float)ANIM_FPS);
        if (anim_div == 0.0f) anim_div = 1.0;
        time_prev = time_now;

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
                        case SDLK_z: {
                            soldiersFire(Game::soldiers.begin());
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

        gameUpdate(float(deltaTime) / 1000.0f);
        render();

        SDL_RenderPresent(renderer);

        frameCounter++;
    }
}

void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
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

    int mixFlags = MIX_INIT_OGG;
    if (!(Mix_Init(mixFlags) & mixFlags))
        exit_error_sdl("Mix_Init failed");

    if (Mix_OpenAudio(48000, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
        exit_error_sdl("Mix_OpenAudio failed");
}

void destroySDL() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
