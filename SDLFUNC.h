#ifndef _SDLFUNC_H_
#define _SDLFUNC_H_


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>


#define WINWIDTH 256
#define WINHEIGH 256

typedef uint8_t bool;
bool mLquit; //main loop quit boolean

uint32_t* fbuffer;

SDL_Event event;
SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

bool InitGraphics(uint32_t flags, uint32_t w, uint32_t h);
void UpdateFB(uint32_t* src, uint32_t pitch);
bool CloseGraphics(void);
void ParseInput(void);

#endif
