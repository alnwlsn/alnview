#include "images.h"

// default argument options
bool init_small_image_only = false;
bool init_no_compress_images = false;
int init_small_image_reduction = 8;
int init_max_restored_hires = MAX_IMAGES;

bool antialiasing = false;
Image *images = NULL;
int images_count = 0;

int series_current = 0;
bool auto_hires_restore = false;
bool auto_hires_discard = false;
int count_hires_restored = 0;
void image_auto_hires_restore(bool s) {
  auto_hires_restore = s;
  auto_hires_discard = s;
}

// sorting options
// call sort_images_by(compare_some_option)
Image *sort_images;
int compare_filepath(const void *a, const void *b) {
  int i = *(const int *)a;
  int j = *(const int *)b;
  return strcmp(sort_images[i].filepath, sort_images[j].filepath);
}
int compare_draw_order(const void *a, const void *b) {
  int i = *(const int *)a;
  int j = *(const int *)b;
  if (sort_images[i].draw_order < sort_images[j].draw_order) return -1;
  if (sort_images[i].draw_order > sort_images[j].draw_order) return 1;
  return 0;
}
int compare_closeness_order(const void *a, const void *b) {
  int i = *(const int *)a;
  int j = *(const int *)b;
  if (sort_images[i].center_closeness < sort_images[j].center_closeness) return -1;
  if (sort_images[i].center_closeness > sort_images[j].center_closeness) return 1;
  return 0;
}
int compare_series_order(const void *a, const void *b) {
  int i = *(const int *)a;
  int j = *(const int *)b;
  if (sort_images[i].series_order < sort_images[j].series_order) return -1;
  if (sort_images[i].series_order > sort_images[j].series_order) return 1;
  return 0;
}
int compare_series_order_rev(const void *a, const void *b) {
  int i = *(const int *)a;
  int j = *(const int *)b;
  if (sort_images[i].series_order > sort_images[j].series_order) return -1;
  if (sort_images[i].series_order < sort_images[j].series_order) return 1;
  return 0;
}
void sort_images_by(int (*cmp)(const void *, const void *)) {
  int indices[MAX_IMAGES];
  for (int i = 0; i < images_count; i++) indices[i] = i;

  sort_images = images;
  qsort(indices, images_count, sizeof(int), cmp);

  for (int i = 0; i < images_count; i++) {
    // images[indices[i]].sort_index = i; //put images[] order into sort_index
    images[i].sort_index = indices[i];  // put order of images[] index into sort_index
  }
}

void image_calculate_handles(int si) {  // finds canvas coords for 4 corners of image, center, distance from center view
  images[si].aX = 0;
  images[si].aY = 0;
  images[si].bX = 0;
  images[si].bY = 0;
  images[si].cX = 0;
  images[si].cY = 0;
  images[si].dX = 0;
  images[si].dY = 0;

  double rpc_x = images[si].x + images[si].rx;
  double rpc_y = images[si].y + images[si].ry;
  double pxc_x = images[si].x + images[si].crop_left;
  double pxc_y = images[si].y - images[si].crop_top;
  images[si].aX = (pxc_x - rpc_x) * cos(images[si].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[si].r * M_PI / 180);
  images[si].aY = (pxc_y - rpc_y) * cos(images[si].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[si].r * M_PI / 180);
  images[si].aX *= images[si].z;
  images[si].aY *= images[si].z;
  images[si].aX += rpc_x;
  images[si].aY += rpc_y;

  rpc_x = images[si].x + images[si].rx;
  rpc_y = images[si].y + images[si].ry;
  pxc_x = images[si].x + images[si].width - images[si].crop_right;
  pxc_y = images[si].y - images[si].crop_top;
  images[si].bX = (pxc_x - rpc_x) * cos(images[si].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[si].r * M_PI / 180);
  images[si].bY = (pxc_y - rpc_y) * cos(images[si].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[si].r * M_PI / 180);
  images[si].bX *= images[si].z;
  images[si].bY *= images[si].z;
  images[si].bX += rpc_x;
  images[si].bY += rpc_y;

  rpc_x = images[si].x + images[si].rx;
  rpc_y = images[si].y + images[si].ry;
  pxc_x = images[si].x + images[si].width - images[si].crop_right;
  pxc_y = images[si].y - images[si].height + images[si].crop_bottom;
  images[si].cX = (pxc_x - rpc_x) * cos(images[si].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[si].r * M_PI / 180);
  images[si].cY = (pxc_y - rpc_y) * cos(images[si].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[si].r * M_PI / 180);
  images[si].cX *= images[si].z;
  images[si].cY *= images[si].z;
  images[si].cX += rpc_x;
  images[si].cY += rpc_y;

  rpc_x = images[si].x + images[si].rx;
  rpc_y = images[si].y + images[si].ry;
  pxc_x = images[si].x + images[si].crop_left;
  pxc_y = images[si].y - images[si].height + images[si].crop_bottom;
  images[si].dX = (pxc_x - rpc_x) * cos(images[si].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[si].r * M_PI / 180);
  images[si].dY = (pxc_y - rpc_y) * cos(images[si].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[si].r * M_PI / 180);
  images[si].dX *= images[si].z;
  images[si].dY *= images[si].z;
  images[si].dX += rpc_x;
  images[si].dY += rpc_y;
  images[si].center_x = (images[si].aX + images[si].bX + images[si].cX + images[si].dX) / 4.0f;
  images[si].center_y = (images[si].aY + images[si].bY + images[si].cY + images[si].dY) / 4.0f;
  images[si].center_closeness = sqrt((images[si].center_x - cv.x) * (images[si].center_x - cv.x) + (images[si].center_y - cv.y) * (images[si].center_y - cv.y));
}

