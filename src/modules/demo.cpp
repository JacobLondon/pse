#include "../modules.hpp"

namespace Modules {

void demo_setup(pse::Context& ctx)
{

}

void demo_update(pse::Context& ctx)
{
    ctx.draw_rect_fill(pse::Red, SDL_Rect{ ctx.mouse.x - 50, ctx.mouse.y - 50, 100, 100 });
}

}