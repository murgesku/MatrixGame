// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include <vector>

// point light
#define POINTLIGHT_FVF   (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2)
#define POINTLIGHT_FVF_V (D3DFVF_XYZ | D3DFVF_DIFFUSE)

typedef struct {
    D3DXVECTOR3 p;  // Vertex position
    DWORD color;
    float tu0, tv0;  // Vertex texture coordinates
    float tu1, tv1;  // Vertex texture coordinates
    // float       tu;
} SPointLightVertex;

typedef struct {
    D3DXVECTOR3 p;  // Vertex position
    DWORD color;
} SPointLightVertexV;

#define POINTLIGHT_ALTITUDE (0.7f)

#define POINTLIGHT_BILLSCALE 1

struct SMatrixMapPoint;

struct SMapPointLight {
    SMatrixMapPoint *mp;
    float lum;
    int addlum_r, addlum_g, addlum_b;
};

struct SPL_VBIB {
private:
    static SPL_VBIB *m_FirstFree;
    static SPL_VBIB *m_LastFree;

    static SPL_VBIB *m_FirstAll;
    static SPL_VBIB *m_LastAll;

    D3D_VB m_VB;
    D3D_IB m_IB;

    int m_VBSize;
    int m_IBSize;

    SRemindCore m_RemindCore;
    SPL_VBIB *m_PrevFree;
    SPL_VBIB *m_NextFree;
    SPL_VBIB *m_PrevAll;
    SPL_VBIB *m_NextAll;

    DWORD m_FVF;

public:
    void NoNeed(void) {
        if (!IS_VB(m_VB)) {
            Release();
        }
        else {
            LIST_ADD(this, m_FirstFree, m_LastFree, m_PrevFree, m_NextFree);
            m_RemindCore.Use(5000);
        }
    }
    void Release(void);  // phisicaly delete

    static void StaticInit(void) {
        m_FirstFree = NULL;
        m_LastFree = NULL;
        m_FirstAll = NULL;
        m_LastAll = NULL;
    }
    static void ReleaseFree(void) {
        while (m_FirstFree)
            m_FirstFree->Release();
    }

    static void ReleaseBuffers(void) {
        SPL_VBIB *b = m_FirstAll;
        for (; b; b = b->m_NextAll) {
            b->DX_Free();
        }
    }

    static SPL_VBIB *GetCreate(int vbsize, int ibsize, DWORD fvf);

    void *LockVB(int vb_size);
    void *LockIB(int ib_size);

    void UnLockVB() { UNLOCK_VB(m_VB); };
    void UnLockIB() { UNLOCK_IB(m_IB); };

    D3D_VB DX_GetVB(void) { return m_VB; };
    D3D_IB DX_GetIB(void) { return m_IB; };

    void DX_Free(void) {
        if (IS_VB(m_VB))
            DESTROY_VB(m_VB);
        if (IS_IB(m_IB))
            DESTROY_IB(m_IB);
    }

    bool DX_Ready(void) const { return IS_VB(m_VB) && IS_IB(m_IB); }
};

class CMatrixEffectPointLight : public CMatrixEffect {
    D3DXVECTOR3 m_Pos;
    float m_Radius;
    DWORD m_Color;
    DWORD m_InitColor;

    float m_KillTime;
    float m_Time;
    float _m_KT;

    int m_NumVerts;
    int m_NumTris;

    D3DXVECTOR2 m_RealDisp;

    SPL_VBIB *m_DX;

    CTextureManaged *m_Tex;

    CBillboard *m_Bill;

    std::vector<SMapPointLight> m_PointLum;

    CMatrixEffectPointLight(const D3DXVECTOR3 &pos, float r, DWORD color, bool drawbill = false);
    virtual ~CMatrixEffectPointLight();

    void BuildLand(void);
    void BuildLandV(void);

    void UpdateData(void);
    void RemoveColorData(void);
    void AddColorData(void);

    void Clear(void);

    static void MarkAllBuffersNoNeed(void);

public:
    friend class CMatrixEffect;

    static void StaticInit(void) { SPL_VBIB::StaticInit(); }
    static void ClearAll(void) { SPL_VBIB::ReleaseFree(); }

    void SetPosAndRadius(const D3DXVECTOR3 &pos, float r) {
        ASSERT(r > 0);
        m_Pos = pos;
        m_Radius = r;
        UpdateData();
    }

    void SetPos(const D3DXVECTOR3 &pos) {
        m_Pos = pos;
        UpdateData();
    }

    void SetColor(DWORD c) {
        if (m_Bill) {
            m_Bill->SetColor(c);
        }
        RemoveColorData();
        m_Color = c;
        AddColorData();
    }

    void SetRadius(float r) {
        ASSERT(r > 0);
        m_Radius = r;
        UpdateData();
    }

    void Kill(float time);

    DWORD GetColor(void) { return m_Color; }

    virtual void BeforeDraw(void) {
        if (FLAG(m_before_draw_done, BDDF_PLIGHT))
            return;
        if (m_Tex)
            m_Tex->Preload();

        // CTexture *tex = (CTexture *)g_Cache->Get(cc_Texture,TEXTURE_PATH_POINTLIGHT_INV, NULL);
        // tex->Preload();

        SETFLAG(m_before_draw_done, BDDF_PLIGHT);
    };
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);

    virtual int Priority(void);
};
