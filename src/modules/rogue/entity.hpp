#pragma once

#include "types.hpp"

namespace Modules {

void entity_insert(Entity& e);
void spawn_entities(); // spawn all entities onto the Map
void spawn_player(); // spawn player at center of Start_i/j
void spawn_stairs(); // spawn stairs at center of End_i/j

// A* https://www.youtube.com/watch?v=icZj67PTFhc
void astar_init();
void astar_reset();
void astar_solve(int start_i, int start_j, int end_i, int end_j);
void astar_walk();

void player_move(int direction);

}