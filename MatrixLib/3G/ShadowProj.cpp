// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "ShadowProj.hpp"

CBigVB<SVOShadowProjVertex> *CVOShadowProj::m_VB;
CBigIB *CVOShadowProj::m_IB;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CVOShadowProj::CVOShadowProj(CHeap *heap) : CMain(), m_Heap(heap) {
    DTRACE();

    // m_Strips = (SVOShadowProjStrip *)HAlloc(sizeof(SVOShadowProjStrip) * 16, heap);
    // m_StripsCnt = 0;
    // m_StripsMax = 16;

    m_VertCnt = 0;
    m_TriCnt = 0;

    m_Tex = NULL;
    if (m_VB == NULL) {
        m_VB = CBigVB<SVOShadowProjVertex>::NewBigVB(m_Heap);
        m_IB = CBigIB::NewBigIB(m_Heap);
    }

    m_preVB.verts = NULL;
    m_preIB.inds = NULL;

    m_DX = 0;
    m_DY = 0;
}

CVOShadowProj::~CVOShadowProj() {
    DTRACE();

    // HFree(m_Strips, m_Heap);

    if (m_preVB.verts) {
        if (m_IB->DelSource(&m_preIB)) {
            CBigIB::DestroyBigIB(m_IB);
            m_IB = NULL;
        }

        if (m_VB->DelSource(&m_preVB)) {
            CBigVB<SVOShadowProjVertex>::DestroyBigVB(m_VB);
            m_VB = NULL;
        }
        HFree(m_preVB.verts, m_Heap);
        // HFree(m_preIB.inds, m_Heap); // do not free this mem, because it is in verts
    }

    if (m_Tex) {
        m_Tex->RefDec();
        if (m_Tex->Ref() <= 0) {
            CCache::Destroy(m_Tex);
        }
    }
}

void CVOShadowProj::DestroyTexture(void) {
    DTRACE();

    if (m_Tex) {
        m_Tex->RefDec();
        if (m_Tex->Ref() <= 0) {
            CCache::Destroy(m_Tex);
        }
        m_Tex = NULL;
    }
}

void CVOShadowProj::Prepare(float dx, float dy, SVOShadowProjVertex *verts, int vertscnt, WORD *idxs, int idxscnt) {
    DTRACE();

    if (vertscnt <= 0) {
        m_Tex = NULL;
        m_VertCnt = 0;
        m_TriCnt = 0;
        m_preVB.verts = 0;
        m_preIB.inds = 0;

        return;
    }
    m_DX = dx;
    m_DY = dy;

    m_VertCnt = vertscnt;
    m_TriCnt = idxscnt - 2;

    m_preVB.size = vertscnt * sizeof(SVOShadowProjVertex);
    m_preIB.size = idxscnt * sizeof(WORD);

    if (m_preVB.verts)
        HFree(m_preVB.verts, m_Heap);

    m_preVB.verts = (SVOShadowProjVertex *)HAlloc(m_preVB.size + m_preIB.size, m_Heap);
    memcpy(m_preVB.verts, verts, m_preVB.size);
    m_VB->AddSource(&m_preVB);

    m_preIB.inds = (WORD *)(((BYTE *)m_preVB.verts) + m_preVB.size);
    memcpy(m_preIB.inds, idxs, m_preIB.size);
    m_IB->AddSource(&m_preIB);
}

void CVOShadowProj::RenderMin(void) {
    DTRACE();

    int vbase = m_preVB.Select(m_VB);
    if (vbase < 0)
        return;
    int ibase = m_preIB.Select(m_IB);
    if (ibase < 0)
        return;

    ASSERT_DX(g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, vbase, 0, m_VertCnt, ibase, m_TriCnt));

    // for (int i = 0; i<m_StripsCnt; ++i)
    //{
    //    ASSERT_DX(g_D3DD->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, vbase, m_Strips[i].minidx, m_Strips[i].vcnt,
    //    m_Strips[i].index + ibase, m_Strips[i].tricnt));
    //}
}
