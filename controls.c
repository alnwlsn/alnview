#include "controls.h"

// modes
bool canvas_rotating_center = 0;  // for rotation of canvas about center of screen
bool canvas_rotating_point = 0;
bool canvas_dragging = 0;  // for panning
bool image_dragging = 0;
bool image_rotating = 0;
bool image_drag_zoom = 0;
bool canvas_drag_zoom = 0;

bool draw_mode = 0;

// states
bool shift_held = 0;  // shift key held
bool ctrl_held = 0;   // control key held
bool tab_held = 0;    // tab key held
bool crop_held = 0;
bool draw_held = 0;
bool draw_pick_held = 0;
bool draw_test_held = 0;

double mouse_canvas_constrain_x = 0;
double mouse_canvas_constrain_y = 0;

int mouseover_or_selected_imi() {  // use either the mouseovered imi or the last selected one
  int imi = image_point_on(mouse_canvas_x, mouse_canvas_y);
  if (imi > -1) {
    return imi;
  }
  return cv.selected_imi;
}
int mouseover_selects_imi() {  // same as or, but selects the mouseovered image if there is one
  int imi = image_point_on(mouse_canvas_x, mouse_canvas_y);
  if (imi > -1) {
    image_series_set(imi);
    cv.selected_imi = imi;
  }
  return cv.selected_imi;
}

int mouseover_selects_imi_or_none() {  // if mouseovered, select image else don't
  int imi = image_point_on(mouse_canvas_x, mouse_canvas_y);
  if (imi > -1) {
    image_series_set(imi);
    cv.selected_imi = imi;
    return cv.selected_imi;
  } else {
    return -1;
  }
}

