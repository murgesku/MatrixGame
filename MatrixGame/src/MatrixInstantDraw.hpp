// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_INSTANT_DRAW_INCLUDE
#define MATRIX_INSTANT_DRAW_INCLUDE

#include "D3DControl.hpp"

struct SVertBase {};

struct SVert_V3_C_UV : public SVertBase {
    D3DXVECTOR3 p;
    DWORD color;
    float tu, tv;
};

struct SVert_V4_UV : public SVertBase {
    D3DXVECTOR4 p;
    float tu, tv;
};

struct SVert_V4_C : public SVertBase {
    D3DXVECTOR4 p;
    DWORD col;
};

struct SVert_V4 : public SVertBase {
    D3DXVECTOR4 p;
};

enum E_FVF {
    IDFVF_V3_C_UV,
    IDFVF_V4_UV,
    IDFVF_V4_C,
    IDFVF_V4,

    IDFVF_CNT
};

struct SOneSet {
    void *accum;
    int accumcntalloc;
    int accumcnt;
    CBaseTexture *tex;
    DWORD tf;
    DWORD tf_used;
};

struct SFVF_VB {
    DWORD fvf;
    int stride;
    int statistic;
    int statistic_max_tex;
    int cursize;  // size of currently allocated VB
    D3D_VB vb;
    SOneSet *sets;
    int sets_cnt;
    int sets_alloc;
    int disp;
};

class CInstDraw : public CMain {
    static SFVF_VB m_FVFs[IDFVF_CNT];
    static E_FVF m_Current;
    static D3D_IB m_IB;
    static int m_IB_Count;

public:
    static void StaticInit(void) {
        memset(m_FVFs, 0, sizeof(m_FVFs));
        m_FVFs[IDFVF_V3_C_UV].fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
        m_FVFs[IDFVF_V3_C_UV].stride = sizeof(SVert_V3_C_UV);

        m_FVFs[IDFVF_V4_UV].fvf = D3DFVF_XYZRHW | D3DFVF_TEX1;
        m_FVFs[IDFVF_V4_UV].stride = sizeof(SVert_V4_UV);

        m_FVFs[IDFVF_V4_C].fvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
        m_FVFs[IDFVF_V4_C].stride = sizeof(SVert_V4_C);

        m_FVFs[IDFVF_V4].fvf = D3DFVF_XYZRHW;
        m_FVFs[IDFVF_V4].stride = sizeof(SVert_V4);

        m_IB = NULL;
        m_IB_Count = 0;

#ifdef _DEBUG
        m_Current = IDFVF_CNT;
#endif
    }

    static void DrawFrameBegin(void);
    static void BeginDraw(E_FVF fvf);
    static void AddVerts(void *v, CBaseTexture *tex);            // add 4 verts
    static void AddVerts(void *v, CBaseTexture *tex, DWORD tf);  // add 4 verts
    static void ActualDraw(void);

    static void MarkAllBuffersNoNeed(void);

    static void ClearAll(void);
};

#endif