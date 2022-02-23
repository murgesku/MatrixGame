// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef SHADOW_PROJ_INCLUDE
#define SHADOW_PROJ_INCLUDE

#include "Cache.hpp"
#include "D3DControl.hpp"
#include "BigVB.hpp"
#include "BigIB.hpp"

struct SVOShadowProjVertex {
    D3DXVECTOR3 v;
    float tu, tv;

    static const DWORD FVF = D3DFVF_XYZ | D3DFVF_TEX1;
};

class CVOShadowCliper : public CMain {
public:
    CVOShadowCliper(){};
    ~CVOShadowCliper(void){};

    virtual void BeforeRender(void) = 0;
    virtual void Render(void) = 0;
};

class CVOShadowProj : public CMain {
    CBaseTexture *m_Tex;  // it can be CTexture or CTextureManaged

protected:
    static CBigVB<SVOShadowProjVertex> *m_VB;
    static CBigIB *m_IB;
    CHeap *m_Heap;
    SBigVBSource<SVOShadowProjVertex> m_preVB;
    SBigIBSource m_preIB;
    float m_DX, m_DY;
    int m_TriCnt;
    int m_VertCnt;

public:
    static void StaticInit(void) {
        m_IB = NULL;
        m_VB = NULL;
    }
    static void MarkAllBuffersNoNeed(void) {
        if (m_VB)
            m_VB->ReleaseBuffers();
        if (m_IB)
            m_IB->ReleaseBuffers();
    }

    CVOShadowProj(CHeap *heap);
    ~CVOShadowProj(void);

    static void BeforeRenderAll(void) {
        if (m_IB)
            m_IB->BeforeDraw();
        if (m_VB)
            m_VB->BeforeDraw();
        ASSERT_DX(g_D3DD->SetFVF(SVOShadowProjVertex::FVF));
    }

    CBaseTexture *GetTexture(void) { return m_Tex; }
    void SetTexture(CBaseTexture *tex) { m_Tex = tex; }
    bool IsProjected(void) const { return m_preVB.verts != NULL; }
    void DX_Prepare(void) {
        m_preVB.Prepare(m_VB);
        m_preIB.Prepare(m_IB);
    };
    void DX_Free(void) {
        m_preVB.MarkNoNeed(m_VB);
        m_preIB.MarkNoNeed(m_IB);
    }

    float GetDX(void) const { return m_DX; }
    float GetDY(void) const { return m_DY; }

    void DestroyTexture(void);

    void Prepare(float dx, float dy, SVOShadowProjVertex *verts, int vertscnt, WORD *idxs, int idxscnt);
    void BeforeRender(void) {
        if (m_Tex) {
            if (m_Tex->IsTextureManaged()) {
                ((CTextureManaged *)m_Tex)->Preload();
            }
            else {
                ((CTexture *)m_Tex)->Preload();
            }
        }
        DX_Prepare();
    }
    void RenderMin(void);
    void Render(void) {
        DTRACE();
#ifdef _DEBUG
        if (!m_Tex)
            ERROR_S(L"Oops, texture is NULL in shadow rendering.");
#else
        if (!m_Tex)
            return;  // just return
#endif

        if (m_Tex->IsTextureManaged()) {
            g_D3DD->SetTexture(0, ((CTextureManaged *)m_Tex)->Tex());
        }
        else {
            g_D3DD->SetTexture(0, ((CTexture *)m_Tex)->Tex());
        }
        RenderMin();
    }
    void RenderCustom(IDirect3DTexture9 *tex) {
        DTRACE();
        g_D3DD->SetTexture(0, tex);
        RenderMin();
    }
};

struct SProjData {
    D3DXVECTOR3 vpos, vx, vy, vz;
};

#endif