#include "RNGTILES.h"

RGBAcol_t shadowCol = {0.0f,0.0f,0.0f,1.0f};


void GenerateMasterBuffers(void){
	
	masterNoise = GenerateNoise(TEXSZ,TEXSZ);
	masterTexture = calloc(TEXSZ*TEXSZ,sizeof(uint32_t));
	
	diffuse1.A=1.0f;	diffuse1.R=1.0f;	diffuse1.G=1.0f;	diffuse1.B=1.0f;
	diffuse2.A=1.0f;	diffuse2.R=1.0f;	diffuse2.G=1.0f;	diffuse2.B=1.0f;
	
}

void HueTest(void){
	
	uint16_t w = 256, h = 256;
	uint32_t* test = GenerateBlank(w,h,0xFFFFFFFF);
	
	RGBAcol_t huetest = {0};
	
	uint32_t i = 0;
	
	for(i=0;i<h;i++){
		
		huetest.R = 0.5f + 0.5f*cos((2*M_PI*i)/h);
		huetest.G = 0.5f + 0.5f*cos((2*M_PI*i)/h + 2*M_PI/3.0f);
		huetest.B = 0.5f + 0.5f*cos((2*M_PI*i)/h + 4*M_PI/3.0f);
		huetest.A = 1.0f;
		
		DrawScanline(0, w, i, Color_RGBA2DW(huetest), test, w);
		
	}
	BlitMasterTexBuffer(test, w, h, 0, 0);
	
	free(test);	
	
}


void MakeMechanicsTopTile(void){
	
	uint16_t w=64, h=128;
	uint32_t* mechbuffer = GenerateBlank(w,h,0x00000000);
	uint32_t* mechdirtbuffer = GenerateTurbulence(w,h,'w',4);

	uint32_t ntubes = 0;
	
	uint8_t dice = rand() % 0xFF; //for RNG
	
	float tubeRadius = 0.5f;
	
	GenerateEngineTube(mechbuffer, w,h);

	for(ntubes=0;ntubes<20;ntubes++){
		tubeRadius = 0.05f/(0.5f*ntubes+1.0f);
		GenerateRandomTubePath(tubeRadius, 3*tubeRadius, 0, 0xFF0000FF, mechbuffer, w,h);
		
	}
	
	Color_RGBATint(mechbuffer,w,h,1.0f, 1.0f, 1.0f);

	MirrorBuffer(mechbuffer,w,h,'y');


	if(dice>0x80){
		mechDome_t ledome = {0};
		ledome.radius = 0.10f + (rand() % 10)/100.0f;
		ledome.pos.x = 0.25f;
		ledome.pos.y = 0.25f + (rand() % 75)/100.0f;
		ledome.sphericity = 0.75f;
		
		ledome.color.A = 1.0f;
		ledome.color.R = 1.0f;
		ledome.color.G = 1.0f;
		ledome.color.B = 1.0f;

		GenerateMechanicsDome(ledome, mechbuffer, w,h);
	}

	Color_DWLayerMultiply(mechbuffer, mechdirtbuffer,w,h);
	
	TileBorder(8, Color_IntensityScale(diffuse2,0.5f), mechbuffer,w,h);

	BlitMasterTexBuffer(mechbuffer,  w,h, 64, 128);

	free(mechdirtbuffer);
	free(mechbuffer);

}


void MakeCockpitTile(void){
	
	RGBAcol_t glassTint = {0.6f,0.8f,1.0f};
	
	uint32_t* glassbuffer = GenerateGlass(glassTint,64,64);
	//BoxBlur(1, glassbuffer, 64, 64);
	BlitMasterTexBuffer(glassbuffer, 64, 64, 128, 128);
	free(glassbuffer);
	
}

void MakeMechanicsBottomTile(void){
	
	uint16_t w = 64, h = 128;
	
	uint32_t* botmechs = GenerateBlank(w,h,0xFFFFFFFF);
	uint32_t* botmechsdirt = GenerateTurbulence(w,h,'w',5);
	uint32_t* greeble = GenerateGreebles(16,w,h);
	
	uint32_t ntubes = 0;
	float tubeRadius = 0.25f;
	
	Color_DWLayerMultiply(botmechs,greeble,w,h);
	
	for(ntubes=0;ntubes<30;ntubes++){
		tubeRadius = 0.05f/(0.5f*ntubes+1.0f);
		GenerateRandomTubePath(tubeRadius, 3*tubeRadius, 0, 0xFF0000FF, botmechs, w, h-(h>>2));
		
	}
	AddLinearGradient(shadowCol,M_PI/2.0f,1.5f,botmechs,w,h);
	AddLinearGradient(shadowCol,M_PI,1.5f,botmechs,w,h);
	MirrorBuffer(botmechs,w,h,'y');
	Color_RGBATint(botmechs,w,h,1.0f,0.8f,0.6f);
	
	AddNoise(0.5f,botmechsdirt,w,h);
	LinearMotionBlur(M_PI/2.0f,4,botmechsdirt,w,h);
	
	Color_DWLayerMultiply(botmechs,botmechsdirt,w,h);
	
	BlitMasterTexBuffer(botmechs,w,h,0,128);
	
	free(botmechs);
	free(botmechsdirt);
	free(greeble);
}

