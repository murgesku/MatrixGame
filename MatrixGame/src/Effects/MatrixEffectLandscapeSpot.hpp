// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

class CMatrixEffectLandscapeSpot;

void SpotTaktConstant(CMatrixEffectLandscapeSpot *spot, float takt);
void SpotTaktAlways(CMatrixEffectLandscapeSpot *spot, float takt);
void SpotTaktPlasmaHit(CMatrixEffectLandscapeSpot *spot, float takt);
void SpotTaktMoveTo(CMatrixEffectLandscapeSpot *spot, float takt);
void SpotTaktPointlight(CMatrixEffectLandscapeSpot *spot, float takt);
void SpotTaktVoronka(CMatrixEffectLandscapeSpot *spot, float takt);

#define LANDSCAPESPOT_FVF (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)
typedef struct {
    D3DXVECTOR3 p;  // Vertex position
    DWORD color;    // Vertex color
    float tu, tv;   // Vertex texture coordinates
} SLandscapeSpotVertex;

#define SPOT_ALTITUDE (0.7f)
#define SPOT_SIZE     (5.0f)

class CMatrixEffectLandscapeSpot : public CMatrixEffect {
    static CMatrixEffectLandscapeSpot *m_First;
    static CMatrixEffectLandscapeSpot *m_Last;

    CMatrixEffectLandscapeSpot *m_Next;
    CMatrixEffectLandscapeSpot *m_Prev;

    D3D_VB m_VB;
    D3D_IB m_IB;

    float m_DX;
    float m_DY;

    SSpotProperties *m_Props;
    CTextureManaged *m_Texture;

    float m_LifeTime;

    int m_CntVerts;
    int m_CntTris;

    BYTE *m_IndsPre;
    BYTE *m_VertsPre;

    DWORD m_Color;

    void BuildLand(const D3DXVECTOR2 &pos, float angle, float scalex, float scaley, float addz, bool scale_by_normal);

    CMatrixEffectLandscapeSpot(const D3DXVECTOR2 &pos, float angle, float scale, ESpotType type = SPOT_CONSTANT);
    virtual ~CMatrixEffectLandscapeSpot();
    void DrawActual(void);

#ifdef _DEBUG
    const char *_file;
    int _line;
#endif

    bool PrepareDX(void);
    static void StaticInit(void) {
        m_First = NULL;
        m_Last = NULL;
    }

    static void MarkAllBuffersNoNeed(void);

public:
    friend class CMatrixEffect;
    friend void SpotTaktConstant(CMatrixEffectLandscapeSpot *spot, float takt);
    friend void SpotTaktAlways(CMatrixEffectLandscapeSpot *spot, float takt);
    friend void SpotTaktPlasmaHit(CMatrixEffectLandscapeSpot *spot, float takt);
    friend void SpotTaktMoveTo(CMatrixEffectLandscapeSpot *spot, float takt);
    friend void SpotTaktPointlight(CMatrixEffectLandscapeSpot *spot, float takt);
    friend void SpotTaktVoronka(CMatrixEffectLandscapeSpot *spot, float takt);

    virtual void BeforeDraw(void) {
        DTRACE();
        PrepareDX();
        m_Texture->Preload();
    };
    virtual void Draw(void){};
    virtual void Takt(float step);
    virtual void Release(void);

#ifdef _DEBUG
    void BeforeRelease(const char *file, int line) {
        _file = file;
        _line = line;
    };
#endif

    virtual int Priority(void);

    static void DrawAll(void);

    void SetColor(DWORD color) { m_Color = color; }
    DWORD GetColor(void) const { return m_Color; }

#ifdef DEAD_CLASS_SPY_ENABLE
    int DCS_UnderSpy(void) const { return sizeof(CMatrixEffectLandscapeSpot); }
#endif
};
