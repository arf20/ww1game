#include "main.hpp"

#include <iostream>

int main(int argc, const char **argv) {
    initSDL();

    loadTerrain();
    std::cout << terrainTextures.size() << std::endl;

    renderLoop();
    destroySDL();
    return 0;
}