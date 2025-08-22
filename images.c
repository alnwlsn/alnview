#include "images.h"

Image *images = NULL;
int images_count = 0;
int series_current = 0;

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
    rectangleCorners s = image_find_corners(i);
    double xs[] = {s.aX, s.bX, s.cX, s.dX};
    double ys[] = {s.aY, s.bY, s.cY, s.dY};
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

int image_load(char *filepath) {  // loads image at filepath, inits width and height
  if (images_count >= MAX_IMAGES) {
    fprintf(stderr, "Too many images to load\n");
  }
  // img->texture = load_texture(renderer, filepath, &img->width, &img->height);
  SDL_Surface *surface = IMG_Load(filepath);
  if (surface) {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    if (texture) {
      images = realloc(images, sizeof(Image) * (images_count + 1));
      Image *img = &images[images_count];
      img->texture = texture;
      img->width = surface->w;
      img->height = surface->h;
      img->inited = 0;
      memcpy(img->filepath, filepath, FILEPATHLEN);
      images_count++;
    } else {
      fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
    }
    SDL_FreeSurface(surface);
  } else {
    fprintf(stderr, "Failed to load %s: %s\n", filepath, IMG_GetError());
  }
  return images_count - 1;
  // fprintf(stdout, "img %d\n", images_count);
}

void images_load_dir(const char *directory, bool show) {  // load all images from directory
  bool no_savefile = 1;
  DIR *dir = opendir(directory);
  if (!dir) {
    perror("Failed to open image directory");
    exit(1);
  }
  struct dirent *entry;
  // collect image files in dir
  while ((entry = readdir(dir))) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, SAVEFILE) == 0) continue;
    char path[512];
    snprintf(path, sizeof(path) - 8, "%s/%s", directory, entry->d_name);
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
  }
  cv.selected_imi = images[0].sort_index;
  if (no_savefile) images_arrange_in_grid();
}

int image_point_inside(double px, double py, rectangleCorners s) {
  // take cross product of each point with target. if all in same direction, is inside
  double c1 = (s.bX - s.aX) * (py - s.aY) - (s.bY - s.aY) * (px - s.aX);
  double c2 = (s.cX - s.bX) * (py - s.bY) - (s.cY - s.bY) * (px - s.bX);
  double c3 = (s.dX - s.cX) * (py - s.cY) - (s.dY - s.cY) * (px - s.cX);
  double c4 = (s.aX - s.dX) * (py - s.dY) - (s.aY - s.dY) * (px - s.dX);

  return ((c1 >= 0 && c2 >= 0 && c3 >= 0 && c4 >= 0) || (c1 <= 0 && c2 <= 0 && c3 <= 0 && c4 <= 0));
}
rectangleCorners image_find_corners(int si) {  // finds canvas coords for 4 corners of image
  rectangleCorners s;
  s.aX = 0;
  s.aY = 0;
  s.bX = 0;
  s.bY = 0;
  s.cX = 0;
  s.cY = 0;
  s.dX = 0;
  s.dY = 0;

  double rpc_x = images[si].x + images[si].rx;
  double rpc_y = images[si].y + images[si].ry;
  double pxc_x = images[si].x + images[si].crop_left;
  double pxc_y = images[si].y - images[si].crop_top;
  s.aX = (pxc_x - rpc_x) * cos(images[si].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[si].r * M_PI / 180);
  s.aY = (pxc_y - rpc_y) * cos(images[si].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[si].r * M_PI / 180);
  s.aX *= images[si].z;
  s.aY *= images[si].z;
  s.aX += rpc_x;
  s.aY += rpc_y;

  rpc_x = images[si].x + images[si].rx;
  rpc_y = images[si].y + images[si].ry;
  pxc_x = images[si].x + images[si].width - images[si].crop_right;
  pxc_y = images[si].y - images[si].crop_top;
  s.bX = (pxc_x - rpc_x) * cos(images[si].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[si].r * M_PI / 180);
  s.bY = (pxc_y - rpc_y) * cos(images[si].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[si].r * M_PI / 180);
  s.bX *= images[si].z;
  s.bY *= images[si].z;
  s.bX += rpc_x;
  s.bY += rpc_y;

  rpc_x = images[si].x + images[si].rx;
  rpc_y = images[si].y + images[si].ry;
  pxc_x = images[si].x + images[si].width - images[si].crop_right;
  pxc_y = images[si].y - images[si].height + images[si].crop_bottom;
  s.cX = (pxc_x - rpc_x) * cos(images[si].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[si].r * M_PI / 180);
  s.cY = (pxc_y - rpc_y) * cos(images[si].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[si].r * M_PI / 180);
  s.cX *= images[si].z;
  s.cY *= images[si].z;
  s.cX += rpc_x;
  s.cY += rpc_y;

  rpc_x = images[si].x + images[si].rx;
  rpc_y = images[si].y + images[si].ry;
  pxc_x = images[si].x + images[si].crop_left;
  pxc_y = images[si].y - images[si].height + images[si].crop_bottom;
  s.dX = (pxc_x - rpc_x) * cos(images[si].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[si].r * M_PI / 180);
  s.dY = (pxc_y - rpc_y) * cos(images[si].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[si].r * M_PI / 180);
  s.dX *= images[si].z;
  s.dY *= images[si].z;
  s.dX += rpc_x;
  s.dY += rpc_y;
  return s;
}

