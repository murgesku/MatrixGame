// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixDebugInfo.hpp"
#include "3g.hpp"
#include <stdexcept>
#include <cstdint>

#include <stupid_logger.hpp>

#define DI_KEY_X 10
#define DI_KEY_Y 10
#define DI_KEY_W 240
#define DI_KEY_H 19
#define DI_VAL_W 400
#define DI_BG_HORIZONTAL_OFFSET 40
#define DI_AVERAGE_CHAR_WIDTH 9     // Can be found with m_Font->GetTextMetricsA(&tm);
#define DI_BG_COLOR 0x40000000

#define MAX_DEBUG_INFO_ITEMS 128

extern IDirect3DDevice9* g_D3DD;

CMatrixDebugInfo::CMatrixDebugInfo()
: m_Font{nullptr}
, m_Pos{DI_KEY_X, DI_KEY_Y}
{
    if (D3D_OK != D3DXCreateFontW(g_D3DD, 25, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                  PROOF_QUALITY, DEFAULT_PITCH, L"MS Sans Serif", &m_Font))
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
        int max_value_len = 0;

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

            if (item.val.length() > max_value_len)
                max_value_len = item.val.length();

            y += DI_KEY_H;
        }

        if (m_Items.size() > 0) {
            // Draw rect-pad background

            D3DXVECTOR2 left_top{0, 0};
            D3DXVECTOR2 right_bot{
                    static_cast<FLOAT>(DI_KEY_W + DI_BG_HORIZONTAL_OFFSET + max_value_len * DI_AVERAGE_CHAR_WIDTH),
                    static_cast<FLOAT>((DI_KEY_Y + DI_KEY_H * 2) + DI_KEY_H * m_Items.size())
            };

            D3DXVECTOR4 tri1[3];
            tri1[0] = D3DXVECTOR4(left_top.x, left_top.y, 0.5f, 1.0f);
            tri1[1] = D3DXVECTOR4(right_bot.x, left_top.y, 0.5f, 1.0f);
            tri1[2] = D3DXVECTOR4(right_bot.x, right_bot.y, 0.5f, 1.0f);

            D3DXVECTOR4 tri2[3];
            tri2[0] = D3DXVECTOR4(left_top.x, left_top.y, 0.5f, 1.0f);
            tri2[1] = D3DXVECTOR4(right_bot.x, right_bot.y, 0.5f, 1.0f);
            tri2[2] = D3DXVECTOR4(left_top.x, right_bot.y, 0.5f, 1.0f);

            g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, DI_BG_COLOR);

            SetAlphaOpSelect(0, D3DTA_TFACTOR);
            SetColorOpSelect(0, D3DTA_TFACTOR);

            g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

            g_D3DD->SetFVF(D3DFVF_XYZRHW);

            g_D3DD->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 1, &tri1, sizeof(D3DXVECTOR4));
            g_D3DD->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 1, &tri2, sizeof(D3DXVECTOR4));
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