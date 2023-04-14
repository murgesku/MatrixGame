// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>

#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include <math.h>

#include "MatrixEffectBillboard.hpp"

D3D_VB CMatrixEffectBillboard::m_VB;
int CMatrixEffectBillboard::m_VB_ref;

CMatrixEffectBillboard::CMatrixEffectBillboard(const D3DXVECTOR3 &pos, float radius1, float radius2, DWORD c1, DWORD c2,
                                               float ttl, float delay, const wchar *tex, const D3DXVECTOR3 &dir,
                                               ADD_TAKT addtakt)
  : CMatrixEffect(), m_Radius1(radius1), m_Radius2(radius2), m_Color1(c1), m_Color2(c2), m_TTL(ttl), m_TTL2(ttl),
    _m_TTL(INVERT(ttl)), m_Delay(delay), m_Dir(dir), m_AddTaktHandler(addtakt) {
    DTRACE();

    m_EffectType = EFFECT_BILLBOARD;

    m_Intense = true;

    m_Tex = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, tex);
    m_Tex->RefInc();

    CreateGeometry();

    m_Mat = g_MatrixMap->GetIdentityMatrix();
    m_Mat._41 = pos.x;
    m_Mat._42 = pos.y;
    m_Mat._43 = pos.z;

    UpdateData();
}

bool CMatrixEffectBillboard::PrepareDX(void) {
    DTRACE();
    if (m_VB)
        return true;

    SBBoardVertex *v;
    CREATE_VB(4 * sizeof(SBBoardVertex), BBOARD_FVF, m_VB);
    if (m_VB == NULL)
        return false;

    LOCK_VB(m_VB, &v);

    v[1].p.x = -1;
    v[1].p.y = 1;
    v[1].p.z = 0;
    v[1].tu = 0;
    v[1].tv = 0;

    v[0].p.x = -1;
    v[0].p.y = -1;
    v[0].p.z = 0;
    v[0].tu = 0;
    v[0].tv = 1;

    v[3].p.x = 1;
    v[3].p.y = 1;
    v[3].p.z = 0;
    v[3].tu = 1;
    v[3].tv = 0;

    v[2].p.x = 1;
    v[2].p.y = -1;
    v[2].p.z = 0;
    v[2].tu = 1;
    v[2].tv = 1;

    UNLOCK_VB(m_VB);
    return true;
}

void CMatrixEffectBillboard::CreateGeometry(void) {
    PrepareDX();
    ++m_VB_ref;
}

void CMatrixEffectBillboard::UpdateData(void) {
    float k = 1.0f - (m_TTL * _m_TTL);

    m_Color = LIC(m_Color1, m_Color2, k);

    float scale = m_Radius1 + (m_Radius2 - m_Radius1) * k * 0.5f;

    VecToMatrixX(m_Mat, *(D3DXVECTOR3 *)&m_Mat._41, m_Dir);

    m_Mat._11 *= scale;
    m_Mat._12 *= scale;
    m_Mat._13 *= scale;
    m_Mat._21 *= scale;
    m_Mat._22 *= scale;
    m_Mat._23 *= scale;
    m_Mat._31 *= scale;
    m_Mat._32 *= scale;
    m_Mat._33 *= scale;
}

void CMatrixEffectBillboard::ReleaseGeometry(void) {
    if ((--m_VB_ref) == 0) {
        if (IS_VB(m_VB)) {
            DESTROY_VB(m_VB);
        }
    }
}

CMatrixEffectBillboard::~CMatrixEffectBillboard() {
    DTRACE();
    ReleaseGeometry();
    m_Tex->RefDecUnload();
}

void CMatrixEffectBillboard::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectBillboard, this, m_Heap);
}

void CMatrixEffectBillboard::Draw(const D3DXMATRIX &m, DWORD color, CTextureManaged *tex, bool intense) {
    DTRACE();

    if (!IS_VB(m_VB))
        return;

    ASSERT_DX(g_D3DD->SetFVF(BBOARD_FVF));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));

    if (intense)
        g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m));

    ASSERT_DX(g_D3DD->SetTexture(0, tex->Tex()));

    g_D3DD->SetStreamSource(0, GET_VB(m_VB), 0, sizeof(SBBoardVertex));
    g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, color);

    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);

    SetColorOpDisable(1);

    g_D3DD->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

    if (intense)
        g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void CMatrixEffectBillboard::Draw(void) {
    DTRACE();
    Draw(m_Mat, m_Color, m_Tex, m_Intense != 0);
}

