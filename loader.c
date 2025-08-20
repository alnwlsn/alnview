#include "loader.h"

char img_dir[FILEPATHLEN] = ".";

// void save_state(const char *filename, Image *images, int count) {
void save_state() {
  FILE *f = fopen(SAVEFILE, "wb");
  if (!f) {
    perror("fopen");
    return;
  }

  uint16_t rev = REVISION;
  fwrite(&rev, sizeof(uint16_t), 1, f);
  fwrite(&cv, sizeof(CanvasView), 1, f);
  fwrite(&images_count, sizeof(int), 1, f);
  fwrite(&selected_imi, sizeof(int), 1, f);

  int max_canvas2 = MAX_CANVAS;
  fwrite(&max_canvas2, sizeof(int), 1, f);
  for (int i = 0; i < max_canvas2; i++) {
    fwrite(&cvp[i], sizeof(CanvasView), 1, f);
  }

  for (int i = 0; i < images_count; i++) {
    ImageSave s;
    s.width = images[i].width;
    s.height = images[i].height;
    s.x = images[i].x;
    s.y = images[i].y;
    s.r = images[i].r;
    s.rx = images[i].rx;
    s.ry = images[i].ry;
    s.z = images[i].z;
    s.opacity = images[i].opacity;
    s.crop_top = images[i].crop_top;
    s.crop_right = images[i].crop_right;
    s.crop_bottom = images[i].crop_bottom;
    s.crop_left = images[i].crop_left;
    s.series_order = images[i].series_order;
    s.draw_order = images[i].draw_order;
    s.sort_index = images[i].sort_index;
    strncpy(s.filepath, images[i].filepath, FILEPATHLEN);

    fwrite(&s, sizeof(ImageSave), 1, f);
  }

  fclose(f);
}

bool load_state(bool show) {
  FILE *f = fopen(SAVEFILE, "rb");
  if (!f) {
    // perror("fopen");
    return 0;
  }

  uint16_t rev = -1;
  fread(&rev, sizeof(uint16_t), 1, f);
  if (rev != REVISION) {
    fprintf(stderr, "%d wrong savefile revision\n", rev);
    return 0;
  }
  fread(&cv, sizeof(CanvasView), 1, f);
  int images_count_load = 0;
  fread(&images_count_load, sizeof(int), 1, f);
  fread(&selected_imi, sizeof(int), 1, f);

  int cvp_i = 0;
  fread(&cvp_i, sizeof(int), 1, f);
  for (int i = 0; i < cvp_i; i++) {
    fread(&cvp[i], sizeof(CanvasView), 1, f);
  }

  for (int i = 0; i < images_count_load; i++) {
    ImageSave s;
    fread(&s, sizeof(ImageSave), 1, f);
    FILE *test = fopen(s.filepath, "rb");
    if (!test) {
      fprintf(stderr, "savefile image not found: %s\n", s.filepath);
      continue;  // skip this one
    }
    fclose(test);
    // printf("%s\n", s.filepath);
    int imi = image_load(s.filepath);
    snprintf(coordText, sizeof(coordText), "loading %s", s.filepath);
    if (show) render_text_screen(coordText);

    images[imi].x = s.x;
    images[imi].y = s.y;
    images[imi].r = s.r;
    images[imi].rx = s.rx;
    images[imi].ry = s.ry;
    images[imi].z = s.z;
    images[imi].opacity = s.opacity;
    images[imi].crop_top = s.crop_top;
    images[imi].crop_right = s.crop_right;
    images[imi].crop_bottom = s.crop_bottom;
    images[imi].crop_left = s.crop_left;
    images[imi].series_order = s.series_order;
    images[imi].draw_order = s.draw_order;
    images[imi].sort_index = s.sort_index;
  }

  fclose(f);

  return 1;
}

void uniload(bool show) {
  if (!load_state(show)) {
    images_load_dir(img_dir, show);  // load all the images
    canvas_zoom_center_fitall();
  } else {
    images_load_dir(img_dir, show);  // still load extra imgs
  }
}