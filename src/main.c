#include <stdio.h>
#include <SDL2/SDL.h>
#include <signal.h>
#include "init.h"
#include "chip8.h"


int main(int argc, char* argv[])
{

  if (argc < 2)
  {
    printf("Not enough arguments\n");
  }
  // init SDL
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("SDL_Init Error: %s\n", SDL_GetError());
    return -1;
  }

  SDL_Window *window = SDL_CreateWindow("Chip8 Emulator",
                                        64,
                                        32,
                                        640, 
                                        480,
                                        SDL_WINDOW_SHOWN);
  if (window == NULL)
  {
    printf("SDL2 Error: %s\n", SDL_GetError()); 
    return -1;
  }

  SDL_Surface *window_surface = SDL_GetWindowSurface(window);
  if(window_surface == NULL)
  {
    printf("SDL2 Error: %s\n", SDL_GetError()); 
    return -1;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1 , SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
  if(renderer == NULL)
  {
    printf("SDL2 Error: %s\n", SDL_GetError()); 
    return -1;
  }

  SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,64,32);

  uint32_t *pixel_buffer = malloc((32 * 6) * sizeof(uint32_t));
  // init chip8
  cpu chip8;
  chip8_init(&chip8);

  // load rom
  if (load_rom(&chip8, argv[1]) == 0)
  {
    return 0;
  }

  int keep_window_open = 1;
  SDL_Event e;
  while (keep_window_open)
  {
    execute_instruction(&chip8);
    while(SDL_PollEvent(&e) > 0)
    {
      switch (e.type)
      {
        case SDL_QUIT:
          keep_window_open = 0;
          break;
      }
    }

    if (chip8.screen_flag != 0) {
      get_screen_buffer(&chip8, pixel_buffer);
      update_screen(pixel_buffer, &texture, &renderer);
      chip8.screen_flag = 0;
    }
   
  }
  free(pixel_buffer);
  return 0;
}
