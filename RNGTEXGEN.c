#include "RNGTEXGEN.h"

const float bumpMapBase = 0.5f;
const float bumpMapDepth = 0.5f;
const float ridgelength = 0.3f;


float randFloatRange(float min, float max){
	
	float t = (rand() % 100) * 0.01f;
	
	return (min*(1.0f-t) + max*t);
	
}


uint32_t RGBA2DW(uint8_t R, uint8_t G, uint8_t B, uint8_t A){
	
	return((A<<24) | (R<<16) | (G<<8) | B);
	
}

void BlitMasterTexBuffer(uint32_t* tbuffer, uint16_t sx, uint16_t sy, uint16_t xoffset, uint16_t yoffset){
	
	uint32_t i = 0, j = 0;
	uint32_t size = sx*sy;
	
	for(i=0;i<size;i++){
		if(i%sx == 0 && i>0){
			j++;
		} 
		*(masterTexture + (i%sx) + xoffset + (((j%sy)+yoffset)<<8)) = *(tbuffer+i); 

		
	}
	
}

uint32_t* GenerateNoise(uint16_t w, uint16_t h){
	
	uint32_t i = 0, arrsize = w*h;
	uint32_t* pix = NULL;
	uint8_t A = 0x00;
	
	pix = malloc(arrsize*sizeof(uint32_t));
	if(pix == NULL){
		
		printf("Failed to allocate memory for smooth noise.\n");
		return NULL;
		
	}
	for(i=0;i<arrsize;i++){
		A = (rand() % 0xFF);
		pix[i] = RGBA2DW(A,A,A,A);
		
	}
	
	return pix;
	
}

uint8_t ScaleNoise(uint32_t* noise, uint16_t w, uint16_t h, float u, float v){
	
	float ufract = 0.0f, vfract = 0.0f;
	float intensity = 0.0f;
	
	int32_t u1 = 0, u2 = 0, v1 = 0, v2 = 0, i = 0, x = 0, y = 0;
	uint32_t offset0, offset1, offset2, offset3;
	uint8_t A = 0;

	ufract = (u - floor(u));
	vfract = (v - floor(v));
	
	//printf("%f %f\n",ufract,vfract);
	
	u1 = (int32_t)(floor(u)) % w;
	v1 = (int32_t)(floor(v)) % h;
	
	u2 = (int32_t)(u1 + 1) % w;
	v2 = (int32_t)(v1 + 1) % h;
	
	offset0 = u1 + v1 * w;
	offset1 = u2 + v1 * w;
	offset2 = u1 + v2 * w;
	offset3 = u2 + v2 * w;
	
	intensity = 0.0f;
	
	intensity += ufract * vfract * (noise[offset3]&0xFF)/255.0f;
	intensity += (1-ufract) * vfract * (noise[offset2]&0xFF)/255.0f;
	intensity += ufract * (1-vfract) * (noise[offset1]&0xFF)/255.0f;
	intensity += (1-ufract) * (1-vfract) * (noise[offset0]&0xFF)/255.0f;


	A = (uint8_t)(255*intensity);

	return A;
}


uint32_t* GenerateTurbulence(uint16_t w, uint16_t h, char col, uint8_t iterations){
	
	uint32_t x, y, i=0;
	uint32_t scale = 0;
	uint8_t A = 0;
	
	uint32_t* pbuffer = calloc(w*h,sizeof(uint32_t));
	uint16_t xoffset = rand() % w;
	uint16_t yoffset = rand() % h;
	
	if(pbuffer == NULL){
		
		fprintf(stderr,"ERROR ALLOCATING TURBULENCE IMAGE BUFFER MEMORY!\n");
		return NULL;
		
	}
	
	for(x = 0;x<w;x++){
		for(y = 0;y<h;y++){
			i = x + y*w;
			 
			A = pbuffer[i];
			for(scale=0;scale<iterations;scale++){

				A += (ScaleNoise(masterNoise,w,h,(1.0f*x+xoffset)/(1<<scale),(1.0f*y+yoffset)/(1<<scale))>>(iterations-scale));

			}
			
			pbuffer[i] = 0xFF000000 | A;
			switch(col){
				
				case 'r':
					pbuffer[i] <<= REDSHIFT;
					break;
				
				case 'g':
					pbuffer[i] <<= GRESHIFT;
					break;
					
				case 'b':
					continue;
					break;
					
				case 'w':
					pbuffer[i] |= (pbuffer[i] << REDSHIFT) | (pbuffer[i] << GRESHIFT);
					break;
					
				default:
				
					break;
			}
		}
	}
	return pbuffer;
}



uint32_t* GenerateBlank(uint16_t w, uint16_t h, uint32_t col){
	
	uint32_t* ret = NULL;
	uint32_t size = w*h, i = 0;
	
	ret = malloc(size*sizeof(uint32_t));
	
	if(ret == NULL){
		fprintf(stderr,"Error allocating memory for new blank texture...\n");
		return ret;
	}
	
	for(i=0;i<size;i++){
		
		ret[i] = col;
		
	}
	return ret;
}

