#pragma once

namespace Modules {

/******************************************************************************
 * Floor Generation
 * 
 * https://web.archive.org/web/20131025132021/http://kuoi.org/~kamikaze/GameDesign/art07_rogue_dungeon.php
 * 
 */

int map_to_graph_index(int index);
int graph_to_map_index(int index);
bool coords_equal(int i0, int j0, int i1, int j1);

void gen_graph(); // generate a set of rooms from globals, write data structure into the global 'Graph'
bool graph_has_unconnected_neighbors_at(int i, int j);  // return true if a room has at least 1 unconnected neighbor
bool room_try_insert(int *out_direction, int i, int j); // use try_insert fns to randomly connect a room
bool up_try_insert(int i, int j);    // try to make room connection up, return false on fail
bool right_try_insert(int i, int j); // try to make room connection right, return false on fail
bool down_try_insert(int i, int j);  // try to make room connection down, return false on fail
bool left_try_insert(int i, int j);  // try to make room connection left, return false on fail

void gen_map(); // generate the map from the graph of the floor
void gen_floor(); // generate entire floor from subroutines
void floor_switch(int direction); // switch to a different floor

}