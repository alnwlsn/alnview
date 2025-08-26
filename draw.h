#pragma once
#include <SDL2/SDL.h>

#include "render.h"

typedef struct {
  double x;
  double y;
  int t;
  double z;
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
void draw_forward_pen();

void draw_pick_open();
void draw_pick_close();