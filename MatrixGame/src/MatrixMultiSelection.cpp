// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixMultiSelection.hpp"
#include "MatrixMap.hpp"
#include "MatrixRobot.hpp"
#include "MatrixFlyer.hpp"
#include "MatrixMapStatic.hpp"

CMultiSelection *CMultiSelection::m_GameSelection;
CMultiSelection *CMultiSelection::m_First;
CMultiSelection *CMultiSelection::m_Last;
int CMultiSelection::m_Time;

#define MS_RAMKA_COLOR  0xFF00FF00
#define MS_RAMKA_COLOR2 0x80000000
//#define MS_SELBACK_COLOR    0x4AFFFFFF

#define MS_Z 0

CMultiSelection::CMultiSelection(const Base::CPoint &pos) : m_LT(pos), m_RB(pos), m_Flags(0) {
    // m_Tex = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_SELBACK);
    // m_Tex->MipmapOff();
    // m_Tex->Preload();
}

CMultiSelection *CMultiSelection::Begin(const Base::CPoint &pos) {
    if (g_MatrixMap->IsPaused())
        return NULL;
    CMultiSelection *s = HNew(g_MatrixHeap) CMultiSelection(pos);
    LIST_ADD(s, m_First, m_Last, m_Prev, m_Next);
    return s;
}

bool CMultiSelection::DrawPass1(void) {
    if (FLAG(m_Flags, MS_FLAG_DIP))
        return false;

    // D3DXVECTOR4  v[4];
    // v[0] = D3DXVECTOR4( float(m_LT.x), float(m_RB.y), 1.0f, 1.0f );
    // v[1] = D3DXVECTOR4( float(m_LT.x), float(m_LT.y), 1.0f, 1.0f );
    // v[2] = D3DXVECTOR4( float(m_RB.x), float(m_RB.y), 1.0f, 1.0f );
    // v[3] = D3DXVECTOR4( float(m_RB.x), float(m_LT.y), 1.0f, 1.0f );

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILENABLE, TRUE));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
    g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILREF, 0x1));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILMASK, 0xffffffff));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE));

    D3DRECT r;
    r.x1 = m_LT.x;
    r.y1 = m_LT.y;
    r.x2 = m_RB.x;
    r.y2 = m_RB.y;
    if (r.x1 > r.x2) {
        r.x1 ^= r.x2;
        r.x2 ^= r.x1;
        r.x1 ^= r.x2;
    }
    if (r.y1 > r.y2) {
        r.y1 ^= r.y2;
        r.y2 ^= r.y1;
        r.y1 ^= r.y2;
    }
    ASSERT_DX(g_D3DD->Clear(1, &r, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL | D3DCLEAR_TARGET, 0, 1.0f, 1));

    // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_COLORWRITEENABLE, 0xf));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP));

    return true;
}

void CMultiSelection::DrawPass2(void) {
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_ZERO));
    g_D3DD->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
}

void CMultiSelection::DrawPassEnd(void) {
    g_D3DD->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILENABLE, FALSE));
}

void CMultiSelection::Draw(void) {
    DWORD ramkacolor = MS_RAMKA_COLOR;
    DWORD ramkacolor2 = MS_RAMKA_COLOR2;
    // DWORD backcolor = MS_SELBACK_COLOR;
    if (FLAG(m_Flags, MS_FLAG_DIP)) {
        int cdt = (m_Time - m_TimeBeforeDip);
        if (cdt > MS_DIP_TIME) {
            HDelete(CMultiSelection, this, g_MatrixHeap);
            return;
        }

        float k = float(cdt) * INVERT(MS_DIP_TIME);

        ramkacolor = LIC(MS_RAMKA_COLOR, (MS_RAMKA_COLOR & 0x00FFFFFF), k);
        ramkacolor2 = LIC(MS_RAMKA_COLOR2, (MS_RAMKA_COLOR2 & 0x00FFFFFF), k);
        // backcolor = LIC(MS_SELBACK_COLOR, (MS_SELBACK_COLOR & 0x00FFFFFF), k);
    }

    //    g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, backcolor);

    // SetColorOpAnyOrder(0,D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    // SetAlphaOpAnyOrder(0,D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    // SetColorOpDisable(1);

    // g_D3DD->SetRenderState(D3DRS_DESTBLEND,  D3DBLEND_ONE  );

    // float ll_x = 0.0f;
    // float ll_y = 0.0f;
    // float rr_x = float(m_RB.x - m_LT.x) * INVERT(128.0f);
    // float rr_y = float(m_RB.y - m_LT.y) * INVERT(128.0f);

    // float dx = (float(g_MatrixMap->GetTime() & 4095) * INVERT(4096.0f)) + rr_x * 0.5f;
    // ll_x -= dx;
    // rr_x -= dx;

    // float dy = (float(g_MatrixMap->GetTime() & 8191) * INVERT(8192.0f)) + rr_y * 0.5f;
    // ll_y += dy;
    // rr_y += dy;

    D3DXVECTOR4 v2[5];
    v2[0] = D3DXVECTOR4(float(m_LT.x), float(m_LT.y), MS_Z, 1.0f);
    v2[1] = D3DXVECTOR4(float(m_RB.x), float(m_LT.y), MS_Z, 1.0f);
    v2[2] = D3DXVECTOR4(float(m_RB.x), float(m_RB.y), MS_Z, 1.0f);
    v2[3] = D3DXVECTOR4(float(m_LT.x), float(m_RB.y), MS_Z, 1.0f);
    v2[4] = v2[0];

    g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, ramkacolor);

    SetColorOpSelect(0, D3DTA_TFACTOR);
    SetAlphaOpSelect(0, D3DTA_TFACTOR);
    SetColorOpDisable(1);

    g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    g_D3DD->SetFVF(D3DFVF_XYZRHW);

    g_D3DD->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &v2, sizeof(D3DXVECTOR4));

    g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, ramkacolor2);

    v2[0].x -= 1.0f;
    v2[0].y -= 1.0f;

    v2[1].x += 1.0f;
    v2[1].y -= 1.0f;

    v2[2].x += 1.0f;
    v2[2].y += 1.0f;

    v2[3].x -= 1.0f;
    v2[3].y += 1.0f;

    v2[4] = v2[0];

    g_D3DD->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &v2, sizeof(D3DXVECTOR4));

    v2[0].x += 2.0f;
    v2[0].y += 2.0f;

    v2[1].x -= 2.0f;
    v2[1].y += 2.0f;

    v2[2].x -= 2.0f;
    v2[2].y -= 2.0f;

    v2[3].x += 2.0f;
    v2[3].y -= 2.0f;

    v2[4] = v2[0];

    g_D3DD->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &v2, sizeof(D3DXVECTOR4));
}

