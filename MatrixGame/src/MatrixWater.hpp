// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "Common.hpp"
#include "D3DControl.hpp"

#define INSHORE_FVF (D3DFVF_XYZ | D3DFVF_TEX1)
struct SInshoreVertex {
    D3DXVECTOR3 p;
    float tu, tv;
};

#define INSHORE_SPEED (0.0005f)

#define INSHOREWAVES_CNT 7
struct SInshorewave {
    static CTextureManaged *m_Tex;
    static D3D_VB m_VB;
    static int m_VB_ref;

    static void Create(int i, const D3DXVECTOR2 &pos, const D3DXVECTOR2 &dir, SInshorewave &iw);

    static void DrawBegin(void);
    static void DrawEnd(void);

    static void StaticInit(void) {
        m_Tex = NULL;
        m_VB = NULL;
        m_VB_ref = 0;
    }

    static bool PrepareVB(void);
    static void MarkAllBuffersNoNeed(void) {
        if (m_VB)
            DESTROY_VB(m_VB);
    };

    void Release(void);
    void Draw(void);

    int m_Index;

    D3DXVECTOR2 pos;
    D3DXVECTOR2 dir;
    float len;
    float t;
    float speed;
    float scale;
};

//#define MATRIX_WATER_VERTEX_FORMAT (D3DFVF_XYZ|D3DFVF_NORMAL/*|D3DFVF_DIFFUSE*/|D3DFVF_TEX3)
#define MATRIX_WATER_VERTEX_FORMAT (D3DFVF_XYZ | D3DFVF_NORMAL /*|D3DFVF_DIFFUSE*/ | D3DFVF_TEX2)
//#define MATRIX_WATER_VERTEX_FORMAT (D3DFVF_XYZ)
struct SMatrixMapWaterVertex {
    D3DXVECTOR3 v;
    D3DXVECTOR3 n;
    float au, av;
    float tu, tv;
    // float mu,mv;
};

class CMatrixWater : public CMain {
    float h[WATER_SIZE * WATER_SIZE];
    float r[WATER_SIZE * WATER_SIZE];
    int f[WATER_SIZE * WATER_SIZE];

    int m_angle;
    int m_next_time;

public:
    D3D_VB m_VB;
    D3D_IB m_IB;
    CTextureManaged *m_WaterTex1;
    CTextureManaged *m_WaterTex2;

    CMatrixWater(void);
    ~CMatrixWater();

    void Clear(void);
    void Init(void);

    bool IsReadyForDraw(void) const { return IS_VB(m_VB); }

    void BeforeDraw(void);
    void Draw(const D3DXMATRIX &m);

    void Takt(int step);
    void FillVB(int k);
};
