// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixMap.hpp"
#include "MatrixMapStatic.hpp"
#include "MatrixObject.hpp"
#include "MatrixRobot.hpp"
#include "MatrixObjectBuilding.hpp"
#include "MatrixRenderPipeline.hpp"
#include "MatrixSkinManager.hpp"
#include "MatrixFlyer.hpp"
#include "MatrixObjectCannon.hpp"

#include "Mem.hpp"

bool FreeObjResources(uintptr_t user) {
    DTRACE();
    auto obj = reinterpret_cast<CMatrixMapStatic*>(user);
    obj->FreeDynamicResources();
    return false;  // not dead
}

CVectorObjectAnim *LoadObject(const wchar *name, CHeap *heap, bool side, const wchar *tex) {
    DTRACE();

    CVectorObjectAnim *voa = HNew(heap) CVectorObjectAnim();
    CVectorObject *vo = (CVectorObject *)g_Cache->Get(cc_VO, name);

    GSParam gsp = side ? GSP_SIDE : GSP_ORDINAL;

    if (tex) {
        vo->PrepareSpecial(OLF_MULTIMATERIAL_ONLY, CSkinManager::GetSkin, gsp);
        if (vo->IsNoSkin(0)) {
            voa->Init(vo, CMatrixEffect::GetBBTexI(BBT_POINTLIGHT), CSkinManager::GetSkin(tex, gsp));
        }
        else {
            voa->Init(vo, CMatrixEffect::GetBBTexI(BBT_POINTLIGHT));
        }
    }
    else {
        vo->PrepareSpecial(OLF_AUTO, CSkinManager::GetSkin, gsp);
        voa->Init(vo, NULL);
    }

    voa->FirstFrame();
    return voa;
}

void UnloadObject(CVectorObjectAnim *o, CHeap *heap) {
    HDelete(CVectorObjectAnim, o, heap);
}

CMatrixMapStatic *CMatrixMapStatic::m_FirstLogicTemp;
CMatrixMapStatic *CMatrixMapStatic::m_LastLogicTemp;

#ifdef _DEBUG
SObjectCore *CMatrixMapStatic::GetCore(const SDebugCallInfo &dci) {
    m_Core->m_dci = dci;

    //#ifdef _DEBUG
    //
    //        FILE *f = fopen("Errors\\"+CStr(int(m_Core))+".log","a");
    //        CStr    s("get ");
    //        s = s + dci._file + " " + dci._line + "\n";
    //        fwrite(s.Get(), s.Len(), 1, f);
    //
    //        fclose(f);
    //
    //#endif

    m_Core->RefInc();
    return m_Core;
}
#endif

CMatrixMapStatic::~CMatrixMapStatic() {
    UnjoinGroup();
    DelLT();
    if (g_MatrixMap->m_TraceStopObj == this) {
        g_MatrixMap->m_TraceStopObj = NULL;
    }
    if (g_MatrixMap->GetPlayerSide()->GetArcadedObject() == this) {
        g_MatrixMap->GetPlayerSide()->SetArcadedObject(NULL);
    }

    g_MatrixMap->RemoveFromAD(this);

    m_Core->m_Object = NULL;
    m_Core->Release();
}

void CMatrixMapStatic::StaticTakt(int ms) {
    DTRACE();

    if (IsAblaze()) {
        DCP();
        int ttl = GetAblazeTTL();
        ttl -= ms;
        if (ttl <= 0) {
            ttl = 0;
            UnmarkAblaze();
        }
        SetAblazeTTL(ttl);
    }
    DCP();
    if (IsShorted()) {
        DCP();
        int ttl = GetShortedTTL();
        ttl -= ms;
        if (ttl <= 0) {
            ttl = 0;
            UnmarkShorted();
            if (IsRobot())
                AsRobot()->SwitchAnimation(ANIMATION_STAY);
        }
        SetShortedTTL(ttl);
    }

    DCP();

    LogicTakt(ms);

    DCP();
}

