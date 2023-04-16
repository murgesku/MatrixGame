// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixWater.hpp"
#include "CReminder.hpp"

#include "BigVB.hpp"
#include "BigIB.hpp"

//#define TEXTURES_OFF

//#define LANDSCAPE_BOTTOM_USE_NORMALES
#define LANDSCAPE_TOP_USE_NORMALES
#define LANDSCAPE_SURF_USE_NORMALES

typedef struct {
    D3DXVECTOR2 pos;
    D3DXVECTOR2 dir;
    bool used;
} SPreInshorewave;

#ifdef LANDSCAPE_BOTTOM_USE_NORMALES
#define MATRIX_MAP_BOTTOM_VERTEX_FORMAT (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2)
#else
#define MATRIX_MAP_BOTTOM_VERTEX_FORMAT (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2)
#endif

typedef struct {
    float u, v;
} Svtc;

struct SMatrixMapVertexBottomInt {
    D3DXVECTOR3 v;
#ifdef LANDSCAPE_BOTTOM_USE_NORMALES
    D3DXVECTOR3 n;
#endif
    DWORD defcol;
};

struct SMatrixMapVertexBottom {
    SMatrixMapVertexBottomInt ivd;
    Svtc tc[2];

    static const DWORD FVF = MATRIX_MAP_BOTTOM_VERTEX_FORMAT;
};

struct SMapZVertex {
    D3DXVECTOR3 v;
    float weight;

    static const DWORD FVF = D3DFVF_XYZB1;
};

class CMatrixMapStatic;
class CMatrixMapTextureFinal;
class CMatrixMapGroup;
class CTerSurface;
class CMatrixShadowProj;

//#define GRPFLAG_VISIBLE     SETBIT(0)
#define GRPFLAG_HASWATER SETBIT(1)
//#define GRPFLAG_HASFLYER    SETBIT(2)
#define GRPFLAG_HASBASE SETBIT(3)

struct SBottomGeometry {
    int idxbase;  // index base
    int tricnt;   // primitives count
    int vertcnt;  // verts count
    int minvidx;  // min vertex index
    int texture;  // texture index
};

struct SObjectCore;

class CMatrixMapGroup : public CMain {
    SRemindCore m_RemindCore;  // must be at begining of class!!!!!!!!

    // static CBigVB<SMapZVertex>              *m_BigVB_Z;
    static CBigVB<SMatrixMapVertexBottom> *m_BigVB_bottom;
    static CBigIB *m_BigIB_bottom;
    // SBigVBSource<SMapZVertex>               m_VertsSource_Z;
    SBigVBSource<SMatrixMapVertexBottom> m_VertsSource_bottom;
    SBigIBSource m_IdxsSource_bottom;
    SBottomGeometry *m_BottomGeometry;
    int m_BottomGeometryCount;

    int m_PosX, m_PosY;
    D3DXVECTOR2 p0, p1;
    D3DXMATRIX m_Matrix;

    float m_maxz, m_minz;     // maximum and minimum height
    float m_maxz_obj;         // with objects (except robots and flyers)
    float m_maxz_obj_robots;  // same as m_maxz_obj, but with robots

    DWORD m_Flags;

    CTextureManaged *m_WaterAlpha;

    SInshorewave *m_Inshorewaves;
    int m_InshorewavesCnt;
    int m_InshoreTime;
    SPreInshorewave *m_PreInshorewaves;
    int m_PreInshorewavesCnt;

    CMatrixMapStatic **m_Objects;
    int m_ObjectsAllocated;
    int m_ObjectsContained;

    CTerSurface **m_Surfaces;
    int m_SurfacesCnt;

    CMatrixShadowProj **m_Shadows;
    int m_ShadowsCnt;

    float m_CamDistSq;  // квадрат растояния до камеры. вычисляется только в графическом такте.

    SObjectCore *m_RenderShadowObject;

    D3DXVECTOR3 *m_VertsTrace;
    int *m_IdxsTrace;
    int m_IdxsTraceCnt;
    int m_VertsTraceCnt;

public:
#ifdef _DEBUG
    static int m_DPCalls;
#endif

