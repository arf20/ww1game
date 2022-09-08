#include "main.hpp"

namespace Game {
    std::vector<Soldier> soldiers;
    std::vector<vector> mapPath;
}

constexpr float gravity = 200.0f;
constexpr float marchVelocity = 60.0f;

// manipulate soldiers
void soldierFire(const std::vector<Game::Soldier>::iterator& soldier) {
    soldier->prevState = soldier->state;
    soldier->state = SoldierState::FIRING;
    Mix_PlayChannel(-1, soldier->character->fireSnd, 0);
}

void spawnSoldier(const std::string& faction, const std::string& rank) {
    Game::Soldier soldier;
    soldier.enemy = false;
    soldier.character = getCharacterByNameAndFaction(rank, getFactionByName(faction));
    soldier.pos.y = Game::mapPath[0].y - soldier.character->size.y;
    soldier.pos.x = 10;
    soldier.vel = { 0.0f, 0.0f };
    soldier.state = SoldierState::MARCHING;
    soldier.frameCounter = 0;
    Game::soldiers.push_back(soldier);
}

void findMapPath() {
    int prevmy = 0;
    for (int mx = 0; mx < selectedMap->width; mx++) {
        int my = 0;
        while (selectedMap->map[my][mx] == ' ') { my++; }
        if (my < prevmy) {
            Game::mapPath.push_back({float(TILE_SIZE * mx), float(TILE_SIZE * (my + 1))});
            Game::mapPath.push_back({float(TILE_SIZE * (mx + 1)), float(TILE_SIZE * my)});
        } else
            Game::mapPath.push_back({float(TILE_SIZE * mx), float(TILE_SIZE * my)});
        prevmy = my;
    }
}

void gameSetup() {
    findMapPath();
}

void gameUpdate(float deltaTime) {
    for (Game::Soldier& soldier : Game::soldiers) {
        if (soldier.state == SoldierState::MARCHING) {
            for (int i = 0; i < Game::mapPath.size(); i++) {
                if (Game::mapPath[i].x > soldier.pos.x + soldier.character->size.x) {
                    soldier.pos += (Game::mapPath[i] - (soldier.pos + soldier.character->size)).unit() * (deltaTime * marchVelocity);
                    break;
                }
            }
        }
    }
}