void images_arrange_in_grid() {  // arranges all images into grid by selection order (do after loading images for first time)
  int columns = ceil(sqrt(images_count));
  int spacing = 20;

  int x = 0, y = 0;
  int max_row_height = 0;
  sort_images_by(compare_series_order);
  for (int i = 0; i < images_count; ++i) {
    int si = images[i].sort_index;
    images[si].x = x;
    images[si].y = -y;
    images[si].r = 0;
    images[si].z = 1;
    x += images[si].width + spacing;
    if (images[si].height > max_row_height) max_row_height = images[si].height;

    if ((i + 1) % columns == 0) {
      x = 0;
      y += max_row_height + spacing;
      max_row_height = 0;
    }
  }
}

void canvas_zoom_center_fitall() {
  if (images_count <= 0) return;
  double minx = DBL_MAX, miny = DBL_MAX;
  double maxx = -DBL_MAX, maxy = -DBL_MAX;
  for (int i = 0; i < images_count; i++) {
    double xs[] = {images[i].aX, images[i].bX, images[i].cX, images[i].dX};
    double ys[] = {images[i].aY, images[i].bY, images[i].cY, images[i].dY};
    for (int j = 0; j <= 3; j++) {
      if (xs[j] <= minx) minx = xs[j];
      if (ys[j] <= miny) miny = ys[j];
      if (xs[j] >= maxx) maxx = xs[j];
      if (ys[j] >= maxy) maxy = ys[j];
    }
  }
  double cex = (maxx + minx) / 2, cey = (maxy + miny) / 2;
  cv.x = cex;
  cv.y = cey;
  minx = cex - minx;
  miny = cey - miny;
  double im_rt = miny / minx;
  miny *= cv.z;
  minx *= cv.z;
  double sc_rt = (double)screen_size_y / (double)screen_size_x;
  if (im_rt <= sc_rt) {
    cv.z *= ((screen_size_x / 2.0f)) / minx;
  } else {
    cv.z *= ((screen_size_y / 2.0f)) / miny;
  }
  cv.z /= 1.05;
}

SDL_Surface *ScaleSurface(SDL_Surface *surface, int reduce) {
  if (!surface || reduce <= 1) return NULL;
  int new_w = (int)(surface->w / reduce);
  int new_h = (int)(surface->h / reduce);
  SDL_Surface *scaled = SDL_CreateRGBSurfaceWithFormat(0, new_w, new_h, surface->format->BitsPerPixel, surface->format->format);
  if (!scaled) {
    printf("SDL_CreateRGBSurfaceWithFormat failed: %s\n", SDL_GetError());
    return NULL;
  }
  SDL_Rect srcRect = {0, 0, surface->w, surface->h};
  SDL_Rect dstRect = {0, 0, new_w, new_h};
  if (SDL_BlitScaled(surface, &srcRect, scaled, &dstRect) < 0) {
    printf("SDL_BlitScaled failed: %s\n", SDL_GetError());
    SDL_FreeSurface(scaled);
    return NULL;
  }
  return scaled;
}

void image_discard_fullres(int imi) {
  if (init_no_compress_images) return;
  if (!images[imi].fullres_exists) return;
  SDL_DestroyTexture(images[imi].texture_fullres);
  images[imi].fullres_exists = false;
  count_hires_restored -= 1;
  #ifdef SHOW_RESTORES_DISCARDS
  printf("discard %d (%d)\n",imi,count_hires_restored);
  #endif
}
void image_discard_fullres_or_auto_hires(int imi) {
  if (auto_hires_discard) {
    for (int i = 0; i < images_count; ++i) {
      image_discard_fullres(i);
    }
  } else {
    image_discard_fullres(imi);
  }
}
void image_restore_fullres(int imi) {
  if (init_no_compress_images) return;
  if (images[imi].fullres_exists) return;
  if (count_hires_restored >= init_max_restored_hires) return;
  SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, images[imi].width, images[imi].height, SDL_BITSPERPIXEL(images[imi].format), images[imi].format);
  int data_size = surface->h * surface->pitch;
  int decoded = LZ4_decompress_safe(images[imi].image_compressed, (char *)surface->pixels, images[imi].image_compressed_size, data_size);
  if (decoded != data_size) {
    SDL_FreeSurface(surface);
    printf("couldn't decompress image\n");
    return;  // decompression failed
  }
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);  // this enables opacity adjustment
  images[imi].texture_fullres = texture;
  SDL_FreeSurface(surface);
  images[imi].fullres_exists = true;
  if (antialiasing) {
    SDL_SetTextureScaleMode(images[imi].texture_fullres, SDL_ScaleModeLinear);
  } else {
    SDL_SetTextureScaleMode(images[imi].texture_fullres, SDL_ScaleModeNearest);
  }
  count_hires_restored += 1;
  #ifdef SHOW_RESTORES_DISCARDS
  printf("restore %d (%d)\n",imi,count_hires_restored);
  #endif
}

