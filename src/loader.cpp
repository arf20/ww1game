#include "main.hpp"

#include <vector>
#include <filesystem>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define ASSET_PATH  "../assets"

void loadTerrain() {
    if (!std::filesystem::exists(ASSET_PATH))
        exit_error("Asset directory does not exist");
    
    if (!std::filesystem::exists(ASSET_PATH "/terrain"))
        exit_error("Terrain directory does not exist");

    for (const auto& entry : std::filesystem::directory_iterator(ASSET_PATH "/terrain")) {
        if (entry.path().extension() != ".png") continue;

        texture t { };
        t.name = entry.path().stem();
        if ((t.texture = IMG_LoadTexture(renderer, entry.path().c_str())) == NULL)
            exit_error_img("IMG_LoadTexture failed");

        terrainTextures.push_back(t);
    }
}
