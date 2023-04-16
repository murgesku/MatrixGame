// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixSkinManager.hpp"
#include "MatrixMap.hpp"
#include "MatrixObject.hpp"

#include "CFile.hpp"

PPSMatrixSkin CSkinManager::m_Skins[GSP_COUNT];
int CSkinManager::m_SkinsCount[GSP_COUNT];

static void SkinPreload(const SSkin *s) {
    const SMatrixSkin *ms = (const SMatrixSkin *)s;
    ms->m_Tex->Preload();
}

static void SkinPreloadGloss(const SSkin *s) {
    const SMatrixSkin *ms = (const SMatrixSkin *)s;
    ms->m_Tex->Preload();
    ms->m_TexGloss->Preload();
}

static void SkinPreloadGlossMask(const SSkin *s) {
    const SMatrixSkin *ms = (const SMatrixSkin *)s;
    ms->m_Tex->Preload();
    ms->m_TexGloss->Preload();
    ms->m_TexMask->Preload();
}

static void SkinPreloadGlossMaskBack(const SSkin *s) {
    const SMatrixSkin *ms = (const SMatrixSkin *)s;
    ms->m_Tex->Preload();
    ms->m_TexGloss->Preload();
    ms->m_TexMask->Preload();
    ms->m_TexBack->Preload();
}

static void SkinPreloadMask(const SSkin *s) {
    const SMatrixSkin *ms = (const SMatrixSkin *)s;
    ms->m_Tex->Preload();
    ms->m_TexMask->Preload();
}

static void SkinPreloadMaskBack(const SSkin *s) {
    const SMatrixSkin *ms = (const SMatrixSkin *)s;
    ms->m_Tex->Preload();
    ms->m_TexMask->Preload();
    ms->m_TexBack->Preload();
}

const SSkin *CSkinManager::GetSkin(const wchar *textures, DWORD gsp) {
    ParamParser t(textures), temp, temp_prev;

    ASSERT(gsp < GSP_COUNT);

    SMatrixSkin sk;

    sk.m_Tex = NULL;
    sk.m_TexGloss = NULL;
    sk.m_TexMask = NULL;
    sk.m_TexBack = NULL;
    sk.m_dtu = 0;
    sk.m_dtv = 0;
    sk.m_tu = 0;
    sk.m_tv = 0;

    bool gloss_off = false;

    DWORD gspp = gsp;

    int n = t.GetCountPar(L"*");
    for (int i = 0; i < n; ++i) {
        temp_prev = temp;
        temp = t.GetStrPar(i, L"*");
        if (temp.empty())
            continue;

        if (i == 0) {
            temp_prev = temp;
            sk.m_Tex = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, temp.c_str());

            sk.m_Tex->Preload();
            if ((sk.m_Tex->Flags() & TF_ALPHABLEND) == 0) {
                if (gsp == GSP_SIDE)
                    gsp = GSP_SIDE_NOALPHA;
            }
        }
        else if (i == 1 && g_Config.m_ObjTexturesGloss) {
            if (temp == L".") {
                gloss_off = true;
            }
            else {
                sk.m_TexGloss = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, temp.c_str());
            }
        }
        else if (i == 2)
            sk.m_TexBack = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, temp.c_str());
        else if (i == 3)
            sk.m_TexMask = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, temp.c_str());
        else if (i == 4) {
            sk.m_dtu = float(temp.GetStrPar(0, L",").GetDouble());
            sk.m_dtv = float(temp.GetStrPar(1, L",").GetDouble());
        }
    }

    if (!gloss_off && g_Config.m_ObjTexturesGloss && sk.m_TexGloss == NULL && gspp == GSP_SIDE) {
        temp_prev += GLOSS_TEXTURE_SUFFIX;
        std::wstring dummy;
        if (CFile::FileExist(dummy, temp_prev.c_str(), L"dds~png")) {
            sk.m_TexGloss = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, temp_prev.c_str());
        }
    }

    for (int i = 0; i < m_SkinsCount[gsp]; ++i) {
        if ((*(m_Skins[gsp][i])) == sk)
            return m_Skins[gsp][i];
    }

    ++m_SkinsCount[gsp];
    m_Skins[gsp] = (PPSMatrixSkin)HAllocEx(m_Skins[gsp], sizeof(PSMatrixSkin) * m_SkinsCount[gsp], g_MatrixHeap);

    SMatrixSkin *ms = (SMatrixSkin *)HAlloc(sizeof(SMatrixSkin), g_MatrixHeap);
    *ms = sk;
    m_Skins[gsp][m_SkinsCount[gsp] - 1] = ms;

    ms->Prepare((GSParam)gsp);

    if (ms->m_TexGloss) {
        if (ms->m_TexMask) {
            if (ms->m_TexBack) {
                ms->m_Preload = SkinPreloadGlossMaskBack;
            }
            else {
                ms->m_Preload = SkinPreloadGlossMask;
            }
        }
        else {
            ms->m_Preload = SkinPreloadGloss;
        }
    }
    else {
        if (ms->m_TexMask) {
            if (ms->m_TexBack) {
                ms->m_Preload = SkinPreloadMaskBack;
            }
            else {
                ms->m_Preload = SkinPreloadMaskBack;
            }
        }
        else {
            ms->m_Preload = SkinPreload;
        }
    }

    return ms;
}

///////////////////////////////////////

static void obj_shadow(const SSkin *sk) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;
    if (ms->m_Tex->Flags() & TF_ALPHATEST) {
        ASSERT_DX(g_D3DD->SetTexture(0, ms->m_Tex->Tex()));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));

        SetAlphaOpSelect(0, D3DTA_TEXTURE);
    }
    else {
        // ASSERT_DX(g_D3DD->SetTexture(0, NULL));
        SetAlphaOpSelect(0, D3DTA_TFACTOR);
        // ASSERT_DX(g_D3DD->SetTexture(0, ms->m_Tex->Tex()));
    }
}

