#pragma once

#include <SDL2/SDL.h>

//view of the canvas in the window
typedef struct {
    float r;  // rotation angle in degrees
    float x;  // center X in canvas coords
    float y;  // center Y in canvas coords
    float z;  // zoom
} CanvasView;

// extern float canvas_rx, canvas_ry;
// extern float ax, ay, bx, by;

void canvas_init();
CanvasView canvas_current();
// void canvas_set_rotation_center(float cx, float cy);
// void canvas_apply_rotation(float dr);
// void canvas_reset();
// void canvas_move(float dx, float dy);
// void canvas_zoom(float factor, float mouse_screen_x, float mouse_screen_y);

// void canvas_start_animation();
// int canvas_animate_step();