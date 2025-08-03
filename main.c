// alnview - multi image viewer
// alnwlsn 2025
// also made with the help of ChatGPT
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#define M_PI 3.14159265358979323846 /* pi */

TTF_Font *font = NULL;

#define FILEPATHLEN 512
#define MAX_IMAGES 50000

// global vars that control the world view
float world_r = 0;                             // angle to rotate world (clockwise)
float world_rx = 0, world_ry = 0;              // point on canvas to rotate about
float world_zoom = 1.0f;                       // zoom on window
float window_offsetX = 0, window_offsetY = 0;  // window position on canvas

char img_dir[FILEPATHLEN] = ".";

typedef struct {
  SDL_Texture *texture;
  int width;
  int height;
  float x;
  float y;
  float r;  // rotation angle, and rotation points
  float rx;
  float ry;
  float zoom;
  int sel_order;
  int draw_order;
  int sort_index;  // when sorted, this points to images[index] of sorted list
  char filepath[FILEPATHLEN];
} Image;

Image *images = NULL;
int images_count = 0;

typedef struct {
  char name[256];
} FileEntry;

int compare_entries(const void *a, const void *b) {
  const FileEntry *fa = (const FileEntry *)a;
  const FileEntry *fb = (const FileEntry *)b;
  return strcmp(fa->name, fb->name);
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

void load_image(SDL_Renderer *renderer, char *filepath) {  // loads image at filepath, inits width and height
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
void load_images_dir(SDL_Renderer *renderer, const char *directory) {  // load all images from directory
  DIR *dir = opendir(directory);
  if (!dir) {
    perror("Failed to open image directory");
    exit(1);
  }
  struct dirent *entry;
  int file_count = 0;
  // collect image files in dir
  while ((entry = readdir(dir))) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
    char path[512];
    snprintf(path, sizeof(path) - 8, "%s/%s", directory, entry->d_name);
    struct stat path_stat;
    if (stat(path, &path_stat) != 0 || !S_ISREG(path_stat.st_mode)) continue;
    load_image(renderer, path);
  }
  closedir(dir);
}

void arrange_images_in_grid() {  // arranges all images into grid by selection order
  int columns = ceil(sqrt(images_count));
  int spacing = 20;

  int x = 0, y = 0;
  int max_row_height = 0;
  sort_images_by(compare_sel_order);
  for (int i = 0; i < images_count; ++i) {
    int si = images[i].sort_index;
    images[si].x = x;
    images[si].y = y;
    images[si].r = 0;
    images[si].rx = 0;
    images[si].ry = 0;
    images[si].zoom = 1;

    x += images[si].width + spacing;
    if (images[si].height > max_row_height) max_row_height = images[si].height;

    if ((i + 1) % columns == 0) {
      x = 0;
      y += max_row_height + spacing;
      max_row_height = 0;
    }
  }
}

void render_text(SDL_Renderer *renderer, char *coordText, int x, int y) {
  SDL_Color textColor = {255, 255, 255, 255};
  SDL_Surface *textSurface = TTF_RenderText_Blended(font, coordText, textColor);
  SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
  SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
  SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
  SDL_FreeSurface(textSurface);
  SDL_DestroyTexture(textTexture);
}

void screen_to_canvas(float *x_out, float *y_out, float x_in, float y_in) {  // converts from mouse (screen) to canvas coords
  float lX = (x_in / world_zoom) - window_offsetX - world_rx;
  float lY = (y_in / world_zoom) - window_offsetY - world_ry;
  *x_out = lX * cos(-world_r * M_PI / 180) - lY * sin(-world_r * M_PI / 180);
  *y_out = lY * cos(-world_r * M_PI / 180) + lX * sin(-world_r * M_PI / 180);
  *x_out += world_rx;  // mouse_rX and mouse_rY is canvas coords
  *y_out += world_ry;
}
void canvas_to_screen(float *x_out, float *y_out, float x_in, float y_in) {  // converts from canvas to screen coords
  float lX = (x_in - world_rx);                                              // remove rotation point offset
  float lY = (y_in - world_ry);
  *x_out = lX * cos(world_r * M_PI / 180) - lY * sin(world_r * M_PI / 180);
  *y_out = lY * cos(world_r * M_PI / 180) + lX * sin(world_r * M_PI / 180);
  *x_out += window_offsetX + world_rx;  // add rotation offset back
  *y_out += window_offsetY + world_ry;
  *x_out *= world_zoom;  // scale the whole thing
  *y_out *= world_zoom;
}

int main(int argc, char *argv[]) {
  TTF_Init();
  font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16);
  if (!font) {
    fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
    exit(1);
  }
  if (argc > 1) {
    strncpy(img_dir, argv[1], sizeof(img_dir) - 1);
    img_dir[sizeof(img_dir) - 1] = '\0';
  }
  SDL_Init(SDL_INIT_VIDEO);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  // SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ); //antialiasing
  SDL_Window *window = SDL_CreateWindow("alnview", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                                        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);  //| SDL_WINDOW_MAXIMIZED);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  // later, do this if not saved
  load_images_dir(renderer, img_dir);
  // assign initial draw order to files in alphabetical order
  // also, initialize offsets
  sort_images_by(compare_filepath);
  for (int i = 0; i < images_count; i++) {
    images[i].x = 0;
    images[i].y = 0;
    images[i].r = 0;
    images[i].rx = 0;
    images[i].ry = 0;
    images[i].zoom = 1;
    images[images[i].sort_index].draw_order = i;
    images[images[i].sort_index].sel_order = i;
  }
  arrange_images_in_grid();
  int window_width = 800, window_height = 600;
  int drag_mouse_x = 0, drag_mouse_y = 0;
  int dragging = 0;
  int quit = 0;
  SDL_Event e;

  while (!quit) {
    // calculate mouse coordinates first
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    float mouse_rY = 0, mouse_rX = 0;
    screen_to_canvas(&mouse_rX, &mouse_rY, mouseX, mouseY);

    while (SDL_PollEvent(&e)) {
      switch (e.type) {  // input loop
        case SDL_QUIT:
          quit = 1;
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (e.button.button == SDL_BUTTON_LEFT) {
            dragging = 1;
            drag_mouse_x = e.button.x;
            drag_mouse_y = e.button.y;
          }
          break;
        case SDL_MOUSEBUTTONUP:
          if (e.button.button == SDL_BUTTON_LEFT) {
            dragging = 0;
          }
          break;
        case SDL_MOUSEMOTION:
          if (dragging) {
            window_offsetX += (e.motion.x - drag_mouse_x) / world_zoom;
            window_offsetY += (e.motion.y - drag_mouse_y) / world_zoom;
            drag_mouse_x = e.motion.x;
            drag_mouse_y = e.motion.y;
          }
          break;
        case SDL_MOUSEWHEEL: {
          float before_x = (mouseX / world_zoom) - window_offsetX;
          float before_y = (mouseY / world_zoom) - window_offsetY;
          if (e.wheel.y > 0)
            world_zoom *= 1.1f;
          else if (e.wheel.y < 0)
            world_zoom /= 1.1f;
          // Clamp if desired
          // if (zoom < 0.05f) zoom = 0.05f;
          // if (zoom > 10.0f) zoom = 10.0f;
          float after_x = (mouseX / world_zoom) - window_offsetX;
          float after_y = (mouseY / world_zoom) - window_offsetY;
          window_offsetX += (after_x - before_x);
          window_offsetY += (after_y - before_y);

        } break;
        case SDL_KEYDOWN:
          switch (e.key.keysym.sym) {
            case SDLK_BACKSLASH:
              // reset zoom
              float before_x = (mouseX / world_zoom) - window_offsetX;
              float before_y = (mouseY / world_zoom) - window_offsetY;
              world_zoom = 1;
              // Clamp if desired
              // if (zoom < 0.05f) zoom = 0.05f;
              // if (zoom > 10.0f) zoom = 10.0f;
              float after_x = (mouseX / world_zoom) - window_offsetX;
              float after_y = (mouseY / world_zoom) - window_offsetY;
              window_offsetX += (after_x - before_x);
              window_offsetY += (after_y - before_y);
              break;
            case SDLK_q:

              break;
            case SDLK_w:
              world_r = 0.0f;
              world_rx = 0;
              world_ry = 0;
              world_zoom = 1.0;
              world_rx = 0;
              world_ry = 0;
              break;
            case SDLK_r: //pick new point to rotate world around
              world_rx = mouse_rX;
              world_ry = mouse_rY;
              break;
            case SDLK_e:
              world_r += 15.0f;
              world_rx = mouse_rX;
              world_ry = mouse_rY;

              // mouse will move to here
              float mouse2_lX = (mouseX / world_zoom) - window_offsetX - world_rx;
              float mouse2_lY = (mouseY / world_zoom) - window_offsetY - world_ry;
              float mouse2_rX = mouse2_lX * cos(-world_r * M_PI / 180) - mouse2_lY * sin(-world_r * M_PI / 180);
              float mouse2_rY = mouse2_lY * cos(-world_r * M_PI / 180) + mouse2_lX * sin(-world_r * M_PI / 180);
              mouse2_rX += world_rx;
              mouse2_rY += world_ry;
              float diff_lX = (mouse2_rX - world_rx);  // move on canvas
              float diff_lY = (mouse2_rY - world_ry);
              float diff_rX = diff_lX * cos(world_r * M_PI / 180) - diff_lY * sin(world_r * M_PI / 180);  // derotate
              float diff_rY = diff_lY * cos(world_r * M_PI / 180) + diff_lX * sin(world_r * M_PI / 180);
              window_offsetX += diff_rX;
              window_offsetY += diff_rY;

              // mouse_lX = (mouseX / world_zoom) - window_offsetX - world_rx;
              // mouse_lY = (mouseY / world_zoom) - window_offsetY - world_ry;
              // mouse_rX = mouse_lX * cos(-world_r * M_PI / 180) - mouse_lY * sin(-world_r * M_PI / 180);
              // mouse_rY = mouse_lY * cos(-world_r * M_PI / 180) + mouse_lX * sin(-world_r * M_PI / 180);
              // mouse_rX += world_rx;  // mouse_rX and mouse_rY is canvas coords
              // mouse_rY += world_ry;

              // printf("x3: %.1f, y3: %.1f\n",mouse_rX,mouse_rY);

              break;
            case SDLK_i:
              window_offsetY -= 1;
              break;
            case SDLK_k:
              window_offsetY += 1;
              break;
            case SDLK_j:
              window_offsetX -= 1;
              break;
            case SDLK_l:
              window_offsetX += 1;
              break;
          }
          break;
      }
    }
    sort_images_by(compare_draw_order);
    SDL_RenderClear(renderer);
    for (int i = 0; i < images_count; ++i) { //render all images onto the canvas
      SDL_Rect dst;
      int si = images[i].sort_index;  // sorted index

      float rY = 0, rX = 0;
      canvas_to_screen(&rX, &rY, images[si].x, images[si].y);

      dst.x = (int)rX;
      dst.y = (int)rY;
      dst.w = (int)(images[si].width * world_zoom);
      dst.h = (int)(images[si].height * world_zoom);
      SDL_Point rp = {0, 0};
      SDL_RenderCopyEx(renderer, images[si].texture, NULL, &dst, world_r, &rp, SDL_FLIP_NONE);
      //  SDL_RenderCopyEx(renderer, images[si].texture, NULL, &dst, world_r, NULL, SDL_FLIP_NONE);
    }

    //debugging text
    char coordText[128];
    snprintf(coordText, sizeof(coordText), "X: %d  Y: %d", mouseX, mouseY);
    render_text(renderer, coordText, 2, 0);
    snprintf(coordText, sizeof(coordText), "X: %.1f Y: %.1f", mouse_rX, mouse_rY);
    render_text(renderer, coordText, 2, 16);
    float mY = 0, mX = 0;
    canvas_to_screen(&mX, &mY, mouse_rX, mouse_rY);
    snprintf(coordText, sizeof(coordText), "X: %.1f  Y: %.1f", mX, mY);
    render_text(renderer, coordText, 2, 32);

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);  // background color

    SDL_RenderPresent(renderer);
    SDL_Delay(16);  // ~60fps
  }

  for (int i = 0; i < images_count; ++i) SDL_DestroyTexture(images[i].texture);
  free(images);
  TTF_CloseFont(font);
  TTF_Quit();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  IMG_Quit();
  SDL_Quit();
  return 0;
}
