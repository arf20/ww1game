#include "main.hpp"

#include <vector>
#include <filesystem>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

void loadTerrain() {
    if (!std::filesystem::exists(ASSET_PATH))
        exit_error("Asset directory does not exist");
    
    if (!std::filesystem::exists(ASSET_PATH "/terrain"))
        exit_error("Terrain directory does not exist");

    for (const auto& entryVariant : std::filesystem::directory_iterator(ASSET_PATH "/terrain")) {
        if (!entryVariant.is_directory()) continue;

        Assets::TerrainVariant variant;
        variant.name = entryVariant.path().filename();

        for (const auto& entryTexture : std::filesystem::directory_iterator(entryVariant.path().string())) {
            if (!entryTexture.is_regular_file()) continue;
            if (entryTexture.path().extension() != ".png") continue;

            Assets::Texture t { };
            t.name = entryTexture.path().stem();
            if ((t.texture = IMG_LoadTexture(renderer, entryTexture.path().c_str())) == NULL)
                exit_error_img("IMG_LoadTexture failed");

            variant.terrainTextures.push_back(t);
        }

        Assets::terrainVariants.push_back(variant);
    }
}

void loadAssets() {
    loadTerrain();
}