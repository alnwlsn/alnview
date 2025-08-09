#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "images.h"
#include "canvas.h"
// #include <math.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

TTF_Font *font = NULL;
char img_dir[FILEPATHLEN] = ".";

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
  SDL_Window *window = SDL_CreateWindow("alnview", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                                        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);  //| SDL_WINDOW_MAXIMIZED);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  images_init(window,renderer);

  images_load_dir(img_dir); //load all the images
  canvas_init();

  

  int quit = 0;
  SDL_Event e;

  while (!quit) {
    while (SDL_PollEvent(&e)) {
      switch (e.type) {  // input loop
        case SDL_QUIT:
          quit = 1;
          break;
      }
    }

    SDL_RenderClear(renderer);

    images_render(canvas_current());

    // debugging text
    // char coordText[128];
    // // snprintf(coordText, sizeof(coordText), "X: %.1f Y: %.1f R: %.1f Z: %.1f", cv.x, cv.y, cv.r, cv.z);
    // render_text(renderer, coordText, 2, 0);
    // // snprintf(coordText, sizeof(coordText), "X: %.1fY: %.1f", mouse_screen_x, mouse_screen_y);
    // snprintf(coordText, sizeof(coordText), "X: %.1fY: %.1f", 4, 4);
    // render_text(renderer, coordText, 2, 16);
    // // snprintf(coordText, sizeof(coordText), "X: %.1f  Y: %.1f", mouse_canvas_x, mouse_canvas_y);
    // render_text(renderer, coordText, 2, 32);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // {  // draw small dot on canvas for testing mouse
    //   float lX = canvas_rx * cv.z;
    //   float lY = -canvas_ry * cv.z;
    //   float tX = lX * cos(-cv.r * M_PI / 180) - lY * sin(-cv.r * M_PI / 180);
    //   float tY = lY * cos(-cv.r * M_PI / 180) + lX * sin(-cv.r * M_PI / 180);
    //   tX += screenX / 2.0f;
    //   tY += screenY / 2.0f;
    //   tX -= cv.x * cv.z * cos(-cv.r * M_PI / 180) + cv.y * cv.z * sin(-cv.r * M_PI / 180);
    //   tY -= -cv.y * cv.z * cos(-cv.r * M_PI / 180) + cv.x * cv.z * sin(-cv.r * M_PI / 180);
      SDL_Rect dot = {(int)(100), (int)(100), (int)fmax(1, 4),  // scale dot size
                      (int)fmax(1, 4)};
      SDL_RenderFillRect(renderer, &dot);
    // }
    // {  // draw small dot on canvas for testing mouse
    //   float lX = ax * cv.z;
    //   float lY = -ay * cv.z;
    //   float tX = lX * cos(-cv.r * M_PI / 180) - lY * sin(-cv.r * M_PI / 180);
    //   float tY = lY * cos(-cv.r * M_PI / 180) + lX * sin(-cv.r * M_PI / 180);
    //   tX += screenX / 2.0f;
    //   tY += screenY / 2.0f;
    //   tX -= cv.x * cv.z * cos(-cv.r * M_PI / 180) + cv.y * cv.z * sin(-cv.r * M_PI / 180);
    //   tY -= -cv.y * cv.z * cos(-cv.r * M_PI / 180) + cv.x * cv.z * sin(-cv.r * M_PI / 180);
    //   SDL_Rect dot = {(int)(tX), (int)(tY), (int)fmax(1, 4),  // scale dot size
    //                   (int)fmax(1, 4)};
    //   SDL_RenderFillRect(renderer, &dot);
    // }
    // {  // draw small dot on canvas for testing mouse
    //   float lX = bx * cv.z;
    //   float lY = -by * cv.z;
    //   float tX = lX * cos(-cv.r * M_PI / 180) - lY * sin(-cv.r * M_PI / 180);
    //   float tY = lY * cos(-cv.r * M_PI / 180) + lX * sin(-cv.r * M_PI / 180);
    //   tX += screenX / 2.0f;
    //   tY += screenY / 2.0f;
    //   tX -= cv.x * cv.z * cos(-cv.r * M_PI / 180) + cv.y * cv.z * sin(-cv.r * M_PI / 180);
    //   tY -= -cv.y * cv.z * cos(-cv.r * M_PI / 180) + cv.x * cv.z * sin(-cv.r * M_PI / 180);
    //   SDL_Rect dot = {(int)(tX), (int)(tY), (int)fmax(1, 4),  // scale dot size
    //                   (int)fmax(1, 4)};
    //   SDL_RenderFillRect(renderer, &dot);
    // }

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
