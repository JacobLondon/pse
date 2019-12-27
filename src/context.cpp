#include <cstdio>
#include <chrono>
#include <thread>

#include "context.hpp"
#include "draw.hpp"

namespace pse {

Context::Context(const char* title, int w, int h, unsigned fps,
    void (*setup)(Context& ctx), void (*update)(Context& ctx))
    : window{ nullptr }, renderer{ nullptr }, event{},
      mouse{ 0, 0 }, keystate{ nullptr },
      frame_target{ fps }, frame_counter{ 0 }, delta_time{ 1.0 },
      screen_width{ w }, screen_height{ h }, title{ title }
{
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        screen_width, screen_height,
        SDL_WINDOW_SHOWN
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

    /**
     * Utility
     */

    auto time_now = []() {
        return std::chrono::high_resolution_clock::now();
    };
    auto time_in_us = [](auto time) {
        return std::chrono::duration_cast<std::chrono::microseconds>(time).count();
    };
    auto sleep_us = [](auto time) {
        std::this_thread::sleep_for(std::chrono::microseconds((long long)(time)));
    };

    /**
     * Update
     */

    double frame_time, frame_time_target = 1.0 / frame_target * 1000000.0;
    auto frame_time_next = time_now(),
         frame_time_diff = time_now();

    setup(*this);

    while (true) {
        // keys
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                goto Quit;
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
        auto frame_time_diff = time_now() - frame_time_next;
        frame_time_next = time_now();
        frame_time = time_in_us(frame_time_diff);
        if (frame_time_target - frame_time > 0)
            sleep_us(frame_time_target - frame_time);

        frame_counter = (frame_counter + 1) % frame_target;
        delta_time = frame_time / 1000000.0;
    }

Quit:
    return;
}

Context::~Context()
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

} // pse