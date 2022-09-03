#pragma once

// == Global includes
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>
#include <vector>
#include <string>

// == Macros
#define WIDTH   1280
#define HEIGHT  720

#define ASSET_PATH  "../assets"

// == Types
namespace Assets {
    struct Texture {
        std::string name;
        int width, height;
        SDL_Texture *texture;
    };

    struct TerrainVariant {
        std::string name;
        std::vector<Texture> terrainTextures;
    };

    struct Map {
        int id;
        std::string name;
        std::string terrainVariantName;
        std::vector<std::string> map;
    };

    struct Campaign {
        std::string name;
        std::string nameNice;
        std::vector<Map> maps;
    };
}

// == Global vars
// owned by loader
namespace Assets {
    extern std::vector<TerrainVariant> terrainVariants;
    extern std::vector<Campaign> campaigns;
}

// owned by renderer
extern SDL_Window *window;
extern SDL_Renderer *renderer;

extern std::vector<Assets::TerrainVariant>::iterator selectedTerrainVariant;
extern std::vector<Map>::iterator selectedMap;

// == Global functions
// Renderer
void initSDL();
void destroySDL();
void renderLoop();

// Loader
void loadAssets();

// Inline util
inline void exit_error(const char *msg) {
    std::cout << msg << std::endl;
    exit(1);
}

inline void exit_error_sdl(const char *msg) {
    std::cout << msg << ": " << SDL_GetError() << std::endl;
    exit(1);
}

inline void error_sdl(const char *msg) {
    std::cout << msg << ": " << SDL_GetError() << std::endl;
}

inline void exit_error_img(const char *msg) {
    std::cout << msg << ": " << IMG_GetError() << std::endl;
    exit(1);
}

inline void error_img(const char *msg) {
    std::cout << msg << ": " << IMG_GetError() << std::endl;
}
