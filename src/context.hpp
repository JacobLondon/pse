#pragma once

#include <SDL2/SDL.h>
#undef main

namespace pse {

struct Context {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;
    
    struct {
        int x, y;
    } mouse;
    unsigned char *keystate;
    
    void (*setup)(Context& ctx);
    void (*update)(Context& ctx);
    
    unsigned frame_target, frame_counter;
    double delta_time;

    int screen_width, screen_height;
    const char* title;


    Context(const char *title, int w, int h, unsigned fps,
        void (*setup)(Context& ctx), void (*update)(Context& ctx));
    ~Context();
};

} // pse