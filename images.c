#include "images.h"

#include "controls.h"

Image *images = NULL;
int images_count = 0;
float imrefAx = 0;
float imrefAy = 0;
float imrefBx = 0;
float imrefBy = 0;

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
    images[si].rx = 0;
    images[si].ry = 0;
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
    images[i].rx = 0;
    images[i].ry = 0;
    images[i].z = 1;
    images[images[i].sort_index].draw_order = i;
    images[images[i].sort_index].sel_order = i;
  }
  images_arrange_in_grid();
}

void images_render() {
  sort_images_by(compare_draw_order);
  for (int i = 0; i < images_count; ++i) {  // render all images onto the canvas
    int si = images[i].sort_index;          // sorted index

    // test override
    if (images[si].draw_order == 1) {
      images[si].rx = 50;
      images[si].ry = -50;
      // images[si].crop_left = 0;
      images[si].r = global_testA;
      images[si].z = (1.0f + global_testB / 10.0);

      imrefBx = images[si].x + images[si].rx * images[si].z;
      imrefBy = images[si].y + images[si].ry * images[si].z;
    }

    // float lrX = (images[si].rx*images[si].z);
    // float lrY = (images[si].ry*images[si].z);
    float lrX = (images[si].rx * images[si].z) * cos(images[si].r * M_PI / 180) -
                (images[si].ry * images[si].z) * sin(images[si].r * M_PI / 180);  // rotate image into place
    float lrY = (images[si].ry * images[si].z) * cos(images[si].r * M_PI / 180) + (images[si].rx * images[si].z) * sin(images[si].r * M_PI / 180);
    // imrefAx = lrdX + images[si].x;
    // imrefAy = lrdY + images[si].y;

    if (images[si].draw_order == 1) {
      imrefAx = lrX;
      imrefAy = lrY;
    }

    float lX = (images[si].x - lrX) * cv.z;
    float lY = -(images[si].y - lrY) * cv.z;
    // rotation from crop offset
    lX += (images[si].crop_left * cv.z * images[si].z) * cos(-images[si].r * M_PI / 180) -
          (images[si].crop_top * cv.z * images[si].z) * sin(-images[si].r * M_PI / 180);
    lY += (images[si].crop_top * cv.z * images[si].z) * cos(-images[si].r * M_PI / 180) +
          (images[si].crop_left * cv.z * images[si].z) * sin(-images[si].r * M_PI / 180);
    // rotation from canvas
    float tX = lX * cos(-cv.r * M_PI / 180) - lY * sin(-cv.r * M_PI / 180);  // rotate image into place
    float tY = lY * cos(-cv.r * M_PI / 180) + lX * sin(-cv.r * M_PI / 180);
    tX += screen_size_x / 2.0f;
    tY += screen_size_y / 2.0f;
    tX -= cv.x * cv.z * cos(-cv.r * M_PI / 180) + cv.y * cv.z * sin(-cv.r * M_PI / 180);  // add in the rotated centroid point
    tY -= -cv.y * cv.z * cos(-cv.r * M_PI / 180) + cv.x * cv.z * sin(-cv.r * M_PI / 180);
    tX += images[si].rx * cv.z * cos(-cv.r * M_PI / 180) + images[si].ry * cv.z * sin(-cv.r * M_PI / 180);  // add in the rotated 
    tY += -images[si].ry * cv.z * cos(-cv.r * M_PI / 180) + images[si].rx * cv.z * sin(-cv.r * M_PI / 180);


    // dst.x = (int)tX;
    // dst.y = (int)tY;
    // dst.w = (int)((images[si].width) * cv.z);
    // dst.h = (int)(images[si].height * cv.z);
    // SDL_Point rp = {0, 0};
    // SDL_RenderCopyEx(renderer, images[si].texture, &src, &dst, -cv.r, &rp, SDL_FLIP_NONE);
    //  SDL_RenderCopyEx(renderer, images[si].texture, NULL, &dst, world_r, NULL, SDL_FLIP_NONE);

    // Calculate the source rectangle (what part of the texture to take)
    SDL_Rect src;
    src.x = images[si].crop_left;
    src.y = images[si].crop_top;
    src.w = images[si].width - images[si].crop_left - images[si].crop_right;
    src.h = images[si].height - images[si].crop_top - images[si].crop_bottom;

    // Calculate the destination rectangle (scaled + positioned)
    SDL_Rect dst;
    dst.x = (int)(tX);
    dst.y = (int)(tY);
    dst.w = (int)(src.w * cv.z * images[si].z);  // scale after cropping
    dst.h = (int)(src.h * cv.z * images[si].z);

    // Rotation pivot (relative to destination rect's top-left)
    SDL_Point rp = {0, 0};

    // Render
    SDL_RenderCopyEx(renderer, images[si].texture, &src, &dst, (-cv.r - images[si].r), &rp, SDL_FLIP_NONE);
  }
}

void images_free() {
  for (int i = 0; i < images_count; ++i) SDL_DestroyTexture(images[i].texture);
  free(images);
}