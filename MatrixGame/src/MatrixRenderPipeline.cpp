// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixRenderPipeline.hpp"
#include "MatrixMap.hpp"
#include "MatrixObject.hpp"

#define REND_TYPE0       0
#define REND_TYPE1       1
#define REND_TYPE2       2
#define REND_TYPE0_GLOSS 3
#define REND_TYPE1_GLOSS 4
#define REND_TYPE2_GLOSS 5

// const char cube_map_shader[] =
//
//    "vs.1.0\n"
//    //"def c64, 0.25f, 0.5f, 1.0f, -1.0f\n"
//
//    "dcl_position v0\n"
//    "dcl_normal v1\n"
//    "dcl_texcoord0 v2\n"
//    "dcl_texcoord1 v3\n"
//
//
//    // r0: camera-space position
//    // r1: camera-space normal
//    // r2: camera-space vertex-eye vector
//    // r3: camera-space reflection vector
//    // r4: texture coordinates
//
//    // Transform position and normal into camera-space
//    "m4x4 r0, v0, c0\n"
//    //"m3x3 r1.xyz, v1, c0\n"
//    //"mov r1.w, c64.z\n"
//
//    //// Compute normalized view vector
//    //"add r2, c8, -r0\n"
//    //"dp3 r3, r2, r2\n"
//    //"rsq r3, r3.w\n"
//    //"mul r2, r2, r3\n"
//
//    //// Compute camera-space reflection vector
//    //"dp3 r3, r1, r2\n"
//    //"mul r1, r1, r3\n"
//    //"add r1, r1, r1\n"
//    //"add r3, r1, -r2\n"
//
//    //// Compute sphere-map texture coords
//    //"mad r4.w, -r3.z, c64.y, c64.y\n"
//    //"rsq r4, r4.w\n"
//    //"mul r4, r3, r4\n"
//    //"mad r4, r4, c64.x, c64.y\n"
//
//    //// Project position
//    //"m4x4 oPos, r0, c4\n"
//    //"mul oT0.xy, r4.xy, c64.zw\n"
//    //"mov oT0.zw, c64.zz\n"
//
//    //"m4x4 r0, v0, c0\n"
//    //"m3x3 r1.xyz, v1, c0\n"
//    //"mov r1.w, c64.z\n"
//
//    //// Compute normalized view vector
//    //"add r2, c8, -r0\n"
//    //"dp3 r3, r2, r2\n"
//    //"rsq r3, r3.w\n"
//    //"mul r2, r2, r3\n"
//
//    //// Compute camera-space reflection vector
//    //"dp3 r3, r1, r2\n"
//    //"mul r1, r1, r3\n"
//    //"add r1, r1, r1\n"
//    //"add r3, r1, -r2\n"
//
//    //// Compute sphere-map texture coords
//    //"mad r4.w, -r3.z, c64.y, c64.y\n"
//    //"rsq r4, r4.w\n"
//    //"mul r4, r3, r4\n"
//    //"mad r4, r4, c64.x, c64.y\n"
//
//    // Project position
//    "m4x4 oPos, r0, c4\n"
//    //"mul oT0.xy, r4.xy, c64.zw\n"
//    //"mov oT0.zw, c64.z\n"
//
//
//    "mov oT0, v2\n"
//    "mov oT1, v3\n"
//
//;

//////////////////////water
void WaterAlpha_t3(CTextureManaged *tex, CTextureManaged *refl, int) {
    DTRACE();

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, g_MatrixMap->m_WaterColor));

    ASSERT_DX(g_D3DD->SetFVF(MATRIX_WATER_VERTEX_FORMAT));

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    ASSERT_DX(g_D3DD->SetTexture(1, tex->Tex()));
    ASSERT_DX(g_D3DD->SetTexture(2, refl->Tex()));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_TEXTURE2, &g_MatrixMap->GetIdentityMatrix()));
    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL));

    SetColorOpSelect(0, D3DTA_CURRENT);
    SetAlphaOpSelect(0, D3DTA_TEXTURE);

    SetColorOpAnyOrder(1, D3DTOP_MODULATE2X, D3DTA_TEXTURE, D3DTA_DIFFUSE);
    SetAlphaOpSelect(1, D3DTA_CURRENT);

    SetColorOp(2, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    SetAlphaOpSelect(2, D3DTA_CURRENT);

    SetColorOpDisable(3);

    // g_D3DD->SetTransform(D3DTS_TEXTURE2, &g_MatrixMap->GetIdentityMatrix());

    ASSERT_DX(g_D3DD->SetStreamSource(0, GET_VB(g_MatrixMap->m_Water->m_VB), 0, sizeof(SMatrixMapWaterVertex)));
    ASSERT_DX(g_D3DD->SetIndices(GET_IB(g_MatrixMap->m_Water->m_IB)));
}

void WaterClearAlpha_t3(void) {
    DTRACE();
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
}

void WaterAlpha_t2_bpp32(CTextureManaged *tex, CTextureManaged *refl, int pass) {
    DTRACE();

    if (pass == 0) {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_COLORWRITEENABLE, 0x8));
        ASSERT_DX(g_D3DD->SetFVF(MATRIX_WATER_VERTEX_FORMAT));

        // ASSERT_DX(g_D3DD->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE ));
        // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA));
        // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA));
        // ASSERT_DX(g_D3DD->SetRenderState( D3DRS_LIGHTING, TRUE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));

        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

        // ASSERT_DX(g_D3DD->SetTexture(1,tex->Tex()));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        // ASSERT_DX(g_D3DD->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX,1));

        SetColorOpSelect(0, D3DTA_CURRENT);
        SetAlphaOpSelect(0, D3DTA_TEXTURE);

        // SetColorOpAnyOrder(1, D3DTOP_MODULATE2X,        D3DTA_TEXTURE, D3DTA_DIFFUSE);
        // SetAlphaOpSelect(1, D3DTA_CURRENT);
        // SetColorOpDisable(2);

        SetColorOpDisable(1);

        ASSERT_DX(g_D3DD->SetStreamSource(0, GET_VB(g_MatrixMap->m_Water->m_VB), 0, sizeof(SMatrixMapWaterVertex)));
        ASSERT_DX(g_D3DD->SetIndices(GET_IB(g_MatrixMap->m_Water->m_IB)));
    }
    else {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_COLORWRITEENABLE, 0x7));

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVDESTALPHA));

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, g_MatrixMap->m_WaterColor));

        ASSERT_DX(g_D3DD->SetTexture(1, refl->Tex()));
        ASSERT_DX(g_D3DD->SetTexture(0, tex->Tex()));

        // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE,	FALSE));
        // ASSERT_DX(g_D3DD->SetFVF(MATRIX_WATER_VERTEX_FORMAT));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, TRUE));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 1));

        // ASSERT_DX(g_D3DD->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));
        // ASSERT_DX(g_D3DD->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL));

        // SetColorOpSelect(0, D3DTA_TEXTURE);
        // SetColorOpDisable(1);

        SetColorOpAnyOrder(0, D3DTOP_MODULATE2X, D3DTA_TEXTURE, D3DTA_DIFFUSE);
        SetColorOp(1, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
        SetColorOpDisable(2);
        SetAlphaOpDisable(0);

        ASSERT_DX(g_D3DD->SetStreamSource(0, GET_VB(g_MatrixMap->m_Water->m_VB), 0, sizeof(SMatrixMapWaterVertex)));
        ASSERT_DX(g_D3DD->SetIndices(GET_IB(g_MatrixMap->m_Water->m_IB)));
    }
}

void WaterClearAlpha_t2_bpp32(void) {
    DTRACE();
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF));
}

void WaterAlpha_t2_bpp16(CTextureManaged *tex, CTextureManaged *refl, int pass) {
    DTRACE();

    // if (pass == 0)
    {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, g_MatrixMap->m_WaterColor));

        ASSERT_DX(g_D3DD->SetFVF(MATRIX_WATER_VERTEX_FORMAT));

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, TRUE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));

        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

        ASSERT_DX(g_D3DD->SetTexture(1, tex->Tex()));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));

        SetColorOpSelect(0, D3DTA_CURRENT);
        SetAlphaOpSelect(0, D3DTA_TEXTURE);

        SetColorOpAnyOrder(1, D3DTOP_MODULATE2X, D3DTA_TEXTURE, D3DTA_DIFFUSE);
        SetAlphaOpSelect(1, D3DTA_CURRENT);
        SetColorOpDisable(2);

        // SetColorOpDisable(1);

        ASSERT_DX(g_D3DD->SetStreamSource(0, GET_VB(g_MatrixMap->m_Water->m_VB), 0, sizeof(SMatrixMapWaterVertex)));
        ASSERT_DX(g_D3DD->SetIndices(GET_IB(g_MatrixMap->m_Water->m_IB)));
    }
    // else
    //{
    //    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP);
    //    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP);

    //    ASSERT_DX(g_D3DD->SetTexture(1,refl->Tex()));
    //    //ASSERT_DX(g_D3DD->SetTexture(0,tex->Tex()));

    // //ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE,	FALSE));
    // //ASSERT_DX(g_D3DD->SetFVF(MATRIX_WATER_VERTEX_FORMAT));
    //    ASSERT_DX(g_D3DD->SetRenderState( D3DRS_LIGHTING, FALSE));

    //    //ASSERT_DX(g_D3DD->SetTextureStageState (0, D3DTSS_TEXCOORDINDEX,0));

    //    //ASSERT_DX(g_D3DD->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));
    //    //ASSERT_DX(g_D3DD->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL));
    //    ASSERT_DX(g_D3DD->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 ));
    //    ASSERT_DX(g_D3DD->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL));

    //    //SetColorOpSelect(0, D3DTA_TEXTURE);
    //    //SetColorOpDisable(1);

    //    SetColorOpSelect(0, D3DTA_CURRENT);
    //    SetAlphaOpSelect(0, D3DTA_TEXTURE);

    //    ASSERT_DX(g_D3DD->SetStreamSource(0,GET_VB(g_MatrixMap->m_Water->m_VB),0,sizeof(SMatrixMapWaterVertex)));
    // ASSERT_DX(g_D3DD->SetIndices(GET_IB(g_MatrixMap->m_Water->m_IB)));
    //}
}

