#include "draw.h"

#include <math.h>

double xp = 0;
double yp = 0;

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

void draw_circle(double x, double y, double r, int n){

        // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        SDL_Vertex verts[n];
        int indices[3*(n-2)]; // triangle indices for triangle fan

        // Center vertex (used for triangle fan)
        SDL_FPoint center = {x, y};

        for (int i = 0; i < n; i++) {
            float angle = 2.0f * M_PI * i / n;
            verts[i].position.x = x + r * cosf(angle);
            verts[i].position.y = y + r * sinf(angle);
            verts[i].color.r = 255;
            verts[i].color.g = 255;
            verts[i].color.b = 255;
            verts[i].color.a = 255;
            verts[i].tex_coord.x = 0; // not used
            verts[i].tex_coord.y = 0; // not used
        }

        // Build indices for triangle fan
        for (int i = 0; i < n-2; i++) {
            indices[3*i + 0] = 0;     // center of fan
            indices[3*i + 1] = i+1;
            indices[3*i + 2] = i+2;
        }

        // Draw triangle fan
        SDL_RenderGeometry(renderer, NULL, verts, n, indices, 3*(n-2));
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
  global_testC = y2;
  draw_line_thickness(renderer, x1s, y1s, x2s, y2s, global_testA);
}

void draw_render() {
    draw_circle(100,100,60,10);
}

void draw_append_point(double x, double y) {
  xp = x;
  yp = y;
}