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
float canvas_rx = 128, canvas_ry = -128;  // point on canvas to rotate about
typedef struct {
  float r;  // angle to rotate world (clockwise)
  float x;  // center of window in canvas coords
  float y;
  float z;  // zoom on window
} CanvasView;
CanvasView cv;
CanvasView waypt[2];

float ax=0, ay=0, bx=0, by=0;

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
    images[si].y = -y;
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
void render_text(SDL_Renderer *renderer, char *coordText, int x, int y) {  // for rendering text on screen
  SDL_Color textColor = {255, 255, 255, 255};
  SDL_Surface *textSurface = TTF_RenderText_Blended(font, coordText, textColor);
  SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
  SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
  SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
  SDL_FreeSurface(textSurface);
  SDL_DestroyTexture(textTexture);
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
  cv.x = 0;
  cv.y = 0;
  cv.r = 0;
  cv.z = 1;

  int drag_mouse_x = 0, drag_mouse_y = 0;
  int dragging = 0;
  int quit = 0;
  int animation = 0;
  int animation_step = 0;
  SDL_Event e;

  while (!quit) {
    // calculate mouse coordinates first
    int mouseX_raw, mouseY_raw;
    SDL_GetMouseState(&mouseX_raw, &mouseY_raw);
    int screenX, screenY;
    SDL_GetWindowSize(window, &screenX, &screenY);

    float mouse_screen_x = mouseX_raw - (screenX / 2.0f);  // mouse screen coords, with 0 at center on screen
    float mouse_screen_y = -mouseY_raw + (screenY / 2.0f);

    float mouse_canvas_x =
        (mouse_screen_x / cv.z) * cos(-cv.r * M_PI / 180) - (mouse_screen_y / cv.z) * sin(-cv.r * M_PI / 180);  // convert mouse to canvas coords
    float mouse_canvas_y = (mouse_screen_y / cv.z) * cos(-cv.r * M_PI / 180) + (mouse_screen_x / cv.z) * sin(-cv.r * M_PI / 180);
    mouse_canvas_x += cv.x;
    mouse_canvas_y += cv.y;

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
            // drag canvas around on screen
            float sdx = (e.motion.x - drag_mouse_x) / cv.z;
            float sdy = -(e.motion.y - drag_mouse_y) / cv.z;
            float cdx = sdx * cos(-cv.r * M_PI / 180) - sdy * sin(-cv.r * M_PI / 180);  // proper coordinate rotation
            float cdy = sdy * cos(-cv.r * M_PI / 180) + sdx * sin(-cv.r * M_PI / 180);
            cv.x -= cdx;
            cv.y -= cdy;
            drag_mouse_x = e.motion.x;
            drag_mouse_y = e.motion.y;
          }
          break;
        case SDL_MOUSEWHEEL: {
          // zoom adjustment centered on mouse
          // find old and new position; offset view accordingly
          float scx1 = (mouse_screen_x / cv.z);
          float scy1 = (mouse_screen_y / cv.z);
          if (e.wheel.y > 0)
            cv.z *= 1.1f;
          else if (e.wheel.y < 0)
            cv.z /= 1.1f;
          float scx2 = (mouse_screen_x / cv.z);
          float scy2 = (mouse_screen_y / cv.z);
          float sdx = scx2 - scx1;
          float sdy = scy2 - scy1;
          float cdx = sdx * cos(-cv.r * M_PI / 180) - sdy * sin(-cv.r * M_PI / 180);  // proper coordinate rotation
          float cdy = sdy * cos(-cv.r * M_PI / 180) + sdx * sin(-cv.r * M_PI / 180);
          cv.x -= cdx;
          cv.y -= cdy;
        } break;
        case SDL_KEYDOWN:
          switch (e.key.keysym.sym) {
            case SDLK_BACKSLASH:
              animation = 1;
              animation_step = 0;
              break;
            case SDLK_5:
              cv.r = 0;
              cv.z = 1.0f;
              cv.x = 128;
              cv.y = -128;
              break;
            case SDLK_6:
              cv.r = 22.5;
              cv.z = 1.0f;
              cv.x = 128;
              cv.y = -128;
              break;
            case SDLK_7:
              cv.r = 45;
              cv.z = 1.0f;
              cv.x = 128;
              cv.y = -128;
              break;
            case SDLK_8:
              cv.r = -45;
              cv.z = 1.0f;
              cv.x = 0;
              cv.y = 0;
              break;
            case SDLK_9:
              waypt[0] = cv;
              break;
            case SDLK_0:
              waypt[1] = cv;
              break;
            case SDLK_o:
              cv = waypt[0];
              break;
            case SDLK_p:
              cv = waypt[1];
              break;
            case SDLK_1:
              float dr = 15.0f;

              cv.r -= dr;
              float sdx = canvas_rx-cv.x;
              float sdy = canvas_ry-cv.y;
              float cx = sdx * cos(dr * M_PI / 180) - sdy * sin(dr * M_PI / 180) + cv.x; 
              float cy = sdy * cos(dr * M_PI / 180) + sdx * sin(dr * M_PI / 180) + cv.y;
              // float cdx = -canvas_rx+cv.x;
              // float cdy = -canvas_ry+cv.y;
              ax = cx;
              ay = cy;
              bx = cv.x-(cx-canvas_rx);
              by = cv.y-(cy-canvas_ry);
              printf("%.1f, %.1f\n", ax, ay);
              
              cv.x = bx;
              cv.y = by;
              break;
            case SDLK_2:
              cv.r += 15.0f;
              break;
            case SDLK_q:
              break;
            case SDLK_w:
              cv.r = 0.0f;
              cv.x = 0;
              cv.y = 0;
              cv.z = 1.0;
              break;
            case SDLK_r:  // pick new point to rotate world around
            {
              canvas_rx = mouse_canvas_x;
              canvas_ry = mouse_canvas_y;
            } break;
            case SDLK_e:
              break;
            case SDLK_i:
              cv.y += 1;
              break;
            case SDLK_k:
              cv.y -= 1;
              break;
            case SDLK_j:
              cv.x -= 1;
              break;
            case SDLK_l:
              cv.x += 1;
              break;
          }
          break;
      }
    }

    if (animation == 1) {  // animation test
      if (animation_step >= 50) {
        animation = 0;
      }
      cv.r = waypt[0].r + (animation_step / 50.0) * (waypt[1].r - waypt[0].r);
      cv.z = waypt[0].z + (animation_step / 50.0) * (waypt[1].z - waypt[0].z);
      cv.x = waypt[0].x + (animation_step / 50.0) * (waypt[1].x - waypt[0].x);
      cv.y = waypt[0].y + (animation_step / 50.0) * (waypt[1].y - waypt[0].y);
      // printf("X: %.1f Y: %.1f R: %.1f Z: %.1f\n", cv.x, cv.y, cv.r, cv.z);
      animation_step += 1;
    }

    sort_images_by(compare_draw_order);
    SDL_RenderClear(renderer);
    for (int i = 0; i < images_count; ++i) {  // render all images onto the canvas
      SDL_Rect dst;
      int si = images[i].sort_index;  // sorted index

      float lX = images[si].x * cv.z;
      float lY = -images[si].y * cv.z;
      float tX = lX * cos(-cv.r * M_PI / 180) - lY * sin(-cv.r * M_PI / 180);  // rotate image into place
      float tY = lY * cos(-cv.r * M_PI / 180) + lX * sin(-cv.r * M_PI / 180);
      tX += screenX / 2.0f;
      tY += screenY / 2.0f;
      tX -= cv.x * cv.z * cos(-cv.r * M_PI / 180) + cv.y * cv.z * sin(-cv.r * M_PI / 180);  // add in the rotated centroid point
      tY -= -cv.y * cv.z * cos(-cv.r * M_PI / 180) + cv.x * cv.z * sin(-cv.r * M_PI / 180);

      dst.x = (int)tX;
      dst.y = (int)tY;
      dst.w = (int)(images[si].width * cv.z);
      dst.h = (int)(images[si].height * cv.z);
      SDL_Point rp = {0, 0};
      SDL_RenderCopyEx(renderer, images[si].texture, NULL, &dst, -cv.r, &rp, SDL_FLIP_NONE);
      //  SDL_RenderCopyEx(renderer, images[si].texture, NULL, &dst, world_r, NULL, SDL_FLIP_NONE);
    }

    // debugging text
    char coordText[128];
    snprintf(coordText, sizeof(coordText), "X: %.1f Y: %.1f R: %.1f Z: %.1f", cv.x, cv.y, cv.r, cv.z);
    render_text(renderer, coordText, 2, 0);
    snprintf(coordText, sizeof(coordText), "X: %.1fY: %.1f", mouse_screen_x, mouse_screen_y);
    render_text(renderer, coordText, 2, 16);
    snprintf(coordText, sizeof(coordText), "X: %.1f  Y: %.1f", mouse_canvas_x, mouse_canvas_y);
    render_text(renderer, coordText, 2, 32);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    {  // draw small dot on canvas for testing mouse
      float lX = canvas_rx * cv.z;
      float lY = -canvas_ry * cv.z;
      float tX = lX * cos(-cv.r * M_PI / 180) - lY * sin(-cv.r * M_PI / 180);
      float tY = lY * cos(-cv.r * M_PI / 180) + lX * sin(-cv.r * M_PI / 180);
      tX += screenX / 2.0f;
      tY += screenY / 2.0f;
      tX -= cv.x * cv.z * cos(-cv.r * M_PI / 180) + cv.y * cv.z * sin(-cv.r * M_PI / 180);
      tY -= -cv.y * cv.z * cos(-cv.r * M_PI / 180) + cv.x * cv.z * sin(-cv.r * M_PI / 180);
      SDL_Rect dot = {(int)(tX), (int)(tY), (int)fmax(1, 4),  // scale dot size
                      (int)fmax(1, 4)};
      SDL_RenderFillRect(renderer, &dot);
    }
    {  // draw small dot on canvas for testing mouse
      float lX = ax * cv.z;
      float lY = -ay * cv.z;
      float tX = lX * cos(-cv.r * M_PI / 180) - lY * sin(-cv.r * M_PI / 180);
      float tY = lY * cos(-cv.r * M_PI / 180) + lX * sin(-cv.r * M_PI / 180);
      tX += screenX / 2.0f;
      tY += screenY / 2.0f;
      tX -= cv.x * cv.z * cos(-cv.r * M_PI / 180) + cv.y * cv.z * sin(-cv.r * M_PI / 180);
      tY -= -cv.y * cv.z * cos(-cv.r * M_PI / 180) + cv.x * cv.z * sin(-cv.r * M_PI / 180);
      SDL_Rect dot = {(int)(tX), (int)(tY), (int)fmax(1, 4),  // scale dot size
                      (int)fmax(1, 4)};
      SDL_RenderFillRect(renderer, &dot);
    }
    {  // draw small dot on canvas for testing mouse
      float lX = bx * cv.z;
      float lY = -by * cv.z;
      float tX = lX * cos(-cv.r * M_PI / 180) - lY * sin(-cv.r * M_PI / 180);
      float tY = lY * cos(-cv.r * M_PI / 180) + lX * sin(-cv.r * M_PI / 180);
      tX += screenX / 2.0f;
      tY += screenY / 2.0f;
      tX -= cv.x * cv.z * cos(-cv.r * M_PI / 180) + cv.y * cv.z * sin(-cv.r * M_PI / 180);
      tY -= -cv.y * cv.z * cos(-cv.r * M_PI / 180) + cv.x * cv.z * sin(-cv.r * M_PI / 180);
      SDL_Rect dot = {(int)(tX), (int)(tY), (int)fmax(1, 4),  // scale dot size
                      (int)fmax(1, 4)};
      SDL_RenderFillRect(renderer, &dot);
    }

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
