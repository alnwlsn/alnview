#include "canvas.h"
#include "images.h"
#include <SDL2/SDL.h>

extern int show_center_mark; //for renderer to show center mark
extern int show_canvas_rotation_point; //for renderer to show another reference mark
extern float canvas_rotation_point_x;
extern float canvas_rotation_point_y;
extern int global_testA;
extern int global_testB;
extern int global_testC;

void controls_process(SDL_Event e);