// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <algorithm>

#include "MatrixMap.hpp"
#include "MatrixObject.hpp"
#include "MatrixObjectBuilding.hpp"
#include "MatrixRenderPipeline.hpp"
#include "MatrixFlyer.hpp"
#include "MatrixRobot.hpp"
#include "MatrixObjectCannon.hpp"
#include "MatrixTerSurface.hpp"
#include "MatrixSampleStateManager.hpp"
#include "MatrixGamePathUtils.hpp"

#include <utils.hpp>

#define MINIMAP_Z 0.0000f

#define MINIMAP_CAM_COLOR 0xFF30FFFF

void CMinimap::SMMTex::Load(CBlockPar *mm, const wchar *name) {
    DTRACE();
    if (tex == NULL) {
        const auto& n = mm->ParGet(name);
        tex = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, n.GetStrPar(0, L",").c_str());
        tex->MipmapOff();
        tex->Preload();

        int x = n.GetStrPar(1, L",").GetInt();
        int y = n.GetStrPar(2, L",").GetInt();

        u0 = float(x) / float(tex->GetSizeX());
        v0 = float(y) / float(tex->GetSizeY());

        u1 = float(x + n.GetStrPar(3, L",").GetInt()) / float(tex->GetSizeX());
        v1 = float(y + n.GetStrPar(4, L",").GetInt()) / float(tex->GetSizeY());
    }
}

void CMinimap::Init(void) {
    DTRACE();
    if (m_Texture == NULL) {
        m_Texture = CACHE_CREATE_TEXTURE();
    }

    CBlockPar *bp = g_MatrixData->BlockGet(PAR_SOURCE_MINIMAP);

    m_Tex[MMT_POINT].Load(bp, PAR_SOURCE_MINIMAP_POINT);
    m_Tex[MMT_ARROW].Load(bp, PAR_SOURCE_MINIMAP_ARROW);

    m_Tex[MMT_FLYER].Load(bp, PAR_SOURCE_MINIMAP_FLYER);
    m_Tex[MMT_ROBOT].Load(bp, PAR_SOURCE_MINIMAP_ROBOT);
    m_Tex[MMT_TURRET].Load(bp, PAR_SOURCE_MINIMAP_TURRET);
    m_Tex[MMT_BASE].Load(bp, PAR_SOURCE_MINIMAP_BASE);
    m_Tex[MMT_FACTORY].Load(bp, PAR_SOURCE_MINIMAP_FACTORY);

    m_Tex[MMT_FLYER_R].Load(bp, PAR_SOURCE_MINIMAP_FLYER_R);
    m_Tex[MMT_ROBOT_R].Load(bp, PAR_SOURCE_MINIMAP_ROBOT_R);
    m_Tex[MMT_TURRET_R].Load(bp, PAR_SOURCE_MINIMAP_TURRET_R);
    m_Tex[MMT_BASE_R].Load(bp, PAR_SOURCE_MINIMAP_BASE_R);
    m_Tex[MMT_FACTORY_R].Load(bp, PAR_SOURCE_MINIMAP_FACTORY_R);

    m_EventsCnt = 0;
    m_Dirty = 1;

#ifdef MINIMAP_SUPPORT_ROTATION
    SetAngle(-g_MatrixMap->m_CameraAngle);
    // g_MatrixMap->m_Minimap.SetAngle();
#endif
}

void CMinimap::Clear(void) {
    DTRACE();
    if (m_Texture) {
        CCache::Destroy(m_Texture);
        m_Texture = NULL;
    }
    if (m_TextureStore) {
        m_TextureStore->Release();
        m_TextureStore = NULL;
    }
}

void CMinimap::StoreTexture(void) {
    DTRACE();
    if (m_Texture == NULL || m_TextureStore != NULL)
        return;

    IDirect3DSurface9 *surf_to, *surf_from;
    if (D3D_OK != g_D3DD->CreateTexture(MINIMAP_SIZE, MINIMAP_SIZE, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM,
                                        &m_TextureStore, NULL)) {
        goto really_lost;
    }

    if (D3D_OK != m_TextureStore->GetSurfaceLevel(0, &surf_to)) {
        m_TextureStore->Release();
        m_TextureStore = NULL;
        goto really_lost;
    }

    if (D3D_OK != m_Texture->Tex()->GetSurfaceLevel(0, &surf_from)) {
        surf_to->Release();
        m_TextureStore->Release();
        m_TextureStore = NULL;
        goto really_lost;
    }

    if (D3D_OK != D3DXLoadSurfaceFromSurface(surf_to, NULL, NULL, surf_from, NULL, NULL, D3DX_FILTER_NONE, 0)) {
        surf_from->Release();
        surf_to->Release();
        m_TextureStore->Release();
        m_TextureStore = NULL;
        goto really_lost;
    }

    surf_from->Release();
    surf_to->Release();

really_lost:
    g_Cache->Destroy(m_Texture);
    m_Texture = NULL;
}

void CMinimap::RestoreTexture(void) {
    DTRACE();
    if (m_Texture != NULL || m_TextureStore == NULL)
        return;

    LPDIRECT3DTEXTURE9 newTexture;
    if (D3D_OK != g_D3DD->CreateTexture(MINIMAP_SIZE, MINIMAP_SIZE, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8,
                                        D3DPOOL_DEFAULT, &newTexture, NULL)) {
        m_TextureStore->Release();
        m_TextureStore = NULL;
        return;
    }

    ASSERT_DX(m_TextureStore->AddDirtyRect(NULL));
    g_D3DD->UpdateTexture(m_TextureStore, newTexture);
    m_TextureStore->Release();
    m_TextureStore = NULL;

    m_Texture = CACHE_CREATE_TEXTURE();
    m_Texture->Set(newTexture);
}

#ifdef MINIMAP_SUPPORT_ROTATION
void CMinimap::SetAngle(float angle) {
    D3DXVECTOR2 center;

    center.x = float(m_PosX) + float(m_SizeX / 2.0);
    center.y = float(m_PosY) + float(m_SizeY / 2.0);

    D3DXMATRIX trans, ma;

    D3DXMatrixTranslation(&trans, -center.x, -center.y, 0);
    D3DXMatrixRotationZ(&ma, angle);

    m_Rotation = trans * ma;

    D3DXMatrixTranslation(&trans, center.x, center.y, 0);

    m_Rotation *= trans;

    m_Dirty = 1;
}
#endif

