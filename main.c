
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>
#include <stdio.h>

const char *WINDOW_TITLE = "SDL3 Image Viewer";
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

struct window {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;
  int running;
};

struct image {
  int width;
  int height;
  int channels;
  unsigned char *data;
};

// Initialize SDL and create window/renderer
void Init(struct window *win) {
  SDL_Init(SDL_INIT_VIDEO);
  win->window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  win->renderer = SDL_CreateRenderer(win->window, NULL);
  SDL_SetRenderDrawColor(win->renderer, 0, 0, 0, 255);
  SDL_RenderClear(win->renderer);
  SDL_RenderPresent(win->renderer);
  win->running = 1;
}

// Clean up SDL
void Destroy(struct window *win) {
  SDL_DestroyRenderer(win->renderer);
  SDL_DestroyWindow(win->window);
  SDL_Quit();
}

// Handle events (quit, escape key)
void HandleEvents(struct window *win) {
  while (SDL_PollEvent(&win->event)) {
    switch (win->event.type) {
    case SDL_EVENT_QUIT:
      win->running = 0;
      break;
    case SDL_EVENT_KEY_DOWN:
      if (win->event.key.key == SDLK_ESCAPE) {
        win->running = 0;
      }
      break;
    default:
      break;
    }
  }
}

// Load image using stb_image
int LoadImage(struct image *img, const char *path) {
  int width, height, channels;
  unsigned char *data = stbi_load(path, &width, &height, &channels, 4);
  if (!data) {
    fprintf(stderr, "Failed to load image: %s\n", stbi_failure_reason());
    return 1;
  }
  *img = (struct image){width, height, channels, data};
  return 0;
}

// Draw image pixel by pixel as rectangles
void DrawImage(struct image *img, SDL_Renderer *renderer, float scale,
               float offsetX, float offsetY) {
  SDL_FRect rect;
  int pixel_size = 1;

  for (int y = 0; y < img->height; y++) {
    for (int x = 0; x < img->width; x++) {
      unsigned char *pixel = img->data + (y * img->width + x) * 4;
      SDL_SetRenderDrawColor(renderer, pixel[0], pixel[1], pixel[2], pixel[3]);

      rect.x = x * pixel_size * scale + offsetX;
      rect.y = y * pixel_size * scale + offsetY;
      rect.w = pixel_size * scale;
      rect.h = pixel_size * scale;

      SDL_RenderFillRect(renderer, &rect);
    }
  }
}

int main(int argc, char *argv[]) {
  struct window win;
  Init(&win);

  struct image img;
  if (LoadImage(&img, "image.jpg")) {
    Destroy(&win);
    return 1;
  }

  float scale = 1.0f;
  float offsetX = 0;
  float offsetY = 0;

  while (win.running) {
    HandleEvents(&win);

    SDL_SetRenderDrawColor(win.renderer, 0, 0, 0, 255);
    SDL_RenderClear(win.renderer);

    DrawImage(&img, win.renderer, scale, offsetX, offsetY);
    SDL_RenderPresent(win.renderer);
  }

  stbi_image_free(img.data);
  Destroy(&win);
  return 0;
}
