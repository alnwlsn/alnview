#include "canvas.h"
#include "controls.h"
#include "images.h"
#include "render.h"

char img_dir[FILEPATHLEN] = ".";

int main(int argc, char *argv[]) {
  render_init();
  if (argc > 1) {
    strncpy(img_dir, argv[1], sizeof(img_dir) - 1);
    img_dir[sizeof(img_dir) - 1] = '\0';
  }
  // SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ); //antialiasing

  canvas_init();  // this actually makes the window

  images_load_dir(img_dir);  // load all the images

  canvas_zoom_center_fitall();

  // images[4].r = 30; //for test

  int quit = 0;
  SDL_Event e;

  while (!quit) {
    canvas_update_cursor();
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
        case SDL_QUIT:
          quit = 1;
          break;
        case SDL_KEYDOWN:
          switch (e.key.keysym.sym) {
            case SDLK_ESCAPE:
              quit = 1;
              break;
          }
          break;
      }
      controls_process(e);  // control loop
    }

    render_canvas();
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
