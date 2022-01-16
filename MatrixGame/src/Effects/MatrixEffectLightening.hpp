// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

// lightening

#define LIGHTENING_SEGMENT_LENGTH 30
#define LIGHTENING_WIDTH          10

#define SHORTED_SEGMENT_LENGTH 5
#define SHORTED_WIDTH          5

class CMatrixEffectLightening : public CMatrixEffect {
    D3DXVECTOR3 m_Pos0;
    D3DXVECTOR3 m_Pos1;
    D3DXVECTOR3 m_Dir;

    CMatrixEffectLightening(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float ttl, float dispers, float width,
                            DWORD color, bool bp = true);
    virtual ~CMatrixEffectLightening();

    CBillboardLine *m_BL;
    int m_BL_cnt;

    CBillboard *m_End0;

    float m_SegLen;

    virtual void UpdateData(void);

    float m_TTL;

    float m_Dispersion;
    float m_Width;
    DWORD m_Color;

    D3DXVECTOR3 m_Perp;

public:
    friend class CMatrixEffect;
    friend class CMatrixEffectShorted;

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);

    virtual int Priority(void) { return 10; };

    void SetPos(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1);
};

class CMatrixEffectShorted : public CMatrixEffectLightening {
    CMatrixEffectShorted(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float ttl, DWORD color);

    float m_Len;
    float _m_BL_cnt;
    float _m_TTL;

    virtual void UpdateData(void);

public:
    friend class CMatrixEffect;

    virtual int Priority(void) { return 1; };

    void SetPos(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1);
};

typedef CMatrixEffectShorted *PCMatrixEffectShorted;