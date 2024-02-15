/*
    ww1game:      Generic WW1 game (?)
    renderer.cpp: Renders frames

    Copyright (C) 2022 Ángel Ruiz Fernandez

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "main.hpp"

#include <iostream>
#include <chrono>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

// util functions
SDL_Texture* getMapTexture(char c) {
    for (Assets::Tile& tx : Game::selectedTerrainVariant->terrainTextures)
        if (tx.name[0] == c) return tx.texture;
    return Assets::missingTextureTexture;
}

void renderTexture(SDL_Texture *t, int w, int h, int x, int y, bool mirror = false) {
    SDL_Rect rect;
    rect.h = h; rect.w = w; rect.x = x; rect.y = y;
    SDL_RendererFlip flip = mirror ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, t, NULL, &rect, 0.0, NULL, flip);
}

#define TEXT_CENTERX    (unsigned int)1
#define TEXT_CENTERY    (unsigned int)2

int renderText(std::string str, TTF_Font* font, int x, int y, unsigned int flags, SDL_Color color) {
    SDL_Surface* surfaceText = TTF_RenderText_Blended(font, str.c_str(), color);
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

void setColor(SDL_Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
}


// globals owned by renderer
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

#define C_WHITE  {255, 255, 255, 255}
#define C_BLACK  {0, 0, 0, 255}
#define C_YELLOW {255, 255, 0, 255}
#define C_RED    {255, 0, 0, 255}  
#define C_GREEN  {0, 255, 0}
#define C_A      {224, 201, 166, 255}

namespace Assets {
    std::vector<Assets::Font>::iterator defaultFont;

    SDL_Texture *missingTextureTexture;
    Mix_Chunk *missingSoundSound;
    Mix_Music *missingMusicMusic;
}

// local stuff
#define ANIM_FPS    7

bool run = true;

float fps = 0.0f;
auto time_prev = std::chrono::high_resolution_clock::now();
long frameCounter = 0;
int anim_div = 0;
bool inMenu = true;

int screenWidth = 1280;
int screenHeight = 720;

int worldOrgX = 0, worldOrgY = 0;

void renderBackground() {
    for (const Assets::Background& background : Assets::backgrounds) {
        if (background.name == Game::selectedMap->backgroundName) {
            setColor(background.skyColor);
            SDL_RenderClear(renderer);
            float factor = float(screenWidth) / float(background.width);
            renderTexture(background.texture, factor * background.width, factor * background.height, 0, (worldOrgY + Game::friendlyMapPath[0].pos.y) - (factor * background.height), false);
            return;
        }
    }

    renderTexture(Assets::missingTextureTexture, screenWidth, screenHeight - (Game::friendlyMapPath[0].pos.y), 0, 0, false);
}

int menuBgIdx = 0;

void renderMenuBackground() {
    auto& background = Assets::backgrounds[menuBgIdx];
    setColor(background.skyColor);
    SDL_RenderClear(renderer);
    float factor = float(screenWidth) / float(background.width);
    renderTexture(background.texture, factor * background.width, factor * background.height, 0, screenHeight - (factor * background.height), false);
}

void renderMap() {
    for (int y = 0; y < Game::selectedMap->height; y++) {
        for (int x = 0; x < Game::selectedMap->width; x++) {
            if (Game::selectedMap->map[y][x] == ' ') continue;
            renderTexture(getMapTexture(Game::selectedMap->map[y][x]), TILE_SIZE, TILE_SIZE, worldOrgX + (TILE_SIZE * x), worldOrgY + (TILE_SIZE * y), false);
        }
    }

    // render flags
    for (int i = 0; i < Game::friendlyMapPath.size(); i++) {
        if (Game::friendlyMapPath[i].type == Game::MapPathPoint::TRENCH && Game::friendlyMapPath[i].action == Game::MapPathPoint::MARCH) {
            renderTexture(Assets::flagpoleTexture, TILE_SIZE, 3 * TILE_SIZE, worldOrgX + Game::friendlyMapPath[i].pos.x - (TILE_SIZE / 2), worldOrgY + Game::friendlyMapPath[i].pos.y - (3 * TILE_SIZE));
            renderTexture(Game::friendlyFaction->flag, 2 * TILE_SIZE, Game::friendlyFaction->flagHeight, worldOrgX + Game::friendlyMapPath[i].pos.x, worldOrgY + Game::friendlyMapPath[i].pos.y - (3 * TILE_SIZE));
        }
    }
    for (int i = 0; i < Game::enemyMapPath.size(); i++) {
        if (Game::enemyMapPath[i].type == Game::MapPathPoint::TRENCH && Game::enemyMapPath[i].action == Game::MapPathPoint::MARCH) {
            renderTexture(Assets::flagpoleTexture, TILE_SIZE, 3 * TILE_SIZE, worldOrgX + Game::enemyMapPath[i].pos.x - (TILE_SIZE / 2), worldOrgY + Game::enemyMapPath[i].pos.y - (3 * TILE_SIZE));
            renderTexture(Game::enemyFaction->flag, 2 * TILE_SIZE, Game::enemyFaction->flagHeight, worldOrgX + Game::enemyMapPath[i].pos.x, worldOrgY + Game::enemyMapPath[i].pos.y - (3 * TILE_SIZE));
        }
    }

    if (debug) {
        for (int i = 0; i < Game::friendlyMapPath.size() - 1; i++) {
            if (Game::friendlyMapPath[i].type == Game::MapPathPoint::GROUND) setColor(C_RED);
            else setColor(C_YELLOW);
            SDL_RenderDrawLineF(renderer, worldOrgX + Game::friendlyMapPath[i].pos.x, worldOrgY + Game::friendlyMapPath[i].pos.y, worldOrgX + Game::friendlyMapPath[i + 1].pos.x, worldOrgY + Game::friendlyMapPath[i + 1].pos.y);
        }

        for (int i = 0; i < Game::enemyMapPath.size() - 1; i++) {
            if (Game::enemyMapPath[i].type == Game::MapPathPoint::GROUND) setColor(C_RED);
            else setColor(C_YELLOW);
            SDL_RenderDrawLineF(renderer, worldOrgX + Game::enemyMapPath[i].pos.x, worldOrgY + Game::enemyMapPath[i].pos.y, worldOrgX + Game::enemyMapPath[i + 1].pos.x, worldOrgY + Game::enemyMapPath[i + 1].pos.y);
        }

        setColor(C_RED);
        SDL_RenderDrawLineF(renderer, worldOrgX + Game::enemyObjective->pos.x, worldOrgY + Game::enemyObjective->pos.y, worldOrgX + Game::enemyObjective->pos.x, worldOrgY + Game::enemyObjective->pos.y - 100);
        setColor(C_GREEN);
        SDL_RenderDrawLineF(renderer, worldOrgX + Game::friendlyObjective->pos.x, worldOrgY + Game::friendlyObjective->pos.y, worldOrgX + Game::friendlyObjective->pos.x, worldOrgY + Game::friendlyObjective->pos.y - 100);
    }
}

void renderBullets() {
    for (const Game::Bullet& bullet : Game::bullets)
        renderTexture(Assets::bulletTexture, 32, 32, worldOrgX + bullet.pos.x, worldOrgY + bullet.pos.y, false);
}

void renderSoldiers(std::vector<Game::Soldier>& soldiers, bool enemy) {
    for (Game::Soldier& soldier : soldiers) {
        switch (soldier.state) {
            case Game::Soldier::FIRING: {
                if (soldier.frameCounter >= soldier.character->fire.size()) { soldier.state = soldier.prevState; soldier.frameCounter = 0;
                    renderTexture(soldier.character->idle, soldier.character->size.x, soldier.character->size.y, worldOrgX + soldier.pos.x, worldOrgY + soldier.pos.y, enemy);
                    continue; }
                renderTexture(soldier.character->fire[soldier.frameCounter], soldier.character->size.x, soldier.character->size.y, worldOrgX + soldier.pos.x, worldOrgY + soldier.pos.y, enemy);
                if (frameCounter == 0) { soldier.frameCounter++; }
            } break;
            case Game::Soldier::SoldierState::DYING: {
                if (soldier.frameCounter >= soldier.character->death.size()) break;
                renderTexture(soldier.character->death[soldier.frameCounter], soldier.character->size.x, soldier.character->size.y, worldOrgX + soldier.pos.x, worldOrgY + soldier.pos.y, enemy);
                if (frameCounter == 0) { soldier.frameCounter++; }
            } break;
            case Game::Soldier::SoldierState::IDLE: {
                renderTexture(soldier.character->idle, soldier.character->size.x, soldier.character->size.y, worldOrgX + soldier.pos.x, worldOrgY + soldier.pos.y, enemy);
            } break;
            case Game::Soldier::SoldierState::MARCHING: {
                if (soldier.frameCounter >= soldier.character->march.size()) soldier.frameCounter = 0;
                renderTexture(soldier.character->march[soldier.frameCounter], soldier.character->size.x, soldier.character->size.y, worldOrgX + soldier.pos.x, worldOrgY + soldier.pos.y, enemy);
                if (frameCounter == 0) { soldier.frameCounter++; }
            } break;
        }
    }
}

void menuKeyHandler(SDL_Keycode key) {
    if (key >= SDLK_0 && key <= SDLK_9) {
        int itemIdx = key - SDLK_0;
        if (Game::selectedCampaign == Assets::campaigns.end()) {
            if (itemIdx < Assets::campaigns.size())
                Game::selectedCampaign = Assets::campaigns.begin() + itemIdx;
                Game::selectedMap = Game::selectedCampaign->maps.end();
        } else {
            if (itemIdx < Game::selectedCampaign->maps.size())
                Game::selectedMap = Game::selectedCampaign->maps.begin() + itemIdx;
        }
    }
}

void renderMenu() {
    //setColor(C_BLACK);
    //SDL_RenderClear(renderer);
    renderMenuBackground();

    renderText("ww1game: arf20's arcade-ish 2D WW1 game (?)", Assets::defaultFont->font20, screenWidth / 2, 50, TEXT_CENTERX, C_BLACK);

    SDL_Rect button;
    button.w = 400;
    button.h = 40;
    button.x = (screenWidth / 2) - 200;
    setColor(C_A);

    if (Game::selectedCampaign == Assets::campaigns.end())
        for (int i = 0; i < Assets::campaigns.size(); i++) {
            button.y = 100 + (i * 60);
            SDL_RenderFillRect(renderer, &button);
            renderText(std::to_string(i) + ". " + Assets::campaigns[i].nameNice, Assets::defaultFont->font20, screenWidth / 2, 120 + (i * 60), TEXT_CENTERX | TEXT_CENTERY, C_BLACK);
        }
    else
        for (int i = 0; i < Game::selectedCampaign->maps.size(); i++) {
            button.y = 100 + (i * 60);
            SDL_RenderFillRect(renderer, &button);
            renderText(std::to_string(i) + ". " + Game::selectedCampaign->maps[i].name, Assets::defaultFont->font20, screenWidth / 2, 120 + (i * 60), TEXT_CENTERX | TEXT_CENTERY, C_BLACK);
        }

    if (Game::selectedCampaign != Assets::campaigns.end())
        if (Game::selectedMap != Game::selectedCampaign->maps.end()) {
            inMenu = false;
            Game::mapSetup();
        }
}

void renderHud() {
    // Render soldiers
    SDL_Rect button;
    setColor(C_A);

    for (int i = 0; i < Game::friendlyFaction->characters.size(); i++) {
        auto& c = Game::friendlyFaction->characters[i];
        button.w = c.size.x; button.h = c.size.y;
        button.x = 10 + ((10 + c.size.x) * i); button.y = screenHeight - (10 + c.size.y);
        SDL_RenderFillRect(renderer, &button);
        renderText(c.nameNice, Assets::defaultFont->font12, button.x + button.w / 2, button.y, TEXT_CENTERX, C_BLACK);
        renderTexture(c.idle, c.size.x, c.size.y, button.x, button.y);
    }

    if (Game::gameMode) {
        int orgx = screenWidth - ((10 + Game::enemyFaction->characters[0].size.x) * Game::enemyFaction->characters.size());
        for (int i = 0; i < Game::enemyFaction->characters.size(); i++) {
            auto& c = Game::enemyFaction->characters[i];
            button.w = c.size.x; button.h = c.size.y;
            button.x = orgx + ((10 + c.size.x) * i); button.y = screenHeight - (10 + c.size.y);
            SDL_RenderFillRect(renderer, &button);
            renderText(c.nameNice, Assets::defaultFont->font12, button.x + button.w / 2, button.y, TEXT_CENTERX, C_BLACK);
            renderTexture(c.idle, c.size.x, c.size.y, button.x, button.y);
        }
    }
}

void gameKeyHandler(SDL_Keycode key) {
    switch (key) {
        case SDLK_c: {
            run = false;
        } break;
        case SDLK_a: {
            worldOrgX += 10;
        } break;
        case SDLK_d: {
            worldOrgX -= 10;
        } break;
        case SDLK_q: {
            for (int i = 0; i < Game::friendlyMapPath.size(); i++) {
                auto& p = Game::friendlyMapPath[i];
                if (p.action == Game::MapPathPoint::HOLD) {
                    if (Game::friendlyObjective != (Game::friendlyMapPath.begin() + i))
                        p.action = Game::MapPathPoint::MARCH;
                    break;
                }
            }
        } break;
        case SDLK_e: {
            for (int i = Game::enemyMapPath.size() - 1; i >= 0; i--) {
                auto& p = Game::enemyMapPath[i];
                if (p.action == Game::MapPathPoint::HOLD) {
                    if (Game::enemyObjective != (Game::enemyMapPath.begin() + i))
                        p.action = Game::MapPathPoint::MARCH;
                    break;
                }
            }
        } break;
    }

    // keys 1-5 spawn friendlies
    if (key >= SDLK_1 && key <= SDLK_5)
        if (key - SDLK_1 < Game::friendlyFaction->characters.size())
            Game::soldierSpawn(Game::friendlyFaction->characters.begin() + (key - SDLK_1), false);
    
    // keys 6-0 (top keyb numerical row) enemies
    if (key >= SDLK_6 && key <= SDLK_9)
        if (5 - (key - SDLK_6 + 1) < Game::enemyFaction->characters.size())
            Game::soldierSpawn(Game::enemyFaction->characters.begin() + 5 - (key - SDLK_6 + 1), true);

    if (key == SDLK_0)
        if (Game::enemyFaction->characters.size() > 0)
            Game::soldierSpawn(Game::enemyFaction->characters.begin(), true);
}

void Renderer::setup() {
    menuBgIdx = std::rand() % Assets::backgrounds.size();
}

void render(float deltaTime) {
    if (inMenu) {
        renderMenu();
    } else {
        Game::update(deltaTime);

        worldOrgY = screenHeight - (TILE_SIZE * Game::selectedMap->height);

        renderBackground();
        renderMap();
        renderBullets();
        renderSoldiers(Game::friendlies, false);
        renderSoldiers(Game::enemies, true);
        renderHud();
    }

    if (debug) {
        renderText(std::string("fps: ") + std::to_string(fps) + " deltaTime: " + std::to_string(deltaTime), Assets::defaultFont->font12, 10, 10, 0, C_BLACK);
        std::string campaginstr = Game::selectedCampaign != Assets::campaigns.end() ? Game::selectedCampaign->nameNice : "(invalid)";
        std::string mapstr;
        if (Game::selectedCampaign != Assets::campaigns.end())
            mapstr = Game::selectedMap != Game::selectedCampaign->maps.end() ? Game::selectedMap->name : "(invalid)";
        else mapstr = "(invalid)";
        renderText(std::string("campaign: ") + campaginstr, Assets::defaultFont->font12, 10, 24, 0, C_BLACK);
        renderText(std::string("map: ") + mapstr, Assets::defaultFont->font12, 10, 38, 0, C_BLACK);

        std::string friendlystr = Game::friendlyFaction != Assets::factions.end() ? Game::friendlyFaction->nameNice : "(invalid)";
        std::string enemystr = Game::enemyFaction != Assets::factions.end() ? Game::enemyFaction->nameNice : "(invalid)";
        renderText(std::string("friendly: ") + friendlystr, Assets::defaultFont->font12, 10, 52, 0, C_BLACK);
        renderText(std::string("enemy: ") + enemystr, Assets::defaultFont->font12, 10, 66, 0, C_BLACK);

	    renderText(std::string("friendlies: ") + std::to_string(Game::friendlies.size())
            + ", casualties " + std::to_string(Game::friendlyCasualties)
            + ", holding " + std::to_string(Game::friendliesHoldingbjective), Assets::defaultFont->font12, 10, 80, 0, C_BLACK);
	    renderText(std::string("enemies: ") + std::to_string(Game::enemies.size())
            + ", casualties " + std::to_string(Game::enemyCasualties)
            + ", holding " + std::to_string(Game::enemiesHoldingObjective), Assets::defaultFont->font12, 10, 94, 0, C_BLACK);
    }
}

// public functions
void Renderer::loop() {
    std::cout << "Running render loop..." << std::endl;
    
    SDL_Event event;
    while (run) {
        //Uint32 time_now = SDL_GetTicks();
        auto time_now = std::chrono::high_resolution_clock::now();
        float deltaTime = (time_now - time_prev).count() / 1000000000.0f;
        fps = (deltaTime > 0.0f) ? 1.0f / deltaTime : 1.0f;
        anim_div = std::roundl(fps / (float)ANIM_FPS);
        if (anim_div == 0.0f) anim_div = 1.0;
        time_prev = time_now;
        if (frameCounter > anim_div) { frameCounter = 0; }

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN: {
                    if (inMenu) menuKeyHandler(event.key.keysym.sym);
                    else gameKeyHandler(event.key.keysym.sym);
                } break;
                case SDL_QUIT: {
                    run = false;
                } break;
            }
        }

        SDL_GetWindowSize(window, &screenWidth, &screenHeight);

        render(deltaTime);

        if (!run) return;
        SDL_RenderPresent(renderer);

        frameCounter++;
    }
}

void Renderer::initSDL() {
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

void Renderer::destroySDL() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    //SDL_Quit();   // this hangs, SDL_Quit might been have called earlier
}