void AddNoise(float intensity, uint32_t* pix, uint16_t w, uint16_t h){
	
	RGBAcol_t pixCol = {0};
	uint32_t datasize = w*h, i=0, c=0;
	float channelval, addval;
	
	for(i=0;i<datasize;i++){
		
		pixCol = Color_DW2RGBA(pix[i]);
		addval = intensity*randFloatRange(0.0f,1.0f);
		
		for(c=0;c<4;c++){
			channelval = *((float*)&pixCol+c);
			
			*((float*)&pixCol+c) = Color_Clamp( channelval + addval,0.0f,1.0f);
			
		}
		
		
		pix[i] = Color_RGBA2DW(pixCol);
		
	}
	
}


void AddLinearGradient(RGBAcol_t col, float angle, float length, uint32_t* pix, uint16_t w, uint16_t h){
	
	vec2D_t dir = {length*cos(angle),length*sin(angle)};
	vec2D_t pixDistance = {0};
	
	uint8_t c = 0;
	uint16_t x, y;
	uint32_t boffset = 0;
	
	RGBAcol_t pixCol = {0};
	float blendfac = 0.0f;
	
	if(length == 0.0f){
		return;
	}
	
	for(y=0;y<h;y++){
		pixDistance.y = (1.0f*y - (h>>1))/h;
		for(x=0;x<w;x++){
			boffset = x + y * w;
			
			pixDistance.x = (1.0f*x - (w>>1))/w;
			
			blendfac = 0.5f - 0.5f*vec2D_Dot(pixDistance,dir);
			
			pixCol = Color_DW2RGBA(pix[boffset]);
			
			
			for(c=0;c<4;c++)
				*((float*)&pixCol+c) = Color_Lerp(*((float*)&pixCol+c),*((float*)&col+c),blendfac);
				
			pix[boffset] = Color_RGBA2DW(pixCol);
			
		}
	}
	
}




//COLOR OPS

RGBAcol_t Color_IntensityScale(RGBAcol_t col, float intensity){
	
	col.R *= intensity;
	col.G *= intensity;
	col.B *= intensity;
	
	return col;
}

RGBAcol_t Color_DW2RGBA(uint32_t col){
	
	RGBAcol_t ret;
	float inv255 = 1.0f/255.0f;
	
	ret.A = ((col>>24)&0xFF)*inv255;
	ret.R = ((col>>16)&0xFF)*inv255;
	ret.G = ((col>>8)&0xFF)*inv255;
	ret.B = ((col)&0xFF)*inv255;
	
	return ret;
	
}

uint32_t Color_RGBA2DW(RGBAcol_t col){
	
	uint8_t A,R,G,B;
	
	A = (uint8_t)(255*col.A);
	R = (uint8_t)(255*col.R);
	G = (uint8_t)(255*col.G);
	B = (uint8_t)(255*col.B);
	
	return((A<<24) | (R<<16) | (G<<8) | B);
	
}

float Color_Clamp(float col, float min, float max){
	
	if(col<min){
		return min;
	}
	else if(col>max){
		return max;
	}
	else{
		return col;
	}
	
	
}

float Color_Lerp(float a, float b, float t){
	
	return Color_Clamp(a*t + b*(1.0f-t),0.0f,1.0f);
	
}

void Color_RGBATint(uint32_t* pix, uint16_t w, uint16_t h, float R, float G, float B){
	
	uint32_t i = 0, datasize = w*h;
	RGBAcol_t pixelCol;
	
	for(i=0;i<datasize;i++){
		
		pixelCol = Color_DW2RGBA(pix[i]);
		
		pixelCol.R *= R;
		pixelCol.G *= G;
		pixelCol.B *= B;
		
		pix[i] = Color_RGBA2DW(pixelCol);
	}

}

void Color_DWLayerMultiply(uint32_t* l1, uint32_t* l2, uint16_t w, uint16_t h){
	
	RGBAcol_t c1, c2;
	
	uint32_t i = 0, datasize = w*h;
	
	for(i=0;i<datasize;i++){
		
		c1 = Color_DW2RGBA(l1[i]);
		c2 = Color_DW2RGBA(l2[i]);
		
		c1.R *=c2.R*c2.A;
		c1.G *=c2.G*c2.A;
		c1.B *=c2.B*c2.A;

		
		l1[i] = Color_RGBA2DW(c1);
		
	}
}

RGBAcol_t Color_Normalize(RGBAcol_t col){
	
	RGBAcol_t ret;
	float max = -1000.0f, val;
	uint8_t i = 0;
	
	for(i=0;i<4;i++){

		if(col.R>max)
			max = col.R;
		else if(col.G>max)
			max = col.G;
		else if(col.B>max)
			max = col.B;
		else if(col.A>max)
			max = col.A;
		
	}
	//printf("Max: %f\n",max);
	
	ret.R = col.R/max;
	ret.G = col.G/max;
	ret.B = col.B/max;
	ret.A = col.A/max;
	
	return ret;
}

//Modifiers


void MirrorBuffer(uint32_t* pix, uint16_t w, uint16_t h, char mirroraxis){
	
	uint32_t x, y;
	uint32_t datasize;
	
	switch(mirroraxis){
		
		case 'y':
			
			for(x=0;x<(w>>1);x++){	
				for(y=0;y<h;y++){
				
					pix[(w-x) + y*w] = pix[x + y*w];
				
				}
			}
			
			break;
			
		case 'x':
			for(y=0;y<(h>>1);y++){
				
				memcpy(pix + (h-1-y)*w,pix + y*w,w*sizeof(uint32_t));
					
				
			}
			break;
		
	}

}

//Primitives

void DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t colval, uint32_t* pix, uint16_t w, uint16_t h){
		
	float dx = x2-x1, dy = y2-y1;
	
	uint16_t dist = 0, i = 0;
	
	if(fabs(dx)>fabs(dy))
		dist = fabs(dx);
	else
		dist = fabs(dy);
	
	float xdiv = dx/(float) dist;
	float ydiv = dy/(float) dist;
	
	float X = x1, Y = y1;
	
	for(i=0;i<dist;i++){
		
		pix[lroundf(X) + (lroundf(Y))*w] = colval;
		X+= xdiv;
		Y+= ydiv;
		
	}
	
	pix[x1 + y1*w] = 0xFFFFAA00;

	pix[x2 + y2*w] = 0xFFFFAA00;
	
}

void DrawScanline(uint16_t x0, uint16_t x1, uint16_t y, uint32_t col, uint32_t* pix, uint16_t w){
	uint16_t x = 0;
	for(x=x0;x<x1;x++)
		pix[x + y*w] = col;
}

void DrawRect(intrect_t box, uint32_t col, uint32_t* pix, uint16_t w){
	uint16_t y=0;
	
	for(y=box.y0;y<box.y1;y++){
		
		DrawScanline(box.x0,box.x1,y,col,pix,w);
		
	}

}

//TUBES-------------------------------------------------------------------------------------------------------------------------

void GenerateTubeSegment(tubePart_t pipe, uint32_t* pix, uint16_t w, uint16_t h){
	
	segment2D_t segment = pipe.segment;
	point2D_t pixPosition = {0};
	vec2D_t pixDistanceA = {0}, pixDistanceB = {0};
	float seglength = 0.0f;
	uint16_t xi,yi,temp;
	uint32_t i = 0;
	uint8_t PXval = 0;
	uint16_t coffset = 0;
	float shadowIntensity = 1.0f, bumpMap = 1.0f;
	float normfactor;
	
	if(w>=h){
		normfactor = 1.0f/w;
	}
	else{
		normfactor = 1.0f/h;
	}
	
	seglength = vec2D_Magnitude(vec2D_Sub(segment.B,segment.A));
	
	segment.dir.x = (segment.B.x-segment.A.x)/seglength;
	segment.dir.y = (segment.B.y-segment.A.y)/seglength;
	
	
	segment.normal.x = segment.dir.y;
	segment.normal.y = -segment.dir.x;
	
	
	for(yi=0;yi<h;yi++){
		pixPosition.y = yi*normfactor;
		for(xi=0;xi<w;xi++){
			
			i = xi+yi*w;
			
			pixPosition.x = xi*normfactor;
			
			pixDistanceA = vec2D_Sub(segment.A,pixPosition);
			pixDistanceB = vec2D_Sub(segment.B,pixPosition);
			
			if(vec2D_Dot(pixDistanceA,segment.dir)>=0.0f && vec2D_Dot(pixDistanceB,segment.dir)<=0.0f){
				
				if(fabs(vec2D_Dot(pixDistanceA,segment.normal))<pipe.radius){
					bumpMap = 1.0f;
					if(pipe.isridged){
						bumpMap = (bumpMapBase+bumpMapDepth*(cos(40*M_PI*vec2D_Dot(pixDistanceA,segment.dir))<ridgelength));
					}
					
					PXval = 0xFF & (uint8_t)(bumpMap*pipe.specIntensity*(0.6f+0.4f*cos(M_PI*vec2D_Dot(pixDistanceA,segment.normal)/pipe.radius)));
					pix[i] = RGBA2DW(PXval,PXval,PXval,0xFF);
					
				}
				
				else{
					
					PXval = pix[i] & 0xFF;
					PXval = (uint8_t) ((Color_Clamp((PXval) - 255.0f*(shadowIntensity*pipe.radius*pipe.radius/pow(2*vec2D_Dot(pixDistanceA,segment.normal),2)),0.0f,255.0f)));

					pix[i] = RGBA2DW(PXval,PXval,PXval,0x00);
					
				}
			}
			
			
		}
	}
}

void GenerateTubeArc(tubePart_t pipe, uint32_t* pix, uint16_t w, uint16_t h){
	
	arc2D_t tarc = pipe.arc;
	point2D_t pixPosition = {0};
	vec2D_t pixDistanceC = {0};
	
	float pixAngle = 0.0f; //get polar coordinates (angle)
	float pixDist = 0.0f; //get polar coordinates (radius)
	
	float shadowIntensity = 0.5f, bumpMap = 1.0f;
	float normfactor;
	
	if(w>=h){
		normfactor = 1.0f/w;
	}
	else{
		normfactor = 1.0f/h;
	}
	
	uint16_t xi = 0, yi = 0, boffset;
	uint8_t PXval = 0;
	
	for(yi=0;yi<h;yi++){
		pixPosition.y = (yi*normfactor);
		for(xi=0;xi<w;xi++){
			
			boffset = xi + yi * w;
			
			pixPosition.x = (xi*normfactor);
			
			pixDistanceC = vec2D_Sub(pixPosition,tarc.center);
			
			
			
			pixAngle = atan2(pixDistanceC.x,pixDistanceC.y);
			
			if(pixAngle>=tarc.startangle && pixAngle<=tarc.endangle){
														
				pixDist = 1.0f*fabs(vec2D_Magnitude(pixDistanceC)-tarc.radius);
				
				if(pixDist<=pipe.radius){
					bumpMap = 1.0f;
					if(pipe.isridged){
						bumpMap = (0.6f+0.4f*(cos(10*pixAngle/(tarc.endangle-tarc.startangle)*M_PI)<ridgelength));
					}
					PXval = 0xFF & (uint8_t) (bumpMap*pipe.specIntensity*(0.6f+0.4f*cos(M_PI*pixDist/pipe.radius)));
					pix[boffset] = RGBA2DW(PXval,PXval,PXval,0xFF);
				}
				else{
					PXval = pix[boffset] & 0xFF;
					PXval = (uint8_t) ((Color_Clamp((PXval) - 255.0f*(shadowIntensity*pipe.radius*pipe.radius/pow(2*pixDist,2)),0.0f,255.0f)));
					pix[boffset] = RGBA2DW(PXval,PXval,PXval,0x00);
				}
				
			}
				
		}
	}
	
}


