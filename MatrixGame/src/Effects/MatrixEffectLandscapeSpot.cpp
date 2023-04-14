// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include <math.h>

#include "MatrixEffectLandscapeSpot.hpp"
#include "MatrixEffectPointLight.hpp"

static SSpotProperties m_SpotProperties[SPOT_TYPES_CNT];

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CMatrixEffectLandscapeSpot *CMatrixEffectLandscapeSpot::m_First;
CMatrixEffectLandscapeSpot *CMatrixEffectLandscapeSpot::m_Last;

#define TIME_TO_OFF (0.1f)
#define TIME_TO_ON  (0.03f)
void SpotTaktConstant(CMatrixEffectLandscapeSpot *spot, float) {
    DTRACE();
    if (spot->m_LifeTime > spot->m_Props->ttl) {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, spot);
#else
        g_MatrixMap->SubEffect(spot);
#endif

        return;
    }

    float k = (float(spot->m_Props->ttl - spot->m_LifeTime) / float(spot->m_Props->ttl));

    if (k < TIME_TO_OFF) {
        byte c = byte(k * (1.0f / TIME_TO_OFF) * 255);

        // byte c = byte(k * 255.0f);

        spot->m_Color = (spot->m_Color & 0x00FFFFFF) + (c << 24);
    }
    else {
        spot->m_Color = (spot->m_Color & 0x00FFFFFF) + (255 << 24);
    }
}

void SpotTaktVoronka(CMatrixEffectLandscapeSpot *spot, float) {
    DTRACE();
    if (spot->m_LifeTime > spot->m_Props->ttl) {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, spot);
#else
        g_MatrixMap->SubEffect(spot);
#endif
        return;
    }

    float k = (float(spot->m_Props->ttl - spot->m_LifeTime) / float(spot->m_Props->ttl));

    if (k < TIME_TO_OFF) {
        byte c = byte(k * (1.0 / TIME_TO_OFF) * 255);
        spot->m_Color = (spot->m_Color & 0x00FFFFFF) + (c << 24);
    }
    else {
        if (k > (1.0f - TIME_TO_ON)) {
            byte c = byte((1.0 - k) * (1.0 / TIME_TO_ON) * 255);
            spot->m_Color = (spot->m_Color & 0x00FFFFFF) + (c << 24);
        }
        else
            spot->m_Color = 0xFFFFFFFF;
    }
}
#undef TIME_TO_ON
#undef TIME_TO_OFF

void SpotTaktAlways(CMatrixEffectLandscapeSpot *, float) {
    DTRACE();
}

void SpotTaktMoveTo(CMatrixEffectLandscapeSpot *spot, float) {
    DTRACE();
    if (spot->m_LifeTime > spot->m_Props->ttl) {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, spot);
#else
        g_MatrixMap->SubEffect(spot);
#endif
        return;
    }
    float k = 1.0f - (float(spot->m_Props->ttl - spot->m_LifeTime) / float(spot->m_Props->ttl));

    byte a = byte((1.0f - KSCALE(k, 0.5f, 1.0f)) * 255);
    byte r = byte((1.0f - KSCALE(k, 0.3f, 1.0f)) * 255);
    byte g = byte((1.0f - KSCALE(k, 0.3f, 0.7f)) * 255);
    byte b = byte((1.0f - KSCALE(k, 0.0f, 1.0f)) * 255);

    spot->m_Color = (a << 24) | (r << 16) | (g << 8) | (b);
}

void SpotTaktPointlight(CMatrixEffectLandscapeSpot *spot, float) {
    DTRACE();
    if (spot->m_LifeTime > spot->m_Props->ttl) {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, spot);
#else
        g_MatrixMap->SubEffect(spot);
#endif
        return;
    }
    float k = 1.0f - (float(spot->m_Props->ttl - spot->m_LifeTime) / float(spot->m_Props->ttl));

    byte c = byte((1.0f - KSCALE(k, 0.0f, 0.7f)) * 255);
    byte r = byte((1.0f - KSCALE(k, 0.0f, 1.0f)) * 255);

    spot->m_Color = 0xFF000000 | (r << 16) | (c << 8) | (c);
}

