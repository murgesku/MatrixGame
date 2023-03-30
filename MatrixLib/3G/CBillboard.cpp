// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "3g.hpp"

#include "CBillboard.hpp"
#include "Mem.hpp"

CBillboard *CBillboard::m_FirstIntense;
CBillboard::PCBillboard CBillboard::bboards[MAX_BBOARDS];
int CBillboard::bboards_left;
int CBillboard::bboards_rite;

CTextureManaged *CBillboard::m_SortableTex;
// CTextureManaged * CBillboard::m_IntenseTex;

D3D_VB CBillboard::m_VB;
D3D_IB CBillboard::m_IB;

void CBillboard::SortEndDraw(const D3DXMATRIX &iview, const D3DXVECTOR3 &campos) {
    DTRACE();

    if (m_VB == NULL || m_IB == NULL)
        Init();

    int ns = 0, ni = 0;
    SBillboardVertex *vb;

    if (bboards_left < bboards_rite) {
        ns = (bboards_rite - bboards_left);

        LOCK_VB_DYNAMIC(m_VB, &vb);

        for (int i = bboards_left; i < bboards_rite; ++i) {
            bboards[i]->UpdateVBSlot(vb, iview);
            vb += 4;
        }
    }

    struct SDrawBillGroup {
        int cntv;  // vertices
        int cntt;  // triangles
        // int vbase;    // no need, always 0
        int ibase;
        int min_idx;
        CTextureManaged *tex;
    };

    SDrawBillGroup groups[MAX_BB_TEXGROUPS];
    int groupscnt = 0;

    if (m_FirstIntense || CBillboardLine::m_First) {
        int ost = (MAX_BBOARDS * 2) - ns;

        if (ns == 0) {
            // lock VB/IB if not locked yet
            LOCK_VB_DYNAMIC(m_VB, &vb);
        }

        CBillboard *cur = m_FirstIntense;
        int n = 0, minvi = ns * 4;

        groups[0].tex = NULL;

        int ibt_ = ns * 6;
        while (cur) {
            cur->UpdateVBSlot(vb, iview);
            vb += 4;

            if (groups[groupscnt].tex == NULL) {
                // new group
                groups[groupscnt].tex = cur->m_TexIntense;
                groups[groupscnt].min_idx = minvi;

#ifdef _DEBUG
                if ((DWORD)groups[groupscnt].tex == 0xACACACAC)
                    debugbreak();
#endif

                // groups[groupscnt].vbase = base;
                groups[groupscnt].ibase = ibt_;
            }

            ++n;
            ++ni;
            minvi += 4;
            ibt_ += 6;
            --ost;

            if (cur->m_Next == cur->m_NextTex) {
                // end of group
                groups[groupscnt].cntv = n * 4;
                groups[groupscnt].cntt = n * 2;
                if (++groupscnt >= MAX_BB_TEXGROUPS)
                    break;
                groups[groupscnt].tex = NULL;

                n = 0;
            }

            if (ost <= 0)
                break;

            cur = cur->m_Next;
        };

        if (groupscnt < MAX_BB_TEXGROUPS && ost > 0) {
            ASSERT(n == 0);

            CBillboardLine *bl = CBillboardLine::m_First;
            while (bl != NULL) {
                bl->UpdateVBSlot(vb, campos);
                vb += 4;

                if (groups[groupscnt].tex == NULL) {
                    // new group
                    groups[groupscnt].tex = bl->m_Tex;
                    groups[groupscnt].min_idx = minvi;
#ifdef _DEBUG
                    if ((DWORD)groups[groupscnt].tex == 0xACACACAC)
                        debugbreak();
#endif
                    // groups[groupscnt].vbase = base;
                    groups[groupscnt].ibase = ibt_;
                }

                ++n;
                ++ni;
                minvi += 4;
                ibt_ += 6;
                --ost;

                if (bl->m_Next == bl->m_NextTex) {
                    // end of group
                    groups[groupscnt].cntv = n * 4;
                    groups[groupscnt].cntt = n * 2;
                    if (++groupscnt >= MAX_BB_TEXGROUPS)
                        break;
                    groups[groupscnt].tex = NULL;

                    n = 0;
                }

                if (ost <= 0)
                    break;

                bl = bl->m_Next;
            }

            ASSERT((ost > 0 && n == 0) || ost <= 0);
        }

        CBillboardLine::m_First = NULL;

        UNLOCK_VB(m_VB);
    }
    else {
        if (ns > 0) {
            UNLOCK_VB(m_VB);
        }
    }

    // draw sorted

    if (ns > 0) {
        ASSERT_DX(g_D3DD->SetFVF(BILLBOARD_FVF));

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHAREF, 0));

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, FALSE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_RESULTARG, D3DTA_CURRENT));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_CURRENT));

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

        SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
        SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
        SetColorOpDisable(1);

        ASSERT_DX(g_D3DD->SetStreamSource(0, GET_VB(m_VB), 0, sizeof(SBillboardVertex)));
        ASSERT_DX(g_D3DD->SetIndices(GET_IB(m_IB)));

        ASSERT_DX(g_D3DD->SetTexture(0, m_SortableTex->Tex()));

        ASSERT_DX(g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, ns * 4, 0, ns * 2));
    }

    if (ni > 0) {
        ASSERT_DX(g_D3DD->SetFVF(BILLBOARD_FVF));

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHAREF, 0));

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));

        SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
        SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
        SetColorOpDisable(1);

        g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

        g_D3DD->SetStreamSource(0, GET_VB(m_VB), 0, sizeof(SBillboardVertex));
        ASSERT_DX(g_D3DD->SetIndices(GET_IB(m_IB)));

        for (int i = 0; i < groupscnt; ++i) {
            groups[i].tex->Preload();
            ASSERT_DX(g_D3DD->SetTexture(0, groups[i].tex->Tex()));
            ASSERT_DX(g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, groups[i].min_idx, groups[i].cntv,
                                                   groups[i].ibase, groups[i].cntt));
        }
    }

    g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHAREF, 0x08));

    // m_First = NULL;
    bboards_left = MAX_BBOARDS >> 1;
    bboards_rite = MAX_BBOARDS >> 1;
    // m_Root = NULL;
    m_FirstIntense = NULL;
}

