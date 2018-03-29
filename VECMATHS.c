#include "VECMATHS.h"

float sign(float x){
	
	return x>=0.0f ? 1.0f : -1.0f;
	
}

float vec2D_Dot(vec2D_t v1, vec2D_t v2){
	
	return v1.x*v2.x + v1.y*v2.y;
	
}

float vec2D_Magnitude(vec2D_t vec){
	
	return sqrt(vec.x*vec.x + vec.y*vec.y);
	
}

vec2D_t vec2D_Scale(vec2D_t vec, float scale){
	
	vec.x*=scale;
	vec.y*=scale;
	
	return vec;
	
}

vec2D_t vec2D_Sub(vec2D_t v1, vec2D_t v2){
	
	v1.x = v2.x-v1.x;
	v1.y = v2.y-v1.y;
	
	return v1;
}

vec2D_t vec2D_Add(vec2D_t v1, vec2D_t v2){
	
	v1.x = v2.x+v1.x;
	v1.y = v2.y+v1.y;
	
	return v1;
}

vec2D_t vec2D_Rotate(vec2D_t vec, float angle){
	
	float c = cos(angle), s = sin(angle);
	
	vec.x = vec.x*c - vec.y*s;
	vec.y = vec.x*s + vec.y*c;
	
	return vec;
}

vec2D_t vec2D_Translate(vec2D_t vec, float x, float y){
	
	vec.x+=x;
	vec.y+=y;
	
	return vec;
	
}

void RotateSegment(segment2D_t* seg, float angle){
	
	float xoffset = (seg->B.x + seg->A.x) / 2.0f;
	float yoffset = (seg->B.y + seg->B.y) / 2.0f;
	
	seg->A = vec2D_Translate(seg->A,-xoffset,-yoffset);
	seg->B = vec2D_Translate(seg->B,-xoffset,-yoffset);
	
}

vec2D_t vec2D_Normalize(vec2D_t vec){
	
	float magnitude = vec2D_Magnitude(vec);
	
	if(magnitude == 0.0f){
		return vec;
	}
	
	vec.x /= magnitude;
	vec.y /= magnitude;
	
	return vec;
	
}

vec2D_t vec2D_Normal(vec2D_t vec, uint8_t dir){
	
	vec2D_t ret = vec;
	if(dir){
		ret.x = vec.y;
		ret.y = -vec.x;
	}
	else{
		ret.x = -vec.y;
		ret.y = vec.x;		
	}
	
	return ret;
	
}
