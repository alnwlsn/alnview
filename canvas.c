#include "canvas.h"


#include <math.h>
#include <stdio.h>
#include "defines.h"

CanvasView cv;  // current view
CanvasView waypt[MAX_CANVAS];

float canvas_rx = 128, canvas_ry = -128;  // point to rotate canvas about

float ax = 0, ay = 0, bx = 0, by = 0;  // debugging

static int animation = 0;
static int animation_step = 0;
static const int animation_steps = 50;

void canvas_init() {
  cv.x = 0;
  cv.y = 0;
  cv.r = 0;
  cv.z = 1.0f;
}

void canvas_set_rotation_center(float cx, float cy) {
  canvas_rx = cx;
  canvas_ry = cy;
}

void canvas_apply_rotation(float dr) {
  cv.r -= dr;
  float sdx = canvas_rx - cv.x;
  float sdy = canvas_ry - cv.y;
  float angle = dr * M_PI / 180.0f;
  float cx = sdx * cos(angle) - sdy * sin(angle) + cv.x;
  float cy = sdy * cos(angle) + sdx * sin(angle) + cv.y;

  ax = cx;
  ay = cy;
  bx = cv.x - (cx - canvas_rx);
  by = cv.y - (cy - canvas_ry);

  cv.x = bx;
  cv.y = by;

  printf("canvas rotated %.1f deg, new center: %.1f, %.1f\n", dr, bx, by);
}

void canvas_reset() {
  cv.r = 0.0f;
  cv.x = 0.0f;
  cv.y = 0.0f;
  cv.z = 1.0f;
}

void canvas_move(float dx, float dy) {
  float angle = -cv.r * M_PI / 180.0f;
  float cdx = dx * cos(angle) - dy * sin(angle);
  float cdy = dy * cos(angle) + dx * sin(angle);
  cv.x -= cdx;
  cv.y -= cdy;
}

void canvas_zoom(float factor, float mouse_screen_x, float mouse_screen_y) {
  float scx1 = mouse_screen_x / cv.z;
  float scy1 = mouse_screen_y / cv.z;

  cv.z *= factor;

  float scx2 = mouse_screen_x / cv.z;
  float scy2 = mouse_screen_y / cv.z;

  float sdx = scx2 - scx1;
  float sdy = scy2 - scy1;

  float angle = -cv.r * M_PI / 180.0f;
  float cdx = sdx * cos(angle) - sdy * sin(angle);
  float cdy = sdy * cos(angle) + sdx * sin(angle);
  cv.x -= cdx;
  cv.y -= cdy;
}

void canvas_start_animation() {
  waypt[0] = cv;
  animation = 1;
  animation_step = 0;
}

int canvas_animate_step() {
  if (!animation) return 0;
  if (animation_step >= animation_steps) {
    animation = 0;
    return 0;
  }

  float t = animation_step / (float)animation_steps;
  cv.r = waypt[0].r + t * (waypt[1].r - waypt[0].r);
  cv.z = waypt[0].z + t * (waypt[1].z - waypt[0].z);
  cv.x = waypt[0].x + t * (waypt[1].x - waypt[0].x);
  cv.y = waypt[0].y + t * (waypt[1].y - waypt[0].y);

  animation_step++;
  return 1;
}

CanvasView canvas_current() { return cv; }

void canvas_render_marker(SDL_Window *window, SDL_Renderer *renderer, float x, float y){

}