void CMinimap::BeforeDraw(void) {
    CTextureManaged *pt = NULL;
    for (int i = 0; i < MMT_CNT; ++i) {
        CTextureManaged *pt1 = m_Tex[i].tex;
        if (pt1 != pt) {
            pt1->Preload();
            pt = pt1;
        }
    }

    if (m_Dirty == 0)
        return;

    m_Viewport.X = m_PosX;
    m_Viewport.Y = m_PosY;
    m_Viewport.Width = m_SizeX;
    m_Viewport.Height = m_SizeY;
    m_Viewport.MinZ = 0.0f;
    m_Viewport.MaxZ = 1.0f;

    m_Delta = D3DXVECTOR2(0, 0);

    int sz = std::max(g_MatrixMap->m_Size.x, g_MatrixMap->m_Size.y);
    float fsz = float(sz) * GLOBAL_SCALE;
    float x0 = (float(g_MatrixMap->m_Size.x - sz) * 0.5f * GLOBAL_SCALE);
    float y0 = (float(g_MatrixMap->m_Size.y - sz) * 0.5f * GLOBAL_SCALE);

    D3DXVECTOR2 lt /*,lb, rt */, rb;
    World2Map(lt, D3DXVECTOR2(x0, y0));
    // World2Map(lb, D3DXVECTOR2(0, g_MatrixMap->m_Size.y * GLOBAL_SCALE));
    // World2Map(rt, D3DXVECTOR2(g_MatrixMap->m_Size.x * GLOBAL_SCALE, 0));
    World2Map(rb, D3DXVECTOR2(x0 + fsz, y0 + fsz));

    if (lt.x > float(m_PosX))
        m_Delta.x = float(m_PosX) - lt.x;
    if (lt.y > float(m_PosY))
        m_Delta.y = float(m_PosY) - lt.y;
    if (rb.x < float(m_PosX + m_SizeX))
        m_Delta.x = float(m_PosX + m_SizeX) - rb.x;
    if (rb.y < float(m_PosY + m_SizeY)) {
        m_Delta.y = float(m_PosY + m_SizeY) - rb.y;
    }

    lt += m_Delta;
    rb += m_Delta;

    m_Verts[0].p = D3DXVECTOR4(lt.x, rb.y, MINIMAP_Z, 1.0f);
    m_Verts[0].tu = 0;
    m_Verts[0].tv = 1.0f;

    m_Verts[1].p = D3DXVECTOR4(lt.x, lt.y, MINIMAP_Z, 1.0f);
    m_Verts[1].tu = 0;
    m_Verts[1].tv = 0.0f;

    m_Verts[2].p = D3DXVECTOR4(rb.x, rb.y, MINIMAP_Z, 1.0f);
    m_Verts[2].tu = 1.0f;
    m_Verts[2].tv = 1.0f;

    m_Verts[3].p = D3DXVECTOR4(rb.x, lt.y, MINIMAP_Z, 1.0f);
    m_Verts[3].tu = 1.0f;
    m_Verts[3].tv = 0.0f;

#ifdef MINIMAP_SUPPORT_ROTATION
    D3DXVec2TransformCoordArray((D3DXVECTOR2 *)&m_Verts[0].p, sizeof(SVert_V4_UV), (D3DXVECTOR2 *)&m_Verts[0].p,
                                sizeof(SVert_V4_UV), &m_Rotation, 4);
#endif

    m_Dirty = 0;
}

void CMinimap::Map2World(D3DXVECTOR2 &out, const D3DXVECTOR2 &in) {
    DTRACE();

    D3DXVECTOR2 pos;

#ifdef MINIMAP_SUPPORT_ROTATION
    D3DXMATRIX ri;
    D3DXMatrixInverse(&ri, NULL, &m_Rotation);
    D3DXVec2TransformCoord(&pos, &in, &ri);
#else
    pos = in;
#endif

    int sz = std::max(g_MatrixMap->m_Size.x, g_MatrixMap->m_Size.y);
    float fsz = (GLOBAL_SCALE * float(sz));

    float fx = (pos.x - m_Delta.x - float(m_PosX) - float(m_SizeX) * 0.5f) * fsz / float(float(m_SizeX));
    float fy = (pos.y - m_Delta.y - float(m_PosY) - float(m_SizeY) * 0.5f) * fsz / float(float(m_SizeY));

    out.x = (fx - m_Center.x + (g_MatrixMap->m_Size.x * GLOBAL_SCALE * 0.5f)) / m_Scale + m_Center.x;
    out.y = (fy - m_Center.y + (g_MatrixMap->m_Size.y * GLOBAL_SCALE * 0.5f)) / m_Scale + m_Center.y;
}

void CMinimap::World2Map(D3DXVECTOR2 &out, const D3DXVECTOR2 &in) {
    DTRACE();

    int sz = std::max(g_MatrixMap->m_Size.x, g_MatrixMap->m_Size.y);
    float fsz = (float(1.0 / GLOBAL_SCALE) / float(sz));

    float fx = ((in.x - m_Center.x) * m_Scale + m_Center.x) - g_MatrixMap->m_Size.x * GLOBAL_SCALE * 0.5f;
    float fy = ((in.y - m_Center.y) * m_Scale + m_Center.y) - g_MatrixMap->m_Size.y * GLOBAL_SCALE * 0.5f;

    // float fx = in.x - g_MatrixMap->m_Size.x * GLOBAL_SCALE * 0.5f;
    // float fy = in.y - g_MatrixMap->m_Size.y * GLOBAL_SCALE * 0.5f;

    out.x = m_Delta.x + float(m_PosX + fx * fsz * float(m_SizeX) + float(m_SizeX) * 0.5f);
    out.y = m_Delta.y + float(m_PosY + fy * fsz * float(m_SizeY) + float(m_SizeY) * 0.5f);
}

void CMinimap::Clip(D3DXVECTOR2 &out, const D3DXVECTOR2 &in) {
    float szx = float(m_SizeY) * 0.5f;
    float szy = float(m_SizeX) * 0.5f;
    D3DXVECTOR2 dir(float(m_PosX) + szx - in.x, float(m_PosY) + szy - in.y);
    // D3DXVec2Normalize(&dir, &dir);
    for (;;) {
        if (dir.x > szx) {
            dir.y = dir.y / dir.x * szx;
            dir.x = szx;
            continue;
        }
        if (dir.x < -szx) {
            dir.y = -dir.y / dir.x * szx;
            dir.x = -szx;
            continue;
        }
        if (dir.y > szy) {
            dir.x = dir.x / dir.y * szy;
            dir.y = szy;
            continue;
        }
        if (dir.y < -szy) {
            dir.x = -dir.x / dir.y * szy;
            dir.y = -szy;
            continue;
        }

        break;
    };
    out.x = float(m_PosX) + szx - dir.x * MINIMAP_OUT_SCALE;
    out.y = float(m_PosY) + szy - dir.y * MINIMAP_OUT_SCALE;
}

void CMinimap::AddEvent(float x, float y, DWORD color1, DWORD color2) {
    DTRACE();
    int idx = m_EventsCnt;
    if (m_EventsCnt >= MINIMAP_MAX_EVENTS) {
        int mini = 0;
        float mint = m_Events[0].ttl;

        for (int i = 1; i < MINIMAP_MAX_EVENTS; ++i) {
            if (m_Events[i].ttl < mint) {
                mini = i;
                mint = m_Events[i].ttl;
            }
        }
        idx = mini;
    }
    else
        ++m_EventsCnt;

    m_Events[idx].pos_in_world = D3DXVECTOR2(x, y);
    m_Events[idx].color1 = color1;
    m_Events[idx].color2 = color2;
    m_Events[idx].ttl = 1;
}

void CMinimap::PauseTakt(float takt) {
    float mul = (float)(1.0 - pow(0.995, double(takt)));
    m_Scale += (m_TgtScale - m_Scale) * mul;
}

void CMinimap::Takt(float takt) {
    DTRACE();

    m_Time += takt;

    int i = 0;
    float dt = float(takt) * INVERT(MINIMAP_EVENT_TTL);
    while (i < m_EventsCnt) {
        m_Events[i].ttl -= dt;
        if (m_Events[i].ttl < 0) {
            m_Events[i] = m_Events[--m_EventsCnt];
            continue;
        }

        ++i;
    }

    float mul = (float)(1.0 - pow(0.995, double(takt)));
    m_Scale += (m_TgtScale - m_Scale) * mul;
}

