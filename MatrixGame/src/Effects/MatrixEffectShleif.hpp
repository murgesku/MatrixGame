// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_EFFECT_SHLEIF
#define MATRIX_EFFECT_SHLEIF

#define SHLEIF_MAX_SMOKES 64
#define SHLEIF_TTL        2000

#include "MatrixEffect.hpp"

class CMatrixEffectShleif : public CMatrixEffect {
    SEffectHandler *m_Smokes;
    int m_SmokesCnt;

    CMatrixEffectShleif(void);
    virtual ~CMatrixEffectShleif();

    float m_TTL;

public:
    friend class CMatrixEffect;

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Takt(float);
    virtual void Release(void);
    virtual int Priority(void) { return 30; };

    void AddSmoke(const D3DXVECTOR3 &pos, float ttl, float puffttl, float spawntime, DWORD color, bool intense,
                  float speed);
    void AddFire(const D3DXVECTOR3 &pos, float ttl, float puffttl, float spawntime, float dispfactor, bool intense,
                 float speed);

    // void AddFlame(const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &speed, float ttl, DWORD
    // hitmask, CMatrixMapStatic * skip, DWORD user, FIRE_END_HANDLER handler);

    void SetTTL(float ttl) { m_TTL = ttl; }
};

#endif