void CMultiSelection::Update(const Base::CPoint &pos, DWORD mask, SELECT_ENUM callback, DWORD param) {
    m_RB = pos;

    CRect r(m_LT.x, m_LT.y, m_RB.x, m_RB.y);
    r.Normalize();

    bool only_one = (abs(r.right - r.left) * abs(r.bottom - r.top)) < 9;

    RemoveSelItems();
    int n = CMatrixMapStatic::GetVisObjCnt();
    for (int i = 0; i < n; ++i) {
        CMatrixMapStatic *o = CMatrixMapStatic::GetVisObj(i);

        if (o->GetSide() != PLAYER_SIDE)
            continue;

        if ((o->GetObjectType() == OBJECT_TYPE_MAPOBJECT) && ((mask & TRACE_OBJECT) == 0))
            continue;
        if (o->IsRobot()) {
            if (!o->IsLiveRobot())
                continue;
            if ((mask & TRACE_ROBOT) == 0)
                continue;
            if (o->AsRobot()->IsCrazy())
                continue;
        }
        if (o->IsBuilding()) {
            if (!o->IsLiveBuilding())
                continue;
            if ((mask & TRACE_BUILDING) == 0)
                continue;
        }
        if (o->IsCannon()) {
            if (!o->IsLiveCannon())
                continue;
            if ((mask & TRACE_CANNON) == 0)
                continue;
        }
        if ((o->GetObjectType() == OBJECT_TYPE_FLYER) && ((mask & TRACE_FLYER) == 0))
            continue;

        if (o->InRect(r)) {
            if (o->IsRobot()) {
                if (!FLAG(m_Flags, MS_FLAG_ROBOTS)) {
                    // no robots... clear all!
                    RemoveSelItems();
                }
                SETFLAG(m_Flags, MS_FLAG_ROBOTS);
            }
            else if (o->IsBuilding()) {
                if (FLAG(m_Flags, MS_FLAG_ROBOTS))
                    continue;  // do not add buildings if there are some robots present
                SETFLAG(m_Flags, MS_FLAG_BUILDINGS);
            }

            if (m_SelItems.size() < 9) {
                m_SelItems.push_back(o);
                if (o->IsRobot()) {
                    if (!o->AsRobot()->IsSelected()) {
                        o->AsRobot()->SelectByGroup();
                    }
                }
            }

            // callback(o, param);
            if (only_one && o->IsRobot())
                break;
        }
    }
}

void CMultiSelection::End(bool add_to_selection) {
    SETFLAG(m_Flags, MS_FLAG_DIP);
    m_TimeBeforeDip = g_MatrixMap->GetTime();

    CRect r(m_LT.x, m_LT.y, m_RB.x, m_RB.y);
    r.Normalize();

    if (!m_SelItems.empty() && add_to_selection) {
        CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();

        for (auto item : m_SelItems)
        {
            ps->GetCurSelGroup()->AddObject(item, -4);
        }
    }

    RemoveSelItems();
}

bool CMultiSelection::FindItem(const CMatrixMapStatic *o)
{
    for (auto item : m_SelItems)
    {
        if (o == item)
        {
            return true;
        }
    }

    return false;
}

void CMultiSelection::Remove(const CMatrixMapStatic *o)
{
    auto item = std::ranges::find(m_SelItems, o);
    if (item != m_SelItems.end())
    {
        m_SelItems.erase(item);
    }
}
