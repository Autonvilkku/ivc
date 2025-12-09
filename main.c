#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_render.h>
#include <stdbool.h>
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

  float scale;
  float offsetX;
  float offsetY;

  bool dragging;
  float dragStartX;
  float dragStartY;
  float imgStartOffsetX;
  float imgStartOffsetY;
};

// Initialize SDL
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

// Load image using stb_image
int LoadImage(struct image *img, const char *path) {
  int width, height, channels;
  unsigned char *data = stbi_load(path, &width, &height, &channels, 4);
  if (!data) {
    fprintf(stderr, "Failed to load image: %s\n", stbi_failure_reason());
    return 1;
  }

  *img = (struct image){.width = width,
                        .height = height,
                        .channels = channels,
                        .data = data,
                        .scale = 1.0f,
                        .offsetX = (WINDOW_WIDTH - width) / 2.0f,
                        .offsetY = (WINDOW_HEIGHT - height) / 2.0f,
                        .dragging = false};
  return 0;
}

// Draw image pixel by pixel
void DrawImage(struct image *img, SDL_Renderer *renderer) {
  SDL_FRect rect;
  int pixel_size = 1;

  for (int y = 0; y < img->height; y++) {
    for (int x = 0; x < img->width; x++) {
      unsigned char *pixel = img->data + (y * img->width + x) * 4;
      SDL_SetRenderDrawColor(renderer, pixel[0], pixel[1], pixel[2], pixel[3]);

      rect.x = x * pixel_size * img->scale + img->offsetX;
      rect.y = y * pixel_size * img->scale + img->offsetY;
      rect.w = pixel_size * img->scale;
      rect.h = pixel_size * img->scale;

      SDL_RenderFillRect(renderer, &rect);
    }
  }
}

// Handle events: quit, keyboard, mouse wheel, drag
void HandleEvents(struct window *win, struct image *img) {
  while (SDL_PollEvent(&win->event)) {
    switch (win->event.type) {
    case SDL_EVENT_QUIT:
      win->running = 0;
      break;

    case SDL_EVENT_KEY_DOWN:
      switch (win->event.key.key) {
      case SDLK_ESCAPE:
        win->running = 0;
        break;
      case SDLK_LEFT:
        img->offsetX += 20;
        break;
      case SDLK_RIGHT:
        img->offsetX -= 20;
        break;
      case SDLK_UP:
        img->offsetY += 20;
        break;
      case SDLK_DOWN:
        img->offsetY -= 20;
        break;
      case SDLK_PLUS:
      case SDLK_KP_PLUS: {
        float oldScale = img->scale;
        img->scale *= 1.1f;
        float cx = WINDOW_WIDTH / 2.0f;
        float cy = WINDOW_HEIGHT / 2.0f;
        img->offsetX = cx - ((cx - img->offsetX) * img->scale / oldScale);
        img->offsetY = cy - ((cy - img->offsetY) * img->scale / oldScale);
        break;
      }
      case SDLK_MINUS:
      case SDLK_KP_MINUS: {
        float oldScale = img->scale;
        img->scale /= 1.1f;
        float cx = WINDOW_WIDTH / 2.0f;
        float cy = WINDOW_HEIGHT / 2.0f;
        img->offsetX = cx - ((cx - img->offsetX) * img->scale / oldScale);
        img->offsetY = cy - ((cy - img->offsetY) * img->scale / oldScale);
        break;
      }
      }
      break;

    case SDL_EVENT_MOUSE_WHEEL: {
      float oldScale = img->scale;
      if (win->event.wheel.y > 0)
        img->scale *= 1.1f;
      else if (win->event.wheel.y < 0)
        img->scale /= 1.1f;

      float mx = win->event.wheel.mouse_x;
      float my = win->event.wheel.mouse_y;
      img->offsetX = mx - ((mx - img->offsetX) * img->scale / oldScale);
      img->offsetY = my - ((my - img->offsetY) * img->scale / oldScale);
      break;
    }

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      if (win->event.button.button == SDL_BUTTON_LEFT) {
        img->dragging = true;
        img->dragStartX = win->event.button.x;
        img->dragStartY = win->event.button.y;
        img->imgStartOffsetX = img->offsetX;
        img->imgStartOffsetY = img->offsetY;
      }
      break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
      if (win->event.button.button == SDL_BUTTON_LEFT) {
        img->dragging = false;
      }
      break;

    case SDL_EVENT_MOUSE_MOTION:
      if (img->dragging) {
        float dx = win->event.motion.x - img->dragStartX;
        float dy = win->event.motion.y - img->dragStartY;
        img->offsetX = img->imgStartOffsetX + dx;
        img->offsetY = img->imgStartOffsetY + dy;
      }
      break;

    default:
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s image.png\n", argv[0]);
    return 1;
  }

  struct window win;
  Init(&win);

  struct image img;
  if (LoadImage(&img, argv[1])) {
    Destroy(&win);
    return 1;
  }

  while (win.running) {
    HandleEvents(&win, &img);

    SDL_SetRenderDrawColor(win.renderer, 0, 0, 0, 255);
    SDL_RenderClear(win.renderer);

    DrawImage(&img, win.renderer);
    SDL_RenderPresent(win.renderer);
  }

  stbi_image_free(img.data);
  Destroy(&win);
  return 0;
}
