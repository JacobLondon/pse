#include <cstdio>
#include <chrono>
#include <thread>

#include "ctx.hpp"
#include "colors.hpp"

namespace pse {

#define US_PER_S 1000000.0

Context::Context(const char* title, int w, int h, size_t fps)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    
    set_window(title, w, h, SDL_WINDOW_SHOWN);

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
        fprintf(stderr, "Error: Failed to initialize SDL_image: %s\n", IMG_GetError());
        exit(-1);
    }

    set_frame_target(fps);
}

void Context::set_window(const char *title, int w, int h, unsigned int flags)
{
    if (window) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }

    window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        screen_width, screen_height,
        flags
    );
    if (!window) {
        fprintf(stderr, "Error: Failed to initialize SDL Window\n");
        exit(-1);
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error: Failed to initialize SDL Renderer\n");
        exit(-1);
    }
}

void Context::run(void (*setup)(Context& ctx), void (*update)(Context& ctx))
{
    srand(time(0));

    auto time_now = []() {
        return std::chrono::high_resolution_clock::now();
    };
    auto time_in_us = [](auto time) {
        return std::chrono::duration_cast<std::chrono::microseconds>(time).count();
    };
    auto sleep_us = [](auto time) {
        std::this_thread::sleep_for(std::chrono::microseconds((long long)(time)));
    };

    double frame_time = 0.0;
    auto frame_time_next = time_now();
    auto frame_time_diff = time_now() - frame_time_next;

    setup(*this);

    while (!done) {
        // keys
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                done = true;
                break;
            default:
                break;
            }
        }
        SDL_GetMouseState(&mouse.x, &mouse.y);
        SDL_PumpEvents();
        keystate = (unsigned char*)SDL_GetKeyboardState(NULL);

        // drawing
        SDL_SetRenderDrawColor(renderer, pse::Black.r, pse::Black.g, pse::Black.b, pse::Black.a);
        SDL_RenderClear(renderer);
        update(*this);
        SDL_RenderPresent(renderer);

        // frame management
        frame_time_diff = time_now() - frame_time_next;
        frame_time_next = time_now();
        frame_time = time_in_us(frame_time_diff);
        if (frame_time_target - frame_time > 0)
            sleep_us(frame_time_target - frame_time);

        frame_counter = (frame_counter + 1) % frame_target;
        delta_time = frame_time / US_PER_S;
    }

    return;
}

Context::~Context()
{
    for (auto t: textures)
        SDL_DestroyTexture(t);
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool Context::check_key(int sdl_scancode)
{
    return keystate[sdl_scancode];
}

bool Context::check_key_invalidate(int sdl_scancode)
{
    bool pressed = keystate[sdl_scancode];
    keystate[sdl_scancode] = 0;
    return pressed;
}

void Context::quit()
{
    done = true;
}

void Context::set_frame_target(size_t target)
{
    frame_target = target;
    frame_counter = 0;
    frame_time_target = 1.0 / target * US_PER_S;
}

} // pse