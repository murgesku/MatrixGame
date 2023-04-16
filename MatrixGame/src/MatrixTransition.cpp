// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixTransition.hpp"
#include "MatrixMap.hpp"
#include "MatrixSampleStateManager.hpp"

void CTransition::Clear(void) {
    DTRACE();

    if (m_Tex) {
        g_Cache->Destroy(m_Tex);
        m_Tex = NULL;

        HFree(m_Geom, g_MatrixHeap);
        m_Geom = NULL;
        m_GeomCnt = 0;
    }
}

void CTransition::BuildTexture(void) {
    DTRACE();

    HDC hdc = GetDC(g_Wnd);

    RECT r;
    GetClientRect(g_Wnd, &r);

    m_ScreenX = r.right - r.left;
    m_ScreenY = r.bottom - r.top;

    CBlockPar *tra = g_MatrixData->BlockGetNE(L"Transitions");
    if (tra == NULL)
        return;
    tra = tra->BlockGetNE(utils::format(L"%d.%d", m_ScreenX, m_ScreenY).c_str());

    if (tra == NULL)
        return;

    m_GeomCnt = tra->ParCount();

    if (m_GeomCnt == 0)
        return;

    CBitmap bm(g_CacheHeap);
    CBitmap bmout(g_CacheHeap);
    bmout.CreateRGB(DetermineGreaterPowerOfTwo(m_ScreenX), DetermineGreaterPowerOfTwo(m_ScreenY));
    bmout.Fill(CPoint(0, 0), bmout.Size(), 0);

    // float diver = 1.0f / float(m_ScreenY / 2);

    m_Geom = (SGeom *)HAllocClear(sizeof(SGeom) * m_GeomCnt, g_MatrixHeap);
    for (int i = 0; i < m_GeomCnt; ++i) {
        const auto da = tra->ParGet(i);

        m_Geom[i].verts[0].p.x = (float)da.GetStrPar(0, L"|").GetStrPar(0, L",").GetDouble();
        m_Geom[i].verts[0].p.y = (float)da.GetStrPar(0, L"|").GetStrPar(1, L",").GetDouble();
        m_Geom[i].verts[0].p.z = 0;
        m_Geom[i].verts[0].p.w = 1;
        m_Geom[i].verts[0].tu = m_Geom[i].verts[0].p.x / float(bmout.SizeX());
        m_Geom[i].verts[0].tv = m_Geom[i].verts[0].p.y / float(bmout.SizeY());

        m_Geom[i].verts[1].p.x = (float)da.GetStrPar(1, L"|").GetStrPar(0, L",").GetDouble();
        m_Geom[i].verts[1].p.y = (float)da.GetStrPar(1, L"|").GetStrPar(1, L",").GetDouble();
        m_Geom[i].verts[1].p.z = 0;
        m_Geom[i].verts[1].p.w = 1;
        m_Geom[i].verts[1].tu = m_Geom[i].verts[1].p.x / float(bmout.SizeX());
        m_Geom[i].verts[1].tv = m_Geom[i].verts[1].p.y / float(bmout.SizeY());

        m_Geom[i].verts[2].p.x = (float)da.GetStrPar(3, L"|").GetStrPar(0, L",").GetDouble();
        m_Geom[i].verts[2].p.y = (float)da.GetStrPar(3, L"|").GetStrPar(1, L",").GetDouble();
        m_Geom[i].verts[2].p.z = 0;
        m_Geom[i].verts[2].p.w = 1;
        m_Geom[i].verts[2].tu = m_Geom[i].verts[2].p.x / float(bmout.SizeX());
        m_Geom[i].verts[2].tv = m_Geom[i].verts[2].p.y / float(bmout.SizeY());

        m_Geom[i].verts[3].p.x = (float)da.GetStrPar(2, L"|").GetStrPar(0, L",").GetDouble();
        m_Geom[i].verts[3].p.y = (float)da.GetStrPar(2, L"|").GetStrPar(1, L",").GetDouble();
        m_Geom[i].verts[3].p.z = 0;
        m_Geom[i].verts[3].p.w = 1;
        m_Geom[i].verts[3].tu = m_Geom[i].verts[3].p.x / float(bmout.SizeX());
        m_Geom[i].verts[3].tv = m_Geom[i].verts[3].p.y / float(bmout.SizeY());

        if (utils::starts_with(tra->ParGetName(i), L"Up,"))
            m_Geom[i].dir.y = -1;
        else if (utils::starts_with(tra->ParGetName(i), L"Down,"))
            m_Geom[i].dir.y = 1;
        else if (utils::starts_with(tra->ParGetName(i), L"Left,"))
            m_Geom[i].dir.x = -1;
        else if (utils::starts_with(tra->ParGetName(i), L"Right,"))
            m_Geom[i].dir.x = 1;

        m_Geom[i].dir *= (float)tra->ParGetName(i).GetStrPar(1, L",").GetDouble();
    }

    bm.WBM_Bitmap(CreateCompatibleBitmap(hdc, r.right, r.bottom));
    bm.WBM_BitmapDC(CreateCompatibleDC(hdc));
    if (SelectObject(bm.WBM_BitmapDC(), bm.WBM_Bitmap()) == 0) {
        ReleaseDC(g_Wnd, hdc);
        return;
    }
    BitBlt(bm.WBM_BitmapDC(), 0, 0, r.right, r.bottom, hdc, 0, 0, SRCCOPY);

    ReleaseDC(g_Wnd, hdc);

    bm.WBM_Save(true);

    bmout.Copy(CPoint(0, 0), bm.Size(), bm, CPoint(0, 0));

    m_Tex = CACHE_CREATE_TEXTUREMANAGED();
    m_Tex->MipmapOff();
    m_Tex->LoadFromBitmap(bmout, D3DFMT_DXT1, 1);
}