void WaterClearAlpha_t2_bpp16(void) {
    DTRACE();
    // ASSERT_DX(g_D3DD->SetRenderState( D3DRS_LIGHTING, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA));
    // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF));
}

void WaterSolid_t2_bpp16(CTextureManaged *tex, CTextureManaged *refl, int) {
    DTRACE();

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, g_MatrixMap->m_WaterColor));

    // ASSERT_DX(g_D3DD->SetTexture(1,refl->Tex()));
    ASSERT_DX(g_D3DD->SetTexture(0, tex->Tex()));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetFVF(MATRIX_WATER_VERTEX_FORMAT));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, TRUE));

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 1));

    // ASSERT_DX(g_D3DD->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));
    // ASSERT_DX(g_D3DD->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL));
    // ASSERT_DX(g_D3DD->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 ));
    // ASSERT_DX(g_D3DD->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL));

    SetColorOpAnyOrder(0, D3DTOP_MODULATE2X, D3DTA_TEXTURE, D3DTA_DIFFUSE);
    // SetColorOp(1, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    SetColorOpDisable(1);
    SetAlphaOpDisable(0);

    ASSERT_DX(g_D3DD->SetStreamSource(0, GET_VB(g_MatrixMap->m_Water->m_VB), 0, sizeof(SMatrixMapWaterVertex)));
    ASSERT_DX(g_D3DD->SetIndices(GET_IB(g_MatrixMap->m_Water->m_IB)));
}

void WaterSolid_t2(CTextureManaged *tex, CTextureManaged *refl, int) {
    DTRACE();

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, g_MatrixMap->m_WaterColor));

    ASSERT_DX(g_D3DD->SetTexture(1, refl->Tex()));
    ASSERT_DX(g_D3DD->SetTexture(0, tex->Tex()));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetFVF(MATRIX_WATER_VERTEX_FORMAT));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, TRUE));

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 1));

    // ASSERT_DX(g_D3DD->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));
    // ASSERT_DX(g_D3DD->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL));

    SetColorOpAnyOrder(0, D3DTOP_MODULATE2X, D3DTA_TEXTURE, D3DTA_DIFFUSE);
    SetColorOp(1, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    SetColorOpDisable(2);
    SetAlphaOpDisable(0);

    ASSERT_DX(g_D3DD->SetStreamSource(0, GET_VB(g_MatrixMap->m_Water->m_VB), 0, sizeof(SMatrixMapWaterVertex)));
    ASSERT_DX(g_D3DD->SetIndices(GET_IB(g_MatrixMap->m_Water->m_IB)));
}

void WaterClearSolid_t2(void) {
    DTRACE();
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));
    // ASSERT_DX(g_D3DD->SetRenderState( D3DRS_ALPHABLENDENABLE,   FALSE ));
    // ASSERT_DX(g_D3DD->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));

    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU));
}

////////////// water end

//////////////// terain

//###############################################################################################################
//###############################################################################################################
//###############################################################################################################

static void empty_clear(void) {}

// type 0

// type 0 texture
// static void textype0(CMatrixMapTextureFinal *tex,int)
//{
//    CTextureManaged *gloss;
//    ASSERT_DX(g_D3DD->SetTexture(0,tex->GetTextureBottom(&gloss)->Tex()));
//}
// static void textype0_macro(CMatrixMapTextureFinal *tex,int)
//{
//    CTextureManaged *gloss;
//    ASSERT_DX(g_D3DD->SetTexture(0,tex->GetTextureBottom(&gloss)->Tex()));
//    ASSERT_DX(g_D3DD->SetTexture(1,g_MatrixMap->m_Macrotexture->Tex()));
//}

// type 0,1 stages

static void type01(int) {
    DTRACE();

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
    SetAlphaOpDisable(0);
    SetColorOpDisable(1);
}

static void type01_macro(int) {
    DTRACE();
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));

    SetAlphaOpDisable(0);

    SetColorOpSelect(0, D3DTA_TEXTURE);
    SetColorOp(1, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
    SetColorOpDisable(3);
}

//###############################################################################################################
//###############################################################################################################
//###############################################################################################################
// type 1

// type 1 texture
// static void textype1(CMatrixMapTextureFinal *tex,int)
//{
//    CTextureManaged *gloss;
//    ASSERT_DX(g_D3DD->SetTexture(0,tex->GetTextureTop(&gloss)->Tex()));
//}
// static void textype1_macro(CMatrixMapTextureFinal *tex,int)
//{
//    CTextureManaged *gloss;
//    ASSERT_DX(g_D3DD->SetTexture(0,tex->GetTextureTop(&gloss)->Tex()));
//	ASSERT_DX(g_D3DD->SetTexture(1,g_MatrixMap->m_Macrotexture->Tex()));
//}

//###############################################################################################################
//###############################################################################################################
//###############################################################################################################

// type 2

// type 2 texture
static void textype2(CMatrixMapTextureFinal *tex, int) {
    // CTextureManaged *gloss;
    // ASSERT_DX(g_D3DD->SetTexture(0,tex->GetTextureBottom(&gloss)->Tex()));
    // ASSERT_DX(g_D3DD->SetTexture(1,tex->GetTextureTop(&gloss)->Tex()));

    // if (tex->IsTopMixed())
    //{
    //    SetColorOp(1, D3DTOP_BLENDTEXTUREALPHAPM, D3DTA_TEXTURE, D3DTA_CURRENT);
    //} else
    //{
    //    SetColorOp(1, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    //}
}

static void textype2_macro_t3(CMatrixMapTextureFinal *tex, int) {
    //   CTextureManaged *gloss;
    //   ASSERT_DX(g_D3DD->SetTexture(0,tex->GetTextureBottom(&gloss)->Tex()));
    //   ASSERT_DX(g_D3DD->SetTexture(1,tex->GetTextureTop(&gloss)->Tex()));
    // ASSERT_DX(g_D3DD->SetTexture(2,g_MatrixMap->m_Macrotexture->Tex()));

    //   if (tex->IsTopMixed())
    //   {
    //       SetColorOp(1, D3DTOP_BLENDTEXTUREALPHAPM, D3DTA_TEXTURE, D3DTA_CURRENT);
    //   } else
    //   {
    //       SetColorOp(1, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    //   }
}

// type 2 stages

static void type2(int) {
    DTRACE();

    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));

    SetAlphaOpDisable(0);

    // bottom texture
    SetColorOpSelect(0, D3DTA_TEXTURE);

    // top texture
    // see textype2

    // diffuse
    SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);

    SetColorOpDisable(3);
}

static void type2_macro_t3(int) {
    DTRACE();

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));
    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 2));

    SetAlphaOpDisable(0);

    // bottom texture
    SetColorOpSelect(0, D3DTA_TEXTURE);

    // top texture
    // see textype2_macro_t3

    // macro

    SetColorOp(2, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);

    // diffuse
    SetColorOpAnyOrder(3, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
    SetColorOpDisable(4);
}

static void type2_macro_clear_t2(void) {
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
}

static void textype2_macro_t2(CMatrixMapTextureFinal *tex, int pass) {
    // if (pass == 0)
    //{
    //    CTextureManaged *gloss;
    //    ASSERT_DX(g_D3DD->SetTexture(0,tex->GetTextureBottom(&gloss)->Tex()));
    //    ASSERT_DX(g_D3DD->SetTexture(1,tex->GetTextureTop(&gloss)->Tex()));

    //    if (tex->IsTopMixed())
    //    {
    //        SetColorOp(1, D3DTOP_BLENDTEXTUREALPHAPM, D3DTA_TEXTURE, D3DTA_CURRENT);
    //    } else
    //    {
    //        SetColorOp(1, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    //    }
    //} else
    //{
    // ASSERT_DX(g_D3DD->SetTexture(0,g_MatrixMap->m_Macrotexture->Tex()));
    //}
}

static void type2_macro_t2(int pass) {
    DTRACE();

    if (pass == 0) {
        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));

        SetAlphaOpDisable(0);

        // bottom texture
        SetColorOpSelect(0, D3DTA_TEXTURE);

        // top texture
        // see textype2_macro_t3

        // diffuse
        SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
        SetColorOpDisable(3);
    }
    else {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 2));

        // macro
        SetColorOpSelect(0, D3DTA_TEXTURE);
        SetAlphaOpSelect(0, D3DTA_TEXTURE);
        // diffuse
        SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
        SetAlphaOpSelect(1, D3DTA_CURRENT);
        SetColorOpDisable(2);
    }
}

// type 3 #######################################################################################################
//###############################################################################################################
//###############################################################################################################

static void type34_clear_t3(void) {
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
    g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_CURRENT);
}
static void type34_clear_t2(void) {
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
    g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_CURRENT);
}

// type 3 texture
static void textype3_t3(CMatrixMapTextureFinal *tex, int) {
    // CTextureManaged *gloss;
    // ASSERT_DX(g_D3DD->SetTexture(2,tex->GetTextureBottom(&gloss)->Tex()));
    // ASSERT_DX(g_D3DD->SetTexture(0,gloss->Tex()));
    // ASSERT_DX(g_D3DD->SetTexture(1,g_MatrixMap->GetReflectionTexture()->Tex()));
}

static void textype3_t2(CMatrixMapTextureFinal *tex, int pass) {
    // CTextureManaged *gloss;
    // if (pass == 0)
    //{
    //    ASSERT_DX(g_D3DD->SetTexture(0,tex->GetTextureBottom(&gloss)->Tex()));
    //} else
    //{
    //    tex->GetTextureBottom(&gloss);
    //    ASSERT_DX(g_D3DD->SetTexture(0,gloss->Tex()));
    //    ASSERT_DX(g_D3DD->SetTexture(1,g_MatrixMap->GetReflectionTexture()->Tex()));
    //}
}

// type 3 stages

static void type34_t3(int) {
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));

    SetAlphaOpDisable(0);

    SetColorOpSelect(0, D3DTA_TEXTURE);
    SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
    g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_TEMP);

    SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
    SetColorOpAnyOrder(3, D3DTOP_ADD, D3DTA_TEMP, D3DTA_CURRENT);
    SetColorOpDisable(4);
}

static void type34_t2(int pass) {
    if (pass == 0) {
        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

        SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
        SetColorOpDisable(1);
        SetAlphaOpDisable(0);
    }
    else {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));

        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

        SetColorOpSelect(0, D3DTA_TEXTURE);
        SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
        SetColorOpDisable(2);
    }
}

