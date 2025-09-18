#include "draw.h"

#include <math.h>

#define dp_none 0
#define dp_lift 1
#define dp_move 2
#define dp_drop 3
#define dp_color 4
#define dp_thickness 5
#define dp_alpha 6
#define dp_opaque 7

#define draw_default_thickness 3

DrawPoint draw_points[draw_points_max];
int draw_len = 0;

double draw_re_last_x = 0;
double draw_re_last_y = 0;
double draw_re_test_x = 0;
double draw_re_test_y = 0;
int draw_re_color_red = 255;
int draw_re_color_blue = 255;
int draw_re_color_green = 255;
int draw_re_color_alpha = 255;
int draw_re_thickness = 3;
bool draw_re_alpha = 0;
bool draw_re_test = 0;

CanvasView draw_cv_last;
int draw_len_last = -1;
SDL_Texture *draw_tex = NULL;
SDL_Texture *draw_tex_a = NULL;

bool draw_pick = 0;
#define pick_n 17
#define pick_w 20
#define pick_b 2

void draw_add(double x, double y, double z, int t) {
  if (draw_len >= draw_points_max) return;
  draw_points[draw_len].x = x;
  draw_points[draw_len].y = y;
  draw_points[draw_len].z = z;
  draw_points[draw_len].t = t;
  draw_len++;
}

void draw_init() {
  for (int i = 0; i < draw_points_max; i++) {
    draw_points[i].t = dp_none;
    draw_points[i].x = 0;
    draw_points[i].y = 0;
    draw_points[i].z = 0;
  }
  draw_cv_last = cv;
  draw_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA4444, SDL_TEXTUREACCESS_TARGET, MAX_SCREEN_X, MAX_SCREEN_Y);
  draw_tex_a = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA4444, SDL_TEXTUREACCESS_TARGET, MAX_SCREEN_X, MAX_SCREEN_Y);
  SDL_SetTextureBlendMode(draw_tex, SDL_BLENDMODE_BLEND);
  SDL_SetTextureAlphaMod(draw_tex, 255);
  SDL_SetRenderTarget(renderer, draw_tex);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  SDL_SetTextureBlendMode(draw_tex_a, SDL_BLENDMODE_MUL);
  SDL_SetRenderTarget(renderer, draw_tex_a);
  SDL_SetTextureAlphaMod(draw_tex_a, 128);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  SDL_SetRenderTarget(renderer, NULL);
}

bool line_intersects_rect(float x1, float y1, float x2, float y2, float rect_x, float rect_y, float w, float h) {
  // fresh from ChatGPT - line–axis-aligned rectangle, Liang–Barsky
  float p[4], q[4];
  float u1 = 0.0f, u2 = 1.0f;

  float dx = x2 - x1;
  float dy = y2 - y1;

  float xmin = rect_x;
  float xmax = rect_x + w;
  float ymin = rect_y;
  float ymax = rect_y + h;

  // Rectangle boundaries
  p[0] = -dx;
  q[0] = x1 - xmin;  // left
  p[1] = dx;
  q[1] = xmax - x1;  // right
  p[2] = -dy;
  q[2] = y1 - ymin;  // bottom
  p[3] = dy;
  q[3] = ymax - y1;  // top

  for (int i = 0; i < 4; i++) {
    if (p[i] == 0) {
      if (q[i] < 0) return false;  // line parallel and outside
    } else {
      float t = q[i] / p[i];
      if (p[i] < 0) {
        if (t > u2) return false;
        if (t > u1) u1 = t;
      } else {
        if (t < u1) return false;
        if (t < u2) u2 = t;
      }
    }
  }
  return true;  // intersection exists
}

