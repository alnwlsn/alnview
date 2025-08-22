#pragma once

#include "canvas.h"
#include "controls.h"
#include "images.h"
#include "loader.h"

extern double canvas_rotation_point_x;
extern double canvas_rotation_point_y;
extern int mouse_raw_last_x;
extern int mouse_raw_last_y;  // for mouse dragging positioning
extern double mouse_screen_last_x;
extern double mouse_screen_last_y;  // for mouse dragging rotation
extern double canvas_initial_rotation;  // for canvas rotation
extern double mouse_initial_angle;

void super_canvas_drag_zoom();
void super_image_drag_zoom();
void super_mouse_last(SDL_Event e);
void super_canvas_center_rotation_init();
void super_canvas_rotation_init();
void super_image_rotating();
void super_canvas_rotating_center();
void super_canvas_rotating_point();
void super_toggle_antialiasing();
void super_toggle_fullscreen();