int image_load(char *filepath) {  // loads image at filepath, inits width and height
  if (images_count >= MAX_IMAGES) {
    fprintf(stderr, "Too many images to load\n");
  }
  SDL_Surface *surfaceRaw = IMG_Load(filepath);
  SDL_Surface *surface = SDL_ConvertSurfaceFormat(surfaceRaw, SDL_PIXELFORMAT_ARGB8888, 0);
  SDL_FreeSurface(surfaceRaw);

  if (surface) {
    images = realloc(images, sizeof(Image) * (images_count + 1));
    Image *img = &images[images_count];
    img->width = surface->w;
    img->height = surface->h;
    img->pitch = surface->pitch;
    img->format = surface->format->format;
    img->inited = 0;
    memcpy(img->filepath, filepath, FILEPATHLEN);

    if (init_no_compress_images) {
      if (init_small_image_only) {  // use the small image as the fullres texture
        SDL_Surface *surface_small = ScaleSurface(surface, init_small_image_reduction);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface_small);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);  // this enables opacity adjustment
        img->texture_fullres = NULL;
        img->fullres_exists = false;
        img->texture_small = texture;
        SDL_FreeSurface(surface_small);
      } else {
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);  // this enables opacity adjustment
        img->texture_fullres = texture;
        img->fullres_exists = true;
        img->texture_small = NULL;
      }
    } else {
      img->texture_fullres = NULL;
      img->fullres_exists = false;
      SDL_Surface *surface_small = ScaleSurface(surface, init_small_image_reduction);
      SDL_Texture *texture_small = SDL_CreateTextureFromSurface(renderer, surface_small);
      SDL_SetTextureBlendMode(texture_small, SDL_BLENDMODE_BLEND);
      img->texture_small = texture_small;
      SDL_FreeSurface(surface_small);
    }

    if (!init_no_compress_images) {
      // store a compressed version of the fullres image to reduce ram
      int data_size = surface->h * surface->pitch;
      int max_dst_size = LZ4_compressBound(data_size);
      char *dst = (char *)malloc(max_dst_size);  // Allocate destination buffer (max compressed size)
      if (!dst) {
        printf("couldn't allocate compressed buffer\n");
      }
      // int compressed_size = LZ4_compress_HC((const char *)surface->pixels, dst, data_size, max_dst_size, LZ4HC_CLEVEL_MAX);
      int compressed_size = LZ4_compress_HC((const char *)surface->pixels, dst, data_size, max_dst_size, 1);
      if (compressed_size <= 0) {
        free(dst);
        printf("couldn't compress\n");
      }
      img->image_compressed = dst;
      img->image_compressed_size = compressed_size;
      printf("comp %4.1f, %d, %d, %s\n", (float)100 * compressed_size / data_size, data_size, compressed_size, filepath);
    }

    // SDL_DestroyTexture(texture);
    images_count++;
    SDL_FreeSurface(surface);
  } else {
    fprintf(stderr, "Failed to load %s: %s\n", filepath, IMG_GetError());
  }
  return images_count - 1;
  // fprintf(stdout, "img %d\n", images_count);
}
void images_load_dir(bool show) {  // load all images from directory
  bool no_savefile = 1;
  DIR *dir = opendir(".");
  if (!dir) {
    perror("Failed to open image directory");
    exit(1);
  }
  struct dirent *entry;
  // collect image files in dir
  while ((entry = readdir(dir))) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, SAVEFILE) == 0) continue;
    char path[512];
    snprintf(path, sizeof(path) - 8, "%s", entry->d_name);
    struct stat path_stat;
    if (stat(path, &path_stat) != 0 || !S_ISREG(path_stat.st_mode)) continue;

    bool already_loaded = 0;
    for (int i = 0; i < images_count; i++) {
      if (strcmp(path, images[i].filepath) == 0) {
        // fprintf(stderr, "tried to load %s but is already loaded\n", path);
        already_loaded = 1;
        images[i].inited = 1;
        no_savefile = 0;
        continue;
      }
    }
    if (already_loaded) continue;

    snprintf(coordText, sizeof(coordText), "loading %s", path);
    if (show) render_text_screen(coordText);

    image_load(path);
  }
  closedir(dir);

  if (images_count <= 0) return;

  // initialize images
  sort_images_by(compare_filepath);

  int draw_order_offset = 0;
  int series_order_offset = 0;
  for (int i = 0; i < images_count; i++) {
    if (images[i].inited) {
      if (draw_order_offset <= images[i].draw_order) draw_order_offset = images[i].draw_order;
      if (series_order_offset <= images[i].series_order) series_order_offset = images[i].series_order;
    }
  }

  for (int i = 0; i < images_count; i++) {
    if (!images[i].inited) {
      images[i].x = 0;
      images[i].y = 0;
      images[i].r = 0;
      images[i].rx = images[i].width / 2;
      images[i].ry = -images[i].height / 2;
      images[i].z = 1;
      images[i].crop_top = 0;
      images[i].crop_right = 0;
      images[i].crop_bottom = 0;
      images[i].crop_left = 0;
      images[i].opacity = 255;
      for (int j = 0; j < MAX_CANVAS; j++){
        images[i].fullres_in_view[j] = 0;
      }
      draw_order_offset += 1;
      series_order_offset += 1;
      if (no_savefile) {
        images[images[i].sort_index].draw_order = i;
        images[images[i].sort_index].series_order = i;
      } else {
        images[i].draw_order = draw_order_offset;
        images[i].series_order = series_order_offset;
      }
    }
    image_calculate_handles(i);  // init
  }
  cv.selected_imi = images[0].sort_index;
  if (no_savefile) {
    images_arrange_in_grid();
    for (int i = 0; i < images_count; i++) {
      image_calculate_handles(i);  // update handles after moving into grid
    }
  }
}

