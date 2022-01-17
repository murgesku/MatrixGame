// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"

#include "ShadowStencil.hpp"
#include "VectorObject.hpp"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CVOShadowStencil *CVOShadowStencil::m_First;
CVOShadowStencil *CVOShadowStencil::m_Last;

void CVOShadowStencil::MarkAllBuffersNoNeed(void) {
    CVOShadowStencil *s = m_First;
    for (; s; s = s->m_Next) {
        s->m_DirtyDX = true;
        if (s->m_VB)
            DESTROY_VB(s->m_VB);
        if (s->m_IB)
            DESTROY_VB(s->m_IB);
    }
}

CVOShadowStencil::CVOShadowStencil(CHeap *heap) : CMain(), m_Heap(heap) {
    DTRACE();

    m_VB = NULL;
    m_VBSize = 0;

    m_IB = NULL;
    m_IBSize = 0;

    m_DirtyDX = 1;

    m_vo = NULL;

    m_FrameFor = -1;
    m_Frames = NULL;
    m_FramesCnt = 0;

    LIST_ADD(this, m_First, m_Last, m_Prev, m_Next);
}

CVOShadowStencil::~CVOShadowStencil() {
    DTRACE();

    for (int i = 0; i < m_FramesCnt; ++i) {
        if (m_Frames[i].m_preVerts)
            HFree(m_Frames[i].m_preVerts, m_Heap);
        if (m_Frames[i].m_preInds)
            HFree(m_Frames[i].m_preInds, m_Heap);
    }
    if (m_Frames)
        HFree(m_Frames, m_Heap);
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
        for (int i = 0; i < m_FramesCnt; ++i) {
            if (m_Frames[i].m_preVerts)
                HFree(m_Frames[i].m_preVerts, m_Heap);
        }
        if (m_Frames)
            HFree(m_Frames, m_Heap);

        m_Frames = (SSSFrameData *)HAllocEx(m_Frames, sizeof(SSSFrameData) * obj.GetFramesCnt(), m_Heap);
        m_FramesCnt = obj.GetFramesCnt();
        for (int i = 0; i < m_FramesCnt; ++i) {
            m_Frames[i].m_preVertsAllocated = sizeof(SVOShadowStencilVertex) * 256;
            m_Frames[i].m_preVerts = (SVOShadowStencilVertex *)HAlloc(m_Frames[i].m_preVertsAllocated, m_Heap);
            m_Frames[i].m_preVertsSize = 0;

            m_Frames[i].m_preIndsAllocated = sizeof(WORD) * 256;
            m_Frames[i].m_preInds = (WORD *)HAlloc(m_Frames[i].m_preIndsAllocated, m_Heap);
            m_Frames[i].m_preIndsSize = 0;

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

    SSSFrameData *fd = m_Frames + frame;
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

    int a = sizeof(SVOShadowStencilVertex) * k->m_EdgeCnt * 4;

    if (a > fd->m_preVertsAllocated) {
        fd->m_preVertsAllocated = a;
        fd->m_preVerts = (SVOShadowStencilVertex *)HAllocEx(fd->m_preVerts, a, m_Heap);
    }

    fd->m_preVertsSize = 0;

    a = sizeof(WORD) * k->m_EdgeCnt * 4;

    if (a > fd->m_preIndsAllocated) {
        fd->m_preIndsAllocated = a;
        fd->m_preInds = (WORD *)HAllocEx(fd->m_preInds, a, m_Heap);
    }

    fd->m_preIndsSize = 0;

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
            _asm int 3
        if (vi1 >= frr->m_EdgeVertexIndexCount)
            _asm int 3
#endif
                    if (verts[vi0] < 0) {
                verts[vi0] = verts_c;
                vi0 = verts_c++;

                const D3DXVECTOR3 *vv = &(((SVOVertex *)(((BYTE *)obj.m_Geometry.m_Vertices.verts) + keb->v00))->v);
                D3DXVECTOR3 vv_(*vv + lenv);

                ((SVOShadowStencilVertex *)(((BYTE *)fd->m_preVerts) + fd->m_preVertsSize))->v = *vv;
                fd->m_preVertsSize += sizeof(SVOShadowStencilVertex);

                ((SVOShadowStencilVertex *)(((BYTE *)fd->m_preVerts) + fd->m_preVertsSize))->v = vv_;
                fd->m_preVertsSize += sizeof(SVOShadowStencilVertex);
            }
        else {
            vi0 = verts[vi0];
        }
        if (verts[vi1] < 0) {
            verts[vi1] = verts_c;
            vi1 = verts_c++;

            const D3DXVECTOR3 *vv = &(((SVOVertex *)(((BYTE *)obj.m_Geometry.m_Vertices.verts) + keb->v01))->v);
            D3DXVECTOR3 vv_(*vv + lenv);

            ((SVOShadowStencilVertex *)(((BYTE *)fd->m_preVerts) + fd->m_preVertsSize))->v = *vv;
            fd->m_preVertsSize += sizeof(SVOShadowStencilVertex);

            ((SVOShadowStencilVertex *)(((BYTE *)fd->m_preVerts) + fd->m_preVertsSize))->v = vv_;
            fd->m_preVertsSize += sizeof(SVOShadowStencilVertex);
        }
        else {
            vi1 = verts[vi1];
        }

        if (((temp0 & 0x80000000) == 0) ^ invert) {
            // 0: vi1 * 2
            // 1: vi0 * 2
            // 2: vi1 * 2 + 1
            // 3: vi0 * 2 + 1

            DWORD *p = (DWORD *)(((BYTE *)fd->m_preInds) + fd->m_preIndsSize);

            *(DWORD *)(p + 0) = (vi1 * 2) | ((vi0 * 2) << 16);
            *(DWORD *)(p + 1) = (vi1 * 2 + 1) | ((vi0 * 2) << 16);
            *(DWORD *)(p + 2) = (vi0 * 2 + 1) | ((vi1 * 2 + 1) << 16);
        }
        else {
            // 0: vi1 * 2 + 1
            // 1: vi0 * 2 + 1
            // 2: vi1 * 2
            // 3: vi0 * 2

            DWORD *p = (DWORD *)(((BYTE *)fd->m_preInds) + fd->m_preIndsSize);

            *(DWORD *)(p + 0) = (vi1 * 2 + 1) | ((vi0 * 2 + 1) << 16);
            *(DWORD *)(p + 1) = (vi1 * 2) | ((vi0 * 2 + 1) << 16);
            *(DWORD *)(p + 2) = (vi0 * 2) | ((vi1 * 2) << 16);
        }
        fd->m_preIndsSize += sizeof(WORD) * 6;

        //
        //
        //
        //
        //
        //
        //        const D3DXVECTOR3 * v1 = &(((SVOVertex *)(((BYTE *)obj.m_Geometry.m_Vertices.verts) + keb->v00))->v);
        //        const D3DXVECTOR3 * v2 = &(((SVOVertex *)(((BYTE *)obj.m_Geometry.m_Vertices.verts) + keb->v01))->v);
        //
        //
        //        D3DXVECTOR3 v3(*v1 + lenv);
        //        D3DXVECTOR3 v4(*v2 + lenv);
        //
        //#define VBUF_LOCAL fd->m_preVerts
        //
        //        if(((temp0 & 0x80000000) == 0) ^ invert)
        //        {
        //
        //			((SVOShadowStencilVertex *)(((BYTE *)VBUF_LOCAL) + fd->m_preVertsSize))->v = *v2;
        //            fd->m_preVertsSize += sizeof(SVOShadowStencilVertex);
        //		    ((SVOShadowStencilVertex *)(((BYTE *)VBUF_LOCAL) + fd->m_preVertsSize))->v = *v1;
        //            fd->m_preVertsSize += sizeof(SVOShadowStencilVertex);
        //			((SVOShadowStencilVertex *)(((BYTE *)VBUF_LOCAL) + fd->m_preVertsSize))->v = v4;
        //            fd->m_preVertsSize += sizeof(SVOShadowStencilVertex);
        //			((SVOShadowStencilVertex *)(((BYTE *)VBUF_LOCAL) + fd->m_preVertsSize))->v = v3;
        //
        //		} else
        //        {
        //			((SVOShadowStencilVertex *)(((BYTE *)VBUF_LOCAL) + fd->m_preVertsSize))->v = v4;
        //            fd->m_preVertsSize += sizeof(SVOShadowStencilVertex);
        //		    ((SVOShadowStencilVertex *)(((BYTE *)VBUF_LOCAL) + fd->m_preVertsSize))->v = v3;
        //            fd->m_preVertsSize += sizeof(SVOShadowStencilVertex);
        //			((SVOShadowStencilVertex *)(((BYTE *)VBUF_LOCAL) + fd->m_preVertsSize))->v = *v2;
        //            fd->m_preVertsSize += sizeof(SVOShadowStencilVertex);
        //			((SVOShadowStencilVertex *)(((BYTE *)VBUF_LOCAL) + fd->m_preVertsSize))->v = *v1;
        //		}
        //        fd->m_preVertsSize += sizeof(SVOShadowStencilVertex);
        //#undef VBUF_LOCAL
        //
        // if (fd->m_preVertsSize == sizeof(SVOShadowStencilVertex) * 4 * 2) break;
    }
}

