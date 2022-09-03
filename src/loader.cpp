#include "main.hpp"

#include <vector>
#include <filesystem>
#include <fstream>
#include <algorithm>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

namespace Assets {
    std::vector<TerrainVariant> terrainVariants;
    std::vector<Campaign> campaigns;
}

std::string makeNameNice(std::string str) {
    std::replace(str.begin(), str.end(), '_', ' ');
    str[0] = std::toupper(str[0]);
    for (int i = 0; i < str.size(); i++)
        if ((str[i] == ' ') && (i + 1 < str.size())) str[i + 1] = std::toupper(str[i + 1]);
    return str;
}

void loadTerrains() {
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
            if ((t.texture = IMG_LoadTexture(renderer, entryTexture.path().c_str())) == NULL) {
                error_img("IMG_LoadTexture failed");
                continue;
            }

            if (SDL_QueryTexture(t.texture, NULL, NULL, &t.width, &t.height) < 0) {
                error_sdl("SDL_QueryTexture failed");
            }

            variant.terrainTextures.push_back(t);
        }

        Assets::terrainVariants.push_back(variant);
    }
}

bool sortMaps(const Assets::Map& a, const Assets::Map& b) {
    return a.id < b.id;
}

void loadMaps() {
    if (!std::filesystem::exists(ASSET_PATH "/campaigns"))
        exit_error("Terrain directory does not exist");

    for (const auto& entryCampaign : std::filesystem::directory_iterator(ASSET_PATH "/campaigns")) {
        if (!entryCampaign.is_directory()) continue;

        Assets::Campaign campaign;
        campaign.name = entryCampaign.path().filename();
        campaign.nameNice = makeNameNice(campaign.name);

        for (const auto& entryMap : std::filesystem::directory_iterator(entryCampaign.path().string())) {
            if (!entryMap.is_regular_file()) continue;
            if (entryMap.path().extension() != ".map") continue;

            Assets::Map m { };
            try { m.id = std::stoi(entryMap.path().stem()); }
            catch (std::invalid_argument ) { 
                std::cout << "Map filename NaN: " << entryMap.path().stem() << std::endl;
                continue;
            }

            std::ifstream fileMap(entryMap.path().string());
            std::vector<std::string> fileMapLines;
            std::string line;
            while (std::getline(fileMap, line))
                fileMapLines.push_back(line);

            if (fileMapLines.size() < 3) {
                std::cout << "Invalid map format, less than 3 lines: " << entryMap.path().stem() << std::endl;
                continue;
            }

            m.name = fileMapLines[0];
            m.terrainVariantName = fileMapLines[1];
            
            for (int i = 2; i < fileMapLines.size(); i++)
                m.map.push_back(fileMapLines[i]);

            if (m.map[0].length() < 1) {
                std::cout << "Invalid map format, at least 1 unit long: " << entryMap.path().stem() << std::endl;
                continue;
            }

            for (int i = 0; i < m.map.size(); i++) {
                if (m.map[i].length() != m.map[0].length()) {
                    std::cout << "Invalid map format, all map lines should be the same length: " << entryMap.path().stem() << ":" << i << std::endl;
                    continue;
                }
            }

            campaign.maps.push_back(m);
        }

        std::sort(campaign.maps.begin(), campaign.maps.end(), sortMaps);

        Assets::campaigns.push_back(campaign);
    }
}

void loadAssets() {
    if (!std::filesystem::exists(ASSET_PATH))
        exit_error("Asset directory does not exist");

    loadTerrains();
    loadMaps();
}