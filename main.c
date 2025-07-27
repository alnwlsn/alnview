#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WINDOW_WIDTH  2000
#define WINDOW_HEIGHT 1200
#define IMG_DIR       "img"

typedef struct {
    SDL_Texture *texture;
    int width;
    int height;
    int x;
    int y;
} Image;

Image *images = NULL;
int image_count = 0;

SDL_Texture* load_texture(SDL_Renderer *renderer, const char *path, int *w, int *h) {
    SDL_Surface *surface = IMG_Load(path);
    if (!surface) {
        fprintf(stderr, "Failed to load %s: %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return NULL;
    }
    *w = surface->w;
    *h = surface->h;
    SDL_FreeSurface(surface);
    return texture;
}

void load_images(SDL_Renderer *renderer) {
    DIR *dir = opendir(IMG_DIR);
    if (!dir) {
        perror("Failed to open img directory");
        exit(1);
    }

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (entry->d_type != DT_REG)
            continue;

        char path[512];
        snprintf(path, sizeof(path), "%s/%s", IMG_DIR, entry->d_name);
        
        images = realloc(images, sizeof(Image) * (image_count + 1));
        Image *img = &images[image_count];

        img->texture = load_texture(renderer, path, &img->width, &img->height);
        if (!img->texture) continue;

        image_count++;
    }
    closedir(dir);
}

void arrange_images_in_grid() {
    int columns = ceil(sqrt(image_count));
    int spacing = 20;

    int x = 0, y = 0;
    int max_row_height = 0;

    for (int i = 0; i < image_count; ++i) {
        images[i].x = x;
        images[i].y = y;

        x += images[i].width + spacing;
        if (images[i].height > max_row_height)
            max_row_height = images[i].height;

        if ((i + 1) % columns == 0) {
            x = 0;
            y += max_row_height + spacing;
            max_row_height = 0;
        }
    }
}

void move_image_to_front(int index) {
    if (index < 0 || index >= image_count - 1) return;
    Image temp = images[index];
    memmove(&images[index], &images[index + 1], sizeof(Image) * (image_count - index - 1));
    images[image_count - 1] = temp;
}

