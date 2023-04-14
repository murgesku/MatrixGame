// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <algorithm>

#include "MatrixMap.hpp"
#include "MatrixMapGroup.hpp"
#include "MatrixObject.hpp"
#include "MatrixRobot.hpp"
#include "MatrixRenderPipeline.hpp"
#include "MatrixTerSurface.hpp"
#include "MatrixShadowManager.hpp"

//#define DRAW_GROUP_BBOX

CBigVB<SMatrixMapVertexBottom> *CMatrixMapGroup::m_BigVB_bottom;
CBigIB *CMatrixMapGroup::m_BigIB_bottom;

#ifdef _DEBUG
int CMatrixMapGroup::m_DPCalls = 0;
#endif

static bool FreeResources(uintptr_t user) {
    DTRACE();

    CMatrixMapGroup* g = reinterpret_cast<CMatrixMapGroup*>(user);
    g->DX_Free();
    return false;
}

void CMatrixMapGroup::MarkAllBuffersNoNeed(void) {
    m_BigVB_bottom->ReleaseBuffers();
    m_BigIB_bottom->ReleaseBuffers();
}

#pragma warning(disable : 4355)
CMatrixMapGroup::CMatrixMapGroup(void) : CMain(), m_RemindCore(FreeResources, reinterpret_cast<uintptr_t>(this)) {
    memset(((BYTE *)this) + sizeof(m_RemindCore) + sizeof(CMain), 0,
           sizeof(CMatrixMapGroup) - sizeof(m_RemindCore) - sizeof(CMain));

    if (m_BigVB_bottom == NULL) {
        m_BigVB_bottom = CBigVB<SMatrixMapVertexBottom>::NewBigVB(g_MatrixHeap);
    }

    if (m_BigIB_bottom == NULL) {
        m_BigIB_bottom = CBigIB::NewBigIB(g_MatrixHeap);
    }

    // RESETFLAG(m_Flags, GRPFLAG_HASFLYER);
    RESETFLAG(m_Flags, GRPFLAG_HASBASE);
}
#pragma warning(default : 4355)

CMatrixMapGroup::~CMatrixMapGroup() {
    Clear();
}

void CMatrixMapGroup::Clear(void) {
    DTRACE();

    if (m_BigVB_bottom && m_VertsSource_bottom.size > 0) {
        if (m_BigVB_bottom->DelSource(&m_VertsSource_bottom)) {
            CBigVB<SMatrixMapVertexBottom>::DestroyBigVB(m_BigVB_bottom);
            m_BigVB_bottom = NULL;
        }
        HFree(m_VertsSource_bottom.verts, g_MatrixHeap);
    }
    if (m_BigIB_bottom && m_IdxsSource_bottom.size > 0) {
        if (m_BigIB_bottom->DelSource(&m_IdxsSource_bottom)) {
            CBigIB::DestroyBigIB(m_BigIB_bottom);
            m_BigIB_bottom = NULL;
        }
        HFree(m_IdxsSource_bottom.inds, g_MatrixHeap);
    }

    if (m_Objects) {
        HFree(m_Objects, g_MatrixHeap);
        m_Objects = NULL;
        m_ObjectsContained = 0;
    }

    if (m_BottomGeometry) {
        HFree(m_BottomGeometry, g_MatrixHeap);
        m_BottomGeometry = NULL;
        m_BottomGeometryCount = 0;
    }

    if (m_Surfaces) {
        HFree(m_Surfaces, g_MatrixHeap);
        m_Surfaces = NULL;
        m_SurfacesCnt = 0;
    }

    if (m_Shadows) {
        HFree(m_Shadows, g_MatrixHeap);
        m_Shadows = NULL;
        m_ShadowsCnt = 0;
    }

    if (m_WaterAlpha) {
        CCache::Destroy(m_WaterAlpha);
        m_WaterAlpha = NULL;
    }

    if (HasWater()) {
        while (m_InshorewavesCnt > 0) {
            m_Inshorewaves[--m_InshorewavesCnt].Release();
        }
        if (m_Inshorewaves) {
            HFree(m_Inshorewaves, g_MatrixHeap);
            m_Inshorewaves = NULL;
        }
        if (m_PreInshorewaves) {
            HFree(m_PreInshorewaves, g_MatrixHeap);
            m_PreInshorewaves = NULL;
        }
    }

    if (m_RenderShadowObject) {
        m_RenderShadowObject->Release();
        m_RenderShadowObject = NULL;
    }

    if (m_VertsTrace) {
        HFree(m_VertsTrace, g_MatrixHeap);
        m_VertsTrace = NULL;
    }
    if (m_IdxsTrace) {
        HFree(m_IdxsTrace, g_MatrixHeap);
        m_IdxsTrace = NULL;
    }
    m_IdxsTraceCnt = 0;
    m_VertsTraceCnt = 0;
}

void CMatrixMapGroup::AddShadow(CMatrixShadowProj *s) {
    if (m_Shadows) {
        int i = 0;
        while (i < m_ShadowsCnt) {
            if (m_Shadows[i] == s)
                return;
            ++i;
        }
    }
    m_Shadows =
            (CMatrixShadowProj **)HAllocEx(m_Shadows, (m_ShadowsCnt + 1) * sizeof(CMatrixShadowProj *), g_MatrixHeap);
    m_Shadows[m_ShadowsCnt] = s;
    ++m_ShadowsCnt;
}

