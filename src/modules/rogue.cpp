/* 
 * TODO
 * 
 * Search file for misc TODOs...
 * 
 * Short Term:
 * Fn move_action: allow for different speed entities to move during the same turn
 * 
 * Long Term:
 *  Items
 *  Enemies
 *  Attacking
 *  SC Key option menu
 * 
 */

#include "../modules.hpp"

#include <cstdio>

namespace Modules {

/**
 * Constants
 */

constexpr unsigned MAX_NEIGHBORS = 4; // Don't touch
constexpr int GRAPH_SIZE = 5;
constexpr int MAP_SIZE = 60;    // ~10x MAP_GRID is good

// the number of tiles square of each room
constexpr int ROOM_WIDTH = MAP_SIZE / GRAPH_SIZE - 1;
constexpr int ROOM_TOLERANCE = ROOM_WIDTH / 2;
constexpr int ROOM_CONNECT_TRIES = 4;
#define ROOM_PATH_MODIFIER 2 / 3 /* INTENDS TO HAVE OPERATOR PRECEDENCE MAKE LHS RVALUE GREATER THAN RHS */
constexpr float ROOM_GONE_CHANCE = 0.05;

constexpr int TILE_SCALING = 60; // tile size modifier on SDL window
constexpr int TILE_WIDTH = TILE_SCALING / 7;

constexpr int ENTITY_MAX = 40; // maximum number of entities

/**
 * Definitions
 */

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
    ID_STAIR_DOWN,
    ID_STAIR_UP,
};

// graph node
struct Room {
    bool is_connected = false;
    bool is_explored = false;
    int index = 0;
    int neighbors[MAX_NEIGHBORS] = { 0 };

    void insert_neighbor(int neighbor) {
        neighbors[index++] = neighbor;
        is_connected = true;
    }
    // check if the given neighbor is already connected to
    bool check_neighbor(int neighbor) {
        for (int i = 0; i < index; ++i) {
            if (neighbors[i] == neighbor)
                return true;
        }
        return false;
    }
    void print() {
        for (int i = 0; i < index; ++i) {
            switch (neighbors[i]) {
                case UP:    printf("Up ");    break;
                case RIGHT: printf("Right "); break;
                case DOWN:  printf("Down ");  break;
                case LEFT:  printf("Left ");  break;
            }
        }
    }
};

int map_to_graph_index(int index);
int graph_to_map_index(int index);

Room Graph[GRAPH_SIZE][GRAPH_SIZE]; // graph nodes to generate a map from
int Map[MAP_SIZE][MAP_SIZE]; // floor plan of every tile on that floor
int Start_i, Start_j, End_i, End_j; // graph locations of starting (spawn) and ending (stair) rooms

struct Entity {
    int graph_x, graph_y;
    int map_x, map_y;
    int id = -1;
    int index;

    bool check_tile(int offset_x, int offset_y) {
        switch (Map[map_y + offset_y][map_x + offset_x]) {
            case FLOOR:
                return true;
            default:
                return false;
        }
    }
    // move with bounds check
    void move(int direction) {
        switch (direction) {
            case UP:    if (check_tile(0, -1)) map_y -= 1; break;
            case RIGHT: if (check_tile(1, 0))  map_x += 1; break;
            case DOWN:  if (check_tile(0, 1))  map_y += 1; break;
            case LEFT:  if (check_tile(-1, 0)) map_x -= 1; break;
            default:
                break;
            }
        graph_x = map_to_graph_index(map_x);
        graph_y = map_to_graph_index(map_y);
    }
};

/**
 * Globals
 */

pse::Context *PSE_Context;

Entity Player{};
bool PlayerCanMove = true;
Entity StairUp{}, StairDown{};
Entity *Entities[ENTITY_MAX];
int EntityIndex = 0;

/**
 * Debug
 */

void debug_print_map();
void debug_print_player();

void debug_print_map()
{
    for (int i = 0; i < MAP_SIZE; ++i) {
        printf("\n");
        for (int j = 0; j < MAP_SIZE; ++j) {
            printf("%c", (char)Map[i][j]);
        }
    }
    printf("\n");
}

void debug_print_player()
{
    printf("(%d, %d)\n", Player.graph_x, Player.graph_y);
}

/**
 * Util
 */

int map_to_graph_index(int index)
{
    return index / ROOM_WIDTH;
}

int graph_to_map_index(int index)
{
    return index * ROOM_WIDTH + ROOM_WIDTH / 2;
}

/**
 * Spawn entities
 */

