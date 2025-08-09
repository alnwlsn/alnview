#include "canvas.h"

#include <math.h>
#include <stdio.h>

#include "defines.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
CanvasView cv;  // current view
CanvasView waypt[MAX_CANVAS];
int screen_size_x;
int screen_size_y;
float mouse_screen_x;
float mouse_screen_y;
float mouse_canvas_x;
float mouse_canvas_y;

void canvas_update_cursor(){
    int mouseX_raw, mouseY_raw;
    SDL_GetMouseState(&mouseX_raw, &mouseY_raw);
    SDL_GetWindowSize(window, &screen_size_x, &screen_size_y);

    mouse_screen_x = mouseX_raw - (screen_size_x / 2.0f);  // mouse screen coords, with 0 at center on screen
    mouse_screen_y = -mouseY_raw + (screen_size_y / 2.0f);

    mouse_canvas_x =
        (mouse_screen_x / cv.z) * cos(-cv.r * M_PI / 180) - (mouse_screen_y / cv.z) * sin(-cv.r * M_PI / 180);  // convert mouse to canvas coords
    mouse_canvas_y = (mouse_screen_y / cv.z) * cos(-cv.r * M_PI / 180) + (mouse_screen_x / cv.z) * sin(-cv.r * M_PI / 180);
    mouse_canvas_x += cv.x;
    mouse_canvas_y += cv.y;
}



void canvas_init() {
  window = SDL_CreateWindow("alnview", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_INIT_X, WINDOW_INIT_Y,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);  //| SDL_WINDOW_MAXIMIZED);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  cv.x = 0;
  cv.y = 0;
  cv.r = 0;
  cv.z = 1.0f;
  for (int i = 0; i < MAX_CANVAS; i++) {
    waypt[i].x = 0;
    waypt[i].y = 0;
    waypt[i].r = 0;
    waypt[i].z = 1.0f;
  }
}

float canvas_rx = 128, canvas_ry = -128;  // point to rotate canvas about

float ax = 0, ay = 0, bx = 0, by = 0;  // debugging

static int animation = 0;
static int animation_step = 0;
static const int animation_steps = 50;

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

void canvas_render_pin(float x, float y) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  float lX = x * cv.z;
  float lY = -y * cv.z;
  float tX = lX * cos(-cv.r * M_PI / 180) - lY * sin(-cv.r * M_PI / 180);
  float tY = lY * cos(-cv.r * M_PI / 180) + lX * sin(-cv.r * M_PI / 180);
  tX += screen_size_x / 2.0f;
  tY += screen_size_y / 2.0f;
  tX -= cv.x * cv.z * cos(-cv.r * M_PI / 180) + cv.y * cv.z * sin(-cv.r * M_PI / 180);
  tY -= -cv.y * cv.z * cos(-cv.r * M_PI / 180) + cv.x * cv.z * sin(-cv.r * M_PI / 180);

  SDL_Rect dot = {(int)(tX), (int)(tY), (int)fmax(1, 5),  // scale dot size
                  (int)fmax(1, 2)};
  SDL_RenderFillRect(renderer, &dot);
  SDL_Rect dot2 = {(int)(tX), (int)(tY), (int)fmax(1, 2),  // scale dot size
                   (int)fmax(1, 5)};
  SDL_RenderFillRect(renderer, &dot2);
}