void CMatrixMapStatic::RecalcTerainColor(void) {
    DTRACE();

    D3DXVECTOR3 vmin, vmax;

    if (this->CalcBounds(vmin, vmax))
        return;

    if (GetObjectType() == OBJECT_TYPE_MAPOBJECT && (((CMatrixMapObject *)this)->m_BurnTimeTotal > 0))
        return;
    DWORD tc = g_MatrixMap->GetColor((vmin.x + vmax.x) * 0.5f, (vmin.y + vmax.y) * 0.5f);
    m_Core->m_TerainColor = LIC(tc, g_MatrixMap->m_Terrain2ObjectTargetColor, g_MatrixMap->m_Terrain2ObjectInfluence);
}

void CMatrixMapStatic::JoinToGroup(void) {
    DTRACE();

    D3DXVECTOR3 vmin, vmax;
    if (this->CalcBounds(vmin, vmax))
        return;

    if (this->IsBase()) {
        D3DXVECTOR3 t(vmin.x, vmin.y, 0);
        m_Core->m_GeoCenter = (t + vmax) * 0.5f;
        (*(D3DXVECTOR2 *)&m_Core->m_GeoCenter) +=
                ((*(D3DXVECTOR2 *)&m_Core->m_GeoCenter) - ((CMatrixBuilding *)this)->m_Pos);
    }
    else {
        m_Core->m_GeoCenter = (vmin + vmax) * 0.5f;
    }

    auto tmp = vmax - vmin;
    m_Core->m_Radius = D3DXVec3Length(&tmp) * 0.5f;

    DWORD tc = g_MatrixMap->GetColor(m_Core->m_GeoCenter.x, m_Core->m_GeoCenter.y);
    if (!IsFlyer()) {
        m_Core->m_TerainColor =
                LIC(tc, g_MatrixMap->m_Terrain2ObjectTargetColor, g_MatrixMap->m_Terrain2ObjectInfluence);
    }
    else {
        m_Core->m_TerainColor = 0xFFFFFFFF;
    }

    // calc shadow bounds

    if (GetObjectType() == OBJECT_TYPE_MAPOBJECT) {
        D3DXVECTOR3 startpos(m_Core->m_GeoCenter.x, m_Core->m_GeoCenter.y, vmax.z);
        m_AdditionalPoint = startpos + g_MatrixMap->m_LightMain * (vmax.z * 2);
        g_MatrixMap->Trace(&m_AdditionalPoint, startpos, m_AdditionalPoint, TRACE_LANDSCAPE | TRACE_WATER);
        AddSphereToBox(vmin, vmax, m_AdditionalPoint, 0);
    }

    // CDText::T(CStr(m_Radius), "!");

    // DDVECT(CStr((int)m_Type), vmin);
    // DDVECT(CStr((int)m_Type), vmax);

    int minx1, miny1, maxx1, maxy1;

    const float groupsize = GLOBAL_SCALE * MAP_GROUP_SIZE;
    const float _1groupsize = 1.0f / groupsize;

    minx1 = TruncFloat(vmin.x * _1groupsize);
    miny1 = TruncFloat(vmin.y * _1groupsize);

    maxx1 = TruncFloat(vmax.x * _1groupsize);
    maxy1 = TruncFloat(vmax.y * _1groupsize);

    // coord * matrix / groupsize
    //    minx1 = int((min.x * m_Matrix._11 + min.y * m_Matrix._21 + m_Matrix._41) * _1groupsize);
    //    miny1 = int((min.x * m_Matrix._12 + min.y * m_Matrix._22 + m_Matrix._42) * _1groupsize);

    //    maxx1 = int((max.x * m_Matrix._11 + max.y * m_Matrix._21 + m_Matrix._41) * _1groupsize);
    //    maxy1 = int((max.x * m_Matrix._12 + max.y * m_Matrix._22 + m_Matrix._42) * _1groupsize);

    if (maxx1 < minx1) {
        // swap
        maxx1 ^= minx1;
        minx1 ^= maxx1;
        maxx1 ^= minx1;
    }
    if (maxy1 < miny1) {
        // swap
        maxy1 ^= miny1;
        miny1 ^= maxy1;
        maxy1 ^= miny1;
    }

    PCMatrixMapGroup InGroups[MAX_GROUPS_PER_OBJECT];
    int newig = 0;

    if (minx1 < 0) {
        minx1 = 0;
        if (0 >= maxx1)
            goto skip;
    }
    if (maxx1 >= g_MatrixMap->m_GroupSize.x) {
        maxx1 = g_MatrixMap->m_GroupSize.x - 1;
        if (maxx1 <= minx1)
            goto skip;
    }
    if (miny1 < 0) {
        miny1 = 0;
        if (0 >= maxy1)
            goto skip;
    }
    if (maxy1 >= g_MatrixMap->m_GroupSize.y) {
        maxy1 = g_MatrixMap->m_GroupSize.y - 1;
        if (maxy1 <= miny1)
            goto skip;
    }

    for (int x = minx1; x <= maxx1; ++x) {
        for (int y = miny1; y <= maxy1; ++y) {
            PCMatrixMapGroup g = g_MatrixMap->GetGroupByIndex(x, y);
            if (g == NULL)
                continue;

            bool already = false;
            int ii = 0;
            while (ii < m_InCnt) {
                if (m_InGroups[ii] == g) {
                    already = true;
                    m_InGroups[ii] = m_InGroups[--m_InCnt];
                    continue;
                }
                ++ii;
            }
            InGroups[newig++] = g;
#ifdef _DEBUG
            if (newig > MAX_GROUPS_PER_OBJECT) {
                debugbreak();
            }
#endif
            if (!already) {
                if (g->IsBaseOn())
                    ++m_NearBaseCnt;
                g->AddObject(this);
                if (!IsFlyer()) {
                    if (IsRobot()) {
                        g->AddNewZObjRobots(vmax.z);
                    }
                    else {
                        g->AddNewZObj(vmax.z);
                    }
                }
            }
        }
    }

    UnjoinGroup();

    if (newig > 0) {
        memcpy(m_InGroups, InGroups, sizeof(PCMatrixMapGroup) * newig);
        m_InCnt = newig;
    }
    else {
    skip:;
        // flyer? above water?
        // aaaa! Carrying robot :)
    }
}

