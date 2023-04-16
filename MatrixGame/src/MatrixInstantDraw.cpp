// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixInstantDraw.hpp"
#include "D3DControl.hpp"

SFVF_VB CInstDraw::m_FVFs[IDFVF_CNT];
E_FVF CInstDraw::m_Current;
D3D_IB CInstDraw::m_IB;
int CInstDraw::m_IB_Count;

void CInstDraw::MarkAllBuffersNoNeed(void) {
    for (int i = 0; i < IDFVF_CNT; ++i) {
        if (IS_VB(m_FVFs[i].vb)) {
            DESTROY_VB(m_FVFs[i].vb);
            m_FVFs[i].cursize = 0;
        }
    }

    if (IS_IB(m_IB))
        DESTROY_IB(m_IB);
    m_IB_Count = 0;
}

void CInstDraw::ClearAll(void) {
    for (int i = 0; i < IDFVF_CNT; ++i) {
        if (IS_VB(m_FVFs[i].vb)) {
            DESTROY_VB(m_FVFs[i].vb);
        }
        if (m_FVFs[i].sets) {
            for (int s = 0; s < m_FVFs[i].sets_alloc; ++s) {
                HFree(m_FVFs[i].sets[s].accum, g_MatrixHeap);
            }
            HFree(m_FVFs[i].sets, g_MatrixHeap);
        }
    }
#ifdef _DEBUG
    m_Current = IDFVF_CNT;
#endif
    memset(m_FVFs, 0, sizeof(m_FVFs));

    if (IS_IB(m_IB))
        DESTROY_IB(m_IB);
    m_IB_Count = 0;
}

void CInstDraw::BeginDraw(E_FVF fvf) {
#ifdef _DEBUG
    if (m_Current != IDFVF_CNT)
        ERROR_S(L"Error #1 in instant draw");
#endif

    m_Current = fvf;
}

void CInstDraw::AddVerts(void *v, CBaseTexture *tex) {
#ifdef _DEBUG
    if (m_Current == IDFVF_CNT)
        ERROR_S(L"Error #2 in instant draw");
#endif

    SFVF_VB *cvb = m_FVFs + m_Current;

    int i = 0;
    for (; i < cvb->sets_cnt; ++i) {
        if (cvb->sets[i].tex == tex)
            break;
    }

    if (i >= cvb->sets_cnt) {
        if (cvb->sets_cnt >= cvb->sets_alloc) {
            ++cvb->sets_alloc;
            cvb->sets = (SOneSet *)HAllocEx(m_FVFs[m_Current].sets, sizeof(SOneSet) * (cvb->sets_alloc), g_MatrixHeap);
            cvb->sets[i].accum = NULL;
            cvb->sets[i].accumcntalloc = 0;
        }
        cvb->sets[i].accumcnt = 0;
        cvb->sets[i].tex = tex;
        cvb->sets[i].tf_used = 0;

        ++cvb->sets_cnt;
    }

    SOneSet *os = cvb->sets + i;

    os->accumcnt += 4;
    if (os->accumcnt > os->accumcntalloc) {
        os->accum = HAllocEx(os->accum, cvb->stride * os->accumcnt, g_MatrixHeap);
        os->accumcntalloc = os->accumcnt;
    }
    memcpy((BYTE *)os->accum + (cvb->stride * (os->accumcnt - 4)), v, cvb->stride * 4);

    cvb->statistic += 4;
}

void CInstDraw::AddVerts(void *v, CBaseTexture *tex, DWORD tf) {
#ifdef _DEBUG
    if (m_Current == IDFVF_CNT) {
        ERROR_S(L"Error #3 in instant draw");
    }
#endif

    SFVF_VB *cvb = m_FVFs + m_Current;

    int i = 0;
    for (; i < cvb->sets_cnt; ++i) {
        if (cvb->sets[i].tex == tex) {
            if (cvb->sets[i].tf_used) {
                if (cvb->sets[i].tf == tf)
                    break;
            }
        }
    }

    if (i >= cvb->sets_cnt) {
        if (cvb->sets_cnt >= cvb->sets_alloc) {
            ++cvb->sets_alloc;
            cvb->sets = (SOneSet *)HAllocEx(m_FVFs[m_Current].sets, sizeof(SOneSet) * (cvb->sets_alloc), g_MatrixHeap);
            cvb->sets[i].accum = NULL;
            cvb->sets[i].accumcntalloc = 0;
        }
        cvb->sets[i].accumcnt = 0;
        cvb->sets[i].tex = tex;
        cvb->sets[i].tf_used = 1;
        cvb->sets[i].tf = tf;

        ++cvb->sets_cnt;
    }

    SOneSet *os = cvb->sets + i;

    os->accumcnt += 4;
    if (os->accumcnt > os->accumcntalloc) {
        os->accum = HAllocEx(os->accum, cvb->stride * os->accumcnt, g_MatrixHeap);
        os->accumcntalloc = os->accumcnt;
    }
    memcpy((BYTE *)os->accum + (cvb->stride * (os->accumcnt - 4)), v, cvb->stride * 4);

    cvb->statistic += 4;
}

