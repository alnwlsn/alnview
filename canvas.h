#pragma once
// makes the SDL window and handles the canvas
#include <SDL2/SDL.h>

#include "defines.h"

// view of the canvas in the window
typedef struct {
  float r;  // rotation angle in degrees
  float x;  // center X in canvas coords
  float y;  // center Y in canvas coords
  float z;  // zoom
} CanvasView;

// globals that control the whole thing
extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern CanvasView cv;
extern int screen_size_x;
extern int screen_size_y;
extern float mouse_screen_x;
extern float mouse_screen_y;
extern float mouse_canvas_x;
extern float mouse_canvas_y;
extern float mouse_angle_about_center;

void canvas_update_cursor();  // update window size and mouse cursor

void canvas_init();                        // initializes SDL window
void canvas_render_pin(float x, float y);  // put pin marker on canvas here

void canvas_drag_screen_by(int dx, int dy);                          // pan canvas on screen by change in mouse screen coords
void canvas_zoom_by(float zoom_factor);                           // zoom canvas, centered on cursor.
void canvas_rotate_about_point_by(float rx, float ry, float angle);  // rotate canvas by angle by point on canvas