void CMatrixMapStatic::UnjoinGroup(void) {
    int ii = m_InCnt;
    while (ii > 0) {
        m_InGroups[--ii]->SubObject(this);
        m_InGroups[ii]->RecalcMaxZ();
        if (m_InGroups[ii]->IsBaseOn())
            --m_NearBaseCnt;
    }
    m_InCnt = 0;
}

void CMatrixMapStatic::ProceedLogic(int takts) {
    //__int64 tv[7];
    // int tc[7];
    // for(int i=0;i<7;i++) { tv[i]=0; tc[i]=0; }

    DTRACE();
    CMatrixMapStatic *ms;
    ms = m_FirstLogicTemp;
    while (ms) {
        g_MatrixMap->m_NextLogicObject = ms->m_NextLogicTemp;
        if (ms != g_MatrixMap->GetPlayerSide()->GetArcadedObject()) {
            //__int64 t1,t2;
            // EObjectType ot=ms->GetObjectType();
            // QueryPerformanceCounter((LARGE_INTEGER *)(&t1));
            ms->StaticTakt(takts);
            // QueryPerformanceCounter((LARGE_INTEGER *)(&t2));
            // tv[ot]+=t2-t1;
            // tc[ot]++;
        }
        ms = g_MatrixMap->m_NextLogicObject;
    }
    //__int64 freq;
    // QueryPerformanceFrequency((LARGE_INTEGER *)(&freq));
    // DM(L"ProceedLogic",std::wstring().Format(L"E=<p=6><d>(<i>) O=<d>(<i>) R=<d>(<i>) B=<d>(<i>) C=<d>(<i>) F=<d>(<i>)",
    //        double(((long double)(tv[0]))/((long double)(freq))),tc[0],
    //        double(((long double)(tv[2]))/((long double)(freq))),tc[2],
    //        double(((long double)(tv[3]))/((long double)(freq))),tc[3],
    //        double(((long double)(tv[4]))/((long double)(freq))),tc[4],
    //        double(((long double)(tv[5]))/((long double)(freq))),tc[5],
    //        double(((long double)(tv[6]))/((long double)(freq))),tc[6]
    //   ).Get());
}

