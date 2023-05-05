// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "ShadowStencil.hpp"
#include "VectorObject.hpp"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static const DWORD SVOShadowStencilVertex_FVF = D3DFVF_XYZ;

CVOShadowStencil *CVOShadowStencil::m_First;
CVOShadowStencil *CVOShadowStencil::m_Last;

void CVOShadowStencil::BeforeRenderAll()
{
    ASSERT_DX(g_D3DD->SetFVF(SVOShadowStencilVertex_FVF));
}

void CVOShadowStencil::MarkAllBuffersNoNeed()
{
    CVOShadowStencil *s = m_First;
    for (; s; s = s->m_Next) {
        s->m_DirtyDX = true;
        if (s->m_VB)
            DESTROY_VB(s->m_VB);
        if (s->m_IB)
            DESTROY_VB(s->m_IB);
    }
}

CVOShadowStencil::CVOShadowStencil()
{
    DTRACE();

    m_VB = NULL;
    m_VBSize = 0;

    m_IB = NULL;
    m_IBSize = 0;

    m_DirtyDX = 1;

    m_vo = NULL;

    m_FrameFor = -1;
    m_FramesCnt = 0;

    LIST_ADD(this, m_First, m_Last, m_Prev, m_Next);
}

CVOShadowStencil::~CVOShadowStencil() {
    DTRACE();

    DX_Free();

    LIST_DEL(this, m_First, m_Last, m_Prev, m_Next);
}

