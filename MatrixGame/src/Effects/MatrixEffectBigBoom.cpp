// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include "../MatrixObject.hpp"
#include "../MatrixObjectRobot.hpp"
#include "../MatrixObjectCannon.hpp"
#include "../MatrixFlyer.hpp"
#include "../MatrixRobot.hpp"
#include <math.h>

#include "MatrixEffectBigBoom.hpp"
#include "MatrixEffectPointLight.hpp"

CTextureManaged *CMatrixEffectBigBoom::m_Tex;
D3D_VB CMatrixEffectBigBoom::m_VB;
D3D_IB CMatrixEffectBigBoom::m_IB;
int CMatrixEffectBigBoom::m_VB_ref;

CMatrixEffectBigBoom::CMatrixEffectBigBoom(const D3DXVECTOR3 &pos, float radius, float ttl, DWORD hitmask,
                                           CMatrixMapStatic *skip, DWORD user, FIRE_END_HANDLER handler, DWORD light)
  : CMatrixEffect(), m_Radius(radius), m_TTL(ttl), _m_TTL(INVERT(ttl)), m_User(user), m_Handler(handler),
    m_hitmask(hitmask), m_skip(skip)
#ifdef _DEBUG
    ,
    m_Light(DEBUG_CALL_INFO)
#endif
{
    DTRACE();

    m_EffectType = EFFECT_BIG_BOOM;
    m_Color = 0xFFFFFFFF;

    if (light != 0) {
        CMatrixEffect::CreatePointLight(&m_Light, pos, 1, light, false);
    }

    m_Mat = g_MatrixMap->GetIdentityMatrix();
    m_Mat._11 = 1;
    m_Mat._22 = 1;
    m_Mat._33 = 1;

    m_Mat._41 = pos.x;
    m_Mat._42 = pos.y;
    m_Mat._43 = pos.z;

    m_k1 = FSRND(3);
    m_k2 = FSRND(3);
    m_k3 = FSRND(3);

    if (m_VB_ref == 0) {
        m_Tex = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_BIGBOOM);
        m_Tex->RefInc();
    }
    ++m_VB_ref;
}

