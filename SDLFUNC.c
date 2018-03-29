#include "SDLFUNC.h"


bool InitGraphics(uint32_t flags, uint32_t w, uint32_t h){
	
	window = NULL;
	renderer = NULL;
	texture = NULL;


    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("Failed to initialize SDL.\n");
        return 0;
    }
	
    window = SDL_CreateWindow("Texture Generator",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,w,h,flags); //SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP
	
    if(window == NULL){
        printf("Failed to create window.\n");
        return 0;
    }
	
	renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED ); //| SDL_RENDERER_PRESENTVSYNC

	texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,
								SDL_TEXTUREACCESS_STATIC,
								w,h);
								
	
	//SDL_ShowCursor(SDL_DISABLE);
	mLquit = 0;
	
	fbuffer = NULL;
	fbuffer = malloc(w*h*sizeof(uint32_t));
	
	if(fbuffer == NULL){
		fprintf(stderr,"FAILED TO ALLOCATE FBUFFER SPACE.\n");
		return 0;
		
	}
	
	return 1;
	
}


void UpdateFB(uint32_t* src, uint32_t pitch){
		
	SDL_UpdateTexture(texture,NULL,src,pitch);
	/*uint32_t* fbuffer;
	SDL_LockTexture(texture,NULL,&fbuffer,&pitch);
	
	memcpy(fbuffer,src,pitch*WINHEIGH);
	
	SDL_UnlockTexture(texture);*/
	//SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer,texture,NULL,NULL);
	SDL_RenderPresent(renderer);
	
}


bool CloseGraphics(void){

    SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);	
	free(fbuffer);
	SDL_Quit();
	
	return 1;
}

void ParseInput(){
	
	if(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			mLquit=1;
		}
		
		if(event.type == SDL_MOUSEMOTION){
			
		}
	}
	
}
