#include "../modules.hpp"

#include <cstdio>

namespace Modules {

constexpr unsigned MAX_NEIGHBORS = 4;
constexpr int MAP_GRID = 3;
constexpr int MAP_SIZE = 27;
constexpr int MAP_SCALING = 30;
unsigned Map[MAP_GRID][MAP_GRID] = { 0 };

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
    // create the grid of empty rooms
    struct Room {
        bool is_connected = false;
        unsigned rooms[MAX_NEIGHBORS] = { 0 };
        unsigned index = 0;
        void insert(unsigned d) {
            rooms[index++] = d;
        }
    };
    Room rooms[MAP_GRID * MAP_GRID];
    for (int i = 0; i < MAP_GRID * MAP_GRID; ++i) {
        rooms[i] = Room{};
    }

    // pick a random room to start with
    //rooms[rand_range(0, MAP_GRID * MAP_GRID)].is_connected = true;

    // determine if there are still rooms to connect
    auto unconnected_neighbors = [&](int i, int j) {
        return (!rooms[i * MAP_GRID + j - 1].is_connected
             || !rooms[i * MAP_GRID + j + 1].is_connected
             || !rooms[(i - 1) * MAP_GRID + j].is_connected
             || !rooms[(i + 1) * MAP_GRID + j].is_connected);
    };

    auto up_try_insert = [&](int i, int j) {
        if (j - 1 >= 0 && rooms[i * MAP_GRID + j - 1].index < MAX_NEIGHBORS - 1) {
            rooms[i * MAP_GRID + j].is_connected = true;
            rooms[i * MAP_GRID + j].insert(i * MAP_GRID + j - 1);
            rooms[i * MAP_GRID + j - 1].is_connected = true;
            rooms[i * MAP_GRID + j - 1].insert(i * MAP_GRID + j);
            return true;
        }
        return false;
    };
    auto down_try_insert = [&](int i, int j) {
        if (j + 1 < MAP_GRID && rooms[i * MAP_GRID + j + 1].index < MAX_NEIGHBORS - 1) {
            rooms[i * MAP_GRID + j].is_connected = true;
            rooms[i * MAP_GRID + j].insert(i * MAP_GRID + j + 1);
            rooms[i * MAP_GRID + j + 1].is_connected = true;
            rooms[i * MAP_GRID + j + 1].insert(i * MAP_GRID + j);
            return true;
        }
        return false;
    };
    auto left_try_insert = [&](int i, int j) {
        if (i - 1 >= 0 && rooms[(i - 1) * MAP_GRID + j].index < MAX_NEIGHBORS - 1) {
            rooms[i * MAP_GRID + j].is_connected = true;
            rooms[i * MAP_GRID + j].insert((i - 1) * MAP_GRID + j);
            rooms[(i - 1) * MAP_GRID + j].is_connected = true;
            rooms[(i - 1) * MAP_GRID + j].insert(i * MAP_GRID + j);
            return true;
        }
        return false;
    };
    auto right_try_insert = [&](int i, int j) {
        if (i + 1 < MAP_GRID && rooms[(i + 1) * MAP_GRID + j].index < MAX_NEIGHBORS - 1) {
            rooms[i * MAP_GRID + j].is_connected = true;
            rooms[i * MAP_GRID + j].insert((i + 1) * MAP_GRID + j);
            rooms[(i + 1) * MAP_GRID + j].is_connected = true;
            rooms[(i + 1) * MAP_GRID + j].insert(i * MAP_GRID + j);
            return true;
        }
        return false;
    };

    // connect rooms randomly
    for (int i = 0; i < MAP_GRID; ++i) {
        for (int j = 0; j < MAP_GRID; ++j) {
            if (rooms[i * MAP_GRID + j].is_connected)
                continue;

            if (!unconnected_neighbors(i, j))
                continue;
            
            int choice = rand_range(0, 4);
            switch (choice) {
            case 0:
                if (!up_try_insert(i, j))
                    continue;
                break;
            case 1:
                if (!down_try_insert(i, j))
                    continue;
                break;
            case 2:
                if (!left_try_insert(i, j))
                    continue;
                break;
            case 3:
                if (!right_try_insert(i, j))
                    continue;
                break;
            default:
                fprintf(stderr, "Error: Invalid room choice: %d\n", choice);
                exit(-1);
            }
        }
    }

    // add rooms to map
    for (int i = 0; i < MAP_GRID; ++i) {
        for (int j = 0; j < MAP_GRID; ++j) {
            Map[i][j] = rooms[i * MAP_GRID + j].index;
        }
    }
}

void rogue_setup(pse::Context& ctx)
{
    room_gen();
}

void rogue_update(pse::Context& ctx)
{
    auto fill_square = [&](SDL_Color c, int i, int j) {
        pse::rect_fill(ctx.renderer, c, SDL_Rect{
            i * MAP_GRID * MAP_SCALING,
            j * MAP_GRID * MAP_SCALING,
            MAP_GRID * MAP_SCALING,
            MAP_GRID * MAP_SCALING});
    };

    if (ctx.keystate[SDL_SCANCODE_SPACE])
        room_gen();

    // draw rooms
    for (int i = 0; i < MAP_GRID; ++i) {
        for (int j = 0; j < MAP_GRID; ++j) {
            switch (Map[i][j]) {
            case 0:
                fill_square(pse::Red, i, j);
                break;
            case 1:
                fill_square(pse::Blue, i, j);
                break;
            case 2:
                fill_square(pse::Yellow, i, j);
                break;
            case 3:
                fill_square(pse::Green, i, j);
                break;
            default:
                fill_square(pse::Magenta, i, j);
                break;
            }
        }
    }
}

}