void CMatrixMapGroup::RemoveShadow(const CMatrixShadowProj *s) {
    if (m_Shadows) {
        int i = 0;
        while (i < m_ShadowsCnt) {
            if (m_Shadows[i] == s) {
                m_Shadows[i] = m_Shadows[--m_ShadowsCnt];
                if (m_ShadowsCnt == 0) {
                    HFree(m_Shadows, g_MatrixHeap);
                    m_Shadows = NULL;
                }
                break;
            }
            ++i;
        }
    }
}

void CMatrixMapGroup::DrawShadowProj(void) {
    DTRACE();

    if (m_CamDistSq > DRAW_SHADOWS_DISTANCE_SQ)
        return;
    D3DXMATRIX m = g_MatrixMap->GetIdentityMatrix();

    for (int i = 0; i < m_ShadowsCnt; ++i) {
        if (m_Shadows[i]->AlreadyDraw())
            continue;

        m._41 = m_Shadows[i]->GetDX();
        m._42 = m_Shadows[i]->GetDY();
        ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m));

        if (m_Shadows[i]->GetTexture() == NULL) {
            if (m_RenderShadowObject == NULL)
                m_RenderShadowObject = m_Shadows[i]->GetOwner()->GetCore(DEBUG_CALL_INFO);
        }
        else {
            m_Shadows[i]->Render();
        }
    }
}

bool CMatrixMapGroup::IsInFrustum(void) const {
    D3DXVECTOR3 mins(p0.x, p0.y, m_minz);
    D3DXVECTOR3 maxs(p1.x, p1.y, m_maxz_obj_robots);
    return g_MatrixMap->m_Camera.IsInFrustum(mins, maxs);
    // return g_MatrixMap->IsInFrustum((mins+maxs) * 0.5f);
}

void CMatrixMapGroup::BuildBottom(int x, int y, BYTE *rawbottom) {
    DTRACE();

    m_PosX = x;
    m_PosY = y;

    m_Matrix = g_MatrixMap->GetIdentityMatrix();

    int w = std::min(MAP_GROUP_SIZE, (g_MatrixMap->m_Size.x - x));
    int h = std::min(MAP_GROUP_SIZE, (g_MatrixMap->m_Size.y - y));

    m_Matrix._41 = ((w >> 1) + x) * GLOBAL_SCALE;
    m_Matrix._42 = ((h >> 1) + y) * GLOBAL_SCALE;

    p0.x = x * GLOBAL_SCALE;
    p0.y = y * GLOBAL_SCALE;

    p1.x = (x + w) * GLOBAL_SCALE;
    p1.y = (y + h) * GLOBAL_SCALE;

    m_maxz = -10000.0;
    m_minz = 10000.0;

    const float macrotexturestep = 1.0f / g_MatrixMap->m_MacrotextureSize;

    m_IdxsSource_bottom.size = 0;
    m_IdxsSource_bottom.inds = NULL;

    m_BottomGeometryCount = *(DWORD *)rawbottom;
    rawbottom += sizeof(DWORD);
    m_BottomGeometry = (SBottomGeometry *)HAlloc(sizeof(SBottomGeometry) * m_BottomGeometryCount, g_MatrixHeap);

    for (int i = 0; i < m_BottomGeometryCount; ++i) {
        m_BottomGeometry[i].texture = *(DWORD *)rawbottom;
        rawbottom += sizeof(DWORD);

        // if (m_BottomGeometry[i].texture < 0) downz = -0.1f;

        DWORD sz = *(DWORD *)rawbottom;
        rawbottom += sizeof(DWORD);

        m_IdxsSource_bottom.inds =
                (WORD *)HAllocEx(m_IdxsSource_bottom.inds, m_IdxsSource_bottom.size + sz, g_MatrixHeap);

        m_BottomGeometry[i].idxbase = m_IdxsSource_bottom.size / 2;

        WORD *ib = (WORD *)(((BYTE *)m_IdxsSource_bottom.inds) + m_IdxsSource_bottom.size);
        WORD *ie = (WORD *)(((BYTE *)ib) + sz);

        memcpy(ib, rawbottom, sz);
        m_IdxsSource_bottom.size += sz;
        rawbottom += sz;

        m_BottomGeometry[i].tricnt = sz / (2 * 3);

        int mini = 1000000;
        int maxi = -1000000;

        for (; ib < ie; ++ib) {
            if (*ib < mini)
                mini = *ib;
            if (*ib > maxi)
                maxi = *ib;
        }
        m_BottomGeometry[i].minvidx = mini;
        m_BottomGeometry[i].vertcnt = maxi - mini + 1;
    }

    DWORD vsz = *(DWORD *)rawbottom;
    rawbottom += sizeof(DWORD);
    int vertcnt = vsz / sizeof(SCompileBottomVert);

    m_VertsSource_bottom.size = vertcnt * sizeof(SMatrixMapVertexBottom);
    m_VertsSource_bottom.verts = (SMatrixMapVertexBottom *)HAlloc(m_VertsSource_bottom.size, g_MatrixHeap);

    static const double ts_inv = 1.0 / double(TEX_BOTTOM_SIZE * g_MatrixMap->m_TexUnionDim);

    for (int i = 0; i < vertcnt; ++i) {
        SCompileBottomVert *v = (SCompileBottomVert *)rawbottom;
        rawbottom += sizeof(SCompileBottomVert);
        SMatrixMapPoint *mp = g_MatrixMap->PointGet(v->x, v->y);

        bool down = (mp->color & 0x80000000) != 0;

        while (!down && v->x >= 1 && v->y >= 1 && v->x < g_MatrixMap->m_Size.x && v->y < g_MatrixMap->m_Size.y) {
            SMatrixMapUnit *mu = g_MatrixMap->UnitGet(v->x, v->y);
            if (!mu->IsDown())
                break;
            if (!(mu - 1)->IsDown())
                break;
            if (!(mu - 1 - g_MatrixMap->m_Size.x)->IsDown())
                break;
            if (!(mu - g_MatrixMap->m_Size.x)->IsDown())
                break;
            down = true;
            mp->color |= 0x80000000;
            break;
        }

        m_VertsSource_bottom.verts[i].ivd.v.x = (v->x - x - (w >> 1)) * GLOBAL_SCALE;
        m_VertsSource_bottom.verts[i].ivd.v.y = (v->y - y - (h >> 1)) * GLOBAL_SCALE;
        m_VertsSource_bottom.verts[i].ivd.v.z = mp->z;  // + downz;

        if (down) {
            m_VertsSource_bottom.verts[i].ivd.v -= mp->n * 0.5f;
        }

        m_VertsSource_bottom.verts[i].ivd.defcol = mp->color;
#ifdef LANDSCAPE_BOTTOM_USE_NORMALES
        m_VertsSource_bottom.verts[i].ivd.n = mp->n;
#endif
        m_VertsSource_bottom.verts[i].tc[0].u = float(ts_inv * v->tx);
        m_VertsSource_bottom.verts[i].tc[0].v = float(ts_inv * v->ty);
        m_VertsSource_bottom.verts[i].tc[1].u = macrotexturestep * v->x;
        m_VertsSource_bottom.verts[i].tc[1].v = macrotexturestep * v->y;

        if (mp->z < m_minz)
            m_minz = mp->z;
        if (mp->z > m_maxz)
            m_maxz = mp->z;
    }

    m_maxz_obj = m_maxz;
    m_maxz_obj_robots = m_maxz;

    m_BigIB_bottom->AddSource(&m_IdxsSource_bottom);
    m_BigVB_bottom->AddSource(&m_VertsSource_bottom);

    if (m_minz < 0) {
        SETFLAG(m_Flags, GRPFLAG_HASWATER);
    }
}

