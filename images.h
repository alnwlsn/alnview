#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "canvas.h"
#include "defines.h"

typedef struct {
  SDL_Texture *texture;
  int width;
  int height;
  double x;
  double y;
  double r;  // rotation angle, and rotation points
  double rx;
  double ry;
  double z;
  uint8_t opacity;
  int crop_top;
  int crop_right;
  int crop_bottom;
  int crop_left;
  int series_order;
  int draw_order;
  int sort_index;  // when sorted, this points to images[index] of sorted list
  char filepath[FILEPATHLEN];
} Image;
extern int images_count;

typedef struct {
  double aX;
  double aY;
  double bX;
  double bY;
  double cX;
  double cY;
  double dX;
  double dY;
} rectangleCorners;

extern Image *images;

extern double imrefAx;
extern double imrefAy;
extern double imrefBx;
extern double imrefBy;
extern double imrefCx;
extern double imrefCy;
extern double imrefDx;
extern double imrefDy;

void images_free();                           // to be called when program exits
void images_load_dir(const char *directory);  // load all images from directory
void images_arrange_in_grid();
int image_load(char *filepath);

int image_point_on(double x, double y);  // returns images[] index of image currently under the given point, or -1 if none

void images_render();

rectangleCorners image_find_corners(int si);

void image_to_on_bottom(int imi);
void image_to_on_top(int imi);
void image_drag_screen_by(int imi, int dx, int dy);
void image_zoom_by(int imi, double zoom_factor);
void image_zoom_reset(int imi);
void image_rotate_by(int imi, double dr);
void image_rotate_snap(int imi, double sr);
void image_rotation_point_set_new(int imi, double x, double y);
void image_rotation_point_set_center(int imi);
void canvas_center_on_image_fit(int imi);
void canvas_zoom_center_fitall();
void canvas_center_on_image(int imi);
void canvas_center_on_nearest_image_in_direction(int imi, double direction);
void image_series_set(int imi);
void image_center_series_next();
void image_center_series_prev();
