// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

class CMatrixMapObject;

class CMatrixShadowProj : public CVOShadowProj {
    int m_DrawFrame;
    CMatrixMapStatic *m_Owner;

public:
    bool AlreadyDraw(void);
    BYTE *ReadGeometry(BYTE *raw);

    CMatrixMapStatic *GetOwner(void) { return m_Owner; }

    CMatrixShadowProj(CHeap *heap, CMatrixMapStatic *owner) : CVOShadowProj(heap), m_DrawFrame(-1), m_Owner(owner) {}
    ~CMatrixShadowProj();
};

class CMatrixShadowCliper : public CVOShadowCliper {
    CVOShadowProj *m_Shadowproj;
    D3DXMATRIX m_World;

public:
    CMatrixShadowCliper(CVOShadowProj *sp) : m_Shadowproj(sp), CVOShadowCliper(){};
    virtual ~CMatrixShadowCliper(void){};

    D3DXMATRIX *WorldMatrix(void) { return &m_World; }

    virtual void Render(void);
    virtual void BeforeRender(void);
};

void ShadowProjBuildGeom(CVOShadowProj &sp, CVectorObjectAnim &obj, int noframe, const D3DXMATRIX &objma,
                         const D3DXMATRIX &iobjma, D3DXVECTOR3 &light, int mapradius, bool join_to_group);
void ShadowProjBuildGeomList(CVOShadowProj &sp, int cnt, CVectorObjectAnim **obj, const int *noframe,
                             const D3DXMATRIX *wm, const D3DXMATRIX &objma, const D3DXMATRIX &iobjma,
                             D3DXVECTOR3 &light, int mapradius, bool join_to_group);

void ShadowProjBuildTexture(CMatrixMapObject *mo, CVOShadowProj &sp, CVectorObjectAnim &obj, int noframe,
                            const D3DXMATRIX &iobjma, int texsize, bool doclip);
void ShadowProjBuildTexture(CVOShadowProj &sp, CVectorObjectAnim &obj, int noframe, const D3DXMATRIX &iobjma,
                            D3DXVECTOR3 &light, int texsize, bool doclip);
void ShadowProjBuildTextureList(CVOShadowProj &sp, int cnt, CVectorObjectAnim **obj, CTexture *tex, int *noframe,
                                D3DXMATRIX *wm, const D3DXMATRIX &objma, const D3DXMATRIX &iobjma, D3DXVECTOR3 &light,
                                int texsize);

void ShadowProjBuildFull(CVOShadowProj &sp, CVectorObjectAnim &obj, int noframe, const D3DXMATRIX &objma,
                         const D3DXMATRIX &iobjma, D3DXVECTOR3 &light, int mapradius, int texsize, bool doclip,
                         bool render_texture, bool join_to_group);
void ShadowProjBuildFullList(CVOShadowProj &sp, int cnt, CVectorObjectAnim **obj, CTexture *tex, int *noframe,
                             D3DXMATRIX *wm, const D3DXMATRIX &objma, const D3DXMATRIX &iobjma, D3DXVECTOR3 &light,
                             int mapradius, int texsize /*,wchar * texid*/, bool join_to_group);
