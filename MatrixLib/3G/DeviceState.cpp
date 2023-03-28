// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "3g.hpp"

#include "DeviceState.hpp"

static D3DRENDERSTATETYPE IndexToRenderState[] = {D3DRS_ZENABLE,
                                                  D3DRS_FILLMODE,
                                                  D3DRS_SHADEMODE,
                                                  D3DRS_ZWRITEENABLE,
                                                  D3DRS_ALPHATESTENABLE,
                                                  D3DRS_LASTPIXEL,
                                                  D3DRS_SRCBLEND,
                                                  D3DRS_DESTBLEND,
                                                  D3DRS_CULLMODE,
                                                  D3DRS_ZFUNC,
                                                  D3DRS_ALPHAREF,
                                                  D3DRS_ALPHAFUNC,
                                                  D3DRS_DITHERENABLE,
                                                  D3DRS_ALPHABLENDENABLE,
                                                  D3DRS_FOGENABLE,
                                                  D3DRS_SPECULARENABLE,
                                                  D3DRS_FOGCOLOR,
                                                  D3DRS_FOGTABLEMODE,
                                                  D3DRS_FOGSTART,
                                                  D3DRS_FOGEND,
                                                  D3DRS_FOGDENSITY,
                                                  D3DRS_RANGEFOGENABLE,
                                                  D3DRS_STENCILENABLE,
                                                  D3DRS_STENCILFAIL,
                                                  D3DRS_STENCILZFAIL,
                                                  D3DRS_STENCILPASS,
                                                  D3DRS_STENCILFUNC,
                                                  D3DRS_STENCILREF,
                                                  D3DRS_STENCILMASK,
                                                  D3DRS_STENCILWRITEMASK,
                                                  D3DRS_TEXTUREFACTOR,
                                                  D3DRS_WRAP0,
                                                  D3DRS_WRAP1,
                                                  D3DRS_WRAP2,
                                                  D3DRS_WRAP3,
                                                  D3DRS_WRAP4,
                                                  D3DRS_WRAP5,
                                                  D3DRS_WRAP6,
                                                  D3DRS_WRAP7,
                                                  D3DRS_CLIPPING,
                                                  D3DRS_LIGHTING,
                                                  D3DRS_AMBIENT,
                                                  D3DRS_FOGVERTEXMODE,
                                                  D3DRS_COLORVERTEX,
                                                  D3DRS_LOCALVIEWER,
                                                  D3DRS_NORMALIZENORMALS,
                                                  D3DRS_DIFFUSEMATERIALSOURCE,
                                                  D3DRS_SPECULARMATERIALSOURCE,
                                                  D3DRS_AMBIENTMATERIALSOURCE,
                                                  D3DRS_EMISSIVEMATERIALSOURCE,
                                                  D3DRS_VERTEXBLEND,
                                                  D3DRS_CLIPPLANEENABLE,
                                                  D3DRS_POINTSIZE,
                                                  D3DRS_POINTSIZE_MIN,
                                                  D3DRS_POINTSPRITEENABLE,
                                                  D3DRS_POINTSCALEENABLE,
                                                  D3DRS_POINTSCALE_A,
                                                  D3DRS_POINTSCALE_B,
                                                  D3DRS_POINTSCALE_C,
                                                  D3DRS_MULTISAMPLEANTIALIAS,
                                                  D3DRS_MULTISAMPLEMASK,
                                                  D3DRS_PATCHEDGESTYLE,
                                                  D3DRS_DEBUGMONITORTOKEN,
                                                  D3DRS_POINTSIZE_MAX,
                                                  D3DRS_INDEXEDVERTEXBLENDENABLE,
                                                  D3DRS_COLORWRITEENABLE,
                                                  D3DRS_TWEENFACTOR,
                                                  D3DRS_BLENDOP,
                                                  D3DRS_POSITIONDEGREE,
                                                  D3DRS_NORMALDEGREE,
                                                  D3DRS_SCISSORTESTENABLE,
                                                  D3DRS_SLOPESCALEDEPTHBIAS,
                                                  D3DRS_ANTIALIASEDLINEENABLE,
                                                  D3DRS_MINTESSELLATIONLEVEL,
                                                  D3DRS_MAXTESSELLATIONLEVEL,
                                                  D3DRS_ADAPTIVETESS_X,
                                                  D3DRS_ADAPTIVETESS_Y,
                                                  D3DRS_ADAPTIVETESS_Z,
                                                  D3DRS_ADAPTIVETESS_W,
                                                  D3DRS_ENABLEADAPTIVETESSELLATION,
                                                  D3DRS_TWOSIDEDSTENCILMODE,
                                                  D3DRS_CCW_STENCILFAIL,
                                                  D3DRS_CCW_STENCILZFAIL,
                                                  D3DRS_CCW_STENCILPASS,
                                                  D3DRS_CCW_STENCILFUNC,
                                                  D3DRS_COLORWRITEENABLE1,
                                                  D3DRS_COLORWRITEENABLE2,
                                                  D3DRS_COLORWRITEENABLE3,
                                                  D3DRS_BLENDFACTOR,
                                                  D3DRS_SRGBWRITEENABLE,
                                                  D3DRS_DEPTHBIAS,
                                                  D3DRS_WRAP8,
                                                  D3DRS_WRAP9,
                                                  D3DRS_WRAP10,
                                                  D3DRS_WRAP11,
                                                  D3DRS_WRAP12,
                                                  D3DRS_WRAP13,
                                                  D3DRS_WRAP14,
                                                  D3DRS_WRAP15,
                                                  D3DRS_SEPARATEALPHABLENDENABLE,
                                                  D3DRS_SRCBLENDALPHA,
                                                  D3DRS_DESTBLENDALPHA,
                                                  D3DRS_BLENDOPALPHA};

