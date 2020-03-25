#include "../modules.hpp"

namespace Modules {

void demo_setup(pse::Context& ctx)
{

}

void demo_update(pse::Context& ctx)
{
    //ctx.draw_rect_fill(pse::Red, SDL_Rect{ ctx.mouse.x - 50, ctx.mouse.y - 50, 100, 100 });
    int x1 = ctx.mouse.x - 100;
    int y1 = ctx.mouse.y + 50;
    int x2 = ctx.mouse.x + 50;
    int y2 = ctx.mouse.y - 50;
    int x3 = ctx.mouse.x - 50;
    int y3 = ctx.mouse.y + 100;
    ctx.draw_tri_fill_scan(pse::Blue, x1, y1, x2, y2, x3, y3);
    ctx.draw_tri(pse::Red, x1, y1, x2, y2, x3, y3);
    //ctx.draw_rect_fill(pse::Green, SDL_Rect{ x2, y2, 50, 50 });
    //ctx.draw_rect_fill(pse::Orange, SDL_Rect{ x1-50, y1-50, 50, 50 });
}

}