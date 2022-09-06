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
    std::vector<Faction> factions;
    std::vector<Font> fonts;
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

        for (const auto& entryTile : std::filesystem::directory_iterator(entryVariant.path().string())) {
            if (!entryTile.is_regular_file()) continue;
            if (entryTile.path().extension() != ".png") continue;

            Assets::Tile tile { };
            tile.name = entryTile.path().stem();
            if ((tile.texture = IMG_LoadTexture(renderer, entryTile.path().c_str())) == NULL) {
                error_img("IMG_LoadTexture failed on assets/terrain/" + variant.name + "/" + tile.name + ".png");
                continue;
            }

            if (SDL_QueryTexture(tile.texture, NULL, NULL, &tile.width, &tile.height) < 0) {
                error_sdl("SDL_QueryTexture failed on assets/terrain/" + variant.name + "/" + tile.name + ".png");
                continue;
            }

            variant.terrainTextures.push_back(tile);
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

            Assets::Map map { };
            try { map.id = std::stoi(entryMap.path().stem()); }
            catch (std::invalid_argument) {
                std::cout << "Map filename NaN: " << entryMap.path().filename() << std::endl;
                continue;
            }

            std::ifstream fileMap(entryMap.path().string());
            std::vector<std::string> fileMapLines;
            std::string line;
            while (std::getline(fileMap, line))
                fileMapLines.push_back(line);

            if (fileMapLines.size() < 3) {
                std::cout << "Invalid map format, less than 3 lines: " << entryMap.path().filename() << std::endl;
                continue;
            }

            map.name = fileMapLines[0];
            map.terrainVariantName = fileMapLines[1];
            
            for (int i = 2; i < fileMapLines.size(); i++)
                map.map.push_back(fileMapLines[i]);

            if (map.map[0].length() < 1) {
                std::cout << "Invalid map format, at least 1 unit long: " << entryMap.path().filename() << std::endl;
                continue;
            }

            for (int i = 0; i < map.map.size(); i++) {
                if (map.map[i].length() != map.map[0].length()) {
                    std::cout << "Invalid map format, all map lines should be the same length: " << entryMap.path().stem() << ":" << i << std::endl;
                    continue;
                }
            }

            map.width = map.map[0].length();
            map.height = map.map.size();

            campaign.maps.push_back(map);
        }

        std::sort(campaign.maps.begin(), campaign.maps.end(), sortMaps);

        Assets::campaigns.push_back(campaign);
    }
}

void loadCharacterAnimation(const std::filesystem::path& path, std::vector<SDL_Texture*>& anim) {
    std::vector<int> frameNs;
    for (const auto& entryFrame : std::filesystem::directory_iterator(path.string())) {
        if (!entryFrame.is_regular_file()) continue;
        if (entryFrame.path().extension() != ".png") continue;

        try { frameNs.push_back(std::stoi(entryFrame.path().stem())); }
        catch (std::invalid_argument) {
            std::cout << "Frame filename NaN: " << entryFrame.path().filename() << std::endl;
            continue;
        }
    }

    std::sort(frameNs.begin(), frameNs.end());

    for (const int& frameN : frameNs) {
        SDL_Texture *frame;
        if ((frame = IMG_LoadTexture(renderer, (path / (std::to_string(frameN) + ".png")).c_str())) == NULL) {
            error_img("IMG_LoadTexture failed on " + (path / (std::to_string(frameN) + ".png")).string());
            anim.push_back(missingTextureTexture);
        }
        anim.push_back(frame);
    }
}

void loadCharacters() {
    if (!std::filesystem::exists(ASSET_PATH "/factions"))
        exit_error("Terrain directory does not exist");

    for (const auto& entryFaction : std::filesystem::directory_iterator(ASSET_PATH "/factions")) {
        if (!entryFaction.is_directory()) continue;

        Assets::Faction faction;
        faction.name = entryFaction.path().filename();
        faction.nameNice = makeNameNice(faction.name);

        for (const auto& entryCharacter : std::filesystem::directory_iterator(entryFaction.path().string())) {
            if (!entryCharacter.is_directory()) continue;

            Assets::Character character;
            character.name = entryCharacter.path().stem();
            character.nameNice = makeNameNice(character.name);

            if (!std::filesystem::exists(entryCharacter.path() / "idle.png")) {
                std::cout << "Warning: No idle texture for " << character.name << std::endl;
                character.idle = missingTextureTexture;
            }

            if ((character.idle = IMG_LoadTexture(renderer, (entryCharacter.path() / "idle.png").c_str())) == NULL) {
                error_img("IMG_LoadTexture failed on assets/" + faction.name + "/" + character.name + "/idle.png");
                character.idle = missingTextureTexture;
            }

            if (SDL_QueryTexture(character.idle, NULL, NULL, &character.width, &character.height) < 0) {
                error_sdl("SDL_QueryTexture failed on assets/" + faction.name + "/" + character.name + "/idle.png");
                character.idle = missingTextureTexture;
            }

            if (!std::filesystem::exists(entryCharacter.path() / "walk")) {
                std::cout << "Warning: No walk animation for " << character.name << std::endl;
                character.march.push_back(missingTextureTexture);
            }

            if (!std::filesystem::exists(entryCharacter.path() / "fire")) {
                std::cout << "Warning: No fire animation for " << character.name << std::endl;
                character.fire.push_back(missingTextureTexture);
            }

            if (!std::filesystem::exists(entryCharacter.path() / "death")) {
                std::cout << "Warning: No death animation for " << character.name << std::endl;
                character.death.push_back(missingTextureTexture);
            }

            loadCharacterAnimation(entryCharacter.path() / "walk", character.march);
            loadCharacterAnimation(entryCharacter.path() / "fire", character.fire);
            loadCharacterAnimation(entryCharacter.path() / "death", character.death);

            faction.characters.push_back(character);
        }

        Assets::factions.push_back(faction);
    }
}

void loadFonts() {
    if (!std::filesystem::exists(ASSET_PATH "/fonts"))
        exit_error("Fonts directory does not exist");

    for (const auto& entryFont : std::filesystem::directory_iterator(ASSET_PATH "/fonts")) {
        if (!entryFont.is_regular_file()) continue;
        if (entryFont.path().extension() != ".ttf") continue;

        Assets::Font font;
        font.name = entryFont.path().stem();
        font.size = 12;
        if ((font.font = TTF_OpenFont(entryFont.path().c_str(), 12)) == NULL)
            std::cout << "Error opening font " << entryFont.path().filename() << ": " << TTF_GetError() << std::endl;

        Assets::fonts.push_back(font);
    }

    for (int i = 0; i < Assets::fonts.size(); i++)
        if (Assets::fonts[i].name == "default") defaultFont = Assets::fonts.begin() + i;
}

void loadAssets() {
    if (!std::filesystem::exists(ASSET_PATH))
        exit_error("Asset directory does not exist");

    if (!std::filesystem::exists(ASSET_PATH "/missing_texture.png"))
        exit_error("Missing texture placeholder texture missing");

     if ((missingTextureTexture = IMG_LoadTexture(renderer, ASSET_PATH "/missing_texture.png")) == NULL)
        exit_error_img("IMG_LoadTexture failed on missing_texture");

    loadTerrains();
    loadMaps();
    loadCharacters();
    loadFonts();
}