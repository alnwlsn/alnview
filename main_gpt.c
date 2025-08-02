#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

char img_dir[512] = ".";

typedef struct {
  SDL_Texture *texture;
  int width;
  int height;
  int x;
  int y;
} Image;

Image *images = NULL;
int image_count = 0;

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *path, int *w, int *h) {
  SDL_Surface *surface = IMG_Load(path);
  if (!surface) {
    fprintf(stderr, "Failed to load %s: %s\n", path, IMG_GetError());
    return NULL;
  } else {
    fprintf(stdout, "loaded %s\n", path);
  }
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!texture) {
    fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
    SDL_FreeSurface(surface);
    return NULL;
  }
  *w = surface->w;
  *h = surface->h;
  SDL_FreeSurface(surface);
  return texture;
}

typedef struct {
  char name[256];
} FileEntry;

int compare_entries(const void *a, const void *b) {
  const FileEntry *fa = (const FileEntry *)a;
  const FileEntry *fb = (const FileEntry *)b;
  return strcmp(fa->name, fb->name);
}

void load_images(SDL_Renderer *renderer, const char *directory) {
  DIR *dir = opendir(directory);
  if (!dir) {
    perror("Failed to open image directory");
    exit(1);
  }

  struct dirent *entry;
  FileEntry *file_list = NULL;
  int file_count = 0;

  // First: collect valid image files
  while ((entry = readdir(dir))) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

    char path[512];
    snprintf(path, sizeof(path) - 8, "%s/%s", directory, entry->d_name);

    struct stat path_stat;
    if (stat(path, &path_stat) != 0 || !S_ISREG(path_stat.st_mode)) continue;

    // Optional: only allow common image extensions
    const char *ext = strrchr(entry->d_name, '.');
    if (!ext || (strcasecmp(ext, ".png") != 0 && strcasecmp(ext, ".jpg") != 0 && strcasecmp(ext, ".jpeg") != 0)) continue;

    file_list = realloc(file_list, sizeof(FileEntry) * (file_count + 1));
    snprintf(file_list[file_count].name, sizeof(file_list[file_count].name), "%s", entry->d_name);
    file_count++;
  }
  closedir(dir);

  // Sort file list alphabetically
  qsort(file_list, file_count, sizeof(FileEntry), compare_entries);

  // Load images in sorted order
  for (int i = 0; i < file_count; ++i) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", directory, file_list[i].name);

    images = realloc(images, sizeof(Image) * (image_count + 1));
    Image *img = &images[image_count];

    img->texture = load_texture(renderer, path, &img->width, &img->height);
    if (!img->texture) continue;

    image_count++;
  }

  free(file_list);
}

void arrange_images_in_grid() {
  int columns = ceil(sqrt(image_count));
  int spacing = 20;

  int x = 0, y = 0;
  int max_row_height = 0;

  for (int i = 0; i < image_count; ++i) {
    images[i].x = x;
    images[i].y = y;

    x += images[i].width + spacing;
    if (images[i].height > max_row_height) max_row_height = images[i].height;

    if ((i + 1) % columns == 0) {
      x = 0;
      y += max_row_height + spacing;
      max_row_height = 0;
    }
  }
}

void get_image_bounds(int *min_x, int *min_y, int *max_x, int *max_y) {
  if (image_count == 0) {
    *min_x = *min_y = *max_x = *max_y = 0;
    return;
  }

  *min_x = images[0].x;
  *min_y = images[0].y;
  *max_x = images[0].x + images[0].width;
  *max_y = images[0].y + images[0].height;

  for (int i = 1; i < image_count; ++i) {
    int x1 = images[i].x;
    int y1 = images[i].y;
    int x2 = x1 + images[i].width;
    int y2 = y1 + images[i].height;

    if (x1 < *min_x) *min_x = x1;
    if (y1 < *min_y) *min_y = y1;
    if (x2 > *max_x) *max_x = x2;
    if (y2 > *max_y) *max_y = y2;
  }
}

void move_image_to_front(int index) {
  if (index < 0 || index >= image_count - 1) return;
  Image temp = images[index];
  memmove(&images[index], &images[index + 1], sizeof(Image) * (image_count - index - 1));
  images[image_count - 1] = temp;
}

