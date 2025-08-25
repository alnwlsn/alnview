#pragma once

//handles the canvas
#include "render.h"

// view of the canvas in the window
typedef struct {
  double r;  // rotation angle in degrees
  double x;  // center X in canvas coords
  double y;  // center Y in canvas coords
  double z;  // zoom
  int selected_imi;
} CanvasView;

// globals that control the whole thing
extern CanvasView cv;
extern CanvasView cvp[];
extern int screen_size_x;
extern int screen_size_y;
extern int mouse_screen_raw_x;
extern int mouse_screen_raw_y;
extern double mouse_screen_x;
extern double mouse_screen_y;
extern double mouse_canvas_x;
extern double mouse_canvas_y;
extern double mouse_angle_about_center;

void canvas_update_cursor();  // update window size and mouse cursor

void canvas_init();                          // initializes SDL window
void canvas_render_pin(double x, double y);  // put pin marker on canvas here

void canvas_drag_screen_by(int dx, int dy);                             // pan canvas on screen by change in mouse screen coords
void canvas_zoom_by(double zoom_factor);                                // zoom canvas, centered on cursor.
void canvas_zoom_reset();                                               // zoom to 1, centered on cursor.
void canvas_rotate_about_point_by(double rx, double ry, double angle);  // rotate canvas by angle by point on canvas
void canvas_zoom_by_at_point(double cx, double cy, double zoom_factor); // zoom centered on specified canvas point
void canvas_to_screen(double canvas_x, double canvas_y, double *screen_x, double *screen_y);  // coordinate conversion

