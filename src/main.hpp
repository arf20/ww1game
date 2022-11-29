#pragma once

// == Global includes
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <iostream>
#include <vector>
#include <string>

// == Macros
#define ASSET_PATH  "../assets"

#define TILE_SIZE   32

// == Types
struct vector {
    float x, y;

    float mod() {
        return std::hypotf(x, y);
    }

    vector unit() {
        return { x / mod(), y / mod() };
    }

    vector operator-(const vector& right) {
        return { x - right.x, y - right.y };
    }

    vector operator+(const vector& right) {
        return { x + right.x, y + right.y };
    }

    void operator+=(const vector& right) {
        x += right.x; y += right.y;
    }

    vector operator*(const float& right) {
        return { x * right, y * right };
    }

    vector operator/(const float& right) {
        return { x / right, y / right };
    }

    vector toPolar() {
        vector v;
        v.x = atan2(y, x);
        v.y = mod();
        return v;
    }
};

inline vector vectorFromPolar(const vector& v) {
    vector t;
    t.x = cos(v.x) * v.y;
    t.y = sin(v.x) * v.y;
    return t;
}

namespace Assets {
    struct Tile {
        std::string name;
        int width, height;
        SDL_Texture *texture;
    };

    struct TerrainVariant {
        std::string name;
        std::vector<Tile> terrainTextures;
    };

    struct Map {
        int id;
        std::string name;
        std::string terrainVariantName;
        std::string backgroundName;
        std::string friendlyFactionName;
        std::string enemyFactionName;
        int width, height;
        std::vector<std::string> map;
    };

    struct Campaign {
        std::string name;
        std::string nameNice;
        std::vector<Map> maps;
    };

    struct Character {
        std::string name;
        std::string nameNice;
        vector size;
        SDL_Texture *idle;
        std::vector<SDL_Texture*> march;
        std::vector<SDL_Texture*> fire;
        std::vector<SDL_Texture*> death;
        Mix_Chunk *fireSnd;
        int fireFrame;

        float marchSpeed;
        float range;
        float rpm;
        float muzzleVel;
        float spread;
        int roundDamage;
        int iHealth;
    };

    struct MusicTrack {
        std::string name;
        Mix_Music *track;
        float duration;
    };

    struct Faction {
        std::string name;
        std::string nameNice;
        MusicTrack victoryMusic;
        std::vector<MusicTrack> gameplayMusic;
        std::vector<Character> characters;
    };

    struct Font {
        std::string name;
        int size;
        TTF_Font* font12;
        TTF_Font* font20;
    };

    struct Background {
        std::string name;
        int width, height;
        SDL_Color skyColor;
        SDL_Texture *texture;
    };
}

namespace Game {
    struct Soldier {
        vector pos;
        vector vel;     // to be used in the future for implementing explosions
        float rand;     // a gaussian random number associated with the soldier
        bool friendly;  // false = enemy
        std::vector<Assets::Character>::iterator character;
        enum SoldierState { IDLE, MARCHING, FIRING, DYING } prevState, state;  // 0 idle, 1 running, 2 firing, 3 dying
        int frameCounter;
        float cooldownTime;
        int health;
    };

    struct MapPathPoint {
        enum PointType { GROUND, TRENCH } type;
        vector pos;
    };

    struct Bullet {
        vector pos;
        vector vel;
        int damage;
        bool fromEnemy;
    };
}

// == Global vars
// owned by main
extern bool debug;

// owned by loader
namespace Assets {
    extern std::vector<TerrainVariant> terrainVariants;
    extern std::vector<Campaign> campaigns;
    extern std::vector<Faction> factions;
    extern std::vector<Font> fonts;
    extern std::vector<Background> backgrounds;
    extern SDL_Texture *bulletTexture;

    extern SDL_Texture *missingTextureTexture;
    extern Mix_Chunk *missingSoundSound;
    extern Mix_Music *missingMusicMusic;

    extern std::vector<Assets::Font>::iterator defaultFont;

    extern MusicTrack menuMusic;
}

// owned by game
namespace Game {
    extern std::vector<Assets::Faction>::iterator friendlyFaction, enemyFaction;
    extern std::vector<Assets::TerrainVariant>::iterator selectedTerrainVariant;
    extern std::vector<Assets::Campaign>::iterator selectedCampaign;
    extern std::vector<Assets::Map>::iterator selectedMap;

    extern std::vector<Soldier> friendlies;
    extern std::vector<Soldier> enemies;
    extern std::vector<MapPathPoint> mapPath;
    extern std::vector<Bullet> bullets;

    extern bool gameMode;
    extern int money;

    extern int friendlyCasualties;
    extern int enemyCasualties;
}

// owned by renderer
extern SDL_Window *window;
extern SDL_Renderer *renderer;

// == Global functions
// Renderer
void initSDL();
void destroySDL();
void renderSetup();
void renderLoop();

// Loader
void loadAssets();

// Game
void soldierSpawn(const std::vector<Assets::Character>::iterator& character, bool enemy);
void soldierDeath(const std::vector<Game::Soldier>::iterator& soldier);
void soldierFire(const std::vector<Game::Soldier>::iterator& soldier);
void mapSetup();
void gameUpdate(float deltaTime);

// Inline util
inline void exit_error(const std::string& msg) {
    std::cout << msg << std::endl;
    exit(1);
}

inline void exit_error_sdl(const std::string& msg) {
    std::cout << msg << ": " << SDL_GetError() << std::endl;
    exit(1);
}

inline void error_sdl(const std::string& msg) {
    std::cout << msg << ": " << SDL_GetError() << std::endl;
}

inline void exit_error_img(const std::string& msg) {
    std::cout << msg << ": " << IMG_GetError() << std::endl;
    exit(1);
}

inline void error_img(const std::string& msg) {
    std::cout << msg << ": " << IMG_GetError() << std::endl;
}

inline std::vector<Assets::Faction>::iterator getFactionByName(std::string name) {
    for (auto it = Assets::factions.begin(); it < Assets::factions.end(); it++)
        if (it->name == name) return it;
    return Assets::factions.end();
}

inline std::vector<Assets::Character>::iterator getCharacterByNameAndFaction(std::string name, std::vector<Assets::Faction>::iterator faction) {
    for (auto it = faction->characters.begin(); it < faction->characters.end(); it++)
        if (it->name == name) return it;
    return faction->characters.end();
}

inline auto getTerrainVariantByName(std::string name) {
    for (auto it = Assets::terrainVariants.begin(); it < Assets::terrainVariants.end(); it++)
        if (it->name == name) return it;
    return Assets::terrainVariants.end();
}