void CBillboard::SortIntense(void) {
    CBillboard *f = m_FirstIntense;

#ifdef _DEBUG

    // for (int cnt = 10000;f;cnt--,f = f->m_Next)
    //{
    //    if (f == this || f == f->m_Next || cnt < 0)
    //    {
    //        break;
    //    }
    //}

    // f = m_FirstIntense;
#endif

    while (f) {
        if (f->m_TexIntense == this->m_TexIntense) {
            m_Next = f->m_Next;
            m_NextTex = f->m_NextTex;
            f->m_Next = this;
            return;
        }
        f = f->m_NextTex;
    }

    // no with such tex

    m_Next = m_FirstIntense;
    m_NextTex = m_FirstIntense;
    m_FirstIntense = this;
}

void CBillboard::Sort(const D3DXMATRIX &sort) {
    if (IsIntense()) {
        SortIntense();
        return;
    }

    bool noleft = (bboards_left <= 0);
    bool norite = (bboards_rite >= MAX_BBOARDS);
    if (noleft && norite)
        return;

    m_Z = Double2Int((sort._13 * m_Pos.x + sort._23 * m_Pos.y + sort._33 * m_Pos.z + sort._43) * 256.0);

    if (bboards_left == bboards_rite) {
        bboards[bboards_left] = this;
        ++bboards_rite;
        return;
    }
    // seek index
    int idx;
    int idx0 = bboards_left;
    int idx1 = bboards_rite;
    for (;;) {
        idx = ((idx1 - idx0) >> 1) + idx0;

        if (bboards[idx]->m_Z < m_Z) {
            // left
            if (idx == idx0)
                break;
            idx1 = idx;
        }
        else {
            // rite
            ++idx;
            if (idx == idx1)
                break;
            idx0 = idx;
        }
    }
    if (!norite && (idx == bboards_rite)) {
        ++bboards_rite;
        bboards[idx] = this;
    }
    else if (!noleft && (idx == bboards_left)) {
        --bboards_left;
        bboards[idx - 1] = this;
    }
    else {
        int lc = (idx - bboards_left);
        int rc = (bboards_rite - idx);
        bool expand_left = norite || ((lc <= rc) && !noleft);

        if (expand_left) {
            memcpy(&bboards[bboards_left - 1], &bboards[bboards_left], sizeof(PCBillboard) * lc);
            --bboards_left;
            bboards[idx - 1] = this;
        }
        else {
            memcopy_back_dword(&bboards[idx + 1], &bboards[idx], rc);
            ++bboards_rite;
            bboards[idx] = this;
        }
    }
}

