// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

// selection
#define SEL_COLOR_CHANGE_TIME 200
#define SEL_SPEED             0.1f
#define SEL_BLUR_CNT          1
#define SEL_SIZE              1.5f
#define SEL_PP_DIST           5.5f

//___________________________________________________________________________________________________
/////////////////////////////////////////////////////////////////////////////////////////////////////

struct SSelPoint {
    CBillboard m_Blur[SEL_BLUR_CNT];
    D3DXVECTOR3 m_Pos[SEL_BLUR_CNT];
    // D3DXVECTOR3 m_Pos;

    ~SSelPoint() {
        for (int i = 0; i < SEL_BLUR_CNT; ++i)
            m_Blur[i].Release();
    }
};

class CMatrixEffectSelection : public CMatrixEffect {
    D3DXVECTOR3 m_Pos;
    float m_Radius;
    float m_InitRadius;

    DWORD m_Color_current;
    DWORD m_ColorTo;    // to
    DWORD m_ColorFrom;  // from
    float m_ColorTime;

    SSelPoint *m_Points;
    int m_SelCnt;

    CMatrixEffectSelection(const D3DXVECTOR3 &pos, float r, DWORD color);
    virtual ~CMatrixEffectSelection();

    void UpdatePositions(void);

public:
    friend class CMatrixEffect;

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);

    virtual int Priority(void) { return MAX_EFFECT_PRIORITY; };

    void SetPos(const D3DXVECTOR3 &pos) {
        m_Pos = pos;
        UpdatePositions();
    }
    void SetRadius(float r) { m_Radius = r; }

    void Kill(void) { SETFLAG(m_Flags, SELF_KIP); }

    void SetColor(DWORD color) {
        m_ColorFrom = m_Color_current;
        m_ColorTo = color;
        m_ColorTime = SEL_COLOR_CHANGE_TIME;
    }

    void SetAlpha(BYTE a) {
        m_ColorFrom = m_Color_current;
        m_ColorTo = (m_Color_current & 0x00FFFFFF) | (a << 24);
        m_ColorTime = SEL_COLOR_CHANGE_TIME;
    }
};