bool image_point_inside(double px, double py, int si) {
  // take cross product of each point with target. if all in same direction, is inside
  double c1 = (images[si].bX - images[si].aX) * (py - images[si].aY) - (images[si].bY - images[si].aY) * (px - images[si].aX);
  double c2 = (images[si].cX - images[si].bX) * (py - images[si].bY) - (images[si].cY - images[si].bY) * (px - images[si].bX);
  double c3 = (images[si].dX - images[si].cX) * (py - images[si].cY) - (images[si].dY - images[si].cY) * (px - images[si].cX);
  double c4 = (images[si].aX - images[si].dX) * (py - images[si].dY) - (images[si].aY - images[si].dY) * (px - images[si].dX);
  return ((c1 >= 0 && c2 >= 0 && c3 >= 0 && c4 >= 0) || (c1 <= 0 && c2 <= 0 && c3 <= 0 && c4 <= 0));
}

int image_point_on(double x, double y) {  // tells which image is under the point
  int image_index = -1;
  int c_draw_order = -999999;
  for (int i = 0; i < images_count; ++i) {
    if (image_point_inside(x, y, i)) {
      if (images[i].draw_order >= c_draw_order) {
        c_draw_order = images[i].draw_order;
        image_index = i;
      }
    }
  }
  return image_index;
}
void image_to_on_top(int imi) {
  if (imi < 0) return;
  int max_draw_order = -999999;
  for (int i = 0; i < images_count; ++i) {
    if (images[i].draw_order >= max_draw_order) {
      max_draw_order = images[i].draw_order;
    }
  }
  if (images[imi].draw_order < max_draw_order) images[imi].draw_order = max_draw_order + 1;
}
void image_to_on_bottom(int imi) {
  if (imi < 0) return;
  int min_draw_order = 999999;
  for (int i = 0; i < images_count; ++i) {
    if (images[i].draw_order <= min_draw_order) {
      min_draw_order = images[i].draw_order;
    }
  }
  if (images[imi].draw_order > min_draw_order) images[imi].draw_order = min_draw_order - 1;
}

void image_drag_screen_by(int imi, int dx, int dy) {
  if (imi < 0) return;
  // pan screen by mouse coords
  double sdx = -dx / cv.z;
  double sdy = -dy / cv.z;
  double cdx = sdx * cos(-cv.r * M_PI / 180) - sdy * sin(-cv.r * M_PI / 180);
  double cdy = sdy * cos(-cv.r * M_PI / 180) + sdx * sin(-cv.r * M_PI / 180);
  images[imi].x -= cdx;
  images[imi].y -= cdy;
}
void image_zoom_by(int imi, double zoom_factor) {
  if (imi < 0) return;
  double Ax = mouse_canvas_x - images[imi].x - images[imi].rx;
  double Ay = mouse_canvas_y - images[imi].y - images[imi].ry;
  images[imi].z *= zoom_factor;
  double Bx = (mouse_canvas_x - images[imi].x - images[imi].rx) * zoom_factor;
  double By = (mouse_canvas_y - images[imi].y - images[imi].ry) * zoom_factor;
  images[imi].x -= (Bx - Ax);
  images[imi].y -= (By - Ay);
}
void image_zoom_reset(int imi) {
  if (imi < 0) return;
  image_zoom_by(imi, 1 / images[imi].z);
  images[imi].z = 1;
}
void image_rotate_by(int imi, double dr) {
  if (imi < 0) return;
  images[imi].r += dr;
}
void image_rotate_snap(int imi, double r) {
  if (imi < 0) return;
  double a = images[imi].r;
  a /= r;
  a = round(a);
  a *= r;
  images[imi].r = a;
}
void image_rotation_point_set_new(int imi, double x, double y) {
  if (imi < 0) return;
  // do this the cheat way: first, position before change
  double rpc_x = images[imi].x + images[imi].rx;
  double rpc_y = images[imi].y + images[imi].ry;
  double pxc_x = images[imi].x + images[imi].crop_left;
  double pxc_y = images[imi].y - images[imi].crop_top;
  double aX = (pxc_x - rpc_x) * cos(images[imi].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[imi].r * M_PI / 180);
  double aY = (pxc_y - rpc_y) * cos(images[imi].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[imi].r * M_PI / 180);
  aX *= images[imi].z;
  aY *= images[imi].z;
  aX += rpc_x;
  aY += rpc_y;

  // find new rotation point (in image coordinates)
  double dX = images[imi].rx;
  double dY = images[imi].ry;
  double cX = (x - images[imi].x - dX) / images[imi].z;
  double cY = (y - images[imi].y - dY) / images[imi].z;
  images[imi].rx = cX * cos(-images[imi].r * M_PI / 180) - cY * sin(-images[imi].r * M_PI / 180) + dX;
  images[imi].ry = cY * cos(-images[imi].r * M_PI / 180) + cX * sin(-images[imi].r * M_PI / 180) + dY;

  // then, after
  rpc_x = images[imi].x + images[imi].rx;
  rpc_y = images[imi].y + images[imi].ry;
  pxc_x = images[imi].x + images[imi].crop_left;
  pxc_y = images[imi].y - images[imi].crop_top;
  double bX = (pxc_x - rpc_x) * cos(images[imi].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[imi].r * M_PI / 180);
  double bY = (pxc_y - rpc_y) * cos(images[imi].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[imi].r * M_PI / 180);
  bX *= images[imi].z;
  bY *= images[imi].z;
  bX += rpc_x;
  bY += rpc_y;
  images[imi].x -= (bX - aX);
  images[imi].y -= (bY - aY);
}

