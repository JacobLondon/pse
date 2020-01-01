#include "../../pse.hpp"

#include "interface.hpp"
#include "gen.hpp"
#include "globals.hpp"
#include "types.hpp"

namespace Modules {

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

    pse::rect_fill(PSE_Context->renderer, c, SDL_Rect{
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
        }
    }
}

void draw_map()
{
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            SDL_Color c;
            switch (FLR.Map[i][j]) {
                case WALL: c = pse::Dark; break;
                case FLOOR:
                    if (Player.graph_x == map_to_graph_index(j) && Player.graph_y == map_to_graph_index(i))
                        c = pse::White;
                    else if (FLR.Graph[map_to_graph_index(i)][map_to_graph_index(j)].is_explored)
                        c = pse::Gray;
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
            case ID_STAIR_DOWN:
                if (FLR.Graph[FLR.StairDown.graph_y][FLR.StairDown.graph_x].is_explored)
                    c = pse::Orange;
                else
                    c = pse::Dark;
                break;
            case ID_STAIR_UP:   c = pse::Salmon; break;
            default:
                c = pse::Magenta;
        }
        pse::rect_fill(PSE_Context->renderer, c, rect);
    }
}

}