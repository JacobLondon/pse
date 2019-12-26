#include "../modules.hpp"

#include <cstdio>

namespace Modules {

constexpr unsigned MAX_NEIGHBORS = 4;
constexpr int MAP_GRID = 3;
constexpr int MAP_SIZE = 27;
constexpr int MAP_SCALING = 30;

// 2. grid data
struct Room {
    bool is_connected = false;
    int rooms[MAX_NEIGHBORS] = { 0 };
    int index = 0;
    void insert(int d) {
        rooms[index++] = d;
    }
};

enum Directions {
    UP,
    RIGHT,
    DOWN,
    LEFT,
};

Room Map[MAP_GRID][MAP_GRID];


/**
 * https://web.archive.org/web/20131025132021/http://kuoi.org/~kamikaze/GameDesign/art07_rogue_dungeon.php
 * 
 * 1. Divide the map into a grid (Rogue uses 3x3, but any size will work).
 * 2. Give each grid a flag indicating if it's "connected" or not, and an array of which grid numbers it's connected to.
 * 3. Pick a random room to start with, and mark it "connected".
 * 4. While there are unconnected neighbor rooms, connect to one of them, make that the current room, mark it "connected", and repeat.
 * 5. While there are unconnected rooms, try to connect them to a random connected neighbor (if a room has no connected neighbors yet, just keep cycling, you'll fill out to it eventually).
 * 6. All rooms are now connected at least once.
 * 7. Make 0 or more random connections to taste; I find rnd(grid_width) random connections looks good.
 * 8. Draw the rooms onto the map, and draw a corridor from the center of each room to the center of each connected room, changing wall blocks into corridors. If your rooms fill most or all of the space of the grid, your corridors will very short - just holes in the wall.
 * 9. Scan the map for corridor squares with 2 bordering walls, 1-2 bordering rooms, and 0-1 bordering corridor, and change those to doors.
 * 10. Place your stairs up in the first room you chose, and your stairs down in the last room chosen in step 5. This will almost always be a LONG way away.
 * 11. All done!
 */
void room_gen();
void room_gen()
{
    // 1. create the grid of empty rooms
    Room rooms[MAP_GRID * MAP_GRID];
    for (int i = 0; i < MAP_GRID * MAP_GRID; ++i) {
        rooms[i] = Room{};
    }

    // determine if there are still rooms to connect
    auto unconnected_neighbors = [&](int i, int j) {
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
    };

    auto up_try_insert = [&](int i, int j) {
        if (j - 1 >= 0 && rooms[i * MAP_GRID + j - 1].index < MAX_NEIGHBORS - 1) {
            rooms[i * MAP_GRID + j].is_connected = true;
            rooms[i * MAP_GRID + j].insert(Directions::UP);
            rooms[i * MAP_GRID + j - 1].is_connected = true;
            rooms[i * MAP_GRID + j - 1].insert(Directions::DOWN);
            return true;
        }
        return false;
    };
    auto down_try_insert = [&](int i, int j) {
        if (j + 1 < MAP_GRID && rooms[i * MAP_GRID + j + 1].index < MAX_NEIGHBORS - 1) {
            rooms[i * MAP_GRID + j].is_connected = true;
            rooms[i * MAP_GRID + j].insert(Directions::DOWN);
            rooms[i * MAP_GRID + j + 1].is_connected = true;
            rooms[i * MAP_GRID + j + 1].insert(Directions::UP);
            return true;
        }
        return false;
    };
    auto left_try_insert = [&](int i, int j) {
        if (i - 1 >= 0 && rooms[(i - 1) * MAP_GRID + j].index < MAX_NEIGHBORS - 1) {
            rooms[i * MAP_GRID + j].is_connected = true;
            rooms[i * MAP_GRID + j].insert(Directions::LEFT);
            rooms[(i - 1) * MAP_GRID + j].is_connected = true;
            rooms[(i - 1) * MAP_GRID + j].insert(Directions::RIGHT);
            return true;
        }
        return false;
    };
    auto right_try_insert = [&](int i, int j) {
        if (i + 1 < MAP_GRID && rooms[(i + 1) * MAP_GRID + j].index < MAX_NEIGHBORS - 1) {
            rooms[i * MAP_GRID + j].is_connected = true;
            rooms[i * MAP_GRID + j].insert(Directions::RIGHT);
            rooms[(i + 1) * MAP_GRID + j].is_connected = true;
            rooms[(i + 1) * MAP_GRID + j].insert(Directions::LEFT);
            return true;
        }
        return false;
    };

    int direction;
    auto room_try_connect = [&](int i, int j) {
        direction = rand_range(0, MAX_NEIGHBORS);
        printf("d = %d, i = %d, j= %d\n", direction, i, j);
        switch (direction) {
            case UP:    return up_try_insert(i, j);
            case RIGHT: return right_try_insert(i, j);
            case DOWN:  return down_try_insert(i, j);
            case LEFT:  return left_try_insert(i, j);
            default:
                fprintf(stderr, "Error: Invalid room choice: %d\n", direction);
                exit(-1);
        }
    };

    // 3. pick a random room to start with
    int curr_i = rand_range(0, MAP_GRID);
    int curr_j = rand_range(0, MAP_GRID);
    rooms[curr_i * MAP_GRID + curr_j].is_connected = true;

    // 4. connect unconnected neighbors
    while (unconnected_neighbors(curr_i, curr_j)) {
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
    /*
    // 5. connect any unconnected rooms
    for (int i = 0; i < MAP_GRID; ++i) {
        for (int j = 0; j < MAP_GRID; ++j) {
            while (!rooms[i * MAP_GRID + j].is_connected) {
                room_try_connect(i, j);
            }
        }
    }*/

    // add rooms to map
    for (int i = 0; i < MAP_GRID; ++i) {
        for (int j = 0; j < MAP_GRID; ++j) {
            Map[i][j] = rooms[i * MAP_GRID + j];
        }
    }
}

void rogue_setup(pse::Context& ctx)
{
    room_gen();
}

void rogue_update(pse::Context& ctx)
{
    auto draw_square = [&](SDL_Color c, int i, int j) {
        pse::rect_fill(ctx.renderer, c, SDL_Rect{
            i * MAP_GRID * MAP_SCALING + MAP_GRID * MAP_SCALING / 20,
            j * MAP_GRID * MAP_SCALING + MAP_GRID * MAP_SCALING / 20,
            MAP_GRID * MAP_SCALING - MAP_GRID * MAP_SCALING / 10,
            MAP_GRID * MAP_SCALING - MAP_GRID * MAP_SCALING / 10});
    };

    auto draw_room = [&](int i, int j) {
        switch (Map[i][j].index) {
            case 0:  draw_square(pse::Red, i, j);     break;
            case 1:  draw_square(pse::Blue, i, j);    break;
            case 2:  draw_square(pse::Yellow, i, j);  break;
            case 3:  draw_square(pse::Green, i, j);   break;
            default: draw_square(pse::Magenta, i, j); break;
        }
    };

    auto draw_doors = [&](int i, int j) {
        for (int k = 0; k < Map[i][j].index; ++k) {
            int x = i * MAP_GRID * MAP_SCALING;
            int y = j * MAP_GRID * MAP_SCALING;
            int w = MAP_GRID * MAP_SCALING / 5;
            switch (Map[i][j].rooms[k]) {
                case UP:
                    pse::rect_fill(ctx.renderer, pse::Purple, SDL_Rect{
                        x + MAP_GRID * MAP_SCALING / 2 - w / 2,
                        y, w, w
                    });
                    break;
                case RIGHT:
                    pse::rect_fill(ctx.renderer, pse::Purple, SDL_Rect{
                        x + MAP_GRID * MAP_SCALING - w,
                        y + MAP_GRID * MAP_SCALING / 2 - w / 2,
                        w, w
                    });
                    break;
                case DOWN:
                    pse::rect_fill(ctx.renderer, pse::Purple, SDL_Rect{
                        x + MAP_GRID * MAP_SCALING / 2 - w / 2,
                        y + MAP_GRID * MAP_SCALING - w,
                        w, w
                    });
                    break;
                case LEFT:
                    pse::rect_fill(ctx.renderer, pse::Purple, SDL_Rect{
                        x,
                        y + MAP_GRID * MAP_SCALING / 2 - w / 2,
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

    // TODO: Some rooms are magenta, and should not be

    // draw rooms
    for (int i = 0; i < MAP_GRID; ++i) {
        for (int j = 0; j < MAP_GRID; ++j) {
            draw_room(i, j);
            draw_doors(i, j);
        }
    }
}

}
