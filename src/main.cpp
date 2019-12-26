#include <cstdio>

#include <chrono>
#include <iostream>
#include <thread>

#include <SDL2/SDL.h>
#undef main

#include "draw.hpp"

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow(
        "PSE",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 480,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "Error: Failed to initialize SDL Window\n");
        exit(-1);
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error: Failed to initialize SDL Renderer\n");
        exit(-1);
    }

    struct {
        int x, y;
    } mouse;
    unsigned char* keystate = (unsigned char *)SDL_GetKeyboardState(NULL);

    unsigned frame_counter = 0,
        frame_target = 60;
    double frame_time, frame_time_target = 1.0 / frame_target * 1000000.0;
    auto frame_time_next = std::chrono::high_resolution_clock::now(),
        frame_time_diff = std::chrono::high_resolution_clock::now();

    SDL_Event e;
    while (true) {
        // keys
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
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
        pse::rect_fill(renderer, pse::Red, SDL_Rect{ mouse.x - 50, mouse.y - 50, 100, 100 });
        SDL_RenderPresent(renderer);
        
        // frame management
        auto frame_time_diff = std::chrono::high_resolution_clock::now() - frame_time_next;
        frame_time_next = std::chrono::high_resolution_clock::now();
        frame_time = std::chrono::duration_cast<std::chrono::microseconds>(frame_time_diff).count();
        if (frame_time_target - frame_time > 0)
            std::this_thread::sleep_for(std::chrono::microseconds((long long)(frame_time_target - frame_time)));
        frame_counter = (frame_counter + 1) % frame_target;
        printf("frame: %2d / %2d | fps: %4.2lf\r", frame_counter, frame_target, 1.0 / ((double)frame_time / 1000000.0));
    }

Quit:
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();

    return 0;
}