inline DWORD ARGB2ABGR(DWORD c) {
    if ((c & 0x00FF00FF) == 0x00FF00FF)
        return 0;
    return (0xFF000000) | ((c & 0xFF00FF00) | ((c & 0x00FF0000) >> 16) | ((c & 0x000000FF) << 16));
}

bool CMatrixMapStatic::RenderToTexture(SRenderTexture *rt, int n, /*float *fff,*/ float anglez, float anglex,
                                       float fov) {
    DTRACE();
#define RENDSZ 256

    for (int i = 0; i < n; rt[i++].tex = NULL)
        ;

    IDirect3DSurface9 *newTarget;
    if (D3D_OK !=
        g_D3DD->CreateRenderTarget(RENDSZ, RENDSZ, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, TRUE, &newTarget, 0)) {
        return false;
    }

    IDirect3DSurface9 *newZ, *oldZ;
    bool CustomZ = (RENDSZ > g_ScreenY) || (RENDSZ > g_ScreenX);
    if (CustomZ) {
        D3DSURFACE_DESC d;
        g_D3DD->GetDepthStencilSurface(&oldZ);
        oldZ->GetDesc(&d);
        if (D3D_OK != g_D3DD->CreateDepthStencilSurface(RENDSZ, RENDSZ, d.Format, d.MultiSampleType,
                                                        d.MultiSampleQuality, TRUE, &newZ, NULL)) {
            newTarget->Release();
            return NULL;
        }
        g_D3DD->SetDepthStencilSurface(newZ);
        newZ->Release();
    }

    D3DLIGHT9 light;
    ZeroMemory(&light, sizeof(D3DLIGHT9));
    light.Type = D3DLIGHT_DIRECTIONAL;
    light.Diffuse.r = 1.0f;
    light.Diffuse.g = 1.0f;
    light.Diffuse.b = 1.0f;
    light.Ambient.r = 0.0f;
    light.Ambient.g = 0.0f;
    light.Ambient.b = 0.0f;
    light.Specular.r = 1.0f;
    light.Specular.g = 1.0f;
    light.Specular.b = 1.0f;
    //	light.Direction	= D3DXVECTOR3(1, -1, 0);
    light.Direction = D3DXVECTOR3(-0.82242596f, 0.56887215f, 0);

    ASSERT_DX(g_D3DD->SetLight(0, &light));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, 0xFFD0D0D0));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, TRUE));
    g_D3DD->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    g_D3DD->SetRenderState(D3DRS_ZENABLE, TRUE);
    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    g_D3DD->LightEnable(0, true);

    D3DXMATRIX matWorld, matView, matProj;
    D3DVIEWPORT9 ViewPort;

    ZeroMemory(&ViewPort, sizeof(D3DVIEWPORT9));

    ViewPort.X = 0;
    ViewPort.Y = 0;
    ViewPort.Width = RENDSZ;
    ViewPort.Height = RENDSZ;

    ViewPort.MinZ = 0.0f;
    ViewPort.MaxZ = 1.0f;

    ASSERT_DX(g_D3DD->SetViewport(&ViewPort));

    // store old render target
    IDirect3DSurface9 *oldTarget;
    ASSERT_DX(g_D3DD->GetRenderTarget(0, &oldTarget));
    ASSERT_DX(g_D3DD->SetRenderTarget(0, newTarget));

    if (FLAG(g_Flags, GFLAG_STENCILAVAILABLE)) {
        ASSERT_DX(g_D3DD->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, 1.0f, 0));
    }
    else {
        ASSERT_DX(g_D3DD->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0));
    }

    ASSERT(IsRobot());

    RNeed(MR_Graph | MR_Matrix);

    float h = AsRobot()->GetChassisHeight();
    float ra = h;
    /*
        if (fff)
        {
            h += fff[0];
            ra += fff[1];
            h += fff[2];
            ra += fff[3];
        }
    */

    CMatrixRobotAI *r = AsRobot();
    for (int i = 0; i < r->m_UnitCnt; ++i) {
        if (r->m_Unit[i].m_Type == MRT_CHASSIS) {
            switch (r->m_Unit[i].u1.s1.m_Kind) {
                case RUK_CHASSIS_ANTIGRAVITY:
                    h -= 5;
                    ra -= 5.5;
                    break;
                case RUK_CHASSIS_HOVERCRAFT:
                    h -= 7;
                    break;
                case RUK_CHASSIS_PNEUMATIC:
                    h -= 9;
                    ra -= 1;
                    break;
                case RUK_CHASSIS_TRACK:
                    h -= 5;
                    // ra -= 1;
                    break;
                case RUK_CHASSIS_WHEEL:
                    h -= 8;
                    ra += 3.5f;
                    break;
            }
            break;
        }
    }
    for (int i = 0; i < r->m_UnitCnt; ++i) {
        if (r->m_Unit[i].m_Type == MRT_ARMOR) {
            switch (r->m_Unit[i].u1.s1.m_Kind) {
                case RUK_ARMOR_ACTIVE:
                    h += 9.0f;
                    ra += 5.5f;
                    break;
                case RUK_ARMOR_FIREPROOF:
                    h += 5.5f;
                    ra += 3.0f;
                    break;
                case RUK_ARMOR_NUCLEAR:
                    h += 13.5;
                    ra += 6;
                    break;
                case RUK_ARMOR_PASSIVE:
                    h += 7.0f;
                    ra += 2.5f;
                    break;
                case RUK_ARMOR_PLASMIC:
                    h += 10.0f;
                    ra += 5.5f;
                    break;
                case RUK_ARMOR_6:
                    h += 7.5f;
                    ra += 6.5f;
                    break;
            }
            break;
        }
    }

    float cdist = float(ra * 0.8 / tan(fov / 2.0));

    D3DXVECTOR3 fc(cdist * TableSin(anglez), cdist * TableCos(anglez), TableSin(anglex) * cdist + h);
    // D3DXVECTOR3 fc(80, -30, h+5);

    D3DXVECTOR3 right;
    auto tmp = D3DXVECTOR3(0, 0, 1);
    D3DXVec3Cross(&right, &fc, &tmp);
    D3DXVec3Normalize(&right, &right);

    auto tmp1 = D3DXVECTOR3(-right.x * 3, -right.y * 3, h);
    auto tmp2 = D3DXVECTOR3(0, 0, 1.0f);
    D3DXMatrixLookAtLH(&matView, &fc, &tmp1, &tmp2);

    D3DXMatrixPerspectiveFovLH(&matProj, fov, 1.0f, 1.0f, 500.0f);
    matView = m_Core->m_IMatrix * matView;

    // D3DXMATRIX mt = r->m_Unit[0].m_Matrix * matView;

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_VIEW, &matView));
    ASSERT_DX(g_D3DD->SetTransform(D3DTS_PROJECTION, &matProj));

    // Render

    ASSERT_DX(g_D3DD->BeginScene());

    AsRobot()->SetInterfaceDraw(true);
    //    g_MatrixMap->m_Camera.SetFrustumCenter(*D3DXVec3TransformCoord(&fc, &fc, &m_Core->m_Matrix));

    D3DXMATRIX imatView;
    D3DXMatrixInverse(&imatView, NULL, &matView);

    g_MatrixMap->m_Camera.SetDrawNowParams(imatView, *D3DXVec3TransformCoord(&fc, &fc, &m_Core->m_Matrix));

    CVectorObject::DrawBegin();
    r->m_Unit[0].m_Graph->SetAnimByName(ANIMATION_NAME_STAY);
    r->m_Unit[0].m_Graph->FirstFrame();
    Draw();
    CVectorObject::DrawEnd();

    AsRobot()->SetInterfaceDraw(false);
    // Return old
    g_MatrixMap->m_Camera.RestoreCameraParams();

    ASSERT_DX(g_D3DD->EndScene());

    ASSERT_DX(g_D3DD->SetRenderTarget(0, oldTarget));

    bool ok = true;
    {
        int curt = 0;

        CBitmap bm(g_CacheHeap);
        CBitmap src(g_CacheHeap);

        D3DLOCKED_RECT lor;
        newTarget->LockRect(&lor, NULL, 0);

        src.CreateRGBA(RENDSZ, RENDSZ, lor.Pitch, lor.pBits);
#if RENDSZ == 512
        src.Make2xSmaller(CPoint(0, 0), src.Size(), bm);
#else
        bm.CreateRGBA(RENDSZ, RENDSZ);
        memcpy(bm.Data(), src.Data(), RENDSZ * RENDSZ * 4);
#endif

        newTarget->UnlockRect();

        ETexSize tsz = TEXSIZE_256;

        for (;;) {
            if (rt[curt].ts >= tsz) {
                int sz = ConvertTexSize(tsz);

                rt[curt].tex = CACHE_CREATE_TEXTUREMANAGED();

                if (D3D_OK != rt[curt].tex->CreateLock(D3DFMT_A8R8G8B8, sz, sz, 1, lor)) {
                    g_Cache->Destroy(rt[curt].tex);
                    rt[curt].tex = NULL;

                    ok = false;
                    goto end;
                }
                memcpy(lor.pBits, bm.Data(), sz * sz * 4);
                rt[curt].tex->UnlockRect();
                ++curt;
                if (curt >= n)
                    goto end;
            }

            bm.Make2xSmaller();

            tsz = ETexSize((int)tsz - 1);
        }
    }