void CMatrixMapGroup::BuildWater(int x, int y) {
    if (HasWater()) {
        x *= MAP_GROUP_SIZE;
        y *= MAP_GROUP_SIZE;

        int w = std::min(MAP_GROUP_SIZE, (g_MatrixMap->m_Size.x - x));
        int h = std::min(MAP_GROUP_SIZE, (g_MatrixMap->m_Size.y - y));

        // const int shadesize = alphasize*alphasize;

        D3DLOCKED_RECT lr;

        m_WaterAlpha = CACHE_CREATE_TEXTUREMANAGED();
        // m_WaterAlpha->SetSMTexture(tex);

        if (D3D_OK != m_WaterAlpha->CreateLock(D3DFMT_A8, WATER_ALPHA_SIZE, WATER_ALPHA_SIZE, 1, lr))
            if (D3D_OK != m_WaterAlpha->CreateLock(D3DFMT_A8L8, WATER_ALPHA_SIZE, WATER_ALPHA_SIZE, 1, lr))
                if (D3D_OK != m_WaterAlpha->CreateLock(D3DFMT_A8R3G3B2, WATER_ALPHA_SIZE, WATER_ALPHA_SIZE, 1, lr))
                    m_WaterAlpha->CreateLock(D3DFMT_A8R8G8B8, WATER_ALPHA_SIZE, WATER_ALPHA_SIZE, 1, lr);

        // dword *shade = (dword *)lr.pBits;
        byte *shade = (byte *)lr.pBits;

        const int pxsz = lr.Pitch / WATER_ALPHA_SIZE;

        const float up_level = -1.0f;
        const float down_level = -20.1f;

        for (int j = 0; j < WATER_ALPHA_SIZE; ++j) {
            for (int i = 0; i < WATER_ALPHA_SIZE; ++i) {
                float wx = (float(i) + 0.5f) * ((float)MAP_GROUP_SIZE * GLOBAL_SCALE / (WATER_ALPHA_SIZE)) + p0.x;
                float wy = (float(j) + 0.5f) * ((float)MAP_GROUP_SIZE * GLOBAL_SCALE / (WATER_ALPHA_SIZE)) + p0.y;

                float wz;

                float scaledx = wx / GLOBAL_SCALE;
                float scaledy = wy / GLOBAL_SCALE;

                int ix = TruncFloat(scaledx);
                int iy = TruncFloat(scaledy);

                SMatrixMapUnit *un = g_MatrixMap->UnitGetTest(ix, iy);
                if (un != NULL && un->IsBridge()) {
                    float kx = scaledx - float(ix);
                    float ky = scaledy - float(iy);

                    SMatrixMapPoint *mp = g_MatrixMap->PointGet(ix, iy);

                    float z0 = mp->z;
                    float z1 = (mp + 1)->z;
                    float z2 = (mp + g_MatrixMap->m_Size.x + 1)->z;
                    float z3 = (mp + g_MatrixMap->m_Size.x + 2)->z;

                    wz = LERPFLOAT(ky, LERPFLOAT(kx, z0, z1), LERPFLOAT(kx, z2, z3));
                }
                else {
                    wz = g_MatrixMap->GetZ(wx, wy);
                }

                byte zz;
                if (wz < down_level)
                    zz = 255;
                else if (wz > up_level)
                    zz = 0;
                else
                    zz = BYTE(255 - int(((wz - down_level) / (up_level - down_level) * 255.0f)));

                *(shade + (j * lr.Pitch) + (i * pxsz) + (pxsz - 1)) = zz;
                // shade[i+j*alphasize] = (int(zz) << shift);
            }
        }

        m_WaterAlpha->UnlockRect();

        if (m_Inshorewaves == NULL && !FLAG(g_MatrixMap->m_Flags, MMFLAG_DISABLEINSHORE_BUILD)) {
            m_Inshorewaves = (SInshorewave *)HAlloc(INSHOREWAVES_CNT * sizeof(SInshorewave), g_MatrixHeap);
            m_InshorewavesCnt = 0;
            m_InshoreTime = 0;

            D3DXVECTOR2 pos;
            D3DXVECTOR2 dir;

            CPoint posc[MAP_GROUP_SIZE * MAP_GROUP_SIZE];
            int posc_cnt = 0;

            for (int yy = 0; yy < h; ++yy) {
                for (int xx = 0; xx < w; ++xx) {
                    SMatrixMapUnit *mu = g_MatrixMap->UnitGet(x + xx, y + yy);
                    if (mu->IsInshore()) {
                        posc[posc_cnt++] = CPoint(xx, yy);
                    }
                    mu->ResetInshore();
                }
            }

            while (posc_cnt > INSHORE_PRE_COUNT) {
                posc[IRND(posc_cnt)] = posc[posc_cnt - 1];
                --posc_cnt;
            }

            while (posc_cnt-- > 0) {
                pos = D3DXVECTOR2((posc[posc_cnt].x + x) * GLOBAL_SCALE + FRND(GLOBAL_SCALE),
                                  (posc[posc_cnt].y + y) * GLOBAL_SCALE + FRND(GLOBAL_SCALE));
                // float z = g_MatrixMap->GetZ(pos.x,pos.y);
                // if (z >= (WATER_LEVEL-3.0f) || z <= (WATER_LEVEL-10.0f)) continue;
                g_MatrixMap->CalcVectorToLandscape(pos, dir);

                pos -= dir * 20;

                if (m_PreInshorewaves) {
                    // if (m_PreInshorewavesCnt < INSHORE_PRE_COUNT)
                    //{
                    m_PreInshorewaves[m_PreInshorewavesCnt].pos = pos;
                    m_PreInshorewaves[m_PreInshorewavesCnt].dir = dir;
                    m_PreInshorewaves[m_PreInshorewavesCnt].used = false;
                    ++m_PreInshorewavesCnt;
                    //} else
                    //    break;
                }
                else {
                    m_PreInshorewaves =
                            (SPreInshorewave *)HAlloc(INSHORE_PRE_COUNT * sizeof(SPreInshorewave), g_MatrixHeap);
                    m_PreInshorewaves[0].pos = pos;
                    m_PreInshorewaves[0].dir = dir;
                    m_PreInshorewaves[0].used = false;
                    m_PreInshorewavesCnt = 1;
                }
            }
        }
    }
}

