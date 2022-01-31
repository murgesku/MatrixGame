// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "Interface.h"
#include "CIFaceElement.h"

//////////////////////////////////////////////////
//Button Class
class CIFaceButton : public  CIFaceElement
{
public:

	bool OnMouseMove(CPoint);
	void OnMouseLBUp();
	bool OnMouseLBDown();
	void OnMouseRBUp();
    bool OnMouseRBDown();

	CIFaceButton();
	~CIFaceButton();
};