static void textype3_macro_t3(CMatrixMapTextureFinal *tex, int pass) {
    //   CTextureManaged *gloss;
    //   ASSERT_DX(g_D3DD->SetTexture(2,tex->GetTextureBottom(&gloss)->Tex()));
    //   ASSERT_DX(g_D3DD->SetTexture(0,gloss->Tex()));
    //   ASSERT_DX(g_D3DD->SetTexture(1,g_MatrixMap->GetReflectionTexture()->Tex()));
    // ASSERT_DX(g_D3DD->SetTexture(3,g_MatrixMap->m_Macrotexture->Tex()));
}

static void type34_clear_macro_t3(void) {
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
    g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_CURRENT);
#ifndef MACROTEXTURE_COORDS
    ASSERT_DX(g_D3DD->SetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
#endif
}

static void type34_macro_t3(int) {
    DTRACE();

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, 1));

    SetAlphaOpDisable(0);

    SetColorOpSelect(0, D3DTA_TEXTURE);
    SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
    g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_TEMP);

    SetColorOpSelect(2, D3DTA_TEXTURE);
    SetColorOp(3, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    SetColorOpAnyOrder(4, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
    SetColorOpAnyOrder(5, D3DTOP_ADD, D3DTA_TEMP, D3DTA_CURRENT);
    SetColorOpDisable(6);
}

// type 4 #######################################################################################################
//###############################################################################################################
//###############################################################################################################

static void textype4_t3(CMatrixMapTextureFinal *tex, int) {
    // CTextureManaged *gloss;
    // ASSERT_DX(g_D3DD->SetTexture(2,tex->GetTextureTop(&gloss)->Tex()));
    // ASSERT_DX(g_D3DD->SetTexture(0,gloss->Tex()));
    // ASSERT_DX(g_D3DD->SetTexture(1,g_MatrixMap->GetReflectionTexture()->Tex()));
}

static void textype4_t2(CMatrixMapTextureFinal *tex, int pass) {
    // CTextureManaged *gloss;
    // if (pass == 0)
    //{
    //    ASSERT_DX(g_D3DD->SetTexture(0,tex->GetTextureTop(&gloss)->Tex()));
    //} else
    //{
    //    tex->GetTextureTop(&gloss);
    //    ASSERT_DX(g_D3DD->SetTexture(0,gloss->Tex()));
    //    ASSERT_DX(g_D3DD->SetTexture(1,g_MatrixMap->GetReflectionTexture()->Tex()));
    //}
}

static void textype4_macro_t3(CMatrixMapTextureFinal *tex, int) {
    //   CTextureManaged *gloss;
    //   ASSERT_DX(g_D3DD->SetTexture(2,tex->GetTextureTop(&gloss)->Tex()));
    //   ASSERT_DX(g_D3DD->SetTexture(0,gloss->Tex()));
    //   ASSERT_DX(g_D3DD->SetTexture(1,g_MatrixMap->GetReflectionTexture()->Tex()));
    // ASSERT_DX(g_D3DD->SetTexture(3,g_MatrixMap->m_Macrotexture->Tex()));
}

// type 5 #######################################################################################################
//###############################################################################################################
//###############################################################################################################

static void type5_clear_t2(void) {
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
}

static void textype5_t2(CMatrixMapTextureFinal *tex, int pass) {
    // CTextureManaged *gloss;
    // if (pass == 0)
    //{
    //    ASSERT_DX(g_D3DD->SetTexture(0,tex->GetTextureBottom(&gloss)->Tex()));
    //    ASSERT_DX(g_D3DD->SetTexture(1,tex->GetTextureTop(&gloss)->Tex()));
    //    // top
    //    if (tex->IsTopMixed())
    //    {
    //        SetColorOp(1, D3DTOP_BLENDTEXTUREALPHAPM, D3DTA_TEXTURE, D3DTA_CURRENT);
    //    } else
    //    {
    //        SetColorOp(1, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    //    }

    //} else
    //{
    //    tex->GetTextureTop(&gloss);
    //    ASSERT_DX(g_D3DD->SetTexture(0,gloss->Tex()));
    //    ASSERT_DX(g_D3DD->SetTexture(1,g_MatrixMap->GetReflectionTexture()->Tex()));
    //}
}

static void type5_t2(int pass) {
    DTRACE();
    if (pass == 0) {
        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));

        SetAlphaOpDisable(0);

        // bottom
        SetColorOpSelect(0, D3DTA_TEXTURE);

        // diffuse
        SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);

        // disable other
        SetColorOpDisable(3);
    }
    else {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 1));  // texture coords as top
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

        SetAlphaOpDisable(0);

        // SetColorOpSelect(1, D3DTA_TEXTURE);
        // SetColorOpDisable(2);

        // gloss
        SetColorOpSelect(0, D3DTA_TEXTURE);
        SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
        // diffuse
        SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
        // disable other
        SetColorOpDisable(3);
    }
}

static void type5_clear_t4(void) {
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
    g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_CURRENT);
}

static void textype5_t4(CMatrixMapTextureFinal *tex, int) {
    // CTextureManaged *gloss;
    // ASSERT_DX(g_D3DD->SetTexture(2,tex->GetTextureBottom(&gloss)->Tex()));
    // ASSERT_DX(g_D3DD->SetTexture(3,tex->GetTextureTop(&gloss)->Tex()));

    // ASSERT_DX(g_D3DD->SetTexture(0,gloss->Tex()));
    // ASSERT_DX(g_D3DD->SetTexture(1,g_MatrixMap->GetReflectionTexture()->Tex()));

    //// top
    // if (tex->IsTopMixed())
    //{
    //    SetColorOp(3, D3DTOP_BLENDTEXTUREALPHAPM, D3DTA_TEXTURE, D3DTA_CURRENT);
    //} else
    //{
    //    SetColorOp(3, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    //}
}

static void type5_t4(int) {
    DTRACE();

    SetAlphaOpDisable(0);

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 1));  // texture coords as top
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, 1));

    // gloss
    SetColorOpSelect(0, D3DTA_TEXTURE);
    SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);

    g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_TEMP);

    // bottom
    SetColorOpSelect(2, D3DTA_TEXTURE);

    // diffuse
    SetColorOpAnyOrder(4, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);

    SetColorOpAnyOrder(5, D3DTOP_ADD, D3DTA_TEMP, D3DTA_CURRENT);

    // disable other
    SetColorOpDisable(6);
}

static void type5_clear_macro_t3(void) {
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
    // ASSERT_DX(g_D3DD->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX,	1 ));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
}

static void textype5_macro_t3(CMatrixMapTextureFinal *tex, int pass) {
    // CTextureManaged *gloss;
    // if (pass == 0)
    //{
    //    ASSERT_DX(g_D3DD->SetTexture(0,tex->GetTextureBottom(&gloss)->Tex()));
    //    ASSERT_DX(g_D3DD->SetTexture(1,tex->GetTextureTop(&gloss)->Tex()));
    // ASSERT_DX(g_D3DD->SetTexture(2,g_MatrixMap->m_Macrotexture->Tex()));

    //    // top
    //    if (tex->IsTopMixed())
    //    {
    //        SetColorOp(1, D3DTOP_BLENDTEXTUREALPHAPM, D3DTA_TEXTURE, D3DTA_CURRENT);
    //    } else
    //    {
    //        SetColorOp(1, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    //    }
    //} else
    //{
    //    tex->GetTextureTop(&gloss);
    //    ASSERT_DX(g_D3DD->SetTexture(0,gloss->Tex()));
    //    ASSERT_DX(g_D3DD->SetTexture(1,g_MatrixMap->GetReflectionTexture()->Tex()));
    //}
}

static void type5_macro_t3(int pass) {
    DTRACE();

    if (pass == 0) {
        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));
        ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 2));

        SetAlphaOpDisable(0);

        // bottom
        SetColorOpSelect(0, D3DTA_TEXTURE);

        // top see textype3

        // macro
        SetColorOp(2, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);

        // diffuse
        SetColorOpAnyOrder(3, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);

        // disable other

        SetColorOpDisable(4);
    }
    else {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 1));  // texture coords as top

        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

        SetAlphaOpDisable(0);

        // gloss
        SetColorOpSelect(0, D3DTA_TEXTURE);
        SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
        // macro
        // SetColorOp(2, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
        // diffuse
        // SetColorOpAnyOrder(3, D3DTOP_MODULATE,          D3DTA_DIFFUSE, D3DTA_CURRENT);
        // disable other
        SetColorOpDisable(2);

#ifndef MACROTEXTURE_COORDS
        ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
#endif
    }
}

static void type5_clear_macro_t5(void) {
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
    g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_CURRENT);
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));
#ifndef MACROTEXTURE_COORDS
    ASSERT_DX(g_D3DD->SetTextureStageState(4, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
#endif
}

static void textype5_macro_t5(CMatrixMapTextureFinal *tex, int) {
    //   CTextureManaged *gloss;
    //   ASSERT_DX(g_D3DD->SetTexture(2,tex->GetTextureBottom(&gloss)->Tex()));
    //   ASSERT_DX(g_D3DD->SetTexture(3,tex->GetTextureTop(&gloss)->Tex()));
    //   ASSERT_DX(g_D3DD->SetTexture(0,gloss->Tex()));
    //   ASSERT_DX(g_D3DD->SetTexture(1,g_MatrixMap->GetReflectionTexture()->Tex()));
    // ASSERT_DX(g_D3DD->SetTexture(4,g_MatrixMap->m_Macrotexture->Tex()));

    //   // top
    //   if (tex->IsTopMixed())
    //   {
    //       SetColorOp(3, D3DTOP_BLENDTEXTUREALPHAPM, D3DTA_TEXTURE, D3DTA_CURRENT);
    //   } else
    //   {
    //       SetColorOp(3, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    //   }
}

static void type5_macro_t5(int pass) {
    DTRACE();

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 1));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, 1));  // texture coords as top
    ASSERT_DX(g_D3DD->SetTextureStageState(4, D3DTSS_TEXCOORDINDEX, 2));

    SetAlphaOpDisable(0);

    // gloss
    SetColorOpSelect(0, D3DTA_TEXTURE);
    SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
    g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_TEMP);

    // bottom
    SetColorOpSelect(2, D3DTA_TEXTURE);

    // top see textype5

    // macro
    SetColorOp(4, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);

    // diffuse
    SetColorOpAnyOrder(5, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);

    SetColorOpAnyOrder(6, D3DTOP_ADD, D3DTA_TEMP, D3DTA_CURRENT);

    // disable other
    SetColorOpDisable(7);
}

