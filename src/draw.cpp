#include <SDL2/SDL.h>

#include "draw.hpp"

namespace pse {

void clear(SDL_Renderer* rend, SDL_Color c)
{
    SDL_SetRenderDrawColor(rend, c.r, c.g, c.b, c.a);
    SDL_RenderClear(rend);
}

void rect(SDL_Renderer* rend, SDL_Color c, SDL_Rect rect)
{
    SDL_SetRenderDrawColor(rend, c.r, c.g, c.b, c.a);
    SDL_RenderDrawRect(rend, &rect);
}

void rect_fill(SDL_Renderer* rend, SDL_Color c, SDL_Rect rect)
{
    SDL_SetRenderDrawColor(rend, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(rend, &rect);
}

void circle(SDL_Renderer* rend, SDL_Color c, int x, int y, int radius)
{
    SDL_SetRenderDrawColor(rend, c.r, c.g, c.b, c.a);

    // https://stackoverflow.com/questions/38334081/howto-draw-circles-arcs-and-vector-graphics-in-sdl

    int diameter = (radius * 2);

    int cx = (radius - 1);
    int cy = 0;
    int tx = 1;
    int ty = 1;
    int error = (tx - diameter);

    while (cx >= cy)
    {
        //  Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(rend, x + cx, y - cy);
        SDL_RenderDrawPoint(rend, x + cx, y + cy);
        SDL_RenderDrawPoint(rend, x - cx, y - cy);
        SDL_RenderDrawPoint(rend, x - cx, y + cy);
        SDL_RenderDrawPoint(rend, x + cy, y - cx);
        SDL_RenderDrawPoint(rend, x + cy, y + cx);
        SDL_RenderDrawPoint(rend, x - cy, y - cx);
        SDL_RenderDrawPoint(rend, x - cy, y + cx);

        if (error <= 0) {
            ++cy;
            error += ty;
            ty += 2;
        }

        if (error > 0) {
            --cx;
            tx += 2;
            error += (tx - diameter);
        }
    }
}

void circle_fill(SDL_Renderer* rend, SDL_Color c, int x, int y, int radius)
{
    SDL_SetRenderDrawColor(rend, c.r, c.g, c.b, c.a);

    // https://stackoverflow.com/questions/28346989/drawing-and-filling-a-circle

    int w, h, dx, dy;

    for (w = 0; w < radius * 2; w++) {
        for (h = 0; h < radius * 2; h++) {

            dx = radius - w; // horizontal offset
            dy = radius - h; // vertical offset

            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(rend, x + dx, y + dy);
            }
        }
    }
}

void line(SDL_Renderer* rend, SDL_Color c, int x1, int y1, int x2, int y2)
{
    SDL_SetRenderDrawColor(rend, c.r, c.g, c.b, c.a);

    SDL_RenderDrawLine(rend, x1, y1, x2, y2);
}

} // pse