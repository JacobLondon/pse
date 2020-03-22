#include "ctx.hpp"
#include "ctx_draw.hpp"
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
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    SDL_RenderDrawLine(renderer, x2, y2, x3, y3);
    SDL_RenderDrawLine(renderer, x3, y3, x1, y1);
}

void Context::draw_tri_fill(SDL_Color c, int x1, int y1, int x2, int y2, int x3, int y3)
{
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

    double mag = (double)fast_sqrtf((float)((x2 - x1) * (x2 - x1)) + (float)((y2 - y1) * (y2 - y1)));
    double xstep = (double)(x2 - x1) / mag;
    double ystep = (double)(y2 - y1) / mag;
    double x, y;
    for (x = x1, y = y1; (int)x != x2 && (int)y != y2; x += xstep, y += ystep) {
        SDL_RenderDrawLine(renderer, (int)x, (int)y, x3, y3);
    }

    mag = (double)fast_sqrtf((float)((x3 - x2) * (x3 - x2)) + (float)((y3 - y2) * (y3 - y2)));
    xstep = (double)(x3 - x2) / mag;
    ystep = (double)(y3 - y2) / mag;
    for (x = x2, y = y2; (int)x != x3 && (int)y != y3; x += xstep, y += ystep) {
        SDL_RenderDrawLine(renderer, (int)x, (int)y, x1, y1);
    }

    mag = (double)fast_sqrtf((float)((x1 - x3) * (x1 - x3)) + (float)((y1 - y3) * (y1 - y3)));
    xstep = (double)(x1 - x3) / mag;
    ystep = (double)(y1 - y3) / mag;
    for (x = x3, y = y3; (int)x != x1 && (int)y != y1; x += xstep, y += ystep) {
        SDL_RenderDrawLine(renderer, (int)x, (int)y, x2, y2);
    }
}

} // pse