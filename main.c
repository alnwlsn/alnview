#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

TTF_Font *font = NULL;

char img_dir[512] = ".";

typedef struct {
  SDL_Texture *texture;
  int width;
  int height;
  int x;
  int y;
  float r;
  int sel_index;
  int draw_layer;
  char filename[512];
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

int main(int argc, char *argv[]) {
  TTF_Init();
  font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16);
  if (!font) {
    fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
    exit(1);
  }

  // Use command-line argument or default to current dir
  if (argc > 1) {
    strncpy(img_dir, argv[1], sizeof(img_dir) - 1);
    img_dir[sizeof(img_dir) - 1] = '\0';
  }

  SDL_Init(SDL_INIT_VIDEO);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

  SDL_Window *window = SDL_CreateWindow("alnview", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                                        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);  //| SDL_WINDOW_MAXIMIZED);

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  load_images(renderer, img_dir);
  //   arrange_images_in_grid();
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
      }
    }
    SDL_RenderClear(renderer);
    for (int i = 0; i < image_count; ++i) {
      SDL_Rect dst;
      dst.x = (int)((images[i].x + offsetX) * zoom);
      dst.y = (int)((images[i].y + offsetY) * zoom);
      dst.w = (int)(images[i].width * zoom);
      dst.h = (int)(images[i].height * zoom);
      SDL_RenderCopy(renderer, images[i].texture, NULL, &dst);
    }

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    // Convert to world coordinates
    float worldX = mouseX / zoom - offsetX;
    float worldY = mouseY / zoom - offsetY;

    char coordText[64];
    snprintf(coordText, sizeof(coordText), "X: %.1f  Y: %.1f", worldX, worldY);

    // Render text
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, coordText, textColor);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_Rect textRect = {2, 2, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    SDL_RenderPresent(renderer);
    SDL_Delay(16);  // ~60fps
  }

  for (int i = 0; i < image_count; ++i) SDL_DestroyTexture(images[i].texture);
  free(images);
  TTF_CloseFont(font);
  TTF_Quit();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  IMG_Quit();
  SDL_Quit();
  return 0;
}