    static void StaticInit(void) {
        // m_BigVB_Z = NULL;
        m_BigVB_bottom = NULL;
        m_BigIB_bottom = NULL;
    }

    CMatrixMapGroup();
    ~CMatrixMapGroup();

    CMatrixMapStatic *GetObject(int i) const;
    int ObjectsCnt(void) const;

    // bool IsVisible(void) const {return FLAG(m_Flags, GRPFLAG_VISIBLE);}
    // void SetVisible(bool vis) {INITFLAG(m_Flags, GRPFLAG_VISIBLE, vis);}

    bool HasWater(void) const { return FLAG(m_Flags, GRPFLAG_HASWATER); }
    // bool HasFlyer(void) const {return FLAG(m_Flags, GRPFLAG_HASFLYER);}
    CTextureManaged *GetWaterAlpha(void) { return m_WaterAlpha; }

    bool IsBaseOn(void) const { return FLAG(m_Flags, GRPFLAG_HASBASE); }

    const D3DXVECTOR2 &GetPos0() const { return p0; }
    const D3DXVECTOR2 &GetPos1() const { return p1; }

    float GetMinZ(void) const { return m_minz; }
    float GetMaxZObj(void) const { return m_maxz_obj; }
    float GetMaxZObjRobots(void) const { return m_maxz_obj_robots; }
    float GetMaxZLand(void) const { return m_maxz; }

    void RecalcMaxZ(void);
    void AddNewZObj(float z) {
        if (z > m_maxz_obj) {
            m_maxz_obj = z;
        }
        if (m_maxz_obj_robots < m_maxz_obj) {
            m_maxz_obj_robots = m_maxz_obj;
        }
    }
    void AddNewZObjRobots(float z) {
        if (z > m_maxz_obj_robots)
            m_maxz_obj_robots = z;
    }

    bool IsInFrustum(void) const;
    inline bool IsPointIn(const D3DXVECTOR2 &p) {
        return (p.x >= p0.x) && (p.x < p1.x) && (p.y >= p0.y) && (p.y < p1.y);
    }

    void AddObject(CMatrixMapStatic *obj);
    void SubObject(CMatrixMapStatic *obj);

    CMatrixMapStatic *FindObjectAny(DWORD mask, const D3DXVECTOR3 &pos, float maxdist, float scale_radius, int &i);
    CMatrixMapStatic *FindObjectAny(DWORD mask, const D3DXVECTOR2 &pos, float maxdist, float scale_radius, int &i);

    void Clear(void);

    void BuildBottom(int x, int y, BYTE *rawbottom);
    void BuildWater(int x, int y);
    void InitInshoreWaves(int n, const float *xx, const float *yy, const float *nxx, const float *nyy);

    void RemoveShadow(const CMatrixShadowProj *s);
    void AddShadow(CMatrixShadowProj *s);
    void DrawShadowProj(void);

    void AddSurface(CTerSurface *surf);

    static void MarkAllBuffersNoNeed(void);

    static void BeforeDrawAll(void) {
        m_BigVB_bottom->BeforeDraw();
        m_BigIB_bottom->BeforeDraw();
    };
    void BeforeDraw(void);
    void Draw(void);
    // void DrawZ(void);

    void DX_Free(void) {
        m_VertsSource_bottom.MarkNoNeed(m_BigVB_bottom);
        m_IdxsSource_bottom.MarkNoNeed(m_BigIB_bottom);
    }
    void DX_Prepare(void) {
        m_VertsSource_bottom.Prepare(m_BigVB_bottom);
        m_IdxsSource_bottom.Prepare(m_BigIB_bottom);
    };
    void BeforeDrawSurfaces(void);

    void DrawInshorewaves(void);
    void SortObjects(const D3DXMATRIX &sort);
    void GraphicTakt(int step);
    void PauseTakt(int step);

    bool Pick(const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, float &t);

#ifdef _DEBUG
    void DrawBBox(void);
#endif
};

typedef CMatrixMapGroup *PCMatrixMapGroup;