void entity_insert(Entity& e);
void spawn_entities(); // spawn all entities onto the Map
void spawn_player(); // spawn player at center of Start_i/j
void spawn_stairs(); // spawn stairs at center of End_i/j

void entity_insert(Entity& e)
{
    if (EntityIndex + 1 < ENTITY_MAX) {
        e.index = EntityIndex;
        Entities[EntityIndex++] = &e;
    }
}

void spawn_entities()
{
    // stop looking at each entity
    for (int i = 0; i < ENTITY_MAX; ++i) {
        Entities[i] = nullptr;
    }
    // reset
    EntityIndex = 0;

    // "permanent" entities
    spawn_player();
    spawn_stairs();
}

void spawn_player()
{
    // spawn player at center of start_i/j room
    Player.map_y = graph_to_map_index(Start_i);
    Player.map_x = graph_to_map_index(Start_j);
    Player.graph_y = Start_i;
    Player.graph_x = Start_j;
    Player.id = ID_PLAYER;
    Graph[Player.graph_y][Player.graph_x].is_explored = true;

    entity_insert(Player);
}

void spawn_stairs()
{
    StairDown.graph_x = End_j;
    StairDown.graph_y = End_i;
    StairDown.map_x = graph_to_map_index(End_j);
    StairDown.map_y = graph_to_map_index(End_i);
    StairDown.id = ID_STAIR_DOWN;

    StairUp.graph_x = Start_j;
    StairUp.graph_y = Start_i;
    StairUp.map_x = graph_to_map_index(Start_j);
    StairUp.map_y = graph_to_map_index(Start_i);
    StairUp.id = ID_STAIR_UP;

    entity_insert(StairDown);
    entity_insert(StairUp);
}

/**
 * Player movement
 */

void player_move(int direction)
{
    Player.move(direction);
    Graph[Player.graph_y][Player.graph_x].is_explored = true;
}

/******************************************************************************
 * Floor Generation
 * 
 * https://web.archive.org/web/20131025132021/http://kuoi.org/~kamikaze/GameDesign/art07_rogue_dungeon.php
 * 
 */

void gen_graph(); // generate a set of rooms from globals, write data structure into the global 'Graph'
bool graph_has_unconnected_neighbors_at(int i, int j);  // return true if a room has at least 1 unconnected neighbor
bool room_try_insert(int *out_direction, int i, int j); // use try_insert fns to randomly connect a room
bool up_try_insert(int i, int j);    // try to make room connection up, return false on fail
bool right_try_insert(int i, int j); // try to make room connection right, return false on fail
bool down_try_insert(int i, int j);  // try to make room connection down, return false on fail
bool left_try_insert(int i, int j);  // try to make room connection left, return false on fail

void gen_map(); // generate the map from the graph of the floor
void gen_floor(); // generate entire floor from subroutines

bool up_try_insert(int i, int j)
{
    if (i - 1 >= 0 && Graph[i - 1][j].index < MAX_NEIGHBORS - 1) {
        Graph[i][j].insert_neighbor(UP);
        Graph[i - 1][j].insert_neighbor(DOWN);
        return true;
    }
    return false;
}

bool right_try_insert(int i, int j)
{
    if (j + 1 < GRAPH_SIZE && Graph[i][j + 1].index < MAX_NEIGHBORS - 1) {
        Graph[i][j].insert_neighbor(RIGHT);
        Graph[i][j + 1].insert_neighbor(LEFT);
        return true;
    }
    return false;
}

bool down_try_insert(int i, int j)
{
    if (i + 1 < GRAPH_SIZE && Graph[i + 1][j].index < MAX_NEIGHBORS - 1) {
        Graph[i][j].insert_neighbor(DOWN);
        Graph[i + 1][j].insert_neighbor(UP);
        return true;
    }
    return false;
}

bool left_try_insert(int i, int j)
{
    if (j - 1 >= 0 && Graph[i][j - 1].index < MAX_NEIGHBORS - 1) {
        Graph[i][j].insert_neighbor(LEFT);
        Graph[i][j - 1].insert_neighbor(RIGHT);
        return true;
    }
    return false;
}

bool room_try_insert(int *out_direction, int i, int j)
{
    // randomly select a direction that was not yet chosen
    do {
        *out_direction = rand_range(0, MAX_NEIGHBORS);
    } while (Graph[i][j].check_neighbor(*out_direction));

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
    if (i - 1 >= 0)         connected = connected || !Graph[i - 1][j].is_connected;
    if (i + 1 < GRAPH_SIZE) connected = connected || !Graph[i + 1][j].is_connected;
    if (j - 1 >= 0)         connected = connected || !Graph[i][j - 1].is_connected;
    if (j + 1 < GRAPH_SIZE) connected = connected || !Graph[i][j + 1].is_connected;
    
    return connected;
}