void SpotTaktPlasmaHit(CMatrixEffectLandscapeSpot *spot, float) {
    DTRACE();
    if (spot->m_LifeTime > spot->m_Props->ttl) {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, spot);
#else
        g_MatrixMap->SubEffect(spot);
#endif
        return;
    }

    float k = 1.0f - (float(spot->m_Props->ttl - spot->m_LifeTime) / float(spot->m_Props->ttl));

    byte a = byte((1.0f - KSCALE(k, 0.5f, 1.0f)) * 255);
    byte r = byte((1.0f - KSCALE(k, 0.3f, 1.0f)) * 255);
    byte g = byte((1.0f - KSCALE(k, 0.3f, 0.7f)) * 255);
    byte b = byte((1.0f - KSCALE(k, 0.0f, 1.0f)) * 255);

    spot->m_Color = (a << 24) | (r << 16) | (g << 8) | (b);
}

void CMatrixEffectLandscapeSpot::MarkAllBuffersNoNeed(void) {
    CMatrixEffectLandscapeSpot *ls = m_First;
    for (; ls; ls = ls->m_Next) {
        if (ls->m_VB)
            DESTROY_VB(ls->m_VB);
        if (ls->m_IB)
            DESTROY_VB(ls->m_IB);
    }
}

bool CMatrixEffectLandscapeSpot::PrepareDX(void) {
    if (IS_VB(m_VB) || IS_IB(m_IB))
        return true;
    if (m_CntVerts == 0)
        return false;

    CREATE_VB(m_CntVerts * sizeof(SLandscapeSpotVertex), LANDSCAPESPOT_FVF, m_VB);
    if (!IS_VB(m_VB))
        return false;

    CREATE_IB16((m_CntTris + 2), m_IB);
    if (!IS_IB(m_IB)) {
        DESTROY_VB(m_VB);
        return false;
    }

    void *buf;

    LOCK_VB(m_VB, &buf);
    memcpy(buf, m_VertsPre, m_CntVerts * sizeof(SLandscapeSpotVertex));
    UNLOCK_VB(m_VB);

    LOCK_IB(m_IB, &buf);
    memcpy(buf, m_IndsPre, (m_CntTris + 2) * sizeof(WORD));
    UNLOCK_IB(m_IB);

    return true;
}

