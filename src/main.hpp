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
        SDL_Texture *texture;
    };

    struct TerrainVariant {
        std::string name;
        std::vector<Texture> terrainTextures;
    };
}

// == Global vars
extern SDL_Window *window;
extern SDL_Renderer *renderer;

namespace Assets {
    extern std::vector<TerrainVariant> terrainVariants;
}

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

inline void exit_error_img(const char *msg) {
    std::cout << msg << ": " << IMG_GetError() << std::endl;
    exit(1);
}