void CVOShadowStencil::DX_Prepare(void) {
    if (!m_DirtyDX)
        return;
    if (m_FrameFor < 0)
        return;

    SSSFrameData *fd = m_Frames + m_FrameFor;

    if (!IS_VB(m_VB) || fd->m_preVertsSize > m_VBSize) {
        if (IS_VB(m_VB)) {
            DESTROY_VB(m_VB);
        }
        CREATE_VB_DYNAMIC(fd->m_preVertsSize, SVOShadowStencilVertex::FVF, m_VB);
        if (m_VB == NULL)
            return;

        m_VBSize = fd->m_preVertsSize;
    }

    if (!IS_IB(m_IB) || fd->m_preIndsSize > m_IBSize) {
        if (IS_VB(m_IB)) {
            DESTROY_VB(m_IB);
        }
        CREATE_IBD16(fd->m_preIndsSize, m_IB);
        if (m_IB == NULL)
            return;

        m_IBSize = fd->m_preIndsSize;
    }

    {
        SVOShadowStencilVertex *p;

        LOCKP_VB_DYNAMIC(m_VB, 0, fd->m_preVertsSize, &p);
        memcpy(p, fd->m_preVerts, fd->m_preVertsSize);
        UNLOCK_VB(m_VB);
    }

    {
        WORD *p;

        LOCKPD_IB(m_IB, 0, fd->m_preIndsSize, &p);
        memcpy(p, fd->m_preInds, fd->m_preIndsSize);
        UNLOCK_IB(m_IB);
    }

    m_DirtyDX = false;
}

// void   CVOShadowStencil::DX_Free(void)
//{
//    if (m_tempVB) DESTROY_VB(m_tempVB);
//
//}

void CVOShadowStencil::Render(const D3DXMATRIX &objma) {
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

    SSSFrameData *fd = m_Frames + m_FrameFor;

    // const uint n = fd->m_preVertsSize / (sizeof(SVOShadowStencilVertex));
    // for (uint i=0; i<n; i+=4)
    //    ASSERT_DX(g_D3DD->DrawPrimitive( D3DPT_TRIANGLESTRIP,i, 2));

    int vcnt = fd->m_preVertsSize / sizeof(SVOShadowStencilVertex);
    int tcnt = fd->m_preIndsSize / (sizeof(WORD) * 3);
    g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vcnt, 0, tcnt);
}
