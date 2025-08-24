#include "draw.h"

#include <math.h>

int xp = 0;
int yp = 0;

void draw_line_thickness(SDL_Renderer *renderer, double x1, double y1, double x2, double y2, double thickness) {
  double angle = atan2(y2 - y1, x2 - x1);
  double dx = sin(angle) * (thickness / 2.0);
  double dy = cos(angle) * (thickness / 2.0);

  SDL_Vertex verts[4] = {{{x1 - dx, y1 + dy}, {255, 255, 255, 255}, {0, 0}},
                         {{x2 - dx, y2 + dy}, {255, 255, 255, 255}, {0, 0}},
                         {{x2 + dx, y2 - dy}, {255, 255, 255, 255}, {0, 0}},
                         {{x1 + dx, y1 - dy}, {255, 255, 255, 255}, {0, 0}}};
  int indices[6] = {0, 1, 2, 2, 3, 0};

  SDL_RenderGeometry(renderer, NULL, verts, 4, indices, 6);
}

void draw_line_thickness_canvas(SDL_Renderer *renderer, double x1, double y1, double x2, double y2, double thickness) {
  double x1s, x2s, y1s, y2s;
  canvas_to_screen(x1, y1, &x1s, &y1s);
  canvas_to_screen(x2, y2, &x2s, &y2s);
  x1s += (screen_size_x / 2);
  x2s += (screen_size_x / 2);
  y1s = -y1s;
  y2s = -y2s;
  y1s += (screen_size_y / 2);
  y2s += (screen_size_y / 2);
  global_testA = thickness * cv.z;
  global_testB = y2s;
  draw_line_thickness(renderer, x1s, y1s, x2s, y2s, global_testA);
}

void draw_render() { draw_line_thickness_canvas(renderer, 0, 0, xp, yp, 3); }

void draw_append_point(int x, int y) {
  xp = x;
  yp = y;
}