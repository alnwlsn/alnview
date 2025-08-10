#include "canvas.h"

#include <math.h>

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
float mouse_angle_about_center;

void canvas_update_cursor() {
  int mouseX_raw, mouseY_raw;
  SDL_GetMouseState(&mouseX_raw, &mouseY_raw);
  SDL_GetWindowSize(window, &screen_size_x, &screen_size_y);

  mouse_screen_x = mouseX_raw - (screen_size_x / 2.0f);  // mouse screen coords, with 0 at center on screen
  mouse_screen_y = -mouseY_raw + (screen_size_y / 2.0f);

  mouse_canvas_x = (mouse_screen_x / cv.z) * cos(-cv.r * M_PI / 180) - (mouse_screen_y / cv.z) * sin(-cv.r * M_PI / 180);  // convert mouse to canvas coords
  mouse_canvas_y = (mouse_screen_y / cv.z) * cos(-cv.r * M_PI / 180) + (mouse_screen_x / cv.z) * sin(-cv.r * M_PI / 180);
  mouse_canvas_x += cv.x;
  mouse_canvas_y += cv.y;

  mouse_angle_about_center = (180 / M_PI) * atan2(mouse_screen_y, mouse_screen_x);
}

void canvas_init() {
  window = SDL_CreateWindow("alnview", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_INIT_X, WINDOW_INIT_Y,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);  //| SDL_WINDOW_MAXIMIZED);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  // globals that need initalized
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

float rx = 128, ry = -128;  // point to rotate canvas about

// float ax = 0, ay = 0, bx = 0, by = 0;  // debugging

// static int animation = 0;
// static int animation_step = 0;
// static const int animation_steps = 50;

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

void canvas_drag_screen_by(int dx, int dy) {
  // pan screen by mouse coords
  float sdx = dx / cv.z;
  float sdy = dy / cv.z;
  float cdx = sdx * cos(-cv.r * M_PI / 180) - sdy * sin(-cv.r * M_PI / 180);
  float cdy = sdy * cos(-cv.r * M_PI / 180) + sdx * sin(-cv.r * M_PI / 180);
  cv.x -= cdx;
  cv.y -= cdy;
}

void canvas_zoom_by(float zoom_factor) {
  // zoom centered on mouse position
  // find old and new position; offset view accordingly
  float scx1 = (mouse_screen_x / cv.z);
  float scy1 = (mouse_screen_y / cv.z);
  cv.z *= zoom_factor;
  float scx2 = (mouse_screen_x / cv.z);
  float scy2 = (mouse_screen_y / cv.z);
  float sdx = scx2 - scx1;
  float sdy = scy2 - scy1;
  float cdx = sdx * cos(-cv.r * M_PI / 180) - sdy * sin(-cv.r * M_PI / 180);
  float cdy = sdy * cos(-cv.r * M_PI / 180) + sdx * sin(-cv.r * M_PI / 180);
  cv.x -= cdx;
  cv.y -= cdy;
}

void canvas_rotate_about_point_by(float rx, float ry, float angle) {  // rotate canvas by angle about point on canvas
  cv.r -= angle;
  float sdx = rx - cv.x;
  float sdy = ry - cv.y;
  float cx = sdx * cos(angle * M_PI / 180) - sdy * sin(angle * M_PI / 180) + cv.x;
  float cy = sdy * cos(angle * M_PI / 180) + sdx * sin(angle * M_PI / 180) + cv.y;
  cv.x = cv.x - (cx - rx);
  cv.y = cv.y - (cy - ry);
}