void draw_line_thickness(double x1, double y1, double x2, double y2, double thickness) {
  // if the line doesn't even cross the screen, don't draw it
  if (!line_intersects_rect(x1, y1, x2, y2, -thickness, -thickness, screen_size_x + thickness * 2, screen_size_y + thickness * 2)) return;

  int n = ((int)thickness / 6) + 2;  // approximate number of sides to draw on caps
  if (n > 36) n = 36;
  global_testD = n;
  int z = 6 + (6 * n);
  double angle = atan2(y2 - y1, x2 - x1);
  double dx = sin(angle) * (thickness / 2.0);
  double dy = cos(angle) * (thickness / 2.0);

  SDL_Vertex verts[z];
  int v = 0;

  // 2 triangles form the main rectangle
  verts[v].position.x = x2 + dx;
  verts[v].position.y = y2 - dy;
  v++;
  verts[v].position.x = x2 - dx;
  verts[v].position.y = y2 + dy;
  v++;
  verts[v].position.x = x1 - dx;
  verts[v].position.y = y1 + dy;
  v++;
  verts[v].position.x = x1 + dx;
  verts[v].position.y = y1 - dy;
  v++;
  verts[v].position.x = x1 - dx;
  verts[v].position.y = y1 + dy;
  v++;
  verts[v].position.x = x2 + dx;
  verts[v].position.y = y2 - dy;
  v++;

  // draw end caps
  for (int i = 0; i < n; i++) {
    verts[v].position.x = x2 + (thickness * sin(angle + (M_PI / n) * (i)) / 2);
    verts[v].position.y = y2 - (thickness * cos(angle + (M_PI / n) * (i)) / 2);
    v++;
    verts[v].position.x = x2;
    verts[v].position.y = y2;
    v++;
    verts[v].position.x = x2 + (thickness * sin(angle + (M_PI / n) * (i + 1)) / 2);
    verts[v].position.y = y2 - (thickness * cos(angle + (M_PI / n) * (i + 1)) / 2);
    v++;

    verts[v].position.x = x1 - (thickness * sin(angle + (M_PI / n) * (i)) / 2);
    verts[v].position.y = y1 + (thickness * cos(angle + (M_PI / n) * (i)) / 2);
    v++;
    verts[v].position.x = x1;
    verts[v].position.y = y1;
    v++;
    verts[v].position.x = x1 - (thickness * sin(angle + (M_PI / n) * (i + 1)) / 2);
    verts[v].position.y = y1 + (thickness * cos(angle + (M_PI / n) * (i + 1)) / 2);
    v++;
  }

  for (int i = 0; i < z; i++) {  // initalize rest of it
    verts[i].color.r = draw_re_color_red;
    verts[i].color.g = draw_re_color_green;
    verts[i].color.b = draw_re_color_blue;
    verts[i].color.a = draw_re_color_alpha;
    verts[i].tex_coord.x = 0;  // not used
    verts[i].tex_coord.y = 0;  // not used
  }

  SDL_RenderGeometry(renderer, NULL, verts, v, NULL, 0);
}

void draw_circle(double x, double y, double r, int n) {
  // from ChatGPT
  SDL_Vertex verts[n];
  int indices[3 * (n - 2)];  // triangle indices for triangle fan
  // Center vertex (used for triangle fan)
  for (int i = 0; i < n; i++) {
    float angle = 2.0f * M_PI * i / n;
    verts[i].position.x = x + r * cosf(angle);
    verts[i].position.y = y + r * sinf(angle);
    verts[i].color.r = draw_re_color_red;
    verts[i].color.g = draw_re_color_green;
    verts[i].color.b = draw_re_color_blue;
    verts[i].color.a = draw_re_color_alpha;
    verts[i].tex_coord.x = 0;  // not used
    verts[i].tex_coord.y = 0;  // not used
  }
  // Build indices for triangle fan
  for (int i = 0; i < n - 2; i++) {
    indices[3 * i + 0] = 0;  // center of fan
    indices[3 * i + 1] = i + 1;
    indices[3 * i + 2] = i + 2;
  }
  SDL_RenderGeometry(renderer, NULL, verts, n, indices, 3 * (n - 2));
}

void draw_line_thickness_canvas(double x1, double y1, double x2, double y2, double thickness) {
  double x1s, x2s, y1s, y2s;
  canvas_to_screen(x1, y1, &x1s, &y1s);
  canvas_to_screen(x2, y2, &x2s, &y2s);
  x1s += (screen_size_x / 2);
  x2s += (screen_size_x / 2);
  y1s = -y1s;
  y2s = -y2s;
  y1s += (screen_size_y / 2);
  y2s += (screen_size_y / 2);
  draw_line_thickness(x1s, y1s, x2s, y2s, thickness * cv.z);
}

