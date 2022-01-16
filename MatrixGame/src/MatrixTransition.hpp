// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_TRANSITION_INCLUDE
#define MATRIX_TRANSITION_INCLUDE

#include "MatrixInstantDraw.hpp"

class CTransition : public CMain {
    struct SGeom {
        D3DXVECTOR2 pos;
        D3DXVECTOR2 dir;
        SVert_V4_UV verts[4];
    };

    CTextureManaged *m_Tex;
    SGeom *m_Geom;
    int m_GeomCnt;

    int m_ScreenX;
    int m_ScreenY;

public:
    CTransition(void) : CMain(), m_Tex(NULL), m_Geom(NULL), m_GeomCnt(0) {}
    ~CTransition() { Clear(); }

    void Clear(void);
    void BuildTexture(void);
    void RenderToPrimaryScreen(void);
    void Render(void);
    void Takt(int ms);
};

#endif