void move_image_to_back(int index) {
  if (index <= 0 || index >= image_count) return;
  Image temp = images[index];
  memmove(&images[1], &images[0], sizeof(Image) * index);
  images[0] = temp;
}

void center_on_image(int index, int window_width, int window_height, float zoom, float *offsetX, float *offsetY) {
  if (index < 0 || index >= image_count) return;

  Image *img = &images[index];
  float center_x = img->x + img->width / 2.0f;
  float center_y = img->y + img->height / 2.0f;

  *offsetX = (window_width / 2.0f) / zoom - center_x;
  *offsetY = (window_height / 2.0f) / zoom - center_y;
}

int main(int argc, char *argv[]) {
  // Use command-line argument or default to current dir
  if (argc > 1) {
    strncpy(img_dir, argv[1], sizeof(img_dir) - 1);
    img_dir[sizeof(img_dir) - 1] = '\0';
  }

  SDL_Init(SDL_INIT_VIDEO);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

  SDL_Window *window =
      SDL_CreateWindow("alnview", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  load_images(renderer, img_dir);
  arrange_images_in_grid();
  float zoom = 1.0f;
  float offsetX = 0, offsetY = 0;
  int dragging = 0;
  int lastX = 0, lastY = 0;
  int image_dragging = -1;  // index of the image being dragged, -1 if none
  int drag_offset_x = 0, drag_offset_y = 0;
  int shift_held = 0;
  int window_width = 640;
  int window_height = 480;
  int viewport_initialized = 0;
  int current_image = 0;

  int quit = 0;
  SDL_Event e;

  while (!quit) {
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
        case SDL_QUIT:
          quit = 1;
          break;
        case SDL_WINDOWEVENT:
          if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
            int new_width = e.window.data1;
            int new_height = e.window.data2;

            // Compute center change in logical coordinates
            float dx = (new_width - window_width) / (2.0f * zoom);
            float dy = (new_height - window_height) / (2.0f * zoom);

            offsetX += dx;
            offsetY += dy;

            window_width = new_width;
            window_height = new_height;
            if (!viewport_initialized) {
              window_width = e.window.data1;
              window_height = e.window.data2;

              int min_x, min_y, max_x, max_y;
              get_image_bounds(&min_x, &min_y, &max_x, &max_y);

              int content_width = max_x - min_x;
              int content_height = max_y - min_y;

              float margin = 40.0f;
              float scale_x = (window_width - margin) / (float)content_width;
              float scale_y = (window_height - margin) / (float)content_height;
              zoom = fmin(scale_x, scale_y);

              offsetX = (window_width / 2.0f) / zoom - (min_x + content_width / 2.0f);
              offsetY = (window_height / 2.0f) / zoom - (min_y + content_height / 2.0f);

              viewport_initialized = 1;
            }
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (e.button.button == SDL_BUTTON_LEFT) {
            dragging = 1;
            lastX = e.button.x;
            lastY = e.button.y;

            float world_x = e.button.x / zoom - offsetX;
            float world_y = e.button.y / zoom - offsetY;

            for (int i = 0; i < image_count; ++i) {
              int x = images[i].x;
              int y = images[i].y;
              int w = images[i].width;
              int h = images[i].height;

              if (world_x >= x && world_x <= x + w && world_y >= y && world_y <= y + h) {
                current_image = i;
                break;
              }
            }
            dragging = 1;
            lastX = e.button.x;
            lastY = e.button.y;
          } else if (e.button.button == SDL_BUTTON_RIGHT) {
            // Convert mouse position to world coordinates
            float world_x = e.button.x / zoom - offsetX;
            float world_y = e.button.y / zoom - offsetY;

            // Check if mouse is over any image
            for (int i = image_count - 1; i >= 0; --i) {  // check from top image down
              int x = images[i].x;
              int y = images[i].y;
              int w = images[i].width;
              int h = images[i].height;
              if (world_x >= x && world_x <= x + w && world_y >= y && world_y <= y + h) {
                image_dragging = i;
                drag_offset_x = world_x - x;
                drag_offset_y = world_y - y;
                break;
              }
            }
          }
          break;
        case SDL_MOUSEBUTTONUP:
          if (e.button.button == SDL_BUTTON_LEFT) {
            dragging = 0;
          } else if (e.button.button == SDL_BUTTON_RIGHT) {
            image_dragging = -1;
          }
          break;
        case SDL_MOUSEMOTION:
          if (dragging) {
            offsetX += (e.motion.x - lastX) / zoom;
            offsetY += (e.motion.y - lastY) / zoom;
            lastX = e.motion.x;
            lastY = e.motion.y;
          }

          if (image_dragging != -1) {
            float world_x = e.motion.x / zoom - offsetX;
            float world_y = e.motion.y / zoom - offsetY;

            images[image_dragging].x = world_x - drag_offset_x;
            images[image_dragging].y = world_y - drag_offset_y;
          }
          break;
        case SDL_MOUSEWHEEL: {
          int mx, my;
          SDL_GetMouseState(&mx, &my);
          float world_x = mx / zoom - offsetX;
          float world_y = my / zoom - offsetY;

          if (shift_held) {
            // Shift is held — zoom only image under cursor
            for (int i = image_count - 1; i >= 0; --i) {
              Image *img = &images[i];
              int x = img->x;
              int y = img->y;
              int w = img->width;
              int h = img->height;

              if (world_x >= x && world_x <= x + w && world_y >= y && world_y <= y + h) {
                float factor = (e.wheel.y > 0) ? 1.1f : 1.0f / 1.1f;

                // Zoom around mouse point relative to image
                float rel_x = (world_x - x) / w;
                float rel_y = (world_y - y) / h;

                int new_w = img->width * factor;
                int new_h = img->height * factor;

                // Adjust position so the point under cursor remains stable
                img->x = world_x - rel_x * new_w;
                img->y = world_y - rel_y * new_h;

                img->width = new_w;
                img->height = new_h;
                break;
              }
            }
          } else {
            // Regular canvas zoom
            float before_x = mx / zoom - offsetX;
            float before_y = my / zoom - offsetY;

            if (e.wheel.y > 0)
              zoom *= 1.1f;
            else if (e.wheel.y < 0)
              zoom /= 1.1f;

            // Clamp if desired
            // if (zoom < 0.05f) zoom = 0.05f;
            // if (zoom > 10.0f) zoom = 10.0f;

            float after_x = mx / zoom - offsetX;
            float after_y = my / zoom - offsetY;

            offsetX += (after_x - before_x);
            offsetY += (after_y - before_y);
          }
          break;
        }
        case SDL_KEYDOWN:
          switch (e.key.keysym.sym) {
            case SDLK_PERIOD:
            case SDLK_COMMA: {
              int mx, my;
              SDL_GetMouseState(&mx, &my);
              float world_x = mx / zoom - offsetX;
              float world_y = my / zoom - offsetY;

              for (int i = image_count - 1; i >= 0; --i) {
                int x = images[i].x;
                int y = images[i].y;
                int w = images[i].width;
                int h = images[i].height;

                if (world_x >= x && world_x <= x + w && world_y >= y && world_y <= y + h) {
                  if (e.key.keysym.sym == SDLK_PERIOD)
                    move_image_to_front(i);
                  else
                    move_image_to_back(i);

                  break;
                }
              }
            } break;
            case SDLK_PAGEDOWN:
              current_image = (current_image + 1) % image_count;
              center_on_image(current_image, window_width, window_height, zoom, &offsetX, &offsetY);
              break;
            case SDLK_PAGEUP:
              current_image = (current_image - 1 + image_count) % image_count;
              center_on_image(current_image, window_width, window_height, zoom, &offsetX, &offsetY);
              break;
            case SDLK_SPACE:
              center_on_image(current_image, window_width, window_height, zoom, &offsetX, &offsetY);
              break;
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
              shift_held = 1;
              break;
          }
          break;
        case SDL_KEYUP:
          switch (e.key.keysym.sym) {
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
              shift_held = 0;
              break;
          }
          break;
      }
    }
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < image_count; ++i) {
      SDL_Rect dst;
      dst.x = (int)((images[i].x + offsetX) * zoom);
      dst.y = (int)((images[i].y + offsetY) * zoom);
      dst.w = (int)(images[i].width * zoom);
      dst.h = (int)(images[i].height * zoom);
      SDL_RenderCopy(renderer, images[i].texture, NULL, &dst);
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(16);  // ~60fps
  }

  for (int i = 0; i < image_count; ++i) SDL_DestroyTexture(images[i].texture);
  free(images);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  IMG_Quit();
  SDL_Quit();
  return 0;
}