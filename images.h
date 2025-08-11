#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "defines.h"
#include "canvas.h"

typedef struct {
  SDL_Texture *texture;
  int width;
  int height;
  float x;
  float y;
  float r;  // rotation angle, and rotation points
  float rx;
  float ry;
  float z;
  uint8_t opacity;
  int crop_top;
  int crop_right;
  int crop_bottom;
  int crop_left;
  int sel_order;
  int draw_order;
  int sort_index;  // when sorted, this points to images[index] of sorted list
  char filepath[FILEPATHLEN];
} Image;

typedef struct {
  float aX;
  float aY;
  float bX;
  float bY;
  float cX;
  float cY;
  float dX;
  float dY;
} rectangleCorners;

extern Image *images;

extern float imrefAx;
extern float imrefAy;
extern float imrefBx;
extern float imrefBy;
extern float imrefCx;
extern float imrefCy;
extern float imrefDx;
extern float imrefDy;

void images_free(); //to be called when program exits
void images_load_dir(const char *directory);  // load all images from directory
void images_arrange_in_grid();

int image_point_on(float x, float y); //returns images[] index of image currently under the given point, or -1 if none

void images_render();

rectangleCorners image_find_corners(int si);

void image_to_on_bottom(int imi);
void image_to_on_top(int imi);
void image_drag_screen_by(int imi, int dx, int dy);
void image_zoom_by(int imi, float zoom_factor);
void image_zoom_reset(int imi);
void image_rotate_by(int imi, float dr);
void image_rotate_snap(int imi, float sr);