void CTransition::Render(void) {
    DTRACE();

    if (m_Tex == NULL)
        return;

    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
    // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHAREF, 0));

    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    SetColorOpSelect(0, D3DTA_TEXTURE);
    SetAlphaOpSelect(0, D3DTA_TEXTURE);
    SetColorOpDisable(1);

    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

    m_Tex->Preload();

    SVert_V4_UV verts[4];

    CInstDraw::BeginDraw(IDFVF_V4_UV);

    for (int i = 0; i < m_GeomCnt; ++i) {
        float posx = (float)floor(m_Geom[i].pos.x) - 0.5f;
        float posy = (float)floor(m_Geom[i].pos.y) - 0.5f;

        memcpy(verts, m_Geom[i].verts, sizeof(verts));
        for (int j = 0; j < 4; ++j) {
            verts[j].p.x += posx;
            verts[j].p.y += posy;
        }

        CInstDraw::AddVerts(verts, m_Tex);
    }

    CInstDraw::ActualDraw();

    // g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE,		FALSE);
    // ASSERT_DX(g_D3DD->SetRenderState( D3DRS_ALPHATESTENABLE,   FALSE ));
    // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHAREF, 0x08));

    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    g_D3DD->SetRenderState(D3DRS_ZENABLE, TRUE);
}

void CTransition::RenderToPrimaryScreen(void) {
    if (m_ScreenX != g_ScreenX || m_ScreenY != g_ScreenY) {
        float kx = float(double(g_ScreenX) / double(m_ScreenX));
        float ky = float(double(g_ScreenY) / double(m_ScreenY));

        for (int i = 0; i < m_GeomCnt; ++i) {
            for (int j = 0; j < 4; ++j) {
                m_Geom[i].verts[j].p.x *= kx;
                m_Geom[i].verts[j].p.y *= ky;
            }
        }
    }

    g_D3DD->BeginScene();
    Render();
    g_D3DD->EndScene();
    g_D3DD->Present(NULL, NULL, NULL, NULL);

    SETFLAG(g_MatrixMap->m_Flags, MMFLAG_TRANSITION);
}

void CTransition::Takt(int ms) {
    float add = float(ms) * 0.5f;
    for (int i = 0; i < m_GeomCnt;) {
        m_Geom[i].pos += m_Geom[i].dir * add;

        int j = 0;
        for (j = 0; j < 4; ++j) {
            float x = m_Geom[i].verts[j].p.x + m_Geom[i].pos.x;
            float y = m_Geom[i].verts[j].p.y + m_Geom[i].pos.y;
            if (x >= 0 && x < float(g_ScreenX) && y >= 0 && y < float(g_ScreenY))
                break;
        }
        if (j == 4) {
            // delete this

            m_Geom[i] = m_Geom[--m_GeomCnt];

            if (m_GeomCnt == 0) {
                RESETFLAG(g_MatrixMap->m_Flags, MMFLAG_TRANSITION);
                Clear();
                return;
            }
            continue;
        }

        ++i;
    }
}