void CMinimap::DrawOutIndicator(const D3DXVECTOR2 &in, float r, DWORD c1, DWORD c2, bool doclip, EMMTex tex) {
    const D3DXVECTOR2 *pos;
    D3DXVECTOR2 cliped;
    if (doclip) {
        Clip(cliped, in);
        pos = &cliped;
    }
    else {
        pos = &in;
    }

    SVert_V4_UV v[4];
    v[0].tu = m_Tex[tex].u0;
    v[0].tv = m_Tex[tex].v1;
    v[1].tu = m_Tex[tex].u0;
    v[1].tv = m_Tex[tex].v0;
    v[2].tu = m_Tex[tex].u1;
    v[2].tv = m_Tex[tex].v1;
    v[3].tu = m_Tex[tex].u1;
    v[3].tv = m_Tex[tex].v0;

    D3DXVECTOR2 dir(float(m_PosX) + float(m_SizeX) * 0.5f - pos->x, float(m_PosY) + float(m_SizeY) * 0.5f - pos->y);
    D3DXVec2Normalize(&dir, &dir);

    DWORD c = LIC(c1, c2, float(sin(M_PI_MUL(m_Time / MINIMAP_FLASH_PERIOD)) * 0.5 + 0.5));

    v[0].p = D3DXVECTOR4(pos->x + r * (dir.x + dir.y), pos->y - r * (dir.x - dir.y), MINIMAP_Z, 1.0f);
    v[1].p = D3DXVECTOR4(pos->x + r * (dir.x - dir.y), pos->y + r * (dir.x + dir.y), MINIMAP_Z, 1.0f);
    v[2].p = D3DXVECTOR4(pos->x - r * (dir.x - dir.y), pos->y - r * (dir.x + dir.y), MINIMAP_Z, 1.0f);
    v[3].p = D3DXVECTOR4(pos->x - r * (dir.x + dir.y), pos->y + r * (dir.x - dir.y), MINIMAP_Z, 1.0f);

    CInstDraw::AddVerts(v, m_Tex[tex].tex, c);
}

void CMinimap::DrawRadar(float x, float y, float radius) {
    CMatrixMapStatic *arcaded = g_MatrixMap->GetPlayerSide()->GetArcadedObject();
    if (arcaded == NULL)
        return;
    float mapradius;

    if (arcaded->IsRobot()) {
        mapradius = arcaded->AsRobot()->m_RadarRadius;
    }
    else
#ifdef _DEBUG
    if (arcaded->IsFlyer())
#endif
    {
        mapradius = g_Config.m_FlyerRadarR;
    }
#ifdef _DEBUG
    else
        debugbreak();
#endif
                ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    g_D3DD->SetRenderState(D3DRS_ZENABLE, FALSE);

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetColorOpDisable(1);

    SVert_V4_UV v[4];
    CInstDraw::BeginDraw(IDFVF_V4_UV);

    D3DXMATRIX m;

    float scale = radius / mapradius;

    float sina = float(sin(g_MatrixMap->m_Camera.GetAngleZ()));
    float cosa = float(cos(g_MatrixMap->m_Camera.GetAngleZ()));

    m._11 = scale * cosa;
    m._12 = -scale * sina;
    m._13 = 0;
    m._14 = 0;
    m._21 = scale * sina;
    m._22 = scale * cosa;
    m._23 = 0;
    m._24 = 0;
    m._31 = 0;
    m._32 = 0;
    m._33 = 0;
    m._34 = 0;
    m._43 = 0;
    m._44 = 1;

    m._41 = scale * (-arcaded->GetGeoCenter().x * cosa - arcaded->GetGeoCenter().y * sina) + x;
    m._42 = scale * (arcaded->GetGeoCenter().x * sina - arcaded->GetGeoCenter().y * cosa) + y;

    // D3DXMatrixTranslation(&trans, -center.x, -center.y, 0);
    // D3DXMatrixRotationZ(&ma, angle);

    // m_Rotation = trans * ma;

    CMatrixMapStatic *ms = CMatrixMapStatic::GetFirstLogic();

    float r2 = mapradius * mapradius;

    for (; ms; ms = ms->GetNextLogic()) {
        EMMTex tex;
        float r;
        switch (ms->GetObjectType()) {
            case OBJECT_TYPE_ROBOTAI:
            {
                if (!ms->IsLiveRobot())
                    continue;
                if (g_MatrixMap->GetPlayerSide()->GetArcadedObject() == ms)
                    continue;
                auto tmp = *(D3DXVECTOR2 *)&ms->GetGeoCenter() - *(D3DXVECTOR2 *)&arcaded->GetGeoCenter();
                if (r2 < D3DXVec2LengthSq(&tmp)) {
                    continue;
                }

                tex = MMT_ROBOT_R;
                r = MINIMAP_ROBOT_R;
                break;
            }
            case OBJECT_TYPE_FLYER:
            {
                if (g_MatrixMap->GetPlayerSide()->GetArcadedObject() == ms)
                    continue;
                auto tmp = *(D3DXVECTOR2 *)&ms->GetGeoCenter() - *(D3DXVECTOR2 *)&arcaded->GetGeoCenter();
                if (r2 < D3DXVec2LengthSq(&tmp)) {
                    continue;
                }
                tex = MMT_FLYER_R;
                r = MINIMAP_FLYER_R;
                break;
            }
            case OBJECT_TYPE_BUILDING:
            {
                if (!ms->IsLiveBuilding())
                    continue;
                auto tmp = *(D3DXVECTOR2 *)&ms->GetGeoCenter() - *(D3DXVECTOR2 *)&arcaded->GetGeoCenter();
                if (r2 < D3DXVec2LengthSq(&tmp)) {
                    continue;
                }
                if (ms->IsBase()) {
                    tex = MMT_BASE_R;
                    r = MINIMAP_BUILDING_BASE_R;
                }
                else {
                    tex = MMT_FACTORY_R;
                    r = MINIMAP_BUILDING_R;
                }
                break;
            }
            case OBJECT_TYPE_CANNON:
            {
                if (!ms->IsLiveCannon()) {
                    continue;
                }
                auto tmp = *(D3DXVECTOR2 *)&ms->GetGeoCenter() - *(D3DXVECTOR2 *)&arcaded->GetGeoCenter();
                if (r2 < D3DXVec2LengthSq(&tmp)) {
                    continue;
                }
                tex = MMT_TURRET_R;
                r = MINIMAP_CANNON_R;
                break;
            }
            default:
            {
                continue;
            }
        }

        DWORD c = (m_Color & 0xFF000000) | g_MatrixMap->GetSideColorMM(ms->GetSide());

        D3DXVECTOR2 pos;
        D3DXVec2TransformCoord(&pos, (D3DXVECTOR2 *)&ms->GetGeoCenter(), &m);
        pos.x = (float)floor(pos.x) + 0.5f;
        pos.y = (float)floor(pos.y) + 0.5f;

        v[0].tu = m_Tex[tex].u0;
        v[0].tv = m_Tex[tex].v1;
        v[1].tu = m_Tex[tex].u0;
        v[1].tv = m_Tex[tex].v0;
        v[2].tu = m_Tex[tex].u1;
        v[2].tv = m_Tex[tex].v1;
        v[3].tu = m_Tex[tex].u1;
        v[3].tv = m_Tex[tex].v0;

        v[0].p = D3DXVECTOR4(pos.x - r, pos.y + r, MINIMAP_Z, 1.0f);
        v[1].p = D3DXVECTOR4(pos.x - r, pos.y - r, MINIMAP_Z, 1.0f);
        v[2].p = D3DXVECTOR4(pos.x + r, pos.y + r, MINIMAP_Z, 1.0f);
        v[3].p = D3DXVECTOR4(pos.x + r, pos.y - r, MINIMAP_Z, 1.0f);

        // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, (m_Color & 0xFF000000) |
        // g_MatrixMap->GetSideColorMM(ms->GetSide()))); ASSERT_DX(g_D3DD->SetTexture(0,tex->Tex()));
        // ASSERT_DX(g_D3DD->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &v, sizeof(SMinimapVertex) ));

        CInstDraw::AddVerts(v, m_Tex[tex].tex, c);
    }

    CInstDraw::ActualDraw();

    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    g_D3DD->SetRenderState(D3DRS_ZENABLE, TRUE);

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
}