static void obj_clear(const SSkin *, DWORD user_param) {
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
    ASSERT_DX(g_D3DD->SetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));

    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_CURRENT));

    g_D3DD->SetTransform(D3DTS_TEXTURE1, &g_MatrixMap->GetIdentityMatrix());
    g_D3DD->SetTransform(D3DTS_TEXTURE3, &g_MatrixMap->GetIdentityMatrix());

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, TRUE));
}

static void obj_clear3stage(const SSkin *, DWORD user_param) {
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));
    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 2));
    ASSERT_DX(g_D3DD->SetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));

    g_D3DD->SetTransform(D3DTS_TEXTURE3, &g_MatrixMap->GetIdentityMatrix());
}

static void obj_clear5stage(const SSkin *, DWORD user_param) {
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_CURRENT));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));
    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 2));
    ASSERT_DX(g_D3DD->SetTextureStageState(5, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));

    ASSERT_DX(g_D3DD->SetTexture(1, NULL));
    ASSERT_DX(g_D3DD->SetTexture(2, NULL));
    ASSERT_DX(g_D3DD->SetTexture(3, NULL));
    ASSERT_DX(g_D3DD->SetTexture(4, NULL));
    ASSERT_DX(g_D3DD->SetTexture(5, NULL));

    g_D3DD->SetTransform(D3DTS_TEXTURE5, &g_MatrixMap->GetIdentityMatrix());
}

static void obj_clear4stage(const SSkin *, DWORD user_param) {
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_CURRENT));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));
    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 2));
    ASSERT_DX(g_D3DD->SetTextureStageState(4, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));

    ASSERT_DX(g_D3DD->SetTexture(1, NULL));
    ASSERT_DX(g_D3DD->SetTexture(2, NULL));
    ASSERT_DX(g_D3DD->SetTexture(3, NULL));
    ASSERT_DX(g_D3DD->SetTexture(4, NULL));

    g_D3DD->SetTransform(D3DTS_TEXTURE4, &g_MatrixMap->GetIdentityMatrix());
}

static void obj_clear2stage(const SSkin *, DWORD user_param) {
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_RESULTARG, D3DTA_CURRENT));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1));
    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 2));
    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));

    g_D3DD->SetTransform(D3DTS_TEXTURE2, &g_MatrixMap->GetIdentityMatrix());
}

static void obj_side_tex6(const SSkin *sk, DWORD user_param, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;

    // gloss present

    ASSERT_DX(g_D3DD->SetTexture(0, ms->m_TexGloss->Tex()));
    ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->GetReflectionTexture()->Tex()));
    ASSERT_DX(g_D3DD->SetTexture(2, ms->m_Tex->Tex()));

    ASSERT_DX(g_D3DD->SetTexture(3, (LPDIRECT3DTEXTURE9)user_param));

    ASSERT_DX(g_D3DD->SetTexture(4, ms->m_TexMask->Tex()));
    if (ms->m_TexBack) {
        ASSERT_DX(g_D3DD->SetTexture(5, ms->m_TexBack->Tex()));
    }
    else {
        ASSERT_DX(g_D3DD->SetTexture(5, NULL));
    }
}

static void obj_side_tex6NA(const SSkin *sk, DWORD, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;

    // gloss present

    ASSERT_DX(g_D3DD->SetTexture(0, ms->m_TexGloss->Tex()));
    ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->GetReflectionTexture()->Tex()));
    ASSERT_DX(g_D3DD->SetTexture(2, ms->m_Tex->Tex()));

    ASSERT_DX(g_D3DD->SetTexture(3, ms->m_TexMask->Tex()));
    if (ms->m_TexBack) {
        ASSERT_DX(g_D3DD->SetTexture(4, ms->m_TexBack->Tex()));
    }
    else {
        ASSERT_DX(g_D3DD->SetTexture(4, NULL));
    }
}

static bool obj_side6(const SSkin *sk, DWORD user_param, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;

    // gloss present

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(4, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(5, D3DTSS_TEXCOORDINDEX, 0));

    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(4, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(4, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(5, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(5, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

    SetAlphaOpSelect(0, D3DTA_CURRENT);
    SetColorOpSelect(0, D3DTA_TEXTURE);

    SetAlphaOpSelect(1, D3DTA_CURRENT);
    SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_TEMP));

    SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpSelect(2, D3DTA_TEXTURE);

    SetColorOp(3, D3DTOP_BLENDCURRENTALPHA, D3DTA_CURRENT, D3DTA_TEXTURE);
    SetAlphaOpDisable(3);

    SetColorOpAnyOrder(4, D3DTOP_MODULATE, D3DTA_CURRENT, D3DTA_DIFFUSE);
    SetAlphaOpSelect(4, D3DTA_TEXTURE);

    // back
    SetAlphaOpSelect(5, D3DTA_CURRENT);
    SetColorOp(5, D3DTOP_BLENDCURRENTALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);

    SetColorOpAnyOrder(6, D3DTOP_ADD, D3DTA_TEMP, D3DTA_CURRENT);
    SetAlphaOpSelect(6, D3DTA_CURRENT);

    SetColorOpDisable(7);

    D3DXMATRIX mmm = g_MatrixMap->GetIdentityMatrix();
    mmm._31 = ms->m_tu;
    mmm._32 = ms->m_tv;

    g_D3DD->SetTransform(D3DTS_TEXTURE5, &mmm);

    ASSERT_DX(g_D3DD->SetTextureStageState(5, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));

    return false;
}

static bool obj_side6NA(const SSkin *sk, DWORD, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;

    // gloss present

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, 0));
    ASSERT_DX(g_D3DD->SetTextureStageState(4, D3DTSS_TEXCOORDINDEX, 0));

    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(4, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(4, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

    SetAlphaOpSelect(0, D3DTA_CURRENT);
    SetColorOpSelect(0, D3DTA_TEXTURE);

    SetAlphaOpSelect(1, D3DTA_CURRENT);
    SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_RESULTARG, D3DTA_TEMP));

    SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpSelect(2, D3DTA_CURRENT);

    SetColorOpAnyOrder(3, D3DTOP_MODULATE, D3DTA_CURRENT, D3DTA_DIFFUSE);
    SetAlphaOpSelect(3, D3DTA_TEXTURE);

    // back
    SetAlphaOpSelect(4, D3DTA_CURRENT);
    SetColorOp(4, D3DTOP_BLENDCURRENTALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);

    SetColorOpAnyOrder(5, D3DTOP_ADD, D3DTA_TEMP, D3DTA_CURRENT);

    SetColorOpDisable(6);

    D3DXMATRIX mmm = g_MatrixMap->GetIdentityMatrix();
    mmm._31 = ms->m_tu;
    mmm._32 = ms->m_tv;

    g_D3DD->SetTransform(D3DTS_TEXTURE4, &mmm);

    ASSERT_DX(g_D3DD->SetTextureStageState(4, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));

    return false;
}