void GenerateRandomTubePath(float minRadius, float maxRadius, uint8_t mainDir, uint32_t tint, uint32_t* pix, uint16_t w, uint16_t h){
	
	tubePart_t pipe = {0};
	
	tubeDir_t tdir, prevDir, startDir = rand()%4;
	
	//type: 0 - goes from top to bottom, 1 - goes from left to right, 2 - goes from left to bottom, 3 - goes from right to bottom
	
	if(maxRadius <= 0.0f){
		
		maxRadius = 0.05f; //just to avoid NaNs
		
	}
	
	float gridunit = 2*maxRadius;
	float pLength = 0.05f;
	float probability = 0.0f;
	
	vec2D_t prevEndPoint;

	uint8_t nDirDice = 0;
	uint32_t segmentcount = 0;
	
	switch(startDir){
		
		case UP:
			
			pipe.segment.A.x = (rand()%8)/10.0f + 0.2f;
			pipe.segment.B.x = pipe.segment.A.x;
			pipe.segment.A.y = 1.0f;
			pipe.segment.B.y = pipe.segment.A.y;
			break;
			
		case DOWN:
			
			pipe.segment.A.x = (rand()%8)/10.0f + 0.2f;
			pipe.segment.B.x = pipe.segment.A.x;
			pipe.segment.A.y = 0.0f;
			pipe.segment.B.y = pipe.segment.A.y;
			break;
		
		case LEFT:
	
			pipe.segment.A.x = 1.0f;
			pipe.segment.B.x = pipe.segment.A.x;
			pipe.segment.A.y = (rand()%8)/10.0f + 0.1f;
			pipe.segment.B.y = pipe.segment.A.y;
			break;
			
		case RIGHT:

			pipe.segment.A.x = 0.0f;
			pipe.segment.B.x = pipe.segment.A.x;
			pipe.segment.A.y = (rand()%8)/10.0f + 0.1f;
			pipe.segment.B.y = pipe.segment.A.y;
			break;
		
	}

	tdir = startDir;
	prevDir = startDir;
	
	pipe.specIntensity = rand()%127 + 127;
	pipe.isridged = rand()%2;
	pipe.radius = randFloatRange(minRadius, maxRadius);
	pipe.arc.radius = 2*pipe.radius;
	
	prevEndPoint = pipe.segment.B; //giving it a starting point
	
	while(pipe.segment.B.x>=0.0f && pipe.segment.B.x<=1.0f && pipe.segment.B.y>=0.0f && pipe.segment.B.y<=1.0f){ //go until the tube hits a wall
		
		probability = (rand()%10)/10.0f;
		
		if(probability < 0.8){
			nDirDice = 0; //keep direction
		}
		else{
			nDirDice = (rand()%2) + 1; //try to change direction
		}
		//Change direction only after the two first segments
		if(segmentcount>1){
			switch(prevDir){
				
				case LEFT:
					
					if(nDirDice == 0){
						tdir = LEFT;
					}
					else if(nDirDice == 1){
						tdir = DOWN;
					}
					else if(nDirDice == 2){
						tdir = UP;
					}
					break;
					
				case RIGHT:
					
					if(nDirDice == 0){
						tdir = RIGHT;
					}
					else if(nDirDice == 1){
						tdir = DOWN;
					}
					else if(nDirDice == 2){
						tdir = UP;
					}
					break;
				
				case UP:
					
					if(nDirDice == 0){
						tdir = UP;
					}
					else if(nDirDice == 1){
						tdir = LEFT;
					}
					else if(nDirDice == 2){
						tdir = RIGHT;
					}
					break;
					
				case DOWN:
									
					if(nDirDice == 0){
						tdir = DOWN;
					}
					else if(nDirDice == 1){
						tdir = LEFT;
					}
					else if(nDirDice == 2){
						tdir = RIGHT;
					}
					break;
				
			}
		}
		switch(tdir){
			
			case LEFT:
				
				if(prevDir == UP){
					pipe.arc.center = vec2D_Translate(prevEndPoint,-pipe.arc.radius,0.0f);
					pipe.arc.startangle = -M_PI/2.0f;
					pipe.arc.endangle = 0.0f;
					GenerateTubeArc(pipe,pix,w,h);
					prevEndPoint.x = pipe.arc.center.x;
					prevEndPoint.y = pipe.arc.center.y-pipe.arc.radius;
					
				}
				else if(prevDir == DOWN){
					
					pipe.arc.center = vec2D_Translate(prevEndPoint,-pipe.arc.radius,0.0f);
					pipe.arc.startangle = -M_PI;
					pipe.arc.endangle = -M_PI/2.0f;
					GenerateTubeArc(pipe,pix,w,h);
					prevEndPoint.x = pipe.arc.center.x;
					prevEndPoint.y = pipe.arc.center.y+pipe.arc.radius;
					
				}
				pipe.segment.B = prevEndPoint;
				pipe.segment.A = vec2D_Translate(pipe.segment.B,-pLength,0.0f);
				GenerateTubeSegment(pipe,pix,w,h);
				prevEndPoint = pipe.segment.A;
				prevDir = LEFT;
				break;
			
			case RIGHT:
				
				if(prevDir == UP){
					pipe.arc.center = vec2D_Translate(prevEndPoint,pipe.arc.radius,0.0f);
					pipe.arc.startangle = 0.0f;
					pipe.arc.endangle = M_PI/2.0f;
					GenerateTubeArc(pipe,pix,w,h);
					prevEndPoint.x = pipe.arc.center.x;
					prevEndPoint.y = pipe.arc.center.y-pipe.arc.radius;
					
				}
				else if(prevDir == DOWN){
					pipe.arc.center = vec2D_Translate(prevEndPoint,pipe.arc.radius,0.0f);
					pipe.arc.startangle = M_PI/2.0f;
					pipe.arc.endangle = M_PI;
					GenerateTubeArc(pipe,pix,w,h);
					prevEndPoint.x = pipe.arc.center.x;
					prevEndPoint.y = pipe.arc.center.y+pipe.arc.radius;
					
				}
				pipe.segment.A = prevEndPoint;
				pipe.segment.B = vec2D_Translate(pipe.segment.A,pLength,0.0f);
				GenerateTubeSegment(pipe,pix,w,h);
				prevEndPoint = pipe.segment.B;
				prevDir = RIGHT;
				break;
				
			case UP:
				
				if(prevDir == RIGHT){
					pipe.arc.center = vec2D_Translate(prevEndPoint,0.0f,-pipe.arc.radius);
					pipe.arc.startangle = -M_PI;
					pipe.arc.endangle = -M_PI/2.0f;
					GenerateTubeArc(pipe,pix,w,h);
					prevEndPoint.x = pipe.arc.center.x+pipe.arc.radius;
					prevEndPoint.y = pipe.arc.center.y;
					
				}
				else if(prevDir == LEFT){
					pipe.arc.center = vec2D_Translate(prevEndPoint,0.0f,-pipe.arc.radius);
					pipe.arc.startangle = M_PI/2.0f;
					pipe.arc.endangle = M_PI;
					GenerateTubeArc(pipe,pix,w,h);
					prevEndPoint.x = pipe.arc.center.x-pipe.arc.radius;
					prevEndPoint.y = pipe.arc.center.y;
					
				}
				pipe.segment.B = prevEndPoint;
				pipe.segment.A = vec2D_Translate(pipe.segment.B,0.0f,-pLength);
				GenerateTubeSegment(pipe,pix,w,h);
				prevEndPoint = pipe.segment.A;
				prevDir = UP;
				break;
				
			case DOWN:
				
				if(prevDir == LEFT){
					pipe.arc.center = vec2D_Translate(prevEndPoint,0.0f,pipe.arc.radius);
					pipe.arc.startangle = 0.0f;
					pipe.arc.endangle = M_PI/2.0f;
					GenerateTubeArc(pipe,pix,w,h);
					prevEndPoint.x = pipe.arc.center.x-pipe.arc.radius;
					prevEndPoint.y = pipe.arc.center.y;
					
				}
				else if(prevDir == RIGHT){
					pipe.arc.center = vec2D_Translate(prevEndPoint,0.0f,pipe.arc.radius);
					pipe.arc.startangle = -M_PI/2.0f;
					pipe.arc.endangle = 0.0f;
					GenerateTubeArc(pipe,pix,w,h);
					prevEndPoint.x = pipe.arc.center.x+pipe.arc.radius;
					prevEndPoint.y = pipe.arc.center.y;
					
				}
				pipe.segment.A = prevEndPoint;
				pipe.segment.B = vec2D_Translate(pipe.segment.A,0.0f,pLength);
				GenerateTubeSegment(pipe,pix,w,h);
				prevEndPoint = pipe.segment.B;
				prevDir = DOWN;
				break;
				
			
		}
		segmentcount++;
	}
	
}

