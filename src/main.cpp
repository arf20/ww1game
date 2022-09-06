#include "main.hpp"

#include <iostream>

void printAssets() {
    std::cout << "Assets:" << std::endl;
    std::cout << "\tTerrain variants [" << Assets::terrainVariants.size() << "]:" << std::endl;
    for (const Assets::TerrainVariant& tvar : Assets::terrainVariants) {
        std::cout << "\t\t" << tvar.name << " [" << tvar.terrainTextures.size() << "]:" << std::endl;
        for (const Assets::Tile& tx : tvar.terrainTextures)
            std::cout << "\t\t\t" << tx.name[0] << ": " << tx.name << " " << tx.width << "x" << tx.height << std::endl;
    }

    std::cout << "\tCampaigns [" << Assets::campaigns.size() << "]:" << std::endl;
    for (const Assets::Campaign& c : Assets::campaigns) {
        std::cout << "\t\t" << c.name << ": \"" << c.nameNice << "\" [" << c.maps.size() << "]:" << std::endl;
        for (const Assets::Map& m : c.maps)
            std::cout << "\t\t\t" << m.id << ": \"" << m.name << "\" " << m.terrainVariantName << " " << m.width << "x" << m.height << std::endl;
    }

    std::cout << "\tFactions [" << Assets::factions.size() << "]:" << std::endl;
    for (const Assets::Faction& f : Assets::factions) {
        std::cout << "\t\t" << f.name << ": \"" << f.nameNice << "\" [" << f.characters.size() << "]:" << std::endl;
        for (const Assets::Character& c : f.characters)
            std::cout << "\t\t\t" << c.name << ": \"" << c.nameNice << "\" idle walk[" << c.march.size() << "] fire[" << c.fire.size() << "] death[" << c.death.size() << "] " << c.width << "x" << c.height << std::endl;
    }

    std::cout << "\tFonts [" << Assets::fonts.size() << "]:" << std::endl;
    for (const Assets::Font& f : Assets::fonts)
        std::cout << "\t\t" << f.name << ": " << f.size << std::endl;
}

int main(int argc, const char **argv) {
    initSDL();

    std::cout << "Loading assets into VRAM..." << std::endl;
    loadAssets();
    printAssets();

    selectedTerrainVariant = Assets::terrainVariants.begin();
    selectedMap = Assets::campaigns[0].maps.begin();

    Game::Soldier soldier;
    soldier.enemy = false;
    soldier.x = 50;
    soldier.y = 50;
    soldier.character = Assets::factions[0].characters.begin();
    soldier.state = soldier_state::MARCHING;
    soldier.frameCounter = 0;
    Game::soldiers.push_back(soldier);

    renderSetup();
    renderLoop();
    
    destroySDL();
    return 0;
}
