#include "super.h"

int mouse_raw_last_x = 0;
int mouse_raw_last_y = 0;  // for mouse dragging positioning
double mouse_screen_last_x = 0;
double mouse_screen_last_y = 0;      // for mouse dragging rotation
double canvas_initial_rotation = 0;  // for canvas rotation about center
double mouse_initial_angle = 0;
double canvas_rotation_point_x = 0;
double canvas_rotation_point_y = 0;

void super_canvas_drag_zoom() {
  double screen_reference_x, screen_reference_y;
  canvas_to_screen(canvas_rotation_point_x, canvas_rotation_point_y, &screen_reference_x, &screen_reference_y);
  double distance1 = sqrt((screen_reference_x - mouse_screen_last_x) * (screen_reference_x - mouse_screen_last_x) +
                          (screen_reference_y - mouse_screen_last_y) * (screen_reference_y - mouse_screen_last_y));
  double distance2 = sqrt((screen_reference_x - mouse_screen_x) * (screen_reference_x - mouse_screen_x) +
                          (screen_reference_y - mouse_screen_y) * (screen_reference_y - mouse_screen_y));
  canvas_zoom_by_at_point(canvas_rotation_point_x, canvas_rotation_point_y, distance2 / distance1);
}

void super_image_drag_zoom() {
  double screen_reference_x, screen_reference_y;
  canvas_to_screen(images[cv.selected_imi].x + images[cv.selected_imi].rx, images[cv.selected_imi].y + images[cv.selected_imi].ry, &screen_reference_x,
                   &screen_reference_y);
  double distance1 = sqrt((screen_reference_x - mouse_screen_last_x) * (screen_reference_x - mouse_screen_last_x) +
                          (screen_reference_y - mouse_screen_last_y) * (screen_reference_y - mouse_screen_last_y));
  double distance2 = sqrt((screen_reference_x - mouse_screen_x) * (screen_reference_x - mouse_screen_x) +
                          (screen_reference_y - mouse_screen_y) * (screen_reference_y - mouse_screen_y));
  images[cv.selected_imi].z *= distance2 / distance1;
}

void super_mouse_last(SDL_Event e) {
  mouse_raw_last_x = e.button.x;
  mouse_raw_last_y = e.button.y;
  mouse_screen_last_x = mouse_screen_x;
  mouse_screen_last_y = mouse_screen_y;
}

void super_canvas_center_rotation_init() {
  mouse_initial_angle = mouse_angle_about_center;
  canvas_initial_rotation = cv.r;
}

void super_canvas_rotation_init() {
  canvas_rotation_point_x = mouse_canvas_x;
  canvas_rotation_point_y = mouse_canvas_y;
}

void super_image_rotating() {
  double screen_reference_x, screen_reference_y;
  canvas_to_screen(images[cv.selected_imi].x + images[cv.selected_imi].rx, images[cv.selected_imi].y + images[cv.selected_imi].ry, &screen_reference_x,
                   &screen_reference_y);
  double dAngle = (180 / M_PI) * (atan2(screen_reference_y - mouse_screen_last_y, screen_reference_x - mouse_screen_last_x) -
                                  atan2(screen_reference_y - mouse_screen_y, screen_reference_x - mouse_screen_x));
  image_rotate_by(cv.selected_imi, -dAngle);
}

void super_canvas_rotating_center() { cv.r = canvas_initial_rotation + (mouse_angle_about_center - mouse_initial_angle); }

void super_canvas_rotating_point() {
  double screen_reference_x, screen_reference_y;
  canvas_to_screen(canvas_rotation_point_x, canvas_rotation_point_y, &screen_reference_x, &screen_reference_y);
  double dAngle = (180 / M_PI) * (atan2(screen_reference_y - mouse_screen_last_y, screen_reference_x - mouse_screen_last_x) -
                                  atan2(screen_reference_y - mouse_screen_y, screen_reference_x - mouse_screen_x));
  canvas_rotate_about_point_by(canvas_rotation_point_x, canvas_rotation_point_y, dAngle);
}

void super_toggle_antialiasing() {
  antialiasing = !antialiasing;
  for (int i = 0; i < images_count; i++) {
    if (antialiasing) {
      if (images[i].fullres_exists) SDL_SetTextureScaleMode(images[i].texture_fullres, SDL_ScaleModeLinear);
      SDL_SetTextureScaleMode(images[i].texture_small, SDL_ScaleModeLinear);
    } else {
      if (images[i].fullres_exists) SDL_SetTextureScaleMode(images[i].texture_fullres, SDL_ScaleModeNearest);
      SDL_SetTextureScaleMode(images[i].texture_small, SDL_ScaleModeNearest);
    }
  }
}

void super_toggle_fullscreen() {
  Uint32 flags = SDL_GetWindowFlags(window);
  if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
    SDL_SetWindowFullscreen(window, 0);  // Back to windowed
  } else {
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);  // Fullscreen
  }
}

void super_opacity_increase() {
  int t = images[cv.selected_imi].opacity + 16;
  if (t > 255) {
    images[cv.selected_imi].opacity = 255;
  } else {
    images[cv.selected_imi].opacity = t;
  }
}
void super_opacity_decrease() {
  int t = images[cv.selected_imi].opacity - 16;
  if (t < 0) {
    images[cv.selected_imi].opacity = 0;
  } else {
    images[cv.selected_imi].opacity = t;
  }
}

void super_reload_single_image(int imi) {
  if (imi < 0) return;
  SDL_DestroyTexture(images[imi].texture_fullres);
  SDL_Surface *surface = IMG_Load(images[imi].filepath);
  if (surface) {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    if (texture) {
      Image *img = &images[imi];
      img->texture_fullres = texture;
      img->width = surface->w;
      img->height = surface->h;
      img->inited = 1;
    } else {
      fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
    }
    SDL_FreeSurface(surface);
  }
}