void canvas_center_on_image(int imi) {
  if (imi < 0) return;
  cv.x = images[imi].center_x;
  cv.y = images[imi].center_y;
  cv.selected_imi = imi;
  series_current = images[cv.selected_imi].series_order;
}
void canvas_center_on_nearest_image_in_direction(int imi, double direction) {
  double nearestDistance = INFINITY;
  int cimi = -1;

  for (int i = 0; i < images_count; i++) {
    if (i != imi) {
      double distance = sqrt(((images[i].center_x - images[imi].center_x) * (images[i].center_x - images[imi].center_x)) +
                             ((images[i].center_y - images[imi].center_y) * (images[i].center_y - images[imi].center_y)));
      double angle = (180 / M_PI) * atan2((images[imi].center_y - images[i].center_y), (images[i].center_x - images[imi].center_x));

      angle += direction + 45 - cv.r;
      while (angle > 360) angle -= 360;
      while (angle < 0) angle += 360;

      if (angle < 90) {
        if (distance < nearestDistance) {
          nearestDistance = distance;
          cimi = i;
        }
      }
    }
  }
  if (cimi > -1) {
    canvas_center_on_image(cimi);
  }
}

void image_rotation_point_set_center(int imi) {  // return image rotation to center
  if (imi < 0) return;
  image_rotation_point_set_new(imi, images[imi].center_x, images[imi].center_y);
}
void canvas_center_on_image_fit(int imi) {
  // currently doesn't work with rotated canvas
  if (imi < 0) return;
  cv.x = images[imi].center_x;  // center view
  cv.y = images[imi].center_y;
  double eX = images[imi].aX, eY = images[imi].aY;
  double xs[] = {images[imi].aX, images[imi].bX, images[imi].cX, images[imi].dX};
  double ys[] = {images[imi].aY, images[imi].bY, images[imi].cY, images[imi].dY};
  for (int i = 0; i <= 3; i++) {
    if (xs[i] <= eX) eX = xs[i];
    if (ys[i] <= eY) eY = ys[i];
  }
  eX = images[imi].center_x - eX;
  eY = images[imi].center_y - eY;
  double im_rt = eY / eX;
  eY *= cv.z;
  eX *= cv.z;
  double sc_rt = (double)screen_size_y / (double)screen_size_x;
  if (im_rt <= sc_rt) {
    cv.z *= ((screen_size_x / 2.0f)) / eX;
  } else {
    cv.z *= ((screen_size_y / 2.0f)) / eY;
  }
  cv.z /= 1.05;
  // printf("%.1f, %.1f %.1f, %.1f\n", im_rt, sc_rt, eX, eY);
}

void image_series_set(int imi) {
  series_current = images[imi].series_order;
  // printf("s%d\n", series_current);
}
void image_center_series_next() {
  int imi = -1;
  sort_images_by(compare_series_order);
  for (int i = 0; i < images_count; i++) {
    if (images[images[i].sort_index].series_order > series_current) {
      series_current = images[images[i].sort_index].series_order;
      // printf("s%d\n", series_current);
      imi = images[i].sort_index;
      break;
    }
  }
  if (imi < 0) {
    series_current = images[images[0].sort_index].series_order;
    imi = images[0].sort_index;
  }
  canvas_center_on_image(imi);
}
void image_center_series_prev() {
  int imi = -1;
  sort_images_by(compare_series_order_rev);
  for (int i = 0; i < images_count; i++) {
    if (images[images[i].sort_index].series_order < series_current) {
      series_current = images[images[i].sort_index].series_order;
      // printf("s%d\n", series_current);
      imi = images[i].sort_index;
      break;
    }
  }
  if (imi < 0) {
    series_current = images[images[0].sort_index].series_order;
    imi = images[0].sort_index;
  }
  canvas_center_on_image(imi);
}

