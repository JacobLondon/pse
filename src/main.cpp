#include <cstdio>

#include "context.hpp"
#include "draw.hpp"

void setup(pse::Context& ctx);
void update(pse::Context& ctx);

void setup(pse::Context& ctx)
{
    
}

void update(pse::Context& ctx)
{
    pse::rect_fill(ctx.renderer, pse::Red,
        SDL_Rect{ ctx.mouse.x - 50, ctx.mouse.y - 50, 100, 100 });
    
    printf("fps: %lf\r", 1.0 / ctx.delta_time);
}

int main()
{
    pse::Context ctx = pse::Context("PSE", 640, 480, 120, setup, update);

    return 0;
}