rectangleCorners image_find_corners_noncrop(int si) {  // finds canvas coords for 4 corners of image
  rectangleCorners s;
  s.aX = 0;
  s.aY = 0;
  s.bX = 0;
  s.bY = 0;
  s.cX = 0;
  s.cY = 0;
  s.dX = 0;
  s.dY = 0;

  double rpc_x = images[si].x + images[si].rx;
  double rpc_y = images[si].y + images[si].ry;
  double pxc_x = images[si].x;
  double pxc_y = images[si].y;
  s.aX = (pxc_x - rpc_x) * cos(images[si].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[si].r * M_PI / 180);
  s.aY = (pxc_y - rpc_y) * cos(images[si].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[si].r * M_PI / 180);
  s.aX *= images[si].z;
  s.aY *= images[si].z;
  s.aX += rpc_x;
  s.aY += rpc_y;

  rpc_x = images[si].x + images[si].rx;
  rpc_y = images[si].y + images[si].ry;
  pxc_x = images[si].x + images[si].width;
  pxc_y = images[si].y;
  s.bX = (pxc_x - rpc_x) * cos(images[si].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[si].r * M_PI / 180);
  s.bY = (pxc_y - rpc_y) * cos(images[si].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[si].r * M_PI / 180);
  s.bX *= images[si].z;
  s.bY *= images[si].z;
  s.bX += rpc_x;
  s.bY += rpc_y;

  rpc_x = images[si].x + images[si].rx;
  rpc_y = images[si].y + images[si].ry;
  pxc_x = images[si].x + images[si].width;
  pxc_y = images[si].y - images[si].height;
  s.cX = (pxc_x - rpc_x) * cos(images[si].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[si].r * M_PI / 180);
  s.cY = (pxc_y - rpc_y) * cos(images[si].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[si].r * M_PI / 180);
  s.cX *= images[si].z;
  s.cY *= images[si].z;
  s.cX += rpc_x;
  s.cY += rpc_y;

  rpc_x = images[si].x + images[si].rx;
  rpc_y = images[si].y + images[si].ry;
  pxc_x = images[si].x;
  pxc_y = images[si].y - images[si].height;
  s.dX = (pxc_x - rpc_x) * cos(images[si].r * M_PI / 180) - (pxc_y - rpc_y) * sin(images[si].r * M_PI / 180);
  s.dY = (pxc_y - rpc_y) * cos(images[si].r * M_PI / 180) + (pxc_x - rpc_x) * sin(images[si].r * M_PI / 180);
  s.dX *= images[si].z;
  s.dY *= images[si].z;
  s.dX += rpc_x;
  s.dY += rpc_y;

  return s;
}

void image_find_center(int si, double *cX, double *cY) {
  rectangleCorners s = image_find_corners(si);
  *cX = (s.aX + s.bX + s.cX + s.dX) / 4.0f;
  *cY = (s.aY + s.bY + s.cY + s.dY) / 4.0f;
}
void image_find_center_noncrop(int si, double *cX, double *cY) {
  rectangleCorners s = image_find_corners_noncrop(si);
  *cX = (s.aX + s.bX + s.cX + s.dX) / 4.0f;
  *cY = (s.aY + s.bY + s.cY + s.dY) / 4.0f;
}

