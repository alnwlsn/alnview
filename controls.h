#include <SDL2/SDL.h>
#include <stdbool.h>

#include "canvas.h"
#include "images.h"

extern bool show_center_mark;            // for renderer to show center mark
extern bool show_canvas_rotation_point;  // for renderer to show another reference mark
extern bool show_image_reference_marks;
extern double canvas_rotation_point_x;
extern double canvas_rotation_point_y;
extern int selected_imi;
extern int global_testA;
extern int global_testB;
extern int global_testC;

void controls_process(SDL_Event e);