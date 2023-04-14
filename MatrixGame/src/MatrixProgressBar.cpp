// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixProgressBar.hpp"
#include "StringConstants.hpp"
#include "MatrixInstantDraw.hpp"
#include "MatrixSampleStateManager.hpp"

CMatrixProgressBar *CMatrixProgressBar::m_First;
CMatrixProgressBar *CMatrixProgressBar::m_Last;
CMatrixProgressBar *CMatrixProgressBar::m_FirstClones;
CMatrixProgressBar *CMatrixProgressBar::m_LastClones;
CTextureManaged *CMatrixProgressBar::m_Tex;

CMatrixProgressBar::CMatrixProgressBar(void) {
    if (m_Tex == NULL) {
        m_Tex = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_PB);
        m_Tex->MipmapOff();
    }
    m_Tex->RefInc();

    m_Coord = (SPBPos *)HAlloc(sizeof(SPBPos), g_MatrixHeap);
    m_CoordCount = 1;
    m_Coord[0].present = true;

    // Modify(x,y,width, height, pos);
    m_Coord->m_X = float(g_ScreenX + 1);

    LIST_ADD(this, m_First, m_Last, m_Prev, m_Next);

#ifdef _DEBUG
    static int ccc = 0;
    counter = ccc++;
#endif
}

CMatrixProgressBar::~CMatrixProgressBar() {
    m_Tex->RefDecUnload();
    if (m_Tex->Ref() <= 0)
        m_Tex = NULL;

    HFree(m_Coord, g_MatrixHeap);

    if (m_CoordCount > 1) {
        LIST_DEL(this, m_FirstClones, m_LastClones, m_Prev, m_Next);
    }
    else {
        LIST_DEL(this, m_First, m_Last, m_Prev, m_Next);
    }
}

void CMatrixProgressBar::CreateClone(EPBCoord pbc, float x, float y, float width) {
    if (m_CoordCount == 1) {
        m_CoordCount = pbc + 1;
        m_Coord = (SPBPos *)HAllocEx(m_Coord, sizeof(SPBPos) * m_CoordCount, g_MatrixHeap);
        for (int i = 1; i < m_CoordCount; ++i) {
            m_Coord[i].present = false;
        }
        m_Coord[pbc].present = true;
        m_Coord[pbc].m_X = x;
        m_Coord[pbc].m_Y = y;
        m_Coord[pbc].m_Width = width;

        LIST_DEL(this, m_First, m_Last, m_Prev, m_Next);
        LIST_ADD(this, m_FirstClones, m_LastClones, m_Prev, m_Next);
    }
    else {
        if (pbc >= m_CoordCount) {
            int oldc = m_CoordCount;
            m_CoordCount = pbc + 1;
            m_Coord = (SPBPos *)HAllocEx(m_Coord, sizeof(SPBPos) * m_CoordCount, g_MatrixHeap);
            for (int i = oldc; i < m_CoordCount; ++i) {
                m_Coord[i].present = false;
            }
        }
        m_Coord[pbc].present = true;
        m_Coord[pbc].m_X = x;
        m_Coord[pbc].m_Y = y;
        m_Coord[pbc].m_Width = width;
    }
}
void CMatrixProgressBar::KillClone(EPBCoord pbc) {
    if (pbc < m_CoordCount) {
        m_Coord[pbc].present = false;

        int i = m_CoordCount - 1;
        while (i >= 0) {
            if (m_Coord[i].present)
                return;
            --i;
        }
        ASSERT(i >= 0);
        int oldc = m_CoordCount;
        m_CoordCount = i + 1;
        if (m_CoordCount == 1) {
            LIST_DEL(this, m_FirstClones, m_LastClones, m_Prev, m_Next);
            LIST_ADD(this, m_First, m_Last, m_Prev, m_Next);
        }
        else if (oldc > m_CoordCount) {
            m_Coord = (SPBPos *)HAllocEx(m_Coord, sizeof(SPBPos) * m_CoordCount, g_MatrixHeap);
        }
    }
}

void CMatrixProgressBar::DrawClones(bool pbd) {
    for (int i = 1; i < m_CoordCount; ++i)
        if (m_Coord[i].present)
            Draw(m_Coord + i, pbd);
}

void CMatrixProgressBar::Draw(bool pbd) {
    Draw(m_Coord, pbd);
}