void CMatrixMapGroup::InitInshoreWaves(int n, const float *xx, const float *yy, const float *nxx, const float *nyy) {
    DTRACE();

    m_Inshorewaves = (SInshorewave *)HAlloc(INSHOREWAVES_CNT * sizeof(SInshorewave), g_MatrixHeap);
    m_InshorewavesCnt = 0;
    m_InshoreTime = 0;

    m_PreInshorewaves = (SPreInshorewave *)HAlloc(n * sizeof(SPreInshorewave), g_MatrixHeap);
    m_PreInshorewavesCnt = n;

    for (int t = 0; t < n; ++t) {
        m_PreInshorewaves[t].pos = D3DXVECTOR2(xx[t], yy[t]);
        m_PreInshorewaves[t].dir = D3DXVECTOR2(nxx[t], nyy[t]);
        m_PreInshorewaves[t].used = false;
    }
}

void CMatrixMapGroup::BeforeDraw(void) {
#ifdef _DEBUG
    if (this == NULL) {
        debugbreak();
    }
#endif

    m_RemindCore.Use(FREE_TIME_PERIOD);
    DX_Prepare();

    for (int i = 0; i < m_BottomGeometryCount; ++i) {
        if (m_BottomGeometry[i].texture >= 0)
            CBottomTextureUnion::Get(m_BottomGeometry[i].texture).Prepare();
    }

    if (HasWater())
        GetWaterAlpha()->Preload();

    if (m_RenderShadowObject) {
        if (m_RenderShadowObject->m_Object) {
            m_RenderShadowObject->m_Object->RNeed(MR_ShadowProjTex | MR_Matrix | MR_Graph);
        }

        m_RenderShadowObject->Release();
        m_RenderShadowObject = NULL;
    }
}
void CMatrixMapGroup::Draw(void) {
    DTRACE();

    int vbase = m_VertsSource_bottom.Select(m_BigVB_bottom);
    if (vbase < 0)
        return;
    int ibase = m_IdxsSource_bottom.Select(m_BigIB_bottom);
    if (ibase < 0)
        return;

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m_Matrix));

    int type = (g_MatrixMap->m_Macrotexture == NULL) ? 0 : 1;

    for (int i = 0; i < m_BottomGeometryCount; ++i) {
        SBottomGeometry *bg = m_BottomGeometry + i;

        if (bg->texture < 0) {
            // ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD,&m));

            SetAlphaOpDisable(0);
            SetColorOpDisable(0);

            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0));
            ASSERT_DX(g_D3DD->SetTexture(0, NULL));
            ASSERT_DX(g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vbase, bg->minvidx, bg->vertcnt,
                                                   ibase + bg->idxbase, bg->tricnt));
            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF));

