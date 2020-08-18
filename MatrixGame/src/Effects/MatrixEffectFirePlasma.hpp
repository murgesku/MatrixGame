// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once


// fireplasma

#define FIRE_PLASMA_W 3
#define FIRE_PLASMA_L 7



class CMatrixEffectFirePlasma : public CMatrixEffect
{

    DWORD               m_UserData;
    FIRE_END_HANDLER    m_Handler;
    DWORD               m_HitMask;
    CMatrixMapStatic *  m_Skip;

    float m_Speed;
    float m_Prevdist;
    D3DXVECTOR3         m_Pos;
    D3DXVECTOR3         m_Dir;
    D3DXVECTOR3         m_End;

    SEffectHandler      m_Light;

    CBillboardLine   m_BBLine;
    CBillboard          m_BB1;
    CBillboard          m_BB2;
    CBillboard          m_BB3;
    CBillboard          m_BB4;


    CMatrixEffectFirePlasma(const D3DXVECTOR3 &start, const D3DXVECTOR3 &end, float speed, DWORD hitmask, CMatrixMapStatic * skip, FIRE_END_HANDLER handler, DWORD user);
	virtual ~CMatrixEffectFirePlasma();

public:
    friend class CMatrixEffect;
    friend class CPolygon;

    virtual void BeforeDraw(void) {}
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);

    virtual int  Priority(void) {return MAX_EFFECT_PRIORITY;};
};


