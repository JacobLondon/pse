/* 
 * TODO
 * 
 * Long Term:
 *  Items
 *  Enemies
 *  Attacking
 *  SC Key option menu
 * 
 */

#include "../../modules.hpp"
#include "entity.hpp"
#include "interface.hpp"
#include "gen.hpp"
#include "globals.hpp"
#include "types.hpp"

#include <cstdio>

namespace Modules {

/**
 * Debug
 */

static void debug_print_map()
{
    for (int i = 0; i < MAP_SIZE; ++i) {
        printf("\n");
        for (int j = 0; j < MAP_SIZE; ++j) {
            printf("%c", (char)FLR.Map[i][j]);
        }
    }
    printf("\n");
}

static void debug_print_player()
{
    printf("(%d, %d) -> (%d, %d)\n", Player.graph_x, Player.graph_y, Player.map_x, Player.map_y);
}

/******************************************************************************
 * PSE Interface
 *
 */

void rogue_setup(pse::Context& ctx)
{
    PSE_Context = &ctx;
    gen_floor();
    astar_init();
}

void rogue_update(pse::Context& ctx)
{
    if (ctx.check_key(SDL_SCANCODE_LSHIFT))
        gen_floor();

    if (ctx.check_key_invalidate(SDL_SCANCODE_K) || ctx.check_key_invalidate(SDL_SCANCODE_UP))
        player_move(UP);
    else if (ctx.check_key_invalidate(SDL_SCANCODE_L) || ctx.check_key_invalidate(SDL_SCANCODE_RIGHT))
        player_move(RIGHT);
    else if (ctx.check_key_invalidate(SDL_SCANCODE_J) || ctx.check_key_invalidate(SDL_SCANCODE_DOWN))
        player_move(DOWN);
    else if (ctx.check_key_invalidate(SDL_SCANCODE_H) || ctx.check_key_invalidate(SDL_SCANCODE_LEFT))
        player_move(LEFT);

    if (coords_equal(Player.map_x, Player.map_y, FLR.StairDown.map_x, FLR.StairDown.map_y)
            && ctx.check_key_invalidate(SDL_SCANCODE_SPACE))
        floor_switch(DOWN);
    else if (coords_equal(Player.map_x, Player.map_y, FLR.StairUp.map_x, FLR.StairUp.map_y)
            && ctx.check_key_invalidate(SDL_SCANCODE_SPACE))
        floor_switch(UP);

    draw_map();
    draw_entities();
    // astar_solve called in player_move
    astar_walk();
    //debug_print_player();
}

}
