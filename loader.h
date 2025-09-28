#pragma once
#include "canvas.h"
#include "images.h"
#include "controls.h"
#include "draw.h"
#include <stdbool.h>

typedef struct {
  // SDL_Texture *texture;   // ignored
  int width;
  int height;
  double x;
  double y;
  double r;
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
  int sort_index;
  char filepath[FILEPATHLEN];
  bool fullres_in_view[MAX_CANVAS];
} ImageSave;

bool load_state(bool show);
void save_state();
void loader_uni(bool show); //universal load (from savefile or not)