// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#define DUST_BILLS_CNT  5


class CMatrixEffectDust : public CMatrixEffect
{
    CMatrixEffectBillboard  m_BBoards[DUST_BILLS_CNT];
    D3DXVECTOR2             m_Dirs[DUST_BILLS_CNT];
    D3DXVECTOR2             m_AddSpeed;
    float   m_TTL;

    CMatrixEffectDust(const D3DXVECTOR2 &pos,const D3DXVECTOR2 &adddir, float ttl);
	virtual ~CMatrixEffectDust();

public:
    friend class CMatrixEffect;

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);

    virtual int  Priority(void) {return 0;};
};



