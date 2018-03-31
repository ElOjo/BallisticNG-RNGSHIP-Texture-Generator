#ifndef _RNGTEXGEN_H_
#define _RNGTEXGEN_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "VECMATHS.h"

#define ALHSHIFT 24
#define REDSHIFT 16
#define GRESHIFT 8
#define BLUSHIFT 0

#define TEXSZ 256

uint32_t* masterNoise;
uint32_t* masterTexture;

typedef enum tubeDir {UP, DOWN, LEFT, RIGHT} tubeDir_t;

typedef struct rect{
	
	float x;
	float y;
	float sx;
	float sy;
	
}rect_t;

typedef struct intrect{
	
	int16_t x0;
	int16_t y0;
	int16_t x1;
	int16_t y1;
	
}intrect_t;

typedef struct RGBAcol{
	
	float R;
	float G;
	float B;
	float A;
	
}RGBAcol_t;

typedef struct tubePart{
	//straight tube
	vec2D_t dir;
	segment2D_t segment;
	//round tube
	arc2D_t arc;
	float radius;
	
	uint8_t specIntensity;
	uint8_t isridged;
	
}tubePart_t;

typedef struct mechDome{
	
	vec2D_t pos;
	float radius;
	float sphericity;
	RGBAcol_t color;
	
}mechDome_t;

float randFloatRange(float min, float max);

uint32_t RGBA2DW(uint8_t R, uint8_t G, uint8_t B, uint8_t A);

void BlitMasterTexBuffer(uint32_t* tbuffer, uint16_t sx, uint16_t sy, uint16_t xoffset, uint16_t yoffset);

uint32_t* GenerateNoise(uint16_t w, uint16_t h);
uint8_t ScaleNoise(uint32_t* noise, uint16_t w, uint16_t h, float u, float v);
uint32_t* GenerateTurbulence(uint16_t w, uint16_t h, char col, uint8_t iterations);
uint32_t* GenerateBlank(uint16_t w, uint16_t h, uint32_t col);

void AddNoise(float intensity, uint32_t* pix, uint16_t w, uint16_t h);
void AddLinearGradient(RGBAcol_t col, float angle, float length, uint32_t* pix, uint16_t w, uint16_t h);


RGBAcol_t Color_IntensityScale(RGBAcol_t col, float intensity);
RGBAcol_t Color_DW2RGBA(uint32_t col);
uint32_t Color_RGBA2DW(RGBAcol_t col);
float Color_Clamp(float col, float min, float max);
float Color_Lerp(float a, float b, float t);
void Color_RGBATint(uint32_t* pix, uint16_t w, uint16_t h, float R, float G, float B);
void Color_DWLayerMultiply(uint32_t* l1, uint32_t* l2, uint16_t w, uint16_t h);

void MirrorBuffer(uint32_t* pix, uint16_t w, uint16_t h, char mirroraxis);

void DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t colval, uint32_t* pix, uint16_t w, uint16_t h);
void DrawScanline(uint16_t x0, uint16_t x1, uint16_t y, uint32_t col, uint32_t* pix, uint16_t w);
void DrawRect(intrect_t box, uint32_t col, uint32_t* pix, uint16_t w);

void GenerateTubeSegment(tubePart_t pipe, uint32_t* pix, uint16_t w, uint16_t h);
void GenerateTubeArc(tubePart_t pipe, uint32_t* pix, uint16_t w, uint16_t h);
void GenerateRandomTubePath(float minRadius, float maxRadius, uint8_t mainDir, uint32_t tint, uint32_t* pix, uint16_t w, uint16_t h);
void GenerateEngineTube(uint32_t* pix, uint16_t w, uint16_t h);
void TileBorder(uint16_t thickness, RGBAcol_t color, uint32_t* pix, uint16_t w, uint16_t h);
void GenerateMechanicsDome(mechDome_t ledome, uint32_t* pix, uint16_t w, uint16_t h);

void LinearMotionBlur(float angle, uint8_t maxiterations, uint32_t* pix, uint16_t w, uint16_t h);
void BoxBlur(uint8_t iterations, uint32_t* pix, uint16_t w, uint16_t h);

uint32_t* GenerateGlass(RGBAcol_t glassTint, uint16_t w, uint16_t h);
uint32_t* GenerateGreebles(uint16_t ndivisions, uint16_t w, uint16_t h);
#endif
