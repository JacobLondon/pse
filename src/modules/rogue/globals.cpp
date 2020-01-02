#include "../../pse.hpp"
#include "globals.hpp"
#include "types.hpp"

#include <deque>

namespace Modules {

pse::Context *PSE_Context;

Entity Player{};
bool PlayerCanMove = true;
Entity *Entities[ENTITY_MAX];
int EntityIndex = 0;

// A* util
Node Nodes[MAP_SIZE][MAP_SIZE];
std::deque<Node *> UntestedNodes;

Floor Dungeon[FLOORS_MAX];
int FloorLevel = 0;
int LastStairDirection = UP;

int SpritePlayerId = 0;
int SpriteEnemyId = 0;
int SpriteFloorLightId = 0;
int SpriteFloorDarkId = 0;
int SpriteWallId = 0;
int SpriteStairUpId = 0;
int SpriteStairDownId = 0;

}