void CMatrixEffectLandscapeSpot::BuildLand(const D3DXVECTOR2 &pos, float angle, float scalex, float scaley, float addz,
                                           bool scale_by_normal) {
    DTRACE();

    typedef struct {
        D3DXVECTOR3 p;
        float tu, tv;
        int index;
        DWORD outside;
    } STempVertex;

    CRect mr;
    D3DXVECTOR2 p[4];
    {
        int x, y;

        float tsin;
        float tcos;
        SinCos(angle, &tsin, &tcos);

        float xc = SPOT_SIZE * scalex * tcos;
        float xs = SPOT_SIZE * scalex * tsin;
        float yc = SPOT_SIZE * scaley * tcos;
        float ys = SPOT_SIZE * scaley * tsin;

        if (scale_by_normal) {
            D3DXVECTOR3 norm;
            g_MatrixMap->GetNormal(&norm, pos.x, pos.y);
            float nscalex = float(acos(fabs(norm.x)) * (2.0 / M_PI));
            float nscaley = float(acos(fabs(norm.y)) * (2.0 / M_PI));
            p[0].x = pos.x + (-xc + ys) * nscalex;
            p[0].y = pos.y + (-xs - yc) * nscaley;
            p[1].x = pos.x + (xc + ys) * nscalex;
            p[1].y = pos.y + (xs - yc) * nscaley;
            p[2].x = pos.x + (xc - ys) * nscalex;
            p[2].y = pos.y + (xs + yc) * nscaley;
            p[3].x = pos.x + (-xc - ys) * nscalex;
            p[3].y = pos.y + (-xs + yc) * nscaley;
        }
        else {
            p[0].x = pos.x + (-xc + ys);
            p[0].y = pos.y + (-xs - yc);
            p[1].x = pos.x + (xc + ys);
            p[1].y = pos.y + (xs - yc);
            p[2].x = pos.x + (xc - ys);
            p[2].y = pos.y + (xs + yc);
            p[3].x = pos.x + (-xc - ys);
            p[3].y = pos.y + (-xs + yc);
        }

        mr.left = Float2Int(p[0].x * INVERT(GLOBAL_SCALE)) - 1;
        mr.right = Float2Int(p[0].x * INVERT(GLOBAL_SCALE)) + 1;
        mr.top = Float2Int(p[0].y * INVERT(GLOBAL_SCALE)) - 1;
        mr.bottom = Float2Int(p[0].y * INVERT(GLOBAL_SCALE)) + 1;

        for (int i = 1; i < 4; ++i) {
            x = Float2Int(p[i].x * INVERT(GLOBAL_SCALE));
            y = Float2Int(p[i].y * INVERT(GLOBAL_SCALE));

            if (x <= mr.left)
                mr.left = x - 1;
            if (x >= mr.right)
                mr.right = x + 1;
            if (y <= mr.top)
                mr.top = y - 1;
            if (y >= mr.bottom)
                mr.bottom = y + 1;
        }
    }

    m_CntVerts = 0;
    if (mr.left < 0)
        mr.left = 0;
    if (mr.top < 0)
        mr.top = 0;
    if (mr.right >= g_MatrixMap->m_Size.x)
        mr.right = g_MatrixMap->m_Size.x - 1;
    if (mr.bottom >= g_MatrixMap->m_Size.y)
        mr.bottom = g_MatrixMap->m_Size.y - 1;
    if (mr.IsEmpty())
        return;

    // calc koefs for U,V calculation

    // double A1 = 2.0 * ((p[2].x - p[3].x - p[1].x + p[0].x) * (p[0].y - p[1].y) - (p[0].x - p[1].x) * (p[2].y - p[3].y
    // - p[1].y + p[0].y));

    double B11 = (p[2].x - p[3].x - p[1].x + p[0].x);   // * (oy - p[0].y)
    double B12 = -(p[2].y - p[3].y - p[1].y + p[0].y);  // * (ox - p[0].x)
    double B13 = (p[3].x - p[0].x) * (p[0].y - p[1].y) - (p[0].x - p[1].x) * (p[3].y - p[0].y);
    double C11 = (p[3].x - p[0].x);   // * (oy - p[0].y)
    double C12 = -(p[3].y - p[0].y);  // *(ox - p[0].x);

    // double A2 = 2.0 * ((p[3].x - p[0].x - p[2].x + p[1].x) * (p[1].y - p[2].y) - (p[1].x - p[2].x) * (p[3].y - p[0].y
    // - p[2].y + p[1].y));
    double B21 = (p[3].x - p[0].x - p[2].x + p[1].x);   // * (oy - p[1].y);
    double B22 = -(p[3].y - p[0].y - p[2].y + p[1].y);  // * (ox - p[1].x)
    double B23 = (p[0].x - p[1].x) * (p[1].y - p[2].y) - (p[1].x - p[2].x) * (p[0].y - p[1].y);
    double C21 = (p[0].x - p[1].x);   // * (oy - p[1].y)
    double C22 = -(p[0].y - p[1].y);  // * (ox - p[1].x);

    // double D1 = sqrt(B1*B1 - 4.0*A1*C1);
    // double D2 = sqrt(B2*B2 - 4.0*A2*C2);
    // double k = 1.0 / (2.0 * A1);
    // double t = k * (-B1 + D1);

    int x, y;
    const int da_size_x = (mr.right - mr.left + 1);
    const int da_size_y = (mr.bottom - mr.top + 1);
    const int da_size = da_size_y * da_size_x;

    BYTE *buff = (BYTE *)_alloca((sizeof(STempVertex) + sizeof(SLandscapeSpotVertex) + sizeof(WORD) * 3) * da_size);

#define TEMP_VERTS() ((STempVertex *)buff)
#define VERTS()      ((SLandscapeSpotVertex *)(buff + (sizeof(STempVertex)) * da_size))
#define IDXS()       ((WORD *)(buff + (sizeof(STempVertex) + sizeof(SLandscapeSpotVertex)) * da_size))
#define ADDVERT(v)                     \
    if (v->index < 0) {                \
        v->index = cidx++;             \
        verts->p = v->p;               \
        verts->tu = v->tu;             \
        verts->tv = v->tv;             \
        verts->color = m_Props->color; \
        ++verts;                       \
        ++m_CntVerts;                  \
    }

    STempVertex *tv = TEMP_VERTS();

    SMatrixMapPoint *mp = g_MatrixMap->PointGet(mr.left, mr.top);
    int addmp = g_MatrixMap->m_Size.x - (mr.right - mr.left);

    m_DX = mr.left * GLOBAL_SCALE;
    m_DY = mr.top * GLOBAL_SCALE;

    for (y = mr.top; y <= mr.bottom; ++y) {
        for (x = mr.left; x <= mr.right; ++x, ++tv, ++mp) {
            tv->p.x = GLOBAL_SCALE * (x - mr.left);
            tv->p.y = GLOBAL_SCALE * (y - mr.top);
            tv->p.z = mp->z + POINTLIGHT_ALTITUDE;

            {
                double dxu = (tv->p.x + m_DX - p[0].x);
                double dyu = (tv->p.y + m_DY - p[0].y);
                double dxv = (tv->p.x + m_DX - p[1].x);
                double dyv = (tv->p.y + m_DY - p[1].y);

                double B = B11 * dyu + B12 * dxu + B13;
                double C = C11 * dyu + C12 * dxu;

                tv->tu = float(-C / B);

                B = B21 * dyv + B22 * dxv + B23;
                C = C21 * dyv + C22 * dxv;

                tv->tv = float(-C / B);

                tv->outside = (tv->tu < 0 || tv->tu > 1) && (tv->tv < 0 || tv->tv > 1);
            }

            tv->index = -1;
        }
        mp += addmp;
    }

    int idxscnt = 0;

    bool strip_in_progress = false;

    tv = TEMP_VERTS();

    SLandscapeSpotVertex *verts = VERTS();
    WORD *idxs = IDXS();

    int cidx = 0;
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

            ADDVERT(tv0);
            ADDVERT(tv1);
            ADDVERT(tv2);
            ADDVERT(tv3);

            if (strip_in_progress) {
                *idxs++ = (WORD)tv2->index;
                *idxs++ = (WORD)tv3->index;
                idxscnt += 2;
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

    if (m_CntVerts == 0) {
        return;
    }
    m_CntTris = idxscnt - 2;

    m_VertsPre = (BYTE *)HAlloc(m_CntVerts * sizeof(SLandscapeSpotVertex), m_Heap);
    memcpy(m_VertsPre, VERTS(), m_CntVerts * sizeof(SLandscapeSpotVertex));

    m_IndsPre = (BYTE *)HAlloc(idxscnt * sizeof(WORD), m_Heap);
    memcpy(m_IndsPre, IDXS(), idxscnt * sizeof(WORD));

#undef ADDVERT
#undef TEMP_VERTS
#undef STRIPS
#undef VERTS
#undef IDXS
}

CMatrixEffectLandscapeSpot::CMatrixEffectLandscapeSpot(const D3DXVECTOR2 &pos, float angle, float scale, ESpotType type)
  : CMatrixEffect(), m_LifeTime(0), m_VertsPre(NULL), m_IndsPre(NULL), m_VB(NULL), m_IB(NULL) {
    DTRACE();
    m_EffectType = EFFECT_LANDSCAPE_SPOT;

    ELIST_ADD(EFFECT_LANDSCAPE_SPOT);

    // m_Pos = D3DXVECTOR3(pos.x, pos.y, g_MatrixMap->GetZ(pos.x, pos.y) + SPOT_ALTITUDE);
    // m_Scale = scale;

    m_DX = 0;
    m_DY = 0;

    m_Props = &m_SpotProperties[type];
    m_Color = m_Props->color;
    m_Texture = m_Props->texture;
    m_Texture->RefInc();

    BuildLand(pos, angle, scale, scale, SPOT_ALTITUDE, FLAG(m_SpotProperties[type].flags, LSFLAG_SCALE_BY_NORMAL));

    if (m_CntVerts == 0) {
        LIST_ADD(this, m_First, m_Last, m_Prev, m_Next);
        m_LifeTime = m_Props->ttl + 1;
        m_VB = NULL;
        m_IB = NULL;
        return;
    }

    LIST_ADD(this, m_First, m_Last, m_Prev, m_Next);

    // DPTR_MEM_SIGNATURE_INIT(sizeof(CMatrixEffectLandscapeSpot));
}

CMatrixEffectLandscapeSpot::~CMatrixEffectLandscapeSpot() {
    DTRACE();
    DCS_INDESTRUCTOR();

    if (m_IndsPre)
        HFree(m_IndsPre, m_Heap);
    if (m_VertsPre)
        HFree(m_VertsPre, m_Heap);

    // if (m_Color == 0x80FFFFFF)
    //{
    //    CBuf b;
    //    CMatrixEffectLandscapeSpot *eff = (CMatrixEffectLandscapeSpot *)ELIST_FIRST(EFFECT_LANDSCAPE_SPOT);
    //    for (;eff;eff = (CMatrixEffectLandscapeSpot *)eff->m_TypeNext)
    //    {
    //        b.StrNZ(utils::from_wstring((std::wstring(eff->Priority()) + L":" + eff->m_Texture->m_Name).Get()).c_str();
    //        b.StrNZ("\r\n");

    //    }
    //    b.SaveInFile(L"Opa!!!.txt");
    //}

    if (IS_VB(m_VB)) {
        DESTROY_VB(m_VB);
    }
    if (IS_VB(m_IB)) {
        DESTROY_IB(m_IB);
    }
    LIST_DEL(this, m_First, m_Last, m_Prev, m_Next);
    m_Texture->RefDecUnload();

    ELIST_DEL(EFFECT_LANDSCAPE_SPOT);
}

void CMatrixEffectLandscapeSpot::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectLandscapeSpot, this, m_Heap);
}