void CMatrixProgressBar::Draw(SPBPos *pos, bool pbd) {
    float h = float(m_Tex->GetSizeY() >> 2);

    if ((Float2Int(pos->m_X) >= g_ScreenX) || (Float2Int(pos->m_Y + h) < 0) ||
        (Float2Int(pos->m_X + pos->m_Width) < 0) || (Float2Int(pos->m_Y) >= g_ScreenY))
        return;

    float posx = (float)floor(pos->m_X) - 0.5f;
    float posy = (float)floor(pos->m_Y) - 0.5f;

    float width = (float)floor(pos->m_Width);  // + 0.5f;

    SVert_V4_UV p[8];

    p[0].p = D3DXVECTOR4(posx, posy + h, PB_Z, 1.0f);
    p[1].p = D3DXVECTOR4(posx, posy, PB_Z, 1.0f);
    p[2].p = D3DXVECTOR4(posx + h, posy + h, PB_Z, 1.0f);
    p[3].p = D3DXVECTOR4(posx + h, posy, PB_Z, 1.0f);
    p[4].p = D3DXVECTOR4(posx + width - h, posy + h, PB_Z, 1.0f);
    p[5].p = D3DXVECTOR4(posx + width - h, posy, PB_Z, 1.0f);
    p[6].p = D3DXVECTOR4(posx + width, posy + h, PB_Z, 1.0f);
    p[7].p = D3DXVECTOR4(posx + width, posy, PB_Z, 1.0f);

    p[0].tu = 0;
    p[0].tv = 0.25f;
    p[1].tu = 0;
    p[1].tv = 0;
    p[2].tu = 0.25;
    p[2].tv = 0.25f;
    p[3].tu = 0.25;
    p[3].tv = 0;
    p[4].tu = 0.75;
    p[4].tv = 0.25;
    p[5].tu = 0.75;
    p[5].tv = 0;
    p[6].tu = 1;
    p[6].tv = 0.25;
    p[7].tu = 1;
    p[7].tv = 0;

    if (!pbd) {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF));

        CInstDraw::AddVerts(p, m_Tex);
        CInstDraw::AddVerts(p + 2, m_Tex);
        CInstDraw::AddVerts(p + 4, m_Tex);
    }
    else {
        DWORD c;
        if (m_Pos < 0.5f)
            c = LIC(PB_COLOR_0, PB_COLOR_1, m_Pos * 2.0f);
        else
            c = LIC(PB_COLOR_1, PB_COLOR_2, m_Pos * 2.0f - 1.0f);

        float pp = (h + h) / width;
        float w = m_Pos * width - (h + h);

        DWORD cc = c | 0xFF000000;

        if (m_Pos < pp) {
            // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR,	c | (Float2Int(255.0f*m_Pos/pp)) << 24));
            // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR,	c | 0xFF000000));

            int i = Float2Int(m_Pos * 5.0f / pp);
            if (i == 0)
                return;
            if (i == 1) {
                p[0].tv = 1;
                p[0].tu = 0.5;
                p[1].tv = 0.75f;
                p[1].tu = 0.5;
                p[2].tv = 1;
                p[2].tu = 1;
                p[3].tv = 0.75f;
                p[3].tu = 1;
            }
            else if (i == 2) {
                p[0].tv = 1;
                p[0].tu = 0;
                p[1].tv = 0.75f;
                p[1].tu = 0;
                p[2].tv = 1;
                p[2].tu = 0.5f;
                p[3].tv = 0.75f;
                p[3].tu = 0.5f;
            }
            else if (i == 3) {
                p[0].tv = 0.75f;
                p[0].tu = 0.5;
                p[1].tv = 0.5f;
                p[1].tu = 0.5;
                p[2].tv = 0.75f;
                p[2].tu = 1;
                p[3].tv = 0.5f;
                p[3].tu = 1;
            }
            else if (i >= 4) {
                p[0].tv = 0.75f;
                p[0].tu = 0;
                p[1].tv = 0.5f;
                p[1].tu = 0;
                p[2].tv = 0.75f;
                p[2].tu = 0.5f;
                p[3].tv = 0.5f;
                p[3].tu = 0.5f;
            }

            p[0].p = D3DXVECTOR4(posx, posy + h, PB_Z, 1.0f);
            p[1].p = D3DXVECTOR4(posx, posy, PB_Z, 1.0f);
            p[2].p = D3DXVECTOR4(posx + h + h, posy + h, PB_Z, 1.0f);
            p[3].p = D3DXVECTOR4(posx + h + h, posy, PB_Z, 1.0f);

            CInstDraw::AddVerts(p, m_Tex, cc);
        }
        else {
            p[0].p = D3DXVECTOR4(posx, posy + h, PB_Z, 1.0f);
            p[1].p = D3DXVECTOR4(posx, posy, PB_Z, 1.0f);
            p[2].p = D3DXVECTOR4(posx + h, posy + h, PB_Z, 1.0f);
            p[3].p = D3DXVECTOR4(posx + h, posy, PB_Z, 1.0f);
            p[4].p = D3DXVECTOR4(posx + h + w, posy + h, PB_Z, 1.0f);
            p[5].p = D3DXVECTOR4(posx + h + w, posy, PB_Z, 1.0f);
            p[6].p = D3DXVECTOR4(posx + h + w + h, posy + h, PB_Z, 1.0f);
            p[7].p = D3DXVECTOR4(posx + h + w + h, posy, PB_Z, 1.0f);

            p[0].tv = 0.5f;
            p[1].tv = 0.25f;
            p[2].tv = 0.5f;
            p[3].tv = 0.25f;
            p[4].tv = 0.5;
            p[5].tv = 0.25f;
            p[6].tv = 0.5;
            p[7].tv = 0.25f;

            if (w < (h + h)) {
                float k = 0.25f * w / h;

                CInstDraw::AddVerts(p + 4, m_Tex, cc);

                p[4].tu = 0.25f + k;
                p[5].tu = 0.25f + k;

                CInstDraw::AddVerts(p, m_Tex, cc);
                CInstDraw::AddVerts(p + 2, m_Tex, cc);
            }
            else {
                CInstDraw::AddVerts(p, m_Tex, cc);
                CInstDraw::AddVerts(p + 2, m_Tex, cc);
                CInstDraw::AddVerts(p + 4, m_Tex, cc);
            }
        }
    }
}

