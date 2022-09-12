#include "main.hpp"

#include <random>

namespace Game {
    std::vector<Soldier> friendlies;
    std::vector<Soldier> enemies;
    std::vector<MapPathPoint> mapPath;
    std::vector<Bullet> bullets;
}

constexpr float gravity = 200.0f;
constexpr float marchSpeed = 60.0f;
constexpr float muzzleVelocity = 300.0f;

constexpr float pistolRpm = 60.0f;
constexpr float rifleRpm = 15.0f;
constexpr float machinegunRpm = 550.0f;

constexpr int pistolDamage = 25;
constexpr int rifleDamage = 66;
constexpr int machinegunDamage = 40;

constexpr float pistolDistance = 10 * TILE_SIZE;
constexpr float rifleDistance = 20 * TILE_SIZE;
constexpr float machinegunDistance = 15 * TILE_SIZE;

int musicPlayingTrack = 0;
std::default_random_engine randgen;
std::normal_distribution<double> soldierGauss(1.0, 0.1);    // variation in soldier capabilities
std::normal_distribution<double> bulletGauss(0.0, 0.1);     // aim inaccuracy

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
    soldier.rand = soldierGauss(randgen);
    soldier.health = 100;

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
    if (soldier->state != Game::Soldier::FIRING) soldier->prevState = soldier->state;
    soldier->state = Game::Soldier::FIRING;
    soldier->frameCounter = 0;
    Mix_PlayChannel(-1, soldier->character->fireSnd, 0);
}

