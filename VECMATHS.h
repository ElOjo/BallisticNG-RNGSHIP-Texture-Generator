#ifndef _TEXGEN_VECMATHS_H_
#define _TEXGEN_VECMATHS_H_

#include <math.h>
#include <float.h>
#include <stdint.h>

typedef struct point2D{
	
	float x;
	float y;
	
}point2D_t;

typedef point2D_t vec2D_t;

typedef struct segment2D{
	
	point2D_t A;
	point2D_t B;
	
	vec2D_t dir;    //directional vector (A to B) (normalized)
	vec2D_t normal; //normal vector (normalized)
	
}segment2D_t;

typedef struct arc2D{
	
	point2D_t center;
	float radius;
	float startangle;
	float endangle;
	
	vec2D_t radvec; //radial vector (normalized)
	vec2D_t normal; //normal vector (normalized)
	
}arc2D_t;

float sign(float x);

float vec2D_Dot(vec2D_t v1, vec2D_t v2);
float vec2D_Magnitude(vec2D_t vec);

vec2D_t vec2D_Scale(vec2D_t vec, float scale);

vec2D_t vec2D_Add(vec2D_t v1, vec2D_t v2);
vec2D_t vec2D_Sub(vec2D_t v1, vec2D_t v2);

vec2D_t vec2D_Rotate(vec2D_t vec, float angle);
vec2D_t vec2D_Translate(vec2D_t vec, float x, float y);

vec2D_t vec2D_Normalize(vec2D_t vec);
vec2D_t vec2D_Normal(vec2D_t vec, uint8_t dir);

void RotateSegment(segment2D_t* seg, float angle);
#endif