//###############################################################################################################
//####     ###   ##     #########################################################################################
//#### #### # ### ### ###########################################################################################
//#### #### # ### ### ###########################################################################################
//####     ## ### ### ###########################################################################################
//#### #### # ### ### ###########################################################################################
//#### #### # ### ### ###########################################################################################
//####     ###   #### ###########################################################################################
//###############################################################################################################

static void TerBotClear(void) {
    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

static void TerBotTex(int tex, int pass) {
    CTextureManaged *t = CBottomTextureUnion::Get(tex).GetTexture();
    ASSERT_DX(g_D3DD->SetTexture(0, t->Tex()));
}
static void TerBotTexM(int tex, int pass) {
    CTextureManaged *t = CBottomTextureUnion::Get(tex).GetTexture();

    ASSERT_DX(g_D3DD->SetTexture(0, t->Tex()));
    ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->m_Macrotexture->Tex()));
}

static void TerBot(int pass) {
    // SetColorOpSelect(0, D3DTA_TEXTURE);
    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
    SetAlphaOpDisable(0);
    SetColorOpDisable(1);

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
}
static void TerBotM(int pass) {
    SetColorOpSelect(0, D3DTA_TEXTURE);
    SetAlphaOpDisable(0);
    SetColorOp(1, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
    SetColorOpDisable(3);

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

    g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
    g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
}

static void TerBotTexM_sux(int tex, int pass) {
    if (pass == 0) {
        CTextureManaged *t = CBottomTextureUnion::Get(tex).GetTexture();
        ASSERT_DX(g_D3DD->SetTexture(0, t->Tex()));
    }
    else
        ASSERT_DX(g_D3DD->SetTexture(0, g_MatrixMap->m_Macrotexture->Tex()));
}

static void TerBotM_sux(int pass) {
    if (pass == 0) {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);

        SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
        SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);

        SetAlphaOpDisable(1);
        SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
        SetColorOpDisable(2);
    }
    else {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

        g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 1);
        SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
        SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
        // SetAlphaOpSelect(0, D3DTA_TEXTURE);
        SetAlphaOpDisable(1);

        SetColorOpDisable(1);

        // SetColorOp(1, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    }
}

static void TerBotClear_sux(void) {
    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
}

//###############################################################################################################
//#####   ## #### #     ##      #################################################################################
//#### ##### #### # #### # ######################################################################################
//#### ##### #### # #### # ######################################################################################
//#####   ## #### # #### #    ###################################################################################
//######## # #### #    ### ######################################################################################
//######## # #### # ## ### ######################################################################################
//#####   ###    ## ### ## ######################################################################################
//###############################################################################################################

static void TerSurfClearGloss(void) {
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
    g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_CURRENT);
    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 2));
}

static void TerSurfTexGloss(CTextureManaged *tex, CTextureManaged *gloss, int pass) {
    ASSERT_DX(g_D3DD->SetTexture(0, gloss->Tex()));
    ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->GetReflectionTexture()->Tex()));
    ASSERT_DX(g_D3DD->SetTexture(2, tex->Tex()));
}

static void TerSurfGloss(int pass, bool wrapy) {
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));

    SetAlphaOpSelect(0, D3DTA_CURRENT);
    SetAlphaOpSelect(1, D3DTA_CURRENT);

    SetColorOpSelect(0, D3DTA_TEXTURE);
    SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
    g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_TEMP);

    SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
    SetAlphaOp(2, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);

    SetColorOpAnyOrder(3, D3DTOP_ADD, D3DTA_TEMP, D3DTA_CURRENT);
    SetAlphaOpSelect(3, D3DTA_CURRENT);

    SetColorOpAnyOrder(4, D3DTOP_MODULATE, D3DTA_TFACTOR, D3DTA_CURRENT);
    SetAlphaOpSelect(4, D3DTA_CURRENT);
    SetColorOpDisable(5);

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);

    if (wrapy) {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    }
    else {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    }

    g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);  // refl
    g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);  // refl
}

static void TerSurfGlossW(int pass, bool wrapy) {
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));

    SetAlphaOpSelect(0, D3DTA_CURRENT);
    SetAlphaOpSelect(1, D3DTA_CURRENT);

    SetColorOpSelect(0, D3DTA_TEXTURE);
    SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
    g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_TEMP);

    SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
    SetAlphaOpSelect(2, D3DTA_TEXTURE);
    SetColorOpAnyOrder(3, D3DTOP_ADD, D3DTA_TEMP, D3DTA_CURRENT);
    SetAlphaOpSelect(3, D3DTA_CURRENT);
    SetColorOpDisable(4);

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);

    if (wrapy) {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    }
    else {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    }

    g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);  // refl
    g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);  // refl
}

// gloss macro 4

static void TerSurfClearGlossM(void) {
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
    g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_CURRENT);
    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 2));
    ASSERT_DX(g_D3DD->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, 3));
}

static void TerSurfTexGlossM(CTextureManaged *tex, CTextureManaged *gloss, int pass) {
    ASSERT_DX(g_D3DD->SetTexture(0, gloss->Tex()));
    ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->GetReflectionTexture()->Tex()));
    ASSERT_DX(g_D3DD->SetTexture(2, tex->Tex()));
    ASSERT_DX(g_D3DD->SetTexture(3, g_MatrixMap->m_Macrotexture->Tex()));
}

static void TerSurfGlossM(int pass, bool wrapy) {
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, 1));

    SetAlphaOpSelect(0, D3DTA_CURRENT);
    SetAlphaOpSelect(1, D3DTA_CURRENT);

    SetColorOpSelect(0, D3DTA_TEXTURE);
    SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
    g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_TEMP);

    SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);

    SetColorOp(3, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    SetAlphaOpSelect(3, D3DTA_CURRENT);

    SetColorOpAnyOrder(4, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
    SetAlphaOpSelect(4, D3DTA_CURRENT);
    SetColorOpAnyOrder(5, D3DTOP_ADD, D3DTA_TEMP, D3DTA_CURRENT);
    SetAlphaOpSelect(5, D3DTA_CURRENT);
    SetColorOpDisable(6);

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);

    if (wrapy) {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    }
    else {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    }
    g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);  // refl
    g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);  // refl

    g_D3DD->SetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);  // macro
    g_D3DD->SetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);  // macro
}

static void TerSurfGlossMW(int pass, bool wrapy) {
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, 1));

    SetAlphaOpSelect(0, D3DTA_CURRENT);
    SetAlphaOpSelect(1, D3DTA_CURRENT);

    SetColorOpSelect(0, D3DTA_TEXTURE);
    SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
    g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_TEMP);

    SetColorOpSelect(2, D3DTA_TEXTURE);
    SetAlphaOpSelect(2, D3DTA_TEXTURE);

    SetColorOp(3, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    SetAlphaOpSelect(3, D3DTA_CURRENT);

    SetColorOpAnyOrder(4, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
    SetAlphaOpSelect(4, D3DTA_CURRENT);

    SetColorOpAnyOrder(5, D3DTOP_ADD, D3DTA_TEMP, D3DTA_CURRENT);
    SetAlphaOpSelect(5, D3DTA_CURRENT);
    SetColorOpDisable(6);

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);

    if (wrapy) {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    }
    else {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    }
    g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);  // refl
    g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);  // refl

    g_D3DD->SetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);  // macro
    g_D3DD->SetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);  // macro
}

// gloss 2

static void TerSurfClearGloss2(void) {
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
}

static void TerSurfTexGloss2(CTextureManaged *tex, CTextureManaged *gloss, int pass) {
    if (pass == 0) {
        ASSERT_DX(g_D3DD->SetTexture(0, tex->Tex()));
    }
    else {
        ASSERT_DX(g_D3DD->SetTexture(0, gloss->Tex()));
        ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->GetReflectionTexture()->Tex()));
    }
}

static void TerSurfGloss2W(int pass, bool wrapy) {
    if (pass == 0) {
        SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
        SetAlphaOpSelect(0, D3DTA_TEXTURE);
        SetAlphaOpDisable(1);
        SetColorOpDisable(1);

        // g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP);
        // g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP);

        g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);

        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);

        if (wrapy) {
            g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        }
        else {
            g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        }
    }
    else {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

        SetColorOpSelect(0, D3DTA_TEXTURE);
        SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
        SetColorOpDisable(2);

        // SetAlphaOpSelect(0, D3DTA_TEXTURE);
        SetAlphaOpDisable(0);

        g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);  // refl
        g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);  // refl
    }
}

static void TerSurfGloss2(int pass, bool wrapy) {
    if (pass == 0) {
        SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
        SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
        SetAlphaOpDisable(1);
        SetColorOpDisable(1);

        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        if (wrapy) {
            g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        }
        else {
            g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        }

        g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
    }
    else {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

        SetColorOpSelect(0, D3DTA_TEXTURE);
        SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
        SetColorOpDisable(2);

        // SetAlphaOpSelect(0, D3DTA_TEXTURE);
        SetAlphaOpDisable(0);

        g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);  // refl
        g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);  // refl
    }
}

// gloss macro 2

static void TerSurfClearGloss2M(void) {
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
}

static void TerSurfTexGloss2M(CTextureManaged *tex, CTextureManaged *gloss, int pass) {
    if (pass == 0) {
        ASSERT_DX(g_D3DD->SetTexture(0, tex->Tex()));
    }
    else {
        ASSERT_DX(g_D3DD->SetTexture(0, gloss->Tex()));
        ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->GetReflectionTexture()->Tex()));
    }
}

void TerSurfM_sux(int pass);
static void TerSurfGloss2M(int pass) {
    // TerSurfM_sux(pass);
    return;
}

