#include "images.h"

#include "controls.h"

Image *images = NULL;
int images_count = 0;
float imrefAx = 0;
float imrefAy = 0;
float imrefBx = 0;
float imrefBy = 0;
float imrefCx = 0;
float imrefCy = 0;
float imrefDx = 0;
float imrefDy = 0;

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
int compare_sel_order(const void *a, const void *b) {
  int i = *(const int *)a;
  int j = *(const int *)b;
  if (sort_images[i].sel_order < sort_images[j].sel_order) return -1;
  if (sort_images[i].sel_order > sort_images[j].sel_order) return 1;
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
  sort_images_by(compare_sel_order);
  for (int i = 0; i < images_count; ++i) {
    int si = images[i].sort_index;
    images[si].x = x;
    images[si].y = -y;
    images[si].r = 0;
    images[si].z = 1;
    images[si].crop_top = 0;
    images[si].crop_right = 0;
    images[si].crop_bottom = 0;
    images[si].crop_left = 0;
    x += images[si].width + spacing;
    if (images[si].height > max_row_height) max_row_height = images[si].height;

    if ((i + 1) % columns == 0) {
      x = 0;
      y += max_row_height + spacing;
      max_row_height = 0;
    }
  }
}

void image_load(char *filepath) {  // loads image at filepath, inits width and height
  if (images_count >= MAX_IMAGES) {
    fprintf(stderr, "Too many images to load\n");
  }
  const char *ext = strrchr(filepath, '.');
  if (!ext || (strcasecmp(ext, ".png") != 0 && strcasecmp(ext, ".jpg") != 0 && strcasecmp(ext, ".jpeg") != 0)) {
    fprintf(stderr, "Can't load file %s\n", filepath);
  } else {
    // make room and load the image
    images = realloc(images, sizeof(Image) * (images_count + 1));
    Image *img = &images[images_count];
    // img->texture = load_texture(renderer, filepath, &img->width, &img->height);
    SDL_Surface *surface = IMG_Load(filepath);
    if (surface) {
      SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
      if (texture) {
        fprintf(stdout, "loaded %s\n", filepath);
        img->texture = texture;
        img->width = surface->w;
        img->height = surface->h;
        memcpy(img->filepath, filepath, FILEPATHLEN);
        images_count++;
      } else {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
      }
      SDL_FreeSurface(surface);
    } else {
      fprintf(stderr, "Failed to load %s: %s\n", filepath, IMG_GetError());
    }
  }
  // fprintf(stdout, "img %d\n", images_count);
}

void images_load_dir(const char *directory) {  // load all images from directory
  DIR *dir = opendir(directory);
  if (!dir) {
    perror("Failed to open image directory");
    exit(1);
  }
  struct dirent *entry;
  // collect image files in dir
  while ((entry = readdir(dir))) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
    char path[512];
    snprintf(path, sizeof(path) - 8, "%s/%s", directory, entry->d_name);
    struct stat path_stat;
    if (stat(path, &path_stat) != 0 || !S_ISREG(path_stat.st_mode)) continue;
    image_load(path);
  }
  closedir(dir);

  // initialize images
  sort_images_by(compare_filepath);
  for (int i = 0; i < images_count; i++) {
    images[i].x = 0;
    images[i].y = 0;
    images[i].r = 0;
    images[i].rx = images[i].width/2;
    images[i].ry = -images[i].height/2;
    images[i].z = 1;
    images[images[i].sort_index].draw_order = i;
    images[images[i].sort_index].sel_order = i;
  }
  images_arrange_in_grid();
}