static void bdi(void) {
    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHAREF, 0));

    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);

    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetColorOpDisable(1);

    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
}

static void adb(void) {
    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHAREF, 0x08));

    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    g_D3DD->SetRenderState(D3DRS_ZENABLE, TRUE);
}

void CMatrixProgressBar::DrawAll(void) {
    DTRACE();

    if (m_First == NULL && m_FirstClones == NULL)
        return;

    bdi();

    CInstDraw::BeginDraw(IDFVF_V4_UV);

    CMatrixProgressBar *pb = m_First;
    while (pb) {
        pb->Draw(false);
        pb = pb->m_Next;
    }
    pb = m_FirstClones;
    while (pb) {
        pb->Draw(false);
        pb = pb->m_Next;
    }

    CInstDraw::ActualDraw();

    CInstDraw::BeginDraw(IDFVF_V4_UV);

    pb = m_First;
    while (pb) {
        pb->Draw(true);
        pb = pb->m_Next;
    }
    pb = m_FirstClones;
    while (pb) {
        pb->Draw(true);
        pb = pb->m_Next;
    }

    CInstDraw::ActualDraw();

    adb();
}

void CMatrixProgressBar::DrawAllClones(void) {
    DTRACE();

    if (m_FirstClones == NULL)
        return;

    bdi();

    //   ASSERT_DX(g_Sampler.SetState(0,D3DSAMP_MAGFILTER,			D3DTEXF_LINEAR));
    // ASSERT_DX(g_Sampler.SetState(0,D3DSAMP_MINFILTER,			D3DTEXF_NONE));
    // ASSERT_DX(g_Sampler.SetState(0,D3DSAMP_MIPFILTER,			D3DTEXF_NONE));

    //   g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP);
    //   g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP);
    // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE,				D3DZB_FALSE));
    // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE,		FALSE));
    // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE,		TRUE));

    //   SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    //   SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    //   SetColorOpDisable(1);

    CInstDraw::BeginDraw(IDFVF_V4_UV);

    CMatrixProgressBar *pb = m_FirstClones;
    while (pb) {
        pb->DrawClones(false);
        pb = pb->m_Next;
    }
    CInstDraw::ActualDraw();

    CInstDraw::BeginDraw(IDFVF_V4_UV);
    pb = m_FirstClones;
    while (pb) {
        pb->DrawClones(true);
        pb = pb->m_Next;
    }
    CInstDraw::ActualDraw();

    adb();
}