void GenerateEngineTube(uint32_t* pix, uint16_t w, uint16_t h){
	
	
	tubePart_t engine = {0};
	
	engine.segment.A.x = (0.5f*w)/h;
	engine.segment.A.y = 0.0f;
	
	engine.segment.B.x = engine.segment.A.x;
	engine.segment.B.y = 1.0f;
	
	engine.specIntensity = rand()%127 + 127;
	
	engine.radius = 0.2f;
	engine.isridged = 1;
	
	GenerateTubeSegment(engine,pix,w,h);
}

void TileBorder(uint16_t thickness, RGBAcol_t color, uint32_t* pix, uint16_t w, uint16_t h){
	
	//for each border iteration until thickness-1:
	//add 1 to leftmost border
	//subtract 1 from rightmost border
	//add 1 to uppermost border
	//subtract 1 from downmost(?) border
	
	//Emulate bevel on last border:
	//on leftmost and uppermost borders increase luminosity
	//on rightmost and downmost borders decrease luminosity
	
	uint16_t cur_thicc = 0;
	uint16_t max_thicc = (w < h ? w : h); // set max thickness as half of the smallest of the two tile dimensions
	
	uint16_t left = 0, right = w-1, top = 0, bottom = h-1; 
	uint16_t x,y;
	
	uint8_t shadowThickness = 10;
	uint8_t dropShadow = 1;
	RGBAcol_t pixColor;
	float shadowIntensity;
	
	
	//if specified thickness is more than max_thicc, clamp it (also accounts for negative, 'cause uint)
	if(thickness>=max_thicc/2){
		thickness = max_thicc/2;
	}
	
	for(cur_thicc = 0; cur_thicc<thickness; cur_thicc++){
		
		shadowIntensity = 1.0f-(1.0f*cur_thicc)/thickness;
		
		pixColor.R = color.R*shadowIntensity;
		pixColor.G = color.G*shadowIntensity;
		pixColor.B = color.B*shadowIntensity;
		
	
		//printf("0x%08X\n",deltaIlum);
		
		for(x=left;x<=right;x++){
			pix[x + top * w] = Color_RGBA2DW(pixColor);
			pix[x + bottom * w] = Color_RGBA2DW(pixColor);
		}
		for(y=top;y<=bottom;y++){
			pix[left + y * w] = Color_RGBA2DW(pixColor);
			pix[right + y * w] = Color_RGBA2DW(pixColor);
		}
		
		left++;
		right--;
		top++;
		bottom--;

	}
	
	if(dropShadow){
		for(cur_thicc = 0; cur_thicc<shadowThickness; cur_thicc++){

			shadowIntensity = (1.0f*cur_thicc)/shadowThickness;
			
			//printf("%f\n",shadowIntensity);
			
			for(x=left;x<=right;x++){
				
				pixColor = Color_DW2RGBA(pix[x + top * w]);
				pixColor.R *= shadowIntensity;
				pixColor.G *= shadowIntensity;
				pixColor.B *= shadowIntensity;
				pix[x + top * w] = Color_RGBA2DW(pixColor);
				
				pixColor = Color_DW2RGBA(pix[x + bottom * w]);
				pixColor.R *= shadowIntensity;
				pixColor.G *= shadowIntensity;
				pixColor.B *= shadowIntensity;
				pix[x + bottom * w] = Color_RGBA2DW(pixColor);
			}
			for(y=top;y<=bottom;y++){
				pixColor = Color_DW2RGBA(pix[left + y * w]);
				pixColor.R *= shadowIntensity;
				pixColor.G *= shadowIntensity;
				pixColor.B *= shadowIntensity;
				pix[left + y * w] = Color_RGBA2DW(pixColor);
				
				pixColor = Color_DW2RGBA(pix[right + y * w]);
				pixColor.R *= shadowIntensity;
				pixColor.G *= shadowIntensity;
				pixColor.B *= shadowIntensity;
				pix[right + y * w] = Color_RGBA2DW(pixColor);

			}
			
			left++;
			right--;
			top++;
			bottom--;
		}
	}
	
}


