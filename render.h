#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#include "canvas.h"
#include "defines.h"
#include "images.h"
#include "super.h"

void render_init();
void render_text(char *coordText, int x, int y);
void render_canvas();
void render_text_screen(char *text);

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern TTF_Font *font;
extern char coordText[600];

// render options
extern bool show_center_mark;            // for renderer to show center mark
extern bool show_canvas_rotation_point;  // for renderer to show another reference mark
extern bool show_image_reference_marks;

// extern double global_testA;
// extern double global_testB;
// extern double global_testC;
// extern double global_testD;
// extern double imrefAx;
// extern double imrefAy;
// extern double imrefBx;
// extern double imrefBy;
// extern double imrefCx;
// extern double imrefCy;
// extern double imrefDx;
// extern double imrefDy;