#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>
#include <vector>
#include <string>

#define WIDTH   1280
#define HEIGHT  720

// == Types
struct texture {
    std::string name;
    SDL_Texture *texture;
};

// == Global vars
extern SDL_Window *window;
extern SDL_Renderer *renderer;

extern std::vector<texture> terrainTextures;

// == Global functions
// Renderer
void initSDL();
void destroySDL();
void renderLoop();

// Loader
void loadTerrain();

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