void CBillboard::Init(void) {
    DTRACE();

    if (m_VB == NULL) {
        CREATE_VB_DYNAMIC(MAX_BBOARDS * 2 * 4 * sizeof(SBillboardVertex), BILLBOARD_FVF, m_VB);
    }

    if (m_IB == NULL) {
        CREATE_IB16(MAX_BBOARDS * 6, m_IB);

        if (m_IB != NULL) {
            WORD *p;
            LOCK_IB(m_IB, &p);
            for (int i = 0; i < MAX_BBOARDS; ++i) {
                DWORD vol = i * 4;
                *(DWORD *)p = vol | ((vol + 1) << 16);
                *(DWORD *)(p + 2) = (vol + 2) | ((vol + 1) << 16);
                *(DWORD *)(p + 4) = (vol + 3) | ((vol + 2) << 16);

                p += 6;
            }
            UNLOCK_IB(m_IB);
        }
    }
}

void CBillboard::Deinit(void) {
    if (m_VB != NULL)
        DESTROY_VB(m_VB);
    if (m_IB != NULL)
        DESTROY_IB(m_IB);
}

CBillboard::CBillboard(TRACE_PARAM_DEF const D3DXVECTOR3 &pos, float scale, float angle, DWORD color,
                       const SBillboardTexture *tex)
  :
#ifdef _DEBUG
    m_file(_file), m_line(_line),
#endif
    m_Pos(pos), m_Scale(scale), m_Color(color), m_Tex(tex) {
    DTRACE();

    SetIntense(false);
    SetAngle(angle, 0, 0);

#ifdef _DEBUG
    release_called = false;
#endif
}

CBillboard::CBillboard(TRACE_PARAM_DEF const D3DXVECTOR3 &pos, float scale, float angle, DWORD color,
                       CTextureManaged *tex)
  :  // intense
#ifdef _DEBUG
    m_file(_file), m_line(_line),
#endif
    m_Pos(pos), m_Scale(scale), m_Color(color), m_TexIntense(tex) {
    DTRACE();

    SetIntense(true);
    SetAngle(angle, 0, 0);

#ifdef _DEBUG
    release_called = false;
#endif
}

void CBillboard::UpdateVBSlot(SBillboardVertex *vb, const D3DXMATRIX &iview) {
    static const D3DXVECTOR3 base[4] = {
            D3DXVECTOR3(-1, -1, 0),
            D3DXVECTOR3(-1, 1, 0),
            D3DXVECTOR3(1, -1, 0),
            D3DXVECTOR3(1, 1, 0),
    };

    float r11 = m_Rot._11 * m_Scale;
    float r12 = m_Rot._12 * m_Scale;
    float r21 = m_Rot._21 * m_Scale;
    float r22 = m_Rot._22 * m_Scale;

    D3DXMATRIX m(

            (r11 * iview._11 + r12 * iview._21), (r11 * iview._12 + r12 * iview._22),
            (r11 * iview._13 + r12 * iview._23), 0,

            (r21 * iview._11 + r22 * iview._21), (r21 * iview._12 + r22 * iview._22),
            (r21 * iview._13 + r22 * iview._23), 0,

            (iview._31) * m_Scale, (iview._32) * m_Scale, (iview._33) * m_Scale, 0,

            m_Rot._41 * iview._11 + m_Rot._42 * iview._21 + m_Pos.x,
            m_Rot._41 * iview._12 + m_Rot._42 * iview._22 + m_Pos.y,
            m_Rot._41 * iview._13 + m_Rot._42 * iview._23 + m_Pos.z, 1);

    D3DXVec3TransformCoordArray(&vb->p, sizeof(SBillboardVertex), base, sizeof(D3DXVECTOR3), &m, 4);

    if (IsIntense()) {
        vb->color = m_Color;
        vb->tu = 0;
        vb->tv = 1;

        ++vb;

        vb->color = m_Color;
        vb->tu = 0;
        vb->tv = 0;

        ++vb;

        vb->color = m_Color;
        vb->tu = 1;
        vb->tv = 1;

        ++vb;

        vb->color = m_Color;
        vb->tu = 1;
        vb->tv = 0;
    }
    else {
        vb->color = m_Color;
        vb->tu = m_Tex->tu0;
        vb->tv = m_Tex->tv1;

        ++vb;

        vb->color = m_Color;
        vb->tu = m_Tex->tu0;
        vb->tv = m_Tex->tv0;

        ++vb;

        vb->color = m_Color;
        vb->tu = m_Tex->tu1;
        vb->tv = m_Tex->tv1;

        ++vb;

        vb->color = m_Color;
        vb->tu = m_Tex->tu1;
        vb->tv = m_Tex->tv0;
    }
}

