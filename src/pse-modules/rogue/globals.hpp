#pragma once

#include <deque>

namespace Modules {

constexpr int GRAPH_SIZE = 4;
constexpr int MAP_SIZE = 60;    // ~10x GRAPH_SIZE is good

// the number of tiles square of each room
constexpr int ROOM_WIDTH = MAP_SIZE / GRAPH_SIZE - 1;
constexpr int ROOM_TOLERANCE = ROOM_WIDTH / 2;
constexpr int ROOM_CONNECT_TRIES = 7;
#define ROOM_PATH_MODIFIER 2 / 3 /* INTENDS TO HAVE OPERATOR PRECEDENCE MAKE LHS RVALUE GREATER THAN RHS */
constexpr float ROOM_GONE_CHANCE = 0.05f;

constexpr int TILE_SCALING = 95; // tile size modifier on SDL window
constexpr int TILE_WIDTH = TILE_SCALING / 7;

constexpr int ENTITY_MAX = 40; // maximum number of entities
constexpr int FLOORS_MAX = 20; // maximum number of floor
constexpr int NEIGHBORS_MAX = 4; // Don't touchs

constexpr int ENEMY_MAX = 10;
constexpr int ENEMY_MIN = 5;

enum MapTile {
    WALL = '#',
    FLOOR = '.',
    EMPTY = ' ',
};

enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT,
};

enum EntityId {
    ID_INVALID = -1,
    ID_PLAYER,
    ID_ENEMY,
    ID_STAIR_DOWN,
    ID_STAIR_UP,
};

struct Node;
struct Room;
struct Entity;
struct Floor;

extern pse::Context *PSE_Context;

extern Entity Player;
extern bool PlayerCanMove;
extern Entity *Entities[ENTITY_MAX];
extern int EntityIndex;

// A* util
extern Node Nodes[MAP_SIZE][MAP_SIZE];
extern std::deque<Node *> UntestedNodes;

extern Floor Dungeon[FLOORS_MAX];
extern int FloorLevel;
extern int LastStairDirection;

#define FLR Dungeon[FloorLevel]

extern int SpritePlayerId;
extern int SpriteEnemyId;
extern int SpriteFloorLightId;
extern int SpriteFloorDarkId;
extern int SpriteWallId;
extern int SpriteStairUpId;
extern int SpriteStairDownId;

}