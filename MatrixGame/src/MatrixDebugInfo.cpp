// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixGame.h"
#include "MatrixDebugInfo.hpp"
#include "Math3D.hpp"
#include "3g.hpp"

#define DI_KEY_X 10
#define DI_KEY_Y 10
#define DI_KEY_W 140
#define DI_KEY_H 14
#define DI_VAL_W 200

CMatrixDebugInfo::CMatrixDebugInfo(void) {
    m_ItemsCnt = 0;
    m_Font = NULL;

    m_Pos.x = DI_KEY_X;
    m_Pos.y = DI_KEY_Y;
}

void CMatrixDebugInfo::Clear(void) {
    for (int i = 0; i < m_ItemsCnt; ++i) {
        using std::wstring;
        HDelete(wstring, m_Items[i].key, g_MatrixHeap);
        HDelete(wstring, m_Items[i].val, g_MatrixHeap);
    }
    m_ItemsCnt = 0;

    ClearFont();
}
void CMatrixDebugInfo::Draw(void) {
    if (!m_Font && m_ItemsCnt)
        InitFont();
#if D3DX_SDK_VERSION >= 21
#else
    m_Font->Begin();
#endif

    try {
        int y = m_Pos.y;
        for (int i = 0; i < m_ItemsCnt; ++i) {
            if (m_Items[i].bttl > 0) {
                continue;
            }
            CRect r;
            r.left = m_Pos.x;
            r.right = m_Pos.x + DI_KEY_W;
            r.top = y;
            r.bottom = y + DI_KEY_H;

            BYTE a = 255;
            if (m_Items[i].ttl < 1000)
                a = (BYTE)Float2Int(float(m_Items[i].ttl) * 0.255f);

            DWORD color = (a << 24) | 0xFFFFFF;

            // DWORD noclip = DT_END_ELLIPSIS;
            // if (m_Items[i].val->length() == 0) noclip = DT_NOCLIP;
#if D3DX_SDK_VERSION >= 21
            m_Font->DrawTextW(NULL, m_Items[i].key->c_str(), m_Items[i].key->length(), (LPRECT)&r,
                              DT_NOCLIP | DT_LEFT | DT_VCENTER | DT_SINGLELINE, color);
#else
            m_Font->DrawTextW(m_Items[i].key->c_str(), m_Items[i].key->length(), (LPRECT)&r,
                              DT_NOCLIP | DT_LEFT | DT_VCENTER | DT_SINGLELINE, color);
#endif

            r.left = r.right + 1;
            r.right = r.left + DI_VAL_W;

#if D3DX_SDK_VERSION >= 21
            m_Font->DrawTextW(NULL, m_Items[i].val->c_str(), m_Items[i].val->length(), (LPRECT)&r,
                              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP, color);
#else
            m_Font->DrawTextW(m_Items[i].val->c_str(), m_Items[i].val->length(), (LPRECT)&r,
                              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP, color);
#endif

            y += DI_KEY_H;
        }
    }
    catch (...) {
    }

#if D3DX_SDK_VERSION >= 21
#else
    m_Font->End();
#endif
}

void CMatrixDebugInfo::T(const wchar *key, const wchar *val, int ttl, int bttl, bool add) {
    int i = 0;
    if (!add) {
        while (i < m_ItemsCnt) {
            if (*m_Items[i].key == key) {
                *(m_Items[i].val) = val;
                m_Items[i].ttl = ttl;
                m_Items[i].bttl = bttl;
                return;
            }
            ++i;
        }
    }

    if (m_ItemsCnt >= MAX_DEBUG_INFO_ITEMS)
        return;

    m_Items[m_ItemsCnt].key = HNew(g_MatrixHeap) std::wstring(key);
    m_Items[m_ItemsCnt].val = HNew(g_MatrixHeap) std::wstring(val);
    m_Items[m_ItemsCnt].ttl = ttl;
    m_Items[m_ItemsCnt].bttl = bttl;
    ++m_ItemsCnt;
}

void CMatrixDebugInfo::Takt(int ms) {
    if (!m_Font && m_ItemsCnt)
        InitFont();

    int i = 0;
    while (i < m_ItemsCnt) {
        if (m_Items[i].bttl > 0) {
            m_Items[i].bttl -= ms;
            ++i;
            continue;
        }

        m_Items[i].ttl -= ms;
        if (m_Items[i].ttl <= 0) {
            --m_ItemsCnt;
            using std::wstring;
            HDelete(wstring, m_Items[i].key, g_MatrixHeap);
            HDelete(wstring, m_Items[i].val, g_MatrixHeap);
            if (i < m_ItemsCnt) {
                memcpy(m_Items + i, m_Items + i + 1, sizeof(SDIItem) * (m_ItemsCnt - i));
            }
            ClearFont();
            continue;
        }
        ++i;
    }
}

void CMatrixDebugInfo::InitFont(void) {
    ClearFont();

#if D3DX_SDK_VERSION >= 21
    // summer update

    if (D3D_OK != D3DXCreateFontW(g_D3DD, 10, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                  DEFAULT_QUALITY, DEFAULT_PITCH, L"MS Sans Serif", &m_Font)) {
        m_Font = NULL;
    }

#else
    HFONT fn = CreateFontA(10, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           DEFAULT_QUALITY, DEFAULT_PITCH, "MS Sans Serif");

    if (D3D_OK != D3DXCreateFont(g_D3DD, fn, &m_Font)) {
        m_Font = NULL;
    }
    DeleteObject(fn);
#endif
}