void CBillboard::DrawNow(const D3DXMATRIX &iview) {
    if (!IsIntense())
        return;

    ASSERT_DX(g_D3DD->SetFVF(BILLBOARD_FVF));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));

    g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
    SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
    SetColorOpDisable(1);

    SBillboardVertex vb[4];

    UpdateVBSlot(vb, iview);

    m_TexIntense->Preload();
    g_D3DD->SetTexture(0, m_TexIntense->Tex());

    D3DXMATRIX wrld;
    D3DXMatrixIdentity(&wrld);
    g_D3DD->SetTransform(D3DTS_WORLD, &wrld);

    g_D3DD->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &vb, sizeof(SBillboardVertex));

    g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

CBillboardLine::CBillboardLine(TRACE_PARAM_DEF const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float width,
                               DWORD color, CTextureManaged *tex)
  :
#ifdef _DEBUG
    m_file(_file), m_line(_line),
#endif
    CMain(), m_Pos0(pos0), m_Pos1(pos1), m_Width(width), m_Color(color), m_Tex(tex) {

#ifdef _DEBUG
    release_called = false;
#endif
}

CBillboardLine *CBillboardLine::m_First;

void CBillboardLine::UpdateVBSlot(SBillboardVertex *vb, const D3DXVECTOR3 &campos) {
    static const D3DXVECTOR3 base[4] = {
            D3DXVECTOR3(0, 0.5f, 0),
            D3DXVECTOR3(0, -0.5f, 0),
            D3DXVECTOR3(1, 0.5f, 0),
            D3DXVECTOR3(1, -0.5f, 0),
    };

    D3DXMATRIX m_Mat;

    D3DXVECTOR3 dir(m_Pos1 - m_Pos0),
            viewdir(campos.x - (m_Pos0.x + m_Pos1.x) * 0.5f, campos.y - (m_Pos0.y + m_Pos1.y) * 0.5f,
                    campos.z - (m_Pos0.z + m_Pos1.z) * 0.5f);

    m_Mat._11 = dir.x;  // x1
    m_Mat._12 = dir.y;  // y1
    m_Mat._13 = dir.z;  // z1
    m_Mat._14 = 0;

    m_Mat._31 = viewdir.x;  // x2
    m_Mat._32 = viewdir.y;  // y2
    m_Mat._33 = viewdir.z;  // z2
    m_Mat._34 = 0;

    D3DXVECTOR3 left((m_Mat._12 * m_Mat._33 - m_Mat._13 * m_Mat._32), (m_Mat._13 * m_Mat._31 - m_Mat._11 * m_Mat._33),
                     (m_Mat._11 * m_Mat._32 - m_Mat._12 * m_Mat._31));

    D3DXVec3Normalize(&left, &left);

    m_Mat._21 = left.x * m_Width;
    m_Mat._22 = left.y * m_Width;
    m_Mat._23 = left.z * m_Width;
    m_Mat._24 = 0;

    m_Mat._41 = m_Pos0.x;
    m_Mat._42 = m_Pos0.y;
    m_Mat._43 = m_Pos0.z;
    m_Mat._44 = 1;

    D3DXVec3TransformCoordArray(&vb->p, sizeof(SBillboardVertex), base, sizeof(D3DXVECTOR3), &m_Mat, 4);

    vb->color = m_Color;
    vb->tu = 0;
    vb->tv = 1;

    ++vb;

    vb->color = m_Color;
    vb->tu = 0;
    vb->tv = 0;

    ++vb;

    vb->color = m_Color;
    vb->tu = 1;
    vb->tv = 1;

    ++vb;

    vb->color = m_Color;
    vb->tu = 1;
    vb->tv = 0;
}

void CBillboardLine::DrawNow(const D3DXVECTOR3 &campos) {
    ASSERT_DX(g_D3DD->SetFVF(BILLBOARD_FVF));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));

    g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
    SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
    SetColorOpDisable(1);

    SBillboardVertex vb[4];

    UpdateVBSlot(vb, campos);

    m_Tex->Preload();
    g_D3DD->SetTexture(0, m_Tex->Tex());

    D3DXMATRIX wrld;
    D3DXMatrixIdentity(&wrld);
    g_D3DD->SetTransform(D3DTS_WORLD, &wrld);

    g_D3DD->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &vb, sizeof(SBillboardVertex));

    g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
}

void CBillboardLine::AddToDrawQueue(void) {
    CBillboardLine *f = m_First;

    while (f) {
        if (f->m_Tex == this->m_Tex) {
            m_Next = f->m_Next;
            m_NextTex = f->m_NextTex;
            f->m_Next = this;
            return;
        }
        f = f->m_NextTex;
    }

    // no with such tex

    m_Next = m_First;
    m_NextTex = m_First;
    m_First = this;
}