void CVOShadowStencil::Build(CVectorObject &obj, int frame, const D3DXVECTOR3 &vLight, float len, bool invert) {
    DTRACE();

    if (obj.m_Geometry.m_EdgesCnt == 0)
        return;

    bool dirty = false;

    if (m_vo != &obj) {
        // prepare frames

        // all the info inside the frames will be rewritten now, so we don't bother destoying are recreating objects
        m_FramesCnt = obj.GetFramesCnt();
        m_Frames.resize(m_FramesCnt);
        for (int i = 0; i < m_FramesCnt; ++i)
        {
            m_Frames[i].m_preVerts.reserve(256);
            m_Frames[i].m_preVerts.clear();

            m_Frames[i].m_preInds.reserve(256);
            m_Frames[i].m_preInds.clear();

            m_Frames[i].m_len = 0;
            m_Frames[i].m_light.all = 0;
        }

        m_vo = &obj;
        dirty = true;
    }

    SVONormal vlight;
    {
        D3DXVECTOR3 vlightf;
        D3DXVec3Normalize(&vlightf, &vLight);

        vlight.s = BYTE(((*(DWORD *)&vlightf.x) >> 31) | (((*(DWORD *)&vlightf.y) >> 30) & 2) |
                        (((*(DWORD *)&vlightf.z) >> 29) & 4));
        MAKE_ABS_FLOAT(vlightf.x);
        MAKE_ABS_FLOAT(vlightf.y);
        MAKE_ABS_FLOAT(vlightf.z);

        // TODO : 255 * ...

        vlight.x = Float2Int(vlightf.x * 255.0f);
        vlight.y = Float2Int(vlightf.y * 255.0f);
        vlight.z = Float2Int(vlightf.z * 255.0f);
    }

    SSSFrameData *fd = &m_Frames[frame];
    SVOKadr *k = obj.m_Geometry.m_Frames + frame;

    len += k->m_Radius * 2 + k->m_GeoCenter.z;

    if (dirty || m_FrameFor != frame) {
        // frame not match
        m_DirtyDX = true;

        m_FrameFor = frame;

        if (!dirty && vlight.all == fd->m_light.all && fabs(len - fd->m_len) < 1.0)
            return;
    }
    else {
        // frame match
        if (vlight.all == fd->m_light.all && fabs(len - fd->m_len) < 1.0)
            return;
    }
    fd->m_len = len;
    fd->m_light.all = vlight.all;
    m_DirtyDX = true;

#define MUL_X(v) (vlight.x * v)
#define MUL_Y(v) (vlight.y * v)
#define MUL_Z(v) (vlight.z * v)

    D3DXVECTOR3 lenv(vLight * len);

    fd->m_preVerts.reserve(k->m_EdgeCnt * 4);
    fd->m_preVerts.clear();

    fd->m_preInds.reserve(k->m_EdgeCnt * 4);
    fd->m_preInds.clear();

    // TAKT_BEGIN();

    SVOKadrEdge *keb = obj.m_Geometry.m_Edges + k->m_EdgeStart;
    SVOKadrEdge *kee = keb + k->m_EdgeCnt;

    SVOFrameRuntime *frr = obj.m_Geometry.m_FramesRuntime + frame;

    int sz = (frr->m_EdgeVertexIndexCount) * sizeof(int);
    int *verts = (int *)_alloca(sz);
    memset(verts, -1, sz);
    int verts_c = 0;

    // struct SSPoly
    //{
    //    DWORD vert;

    //    int   Get0(void) const {return vert & 32767;}
    //    int   Get1(void) const {return (vert>>15) & 32767;}
    //    bool  Flag(void) const {return (vert&0x80000000) != 0;}

    //    void  Init(int v0, int v1, bool flag)
    //    {
    //        vert = v0 | (v1<<15) | (flag?0x80000000:0);
    //    }

    //} * polys = (SSPoly *)_alloca(4096 * sizeof(SSPoly));
    // int polys_c = 0;

    for (; keb < kee; ++keb) {
        int nx1, ny1, nz1;
        int nx2, ny2, nz2;

        BYTE sign0 = keb->n0.s ^ vlight.s;
        BYTE sign1 = keb->n1.s ^ vlight.s;

        nx1 = (MUL_X(keb->n0.x) ^ ((sign0 & 1) ? 0xFFFFFFFF : 0)) + (sign0 & 1);
        nx2 = (MUL_X(keb->n1.x) ^ ((sign1 & 1) ? 0xFFFFFFFF : 0)) + (sign1 & 1);

        sign0 >>= 1;
        sign1 >>= 1;

        ny1 = (MUL_Y(keb->n0.y) ^ ((sign0 & 1) ? 0xFFFFFFFF : 0)) + (sign0 & 1);
        ny2 = (MUL_Y(keb->n1.y) ^ ((sign1 & 1) ? 0xFFFFFFFF : 0)) + (sign1 & 1);

        sign0 >>= 1;
        sign1 >>= 1;

        nz1 = (MUL_Z(keb->n0.z) ^ ((sign0 & 1) ? 0xFFFFFFFF : 0)) + (sign0 & 1);
        nz2 = (MUL_Z(keb->n1.z) ^ ((sign1 & 1) ? 0xFFFFFFFF : 0)) + (sign1 & 1);

        DWORD temp0 = (DWORD)(nx1 + ny1 + nz1);
        DWORD temp1 = (DWORD)(nx2 + ny2 + nz2);
        if (((temp0 ^ temp1) & (0x80000000)) == 0)
            continue;

        // int temp0 = (nx1 + ny1 + nz1);
        // int temp1 = (nx2 + ny2 + nz2);
        // if ((temp0 * temp1) < 0) continue;
        // if (((temp0 ^ temp1) & (0x80000000))==0) continue;
        // if (temp0 >= 1 && temp1 >= 1) continue;
        // if (temp0 < 1 && temp1 < 1) continue;

        int vi0 = keb->v00 / sizeof(SVOVertex) - frr->m_EdgeVertexIndexMin,
            vi1 = keb->v01 / sizeof(SVOVertex) - frr->m_EdgeVertexIndexMin;
#ifdef _DEBUG
        if (vi0 >= frr->m_EdgeVertexIndexCount)
            debugbreak();
        if (vi1 >= frr->m_EdgeVertexIndexCount)
            debugbreak();
#endif
        if (verts[vi0] < 0) {
            verts[vi0] = verts_c;
            vi0 = verts_c++;

            const D3DXVECTOR3 *vv = &(((SVOVertex *)(((BYTE *)obj.m_Geometry.m_Vertices.verts) + keb->v00))->v);
            D3DXVECTOR3 vv_(*vv + lenv);

            fd->m_preVerts.emplace_back(*vv);
            fd->m_preVerts.emplace_back(vv_);
        }
        else {
            vi0 = verts[vi0];
        }
        if (verts[vi1] < 0) {
            verts[vi1] = verts_c;
            vi1 = verts_c++;

            const D3DXVECTOR3 *vv = &(((SVOVertex *)(((BYTE *)obj.m_Geometry.m_Vertices.verts) + keb->v01))->v);
            D3DXVECTOR3 vv_(*vv + lenv);

            fd->m_preVerts.emplace_back(*vv);
            fd->m_preVerts.emplace_back(vv_);
        }
        else {
            vi1 = verts[vi1];
        }

        DWORD p0, p1, p2;

        if (((temp0 & 0x80000000) == 0) ^ invert)
        {
            // 0: vi1 * 2
            // 1: vi0 * 2
            // 2: vi1 * 2 + 1
            // 3: vi0 * 2 + 1

            p0 = (vi1 * 2) | ((vi0 * 2) << 16);
            p1 = (vi1 * 2 + 1) | ((vi0 * 2) << 16);
            p2 = (vi0 * 2 + 1) | ((vi1 * 2 + 1) << 16);
        }
        else
        {
            // 0: vi1 * 2 + 1
            // 1: vi0 * 2 + 1
            // 2: vi1 * 2
            // 3: vi0 * 2

            p0 = (vi1 * 2 + 1) | ((vi0 * 2 + 1) << 16);
            p1 = (vi1 * 2) | ((vi0 * 2 + 1) << 16);
            p2 = (vi0 * 2) | ((vi1 * 2) << 16);
        }

        fd->m_preInds.emplace_back(0xFFFF & p0);
        fd->m_preInds.emplace_back(0xFFFF & p0 >> 16);
        fd->m_preInds.emplace_back(0xFFFF & p1);
        fd->m_preInds.emplace_back(0xFFFF & p1 >> 16);
        fd->m_preInds.emplace_back(0xFFFF & p2);
        fd->m_preInds.emplace_back(0xFFFF & p2 >> 16);
    }
}

