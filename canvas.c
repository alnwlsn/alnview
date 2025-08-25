#include "canvas.h"

CanvasView cv;  // current view
CanvasView cvp[MAX_CANVAS];
int screen_size_x;
int screen_size_y;
int mouse_screen_raw_x;
int mouse_screen_raw_y;
double mouse_screen_x;
double mouse_screen_y;
double mouse_canvas_x;
double mouse_canvas_y;
double mouse_angle_about_center;

void canvas_update_cursor() {
  SDL_GetMouseState(&mouse_screen_raw_x, &mouse_screen_raw_y);
  SDL_GetWindowSize(window, &screen_size_x, &screen_size_y);

  mouse_screen_x = mouse_screen_raw_x - (screen_size_x / 2.0f);  // mouse screen coords, with 0 at center on screen
  mouse_screen_y = -mouse_screen_raw_y + (screen_size_y / 2.0f);

  mouse_canvas_x = (mouse_screen_x / cv.z) * cos(-cv.r * M_PI / 180) - (mouse_screen_y / cv.z) * sin(-cv.r * M_PI / 180);  // convert mouse to canvas coords
  mouse_canvas_y = (mouse_screen_y / cv.z) * cos(-cv.r * M_PI / 180) + (mouse_screen_x / cv.z) * sin(-cv.r * M_PI / 180);
  mouse_canvas_x += cv.x;
  mouse_canvas_y += cv.y;

  mouse_angle_about_center = (180 / M_PI) * atan2(mouse_screen_y, mouse_screen_x);
}

void canvas_to_screen(double canvas_x, double canvas_y, double *screen_x, double *screen_y) {
  double sdx = canvas_x - cv.x;
  double sdy = canvas_y - cv.y;
  double cx = sdx * cos(cv.r * M_PI / 180) - sdy * sin(cv.r * M_PI / 180);
  double cy = sdy * cos(cv.r * M_PI / 180) + sdx * sin(cv.r * M_PI / 180);
  *screen_x = cx * cv.z;
  *screen_y = cy * cv.z;
}

void canvas_init() {
  // globals that need initalized
  cv.x = 0;
  cv.y = 0;
  cv.r = 0;
  cv.z = 1.0f;
  cv.selected_imi = 0;

  for (int i = 0; i < MAX_CANVAS; i++) {
    cvp[i].x = 0;
    cvp[i].y = 0;
    cvp[i].r = 0;
    cvp[i].z = 1.0f;
    cv.selected_imi = 0;
  }

  canvas_update_cursor();
}

void canvas_render_pin(double x, double y) {
  double lX = x * cv.z;
  double lY = -y * cv.z;
  double tX = lX * cos(-cv.r * M_PI / 180) - lY * sin(-cv.r * M_PI / 180);
  double tY = lY * cos(-cv.r * M_PI / 180) + lX * sin(-cv.r * M_PI / 180);
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
  double sdx = dx / cv.z;
  double sdy = dy / cv.z;
  double cdx = sdx * cos(-cv.r * M_PI / 180) - sdy * sin(-cv.r * M_PI / 180);
  double cdy = sdy * cos(-cv.r * M_PI / 180) + sdx * sin(-cv.r * M_PI / 180);
  cv.x -= cdx;
  cv.y -= cdy;
}

void canvas_zoom_by(double zoom_factor) {
  // zoom centered on mouse position
  // find old and new position; offset view accordingly
  double scx1 = (mouse_screen_x / cv.z);
  double scy1 = (mouse_screen_y / cv.z);
  cv.z *= zoom_factor;
  double scx2 = (mouse_screen_x / cv.z);
  double scy2 = (mouse_screen_y / cv.z);
  double sdx = scx2 - scx1;
  double sdy = scy2 - scy1;
  double cdx = sdx * cos(-cv.r * M_PI / 180) - sdy * sin(-cv.r * M_PI / 180);
  double cdy = sdy * cos(-cv.r * M_PI / 180) + sdx * sin(-cv.r * M_PI / 180);
  cv.x -= cdx;
  cv.y -= cdy;
}

void canvas_zoom_by_at_point(double cx, double cy, double zoom_factor) {
  // zoom centered on specific canvas point
  // find old and new position; offset view accordingly
  double sx, sy;
  canvas_to_screen(cx, cy, &sx, &sy);
  double scx1 = (sx / cv.z);
  double scy1 = (sy / cv.z);
  cv.z *= zoom_factor;
  double scx2 = (sx / cv.z);
  double scy2 = (sy / cv.z);
  double sdx = scx2 - scx1;
  double sdy = scy2 - scy1;
  double cdx = sdx * cos(-cv.r * M_PI / 180) - sdy * sin(-cv.r * M_PI / 180);
  double cdy = sdy * cos(-cv.r * M_PI / 180) + sdx * sin(-cv.r * M_PI / 180);
  cv.x -= cdx;
  cv.y -= cdy;
}

void canvas_zoom_reset() {
  canvas_zoom_by(1 / cv.z);
  cv.z = 1;
}

void canvas_rotate_about_point_by(double rx, double ry, double angle) {  // rotate canvas by angle about point on canvas
  cv.r -= angle;
  double sdx = rx - cv.x;
  double sdy = ry - cv.y;
  double cx = sdx * cos(angle * M_PI / 180) - sdy * sin(angle * M_PI / 180) + cv.x;
  double cy = sdy * cos(angle * M_PI / 180) + sdx * sin(angle * M_PI / 180) + cv.y;
  cv.x = cv.x - (cx - rx);
  cv.y = cv.y - (cy - ry);
}
