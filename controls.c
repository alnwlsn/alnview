#include "controls.h"

#include <stdbool.h>

// modes
bool canvas_rotating_center = 0;  // for rotation of canvas about center of screen
bool canvas_rotating_point = 0;
bool mouse_dragging = 0;  // for panning
bool image_dragging = 0;
bool image_rotating = 0;
bool image_drag_zoom = 0;
bool canvas_drag_zoom = 0;

// states
int shift_held = 0;  // shift key held
int ctrl_held = 0;   // control key held
int tab_held = 0;    // tab key held
int dragged_imi = -1;

// move references
int mouse_raw_last_x = 0, mouse_raw_last_y = 0;           // for mouse dragging positioning
double mouse_screen_last_x = 0, mouse_screen_last_y = 0;  // for mouse dragging rotation
double canvas_initial_rotation = 0;
double mouse_initial_angle = 0;

int animation = 0;
int animation_step = 0;

// globals
int selected_imi = 0;
double canvas_rotation_point_x = 0;
double canvas_rotation_point_y = 0;

// render options
bool show_center_mark = 0;            // for rendering, show a center mark if true
bool show_canvas_rotation_point = 0;  // for renderer to show another reference mark
bool show_image_reference_marks = 0;  // show corners, base and rotation point of selected image
int global_testA = 0;
int global_testB = 0;
int global_testC = 0;

int mouseover_or_selected_imi() {  // use either the mouseovered imi or the last selected one
  int imi = image_point_on(mouse_canvas_x, mouse_canvas_y);
  if (imi > -1) {
    return imi;
  }
  return selected_imi;
}
int mouseover_selects_imi() {  // same as or, but selects the mouseovered image if there is one
  int imi = image_point_on(mouse_canvas_x, mouse_canvas_y);
  if (imi > -1) {
    selected_imi = imi;
  }
  return selected_imi;
}