void image_crop(int imi) {
  double taX = 0;
  double taY = 0;
  double tbX = 0;
  double tbY = 0;
  double tcX = 0;
  double tcY = 0;
  double tdX = 0;
  double tdY = 0;

  double rpc_x = images[imi].x + images[imi].rx;
  double rpc_y = images[imi].y + images[imi].ry;
  double pxc_x = images[imi].x;
  double pxc_y = images[imi].y;
  taX = (pxc_x - rpc_x) * cos(images[imi].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[imi].r * M_PI / 180);
  taY = (pxc_y - rpc_y) * cos(images[imi].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[imi].r * M_PI / 180);
  taX *= images[imi].z;
  taY *= images[imi].z;
  taX += rpc_x;
  taY += rpc_y;

  rpc_x = images[imi].x + images[imi].rx;
  rpc_y = images[imi].y + images[imi].ry;
  pxc_x = images[imi].x + images[imi].width;
  pxc_y = images[imi].y;
  tbX = (pxc_x - rpc_x) * cos(images[imi].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[imi].r * M_PI / 180);
  tbY = (pxc_y - rpc_y) * cos(images[imi].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[imi].r * M_PI / 180);
  tbX *= images[imi].z;
  tbY *= images[imi].z;
  tbX += rpc_x;
  tbY += rpc_y;

  rpc_x = images[imi].x + images[imi].rx;
  rpc_y = images[imi].y + images[imi].ry;
  pxc_x = images[imi].x + images[imi].width;
  pxc_y = images[imi].y - images[imi].height;
  tcX = (pxc_x - rpc_x) * cos(images[imi].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[imi].r * M_PI / 180);
  tcY = (pxc_y - rpc_y) * cos(images[imi].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[imi].r * M_PI / 180);
  tcX *= images[imi].z;
  tcY *= images[imi].z;
  tcX += rpc_x;
  tcY += rpc_y;

  rpc_x = images[imi].x + images[imi].rx;
  rpc_y = images[imi].y + images[imi].ry;
  pxc_x = images[imi].x;
  pxc_y = images[imi].y - images[imi].height;
  tdX = (pxc_x - rpc_x) * cos(images[imi].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[imi].r * M_PI / 180);
  tdY = (pxc_y - rpc_y) * cos(images[imi].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[imi].r * M_PI / 180);
  tdX *= images[imi].z;
  tdY *= images[imi].z;
  tdX += rpc_x;
  tdY += rpc_y;

  // how we would apply crop to each side based on mouse
  double crop_top_d = ((taY - mouse_canvas_y) * cos(images[imi].r * M_PI / 180) - (taX - mouse_canvas_x) * sin(images[imi].r * M_PI / 180)) / images[imi].z;
  double crop_bottom_d = -((tcY - mouse_canvas_y) * cos(images[imi].r * M_PI / 180) - (tcX - mouse_canvas_x) * sin(images[imi].r * M_PI / 180)) / images[imi].z;
  double crop_left_d = -((taX - mouse_canvas_x) * cos(images[imi].r * M_PI / 180) + (taY - mouse_canvas_y) * sin(images[imi].r * M_PI / 180)) / images[imi].z;
  double crop_right_d = ((tcX - mouse_canvas_x) * cos(images[imi].r * M_PI / 180) + (tcY - mouse_canvas_y) * sin(images[imi].r * M_PI / 180)) / images[imi].z;

  // handles
  double abX = (images[imi].aX + images[imi].bX) / 2.0f;  // top
  double abY = (images[imi].aY + images[imi].bY) / 2.0f;
  double bcX = (images[imi].cX + images[imi].bX) / 2.0f;  // right
  double bcY = (images[imi].cY + images[imi].bY) / 2.0f;
  double cdX = (images[imi].cX + images[imi].dX) / 2.0f;  // bottom
  double cdY = (images[imi].cY + images[imi].dY) / 2.0f;
  double daX = (images[imi].aX + images[imi].dX) / 2.0f;  // left
  double daY = (images[imi].aY + images[imi].dY) / 2.0f;

  // distance to each corner and side (handle distances)
  double d_t = sqrt((mouse_canvas_x - abX) * (mouse_canvas_x - abX) + (mouse_canvas_y - abY) * (mouse_canvas_y - abY));
  double d_tr =
      sqrt((mouse_canvas_x - images[imi].bX) * (mouse_canvas_x - images[imi].bX) + (mouse_canvas_y - images[imi].bY) * (mouse_canvas_y - images[imi].bY));
  double d_r = sqrt((mouse_canvas_x - bcX) * (mouse_canvas_x - bcX) + (mouse_canvas_y - bcY) * (mouse_canvas_y - bcY));
  double d_rb =
      sqrt((mouse_canvas_x - images[imi].cX) * (mouse_canvas_x - images[imi].cX) + (mouse_canvas_y - images[imi].cY) * (mouse_canvas_y - images[imi].cY));
  double d_b = sqrt((mouse_canvas_x - cdX) * (mouse_canvas_x - cdX) + (mouse_canvas_y - cdY) * (mouse_canvas_y - cdY));
  double d_bl =
      sqrt((mouse_canvas_x - images[imi].dX) * (mouse_canvas_x - images[imi].dX) + (mouse_canvas_y - images[imi].dY) * (mouse_canvas_y - images[imi].dY));
  double d_l = sqrt((mouse_canvas_x - daX) * (mouse_canvas_x - daX) + (mouse_canvas_y - daY) * (mouse_canvas_y - daY));
  double d_lt =
      sqrt((mouse_canvas_x - images[imi].aX) * (mouse_canvas_x - images[imi].aX) + (mouse_canvas_y - images[imi].aY) * (mouse_canvas_y - images[imi].aY));

  // find handle cursor is closest to
  int dimin = 0;
  double dvmin = INFINITY;
  double d[] = {d_t, d_tr, d_r, d_rb, d_b, d_bl, d_l, d_lt};
  for (int i = 0; i <= 7; i++) {
    if (d[i] <= dvmin) {
      dvmin = d[i];
      dimin = i;
    }
  }

  switch (dimin) {
    case 0:  // top
      // imrefAx = abX;
      // imrefAy = abY;
      images[imi].crop_top = (int)crop_top_d;
      break;
    case 1:  // top right
      // imrefAx = images[imi].bX;
      // imrefAy = images[imi].bY;
      images[imi].crop_top = (int)crop_top_d;
      images[imi].crop_right = (int)crop_right_d;
      break;
    case 2:  // right
      // imrefAx = bcX;
      // imrefAy = bcY;
      images[imi].crop_right = (int)crop_right_d;
      break;
    case 3:  // right bottom
      // imrefAx = images[imi].cX;
      // imrefAy = images[imi].cY;
      images[imi].crop_right = (int)crop_right_d;
      images[imi].crop_bottom = (int)crop_bottom_d;
      break;
    case 4:  // bottom
      // imrefAx = cdX;
      // imrefAy = cdY;
      images[imi].crop_bottom = (int)crop_bottom_d;
      break;
    case 5:  // bottom left
      // imrefAx = images[imi].dX;
      // imrefAy = images[imi].dY;
      images[imi].crop_bottom = (int)crop_bottom_d;
      images[imi].crop_left = (int)crop_left_d;
      break;
    case 6:  // left
      // imrefAx = daX;
      // imrefAy = daY;
      images[imi].crop_left = (int)crop_left_d;
      break;
    case 7:  // left top
      // imrefAx = images[imi].aX;
      // imrefAy = images[imi].aY;
      images[imi].crop_left = (int)crop_left_d;
      images[imi].crop_top = (int)crop_top_d;
      break;
  }

  if (images[imi].crop_top <= 0) images[imi].crop_top = 0;
  if (images[imi].crop_bottom <= 0) images[imi].crop_bottom = 0;
  if (images[imi].crop_left <= 0) images[imi].crop_left = 0;
  if (images[imi].crop_right <= 0) images[imi].crop_right = 0;
}
void image_uncrop(int imi) {
  images[imi].crop_top = 0;
  images[imi].crop_bottom = 0;
  images[imi].crop_left = 0;
  images[imi].crop_right = 0;
}

