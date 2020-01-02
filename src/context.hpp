#pragma once

#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#undef main

namespace pse {

struct Context {
    // SDL bindings
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;

    // input devices
    struct {
        int x, y;
    } mouse;
    unsigned char *keystate;

    // frame stats
    unsigned frame_target, frame_counter;
    double delta_time;

    // window data
    int screen_width, screen_height;
    const char* title;
    bool done;

    Context(const char *title, int w, int h, unsigned fps,
        void (*setup)(Context& ctx), void (*update)(Context& ctx),
        time_t *seed = 0);
    ~Context();
    bool check_key(int sdl_scancode);
    bool check_key_invalidate(int sdl_scancode);
    void quit();
};

} // pse