end:;
    newTarget->Release();
    oldTarget->Release();
    if (CustomZ) {
        g_D3DD->SetDepthStencilSurface(oldZ);
        oldZ->Release();
    }

    ZeroMemory(&light, sizeof(D3DLIGHT9));
    light.Type = D3DLIGHT_DIRECTIONAL;
    light.Diffuse.r = GetColorR(g_MatrixMap->m_LightMainColorObj);
    light.Diffuse.g = GetColorG(g_MatrixMap->m_LightMainColorObj);
    light.Diffuse.b = GetColorB(g_MatrixMap->m_LightMainColorObj);
    light.Ambient.r = 0.0f;
    light.Ambient.g = 0.0f;
    light.Ambient.b = 0.0f;
    light.Specular.r = GetColorR(g_MatrixMap->m_LightMainColorObj);
    light.Specular.g = GetColorG(g_MatrixMap->m_LightMainColorObj);
    light.Specular.b = GetColorB(g_MatrixMap->m_LightMainColorObj);
    light.Direction = g_MatrixMap->m_LightMain;
    ASSERT_DX(g_D3DD->SetLight(0, &light));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, g_MatrixMap->m_AmbientColorObj));

    g_D3DD->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, FALSE));
    return ok;
}

#ifdef SHOW_ASSIGNED_GROUPS
void CMatrixMapStatic::ShowGroups(void) {
    for (int ii = 0; ii < m_InCnt; ++ii) {
        if (m_InGroups[ii]) {
#define v2v3(v) D3DXVECTOR3(v.x, v.y, 10)
            CHelper::Create(1, 0)->Triangle(v2v3(m_InGroups[ii]->GetPos0()),
                                            D3DXVECTOR3(m_InGroups[ii]->GetPos1().x, m_InGroups[ii]->GetPos0().y, 10),
                                            v2v3(m_InGroups[ii]->GetPos1()), 0x80FF0000);

            CHelper::Create(1, 0)->Triangle(v2v3(m_InGroups[ii]->GetPos0()), v2v3(m_InGroups[ii]->GetPos1()),
                                            D3DXVECTOR3(m_InGroups[ii]->GetPos0().x, m_InGroups[ii]->GetPos1().y, 10),
                                            0x80FF0000);
        }
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////////

// sorting mechanics

PCMatrixMapStatic CMatrixMapStatic::objects[MAX_OBJECTS_PER_SCREEN];
int CMatrixMapStatic::objects_left;
int CMatrixMapStatic::objects_rite;

void CMatrixMapStatic::SortBegin(void) {
    objects_left = MAX_OBJECTS_PER_SCREEN >> 1;
    objects_rite = MAX_OBJECTS_PER_SCREEN >> 1;
}

void CMatrixMapStatic::SortEndRecalcTerainColor(void) {
    DTRACE();

    for (int i = objects_left; i < objects_rite; ++i) {
        objects[i]->RecalcTerainColor();
    }
}
void CMatrixMapStatic::CalcDistances(void) {
    for (int i = objects_left; i < objects_rite; ++i) {
        // calc m_CamDistSq
        auto tmp = g_MatrixMap->m_Camera.GetFrustumCenter() - objects[i]->m_Core->m_GeoCenter;
        objects[i]->m_CamDistSq =
                D3DXVec3LengthSq(&tmp);
    }
}

void CMatrixMapStatic::SortEndGraphicTakt(int step) {
    DTRACE();

    for (int i = objects_left; i < objects_rite; ++i) {
        // calc m_CamDistSq
        auto tmp = g_MatrixMap->m_Camera.GetFrustumCenter() - objects[i]->m_Core->m_GeoCenter;
        objects[i]->m_CamDistSq =
                D3DXVec3LengthSq(&tmp);
        objects[i]->Takt(step);
    }
}

void CMatrixMapStatic::SortEndDraw(void) {
    DTRACE();

    for (int i = objects_left; i < objects_rite; ++i) {
        objects[i]->Draw();
    }
}

void CMatrixMapStatic::SortEndBeforeDraw(void) {
    DTRACE();

    for (int i = objects_left; i < objects_rite; ++i) {
        objects[i]->m_RemindCore.Use(FREE_TIME_PERIOD);
        objects[i]->BeforeDraw();
    }
}

void CMatrixMapStatic::SortEndDrawShadowProj(void) {
    DTRACE();

    for (int i = objects_left; i < objects_rite; ++i) {
        if (objects[i]->m_CamDistSq < DRAW_SHADOWS_DISTANCE_SQ)
            objects[i]->DrawShadowProj();
    }
}

void CMatrixMapStatic::SortEndDrawShadowStencil(void) {
    DTRACE();

    for (int i = objects_left; i < objects_rite; ++i) {
        if (objects[i]->m_CamDistSq < DRAW_SHADOWS_DISTANCE_SQ)
            objects[i]->DrawShadowStencil();
    }
}

void CMatrixMapStatic::Sort(const D3DXMATRIX &sort) {
    if (FLAG(m_ObjectState, OBJECT_STATE_INVISIBLE))
        return;

    if (g_MatrixMap->GetCurrentFrame() == m_LastVisFrame) {
        return;
    }  // already sorted
    m_LastVisFrame = g_MatrixMap->GetCurrentFrame();

    // check visibility with frustum...

    if (IsFlyer()) {
        if (!g_MatrixMap->m_Camera.IsInFrustum(GetGeoCenter(), GetRadius()))
            return;
    }
    else {
        if (!g_MatrixMap->m_Camera.IsInFrustum(m_AdditionalPoint)) {
            if (!g_MatrixMap->m_Camera.IsInFrustum(GetGeoCenter(), GetRadius()))
                return;
        }
    }

#if SHOW_ASSIGNED_GROUPS
    if (SHOW_ASSIGNED_GROUPS == GetObjectType())
        ShowGroups();
#endif

    bool noleft = (objects_left <= 0);
    bool norite = (objects_rite >= MAX_OBJECTS_PER_SCREEN);

    m_Z = Double2Int((sort._13 * m_Core->m_GeoCenter.x + sort._23 * m_Core->m_GeoCenter.y +
                      sort._33 * m_Core->m_GeoCenter.z + sort._43) *
                     256.0);

    if (objects_left == objects_rite) {
        objects[objects_left] = this;
        ++objects_rite;
        return;
    }
    // seek index
    int idx;
    int idx0 = objects_left;
    int idx1 = objects_rite;
    for (;;) {
        idx = ((idx1 - idx0) >> 1) + idx0;

        if (m_Z < objects[idx]->m_Z) {
            // left
            if (idx == idx0)
                break;
            idx1 = idx;
        }
        else {
            // rite
            ++idx;
            if (idx == idx1)
                break;
            idx0 = idx;
        }
    }

    if (noleft && norite) {
        if (idx == objects_rite)
            return;  // far object
        --objects_rite;
        norite = false;
        goto insert;
    }

    if (!norite && (idx == objects_rite)) {
        ++objects_rite;
        objects[idx] = this;
    }
    else if (!noleft && (idx == objects_left)) {
        --objects_left;
        objects[idx - 1] = this;
    }
    else {
    insert:
        int lc = (idx - objects_left);
        int rc = (objects_rite - idx);
        bool expand_left = norite || ((lc <= rc) && !noleft);

        if (expand_left) {
            memcpy(&objects[objects_left - 1], &objects[objects_left], sizeof(PCMatrixMapStatic) * lc);
            --objects_left;
            objects[idx - 1] = this;
        }
        else {
            memcopy_back_dword(&objects[idx + 1], &objects[idx], rc);
            ++objects_rite;
            objects[idx] = this;
        }
    }

    WillDraw();
}

CMatrixMapStatic *CMatrixMapStatic::GetVisObj(int i) {
    return objects[objects_left + i];
}

int CMatrixMapStatic::GetVisObjCnt(void) {
    return objects_rite - objects_left;
}

void CMatrixMapStatic::RemoveFromSorted(CMatrixMapStatic *ms) {
    for (int i = objects_left; i < objects_rite; ++i) {
        if (objects[i] == ms) {
            objects[i] = objects[--objects_rite];
            return;
        }
    }
}

bool CMatrixMapStatic::EnumVertsHandler(const SVOVertex &v, DWORD data) {
    CMatrixMapStatic::SEVH_data *d = (CMatrixMapStatic::SEVH_data *)data;
    D3DXVECTOR3 p;
    D3DXVec3TransformCoord(&p, &v.v, &d->m);
    if (d->rect->IsInRect(CPoint(Float2Int(p.x), Float2Int(p.y)))) {
        d->found = true;
        return true;
    }
    return false;
}

CMatrixMapStatic *CMatrixMapStatic::m_FirstVisNew;
CMatrixMapStatic *CMatrixMapStatic::m_LastVisNew;

CMatrixMapStatic *CMatrixMapStatic::m_FirstVisOld;
CMatrixMapStatic *CMatrixMapStatic::m_LastVisOld;

void CMatrixMapStatic::OnEndOfDraw(void) {
    // TODO :
    return;
    CMatrixMapStatic *f = m_FirstVisOld;
    for (; f;) {
        CMatrixMapStatic *n = f->m_NextVis;

        if (f->GetObjectType() == OBJECT_TYPE_MAPOBJECT) {
            ((CMatrixMapObject *)f)->OnOutScreen();
        }
        else if (f->GetObjectType() == OBJECT_TYPE_FLYER) {
            ((CMatrixFlyer *)f)->OnOutScreen();
        }
        else if (f->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
            ((CMatrixRobotAI *)f)->OnOutScreen();
        }
        else if (f->GetObjectType() == OBJECT_TYPE_BUILDING) {
            ((CMatrixBuilding *)f)->OnOutScreen();
        }
        f->m_NextVis = NULL;
        f->m_PrevVis = NULL;

        f = n;
    }

    m_FirstVisOld = m_FirstVisNew;
    m_LastVisOld = m_LastVisNew;
    m_FirstVisNew = NULL;
    m_LastVisNew = NULL;
}

bool CMatrixMapStatic::IsLiveCannon(void) const {
    return IsCannon() && ((CMatrixCannon *)this)->m_CurrState != CANNON_DIP;
}

bool CMatrixMapStatic::IsLiveActiveCannon(void) const {
    return IsCannon() && ((CMatrixCannon *)this)->m_CurrState != CANNON_DIP &&
           ((CMatrixCannon *)this)->m_CurrState != CANNON_UNDER_CONSTRUCTION;
}

bool CMatrixMapStatic::IsBase(void) const {
    if (GetObjectType() == OBJECT_TYPE_BUILDING) {
        if (((CMatrixBuilding *)this)->m_Kind == BUILDING_BASE)
            return true;
    }
    return false;
}

bool CMatrixMapStatic::IsLiveBuilding(void) const {
    return IsBuilding() && ((CMatrixBuilding *)this)->m_State != BUILDING_DIP &&
           ((CMatrixBuilding *)this)->m_State != BUILDING_DIP_EXPLODED;
}
