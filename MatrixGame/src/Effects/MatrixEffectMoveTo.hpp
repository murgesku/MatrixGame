// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_EFFECT_MOVETO_INCLUDE
#define MATRIX_EFFECT_MOVETO_INCLUDE

class CMatrixEffectMoveto;
struct SPointMoveTo {
    D3DXMATRIX m;

    void Init(CMatrixEffectMoveto *host, int i);
    void Change(CMatrixEffectMoveto *host, int i, float k);
    void Draw(CMatrixEffectMoveto *host);
};

class CMatrixEffectMoveto : public CMatrixEffect {
    D3DXVECTOR3 m_Pos;

    SPointMoveTo m_Pts[6];
    float m_TTL;

    static CTextureManaged *m_Tex;
    static int m_RefCnt;

    CMatrixEffectMoveto(const D3DXVECTOR3 &center);
    virtual ~CMatrixEffectMoveto();

    static void StaticInit(void) {
        m_Tex = NULL;
        m_RefCnt = 0;
    }

public:
    friend class CMatrixEffect;
    friend struct SPointMoveTo;

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Takt(float);
    virtual void Release(void);
    virtual int Priority(void) { return MAX_EFFECT_PRIORITY; };
};

#endif