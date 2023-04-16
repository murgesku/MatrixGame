// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <algorithm>

#include "MatrixMap.hpp"
#include "MatrixShadowManager.hpp"
#include "MatrixObject.hpp"

CMatrixShadowProj::~CMatrixShadowProj() {
    int cnt = g_MatrixMap->m_GroupSize.x * g_MatrixMap->m_GroupSize.y;
    CMatrixMapGroup **mg = g_MatrixMap->m_Group;
    for (; cnt > 0; --cnt) {
        if (*mg) {
            (*mg)->RemoveShadow(this);
        }
        ++mg;
    }
}

bool CMatrixShadowProj::AlreadyDraw(void) {
    if (m_DrawFrame == g_MatrixMap->GetCurrentFrame())
        return true;
    m_DrawFrame = g_MatrixMap->GetCurrentFrame();
    return false;
}

BYTE *CMatrixShadowProj::ReadGeometry(BYTE *raw) {
    CPoint minp;
    memcpy(&minp, raw, sizeof(CPoint));
    raw += sizeof(CPoint);

    m_DX = minp.x * GLOBAL_SCALE;
    m_DY = minp.y * GLOBAL_SCALE;

    m_VertCnt = *(int *)raw;
    raw += sizeof(int);
    m_preVB.size = m_VertCnt * sizeof(SVOShadowProjVertex);
    m_preIB.size = *(int *)raw;
    raw += sizeof(int);
    m_preVB.verts = (SVOShadowProjVertex *)HAlloc(m_preVB.size + m_preIB.size, m_Heap);
    for (int i = 0; i < m_VertCnt; ++i) {
        struct SP {
            WORD x, y;
            float tu, tv;
        } * spt;

        spt = (SP *)raw;
        raw += sizeof(SP);

        SMatrixMapPoint *mp = g_MatrixMap->PointGet(spt->x, spt->y);

        m_preVB.verts[i].v.x = (spt->x - minp.x) * GLOBAL_SCALE;
        m_preVB.verts[i].v.y = (spt->y - minp.y) * GLOBAL_SCALE;
        m_preVB.verts[i].v.z = mp->z;

        D3DXVECTOR3 N;
        g_MatrixMap->GetNormal(&N, (spt->x) * GLOBAL_SCALE, (spt->y) * GLOBAL_SCALE);
        m_preVB.verts[i].v += N * 0.1f;

        m_preVB.verts[i].tu = spt->tu;
        m_preVB.verts[i].tv = spt->tv;
    }

    // m_preIB.inds = (WORD *)HAlloc(m_preIB.size, m_Heap);
    m_preIB.inds = (WORD *)(((BYTE *)m_preVB.verts) + m_preVB.size);
    memcpy(m_preIB.inds, raw, m_preIB.size);

    m_TriCnt = m_preIB.size / sizeof(WORD) - 2;

    m_VB->AddSource(&m_preVB);
    m_IB->AddSource(&m_preIB);

    return raw + m_preIB.size;
}