int image_point_inside(double px, double py, rectangleCorners s) {
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

  float rpc_x = images[si].x + images[si].rx;
  float rpc_y = images[si].y + images[si].ry;
  float pxc_x = images[si].x + images[si].crop_left;
  float pxc_y = images[si].y - images[si].crop_top;
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
int image_point_on(float x, float y) {  // tells which image is under the point
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
  float sdx = -dx / cv.z;
  float sdy = -dy / cv.z;
  float cdx = sdx * cos(-cv.r * M_PI / 180) - sdy * sin(-cv.r * M_PI / 180);
  float cdy = sdy * cos(-cv.r * M_PI / 180) + sdx * sin(-cv.r * M_PI / 180);
  images[imi].x -= cdx;
  images[imi].y -= cdy;
}

void image_zoom_by(int imi, float zoom_factor) {
  if (imi < 0) return;
  float Ax = mouse_canvas_x - images[imi].x - images[imi].rx;
  float Ay = mouse_canvas_y - images[imi].y - images[imi].ry;
  images[imi].z *= zoom_factor;
  float Bx = (mouse_canvas_x - images[imi].x - images[imi].rx) * zoom_factor;
  float By = (mouse_canvas_y - images[imi].y - images[imi].ry) * zoom_factor;
  images[imi].x -= (Bx - Ax);
  images[imi].y -= (By - Ay);
}

void image_zoom_reset(int imi) {
  if (imi < 0) return;
  image_zoom_by(imi, 1 / images[imi].z);
  images[imi].z = 1;
}

void image_rotate_by(int imi, float dr) {
  if (imi < 0) return;
  images[imi].r += dr;
}

void images_render() {
  sort_images_by(compare_draw_order);
  for (int i = 0; i < images_count; ++i) {  // render all images onto the canvas
    int si = images[i].sort_index;          // sorted index

    // test override
    if (images[si].sel_order == 1) {
      images[si].rx = 50;
      images[si].ry = -50;
      images[si].crop_left = global_testC;
      images[si].crop_top = global_testC / 2;
      images[si].crop_right = global_testC;
      images[si].crop_bottom = global_testC / 2;
      // images[si].r = global_testA;
      // images[si].z = (1.0f + global_testB / 10.0);
    }

    // crop can't be negative
    if (images[si].crop_left < 0) images[si].crop_left = 0;
    if (images[si].crop_right < 0) images[si].crop_right = 0;
    if (images[si].crop_top < 0) images[si].crop_top = 0;
    if (images[si].crop_bottom < 0) images[si].crop_bottom = 0;

    // rotation from image
    float lrX = (images[si].rx * images[si].z) * cos(images[si].r * M_PI / 180) - (images[si].ry * images[si].z) * sin(images[si].r * M_PI / 180);
    float lrY = (images[si].ry * images[si].z) * cos(images[si].r * M_PI / 180) + (images[si].rx * images[si].z) * sin(images[si].r * M_PI / 180);

    float lX = (images[si].x - lrX) * cv.z;
    float lY = -(images[si].y - lrY) * cv.z;
    // rotation from crop offset
    lX += (images[si].crop_left * cv.z * images[si].z) * cos(-images[si].r * M_PI / 180) -
          (images[si].crop_top * cv.z * images[si].z) * sin(-images[si].r * M_PI / 180);
    lY += (images[si].crop_top * cv.z * images[si].z) * cos(-images[si].r * M_PI / 180) +
          (images[si].crop_left * cv.z * images[si].z) * sin(-images[si].r * M_PI / 180);
    // rotation from canvas
    float tX = (lX + (-cv.x + images[si].rx) * cv.z) * cos(-cv.r * M_PI / 180) - (lY + (cv.y - images[si].ry) * cv.z) * sin(-cv.r * M_PI / 180) +
               (screen_size_x / 2.0f);  // rotate image into place
    float tY = (lY + (cv.y - images[si].ry) * cv.z) * cos(-cv.r * M_PI / 180) + (lX + (-cv.x + images[si].rx) * cv.z) * sin(-cv.r * M_PI / 180) +
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
    SDL_RenderCopyEx(renderer, images[si].texture, &src, &dst, (-cv.r - images[si].r), &rp, SDL_FLIP_NONE);
  }
}

void images_free() {
  for (int i = 0; i < images_count; ++i) SDL_DestroyTexture(images[i].texture);
  free(images);
}