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

void super_canvas_drag_zoom();
void super_image_drag_zoom();