static void obj_side_tex4(const SSkin *sk, DWORD user_param, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;

    if (pass == 0) {
        if (ms->m_TexGloss) {
            // gloss present
            LPDIRECT3DTEXTURE9 coltex = (LPDIRECT3DTEXTURE9)user_param;

            ASSERT_DX(g_D3DD->SetTexture(0, ms->m_TexGloss->Tex()));
            ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->GetReflectionTexture()->Tex()));
            ASSERT_DX(g_D3DD->SetTexture(2, ms->m_Tex->Tex()));

            // if (ms->m_TexBack)  ASSERT_DX(g_D3DD->SetTexture(2, ms->m_TexBack->Tex()));

            ASSERT_DX(g_D3DD->SetTexture(3, coltex));
        }
        else if (ms->m_TexMask) {
            LPDIRECT3DTEXTURE9 coltex = (LPDIRECT3DTEXTURE9)user_param;

            ASSERT_DX(g_D3DD->SetTexture(0, ms->m_Tex->Tex()));
            ASSERT_DX(g_D3DD->SetTexture(1, coltex));

            ASSERT_DX(g_D3DD->SetTexture(2, ms->m_TexMask->Tex()));
            if (ms->m_TexBack) {
                ASSERT_DX(g_D3DD->SetTexture(3, ms->m_TexBack->Tex()));
            }
            else {
                ASSERT_DX(g_D3DD->SetTexture(3, NULL));
            }
        }
        else {
            LPDIRECT3DTEXTURE9 coltex = (LPDIRECT3DTEXTURE9)user_param;

            ASSERT_DX(g_D3DD->SetTexture(0, ms->m_Tex->Tex()));
            ASSERT_DX(g_D3DD->SetTexture(1, coltex));
        }
    }
    else {
        // mask / back proceed
        ASSERT_DX(g_D3DD->SetTexture(0, ms->m_TexMask->Tex()));
        if (ms->m_TexBack) {
            ASSERT_DX(g_D3DD->SetTexture(1, ms->m_TexBack->Tex()));
        }
        else {
            ASSERT_DX(g_D3DD->SetTexture(1, NULL));
        }
    }
}

static bool obj_side4(const SSkin *sk, DWORD user_param, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;

    if (pass == 0) {
        if (ms->m_TexGloss) {
            // gloss present

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

            SetColorOpAnyOrder(4, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
            SetColorOpAnyOrder(5, D3DTOP_ADD, D3DTA_TEMP, D3DTA_CURRENT);

            SetColorOpDisable(6);

            return ms->m_TexMask != NULL;  // if mask present, do 2nd pass
        }
        else if (ms->m_TexMask) {
            ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

            ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));
            ASSERT_DX(g_D3DD->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, 0));

            SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
            SetAlphaOpSelect(0, D3DTA_TEXTURE);

            SetColorOp(1, D3DTOP_BLENDCURRENTALPHA, D3DTA_CURRENT, D3DTA_TEXTURE);
            SetAlphaOpSelect(1, D3DTA_CURRENT);

            SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_CURRENT, D3DTA_DIFFUSE);
            SetAlphaOpSelect(2, D3DTA_TEXTURE);

            SetAlphaOpSelect(3, D3DTA_CURRENT);
            SetColorOp(3, D3DTOP_BLENDCURRENTALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);

            ASSERT_DX(g_D3DD->SetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
            ASSERT_DX(g_D3DD->SetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

            D3DXMATRIX mmm = g_MatrixMap->GetIdentityMatrix();
            mmm._31 = ms->m_tu;
            mmm._32 = ms->m_tv;

            g_D3DD->SetTransform(D3DTS_TEXTURE3, &mmm);

            ASSERT_DX(g_D3DD->SetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));

            SetColorOpDisable(4);
        }
        else {
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
    }
    else {
        // mask / back proceed

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

        SetAlphaOpSelect(0, D3DTA_TEXTURE);
        SetColorOpSelect(0, D3DTA_CURRENT);

        SetAlphaOpSelect(1, D3DTA_CURRENT);
        SetColorOpSelect(1, D3DTA_TEXTURE);

        SetColorOpDisable(2);

        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

        D3DXMATRIX mmm = g_MatrixMap->GetIdentityMatrix();
        mmm._31 = ms->m_tu;
        mmm._32 = ms->m_tv;

        g_D3DD->SetTransform(D3DTS_TEXTURE1, &mmm);
    }

    return false;
}

