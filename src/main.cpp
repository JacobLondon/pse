#include <cstdio>

#include <SDL2/SDL.h>
#undef main

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

    SDL_Event e;
    while (true) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                goto Quit;
            default:
                break;
            }
        }
    }

Quit:
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();

    return 0;
}
