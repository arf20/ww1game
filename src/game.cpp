#include "main.hpp"

namespace Game {
    std::vector<Soldier> soldiers;
}

constexpr float gravity = 200.0f;
constexpr float marchVelocity = 55.0f;

void soldiersFire(const std::vector<Game::Soldier>::iterator& soldier) {
    soldier->prevState = soldier->state;
    soldier->state = SoldierState::FIRING;
    Mix_PlayChannel(-1, soldier->character->fireSnd, 0);
}

void gameUpdate(float deltaTime) {
    for (Game::Soldier& soldier : Game::soldiers) {
        soldier.vy += deltaTime * gravity;

        float brcx = soldier.x + soldier.character->width;
        float brcy = soldier.y + soldier.character->height;

        if (!((brcy < 0) || (brcy / TILE_SIZE >= selectedMap->height) || (brcx < 0) || (brcx / TILE_SIZE >= selectedMap->width)))
            if (selectedMap->map[brcy / TILE_SIZE][brcx / TILE_SIZE] != ' ')  // collision with floor
                soldier.vy = 0;

        // if this point is inside a solid tile, climb uphill
        float uphill_cpy = brcy - 10;
        float uphill_cpx = brcx - 20;

        if (!((uphill_cpy < 0) || (uphill_cpy / TILE_SIZE >= selectedMap->height) || (uphill_cpx < 0) || (uphill_cpx / TILE_SIZE >= selectedMap->width)))
            if (selectedMap->map[uphill_cpy / TILE_SIZE][uphill_cpx / TILE_SIZE] != ' ')  // collision with uphill
                soldier.vy = -marchVelocity;

        if (soldier.state == SoldierState::MARCHING)
            soldier.vx = marchVelocity;
        else
            soldier.vx = 0;

        soldier.x += deltaTime * soldier.vx;
        soldier.y += deltaTime * soldier.vy;
    }
}