static void obj_side_tex4NA(const SSkin *sk, DWORD, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;

    if (pass == 0) {
        if (ms->m_TexGloss) {
            ASSERT_DX(g_D3DD->SetTexture(0, ms->m_TexGloss->Tex()));
            ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->GetReflectionTexture()->Tex()));
            ASSERT_DX(g_D3DD->SetTexture(2, ms->m_Tex->Tex()));
        }
        else if (ms->m_TexMask) {
            ASSERT_DX(g_D3DD->SetTexture(0, ms->m_Tex->Tex()));

            ASSERT_DX(g_D3DD->SetTexture(1, ms->m_TexMask->Tex()));
            if (ms->m_TexBack) {
                ASSERT_DX(g_D3DD->SetTexture(2, ms->m_TexBack->Tex()));
            }
            else {
                ASSERT_DX(g_D3DD->SetTexture(2, NULL));
            }
        }
        else {
            ASSERT_DX(g_D3DD->SetTexture(0, ms->m_Tex->Tex()));
        }
    }
    else {
        // mask / back proceed
        ASSERT_DX(g_D3DD->SetTexture(0, ms->m_TexMask->Tex()));
        if (ms->m_TexBack) {
            ASSERT_DX(g_D3DD->SetTexture(1, ms->m_TexBack->Tex()));
        }
        else {
            ASSERT_DX(g_D3DD->SetTexture(1, NULL));
        }
    }
}

static bool obj_side4NA(const SSkin *sk, DWORD, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;

    if (pass == 0) {
        if (ms->m_TexGloss) {
            // gloss present

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
            SetAlphaOpDisable(2);

            SetColorOpAnyOrder(3, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
            SetColorOpAnyOrder(4, D3DTOP_ADD, D3DTA_TEMP, D3DTA_CURRENT);

            SetColorOpDisable(5);

            return ms->m_TexMask != NULL;  // if mask present, do 2nd pass
        }
        else if (ms->m_TexMask) {
            ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

            ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));
            ASSERT_DX(g_D3DD->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, 0));

            SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
            SetAlphaOpSelect(0, D3DTA_TEXTURE);

            SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_CURRENT, D3DTA_DIFFUSE);
            SetAlphaOpSelect(1, D3DTA_TEXTURE);

            SetAlphaOpSelect(2, D3DTA_CURRENT);
            SetColorOp(2, D3DTOP_BLENDCURRENTALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);

            ASSERT_DX(g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
            ASSERT_DX(g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

            D3DXMATRIX mmm = g_MatrixMap->GetIdentityMatrix();
            mmm._31 = ms->m_tu;
            mmm._32 = ms->m_tv;

            // g_MatrixMap->m_DI.T(L"tutv", std::wstring(ms->m_tu) + L"," + std::wstring(ms->m_tv));

            g_D3DD->SetTransform(D3DTS_TEXTURE2, &mmm);

            ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));

            SetColorOpDisable(3);
        }
        else {
            ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

            SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
            SetAlphaOpDisable(0);

            SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_CURRENT, D3DTA_DIFFUSE);
            SetColorOpDisable(2);

            return false;
        }
    }
    else {
        // mask / back proceed

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

        SetAlphaOpSelect(0, D3DTA_TEXTURE);
        SetColorOpSelect(0, D3DTA_CURRENT);

        SetAlphaOpSelect(1, D3DTA_CURRENT);
        SetColorOpSelect(1, D3DTA_TEXTURE);

        SetColorOpDisable(2);

        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

        D3DXMATRIX mmm = g_MatrixMap->GetIdentityMatrix();
        mmm._31 = ms->m_tu;
        mmm._32 = ms->m_tv;

        g_D3DD->SetTransform(D3DTS_TEXTURE1, &mmm);
    }

    return false;
}

static bool obj_side3(const SSkin *sk, DWORD user_param, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;
    if (pass == 0) {
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

        SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
        SetAlphaOpSelect(0, D3DTA_TEXTURE);

        SetColorOp(1, D3DTOP_BLENDCURRENTALPHA, D3DTA_CURRENT, D3DTA_TEXTURE);
        SetAlphaOpDisable(1);

        SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_CURRENT, D3DTA_DIFFUSE);
        SetColorOpDisable(3);

        return ms->m_TexGloss != NULL || ms->m_TexMask != NULL;
    }
    else if (pass == 1) {
        if (ms->m_TexGloss != NULL) {
            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, FALSE));

            ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

            SetAlphaOpDisable(0);

            SetColorOpSelect(0, D3DTA_TEXTURE);
            SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
            SetColorOpDisable(2);

            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE));
            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));

            return ms->m_TexMask != NULL;
        }
        else if (ms->m_TexMask != NULL) {
            goto mask;
        }
    }
    else {
    mask:
        // pass 2. back / mask
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

        SetAlphaOpSelect(0, D3DTA_TEXTURE);
        SetColorOpSelect(0, D3DTA_CURRENT);

        SetAlphaOpSelect(1, D3DTA_CURRENT);
        // SetAlphaOpDisable(1);
        SetColorOpSelect(1, D3DTA_TEXTURE);

        // SetColorOpSelect(1, D3DTA_DIFFUSE);

        SetColorOpDisable(2);

        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

        D3DXMATRIX mmm = g_MatrixMap->GetIdentityMatrix();
        mmm._31 = ms->m_tu;
        mmm._32 = ms->m_tv;

        g_D3DD->SetTransform(D3DTS_TEXTURE1, &mmm);
    }

    return false;
}

