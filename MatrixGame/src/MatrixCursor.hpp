// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixMap.hpp"

#define CURSOR_VISIBLE     1
#define CURSOR_REVERSEANIM 2

class CMatrixCursor : public CMain {
    DWORD m_CursorFlags;
    CPoint m_Pos;
    CPoint m_HotSpot;
    std::wstring m_CurCursor;
    CTextureManaged *m_CursorTexture;
    int m_CursorSize;
    HICON *m_CursorIcons;
    HCURSOR m_OldCursor;
    int m_FramesCnt;
    int m_FrameInc;
    int m_Frame;
    int m_NextCursorTime;
    int m_CursorTimePeriod;
    int m_CursorInTexLine;
    float m_TexSizeXInversed, m_TexSizeYInversed;
    float m_u0, m_v0, m_u1, m_v1;
    void CalcUV(void);

    int m_Time;

public:
    CMatrixCursor(void) {
        memset(this, 0, sizeof(CMatrixCursor));
        m_OldCursor = SetCursor(NULL);
    }
    ~CMatrixCursor(void) { Clear(); }

    void Draw(void);

    void Select(const std::wstring& name);
    void Takt(int ms);
    void Clear(void);
    void SetVisible(bool flag);

    const CPoint &GetPos(void) const { return m_Pos; }
    int GetPosX(void) const { return m_Pos.x; }
    int GetPosY(void) const { return m_Pos.y; }
    void SetPos(const CPoint &pos) { SetPos(pos.x, pos.y); }
    void SetPos(int xx, int yy) {
        m_Pos.x = xx;
        m_Pos.y = yy;
    }
};
