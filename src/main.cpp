#include "main.hpp"

#include <iostream>

void printAssets() {
    std::cout << "Terrain variants [" << Assets::terrainVariants.size() << "]:" << std::endl;
    for (Assets::TerrainVariant& tvar : Assets::terrainVariants) {
        std::cout << "\t" << tvar.name << " [" << tvar.terrainTextures.size() << "]:" << std::endl;
        for (Assets::Texture& tx : tvar.terrainTextures) {
            std::cout << "\t\t" << tx.name[0] << ": " << tx.name << std::endl;
        }
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
