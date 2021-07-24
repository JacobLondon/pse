#include <cstdio>

#include "../../pse.hpp"
#include "entity.hpp"
#include "gen.hpp"
#include "globals.hpp"
#include "types.hpp"

namespace Modules {

int map_to_graph_index(int index)
{
    return index / ROOM_WIDTH;
}

int graph_to_map_index(int index)
{
    return index * ROOM_WIDTH + ROOM_WIDTH / 2;
}

bool coords_equal(int i0, int j0, int i1, int j1)
{
    return i0 == i1 && j0 == j1;
}

bool up_try_insert(int i, int j)
{
    if (i - 1 >= 0 && FLR.Graph[i - 1][j].index < NEIGHBORS_MAX - 1) {
        FLR.Graph[i][j].insert_neighbor(UP);
        FLR.Graph[i - 1][j].insert_neighbor(DOWN);
        return true;
    }
    return false;
}

bool right_try_insert(int i, int j)
{
    if (j + 1 < GRAPH_SIZE && FLR.Graph[i][j + 1].index < NEIGHBORS_MAX - 1) {
        FLR.Graph[i][j].insert_neighbor(RIGHT);
        FLR.Graph[i][j + 1].insert_neighbor(LEFT);
        return true;
    }
    return false;
}

bool down_try_insert(int i, int j)
{
    if (i + 1 < GRAPH_SIZE && FLR.Graph[i + 1][j].index < NEIGHBORS_MAX - 1) {
        FLR.Graph[i][j].insert_neighbor(DOWN);
        FLR.Graph[i + 1][j].insert_neighbor(UP);
        return true;
    }
    return false;
}

bool left_try_insert(int i, int j)
{
    if (j - 1 >= 0 && FLR.Graph[i][j - 1].index < NEIGHBORS_MAX - 1) {
        FLR.Graph[i][j].insert_neighbor(LEFT);
        FLR.Graph[i][j - 1].insert_neighbor(RIGHT);
        return true;
    }
    return false;
}

bool room_try_insert(int *out_direction, int i, int j)
{
    // randomly select a direction that was not yet chosen
    do {
        *out_direction = rand_range(0, NEIGHBORS_MAX);
    } while (FLR.Graph[i][j].check_neighbor(*out_direction));

    // attempt to connect to the direction
    switch (*out_direction) {
        case UP:    return up_try_insert(i, j);
        case RIGHT: return right_try_insert(i, j);
        case DOWN:  return down_try_insert(i, j);
        case LEFT:  return left_try_insert(i, j);
        default:
            fprintf(stderr, "Error: Invalid room choice: %d\n", *out_direction);
            exit(-1);
    }

    // out variable direction can be recorded
};

bool graph_has_unconnected_neighbors_at(int i, int j)
{
    bool connected = false;

    // bounds check before checking neighbors
    if (i - 1 >= 0)         connected = connected || !FLR.Graph[i - 1][j].is_connected;
    if (i + 1 < GRAPH_SIZE) connected = connected || !FLR.Graph[i + 1][j].is_connected;
    if (j - 1 >= 0)         connected = connected || !FLR.Graph[i][j - 1].is_connected;
    if (j + 1 < GRAPH_SIZE) connected = connected || !FLR.Graph[i][j + 1].is_connected;
    
    return connected;
}

void gen_graph()
{
    // clear global Graph
    for (int i = 0; i < GRAPH_SIZE; ++i) {
        for (int j = 0; j < GRAPH_SIZE; ++j) {
            FLR.Graph[i][j] = Room{};
        }
    }

    // pick a random room to start with (for random walk and player spawn)
    int curr_i = rand_range(0, GRAPH_SIZE);
    int curr_j = rand_range(0, GRAPH_SIZE);
    FLR.Graph[curr_i][curr_j].is_connected = true;
    FLR.Start_i = curr_i; FLR.Start_j = curr_j;

    // connect unconnected neighbors, change the state of direction
    int direction; // direction is modified within room_try_insert

    // random walk across the graph until blocked or finished (no backtracking)
    while (graph_has_unconnected_neighbors_at(curr_i, curr_j)) {
        if (room_try_insert(&direction, curr_i, curr_j)) {
            switch (direction) {
                case UP:    curr_i -= 1; break;
                case RIGHT: curr_j += 1; break;
                case DOWN:  curr_i += 1; break;
                case LEFT:  curr_j -= 1; break;
                default:
                    fprintf(stderr, "Error: Invalid room direction: %d\n", direction);
                    exit(-1);
            }
        }
    }
    // record stair room
    FLR.End_i = curr_i; FLR.End_j = curr_j;

    // connect any still unconnected rooms with at least 2 neighbors, prevent dead room connections
    for (int i = 0; i < GRAPH_SIZE; ++i) {
        for (int j = 0; j < GRAPH_SIZE; ++j) {
            if (!FLR.Graph[i][j].is_connected) {
                int tries = 0;
                while (FLR.Graph[i][j].index < 2) {
                    // direction still passed, but not needed
                    room_try_insert(&direction, i, j);
                    if (tries++ > ROOM_CONNECT_TRIES)
                        break;
                }
            }
        }
    }
}

void gen_map()
{
    // create map of just walls
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            FLR.Map[i][j] = WALL;
        }
    }

    // draw rooms and their doors for each node in the graph into the Map
    for (int i = 0; i < GRAPH_SIZE; ++i) {
        for (int j = 0; j < GRAPH_SIZE; ++j) {
            // ignore empty nodes
            if (FLR.Graph[i][j].index == 0)
                continue;

            // random room width and height relative to the map
            FLR.Graph[i][j].map_w = rand_range(ROOM_TOLERANCE, ROOM_WIDTH);
            FLR.Graph[i][j].map_h = rand_range(ROOM_TOLERANCE, ROOM_WIDTH);
            FLR.Graph[i][j].map_i = i * ROOM_WIDTH;
            FLR.Graph[i][j].map_j = j * ROOM_WIDTH;
            FLR.Graph[i][j].center_i = i * ROOM_WIDTH + ROOM_WIDTH / 2;
            FLR.Graph[i][j].center_j = j * ROOM_WIDTH + ROOM_WIDTH / 2;

            // temp alias
            int& room_w = FLR.Graph[i][j].map_w;
            int& room_h = FLR.Graph[i][j].map_h;
            int& room_i = FLR.Graph[i][j].map_i;
            int& room_j = FLR.Graph[i][j].map_j;
            int& center_i = FLR.Graph[i][j].center_i;
            int& center_j = FLR.Graph[i][j].center_j;

            // skip filling room area if a gone room is to be used
            if (rand_uniform() < ROOM_GONE_CHANCE) {
                FLR.Graph[i][j].is_gone = true;
                goto create_corridor;
            }

            // fill room area with floor
            for (int ri = room_i + room_h / ROOM_TOLERANCE; ri < room_i + room_h; ++ri) {
                for (int rj = room_j + room_w / ROOM_TOLERANCE; rj < room_j + room_w; ++rj) {
                    FLR.Map[ri][rj] = FLOOR;
                }
            }

create_corridor:
            // walk towards door in each direction from room center if it has a door that way
            if (FLR.Graph[i][j].check_neighbor(DOWN)) {
                for (int ci = center_i; ci < i * ROOM_WIDTH + ROOM_WIDTH * ROOM_PATH_MODIFIER; ++ci)
                    FLR.Map[ci][center_j] = FLOOR;
            }
            if (FLR.Graph[i][j].check_neighbor(UP)) {
                for (int ci = center_i; ci > i * ROOM_WIDTH - ROOM_WIDTH * ROOM_PATH_MODIFIER; --ci)
                    FLR.Map[ci][center_j] = FLOOR;
            }
            if (FLR.Graph[i][j].check_neighbor(RIGHT))
                for (int cj = center_j; cj < j * ROOM_WIDTH + ROOM_WIDTH * ROOM_PATH_MODIFIER; ++cj)
                    FLR.Map[center_i][cj] = FLOOR;
            if (FLR.Graph[i][j].check_neighbor(LEFT))
                for (int cj = center_j; cj > j * ROOM_WIDTH - ROOM_WIDTH * ROOM_PATH_MODIFIER; --cj)
                    FLR.Map[center_i][cj] = FLOOR;
        }
    }

    // draw surrounding border wall around entire map
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            if (i == 0 || i == MAP_SIZE - 1 || j == 0 || j == MAP_SIZE - 1)
                FLR.Map[i][j] = WALL;
        }
    }
}

void gen_floor()
{
    gen_graph();
    gen_map();
    spawn_entities();
}

void floor_switch(int direction)
{
    switch (direction) {
    case UP:
        if (FloorLevel - 1 >= 0) {
            FloorLevel--;
            LastStairDirection = DOWN;
        }
        else {
            printf("No rooms above...\n");
            return;
        }
        break;
    case DOWN:
        if (FloorLevel + 1 < FLOORS_MAX) {
            FloorLevel++;
            LastStairDirection = UP;
            if (!FLR.visited)
                gen_floor();
        }
        else
            printf("You've explored all rooms!\n");
        break;
    default:
        fprintf(stderr, "Error: Invalid floor switch %d\n", direction);
        exit(-1);
    }
    spawn_entities();

    printf("Floor: %d\n", FloorLevel);
}

}