void move_image_to_back(int index) {
    if (index <= 0 || index >= image_count) return;
    Image temp = images[index];
    memmove(&images[1], &images[0], sizeof(Image) * index);
    images[0] = temp;
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    SDL_Window *window = SDL_CreateWindow("alnview",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    load_images(renderer);
    arrange_images_in_grid();
    float zoom = 1.0f;
    float offsetX = 0, offsetY = 0;
    int dragging = 0;
    int lastX = 0, lastY = 0;
    int image_dragging = -1;  // index of the image being dragged, -1 if none
    int drag_offset_x = 0, drag_offset_y = 0;
    int shift_held = 0;


        // Compute bounding box of the entire layout
    int min_x = 999999, min_y = 999999;
    int max_x = -999999, max_y = -999999;

    for (int i = 0; i < image_count; ++i) {
        int x1 = images[i].x;
        int y1 = images[i].y;
        int x2 = x1 + images[i].width;
        int y2 = y1 + images[i].height;

        if (x1 < min_x) min_x = x1;
        if (y1 < min_y) min_y = y1;
        if (x2 > max_x) max_x = x2;
        if (y2 > max_y) max_y = y2;
    }

    int total_width = max_x - min_x;
    int total_height = max_y - min_y;

    // Calculate initial zoom to fit all images
    float zoom_x = (float)WINDOW_WIDTH / total_width;
    float zoom_y = (float)WINDOW_HEIGHT / total_height;
    zoom = zoom_x < zoom_y ? zoom_x : zoom_y;

    // Optional: apply padding (e.g. 90%)
    zoom *= 0.9f;

    // Center the layout in the window
    offsetX = (WINDOW_WIDTH / zoom - total_width) / 2.0f - min_x;
    offsetY = (WINDOW_HEIGHT / zoom - total_height) / 2.0f - min_y;

    int quit = 0;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                quit = 1;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    dragging = 1;
                    lastX = e.button.x;
                    lastY = e.button.y;
                }
                if (e.button.button == SDL_BUTTON_LEFT) {
                    dragging = 1;
                    lastX = e.button.x;
                    lastY = e.button.y;
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    // Convert mouse position to world coordinates
                    float world_x = e.button.x / zoom - offsetX;
                    float world_y = e.button.y / zoom - offsetY;

                    // Check if mouse is over any image
                    for (int i = image_count - 1; i >= 0; --i) {  // check from top image down
                        int x = images[i].x;
                        int y = images[i].y;
                        int w = images[i].width;
                        int h = images[i].height;
                        if (world_x >= x && world_x <= x + w &&
                            world_y >= y && world_y <= y + h) {
                            image_dragging = i;
                            drag_offset_x = world_x - x;
                            drag_offset_y = world_y - y;
                            break;
                        }
                    }
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    dragging = 0;
                }
                if (e.button.button == SDL_BUTTON_LEFT) {
                    dragging = 0;
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    image_dragging = -1;
                }
                break;
            case SDL_MOUSEMOTION:
                if (dragging) {
                    offsetX += (e.motion.x - lastX) / zoom;
                    offsetY += (e.motion.y - lastY) / zoom;
                    lastX = e.motion.x;
                    lastY = e.motion.y;
                }
                if (dragging) {
                    offsetX += (e.motion.x - lastX) / zoom;
                    offsetY += (e.motion.y - lastY) / zoom;
                    lastX = e.motion.x;
                    lastY = e.motion.y;
                }

                if (image_dragging != -1) {
                    float world_x = e.motion.x / zoom - offsetX;
                    float world_y = e.motion.y / zoom - offsetY;

                    images[image_dragging].x = world_x - drag_offset_x;
                    images[image_dragging].y = world_y - drag_offset_y;
                }
                break;
            case SDL_MOUSEWHEEL: {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                float world_x = mx / zoom - offsetX;
                float world_y = my / zoom - offsetY;

                if (shift_held) {
                    // Shift is held — zoom only image under cursor
                    for (int i = image_count - 1; i >= 0; --i) {
                        Image *img = &images[i];
                        int x = img->x;
                        int y = img->y;
                        int w = img->width;
                        int h = img->height;

                        if (world_x >= x && world_x <= x + w &&
                            world_y >= y && world_y <= y + h) {

                            float factor = (e.wheel.y > 0) ? 1.1f : 1.0f / 1.1f;

                            // Zoom around mouse point relative to image
                            float rel_x = (world_x - x) / w;
                            float rel_y = (world_y - y) / h;

                            int new_w = img->width * factor;
                            int new_h = img->height * factor;

                            // Adjust position so the point under cursor remains stable
                            img->x = world_x - rel_x * new_w;
                            img->y = world_y - rel_y * new_h;

                            img->width = new_w;
                            img->height = new_h;
                            break;
                        }
                    }
                } else {
                    // Regular canvas zoom
                    float before_x = mx / zoom - offsetX;
                    float before_y = my / zoom - offsetY;

                    if (e.wheel.y > 0)
                        zoom *= 1.1f;
                    else if (e.wheel.y < 0)
                        zoom /= 1.1f;

                    // Clamp if desired
                    // if (zoom < 0.05f) zoom = 0.05f;
                    // if (zoom > 10.0f) zoom = 10.0f;

                    float after_x = mx / zoom - offsetX;
                    float after_y = my / zoom - offsetY;

                    offsetX += (after_x - before_x);
                    offsetY += (after_y - before_y);
                }
                break;
            }
            case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_PERIOD || e.key.keysym.sym == SDLK_COMMA) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                float world_x = mx / zoom - offsetX;
                float world_y = my / zoom - offsetY;

                for (int i = image_count - 1; i >= 0; --i) {
                    int x = images[i].x;
                    int y = images[i].y;
                    int w = images[i].width;
                    int h = images[i].height;

                    if (world_x >= x && world_x <= x + w &&
                        world_y >= y && world_y <= y + h) {

                        if (e.key.keysym.sym == SDLK_PERIOD)
                            move_image_to_front(i);
                        else
                            move_image_to_back(i);

                        break;
                    }
                }
            }
            if (e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT)
                shift_held = 1;
            // (keep the existing . and , logic here)
            break;

            case SDL_KEYUP:
                if (e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT)
                    shift_held = 0;
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < image_count; ++i) {
            SDL_Rect dst;
            dst.x = (int)((images[i].x + offsetX) * zoom);
            dst.y = (int)((images[i].y + offsetY) * zoom);
            dst.w = (int)(images[i].width * zoom);
            dst.h = (int)(images[i].height * zoom);
            SDL_RenderCopy(renderer, images[i].texture, NULL, &dst);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60fps
    }

    for (int i = 0; i < image_count; ++i)
        SDL_DestroyTexture(images[i].texture);
    free(images);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