void CMinimap::Draw(void) {
    DTRACE();

    D3DVIEWPORT9 wp;
    ASSERT_DX(g_D3DD->GetViewport(&wp));
    ASSERT_DX(g_D3DD->SetViewport(&m_Viewport));

    ASSERT_DX(g_D3DD->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0));

    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    g_D3DD->SetRenderState(D3DRS_ZENABLE, FALSE);

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, m_Color));

    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpSelect(0, D3DTA_TFACTOR);
    SetColorOpDisable(1);

    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

    if (m_Texture) {
        CInstDraw::BeginDraw(IDFVF_V4_UV);
        CInstDraw::AddVerts(m_Verts, m_Texture);
        CInstDraw::ActualDraw();
    }

    CInstDraw::BeginDraw(IDFVF_V4_UV);

    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

    // ASSERT_DX(g_D3DD->SetTexture(0,m_Texture->Tex()));
    // ASSERT_DX(g_D3DD->SetStreamSource( 0, GET_VB(m_VB), 0, sizeof(SMinimapVertex) ));
    // ASSERT_DX(g_D3DD->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 ));

    SVert_V4_UV v[4];
    SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);

    for (int i = 0; i < m_EventsCnt; ++i) {
        D3DXVECTOR2 pos;
        World2Map(pos, m_Events[i].pos_in_world);
#ifdef MINIMAP_SUPPORT_ROTATION
        D3DXVec2TransformCoord(&pos, &pos, &m_Rotation);
#endif
        if (IsOut(pos)) {
            // ASSERT_DX(g_D3DD->SetTexture(0,m_TexArrow->Tex()));

            Clip(pos, pos);

            DrawOutIndicator(pos, MINIMAP_OUT_INDICATOR_R, m_Events[i].color1, m_Events[i].color2, false, MMT_ARROW);
        }
        else {
            float k = 1.0f - m_Events[i].ttl;
            float r = LERPFLOAT(k, MINIMAP_EVENT_R1, MINIMAP_EVENT_R2);

            DWORD c = LIC(m_Events[i].color1, m_Events[i].color2, k);

            v[0].tu = m_Tex[MMT_POINT].u0;
            v[0].tv = m_Tex[MMT_POINT].v1;
            v[1].tu = m_Tex[MMT_POINT].u0;
            v[1].tv = m_Tex[MMT_POINT].v0;
            v[2].tu = m_Tex[MMT_POINT].u1;
            v[2].tv = m_Tex[MMT_POINT].v1;
            v[3].tu = m_Tex[MMT_POINT].u1;
            v[3].tv = m_Tex[MMT_POINT].v0;

            v[0].p = D3DXVECTOR4(pos.x - r, pos.y + r, MINIMAP_Z, 1.0f);
            v[1].p = D3DXVECTOR4(pos.x - r, pos.y - r, MINIMAP_Z, 1.0f);
            v[2].p = D3DXVECTOR4(pos.x + r, pos.y + r, MINIMAP_Z, 1.0f);
            v[3].p = D3DXVECTOR4(pos.x + r, pos.y - r, MINIMAP_Z, 1.0f);

            // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, c));
            // ASSERT_DX(g_D3DD->SetTexture(0,m_TexPoint->Tex()));
            // ASSERT_DX(g_D3DD->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &v, sizeof(SMinimapVertex) ));

            CInstDraw::AddVerts(v, m_Tex[MMT_POINT].tex, c);
        }
    }

    CMatrixMapStatic *ms = CMatrixMapStatic::GetFirstLogic();

    for (; ms; ms = ms->GetNextLogic()) {
        EMMTex tex;
        float r;
        bool flash = false;
        switch (ms->GetObjectType()) {
            case OBJECT_TYPE_ROBOTAI:
                if (!ms->IsLiveRobot())
                    continue;
                tex = MMT_ROBOT;
                r = MINIMAP_ROBOT_R;
                if (ms->AsRobot()->m_MiniMapFlashTime > 0)
                    flash = (g_MatrixMap->GetTime() & 128) == 0 && ms->AsRobot()->m_Side == PLAYER_SIDE;
                // flash = true;
                break;
            case OBJECT_TYPE_FLYER:
                tex = MMT_FLYER;
                r = MINIMAP_FLYER_R;
                break;
            case OBJECT_TYPE_BUILDING:
                if (!ms->IsLiveBuilding())
                    continue;
                if (ms->IsBase()) {
                    tex = MMT_BASE;
                    r = MINIMAP_BUILDING_BASE_R;
                }
                else {
                    tex = MMT_FACTORY;
                    r = MINIMAP_BUILDING_R;
                }
                break;
            case OBJECT_TYPE_CANNON:
                if (!ms->IsLiveCannon())
                    continue;
                tex = MMT_TURRET;
                r = MINIMAP_CANNON_R;

                if (ms->AsCannon()->m_MiniMapFlashTime > 0)
                    flash = (g_MatrixMap->GetTime() & 128) == 0 && ms->GetSide() == PLAYER_SIDE;

                break;
            default: {
                continue;
            }
        }
        D3DXVECTOR2 pos;
        World2Map(pos, *(D3DXVECTOR2 *)&ms->GetGeoCenter());

#ifdef MINIMAP_SUPPORT_ROTATION
        D3DXVec2TransformCoord(&pos, &pos, &m_Rotation);
#endif

        DWORD c;
        if (flash) {
            r *= 2;
            c = 0xFFFFFFFF;
        }
        else {
            c = (m_Color & 0xFF000000) | g_MatrixMap->GetSideColorMM(ms->GetSide());
        }

        pos.x = (float)floor(pos.x) - 0.5f;
        pos.y = (float)floor(pos.y) - 0.5f;

        v[0].tu = m_Tex[tex].u0;
        v[0].tv = m_Tex[tex].v1;
        v[1].tu = m_Tex[tex].u0;
        v[1].tv = m_Tex[tex].v0;
        v[2].tu = m_Tex[tex].u1;
        v[2].tv = m_Tex[tex].v1;
        v[3].tu = m_Tex[tex].u1;
        v[3].tv = m_Tex[tex].v0;

        v[0].p = D3DXVECTOR4(pos.x - r, pos.y + r, MINIMAP_Z, 1.0f);
        v[1].p = D3DXVECTOR4(pos.x - r, pos.y - r, MINIMAP_Z, 1.0f);
        v[2].p = D3DXVECTOR4(pos.x + r, pos.y + r, MINIMAP_Z, 1.0f);
        v[3].p = D3DXVECTOR4(pos.x + r, pos.y - r, MINIMAP_Z, 1.0f);

        // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, ));
        // ASSERT_DX(g_D3DD->SetTexture(0,tex->Tex()));
        // ASSERT_DX(g_D3DD->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &v, sizeof(SMinimapVertex) ));

        // DWORD c = (m_Color & 0xFF000000) | g_MatrixMap->GetSideColorMM(ms->GetSide());
        CInstDraw::AddVerts(v, m_Tex[tex].tex, c);
    }

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF));

    // D3DXVECTOR2 fp;
    // CMatrixFlyer *f = g_MatrixMap->GetPlayerSide()->GetFlyer();
    // World2Map(fp, D3DXVECTOR2(f->GetPos().x, f->GetPos().y));

    // ASSERT_DX(g_D3DD->SetTexture(0,m_TexFlyer->Tex()));
    // if (IsOut(fp))
    //{
    //    DrawOutIndicator(fp, MINIMAP_OUT_INDICATOR_R, MINIMAP_FLYER_C1, MINIMAP_FLYER_C2,true);

    //} else
    //{
    //    float sn,cs;
    //    SinCos(f->GetAngle(), &cs, &sn);

    //    float r = MINIMAP_FLYER_R;

    //    v[0].p = D3DXVECTOR4( fp.x + r * (cs - sn), fp.y - r * (cs + sn), MINIMAP_Z, 1.0f );
    //    v[1].p = D3DXVECTOR4( fp.x + r * (cs + sn), fp.y + r * (cs - sn), MINIMAP_Z, 1.0f );
    //    v[2].p = D3DXVECTOR4( fp.x - r * (cs + sn), fp.y - r * (cs - sn), MINIMAP_Z, 1.0f );
    //    v[3].p = D3DXVECTOR4( fp.x - r * (cs - sn), fp.y + r * (cs + sn), MINIMAP_Z, 1.0f );

    //    D3DXVec2TransformCoordArray((D3DXVECTOR2 *)&v[0].p, sizeof(SMinimapVertex), (D3DXVECTOR2 *)&v[0].p,
    //    sizeof(SMinimapVertex), &m_Rotation, 4); ASSERT_DX(g_D3DD->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &v,
    //    sizeof(SMinimapVertex) ));
    //}

    CInstDraw::ActualDraw();

    float k;
    D3DXVECTOR4 campts[5];

    // right top <- left top
    if (g_MatrixMap->m_Camera.GetFrustumLT().z >= 0) {
        k = 100000;
    }
    else {
        k = (-g_MatrixMap->m_Camera.GetFrustumCenter().z) / g_MatrixMap->m_Camera.GetFrustumLT().z;
    }
    campts[0].x = g_MatrixMap->m_Camera.GetFrustumCenter().x + g_MatrixMap->m_Camera.GetFrustumLT().x * k;
    campts[0].y = g_MatrixMap->m_Camera.GetFrustumCenter().y + g_MatrixMap->m_Camera.GetFrustumLT().y * k;
    campts[0].z = MINIMAP_Z;
    campts[0].w = 1.0f;

    if (g_MatrixMap->m_Camera.GetFrustumRT().z >= 0) {
        k = 100000;
    }
    else {
        k = (-g_MatrixMap->m_Camera.GetFrustumCenter().z) / g_MatrixMap->m_Camera.GetFrustumRT().z;
    }
    campts[1].x = g_MatrixMap->m_Camera.GetFrustumCenter().x + g_MatrixMap->m_Camera.GetFrustumRT().x * k;
    campts[1].y = g_MatrixMap->m_Camera.GetFrustumCenter().y + g_MatrixMap->m_Camera.GetFrustumRT().y * k;
    campts[1].z = MINIMAP_Z;
    campts[1].w = 1.0f;

    if (g_MatrixMap->m_Camera.GetFrustumRB().z >= 0) {
        k = 100000;
    }
    else {
        k = (-g_MatrixMap->m_Camera.GetFrustumCenter().z) / g_MatrixMap->m_Camera.GetFrustumRB().z;
    }
    campts[2].x = g_MatrixMap->m_Camera.GetFrustumCenter().x + g_MatrixMap->m_Camera.GetFrustumRB().x * k;
    campts[2].y = g_MatrixMap->m_Camera.GetFrustumCenter().y + g_MatrixMap->m_Camera.GetFrustumRB().y * k;
    campts[2].z = MINIMAP_Z;
    campts[2].w = 1.0f;

    if (g_MatrixMap->m_Camera.GetFrustumLB().z >= 0) {
        k = 100000;
    }
    else {
        k = (-g_MatrixMap->m_Camera.GetFrustumCenter().z) / g_MatrixMap->m_Camera.GetFrustumLB().z;
    }
    campts[3].x = g_MatrixMap->m_Camera.GetFrustumCenter().x + g_MatrixMap->m_Camera.GetFrustumLB().x * k;
    campts[3].y = g_MatrixMap->m_Camera.GetFrustumCenter().y + g_MatrixMap->m_Camera.GetFrustumLB().y * k;
    campts[3].z = MINIMAP_Z;
    campts[3].w = 1.0f;

    auto Vec4World2Map = [this](D3DXVECTOR4& vec) {
        D3DXVECTOR2 in, out;
        in.x = vec.x;
        in.y = vec.y;
        World2Map(out, in);
        vec.x = out.x;
        vec.y = out.y;
    };

    Vec4World2Map(campts[0]);
    Vec4World2Map(campts[1]);
    Vec4World2Map(campts[2]);
    Vec4World2Map(campts[3]);