void gen_graph()
{
    // clear global Graph
    for (int i = 0; i < GRAPH_SIZE; ++i) {
        for (int j = 0; j < GRAPH_SIZE; ++j) {
            Graph[i][j] = Room{};
        }
    }

    // pick a random room to start with (for random walk and player spawn)
    int curr_i = rand_range(0, GRAPH_SIZE);
    int curr_j = rand_range(0, GRAPH_SIZE);
    Graph[curr_i][curr_j].is_connected = true;
    Start_i = curr_i; Start_j = curr_j;

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
    End_i = curr_i; End_j = curr_j;

    // connect any still unconnected rooms with at least 2 neighbors, prevent dead room connections
    for (int i = 0; i < GRAPH_SIZE; ++i) {
        for (int j = 0; j < GRAPH_SIZE; ++j) {
            if (!Graph[i][j].is_connected) {
                int tries = 0;
                while (Graph[i][j].index < 2) {
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
            Map[i][j] = WALL;
        }
    }

    // draw rooms and their doors for each node in the graph into the Map
    for (int i = 0; i < GRAPH_SIZE; ++i) {
        for (int j = 0; j < GRAPH_SIZE; ++j) {
            // ignore empty nodes
            if (Graph[i][j].index == 0)
                continue;

            // random room width and height relative to the map
            int room_w = rand_range(ROOM_TOLERANCE, ROOM_WIDTH);
            int room_h = rand_range(ROOM_TOLERANCE, ROOM_WIDTH);
            int room_i = i * ROOM_WIDTH;
            int room_j = j * ROOM_WIDTH;
            int center_i = i * ROOM_WIDTH + ROOM_WIDTH / 2;
            int center_j = j * ROOM_WIDTH + ROOM_WIDTH / 2;

            // skip filling room area if a gone room is to be used
            if (rand_uniform() < ROOM_GONE_CHANCE)
                goto create_corridor;

            // fill room area with floor
            for (int ri = room_i + room_h / ROOM_TOLERANCE; ri < room_i + room_h; ++ri) {
                for (int rj = room_j + room_w / ROOM_TOLERANCE; rj < room_j + room_w; ++rj) {
                    Map[ri][rj] = FLOOR;
                }
            }

create_corridor:
            // walk towards door in each direction from room center if it has a door that way
            if (Graph[i][j].check_neighbor(DOWN)) {
                for (int ci = center_i; ci < i * ROOM_WIDTH + ROOM_WIDTH * ROOM_PATH_MODIFIER; ++ci)
                    Map[ci][center_j] = FLOOR;
            }
            if (Graph[i][j].check_neighbor(UP)) {
                for (int ci = center_i; ci > i * ROOM_WIDTH - ROOM_WIDTH * ROOM_PATH_MODIFIER; --ci)
                    Map[ci][center_j] = FLOOR;
            }
            if (Graph[i][j].check_neighbor(RIGHT))
                for (int cj = center_j; cj < j * ROOM_WIDTH + ROOM_WIDTH * ROOM_PATH_MODIFIER; ++cj)
                    Map[center_i][cj] = FLOOR;
            if (Graph[i][j].check_neighbor(LEFT))
                for (int cj = center_j; cj > j * ROOM_WIDTH - ROOM_WIDTH * ROOM_PATH_MODIFIER; --cj)
                    Map[center_i][cj] = FLOOR;
        }
    }

    // draw surrounding border wall around entire map
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            if (i == 0 || i == MAP_SIZE - 1 || j == 0 || j == MAP_SIZE - 1)
                Map[i][j] = WALL;
        }
    }
}

void gen_floor()
{
    gen_graph();
    gen_map();
    spawn_entities();
}

/******************************************************************************
 * Drawing
 * 
 */

void draw_graph();
void draw_graph_room(int i, int j);
void draw_graph_doors(int i, int j);
void draw_map();
void draw_entities();

void draw_graph()
{
    // draw rooms
    for (int i = 0; i < GRAPH_SIZE; ++i) {
        for (int j = 0; j < GRAPH_SIZE; ++j) {
            draw_graph_room(i, j);
            // can't put draw_corridors here??????????????
            //draw_corridors(i, j);
        }
    }
    // why can't this go above????
    for (int i = 0; i < GRAPH_SIZE; ++i) {
        for (int j = 0; j < GRAPH_SIZE; ++j) {
            draw_graph_doors(i, j);
        }
    }
}

