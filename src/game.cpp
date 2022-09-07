#include "main.hpp"

namespace Game {
    std::vector<Soldier> soldiers;
}

constexpr float gravity = 200.0f;
constexpr float marchVelocity = 1.0f;

void gameUpdate(float deltaTime) {
    for (Game::Soldier& soldier : Game::soldiers) {
        soldier.vy += deltaTime * gravity;

        if (!((soldier.y < 0) || (soldier.y / TILE_SIZE >= selectedMap->height) || (soldier.x < 0) || (soldier.x / TILE_SIZE >= selectedMap->width)))
            if (selectedMap->map[(soldier.y + soldier.character->height) / TILE_SIZE][soldier.x / TILE_SIZE] != ' ')  // collision
                soldier.vy = 0;

        soldier.x += deltaTime * soldier.vx;
        soldier.y += deltaTime * soldier.vy;
    }
}