void GenerateMechanicsDome(mechDome_t ledome, uint32_t* pix, uint16_t w, uint16_t h){
	
	uint16_t x,y;
	uint32_t offset;
	float reflection;
	float aspratio = (1.0f * w)/h;
	vec2D_t pixPos = {0}, pixDistance = {0};
	RGBAcol_t pixColor = {0};
	/*
	typedef struct mechDome{
	
		vec2D_t pos;
		float radius;
		float sphericity;
		RGBAcol_t color;
	
	}mechDome_t;
	*/
	
	for(y=0;y<h;y++){
		pixPos.y = (1.0f * y)/h;
		
		for(x=0;x<w;x++){
			
			pixPos.x = (aspratio * x)/w;
			offset = x + y * w;
			
			pixDistance = vec2D_Sub(pixPos,ledome.pos);
			
			if(vec2D_Magnitude(pixDistance) <= ledome.radius){
				
				reflection = 1.0f - ledome.sphericity*pow(vec2D_Magnitude(pixDistance)/ledome.radius,2);
				pix[offset] = Color_RGBA2DW(Color_IntensityScale(ledome.color,reflection));
				
				if(vec2D_Magnitude(pixDistance) <= 0.6f*ledome.radius){
					
					pixColor = Color_DW2RGBA(pix[offset]);
					
					if(vec2D_Magnitude(pixDistance) > 0.55f*ledome.radius)
						pixColor = Color_IntensityScale(pixColor,0.5f);
					
					else if(fabs(pixDistance.x) >= 0.1f*ledome.radius && fabs(pixDistance.y) >= 0.1f*ledome.radius){
						pixColor = Color_IntensityScale(pixColor,1.0f - pow(Color_Clamp(vec2D_Magnitude(pixDistance)/ledome.radius,0.0f,1.0f),2));
					}
					
					pix[offset] = Color_RGBA2DW(pixColor);
					
				}
				
				else if(vec2D_Magnitude(pixDistance) > 0.7f*ledome.radius){
					
					pixColor = Color_DW2RGBA(pix[offset]);
					pixColor = Color_IntensityScale(pixColor,1.0f - pow(Color_Clamp(0.5f*vec2D_Magnitude(pixDistance)/ledome.radius,0.0f,1.0f),2));
					pix[offset] = Color_RGBA2DW(pixColor);
					
				}
				
			}
			else{
				
				pixColor = Color_DW2RGBA(pix[offset]);
				pixColor = Color_IntensityScale(pixColor,Color_Clamp(0.3*vec2D_Magnitude(pixDistance)/ledome.radius,0.0f,1.0f));
				pix[offset] = Color_RGBA2DW(pixColor);
				
			}
			
		}
		
	}
	
	
}


