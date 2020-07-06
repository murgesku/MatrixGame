// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_RENDER_PIPELINE_INCLUDE
#define MATRIX_RENDER_PIPELINE_INCLUDE

#include "stdafx.h"
#include "matrixmaptexture.hpp"
#include "matrixmapgroup.hpp"
#include "matrixmapstatic.hpp"
#include "MatrixTerSurface.hpp"

typedef void (*WATER_SETUP)(CTextureManaged *tex, CTextureManaged *refl, int pass);
typedef void (*SETUP_CLEAR)(void);

//typedef void (*TERRAIN_TEX_SETUP)(CMatrixMapTextureFinal *tex, int pass);
//typedef void (*TERRAIN_SETUP)(int pass);

typedef void (*TERSURF_TEX_SETUP)(CTextureManaged *tex, CTextureManaged *gloss, int pass);
typedef void (*TERSURF_SETUP)(int pass, bool wrapy);

typedef void (*TERBOT_TEX_SETUP)(int tex, int pass);
typedef void (*TERBOT_SETUP)(int pass);

class CRenderPipeline : public CMain
{
public:

    //IDirect3DVertexShader9  *m_CubeMapShader;
    //IDirect3DVertexDeclaration9 *m_VertexDecl;

    //IDirect3DCubeTexture9 *m_CubeTex;

    CRenderPipeline(void);
    ~CRenderPipeline(void);

    void    SetupTerrains(bool macro);

    // water
    int m_WaterPassSolid;        // passes count
    int m_WaterPassAlpha;
    WATER_SETUP m_WaterSolid;
    WATER_SETUP m_WaterAlpha;
    SETUP_CLEAR m_WaterClearSolid;
    SETUP_CLEAR m_WaterClearAlpha;

    // terain
    //TERRAIN_SETUP       m_Ter[TERRAIN_RENDER_TYPES_CNT];        // prepare  stages
    //TERRAIN_TEX_SETUP   m_TerTex[TERRAIN_RENDER_TYPES_CNT];     // prepare  textures
    //int                 m_TerPass[TERRAIN_RENDER_TYPES_CNT];
    //SETUP_CLEAR         m_TerClear[TERRAIN_RENDER_TYPES_CNT];

    // terain surfaces
    TERSURF_SETUP       m_TerSurf[SURF_TYPES_COUNT];                              // prepare  stages
    TERSURF_TEX_SETUP   m_TerSurfTex[SURF_TYPES_COUNT];                           // prepare  textures
    int                 m_TerSurfPass[SURF_TYPES_COUNT];
    SETUP_CLEAR         m_TerSurfClear[SURF_TYPES_COUNT];

    // terain bottom
    TERBOT_SETUP       m_TerBot[2];                              // prepare  stages
    TERBOT_TEX_SETUP   m_TerBotTex[2];                           // prepare  textures
    int                m_TerBotPass[2];
    SETUP_CLEAR        m_TerBotClear[2];

    // objects
    //OBJECT_SETUP_TEX    m_ObjTex[OBJ_RENDER_TYPES_CNT];
    //OBJECT_SETUP_STAGES m_ObjStages[OBJ_RENDER_TYPES_CNT];
    //SETUP_CLEAR         m_ObjClear;

};



#endif