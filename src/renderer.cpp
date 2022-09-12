#include "main.hpp"

#include <iostream>
#include <chrono>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

// util functions
SDL_Texture* getMapTexture(char c) {
    for (Assets::Tile& tx : Assets::selectedTerrainVariant->terrainTextures)
        if (tx.name[0] == c) return tx.texture;
    return Assets::missingTextureTexture;
}

void renderTexture(SDL_Texture *t, int w, int h, int x, int y, bool mirror) {
    SDL_Rect rect;
    rect.h = h; rect.w = w; rect.x = x; rect.y = y;
    SDL_RendererFlip flip = mirror ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, t, NULL, &rect, 0.0, NULL, flip);
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

namespace Assets {
    std::vector<Assets::Faction>::iterator selectedFaction;
    std::vector<Assets::TerrainVariant>::iterator selectedTerrainVariant;
    std::vector<Assets::Map>::iterator selectedMap;
    std::vector<Assets::Font>::iterator defaultFont;

    SDL_Texture *missingTextureTexture;
    Mix_Chunk *missingSoundSound;
    Mix_Music *missingMusicMusic;
}

// local stuff
#define ANIM_FPS    7

float fps = 0.0f;
std::chrono::_V2::system_clock::time_point time_prev = std::chrono::high_resolution_clock::now();
long frameCounter = 0;
int anim_div = 0;

int screenWidth = 1280;
int screenHeight = 720;

int worldOrgX = 0, worldOrgY = 0;

void renderBackground() {
    for (const Assets::Background& background : Assets::backgrounds) {
        if (background.name == Assets::selectedMap->backgroundName) {
            SDL_SetRenderDrawColor(renderer, background.skyColor.r, background.skyColor.g, background.skyColor.b, SDL_ALPHA_OPAQUE);
            SDL_RenderClear(renderer);
            float factor = float(screenWidth) / float(background.width);
            renderTexture(background.texture, factor * background.width, factor * background.height, 0, (worldOrgY + Game::mapPath[0].pos.y) - (factor * background.height), false);
            return;
        }
    }

    renderTexture(Assets::missingTextureTexture, screenWidth, screenHeight - (Game::mapPath[0].pos.y), 0, 0, false);
}

void renderMap() {
    for (int y = 0; y < Assets::selectedMap->height; y++) {
        for (int x = 0; x < Assets::selectedMap->width; x++) {
            if (Assets::selectedMap->map[y][x] == ' ') continue;
            renderTexture(getMapTexture(Assets::selectedMap->map[y][x]), TILE_SIZE, TILE_SIZE, worldOrgX + (TILE_SIZE * x), worldOrgY + (TILE_SIZE * y), false);
        }
    }

    if (debug)
        for (int i = 0; i < Game::mapPath.size() - 1; i++) {
            if (Game::mapPath[i].type == Game::MapPathPoint::GROUND) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            else SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderDrawLineF(renderer, worldOrgX + Game::mapPath[i].pos.x, worldOrgY + Game::mapPath[i].pos.y, worldOrgX + Game::mapPath[i + 1].pos.x, worldOrgY + Game::mapPath[i + 1].pos.y);
        }
}

void renderBullets() {
    for (const Game::Bullet& bullet : Game::bullets)
        renderTexture(Assets::bulletTexture, 16, 16, worldOrgX + bullet.pos.x, worldOrgY + bullet.pos.y, false);
}

void renderSoldiers(std::vector<Game::Soldier>& soldiers, bool enemy) {
    for (Game::Soldier& soldier : soldiers) {
        switch (soldier.state) {
            case Game::Soldier::FIRING: {
                if (soldier.frameCounter >= soldier.character->fire.size()) { soldier.state = soldier.prevState; soldier.frameCounter = 0;
                    renderTexture(soldier.character->idle, soldier.character->size.x, soldier.character->size.y, worldOrgX + soldier.pos.x, worldOrgY + soldier.pos.y, enemy);
                    continue; }
                renderTexture(soldier.character->fire[soldier.frameCounter], soldier.character->size.x, soldier.character->size.y, worldOrgX + soldier.pos.x, worldOrgY + soldier.pos.y, enemy);
                if ((frameCounter % anim_div) == 0) soldier.frameCounter++;
            } break;
            case Game::Soldier::SoldierState::DYING: {
                if (soldier.frameCounter >= soldier.character->death.size()) break;
                renderTexture(soldier.character->death[soldier.frameCounter], soldier.character->size.x, soldier.character->size.y, worldOrgX + soldier.pos.x, worldOrgY + soldier.pos.y, enemy);
                if ((frameCounter % anim_div) == 0) soldier.frameCounter++;
            } break;
            case Game::Soldier::SoldierState::IDLE: {
                renderTexture(soldier.character->idle, soldier.character->size.x, soldier.character->size.y, worldOrgX + soldier.pos.x, worldOrgY + soldier.pos.y, enemy);
            } break;
            case Game::Soldier::SoldierState::MARCHING: {
                if (soldier.frameCounter >= soldier.character->march.size()) soldier.frameCounter = 0;
                renderTexture(soldier.character->march[soldier.frameCounter], soldier.character->size.x, soldier.character->size.y, worldOrgX + soldier.pos.x, worldOrgY + soldier.pos.y, enemy);
                if ((frameCounter % anim_div) == 0) soldier.frameCounter++;
            } break;
        }
    }
}

void renderSetup() {
    
}

void render(float deltaTime) {
    worldOrgY = screenHeight - (TILE_SIZE * Assets::selectedMap->height);

    renderBackground();
    renderMap();
    renderBullets();
    renderSoldiers(Game::friendlies, false);
    renderSoldiers(Game::enemies, true);

    if (debug)
        renderText(std::string("fps: ") + std::to_string(fps) + " deltaTime: " + std::to_string(deltaTime), Assets::defaultFont->font, 10, 10, 0, { 255, 255, 255, 255 });
}

// public functions
void renderLoop() {
    bool run = true;
    SDL_Event event;
    while (run) {
        //Uint32 time_now = SDL_GetTicks();
        std::chrono::_V2::system_clock::time_point time_now = std::chrono::high_resolution_clock::now();
        float deltaTime = (time_now - time_prev).count() / 1000000000.0f;
        fps = (deltaTime > 0.0f) ? 1.0f / deltaTime : 1.0f;
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
                        case SDLK_e: {
                            for (Game::MapPathPoint& p : Game::mapPath)
                                if (p.type == Game::MapPathPoint::TRENCH) {
                                    p.type = Game::MapPathPoint::GROUND;
                                    break;
                                }
                        } break;
                        case SDLK_1: {
                            soldierSpawn("german_empire", "officer", false);
                        } break;
                        case SDLK_2: {
                            soldierSpawn("german_empire", "officer", true);
                        } break;
                    }
                } break;
                case SDL_QUIT: {
                    run = false;
                } break;
            }
        }

        SDL_GetWindowSize(window, &screenWidth, &screenHeight);

        gameUpdate(deltaTime);
        render(deltaTime);

        if (!run) return;
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

    if (Mix_OpenAudio(48000, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
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