#ifdef _DEBUG
            m_DPCalls++;
#endif
        }
        else {
            for (int pass = 0; pass < g_Render->m_TerBotPass[type]; ++pass) {
                g_Render->m_TerBotTex[type](bg->texture, pass);
                g_Render->m_TerBot[type](pass);

                ASSERT_DX(g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vbase, bg->minvidx, bg->vertcnt,
                                                       ibase + bg->idxbase, bg->tricnt));
#ifdef _DEBUG
                m_DPCalls++;
#endif
                // ASSERT_DX(g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,4,0,2));
            }
            g_Render->m_TerBotClear[type]();
        }
    }

    //{
    //    static int i1 = -1;
    //    int prei = i1;
    //    if (g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_F7) {g_MatrixMap->m_KeyDown = false; i1--; if (i1
    //    < 0) i1 = CTerSurface::GetSufracesCount()-1;} if (g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_F8)
    //    {g_MatrixMap->m_KeyDown = false; i1++; if (i1 >= CTerSurface::GetSufracesCount()) i1 = 0;} CTerSurface *su =
    //    CTerSurface::GetSufrace(i1); if (prei != i1 && prei >=0) CTerSurface::GetSufrace(prei)->unstable = false;
    //    su->unstable = true;

    //    for (int i=0;i<m_SurfacesCnt;++i)
    //    {
    //        if (m_Surfaces[i] == su)
    //        {
    //            DrawBBox();
    //            break;
    //        }
    //    }

    //}

    // DrawBBox();

    // int xx = p0.x / GLOBAL_SCALE;
    // int yy = p0.y / GLOBAL_SCALE;

    // for (int i=0;i<11;++i)
    //    for (int j=0;j<11;++j)
    //    {
    //        SMatrixMapPoint *mp = g_MatrixMap->PointGetTest(i + xx, j+yy);
    //        if (mp && mp->color & 0x80000000)
    //        {
    //            D3DXVECTOR3 from((i + xx)*GLOBAL_SCALE,(j + yy)*GLOBAL_SCALE,mp->z);
    //            CHelper::Create(1,0)->Sphere(from, 5,15, 0xFF00FF00);
    //        }
    //    }
}

#if (defined _DEBUG) && !(defined _RELDEBUG)
void CMatrixMapGroup::DrawBBox(void) {
    CHelper::Create(1, 0)->BoundBox(D3DXVECTOR3(p0.x, p0.y, m_minz), D3DXVECTOR3(p1.x, p1.y, m_maxz_obj_robots));
}

#endif
//////// objects

CMatrixMapStatic *CMatrixMapGroup::FindObjectAny(DWORD mask, const D3DXVECTOR2 &pos, float maxdist, float scale_radius,
                                                 int &i) {
    DTRACE();
    // float dmax = maxdist*maxdist;
    //    float mindist = maxdist * 2;
    CMatrixMapStatic **ms = m_Objects + i;
    int cnt = m_ObjectsContained - i;
    for (; cnt-- > 0; ++ms) {
        if (!(*ms)->FitToMask(mask))
            continue;

        auto tmp = *(D3DXVECTOR2 *)&(*ms)->GetGeoCenter() - pos;
        float dist =
                D3DXVec2Length(&tmp) - (*ms)->GetRadius() * scale_radius;
        // if (dist< 0) dist = 0;
        if (dist < maxdist) {
            i = ms - m_Objects + 1;
            return *ms;
        }
    }
    return NULL;
}

CMatrixMapStatic *CMatrixMapGroup::FindObjectAny(DWORD mask, const D3DXVECTOR3 &pos, float maxdist, float scale_radius,
                                                 int &i) {
    DTRACE();
    // float dmax = maxdist*maxdist;
    //    float mindist = maxdist * 2;
    CMatrixMapStatic **ms = m_Objects + i;
    int cnt = m_ObjectsContained - i;
    for (; cnt-- > 0; ++ms) {
        if (!(*ms)->FitToMask(mask))
            continue;

        auto tmp = (*ms)->GetGeoCenter() - pos;
        float dist = D3DXVec3Length(&tmp) - (*ms)->GetRadius() * scale_radius;
        // if (dist< 0) dist = 0;
        if (dist < maxdist) {
            i = ms - m_Objects + 1;
            return *ms;
        }
    }
    return NULL;
}

