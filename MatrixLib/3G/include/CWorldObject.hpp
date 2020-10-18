// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef CWORLDOBJECT_H
#define CWORLDOBJECT_H

#include "d3dx9.h"

#include "../../Base/include/CList.hpp"

class CCoordSystem;

class CWorldObject : public CMain , public CListInterface<CWorldObject>
{
    CCoordSystem *_owner;   // координатная система - владелец

public:
    CWorldObject(CCoordSystem * cs):
    _owner(cs)
    {
    }

    virtual void Render(void) = 0;

};


#endif