void draw_graph_room(int i, int j)
{
    SDL_Color c;
    if (i == Start_i && j == Start_j)
        c = pse::Sky;
    else if (i == End_i && j == End_j)
        c = pse::Orange;
    else if (Graph[i][j].index == 0)
        c = pse::Red;
    else
        c = pse::Blue;

    pse::rect_fill(PSE_Context->renderer, c, SDL_Rect{
        i * TILE_SCALING + TILE_SCALING / 20,
        j * TILE_SCALING + TILE_SCALING / 20,
        TILE_SCALING - TILE_SCALING / 10,
        TILE_SCALING - TILE_SCALING / 10 });
}

void draw_graph_doors(int i, int j)
{
    for (int k = 0; k < Graph[i][j].index; ++k) {
        int x = j * TILE_SCALING;
        int y = i * TILE_SCALING;
        switch (Graph[i][j].neighbors[k]) {
        case UP:
            pse::rect_fill(PSE_Context->renderer, pse::Purple, SDL_Rect{
                x + TILE_SCALING / 2 - TILE_WIDTH / 2,
                y, TILE_WIDTH, TILE_WIDTH
                });
            break;
        case RIGHT:
            pse::rect_fill(PSE_Context->renderer, pse::Purple, SDL_Rect{
                x + TILE_SCALING - TILE_WIDTH,
                y + TILE_SCALING / 2 - TILE_WIDTH / 2,
                TILE_WIDTH, TILE_WIDTH
                });
            break;
        case DOWN:
            pse::rect_fill(PSE_Context->renderer, pse::Purple, SDL_Rect{
                x + TILE_SCALING / 2 - TILE_WIDTH / 2,
                y + TILE_SCALING - TILE_WIDTH,
                TILE_WIDTH, TILE_WIDTH
                });
            break;
        case LEFT:
            pse::rect_fill(PSE_Context->renderer, pse::Purple, SDL_Rect{
                x,
                y + TILE_SCALING / 2 - TILE_WIDTH / 2,
                TILE_WIDTH, TILE_WIDTH
                });
            break;
        default:
            break;
        }
    }
}

void draw_map()
{
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            SDL_Color c;
            switch (Map[i][j]) {
                case WALL: c = pse::Dark; break;
                case FLOOR:
                    if (Player.graph_x == map_to_graph_index(j) && Player.graph_y == map_to_graph_index(i))
                        c = pse::Aqua;
                    else if (Graph[map_to_graph_index(i)][map_to_graph_index(j)].is_explored)
                        c = pse::White;
                    else
                        c = pse::Dark;
                    break;
                case EMPTY: c = pse::Black; break;
                default:
                    c = pse::Magenta;
            }
            pse::rect_fill(PSE_Context->renderer, c, SDL_Rect{
                j * TILE_WIDTH, i * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH
            });
        }
    }
}

void draw_entities()
{
    // traverse backwards, make first inserted displayed on top
    for (int i = EntityIndex - 1; i >= 0; --i) {
        if (!Entities[i]) {
#ifdef DEBUG
            printf("Invalid entity: %d\n", i);
#endif
            break;
        }
        
        SDL_Rect rect{ Entities[i]->map_x * TILE_WIDTH, Entities[i]->map_y * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH };
        SDL_Color c;

        switch (Entities[i]->id) {
            case ID_PLAYER:     c = pse::Red; break;
            case ID_STAIR_DOWN: c = pse::Orange; break;
            case ID_STAIR_UP:   c = pse::Salmon; break;
            default:
                c = pse::Magenta;
        }
        pse::rect_fill(PSE_Context->renderer, c, rect);
    }
}

/******************************************************************************
 * PSE Interface
 *
 */

void rogue_setup(pse::Context& ctx)
{
    PSE_Context = &ctx;
    gen_floor();
}

void rogue_update(pse::Context& ctx)
{
    if (ctx.check_key(SDL_SCANCODE_SPACE))
        gen_floor();

    if (ctx.check_key_invalidate(SDL_SCANCODE_W))
        player_move(UP);
    else if (ctx.check_key_invalidate(SDL_SCANCODE_D))
        player_move(RIGHT);
    else if (ctx.check_key_invalidate(SDL_SCANCODE_S))
        player_move(DOWN);
    else if (ctx.check_key_invalidate(SDL_SCANCODE_A))
        player_move(LEFT);

    if (Player.map_x == StairDown.map_x && Player.map_y == StairDown.map_y)
        gen_floor();

    draw_map();
    draw_entities();
}

}
