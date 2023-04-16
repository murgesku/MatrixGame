// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixMap.hpp"

#include "CFile.hpp"

#define CURSOR_Z 0.0f

void CMatrixCursor::SetVisible(bool flag) {
    if (!g_Config.m_SoftwareCursor) {
        if (FLAG(m_CursorFlags, CURSOR_VISIBLE) && !flag) {
            while (ShowCursor(false) >= 0)
                ;
            g_D3DD->ShowCursor(false);
            SetCursor(NULL);
        }
        else if (!FLAG(m_CursorFlags, CURSOR_VISIBLE) && flag) {
            SetCursor(m_CursorIcons[m_Frame]);
            int cntTry = 0;
            int cntCurs = ShowCursor(true);
            int cntCursPrev = cntCurs - 1;
            while (cntCurs < 0) {
                if (cntCurs == cntCursPrev)
                    ++cntTry;
                if (cntTry > 100) {
                    g_Config.m_SoftwareCursor = true;
                    m_CurCursor.clear();
                    Select(CURSOR_ARROW);
                    INITFLAG(m_CursorFlags, CURSOR_VISIBLE, flag);
                    return;
                }
                cntCursPrev = cntCurs;
                cntCurs = ShowCursor(true);
            }
            g_D3DD->ShowCursor(true);
            ShowCursor(true);
        }
    }
    INITFLAG(m_CursorFlags, CURSOR_VISIBLE, flag);
}

void CMatrixCursor::Select(const std::wstring& name) {
    DTRACE();

    if (m_CurCursor == name)
    {
        return;
    }
    DCP();
    m_CurCursor = name;

    Clear();
    DCP();

    m_Frame = 0;

    ParamParser n;
    DCP();
    int idx;
    for (idx = 0; idx < g_Config.m_CursorsCnt; ++idx) {
        if (g_Config.m_Cursors[idx].key == name) {
            n = g_Config.m_Cursors[idx].val;

            m_HotSpot.x = n.GetStrPar(1, L"?").GetStrPar(0, L",").GetInt();
            m_HotSpot.y = n.GetStrPar(1, L"?").GetStrPar(1, L",").GetInt();

            m_FramesCnt = n.GetStrPar(1, L"?").GetStrPar(2, L",").GetInt();
            m_CursorSize = n.GetStrPar(1, L"?").GetStrPar(3, L",").GetInt();
            m_CursorTimePeriod = n.GetStrPar(1, L"?").GetStrPar(4, L",").GetInt();

            if (m_FramesCnt < 0) {
                m_FramesCnt = -m_FramesCnt;
                SETFLAG(m_CursorFlags, CURSOR_REVERSEANIM);
            }
            else
                RESETFLAG(m_CursorFlags, CURSOR_REVERSEANIM);

            ASSERT(m_FramesCnt >= 1);

            m_FrameInc = 1;

            n = n.GetStrPar(0, L"?");

            break;
        }
    }
    DCP();

    if (g_Config.m_SoftwareCursor) {
        m_CursorTexture = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, n.c_str());
        m_CursorTexture->MipmapOff();
        m_CursorTexture->Load();
        m_CursorInTexLine = m_CursorTexture->GetSizeX() / m_CursorSize;
        m_TexSizeXInversed = 1.0f / float(m_CursorTexture->GetSizeX());
        m_TexSizeYInversed = 1.0f / float(m_CursorTexture->GetSizeY());

        while (ShowCursor(false) >= 0)
            ;
        g_D3DD->ShowCursor(false);
        CalcUV();
    }
    else {
        CBitmap frame(g_CacheHeap);
        frame.CreateRGBA(m_CursorSize, m_CursorSize);
        CBitmap bm(g_CacheHeap);
        DCP();

        std::wstring tn;
        DCP();

        CFile::FileExist(tn, n.c_str(), CacheExtsTex);
        DCP();

        bm.LoadFromPNG(tn.c_str());
        DCP();

        int i = 0, x = 0, y = 0;

        m_CursorIcons = (HICON *)HAlloc(sizeof(HICON) * m_FramesCnt, g_MatrixHeap);

        ICONINFO ii;
        ii.fIcon = FALSE;
        ii.xHotspot = m_HotSpot.x;
        ii.yHotspot = m_HotSpot.y;
        do {
            if (x + m_CursorSize > bm.SizeX()) {
                x = 0;
                y += m_CursorSize;
            }
            if (y + m_CursorSize > bm.SizeY()) {
                break;
            }
            frame.Copy(CPoint(0, 0), frame.Size(), bm, CPoint(x, y));
            frame.WBM_Init();
            DCP();

            ii.hbmMask = frame.WBM_Bitmap();
            ii.hbmColor = frame.WBM_Bitmap();

            m_CursorIcons[i++] = CreateIconIndirect(&ii);
            DCP();

            x += m_CursorSize;
        }
        while (i < m_FramesCnt);

        SetCursor(m_CursorIcons[0]);
        if (FLAG(m_CursorFlags, CURSOR_VISIBLE))
            while (ShowCursor(true) < 0)
                ;
        DCP();
    }
}