static bool obj_side3NA(const SSkin *sk, DWORD, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;
    if (pass == 0) {
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

        SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
        SetAlphaOpDisable(0);

        SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_CURRENT, D3DTA_DIFFUSE);
        SetColorOpDisable(2);

        return ms->m_TexGloss != NULL || ms->m_TexMask != NULL;
    }
    else if (pass == 1) {
        if (ms->m_TexGloss != NULL) {
            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, FALSE));

            ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

            SetAlphaOpDisable(0);

            SetColorOpSelect(0, D3DTA_TEXTURE);
            SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
            SetColorOpDisable(2);

            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE));
            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));

            return ms->m_TexMask != NULL;
        }
        else if (ms->m_TexMask != NULL) {
            goto mask;
        }
    }
    else {
    mask:
        // pass 2. back / mask
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

        SetAlphaOpSelect(0, D3DTA_TEXTURE);
        SetColorOpSelect(0, D3DTA_CURRENT);

        SetAlphaOpSelect(1, D3DTA_CURRENT);
        // SetAlphaOpDisable(1);
        SetColorOpSelect(1, D3DTA_TEXTURE);

        // SetColorOpSelect(1, D3DTA_DIFFUSE);

        SetColorOpDisable(2);

        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

        D3DXMATRIX mmm = g_MatrixMap->GetIdentityMatrix();
        mmm._31 = ms->m_tu;
        mmm._32 = ms->m_tv;

        g_D3DD->SetTransform(D3DTS_TEXTURE1, &mmm);
    }

    return false;
}

static void obj_side_tex2(const SSkin *sk, DWORD user_param, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;
    if (pass == 0) {
        LPDIRECT3DTEXTURE9 coltex = (LPDIRECT3DTEXTURE9)user_param;

        ASSERT_DX(g_D3DD->SetTexture(0, ms->m_Tex->Tex()));
        ASSERT_DX(g_D3DD->SetTexture(1, coltex));
    }
    else if (pass == 1) {
        if (ms->m_TexGloss != NULL) {
            ASSERT_DX(g_D3DD->SetTexture(0, ms->m_TexGloss->Tex()));
            ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->GetReflectionTexture()->Tex()));
        }
        else if (ms->m_TexMask != NULL) {
            goto mask;
        }
    }
    else {
    mask:
        // mask / back proceed
        ASSERT_DX(g_D3DD->SetTexture(0, ms->m_TexMask->Tex()));
        if (ms->m_TexBack) {
            ASSERT_DX(g_D3DD->SetTexture(1, ms->m_TexBack->Tex()));
        }
        else {
            ASSERT_DX(g_D3DD->SetTexture(1, NULL));
        }
    }
}

static void obj_side_tex2NA(const SSkin *sk, DWORD, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;
    if (pass == 0) {
        ASSERT_DX(g_D3DD->SetTexture(0, ms->m_Tex->Tex()));
    }
    else if (pass == 1) {
        if (ms->m_TexGloss != NULL) {
            ASSERT_DX(g_D3DD->SetTexture(0, ms->m_TexGloss->Tex()));
            ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->GetReflectionTexture()->Tex()));
        }
        else if (ms->m_TexMask != NULL) {
            goto mask;
        }
    }
    else {
    mask:
        // mask / back proceed
        ASSERT_DX(g_D3DD->SetTexture(0, ms->m_TexMask->Tex()));
        if (ms->m_TexBack) {
            ASSERT_DX(g_D3DD->SetTexture(1, ms->m_TexBack->Tex()));
        }
        else {
            ASSERT_DX(g_D3DD->SetTexture(1, NULL));
        }
    }
}

static bool obj_side2(const SSkin *sk, DWORD user_param, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;
    if (pass == 0) {
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

        SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
        SetAlphaOpSelect(0, D3DTA_TEXTURE);

        SetColorOp(1, D3DTOP_BLENDCURRENTALPHA, D3DTA_CURRENT, D3DTA_TEXTURE);
        SetAlphaOpDisable(1);

        SetColorOpDisable(2);

        return ms->m_TexGloss != NULL || ms->m_TexMask != NULL;
    }
    else if (pass == 1) {
        if (ms->m_TexGloss != NULL) {
            ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

            SetAlphaOpDisable(0);

            SetColorOpSelect(0, D3DTA_TEXTURE);
            SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
            SetColorOpDisable(2);

            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE));
            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));

            return ms->m_TexMask != NULL;
        }
        else if (ms->m_TexMask != NULL) {
            goto mask;
        }
    }
    else {
    mask:
        // pass 2. back / mask
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

        SetAlphaOpSelect(0, D3DTA_TEXTURE);
        SetColorOpSelect(0, D3DTA_CURRENT);

        SetAlphaOpSelect(1, D3DTA_CURRENT);
        // SetAlphaOpDisable(1);
        SetColorOpSelect(1, D3DTA_TEXTURE);

        // SetColorOpSelect(1, D3DTA_DIFFUSE);

        SetColorOpDisable(2);

        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

        D3DXMATRIX mmm = g_MatrixMap->GetIdentityMatrix();
        mmm._31 = ms->m_tu;
        mmm._32 = ms->m_tv;

        g_D3DD->SetTransform(D3DTS_TEXTURE1, &mmm);
    }

    return false;
}

static bool obj_side2NA(const SSkin *sk, DWORD, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;
    if (pass == 0) {
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

        SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
        SetAlphaOpDisable(0);

        SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_CURRENT, D3DTA_DIFFUSE);

        SetColorOpDisable(2);

        return ms->m_TexGloss != NULL || ms->m_TexMask != NULL;
    }
    else if (pass == 1) {
        if (ms->m_TexGloss != NULL) {
            ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

            SetAlphaOpDisable(0);

            SetColorOpSelect(0, D3DTA_TEXTURE);
            SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
            SetColorOpDisable(2);

            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE));
            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));

            return ms->m_TexMask != NULL;
        }
        else if (ms->m_TexMask != NULL) {
            goto mask;
        }
    }
    else {
    mask:
        // pass 2. back / mask
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

        SetAlphaOpSelect(0, D3DTA_TEXTURE);
        SetColorOpSelect(0, D3DTA_CURRENT);

        SetAlphaOpSelect(1, D3DTA_CURRENT);
        // SetAlphaOpDisable(1);
        SetColorOpSelect(1, D3DTA_TEXTURE);

        // SetColorOpSelect(1, D3DTA_DIFFUSE);

        SetColorOpDisable(2);

        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

        ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
        ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

        D3DXMATRIX mmm = g_MatrixMap->GetIdentityMatrix();
        mmm._31 = ms->m_tu;
        mmm._32 = ms->m_tv;

        g_D3DD->SetTransform(D3DTS_TEXTURE1, &mmm);
    }

    return false;
}

