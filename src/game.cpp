#include "main.hpp"

#include <random>

namespace Game {
    std::vector<Assets::TerrainVariant>::iterator selectedTerrainVariant;
    std::vector<Assets::Campaign>::iterator selectedCampaign;
    std::vector<Assets::Map>::iterator selectedMap;

    std::vector<Assets::Faction>::iterator friendlyFaction, enemyFaction;

    std::vector<Soldier> friendlies, enemies;
    std::vector<MapPathPoint> mapPath;
    std::vector<Bullet> bullets;
}

constexpr float gravity = 200.0f;
constexpr float marchSpeed = 60.0f;
constexpr float muzzleVelocity = 300.0f;

int musicPlayingTrack = 0;
std::default_random_engine randgen;
std::normal_distribution<double> soldierGauss(1.0, 0.1);    // variation in soldier capabilities
std::normal_distribution<double> bulletGauss(0.0, 1.0);     // aim inaccuracy

// manipulate soldiers
void soldierSpawn(const std::vector<Assets::Character>::iterator& character, bool enemy) {
    Game::Soldier soldier;
    soldier.character = character;
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
    soldier.rand = soldierGauss(randgen);
    soldier.health = soldier.character->iHealth;

    if (enemy)
        Game::enemies.push_back(soldier);
    else
        Game::friendlies.push_back(soldier);
}

void soldierDeath(const std::vector<Game::Soldier>::iterator& soldier) {
    if (soldier->state == Game::Soldier::DYING) return;
    soldier->state = Game::Soldier::DYING;
    soldier->frameCounter = 0;
}

void soldierFire(const std::vector<Game::Soldier>::iterator& soldier) {
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
        point.type = Game::MapPathPoint::GROUND;

        if (my < prevmy) {
            point.pos = {float(TILE_SIZE * mx), float(TILE_SIZE * (my + 1))}; Game::mapPath.push_back(point);
            point.pos = {float(TILE_SIZE * (mx + 1)), float(TILE_SIZE * my)}; Game::mapPath.push_back(point);
        } else {
            point.pos = {float(TILE_SIZE * mx), float(TILE_SIZE * my)}; Game::mapPath.push_back(point);
        }

        if (Game::selectedMap->map[my][mx] == 't') {
            point.type = Game::MapPathPoint::TRENCH;
            point.pos = {float((TILE_SIZE * mx) + (TILE_SIZE / 2)), float(TILE_SIZE * (my + 1))}; Game::mapPath.push_back(point);
        }
        prevmy = my;
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
    for (int i = 0; i < Game::mapPath.size() - 2; i++) {
        if (doIntersect(a, b, Game::mapPath[i].pos, Game::mapPath[i + 1].pos)) {
            return true;
        }
    }
    return false;
}

// ============== game itself ==============
void mapSetup() {
    findMapPath();
    Game::selectedTerrainVariant = getTerrainVariantByName(Game::selectedMap->terrainVariantName);
}

void updateBullets(float deltaTime) {
    for (auto it = Game::bullets.begin(); it < Game::bullets.end(); it++) {
        Game::Bullet& bullet = *it;
        vector b1 = bullet.pos;     // get travel segment
        bullet.pos += bullet.vel * deltaTime;
        vector b2 = bullet.pos;

        // if out of the map
        if (bullet.pos.x < 0.0f || bullet.pos.x > Game::mapPath[Game::mapPath.size() - 1].pos.x) { Game::bullets.erase(it); continue; }

        // map collision, intersect travel segment with all map segments
        if (intersectsMap(b1, b2)) {
            Game::bullets.erase(it);
            continue;
        }

        // soldier collision, no friendly fire
        if (bullet.fromEnemy) {
            for (Game::Soldier& soldier : Game::friendlies)
                if ((bullet.pos.x > soldier.pos.x) && (bullet.pos.y > soldier.pos.y) && (bullet.pos.x < soldier.pos.x + soldier.character->size.x) && (bullet.pos.y < soldier.pos.y + soldier.character->size.y)) {
                    soldier.health -= bullet.damage;
                    Game::bullets.erase(it); break;
                }
        } else {
            for (Game::Soldier& soldier : Game::enemies)
                if ((bullet.pos.x > soldier.pos.x) && (bullet.pos.y > soldier.pos.y) && (bullet.pos.x < soldier.pos.x + soldier.character->size.x) && (bullet.pos.y < soldier.pos.y + soldier.character->size.y)) {
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

// targetEnemies relatively to 'soldiers'
void updateFaction(std::vector<Game::Soldier>& soldiers, const std::vector<Game::Soldier>& targetEnemies, bool dir, float deltaTime) {
    for (auto it = soldiers.begin(); it < soldiers.end(); it++) {
        Game::Soldier& soldier = *it;

        // soldier die when health runs out
        if (soldier.health <= 0) soldierDeath(it);

        if (soldier.state == Game::Soldier::DYING) {
            if (soldier.frameCounter >= soldier.character->death.size()) soldiers.erase(it);
            continue;
        }

        auto nearestTarget = findNearestTarget(soldier, targetEnemies);

        bool mapcheck = true;
        if (nearestTarget != targetEnemies.end()) {
            vector muzzlePoint = {soldier.pos.x + (2.0f * soldier.character->size.x / 3.0f), soldier.pos.y + (soldier.character->size.x / 3.0f)};
            vector targetPoint = (nearestTarget->character->size / 2.0f) + nearestTarget->pos;

            if (intersectsMap(muzzlePoint, targetPoint)) goto mapcalc;

            mapcheck = false;
            if (soldier.state != Game::Soldier::FIRING) {
                soldier.prevState = soldier.state;
                soldier.state = Game::Soldier::IDLE;
            }
            if (soldier.cooldownTime <= 0.0f) {
                soldierFire(it);
                if (soldier.frameCounter == soldier.character->fireFrame) {
                    vector vel = (targetPoint - muzzlePoint).unit() * soldier.character->muzzleVel;
                    vector polarVel = vel.toPolar();
                    polarVel.x += soldier.character->spread * bulletGauss(randgen);
                    vel = vectorFromPolar(polarVel);
                    bulletSpawn(muzzlePoint, vel, soldier.character->roundDamage, dir);
                    soldier.cooldownTime = soldier.character->rpm / 60.0f;
                    Mix_PlayChannel(-1, soldier.character->fireSnd, 0);
                }
            }
        }
        soldier.cooldownTime -= deltaTime;

        mapcalc:
        if (!dir) {   // advance direction: false = right, true = left
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
                        soldier.pos += (Game::mapPath[i].pos - center).unit() * (deltaTime * soldier.rand * soldier.character->marchSpeed);
                    }
                    break;
                }
            }
        }
        else {
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
                        soldier.pos += (Game::mapPath[i].pos - center).unit() * (deltaTime * soldier.rand * soldier.character->marchSpeed);
                    }

                    break;
                }
            }
        }
    }
}

void gameUpdate(float deltaTime) {
    updateBullets(deltaTime);

    updateFaction(Game::friendlies, Game::enemies, false, deltaTime);
    updateFaction(Game::enemies, Game::friendlies, true, deltaTime);
    
    
    if (Mix_PlayingMusic() == 0) {
        if (musicPlayingTrack >= Game::friendlyFaction->gameplayMusic.size()) musicPlayingTrack = 0;
        if (Mix_PlayMusic(Game::friendlyFaction->gameplayMusic[musicPlayingTrack].track, 0) < 0) {
            error_sdl("Error playing music");
        }
        musicPlayingTrack++;
    }
}
