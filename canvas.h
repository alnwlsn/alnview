#pragma once
//makes the SDL window and handles the canvas

#include <SDL2/SDL.h>

//view of the canvas in the window
typedef struct {
    float r;  // rotation angle in degrees
    float x;  // center X in canvas coords
    float y;  // center Y in canvas coords
    float z;  // zoom
} CanvasView;

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern CanvasView cv;

// extern float canvas_rx, canvas_ry;
// extern float ax, ay, bx, by;

void canvas_init();
void canvas_render_marker(float x, float y);
// void canvas_set_rotation_center(float cx, float cy);
// void canvas_apply_rotation(float dr);
// void canvas_reset();
// void canvas_move(float dx, float dy);
// void canvas_zoom(float factor, float mouse_screen_x, float mouse_screen_y);

// void canvas_start_animation();
// int canvas_animate_step();