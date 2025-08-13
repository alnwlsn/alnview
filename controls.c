#include "controls.h"

#include <stdbool.h>

// modes
bool canvas_rotating_center = 0;  // for rotation of canvas about center of screen
bool canvas_rotating_point = 0;
bool mouse_dragging = 0;  // for panning
bool image_dragging = 0;
bool image_rotating = 0;

// states
int shift_held = 0;  // shift key held
int ctrl_held = 0;   // control key held
int tab_held = 0;
int dragged_imi = -1;


// move references
int mouse_raw_last_x = 0, mouse_raw_last_y = 0;           // for mouse dragging positioning
double mouse_screen_last_x = 0, mouse_screen_last_y = 0;  // for mouse dragging rotation
double canvas_initial_rotation = 0;
double mouse_initial_angle = 0;

int animation = 0;
int animation_step = 0;

// globals
int last_dragged_imi = 0;
double canvas_rotation_point_x = 0;
double canvas_rotation_point_y = 0;
bool show_center_mark = 0;            // for rendering, show a center mark if true
bool show_canvas_rotation_point = 0;  // for renderer to show another reference mark
bool show_image_reference_marks = 0;  // show corners, base and rotation point of selected image
int global_testA = 0;
int global_testB = 0;
int global_testC = 0;

int select_imi() {  // currently selected image
  if (dragged_imi > -1) {
    return dragged_imi;
  } else {
    int imi = image_point_on(mouse_canvas_x, mouse_canvas_y);
    return imi;
  }
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
        if (ctrl_held) {
          canvas_rotation_point_x = mouse_canvas_x;
          canvas_rotation_point_y = mouse_canvas_y;
        }
        if (shift_held) {
          image_rotation_point_set_new(last_dragged_imi, mouse_canvas_x, mouse_canvas_y);
        }
      } else if (e.button.button == SDL_BUTTON_MIDDLE) {
        if (shift_held) {  // middle click and drag to rotate canvas about center
          mouse_initial_angle = mouse_angle_about_center;
          canvas_initial_rotation = cv.r;
          canvas_rotating_center = 1;
        } else if (ctrl_held) {  // middle click and drag to rotate canvas about reference
          canvas_rotating_point = 1;
          mouse_screen_last_x = mouse_screen_x;
          mouse_screen_last_y = mouse_screen_y;
        } else {
          image_rotating = 1;
          mouse_screen_last_x = mouse_screen_x;
          mouse_screen_last_y = mouse_screen_y;
          show_image_reference_marks = 1;
        }
      } else if (e.button.button == SDL_BUTTON_RIGHT) {
        int imi = image_point_on(mouse_canvas_x, mouse_canvas_y);
        if (imi > -1) {
          last_dragged_imi = imi;
          dragged_imi = imi;
          image_dragging = 1;
          mouse_raw_last_x = e.button.x;
          mouse_raw_last_y = e.button.y;
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
        dragged_imi = -1;
      }
      break;
    case SDL_MOUSEMOTION:
      if (mouse_dragging) {
        canvas_drag_screen_by(e.motion.x - mouse_raw_last_x, mouse_raw_last_y - e.motion.y);
        mouse_raw_last_x = e.motion.x;
        mouse_raw_last_y = e.motion.y;
      }
      if (image_dragging) {
        image_drag_screen_by(dragged_imi, e.motion.x - mouse_raw_last_x, mouse_raw_last_y - e.motion.y);
        mouse_raw_last_x = e.motion.x;
        mouse_raw_last_y = e.motion.y;
      }
      if (image_rotating) {
        double screen_reference_x, screen_reference_y;
        canvas_to_screen(images[last_dragged_imi].x + images[last_dragged_imi].rx, images[last_dragged_imi].y + images[last_dragged_imi].ry,
                         &screen_reference_x, &screen_reference_y);
        double dAngle = (180 / M_PI) * (atan2(screen_reference_y - mouse_screen_last_y, screen_reference_x - mouse_screen_last_x) -
                                        atan2(screen_reference_y - mouse_screen_y, screen_reference_x - mouse_screen_x));
        mouse_screen_last_x = mouse_screen_x;
        mouse_screen_last_y = mouse_screen_y;
        image_rotate_by(last_dragged_imi, -dAngle);
      } else if (canvas_rotating_center) {
        cv.r = canvas_initial_rotation + (mouse_angle_about_center - mouse_initial_angle);
      } else if (canvas_rotating_point) {  // canvas rotation about reference point
        double screen_reference_x, screen_reference_y;
        canvas_to_screen(canvas_rotation_point_x, canvas_rotation_point_y, &screen_reference_x, &screen_reference_y);
        double dAngle = (180 / M_PI) * (atan2(screen_reference_y - mouse_screen_last_y, screen_reference_x - mouse_screen_last_x) -
                                        atan2(screen_reference_y - mouse_screen_y, screen_reference_x - mouse_screen_x));
        mouse_screen_last_x = mouse_screen_x;
        mouse_screen_last_y = mouse_screen_y;
        canvas_rotate_about_point_by(canvas_rotation_point_x, canvas_rotation_point_y, dAngle);
      }
      break;
    case SDL_MOUSEWHEEL: {
      if (shift_held) {
        if (e.wheel.y > 0)
          image_zoom_by(select_imi(), ZOOM_FACTOR);
        else if (e.wheel.y < 0)
          image_zoom_by(select_imi(), 1 / ZOOM_FACTOR);
      } else if (ctrl_held) {
        if (e.wheel.y > 0)
          images[last_dragged_imi].z *= ZOOM_FACTOR;
        else if (e.wheel.y < 0)
          images[last_dragged_imi].z /= ZOOM_FACTOR;
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
          show_canvas_rotation_point = 0;
          break;
        case SDLK_TAB:
          tab_held = 0;
          break;
      }
      break;
    case SDL_KEYDOWN:
      switch (e.key.keysym.sym) {  // keys regardless of modifiers
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
          shift_held = 1;
          show_center_mark = 1;
          show_image_reference_marks = 1;
          break;
        case SDLK_LCTRL:
        case SDLK_RCTRL:
          ctrl_held = 1;
          show_canvas_rotation_point = 1;
          break;
        case SDLK_TAB:
          tab_held = 1;
          show_center_mark = 0;
          break;
        case SDLK_MINUS:
          cv.r = 0;  // reset canvas rotation
          break;
        case SDLK_EQUALS:
          canvas_zoom_reset();
          break;
        case SDLK_QUOTE:
          image_zoom_reset(select_imi());
          break;
        case SDLK_SEMICOLON:
          image_rotate_snap(last_dragged_imi, 90);
          break;
        case SDLK_COMMA:
          image_to_on_bottom(select_imi());
          break;
        case SDLK_PERIOD:
          image_to_on_top(select_imi());
          break;
        case SDLK_z:
          images[last_dragged_imi].opacity -= 1;
          break;
        case SDLK_x:
          images[last_dragged_imi].opacity += 1;
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
        case SDLK_f: {
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
            image_rotate_by(last_dragged_imi, ROTATE_STEP);
            break;
          case SDLK_RIGHTBRACKET:
            image_rotate_by(last_dragged_imi, -ROTATE_STEP);
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
          case SDLK_p:
            canvas_rotation_point_x = mouse_canvas_x;
            canvas_rotation_point_y = mouse_canvas_y;
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
      //   switch (e.key.keysym.sym) {
      // case SDLK_KP_RIGHTBRACE
      // case SDLK_BACKSLASH:
      //   animation = 1;
      //   animation_step = 0;
      //   break;
      // case SDLK_5:
      //   cv.r = 0;
      //   cv.z = 1.0f;
      //   cv.x = 128;
      //   cv.y = -128;
      //   break;
      // case SDLK_6:
      //   cv.r = 22.5;
      //   cv.z = 1.0f;
      //   cv.x = 128;
      //   cv.y = -128;
      //   break;
      // case SDLK_7:
      //   cv.r = 45;
      //   cv.z = 1.0f;
      //   cv.x = 128;
      //   cv.y = -128;
      //   break;
      // case SDLK_8:
      //   cv.r = -45;
      //   cv.z = 1.0f;
      //   cv.x = 0;
      //   cv.y = 0;
      //   break;
      // case SDLK_1:
      //   double dr = 15.0f;

      //   cv.r -= dr;
      //   double sdx = canvas_rx - cv.x;
      //   double sdy = canvas_ry - cv.y;
      //   double cx = sdx * cos(dr * M_PI / 180) - sdy * sin(dr * M_PI / 180) + cv.x;
      //   double cy = sdy * cos(dr * M_PI / 180) + sdx * sin(dr * M_PI / 180) + cv.y;
      //   // double cdx = -canvas_rx+cv.x;
      //   // double cdy = -canvas_ry+cv.y;
      //   ax = cx;
      //   ay = cy;
      //   bx = cv.x - (cx - canvas_rx);
      //   by = cv.y - (cy - canvas_ry);
      //   printf("%.1f, %.1f\n", ax, ay);

      //   cv.x = bx;
      //   cv.y = by;
      //       break;
      //     case SDLK_2:
      //       cv.r += 15.0f;
      //       break;
      //     case SDLK_q:
      //       break;
      //     case SDLK_w:
      //       cv.r = 0.0f;
      //       cv.x = 0;
      //       cv.y = 0;
      //       cv.z = 1.0;
      //       break;
      //     case SDLK_r:  // pick new point to rotate world around
      //     {
      //       //   canvas_rx = mouse_canvas_x;
      //       //   canvas_ry = mouse_canvas_y;
      //     } break;
      //     case SDLK_e:
      //       break;
      //     case SDLK_i:
      //       cv.y += 1;
      //       break;
      //     case SDLK_k:
      //       cv.y -= 1;
      //       break;
      //     case SDLK_j:
      //       cv.x -= 1;
      //       break;
      //     case SDLK_l:
      //       cv.x += 1;
      //       break;
      //   }
      //   break;
  }
}