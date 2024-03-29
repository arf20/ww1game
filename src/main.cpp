/*
    ww1game:  Generic WW1 game (?)
    main.cpp: Entry point

    Copyright (C) 2022 Ángel Ruiz Fernandez

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifdef _WIN32
	#define SDL_MAIN_HANDLED    // ah yes, windows shenanigans
#endif

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
        std::cout << "\t\t" << f.name << ": \"" << f.nameNice << "\" " << std::string(f.flag != Assets::missingTextureTexture ? "flag " : " ") <<  "[" << f.characters.size() << "]:" << std::endl;
        for (const Assets::Character& c : f.characters)
            std::cout << "\t\t\t" << c.name << ": \"" << c.nameNice << "\" idle walk[" << c.march.size() << "] fire[" << c.fire.size() << "] death[" << c.death.size() << "] " << c.size.x << "x" << c.size.y <<
                " " << (c.fireSnd == Assets::missingSoundSound ? "(missing fire snd)" : "firesnd") << std::endl;
        std::cout << "\t\t\tMusic [" << f.gameplayMusic.size() + 1 << "]:" << std::endl;
        std::cout << "\t\t\t\tvictory" << std::endl;
        for (const Assets::MusicTrack& t : f.gameplayMusic)
            std::cout << "\t\t\t\t" << t.name << std::endl;
    }

    std::cout << "\tFonts [" << Assets::fonts.size() << "]:" << std::endl;
    for (const Assets::Font& f : Assets::fonts)
        std::cout << "\t\t" << f.name << ": " << f.size << std::endl;

    std::cout << "\tBackgrounds [" << Assets::backgrounds.size() << "]:" << std::endl;
    for (const Assets::Background& b : Assets::backgrounds)
        std::cout << "\t\t" << b.name << ": " << b.width << "x" << b.height << std::endl;
}

int main(int argc, const char **argv) {
    std::cout << "ww1game " ARFMINESWEEPER_VERSION "-" ARFMINESWEEPER_NUM_COMMIT " Copyright (C) 2024 Angel Ruiz Fernandez arf20 <arf20@arf20.com>" << std::endl <<
        "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>"  << std::endl <<
        "This is free software: you are free to change and redistribute it. "  << std::endl <<
        "This program comes with ABSOLUTELY NO WARRANTY."  << std::endl;

    Renderer::initSDL();

    for (std::string path : (std::vector<std::string>)ASSET_SEARCH_PATHS)
        Assets::load(path);
    printAssets();

    if (Assets::campaigns.size() == 0) exit_error("Error: No assets found.");

    Game::selectedCampaign = Assets::campaigns.end();

    Game::friendlyFaction = Assets::factions.end();
    Game::enemyFaction = Assets::factions.end();

    Renderer::setup();
    Renderer::loop();
    
    Renderer::destroySDL();

    return 0;
}
