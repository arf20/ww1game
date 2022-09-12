#include "main.hpp"

#include <random>

namespace Game {
    std::vector<Soldier> friendlies;
    std::vector<Soldier> enemies;
    std::vector<MapPathPoint> mapPath;
    std::vector<Bullet> bullets;
}

constexpr float gravity = 200.0f;
constexpr float marchVelocity = 60.0f;
constexpr float muzzleVelocity = 300.0f;

constexpr float pistolRpm = 60.0f;
constexpr float rifleRpm = 15.0f;
constexpr float machinegunRpm = 550.0f;

int musicPlayingTrack = 0;
std::default_random_engine randgen;
std::normal_distribution<double> gauss(0.0, 0.1);

// manipulate soldiers
void soldierSpawn(const std::string& faction, const std::string& rank, bool enemy) {
    Game::Soldier soldier;
    soldier.character = getCharacterByNameAndFaction(rank, getFactionByName(faction));
    if (enemy) {
        // enemy spawn point
        soldier.pos.y = Game::mapPath[Game::mapPath.size() - 1].pos.y - soldier.character->size.y;
        soldier.pos.x = Game::mapPath[Game::mapPath.size() - 1].pos.x - 10;
    } else {
        // friendly spawn point
        soldier.pos.y = Game::mapPath[0].pos.y - soldier.character->size.y;
        soldier.pos.x = 10;
    }
    soldier.vel = { 0.0f, 0.0f };
    soldier.state = Game::Soldier::MARCHING;
    soldier.frameCounter = 0;
    soldier.cooldownTime = 0.0f;

    if (enemy)
        Game::enemies.push_back(soldier);
    else
        Game::friendlies.push_back(soldier);
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

// manipulate bullets
void bulletSpawn(vector pos, vector vel) {
    Game::Bullet bullet;
    bullet.pos = pos;
    bullet.vel = vel;
    Game::bullets.push_back(bullet);
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
    for (auto it = Game::bullets.begin(); it < Game::bullets.end(); it++) {
        Game::Bullet& bullet = *it;
        bullet.pos += bullet.vel * deltaTime;
    }

    // update friendlies
    for (auto it = Game::friendlies.begin(); it < Game::friendlies.end(); it++) {
        Game::Soldier& soldier = *it;
        if (soldier.state == Game::Soldier::DYING) {
            if (soldier.frameCounter >= soldier.character->death.size()) Game::friendlies.erase(it);
            continue;
        }

        bool mapcheck = true;
        for (const Game::Soldier enemy : Game::enemies) {
            if (enemy.pos.x - soldier.pos.x < 15 * TILE_SIZE) {
                mapcheck = false;
                if (soldier.state != Game::Soldier::FIRING && soldier.state != Game::Soldier::FIRING) {
                    soldier.prevState = soldier.state;
                    soldier.state = Game::Soldier::IDLE;
                }
                if (soldier.cooldownTime <= 0.0f) {
                    soldierFire(it);
                    vector muzzlePoint = {soldier.pos.x + (2.0f * soldier.character->size.x / 3.0f), soldier.pos.y + (soldier.character->size.x / 3.0f)};
                    vector enemyCenter = (enemy.character->size / 2.0f) + enemy.pos;
                    vector vel = (enemyCenter - muzzlePoint).unit() * muzzleVelocity;
                    vector polarVel = vel.toPolar();
                    polarVel.x += gauss(randgen);
                    vel = vectorFromPolar(polarVel);
                    bulletSpawn(muzzlePoint, vel);
                    soldier.cooldownTime = pistolRpm / 60.0f;
                }
                break;
            }
        }
        soldier.cooldownTime -= deltaTime;

        for (int i = 1; i < Game::mapPath.size(); i++) {
            if (Game::mapPath[i].pos.x > soldier.pos.x + (soldier.character->size.x / 2.0f)) {
                if (mapcheck)
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

    // update enemies
    for (auto it = Game::enemies.begin(); it < Game::enemies.end(); it++) {
        Game::Soldier& soldier = *it;
        if (soldier.state == Game::Soldier::DYING) {
            if (soldier.frameCounter >= soldier.character->death.size()) Game::enemies.erase(it);
            continue;
        }

        bool mapcheck = true;
        for (const Game::Soldier friendly : Game::friendlies) {
            if (soldier.pos.x - friendly.pos.x < 15 * TILE_SIZE) {
                mapcheck = false;
                if (soldier.state != Game::Soldier::FIRING && soldier.state != Game::Soldier::FIRING) {
                    soldier.prevState = soldier.state;
                    soldier.state = Game::Soldier::IDLE;
                }
                if (soldier.cooldownTime <= 0.0f) {
                    soldierFire(it);
                    vector muzzlePoint = {soldier.pos.x + (2.0f * soldier.character->size.x / 3.0f), soldier.pos.y + (soldier.character->size.x / 3.0f)};
                    vector friendlyCenter = (friendly.character->size / 2.0f) + friendly.pos;
                    vector vel = (friendlyCenter - muzzlePoint).unit() * muzzleVelocity;
                    vector polarVel = vel.toPolar();
                    polarVel.x += gauss(randgen);
                    vel = vectorFromPolar(polarVel);
                    bulletSpawn(muzzlePoint, vel);
                    soldier.cooldownTime = pistolRpm / 60.0f;
                }
                break;
            }
        }
        soldier.cooldownTime -= deltaTime;

        for (int i = Game::mapPath.size() - 2; i >= 0; i--) {
            if (Game::mapPath[i].pos.x < soldier.pos.x + (soldier.character->size.x / 2.0f)) {
                if (mapcheck)
                    if (Game::mapPath[i + 1].type == Game::MapPathPoint::GROUND) {
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