// manipulate bullets
void bulletSpawn(vector pos, vector vel, Game::Bullet::BulletType type, bool fromEnemy) {
    Game::Bullet bullet;
    bullet.pos = pos;
    bullet.vel = vel;
    bullet.type = type;
    bullet.fromEnemy = fromEnemy;
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

void gameSetup() {
    findMapPath();
}

void gameUpdate(float deltaTime) {
    // update bullets
    for (auto it = Game::bullets.begin(); it < Game::bullets.end(); it++) {
        Game::Bullet& bullet = *it;
        vector b1 = bullet.pos;
        bullet.pos += bullet.vel * deltaTime;
        vector b2 = bullet.pos;

        // if out of the map
        if (bullet.pos.x < 0.0f || bullet.pos.x > Game::mapPath[Game::mapPath.size() - 1].pos.x) { Game::bullets.erase(it); continue; }

        // map collision
        bool cont = false;
        for (int i = 0; i < Game::mapPath.size() - 2; i++) {
            if (doIntersect(b1, b2, Game::mapPath[i].pos, Game::mapPath[i + 1].pos)) {
                Game::bullets.erase(it); cont = true; break;
            }
        }
        if (cont) continue;

        // soldier collision, no friendly fire
        if (bullet.fromEnemy) {
            for (Game::Soldier& soldier : Game::friendlies)
                if ((bullet.pos.x > soldier.pos.x) && (bullet.pos.y > soldier.pos.y) && (bullet.pos.x < soldier.pos.x + soldier.character->size.x) && (bullet.pos.y < soldier.pos.y + soldier.character->size.y)) {
                    soldier.health -= pistolDamage;
                    Game::bullets.erase(it); cont = true; break;
                }
        } else {
            for (Game::Soldier& soldier : Game::enemies)
                if ((bullet.pos.x > soldier.pos.x) && (bullet.pos.y > soldier.pos.y) && (bullet.pos.x < soldier.pos.x + soldier.character->size.x) && (bullet.pos.y < soldier.pos.y + soldier.character->size.y)) {
                    soldier.health -= pistolDamage;
                    Game::bullets.erase(it); cont = true; break;
                }
        }
    }

    // update friendlies
    for (auto it = Game::friendlies.begin(); it < Game::friendlies.end(); it++) {
        Game::Soldier& soldier = *it;

        if (soldier.health <= 0) soldierDeath(it);

        if (soldier.state == Game::Soldier::DYING) {
            if (soldier.frameCounter >= soldier.character->death.size()) Game::friendlies.erase(it);
            continue;
        }

        // find nearest enemy
        std::vector<Game::Soldier>::iterator nearestEnemy = Game::enemies.end();
        for (auto it = Game::enemies.begin(); it < Game::enemies.end(); it++) {
            const Game::Soldier& enemy = *it;
            if (abs(enemy.pos.x - soldier.pos.x) < soldier.rand * pistolDistance) {
                if (nearestEnemy == Game::enemies.end()) { nearestEnemy = it; continue; }
                if (abs(enemy.pos.x - soldier.pos.x) < abs(nearestEnemy->pos.x - soldier.pos.x)) { nearestEnemy = it; continue; }
            }
        }

        bool mapcheck = true;
        if (nearestEnemy != Game::enemies.end()) {
            mapcheck = false;
            if (soldier.state != Game::Soldier::FIRING) {
                soldier.prevState = soldier.state;
                soldier.state = Game::Soldier::IDLE;
            }
            if (soldier.cooldownTime <= 0.0f) {
                soldierFire(it);
                vector muzzlePoint = {soldier.pos.x + (2.0f * soldier.character->size.x / 3.0f), soldier.pos.y + (soldier.character->size.x / 3.0f)};
                vector enemyCenter = (nearestEnemy->character->size / 2.0f) + nearestEnemy->pos;
                vector vel = (enemyCenter - muzzlePoint).unit() * muzzleVelocity;
                vector polarVel = vel.toPolar();
                polarVel.x += bulletGauss(randgen);
                vel = vectorFromPolar(polarVel);
                bulletSpawn(muzzlePoint, vel, Game::Bullet::PISTOL, false);
                soldier.cooldownTime = pistolRpm / 60.0f;
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
                    soldier.pos += (Game::mapPath[i].pos - center).unit() * (deltaTime * soldier.rand * marchSpeed);
                }
                break;
            }
        }
    }

    // update enemies
    for (auto it = Game::enemies.begin(); it < Game::enemies.end(); it++) {
        Game::Soldier& soldier = *it;

        if (soldier.health <= 0) soldierDeath(it);

        if (soldier.state == Game::Soldier::DYING) {
            if (soldier.frameCounter >= soldier.character->death.size()) Game::enemies.erase(it);
            continue;
        }

        // find nearest friendly
        std::vector<Game::Soldier>::iterator nearestFriendly = Game::friendlies.end();
        for (auto it = Game::friendlies.begin(); it < Game::friendlies.end(); it++) {
            const Game::Soldier& enemy = *it;
            if (abs(enemy.pos.x - soldier.pos.x) < soldier.rand * pistolDistance) {
                if (nearestFriendly == Game::friendlies.end()) { nearestFriendly = it; continue; }
                if (abs(enemy.pos.x - soldier.pos.x) < abs(nearestFriendly->pos.x - soldier.pos.x)) { nearestFriendly = it; continue; }
            }
        }

        bool mapcheck = true;
        if (nearestFriendly != Game::friendlies.end()) {
            mapcheck = false;
            if (soldier.state != Game::Soldier::FIRING && soldier.state != Game::Soldier::FIRING) {
                soldier.prevState = soldier.state;
                soldier.state = Game::Soldier::IDLE;
            }
            if (soldier.cooldownTime <= 0.0f) {
                soldierFire(it);
                vector muzzlePoint = {soldier.pos.x + (2.0f * soldier.character->size.x / 3.0f), soldier.pos.y + (soldier.character->size.x / 3.0f)};
                vector friendlyCenter = (nearestFriendly->character->size / 2.0f) + nearestFriendly->pos;
                vector vel = (friendlyCenter - muzzlePoint).unit() * muzzleVelocity;
                vector polarVel = vel.toPolar();
                polarVel.x += bulletGauss(randgen);
                vel = vectorFromPolar(polarVel);
                bulletSpawn(muzzlePoint, vel, Game::Bullet::PISTOL, true);
                soldier.cooldownTime = pistolRpm / 60.0f;
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
                    soldier.pos += (Game::mapPath[i].pos - center).unit() * (deltaTime * soldier.rand * marchSpeed);
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