void CMatrixMapGroup::SortObjects(const D3DXMATRIX &sort) {
    DTRACE();
    ////////////////////////// draw group objects
    if (m_Objects) {
        CMatrixMapStatic **mo;

        // Draw object
        mo = m_Objects;

        int cnt = m_ObjectsContained;

        while ((cnt--) > 0) {
            (*mo)->Sort(sort);
            ++mo;
        }
    }
}

void CMatrixMapGroup::PauseTakt(int step) {
    auto tmp = g_MatrixMap->m_Camera.GetFrustumCenter() -
                               D3DXVECTOR3(0.5f * (p0.x + p1.x), 0.5f * (p0.y + p1.y), 0.5f * (m_minz + m_maxz));
    m_CamDistSq = D3DXVec3LengthSq(&tmp);
}

void CMatrixMapGroup::GraphicTakt(int step) {
    // inshore waves...
    auto tmp = g_MatrixMap->m_Camera.GetFrustumCenter() -
                               D3DXVECTOR3(0.5f * (p0.x + p1.x), 0.5f * (p0.y + p1.y), 0.5f * (m_minz + m_maxz));

    m_CamDistSq = D3DXVec3LengthSq(&tmp);

    if (!HasWater())
        return;
    if (m_PreInshorewavesCnt == 0)
        return;

    float time = INSHORE_SPEED * float(step);
    if (time > 0.5f)
        time = 0.5f;
    int i = 0;
    while (i < m_InshorewavesCnt) {
        m_Inshorewaves[i].t += time * m_Inshorewaves[i].speed;
        m_Inshorewaves[i].scale = 13 + KSCALE(m_Inshorewaves[i].t, 0, 0.7f) * 7;

        if (m_Inshorewaves[i].t > 1.0f) {
            m_PreInshorewaves[m_Inshorewaves[i].m_Index].used = false;
            m_Inshorewaves[i].Release();
            --m_InshorewavesCnt;
            m_Inshorewaves[i] = m_Inshorewaves[m_InshorewavesCnt];
            continue;
        }
        ++i;
    }

    if (m_InshorewavesCnt >= INSHOREWAVES_CNT)
        return;

    int idx = IRND(m_PreInshorewavesCnt);
    if (!m_PreInshorewaves[idx].used) {
        SInshorewave::Create(idx, m_PreInshorewaves[idx].pos, m_PreInshorewaves[idx].dir,
                             m_Inshorewaves[m_InshorewavesCnt]);
        m_PreInshorewaves[idx].used = true;
        ++m_InshorewavesCnt;
    }
}

void CMatrixMapGroup::DrawInshorewaves(void) {
    int i = 0;
    while (i < m_InshorewavesCnt) {
        m_Inshorewaves[i].Draw();
        ++i;
    }
}

void CMatrixMapGroup::RecalcMaxZ(void) {
    int cnt = m_ObjectsContained;
    m_maxz_obj = m_maxz;
    m_maxz_obj_robots = m_maxz;
    while (--cnt >= 0) {
        CMatrixMapStatic *ms = m_Objects[cnt];
        if (ms->GetObjectType() == OBJECT_TYPE_FLYER)
            continue;

        D3DXVECTOR3 mins, maxs;
        ms->CalcBounds(mins, maxs);

        if (ms->IsLiveRobot()) {
            if (maxs.z > m_maxz_obj_robots)
                m_maxz_obj_robots = maxs.z;
        }
        else {
            if (maxs.z > m_maxz_obj)
                m_maxz_obj = maxs.z;
        }
    }
    if (m_maxz_obj_robots < m_maxz_obj)
        m_maxz_obj_robots = m_maxz_obj;
}

void CMatrixMapGroup::AddSurface(CTerSurface *surf) {
    ++m_SurfacesCnt;
    m_Surfaces = (CTerSurface **)HAllocEx(m_Surfaces, sizeof(CTerSurface *) * m_SurfacesCnt, g_MatrixHeap);
    m_Surfaces[m_SurfacesCnt - 1] = surf;
}

void CMatrixMapGroup::BeforeDrawSurfaces(void) {
    for (int i = 0; i < m_SurfacesCnt; ++i) {
        m_Surfaces[i]->BeforeDraw();
    }
}

