#include "canvas.h"
#include <SDL2/SDL.h>

extern int show_center_mark; //for renderer to show center mark
extern int show_reference_mark; //for renderer to show another reference mark
extern float reference_mark_x;
extern float reference_mark_y;

void controls_process(SDL_Event e);