void LinearMotionBlur(float angle, uint8_t maxiterations, uint32_t* pix, uint16_t w, uint16_t h){
	
	RGBAcol_t currentpixCol;
	RGBAcol_t neighbourpixCol; 
	
	uint8_t iterations = 0; //blur iterations
	int8_t i = 0, j = 0; //matrix iterators
	int16_t x = 0, y = 0; //current pixel coordinates
	uint32_t neighbourOffset, boffset; //graphics array offsets
	

	uint32_t* ret = malloc(w*h*sizeof(uint32_t));
	
	if(ret == NULL){
		fprintf(stderr,"Failed to allocate output buffer for blurring operation.\n");
		return;
	}
	
	for(iterations = 0; iterations < maxiterations; iterations++){
		
		for(y=0; y<h; y++){
			for(x=0; x<w; x++){
				
				boffset = x + y*w;
				neighbourOffset = ((uint32_t)lroundf(x + iterations*cos(angle)) % w) + ((uint32_t)lroundf(y + iterations*sin(angle)) % h) * w;
				
				currentpixCol = Color_DW2RGBA(pix[boffset]);
				neighbourpixCol = Color_DW2RGBA(pix[neighbourOffset]);
				
				currentpixCol = Color_IntensityScale(currentpixCol,0.5f);
				
				currentpixCol.R += neighbourpixCol.R*0.5f;
				currentpixCol.G += neighbourpixCol.G*0.5f;
				currentpixCol.B += neighbourpixCol.B*0.5f;
				
				ret[boffset] = Color_RGBA2DW(currentpixCol);
				
			}
		}
		memcpy(pix,ret,w*h*sizeof(uint32_t));
	}
	
	free(ret);
	
}

void BoxBlur(uint8_t iterations, uint32_t* pix, uint16_t w, uint16_t h){
	
	uint32_t* ret = malloc(w*h*sizeof(uint32_t));
	
	if(ret == NULL){
		fprintf(stderr,"Failed to allocate output buffer for blurring operation.\n");
		return;
	}
	
	int16_t iter, i, j, c;
	int16_t x, y;
	int32_t boffset, noffset; 
	
	RGBAcol_t bCol, nCol;
	uint8_t matsize = 3;
	
	float blur = 1.0f;
	float averageval = blur/(matsize*matsize);
	float pixweight = averageval;
	
	for(iter=0;iter<iterations;iter++){
		for(y=0;y<h;y++){
			for(x=0;x<w;x++){
				boffset = x + y*w;
				for(c=0;c<4;c++)
					*((float*)&bCol+c) = 0.0f;
				
				for(i=0;i<matsize;i++){
					for(j=0;j<matsize;j++){
					
						noffset = (((x + i-(matsize>>1))%w) + (((y+((matsize-1)>>1)) + j-(matsize>>1))%h) * w);
						nCol = Color_DW2RGBA(pix[noffset]);
						
						for(c=0;c<4;c++){
							*((float*)&bCol+c) += *((float*)&nCol+c);
							*((float*)&bCol+c) = Color_Clamp( *((float*)&bCol+c), 0.0f, 1.0f);
						}
					}
				}
				ret[boffset] = Color_RGBA2DW(Color_IntensityScale(bCol,averageval));
			}
		}
		
		memcpy(pix,ret,w*h*sizeof(uint32_t));
	}
	free(ret);
	
}

uint32_t* GenerateGlass(RGBAcol_t glassTint, uint16_t w, uint16_t h){
	
	uint32_t* pix = NULL;
	
	RGBAcol_t pixCol = {0};
	
	uint16_t x = 0, y = 0;
	uint32_t offset = 0;
	
	float glassShading = 0.0f;
	
	pix = GenerateTurbulence(w,h,'w',8);
	AddNoise(0.1f,pix,w,h);
	
	Color_RGBATint(pix, w, h, glassTint.R, glassTint.G, glassTint.B);
	
	for(y=0;y<h;y++){
		for(x=0;x<w;x++){
			offset = x + y*w;
			pixCol = Color_DW2RGBA(pix[offset]);
			
			glassShading = 1.0f - 0.6f*Color_Clamp(fabs(1.0f*x - (w>>1))/w + fabs(1.0f*y - (h>>1))/h,0.0f,1.0f);
			
			pixCol = Color_IntensityScale(pixCol,glassShading);
			
			pix[offset] = Color_RGBA2DW(pixCol);
			
		}
	}
	LinearMotionBlur(M_PI/2.0f, 5, pix, w, h);

	return pix;
	
}


//GREEBLES