void CInstDraw::DrawFrameBegin(void) {
    for (int i = 0; i < IDFVF_CNT; ++i) {
        m_FVFs[i].disp = 0;
    }
}

void CInstDraw::ActualDraw(void) {
#ifdef _DEBUG
    if (m_Current == IDFVF_CNT)
        ERROR_S(L"Error #0 in instant draw");
#endif
    int disp = 0;

    SFVF_VB *cvb = m_FVFs + m_Current;

    if (cvb->statistic == 0)
        goto final;

    if (!IS_VB(cvb->vb) || (cvb->statistic + cvb->disp) > cvb->cursize) {
        cvb->cursize = (cvb->statistic + cvb->disp);

        if (IS_VB(cvb->vb))
            DESTROY_VB(cvb->vb);
        CREATE_VB_DYNAMIC(cvb->stride * (cvb->statistic + cvb->disp), cvb->fvf, cvb->vb);
        if (!IS_VB(cvb->vb)) {
            cvb->cursize = 0;
            goto final;
        }
    }

    byte *verts;
    if (cvb->disp == 0) {
        LOCKP_VB_DYNAMIC(cvb->vb, 0, cvb->stride * cvb->statistic, &verts);
    }
    else {
        LOCKP_VB_NO(cvb->vb, cvb->stride * cvb->disp, cvb->stride * cvb->statistic, &verts);
    }

    for (int i = 0; i < cvb->sets_cnt; ++i) {
        if (cvb->sets[i].accumcnt > cvb->statistic_max_tex) {
            cvb->statistic_max_tex = cvb->sets[i].accumcnt;
        }

        int sz = cvb->sets[i].accumcnt * cvb->stride;
        memcpy(verts, cvb->sets[i].accum, sz);
        verts += sz;
    }

    UNLOCK_VB(cvb->vb);

    if (!IS_IB(m_IB) || cvb->statistic_max_tex > m_IB_Count) {
        m_IB_Count = cvb->statistic_max_tex;

        if (IS_IB(m_IB))
            DESTROY_VB(m_IB);
        CREATE_IBD16(cvb->statistic_max_tex * sizeof(WORD) * 6 / 4, m_IB);
        if (!IS_IB(m_IB)) {
            m_IB_Count = 0;
            goto final;
        }

        WORD *p;
        LOCKD_IB(m_IB, &p);

        int cc = cvb->statistic_max_tex / 4;
        for (int i = 0; i < cc; ++i) {
            DWORD vol = i * 4;
            *(DWORD *)p = vol | ((vol + 1) << 16);
            *(DWORD *)(p + 2) = (vol + 2) | ((vol + 1) << 16);
            *(DWORD *)(p + 4) = (vol + 3) | ((vol + 2) << 16);

            p += 6;
        }

        UNLOCK_IB(m_IB);
    }

    // draw!

    g_D3DD->SetStreamSource(0, GET_VB(cvb->vb), 0, cvb->stride);
    ASSERT_DX(g_D3DD->SetIndices(GET_IB(m_IB)));
    g_D3DD->SetFVF(cvb->fvf);

    for (int i = 0; i < cvb->sets_cnt; ++i) {
        CBaseTexture *tex = cvb->sets[i].tex;

        if (cvb->sets[i].tf_used) {
            g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, cvb->sets[i].tf);
        }

        if (tex != NULL) {
            if (tex->IsTextureManaged()) {
                ASSERT_DX(g_D3DD->SetTexture(0, ((CTextureManaged *)tex)->Tex()));
            }
            else {
                ASSERT_DX(g_D3DD->SetTexture(0, ((CTexture *)tex)->Tex()));
            }
        }

        g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, cvb->disp + disp, 0, cvb->sets[i].accumcnt, 0,
                                     cvb->sets[i].accumcnt / 2);

        disp += cvb->sets[i].accumcnt;
    }

    cvb->disp += disp;

final:
    cvb->statistic = 0;
    cvb->statistic_max_tex = 0;
    cvb->sets_cnt = 0;

#ifdef _DEBUG
    m_Current = IDFVF_CNT;
#endif
}
