#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "canvas.h"
#include "controls.h"
#include "images.h"
#include "loader.h"
#include "render.h"

int main(int argc, char *argv[]) {
  render_init();
  if (argc > 1) {  // given argument is the image folder to use; chdir to it
    if (chdir(argv[1]) != 0) {
      perror("chdir failed");
      // return 1;
      // fail here means current working directory will be used instead
    }
  }

  canvas_init();  // this actually makes the window

  loader_uni(1);

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
