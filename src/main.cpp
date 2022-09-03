#include "main.hpp"

#include <iostream>

void printAssets() {
    std::cout << "Assets:" << std::endl;
    std::cout << "\tTerrain variants [" << Assets::terrainVariants.size() << "]:" << std::endl;
    for (const Assets::TerrainVariant& tvar : Assets::terrainVariants) {
        std::cout << "\t\t" << tvar.name << " [" << tvar.terrainTextures.size() << "]:" << std::endl;
        for (const Assets::Texture& tx : tvar.terrainTextures)
            std::cout << "\t\t\t" << tx.name[0] << ": " << tx.name << " " << tx.width << "x" << tx.height << std::endl;
    }

    std::cout << "\tCampaigns [" << Assets::campaigns.size() << "]:" << std::endl;
    for (const Assets::Campaign& c : Assets::campaigns) {
        std::cout << "\t\t" << c.name << ": \"" << c.nameNice << "\" [" << c.maps.size() << "]:" << std::endl;
        for (const Assets::Map& m : c.maps)
            std::cout << "\t\t\t" << m.id << ": \"" << m.name << "\" " << m.terrainVariantName << " " << m.map[0].length() << "x" << m.map.size() << std::endl;
    }
}

int main(int argc, const char **argv) {
    initSDL();

    std::cout << "Loading assets into VRAM..." << std::endl;
    loadAssets();
    printAssets();

    renderLoop();
    
    destroySDL();
    return 0;
}