void draw_render_pick() {
  int ox = pick_n * (pick_w + pick_b);
  SDL_Rect p = {screen_size_x - ox, 0, ox, pick_w + pick_b + pick_b};  // background
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 192);
  SDL_RenderFillRect(renderer, &p);
  p.h = pick_w;
  p.w = pick_w;
  p.y = pick_b;
  p.x = screen_size_x - ox + pick_b;
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // white
  SDL_RenderFillRect(renderer, &p);
  p.x += pick_b + pick_w;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // black
  SDL_RenderFillRect(renderer, &p);
  p.x += pick_b + pick_w;
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // red
  SDL_RenderFillRect(renderer, &p);
  p.x += pick_b + pick_w;
  SDL_SetRenderDrawColor(renderer, 255, 128, 0, 255);  // orange
  SDL_RenderFillRect(renderer, &p);
  p.x += pick_b + pick_w;
  SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);  // yellow
  SDL_RenderFillRect(renderer, &p);
  p.x += pick_b + pick_w;
  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // green
  SDL_RenderFillRect(renderer, &p);
  p.x += pick_b + pick_w;
  SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);  // cyan
  SDL_RenderFillRect(renderer, &p);
  p.x += pick_b + pick_w;
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);  // blue
  SDL_RenderFillRect(renderer, &p);
  p.x += pick_b + pick_w;
  SDL_SetRenderDrawColor(renderer, 128, 0, 255, 255);  // purple
  SDL_RenderFillRect(renderer, &p);
  p.x += pick_b + pick_w;
  SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);  // magenta
  SDL_RenderFillRect(renderer, &p);
  p.x += pick_b + pick_w + (pick_w / 2);
  draw_circle(p.x, (pick_w + pick_b + pick_b) / 2, 1, 17);
  p.x += pick_b + pick_w;
  draw_circle(p.x, (pick_w + pick_b + pick_b) / 2, 3, 17);
  p.x += pick_b + pick_w;
  draw_circle(p.x, (pick_w + pick_b + pick_b) / 2, 6, 17);
  p.x += pick_b + pick_w;
  draw_circle(p.x, (pick_w + pick_b + pick_b) / 2, 8, 17);
  p.x += pick_b + pick_w;
  draw_circle(p.x, (pick_w + pick_b + pick_b) / 2, 10, 17);
  p.x += pick_b + (pick_w / 2);
  SDL_SetRenderDrawColor(renderer, draw_re_color_red, draw_re_color_green, draw_re_color_blue, 128);
  SDL_RenderFillRect(renderer, &p);
  p.x += pick_b + pick_w;
  SDL_SetRenderDrawColor(renderer, draw_re_color_red, draw_re_color_green, draw_re_color_blue, 255);
  SDL_RenderFillRect(renderer, &p);
}
void draw_pick_select() {
  int pick = -1;
  int ox = pick_n * (pick_w + pick_b);
  int mousePickY = mouse_screen_raw_y;
  int mousePickX = mouse_screen_raw_x + ox - screen_size_x;
  if (mousePickY < pick_w + pick_b + pick_b && mousePickX > 0) {
    mousePickX -= pick_b;
    pick = mousePickX / (pick_w + pick_b);
    // for(int i=0;i<pick;i++)printf("##");
  }
  // printf("%d\n",pick);
  switch (pick) {
    case 0:
      draw_add(255, 255, 255, dp_color);  // color white
      break;
    case 1:
      draw_add(0, 0, 0, dp_color);  // color black
      break;
    case 2:
      draw_add(255, 0, 0, dp_color);  // color red
      break;
    case 3:
      draw_add(255, 128, 0, dp_color);  // color orange
      break;
    case 4:
      draw_add(255, 255, 0, dp_color);  // color yellow
      break;
    case 5:
      draw_add(0, 255, 0, dp_color);  // color green
      break;
    case 6:
      draw_add(0, 255, 255, dp_color);  // color cyan
      break;
    case 7:
      draw_add(0, 0, 255, dp_color);  // color blue
      break;
    case 8:
      draw_add(128, 0, 255, dp_color);  // color purple
      break;
    case 9:
      draw_add(255, 0, 255, dp_color);  // color magenta
      break;
    case 10:
      draw_add(0, 0, 1, dp_thickness);
      break;
    case 11:
      draw_add(0, 0, 3, dp_thickness);
      break;
    case 12:
      draw_add(0, 0, 25, dp_thickness);
      break;
    case 13:
      draw_add(0, 0, 75, dp_thickness);
      break;
    case 14:
      draw_add(0, 0, 150, dp_thickness);
      break;
    case 15:
      draw_add(0, 0, 0, dp_alpha);
      draw_re_alpha = 1;
      break;
    case 16:
      draw_add(0, 0, 0, dp_opaque);
      draw_re_alpha = 0;
      break;
  }
}

