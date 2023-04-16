// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>

#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include <math.h>

#include "MatrixEffectPointLight.hpp"

SPL_VBIB *SPL_VBIB::m_FirstFree;
SPL_VBIB *SPL_VBIB::m_LastFree;
SPL_VBIB *SPL_VBIB::m_FirstAll;
SPL_VBIB *SPL_VBIB::m_LastAll;

static bool UnloadDX(uintptr_t user) {
    auto r = reinterpret_cast<SPL_VBIB*>(user);
    r->Release();
    return true;  // dead
}

void *SPL_VBIB::LockVB(int vbsize) {
    if (vbsize > m_VBSize) {
        const int add = 2 * sizeof(SPointLightVertex) * sizeof(SPointLightVertexV);
        if (IS_VB(m_VB))
            DESTROY_VB(m_VB);
        CREATE_VB_DYNAMIC(vbsize + add, m_FVF, m_VB);
        m_VBSize = vbsize + add;
    }
    if (!IS_VB(m_VB))
        return NULL;

    void *buf;
    LOCKP_VB_DYNAMIC(m_VB, 0, vbsize, &buf);
    return buf;
}
void *SPL_VBIB::LockIB(int ib_size) {
    if (ib_size > m_IBSize) {
        if (IS_IB(m_IB))
            DESTROY_IB(m_IB);
        CREATE_IBD16((ib_size + 256), m_IB);
        m_IBSize = (ib_size + 256);
    }

    if (!IS_IB(m_IB))
        return NULL;

    void *buf;
    LOCKPD_IB(m_IB, 0, ib_size, &buf);
    return buf;
}

void SPL_VBIB::Release(void) {
    LIST_DEL(this, m_FirstAll, m_LastAll, m_PrevAll, m_NextAll);
    LIST_DEL(this, m_FirstFree, m_LastFree, m_PrevFree, m_NextFree);
    m_RemindCore.Down();

    if (IS_VB(m_VB))
        DESTROY_VB(m_VB);
    if (IS_IB(m_IB))
        DESTROY_IB(m_IB);

    HFree(this, CMatrixEffect::m_Heap);
}

SPL_VBIB *SPL_VBIB::GetCreate(int vbsize, int ibsize, DWORD fvf) {
    SPL_VBIB *r;
    if (m_FirstFree) {
        SPL_VBIB *candidate = NULL;
        r = m_FirstFree;
        for (; r; r = r->m_NextFree) {
            if (r->m_FVF != fvf)
                continue;
            if (candidate == NULL)
                candidate = r;
            if (vbsize <= r->m_VBSize && ibsize <= r->m_IBSize) {
                candidate = r;
                break;
            }
            if (ibsize <= r->m_IBSize) {
                candidate = r;
                continue;
            }
            if (vbsize <= r->m_VBSize) {
                candidate = r;
                continue;
            }
        }

        if (candidate == NULL)
            goto create;

        LIST_DEL_CLEAR(candidate, m_FirstFree, m_LastFree, m_PrevFree, m_NextFree);
        candidate->m_RemindCore.Down();
        r = candidate;
    }
    else {
    create:
        r = (SPL_VBIB *)HAllocClear(sizeof(SPL_VBIB), CMatrixEffect::m_Heap);
        new(&r->m_RemindCore) SRemindCore(UnloadDX, reinterpret_cast<uintptr_t>(r));
        r->m_FVF = fvf;
        LIST_ADD(r, m_FirstAll, m_LastAll, m_PrevAll, m_NextAll);
    }
    return r;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CMatrixEffectPointLight::CMatrixEffectPointLight(const D3DXVECTOR3 &pos, float r, DWORD color, bool drawbill)
  : CMatrixEffect(), m_Pos(pos), m_Radius(r), m_Color(color), m_Bill(0), m_Time(0)
{
    DTRACE();

    m_EffectType = EFFECT_POINT_LIGHT;
    ELIST_ADD(EFFECT_POINT_LIGHT);

    m_RealDisp.x = 0;
    m_RealDisp.y = 0;

    if (!g_Config.m_VertexLight) {
        m_Tex = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_POINTLIGHT);
    }
    else {
        m_Tex = NULL;
    }

    m_Bill = NULL;
    if (drawbill) {
        m_Bill = (CBillboard *)HAlloc(sizeof(CBillboard), m_Heap);
        if (m_BBTextures[BBT_POINTLIGHT].IsIntense()) {
            new(m_Bill) CBillboard(TRACE_PARAM_CALL pos, r * POINTLIGHT_BILLSCALE, 0, color,
                                           m_BBTextures[BBT_POINTLIGHT].tex);
        }
        else {
            new(m_Bill) CBillboard(TRACE_PARAM_CALL pos, r * POINTLIGHT_BILLSCALE, 0, color,
                                           &m_BBTextures[BBT_POINTLIGHT].bbt);
        }
    }

    // m_Strips = (SPointLightStrip *)HAlloc(sizeof(SPointLightStrip) * 16, m_Heap);
    // m_StripsCnt = 0;
    // m_StripsMax = 16;

    // xxx = 0;

    m_DX = NULL;

    UpdateData();
}