#ifdef MINIMAP_SUPPORT_ROTATION
    D3DXVec2TransformCoordArray((D3DXVECTOR2 *)&campts[0], sizeof(D3DXVECTOR4), (D3DXVECTOR2 *)&campts[0],
                                sizeof(D3DXVECTOR4), &m_Rotation, 4);
#endif

    campts[4] = campts[0];

    g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, MINIMAP_CAM_COLOR);

    SetColorOpSelect(0, D3DTA_TFACTOR);
    SetAlphaOpSelect(0, D3DTA_TFACTOR);
    SetColorOpDisable(1);

    g_D3DD->SetFVF(D3DFVF_XYZRHW);
    g_D3DD->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &campts, sizeof(D3DXVECTOR4));

    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    g_D3DD->SetRenderState(D3DRS_ZENABLE, TRUE);

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

    ASSERT_DX(g_D3DD->SetViewport(&wp));
}

void CMinimap::RenderBackground(const std::wstring &name, DWORD uniq) {
    DTRACE();
    SETFLAG(g_MatrixMap->m_Flags, MMFLAG_DISABLE_DRAW_OBJECT_LIGHTS);

    std::wstring mmname = PathToOutputFiles(FOLDER_NAME_CACHE) + L"\\" + name + L".";

    for (int i = 0; i < 8; i++) {
        mmname += (char)('A' + (uniq & 0x0F));
        uniq >>= 4;
    }

    // create and set render target
    LPDIRECT3DTEXTURE9 newTexture;
    IDirect3DSurface9 *newTarget;

    if (D3D_OK != g_D3DD->CreateTexture(MINIMAP_SIZE, MINIMAP_SIZE, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8,
                                        D3DPOOL_DEFAULT, &newTexture, NULL)) {
        return;
    }
    // newTexture->SetPriority(0xFFFFFFFF);
    ASSERT_DX(newTexture->GetSurfaceLevel(0, &newTarget));

    {
        WIN32_FIND_DATAW fd;
        HANDLE ff = FindFirstFileW(mmname.c_str(), &fd);
        if (ff != INVALID_HANDLE_VALUE) {
            CBitmap bm(g_CacheHeap);
            bm.LoadFromPNG(mmname.c_str());
            if (bm.SizeX() != MINIMAP_SIZE || bm.SizeY() != MINIMAP_SIZE)
                goto render;

            LPDIRECT3DTEXTURE9 lt;
            ASSERT_DX(g_D3DD->CreateTexture(MINIMAP_SIZE, MINIMAP_SIZE, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &lt,
                                            NULL));

            D3DLOCKED_RECT lr;
            ASSERT_DX(lt->LockRect(0, &lr, NULL, 0));

            BYTE *sou = (BYTE *)bm.Data();
            BYTE *des = (BYTE *)lr.pBits;
            for (int y = 0; y < MINIMAP_SIZE;
                 y++, sou += bm.Pitch() - MINIMAP_SIZE * 3, des += lr.Pitch - MINIMAP_SIZE * 4) {
                for (int x = 0; x < MINIMAP_SIZE; x++, sou += 3, des += 4) {
                    *(des) = *(sou + 2);
                    *(des + 1) = *(sou + 1);
                    *(des + 2) = *(sou + 0);
                    *(des + 3) = 255;
                }
            }

            ASSERT_DX(lt->UnlockRect(0));

            IDirect3DSurface9 *ltt;
            ASSERT_DX(lt->GetSurfaceLevel(0, &ltt));

            HRESULT hr = g_D3DD->UpdateSurface(ltt, NULL, newTarget, NULL);

            ltt->Release();
            lt->Release();
            newTarget->Release();
            FindClose(ff);

            if (hr == D3D_OK) {
                m_Texture->Set(newTexture);

                // MessageBox(NULL, CStr(int(newTexture)), "tex 1", MB_OK);
                // MessageBox(NULL, CStr(int(m_Texture->Tex())), "", MB_OK);

                RESETFLAG(g_MatrixMap->m_Flags, MMFLAG_DISABLE_DRAW_OBJECT_LIGHTS);
                return;
            }
        }
    }
render:

    IDirect3DSurface9 *newZ, *oldZ = NULL;
    bool CustomZ = (MINIMAP_SIZE > g_ScreenY) || (MINIMAP_SIZE > g_ScreenX);
    if (CustomZ) {
        D3DSURFACE_DESC d;
        g_D3DD->GetDepthStencilSurface(&oldZ);
        oldZ->GetDesc(&d);
        g_D3DD->CreateDepthStencilSurface(MINIMAP_SIZE, MINIMAP_SIZE, d.Format, d.MultiSampleType, d.MultiSampleQuality,
                                          TRUE, &newZ, NULL);
        g_D3DD->SetDepthStencilSurface(newZ);
        newZ->Release();
    }

    // store old viewport
    D3DVIEWPORT9 oldViewport;
    ASSERT_DX(g_D3DD->GetViewport(&oldViewport));

    // set new viewport
    D3DVIEWPORT9 newViewport;
    newViewport.X = 0;
    newViewport.Y = 0;
    newViewport.Width = MINIMAP_SIZE;
    newViewport.Height = MINIMAP_SIZE;
    newViewport.MinZ = 0.0f;
    newViewport.MaxZ = 1.0f;
    FAILED_DX(g_D3DD->SetViewport(&newViewport));

    // store old render target
    IDirect3DSurface9 *oldTarget;
    FAILED_DX(g_D3DD->GetRenderTarget(0, &oldTarget));

    ASSERT_DX(g_D3DD->SetRenderTarget(0, newTarget));

    ASSERT_DX(g_D3DD->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000L, 1.0f, 0L));

    D3DXMATRIX mView;
    D3DXVECTOR3 campos(float(g_MatrixMap->m_Size.x) * (GLOBAL_SCALE / 2),
                       float(g_MatrixMap->m_Size.y) * (GLOBAL_SCALE / 2), 1300);
    auto tmp1 = campos - D3DXVECTOR3(0, 0, 1);
    auto tmp2 = D3DXVECTOR3(0, -1, 0);
    D3DXMatrixLookAtLH(&mView, &campos, &tmp1, &tmp2);

    int sz = std::max(g_MatrixMap->m_Size.x, g_MatrixMap->m_Size.y);
    float fsz = (float(sz) * GLOBAL_SCALE);
    float scale = 1.0f / fsz;
    // scale = 0.1;

    mView._11 *= scale;
    mView._21 *= scale;
    mView._31 *= scale;
    mView._41 *= scale;
    mView._12 *= scale;
    mView._22 *= scale;
    mView._32 *= scale;
    mView._42 *= scale;

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_VIEW, &mView));
    // ASSERT_DX(g_D3DD->SetTransform( D3DTS_WORLD, &GetIdentityMatrix() ));
    D3DXMATRIX mProj(-2, 0, 0, 0, 0, 2, 0, 0, 0, 0, float(1.0 / (10000 - 1)), 0, 0, 0, -float(1.0 / (10000 - 1)), 1);
    D3DXMatrixOrthoLH(&mProj, 1.0f, 1.0f, 1.0f, 10000.0f);

    // UpdateAfterMovingCamera();
    // D3DXMatrixPerspectiveFovLH(&mProj,D3DX_PI/3/*6*/,1, 1.0f, 1500.0f);
    ASSERT_DX(g_D3DD->SetTransform(D3DTS_PROJECTION, &mProj));
    // ASSERT_DX(g_D3DD->SetTransform( D3DTS_VIEW, &m_MatView));

    g_MatrixMap->BeforeDrawLandscape(true);
    if (CTerSurface::IsSurfacesPresent())
        g_MatrixMap->BeforeDrawLandscapeSurfaces(true);
    g_MatrixMap->m_Water->BeforeDraw();

    S3D_Default();
    D3DMATERIAL9 mtrl;
    ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
    mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
    mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
    mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
    mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
    mtrl.Specular.r = 0.5f;
    mtrl.Specular.g = 0.5f;
    mtrl.Specular.b = 0.5f;
    mtrl.Specular.a = 0.5f;
    g_D3DD->SetMaterial(&mtrl);
    g_D3DD->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);

    D3DXVECTOR3 vecDir;
    D3DLIGHT9 light;
    ZeroMemory(&light, sizeof(D3DLIGHT9));
    light.Type = D3DLIGHT_DIRECTIONAL;  // D3DLIGHT_POINT;//D3DLIGHT_DIRECTIONAL;
    light.Diffuse.r = GetColorR(g_MatrixMap->m_LightMainColorObj);
    light.Diffuse.g = GetColorG(g_MatrixMap->m_LightMainColorObj);
    light.Diffuse.b = GetColorB(g_MatrixMap->m_LightMainColorObj);
    light.Ambient.r = 0.0f;
    light.Ambient.g = 0.0f;
    light.Ambient.b = 0.0f;
    light.Specular.r = GetColorR(g_MatrixMap->m_LightMainColorObj);
    light.Specular.g = GetColorG(g_MatrixMap->m_LightMainColorObj);
    light.Specular.b = GetColorB(g_MatrixMap->m_LightMainColorObj);
    // light.Range       = 0;
    light.Direction = g_MatrixMap->m_LightMain;
    //	light.Direction=D3DXVECTOR3(250.0f,-50.0f,-250.0f);
    //	D3DXVec3Normalize((D3DXVECTOR3 *)(&(light.Direction)),(D3DXVECTOR3 *)(&(light.Direction)));
    ASSERT_DX(g_D3DD->SetLight(0, &light));
    ASSERT_DX(g_D3DD->LightEnable(0, TRUE));

    ASSERT_DX(g_D3DD->BeginScene());

    D3DXMATRIX m = g_MatrixMap->GetIdentityMatrix();
    int curpass;
    m._11 *= GLOBAL_SCALE * (float)(MAP_GROUP_SIZE) / (float)(WATER_SIZE);
    m._22 *= GLOBAL_SCALE * (float)(MAP_GROUP_SIZE) / (float)(WATER_SIZE);
    m._33 *= GLOBAL_SCALE * (float)(MAP_GROUP_SIZE) / (float)(WATER_SIZE);
    m._43 = WATER_LEVEL;

    for (curpass = 0; curpass < g_Render->m_WaterPassSolid; ++curpass) {
        g_Render->m_WaterSolid(g_MatrixMap->m_Water->m_WaterTex1, g_MatrixMap->m_Water->m_WaterTex2, curpass);

        float x0 = ((g_MatrixMap->m_Size.x * GLOBAL_SCALE) - fsz) * 0.5f;
        float x1 = ((g_MatrixMap->m_Size.x * GLOBAL_SCALE) + fsz) * 0.5f;
        float y0 = ((g_MatrixMap->m_Size.y * GLOBAL_SCALE) - fsz) * 0.5f;
        float y1 = ((g_MatrixMap->m_Size.y * GLOBAL_SCALE) + fsz) * 0.5f;

        for (float x = x0; x < x1; x += MAP_GROUP_SIZE * GLOBAL_SCALE)
            for (float y = y0; y < y1; y += MAP_GROUP_SIZE * GLOBAL_SCALE) {
                m._41 = x;
                m._42 = y;
                g_MatrixMap->m_Water->Draw(m);
            }
    }

    g_Render->m_WaterClearSolid();

    g_MatrixMap->DrawLandscape(true);
    if (CTerSurface::IsSurfacesPresent())
        g_MatrixMap->DrawLandscapeSurfaces(true);
    //    CMatrixMapTextureFinal::UnloadAll();

    //	int cnt = m_GroupSize.x*m_GroupSize.y;
    //	CMatrixMapGroup **md = m_Group;

    // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE,				D3DZB_TRUE));
    // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE,		FALSE));

    for (curpass = 0; curpass < g_Render->m_WaterPassAlpha; ++curpass) {
        g_Render->m_WaterAlpha(g_MatrixMap->m_Water->m_WaterTex1, g_MatrixMap->m_Water->m_WaterTex2, curpass);

        int cnt = g_MatrixMap->m_GroupSize.x * g_MatrixMap->m_GroupSize.y;
        CMatrixMapGroup **md = g_MatrixMap->m_Group;

        while ((cnt--) > 0) {
            if (((*md) == NULL) || !(*(md))->HasWater()) {
                ++md;
                continue;
            }

            if (curpass == 0)
                ASSERT_DX(g_D3DD->SetTexture(0, (*md)->GetWaterAlpha()->Tex()));

            const D3DXVECTOR2 &p = (*(md))->GetPos0();
            m._41 = p.x;
            m._42 = p.y;
            g_MatrixMap->m_Water->Draw(m);
            ++md;
        }
    }

    g_Render->m_WaterClearAlpha();

    // unload resources
    {
        int cnt = g_MatrixMap->m_GroupSize.x * g_MatrixMap->m_GroupSize.y;
        CMatrixMapGroup **md = g_MatrixMap->m_Group;
        while ((cnt--) > 0) {
            if ((*md) != NULL) {
                (*md)->DX_Free();
            }
        }
    }

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));

    ASSERT_DX(g_D3DD->EndScene());

    FAILED_DX(g_D3DD->SetRenderTarget(0, oldTarget));
    //	FAILED_DX(g_D3DD->SetDepthStencilSurface(oldZBuffer));

    oldTarget->Release();
    newTarget->Release();

    m_Texture->Set(newTexture);

    if (CustomZ) {
        g_D3DD->SetDepthStencilSurface(oldZ);
        oldZ->Release();
    }

    // m_MiniMap = (CTexture *)g_Cache->Get(cc_Texture,L"Matrix\\helicopter_w.png");

    ASSERT_DX(g_D3DD->SetViewport(&oldViewport));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, g_MatrixMap->m_AmbientColorObj));

    CMatrixMapStatic *ms = CMatrixMapStatic::GetFirstLogic();
    while (ms != NULL) {
        if (ms->IsLiveBuilding()) {
            ((CMatrixBuilding *)ms)->LogicTakt(100000);
            ms->RNeed(MR_Matrix | MR_Graph | MR_MiniMap);
            // RenderBuildingToMiniMapBackground((CMatrixBuilding *)ms);
        }
        ms = ms->GetNextLogic();
    }

    {
        CreateDirectoryW(PathToOutputFiles(FOLDER_NAME_CACHE).c_str(), NULL);

        // seek files
        auto n = utils::format(L"%ls\\%ls.*",
            PathToOutputFiles(FOLDER_NAME_CACHE),
            name.c_str());

        HANDLE ff;
        for (;;) {
            WIN32_FIND_DATAW fd;
            ff = FindFirstFileW(n.c_str(), &fd);
            if (ff != INVALID_HANDLE_VALUE) {
                auto nn =
                    utils::format(
                        L"%ls\\%ls",
                        PathToOutputFiles(FOLDER_NAME_CACHE),
                        fd.cFileName);
                DeleteFileW(nn.c_str());
            }
            else {
                break;
            }
            FindClose(ff);
        }

        LPDIRECT3DTEXTURE9 out;
        IDirect3DSurface9 *outtgt;
        ASSERT_DX(g_D3DD->CreateTexture(MINIMAP_SIZE, MINIMAP_SIZE, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &out,
                                        NULL));

        ASSERT_DX(newTexture->GetSurfaceLevel(0, &newTarget));
        ASSERT_DX(out->GetSurfaceLevel(0, &outtgt));

        ASSERT_DX(g_D3DD->GetRenderTargetData(newTarget, outtgt));

        D3DLOCKED_RECT lr;
        outtgt->LockRect(&lr, NULL, 0);

        CBitmap bm(g_CacheHeap);
        bm.CreateRGBA(MINIMAP_SIZE, MINIMAP_SIZE, lr.Pitch, lr.pBits);
        bm.SwapByte(CPoint(0, 0), CPoint(MINIMAP_SIZE, MINIMAP_SIZE), 0, 2);

        CBitmap bm2(g_CacheHeap);
        bm2.CreateRGB(MINIMAP_SIZE, MINIMAP_SIZE);
        bm2.Copy(CPoint(0, 0), bm.Size(), bm, CPoint(0, 0));

        //*(des)=*(sou+2);
        //*(des+2)=*(sou+0);

        bm2.SaveInPNG(mmname.c_str());

        outtgt->UnlockRect();

        newTarget->Release();
        outtgt->Release();
        out->Release();
    }
    RESETFLAG(g_MatrixMap->m_Flags, MMFLAG_DISABLE_DRAW_OBJECT_LIGHTS);
}