void CMatrixEffectLandscapeSpot::Takt(float step) {
    DTRACE();
    if (m_CntVerts == 0) {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        g_MatrixMap->SubEffect(this);
#endif
        return;
    };
    m_LifeTime += step;
    m_Props->func(this, step);
}

void CMatrixEffectLandscapeSpot::DrawActual() {
    DTRACE();
    if (!IS_VB(m_VB) || !IS_IB(m_IB))
        return;

    D3DXMATRIX m = g_MatrixMap->GetIdentityMatrix();
    m._41 = m_DX;
    m._42 = m_DY;
    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m));

    g_D3DD->SetStreamSource(0, GET_VB(m_VB), 0, sizeof(SLandscapeSpotVertex));
    g_D3DD->SetIndices(GET_IB(m_IB));

    g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, m_Color);

    g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, m_CntVerts, 0, m_CntTris);
}

void CMatrixEffectLandscapeSpot::DrawAll(void) {
    DTRACE();

    if (!m_First)
        return;

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));

    ASSERT_DX(g_D3DD->SetFVF(LANDSCAPESPOT_FVF));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));

    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);

    SetColorOpDisable(1);

    CTextureManaged *m_LastTexture = 0;
    CMatrixEffectLandscapeSpot *el = m_First;
    while (el) {
        if (FLAG(el->m_Props->flags, LSFLAG_INTENSE)) {
            g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        }
        else {
            g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        }
        if (m_LastTexture != el->m_Texture) {
            ASSERT_DX(g_D3DD->SetTexture(0, el->m_Texture->Tex()));
            m_LastTexture = el->m_Texture;
        }
        el->DrawActual();
        el = el->m_Next;
    }
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    // g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
    g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

int CMatrixEffectLandscapeSpot::Priority(void) {
    if (m_Props->func == SpotTaktAlways)
        return MAX_EFFECT_PRIORITY;
    if (m_Props->func == SpotTaktConstant)
        return 100;
    if (m_Props->func == SpotTaktPlasmaHit)
        return 0;
    if (m_Props->func == SpotTaktVoronka)
        return 500;

    return 0;
};
