#pragma once

// misc and helper functions for controls

#include "canvas.h"
#include "controls.h"
#include "images.h"
#include "loader.h"
#include "draw.h"

extern int mouse_raw_last_x;
extern int mouse_raw_last_y;  // for mouse dragging positioning
extern double mouse_screen_last_x;
extern double mouse_screen_last_y;      // for mouse dragging rotation
extern double canvas_initial_rotation;  // for canvas rotation
extern double mouse_initial_angle;
extern double canvas_rotation_point_x;
extern double canvas_rotation_point_y;

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
void super_opacity_increase();
void super_opacity_decrease();
void super_reload_single_image(int imi);