static void ShadowProjBuildGeomInt(CVOShadowProj &sp, const SProjData &pd, const D3DXMATRIX &objma, int mapradius,
                                   bool join_to_group) {
    DTRACE();

    int msx = Float2Int(objma._41 / GLOBAL_SCALE) - mapradius;
    int msy = Float2Int(objma._42 / GLOBAL_SCALE) - mapradius;
    int mex = std::min(g_MatrixMap->m_Size.x, msx + mapradius * 2);
    int mey = std::min(g_MatrixMap->m_Size.y, msy + mapradius * 2);
    msx = std::max(0, msx);
    msy = std::max(0, msy);
    if (msx >= mex || msy >= mey)
        return;

    D3DXVECTOR3 vpos((pd.vx + pd.vy) * 0.5f + pd.vpos), vx, vy, vz;

    D3DXVec3TransformCoord(&vpos, &vpos, &objma);
    D3DXVec3TransformNormal(&vx, &pd.vx, &objma);
    D3DXVec3TransformNormal(&vy, &pd.vy, &objma);
    D3DXVec3TransformNormal(&vz, &pd.vz, &objma);

    float _sx = -1.0f / D3DXVec3Length(&vx);
    float _sy = -1.0f / D3DXVec3Length(&vy);

    D3DXMATRIX mView;
    auto tmp1 = vpos - vz;
    auto tmp2 = -vy;
    D3DXMatrixLookAtLH(&mView, &tmp1, &vpos, &tmp2);

    mView._11 *= _sx;
    mView._21 *= _sx;
    mView._31 *= _sx;
    mView._41 *= _sx;
    mView._12 *= _sy;
    mView._22 *= _sy;
    mView._32 *= _sy;
    mView._42 *= _sy;

    typedef struct {
        D3DXVECTOR3 p;
        float tu, tv;
        bool outside;
        int index;
    } STempVertex;

    int x, y;
    SMatrixMapUnit *un;

    const int da_size_x = (mex - msx + 1);
    const int da_size_y = (mey - msy + 1);
    const int da_size = da_size_y * da_size_x;

    BYTE *buff = (BYTE *)_alloca((sizeof(STempVertex) + sizeof(SVOShadowProjVertex) + sizeof(WORD) * 3) * da_size);

#define TEMP_VERTS() ((STempVertex *)buff)
#define VERTS()      ((SVOShadowProjVertex *)(buff + (sizeof(STempVertex)) * da_size))
#define IDXS()       ((WORD *)(buff + (sizeof(STempVertex) + sizeof(SVOShadowProjVertex)) * da_size))

    STempVertex *tv = TEMP_VERTS();

    float dx, dy;

    dx = GLOBAL_SCALE * float(msx);
    dy = GLOBAL_SCALE * float(msy);

    bool prj_off = true;
    for (y = msy; y <= mey; ++y) {
        for (x = msx; x <= mex; ++x, ++tv) {
            tv->p.x = GLOBAL_SCALE * float(x - msx);
            tv->p.y = GLOBAL_SCALE * float(y - msy);
            tv->p.z = g_MatrixMap->PointGet(x, y)->z + SHADOW_ALTITUDE;

            float tx = tv->p.x + dx;
            float ty = tv->p.y + dy;

            tv->tu = (mView._11 * tx + mView._21 * ty + mView._31 * tv->p.z + mView._41) + 0.5f;
            tv->tv = (mView._12 * tx + mView._22 * ty + mView._32 * tv->p.z + mView._42) + 0.5f;

            tv->outside = (tv->tu < 0 || tv->tu >= 1.0f || tv->tv < 0 || tv->tv >= 1.0f);
            prj_off &= tv->outside;
            tv->index = -1;
        }
    }

    if (prj_off) {
        sp.Prepare(0, 0, 0, 0, 0, 0);
        return;
    }

    int stripscnt = 0;
    int vertscnt = 0;
    int idxscnt = 0;

    bool strip_in_progress = false;

    tv = TEMP_VERTS();

    SVOShadowProjVertex *verts = VERTS();
    WORD *idxs = IDXS();

    CMatrixMapGroup *pg = NULL;

    int cidx = 0;
    for (y = 0; y < (da_size_y - 1); ++y) {
        for (x = 0; x < (da_size_x - 1); ++x, ++tv) {
            if (tv->outside && (tv + 1)->outside && (tv + da_size_x)->outside && (tv + da_size_x + 1)->outside) {
                strip_in_progress = false;
                continue;
            }
            un = g_MatrixMap->UnitGet(x + msx, y + msy);
            if (!un->IsLand()) {
                strip_in_progress = false;
                continue;
            }

            if (join_to_group) {
                int gx = (x + msx) / MAP_GROUP_SIZE;
                int gy = (y + msy) / MAP_GROUP_SIZE;
                CMatrixMapGroup *g = g_MatrixMap->GetGroupByIndexTest(gx, gy);
                if (g != pg) {
                    pg = g;
                    if (g) {
                        g->AddShadow((CMatrixShadowProj *)&sp);
                    }
                }
            }

            STempVertex *tv0 = tv + da_size_x;
            STempVertex *tv1 = tv;
            STempVertex *tv2 = tv + da_size_x + 1;
            STempVertex *tv3 = tv + 1;

            if (tv0->index < 0) {
                tv0->index = cidx++;
                verts->v = tv0->p;
                verts->tu = tv0->tu;
                verts->tv = tv0->tv;
                ++verts;
                ++vertscnt;
            }
            if (tv1->index < 0) {
                tv1->index = cidx++;
                verts->v = tv1->p;
                verts->tu = tv1->tu;
                verts->tv = tv1->tv;
                ++verts;
                ++vertscnt;
            }

            if (tv2->index < 0) {
                tv2->index = cidx++;
                verts->v = tv2->p;
                verts->tu = tv2->tu;
                verts->tv = tv2->tv;
                ++verts;
                ++vertscnt;
            }

            if (tv3->index < 0) {
                tv3->index = cidx++;
                verts->v = tv3->p;
                verts->tu = tv3->tu;
                verts->tv = tv3->tv;
                ++verts;
                ++vertscnt;
            }

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

    // DM("size", CStr(vertscnt));
    sp.Prepare(dx, dy, VERTS(), vertscnt, IDXS(), idxscnt);

    // sp.Prepare();

#undef TEMP_VERTS
#undef STRIPS
#undef VERTS
#undef IDXS
}

void ShadowProjBuildGeom(CVOShadowProj &sp, CVectorObjectAnim &obj, int noframe, const D3DXMATRIX &objma,
                         const D3DXMATRIX &iobjma, D3DXVECTOR3 &light, int mapradius, bool join_to_group) {
    DTRACE();

    SProjData pd;
    D3DXVECTOR3 lv;

    D3DXVec3TransformNormal(&lv, &light, &iobjma);

    obj.CalcShadowProjMatrix(noframe, pd, lv, 6.0f);

    ShadowProjBuildGeomInt(sp, pd, objma, mapradius, join_to_group);
}

void ShadowProjBuildGeomList(CVOShadowProj &sp, int cnt, CVectorObjectAnim **obj, const int *noframe,
                             const D3DXMATRIX *wm, const D3DXMATRIX &objma, const D3DXMATRIX &iobjma,
                             D3DXVECTOR3 &light, int mapradius, bool join_to_group) {
    DTRACE();

    SProjData pd;
    D3DXVECTOR3 lv;

    D3DXVec3TransformNormal(&lv, &light, &iobjma);
    CVectorObject::CalcShadowProjMatrix(cnt, obj, noframe, wm, pd, lv, 6.0f);
    ShadowProjBuildGeomInt(sp, pd, objma, mapradius, join_to_group);
}

void ShadowProjBuildFull(CVOShadowProj &sp, CVectorObjectAnim &obj, int noframe, const D3DXMATRIX &objma,
                         const D3DXMATRIX &iobjma, D3DXVECTOR3 &light, int mapradius, int texsize, bool doclip,
                         bool render_texture, bool join_to_group) {
    DTRACE();

    SProjData pd;
    D3DXVECTOR3 lv;

    D3DXVec3TransformNormal(&lv, &light, &iobjma);

    obj.CalcShadowProjMatrix(noframe, pd, lv, 6.0f);

    ShadowProjBuildGeomInt(sp, pd, objma, mapradius, join_to_group);
    if (!sp.IsProjected())
        return;

    if (!render_texture)
        return;

    sp.DestroyTexture();

    CTextureManaged *tex;
    CMatrixShadowCliper *clipper = NULL;
    CMatrixShadowCliper clip(&sp);

    if (doclip) {
        if (sp.IsProjected()) {
            D3DXMATRIX m = g_MatrixMap->GetIdentityMatrix();
            m._41 = sp.GetDX();
            m._42 = sp.GetDY();

            *clip.WorldMatrix() = m * iobjma;
            clipper = &clip;
        }
    }

    CVectorObjectAnim *pobj = &obj;
    if (obj.VO()->GetFramesCnt() > 1) {
        CTexture *tex = CACHE_CREATE_TEXTURE();
        CVectorObject::CalcShadowTexture(1, &pobj, &noframe, NULL, pd, texsize, clipper, tex);
        tex->RefInc();
        sp.SetTexture(tex);
    }
    else {
        tex = CVectorObject::CalcShadowTexture(1, &pobj, &noframe, NULL, pd, texsize, clipper);
        tex->RefInc();
        sp.SetTexture(tex);
    }
}

void ShadowProjBuildTexture(CVOShadowProj &sp, CVectorObjectAnim &obj, int noframe, const D3DXMATRIX &iobjma,
                            D3DXVECTOR3 &light, int texsize, bool doclip) {
    DTRACE();

    SProjData pd;
    D3DXVECTOR3 lv;

    D3DXVec3TransformNormal(&lv, &light, &iobjma);

    obj.CalcShadowProjMatrix(noframe, pd, lv, 6.0f);

    if (obj.VO()->GetFramesCnt() <= 1) {
        sp.DestroyTexture();
    }

    CMatrixShadowCliper *clipper = NULL;
    CMatrixShadowCliper clip(&sp);

    if (doclip) {
        if (sp.IsProjected()) {
            D3DXMATRIX m = g_MatrixMap->GetIdentityMatrix();
            m._41 = sp.GetDX();
            m._42 = sp.GetDY();

            *clip.WorldMatrix() = m * iobjma;
            clipper = &clip;
        }
    }

    CVectorObjectAnim *pobj = &obj;
    if (obj.VO()->GetFramesCnt() > 1) {
        if (sp.GetTexture()) {
            // CTexture *tex = CACHE_CREATE_TEXTURE();
            CVectorObject::CalcShadowTexture(1, &pobj, &noframe, NULL, pd, texsize, clipper, sp.GetTexture());
            // sp.SetTexture(tex);
        }
        else {
            CTexture *tex = CACHE_CREATE_TEXTURE();
            CVectorObject::CalcShadowTexture(1, &pobj, &noframe, NULL, pd, texsize, clipper, tex);
            tex->RefInc();
            sp.SetTexture(tex);
        }
    }
    else {
        CTextureManaged *tex = CVectorObject::CalcShadowTexture(1, &pobj, &noframe, NULL, pd, texsize, clipper);
        tex->RefInc();
        sp.SetTexture(tex);
    }
}

void ShadowProjBuildTexture(CMatrixMapObject *mo, CVOShadowProj &sp, CVectorObjectAnim &obj, int noframe,
                            const D3DXMATRIX &iobjma, int texsize, bool doclip) {
    DTRACE();

    if (obj.VO()->GetFramesCnt() <= 1) {
        sp.DestroyTexture();
    }

    CMatrixShadowCliper *clipper = NULL;
    CMatrixShadowCliper clip(&sp);

    if (doclip) {
        if (sp.IsProjected()) {
            D3DXMATRIX m = g_MatrixMap->GetIdentityMatrix();
            m._41 = sp.GetDX();
            m._42 = sp.GetDY();

            *clip.WorldMatrix() = m * iobjma;
            clipper = &clip;
        }
    }

    mo->SetupMatricesForShadowTextureCalc();

    CVectorObjectAnim *pobj = &obj;
    if (obj.VO()->GetFramesCnt() > 1) {
        if (sp.GetTexture()) {
            // CTexture *tex = CACHE_CREATE_TEXTURE();
            CVectorObject::CalcShadowTextureWOMat(1, &pobj, &noframe, NULL, texsize, clipper, sp.GetTexture());
            // sp.SetTexture(tex);
        }
        else {
            CTexture *tex = CACHE_CREATE_TEXTURE();
            CVectorObject::CalcShadowTextureWOMat(1, &pobj, &noframe, NULL, texsize, clipper, tex);
            if (tex)
                tex->RefInc();
            sp.SetTexture(tex);
        }
    }
    else {
        CTextureManaged *tex = CVectorObject::CalcShadowTextureWOMat(1, &pobj, &noframe, NULL, texsize, clipper);
        if (tex)
            tex->RefInc();
        sp.SetTexture(tex);
    }
}

void ShadowProjBuildTextureList(CVOShadowProj &sp, int cnt, CVectorObjectAnim **obj, CTexture *tex, int *noframe,
                                D3DXMATRIX *wm, const D3DXMATRIX &objma, const D3DXMATRIX &iobjma, D3DXVECTOR3 &light,
                                int texsize) {
    DTRACE();

    SProjData pd;
    D3DXVECTOR3 lv;

    D3DXVec3TransformNormal(&lv, &light, &iobjma);
    //(*obj)->CalcShadowProjMatrix(cnt,obj,noframe,wm,pd,lv,6.0f);
    CVectorObject::CalcShadowProjMatrix(cnt, obj, noframe, wm, pd, lv, 6.0f);

    sp.DestroyTexture();

    if (!tex) {
        tex = CACHE_CREATE_TEXTURE();
    }
    CVectorObject::CalcShadowTexture(cnt, obj, noframe, wm, pd, texsize, NULL, tex);
    sp.SetTexture(tex);
}

void ShadowProjBuildFullList(CVOShadowProj &sp, int cnt, CVectorObjectAnim **obj, CTexture *tex, const int *noframe,
                             const D3DXMATRIX *wm, const D3DXMATRIX &objma, const D3DXMATRIX &iobjma,
                             D3DXVECTOR3 &light, int mapradius, int texsize, bool join_to_group) {
    DTRACE();

    SProjData pd;
    D3DXVECTOR3 lv;

    D3DXVec3TransformNormal(&lv, &light, &iobjma);
    //(*obj)->CalcShadowProjMatrix(cnt,obj,noframe,wm,pd,lv,6.0f);
    CVectorObject::CalcShadowProjMatrix(cnt, obj, noframe, wm, pd, lv, 6.0f);

    ShadowProjBuildGeomInt(sp, pd, objma, mapradius, join_to_group);
    if (!sp.IsProjected())
        return;

    sp.DestroyTexture();

    if (!tex) {
        tex = CACHE_CREATE_TEXTURE();
    }
    CVectorObject::CalcShadowTexture(cnt, obj, noframe, wm, pd, texsize, NULL, tex);
    sp.SetTexture(tex);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CMatrixShadowCliper::BeforeRender(void) {
    DTRACE();

    m_Shadowproj->DX_Prepare();
    CVOShadowProj::BeforeRenderAll();
}

void CMatrixShadowCliper::Render(void) {
    DTRACE();

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m_World));
    ASSERT_DX(g_D3DD->SetTexture(0, NULL));

    m_Shadowproj->RenderMin();
}