void CMatrixMapGroup::AddObject(CMatrixMapStatic *obj) {
    DTRACE();

    if (m_Objects) {
        if (m_ObjectsContained < m_ObjectsAllocated) {
            m_Objects[m_ObjectsContained++] = obj;
        }
        else {
            m_ObjectsAllocated += 16;
            m_Objects = (CMatrixMapStatic **)HAllocEx(m_Objects, m_ObjectsAllocated * sizeof(CMatrixMapStatic **),
                                                      g_MatrixHeap);
            m_Objects[m_ObjectsContained++] = obj;
        }
    }
    else {
        m_ObjectsAllocated = 16;
        m_ObjectsContained = 1;
        m_Objects = (CMatrixMapStatic **)HAlloc(m_ObjectsAllocated * sizeof(CMatrixMapStatic **), g_MatrixHeap);
        m_Objects[0] = obj;
    }

    // if (obj->GetObjectType() == OBJECT_TYPE_FLYER)
    //{
    //    SETFLAG(m_Flags, GRPFLAG_HASFLYER);
    //} else
    if (obj->IsBase()) {
        SETFLAG(m_Flags, GRPFLAG_HASBASE);
    }
}

void CMatrixMapGroup::SubObject(CMatrixMapStatic *obj) {
    DTRACE();

    int i = 0;

    ASSERT(m_ObjectsContained > 0);

    do {
        if (m_Objects[i] == obj) {
            m_Objects[i] = m_Objects[--m_ObjectsContained];
            // if (obj->GetObjectType() == OBJECT_TYPE_FLYER)
            //{
            //    RESETFLAG(m_Flags, GRPFLAG_HASFLYER);
            //} else
            if (obj->IsBase()) {
                RESETFLAG(m_Flags, GRPFLAG_HASBASE);
            }
#ifdef _DEBUG
            // check that
            while (i < m_ObjectsContained) {
                if (m_Objects[i] == obj)
                    ERROR_S(L"The same object is in the group with more than one times");
                ++i;
            }
#endif
            return;
        }
        ++i;
    }
    while (i < m_ObjectsContained);

    ERROR_S(L"Object not found in the group");
}

