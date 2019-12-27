#include "../modules.hpp"

#include <cstdio>

namespace Modules {

/**
 * Constants
 */

constexpr unsigned MAX_NEIGHBORS = 4;
constexpr int MAP_GRID = 3;
constexpr int MAP_SIZE = 27;
constexpr int MAP_SCALING = 60;

/**
 * Definitions
 */

enum Directions {
    UP,
    RIGHT,
    DOWN,
    LEFT,
};

// 2. grid data
struct Room {
    bool is_connected = false;
    int neighbors[MAX_NEIGHBORS] = { 0 };
    int index = 0;

    void insert_neighbor(int neighbor) {
        neighbors[index++] = neighbor;
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
    // check if the given neighbor is already connected to
    bool check_neighbor(int neighbor) {
        for (int i = 0; i < index; ++i) {
            if (neighbors[i] == neighbor)
                return true;
        }
        return false;
    }
};

/**
 * Globals
 */

Room Map[MAP_GRID][MAP_GRID];


/******************************************************************************
 * Room Generation
 * 
 * https://web.archive.org/web/20131025132021/http://kuoi.org/~kamikaze/GameDesign/art07_rogue_dungeon.php
 * 
 * 1. Divide the map into a grid (Rogue uses 3x3, but any size will work).
 * 2. Give each grid a flag indicating if it's "connected" or not, and an array of which grid numbers it's connected to.
 * 3. Pick a random room to start with, and mark it "connected".
 * 4. While there are unconnected neighbor rooms, connect to one of them, make that the current room, mark it "connected", and repeat.
 * 5. While there are unconnected rooms, try to connect them to a random connected neighbor
 *    (if a room has no connected neighbors yet, just keep cycling, you'll fill out to it eventually).
 * 6. All rooms are now connected at least once.
 * 7. Make 0 or more random connections to taste; I find rnd(grid_width) random connections looks good.
 * 8. Draw the rooms onto the map, and draw a corridor from the center of each room to the center of each connected room,
 *    changing wall blocks into corridors. If your rooms fill most or all of the space of the grid,
 *    your corridors will very short - just holes in the wall.
 * 9. Scan the map for corridor squares with 2 bordering walls, 1-2 bordering rooms, and 0-1 bordering corridor, and change those to doors.
 * 10. Place your stairs up in the first room you chose, and your stairs down in the last room chosen in step 5.
 *     This will almost always be a LONG way away.
 * 11. All done!
 */

void room_gen(); // generate a set of rooms from globals, write data structure into the global 'Map'
bool unconnected_neighbors(Room *rooms, int i, int j);  // return true if a room has at least 1 unconnected neighbor
bool up_try_insert(Room *rooms, int i, int j);          // try to make room connection up, return false on fail
bool right_try_insert(Room *rooms, int i, int j);       // try to make room connection right, return false on fail
bool down_try_insert(Room *rooms, int i, int j);        // try to make room connection down, return false on fail
bool left_try_insert(Room *rooms, int i, int j);        // try to make room connection left, return false on fail

bool up_try_insert(Room *rooms, int i, int j)
{
    if (j - 1 >= 0
        && rooms[i * MAP_GRID + j - 1].index < MAX_NEIGHBORS - 1
        && !rooms[i * MAP_GRID + j].check_neighbor(UP))
    {
        rooms[i * MAP_GRID + j].is_connected = true;
        rooms[i * MAP_GRID + j].insert_neighbor(UP);
        rooms[i * MAP_GRID + j - 1].is_connected = true;
        rooms[i * MAP_GRID + j - 1].insert_neighbor(DOWN);
        return true;
    }
    return false;
}

bool right_try_insert(Room *rooms, int i, int j)
{
    if (i + 1 < MAP_GRID
        && rooms[(i + 1) * MAP_GRID + j].index < MAX_NEIGHBORS - 1
        && !rooms[i * MAP_GRID + j].check_neighbor(RIGHT))
    {
        rooms[i * MAP_GRID + j].is_connected = true;
        rooms[i * MAP_GRID + j].insert_neighbor(RIGHT);
        rooms[(i + 1) * MAP_GRID + j].is_connected = true;
        rooms[(i + 1) * MAP_GRID + j].insert_neighbor(LEFT);
        return true;
    }
    return false;
}

bool down_try_insert(Room *rooms, int i, int j)
{
    if (j + 1 < MAP_GRID
        && rooms[i * MAP_GRID + j + 1].index < MAX_NEIGHBORS - 1
        && !rooms[i * MAP_GRID + j].check_neighbor(DOWN))
    {
        rooms[i * MAP_GRID + j].is_connected = true;
        rooms[i * MAP_GRID + j].insert_neighbor(DOWN);
        rooms[i * MAP_GRID + j + 1].is_connected = true;
        rooms[i * MAP_GRID + j + 1].insert_neighbor(UP);
        return true;
    }
    return false;
}

bool left_try_insert(Room *rooms, int i, int j)
{
    if (i - 1 >= 0
        && rooms[(i - 1) * MAP_GRID + j].index < MAX_NEIGHBORS - 1
        && !rooms[i * MAP_GRID + j].check_neighbor(LEFT))
    {
        rooms[i * MAP_GRID + j].is_connected = true;
        rooms[i * MAP_GRID + j].insert_neighbor(LEFT);
        rooms[(i - 1) * MAP_GRID + j].is_connected = true;
        rooms[(i - 1) * MAP_GRID + j].insert_neighbor(RIGHT);
        return true;
    }
    return false;
}

bool unconnected_neighbors(Room *rooms, int i, int j)
{
    bool connected = false;
    if (j - 1 >= 0)
        connected = connected || !rooms[i * MAP_GRID + j - 1].is_connected;
    if (j + 1 < MAP_GRID)
        connected = connected || !rooms[i * MAP_GRID + j + 1].is_connected;
    if (i - 1 >= 0)
        connected = connected || !rooms[(i - 1) * MAP_GRID + j].is_connected;
    if (i + 1 < MAP_GRID)
        connected = connected || !rooms[(i + 1) * MAP_GRID + j].is_connected;
    
    return connected;
}

void room_gen()
{
    // 1. create the grid of empty rooms
    Room rooms[MAP_GRID * MAP_GRID];
    for (int i = 0; i < MAP_GRID * MAP_GRID; ++i) {
        rooms[i] = Room{};
    }

    // 3. pick a random room to start with
    int curr_i = rand_range(0, MAP_GRID);
    int curr_j = rand_range(0, MAP_GRID);
    rooms[curr_i * MAP_GRID + curr_j].is_connected = true;

    // 4. connect unconnected neighbors, change the state of direction
    int direction;
    auto room_try_connect = [&](int i, int j) {
        direction = rand_range(0, MAX_NEIGHBORS);
        switch (direction) {
            case UP:    return up_try_insert(rooms, i, j);
            case RIGHT: return right_try_insert(rooms, i, j);
            case DOWN:  return down_try_insert(rooms, i, j);
            case LEFT:  return left_try_insert(rooms, i, j);
            default:
                fprintf(stderr, "Error: Invalid room choice: %d\n", direction);
                exit(-1);
        }
    };

    // change current room selection from chosen direction
    while (unconnected_neighbors(rooms, curr_i, curr_j)) {
        if (room_try_connect(curr_i, curr_j)) {
            switch (direction) {
                case UP:    curr_j -= 1; break;
                case RIGHT: curr_i += 1; break;
                case DOWN:  curr_j += 1; break;
                case LEFT:  curr_i -= 1; break;
                default:
                    fprintf(stderr, "Error: Invalid room direction: %d\n", direction);
                    exit(-1);
            }
        }
    }
    
    // 5. connect any still unconnected rooms with at least 2 neighbors, prevent dead room connections
    for (int i = 0; i < MAP_GRID; ++i) {
        for (int j = 0; j < MAP_GRID; ++j) {
            if (!rooms[i * MAP_GRID + j].is_connected) {
                while (rooms[i * MAP_GRID + j].index < 2)
                    room_try_connect(i, j);
            }
        }
    }

    // add rooms to map
    for (int i = 0; i < MAP_GRID; ++i) {
        for (int j = 0; j < MAP_GRID; ++j) {
            Map[i][j] = rooms[i * MAP_GRID + j];
        }
    }

}

/******************************************************************************
 * PSE Interface
 * 
 */

void rogue_setup(pse::Context& ctx)
{
    room_gen();
}

void rogue_update(pse::Context& ctx)
{
    auto draw_square = [&](SDL_Color c, int i, int j) {
        pse::rect_fill(ctx.renderer, c, SDL_Rect{
            i * MAP_SCALING + MAP_SCALING / 20,
            j * MAP_SCALING + MAP_SCALING / 20,
            MAP_SCALING - MAP_SCALING / 10,
            MAP_SCALING - MAP_SCALING / 10});
    };

    auto draw_room = [&](int i, int j) {
        switch (Map[i][j].index) {
            case 0:  draw_square(pse::Red, i, j);     break;
            case 1:  draw_square(pse::Blue, i, j);    break;
            case 2:  draw_square(pse::Yellow, i, j);  break;
            case 3:  draw_square(pse::Green, i, j);   break;
            default:
                draw_square(pse::Magenta, i, j);
                break;
        }
    };

    auto draw_doors = [&](int i, int j) {
        for (int k = 0; k < Map[i][j].index; ++k) {
            int x = i * MAP_SCALING;
            int y = j * MAP_SCALING;
            int w = MAP_SCALING / 5;
            switch (Map[i][j].neighbors[k]) {
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

    if (ctx.keystate[SDL_SCANCODE_SPACE])
        room_gen();

    // draw rooms
    for (int i = 0; i < MAP_GRID; ++i) {
        for (int j = 0; j < MAP_GRID; ++j) {
            draw_room(i, j);
            draw_doors(i, j);
        }
    }
}

}