void images_render() {
  for (int i = 0; i < images_count; ++i) {
    image_calculate_handles(i);
  }

  sort_images_by(compare_draw_order);
  for (int i = 0; i < images_count; ++i) {  // render all images onto the canvas
    int si = images[i].sort_index;          // sorted index

    bool is_not_offscreen = true;
    // since we already know the image center to the screen center, use it to tell if the image is fully offscreen
    double img_max_from_center =
        sqrt((images[si].width * images[si].width * images[si].z * images[si].z) + (images[si].height * images[si].height * images[si].z * images[si].z)) / 2;
    double screen_max_from_center = sqrt(((screen_size_x * screen_size_x) + (screen_size_y * screen_size_y)) / (cv.z * cv.z * 4));
    if (images[si].center_closeness > img_max_from_center + screen_max_from_center) {
      is_not_offscreen = false;
    }

    bool stop_reload = false;
    if (cv.z * images[si].z <= 1.0 / init_small_image_reduction && auto_hires_discard) {  // discard on zoom out
      image_discard_fullres(si);
      stop_reload = true;
    }
    if (!is_not_offscreen && auto_hires_discard) {  // discard if off screen
      if (images[si].offscreen_frame_count <= DISCARD_HIRES_IF_OFFSCREEN_FOR) {
        images[si].offscreen_frame_count++;
        if (images[si].offscreen_frame_count >= DISCARD_HIRES_IF_OFFSCREEN_FOR) {
          image_discard_fullres(si);
          stop_reload = true;
        }
      }
    }

    if (!stop_reload && is_not_offscreen && auto_hires_restore) {  // restore if onscreen
      if (image_point_inside(mouse_canvas_x, mouse_canvas_y, si)) {
        image_restore_fullres(si);
      }
    }

    // crop can't be negative
    if (images[si].crop_left < 0) images[si].crop_left = 0;
    if (images[si].crop_right < 0) images[si].crop_right = 0;
    if (images[si].crop_top < 0) images[si].crop_top = 0;
    if (images[si].crop_bottom < 0) images[si].crop_bottom = 0;

    // rotation from image
    double lrX = (images[si].rx * images[si].z) * cos(images[si].r * M_PI / 180) - (images[si].ry * images[si].z) * sin(images[si].r * M_PI / 180);
    double lrY = (images[si].ry * images[si].z) * cos(images[si].r * M_PI / 180) + (images[si].rx * images[si].z) * sin(images[si].r * M_PI / 180);

    double lX = (images[si].x - lrX) * cv.z;
    double lY = -(images[si].y - lrY) * cv.z;
    // rotation from crop offset
    if (images[si].fullres_exists) {
      lX += (images[si].crop_left * cv.z * images[si].z) * cos(-images[si].r * M_PI / 180) -
            (images[si].crop_top * cv.z * images[si].z) * sin(-images[si].r * M_PI / 180);
      lY += (images[si].crop_top * cv.z * images[si].z) * cos(-images[si].r * M_PI / 180) +
            (images[si].crop_left * cv.z * images[si].z) * sin(-images[si].r * M_PI / 180);
    } else {
      lX += ((int)(images[si].crop_left / init_small_image_reduction) * init_small_image_reduction * cv.z * images[si].z) * cos(-images[si].r * M_PI / 180) -
            ((int)(images[si].crop_top / init_small_image_reduction) * init_small_image_reduction * cv.z * images[si].z) * sin(-images[si].r * M_PI / 180);
      lY += ((int)(images[si].crop_top / init_small_image_reduction) * init_small_image_reduction * cv.z * images[si].z) * cos(-images[si].r * M_PI / 180) +
            ((int)(images[si].crop_left / init_small_image_reduction) * init_small_image_reduction * cv.z * images[si].z) * sin(-images[si].r * M_PI / 180);
    }
    // rotation from canvas
    double tX = (lX + (-cv.x + images[si].rx) * cv.z) * cos(-cv.r * M_PI / 180) - (lY + (cv.y - images[si].ry) * cv.z) * sin(-cv.r * M_PI / 180) +
                (screen_size_x / 2.0f);  // rotate image into place
    double tY = (lY + (cv.y - images[si].ry) * cv.z) * cos(-cv.r * M_PI / 180) + (lX + (-cv.x + images[si].rx) * cv.z) * sin(-cv.r * M_PI / 180) +
                (screen_size_y / 2.0f);

    // apply crop
    SDL_Rect src;
    src.x = images[si].crop_left;
    src.y = images[si].crop_top;
    src.w = images[si].width - images[si].crop_left - images[si].crop_right;
    src.h = images[si].height - images[si].crop_top - images[si].crop_bottom;
    if (!images[si].fullres_exists) {
      src.x = images[si].crop_left / init_small_image_reduction;
      src.y = images[si].crop_top / init_small_image_reduction;
      src.w = (images[si].width / init_small_image_reduction - images[si].crop_left / init_small_image_reduction -
               images[si].crop_right / init_small_image_reduction);
      src.h = (images[si].height / init_small_image_reduction - images[si].crop_top / init_small_image_reduction -
               images[si].crop_bottom / init_small_image_reduction);
    }

    SDL_Rect dst;
    dst.x = (int)(tX);
    dst.y = (int)(tY);
    if (images[si].fullres_exists) {
      dst.w = (int)(src.w * cv.z * images[si].z);
      dst.h = (int)(src.h * cv.z * images[si].z);
    } else {
      dst.w = (int)(src.w * init_small_image_reduction * cv.z * images[si].z);
      dst.h = (int)(src.h * init_small_image_reduction * cv.z * images[si].z);
    }

    if (is_not_offscreen) {
      images[si].offscreen_frame_count = 0;
      SDL_Point rp = {0, 0};
      if (images[si].fullres_exists) {
        SDL_SetTextureAlphaMod(images[si].texture_fullres, images[si].opacity);
        SDL_RenderCopyEx(renderer, images[si].texture_fullres, &src, &dst, (-cv.r - images[si].r), &rp, SDL_FLIP_NONE);
      } else {
        SDL_SetTextureAlphaMod(images[si].texture_small, images[si].opacity);
        SDL_RenderCopyEx(renderer, images[si].texture_small, &src, &dst, (-cv.r - images[si].r), &rp, SDL_FLIP_NONE);
      }
    }
  }
}

void images_unload() {
  for (int i = 0; i < images_count; ++i) {
    if (images[i].fullres_exists) {
      SDL_DestroyTexture(images[i].texture_fullres);
      images[i].fullres_exists = false;
    }
    SDL_DestroyTexture(images[i].texture_small);
  }
  images_count = 0;
}
void images_free() {
  images_unload();
  free(images);
}

void canvas_use_cvp(int ci) {
  if (ci >= MAX_CANVAS) return;
  cv = cvp[ci];
  image_series_set(cv.selected_imi);
  for (int i = 0; i < images_count; ++i) {
    if(images[i].fullres_in_view[ci] == 1){
      image_restore_fullres(i);
    }
  }
}
void canvas_set_cvp(int ci) {
  if (ci >= MAX_CANVAS) return;
  cvp[ci] = cv;
  for (int i = 0; i < images_count; ++i) {
    if(images[i].fullres_exists){
      images[i].fullres_in_view[ci] = 1;
    }else{
      images[i].fullres_in_view[ci] = 0;
    }
  }
}