bool CMatrixEffectBigBoom::PrepareDX(void) {
    DTRACE();

    if (m_VB || m_IB)
        return true;

    struct STri {
        WORD i1;
        WORD i2;
        WORD i3;
    } m_Tris[BB_TRI_CNT];

    SBBVertex m_Pts[BB_PTS_CNT];

    CREATE_VB(sizeof(m_Pts), BB_FVF, m_VB);
    if (!IS_VB(m_VB))
        return false;
    CREATE_IB16(sizeof(m_Tris), m_IB);
    if (!IS_IB(m_IB)) {
        DESTROY_VB(m_VB);
        return false;
    }

    int m_npts = 5;
    int m_ntris = 6;
    m_Pts[0].p = D3DXVECTOR3(0, 0, 1);

    m_Pts[1].p = D3DXVECTOR3(1, 0, 0);
    m_Pts[2].p = D3DXVECTOR3(-0.5f, 0.86602539f, 0);
    m_Pts[3].p = D3DXVECTOR3(-0.5f, -0.86602539f, 0);

    m_Pts[4].p = D3DXVECTOR3(0, 0, -1);

    m_Tris[0].i1 = 0;
    m_Tris[0].i2 = 1;
    m_Tris[0].i3 = 2;
    m_Tris[1].i1 = 0;
    m_Tris[1].i2 = 1;
    m_Tris[1].i3 = 3;
    m_Tris[2].i1 = 0;
    m_Tris[2].i2 = 2;
    m_Tris[2].i3 = 3;

    m_Tris[3].i1 = 4;
    m_Tris[3].i2 = 1;
    m_Tris[3].i3 = 2;
    m_Tris[4].i1 = 4;
    m_Tris[4].i2 = 1;
    m_Tris[4].i3 = 3;
    m_Tris[5].i1 = 4;
    m_Tris[5].i2 = 2;
    m_Tris[5].i3 = 3;

    int pass = BB_PASS_CNT;
    int newntris = m_ntris;
    while (pass-- > 0) {
        for (int i = 0; i < m_ntris; i++) {
            D3DXVECTOR3 dir(
                    (m_Pts[m_Tris[i].i1].p.x + m_Pts[m_Tris[i].i2].p.x + m_Pts[m_Tris[i].i3].p.x) * (1.0f / 3.0f),
                    (m_Pts[m_Tris[i].i1].p.y + m_Pts[m_Tris[i].i2].p.y + m_Pts[m_Tris[i].i3].p.y) * (1.0f / 3.0f),
                    (m_Pts[m_Tris[i].i1].p.z + m_Pts[m_Tris[i].i2].p.z + m_Pts[m_Tris[i].i3].p.z) * (1.0f / 3.0f));
            D3DXVec3Normalize(&m_Pts[m_npts].p, &dir);

            D3DXVECTOR3 dir12((m_Pts[m_Tris[i].i1].p.x + m_Pts[m_Tris[i].i2].p.x) * (0.5f),
                              (m_Pts[m_Tris[i].i1].p.y + m_Pts[m_Tris[i].i2].p.y) * (0.5f),
                              (m_Pts[m_Tris[i].i1].p.z + m_Pts[m_Tris[i].i2].p.z) * (0.5f));
            D3DXVec3Normalize(&m_Pts[m_npts + 1].p, &dir12);

            D3DXVECTOR3 dir13((m_Pts[m_Tris[i].i1].p.x + m_Pts[m_Tris[i].i3].p.x) * (0.5f),
                              (m_Pts[m_Tris[i].i1].p.y + m_Pts[m_Tris[i].i3].p.y) * (0.5f),
                              (m_Pts[m_Tris[i].i1].p.z + m_Pts[m_Tris[i].i3].p.z) * (0.5f));
            D3DXVec3Normalize(&m_Pts[m_npts + 2].p, &dir13);

            D3DXVECTOR3 dir23((m_Pts[m_Tris[i].i2].p.x + m_Pts[m_Tris[i].i3].p.x) * (0.5f),
                              (m_Pts[m_Tris[i].i2].p.y + m_Pts[m_Tris[i].i3].p.y) * (0.5f),
                              (m_Pts[m_Tris[i].i2].p.z + m_Pts[m_Tris[i].i3].p.z) * (0.5f));
            D3DXVec3Normalize(&m_Pts[m_npts + 3].p, &dir23);

            m_Tris[newntris].i1 = (WORD)m_npts;
            m_Tris[newntris].i2 = (WORD)m_Tris[i].i1;
            m_Tris[newntris].i3 = (WORD)(m_npts + 1);

            m_Tris[newntris + 1].i1 = (WORD)m_npts;
            m_Tris[newntris + 1].i2 = (WORD)(m_npts + 1);
            m_Tris[newntris + 1].i3 = (WORD)(m_Tris[i].i2);

            m_Tris[newntris + 2].i1 = (WORD)m_npts;
            m_Tris[newntris + 2].i2 = (WORD)(m_npts + 3);
            m_Tris[newntris + 2].i3 = (WORD)m_Tris[i].i3;

            m_Tris[newntris + 3].i1 = (WORD)m_npts;
            m_Tris[newntris + 3].i2 = (WORD)m_Tris[i].i3;
            m_Tris[newntris + 3].i3 = (WORD)(m_npts + 2);

            m_Tris[newntris + 4].i1 = (WORD)m_npts;
            m_Tris[newntris + 4].i2 = (WORD)(m_npts + 2);
            m_Tris[newntris + 4].i3 = (WORD)m_Tris[i].i1;

            m_Tris[i].i1 = (WORD)m_npts;
            m_Tris[i].i3 = (WORD)(m_npts + 3);

            newntris += 5;
            m_npts += 4;
        }
        m_ntris = newntris;
    }

    for (int i = 0; i < BB_PTS_CNT; ++i) {
        m_Pts[i].x = m_Pts[i].p.x;
        m_Pts[i].y = m_Pts[i].p.y;
        m_Pts[i].z = m_Pts[i].p.z;
    }

    void *pbuf;
    LOCK_VB(m_VB, &pbuf);
    memcpy(pbuf, &m_Pts, sizeof(m_Pts));
    UNLOCK_VB(m_VB);

    LOCK_IB(m_IB, &pbuf);
    memcpy(pbuf, &m_Tris, sizeof(m_Tris));
    UNLOCK_IB(m_IB);

    return true;
}

CMatrixEffectBigBoom::~CMatrixEffectBigBoom() {
    DTRACE();
    if ((--m_VB_ref) == 0) {
        if (IS_VB(m_VB)) {
            DESTROY_VB(m_VB);
        }
        if (IS_VB(m_IB)) {
            DESTROY_IB(m_IB);
        }
        m_Tex->RefDecUnload();
        m_Tex = NULL;
    }

    if (m_Light.effect) {
        ((CMatrixEffectPointLight *)m_Light.effect)->Kill(1000);
        m_Light.Unconnect();
    }

    if (m_Handler)
        m_Handler(TRACE_STOP_NONE, D3DXVECTOR3(m_Mat._41, m_Mat._42, WATER_LEVEL), m_User, FEHF_LASTHIT);
}

void CMatrixEffectBigBoom::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectBigBoom, this, m_Heap);
}

