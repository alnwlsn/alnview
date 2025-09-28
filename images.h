#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <lz4hc.h>

#include "defines.h"
#include "canvas.h"
#include "render.h"

typedef struct {
  SDL_Texture *texture_fullres;
  SDL_Texture *texture_small;
  char *image_compressed;
  int image_compressed_size;
  int image_raw_size;
  Uint32 format; // SDL pixel format enum
  int width;
  int height;
  int pitch;
  bool fullres_exists;
  int offscreen_frame_count;
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
  bool fullres_in_view[MAX_CANVAS];
  bool inited;
  int center_closeness_index;
  double center_closeness;
  double aX; //corners
  double aY;
  double bX;
  double bY;
  double cX;
  double cY;
  double dX;
  double dY;
  double center_x; //center
  double center_y;
} Image;

extern bool init_small_image_only;
extern bool init_no_compress_images;
extern int init_small_image_reduction;
extern int init_max_restored_hires;

extern bool antialiasing;
extern int images_count;
extern Image *images;

void images_unload();                                    // call before reloading images
void images_free();                                      // to be called when program exits
void images_load_dir(bool show);  // load all images from working directory
void images_arrange_in_grid();
int image_load(char *filepath);  // load one image, returns images[] index
int image_point_on(double x, double y);  // returns images[] index of image currently under the given point, or -1 if none
void images_render();
void image_find_corners(int si);
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
void image_crop(int imi);
void image_uncrop(int imi);
void image_restore_fullres(int imi);
void image_discard_fullres(int imi);
void image_discard_fullres_or_auto_hires(int imi);
void image_auto_hires_restore(bool s);

void image_reload(int imi);

void canvas_use_cvp(int ci);
void canvas_set_cvp(int ci);