static void obj_ord_tex4(const SSkin *sk, DWORD user_param, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;

    CTextureManaged *basetex;

    if (user_param) {
        CMatrixMapObject *o = (CMatrixMapObject *)user_param;
        if (o->m_BurnSkinVis == 255) {
            basetex = o->m_BurnSkin->m_Tex;
            goto burned;
        }

        if (pass == 0) {
            DWORD tf = (o->GetTerrainColor() & 0x00FFFFFF) | (o->m_BurnSkinVis << 24);
            g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, tf);

            ASSERT_DX(g_D3DD->SetTexture(0, ms->m_Tex->Tex()));
            ASSERT_DX(g_D3DD->SetTexture(1, o->m_BurnSkin->m_Tex->Tex()));
            if (ms->m_Tex->Flags() & TF_ALPHATEST) {
                ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
            }
        }
        else {
            // always 2nd pass - gloss!!!!
            ASSERT_DX(g_D3DD->SetTexture(0, ms->m_TexGloss->Tex()));
            ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->GetReflectionTexture()->Tex()));
        }
    }
    else {
        basetex = ms->m_Tex;

    burned:;

        if (pass == 0) {
            if (ms->m_TexGloss) {
                ASSERT_DX(g_D3DD->SetTexture(0, ms->m_TexGloss->Tex()));
                ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->GetReflectionTexture()->Tex()));
                ASSERT_DX(g_D3DD->SetTexture(2, basetex->Tex()));

                if (basetex->Flags() & TF_ALPHATEST) {
                    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
                }
            }
            else {
                ASSERT_DX(g_D3DD->SetTexture(0, basetex->Tex()));
                if (ms->m_TexMask != NULL) {
                    ASSERT_DX(g_D3DD->SetTexture(1, ms->m_TexMask->Tex()));
                    if (ms->m_TexBack) {
                        ASSERT_DX(g_D3DD->SetTexture(2, ms->m_TexBack->Tex()));
                    }
                    else {
                        ASSERT_DX(g_D3DD->SetTexture(2, NULL));
                    }
                }
                if (ms->m_Tex->Flags() & TF_ALPHATEST) {
                    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
                }
            }
        }
        else if (pass == 1) {
            // mask / back proceed
            ASSERT_DX(g_D3DD->SetTexture(0, ms->m_TexMask->Tex()));
            if (ms->m_TexBack) {
                ASSERT_DX(g_D3DD->SetTexture(1, ms->m_TexBack->Tex()));
            }
            else {
                ASSERT_DX(g_D3DD->SetTexture(1, NULL));
            }
        }
        else {
        }
    }
}

static bool obj_ord4(const SSkin *sk, DWORD user_param, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;
    if (user_param) {
        CMatrixMapObject *o = (CMatrixMapObject *)user_param;
        if (o->m_BurnSkinVis == 255)
            goto burned;

        if (pass == 0) {
            ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

            SetColorOpSelect(0, D3DTA_TEXTURE);
            SetAlphaOpSelect(0, D3DTA_TEXTURE);

            SetColorOp(1, D3DTOP_BLENDFACTORALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
            SetAlphaOp(1, D3DTOP_BLENDFACTORALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);

            SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
            SetAlphaOpSelect(2, D3DTA_CURRENT);

            SetColorOpAnyOrder(3, D3DTOP_MODULATE, D3DTA_TFACTOR, D3DTA_CURRENT);
            SetAlphaOpSelect(3, D3DTA_CURRENT);

            SetColorOpDisable(4);

            return ms->m_TexGloss != NULL;
        }
        else if (pass == 1) {
            ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

            ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));

            SetAlphaOpSelect(0, D3DTA_CURRENT);
            SetColorOpSelect(0, D3DTA_TEXTURE);
            SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);

            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE));
            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));

            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

            return false;
        }
    }
    else {
    burned:;
        if (pass == 0) {
            if (ms->m_TexGloss) {
                ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

                ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
                ASSERT_DX(
                        g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

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

                return ms->m_TexMask != NULL;
            }
            else {
                // ordinal mapping
                ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

                SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
                // SetColorOpSelect(0, D3DTA_TEXTURE);
                // SetColorOpSelect(0, D3DTA_DIFFUSE);
                SetAlphaOpSelect(0, D3DTA_TEXTURE);

                SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TFACTOR, D3DTA_CURRENT);

                if (ms->m_TexMask == NULL) {
                    SetAlphaOpSelect(1, D3DTA_CURRENT);
                    SetColorOpDisable(2);

                    // bla
                    // SetColorOpSelect(0, D3DTA_TEXTURE);
                    // SetColorOpDisable(1);
                    // SetAlphaOpDisable(0);
                }
                else {
                    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_RESULTARG, D3DTA_TEMP));
                    SetColorOpSelect(1, D3DTA_TEMP);

                    SetAlphaOpSelect(1, D3DTA_TEXTURE);
                    SetAlphaOpSelect(2, D3DTA_TEMP);
                    SetColorOp(2, D3DTOP_BLENDCURRENTALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);

                    ASSERT_DX(g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
                    ASSERT_DX(g_D3DD->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

                    D3DXMATRIX mmm = g_MatrixMap->GetIdentityMatrix();
                    mmm._31 = ms->m_tu;
                    mmm._32 = ms->m_tv;

                    g_D3DD->SetTransform(D3DTS_TEXTURE2, &mmm);

                    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
                    ASSERT_DX(g_D3DD->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 0));
                    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

                    SetColorOpDisable(3);
                }
                return false;
            }
        }
        if (pass == 1) {
            // pass 2. back / mask
            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
            ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

            SetAlphaOpSelect(0, D3DTA_TEXTURE);
            SetColorOpSelect(0, D3DTA_CURRENT);

            SetAlphaOpSelect(1, D3DTA_CURRENT);
            // SetAlphaOpDisable(1);
            SetColorOpSelect(1, D3DTA_TEXTURE);

            // SetColorOpSelect(1, D3DTA_DIFFUSE);

            SetColorOpDisable(2);

            ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
            ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
            ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
            ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

            ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

            D3DXMATRIX mmm = g_MatrixMap->GetIdentityMatrix();
            mmm._31 = ms->m_tu;
            mmm._32 = ms->m_tv;

            g_D3DD->SetTransform(D3DTS_TEXTURE1, &mmm);
        }
    }

    return false;
}