void CMatrixEffectBigBoom::BeforeDraw(void) {
    DTRACE();
    if (FLAG(m_Flags, BIGBOOMF_PREPARED))
        return;
    m_Tex->Preload();
    SETFLAG(m_Flags, BIGBOOMF_PREPARED);
    PrepareDX();
};

void CMatrixEffectBigBoom::Draw(void) {
    DTRACE();

    if (!IS_VB(m_VB) || !IS_IB(m_IB))
        return;

    RESETFLAG(m_Flags, BIGBOOMF_PREPARED);
    ASSERT_DX(g_D3DD->SetFVF(BB_FVF));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));

    g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

    ASSERT_DX(g_D3DD->SetTexture(0, m_Tex->Tex()));

    g_D3DD->SetStreamSource(0, GET_VB(m_VB), 0, sizeof(SBBVertex));
    g_D3DD->SetIndices(GET_IB(m_IB));
    g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, m_Color);

    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);

    SetColorOpDisable(1);

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3));
    // ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));
    // ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_SPHEREMAP ));
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_TEXTURE0, &m_TMat));

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m_Mat));
    g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, BB_PTS_CNT, 0, BB_TRI_CNT);

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
    // ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0 ));

    g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

bool CMatrixEffectBigBoom::BoomEnum(const D3DXVECTOR3 &center, CMatrixMapStatic *ms, DWORD user) {
    CMatrixEffectBigBoom *boom = (CMatrixEffectBigBoom *)user;
    D3DXVECTOR3 anorm;
    auto tmp = center - ms->GetGeoCenter();
    D3DXVec3Normalize(&anorm, &tmp);
    boom->m_Handler(ms, ms->GetGeoCenter() + anorm * ms->GetRadius(), boom->m_User, 0);
    return true;
}

bool CMatrixEffectBigBoom::BoomEnumNaklon(const D3DXVECTOR3 &center, CMatrixMapStatic *ms, DWORD user) {
    CMatrixEffectBigBoom *boom = (CMatrixEffectBigBoom *)user;
    if (ms->IsLiveRobot()) {
        D3DXVECTOR3 anorm;
        auto tmp = ms->GetGeoCenter() - center;
        D3DXVec3Normalize(&anorm, &tmp);
        ms->AsRobot()->ApplyNaklon(anorm * 0.5f);
    }
    return true;
}

void CMatrixEffectBigBoom::Takt(float step) {
    DTRACE();

    m_TTL -= step;
    if (m_TTL < 0) {
        if (m_Handler) {
            if (m_hitmask & TRACE_WATER) {
                if ((m_Mat._43 - m_Radius) <= WATER_LEVEL) {
                    m_Handler(TRACE_STOP_WATER, D3DXVECTOR3(m_Mat._41, m_Mat._42, WATER_LEVEL), m_User, 0);
                }
            }
            if (m_hitmask & TRACE_LANDSCAPE) {
                float z = g_MatrixMap->GetZ(m_Mat._41, m_Mat._42);
                if ((m_Mat._43 - m_Radius) <= z) {
                    m_Handler(TRACE_STOP_LANDSCAPE, D3DXVECTOR3(m_Mat._41, m_Mat._42, z), m_User, 0);
                }
            }
            if (m_hitmask & TRACE_ANYOBJECT) {
                g_MatrixMap->FindObjects(*(D3DXVECTOR3 *)&m_Mat._41, m_Radius, 1, m_hitmask, m_skip, BoomEnum,
                                         (DWORD)this);
            }
        }
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        g_MatrixMap->SubEffect(this);
#endif
        return;
    }

    if (m_Handler) {
        if (m_hitmask & TRACE_ANYOBJECT) {
            g_MatrixMap->FindObjects(*(D3DXVECTOR3 *)&m_Mat._41, m_Radius, 1, m_hitmask, m_skip, BoomEnumNaklon,
                                     (DWORD)this);
        }
    }

    float k = (m_TTL * _m_TTL);

    if (FLAG(g_MatrixMap->m_Flags, MMFLAG_AUTOMATIC_MODE)) {
        if (k < 0.5f) {
            g_MatrixMap->m_Camera.Stat();
        }
    }

    int a = Float2Int(k * 512);
    if (a > 255)
        a = 255;

    m_Color = (m_Color & 0x00FFFFFF) | (a << 24);

    float scale = m_Radius - (m_Radius * k);

    D3DXMATRIX m, m1;
    D3DXMatrixRotationYawPitchRoll(&m, k * m_k1, k * m_k2, k * m_k3);
    D3DXMatrixScaling(&m1, k * 2, k * 2, k * 2);
    m_TMat = m * m1;

    m_Mat._11 = scale;
    m_Mat._22 = scale;
    m_Mat._33 = scale;

    if (m_Light.effect) {
        ((CMatrixEffectPointLight *)m_Light.effect)->SetRadius(scale);
    }
}
