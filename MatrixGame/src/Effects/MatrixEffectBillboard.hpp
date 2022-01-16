// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

// billboard

#define BBOARD_FVF (D3DFVF_XYZ | D3DFVF_TEX1)
struct SBBoardVertex {
    D3DXVECTOR3 p;  // Vertex position
    float tu, tv;   // Vertex texture coordinates
};

class CMatrixEffectBillboard : public CMatrixEffect {
protected:
    CTextureManaged *m_Tex;
    DWORD m_Color;
    DWORD m_Color1;
    DWORD m_Color2;
    float m_TTL;
    float m_TTL2;
    float m_Delay;
    float _m_TTL;
    float m_Radius1;
    float m_Radius2;

    D3DXMATRIX m_Mat;
    D3DXVECTOR3 m_Dir;

    ADD_TAKT m_AddTaktHandler;

    static D3D_VB m_VB;
    static int m_VB_ref;

    CMatrixEffectBillboard(void){};  // does nothing
    CMatrixEffectBillboard(const D3DXVECTOR3 &pos, float radius1, float radius2, DWORD c1, DWORD c2, float ttl,
                           float delay, const wchar *tex, const D3DXVECTOR3 &dir, ADD_TAKT addtakt = NULL);
    virtual ~CMatrixEffectBillboard();

    void UpdateData(void);

    void SetColor(DWORD color1, DWORD color2) {
        m_Color1 = color1;
        m_Color2 = color2;
        UpdateData();
    }
    void Takt2(float step);

    static void CreateGeometry(void);
    static void ReleaseGeometry(void);

    static bool PrepareDX(void);
    static void StaticInit(void) {
        m_VB = NULL;
        m_VB_ref = 0;
    }

    static void MarkAllBuffersNoNeed(void) {
        if (m_VB)
            DESTROY_VB(m_VB);
    }

public:
    static void Draw(const D3DXMATRIX &m, DWORD color, CTextureManaged *tex, bool intense);

    DWORD m_Intense;

    void SetColor(DWORD color) { m_Color = color; }

    void SetPos(const D3DXVECTOR3 &pos) {
        m_Mat._41 = pos.x;
        m_Mat._42 = pos.y;
        m_Mat._43 = pos.z;
    }

    friend class CMatrixEffect;
    friend class CMatrixEffectZahvat;
    friend class CMatrixEffectDust;
    friend class CMatrixEffectMoveto;

    virtual void BeforeDraw(void) {
        DTRACE();
        PrepareDX();
        m_Tex->Preload();
    };
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);

    virtual int Priority(void);
};

class CMatrixEffectBillboardLine : public CMatrixEffect {
protected:
    DWORD m_Color1;
    DWORD m_Color2;
    float _m_TTL;
    float m_TTL;

    CBillboardLine m_Bl;

    // bool    zzz;

    CMatrixEffectBillboardLine(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float width, DWORD c1, DWORD c2,
                               float ttl, CTextureManaged *tex);
    virtual ~CMatrixEffectBillboardLine();

public:
    friend class CMatrixEffect;

    virtual void BeforeDraw(void){};
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);

    virtual int Priority(void);
};

#define BBS_TTL     2000
#define BBS_FADE    0.7f
#define BBS_SPEED   0.02f
#define BBS_DX      6
#define BBS_ICON_DX 12

class CMatrixEffectBillboardScore : public CMatrixEffect {
    struct SBB {
        CBillboard bb;
        float disp;
    };

    SBB *m_bbs;
    int m_bbs_count;

    D3DXVECTOR3 m_Pos;
    float m_TTL;
    DWORD m_Color;

    CMatrixEffectBillboardScore(const wchar *n, const D3DXVECTOR3 &pos, DWORD color);

public:
    ~CMatrixEffectBillboardScore();

    friend class CMatrixEffect;

    virtual void BeforeDraw(void){};
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);

    virtual int Priority(void) { return MAX_EFFECT_PRIORITY; };
};