void draw_render() {
  global_testA = draw_len;
  // draw_line_thickness_canvas(0, 0, xp, yp, 3);
  bool canvas_changed = (cv.r != draw_cv_last.r || cv.x != draw_cv_last.x || cv.y != draw_cv_last.y || cv.r != draw_cv_last.r);
  bool draw_changed = (draw_len != draw_len_last);
  draw_re_color_alpha = 255;
  if (canvas_changed || draw_changed) {
    int draw_start = draw_len_last - 1;
    if (canvas_changed || draw_len < draw_len_last) {
      SDL_SetRenderTarget(renderer, draw_tex_a);
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
      SDL_RenderClear(renderer);
      SDL_SetRenderTarget(renderer, draw_tex);
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
      SDL_RenderClear(renderer);
      draw_cv_last = cv;
      draw_start = 0;
      draw_re_last_x = 0;
      draw_re_last_y = 0;
      draw_re_color_red = 255;
      draw_re_color_blue = 255;
      draw_re_color_green = 255;
      draw_re_thickness = 3;
      draw_re_alpha = 0;
    }
    if (draw_re_alpha) {
      SDL_SetRenderTarget(renderer, draw_tex_a);
    } else {
      SDL_SetRenderTarget(renderer, draw_tex);
    }
    for (int i = draw_start; i < draw_len; i++) {
      switch (draw_points[i].t) {
        case dp_drop:
          if (draw_points[i + 1].t != dp_move) {
            draw_line_thickness_canvas(draw_points[i].x, draw_points[i].y, draw_points[i].x, draw_points[i].y, draw_re_thickness);
          } else {
            draw_re_last_x = draw_points[i].x;
            draw_re_last_y = draw_points[i].y;
          }
          break;
        case dp_move:
          draw_line_thickness_canvas(draw_re_last_x, draw_re_last_y, draw_points[i].x, draw_points[i].y, draw_re_thickness);

          draw_re_last_x = draw_points[i].x;
          draw_re_last_y = draw_points[i].y;
          break;
        case dp_color:
          draw_re_color_red = draw_points[i].x;
          draw_re_color_green = draw_points[i].y;
          draw_re_color_blue = draw_points[i].z;
          break;
        case dp_thickness:
          draw_re_thickness = draw_points[i].z;
          break;
        case dp_alpha:
          SDL_SetRenderTarget(renderer, draw_tex_a);
          draw_re_alpha = 1;
          break;
        case dp_opaque:
          SDL_SetRenderTarget(renderer, draw_tex);
          draw_re_alpha = 0;
          break;
      }
      // draw_line_thickness_canvas(draw_points[i].x, draw_points[i].y, draw_points[i + 1].x, draw_points[i + 1].y, 3);
    }
    draw_len_last = draw_len;
    SDL_SetRenderTarget(renderer, NULL);

    // printf("\n");
    // for (int i = 0; i < draw_len; i++) {
    //   printf("%d", draw_points[i].t);
    //   if ((i + 1) % 100 == 0) printf("\n");
    // }
    // printf("\n");
  }

  SDL_Rect dst = {0, 0, MAX_SCREEN_X, MAX_SCREEN_Y};

  SDL_SetRenderTarget(renderer, NULL);
  SDL_RenderCopy(renderer, draw_tex_a, NULL, &dst);
  SDL_RenderCopy(renderer, draw_tex, NULL, &dst);

  if (draw_re_test) {
    if (draw_re_alpha) {
      draw_re_color_alpha = 128;
    }  // not quite correct, but at least you can see through the line
    draw_line_thickness_canvas(draw_re_last_x, draw_re_last_y, draw_re_test_x, draw_re_test_y, draw_re_thickness);
  }

  if (draw_pick) {
    draw_render_pick();
  }
}

void draw_lift_pen() { draw_add(0, 0, 0, dp_lift); }
void draw_drop_pen(double x, double y) {
  draw_add(x, y, 0, dp_drop);
  draw_re_last_x = x;
  draw_re_last_y = y;
}
void draw_move_pen(double x, double y) {
  draw_add(x, y, 0, dp_move);
  draw_re_last_x = x;
  draw_re_last_y = y;
}
void draw_back_pen() {
  int i = draw_len - 1;
  if (i < 0) {
    i = 0;
    return;
  }
  while (i > 0) {
    if (draw_points[i].t == dp_drop) break;
    i--;
  }
  if (i > 0) i--;
  draw_len = i + 1;
}
void draw_forward_pen() {
  int i = draw_len + 1;
  while (i < draw_points_max) {
    if (draw_points[i].t == dp_lift) break;
    if (draw_points[i].t == dp_none) return;
    i++;
  }
  // if (i > 0) i--;
  draw_len = i+1;
}
void draw_pick_open() { draw_pick = 1; }
void draw_pick_close() {
  draw_pick = 0;
  draw_pick_select();
}
void draw_test_pen(double x, double y) {
  draw_re_test_x = x;
  draw_re_test_y = y;
  draw_re_test = 1;
}
void draw_commit_pen() {
  draw_re_test = 0;
  draw_drop_pen(draw_re_last_x, draw_re_last_y);
  draw_move_pen(draw_re_test_x, draw_re_test_y);
  draw_lift_pen();
}