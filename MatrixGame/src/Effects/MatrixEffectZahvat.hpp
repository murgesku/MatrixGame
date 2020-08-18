// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once


#define ZAHVAT_SPOT_SIZE        6
#define ZAHVAT_SPOT_GRAY1       0x00ffffff
#define ZAHVAT_SPOT_GRAY2       0xf0808080
#define ZAHVAT_FLASH_PERIOD     3000

class CMatrixEffectZahvat : public CMatrixEffect
{
    int         m_Count;
    CMatrixEffectBillboard *m_BBoards;

    CMatrixEffectZahvat(const D3DXVECTOR3 &pos, float radius, float angle, int cnt);
	virtual ~CMatrixEffectZahvat();

public:
    friend class CMatrixEffect;

    void UpdateData(DWORD color, int count);

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);

    virtual int  Priority(void) {return MAX_EFFECT_PRIORITY;};
};



