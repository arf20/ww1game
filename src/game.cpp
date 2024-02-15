/*
    ww1game:  Generic WW1 game (?)
    game.cpp: Game mechanics and logic

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

#include "main.hpp"

#include <random>
#include <algorithm>

namespace Game {
    std::vector<Assets::TerrainVariant>::iterator selectedTerrainVariant;
    std::vector<Assets::Campaign>::iterator selectedCampaign;
    std::vector<Assets::Map>::iterator selectedMap;

    std::vector<Assets::Faction>::iterator friendlyFaction, enemyFaction;

    std::vector<Soldier> friendlies, enemies;
    std::vector<MapPathPoint> friendlyMapPath, enemyMapPath;
    std::vector<MapPathPoint>::iterator friendlyObjective, enemyObjective;
    
    std::vector<Bullet> bullets;

    bool gameMode = true;                                       // 1 = sandbox, 0 = against AI
    int money = 0;

    int friendlyCasualties = 0;
    int enemyCasualties = 0;

    int friendliesHoldingbjective = 0;
    int enemiesHoldingObjective = 0;
}

constexpr float gravity = 200.0f;

int musicPlayingTrack = 0;

std::default_random_engine randgen;
std::normal_distribution<double> soldierGauss(1.0, 0.1);    // variation in soldier capabilities
std::normal_distribution<double> bulletGauss(0.0, 1.0);     // aim inaccuracy

// manipulate soldiers
void Game::soldierSpawn(const std::vector<Assets::Character>::iterator& character, bool enemy) {
    Game::Soldier soldier;
    soldier.character = character;
    if (enemy) {
        // enemy spawn point
        soldier.pos.y = Game::enemyMapPath[Game::enemyMapPath.size() - 1].pos.y - soldier.character->size.y;
        soldier.pos.x = Game::enemyMapPath[Game::enemyMapPath.size() - 1].pos.x - (soldier.character->size.x / 2.0f) - 1.0f;
        soldier.friendly = false;
    } else {
        // friendly spawn point
        soldier.pos.y = Game::friendlyMapPath[0].pos.y - soldier.character->size.y;
        soldier.pos.x = Game::friendlyMapPath[0].pos.x - (soldier.character->size.x / 2.0f) + 1.0f;
        soldier.friendly = true;
    }
    soldier.vel = { 0.0f, 0.0f };
    soldier.state = Game::Soldier::MARCHING;
    soldier.frameCounter = 0;
    soldier.cooldownTime = 0.0f;
    soldier.rand = soldierGauss(randgen);
    soldier.health = soldier.character->iHealth;

    if (enemy)
        Game::enemies.push_back(soldier);
    else
        Game::friendlies.push_back(soldier);
}

void Game::soldierDeath(const std::vector<Game::Soldier>::iterator& soldier) {
    if (soldier->state == Game::Soldier::DYING) return;
    soldier->state = Game::Soldier::DYING;
    soldier->frameCounter = 0;

    // enemy or friendly... improve this
    if (soldier->friendly) Game::friendlyCasualties++;
    else Game::enemyCasualties++;
}

void Game::soldierFire(const std::vector<Game::Soldier>::iterator& soldier) {
    if (soldier->state == Game::Soldier::DYING) return;
    if (soldier->state == Game::Soldier::FIRING) return;
    soldier->prevState = soldier->state;
    soldier->state = Game::Soldier::FIRING;
    soldier->frameCounter = 0;
}

// manipulate bullets
void bulletSpawn(vector pos, vector vel, int damage, bool fromEnemy) {
    Game::Bullet bullet;
    bullet.pos = pos;
    bullet.vel = vel;
    bullet.damage = damage;
    bullet.fromEnemy = fromEnemy;
    Game::bullets.push_back(bullet);
}

// build a vector of points from map
void findMapPath() {
    int prevmy = 0;
    for (int mx = 0; mx < Game::selectedMap->width; mx++) {
        int my = 0;
        while (Game::selectedMap->map[my][mx] == ' ') { my++; }

        Game::MapPathPoint point;
        point.type = Game::MapPathPoint::GROUND;    // when GROUND, action is ignored
        point.action = Game::MapPathPoint::MARCH;

        if (my < prevmy) {
            point.pos = {float(TILE_SIZE * mx), float(TILE_SIZE * (my + 1))};
            Game::friendlyMapPath.push_back(point);
            point.pos = {float(TILE_SIZE * (mx + 1)), float(TILE_SIZE * my)};
            Game::friendlyMapPath.push_back(point);
        } else {
            point.pos = {float(TILE_SIZE * mx), float(TILE_SIZE * my)};
            Game::friendlyMapPath.push_back(point);
        }

        if (Game::selectedMap->map[my][mx] == 't') {
            point.type = Game::MapPathPoint::TRENCH;
            point.action = Game::MapPathPoint::HOLD;
            point.pos = {float((TILE_SIZE * mx) + (TILE_SIZE / 2)), float(TILE_SIZE * (my + 1))};
            Game::friendlyMapPath.push_back(point);
        }
        prevmy = my;
    }

    // append two points further in the edges
    Game::MapPathPoint point;
    point.type = Game::MapPathPoint::GROUND;
    point.action = Game::MapPathPoint::MARCH;
    point.pos = { Game::friendlyMapPath[0].pos.x - 40.0f, Game::friendlyMapPath[0].pos.y};
    Game::friendlyMapPath.insert(Game::friendlyMapPath.begin(), point);

    point.pos = { Game::friendlyMapPath[Game::friendlyMapPath.size() - 1].pos.x + 80.0f, Game::friendlyMapPath[0].pos.y};
    Game::friendlyMapPath.push_back(point);

    // copy friendly path to enemy path, they are the same
    Game::enemyMapPath = Game::friendlyMapPath;

    // get objectives
    for (int i = Game::friendlyMapPath.size() - 1; i >= 0; i--) {
        if (Game::friendlyMapPath[i].type == Game::MapPathPoint::TRENCH) {
            Game::friendlyObjective = Game::friendlyMapPath.begin() + i;
            break;
        }
    }

    for (int i = 0; i < Game::enemyMapPath.size(); i++) {
        if (Game::enemyMapPath[i].type == Game::MapPathPoint::TRENCH) {
            Game::enemyObjective = Game::enemyMapPath.begin() + i;
            break;
        }
    }
}

// line-line intersection alg
bool onSegment(vector p, vector q, vector r) {
    if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
        q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y))
       return true;
    return false;
}

int orientation(vector p, vector q, vector r) {
    int val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);
    if (val == 0) return 0;
    return (val > 0)? 1: 2;
}

bool doIntersect(vector p1, vector q1, vector p2, vector q2) {
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);
  
    if (o1 != o2 && o3 != o4)
        return true;
  
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;
  
    return false;
}

bool intersectsMap(const vector& a, const vector& b) {
    for (int i = 0; i < Game::friendlyMapPath.size() - 2; i++) {
        if (doIntersect(a, b, Game::friendlyMapPath[i].pos, Game::friendlyMapPath[i + 1].pos)) {
            return true;
        }
    }
    return false;
}

// ============== game itself ==============
void Game::mapSetup() {
    findMapPath();
    Game::selectedTerrainVariant = getTerrainVariantByName(Game::selectedMap->terrainVariantName);
    Game::friendlyFaction = getFactionByName(Game::selectedMap->friendlyFactionName);
    Game::enemyFaction = getFactionByName(Game::selectedMap->enemyFactionName);
}

void updateBullets(float deltaTime) {
    for (auto it = Game::bullets.begin(); it < Game::bullets.end(); it++) {
        Game::Bullet& bullet = *it;
        vector b1 = bullet.pos;     // get travel segment
        bullet.pos += bullet.vel * deltaTime;
        vector b2 = bullet.pos;

        // if out of the map
        if (bullet.pos.x < 0.0f || bullet.pos.x > Game::friendlyMapPath[Game::friendlyMapPath.size() - 1].pos.x) { Game::bullets.erase(it); continue; }

        // map collision, intersect travel segment with all map segments
        if (intersectsMap(b1, b2)) {
            Game::bullets.erase(it);
            continue;
        }

        // soldier collision, no friendly fire
        if (bullet.fromEnemy) {
            for (Game::Soldier& soldier : Game::friendlies)
                if ((soldier.state != Game::Soldier::DYING) && (bullet.pos.x > soldier.pos.x) && (bullet.pos.y > soldier.pos.y) && (bullet.pos.x < soldier.pos.x + soldier.character->size.x) && (bullet.pos.y < soldier.pos.y + soldier.character->size.y)) {
                    soldier.health -= bullet.damage;
                    Game::bullets.erase(it); break;
                }
        } else {
            for (Game::Soldier& soldier : Game::enemies)
                if ((soldier.state != Game::Soldier::DYING) && (bullet.pos.x > soldier.pos.x) && (bullet.pos.y > soldier.pos.y) && (bullet.pos.x < soldier.pos.x + soldier.character->size.x) && (bullet.pos.y < soldier.pos.y + soldier.character->size.y)) {
                    soldier.health -= bullet.damage;
                    Game::bullets.erase(it); break;
                }
        }
    }
}

auto findNearestTarget(const Game::Soldier& soldier, const std::vector<Game::Soldier>& targetEnemies) {
    auto nearestEnemy = targetEnemies.end();
    for (auto it = targetEnemies.begin(); it < targetEnemies.end(); it++) {
        const Game::Soldier& enemy = *it;
        if (abs(enemy.pos.x - soldier.pos.x) < soldier.rand * soldier.character->range * TILE_SIZE) {
            if (nearestEnemy == targetEnemies.end()) { nearestEnemy = it; continue; }
            if (abs(enemy.pos.x - soldier.pos.x) < abs(nearestEnemy->pos.x - soldier.pos.x)) { nearestEnemy = it; continue; }
        }
    }
    return nearestEnemy;
}

void resetTrenches(std::vector<Game::Soldier>& soldiers) {
    // if soldiers is empty we can't check the side of the soldiers themselves
    if (soldiers.size() < 1) {
        if (soldiers.data() == Game::friendlies.data()) {
            for (int i = 0; i < Game::friendlyMapPath.size(); i++)
                if (Game::friendlyMapPath[i].type == Game::MapPathPoint::TRENCH)
                    if (Game::friendlyMapPath[i].action != Game::MapPathPoint::HOLD)
                            Game::friendlyMapPath[i].action = Game::MapPathPoint::HOLD;
        }
        else {
            for (int i = 0; i < Game::enemyMapPath.size(); i++)
                if (Game::enemyMapPath[i].type == Game::MapPathPoint::TRENCH)
                    if (Game::enemyMapPath[i].action != Game::MapPathPoint::HOLD)
                            Game::enemyMapPath[i].action = Game::MapPathPoint::HOLD;
        }
        return;
    }

    // find leftmost and rightmost soldiers
    float minx = Game::selectedMap->width * TILE_SIZE;
    float maxx = 0.0f;
    std::vector<Game::Soldier>::iterator leftmost = soldiers.end();
    std::vector<Game::Soldier>::iterator rightmost = soldiers.end();
    for (int i = 0; i < soldiers.size(); i++) {
        if (soldiers[i].pos.x < minx) { minx = soldiers[i].pos.x; leftmost = soldiers.begin() + i; }
        if (soldiers[i].pos.x > maxx) { maxx = soldiers[i].pos.x; rightmost = soldiers.begin() + i; }
    }

    // if there are trenches on clear more advanced than the most advanced soldier, reset it
    if (soldiers[0].friendly) {
        if (rightmost != soldiers.end())
            for (int i = 0; i < Game::friendlyMapPath.size(); i++)
                if (Game::friendlyMapPath[i].type == Game::MapPathPoint::TRENCH)
                    if (Game::friendlyMapPath[i].action != Game::MapPathPoint::HOLD)
                        if (Game::friendlyMapPath[i].pos.x > rightmost->pos.x + (rightmost->character->size.x / 2.0f))
                            Game::friendlyMapPath[i].action = Game::MapPathPoint::HOLD;
    }
    else {
        if (leftmost != soldiers.end())
            for (int i = 0; i < Game::enemyMapPath.size(); i++)
                if (Game::enemyMapPath[i].type == Game::MapPathPoint::TRENCH)
                    if (Game::enemyMapPath[i].action != Game::MapPathPoint::HOLD)
                        if (Game::enemyMapPath[i].pos.x < leftmost->pos.x + (leftmost->character->size.x / 2.0f))
                            Game::enemyMapPath[i].action = Game::MapPathPoint::HOLD;
    }
}

// targetEnemies relative to 'soldiers'
void updateFaction(std::vector<Game::Soldier>& soldiers, const std::vector<Game::Soldier>& targetEnemies, float deltaTime) {
    int fho = 0, eho = 0;

    for (auto it = soldiers.begin(); it < soldiers.end(); it++) {
        Game::Soldier& soldier = *it;

        // death logic
        if (soldier.health <= 0)
            Game::soldierDeath(it);

        if (soldier.state == Game::Soldier::DYING) {
            if (soldier.frameCounter >= soldier.character->death.size()) soldiers.erase(it);
            continue;
        }

        // firing logic
        auto nearestTarget = findNearestTarget(soldier, targetEnemies);

        bool mapcheck = true;
        if (nearestTarget != targetEnemies.end()) {
            vector muzzlePoint = {soldier.pos.x + (3.0f * soldier.character->size.x / 4.0f), soldier.pos.y + (soldier.character->size.x / 3.0f)};
            vector targetPointBody = (nearestTarget->character->size / 2.0f) + nearestTarget->pos;
            vector targetPointHead = nearestTarget->pos; targetPointHead.y += nearestTarget->character->size.y / 4.0f;

            bool aimToHead = false;
            if (intersectsMap(muzzlePoint, targetPointBody)) {
                aimToHead = true;
                if (intersectsMap(muzzlePoint, targetPointHead))
                    goto mapcalc;
            }

            mapcheck = false;
            if (soldier.state != Game::Soldier::FIRING) {
                soldier.prevState = soldier.state;
                soldier.state = Game::Soldier::IDLE;
            }
            if (soldier.cooldownTime <= 0.0f) {
                soldierFire(it);
                if (soldier.frameCounter == soldier.character->fireFrame) {
                    vector vel = ((aimToHead ? targetPointHead : targetPointBody) - muzzlePoint).unit() * soldier.character->muzzleVel;
                    vector polarVel = vel.toPolar();
                    polarVel.x += soldier.character->spread * bulletGauss(randgen);
                    vel = vectorFromPolar(polarVel);
                    bulletSpawn(muzzlePoint, vel, soldier.character->roundDamage, !soldier.friendly);
                    soldier.cooldownTime = soldier.character->rpm / 60.0f;
                    Mix_PlayChannel(-1, soldier.character->fireSnd, 0);
                }
            }
        }
        soldier.cooldownTime -= deltaTime;

        // movement logic
        mapcalc:
        if (soldier.friendly) {   // friendly
            for (int i = 1; i < Game::friendlyMapPath.size(); i++) {
                if (Game::friendlyMapPath[i].pos.x > soldier.pos.x + (soldier.character->size.x / 2.0f)) {
                    if (mapcheck)
                        if (Game::friendlyMapPath[i - 1].action == Game::MapPathPoint::MARCH) {
                            soldier.prevState = soldier.state;
                            soldier.state = Game::Soldier::MARCHING;
                        } else {
                            soldier.prevState = soldier.state;
                            soldier.state = Game::Soldier::IDLE;
                        }
                    if (soldier.state == Game::Soldier::SoldierState::MARCHING) {
                        vector center = soldier.pos;
                        center.x += soldier.character->size.x / 2.0f; center.y += soldier.character->size.y;
                        soldier.pos += (Game::friendlyMapPath[i].pos - center).unit() * (deltaTime * soldier.rand * soldier.character->marchSpeed);
                    }
                    break;
                }
            }
        }
        else {
            for (int i = Game::enemyMapPath.size() - 2; i >= 0; i--) {
                if (Game::enemyMapPath[i].pos.x < soldier.pos.x + (soldier.character->size.x / 2.0f)) {
                    if (mapcheck)
                        if (Game::enemyMapPath[i + 1].action == Game::MapPathPoint::MARCH) {
                            soldier.prevState = soldier.state;
                            soldier.state = Game::Soldier::MARCHING;
                        } else {
                            soldier.prevState = soldier.state;
                            soldier.state = Game::Soldier::IDLE;
                        }
                    if (soldier.state == Game::Soldier::SoldierState::MARCHING) {
                        vector center = soldier.pos;
                        center.x += soldier.character->size.x / 2.0f; center.y += soldier.character->size.y;
                        soldier.pos += (Game::enemyMapPath[i].pos - center).unit() * (deltaTime * soldier.rand * soldier.character->marchSpeed);
                    }

                    break;
                }
            }
        }

        if (soldier.friendly) {
            if (abs((soldier.pos.x + (soldier.character->size.x / 2.0f)) - Game::friendlyObjective->pos.x) <= float(TILE_SIZE))
                fho++;
        }
        else {
            if (abs((soldier.pos.x + (soldier.character->size.x / 2.0f)) - Game::enemyObjective->pos.x) <= float(TILE_SIZE))
                eho++;
        }
    }

    if (soldiers.size() > 0)
        if (soldiers[0].friendly)
            Game::friendliesHoldingbjective = fho;
        else
            Game::enemiesHoldingObjective = eho;

    resetTrenches(soldiers);
}

void Game::update(float deltaTime) {
    updateBullets(deltaTime);

    updateFaction(Game::friendlies, Game::enemies, deltaTime);
    updateFaction(Game::enemies, Game::friendlies, deltaTime);
    
    if (Mix_PlayingMusic() == 0) {
        if (musicPlayingTrack >= Game::friendlyFaction->gameplayMusic.size()) musicPlayingTrack = 0;
        if (Mix_PlayMusic(Game::friendlyFaction->gameplayMusic[musicPlayingTrack].track, 0) < 0) {
            error_sdl("Error playing music");
        }
        musicPlayingTrack++;
    }
}
