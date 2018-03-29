#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "SDLFUNC.h" //DELETE IF YOU DON'T WANT TO USE SDL2
#include "RNGTILES.h"
#include "qdbmp/qdbmp.h"



int main(int argc, char* argv[]){

	printf("I'm alive yo!\n\nRandom Texture Generator for the RNGShip Generator.\n\n");
	
	InitGraphics(SDL_INIT_VIDEO,TEXSZ,TEXSZ); //DELETE IF YOU DON'T WANT TO USE SDL2
	
	
	srand(time(NULL));
	GenerateMasterBuffers();
	MakeFuselageTile();
	MakeMechanicsTopTile();
	MakeMechanicsBottomTile();
	MakeIntakeTile();
	MakeCockpitTile();
	MakeExhaustTile();
	MakeExhaustInnerTile();
	
	
	
	memcpy(fbuffer,masterTexture,TEXSZ*TEXSZ*sizeof(uint32_t));
	//DELETE FROM HERE IF YOU DON'T WANT TO USE SDL2----------------
	while(!mLquit){

		UpdateFB(fbuffer, TEXSZ*sizeof(uint32_t));
		ParseInput();
		SDL_Delay(10);
		
	}
	//TO HERE-----------------------------------------
	
	printf("SAVING BITMAP WITH QDBMP BY Chai Braudo!\n\n");
	BMP* bmp = NULL;
	uint16_t x,y,i;
	bmp = BMP_Create(TEXSZ,TEXSZ,32);
	
	for(x=0;x<TEXSZ;x++){
		for(y=0;y<TEXSZ;y++){
			i = x + y*TEXSZ;
			BMP_SetPixelRGB(bmp,x,y,(masterTexture[i]>>16) & 0xFF,(masterTexture[i]>>8) & 0xFF,masterTexture[i] & 0xFF);
		}
	}
	
	BMP_WriteFile(bmp,"out.bmp");
	
	BMP_Free(bmp);
	
	//Clean up
	free(masterTexture);
	free(masterNoise);

	CloseGraphics(); //DELETE IF YOU DON'T WANT TO USE SDL2

	return 0;
}

