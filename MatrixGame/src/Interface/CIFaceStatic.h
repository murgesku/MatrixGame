// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "Interface.h"
#include "CIFaceElement.h"

////////////////////////////////////////////////////////
//Static Image Class

class CIFaceStatic : public CIFaceElement
{
public:

	bool OnMouseLBDown();
    void OnMouseLBUp();
	bool OnMouseMove(CPoint);
    bool OnMouseRBDown();

    CIFaceStatic();
	~CIFaceStatic();
};