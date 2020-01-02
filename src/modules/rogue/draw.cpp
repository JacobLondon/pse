#include "../../pse.hpp"

#include "draw.hpp"
#include "gen.hpp"
#include "globals.hpp"
#include "types.hpp"

namespace Modules {

void load_sprites()
{
    SpritePlayerId     = PSE_Context->load_image("src/modules/rogue/textures/player.png");
    SpriteEnemyId      = PSE_Context->load_image("src/modules/rogue/textures/enemy.png");
    SpriteFloorLightId = PSE_Context->load_image("src/modules/rogue/textures/floor_light.png");
    SpriteFloorDarkId  = PSE_Context->load_image("src/modules/rogue/textures/floor_dark.png");
    SpriteWallId       = PSE_Context->load_image("src/modules/rogue/textures/wall.png");
    SpriteStairUpId    = PSE_Context->load_image("src/modules/rogue/textures/stair_up.png");
    SpriteStairDownId  = PSE_Context->load_image("src/modules/rogue/textures/stair_down.png");
}

void draw_graph()
{
    // draw rooms and doors
    for (int i = 0; i < GRAPH_SIZE; ++i) {
        for (int j = 0; j < GRAPH_SIZE; ++j) {
            draw_graph_room(i, j);
        }
    }
    for (int i = 0; i < GRAPH_SIZE; ++i) {
        for (int j = 0; j < GRAPH_SIZE; ++j) {
            draw_graph_doors(i, j);
        }
    }
}

void draw_graph_room(int i, int j)
{
    SDL_Color c;
    if (i == FLR.Start_i && j == FLR.Start_j)
        c = pse::Sky;
    else if (i == FLR.End_i && j == FLR.End_j)
        c = pse::Orange;
    else if (FLR.Graph[i][j].index == 0)
        c = pse::Red;
    else
        c = pse::Blue;

    PSE_Context->draw_rect_fill(c, SDL_Rect{
        i * TILE_SCALING + TILE_SCALING / 20,
        j * TILE_SCALING + TILE_SCALING / 20,
        TILE_SCALING - TILE_SCALING / 10,
        TILE_SCALING - TILE_SCALING / 10 });
}

void draw_graph_doors(int i, int j)
{
    for (int k = 0; k < FLR.Graph[i][j].index; ++k) {
        int x = j * TILE_SCALING;
        int y = i * TILE_SCALING;
        switch (FLR.Graph[i][j].neighbors[k]) {
        case UP:
            PSE_Context->draw_rect_fill(pse::Purple, SDL_Rect{
                x + TILE_SCALING / 2 - TILE_WIDTH / 2,
                y, TILE_WIDTH, TILE_WIDTH
                });
            break;
        case RIGHT:
            PSE_Context->draw_rect_fill(pse::Purple, SDL_Rect{
                x + TILE_SCALING - TILE_WIDTH,
                y + TILE_SCALING / 2 - TILE_WIDTH / 2,
                TILE_WIDTH, TILE_WIDTH
                });
            break;
        case DOWN:
            PSE_Context->draw_rect_fill(pse::Purple, SDL_Rect{
                x + TILE_SCALING / 2 - TILE_WIDTH / 2,
                y + TILE_SCALING - TILE_WIDTH,
                TILE_WIDTH, TILE_WIDTH
                });
            break;
        case LEFT:
            PSE_Context->draw_rect_fill(pse::Purple, SDL_Rect{
                x,
                y + TILE_SCALING / 2 - TILE_WIDTH / 2,
                TILE_WIDTH, TILE_WIDTH
                });
            break;
        }
    }
}

void draw_map()
{
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            SDL_Color c;
            SDL_Rect tile = SDL_Rect{ j * TILE_WIDTH, i * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH };
            switch (FLR.Map[i][j]) {
                case WALL:
                    PSE_Context->draw_image(SpriteWallId, tile);
                    break;
                case FLOOR:
                    if (Player.graph_x == map_to_graph_index(j) && Player.graph_y == map_to_graph_index(i))
                        PSE_Context->draw_image(SpriteFloorLightId, tile);
                    else if (FLR.Graph[map_to_graph_index(i)][map_to_graph_index(j)].is_explored)
                        PSE_Context->draw_image(SpriteFloorDarkId, tile);
                    else
                        PSE_Context->draw_image(SpriteWallId, tile);
                    break;
                case EMPTY:
                    PSE_Context->draw_rect_fill(pse::Black, tile);
                    break;
                default:
                    PSE_Context->draw_rect_fill(pse::Magenta, tile);
            }
        }
    }
}

void draw_entities()
{
    // traverse backwards, make first inserted displayed on top
    for (int i = 0; i < EntityIndex; ++i) {
        if (!Entities[i]) {
            #ifdef DEBUG
                printf("Invalid entity: %d\n", i);
            #endif
            break;
        }
        
        SDL_Rect rect{ Entities[i]->map_x * TILE_WIDTH, Entities[i]->map_y * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH };
        SDL_Color c;

        switch (Entities[i]->id) {
            case ID_PLAYER:
                PSE_Context->draw_image(SpritePlayerId, rect);
                break;
            case ID_ENEMY:
                if (!FLR.Graph[Entities[i]->graph_y][Entities[i]->graph_x].is_explored)
                    PSE_Context->draw_image(SpriteWallId, rect);
                else if (coords_equal(Player.graph_x, Player.graph_y, Entities[i]->graph_x, Entities[i]->graph_y))
                    PSE_Context->draw_image(SpriteEnemyId, rect);
                else
                    PSE_Context->draw_image(SpriteFloorDarkId, rect);
                break;
            case ID_STAIR_DOWN:
                if (FLR.Graph[FLR.StairDown.graph_y][FLR.StairDown.graph_x].is_explored)
                    PSE_Context->draw_image(SpriteStairDownId, rect);
                else
                    PSE_Context->draw_image(SpriteWallId, rect);
                break;
            case ID_STAIR_UP:
                PSE_Context->draw_image(SpriteStairUpId, rect);
                break;
            default:
                PSE_Context->draw_rect_fill(pse::Magenta, rect);
        }
    }
}

}