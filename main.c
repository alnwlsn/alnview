#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "canvas.h"
#include "controls.h"
#include "images.h"
#include "loader.h"
#include "render.h"

extern char* optarg;

int main(int argc, char *argv[]) {
  render_init();
  int opt;
  while ((opt = getopt(argc, argv, "mus:n:")) != -1) {
    switch (opt) {
      case 'm':
        init_monitor_file_changes = 1;
        break;
      case 'u':
        init_no_compress_images = 1;
        break;
      case 's':
        init_small_image_reduction = atoi(optarg);
        if(init_no_compress_images){
          init_small_image_only = true;
        }
        break;
      case 'n':
        init_max_restored_hires = atoi(optarg);
        break;
      default:
        fprintf(stderr, "Usage: %s [OPTIONS] [DIRECTORY]\n", argv[0]);
        return 1;
    }
  }

  if (argc > 1) {  // last given argument is the image folder to use; chdir to it
    if (chdir(argv[argc-1]) != 0) {
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
      }
      if (controls_process(e)) quit = 1;  // control loop
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