void CMinimap::RenderObjectToBackground(CMatrixMapStatic *s) {
    DTRACE();
    SETFLAG(g_MatrixMap->m_Flags, MMFLAG_DISABLE_DRAW_OBJECT_LIGHTS);

    bool f1 = g_Config.m_ShowStencilShadows;
    bool f2 = g_Config.m_ShowProjShadows;
    g_Config.m_ShowStencilShadows = false;
    g_Config.m_ShowProjShadows = false;
    s->BeforeDraw();
    g_Config.m_ShowStencilShadows = f1;
    g_Config.m_ShowProjShadows = f2;

    IDirect3DSurface9 *newZ, *oldZ = NULL;
    bool CustomZ = (MINIMAP_SIZE > g_ScreenY) || (MINIMAP_SIZE > g_ScreenX);
    if (CustomZ) {
        D3DSURFACE_DESC d;
        g_D3DD->GetDepthStencilSurface(&oldZ);
        oldZ->GetDesc(&d);
        g_D3DD->CreateDepthStencilSurface(MINIMAP_SIZE, MINIMAP_SIZE, d.Format, d.MultiSampleType, d.MultiSampleQuality,
                                          TRUE, &newZ, NULL);
        g_D3DD->SetDepthStencilSurface(newZ);
        newZ->Release();
    }

    // store old viewport
    D3DVIEWPORT9 oldViewport;
    ASSERT_DX(g_D3DD->GetViewport(&oldViewport));

    // set new viewport
    D3DVIEWPORT9 newViewport;
    newViewport.X = 0;
    newViewport.Y = 0;
    newViewport.Width = MINIMAP_SIZE;
    newViewport.Height = MINIMAP_SIZE;
    newViewport.MinZ = 0.0f;
    newViewport.MaxZ = 1.0f;
    FAILED_DX(g_D3DD->SetViewport(&newViewport));

    // store old render target
    IDirect3DSurface9 *oldTarget;
    FAILED_DX(g_D3DD->GetRenderTarget(0, &oldTarget));

    // create and set render target
    IDirect3DSurface9 *newTarget;

    // MessageBox(NULL, CStr(int(m_Texture->Tex())), "", MB_OK);

    ASSERT_DX(m_Texture->Tex()->GetSurfaceLevel(0, &newTarget));
    ASSERT_DX(g_D3DD->SetRenderTarget(0, newTarget));

    if (FLAG(g_Flags, GFLAG_STENCILAVAILABLE)) {
        FAILED_DX(g_D3DD->Clear(0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0x0000F000L, 1.0f, 0L));
    }
    else {
        FAILED_DX(g_D3DD->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0x0000F000L, 1.0f, 0L));
    }

    D3DXMATRIX mView;
    D3DXVECTOR3 campos(float(g_MatrixMap->m_Size.x) * (GLOBAL_SCALE / 2),
                       float(g_MatrixMap->m_Size.y) * (GLOBAL_SCALE / 2), 1300);
    auto tmp1 = campos - D3DXVECTOR3(0, 0, 1);
    auto tmp2 = D3DXVECTOR3(0, -1, 0);
    D3DXMatrixLookAtLH(&mView, &campos, &tmp1, &tmp2);

    int sz = std::max(g_MatrixMap->m_Size.x, g_MatrixMap->m_Size.y);
    float fsz = (float(sz) * GLOBAL_SCALE);
    float scale = 1.0f / fsz;

    mView._11 *= scale;
    mView._21 *= scale;
    mView._31 *= scale;
    mView._41 *= scale;
    mView._12 *= scale;
    mView._22 *= scale;
    mView._32 *= scale;
    mView._42 *= scale;

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_VIEW, &mView));
    D3DXMATRIX mProj;
    D3DXMatrixOrthoLH(&mProj, 1.0f, 1.0f, 1.0f, 10000.0f);

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_PROJECTION, &mProj));

    ASSERT_DX(g_D3DD->BeginScene());

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, g_MatrixMap->m_AmbientColorObj));

    CVectorObject::DrawBegin();
    s->Draw();
    CVectorObject::DrawEnd();

    ASSERT_DX(g_D3DD->EndScene());

    FAILED_DX(g_D3DD->SetRenderTarget(0, oldTarget));

    oldTarget->Release();
    newTarget->Release();

    if (CustomZ) {
        g_D3DD->SetDepthStencilSurface(oldZ);
        oldZ->Release();
    }

    ASSERT_DX(g_D3DD->SetViewport(&oldViewport));
    RESETFLAG(g_MatrixMap->m_Flags, MMFLAG_DISABLE_DRAW_OBJECT_LIGHTS);
}

