#include "super.h"

double canvas_rotation_point_x = 0;
double canvas_rotation_point_y = 0;
int mouse_raw_last_x = 0;
int mouse_raw_last_y = 0;  // for mouse dragging positioning
double mouse_screen_last_x = 0;
double mouse_screen_last_y = 0;      // for mouse dragging rotation
double canvas_initial_rotation = 0;  // for canvas rotation about center
double mouse_initial_angle = 0;

bool antialiasing = 0;

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
  canvas_to_screen(images[selected_imi].x + images[selected_imi].rx, images[selected_imi].y + images[selected_imi].ry, &screen_reference_x,
                   &screen_reference_y);
  double distance1 = sqrt((screen_reference_x - mouse_screen_last_x) * (screen_reference_x - mouse_screen_last_x) +
                          (screen_reference_y - mouse_screen_last_y) * (screen_reference_y - mouse_screen_last_y));
  double distance2 = sqrt((screen_reference_x - mouse_screen_x) * (screen_reference_x - mouse_screen_x) +
                          (screen_reference_y - mouse_screen_y) * (screen_reference_y - mouse_screen_y));
  images[selected_imi].z *= distance2 / distance1;
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
  canvas_to_screen(images[selected_imi].x + images[selected_imi].rx, images[selected_imi].y + images[selected_imi].ry, &screen_reference_x,
                   &screen_reference_y);
  double dAngle = (180 / M_PI) * (atan2(screen_reference_y - mouse_screen_last_y, screen_reference_x - mouse_screen_last_x) -
                                  atan2(screen_reference_y - mouse_screen_y, screen_reference_x - mouse_screen_x));
  image_rotate_by(selected_imi, -dAngle);
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
      SDL_SetTextureScaleMode(images[i].texture, SDL_ScaleModeLinear);
    } else {
      SDL_SetTextureScaleMode(images[i].texture, SDL_ScaleModeNearest);
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