// void TerSurfMW_sux(int pass);
static void TerSurfGloss2MW(int pass) {
    if (pass == 0) {
        SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
        SetAlphaOpSelect(0, D3DTA_TEXTURE);
        SetAlphaOpDisable(1);
        SetColorOpDisable(1);

        // g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP);
        // g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP);

        g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
    }
    else {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

        SetColorOpSelect(0, D3DTA_TEXTURE);
        SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
        SetColorOpDisable(2);

        // SetAlphaOpSelect(0, D3DTA_TEXTURE);
        SetAlphaOpDisable(0);
    }
}

static void TerSurfClear(void) {}

static void TerSurfTex(CTextureManaged *tex, CTextureManaged *gloss, int pass) {
    ASSERT_DX(g_D3DD->SetTexture(0, tex->Tex()));
}
static void TerSurfTexM(CTextureManaged *tex, CTextureManaged *gloss, int pass) {
    ASSERT_DX(g_D3DD->SetTexture(0, tex->Tex()));
    ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->m_Macrotexture->Tex()));
}

static void TerSurf(int pass, bool wrapy) {
    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);

    SetAlphaOpDisable(1);
    SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);

    SetColorOpDisable(2);

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    if (wrapy) {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    }
    else {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    }

    g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
}

static void TerSurfW(int pass, bool wrapy) {
    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
    SetAlphaOpSelect(0, D3DTA_TEXTURE);
    SetAlphaOpDisable(1);
    SetColorOpDisable(1);

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    if (wrapy) {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    }
    else {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    }

    g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
}

static void TerSurfMW(int pass, bool wrapy) {
    SetColorOpSelect(0, D3DTA_TEXTURE);
    SetAlphaOpSelect(0, D3DTA_TEXTURE);

    // SetAlphaOp(1, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    SetAlphaOpSelect(1, D3DTA_CURRENT);
    SetColorOp(1, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);

    SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
    SetAlphaOpSelect(2, D3DTA_CURRENT);

    SetAlphaOpDisable(3);
    SetColorOpDisable(3);

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    if (wrapy) {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    }
    else {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    }

    g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

    g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
    g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
}

static void TerSurfM(int pass, bool wrapy) {
    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);

    // SetAlphaOpDisable(1);
    SetAlphaOpSelect(1, D3DTA_CURRENT);
    SetAlphaOpSelect(2, D3DTA_CURRENT);

    SetColorOp(1, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
    SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
    SetColorOpDisable(3);

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    if (wrapy) {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    }
    else {
        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    }
    g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

    g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
    g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
}

static void TerSurfMW_sux(int pass, bool wrapy) {
    if (pass == 0) {
        SetColorOpSelect(0, D3DTA_TEXTURE);
        SetAlphaOpSelect(0, D3DTA_TEXTURE);

        SetAlphaOpSelect(1, D3DTA_CURRENT);
        SetColorOp(1, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);

        SetAlphaOpDisable(2);
        SetColorOpDisable(2);

        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        if (wrapy) {
            g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        }
        else {
            g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        }
        g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

        g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
    }
    else {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR));

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0x00FFFFFF));

        SetColorOp(0, D3DTOP_BLENDTEXTUREALPHA, D3DTA_DIFFUSE, D3DTA_TFACTOR);
        SetAlphaOpDisable(0);
        SetColorOpDisable(1);
    }
}

static void TerSurfTexM_sux(CTextureManaged *tex, CTextureManaged *gloss, int pass) {
    if (pass == 0) {
        ASSERT_DX(g_D3DD->SetTexture(0, tex->Tex()));
        ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->m_Macrotexture->Tex()));
    }
    // else
}

static void TerSurfM_sux(int pass, bool wrapy) {
    if (pass == 0) {
        SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
        SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);

        SetAlphaOpSelect(1, D3DTA_CURRENT);
        SetColorOp(1, D3DTOP_BLENDTEXTUREALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);

        SetAlphaOpDisable(2);
        SetColorOpDisable(2);

        g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        if (wrapy) {
            g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        }
        else {
            g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        }

        g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);  // wrap
        g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);  // wrap

        g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
    }
    else {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR));

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0x00FFFFFF));

        SetColorOp(0, D3DTOP_BLENDTEXTUREALPHA, D3DTA_DIFFUSE, D3DTA_TFACTOR);
        SetAlphaOpDisable(0);
        SetColorOpDisable(1);
    }
}

static void TerSurfClear_sux(void) {
    g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
}

//###############################################################################################################
//#######        ###      ######### #############################################################################
//###### ######## ## ##### ######## #############################################################################
//###### ######## ## ##### ######## #############################################################################
//###### ######## ##      ######### #############################################################################
//###### ######## ## ##### ######## #############################################################################
//###### ######## ## ##### ## ##### #############################################################################
//#######        ###      ####     ##############################################################################
//###############################################################################################################

static void obj_clear(void) {
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));

    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_CURRENT));
}

// void obj_ordinal_tex_t2(SVOSurface *vo, DWORD user_param, int )
//{
//    if (user_param == 0)
//    {
//        ASSERT_DX(g_D3DD->SetTexture(0, vo->m_Tex->Tex()));
//        if (vo->m_Tex->Flags() & TF_TRANS)
//        {
//            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
//        }
//    } else
//    {
//
//        CMatrixMapObject *o = (CMatrixMapObject *)user_param;
//
//        if (o->m_BurnTexVis == 255)
//        {
//            g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR,o->GetTerrainColor());
//            ASSERT_DX(g_D3DD->SetTexture(0, o->m_BurnTex->Tex()));
//            if (o->m_BurnTex->Flags() & TF_TRANS)
//            {
//                ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
//            }
//        } else
//        {
//            ASSERT_DX(g_D3DD->SetTexture(0, vo->m_Tex->Tex()));
//            if (vo->m_Tex->Flags() & TF_TRANS)
//            {
//                ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
//            }
//        }
//    }
//}

bool obj_ordinal_t2(DWORD user_param, int) {
    obj_clear();

    // ordinal mapping
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
    SetAlphaOpSelect(0, D3DTA_TEXTURE);

    SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TFACTOR, D3DTA_CURRENT);
    SetAlphaOpSelect(1, D3DTA_CURRENT);

    SetColorOpDisable(2);

    return false;
}

// void obj_ordinal_tex_t4(SVOSurface *vo, DWORD user_param, int )
//{
//    if (user_param == 0)
//    {
//        ASSERT_DX(g_D3DD->SetTexture(0, vo->m_Tex->Tex()));
//        if (vo->m_Tex->Flags() & TF_TRANS)
//        {
//            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
//        }
//    } else
//    {
//
//        CMatrixMapObject *o = (CMatrixMapObject *)user_param;
//
//        if (o->m_BurnTexVis == 255)
//        {
//            g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR,o->GetTerrainColor());
//            ASSERT_DX(g_D3DD->SetTexture(0, o->m_BurnTex->Tex()));
//            if (o->m_BurnTex->Flags() & TF_TRANS)
//            {
//                ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
//            }
//        } else
//        {
//            DWORD tf = (o->GetTerrainColor() & 0x00FFFFFF) | (o->m_BurnTexVis << 24);
//            g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR,tf);
//
//            ASSERT_DX(g_D3DD->SetTexture(0, vo->m_Tex->Tex()));
//            ASSERT_DX(g_D3DD->SetTexture(1, o->m_BurnTex->Tex()));
//            if (vo->m_Tex->Flags() & TF_TRANS)
//            {
//                ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
//                //ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
//                //ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
//            }
//        }
//    }
//}

bool obj_ordinal_t4(DWORD user_param, int) {
    obj_clear();

    if (user_param == 0) {
    one_tex:
        // ordinal mapping
        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

        SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
        SetAlphaOpSelect(0, D3DTA_TEXTURE);

        SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TFACTOR, D3DTA_CURRENT);
        SetAlphaOpSelect(1, D3DTA_CURRENT);

        SetColorOpDisable(2);
    }
    else {
        CMatrixMapObject *o = (CMatrixMapObject *)user_param;
        if (o->m_BurnSkinVis == 255)
            goto one_tex;

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

        SetColorOpSelect(0, D3DTA_TEXTURE);
        SetAlphaOpSelect(0, D3DTA_TEXTURE);

        SetColorOp(1, D3DTOP_BLENDFACTORALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
        SetAlphaOp(1, D3DTOP_BLENDFACTORALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
        // SetAlphaOpSelect(1, D3DTA_TEXTURE);

        SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
        SetAlphaOpSelect(2, D3DTA_CURRENT);

        SetColorOpAnyOrder(3, D3DTOP_MODULATE, D3DTA_TFACTOR, D3DTA_CURRENT);
        SetAlphaOpSelect(3, D3DTA_CURRENT);

        SetColorOpDisable(4);
    }

    return false;
}

// void obj_ordinal_tex_gloss_t4(SVOSurface *vo, DWORD user_param, int )
//{
//    if (user_param == 0)
//    {
//        ASSERT_DX(g_D3DD->SetTexture(0, vo->m_TexGloss->Tex()));
//        ASSERT_DX(g_D3DD->SetTexture(1,g_MatrixMap->GetReflectionTexture()->Tex()));
//        ASSERT_DX(g_D3DD->SetTexture(2, vo->m_Tex->Tex()));
//
//
//        if (vo->m_Tex->Flags() & TF_TRANS)
//        {
//            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
//        }
//    } else
//    {
//
//        CMatrixMapObject *o = (CMatrixMapObject *)user_param;
//
//        if (o->m_BurnTexVis == 255)
//        {
//            g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR,o->GetTerrainColor());
//            ASSERT_DX(g_D3DD->SetTexture(0, o->m_BurnTex->Tex()));
//            if (o->m_BurnTex->Flags() & TF_TRANS)
//            {
//                ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
//            }
//        } else
//        {
//            DWORD tf = (o->GetTerrainColor() & 0x00FFFFFF) | (o->m_BurnTexVis << 24);
//            g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR,tf);
//
//            ASSERT_DX(g_D3DD->SetTexture(0, vo->m_TexGloss->Tex()));
//            ASSERT_DX(g_D3DD->SetTexture(1,g_MatrixMap->GetReflectionTexture()->Tex()));
//            ASSERT_DX(g_D3DD->SetTexture(2, vo->m_Tex->Tex()));
//            ASSERT_DX(g_D3DD->SetTexture(5, o->m_BurnTex->Tex()));
//            if (vo->m_Tex->Flags() & TF_TRANS)
//            {
//                ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
//                //ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
//                //ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
//            }
//        }
//    }
//}

bool obj_ordinal_gloss_t4(DWORD user_param, int) {
    obj_clear();

    if (user_param == 0) {
        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

        ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));

        SetAlphaOpSelect(0, D3DTA_CURRENT);
        SetColorOpSelect(0, D3DTA_TEXTURE);
        SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);

        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_TEMP));
        SetAlphaOpSelect(1, D3DTA_CURRENT);

        SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
        SetAlphaOpSelect(2, D3DTA_TEXTURE);

        SetColorOpAnyOrder(3, D3DTOP_MODULATE, D3DTA_TFACTOR, D3DTA_CURRENT);
        SetAlphaOpSelect(3, D3DTA_CURRENT);

        SetColorOpAnyOrder(4, D3DTOP_ADD, D3DTA_TEMP, D3DTA_CURRENT);
        SetAlphaOpSelect(4, D3DTA_CURRENT);

        SetColorOpDisable(5);
    }
    else {
        CMatrixMapObject *o = (CMatrixMapObject *)user_param;
        if (o->m_BurnSkinVis == 255) {
            // only one texture without gloss...
            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));

            ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

            SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
            SetAlphaOpSelect(0, D3DTA_TEXTURE);

            SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TFACTOR, D3DTA_CURRENT);
            SetAlphaOpSelect(1, D3DTA_CURRENT);

            SetColorOpDisable(2);

            return false;
        }

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

        ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(5, D3DTSS_TEXCOORDINDEX, 0));

        SetAlphaOpSelect(0, D3DTA_CURRENT);
        SetColorOpSelect(0, D3DTA_TEXTURE);
        SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_TEMP));

        SetAlphaOpSelect(1, D3DTA_CURRENT);

        SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
        SetAlphaOpSelect(2, D3DTA_TEXTURE);

        SetColorOpAnyOrder(3, D3DTOP_MODULATE, D3DTA_TFACTOR, D3DTA_CURRENT);
        SetAlphaOpSelect(3, D3DTA_CURRENT);

        SetColorOpAnyOrder(4, D3DTOP_ADD, D3DTA_TEMP, D3DTA_CURRENT);
        SetAlphaOpSelect(4, D3DTA_CURRENT);

        SetColorOp(5, D3DTOP_BLENDFACTORALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
        SetAlphaOp(5, D3DTOP_BLENDFACTORALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);

        SetColorOpDisable(6);
    }

    return false;
}