void CMatrixEffectPointLight::MarkAllBuffersNoNeed(void) {
    SPL_VBIB::ReleaseFree();
    SPL_VBIB::ReleaseBuffers();
}

void CMatrixEffectPointLight::Clear(void) {
    DTRACE();

    // if(IS_VB(m_VB)) { DESTROY_VB(m_VB); }
    // if(IS_IB(m_IB)) { DESTROY_IB(m_IB); }

    RemoveColorData();
    m_PointLum.clear();
    // m_StripsCnt=0;
}

CMatrixEffectPointLight::~CMatrixEffectPointLight() {
    DTRACE();

    ELIST_DEL(EFFECT_POINT_LIGHT);

    Clear();

    if (m_Bill) {
        m_Bill->Release();
        HFree(m_Bill, m_Heap);
    }

    if (m_DX)
        m_DX->NoNeed();

    // HFree(m_Strips, m_Heap);
}

void CMatrixEffectPointLight::BuildLand(void) {
    DTRACE();

    typedef struct {
        D3DXVECTOR3 p;
        float tu0, tv0;
        float tu1;
        DWORD outside;
        int index;
        SMatrixMapPoint *mp;
        float lum;
    } STempVertex;

    CRect mr(Float2Int((m_Pos.x - m_Radius) * INVERT(GLOBAL_SCALE)) - 1,
             Float2Int((m_Pos.y - m_Radius) * INVERT(GLOBAL_SCALE)) - 1,
             1 + Float2Int((m_Pos.x + m_Radius) * INVERT(GLOBAL_SCALE)),
             1 + Float2Int((m_Pos.y + m_Radius) * INVERT(GLOBAL_SCALE)));

    int x, y;
    const int da_size_x = (mr.right - mr.left + 1);
    const int da_size_y = (mr.bottom - mr.top + 1);
    const int da_size = da_size_y * da_size_x;

    BYTE *buff = (BYTE *)_alloca((sizeof(STempVertex) + sizeof(SPointLightVertex) + sizeof(WORD) * 3) * da_size);

#define TEMP_VERTS() ((STempVertex *)buff)
#define VERTS()      ((SPointLightVertex *)(buff + (sizeof(STempVertex)) * da_size))
#define IDXS()       ((WORD *)(buff + (sizeof(STempVertex) + sizeof(SPointLightVertex)) * da_size))
#define ADDVERT(v)                                 \
    if (v->index < 0) {                            \
        v->index = cidx++;                         \
        verts->p = v->p;                           \
        verts->tu0 = v->tu0;                       \
        verts->tv0 = v->tv0;                       \
        /*verts->color = m_Color;*/                \
        verts->color = 0;                          \
        verts->tu1 = v->tu1;                       \
        verts->tv1 = 0.5f;                         \
        if (v->mp != NULL) {                       \
            light.lum = v->lum;                    \
            light.mp = v->mp;                      \
            m_PointLum.push_back(light);           \
        }                                          \
        ++verts;                                   \
        ++m_NumVerts;                              \
    }                                              \
    else {                                         \
        if (v->index < midx)                       \
            midx = v->index;                       \
        if (v->index > (midx + madx))              \
            madx = v->index - midx;                \
    }

    STempVertex *tv = TEMP_VERTS();

    float k = INVERT(m_Radius);

    SMatrixMapPoint *mp = g_MatrixMap->PointGet(mr.left, mr.top);
    int addmp = g_MatrixMap->m_Size.x - (mr.right - mr.left);

    m_RealDisp.x = mr.left * GLOBAL_SCALE;
    m_RealDisp.y = mr.top * GLOBAL_SCALE;

    D3DXVECTOR3 mypos;
    mypos.x = m_Pos.x - m_RealDisp.x;
    mypos.y = m_Pos.y - m_RealDisp.y;
    mypos.z = m_Pos.z;

    for (y = mr.top; y <= mr.bottom; ++y) {
        for (x = mr.left; x <= mr.right; ++x, ++tv, ++mp) {
            tv->p.x = GLOBAL_SCALE * (x - mr.left);
            tv->p.y = GLOBAL_SCALE * (y - mr.top);
            if (x >= 0 && x <= g_MatrixMap->m_Size.x && y >= 0 && y <= g_MatrixMap->m_Size.y) {
                tv->mp = mp;
                tv->p.z = g_MatrixMap->PointGet(x, y)->z + POINTLIGHT_ALTITUDE;
                if (tv->p.z < WATER_LEVEL)
                    tv->p.z = WATER_LEVEL;
            }
            else {
                tv->p.z = WATER_LEVEL;
                tv->mp = NULL;
            }

            D3DXVECTOR3 dd((mypos - tv->p) * k);

            tv->lum = (1.0f - (D3DXVec3LengthSq(&dd)));

            dd = dd * 0.5f + D3DXVECTOR3(0.5f, 0.5f, 0.5f);

            tv->tu0 = dd.x;
            tv->tv0 = dd.y;

            tv->tu1 = dd.z;

            if (tv->lum < 0) {
                tv->outside = true;
                tv->lum = 0;
            }
            else
                tv->outside = false;
            tv->index = -1;
        }
        mp += addmp;
    }

    m_NumVerts = 0;
    int idxscnt = 0;

    bool strip_in_progress = false;

    tv = TEMP_VERTS();

    //    SPointLightStrip *strips = STRIPS();
    SPointLightVertex *verts = VERTS();
    WORD *idxs = IDXS();

    int cidx = 0;
    int midx = da_size;
    int madx = 0;
    for (y = 0; y < (da_size_y - 1); ++y) {
        for (x = 0; x < (da_size_x - 1); ++x, ++tv) {
            if (tv->outside && (tv + 1)->outside && (tv + da_size_x)->outside && (tv + da_size_x + 1)->outside) {
                strip_in_progress = false;
                continue;
            }

            STempVertex *tv0 = tv + da_size_x;
            STempVertex *tv1 = tv;
            STempVertex *tv2 = tv + da_size_x + 1;
            STempVertex *tv3 = tv + 1;

            SMapPointLight light;

            ADDVERT(tv0);
            ADDVERT(tv1);
            ADDVERT(tv2);
            ADDVERT(tv3);

            if (strip_in_progress) {
                *(DWORD *)idxs = tv2->index | (tv3->index << 16);
                idxscnt += 2;
                idxs += 2;
            }
            else {
                if (idxscnt > 0) {
                    *(DWORD *)idxs = *(idxs - 1) | (tv0->index << 16);
                    idxscnt += 2;
                    idxs += 2;
                }

                *(DWORD *)(idxs + 0) = tv0->index | (tv1->index << 16);
                *(DWORD *)(idxs + 2) = tv2->index | (tv3->index << 16);

                idxs += 4;
                idxscnt += 4;
                strip_in_progress = true;
            }
        }
        strip_in_progress = false;
        ++tv;
    }

    if (m_NumVerts == 0) {
        return;
    }

    int sz = (m_NumVerts) * sizeof(SPointLightVertex);
    int szi = idxscnt * sizeof(WORD);

    if (m_DX == NULL) {
        m_DX = SPL_VBIB::GetCreate(sz, szi, POINTLIGHT_FVF);
    }

    m_NumTris = idxscnt - 2;

    void *buf;

    buf = m_DX->LockVB(sz);
    if (buf == NULL) {
        m_DX->DX_Free();
        return;
    }

    memcpy(buf, VERTS(), sz);
    m_DX->UnLockVB();

    buf = m_DX->LockIB(szi);
    if (buf == NULL) {
        m_DX->DX_Free();
        return;
    }

    memcpy(buf, IDXS(), szi);
    m_DX->UnLockIB();

#undef ADDVERT
#undef TEMP_VERTS
#undef STRIPS
#undef VERTS
#undef IDXS
}