static void obj_ord_tex2(const SSkin *sk, DWORD user_param, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;
    CTextureManaged *basetex;
    if (user_param) {
        CMatrixMapObject *o = (CMatrixMapObject *)user_param;
        if (pass == 0) {
            if (o->m_BurnSkinVis == 255) {
                basetex = o->m_BurnSkin->m_Tex;
                goto burned;
            }

            DWORD tf = (o->GetTerrainColor() & 0x00FFFFFF) | (o->m_BurnSkinVis << 24);
            g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, tf);

            ASSERT_DX(g_D3DD->SetTexture(0, ms->m_Tex->Tex()));
            ASSERT_DX(g_D3DD->SetTexture(1, o->m_BurnSkin->m_Tex->Tex()));
            if (ms->m_Tex->Flags() & TF_ALPHATEST) {
                ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
            }
        }
        else {
            goto nextpass;
        }
    }
    else {
        if (pass == 0) {
            basetex = ms->m_Tex;
        burned:;

            ASSERT_DX(g_D3DD->SetTexture(0, basetex->Tex()));
            if (ms->m_Tex->Flags() & TF_ALPHATEST) {
                ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
            }
        }
        else {
        nextpass:;
            if (pass == 1) {
                if (ms->m_TexGloss != NULL) {
                    ASSERT_DX(g_D3DD->SetTexture(0, ms->m_TexGloss->Tex()));
                    ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->GetReflectionTexture()->Tex()));
                }
                else if (ms->m_TexMask != NULL) {
                    goto mask;
                }
            }
            else {
            mask:
                // mask / back proceed
                ASSERT_DX(g_D3DD->SetTexture(0, ms->m_TexMask->Tex()));
                if (ms->m_TexBack) {
                    ASSERT_DX(g_D3DD->SetTexture(1, ms->m_TexBack->Tex()));
                }
                else {
                    ASSERT_DX(g_D3DD->SetTexture(1, NULL));
                }
            }
        }
    }
}

static bool obj_ord2(const SSkin *sk, DWORD user_param, int pass) {
    SMatrixSkin *ms = (SMatrixSkin *)sk;
    if (user_param) {
        CMatrixMapObject *o = (CMatrixMapObject *)user_param;
        if (o->m_BurnSkinVis == 255)
            goto burned;

        if (pass == 0) {
            ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
            ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

            SetColorOpSelect(0, D3DTA_TEXTURE);
            SetAlphaOpSelect(0, D3DTA_TEXTURE);

            SetColorOp(1, D3DTOP_BLENDFACTORALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);
            SetAlphaOp(1, D3DTOP_BLENDFACTORALPHA, D3DTA_TEXTURE, D3DTA_CURRENT);

            SetColorOpAnyOrder(2, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_CURRENT);
            SetAlphaOpSelect(2, D3DTA_CURRENT);

            // SetColorOpAnyOrder(3, D3DTOP_MODULATE, D3DTA_TFACTOR, D3DTA_CURRENT);
            // SetAlphaOpSelect(3, D3DTA_CURRENT);

            SetColorOpDisable(3);

            return ms->m_TexGloss != NULL;
        }
        else {
            goto nextpass;
        }
    }
    else {
    burned:;
        if (pass == 0) {
            // ordinal mapping
            ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

            SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
            SetAlphaOpSelect(0, D3DTA_TEXTURE);

            SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TFACTOR, D3DTA_CURRENT);
            SetAlphaOpSelect(1, D3DTA_CURRENT);

            SetColorOpDisable(2);

            return ms->m_TexGloss != NULL || ms->m_TexMask != NULL;
        }
        else {
        nextpass:;
            if (pass == 1) {
                if (ms->m_TexGloss != NULL) {
                    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

                    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
                    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX,
                                                           D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

                    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
                    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

                    SetAlphaOpDisable(0);

                    SetColorOpSelect(0, D3DTA_TEXTURE);
                    SetColorOpAnyOrder(1, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_CURRENT);
                    SetColorOpDisable(2);

                    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE));
                    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));

                    return ms->m_TexMask != NULL;
                }
                else if (ms->m_TexMask != NULL) {
                    goto mask;
                }
            }
            else {
            mask:
                // pass 2. back / mask
                ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
                ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

                SetAlphaOpSelect(0, D3DTA_TEXTURE);
                SetColorOpSelect(0, D3DTA_CURRENT);

                SetAlphaOpSelect(1, D3DTA_CURRENT);
                // SetAlphaOpDisable(1);
                SetColorOpSelect(1, D3DTA_TEXTURE);

                // SetColorOpSelect(1, D3DTA_DIFFUSE);

                SetColorOpDisable(2);

                ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
                ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
                ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
                ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

                ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
                ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
                ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

                D3DXMATRIX mmm = g_MatrixMap->GetIdentityMatrix();
                mmm._31 = ms->m_tu;
                mmm._32 = ms->m_tv;

                g_D3DD->SetTransform(D3DTS_TEXTURE1, &mmm);
            }
        }
    }

    return false;
}