///////////// side

bool obj_side_t3(DWORD user_param, int) {
    obj_clear();

    // ordinal mapping
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpSelect(0, D3DTA_TEXTURE);

    SetColorOp(1, D3DTOP_BLENDCURRENTALPHA, D3DTA_CURRENT, D3DTA_TEXTURE);
    SetAlphaOpDisable(1);

    SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_CURRENT, D3DTA_DIFFUSE);

    SetColorOpDisable(3);

    return false;
}

// void obj_side_tex_t2(SVOSurface *vo, DWORD user_param, int )
//{
//    LPDIRECT3DTEXTURE9 coltex = (LPDIRECT3DTEXTURE9)user_param;
//
//    ASSERT_DX(g_D3DD->SetTexture(0, vo->m_Tex->Tex()));
//    ASSERT_DX(g_D3DD->SetTexture(1, coltex));
//}

bool obj_side_t2(DWORD user_param, int) {
    obj_clear();

    // ASSERT_DX(g_D3DD->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 ));
    // static float da = 0;
    // D3DXMATRIX m;
    // D3DXMatrixRotationZ(&m,  da += 0.005f);
    // g_D3DD->SetTransform(D3DTS_TEXTURE0, &m);

    // ordinal mapping
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpSelect(0, D3DTA_TEXTURE);

    SetColorOp(1, D3DTOP_BLENDCURRENTALPHA, D3DTA_CURRENT, D3DTA_TEXTURE);
    SetAlphaOpDisable(1);

    // SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_CURRENT, D3DTA_DIFFUSE);

    SetColorOpDisable(2);

    return false;
}

// void obj_side_tex_gloss_t5(SVOSurface *vo, DWORD user_param, int )
//{
//    LPDIRECT3DTEXTURE9 coltex = (LPDIRECT3DTEXTURE9)user_param;
//
//    ASSERT_DX(g_D3DD->SetTexture(0, vo->m_TexGloss->Tex()));
//    ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->GetReflectionTexture()->Tex()));
//    ASSERT_DX(g_D3DD->SetTexture(2, vo->m_Tex->Tex()));
//    ASSERT_DX(g_D3DD->SetTexture(3, coltex));
//}

bool obj_side_gloss_t5(DWORD user_param, int) {
    obj_clear();

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, 0));

    SetAlphaOpSelect(0, D3DTA_CURRENT);
    SetColorOpSelect(0, D3DTA_TEXTURE);

    SetAlphaOpSelect(1, D3DTA_CURRENT);
    SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_TEMP));

    SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpSelect(2, D3DTA_TEXTURE);

    SetColorOp(3, D3DTOP_BLENDCURRENTALPHA, D3DTA_CURRENT, D3DTA_TEXTURE);
    SetAlphaOpDisable(3);

    SetColorOpAnyOrder(4, D3DTOP_ADD, D3DTA_TEMP, D3DTA_CURRENT);

    SetColorOpDisable(5);

    return false;
}

/////////