void CMatrixEffectPointLight::BuildLandV(void) {
    DTRACE();

    typedef struct {
        D3DXVECTOR3 p;
        DWORD outside;
        int index;
        SMatrixMapPoint *mp;
        float lum;
    } STempVertex;

    CRect mr(Float2Int((m_Pos.x - m_Radius) * INVERT(GLOBAL_SCALE)) - 1,
             Float2Int((m_Pos.y - m_Radius) * INVERT(GLOBAL_SCALE)) - 1,
             1 + Float2Int((m_Pos.x + m_Radius) * INVERT(GLOBAL_SCALE)),
             1 + Float2Int((m_Pos.y + m_Radius) * INVERT(GLOBAL_SCALE)));

    int x, y;
    const int da_size_x = (mr.right - mr.left + 1);
    const int da_size_y = (mr.bottom - mr.top + 1);
    const int da_size = da_size_y * da_size_x;

    BYTE *buff = (BYTE *)_alloca((sizeof(STempVertex) + sizeof(SPointLightVertexV) + sizeof(WORD) * 3) * da_size);

#define TEMP_VERTS() ((STempVertex *)buff)
#define VERTS()      ((SPointLightVertexV *)(buff + (sizeof(STempVertex)) * da_size))
#define IDXS()       ((WORD *)(buff + (sizeof(STempVertex) + sizeof(SPointLightVertexV)) * da_size))
#define ADDVERT(v)                                                   \
    if (v->index < 0) {                                              \
        v->index = cidx++;                                           \
        verts->p = v->p;                                             \
        /*verts->color = m_Color;*/                                  \
        verts->color = (Float2Int(v->lum * 255) << 24) | 0x00FFFFFF; \
        if (v->mp != NULL) {                                         \
            light.lum = v->lum;                                      \
            light.mp = v->mp;                                        \
            m_PointLum.push_back(light);                             \
        }                                                            \
        ++verts;                                                     \
        ++m_NumVerts;                                                \
    }                                                                \
    else {                                                           \
        if (v->index < midx)                                         \
            midx = v->index;                                         \
        if (v->index > (midx + madx))                                \
            madx = v->index - midx;                                  \
    }

    STempVertex *tv = TEMP_VERTS();

    float k = INVERT(m_Radius);

    SMatrixMapPoint *mp = g_MatrixMap->PointGet(mr.left, mr.top);
    int addmp = g_MatrixMap->m_Size.x - (mr.right - mr.left);

    m_RealDisp.x = mr.left * GLOBAL_SCALE;
    m_RealDisp.y = mr.top * GLOBAL_SCALE;

    D3DXVECTOR3 mypos;
    mypos.x = m_Pos.x - m_RealDisp.x;
    mypos.y = m_Pos.y - m_RealDisp.y;
    mypos.z = m_Pos.z;

    for (y = mr.top; y <= mr.bottom; ++y) {
        for (x = mr.left; x <= mr.right; ++x, ++tv, ++mp) {
            tv->p.x = GLOBAL_SCALE * (x - mr.left);
            tv->p.y = GLOBAL_SCALE * (y - mr.top);
            if (x >= 0 && x <= g_MatrixMap->m_Size.x && y >= 0 && y <= g_MatrixMap->m_Size.y) {
                tv->mp = mp;
                tv->p.z = mp->z + POINTLIGHT_ALTITUDE;
                if (tv->p.z < WATER_LEVEL)
                    tv->p.z = WATER_LEVEL;
            }
            else {
                tv->p.z = WATER_LEVEL;
                tv->mp = NULL;
            }

            D3DXVECTOR3 dd((mypos - tv->p) * k);

            // if (tv->mp && D3DXVec3Dot(&dd, &mp->n) < 0)
            //{
            //    tv->outside = true;
            //    tv->lum = 0;

            //} else
            {
                tv->lum = (1.0f - (D3DXVec3LengthSq(&dd)));

                if (tv->lum < 0) {
                    tv->outside = true;
                    tv->lum = 0;
                }
                else
                    tv->outside = false;
            }
            tv->index = -1;
        }
        mp += addmp;
    }

    m_NumVerts = 0;
    int idxscnt = 0;

    bool strip_in_progress = false;

    tv = TEMP_VERTS();

    SPointLightVertexV *verts = VERTS();
    WORD *idxs = IDXS();

    int cidx = 0;
    int midx = da_size;
    int madx = 0;
    for (y = 0; y < (da_size_y - 1); ++y) {
        for (x = 0; x < (da_size_x - 1); ++x, ++tv) {
            if (tv->outside && (tv + 1)->outside && (tv + da_size_x)->outside && (tv + da_size_x + 1)->outside) {
                strip_in_progress = false;
                continue;
            }

            STempVertex *tv0 = tv + da_size_x;
            STempVertex *tv1 = tv;
            STempVertex *tv2 = tv + da_size_x + 1;
            STempVertex *tv3 = tv + 1;

            SMapPointLight light;

            ADDVERT(tv0);
            ADDVERT(tv1);
            ADDVERT(tv2);
            ADDVERT(tv3);

            if (strip_in_progress) {
                *(DWORD *)idxs = tv2->index | (tv3->index << 16);
                idxscnt += 2;
                idxs += 2;
            }
            else {
                if (idxscnt > 0) {
                    *(DWORD *)idxs = *(idxs - 1) | (tv0->index << 16);
                    idxscnt += 2;
                    idxs += 2;
                }

                *(DWORD *)(idxs + 0) = tv0->index | (tv1->index << 16);
                *(DWORD *)(idxs + 2) = tv2->index | (tv3->index << 16);

                idxs += 4;
                idxscnt += 4;
                strip_in_progress = true;
            }
        }
        strip_in_progress = false;
        ++tv;
    }

    if (m_NumVerts == 0) {
        return;
    }
    m_NumTris = idxscnt - 2;

    int sz = (m_NumVerts) * sizeof(SPointLightVertexV);
    int szi = idxscnt * sizeof(WORD);

    if (m_DX == NULL) {
        m_DX = SPL_VBIB::GetCreate(sz, szi, POINTLIGHT_FVF_V);
    }

    //#ifdef _DEBUG
    //    for (int ii=0;ii<m_NumVerts;++ii)
    //    {
    //
    //        D3DXVECTOR3 p(m_RealDisp.x + VERTS()[ii].p.x, m_RealDisp.y+VERTS()[ii].p.y, VERTS()[ii].p.z);
    //        CHelper::Create(1,0)->Line(p, D3DXVECTOR3(0,0,10) + p);
    //
    //
    //    }
    //#endif

    void *buf;

    buf = m_DX->LockVB(sz);
    if (buf == NULL) {
        m_DX->DX_Free();
        return;
    }

    memcpy(buf, VERTS(), sz);
    m_DX->UnLockVB();

    buf = m_DX->LockIB(szi);
    if (buf == NULL) {
        m_DX->DX_Free();
        return;
    }

    memcpy(buf, IDXS(), szi);
    m_DX->UnLockIB();

#undef ADDVERT
#undef TEMP_VERTS
#undef STRIPS
#undef VERTS
#undef IDXS
}

