#pragma once

#include "types.hpp"

namespace Modules {

void entity_insert(Entity *e);
void rand_room_tile(int gi, int gj, int *i, int *j); // get a random tile within the room at the grid coord
void rand_room_tile_no_overlap(int gi, int gj, int *i, int *j); // get a random tile within the room at the grid coord
bool empty_coords(int i, int j); // true if there is another entity on the given coordinates

void spawn_entities(); // spawn all entities onto the Map
void spawn_player(); // spawn player at center of Start_i/j
void spawn_stairs(); // spawn stairs at center of End_i/j
void spawn_enemies(); // at random locations

// A* https://www.youtube.com/watch?v=icZj67PTFhc
void astar_init();
void astar_reset();
void astar_solve(int start_i, int start_j, int end_i, int end_j);
void astar_walk(int *start_i, int *start_j, int end_i, int end_j);

void entity_move(int direction); // move player in direction and all other entities
void player_move(int direction);
void enemy_move();

}