void CMatrixEffectBillboard::Takt(float step) {
    DTRACE();

    if (m_Delay > 0) {
        m_Delay -= step;
    }
    if (m_Delay < 0) {
        m_TTL += m_Delay;
        m_Delay = 0;
    }
    else if (m_Delay > 0)
        return;

    m_TTL -= step;
    if (m_TTL < 0) {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        g_MatrixMap->SubEffect(this);
#endif
        return;
    }

    if (m_AddTaktHandler) {
        m_AddTaktHandler(this, step);
    }

    UpdateData();
}

void CMatrixEffectBillboard::Takt2(float step) {
    DTRACE();

    m_TTL -= step;
    if (m_TTL < 0) {
        m_TTL = m_TTL2;
    }

    UpdateData();
}

int CMatrixEffectBillboard::Priority(void) {
    return g_MatrixMap->m_Camera.IsInFrustum(*(D3DXVECTOR3 *)&m_Mat._41) ? (10) : (0);
}

//////////////////////////////////////////////////////////

CMatrixEffectBillboardLine::CMatrixEffectBillboardLine(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float width,
                                                       DWORD c1, DWORD c2, float ttl, CTextureManaged *tex)
  : CMatrixEffect(), m_Color1(c1), m_Color2(c2), m_TTL(ttl), _m_TTL(INVERT(ttl)),
    m_Bl(TRACE_PARAM_CALL pos0, pos1, width, c1, tex) {
    DTRACE();
    m_EffectType = EFFECT_BILLBOARDLINE;
}

CMatrixEffectBillboardLine::~CMatrixEffectBillboardLine() {
    DTRACE();
    // DM("bl","destr");
    m_Bl.Release();
}

void CMatrixEffectBillboardLine::Release(void) {
    DTRACE();

    // DM("bl","release");
    SetDIP();
    HDelete(CMatrixEffectBillboardLine, this, m_Heap);
}

void CMatrixEffectBillboardLine::Draw(void) {
    DTRACE();

    // DM("bl","draw");
    m_Bl.AddToDrawQueue();
}

void CMatrixEffectBillboardLine::Takt(float step) {
    DTRACE();

    // DM("bl","takt");

    m_TTL -= step;
    if (m_TTL < 0) {
        // DM("bl","takt_sub");
        // zzz = 1;
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        g_MatrixMap->SubEffect(this);
#endif
        return;
    }

    float k = 1.0f - (m_TTL * _m_TTL);
    m_Bl.SetColor(LIC(m_Color1, m_Color2, k));
}

int CMatrixEffectBillboardLine::Priority(void) {
    bool inf = g_MatrixMap->m_Camera.IsInFrustum(m_Bl.GetPos0());
    if (inf) {
        return 20;
    }

    return g_MatrixMap->m_Camera.IsInFrustum(m_Bl.GetPos0()) ? (20) : (5);
}

//////////////////////////////////////////////////////////