void CMatrixEffectPointLight::UpdateData(void) {
    DTRACE();
    Clear();

    if (m_Bill) {
        m_Bill->SetPos(m_Pos);
        m_Bill->SetColor(m_Color);
        m_Bill->SetScale(m_Radius * POINTLIGHT_BILLSCALE);
    }

    // RemoveColorData();
    if (g_Config.m_VertexLight) {
        BuildLandV();
    }
    else {
        BuildLand();
    }
    AddColorData();
    g_MatrixMap->MarkNeedRecalcTerainColor();
}

void CMatrixEffectPointLight::RemoveColorData(void)
{
    if (m_PointLum.empty())
    {
        return;
    }

    for (auto& item : m_PointLum)
    {
        item.mp->lum_r -= item.addlum_r;
        item.mp->lum_g -= item.addlum_g;
        item.mp->lum_b -= item.addlum_b;
    }
}

void CMatrixEffectPointLight::AddColorData(void)
{
    if (m_PointLum.empty())
    {
        return;
    }

    for (auto& item : m_PointLum)
    {
        item.addlum_r = Float2Int(item.lum * float((m_Color >> 16) & 255));
        item.addlum_g = Float2Int(item.lum * float((m_Color >> 8) & 255));
        item.addlum_b = Float2Int(item.lum * float((m_Color >> 0) & 255));

        item.mp->lum_r += item.addlum_r;
        item.mp->lum_g += item.addlum_g;
        item.mp->lum_b += item.addlum_b;
    }

    g_MatrixMap->MarkNeedRecalcTerainColor();
    // if (IS_VB(m_VB) && g_MatrixMap->m_Config.m_VertexLight)
    //{

    //    SPointLightVertexV *verts;
    //    LOCK_VB(m_VB,&verts);
    //    for (int i = 0; i<m_NumVerts; ++i)
    //    {
    //        verts->color = (verts->color&0xFF000000) | (0x00FFFFFF | m_Color);
    //        ++verts;
    //    }
    //    UNLOCK_VB(m_VB);
    //}
}