void SMatrixSkin::Prepare6Side(void) {
    if (m_TexGloss && m_TexMask)
    // if (0)
    {
        m_SetupStages = obj_side6;
        m_SetupTex = obj_side_tex6;
        m_SetupClear = obj_clear5stage;
    }
    else {
        Prepare4Side();
    }
}

void SMatrixSkin::Prepare6SideNA(void) {
    if (m_TexGloss && m_TexMask)
    // if (0)
    {
        m_SetupStages = obj_side6NA;
        m_SetupTex = obj_side_tex6NA;
        m_SetupClear = obj_clear4stage;
    }
    else {
        Prepare4SideNA();
    }
}

void SMatrixSkin::Prepare6Ordinal(void) {
    m_SetupStages = obj_ord4;
    m_SetupTex = obj_ord_tex4;
    if (m_TexGloss == NULL && m_TexMask != NULL) {
        m_SetupClear = obj_clear2stage;
    }
}

void SMatrixSkin::Prepare4Side(void) {
    m_SetupStages = obj_side4;
    m_SetupTex = obj_side_tex4;

    if (m_TexGloss == NULL && m_TexMask != NULL) {
        m_SetupClear = obj_clear3stage;
    }
}

void SMatrixSkin::Prepare4SideNA(void) {
    m_SetupStages = obj_side4NA;
    m_SetupTex = obj_side_tex4NA;

    if (m_TexGloss == NULL && m_TexMask != NULL) {
        m_SetupClear = obj_clear2stage;
    }
}

void SMatrixSkin::Prepare4Ordinal(void) {
    m_SetupStages = obj_ord4;
    m_SetupTex = obj_ord_tex4;

    if (m_TexGloss == NULL && m_TexMask != NULL) {
        m_SetupClear = obj_clear2stage;
    }
}

void SMatrixSkin::Prepare3Side(void) {
    m_SetupStages = obj_side3;
    m_SetupTex = obj_side_tex2;
}

void SMatrixSkin::Prepare3SideNA(void) {
    m_SetupStages = obj_side3NA;
    m_SetupTex = obj_side_tex2NA;
}

void SMatrixSkin::Prepare3Ordinal(void) {
    m_SetupStages = obj_ord2;
    m_SetupTex = obj_ord_tex2;
}

void SMatrixSkin::Prepare2Side(void) {
    m_SetupStages = obj_side2;
    m_SetupTex = obj_side_tex2;
}

void SMatrixSkin::Prepare2SideNA(void) {
    m_SetupStages = obj_side2NA;
    m_SetupTex = obj_side_tex2NA;
}

void SMatrixSkin::Prepare2Ordinal(void) {
    m_SetupStages = obj_ord2;
    m_SetupTex = obj_ord_tex2;
}

void SMatrixSkin::Prepare(GSParam gsp) {
    m_SetupClear = obj_clear;
    m_SetupTexShadow = obj_shadow;

    if (g_D3DDCaps.MaxSimultaneousTextures >= 6) {
        if (gsp == GSP_SIDE) {
            Prepare6Side();
        }
        else if (gsp == GSP_SIDE_NOALPHA) {
            Prepare6SideNA();
        }
        else {
            Prepare6Ordinal();
        }
    }
    else if (g_D3DDCaps.MaxSimultaneousTextures >= 4) {
        if (gsp == GSP_SIDE) {
            Prepare4Side();
        }
        else if (gsp == GSP_SIDE_NOALPHA) {
            Prepare4SideNA();
        }
        else {
            Prepare4Ordinal();
        }
    }
    else if (g_D3DDCaps.MaxSimultaneousTextures >= 3) {
        if (gsp == GSP_SIDE) {
            Prepare3Side();
        }
        else if (gsp == GSP_SIDE_NOALPHA) {
            Prepare3SideNA();
        }
        else {
            Prepare3Ordinal();
        }
    }
    else if (g_D3DDCaps.MaxSimultaneousTextures >= 2) {
        if (gsp == GSP_SIDE) {
            Prepare2Side();
        }
        else if (gsp == GSP_SIDE_NOALPHA) {
            Prepare2SideNA();
        }
        else {
            Prepare2Ordinal();
        }
    }
    else {
        ERROR_S(L"Sorry, your video card is not supported.");
    }
}

void CSkinManager::Clear(void) {
    for (int gsp = 0; gsp < GSP_COUNT; ++gsp) {
        for (int i = 0; i < m_SkinsCount[gsp]; ++i) {
            HFree(m_Skins[gsp][i], g_MatrixHeap);
        }
        if (m_Skins[gsp])
            HFree(m_Skins[gsp], g_MatrixHeap);
    }
}

void CSkinManager::Takt(float cms) {
    for (int gsp = 0; gsp < GSP_COUNT; ++gsp) {
        for (int i = 0; i < m_SkinsCount[gsp]; ++i) {
            SMatrixSkin *ms = m_Skins[gsp][i];
            ms->m_tu += cms * ms->m_dtu;
            ms->m_tv += cms * ms->m_dtv;
            if (ms->m_tu > 1)
                ms->m_tu -= 1;
            if (ms->m_tv > 1)
                ms->m_tv -= 1;
            if (ms->m_tu < -1)
                ms->m_tu += 1;
            if (ms->m_tv < -1)
                ms->m_tv += 1;
        }
    }
}