#include <algorithm>

#include "ctx.hpp"
#include "util.hpp"

namespace pse {

int Context::load_image(const char *path)
{
    SDL_Texture *t = IMG_LoadTexture(renderer, path);
    if (!t) {
        fprintf(stderr, "Error: Invalid texture/path: '%s'\n", path);
        exit(-1);
    }
    textures.push_back(t);
    return (int)textures.size() - 1;
}

void Context::draw_image(int id, SDL_Rect rect)
{
    SDL_RenderCopy(renderer, textures[id], NULL, &rect);
}

void Context::draw_clear(SDL_Color c)
{
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderClear(renderer);
}

void Context::draw_rect(SDL_Color c, SDL_Rect rect)
{
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderDrawRect(renderer, &rect);
}

void Context::draw_rect_fill(SDL_Color c, SDL_Rect rect)
{
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(renderer, &rect);
}

void Context::draw_circle(SDL_Color c, int x, int y, int radius)
{
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

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
        SDL_RenderDrawPoint(renderer, x + cx, y - cy);
        SDL_RenderDrawPoint(renderer, x + cx, y + cy);
        SDL_RenderDrawPoint(renderer, x - cx, y - cy);
        SDL_RenderDrawPoint(renderer, x - cx, y + cy);
        SDL_RenderDrawPoint(renderer, x + cy, y - cx);
        SDL_RenderDrawPoint(renderer, x + cy, y + cx);
        SDL_RenderDrawPoint(renderer, x - cy, y - cx);
        SDL_RenderDrawPoint(renderer, x - cy, y + cx);

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

void Context::draw_circle_fill(SDL_Color c, int x, int y, int radius)
{
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

    // https://stackoverflow.com/questions/28346989/drawing-and-filling-a-circle

    int w, h, dx, dy;

    for (w = 0; w < radius * 2; w++) {
        for (h = 0; h < radius * 2; h++) {

            dx = radius - w; // horizontal offset
            dy = radius - h; // vertical offset

            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}

void Context::draw_line(SDL_Color c, int x1, int y1, int x2, int y2)
{
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

void Context::draw_tri(SDL_Color c, int x1, int y1, int x2, int y2, int x3, int y3)
{
    if (x1 == x2 == x3) return;
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    SDL_RenderDrawLine(renderer, x2, y2, x3, y3);
    SDL_RenderDrawLine(renderer, x3, y3, x1, y1);
}

void Context::draw_tri_fill(SDL_Color c, int x1, int y1, int x2, int y2, int x3, int y3)
{
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

    // https://www.youtube.com/watch?v=PahbNFypubE ~10:30

    // find the top point
    int y, t;

    // sort points
    if (y2 < y1) {
        t = x1; x1 = x2; x2 = t;
        t = y1; y1 = y2; y2 = t;
    }
    if (y3 < y1) {
        t = x1; x1 = x3; x3 = t;
        t = y1; y1 = y3; y3 = t;
    }
    if (y3 < y2) {
        t = x2; x2 = x3; x3 = t;
        t = y2; y2 = y3; y3 = t;
    }

    // triangle has no area
    if (y1 == y3) return;

    if (y1 < y2) {
        //slopes[shortside] = (y2 - y1) / (x1 - x2);
        for (y = y1; y < y2; y++) {
            SDL_RenderDrawLine(renderer, x1 + (x2 - x1) * (y - y1) / (y2 - y1), y, x1 + (x3 - x1) * (y - y1) / (y3 - y1), y);
        }
    }
    if (y2 < y3) {
        for (y = y2; y < y3; y++) {
            SDL_RenderDrawLine(renderer, x2 + (x3 - x2) * (y - y2) / (y3 - y2), y, x1 + (x3 - x1) * (y - y1) / (y3 - y1), y);
        }
    }
}

} // pse