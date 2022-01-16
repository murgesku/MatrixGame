// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

// konus

#define KONUS_FVF (D3DFVF_XYZ | D3DFVF_TEX1)
struct SKonusVertex {
    D3DXVECTOR3 p;  // Vertex position
    float tu, tv;   // Vertex texture coordinates
};

#define KONUS_NUM_SECTORS 7

class CMatrixEffectKonus : public CMatrixEffect {
protected:
    CTextureManaged *m_Tex;
    DWORD m_Color;  // diffuse color
    float m_TTL;
    float m_Radius;
    float m_Height;
    float m_Angle;
    DWORD m_Intense;

    float _m_TTL;

    D3DXVECTOR3 m_Pos;
    D3DXVECTOR3 m_Dir;
    D3DXMATRIX m_Mat;

    static D3D_VB m_VB;
    static int m_VB_ref;

    CMatrixEffectKonus(const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir, float radius, float height, float angle,
                       float ttl, bool intense, CTextureManaged *tex);
    virtual ~CMatrixEffectKonus();

    void UpdateMatrix(void);

    static bool PrepareDX(void);
    static void StaticInit(void) {
        m_VB = NULL;
        m_VB_ref = 0;
    }

    static void MarkAllBuffersNoNeed(void) {
        if (IS_VB(m_VB)) {
            DESTROY_VB(m_VB);
        }
    }

public:
    friend class CMatrixEffect;
    friend class CVolcano;

    virtual void BeforeDraw(void) {
        DTRACE();
        PrepareDX();
        m_Tex->Preload();
    };
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);

    virtual int Priority(void) { return 10; };

    void Modify(const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir) {
        DTRACE();
        m_Pos = pos;
        m_Dir = dir;
        UpdateMatrix();
    }
    void Modify(const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, float radius, float height, float angle) {
        DTRACE();
        m_Pos = pos;
        m_Dir = dir;
        m_Radius = radius;
        m_Height = height;
        m_Angle = angle;
        UpdateMatrix();
    }
};

class CMatrixEffectKonusSplash : public CMatrixEffectKonus {
    float m_HeightInit;
    float m_RadiusInit;
    D3DXVECTOR3 m_PosInit;

public:
    CMatrixEffectKonusSplash(const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir, float radius, float height, float angle,
                             float ttl, bool intense, CTextureManaged *tex)
      : CMatrixEffectKonus(start, dir, radius, height, angle, ttl, intense, tex), m_HeightInit(height),
        m_RadiusInit(radius), m_PosInit(start) {
        m_EffectType = EFFECT_SPLASH;
        DTRACE();
        Takt(0);
    }
    virtual void Takt(float step);

    virtual int Priority(void) { return 0; };
};
