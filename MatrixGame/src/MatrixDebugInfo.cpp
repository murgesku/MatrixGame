// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixDebugInfo.hpp"
#include <stdexcept>
#include <cstdint>

#define DI_KEY_X 10
#define DI_KEY_Y 10
#define DI_KEY_W 140
#define DI_KEY_H 14
#define DI_VAL_W 200

#define MAX_DEBUG_INFO_ITEMS 128

extern IDirect3DDevice9* g_D3DD;

CMatrixDebugInfo::CMatrixDebugInfo()
: m_Font{nullptr}
, m_Pos{DI_KEY_X, DI_KEY_Y}
{
    if (D3D_OK != D3DXCreateFontW(g_D3DD, 10, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                  DEFAULT_QUALITY, DEFAULT_PITCH, L"MS Sans Serif", &m_Font))
    {
        throw std::runtime_error("Failed to create font with D3DXCreateFontW()");
    }
}

CMatrixDebugInfo::~CMatrixDebugInfo()
{
    m_Font->Release();
};

void CMatrixDebugInfo::Draw()
{
    try
    {
        int y = m_Pos.y;
        for (auto& item : m_Items)
        {
            if (item.bttl > 0)
            {
                continue;
            }
            CRect r;
            r.left = m_Pos.x;
            r.right = m_Pos.x + DI_KEY_W;
            r.top = y;
            r.bottom = y + DI_KEY_H;

            uint8_t alpha = 255;
            if (item.ttl < 1000)
            {
                alpha = (uint8_t)static_cast<int>(float(item.ttl) * 0.255f);
            }

            uint32_t color = (alpha << 24) | 0xFFFFFF;

            m_Font->DrawTextW(NULL, item.key.c_str(), item.key.length(), (LPRECT)&r,
                              DT_NOCLIP | DT_LEFT | DT_VCENTER | DT_SINGLELINE, color);

            r.left = r.right + 1;
            r.right = r.left + DI_VAL_W;

            m_Font->DrawTextW(NULL, item.val.c_str(), item.val.length(), (LPRECT)&r,
                              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP, color);

            y += DI_KEY_H;
        }
    }
    catch (...)
    {
    }
}

void CMatrixDebugInfo::T(const wchar_t *key, const wchar_t *val, int ttl, int bttl, bool add)
{
    if (!add)
    {
        auto iter = std::ranges::find_if(m_Items, [key](const auto& item) { return item.key == key; });
        if (iter != m_Items.end())
        {
            iter->val = val;
            iter->ttl = ttl;
            iter->bttl = bttl;
            return;
        }
    }

    if (m_Items.size() >= MAX_DEBUG_INFO_ITEMS)
        return;

    m_Items.push_back(SDIItem{key, val, ttl, bttl});
}

void CMatrixDebugInfo::Takt(int ms)
{
    for (auto iter = m_Items.begin(); iter < m_Items.end();)
    {
        if (iter->bttl > 0)
        {
            iter->bttl -= ms;
            ++iter;
            continue;
        }

        iter->ttl -= ms;
        if (iter->ttl <= 0)
        {
            iter = m_Items.erase(iter);
            continue;
        }
        ++iter;
    }
}

void CMatrixDebugInfo::SetStartPos(const CPoint& pos)
{
    m_Pos = pos;
}

void CMatrixDebugInfo::OnLostDevice()
{
    m_Font->OnLostDevice();
}

void CMatrixDebugInfo::OnResetDevice()
{
    m_Font->OnResetDevice();
}