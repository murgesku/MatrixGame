// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef CVERTEX_H
#define CVERTEX_H

#include "d3dx9.h"

struct SVertex
{
    D3DXVECTOR3 v;
    D3DXVECTOR3 n;
    float tu,tv;
};


#endif