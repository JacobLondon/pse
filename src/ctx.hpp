#pragma once

#include <time.h>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#undef main

namespace pse {

// 4:3
#define PSE_RESOLUTION_43_640_480 640, 480
#define PSE_RESOLUTION_43_800_600 800, 600
#define PSE_RESOLUTION_43_1024_768 1024, 768

// 16:9
#define PSE_RESOLUTION_169_1280_720 1280, 720
#define PSE_RESOLUTION_169_1360_768 1360, 768
#define PSE_RESOLUTION_169_1366_768 1366, 768
#define PSE_RESOLUTION_169_1600_900 1600, 900
#define PSE_RESOLUTION_169_1920_1080 1920, 1080

struct Context {
    // SDL bindings
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;
    std::vector<SDL_Texture *> textures;

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

    int load_image(const char *path); // put an image into textures, return its ID
    void draw_image(int id, SDL_Rect rect); // draw an image to coordinates
    void draw_clear(SDL_Color c); // clear entire surface
    void draw_rect(SDL_Color c, SDL_Rect rect); // draw rectangle outline
    void draw_rect_fill(SDL_Color c, SDL_Rect rect); // draw filled rectangle
    void draw_circle(SDL_Color c, int x, int y, int radius); // draw circle outline
    void draw_circle_fill(SDL_Color c, int x, int y, int radius); // draw filled circle
    void draw_line(SDL_Color c, int x1, int y1, int x2, int y2); // draw a line
    void draw_tri(SDL_Color c, int x1, int y1, int x2, int y2, int x3, int y3);
    void draw_tri_fill(SDL_Color c, int x1, int y1, int x2, int y2, int x3, int y3);
    void draw_tri_fast_square(SDL_Color c, int x1, int y1, int x2, int y2, int x3, int y3);
    void draw_tri_fast_depth(SDL_Color c, int x1, int y1, int x2, int y2, int x3, int y3, int depth);
};

} // pse