#ifndef _RNGTILES_H_
#define _RNGTILES_H_

#include "RNGTEXGEN.h"

RGBAcol_t diffuse1;
RGBAcol_t diffuse2;

void GenerateMasterBuffers(void);
void HueTest(void);
void MakeMechanicsTopTile(void);
void MakeMechanicsBottomTile(void);
void MakeCockpitTile(void);
void MakeIntakeTile(void);
void MakeExhaustTile(void);
void MakeExhaustInnerTile(void);
void MakeFuselageTile(void);

#endif