void CRenderPipeline::SetupTerrains(bool macro) {
    // + - done for this adapter
    // - - not present. used other stuff. visual quality loss
    // * - the same as previous adapter
    // ? - under develop
    //
    // sim tex cnt  |  2  |  3  |  4  |  5  |
    //______________|_____|_____|_____|_____|
    // type 0       |  +  |  *  |  *  |  *  |
    // type 1       |  +  |  *  |  *  |  *  |
    // type 2       |  +  |  *  |  *  |  *  |
    // type 3  (0g) |  +  |  +  |  *  |  *  |
    // type 4  (1g) |  +  |  +  |  *  |  *  |
    // type 5  (2g) |  +  |  *  |  ?  |  *  |
    // type 0m      |  +  |  *  |  *  |  *  |
    // type 1m      |  +  |  *  |  *  |  *  |
    // type 2m      |  +  |  +  |  *  |  *  |
    // type 3m (0g) |  -  |  +  |     |     |
    // type 4m (1g) |  -  |  +  |     |     |
    // type 5m (2g) |  -  |  +  |     |     |

    // if (macro)
    //{
    //    // type 0
    //    m_Ter[REND_TYPE0] = type01_macro;
    //    m_TerTex[REND_TYPE0] = textype0_macro;
    //    m_TerPass[REND_TYPE0] = 1;
    //    m_TerClear[REND_TYPE0] = empty_clear;

    //    // type1
    //    m_Ter[REND_TYPE1] = type01_macro;
    //    m_TerTex[REND_TYPE1] = textype1_macro;
    //    m_TerPass[REND_TYPE1] = 1;
    //    m_TerClear[REND_TYPE1] = empty_clear;

    //    if (g_D3DDCaps.MaxSimultaneousTextures >= 5)
    //    {
    //        // TO DO

    //        m_Ter[REND_TYPE0_GLOSS] = type34_macro_t3;
    //        m_TerTex[REND_TYPE0_GLOSS] = textype3_macro_t3;
    //        m_TerPass[REND_TYPE0_GLOSS] = 1;
    //        m_TerClear[REND_TYPE0_GLOSS] = type34_clear_macro_t3;

    //        m_Ter[REND_TYPE1_GLOSS] = type34_macro_t3;
    //        m_TerTex[REND_TYPE1_GLOSS] = textype4_macro_t3;
    //        m_TerPass[REND_TYPE1_GLOSS] = 1;
    //        m_TerClear[REND_TYPE1_GLOSS] = type34_clear_macro_t3;

    //        m_Ter[REND_TYPE2_GLOSS] = type5_macro_t5;
    //        m_TerTex[REND_TYPE2_GLOSS] = textype5_macro_t5;
    //        m_TerPass[REND_TYPE2_GLOSS] = 1;
    //        m_TerClear[REND_TYPE2_GLOSS] = type5_clear_macro_t5;

    //        m_Ter[REND_TYPE2] = type2_macro_t3;
    //        m_TerTex[REND_TYPE2] = textype2_macro_t3;
    //        m_TerPass[REND_TYPE2] = 1;
    //        m_TerClear[REND_TYPE2] = empty_clear;

    //    } else if (g_D3DDCaps.MaxSimultaneousTextures >= 3)
    //    {

    //        m_Ter[REND_TYPE0_GLOSS] = type34_macro_t3;
    //        m_TerTex[REND_TYPE0_GLOSS] = textype3_macro_t3;
    //        m_TerPass[REND_TYPE0_GLOSS] = 1;
    //        m_TerClear[REND_TYPE0_GLOSS] = type34_clear_macro_t3;

    //        m_Ter[REND_TYPE1_GLOSS] = type34_macro_t3;
    //        m_TerTex[REND_TYPE1_GLOSS] = textype4_macro_t3;
    //        m_TerPass[REND_TYPE1_GLOSS] = 1;
    //        m_TerClear[REND_TYPE1_GLOSS] = type34_clear_macro_t3;

    //        m_Ter[REND_TYPE2_GLOSS] = type5_macro_t3;
    //        m_TerTex[REND_TYPE2_GLOSS] = textype5_macro_t3;
    //        m_TerPass[REND_TYPE2_GLOSS] = 2;
    //        m_TerClear[REND_TYPE2_GLOSS] = type5_clear_macro_t3;

    //        m_Ter[REND_TYPE2] = type2_macro_t3;
    //        m_TerTex[REND_TYPE2] = textype2_macro_t3;
    //        m_TerPass[REND_TYPE2] = 1;
    //        m_TerClear[REND_TYPE2] = empty_clear;

    //    } else
    //    {

    //        // type 0
    //        m_Ter[REND_TYPE0_GLOSS] = type01_macro;
    //        m_TerTex[REND_TYPE0_GLOSS] = textype0_macro;
    //        m_TerPass[REND_TYPE0_GLOSS] = 1;
    //        m_TerClear[REND_TYPE0_GLOSS] = empty_clear;

    //        // type1
    //        m_Ter[REND_TYPE1_GLOSS] = type01_macro;
    //        m_TerTex[REND_TYPE1_GLOSS] = textype1_macro;
    //        m_TerPass[REND_TYPE1_GLOSS] = 1;
    //        m_TerClear[REND_TYPE1_GLOSS] = empty_clear;

    //        m_Ter[REND_TYPE2_GLOSS] = type2_macro_t2;
    //        m_TerTex[REND_TYPE2_GLOSS] = textype2_macro_t2;
    //        m_TerPass[REND_TYPE2_GLOSS] = 2;
    //        m_TerClear[REND_TYPE2_GLOSS] = type2_macro_clear_t2;

    //        // type 0

    //        // type 1

    //        m_Ter[REND_TYPE2] = type2_macro_t2;
    //        m_TerTex[REND_TYPE2] = textype2_macro_t2;
    //        m_TerPass[REND_TYPE2] = 2;
    //        m_TerClear[REND_TYPE2] = type2_macro_clear_t2;

    //    }
    //} else
    //{

    //    // type 0
    //    m_Ter[REND_TYPE0] = type01;
    //    m_TerTex[REND_TYPE0] = textype0;
    //    m_TerPass[REND_TYPE0] = 1;
    //    m_TerClear[REND_TYPE0] = empty_clear;

    //    // type1
    //    m_Ter[REND_TYPE1] = type01;
    //    m_TerTex[REND_TYPE1] = textype1;
    //    m_TerPass[REND_TYPE1] = 1;
    //    m_TerClear[REND_TYPE1] = empty_clear;

    //    // type 2
    //    m_Ter[REND_TYPE2] = type2;
    //    m_TerTex[REND_TYPE2] = textype2;
    //    m_TerPass[REND_TYPE2] = 1;
    //    m_TerClear[REND_TYPE2] = empty_clear;

    //    if (g_D3DDCaps.MaxSimultaneousTextures >= 4)    // may be need 2 change this to 5...
    //    {
    //        m_Ter[REND_TYPE0_GLOSS] = type34_t3;
    //        m_TerTex[REND_TYPE0_GLOSS] = textype3_t3;
    //        m_TerPass[REND_TYPE0_GLOSS] = 1;
    //        m_TerClear[REND_TYPE0_GLOSS] = type34_clear_t3;

    //        m_Ter[REND_TYPE1_GLOSS] = type34_t3;
    //        m_TerTex[REND_TYPE1_GLOSS] = textype4_t3;
    //        m_TerPass[REND_TYPE1_GLOSS] = 1;
    //        m_TerClear[REND_TYPE1_GLOSS] = type34_clear_t3;

    //        m_Ter[REND_TYPE2_GLOSS] = type5_t4;
    //        m_TerTex[REND_TYPE2_GLOSS] = textype5_t4;
    //        m_TerPass[REND_TYPE2_GLOSS] = 1;
    //        m_TerClear[REND_TYPE2_GLOSS] = type5_clear_t4;

    //    } else if (g_D3DDCaps.MaxSimultaneousTextures >= 3)
    //    {
    //        m_Ter[REND_TYPE0_GLOSS] = type34_t3;
    //        m_TerTex[REND_TYPE0_GLOSS] = textype3_t3;
    //        m_TerPass[REND_TYPE0_GLOSS] = 1;
    //        m_TerClear[REND_TYPE0_GLOSS] = type34_clear_t3;

    //        m_Ter[REND_TYPE1_GLOSS] = type34_t3;
    //        m_TerTex[REND_TYPE1_GLOSS] = textype4_t3;
    //        m_TerPass[REND_TYPE1_GLOSS] = 1;
    //        m_TerClear[REND_TYPE1_GLOSS] = type34_clear_t3;

    //        m_Ter[REND_TYPE2_GLOSS] = type5_t2;
    //        m_TerTex[REND_TYPE2_GLOSS] = textype5_t2;
    //        m_TerPass[REND_TYPE2_GLOSS] = 2;
    //        m_TerClear[REND_TYPE2_GLOSS] = type5_clear_t2;
    //    } else
    //    {
    //        m_Ter[REND_TYPE0_GLOSS] = type34_t2;
    //        m_TerTex[REND_TYPE0_GLOSS] = textype3_t2;
    //        m_TerPass[REND_TYPE0_GLOSS] = 2;
    //        m_TerClear[REND_TYPE0_GLOSS] = type34_clear_t2;

    //        m_Ter[REND_TYPE1_GLOSS] = type34_t2;
    //        m_TerTex[REND_TYPE1_GLOSS] = textype4_t2;
    //        m_TerPass[REND_TYPE1_GLOSS] = 2;
    //        m_TerClear[REND_TYPE1_GLOSS] = type34_clear_t2;

    //        m_Ter[REND_TYPE2_GLOSS] = type5_t2;
    //        m_TerTex[REND_TYPE2_GLOSS] = textype5_t2;
    //        m_TerPass[REND_TYPE2_GLOSS] = 2;
    //        m_TerClear[REND_TYPE2_GLOSS] = type5_clear_t2;
    //    }

    //}
}