static D3DSAMPLERSTATETYPE IndexToSamplerState[] = {
        D3DSAMP_ADDRESSU,    D3DSAMP_ADDRESSV,     D3DSAMP_ADDRESSW,      D3DSAMP_BORDERCOLOR, D3DSAMP_MAGFILTER,
        D3DSAMP_MINFILTER,   D3DSAMP_MIPFILTER,    D3DSAMP_MIPMAPLODBIAS, D3DSAMP_MAXMIPLEVEL, D3DSAMP_MAXANISOTROPY,
        D3DSAMP_SRGBTEXTURE, D3DSAMP_ELEMENTINDEX, D3DSAMP_DMAPOFFSET};

void CDeviceState::StoreState(DWORD states) {
    DTRACE();
    if (FLAG(states, DEVSTATE_RENDERSTATES)) {
        const int cnt = sizeof(IndexToRenderState) / sizeof(IndexToRenderState[0]);
        ASSERT(cnt == RS_COUNT);
        for (int i = 0; i < cnt; ++i) {
            m_Device->GetRenderState(IndexToRenderState[i], m_RenderStates + i);
        }
    }

    if (FLAG(states, DEVSTATE_SAMPLERSTATES)) {
        const int cnt = sizeof(IndexToSamplerState) / sizeof(IndexToSamplerState[0]);
        ASSERT(cnt == SS_COUNT);

        for (int l = 0; l < 8; ++l) {
            for (int i = 0; i < cnt; ++i) {
                m_Device->GetSamplerState(l, IndexToSamplerState[i], m_SamplerStates + (l * SS_COUNT + i));
            }
        }
    }

    if (FLAG(states, DEVSTATE_LIGHTS)) {
        memset(m_Lights, 0, sizeof(m_Lights));
        for (int i = 0; i < LI_COUNT; ++i) {
            m_Device->GetLight(i, m_Lights + i);
            m_Device->GetLightEnable(i, m_LightsEnabled + i);
        }
    }

    if (FLAG(states, DEVSTATE_MATERIAL)) {
        m_Device->GetMaterial(&m_Material);
    }
}
void CDeviceState::RestoreState(DWORD states) {
    if (FLAG(states, DEVSTATE_RENDERSTATES)) {
        const int cnt = sizeof(IndexToRenderState) / sizeof(IndexToRenderState[0]);
        ASSERT(cnt == RS_COUNT);
        for (int i = 0; i < cnt; ++i) {
            m_Device->SetRenderState(IndexToRenderState[i], m_RenderStates[i]);
        }
    }

    if (FLAG(states, DEVSTATE_SAMPLERSTATES)) {
        const int cnt = sizeof(IndexToSamplerState) / sizeof(IndexToSamplerState[0]);
        ASSERT(cnt == SS_COUNT);

        for (int l = 0; l < 8; ++l) {
            for (int i = 0; i < cnt; ++i) {
                m_Device->SetSamplerState(l, IndexToSamplerState[i], m_SamplerStates[l * SS_COUNT + i]);
            }
        }
    }
    if (FLAG(states, DEVSTATE_LIGHTS)) {
        for (int i = 0; i < LI_COUNT; ++i) {
            if (m_Lights[i].Type != 0) {
                m_Device->SetLight(i, m_Lights + i);
            }
            m_Device->LightEnable(i, m_LightsEnabled[i]);
        }
    }
    if (FLAG(states, DEVSTATE_MATERIAL)) {
        m_Device->SetMaterial(&m_Material);
    }
}