void MakeIntakeTile(void){
	uint16_t w = 64, h = 64, i = 0;
	
	uint32_t* intake = GenerateBlank(w,h,0xFF000000);
	uint32_t* dirt = GenerateTurbulence(w,h,'w',4);
	
	for(i=0;i<h;i++){
		
		if(i%24 == 0 && i>0){
			DrawLine(i,0,0,i,0xFFFFFFFF,intake,w,h);
			DrawLine(0,w-i-1,i,h,0xFFFFFFFF,intake,w,h);
			
			DrawLine(w-i-1,w-1,h,h-i-1,0xFFFFFFFF,intake,w,h);
			DrawLine(i,0,w-1,h-i-1,0xFFFFFFFF,intake,w,h);
		}
	}
	DrawLine(0,0,w-1,h-1,0xFFFFFFFF,intake,w,h);
	DrawLine(0,h-1,w-1,0,0xFFFFFFFF,intake,w,h);
	
	LinearMotionBlur(M_PI/2.0f,2,intake,w,h);
	Color_DWLayerMultiply(intake,dirt,w,h);
	
	Color_RGBATint(intake,w,h,1.0f,1.0f,1.0f);
	
	AddLinearGradient(shadowCol,0.0f,2.0f,intake,w,h);
	AddLinearGradient(shadowCol,M_PI,2.0f,intake,w,h);
	
	BlitMasterTexBuffer(intake,w,h,128,192);
	free(intake);
	free(dirt);
}

void MakeExhaustTile(void){
	
	uint16_t w = 64, h = 64;
	
	uint32_t* exhaust = GenerateBlank(w,h,0xFFFFFFFF);
	uint32_t* exhdirt = GenerateTurbulence(w,h,'w',4);
	uint32_t* greeble = GenerateGreebles(8,w,h);
	
	
	Color_RGBATint(exhaust,w,h,0.25f,0.2f,0.15f);
	AddNoise(0.25f,exhdirt,w,h);
	LinearMotionBlur(M_PI/2.0f,5,exhdirt,w,h);
	
	Color_DWLayerMultiply(exhaust,greeble,w,h);
	Color_DWLayerMultiply(exhaust,exhdirt,w,h);
	
	AddLinearGradient(shadowCol,M_PI/2.0f,3.0f,exhaust,w,h);
	
	BlitMasterTexBuffer(exhaust,w,h,192,128);
	
	free(exhaust);
	free(exhdirt);
	free(greeble);
}

void MakeExhaustInnerTile(void){
	uint16_t w = 64, h = 64, i = 0;
	
	uint32_t* exhaustinner = GenerateBlank(w,h,0xFF000000);
	uint32_t* dirt = GenerateTurbulence(w,h,'w',4);
	
	for(i=0;i<h;i++){
		
		if(i%8 == 0 && i>0)
			DrawScanline(0,w,i,0xFFFFFFFF,exhaustinner,w);
	}

	LinearMotionBlur(M_PI/2.0f,2,exhaustinner,w,h);
	Color_DWLayerMultiply(exhaustinner,dirt,w,h);
	
	Color_RGBATint(exhaustinner,w,h,0.7f,0.6f,0.5f);
	
	AddLinearGradient(shadowCol,0.0f,1.0f,exhaustinner,w,h);
	AddLinearGradient(shadowCol,M_PI,1.0f,exhaustinner,w,h);
	
	BlitMasterTexBuffer(exhaustinner,w,h,192,192);
	free(exhaustinner);
	free(dirt);
}


void MakeFuselageTile(void){
	
	uint16_t w = 128, h = 128;
	
	uint32_t* dif1 = GenerateGreebles(4,w,h);
	uint32_t* dif2 = GenerateGreebles(4,w,h);
	
	uint32_t* dirt = GenerateTurbulence(w,h,'w',8);
	
	diffuse1.R = (rand() % 100) / 100.0f;
	diffuse1.G = (rand() % 100) / 100.0f;
	diffuse1.B = (rand() % 100) / 100.0f;
	
	diffuse2.R = 1.0f-diffuse1.R;
	diffuse2.G = 1.0f-diffuse1.G;
	diffuse2.B = 1.0f-diffuse1.B;
	
	Color_RGBATint(dif1,w,h,diffuse1.R,diffuse1.G,diffuse1.B);
	Color_RGBATint(dif2,w,h,diffuse2.R,diffuse2.G,diffuse2.B);
	
	Color_DWLayerMultiply(dif1,dirt,w,h);
	Color_DWLayerMultiply(dif2,dirt,w,h);
	
	BlitMasterTexBuffer(dif1,w,h,0,0);
	BlitMasterTexBuffer(dif2,w,h,128,0);
	
	free(dif1);
	free(dif2);
	free(dirt);
}

