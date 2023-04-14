// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_TER_SURFACE_INCLUDE
#define MATRIX_TER_SURFACE_INCLUDE

#include "BigVB.hpp"
#include "BigIB.hpp"

#define SURFF_MACRO SETBIT(0)
#define SURFF_GLOSS SETBIT(1)
#define SURFF_WHITE SETBIT(2)

#define SURFF_WRAPY SETBIT(3)

#define SURF_TYPES_COUNT 8

#define SURF_TYPE                   (0)
#define SURF_TYPE_MACRO             (SURFF_MACRO)
#define SURF_TYPE_GLOSS             (SURFF_GLOSS)
#define SURF_TYPE_MACRO_GLOSS       (SURFF_MACRO | SURFF_GLOSS)
#define SURF_TYPE_WHITE             (SURFF_WHITE)
#define SURF_TYPE_MACRO_WHITE       (SURFF_MACRO | SURFF_WHITE)
#define SURF_TYPE_GLOSS_WHITE       (SURFF_GLOSS | SURFF_WHITE)
#define SURF_TYPE_MACRO_GLOSS_WHITE (SURFF_MACRO | SURFF_GLOSS | SURFF_WHITE)
#define SURF_FLAG_MASK              (SURFF_MACRO | SURFF_GLOSS | SURFF_WHITE)

class CTerSurface : public Base::CMain {
    struct STerSurfVertex {
        D3DXVECTOR3 p;
        D3DXVECTOR3 n;
        DWORD color;
        float tu;
        float tv;

        static const DWORD FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1);
    };
    struct STerSurfVertexM {
        D3DXVECTOR3 p;
        D3DXVECTOR3 n;
        DWORD color;
        float tu;
        float tv;
        float tum;
        float tvm;

        static const DWORD FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2);
    };

    DWORD m_Flags;
    DWORD m_Color;
    CTextureManaged *m_Tex;
    CTextureManaged *m_TexGloss;

    int m_DrawFrameMarker;
    int m_Index;  // draw order

    static CBigIB *m_BigIB;
    static CBigVB<STerSurfVertex> *m_BigVB;
    static CBigVB<STerSurfVertexM> *m_BigVBM;

    // union {
        SBigVBSource<STerSurfVertex> m_VertsSource;
        SBigVBSource<STerSurfVertexM> m_VertsSourceM;
    // };
    SBigIBSource m_IdxsSource;

    static CTerSurface *m_Surfaces;
    static int m_SurfacesCnt;

    static CTerSurface **m_SurfacesDraw;
    static int m_SurfaceLeft;
    static int m_SurfaceRite;

    float m_DispX;
    float m_DispY;
    // private draw! use DrawAll. its draw all surfaces BeforeDraw was called.
    void Draw(void);

public:
    static void StaticInit(void) {
        m_BigIB = NULL;
        m_BigVB = NULL;
        m_BigVBM = NULL;
        m_Surfaces = NULL;
        m_SurfacesCnt = 0;
    }
    //#ifdef _DEBUG
    //
    //    static int GetSufracesCount(void) {return m_SurfacesCnt;}
    //    static CTerSurface *GetSufrace(int i) {return m_Surfaces+i;}
    //    DWORD unstable;
    //
    //#endif

    static bool IsSurfacesPresent(void) { return m_SurfacesCnt != 0; };
    static void AllocSurfaces(int n);
    // static void PrepareSurfaces(bool macro);
    static void ClearSurfaces(void);

    static void Load(int i, BYTE *raw) { m_Surfaces[i].Load(raw); };
    static void LoadM(int i, BYTE *raw) { m_Surfaces[i].LoadM(raw); };

    static void BeforeDrawAll(void) {
        if (m_BigVB)
            m_BigVB->BeforeDraw();
        if (m_BigVBM)
            m_BigVBM->BeforeDraw();
        m_BigIB->BeforeDraw();

        m_SurfaceLeft = m_SurfacesCnt >> 1;
        m_SurfaceRite = m_SurfacesCnt >> 1;
    }

    static void MarkAllBuffersNoNeed(void) {
        if (m_BigVB)
            m_BigVB->ReleaseBuffers();
        if (m_BigVBM)
            m_BigVBM->ReleaseBuffers();
        if (m_BigIB)
            m_BigIB->ReleaseBuffers();
    }

    static void DrawAll(void);
    static void SetFVF(void);
    static void SetFVFM(void);

    CTerSurface(void);
    ~CTerSurface(void);

    void BeforeDraw(void);

    void Load(BYTE *raw);
    void LoadM(BYTE *raw);
};

#endif