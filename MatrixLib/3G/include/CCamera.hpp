// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef CCAMERA_H
#define CCAMERA_H

#include "d3dx9.h"
#include "CCoordSystem.hpp"

// camera
class CCamera : public CWorldObject
{


    D3DXVECTOR3    _pos;
    D3DXQUATERNION _rot;

    D3DXMATRIX   _view;
    D3DXMATRIX   _proj;

public:

    CCamera(CCoordSystem * cs):
    CWorldObject(cs)
    {
    }

    virtual void Render(void)
    {
        //SetTransform
    }
    
    

};


#endif