void CVOShadowStencil::DX_Prepare(void) {
    if (!m_DirtyDX)
        return;
    if (m_FrameFor < 0)
        return;

    SSSFrameData *fd = &m_Frames[m_FrameFor];

    if (!IS_VB(m_VB) || fd->m_preVerts.size() > m_VBSize) {
        if (IS_VB(m_VB)) {
            DESTROY_VB(m_VB);
        }
        CREATE_VB_DYNAMIC(fd->m_preVerts.size() * sizeof(SVOShadowStencilVertex), SVOShadowStencilVertex_FVF, m_VB);
        if (m_VB == NULL)
            return;

        m_VBSize = fd->m_preVerts.size();
    }

    if (!IS_IB(m_IB) || fd->m_preInds.size() > m_IBSize)
    {
        if (IS_VB(m_IB))
        {
            DESTROY_VB(m_IB);
        }
        CREATE_IBD16(fd->m_preInds.size() * sizeof(WORD), m_IB);
        if (m_IB == NULL)
            return;

        m_IBSize = fd->m_preInds.size();
    }

    {
        SVOShadowStencilVertex *p;

        LOCKP_VB_DYNAMIC(m_VB, 0, fd->m_preVerts.size() * sizeof(SVOShadowStencilVertex), &p);
        memcpy(p, fd->m_preVerts.data(), fd->m_preVerts.size() * sizeof(SVOShadowStencilVertex));
        UNLOCK_VB(m_VB);
    }

    {
        WORD *p;

        LOCKPD_IB(m_IB, 0, fd->m_preInds.size() * sizeof(WORD), &p);
        memcpy(p, fd->m_preInds.data(), fd->m_preInds.size() * sizeof(WORD));
        UNLOCK_IB(m_IB);
    }

    m_DirtyDX = false;
}

void CVOShadowStencil::Render(const D3DXMATRIX &objma)
{
    DTRACE();

    if (m_DirtyDX)
        DX_Prepare();

    if (!IS_VB(m_VB))
        return;
    if (!IS_IB(m_IB))
        return;

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &objma));

    g_D3DD->SetStreamSource(0, GET_VB(m_VB), 0, sizeof(SVOShadowStencilVertex));
    g_D3DD->SetIndices(GET_IB(m_IB));

    SSSFrameData& fd = m_Frames[m_FrameFor];

    int vcnt = fd.m_preVerts.size();
    int tcnt = fd.m_preInds.size() / 3;
    g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vcnt, 0, tcnt);
}