void RectInnerShadow(intrect_t rect, uint16_t thickness, uint32_t* pix, uint16_t w, uint16_t h){
	
	//for each border iteration until thickness-1:
	//add 1 to leftmost border
	//subtract 1 from rightmost border
	//add 1 to uppermost border
	//subtract 1 from downmost(?) border
	
	//Emulate bevel on last border:
	//on leftmost and uppermost borders increase luminosity
	//on rightmost and downmost borders decrease luminosity
	
	uint16_t cur_thicc = 0;
	uint16_t max_thicc = (w < h ? w : h); // set max thickness as half of the smallest of the two tile dimensions
	
	uint16_t left = rect.x0, right = rect.x1, top = rect.y0, bottom = rect.y1-1; 
	uint16_t x,y;
	
	uint8_t shadowThickness = thickness;
	uint8_t dropShadow = 1;
	RGBAcol_t pixColor;
	float shadowIntensity, shadVal = 0.3f;
	
	
	//printf("0x%08X\n",deltaIlum);


	for(cur_thicc = 0; cur_thicc<shadowThickness; cur_thicc++){

		shadowIntensity = sqrt((1.0f-shadVal) + (shadVal*(cur_thicc))/(shadowThickness));
		
		//printf("%f\n",shadowIntensity);
		
		for(x=left;x<right;x++){
			
			pixColor = Color_DW2RGBA(pix[x + top * w]);
			pixColor.R *= shadowIntensity;
			pixColor.G *= shadowIntensity;
			pixColor.B *= shadowIntensity;
			pix[x + top * w] = Color_RGBA2DW(pixColor);
			
			pixColor = Color_DW2RGBA(pix[x + bottom * w]);
			pixColor.R *= shadowIntensity;
			pixColor.G *= shadowIntensity;
			pixColor.B *= shadowIntensity;
			pix[x + bottom * w] = Color_RGBA2DW(pixColor);
			
		}
		for(y=top;y<=bottom;y++){
			pixColor = Color_DW2RGBA(pix[left + y * w]);
			pixColor.R *= shadowIntensity;
			pixColor.G *= shadowIntensity;
			pixColor.B *= shadowIntensity;
			pix[left + y * w] = Color_RGBA2DW(pixColor);
			
			pixColor = Color_DW2RGBA(pix[right + y * w]);
			pixColor.R *= shadowIntensity;
			pixColor.G *= shadowIntensity;
			pixColor.B *= shadowIntensity;
			pix[right + y * w] = Color_RGBA2DW(pixColor);

		}
		
		left++;
		right--;
		top++;
		bottom--;
	}
	
	
}



void DrawGreebleElement(intrect_t panel, uint32_t greebleColor, uint32_t* pix, uint16_t w, uint16_t h){
	
	DrawRect(panel,greebleColor,pix,w);
	RectInnerShadow(panel, (panel.x1-panel.x0)>>2, pix, w, h);
	
}

uint32_t* GenerateGreebles(uint16_t ndivisions, uint16_t w, uint16_t h){
	
	uint32_t* pix = NULL;
	
	uint8_t* greebleGrid = calloc(ndivisions*ndivisions,sizeof(uint8_t));
	
	if(greebleGrid == NULL){
		fprintf(stderr,"Failed to generate greeble grid\n");
		return pix;
		
	}
	
	uint32_t greebleColor = 0x00000000;
	uint8_t shade = 0x00, isdoubleW = 0x00, isdoubleH = 0x00;
			
	uint16_t dw = w/ndivisions, dh = h/ndivisions, i = 0, j = 0, ggi = 0;
	
	uint8_t doubleprob = 0;
	
	intrect_t panel = {0, 0, dw,dh};
	
	pix = GenerateBlank(w,h,0xFF808080);
	
	if(pix == NULL){
		fprintf(stderr,"ERROR ALLOC: greebles\n");
		return pix;
	}
	
	for(j=0;j<ndivisions;j++){
		
		for(i=0;i<ndivisions;i++){
			
			ggi = i + j*ndivisions;
			
			if(greebleGrid[ggi] == 0xFF) //is this space already taken?
				continue; //oh.. ok...
				
			else
				greebleGrid[ggi] = 0xFF; //no? well, it's mine now!
			
			doubleprob = rand() % 0xFF;
			isdoubleW = 0x00;
			isdoubleH = 0x00;
			
			if(doubleprob > 0x55 && doubleprob <= 0xAA){
				if((i+1)*dw<w){ //check if index is within range
					if(greebleGrid[i+1 + j*ndivisions] == 0x00){ //check if next grid space is clear
						isdoubleW = 0x01; //cleared to draw double sized greeble
						greebleGrid[i+1 + j*ndivisions] = 0xFF; //set grid space to occupied
						
					}
				}
			}
			else if(doubleprob>0xAA){
				if((j+1)*dh<h){ 
					if(greebleGrid[i + (j+1)*ndivisions] == 0x00){
						isdoubleH = 0x01;
						greebleGrid[i + (j+1)*ndivisions] = 0xFF;

					}
				}
			}
			
			shade = 150 + rand() % 50;
			greebleColor = 0xFF000000 | (shade<<16) | (shade<<8) | shade; 
				
			panel.x0 = i*dw;
			panel.x1 = panel.x0 + (dw<<isdoubleW);
			panel.y0 = j*dh;
			panel.y1 = panel.y0 + (dh<<isdoubleH);
			
			DrawGreebleElement(panel,greebleColor,pix,w,h);
			
			if(isdoubleW)
				i++;	
		}
	}
	
	
	free(greebleGrid);
	
	return pix;
}