void controls_process(SDL_Event e) {
  switch (e.type) {
    case SDL_MOUSEBUTTONDOWN:
      if (e.button.button == SDL_BUTTON_LEFT) {
        mouse_dragging = 1;
        mouse_raw_last_x = e.button.x;
        mouse_raw_last_y = e.button.y;
        mouse_screen_last_x = mouse_screen_x;
        mouse_screen_last_y = mouse_screen_y;
        if (tab_held) {
          canvas_rotation_point_x = mouse_canvas_x;
          canvas_rotation_point_y = mouse_canvas_y;
        }
        if (shift_held) {
          image_rotation_point_set_new(selected_imi, mouse_canvas_x, mouse_canvas_y);
        }
      } else if (e.button.button == SDL_BUTTON_MIDDLE) {
        if (tab_held) {  // rotate canvas about reference
          canvas_rotating_point = 1;
          mouse_screen_last_x = mouse_screen_x;
          mouse_screen_last_y = mouse_screen_y;
        } else if (ctrl_held) {  // rotate canvas about center
          mouse_initial_angle = mouse_angle_about_center;
          canvas_initial_rotation = cv.r;
          canvas_rotating_center = 1;
        } else {  // shift or no shift; rotate image
          image_rotating = 1;
          mouse_screen_last_x = mouse_screen_x;
          mouse_screen_last_y = mouse_screen_y;
          show_image_reference_marks = 1;
        }
      } else if (e.button.button == SDL_BUTTON_RIGHT) {
        if (shift_held) {
          image_drag_zoom = 1;
          mouse_screen_last_x = mouse_screen_x;
          mouse_screen_last_y = mouse_screen_y;
        } else if (tab_held) {
          canvas_drag_zoom = 1;
          mouse_screen_last_x = mouse_screen_x;
          mouse_screen_last_y = mouse_screen_y;
        } else if (ctrl_held) {
          int imi = image_point_on(mouse_canvas_x, mouse_canvas_y);
          if (imi > -1) {
            image_series_set(imi);
            selected_imi = imi;
          }
        } else {
          int imi = image_point_on(mouse_canvas_x, mouse_canvas_y);
          if (imi > -1) {
            image_series_set(imi);
            selected_imi = imi;
            image_dragging = 1;
            mouse_raw_last_x = e.button.x;
            mouse_raw_last_y = e.button.y;
          }
        }
      }
      break;
    case SDL_MOUSEBUTTONUP:
      if (e.button.button == SDL_BUTTON_LEFT) {
        mouse_dragging = 0;
      } else if (e.button.button == SDL_BUTTON_MIDDLE) {
        canvas_rotating_center = 0;
        show_canvas_rotation_point = 0;
        canvas_rotating_point = 0;
        image_rotating = 0;
        show_image_reference_marks = 0;
      } else if (e.button.button == SDL_BUTTON_RIGHT) {
        image_dragging = 0;
        image_drag_zoom = 0;
        canvas_drag_zoom = 0;
      }
      break;
    case SDL_MOUSEMOTION:
      if (mouse_dragging) {
        canvas_drag_screen_by(e.motion.x - mouse_raw_last_x, mouse_raw_last_y - e.motion.y);
      }
      if (image_dragging) {
        image_drag_screen_by(selected_imi, e.motion.x - mouse_raw_last_x, mouse_raw_last_y - e.motion.y);
      }
      if (image_rotating) {
        double screen_reference_x, screen_reference_y;
        canvas_to_screen(images[selected_imi].x + images[selected_imi].rx, images[selected_imi].y + images[selected_imi].ry, &screen_reference_x,
                         &screen_reference_y);
        double dAngle = (180 / M_PI) * (atan2(screen_reference_y - mouse_screen_last_y, screen_reference_x - mouse_screen_last_x) -
                                        atan2(screen_reference_y - mouse_screen_y, screen_reference_x - mouse_screen_x));
        image_rotate_by(selected_imi, -dAngle);
      } else if (canvas_rotating_center) {
        cv.r = canvas_initial_rotation + (mouse_angle_about_center - mouse_initial_angle);
      } else if (canvas_rotating_point) {  // canvas rotation about reference point
        double screen_reference_x, screen_reference_y;
        canvas_to_screen(canvas_rotation_point_x, canvas_rotation_point_y, &screen_reference_x, &screen_reference_y);
        double dAngle = (180 / M_PI) * (atan2(screen_reference_y - mouse_screen_last_y, screen_reference_x - mouse_screen_last_x) -
                                        atan2(screen_reference_y - mouse_screen_y, screen_reference_x - mouse_screen_x));
        canvas_rotate_about_point_by(canvas_rotation_point_x, canvas_rotation_point_y, dAngle);
      }
      if (image_drag_zoom) {
        double screen_reference_x, screen_reference_y;
        canvas_to_screen(images[selected_imi].x + images[selected_imi].rx, images[selected_imi].y + images[selected_imi].ry, &screen_reference_x,
                         &screen_reference_y);
        double distance1 = sqrt((screen_reference_x - mouse_screen_last_x) * (screen_reference_x - mouse_screen_last_x) +
                                (screen_reference_y - mouse_screen_last_y) * (screen_reference_y - mouse_screen_last_y));
        double distance2 = sqrt((screen_reference_x - mouse_screen_x) * (screen_reference_x - mouse_screen_x) +
                                (screen_reference_y - mouse_screen_y) * (screen_reference_y - mouse_screen_y));
        images[selected_imi].z *= distance2 / distance1;
      }
      if (canvas_drag_zoom) {
        double screen_reference_x, screen_reference_y;
        canvas_to_screen(canvas_rotation_point_x, canvas_rotation_point_y, &screen_reference_x, &screen_reference_y);
        double distance1 = sqrt((screen_reference_x - mouse_screen_last_x) * (screen_reference_x - mouse_screen_last_x) +
                                (screen_reference_y - mouse_screen_last_y) * (screen_reference_y - mouse_screen_last_y));
        double distance2 = sqrt((screen_reference_x - mouse_screen_x) * (screen_reference_x - mouse_screen_x) +
                                (screen_reference_y - mouse_screen_y) * (screen_reference_y - mouse_screen_y));
        canvas_zoom_by_at_point(canvas_rotation_point_x, canvas_rotation_point_y, distance2 / distance1);
      }
      mouse_raw_last_x = e.motion.x;
      mouse_raw_last_y = e.motion.y;
      mouse_screen_last_x = mouse_screen_x;
      mouse_screen_last_y = mouse_screen_y;
      break;
    case SDL_MOUSEWHEEL: {
      if (ctrl_held) {
        if (e.wheel.y > 0)
          image_zoom_by(mouseover_or_selected_imi(), ZOOM_FACTOR);
        else if (e.wheel.y < 0)
          image_zoom_by(mouseover_or_selected_imi(), 1 / ZOOM_FACTOR);
      } else if (shift_held) {
        if (e.wheel.y > 0)
          images[selected_imi].z *= ZOOM_FACTOR;
        else if (e.wheel.y < 0)
          images[selected_imi].z /= ZOOM_FACTOR;
      } else if (tab_held) {
        if (e.wheel.y > 0)
          canvas_zoom_by_at_point(canvas_rotation_point_x, canvas_rotation_point_y, ZOOM_FACTOR);
        else if (e.wheel.y < 0)
          canvas_zoom_by_at_point(canvas_rotation_point_x, canvas_rotation_point_y, 1 / ZOOM_FACTOR);
      } else {
        if (e.wheel.y > 0)
          canvas_zoom_by(ZOOM_FACTOR);
        else if (e.wheel.y < 0)
          canvas_zoom_by(1 / ZOOM_FACTOR);
      }

    } break;
    case SDL_KEYUP:
      switch (e.key.keysym.sym) {  // unmodified keys up
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
          shift_held = 0;
          show_center_mark = 0;
          show_image_reference_marks = 0;
          break;
        case SDLK_LCTRL:
        case SDLK_RCTRL:
          ctrl_held = 0;
          show_center_mark = 0;
          break;
        case SDLK_TAB:
          tab_held = 0;
          show_canvas_rotation_point = 0;
          break;
      }
      break;
    case SDL_KEYDOWN:
      switch (e.key.keysym.sym) {  // keys regardless of modifiers
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
          shift_held = 1;
          show_image_reference_marks = 1;
          break;
        case SDLK_LCTRL:
        case SDLK_RCTRL:
          ctrl_held = 1;
          show_center_mark = 1;
          break;
        case SDLK_TAB:
          tab_held = 1;
          show_canvas_rotation_point = 1;
          break;
        case SDLK_MINUS:
          cv.r = 0;  // reset canvas rotation
          break;
        case SDLK_EQUALS:
          canvas_zoom_reset();
          break;
        case SDLK_QUOTE:
          image_zoom_reset(mouseover_or_selected_imi());
          break;
        case SDLK_SEMICOLON:
          image_rotate_snap(selected_imi, 90);
          break;
        case SDLK_COMMA:
          image_to_on_bottom(selected_imi);
          break;
        case SDLK_PERIOD:
          image_to_on_top(selected_imi);
          break;
        case SDLK_r:
          image_rotate_by(mouseover_selects_imi(), 90);
          break;
        case SDLK_SLASH:
          image_rotation_point_set_center(selected_imi);
          break;
        case SDLK_RETURN: {
          int imi = mouseover_selects_imi();
          canvas_center_on_image_fit(imi);
          image_series_set(imi);
        } break;
        case SDLK_PAGEUP:
          image_center_series_prev();
          break;
        case SDLK_PAGEDOWN:
          image_center_series_next();
          break;
        case SDLK_SPACE:
          canvas_center_on_image(mouseover_selects_imi());
          break;
        case SDLK_BACKQUOTE:
          canvas_zoom_center_fitall();
          break;
        case SDLK_z:
          images[selected_imi].opacity -= 16;
          break;
        case SDLK_x:
          images[selected_imi].opacity += 16;
          break;
        case SDLK_b:
          global_testA += 1;
          break;
        case SDLK_v:
          global_testA -= 1;
          break;
        case SDLK_l:
          global_testB -= 1;
          break;
        case SDLK_k:
          global_testB += 1;
          break;
        case SDLK_n:
          global_testC -= 1;
          break;
        case SDLK_m:
          global_testC += 1;
          break;
        case SDLK_F11: {
          Uint32 flags = SDL_GetWindowFlags(window);
          if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
            SDL_SetWindowFullscreen(window, 0);  // Back to windowed
          } else {
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);  // Fullscreen
          }
        } break;
      }
      if (shift_held) {  // keys + shift key held
        switch (e.key.keysym.sym) {
          case SDLK_LEFTBRACKET:
            image_rotate_by(selected_imi, ROTATE_STEP);
            break;
          case SDLK_RIGHTBRACKET:
            image_rotate_by(selected_imi, -ROTATE_STEP);
            break;
        }
      } else if (ctrl_held) {  // key + control held
        switch (e.key.keysym.sym) {
          case SDLK_LEFTBRACKET:
            canvas_rotate_about_point_by(canvas_rotation_point_x, canvas_rotation_point_y, -ROTATE_STEP);  // canvas rotation about cursor position
            break;
          case SDLK_RIGHTBRACKET:
            canvas_rotate_about_point_by(canvas_rotation_point_x, canvas_rotation_point_y, ROTATE_STEP);
            break;
        }
      } else {  // keys with no modifiers
        switch (e.key.keysym.sym) {
          case SDLK_LEFTBRACKET:
            cv.r += ROTATE_STEP;  // canvas rotation about center of window
            break;
          case SDLK_RIGHTBRACKET:
            cv.r -= ROTATE_STEP;
            break;
        }
      }
      break;
  }
  global_testA = image_drag_zoom + image_rotating * 2;
}