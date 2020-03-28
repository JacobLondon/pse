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

class Context {
private:
    // SDL bindings
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Event event = {0};
public:
    std::vector<SDL_Texture *> textures{};

    // input devices
    struct {
        int x;
        int y;
    } mouse = {
        0, 0
    };
    unsigned char *keystate = nullptr;

    // frame stats
private:
    double frame_time_target = 0.0;
public:
    size_t frame_target = 60;
    size_t frame_counter = 0;
    double delta_time = 1.0;

    // window data
    int screen_width = 640;
    int screen_height = 480;
    const char* title = nullptr;
    bool done = false;

    Context(const char *title, int w, int h, size_t fps);
    ~Context();
    void set_window(const char *title, int w, int h, unsigned int flags);
    void run(void (*setup)(Context& ctx), void (*update)(Context& ctx));
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
private:
    void set_frame_target(size_t target);
};

} // pse