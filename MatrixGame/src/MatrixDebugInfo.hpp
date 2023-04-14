// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIXDEBUGINFO_INCLUDE
#define MATRIXDEBUGINFO_INCLUDE

#include "CMain.hpp"

#include "d3d9.h"
#include "d3dx9tex.h"

#include <string>

#define MAX_DEBUG_INFO_ITEMS 128

#define DI_DRAWFPS       SETBIT(0)
#define DI_VISOBJ        SETBIT(1)
#define DI_TMEM          SETBIT(2)
#define DI_TARGETCOORD   SETBIT(3)
#define DI_SIDEINFO      SETBIT(4)
#define DI_ACTIVESOUNDS  SETBIT(5)
#define DI_FRUSTUMCENTER SETBIT(6)

struct SDIItem {
    std::wstring* key;
    std::wstring* val;
    int ttl;
    int bttl;
};

class CMatrixDebugInfo : public CMain {
    SDIItem m_Items[MAX_DEBUG_INFO_ITEMS];
    int m_ItemsCnt;
    ID3DXFont *m_Font;

    CPoint m_Pos;

public:
    CMatrixDebugInfo(void);
    ~CMatrixDebugInfo(void) { Clear(); };

    void Clear(void);
    void Draw(void);
    void Takt(int ms);

    void SetStartPos(const CPoint &pos) { m_Pos = pos; }

    void T(const wchar *key, const wchar *val, int ttl = 3000, int bttl = 0, bool add = false);

    void InitFont(void);
    void ClearFont(void) {
        if (m_Font) {
            m_Font->Release();
            m_Font = NULL;
        }
    };

    void OnLostDevice(void) {
        if (m_Font)
            m_Font->OnLostDevice();
    }
    void OnResetDevice(void) {
        if (m_Font)
            m_Font->OnResetDevice();
    }
};

#endif