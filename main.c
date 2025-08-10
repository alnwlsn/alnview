#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "canvas.h"
#include "controls.h"
#include "images.h"

TTF_Font *font = NULL;
char img_dir[FILEPATHLEN] = ".";

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
  font = TTF_OpenFont("font.ttf", 16);
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

  canvas_init();  // this actually makes the window

  images_load_dir(img_dir);  // load all the images

  int quit = 0;
  SDL_Event e;

  while (!quit) {
    canvas_update_cursor();
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
        case SDL_QUIT:
          quit = 1;
          break;
      }
      controls_process(e);  // control loop
    }

    SDL_RenderClear(renderer);
    images_render();

    // debugging text
    char coordText[128];
    snprintf(coordText, sizeof(coordText), "X: %.1f Y: %.1f R: %.1f Z: %.1f", cv.x, cv.y, cv.r, cv.z);
    render_text(renderer, coordText, 2, 0);
    snprintf(coordText, sizeof(coordText), "X: %.1f Y: %.1f A: %.1f", mouse_screen_x, mouse_screen_y, mouse_angle_about_center);
    render_text(renderer, coordText, 2, 16);
    snprintf(coordText, sizeof(coordText), "X: %.1f  Y: %.1f", mouse_canvas_x, mouse_canvas_y);
    render_text(renderer, coordText, 2, 32);

    canvas_render_pin(0, 0);
    canvas_render_pin(-100, -100);

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);  // background color
    SDL_RenderPresent(renderer);
    SDL_Delay(16);  // ~60fps
  }

  images_free();
  TTF_CloseFont(font);
  TTF_Quit();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  IMG_Quit();
  SDL_Quit();
  return 0;
}
