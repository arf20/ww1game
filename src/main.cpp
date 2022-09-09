#include "main.hpp"

#include <iostream>

bool debug = true;

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
            std::cout << "\t\t\t" << m.id << ": \"" << m.name << "\" " << m.terrainVariantName << " " << m.backgroundName << " " << m.width << "x" << m.height << std::endl;
    }

    std::cout << "\tFactions [" << Assets::factions.size() << "]:" << std::endl;
    for (const Assets::Faction& f : Assets::factions) {
        std::cout << "\t\t" << f.name << ": \"" << f.nameNice << "\" [" << f.characters.size() << "]:" << std::endl;
        for (const Assets::Character& c : f.characters)
            std::cout << "\t\t\t" << c.name << ": \"" << c.nameNice << "\" idle walk[" << c.march.size() << "] fire[" << c.fire.size() << "] death[" << c.death.size() << "] " << c.size.x << "x" << c.size.y <<
                " " << (c.fireSnd == Assets::missingSoundSound ? "(missing fire snd)" : "firesnd") << std::endl;
        std::cout << "\t\tMusic [" << f.gameplayMusic.size() + 1 << "]:" << std::endl;
        std::cout << "\t\t\tvictory" << std::endl;
        for (const Assets::MusicTrack& t : f.gameplayMusic)
            std::cout << "\t\t\t" << t.name << std::endl;
    }

    std::cout << "\tFonts [" << Assets::fonts.size() << "]:" << std::endl;
    for (const Assets::Font& f : Assets::fonts)
        std::cout << "\t\t" << f.name << ": " << f.size << std::endl;

    std::cout << "\tBackgrounds [" << Assets::backgrounds.size() << "]:" << std::endl;
    for (const Assets::Background& b : Assets::backgrounds)
        std::cout << "\t\t" << b.name << ": " << b.width << "x" << b.height << std::endl;
}

int main(int argc, const char **argv) {
    initSDL();

    loadAssets();
    printAssets();

    Assets::selectedFaction = Assets::factions.begin();
    Assets::selectedTerrainVariant = Assets::terrainVariants.begin();
    Assets::selectedMap = Assets::campaigns[0].maps.begin();

    gameSetup();

    renderSetup();
    renderLoop();
    
    destroySDL();
    return 0;
}
