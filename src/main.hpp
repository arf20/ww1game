#pragma once

// == Global includes
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <vector>
#include <string>

// == Macros
#define ASSET_PATH  "../assets"

#define TILE_SIZE   32

// == Types
namespace Assets {
    struct Tile {
        std::string name;
        int width, height;
        SDL_Texture *texture;
    };

    struct TerrainVariant {
        std::string name;
        std::vector<Tile> terrainTextures;
    };

    struct Map {
        int id;
        std::string name;
        std::string terrainVariantName;
        int width, height;
        std::vector<std::string> map;
    };

    struct Campaign {
        std::string name;
        std::string nameNice;
        std::vector<Map> maps;
    };

    struct Character {
        std::string name;
        std::string nameNice;
        int width, height;
        SDL_Texture *idle;
        std::vector<SDL_Texture*> march;
        std::vector<SDL_Texture*> fire;
        std::vector<SDL_Texture*> death;
    };

    struct Faction {
        std::string name;
        std::string nameNice;
        std::vector<Character> characters;
    };

    struct Font {
        std::string name;
        int size;
        TTF_Font* font;
    };
}

enum SoldierState { IDLE, MARCHING, FIRING, DYING };

namespace Game {
    struct Soldier {
        bool enemy; // (false = friend)
        int x, y;
        int vx, vy;
        std::vector<Assets::Character>::iterator character;
        SoldierState prevState, state;  // 0 idle, 1 running, 2 firing, 3 dying
        int frameCounter;
    };
}

// == Global vars
// owned by loader
namespace Assets {
    extern std::vector<TerrainVariant> terrainVariants;
    extern std::vector<Campaign> campaigns;
    extern std::vector<Faction> factions;
    extern std::vector<Font> fonts;
}

// owned by game
namespace Game {
    extern std::vector<Soldier> soldiers;
}

// owned by renderer
extern SDL_Window *window;
extern SDL_Renderer *renderer;

extern SDL_Texture *missingTextureTexture;

extern std::vector<Assets::TerrainVariant>::iterator selectedTerrainVariant;
extern std::vector<Assets::Map>::iterator selectedMap;
extern std::vector<Assets::Font>::iterator defaultFont;

// == Global functions
// Renderer
void initSDL();
void destroySDL();
void renderSetup();
void renderLoop();

// Loader
void loadAssets();

// Game
void gameUpdate(float deltaTime);

// Inline util
inline void exit_error(const std::string& msg) {
    std::cout << msg << std::endl;
    exit(1);
}

inline void exit_error_sdl(const std::string& msg) {
    std::cout << msg << ": " << SDL_GetError() << std::endl;
    exit(1);
}

inline void error_sdl(const std::string& msg) {
    std::cout << msg << ": " << SDL_GetError() << std::endl;
}

inline void exit_error_img(const std::string& msg) {
    std::cout << msg << ": " << IMG_GetError() << std::endl;
    exit(1);
}

inline void error_img(const std::string& msg) {
    std::cout << msg << ": " << IMG_GetError() << std::endl;
}
