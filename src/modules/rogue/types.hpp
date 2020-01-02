#pragma once

#include "../../pse.hpp"
#include "globals.hpp"

#include <vector>

namespace Modules {

struct Node {
    bool obstacle = false;
    bool visited = false;
    float global_goal;
    float local_goal;
    int x, y;
    std::vector<Node *> neighbors;
    Node *parent;

    static bool cmp(Node *a, Node *b) {
        return a->global_goal < b->global_goal;
    }
    static float dist(Node *a, Node *b) {
        return fast_sqrtf((a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y));
    }
};

// graph node
struct Room {
    bool is_connected = false;
    bool is_explored = false;
    bool is_gone = false;
    int index = 0;
    int neighbors[NEIGHBORS_MAX] = { 0 };
    int map_h, map_w;
    int map_i, map_j;
    int center_i, center_j;

    void insert_neighbor(int neighbor);
    // check if the given neighbor is already connected to
    bool check_neighbor(int neighbor);
    // range within the room, not including outer walls
    void rand_tile(int* i, int *j);
    void rand_corridor(int *i, int *j);
    void print();
};

struct Entity {
    int graph_x, graph_y;
    int map_x, map_y;
    int id = -1;
    bool is_enemy = false;

    bool check_tile(int offset_x, int offset_y);
    // move with bounds check
    void move(int direction);
};

struct Floor {
    Room Graph[GRAPH_SIZE][GRAPH_SIZE]; // graph nodes to generate a map from
    int Map[MAP_SIZE][MAP_SIZE]; // floor plan of every tile on that floor
    int Start_i, Start_j, End_i, End_j; // graph locations of starting (spawn) and ending (stair) rooms
    bool visited = false;
    Entity StairUp, StairDown;
};

}