bool CMatrixMapGroup::Pick(const D3DXVECTOR3 &pos, const D3DXVECTOR3 &_dir, float &outt) {
    D3DXVECTOR3 _orig(pos - *(D3DXVECTOR3 *)&m_Matrix._41);

    if (m_IdxsTraceCnt == 0) {
        // rebuilding trace geometry
        m_VertsTrace =
                (D3DXVECTOR3 *)HAlloc(sizeof(D3DXVECTOR3) * (MAP_GROUP_SIZE + 1) * (MAP_GROUP_SIZE + 1), g_MatrixHeap);
        m_IdxsTrace = (int *)HAlloc(sizeof(int) * MAP_GROUP_SIZE * MAP_GROUP_SIZE * 2 * 3, g_MatrixHeap);
        int sz = sizeof(int) * (MAP_GROUP_SIZE + 1) * (MAP_GROUP_SIZE + 1);
        int *set = (int *)_alloca(sz);
        memset(set, -1, sz);

        int x = m_PosX;
        int y = m_PosY;

        int w = std::min(MAP_GROUP_SIZE, (g_MatrixMap->m_Size.x - x));
        int h = std::min(MAP_GROUP_SIZE, (g_MatrixMap->m_Size.y - y));

        int cv = 0;
        int ci = 0;

        SMatrixMapPoint *mp = g_MatrixMap->PointGet(x, y);
        for (int yy = 0; yy < h; ++yy, mp += g_MatrixMap->m_Size.x - w + 1) {
            for (int xx = 0; xx < w; ++xx, ++mp) {
                SMatrixMapUnit *mu = g_MatrixMap->UnitGet(x + xx, y + yy);
                if (mu->IsLand()) {
                    int mul = (MAP_GROUP_SIZE + 1) * (yy + 0);
                    int *v1 = set + mul + xx;
                    int *v3 = set + mul + xx + 1;
                    mul += (MAP_GROUP_SIZE + 1);
                    int *v0 = set + mul + xx;
                    int *v2 = set + mul + xx + 1;

                    if (*v0 < 0) {
                        m_VertsTrace[cv].x = ((xx + 0) - (w >> 1)) * GLOBAL_SCALE;
                        m_VertsTrace[cv].y = ((yy + 1) - (h >> 1)) * GLOBAL_SCALE;
                        m_VertsTrace[cv].z = (mp + g_MatrixMap->m_Size.x + 1)->z;
                        *v0 = cv++;
                    }

                    if (*v1 < 0) {
                        m_VertsTrace[cv].x = ((xx + 0) - (w >> 1)) * GLOBAL_SCALE;
                        m_VertsTrace[cv].y = ((yy + 0) - (h >> 1)) * GLOBAL_SCALE;
                        m_VertsTrace[cv].z = (mp)->z;
                        *v1 = cv++;
                    }

                    if (*v2 < 0) {
                        m_VertsTrace[cv].x = ((xx + 1) - (w >> 1)) * GLOBAL_SCALE;
                        m_VertsTrace[cv].y = ((yy + 1) - (h >> 1)) * GLOBAL_SCALE;
                        m_VertsTrace[cv].z = (mp + g_MatrixMap->m_Size.x + 2)->z;
                        *v2 = cv++;
                    }

                    if (*v3 < 0) {
                        m_VertsTrace[cv].x = ((xx + 1) - (w >> 1)) * GLOBAL_SCALE;
                        m_VertsTrace[cv].y = ((yy + 0) - (h >> 1)) * GLOBAL_SCALE;
                        m_VertsTrace[cv].z = (mp + 1)->z;
                        *v3 = cv++;
                    }

                    m_IdxsTrace[ci + 0] = *v0;
                    m_IdxsTrace[ci + 1] = *v1;
                    m_IdxsTrace[ci + 2] = *v2;

                    m_IdxsTrace[ci + 3] = *v1;
                    m_IdxsTrace[ci + 4] = *v3;
                    m_IdxsTrace[ci + 5] = *v2;

                    ci += 6;
                }
            }
        }

        m_IdxsTraceCnt = ci;
        m_VertsTraceCnt = cv;
    }

    BYTE *vflags = (BYTE *)_alloca(m_VertsTraceCnt);

    D3DXVECTOR3 *cv;

    D3DXVECTOR3 adir;

    *(((DWORD *)&adir) + 0) = *(((DWORD *)&_dir) + 0) & 0x7FFFFFFF;  // adir.x = abs(pd._dir.x);
    *(((DWORD *)&adir) + 1) = *(((DWORD *)&_dir) + 1) & 0x7FFFFFFF;  // adir.y = abs(pd._dir.y);
    *(((DWORD *)&adir) + 2) = *(((DWORD *)&_dir) + 2) & 0x7FFFFFFF;  // adir.z = abs(pd._dir.z);

    if (adir.x >= adir.y) {
        if (adir.x >= adir.z) {
            // YZ

            float kk = 1.0f / _dir.x;
            float k[2] = {-_dir.y * kk, -_dir.z * kk};
            float coordc[2] = {_orig.y + _orig.x * k[0], _orig.z + _orig.x * k[1]};
            for (int i = 0; i < m_VertsTraceCnt; ++i) {
                cv = m_VertsTrace + i;

                float coord[2] = {cv->y + cv->x * k[0] - coordc[0], cv->z + cv->x * k[1] - coordc[1]};
                DWORD t0 = *((DWORD *)&coord[0]);
                DWORD t1 = *((DWORD *)&coord[1]);
                BYTE r0 = (BYTE((t0 >> 31) + 1) & BYTE(((t0 & 0x7FFFFFF) == 0) ? 0 : -1));
                BYTE r1 = (BYTE(((t1 >> 31) + 1) << 2) & BYTE(((t1 & 0x7FFFFFF) == 0) ? 0 : -1));

                *(vflags + i) = r0 | r1;
            }
        }
        else {
            // XY
            goto calcxy;
        }
    }
    else {
        if (adir.y >= adir.z) {
            // XZ
            float kk = 1.0f / _dir.y;
            float k[2] = {k[0] = -_dir.x * kk, k[1] = -_dir.z * kk};

            float coordc[2] = {_orig.x + _orig.y * k[0], _orig.z + _orig.y * k[1]};
            for (int i = 0; i < m_VertsTraceCnt; ++i) {
                cv = m_VertsTrace + i;

                float coord[2] = {cv->x + cv->y * k[0] - coordc[0], cv->z + cv->y * k[1] - coordc[1]};
                DWORD t0 = *((DWORD *)&coord[0]);
                DWORD t1 = *((DWORD *)&coord[1]);
                BYTE r0 = (BYTE((t0 >> 31) + 1) & BYTE(((t0 & 0x7FFFFFF) == 0) ? 0 : -1));
                BYTE r1 = (BYTE(((t1 >> 31) + 1) << 2) & BYTE(((t1 & 0x7FFFFFF) == 0) ? 0 : -1));

                *(vflags + i) = r0 | r1;
            }
        }
        else {
        calcxy:
            // XY

            float kk = 1.0f / _dir.z;
            float k[2] = {k[0] = -_dir.x * kk, -_dir.y * kk};

            float coordc[2] = {_orig.x + _orig.z * k[0], _orig.y + _orig.z * k[1]};

            for (int i = 0; i < m_VertsTraceCnt; ++i) {
                cv = m_VertsTrace + i;

                float coord[2] = {cv->x + cv->z * k[0] - coordc[0], cv->y + cv->z * k[1] - coordc[1]};
                DWORD t0 = *((DWORD *)&coord[0]);
                DWORD t1 = *((DWORD *)&coord[1]);
                BYTE r0 = (BYTE((t0 >> 31) + 1) & BYTE(((t0 & 0x7FFFFFF) == 0) ? 0 : -1));
                BYTE r1 = (BYTE(((t1 >> 31) + 1) << 2) & BYTE(((t1 & 0x7FFFFFF) == 0) ? 0 : -1));

                *(vflags + i) = r0 | r1;
            }
        }
    }

    int *trb = m_IdxsTrace;
    int *tre = trb + m_IdxsTraceCnt;

    float kt, ku, kv, okt = 1e20f;
    bool rv = false;

    for (; trb < tre; trb += 3) {
        if ((*(vflags + *(trb + 0)) & *(vflags + *(trb + 1)) & *(vflags + *(trb + 2))) == 0) {
            if (IntersectTriangle(_orig, _dir, *(m_VertsTrace + *(trb + 0)), *(m_VertsTrace + *(trb + 1)),
                                  *(m_VertsTrace + *(trb + 2)), kt, ku, kv) &&
                kt < okt && kt >= 0.0f) {
                okt = kt;
                rv = true;
            }
        }
    }

    if (rv) {
        outt = okt;
    }
    return rv;
}