// if (m_KeyDown && m_KeyScan == KEY_PGDN) {m_KeyDown = false; m_Minimap.SetOutParams(m_Minimap.GetScale() * 0.8f);}
// if (m_KeyDown && m_KeyScan == KEY_PGUP) {m_KeyDown = false; m_Minimap.SetOutParams(m_Minimap.GetScale() * 1.25f);}

void __stdcall CMinimap::ButtonZoomIn(void *object) {
    SetOutParams(GetScale() * 1.8f);
    CSound::Play(S_MAP_PLUS, SL_ALL);
}

void __stdcall CMinimap::ButtonZoomOut(void *object) {
    SetOutParams(GetScale() * 0.5f);
    CSound::Play(S_MAP_MINUS, SL_ALL);
}

void __stdcall CMinimap::ButtonClick(void *object) {
    CPoint mp = g_MatrixMap->m_Cursor.GetPos();
    D3DXVECTOR2 tgt;
    if (CalcMinimap2World(tgt)) {
        g_MatrixMap->m_Camera.SetXYStrategy(tgt);
    }
}

bool CMinimap::CalcMinimap2World(D3DXVECTOR2 &tgt) {
    CPoint mp = g_MatrixMap->m_Cursor.GetPos();
    if (mp.x >= m_PosX && mp.x <= m_PosX + m_SizeX && mp.y >= m_PosY && mp.y <= m_PosY + m_SizeY) {
        D3DXVECTOR2 t;
        Map2World(t, D3DXVECTOR2(float(mp.x), float(mp.y)));

        // CHelper::Create(10000,111)->Cone(D3DXVECTOR3(t.x, t.y, 0), D3DXVECTOR3(t.x, t.y, 1000), 30, 30, 0xFFFFFFFF,
        // 0xFFFFFFFF, 20);

        tgt = t;
        return true;
    }
    return false;
}

void __stdcall CMinimap::ShowPlayerBots(void *object) {
    CMatrixMapStatic *objects = CMatrixMapStatic::GetFirstLogic();
    while (objects) {
        if (objects->IsRobot() && objects->GetSide() == PLAYER_SIDE) {
            g_MatrixMap->m_Minimap.AddEvent(objects->GetGeoCenter().x, objects->GetGeoCenter().y, EVENT_SHOWPB_C1,
                                            EVENT_SHOWPB_C2);
        }
        objects = objects->GetNextLogic();
    }
}