void CMatrixEffectPointLight::Draw(void) {
    DTRACE();

    if (m_Bill)
        m_Bill->Sort(g_MatrixMap->m_Camera.GetViewMatrix());
    if (!m_DX)
        return;
    if (m_NumVerts == 0 || !m_DX->DX_Ready())
        return;

    g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));

    if (g_Config.m_VertexLight) {
        g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, m_Color);

        // CDText::T("col", (int)m_Color);
        SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_TFACTOR);
        SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_TFACTOR);
        // SetColorOpSelect(0, D3DTA_DIFFUSE);
        // SetAlphaOpSelect(0, D3DTA_DIFFUSE);
        SetColorOpDisable(1);

        ASSERT_DX(g_D3DD->SetFVF(POINTLIGHT_FVF_V));
        ASSERT_DX(g_D3DD->SetStreamSource(0, GET_VB(m_DX->DX_GetVB()), 0, sizeof(SPointLightVertexV)));
    }
    else {
        g_D3DD->SetTexture(0, m_Tex->Tex());
        g_D3DD->SetTexture(1, m_Tex->Tex());

        g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, m_Color);

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));

        // SetColorOp(0, D3DTOP_SUBTRACT, D3DTA_TFACTOR,  D3DTA_TEXTURE);
        // SetColorOpSelect(0, D3DTA_TEXTURE );
        SetColorOpSelect(0, D3DTA_TEXTURE | D3DTA_COMPLEMENT);
        SetAlphaOpSelect(0, D3DTA_TEXTURE);

        SetColorOp(1, D3DTOP_SUBTRACT, D3DTA_CURRENT, D3DTA_TEXTURE);
        SetAlphaOpSelect(1, D3DTA_CURRENT);

        SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_CURRENT, D3DTA_TFACTOR);
        SetAlphaOpSelect(2, D3DTA_CURRENT);
        SetColorOpDisable(3);

        // SetColorOpSelect(0, D3DTA_TEXTURE);
        ASSERT_DX(g_D3DD->SetFVF(POINTLIGHT_FVF));
        ASSERT_DX(g_D3DD->SetStreamSource(0, GET_VB(m_DX->DX_GetVB()), 0, sizeof(SPointLightVertex)));
    }

    D3DXMATRIX m(g_MatrixMap->GetIdentityMatrix());
    m._41 = m_RealDisp.x;
    m._42 = m_RealDisp.y;

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m));

    ASSERT_DX(g_D3DD->SetIndices(GET_IB(m_DX->DX_GetIB())));

    ASSERT_DX(g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, m_NumVerts, 0, m_NumTris));

    g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
}
void CMatrixEffectPointLight::Takt(float step) {
    DTRACE();

    if (FLAG(m_Flags, PLF_KIP)) {
        if (m_Time < m_KillTime) {
            int a2 = int((m_InitColor >> 24) & 255);
            int r2 = int((m_InitColor >> 16) & 255);
            int g2 = int((m_InitColor >> 8) & 255);
            int b2 = int((m_InitColor >> 0) & 255);

            float k = float(m_KillTime - m_Time) * _m_KT;

            int a = Float2Int(k * a2);
            int r = Float2Int(k * r2);
            int g = Float2Int(k * g2);
            int b = Float2Int(k * b2);
            SetColor((a << 24) | (r << 16) | (g << 8) | b);
        }
        else {
#ifdef _DEBUG
            g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
            g_MatrixMap->SubEffect(this);
#endif
            return;
        }
    }
    m_Time += step;

    /*
        static float dz = 1;

        m_Pos.z += dz * (step) * 0.1f;
        if (m_Pos.z > m_Radius) dz = -dz;
        if (m_Pos.z < 1) dz = -dz;

        CDText::T("Z", m_Pos.z);
    */
    // UpdateData();
}
void CMatrixEffectPointLight::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectPointLight, this, m_Heap);
}

void CMatrixEffectPointLight::Kill(float time) {
    SETFLAG(m_Flags, PLF_KIP);
    m_KillTime = m_Time + time;
    m_InitColor = m_Color;
    _m_KT = INVERT(time);
}

int CMatrixEffectPointLight::Priority(void) {
    return g_MatrixMap->m_Camera.IsInFrustum(m_Pos) ? (500) : (5);
}