int image_point_on(double x, double y) {  // tells which image is under the point
  int image_index = -1;
  int c_draw_order = -999999;
  for (int i = 0; i < images_count; ++i) {
    if (image_point_inside(x, y, image_find_corners(i))) {
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
  image_find_center(imi, &cv.x, &cv.y);
  // rectangleCorners s = image_find_corners(imi);
  // double cX = (s.aX + s.bX + s.cX + s.dX) / 4.0f;
  // double cY = (s.aY + s.bY + s.cY + s.dY) / 4.0f;
  // cv.x = cX;  // center view
  // cv.y = cY;
  cv.selected_imi = imi;
  series_current = images[cv.selected_imi].series_order;
}

void canvas_center_on_nearest_image_in_direction(int imi, double direction) {
  double cX, cY;
  image_find_center(imi, &cX, &cY);
  double nearestDistance = INFINITY;
  int cimi = -1;

  for (int i = 0; i < images_count; i++) {
    if (i != imi) {
      double dX, dY;
      image_find_center(i, &dX, &dY);

      double distance = sqrt(((dX - cX) * (dX - cX)) + ((dY - cY) * (dY - cY)));
      double angle = (180 / M_PI) * atan2((cY - dY), (dX - cX));

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
  double cX, cY;
  image_find_center(imi, &cX, &cY);
  image_rotation_point_set_new(imi, cX, cY);
}
void canvas_center_on_image_fit(int imi) {
  // currently doesn't work with rotated canvas
  if (imi < 0) return;
  rectangleCorners s = image_find_corners(imi);
  double cX = (s.aX + s.bX + s.cX + s.dX) / 4.0f;
  double cY = (s.aY + s.bY + s.cY + s.dY) / 4.0f;
  cv.x = cX;  // center view
  cv.y = cY;
  double eX = s.aX, eY = s.aY;
  double xs[] = {s.aX, s.bX, s.cX, s.dX};
  double ys[] = {s.aY, s.bY, s.cY, s.dY};
  for (int i = 0; i <= 3; i++) {
    if (xs[i] <= eX) eX = xs[i];
    if (ys[i] <= eY) eY = ys[i];
  }
  eX = cX - eX;
  eY = cY - eY;
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
  rectangleCorners s = image_find_corners_noncrop(imi);

  // how we would apply crop to each side based on mouse
  double crop_top_d = ((s.aY - mouse_canvas_y) * cos(images[imi].r * M_PI / 180) - (s.aX - mouse_canvas_x) * sin(images[imi].r * M_PI / 180)) / images[imi].z;
  double crop_bottom_d =
      -((s.cY - mouse_canvas_y) * cos(images[imi].r * M_PI / 180) - (s.cX - mouse_canvas_x) * sin(images[imi].r * M_PI / 180)) / images[imi].z;
  double crop_left_d = -((s.aX - mouse_canvas_x) * cos(images[imi].r * M_PI / 180) + (s.aY - mouse_canvas_y) * sin(images[imi].r * M_PI / 180)) / images[imi].z;
  double crop_right_d = ((s.cX - mouse_canvas_x) * cos(images[imi].r * M_PI / 180) + (s.cY - mouse_canvas_y) * sin(images[imi].r * M_PI / 180)) / images[imi].z;

  // handles
  s = image_find_corners(imi);
  double abX = (s.aX + s.bX) / 2.0f;  // top
  double abY = (s.aY + s.bY) / 2.0f;
  double bcX = (s.cX + s.bX) / 2.0f;  // right
  double bcY = (s.cY + s.bY) / 2.0f;
  double cdX = (s.cX + s.dX) / 2.0f;  // bottom
  double cdY = (s.cY + s.dY) / 2.0f;
  double daX = (s.aX + s.dX) / 2.0f;  // left
  double daY = (s.aY + s.dY) / 2.0f;

  // distance to each corner and side (handle distances)
  double d_t = sqrt((mouse_canvas_x - abX) * (mouse_canvas_x - abX) + (mouse_canvas_y - abY) * (mouse_canvas_y - abY));
  double d_tr = sqrt((mouse_canvas_x - s.bX) * (mouse_canvas_x - s.bX) + (mouse_canvas_y - s.bY) * (mouse_canvas_y - s.bY));
  double d_r = sqrt((mouse_canvas_x - bcX) * (mouse_canvas_x - bcX) + (mouse_canvas_y - bcY) * (mouse_canvas_y - bcY));
  double d_rb = sqrt((mouse_canvas_x - s.cX) * (mouse_canvas_x - s.cX) + (mouse_canvas_y - s.cY) * (mouse_canvas_y - s.cY));
  double d_b = sqrt((mouse_canvas_x - cdX) * (mouse_canvas_x - cdX) + (mouse_canvas_y - cdY) * (mouse_canvas_y - cdY));
  double d_bl = sqrt((mouse_canvas_x - s.dX) * (mouse_canvas_x - s.dX) + (mouse_canvas_y - s.dY) * (mouse_canvas_y - s.dY));
  double d_l = sqrt((mouse_canvas_x - daX) * (mouse_canvas_x - daX) + (mouse_canvas_y - daY) * (mouse_canvas_y - daY));
  double d_lt = sqrt((mouse_canvas_x - s.aX) * (mouse_canvas_x - s.aX) + (mouse_canvas_y - s.aY) * (mouse_canvas_y - s.aY));

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
      // imrefAx = s.bX;
      // imrefAy = s.bY;
      images[imi].crop_top = (int)crop_top_d;
      images[imi].crop_right = (int)crop_right_d;
      break;
    case 2:  // right
      // imrefAx = bcX;
      // imrefAy = bcY;
      images[imi].crop_right = (int)crop_right_d;
      break;
    case 3:  // right bottom
      // imrefAx = s.cX;
      // imrefAy = s.cY;
      images[imi].crop_right = (int)crop_right_d;
      images[imi].crop_bottom = (int)crop_bottom_d;
      break;
    case 4:  // bottom
      // imrefAx = cdX;
      // imrefAy = cdY;
      images[imi].crop_bottom = (int)crop_bottom_d;
      break;
    case 5:  // bottom left
      // imrefAx = s.dX;
      // imrefAy = s.dY;
      images[imi].crop_bottom = (int)crop_bottom_d;
      images[imi].crop_left = (int)crop_left_d;
      break;
    case 6:  // left
      // imrefAx = daX;
      // imrefAy = daY;
      images[imi].crop_left = (int)crop_left_d;
      break;
    case 7:  // left top
      // imrefAx = s.aX;
      // imrefAy = s.aY;
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
  sort_images_by(compare_draw_order);
  for (int i = 0; i < images_count; ++i) {  // render all images onto the canvas
    int si = images[i].sort_index;          // sorted index

    // // test override
    // if (images[si].series_order == 1) {
    //   // images[si].rx = 50;
    //   // images[si].ry = -50;
    //   // images[si].crop_left = global_testC;
    //   // images[si].crop_top = global_testC / 2;
    //   // images[si].crop_right = global_testC;
    //   // images[si].crop_bottom = global_testC / 2;
    //   // images[si].r = global_testA;
    //   // images[si].z = (1.0f + global_testB / 10.0);
    // }

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
    lX += (images[si].crop_left * cv.z * images[si].z) * cos(-images[si].r * M_PI / 180) -
          (images[si].crop_top * cv.z * images[si].z) * sin(-images[si].r * M_PI / 180);
    lY += (images[si].crop_top * cv.z * images[si].z) * cos(-images[si].r * M_PI / 180) +
          (images[si].crop_left * cv.z * images[si].z) * sin(-images[si].r * M_PI / 180);
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

    SDL_Rect dst;
    dst.x = (int)(tX);
    dst.y = (int)(tY);
    dst.w = (int)(src.w * cv.z * images[si].z);
    dst.h = (int)(src.h * cv.z * images[si].z);

    SDL_Point rp = {0, 0};
    SDL_SetTextureAlphaMod(images[si].texture, images[si].opacity);
    SDL_RenderCopyEx(renderer, images[si].texture, &src, &dst, (-cv.r - images[si].r), &rp, SDL_FLIP_NONE);
  }
}

void images_unload() {
  for (int i = 0; i < images_count; ++i) SDL_DestroyTexture(images[i].texture);
  images_count = 0;
}

void images_free() {
  images_unload();
  free(images);
}