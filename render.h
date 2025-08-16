#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "defines.h"

void render_init();
void render_text(char *coordText, int x, int y);
void render_canvas();
void render_text_screen(char *text);

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern TTF_Font *font;
extern char coordText[600];