void CMatrixCursor::CalcUV(void) {
    int y = TruncFloat(float(m_Frame * m_CursorSize) * m_TexSizeXInversed);
    int x = m_Frame - y * m_CursorInTexLine;

    m_u0 = float(x * m_CursorSize) * m_TexSizeXInversed + m_TexSizeXInversed * 0.5f;
    m_v0 = float(y * m_CursorSize) * m_TexSizeYInversed + m_TexSizeYInversed * 0.5f;

    m_u1 = float((x + 1) * m_CursorSize) * m_TexSizeXInversed - m_TexSizeXInversed * 0.5f;
    m_v1 = float((y + 1) * m_CursorSize) * m_TexSizeYInversed - m_TexSizeYInversed * 0.5f;
}

void CMatrixCursor::Takt(int ms) {
    DTRACE();

    m_Time += ms;

    if (m_FramesCnt < 2)
        return;
    int oldframe = m_Frame;
    while (m_Time > m_NextCursorTime) {
        m_NextCursorTime += m_CursorTimePeriod;

        if (!FLAG(m_CursorFlags, CURSOR_REVERSEANIM)) {
            ++m_Frame;
            if (m_Frame >= m_FramesCnt)
                m_Frame = 0;
        }
        else {
            m_Frame += m_FrameInc;
            if (m_FrameInc > 0) {
                if (m_Frame >= m_FramesCnt) {
                    m_Frame -= 2;
                    m_FrameInc = -m_FrameInc;
                }
            }
            else {
                if (m_Frame < 0) {
                    m_Frame += 2;
                    m_FrameInc = -m_FrameInc;
                }
            }
        }
    }
    if (oldframe != m_Frame && FLAG(m_CursorFlags, CURSOR_VISIBLE)) {
        if (!g_Config.m_SoftwareCursor) {
            // switch hardware icon
            SetCursor(m_CursorIcons[m_Frame]);
        }
        else {
            CalcUV();
        }
    }
}

void CMatrixCursor::Clear(void) {
    SetCursor(m_OldCursor);

    if (m_CursorTexture) {
        m_CursorTexture->Unload();
        m_CursorTexture = NULL;
    }

    if (m_CursorIcons) {
        for (int i = 0; i < m_FramesCnt; ++i) {
            DestroyIcon(m_CursorIcons[i]);
        }
        HFree(m_CursorIcons, g_MatrixHeap);
        m_CursorIcons = NULL;
    }
}

void CMatrixCursor::Draw(void) {
    DTRACE();

    if (g_Config.m_SoftwareCursor && FLAG(m_CursorFlags, CURSOR_VISIBLE)) {
        SVert_V4_UV v[4];

        v[0].tu = m_u0;
        v[0].tv = m_v1;
        v[1].tu = m_u0;
        v[1].tv = m_v0;
        v[2].tu = m_u1;
        v[2].tv = m_v1;
        v[3].tu = m_u1;
        v[3].tv = m_v0;

        SetAlphaOpSelect(0, D3DTA_TEXTURE);
        SetColorOpSelect(0, D3DTA_TEXTURE);
        SetColorOpDisable(1);

        g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        g_D3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));

        // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF));

        int x0 = m_Pos.x - m_HotSpot.x;
        int y0 = m_Pos.y - m_HotSpot.y;
        int x1 = x0 + m_CursorSize;
        int y1 = y0 + m_CursorSize;

        v[0].p = D3DXVECTOR4(float(x0) - 0.5f, float(y1) - 0.5f, CURSOR_Z, 1.0f);
        v[1].p = D3DXVECTOR4(float(x0) - 0.5f, float(y0) - 0.5f, CURSOR_Z, 1.0f);
        v[2].p = D3DXVECTOR4(float(x1) - 0.5f, float(y1) - 0.5f, CURSOR_Z, 1.0f);
        v[3].p = D3DXVECTOR4(float(x1) - 0.5f, float(y0) - 0.5f, CURSOR_Z, 1.0f);

        CInstDraw::BeginDraw(IDFVF_V4_UV);
        CInstDraw::AddVerts(v, m_CursorTexture);
        CInstDraw::ActualDraw();

        g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

        g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        g_D3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    }
}