CRenderPipeline::CRenderPipeline(void) {
    memset(this, 0, sizeof(CRenderPipeline));
    // check video card
    if (g_D3DDCaps.MaxSimultaneousTextures < 2
        // || (g_D3DDCaps.PrimitiveMiscCaps & D3DPMISCCAPS_TSSARGTEMP) == 0
    ) {
        ERROR_S(L"Sorry, your videocard not supported yet... contact with zakker@elementalgames.com");
    }

    // water

    m_WaterSolid = WaterSolid_t2;
    m_WaterClearSolid = WaterClearSolid_t2;
    m_WaterPassSolid = 1;

    if (g_D3DDCaps.MaxSimultaneousTextures >= 3) {
        m_WaterAlpha = WaterAlpha_t3;
        m_WaterClearAlpha = WaterClearAlpha_t3;
        m_WaterPassAlpha = 1;
    }
    else {
        D3DDISPLAYMODE d3ddm;
        FAILED_DX(g_D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm));
        if (d3ddm.Format == D3DFMT_X8R8G8B8) {
            m_WaterAlpha = WaterAlpha_t2_bpp32;
            m_WaterClearAlpha = WaterClearAlpha_t2_bpp32;
            m_WaterPassAlpha = 2;
        }
        else {
            m_WaterAlpha = WaterAlpha_t2_bpp16;
            m_WaterClearAlpha = WaterClearAlpha_t2_bpp16;
            m_WaterPassAlpha = 1;
            m_WaterSolid = WaterSolid_t2_bpp16;
        }
    }

    // landscape bottom geometry

    m_TerBotClear[0] = TerBotClear;
    m_TerBotTex[0] = TerBotTex;
    m_TerBot[0] = TerBot;
    m_TerBotPass[0] = 1;

    // if (g_D3DDCaps.MaxSimultaneousTextures >= 3)
    if (true) {
        m_TerBotTex[1] = TerBotTexM;
        m_TerBotClear[1] = TerBotClear;
        m_TerBot[1] = TerBotM;
        m_TerBotPass[1] = 1;
    }
    else {
        m_TerBotTex[1] = TerBotTexM_sux;
        m_TerBotClear[1] = TerBotClear_sux;
        m_TerBot[1] = TerBotM_sux;
        m_TerBotPass[1] = 2;
    }

    // landscape surfaces

    m_TerSurfClear[SURF_TYPE] = TerSurfClear;
    m_TerSurfTex[SURF_TYPE] = TerSurfTex;
    m_TerSurf[SURF_TYPE] = TerSurf;
    m_TerSurfPass[SURF_TYPE] = 1;

    m_TerSurfClear[SURF_TYPE_WHITE] = TerSurfClear;
    m_TerSurfTex[SURF_TYPE_WHITE] = TerSurfTex;
    m_TerSurf[SURF_TYPE_WHITE] = TerSurfW;
    m_TerSurfPass[SURF_TYPE_WHITE] = 1;

    if (g_D3DDCaps.MaxSimultaneousTextures >= 4) {
        m_TerSurfClear[SURF_TYPE_MACRO_WHITE] = TerSurfClear;
        m_TerSurfTex[SURF_TYPE_MACRO_WHITE] = TerSurfTexM;
        m_TerSurf[SURF_TYPE_MACRO_WHITE] = TerSurfMW;
        m_TerSurfPass[SURF_TYPE_MACRO_WHITE] = 1;

        m_TerSurfClear[SURF_TYPE_MACRO] = TerSurfClear;
        m_TerSurfTex[SURF_TYPE_MACRO] = TerSurfTexM;
        m_TerSurf[SURF_TYPE_MACRO] = TerSurfM;
        m_TerSurfPass[SURF_TYPE_MACRO] = 1;

        m_TerSurfClear[SURF_TYPE_GLOSS] = TerSurfClearGloss;
        m_TerSurfTex[SURF_TYPE_GLOSS] = TerSurfTexGloss;
        m_TerSurf[SURF_TYPE_GLOSS] = TerSurfGloss;
        m_TerSurfPass[SURF_TYPE_GLOSS] = 1;

        m_TerSurfClear[SURF_TYPE_GLOSS_WHITE] = TerSurfClearGloss;
        m_TerSurfTex[SURF_TYPE_GLOSS_WHITE] = TerSurfTexGloss;
        m_TerSurf[SURF_TYPE_GLOSS_WHITE] = TerSurfGlossW;
        m_TerSurfPass[SURF_TYPE_GLOSS_WHITE] = 1;

        m_TerSurfClear[SURF_TYPE_MACRO_GLOSS] = TerSurfClearGlossM;
        m_TerSurfTex[SURF_TYPE_MACRO_GLOSS] = TerSurfTexGlossM;
        m_TerSurf[SURF_TYPE_MACRO_GLOSS] = TerSurfGlossM;
        m_TerSurfPass[SURF_TYPE_MACRO_GLOSS] = 1;

        m_TerSurfClear[SURF_TYPE_MACRO_GLOSS_WHITE] = TerSurfClearGlossM;
        m_TerSurfTex[SURF_TYPE_MACRO_GLOSS_WHITE] = TerSurfTexGlossM;
        m_TerSurf[SURF_TYPE_MACRO_GLOSS_WHITE] = TerSurfGlossMW;
        m_TerSurfPass[SURF_TYPE_MACRO_GLOSS_WHITE] = 1;
    }
    else if (g_D3DDCaps.MaxSimultaneousTextures >= 3) {
        m_TerSurfClear[SURF_TYPE_MACRO_WHITE] = TerSurfClear;
        m_TerSurfTex[SURF_TYPE_MACRO_WHITE] = TerSurfTexM;
        m_TerSurf[SURF_TYPE_MACRO_WHITE] = TerSurfMW;
        m_TerSurfPass[SURF_TYPE_MACRO_WHITE] = 1;

        m_TerSurfClear[SURF_TYPE_MACRO] = TerSurfClear;
        m_TerSurfTex[SURF_TYPE_MACRO] = TerSurfTexM;
        m_TerSurf[SURF_TYPE_MACRO] = TerSurfM;
        m_TerSurfPass[SURF_TYPE_MACRO] = 1;

        m_TerSurfClear[SURF_TYPE_GLOSS] = TerSurfClearGloss2;
        m_TerSurfTex[SURF_TYPE_GLOSS] = TerSurfTexGloss2;
        m_TerSurf[SURF_TYPE_GLOSS] = TerSurfGloss2;
        m_TerSurfPass[SURF_TYPE_GLOSS] = 2;

        m_TerSurfClear[SURF_TYPE_GLOSS_WHITE] = TerSurfClearGloss2;
        m_TerSurfTex[SURF_TYPE_GLOSS_WHITE] = TerSurfTexGloss2;
        m_TerSurf[SURF_TYPE_GLOSS_WHITE] = TerSurfGloss2W;
        m_TerSurfPass[SURF_TYPE_GLOSS_WHITE] = 2;

        m_TerSurfClear[SURF_TYPE_MACRO_GLOSS] = TerSurfClear_sux;
        m_TerSurfTex[SURF_TYPE_MACRO_GLOSS] = TerSurfTexM_sux;
        m_TerSurf[SURF_TYPE_MACRO_GLOSS] = TerSurfM_sux;
        m_TerSurfPass[SURF_TYPE_MACRO_GLOSS] = 2;

        m_TerSurfClear[SURF_TYPE_MACRO_GLOSS_WHITE] = TerSurfClear_sux;
        m_TerSurfTex[SURF_TYPE_MACRO_GLOSS_WHITE] = TerSurfTexM_sux;
        m_TerSurf[SURF_TYPE_MACRO_GLOSS_WHITE] = TerSurfMW_sux;
        m_TerSurfPass[SURF_TYPE_MACRO_GLOSS_WHITE] = 2;
    }
    else {
        m_TerSurfClear[SURF_TYPE_MACRO_WHITE] = TerSurfClear_sux;
        m_TerSurfTex[SURF_TYPE_MACRO_WHITE] = TerSurfTexM_sux;
        m_TerSurf[SURF_TYPE_MACRO_WHITE] = TerSurfMW_sux;
        m_TerSurfPass[SURF_TYPE_MACRO_WHITE] = 2;

        m_TerSurfClear[SURF_TYPE_MACRO] = TerSurfClear_sux;
        m_TerSurfTex[SURF_TYPE_MACRO] = TerSurfTexM_sux;
        m_TerSurf[SURF_TYPE_MACRO] = TerSurfM_sux;
        m_TerSurfPass[SURF_TYPE_MACRO] = 2;

        m_TerSurfClear[SURF_TYPE_GLOSS] = TerSurfClearGloss2;
        m_TerSurfTex[SURF_TYPE_GLOSS] = TerSurfTexGloss2;
        m_TerSurf[SURF_TYPE_GLOSS] = TerSurfGloss2;
        m_TerSurfPass[SURF_TYPE_GLOSS] = 2;

        m_TerSurfClear[SURF_TYPE_GLOSS_WHITE] = TerSurfClearGloss2;
        m_TerSurfTex[SURF_TYPE_GLOSS_WHITE] = TerSurfTexGloss2;
        m_TerSurf[SURF_TYPE_GLOSS_WHITE] = TerSurfGloss2W;
        m_TerSurfPass[SURF_TYPE_GLOSS_WHITE] = 2;

        m_TerSurfClear[SURF_TYPE_MACRO_GLOSS] = TerSurfClear_sux;
        m_TerSurfTex[SURF_TYPE_MACRO_GLOSS] = TerSurfTexM_sux;
        m_TerSurf[SURF_TYPE_MACRO_GLOSS] = TerSurfM_sux;
        m_TerSurfPass[SURF_TYPE_MACRO_GLOSS] = 2;

        m_TerSurfClear[SURF_TYPE_MACRO_GLOSS_WHITE] = TerSurfClear_sux;
        m_TerSurfTex[SURF_TYPE_MACRO_GLOSS_WHITE] = TerSurfTexM_sux;
        m_TerSurf[SURF_TYPE_MACRO_GLOSS_WHITE] = TerSurfMW_sux;
        m_TerSurfPass[SURF_TYPE_MACRO_GLOSS_WHITE] = 2;
    }

    // objects

    // if (g_D3DDCaps.MaxSimultaneousTextures >= 5)
    //{
    //    //OBJ_RENDER_ORDINAL
    //    m_ObjStages[OBJ_RENDER_ORDINAL] = obj_ordinal_t4;
    //    m_ObjTex[OBJ_RENDER_ORDINAL] = obj_ordinal_tex_t4;
    //
    //    //OBJ_RENDER_ORDINAL_GLOSS
    //    m_ObjStages[OBJ_RENDER_ORDINAL_GLOSS] = obj_ordinal_gloss_t4;
    //    m_ObjTex[OBJ_RENDER_ORDINAL_GLOSS] = obj_ordinal_tex_gloss_t4;
    //
    //    //OBJ_RENDER_SIDE
    //    m_ObjStages[OBJ_RENDER_SIDE] = obj_side_t3;
    //    m_ObjTex[OBJ_RENDER_SIDE] = obj_side_tex_t2;
    //
    //    //OBJ_RENDER_SIDE_GLOSS
    //    m_ObjStages[OBJ_RENDER_SIDE_GLOSS] = obj_side_gloss_t5;
    //    m_ObjTex[OBJ_RENDER_SIDE_GLOSS] = obj_side_tex_gloss_t5;

    //} else if (g_D3DDCaps.MaxSimultaneousTextures >= 4)
    //{
    //    //OBJ_RENDER_ORDINAL
    //    m_ObjStages[OBJ_RENDER_ORDINAL] = obj_ordinal_t4;
    //    m_ObjTex[OBJ_RENDER_ORDINAL] = obj_ordinal_tex_t4;
    //
    //    //OBJ_RENDER_ORDINAL_GLOSS
    //    m_ObjStages[OBJ_RENDER_ORDINAL_GLOSS] = obj_ordinal_gloss_t4;
    //    m_ObjTex[OBJ_RENDER_ORDINAL_GLOSS] = obj_ordinal_tex_gloss_t4;
    //
    //    //OBJ_RENDER_SIDE
    //    m_ObjStages[OBJ_RENDER_SIDE] = obj_side_t3;
    //    m_ObjTex[OBJ_RENDER_SIDE] = obj_side_tex_t2;
    //
    //    //OBJ_RENDER_SIDE_GLOSS
    //    m_ObjStages[OBJ_RENDER_SIDE_GLOSS] = obj_side_gloss_t5;
    //    m_ObjTex[OBJ_RENDER_SIDE_GLOSS] = obj_side_tex_gloss_t5;
    //} else
    //{
    //    RESETFLAG(g_Flags, GFLAG_OBJGLOSS);

    //    //OBJ_RENDER_ORDINAL
    //    m_ObjStages[OBJ_RENDER_ORDINAL] = obj_ordinal_t2;
    //    m_ObjTex[OBJ_RENDER_ORDINAL] = obj_ordinal_tex_t2;
    //
    //    //OBJ_RENDER_ORDINAL_GLOSS
    //    m_ObjStages[OBJ_RENDER_ORDINAL_GLOSS] = obj_ordinal_t2;
    //    m_ObjTex[OBJ_RENDER_ORDINAL_GLOSS] = obj_ordinal_tex_t2;
    //
    //    //OBJ_RENDER_SIDE
    //    m_ObjStages[OBJ_RENDER_SIDE] = obj_side_t2;
    //    m_ObjTex[OBJ_RENDER_SIDE] = obj_side_tex_t2;
    //
    //    //OBJ_RENDER_SIDE_GLOSS
    //    m_ObjStages[OBJ_RENDER_SIDE_GLOSS] = obj_side_t2;
    //    m_ObjTex[OBJ_RENDER_SIDE_GLOSS] = obj_side_tex_t2;
    //}

    // m_ObjClear = obj_clear;

    //    // compile shaders
    //    DWORD dwFlags = 0;
    //#ifdef _DEBUG
    //    dwFlags |= D3DXSHADER_DEBUG;
    //#endif
    //
    //    ID3DXBuffer *pVS;
    //    ID3DXBuffer *pErr;
    //    if (D3D_OK !=  D3DXAssembleShader( cube_map_shader , sizeof(cube_map_shader) -1, NULL, NULL, dwFlags , &pVS ,
    //    &pErr ))
    //    {
    //        ERROR_S(std::wstring((const char*)pErr->GetBufferPointer()));
    //    }
    //
    //    DWORD *dd = (DWORD*)pVS->GetBufferPointer();
    //
    //    ASSERT_DX(g_D3DD->CreateVertexShader( (DWORD*)pVS->GetBufferPointer(), &m_CubeMapShader ));
    //    pVS->Release();

    // D3DVERTEXELEMENT9 vdis[] =
    //{
    //    {0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    //    {0, 12, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
    //    {0, 24, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    //    {0, 32, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},

    //    D3DDECL_END()
    //};
    // ASSERT_DX(g_D3DD->CreateVertexDeclaration(vdis, &m_VertexDecl));

    // ASSERT_DX(g_D3DD->CreateCubeTexture(g_MatrixMap->GetReflectionTexture()->GetSizeX(), 1, 0, D3DFMT_X8R8G8B8,
    // D3DPOOL_DEFAULT , &m_CubeTex, NULL)); m_CubeTex->
}

CRenderPipeline::~CRenderPipeline(void) {
    // m_CubeMapShader->Release();
    // m_VertexDecl->Release();
}
