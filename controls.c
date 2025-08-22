#include "controls.h"

#include <stdbool.h>

// modes
bool canvas_rotating_center = 0;  // for rotation of canvas about center of screen
bool canvas_rotating_point = 0;
bool canvas_dragging = 0;  // for panning
bool image_dragging = 0;
bool image_rotating = 0;
bool image_drag_zoom = 0;
bool canvas_drag_zoom = 0;

// states
int shift_held = 0;  // shift key held
int ctrl_held = 0;   // control key held
int tab_held = 0;    // tab key held
bool crop_held = 0;

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
    image_series_set(imi);
    selected_imi = imi;
  }
  return selected_imi;
}

int mouseover_selects_imi_or_none() {  // if mouseovered, select image else don't
  int imi = image_point_on(mouse_canvas_x, mouse_canvas_y);
  if (imi > -1) {
    image_series_set(imi);
    selected_imi = imi;
    return selected_imi;
  } else {
    return -1;
  }
}

void controls_process(SDL_Event e) {
  switch (e.type) {
    case SDL_MOUSEBUTTONDOWN:
      super_mouse_last(e);
      if (e.button.button == SDL_BUTTON_LEFT) {
        canvas_dragging = 1;
        if (tab_held) super_canvas_rotation_init();
        if (shift_held) image_rotation_point_set_new(selected_imi, mouse_canvas_x, mouse_canvas_y);
      } else if (e.button.button == SDL_BUTTON_MIDDLE) {
        if (tab_held) {  // rotate canvas about reference
          canvas_rotating_point = 1;
        } else if (ctrl_held) {  // rotate canvas about center
          super_canvas_center_rotation_init();
          canvas_rotating_center = 1;
        } else {  // shift or no shift; rotate image
          image_rotating = 1;
          show_image_reference_marks = 1;
        }
      } else if (e.button.button == SDL_BUTTON_RIGHT) {
        if (shift_held) {
          image_drag_zoom = 1;
        } else if (tab_held) {
          canvas_drag_zoom = 1;
        } else if (ctrl_held) {
          mouseover_selects_imi();  // select image
        } else {
          if (mouseover_selects_imi_or_none() > -1) image_dragging = 1;
        }
      }
      break;
    case SDL_MOUSEBUTTONUP:
      if (e.button.button == SDL_BUTTON_LEFT) {
        canvas_dragging = 0;
      } else if (e.button.button == SDL_BUTTON_MIDDLE) {
        canvas_rotating_center = 0;
        show_canvas_rotation_point = 0;
        canvas_rotating_point = 0;
        image_rotating = 0;
        if (!shift_held) show_image_reference_marks = 0;
      } else if (e.button.button == SDL_BUTTON_RIGHT) {
        image_dragging = 0;
        image_drag_zoom = 0;
        canvas_drag_zoom = 0;
      }
      break;
    case SDL_MOUSEMOTION:
      if (crop_held) image_crop(selected_imi);
      if (canvas_dragging) canvas_drag_screen_by(e.motion.x - mouse_raw_last_x, mouse_raw_last_y - e.motion.y);
      if (image_dragging) image_drag_screen_by(selected_imi, e.motion.x - mouse_raw_last_x, mouse_raw_last_y - e.motion.y);
      if (image_rotating) super_image_rotating();
      if (canvas_rotating_center)
        super_canvas_rotating_center();
      else if (canvas_rotating_point)
        super_canvas_rotating_point();
      if (image_drag_zoom) super_image_drag_zoom();
      if (canvas_drag_zoom) super_canvas_drag_zoom();
      super_mouse_last(e);
      break;
    case SDL_MOUSEWHEEL: {
      if (ctrl_held) {
        if (e.wheel.y > 0)
          image_zoom_by(mouseover_selects_imi_or_none(), ZOOM_FACTOR);
        else if (e.wheel.y < 0)
          image_zoom_by(mouseover_selects_imi_or_none(), 1 / ZOOM_FACTOR);
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
        case SDLK_c:
          crop_held = 0;
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
        case SDLK_a:
          super_toggle_antialiasing();
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
          image_rotate_snap(mouseover_selects_imi(), 90);
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
        case SDLK_c:
          if (crop_held == 0) {
            image_crop(selected_imi);
          }
          crop_held = 1;
          break;
        case SDLK_f:
          image_uncrop(mouseover_or_selected_imi());
          break;
        case SDLK_w: {
          CanvasView gv = cv;
          images_unload();
          loader_uni(0);
          cv = gv;
        } break;
        case SDLK_s:
          save_state();
          break;
        case SDLK_UP:
          canvas_center_on_nearest_image_in_direction(selected_imi, 90);
          break;
        case SDLK_DOWN:
          canvas_center_on_nearest_image_in_direction(selected_imi, 270);
          break;
        case SDLK_LEFT:
          canvas_center_on_nearest_image_in_direction(selected_imi, 180);
          break;
        case SDLK_RIGHT:
          canvas_center_on_nearest_image_in_direction(selected_imi, 0);
          break;
        case SDLK_F11:
          super_toggle_fullscreen();
          break;
      }
      if (shift_held) {  // keys + shift key held
        switch (e.key.keysym.sym) {
          case SDLK_LEFTBRACKET:
            image_rotate_by(selected_imi, ROTATE_STEP);
            break;
          case SDLK_RIGHTBRACKET:
            image_rotate_by(selected_imi, -ROTATE_STEP);
            break;
          case SDLK_0:
            cvp[0] = cv;
            break;
          case SDLK_1:
            cvp[1] = cv;
            break;
          case SDLK_2:
            cvp[2] = cv;
            break;
          case SDLK_3:
            cvp[3] = cv;
            break;
          case SDLK_4:
            cvp[4] = cv;
            break;
          case SDLK_5:
            cvp[5] = cv;
            break;
          case SDLK_6:
            cvp[6] = cv;
            break;
          case SDLK_7:
            cvp[7] = cv;
            break;
          case SDLK_8:
            cvp[8] = cv;
            break;
          case SDLK_9:
            cvp[9] = cv;
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
          case SDLK_0:
            cv = cvp[0];
            break;
          case SDLK_1:
            cv = cvp[1];
            break;
          case SDLK_2:
            cv = cvp[2];
            break;
          case SDLK_3:
            cv = cvp[3];
            break;
          case SDLK_4:
            cv = cvp[4];
            break;
          case SDLK_5:
            cv = cvp[5];
            break;
          case SDLK_6:
            cv = cvp[6];
            break;
          case SDLK_7:
            cv = cvp[7];
            break;
          case SDLK_8:
            cv = cvp[8];
            break;
          case SDLK_9:
            cv = cvp[9];
            break;
        }
      }
      break;
  }
}