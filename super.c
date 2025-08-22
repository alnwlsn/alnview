#include "super.h"

double canvas_rotation_point_x = 0;
double canvas_rotation_point_y = 0;
int mouse_raw_last_x = 0;
int mouse_raw_last_y = 0;           // for mouse dragging positioning
double mouse_screen_last_x = 0;
double mouse_screen_last_y= 0;  // for mouse dragging rotation

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

