// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "BaseDef.hpp"

#include <d3dx9core.h>

#include <string>
#include <vector>

#define DI_DRAWFPS       SETBIT(0)
#define DI_VISOBJ        SETBIT(1)
#define DI_TMEM          SETBIT(2)
#define DI_TARGETCOORD   SETBIT(3)
#define DI_SIDEINFO      SETBIT(4)
#define DI_ACTIVESOUNDS  SETBIT(5)
#define DI_FRUSTUMCENTER SETBIT(6)

struct SDIItem {
    std::wstring key;
    std::wstring val;
    int ttl;
    int bttl;
};

class CMatrixDebugInfo
{
    std::vector<SDIItem> m_Items;
    ID3DXFont* m_Font;

    CPoint m_Pos;

public:
    CMatrixDebugInfo();
    ~CMatrixDebugInfo();

    void Draw();
    void Takt(int ms);

    void SetStartPos(const CPoint& pos);

    void T(const wchar_t* key, const wchar_t* val, int ttl = 3000, int bttl = 0, bool add = false);

    void OnLostDevice();
    void OnResetDevice();
};