bool controls_process(SDL_Event e) {
  bool quit = 0;
  switch (e.type) {
    case SDL_MOUSEBUTTONDOWN:
      super_mouse_last(e);
      if (draw_mode) {
        if (e.button.button == SDL_BUTTON_LEFT) {
          if (!draw_held) {
            draw_drop_pen(mouse_canvas_x, mouse_canvas_y);
          }
          draw_held = 1;
        } else if (e.button.button == SDL_BUTTON_MIDDLE) {
          canvas_dragging = 1;
        } else if (e.button.button == SDL_BUTTON_RIGHT) {
          if (!draw_test_held) {
            draw_test_pen(mouse_canvas_x, mouse_canvas_y);
          }
          draw_test_held = 1;
          break;
        }
      } else {
        if (e.button.button == SDL_BUTTON_LEFT) {
          canvas_dragging = 1;
          if (tab_held) super_canvas_rotation_init();
          if (shift_held) image_rotation_point_set_new(cv.selected_imi, mouse_canvas_x, mouse_canvas_y);
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
            image_dragging = 1;
          } else {
            if (mouseover_selects_imi_or_none() > -1) image_dragging = 1;
          }
        }
      }
      break;
    case SDL_MOUSEBUTTONUP:
      if (e.button.button == SDL_BUTTON_LEFT) {
        canvas_dragging = 0;
        if (draw_held) {
          draw_held = 0;
          draw_lift_pen();
        }
      } else if (e.button.button == SDL_BUTTON_MIDDLE) {
        canvas_rotating_center = 0;
        show_canvas_rotation_point = 0;
        canvas_rotating_point = 0;
        image_rotating = 0;
        canvas_dragging = 0;
        if (!shift_held) show_image_reference_marks = 0;
      } else if (e.button.button == SDL_BUTTON_RIGHT) {
        image_dragging = 0;
        image_drag_zoom = 0;
        canvas_drag_zoom = 0;
        if (draw_test_held) {
          draw_test_held = 0;
          draw_commit_pen();
        }
      }
      break;
    case SDL_MOUSEMOTION:
      if (crop_held) image_crop(cv.selected_imi);
      if (canvas_dragging) canvas_drag_screen_by(e.motion.x - mouse_raw_last_x, mouse_raw_last_y - e.motion.y);
      if (image_dragging) image_drag_screen_by(cv.selected_imi, e.motion.x - mouse_raw_last_x, mouse_raw_last_y - e.motion.y);
      if (image_rotating) super_image_rotating();
      if (canvas_rotating_center)
        super_canvas_rotating_center();
      else if (canvas_rotating_point)
        super_canvas_rotating_point();
      if (image_drag_zoom) super_image_drag_zoom();
      if (canvas_drag_zoom) super_canvas_drag_zoom();
      if (draw_held) {
        if (shift_held) {
          draw_move_pen(mouse_canvas_x, mouse_canvas_constrain_y);
        } else if (ctrl_held) {
          draw_move_pen(mouse_canvas_constrain_x, mouse_canvas_y);
        } else {
          draw_move_pen(mouse_canvas_x, mouse_canvas_y);
        }
      }
      if (draw_test_held) {
        draw_test_pen(mouse_canvas_x, mouse_canvas_y);
      }
      super_mouse_last(e);
      break;
    case SDL_MOUSEWHEEL: {
      if (ctrl_held) {
        if (e.wheel.y > 0)
          image_zoom_by(image_point_on(mouse_canvas_x, mouse_canvas_y), ZOOM_FACTOR);
        else if (e.wheel.y < 0)
          image_zoom_by(image_point_on(mouse_canvas_x, mouse_canvas_y), 1 / ZOOM_FACTOR);
      } else if (shift_held) {
        if (e.wheel.y > 0)
          images[cv.selected_imi].z *= ZOOM_FACTOR;
        else if (e.wheel.y < 0)
          images[cv.selected_imi].z /= ZOOM_FACTOR;
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
        case SDLK_SPACE:
          show_image_reference_marks = 0;
          if (draw_pick_held) {
            draw_pick_held = 0;
            draw_pick_close();
          }
          break;
      }
      break;
    case SDL_KEYDOWN:
      switch (e.key.keysym.sym) {  // keys regardless of modifiers
        case SDLK_ESCAPE:
          draw_mode = 0;
          break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
          if (draw_mode) {
          } else {
            if (shift_held == 0) mouse_canvas_constrain_y = mouse_canvas_y;
            shift_held = 1;
            show_image_reference_marks = 1;
          }
          shift_held = 1;
          break;
        case SDLK_LCTRL:
        case SDLK_RCTRL:
          if (ctrl_held == 0) mouse_canvas_constrain_x = mouse_canvas_x;
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
        case SDLK_o:
          cv.r = 0;  // reset canvas rotation
          break;
        case SDLK_p:
          canvas_zoom_reset();
          break;
        case SDLK_QUOTE:
          image_zoom_reset(mouseover_or_selected_imi());
          break;
        case SDLK_SEMICOLON:
          image_rotate_snap(mouseover_selects_imi(), 90);
          break;
        case SDLK_COMMA:
          image_to_on_bottom(cv.selected_imi);
          break;
        case SDLK_PERIOD:
          image_to_on_top(cv.selected_imi);
          break;
        case SDLK_r:
          image_rotate_by(mouseover_selects_imi(), -90);
          break;
        case SDLK_SLASH:
          image_rotation_point_set_center(cv.selected_imi);
          break;
        case SDLK_RETURN:
          if (draw_mode) {
            draw_drop_pen(mouse_canvas_x, mouse_canvas_y);
            draw_lift_pen();
          } else {
            canvas_center_on_image_fit(mouseover_selects_imi());
          }
          break;
        case SDLK_PAGEUP:
          image_center_series_prev();
          break;
        case SDLK_PAGEDOWN:
          image_center_series_next();
          break;
        case SDLK_v:
          canvas_center_on_image(cv.selected_imi);
          break;
        case SDLK_SPACE:
          if (draw_mode) {
            if (!draw_pick_held) {
              draw_pick_open();
            }
            draw_pick_held = 1;
          } else {
            mouseover_selects_imi();
            show_image_reference_marks = 1;
          }
          break;
        case SDLK_BACKQUOTE:
          canvas_zoom_center_fitall();
          break;
        case SDLK_z:
          super_opacity_decrease();
          break;
        case SDLK_x:
          super_opacity_increase();
          break;
        case SDLK_w: {
          CanvasView gv = cv;
          images_unload();
          loader_uni(0);
          cv = gv;
        } break;
        case SDLK_e:
          super_reload_single_image(mouseover_or_selected_imi());
          break;
        case SDLK_s:
          save_state();
          break;
        case SDLK_UP:
          canvas_center_on_nearest_image_in_direction(cv.selected_imi, 90);
          break;
        case SDLK_DOWN:
          canvas_center_on_nearest_image_in_direction(cv.selected_imi, 270);
          break;
        case SDLK_LEFT:
          canvas_center_on_nearest_image_in_direction(cv.selected_imi, 180);
          break;
        case SDLK_RIGHT:
          canvas_center_on_nearest_image_in_direction(cv.selected_imi, 0);
          break;
        case SDLK_F11:
          super_toggle_fullscreen();
          break;
        case SDLK_d:
          draw_mode = 1;
          break;
        case SDLK_n:
          image_discard_fullres_or_auto_hires(mouseover_or_selected_imi());
          break;
        case SDLK_b:
          image_restore_fullres(mouseover_or_selected_imi());
          break;
      }
      if (shift_held) {  // keys + shift key held
        switch (e.key.keysym.sym) {
          case SDLK_LEFTBRACKET:
            image_rotate_by(cv.selected_imi, ROTATE_STEP);
            break;
          case SDLK_RIGHTBRACKET:
            image_rotate_by(cv.selected_imi, -ROTATE_STEP);
            break;
          case SDLK_0:
            canvas_set_cvp(0);
            break;
          case SDLK_1:
            canvas_set_cvp(1);
            break;
          case SDLK_2:
            canvas_set_cvp(2);
            break;
          case SDLK_3:
            canvas_set_cvp(3);
            break;
          case SDLK_4:
            canvas_set_cvp(4);
            break;
          case SDLK_5:
            canvas_set_cvp(5);
            break;
          case SDLK_6:
            canvas_set_cvp(6);
            break;
          case SDLK_7:
            canvas_set_cvp(7);
            break;
          case SDLK_8:
            canvas_set_cvp(8);
            break;
          case SDLK_9:
            canvas_set_cvp(9);
            break;
          case SDLK_q:
            quit = 1;
            break;
          case SDLK_BACKSPACE:
            draw_forward_pen();
            break;
          case SDLK_c:
            image_uncrop(mouseover_or_selected_imi());
            break;
          case SDLK_g:
            image_auto_hires_restore(false);
            break;
        }
      } else if (tab_held) {  // key
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
            canvas_use_cvp(0);
            break;
          case SDLK_1:
            canvas_use_cvp(1);
            break;
          case SDLK_2:
            canvas_use_cvp(2);
            break;
          case SDLK_3:
            canvas_use_cvp(3);
            break;
          case SDLK_4:
            canvas_use_cvp(4);
            break;
          case SDLK_5:
            canvas_use_cvp(5);
            break;
          case SDLK_6:
            canvas_use_cvp(6);
            break;
          case SDLK_7:
            canvas_use_cvp(7);
            break;
          case SDLK_8:
            canvas_use_cvp(8);
            break;
          case SDLK_9:
            canvas_use_cvp(9);
            break;
          case SDLK_BACKSPACE:
            draw_back_pen();
            break;
          case SDLK_c:
            if (crop_held == 0) {
              image_crop(cv.selected_imi);
            }
            crop_held = 1;
            break;
          case SDLK_g:
            image_auto_hires_restore(true);
            break;
        }
      }
      break;
  }
  return quit;
}