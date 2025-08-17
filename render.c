#include "render.h"

#include "canvas.h"
#include "controls.h"
#include "images.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
TTF_Font *font = NULL;
char coordText[600];

char* get_resource_path(const char *file) {
    // SDL_GetBasePath gives you the directory containing the executable
    char *base = SDL_GetBasePath();
    if (!base) {
        // fallback: current working directory
        base = SDL_strdup("./");
    }

    size_t len = strlen(base) + strlen(file) + 1;
    char *full_path = (char*)malloc(len);
    if (full_path) {
        strcpy(full_path, base);
        strcat(full_path, file);
    }
    SDL_free(base);  // SDL_GetBasePath uses SDL_malloc internally
    return full_path;
}

void render_text(char *text, int x, int y) {  // for rendering text on screen
  SDL_Color textColor = {255, 255, 255, 255};
  SDL_Surface *textSurface = TTF_RenderText_Blended(font, text, textColor);
  SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
  SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
  SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
  SDL_FreeSurface(textSurface);
  SDL_DestroyTexture(textTexture);
}

void render_text_center(char *text, int x, int y) {  // for rendering text on screen
  SDL_Color textColor = {255, 255, 255, 255};
  SDL_Surface *textSurface = TTF_RenderText_Blended(font, text, textColor);
  SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
  SDL_Rect textRect = {x-(textSurface->w/2), y-(textSurface->h/2), textSurface->w, textSurface->h};
  SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
  SDL_FreeSurface(textSurface);
  SDL_DestroyTexture(textTexture);
}

void render_text_screen(char *text){
    SDL_RenderClear(renderer);
    render_text_center(text, WINDOW_INIT_X/2, WINDOW_INIT_Y/2);
    SDL_SetRenderDrawColor(renderer, BGCOLOR_R, BGCOLOR_G, BGCOLOR_B, 255);
    SDL_RenderPresent(renderer);
}

void render_init() {
  TTF_Init();
  font = TTF_OpenFont(get_resource_path("font.ttf"), 16);
  if (!font) {
    fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
    exit(1);
  }
  SDL_Init(SDL_INIT_VIDEO);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  window = SDL_CreateWindow("alnview", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_INIT_X, WINDOW_INIT_Y,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);  //| SDL_WINDOW_MAXIMIZED);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

void render_canvas() {
  SDL_RenderClear(renderer);
  images_render();

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  // debugging text
  snprintf(coordText, sizeof(coordText), "X: %.1f Y: %.1f R: %.1f Z: %.1f", cv.x, cv.y, cv.r, cv.z);
  render_text(coordText, 2, 0);
  snprintf(coordText, sizeof(coordText), "X: %.1f Y: %.1f A: %.1f", mouse_screen_x, mouse_screen_y, mouse_angle_about_center);
  render_text(coordText, 2, 16);
  snprintf(coordText, sizeof(coordText), "X: %.1f  Y: %.1f", mouse_canvas_x, mouse_canvas_y);
  render_text(coordText, 2, 32);
  double ax, ay;
  canvas_to_screen(canvas_rotation_point_x, canvas_rotation_point_y, &ax, &ay);
  snprintf(coordText, sizeof(coordText), "X: %.1f  Y: %.1f sX: %.1f  sY: %.1f", canvas_rotation_point_x, canvas_rotation_point_y, ax, ay);
  render_text(coordText, 2, 48);
  snprintf(coordText, sizeof(coordText), "TA: %d TB: %d TC: %d M: %d", global_testA, global_testB, global_testC,
           image_point_on(mouse_canvas_x, mouse_canvas_y));
  render_text(coordText, 2, 64);

  if (show_center_mark) {
    canvas_render_pin(cv.x, cv.y);
  }

  if (show_canvas_rotation_point) {
    canvas_render_pin(canvas_rotation_point_x, canvas_rotation_point_y);
  }

  // int imir = image_point_on(mouse_canvas_x, mouse_canvas_y);
  if (show_image_reference_marks) {
    // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    // canvas_render_pin(images[selected_imi].x, images[selected_imi].y); //image origin
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    canvas_render_pin(images[selected_imi].x + images[selected_imi].rx, images[selected_imi].y + images[selected_imi].ry);
    rectangleCorners s = image_find_corners(selected_imi);
    canvas_render_pin(s.aX, s.aY);
    canvas_render_pin(s.bX, s.bY);
    canvas_render_pin(s.cX, s.cY);
    canvas_render_pin(s.dX, s.dY);
  }

  // SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
  // canvas_render_pin(imrefAx, imrefAy);
  // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  // canvas_render_pin(imrefBx, imrefBy);
  // canvas_render_pin(imrefCx, imrefCy);
  // canvas_render_pin(imrefDx, imrefDy);

  SDL_SetRenderDrawColor(renderer, BGCOLOR_R, BGCOLOR_G, BGCOLOR_B, 255);  // background color
  SDL_RenderPresent(renderer);
}