#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "render.h"

typedef struct {
  double x;
  double y;
  int type;
  int thickness;
} DrawPoint;

#define draw_points_max 100000
extern DrawPoint draw_points[draw_points_max];
extern int draw_len;

void draw_init();
void draw_render();

void draw_lift_pen();
void draw_drop_pen(double x, double y);
void draw_move_pen(double x, double y);
void draw_back_pen();