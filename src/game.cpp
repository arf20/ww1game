#include "main.hpp"

namespace Game {
    std::vector<Soldier> soldiers;
    std::vector<MapPathPoint> mapPath;
}

constexpr float gravity = 200.0f;
constexpr float marchVelocity = 60.0f;

int musicPlayingTrack = 0;

// manipulate soldiers
void soldierSpawn(const std::string& faction, const std::string& rank) {
    Game::Soldier soldier;
    soldier.enemy = false;
    soldier.character = getCharacterByNameAndFaction(rank, getFactionByName(faction));
    soldier.pos.y = Game::mapPath[0].pos.y - soldier.character->size.y;
    soldier.pos.x = 10;
    soldier.vel = { 0.0f, 0.0f };
    soldier.state = Game::Soldier::MARCHING;
    soldier.frameCounter = 0;
    Game::soldiers.push_back(soldier);
}

void soldierDeath(const std::vector<Game::Soldier>::iterator& soldier) {
    soldier->state = Game::Soldier::DYING;
    soldier->frameCounter = 0;
}

void soldierFire(const std::vector<Game::Soldier>::iterator& soldier) {
    if (soldier->state == Game::Soldier::DYING) return;
    if (soldier->state != Game::Soldier::FIRING) soldier->prevState = soldier->state;
    soldier->state = Game::Soldier::FIRING;
    soldier->frameCounter = 0;
    Mix_PlayChannel(-1, soldier->character->fireSnd, 0);
}

void findMapPath() {
    int prevmy = 0;
    for (int mx = 0; mx < Assets::selectedMap->width; mx++) {
        int my = 0;
        while (Assets::selectedMap->map[my][mx] == ' ') { my++; }

        Game::MapPathPoint point;
        point.type = Game::MapPathPoint::GROUND;

        if (my < prevmy) {
            point.pos = {float(TILE_SIZE * mx), float(TILE_SIZE * (my + 1))}; Game::mapPath.push_back(point);
            point.pos = {float(TILE_SIZE * (mx + 1)), float(TILE_SIZE * my)}; Game::mapPath.push_back(point);
        } else {
            point.pos = {float(TILE_SIZE * mx), float(TILE_SIZE * my)}; Game::mapPath.push_back(point);
        }

        if (Assets::selectedMap->map[my][mx] == 't') {
            point.type = Game::MapPathPoint::TRENCH;
            point.pos = {float((TILE_SIZE * mx) + (TILE_SIZE / 2)), float(TILE_SIZE * (my + 1))}; Game::mapPath.push_back(point);
        }
        prevmy = my;
    }
}

void gameSetup() {
    findMapPath();
}

void gameUpdate(float deltaTime) {
    // update soldiers
    for (Game::Soldier& soldier : Game::soldiers) {
        if (soldier.state == Game::Soldier::DYING) continue;
        for (int i = 1; i < Game::mapPath.size(); i++) {
            if (Game::mapPath[i].pos.x > soldier.pos.x + (soldier.character->size.x / 2.0f)) {
                if (soldier.state != Game::Soldier::FIRING)
                    if (Game::mapPath[i - 1].type == Game::MapPathPoint::GROUND) {
                        soldier.prevState = soldier.state;
                        soldier.state = Game::Soldier::MARCHING;
                    } else {
                        soldier.prevState = soldier.state;
                        soldier.state = Game::Soldier::IDLE;
                    }

                if (soldier.state == Game::Soldier::SoldierState::MARCHING) {
                    vector center = soldier.pos;
                    center.x += soldier.character->size.x / 2.0f; center.y += soldier.character->size.y;
                    soldier.pos += (Game::mapPath[i].pos - center).unit() * (deltaTime * marchVelocity);
                }

                break;
            }
        }
    }

    if (Mix_PlayingMusic() == 0) {
        if (musicPlayingTrack >= Assets::selectedFaction->gameplayMusic.size()) musicPlayingTrack = 0;
        if (Mix_PlayMusic(Assets::selectedFaction->gameplayMusic[musicPlayingTrack].track, 0) < 0) {
            error_sdl("Error playing music");
        }
        musicPlayingTrack++;
    }
}
