#include "../modules.hpp"

#include <cstdio>
#include <time.h>

namespace Modules {

/**
 * Constants
 */

constexpr unsigned MAX_NEIGHBORS = 4;
constexpr int MAP_GRID = 6;
constexpr int MAP_SIZE = 60;    // ~10x MAP_GRID
constexpr int MAP_SCALING = 60;
// the number of tiles square of each room
constexpr int ROOM_WIDTH = MAP_SIZE / MAP_GRID - 1;
constexpr int ROOM_TOLERANCE = ROOM_WIDTH / 2;
constexpr int ROOM_CONNECT_TRIES = 40;
#define ROOM_PATH_MODIFIER 2 / 3 /* INTENDS TO HAVE OPERATOR PRECEDENCE MAKE LHS RVALUE GREATER THAN RHS */

constexpr int SQ_WIDTH = MAP_SCALING / 7;
constexpr int PLAYER_MOVE_COOLDOWN = 50; // milliseconds

/**
 * Definitions
 */

enum BuildingBlocks {
    WALL = '#',
    FLOOR = '.',
    EMPTY = ' ',
    STAIR = '%',
};

enum Directions {
    UP,
    RIGHT,
    DOWN,
    LEFT,
};

// graph node
struct Room {
    bool is_connected = false;
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

/**
 * Globals
 */

Room Graph[MAP_GRID][MAP_GRID]; // graph nodes to generate a map from
int Map[MAP_SIZE][MAP_SIZE]; // floor plan of every tile on that floor
int Start_i, Start_j, End_i, End_j; // graph locations of starting (spawn) and ending (stair) rooms

int Player_x, Player_y; // map index of player
int Stair_x, Stair_y; // map index of stair
clock_t PlayerLastMove = 0; // last movement for cooldown

/**
 * Player movement
 */

void player_spawn(); // spawn player at center of Start_i/j
bool player_check_tile(int offset_x, int offset_y); // check if the player_x/y + offset_x/y is on a walkable tile
void player_move(int direction); // move the player, ensures the tile to walk upon is walkable, else no movement

void player_spawn()
{
    // spawn player at center of start_i/j room
    Player_y = Start_i * ROOM_WIDTH + ROOM_WIDTH / 2;
    Player_x = Start_j * ROOM_WIDTH + ROOM_WIDTH / 2;
}

bool player_check_tile(int offset_x, int offset_y)
{
    switch (Map[Player_y + offset_y][Player_x + offset_x]) {
        case FLOOR:
        case STAIR:
            return true;
        default:
            return false;
    }
}

void player_move(int direction)
{
    // only let the player move if the cooldown is over
    if (clock() - PlayerLastMove < PLAYER_MOVE_COOLDOWN)
        return;
    
    PlayerLastMove = clock();

    // move with bounds check
    switch (direction) {
        case UP:    if (player_check_tile(0, -1)) Player_y -= 1; break;
        case RIGHT: if (player_check_tile(1, 0))  Player_x += 1; break;
        case DOWN:  if (player_check_tile(0, 1))  Player_y += 1; break;
        case LEFT:  if (player_check_tile(-1, 0)) Player_x -= 1; break;
        default:
            break;
    }
}

/******************************************************************************
 * Floor Generation
 * 
 * https://web.archive.org/web/20131025132021/http://kuoi.org/~kamikaze/GameDesign/art07_rogue_dungeon.php
 * 
 */

void gen_graph(); // generate a set of rooms from globals, write data structure into the global 'Graph'
bool has_unconnected_neighbors(int i, int j);  // return true if a room has at least 1 unconnected neighbor
bool room_try_connect(int *out_direction, int i, int j); // use try_insert fns to randomly connect a room
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
    if (j + 1 < MAP_GRID && Graph[i][j + 1].index < MAX_NEIGHBORS - 1) {
        Graph[i][j].insert_neighbor(RIGHT);
        Graph[i][j + 1].insert_neighbor(LEFT);
        return true;
    }
    return false;
}

bool down_try_insert(int i, int j)
{
    if (i + 1 < MAP_GRID && Graph[i + 1][j].index < MAX_NEIGHBORS - 1) {
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

bool room_try_connect(int *out_direction, int i, int j)
{
    // randomly select a direction that was not yet chosen
    do {
        *out_direction = rand_range(0, MAX_NEIGHBORS);
    } while (Graph[i][j].check_neighbor(*out_direction));

    switch (*out_direction) {
        case UP:    return up_try_insert(i, j);
        case RIGHT: return right_try_insert(i, j);
        case DOWN:  return down_try_insert(i, j);
        case LEFT:  return left_try_insert(i, j);
        default:
            fprintf(stderr, "Error: Invalid room choice: %d\n", *out_direction);
            exit(-1);
    }
};

bool has_unconnected_neighbors(int i, int j)
{
    bool connected = false;

    // bounds check before checking neighbors
    if (i - 1 >= 0)       connected = connected || !Graph[i - 1][j].is_connected;
    if (i + 1 < MAP_GRID) connected = connected || !Graph[i + 1][j].is_connected;
    if (j - 1 >= 0)       connected = connected || !Graph[i][j - 1].is_connected;
    if (j + 1 < MAP_GRID) connected = connected || !Graph[i][j + 1].is_connected;
    
    return connected;
}

void gen_graph()
{
    // clear global Graph
    for (int i = 0; i < MAP_GRID; ++i) {
        for (int j = 0; j < MAP_GRID; ++j) {
            Graph[i][j] = Room{};
        }
    }

    // pick a random room to start with (for random walk and player spawn)
    int curr_i = rand_range(0, MAP_GRID);
    int curr_j = rand_range(0, MAP_GRID);
    Graph[curr_i][curr_j].is_connected = true;
    Start_i = curr_i; Start_j = curr_j;

    // connect unconnected neighbors, change the state of direction
    int direction; // direction is modified within room_try_connect

    // random walk across the graph until blocked or finished (no backtracking)
    while (has_unconnected_neighbors(curr_i, curr_j)) {
        if (room_try_connect(&direction, curr_i, curr_j)) {
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
    for (int i = 0; i < MAP_GRID; ++i) {
        for (int j = 0; j < MAP_GRID; ++j) {
            if (!Graph[i][j].is_connected) {
                int tries = 0;
                while (Graph[i][j].index < 2) {
                    room_try_connect(&direction, i, j);
                    if (tries++ > ROOM_CONNECT_TRIES)
                        break;
                }
            }
        }
    }
}

void gen_map()
{
    // Create map with rooms
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            Map[i][j] = WALL;
        }
    }

    // draw rooms and their doors for each node in the graph
    for (int mi = 0; mi < MAP_GRID; ++mi) {
        for (int mj = 0; mj < MAP_GRID; ++mj) {
            // ignore empty nodes
            if (Graph[mi][mj].index == 0)
                continue;

            // random room width and height relative to the map
            int room_w = rand_range(ROOM_TOLERANCE, ROOM_WIDTH);
            int room_h = rand_range(ROOM_TOLERANCE, ROOM_WIDTH);
            int room_i = mi * ROOM_WIDTH;
            int room_j = mj * ROOM_WIDTH;
            int center_i = mi * ROOM_WIDTH + ROOM_WIDTH / 2;
            int center_j = mj * ROOM_WIDTH + ROOM_WIDTH / 2;

            // fill room area with floor
            for (int i = room_i; i < room_i + room_h; ++i) {
                for (int j = room_j; j < room_j + room_w; ++j) {
                    Map[i][j] = FLOOR;
                }
            }

            // walk towards door in each direction from room center if it has a door that way
            if (Graph[mi][mj].check_neighbor(DOWN)) {
                for (int i = center_i; i < mi * ROOM_WIDTH + ROOM_WIDTH * ROOM_PATH_MODIFIER; ++i)
                    Map[i][center_j] = FLOOR;
            }
            if (Graph[mi][mj].check_neighbor(UP)) {
                for (int i = center_i; i > mi* ROOM_WIDTH - ROOM_WIDTH * ROOM_PATH_MODIFIER; --i)
                    Map[i][center_j] = FLOOR;
            }
            if (Graph[mi][mj].check_neighbor(RIGHT))
                for (int j = center_j; j < mj * ROOM_WIDTH + ROOM_WIDTH * ROOM_PATH_MODIFIER; ++j)
                    Map[center_i][j] = FLOOR;
            if (Graph[mi][mj].check_neighbor(LEFT))
                for (int j = center_j; j > mj* ROOM_WIDTH - ROOM_WIDTH * ROOM_PATH_MODIFIER; --j)
                    Map[center_i][j] = FLOOR;

            // draw surrounding border wall around entire map
            for (int i = 0; i < MAP_SIZE; ++i) {
                for (int j = 0; j < MAP_SIZE; ++j) {
                    if (i == 0 || i == MAP_SIZE - 1 || j == 0 || j == MAP_SIZE - 1)
                        Map[i][j] = WALL;
                }
            }
        }
    }

    // place player in the map
    player_spawn();

    // place stairs
    Stair_x = End_j * ROOM_WIDTH + ROOM_WIDTH / 2;
    Stair_y = End_i * ROOM_WIDTH + ROOM_WIDTH / 2;
    Map[Stair_y][Stair_x] = STAIR;


    /*for (int i = 0; i < MAP_SIZE; ++i) {
        printf("\n");
        for (int j = 0; j < MAP_SIZE; ++j) {
            printf("%c", (char)Map[i][j]);
        }
    }
    printf("\n");*/
}

void gen_floor()
{
    gen_graph();
    gen_map();
}

/******************************************************************************
 * Drawing and PSE Interface
 * 
 */

void draw_graph(pse::Context& ctx);
void draw_map(pse::Context& ctx);
void draw_player(pse::Context& ctx);

void draw_graph(pse::Context& ctx)
{
    auto draw_room = [&](int i, int j) {
        SDL_Color c;
        if (i == Start_i && j == Start_j)
            c = pse::Sky;
        else if (i == End_i && j == End_j)
            c = pse::Orange;
        else if (Graph[i][j].index == 0)
            c = pse::Red;
        else
            c = pse::Blue;
        
        pse::rect_fill(ctx.renderer, c, SDL_Rect{
            i * MAP_SCALING + MAP_SCALING / 20,
            j * MAP_SCALING + MAP_SCALING / 20,
            MAP_SCALING - MAP_SCALING / 10,
            MAP_SCALING - MAP_SCALING / 10});
    };

    auto draw_doors = [&](int i, int j) {
        for (int k = 0; k < Graph[i][j].index; ++k) {
            int x = j * MAP_SCALING;
            int y = i * MAP_SCALING;
            int w = MAP_SCALING / 5;
            switch (Graph[i][j].neighbors[k]) {
                case UP:
                    pse::rect_fill(ctx.renderer, pse::Purple, SDL_Rect{
                        x + MAP_SCALING / 2 - w / 2,
                        y, w, w
                    });
                    break;
                case RIGHT:
                    pse::rect_fill(ctx.renderer, pse::Purple, SDL_Rect{
                        x + MAP_SCALING - w,
                        y + MAP_SCALING / 2 - w / 2,
                        w, w
                    });
                    break;
                case DOWN:
                    pse::rect_fill(ctx.renderer, pse::Purple, SDL_Rect{
                        x + MAP_SCALING / 2 - w / 2,
                        y + MAP_SCALING - w,
                        w, w
                    });
                    break;
                case LEFT:
                    pse::rect_fill(ctx.renderer, pse::Purple, SDL_Rect{
                        x,
                        y + MAP_SCALING / 2 - w / 2,
                        w, w
                    });
                    break;
                default:
                    break;
            }
        }
    };

    // draw rooms
    for (int i = 0; i < MAP_GRID; ++i) {
        for (int j = 0; j < MAP_GRID; ++j) {
            draw_room(i, j);
            // can't put draw_doors here??????????????
            //draw_doors(i, j);
        }
    }
    // why can't this go above????
    for (int i = 0; i < MAP_GRID; ++i) {
        for (int j = 0; j < MAP_GRID; ++j) {
            draw_doors(i, j);
        }
    }
}

void draw_map(pse::Context& ctx)
{
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            SDL_Color c;
            switch (Map[i][j]) {
                case WALL:  c = pse::Dark; break;
                case FLOOR: c = pse::White; break;
                case EMPTY: c = pse::Black; break;
                case STAIR: c = pse::Orange; break;
                default:    c = pse::Magenta;
            }
            pse::rect_fill(ctx.renderer, c, SDL_Rect{
                j * SQ_WIDTH, i * SQ_WIDTH, SQ_WIDTH, SQ_WIDTH
            });
        }
    }
}

void draw_player(pse::Context& ctx)
{
    pse::rect_fill(ctx.renderer, pse::Red, SDL_Rect{
        Player_x * SQ_WIDTH, Player_y * SQ_WIDTH, SQ_WIDTH, SQ_WIDTH
    });
}

void rogue_setup(pse::Context& ctx)
{
    gen_floor();
}

void rogue_update(pse::Context& ctx)
{
    if (ctx.keystate[SDL_SCANCODE_SPACE])
        gen_floor();

    if (ctx.keystate[SDL_SCANCODE_W])
        player_move(UP);
    else if (ctx.keystate[SDL_SCANCODE_D])
        player_move(RIGHT);
    else if (ctx.keystate[SDL_SCANCODE_S])
        player_move(DOWN);
    else if (ctx.keystate[SDL_SCANCODE_A])
        player_move(LEFT);

    draw_map(ctx);
    draw_player(ctx);

    if (Player_x == Stair_x && Player_y == Stair_y)
        gen_floor();
}

}
