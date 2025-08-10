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
  int crop_top;
  int crop_right;
  int crop_bottom;
  int crop_left;
  int sel_order;
  int draw_order;
  int sort_index;  // when sorted, this points to images[index] of sorted list
  char filepath[FILEPATHLEN];
} Image;

extern float imrefAx;
extern float imrefAy;
extern float imrefBx;
extern float imrefBy;

void images_free(); //to be called when program exits
void images_load_dir(const char *directory);  // load all images from directory
void images_arrange_in_grid();

void images_render();