CMatrixEffectBillboardScore::CMatrixEffectBillboardScore(const wchar *n, const D3DXVECTOR3 &pos, DWORD color)
  : m_Pos(pos), m_Color(color) {
    m_EffectType = EFFECT_SCORE;

    m_bbs_count = std::wcslen(n);
    m_bbs = (SBB *)HAlloc(sizeof(SBB) * m_bbs_count, m_Heap);

    m_TTL = BBS_TTL;

    D3DXVECTOR2 disp(float(-BBS_DX / 2 * m_bbs_count), 0);

    int addw = 0;
    for (int i = 0; i < m_bbs_count; ++i) {
        float scale = 6;

        const SBillboardTexture *tex = GetBBTex(BBT_SCOREPLUS);
        {
            m_Color = 0xFFFFFFFF;
            scale = 2.5f;
        }
        if (n[i] == '0') {
            tex = GetBBTex(BBT_SCORE0);
            m_Color = 0xFFFFFFFF;
            scale = 2.5f;
            disp.x += BBS_DX + addw;
            addw = 0;
        }
        else if (n[i] == '1') {
            tex = GetBBTex(BBT_SCORE1);
            m_Color = 0xFFFFFFFF;
            scale = 2.5f;
            disp.x += BBS_DX + addw - 3;
            addw = 0;
        }
        else if (n[i] == '2') {
            tex = GetBBTex(BBT_SCORE2);
            m_Color = 0xFFFFFFFF;
            scale = 2.5f;
            disp.x += BBS_DX + addw;
            addw = 0;
        }
        else if (n[i] == '3') {
            tex = GetBBTex(BBT_SCORE3);
            m_Color = 0xFFFFFFFF;
            scale = 2.5f;
            disp.x += BBS_DX + addw;
            addw = 0;
        }
        else if (n[i] == '4') {
            tex = GetBBTex(BBT_SCORE4);
            m_Color = 0xFFFFFFFF;
            scale = 2.5f;
            disp.x += BBS_DX + addw;
            addw = 0;
        }
        else if (n[i] == '5') {
            tex = GetBBTex(BBT_SCORE5);
            m_Color = 0xFFFFFFFF;
            scale = 2.5f;
            disp.x += BBS_DX + addw;
            addw = 0;
        }
        else if (n[i] == '6') {
            tex = GetBBTex(BBT_SCORE6);
            m_Color = 0xFFFFFFFF;
            scale = 2.5f;
            disp.x += BBS_DX + addw;
            addw = 0;
        }
        else if (n[i] == '7') {
            tex = GetBBTex(BBT_SCORE7);
            m_Color = 0xFFFFFFFF;
            scale = 2.5f;
            disp.x += BBS_DX + addw;
            addw = 0;
        }
        else if (n[i] == '8') {
            tex = GetBBTex(BBT_SCORE8);
            m_Color = 0xFFFFFFFF;
            scale = 2.5f;
            disp.x += BBS_DX + addw;
            addw = 0;
        }
        else if (n[i] == '9') {
            tex = GetBBTex(BBT_SCORE9);
            m_Color = 0xFFFFFFFF;
            scale = 2.5f;
            disp.x += BBS_DX + addw;
            addw = 0;
        }
        else if (n[i] == 'e') {
            tex = GetBBTex(BBT_SCOREICON1);
            m_Color = 0xFFFFFFFF;
            scale = 6;
            disp.x += BBS_ICON_DX;
            addw = 3;
        }
        else if (n[i] == 'p') {
            tex = GetBBTex(BBT_SCOREICON2);
            m_Color = 0xFFFFFFFF;
            scale = 6;
            disp.x += BBS_ICON_DX;
            addw = 3;
        }
        else if (n[i] == 'b') {
            tex = GetBBTex(BBT_SCOREICON3);
            m_Color = 0xFFFFFFFF;
            scale = 6;
            disp.x += BBS_ICON_DX;
            addw = 3;
        }
        else if (n[i] == 't') {
            tex = GetBBTex(BBT_SCOREICON4);
            m_Color = 0xFFFFFFFF;
            scale = 6;
            disp.x += BBS_ICON_DX;
            addw = 3;
        }
        else if (n[i] == 'a') {
            tex = GetBBTex(BBT_SCOREICONS);
            m_Color = 0xFFFFFFFF;
            scale = 12;
            disp.x += BBS_ICON_DX * 2;
            addw = 10;
        }

        new(&m_bbs[i].bb) CBillboard(TRACE_PARAM_CALL m_Pos, scale, 0, m_Color, tex);
        m_bbs[i].bb.DisplaceTo(disp);

        m_bbs[i].disp = disp.x;
    }
}

CMatrixEffectBillboardScore::~CMatrixEffectBillboardScore() {
    for (int i = 0; i < m_bbs_count; ++i) {
        m_bbs[i].bb.Release();
    }
    HFree(m_bbs, m_Heap);
}

void CMatrixEffectBillboardScore::Draw(void) {
    for (int i = 0; i < m_bbs_count; ++i) {
        m_bbs[i].bb.Sort(g_MatrixMap->m_Camera.GetViewMatrix());
    }
}

void CMatrixEffectBillboardScore::Takt(float step) {
    m_TTL -= step;
    if (m_TTL < 0) {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        g_MatrixMap->SubEffect(this);
#endif
        return;
    }

    float k = (1.0f - m_TTL / BBS_TTL);
    k = KSCALE(k, BBS_FADE, 1);
    DWORD c = LIC(m_Color, (m_Color & 0x00FFFFFF), k);

    for (int i = 0; i < m_bbs_count; ++i) {
        m_bbs[i].bb.m_Pos.z += step * BBS_SPEED;
        m_bbs[i].bb.SetColor(c);
    }
}
void CMatrixEffectBillboardScore::Release(void) {
    SetDIP();
    HDelete(CMatrixEffectBillboardScore, this, m_Heap);
}
