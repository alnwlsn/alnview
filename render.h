#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "defines.h"

void render_init();
void render_text(SDL_Renderer *renderer, char *coordText, int x, int y);
void render_canvas();

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern TTF_Font *font;