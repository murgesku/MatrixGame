// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <algorithm>

#include "MatrixMap.hpp"
#include "MatrixShadowManager.hpp"
#include "MatrixObjectCannon.hpp"
#include "MatrixRobot.hpp"
#include "MatrixObjectBuilding.hpp"
#include "MatrixFlyer.hpp"
#include "ShadowStencil.hpp"
#include "Interface/CInterface.h"

#include "Effects/MatrixEffectExplosion.hpp"

float CMatrixCannon::GetSeekRadius(void) {
    return g_Config.m_CannonsProps[m_Num - 1].seek_radius;
}

float CMatrixCannon::GetStrength(void) {
    return g_Config.m_CannonsProps[m_Num - 1].m_Strength * (0.4f + 0.6f * (m_HitPoint / m_HitPointMax));
}

void CMatrixCannon::FireHandler(CMatrixMapStatic *hit, const D3DXVECTOR3 &pos, DWORD user, DWORD flags) {
    SObjectCore *oc = (SObjectCore *)user;

    if (oc->m_Object && FLAG(flags, FEHF_DAMAGE_ROBOT)) {
        // попадание!
        // обновим тайминг косой стрельбы
        oc->m_Object->AsCannon()->m_TimeFromFire = CANNON_TIME_FROM_FIRE;
    }

    if (oc->m_Object && oc->m_Object->AsCannon()->IsRefProtect()) {
        oc->m_Object->AsCannon()->SetRefProtectHit();
    }
    else {
        if (FLAG(flags, FEHF_LASTHIT)) {
            oc->Release();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CMatrixCannon::CMatrixCannon(void) : CMatrixMapStatic(), m_UnderAttackTime(0) {
    DTRACE();
    m_Core->m_Type = OBJECT_TYPE_CANNON;

    m_ShadowProj = NULL;

    ZeroMemory(&m_Pos, sizeof(D3DXVECTOR2));
    m_Side = 0;

    m_UnitCnt = 0;
    ZeroMemory(m_Unit, sizeof(m_Unit));

    m_WeaponCnt = 0;

    m_ShadowType = SHADOW_STENCIL;
    m_ShadowSize = 128;

    InitMaxHitpoint(8000);

    m_TargetCore = NULL;

    m_AngleX = 0;
    m_Angle = 0;

    memset(&m_Weapons, 0, sizeof(m_Weapons));

    m_ParentBuilding = NULL;

    m_Place = -1;

    m_PB.Modify(1000000, 0, PB_CANNON_WIDTH, 1);

    m_FireNextThinkTime = g_MatrixMap->GetTime() + CANNON_FIRE_THINK_PERIOD;

    m_NullTargetTime = 0;
    m_TimeFromFire = CANNON_TIME_FROM_FIRE;

    m_CurrState = CANNON_IDLE;
    m_TargetDisp = D3DXVECTOR3(0, 0, 0);
    m_ShowHitpointTime = 0;

    RESETFLAG(m_ObjectState, OBJECT_CANNON_REF_PROTECTION);
    RESETFLAG(m_ObjectState, OBJECT_CANNON_REF_PROTECTION_HIT);

    m_AddH = 0;
    m_LastDelayDamageSide = 0;
    m_MiniMapFlashTime = 0;
    m_NextTimeAblaze = 0;
}

CMatrixCannon::~CMatrixCannon() {
    DTRACE();
    ReleaseMe();
    UnitClear();
    if (m_ShadowProj) {
        HDelete(CMatrixShadowProj, m_ShadowProj, g_MatrixHeap);
        m_ShadowProj = NULL;
    }

    for (int i = 0; i < m_WeaponCnt; ++i) {
        if (m_Weapons[i]) {
            m_Weapons[i]->Release();
        }
    }
    if (m_TargetCore) {
        m_TargetCore->Release();
    }
}

void CMatrixCannon::UnitClear(void) {
    DTRACE();

    for (int i = 0; i < m_UnitCnt; ++i) {
        if (m_Unit[i].m_Graph) {
            UnloadObject(m_Unit[i].m_Graph, NULL);
            m_Unit[i].m_Graph = NULL;
        }
        if (m_CurrState == CANNON_DIP) {
            if (m_Unit[i].m_Smoke.effect) {
                ((CMatrixEffectSmoke *)m_Unit[i].m_Smoke.effect)->SetTTL(1000);
                m_Unit[i].m_Smoke.Unconnect();
            }
        }
        else {
            if (m_Unit[i].u1.s1.m_ShadowStencil) {
                HDelete(CVOShadowStencil, m_Unit[i].u1.s1.m_ShadowStencil, NULL);
                m_Unit[i].u1.s1.m_ShadowStencil = NULL;
            }
        }
    }
    ZeroMemory(m_Unit, sizeof(m_Unit));

    RChange(MR_ShadowStencil | MR_ShadowProjGeom | MR_ShadowProjTex | MR_Graph);
}

void CMatrixCannon::BoundGet(D3DXVECTOR3 &bmin, D3DXVECTOR3 &bmax) {
    DTRACE();
    bmin = D3DXVECTOR3(1e30f, 1e30f, 1e30f);
    bmax = D3DXVECTOR3(-1e30f, -1e30f, -1e30f);

    D3DXVECTOR3 tmin;
    D3DXVECTOR3 tmax;
    D3DXVECTOR3 v[8];

    for (int i = 0; i < m_UnitCnt; i++) {
        m_Unit[i].m_Graph->GetBound(tmin, tmax);

        v[0] = D3DXVECTOR3(tmin.x, tmin.y, tmin.z);
        v[1] = D3DXVECTOR3(tmax.x, tmin.y, tmin.z);
        v[2] = D3DXVECTOR3(tmax.x, tmax.y, tmin.z);
        v[3] = D3DXVECTOR3(tmin.x, tmax.y, tmin.z);
        v[4] = D3DXVECTOR3(tmin.x, tmin.y, tmax.z);
        v[5] = D3DXVECTOR3(tmax.x, tmin.y, tmax.z);
        v[6] = D3DXVECTOR3(tmax.x, tmax.y, tmax.z);
        v[7] = D3DXVECTOR3(tmin.x, tmax.y, tmax.z);

        for (int u = 0; u < 8; u++) {
            D3DXVec3TransformCoord(&(v[u]), &(v[u]), &(m_Unit[i].m_Matrix));
            bmin.x = std::min(bmin.x, v[u].x);
            bmin.y = std::min(bmin.y, v[u].y);
            bmin.z = std::min(bmin.z, v[u].z);
            bmax.x = std::max(bmax.x, v[u].x);
            bmax.y = std::max(bmax.y, v[u].y);
            bmax.z = std::max(bmax.z, v[u].z);
        }
    }
}

void CMatrixCannon::RNeed(DWORD need) {
    DTRACE();

    if (m_CurrState == CANNON_DIP)
        return;

    if (need & m_RChange & (MR_Graph)) {
        m_RChange &= ~MR_Graph;

        ASSERT(m_UnitCnt == 0);

        float hp = (float)g_Config.m_CannonsProps[m_Num - 1].m_Hitpoint;
        InitMaxHitpoint(hp);

        std::wstring ctype = utils::format(L"%d", m_Num);

        // basis
        m_Unit[0].m_Type = CUT_BASIS;
        m_Unit[0].m_Graph =
                LoadObject((std::wstring(OBJECT_PATH_CANNONS) + L"Basis.vo").c_str(), g_MatrixHeap, true);

        m_UnitCnt = 0;
        for (;;) {
            const wchar *nam = m_Unit[m_UnitCnt].m_Graph->VO()->GetMatrixNameById(20);
            if (nam == NULL)
                break;

            ++m_UnitCnt;

            ECannonUnitType type = CUT_EMPTY;
            if (*nam == 'T')
                type = CUT_TURRET;
            else if (*nam == 'S')
                type = CUT_SHAFT;

            if (type == CUT_EMPTY)
                ERROR_E;

            m_Unit[m_UnitCnt].m_Type = type;

            m_Unit[m_UnitCnt].m_Graph = LoadObject(
                    (std::wstring(OBJECT_PATH_CANNONS) + nam + ctype + L".vo").c_str(), g_MatrixHeap, true);

            if (type == CUT_SHAFT) {
                SCannonProps *props = &g_Config.m_CannonsProps[m_Num - 1];
                m_WeaponCnt = 0;
                int n = m_Unit[m_UnitCnt].m_Graph->VO()->GetMatrixCount();
                for (int i = 0; i < n; ++i) {
                    int id = m_Unit[m_UnitCnt].m_Graph->VO()->GetMatrixId(i);
                    if (id >= 50 && id <= 59) {
                        m_FireMatrix[m_WeaponCnt] = i;
                        m_Weapons[m_WeaponCnt] = (CMatrixEffectWeapon *)CMatrixEffect::CreateWeapon(
                                D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 1), (DWORD)this->m_Core, FireHandler,
                                props->weapon);
                        m_Weapons[m_WeaponCnt]->SetOwner(this);
                        ++m_WeaponCnt;
                    }
                }
            }
        }

        ++m_UnitCnt;
    }
    if (need & m_RChange & (MR_Matrix)) {
        m_RChange &= ~MR_Matrix;

        D3DXMatrixRotationZ(&m_Core->m_Matrix, m_Angle);

        m_Core->m_Matrix._41 = m_Pos.x;
        m_Core->m_Matrix._42 = m_Pos.y;

        int px = TruncFloat(m_Pos.x * INVERT(GLOBAL_SCALE));
        int py = TruncFloat(m_Pos.y * INVERT(GLOBAL_SCALE));

        int cnt = 0;
        float rv = 0;
        SMatrixMapPoint *mp = g_MatrixMap->PointGetTest(px, py);
        if (mp) {
            cnt++;
            rv += mp->z;
        }
        mp = g_MatrixMap->PointGetTest(px + 1, py);
        if (mp) {
            cnt++;
            rv += mp->z;
        }
        mp = g_MatrixMap->PointGetTest(px, py + 1);
        if (mp) {
            cnt++;
            rv += mp->z;
        }
        mp = g_MatrixMap->PointGetTest(px + 1, py + 1);
        if (mp) {
            cnt++;
            rv += mp->z;
        }

        m_Core->m_Matrix._43 = rv / cnt + m_AddH;

        D3DXMatrixInverse(&m_Core->m_IMatrix, NULL, &m_Core->m_Matrix);

        D3DXMATRIX mx, m;
        const D3DXMATRIX *tm;
        D3DXVECTOR3 p(0, 0, 0);

        for (int i = 0; i < m_UnitCnt; i++) {
            D3DXMatrixRotationZ(&m, m_Unit[i].u1.s1.m_Angle);
            *(D3DXVECTOR3 *)&m._41 = p;

            if (m_Unit[i].m_Type == CUT_SHAFT) {
                D3DXMatrixRotationX(&mx, m_AngleX);

                mx *= m;

                D3DXMatrixMultiply(&m_Unit[i].m_Matrix, &mx, &m_Core->m_Matrix);

                for (int w = 0; w < m_WeaponCnt; ++w) {
                    const D3DXMATRIX *f = m_Unit[i].m_Graph->GetMatrix(m_FireMatrix[w]);
                    D3DXMATRIX weapm = (*f) * m_Unit[i].m_Matrix;

                    m_FireFrom[w] = *(D3DXVECTOR3 *)&weapm._41;
                    m_FireDir[w] = *(D3DXVECTOR3 *)&weapm._21;
                }
            }
            else {
                m_Unit[i].m_Matrix = m * m_Core->m_Matrix;
            }

            D3DXMatrixInverse(&m_Unit[i].u1.s1.m_IMatrix, NULL, &m_Unit[i].m_Matrix);

            if (i == (m_UnitCnt - 1)) {
                m_FireCenter = *(D3DXVECTOR3 *)&m_Unit[i].m_Matrix._41;
                break;
            }

            ASSERT(m_Unit[i].m_Graph);
            tm = m_Unit[i].m_Graph->GetMatrixById(20);

            // this only takes into account Z-axis rotation.
            // If you uses other then D3DXMatrixRotationZ rotation, please correct following code
            p.x += tm->_41 * m._11 + tm->_42 * m._21;
            p.y += tm->_41 * m._12 + tm->_42 * m._22;
            p.z += tm->_43;
        }
    }
    /*
    if(need & m_RChange & (MR_Sort)) {
        m_RChange&=~MR_Sort;

        D3DXVECTOR3 pz=D3DXVECTOR3(0,0,0);
        D3DXVec3TransformCoord(&pz,&pz,&(m_Matrix*g_MatrixMap->GetSortMatrix()));
        m_Z=pz.z;
    }
    if(need & m_RChange & (MR_GraphSort)) {
        m_RChange&=~MR_GraphSort;

        for(int i=0;i<m_UnitCnt;i++) {
            ASSERT(m_Unit[i].m_Graph);
            m_Unit[i].m_Graph->SortIndexForAlpha(m_Matrix*g_MatrixMap->GetViewMatrix());
        }
    }
    */
    if (need & m_RChange & MR_ShadowStencil) {
        m_RChange &= ~MR_ShadowStencil;

        if (m_ShadowType != SHADOW_STENCIL) {
            for (int i = 0; i < m_UnitCnt; i++) {
                if (m_Unit[i].u1.s1.m_ShadowStencil) {
                    HDelete(CVOShadowStencil, m_Unit[i].u1.s1.m_ShadowStencil, g_MatrixHeap);
                    m_Unit[i].u1.s1.m_ShadowStencil = NULL;
                }
            }
        }
        else {
            for (int i = 0; i < m_UnitCnt; i++) {
                ASSERT(m_Unit[i].m_Graph);
                ASSERT(m_Unit[i].m_Graph->VO());

                // if(m_Unit[i].m_Graph->VO()->EdgeExist()) {
                if (!m_Unit[i].u1.s1.m_ShadowStencil)
                    m_Unit[i].u1.s1.m_ShadowStencil = HNew(g_MatrixHeap) CVOShadowStencil();

                //					if(!(m_Unit[i].m_Graph->VO()->EdgeExist())) m_Unit[i].m_Graph->VO()->EdgeBuild();

                // STENCIL
                // m_Unit[i].u1.s1.m_ShadowStencil->Build(*(m_Unit[i].m_Graph->VO()),m_Unit[i].m_Graph->FrameVO(),
                //    g_MatrixMap->m_LightMain,m_Unit[i].m_Matrix,IsNearBase()?g_MatrixMap->m_ShadowPlaneCutBase:g_MatrixMap->m_ShadowPlaneCut);

                D3DXVECTOR3 light;
                D3DXVec3TransformNormal(&light, &g_MatrixMap->m_LightMain, &m_Unit[i].u1.s1.m_IMatrix);

                // D3DXPLANE cutpl;
                // D3DXPlaneTransform(&cutpl,
                // IsNearBase()?(&g_MatrixMap->m_ShadowPlaneCutBase):(&g_MatrixMap->m_ShadowPlaneCut),
                // &m_Unit[i].m_IMatrix);
                float len = (GetRadius() * 2) + m_Unit[i].m_Matrix._43 -
                            (IsNearBase() ? g_MatrixMap->m_GroundZBase : g_MatrixMap->m_GroundZ);
                m_Unit[i].u1.s1.m_ShadowStencil->Build(*(m_Unit[i].m_Graph->VO()), m_Unit[i].m_Graph->GetVOFrame(), light,
                                                 len, false);
            }
        }
    }
    if (need & m_RChange & MR_ShadowProjGeom) {
        m_RChange &= ~MR_ShadowProjGeom;

        if (m_ShadowType != SHADOW_PROJ_DYNAMIC) {
            if (m_ShadowProj) {
                HDelete(CMatrixShadowProj, m_ShadowProj, g_MatrixHeap);
                m_ShadowProj = NULL;
            }
        }
        else {
            if (!m_ShadowProj)
                m_ShadowProj = HNew(g_MatrixHeap) CMatrixShadowProj(g_MatrixHeap, this);

            BYTE *buf = (BYTE *)_alloca(sizeof(CVectorObjectAnim *) * m_UnitCnt + sizeof(int) * m_UnitCnt +
                                        sizeof(D3DXMATRIX) * m_UnitCnt);
            CVectorObjectAnim **obj = (CVectorObjectAnim **)buf;
            int *noframe = (int *)(buf + sizeof(CVectorObjectAnim *) * m_UnitCnt);
            D3DXMATRIX *wm = (D3DXMATRIX *)(buf + sizeof(CVectorObjectAnim *) * m_UnitCnt + sizeof(int) * m_UnitCnt);

            for (int i = 0; i < m_UnitCnt; i++) {
                ASSERT(m_Unit[i].m_Graph);
                ASSERT(m_Unit[i].m_Graph->VO());

                obj[i] = m_Unit[i].m_Graph;
                noframe[i] = m_Unit[i].m_Graph->GetVOFrame();
                wm[i] = m_Unit[i].m_Matrix * m_Core->m_IMatrix;
            }

            ShadowProjBuildGeomList(*m_ShadowProj, m_UnitCnt, obj, noframe, wm, m_Core->m_Matrix, m_Core->m_IMatrix,
                                    g_MatrixMap->m_LightMain, int(100 / GLOBAL_SCALE), true);

            if (!(m_ShadowProj->IsProjected())) {
                HDelete(CMatrixShadowProj, m_ShadowProj, g_MatrixHeap);
                m_ShadowProj = NULL;
            }
        }
    }
    if (need & m_RChange & MR_ShadowProjTex) {
        m_RChange &= ~MR_ShadowProjTex;

        if (m_ShadowProj != NULL) {
            BYTE *buf = (BYTE *)_alloca(sizeof(CVectorObjectAnim *) * m_UnitCnt + sizeof(int) * m_UnitCnt +
                                        sizeof(D3DXMATRIX) * m_UnitCnt);
            CVectorObjectAnim **obj = (CVectorObjectAnim **)buf;
            int *noframe = (int *)(buf + sizeof(CVectorObjectAnim *) * m_UnitCnt);
            D3DXMATRIX *wm = (D3DXMATRIX *)(buf + sizeof(CVectorObjectAnim *) * m_UnitCnt + sizeof(int) * m_UnitCnt);

            for (int i = 0; i < m_UnitCnt; i++) {
                ASSERT(m_Unit[i].m_Graph);
                ASSERT(m_Unit[i].m_Graph->VO());

                obj[i] = m_Unit[i].m_Graph;
                noframe[i] = m_Unit[i].m_Graph->GetVOFrame();
                wm[i] = m_Unit[i].m_Matrix * m_Core->m_IMatrix;
            }

            CTexture *tex = NULL;

            ShadowProjBuildTextureList(*m_ShadowProj, m_UnitCnt, obj, tex, noframe, wm, m_Core->m_Matrix,
                                       m_Core->m_IMatrix, g_MatrixMap->m_LightMain, m_ShadowSize);
        }
    }
}

void CMatrixCannon::Takt(int cms) {
    DTRACE();

    for (int i = 0; i < m_UnitCnt; i++) {
        if (m_Unit[i].m_Graph) {
            if (m_Unit[i].m_Graph->Takt(cms)) {
                if (m_ShadowType == SHADOW_STENCIL)
                    RChange(MR_ShadowStencil);
                else if (m_ShadowType == SHADOW_PROJ_DYNAMIC)
                    RChange(MR_ShadowProjTex);
            }

            if (m_Unit[i].m_Graph->IsAnimEnd())
                m_Unit[i].m_Graph->SetAnimByName(ANIMATION_NAME_IDLE);
        }
    }

    // RChange(MR_Matrix);
}

bool CMatrixCannon::Pick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, float *outt) const {
    DTRACE();
    if (m_CurrState == CANNON_DIP)
        return false;
    for (int i = 0; i < m_UnitCnt; i++) {
        if (m_Unit[i].m_Graph) {
            if (m_Unit[i].m_Graph->Pick(m_Unit[i].m_Matrix, m_Unit[i].u1.s1.m_IMatrix, orig, dir, outt))
                return true;
        }
    }
    return false;
}

void CMatrixCannon::FreeDynamicResources(void) {
    DTRACE();

    if (m_ShadowProj && (m_ShadowType == SHADOW_PROJ_DYNAMIC)) {
        HDelete(CMatrixShadowProj, m_ShadowProj, g_MatrixHeap);
        m_ShadowProj = NULL;
        RChange(MR_ShadowProjGeom);
    }
    else if (m_ShadowType == SHADOW_STENCIL) {
        for (int i = 0; i < m_UnitCnt; ++i) {
            if (m_Unit[i].u1.s1.m_ShadowStencil)
                m_Unit[i].u1.s1.m_ShadowStencil->DX_Free();
        }
    }
}

void CMatrixCannon::BeforeDraw(void) {
    DTRACE();
    DWORD sh = (g_Config.m_ShowProjShadows ? (MR_ShadowProjGeom | MR_ShadowProjTex) : 0) |
               (g_Config.m_ShowStencilShadows ? MR_ShadowStencil : 0);
    RNeed(MR_Matrix | MR_Graph | sh);

    if (m_ShowHitpointTime > 0 && m_HitPoint > 0 && m_CurrState != CANNON_DIP) {
        D3DXVECTOR3 pos(GetGeoCenter());
        if (TRACE_STOP_NONE ==
            g_MatrixMap->Trace(NULL, g_MatrixMap->m_Camera.GetFrustumCenter(), pos, TRACE_LANDSCAPE, NULL)) {
            D3DXVECTOR2 p = g_MatrixMap->m_Camera.Project(pos, g_MatrixMap->GetIdentityMatrix());
            m_PB.Modify(p.x - (PB_CANNON_WIDTH * 0.5f), p.y - GetRadius(), m_HitPoint * m_MaxHitPointInversed);
        }
    }

    for (int i = 0; i < m_UnitCnt; i++) {
        m_Unit[i].m_Graph->BeforeDraw();
        if (m_CurrState != CANNON_DIP && m_Unit[i].u1.s1.m_ShadowStencil)
            m_Unit[i].u1.s1.m_ShadowStencil->BeforeRender();
    }

    if (m_CurrState != CANNON_DIP && m_ShadowProj)
        m_ShadowProj->BeforeRender();
}

void CMatrixCannon::Draw(void) {
    // g_D3DD->SetRenderState( D3DRS_NORMALIZENORMALS,  TRUE );
    DWORD coltex = (DWORD)g_MatrixMap->GetSideColorTexture(m_Side)->Tex();

    for (int i = 0; i < 4; i++) {
        ASSERT_DX(g_D3DD->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&g_MatrixMap->m_BiasCannons))));
    }

    if (m_CurrState == CANNON_DIP) {
        g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFF808080);
        for (int i = 0; i < m_UnitCnt; i++) {
            if (m_Unit[i].u1.s2.m_TTL <= 0)
                continue;
            ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &(m_Unit[i].m_Matrix)));
            m_Unit[i].m_Graph->Draw(coltex);
        }
    }
    else {
        if (m_CurrState == CANNON_UNDER_CONSTRUCTION) {
            g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFF00FF00);
        }
        else {
            g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, m_Core->m_TerainColor);
        }
        for (int i = 0; i < m_UnitCnt; i++) {
            ASSERT(m_Unit[i].m_Graph);

            ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &(m_Unit[i].m_Matrix)));
            m_Unit[i].m_Graph->Draw(coltex);
        }
    }
    // g_D3DD->SetRenderState( D3DRS_NORMALIZENORMALS,  FALSE );

    /*
    for (int k=0; k<100; ++k)
    {
        D3DXVECTOR3 d(FSRND(1),FSRND(1),FSRND(1));
        D3DXVec3Normalize(&d, &d);
        CHelper::Create(1,0)->Line(m_GeoCenter, m_GeoCenter + d*m_Radius);
    }
    */
}

void CMatrixCannon::DrawShadowStencil(void) {
    DTRACE();

    if (m_ShadowType != SHADOW_STENCIL)
        return;

    for (int i = 0; i < m_UnitCnt; i++) {
        if (m_Unit[i].u1.s1.m_ShadowStencil) {
            m_Unit[i].u1.s1.m_ShadowStencil->Render(m_Unit[i].m_Matrix);
        }
    }
}

void CMatrixCannon::DrawShadowProj(void) {
    //   DTRACE();
    // if(!m_ShadowProj) return;
    //   D3DXMATRIX m = g_MatrixMap->GetIdentityMatrix();
    //   m._41 = m_ShadowProj->GetDX();
    //   m._42 = m_ShadowProj->GetDY();
    // ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD,&m));

    //   m_ShadowProj->Render();
}

void CMatrixCannon::OnLoad(void) {
    DTRACE();

    UnitInit(m_Num);
}

bool CMatrixCannon::CalcBounds(D3DXVECTOR3 &minv, D3DXVECTOR3 &maxv) {
    if (m_UnitCnt == 0)
        return true;

    // RChange(MR_Matrix);
    RNeed(MR_Matrix | MR_Graph);

    D3DXVECTOR3 bminout, bmaxout;

    minv.x = 1e30f;
    minv.y = 1e30f;
    minv.z = 1e30f;
    maxv.x = -1e30f;
    maxv.y = -1e30f;
    maxv.z = -1e30f;

    for (int u = 1; u < m_UnitCnt; ++u)  // skip basis
    {
        int cnt = m_Unit[u].m_Graph->VO()->GetFramesCnt();
        for (int i = 0; i < cnt; i++) {
            m_Unit[u].m_Graph->VO()->GetBound(i, m_Unit[u].m_Matrix, bminout, bmaxout);

            minv.x = std::min(minv.x, bminout.x);
            minv.y = std::min(minv.y, bminout.y);
            minv.z = std::min(minv.z, bminout.z);
            maxv.x = std::max(maxv.x, bmaxout.x);
            maxv.y = std::max(maxv.y, bmaxout.y);
            maxv.z = std::max(maxv.z, bmaxout.z);
        }
    }

    return false;

    /*	RNeed(MR_Graph|MR_Matrix);

        SVOFrame *f = m_Unit[0].m_Graph->VO()->FrameGet(0);
        minv.x = f->m_MinX;
        minv.y = f->m_MinY;
        maxv.x = f->m_MaxX;
        maxv.y = f->m_MaxY;

        for (int u = 0; u<m_UnitCnt; ++u)
        {
            int cnt = m_Unit[u].m_Graph->VO()->Header()->m_FrameCnt;
            for (int i = 1; i<cnt; ++i)
            {
                SVOFrame *f = m_Unit[u].m_Graph->VO()->FrameGet(i);

                minv.x = min(minv.x,f->m_MinX);
                minv.y = min(minv.y,f->m_MinY);
                maxv.x = max(maxv.x,f->m_MaxX);
                maxv.y = max(maxv.y,f->m_MaxY);
            }
        }

        return false;*/
}

void CMatrixCannon::DIPTakt(float ms) {
    DTRACE();

    D3DXVECTOR3 *pos;
    D3DXMATRIX *mat;

    bool del = true;
    for (int i = 0; i < m_UnitCnt; ++i) {
        if (m_Unit[i].u1.s2.m_TTL <= 0)
            continue;
        m_Unit[i].u1.s2.m_TTL -= ms;

        pos = &m_Unit[i].u1.s2.m_Pos;

        if (m_Unit[i].u1.s2.m_TTL <= 0) {
            // create explosive

            pos->z += GetRadius();
            CMatrixEffect::CreateExplosion(*pos, ExplosionRobotBoomSmall, true);
            if (m_Unit[i].m_Smoke.effect) {
                ((CMatrixEffectSmoke *)m_Unit[i].m_Smoke.effect)->SetTTL(1000);
                m_Unit[i].m_Smoke.Unconnect();
            }

            continue;
        }

        del = false;

        if (IS_ZERO_VECTOR(m_Unit[i].u1.s2.m_Velocity)) {
            continue;
        }

        D3DXVECTOR3 oldpos = *pos;

        m_Unit[i].u1.s2.m_Velocity.z -= 0.0002f * ms;
        (*pos) += m_Unit[i].u1.s2.m_Velocity * float(ms);
        mat = &m_Unit[i].m_Matrix;

        D3DXVECTOR3 hitpos;
        CMatrixMapStatic *o = g_MatrixMap->Trace(&hitpos, oldpos, *pos, TRACE_ALL, this);
        if (o == TRACE_STOP_WATER) {
            // in water
            if (g_MatrixMap->GetZ(hitpos.x, hitpos.y) < WATER_LEVEL) {
                m_Unit[i].u1.s2.m_TTL = 0;
                CMatrixEffect::CreateKonusSplash(
                        hitpos, D3DXVECTOR3(0, 0, 1), 10, 5, FSRND(M_PI), 1000, true,
                        (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_SPLASH));
                if (m_Unit[i].m_Smoke.effect) {
                    ((CMatrixEffectSmoke *)m_Unit[i].m_Smoke.effect)->SetTTL(1000);
                    m_Unit[i].m_Smoke.Unconnect();
                }
            }
        }
        else if (o == TRACE_STOP_LANDSCAPE) {
            // SND:
            m_Unit[i].u1.s2.m_Velocity = D3DXVECTOR3(0, 0, 0);
            m_Unit[i].u1.s2.m_Pos = hitpos;
        }
        else if (IS_TRACE_STOP_OBJECT(o)) {
            o->Damage(WEAPON_DEBRIS, hitpos, m_Unit[i].u1.s2.m_Velocity, 0, NULL);
            m_Unit[i].u1.s2.m_TTL = 1;
        }

        // if (pos->z < 0) pos->z = 0;

        float time = float(g_MatrixMap->GetTime());

        D3DXMATRIX m0, m1(g_MatrixMap->GetIdentityMatrix()), m2(g_MatrixMap->GetIdentityMatrix());
        const D3DXVECTOR3 &pos1 = m_Unit[i].m_Graph->VO()->GetFrameGeoCenter(m_Unit[i].m_Graph->GetVOFrame());
        *(D3DXVECTOR3 *)&m1._41 = pos1;
        *(D3DXVECTOR3 *)&m2._41 = -pos1;
        D3DXMatrixRotationYawPitchRoll(&m0, m_Unit[i].u1.s2.m_dy * time, m_Unit[i].u1.s2.m_dp * time, m_Unit[i].u1.s2.m_dr * time);
        *mat = m2 * m0 * m1;

        mat->_41 = pos->x;
        mat->_42 = pos->y;
        mat->_43 = pos->z;

        if (m_Unit[i].m_Smoke.effect) {
            ((CMatrixEffectSmoke *)m_Unit[i].m_Smoke.effect)->SetPos(*pos);
        }
    }

    if (del) {
        g_MatrixMap->StaticDelete(this);
    }
}

struct FTData {
    D3DXVECTOR3 *cdir;
    float coss;
    int side;
    float dist_cur;
    float dist_fire;
    CMatrixMapStatic **target;
    CMatrixMapStatic *skip;
};

static bool FindTarget(const D3DXVECTOR3 &center, CMatrixMapStatic *ms, DWORD user) {
    FTData *d = (FTData *)user;

    if (ms->GetSide() == d->side)
        return true;

    D3DXVECTOR3 dir(ms->GetGeoCenter() - center);

    float distc = D3DXVec3LengthSq(&dir);
    if (distc > d->dist_fire && d->dist_cur < d->dist_fire)
        return true;
    bool match = distc < d->dist_fire && d->dist_cur > d->dist_fire;
    D3DXVec3Normalize(&dir, &dir);

    float dot = D3DXVec3Dot(&dir, d->cdir);

    if (match || dot > d->coss) {
        CMatrixMapStatic *cel =
                g_MatrixMap->Trace(NULL, center, ms->GetGeoCenter(),
                                   TRACE_OBJECTSPHERE | TRACE_ROBOT | TRACE_FLYER | TRACE_LANDSCAPE, d->skip);

        if (cel == ms) {
            // ландшафт не помеха. тогда может объект за зданием или за другой батвой?
            cel = g_MatrixMap->Trace(NULL, center, ms->GetGeoCenter(), TRACE_BUILDING | TRACE_CANNON | TRACE_OBJECT,
                                     d->skip);
            if (cel == NULL) {
                d->dist_cur = distc;
                d->coss = dot;
                *d->target = ms;
            }
        }
    }
    return true;
}

void CMatrixCannon::BeginFireAnimation(void) {
    for (int i = 0; i < m_UnitCnt; i++) {
        if (m_Unit[i].m_Type == CUT_SHAFT) {
            if (m_Unit[i].m_Graph->SetAnimByNameNoBegin(ANIMATION_NAME_FIRELOOP)) {
                // not looped
                m_Unit[i].m_Graph->SetAnimByName(ANIMATION_NAME_FIRE, 0);
            }
            break;
        }
    }
}

void CMatrixCannon::EndFireAnimation(void) {
    for (int i = 0; i < m_UnitCnt; i++) {
        if (m_Unit[i].m_Type == CUT_SHAFT) {
            m_Unit[i].m_Graph->SetAnimLooped(0);
            break;
        }
    }
}
void CMatrixCannon::PauseTakt(int takt) {
    DTRACE();
    SetPBOutOfScreen();
    if (m_CurrState == CANNON_DIP)
        return;
    if (m_CurrState != CANNON_UNDER_CONSTRUCTION) {
        if (m_ShowHitpointTime > 0) {
            m_ShowHitpointTime -= takt;
            if (m_ShowHitpointTime < 0)
                m_ShowHitpointTime = 0;
        }
    }
    else {
        m_ShowHitpointTime = 1;
    }
}

void CMatrixCannon::LogicTakt(int takt) {
    DTRACE();
    SetPBOutOfScreen();

    if (m_CurrState == CANNON_DIP) {
        DIPTakt(float(takt));
        return;
    }

    if (m_MiniMapFlashTime > 0) {
        m_MiniMapFlashTime -= takt;
    }

    if (m_ParentBuilding && m_ParentBuilding->m_Side != m_Side) {
        SetSide(m_ParentBuilding->m_Side);

        CMatrixMapStatic *ms = CMatrixMapStatic::GetFirstLogic();
        while (ms) {
            if (ms->IsRobot()) {
                if (ms->AsRobot()->GetEnv()->SearchEnemy(this))
                    ms->AsRobot()->GetEnv()->RemoveFromList(this);
            }
            ms = ms->GetNextLogic();
        }
    }

    if (m_UnderAttackTime > 0) {
        m_UnderAttackTime -= takt;
        if (m_UnderAttackTime < 0)
            m_UnderAttackTime = 0;
    }

    if (m_CurrState != CANNON_UNDER_CONSTRUCTION) {
        if (m_ShowHitpointTime > 0) {
            m_ShowHitpointTime -= takt;
            if (m_ShowHitpointTime < 0)
                m_ShowHitpointTime = 0;
        }
    }
    else {
        // if (m_ParentBuilding && m_ParentBuilding->m_BS.m_Top == this)
        //{

        //    m_NextTimeAblaze += takt;
        //    float percent_done = float(m_NextTimeAblaze) / float(g_Config.m_Timings[UNIT_TURRET]);

        //    if (m_NextTimeAblaze > )
        //    SetHitPoint(GetMaxHitPoint() * percent_done);
        //}

        m_ShowHitpointTime = 1;
        return;
    }

    if (IsAblaze()) {
        while (g_MatrixMap->GetTime() > m_NextTimeAblaze) {
            m_NextTimeAblaze += OBJECT_ROBOT_ABLAZE_PERIOD_EFFECT;

            D3DXVECTOR3 dir, pos;
            float t;

            int cnt = 4;
            do {
                pos.x = m_Core->m_Matrix._41 + FSRND(m_Core->m_Radius);
                pos.y = m_Core->m_Matrix._42 + FSRND(m_Core->m_Radius);
                pos.z = m_Core->m_Matrix._43 + FRND(m_Core->m_Radius * 2);
                auto tmp = D3DXVECTOR3(m_Core->m_Matrix._41 - pos.x,
                                       m_Core->m_Matrix._42 - pos.y,
                                       m_Core->m_Matrix._43 - pos.z);
                D3DXVec3Normalize(&dir, &tmp);
            }
            while (!Pick(pos, dir, &t) && (--cnt > 0));

            if (cnt > 0) {
                CMatrixEffect::CreateFire(NULL, pos + dir * (t + 2), 100, 2500, 10, 2.5f, false);
            }

            for (int i = 0; i < OBJECT_ROBOT_ABLAZE_PERIOD_EFFECT; i += OBJECT_ROBOT_ABLAZE_PERIOD)
                if (Damage(WEAPON_ABLAZE, pos, dir, m_LastDelayDamageSide, NULL))
                    return;
        }
    }
    if (IsShorted()) {
        while (g_MatrixMap->GetTime() > m_NextTimeShorted) {
            m_NextTimeShorted += OBJECT_SHORTED_PERIOD;

            D3DXVECTOR3 d1(GetGeoCenter()), d2(GetGeoCenter()), dir, pos;
            float t;

            int cnt = 4;
            do {
                pos.x = m_Core->m_Matrix._41 + FSRND(m_Core->m_Radius);
                pos.y = m_Core->m_Matrix._42 + FSRND(m_Core->m_Radius);
                pos.z = m_Core->m_Matrix._43 + FRND(m_Core->m_Radius * 2);
                auto tmp = D3DXVECTOR3(m_Core->m_Matrix._41 - pos.x,
                                       m_Core->m_Matrix._42 - pos.y,
                                       m_Core->m_Matrix._43 - pos.z);
                D3DXVec3Normalize(&dir, &tmp);
            }
            while (!Pick(pos, dir, &t) && (--cnt > 0));
            if (cnt > 0) {
                d1 = pos + dir * t;
            }

            do {
                pos.x = m_Core->m_Matrix._41 + FSRND(m_Core->m_Radius);
                pos.y = m_Core->m_Matrix._42 + FSRND(m_Core->m_Radius);
                pos.z = m_Core->m_Matrix._43 + FRND(m_Core->m_Radius * 2);
                auto tmp = D3DXVECTOR3(m_Core->m_Matrix._41 - pos.x,
                                       m_Core->m_Matrix._42 - pos.y,
                                       m_Core->m_Matrix._43 - pos.z);
                D3DXVec3Normalize(&dir, &tmp);

                Pick(pos, dir, &t);
            }
            while (!Pick(pos, dir, &t) && (--cnt > 0));
            if (cnt > 0) {
                d2 = pos + dir * t;
                CMatrixEffect::CreateShorted(d1, d2, FRND(400) + 100);
            }
            if (Damage(WEAPON_SHORTED, pos, dir, m_LastDelayDamageSide, NULL))
                return;
        }
        return;
    }

    // cannon logic!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    SCannonProps *props = &g_Config.m_CannonsProps[m_Num - 1];

    bool itstime = false;
    int delta = m_FireNextThinkTime - g_MatrixMap->GetTime();
    if (delta < 0 || delta > CANNON_FIRE_THINK_PERIOD) {
        itstime = true;
        m_FireNextThinkTime = g_MatrixMap->GetTime() + CANNON_FIRE_THINK_PERIOD;
    }

    if (itstime)  // время пить херши :)
    {
        // Seek new target
        // seek side target

        CMatrixMapStatic *tgt = NULL;

        FTData data;
        // data.dist = props->seek_radius * props->seek_radius;
        data.coss = -1;
        data.target = &tgt;
        data.side = m_Side;
        data.skip = this;

        data.dist_fire = POW2(GetFireRadius());
        data.dist_cur = POW2(props->seek_radius);

        // seek robots and flyers

        data.cdir = m_FireDir + 0;

        m_TargetDisp = D3DXVECTOR3(0, 0, 0);

        g_MatrixMap->FindObjects(GetGeoCenter(), props->seek_radius, 1, TRACE_ROBOT | TRACE_FLYER, NULL, FindTarget,
                                 (DWORD)&data);

        if (tgt) {
            if (m_TargetCore)
                m_TargetCore->Release();
            m_TargetCore = tgt->GetCore(DEBUG_CALL_INFO);
        }
        else {
            // цель не найдена (уехала далеко наверное)

            if (m_TargetCore)
                m_TargetCore->Release();
            m_TargetCore = NULL;
        }
    }

    if (m_TargetCore == NULL) {
    no_target:
        // цели нет.

        if (m_NullTargetTime > 0) {
            // стрелять надо...

            // делаем вид, что стреляем мимо...
            m_TimeFromFire -= takt;
            if (m_TimeFromFire < 0)
                m_TimeFromFire = 0;

            // продолжаем стрельбу в течении m_NullTargetTime времени
            m_NullTargetTime -= takt;
            if (m_NullTargetTime <= 0) {
                for (int i = 0; i < m_WeaponCnt; ++i) {
                    if (m_Weapons[i]) {
                        m_Weapons[i]->FireEnd();
                    }
                }
                EndFireAnimation();
                m_NullTargetTime = 0;
                return;
            }
        }
        else {
            // все, стрельба окончена. обновим тайминг косой стрельбы
            m_TimeFromFire = CANNON_TIME_FROM_FIRE;
            m_TargetDisp = D3DXVECTOR3(0, 0, 0);
        }

        // отрабатываем логику оружия.
        bool firewas = false;
        for (int i = 0; i < m_WeaponCnt; ++i) {
            if (m_Weapons[i]) {
                SETFLAG(m_ObjectState, OBJECT_CANNON_REF_PROTECTION);
                RESETFLAG(m_ObjectState, OBJECT_CANNON_REF_PROTECTION_HIT);

                m_Weapons[i]->ResetFireCount();
                m_Weapons[i]->Takt(float(takt));
                firewas |= m_Weapons[i]->IsFireWas();
                bool hw = m_Weapons[i]->IsHitWas();
                if (hw || FLAG(m_ObjectState, OBJECT_CANNON_REF_PROTECTION_HIT)) {
                    m_TimeFromFire = CANNON_TIME_FROM_FIRE;
                }
                else {
                    m_Core->m_Ref += m_Weapons[i]->GetFireCount();
                }

                RESETFLAG(m_ObjectState, OBJECT_CANNON_REF_PROTECTION);
                RESETFLAG(m_ObjectState, OBJECT_CANNON_REF_PROTECTION_HIT);
            }
        }

        if (firewas)
            BeginFireAnimation();

        return;
    }
    else {
        // цель есть!!!
        // доварачиваем дуло

        bool matchz = false;
        bool matchx = false;

        D3DXVECTOR3 tgtpos;

        {
            tgtpos = m_TargetCore->m_GeoCenter + m_TargetDisp;
            // tgtpos.z += FSRND(5);
        }

        RChange(MR_Matrix);
        RNeed(MR_Matrix);

        float mul = 0;
        if (props->max_da == 0) {
            mul = (float)(1.0 - pow(0.995, double(takt)));
        }

        D3DXVECTOR2 dir(-(tgtpos.x - m_FireCenter.x), (tgtpos.y - m_FireCenter.y));

        float dang = (float)atan2(dir.x, dir.y);
        float cang = m_Unit[1].u1.s1.m_Angle + m_Angle;

        float da = float(AngleDist(cang, dang));

        if (props->max_da == 0) {
            if (fabs(da) < CANNNON_MIN_DANGLE) {
                matchz = true;
                da = 0;
                m_Unit[1].u1.s1.m_Angle = dang;
            }
            else {
                da *= mul;
                m_Unit[1].u1.s1.m_Angle += da;
            }
        }
        else {
            if (fabs(da) < (props->max_da + 0.001)) {
                m_Unit[1].u1.s1.m_Angle += da;
                matchz = true;
            }
            else if (da < 0)
                m_Unit[1].u1.s1.m_Angle -= props->max_da;
            else
                m_Unit[1].u1.s1.m_Angle += props->max_da;
        }

        m_Unit[2].u1.s1.m_Angle = m_Unit[1].u1.s1.m_Angle;

        RChange(MR_Matrix);
        RNeed(MR_Matrix);

        dir = D3DXVECTOR2(-(tgtpos.x - m_FireCenter.x), (tgtpos.y - m_FireCenter.y));

        dang = (float)atan2((tgtpos.z - m_FireCenter.z), D3DXVec2Length(&dir));
        if (dang > props->max_top_angle)
            dang = props->max_top_angle;
        else if (dang < props->max_bottom_angle)
            dang = props->max_bottom_angle;

        da = float(AngleDist(m_AngleX, dang));

        if (props->max_da == 0) {
            if (fabs(da) < CANNNON_MIN_DANGLE) {
                matchx = true;
                da = 0;

                m_AngleX = dang;
            }
            else {
                da *= mul;
                m_AngleX += da;
            }
        }
        else {
            if (fabs(da) < (props->max_da + 0.001)) {
                m_AngleX += da;
                matchx = true;
            }
            else if (da < 0)
                m_AngleX -= props->max_da;
            else
                m_AngleX += props->max_da;
        }

        RChange(MR_Matrix | MR_ShadowStencil | MR_ShadowProjTex);
        RNeed(MR_Matrix);

        // доварачиваем оружие

        for (int i = 0; i < m_WeaponCnt; ++i) {
            if (m_Weapons[i]) {
                m_Weapons[i]->Modify(m_FireFrom[i], m_FireDir[i], D3DXVECTOR3(0, 0, 0));
            }
        }

        // а проверм-ка, надо ли стрелять....

        if (!g_Config.m_CannonsLogic) {
            // логику вообще отрубили :(

            m_TargetCore->Release();
            m_TargetCore = NULL;

            goto no_target;
        }

        if (!itstime)
            goto no_target;  // не время палить во все стороны :)

        if (!matchx || !matchz) {
            // еще не навелись...
            goto no_target;
        }

        // проверка попадания цели в зону поражения
        auto tmp = m_TargetCore->m_GeoCenter - GetGeoCenter();
        float dq = D3DXVec3LengthSq(&tmp);
        float ddq = m_Weapons[0]->GetWeaponDist();
        if (dq > POW2(ddq)) {
            // нет, не дострелим, ей богу не дострелим...
            goto no_target;
        }

        // и все-таки еще разик проверим :)

        for (int i = 0; i < m_WeaponCnt; ++i) {
            if (m_Weapons[i]) {
                D3DXVECTOR3 hp = m_FireFrom[i] + m_FireDir[i] * m_Weapons[i]->GetWeaponDist();
                CMatrixMapStatic *s = g_MatrixMap->Trace(&hp, m_FireFrom[i], hp, TRACE_ALL, this);

                float dist = DistOtrezokPoint(m_FireFrom[i], hp, m_TargetCore->m_GeoCenter);
                if (dist > (m_TargetCore->m_Radius * 2)) {
                    // так. все равно промажем.
                    goto no_target;
                }
            }
        }
    }

    // yo yo! цель в зоне обстрела!!! жмем на гашетку!

    /*    if (m_TargetCore && m_TargetCore->m_Object && m_TargetCore->m_Object->GetObjectType() == OBJECT_TYPE_ROBOTAI)
        {
            CMatrixRobotAI *tgt = (CMatrixRobotAI *)m_TargetCore->m_Object;

            if(!tgt->GetEnv()->SearchEnemy(this))
            {
                tgt->GetEnv()->AddToList(this);
            }
        }*/

    m_NullTargetTime = CANNON_NULL_TARGET_TIME;  // типа, чтобы стрелять еще после потери цели...

    for (int i = 0; i < m_WeaponCnt; ++i) {
        if (m_Weapons[i]) {
            m_Weapons[i]->FireBegin(D3DXVECTOR3(0, 0, 0), this);
        }
    }

    // и отрабатываем логику
    bool firewas = false;
    for (int i = 0; i < m_WeaponCnt; ++i) {
        if (m_Weapons[i]) {
            SETFLAG(m_ObjectState, OBJECT_CANNON_REF_PROTECTION);
            RESETFLAG(m_ObjectState, OBJECT_CANNON_REF_PROTECTION_HIT);

            m_Weapons[i]->ResetFireCount();
            m_Weapons[i]->Takt(float(takt));
            firewas |= m_Weapons[i]->IsFireWas();
            bool hw = m_Weapons[i]->IsHitWas();
            if (hw || FLAG(m_ObjectState, OBJECT_CANNON_REF_PROTECTION_HIT)) {
                m_TimeFromFire = CANNON_TIME_FROM_FIRE;
            }
            else {
                m_Core->m_Ref += m_Weapons[i]->GetFireCount();
            }

            RESETFLAG(m_ObjectState, OBJECT_CANNON_REF_PROTECTION);
            RESETFLAG(m_ObjectState, OBJECT_CANNON_REF_PROTECTION_HIT);
        }
    }

    if (firewas)
        BeginFireAnimation();

    m_TimeFromFire -= takt;
    if (m_TimeFromFire <= 0) {
        // стреляем мимо!
        // корректируем дуло
        m_TargetDisp = D3DXVECTOR3(FSRND(m_TargetCore->m_Radius * 0.5f), FSRND(m_TargetCore->m_Radius * 0.5f),
                                   FSRND(m_TargetCore->m_Radius * 0.5f));
        m_TimeFromFire = CANNON_TIME_FROM_FIRE;
    }

    //    CHelper::Create(1,0)->Line(D3DXVECTOR3(m_GeoCenter.x, m_GeoCenter.y, 20),
    //                            D3DXVECTOR3(m_GeoCenter.x, m_GeoCenter.y, 20) + 100 *
    //                            D3DXVECTOR3(cos(m_Unit[1].m_Angle), sin(m_Unit[1].m_Angle), 0));
}

bool CMatrixCannon::Damage(EWeapon weap, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &, int attacker_side,
                           CMatrixMapStatic *attaker) {
    DTRACE();

    bool friendly_fire = false;
    float damagek = 0.0;
    int idx = 0;

    if (weap == WEAPON_INSTANT_DEATH)
        goto inst_death;

    if (IsInvulnerable())
        return false;

    if (m_CurrState == CANNON_DIP)
        return true;

    friendly_fire = (attacker_side != 0) && (attacker_side == m_Side);
    damagek =
            (friendly_fire || m_Side != PLAYER_SIDE) ? 1.0f : g_MatrixMap->m_Difficulty.k_damage_enemy_to_player;
    if (friendly_fire && m_Side == PLAYER_SIDE)
        damagek = damagek * g_MatrixMap->m_Difficulty.k_friendly_fire;

    idx = Weap2Index(weap);
    if (weap == WEAPON_REPAIR) {
        m_HitPoint +=
                friendly_fire ? g_Config.m_CannonDamages[idx].friend_damage : g_Config.m_CannonDamages[idx].damage;

        if (m_HitPoint > m_HitPointMax) {
            m_HitPoint = m_HitPointMax;
        }
        m_PB.Modify(m_HitPoint * m_MaxHitPointInversed);

        return false;
    }

    CMatrixEffectWeapon::SoundHit(weap, pos);

    if (m_HitPoint > g_Config.m_CannonDamages[idx].mindamage) {
        m_HitPoint -= damagek * (friendly_fire ? g_Config.m_CannonDamages[idx].friend_damage
                                               : g_Config.m_CannonDamages[idx].damage);
        if (m_HitPoint >= 0) {
            m_PB.Modify(m_HitPoint * m_MaxHitPointInversed);
        }
        else {
            m_PB.Modify(0);
        }
        if (!friendly_fire)
            m_MiniMapFlashTime = FLASH_PERIOD;
        if (FLAG(g_MatrixMap->m_Flags, MMFLAG_FLYCAM)) {
            if (attaker)
                g_MatrixMap->m_Camera.AddWarPair(this, attaker);
        }
    }

    if (m_Side == PLAYER_SIDE && !friendly_fire) {
        if (m_UnderAttackTime == 0) {
            int ss = IRND(3);
            if (ss == 0)
                CSound::Play(S_SIDE_UNDER_ATTACK_1);
            else if (ss == 1)
                CSound::Play(S_SIDE_UNDER_ATTACK_2);
            else if (ss == 2)
                CSound::Play(S_SIDE_UNDER_ATTACK_3);
        }
        m_UnderAttackTime = UNDER_ATTACK_IDLE_TIME;
    }

    if (weap == WEAPON_FLAMETHROWER) {
        MarkAblaze();
        m_LastDelayDamageSide = attacker_side;

        int ttl = GetAblazeTTL();
        ttl += 300;
        if (ttl > 5000)
            ttl = 5000;
        SetAblazeTTL(ttl);

        m_NextTimeAblaze = g_MatrixMap->GetTime();
    }
    else if (weap == WEAPON_LIGHTENING) {
        m_LastDelayDamageSide = attacker_side;
        MarkShorted();
        SetShortedTTL(GetShortedTTL() + 500);

        m_NextTimeShorted = g_MatrixMap->GetTime();

        for (int i = 0; i < m_WeaponCnt; ++i) {
            if (m_Weapons[i]) {
                m_Weapons[i]->FireEnd();
            }
        }
        for (int i = 0; i < m_UnitCnt; i++) {
            if (m_Unit[i].m_Type == CUT_SHAFT) {
                m_Unit[i].m_Graph->SetAnimLooped(0);
                break;
            }
        }
    }
    else {
        m_LastDelayDamageSide = 0;
    }

    if (m_HitPoint > 0) {
        if (weap != WEAPON_ABLAZE && weap != WEAPON_SHORTED && weap != WEAPON_LIGHTENING &&
            weap != WEAPON_FLAMETHROWER) {
            CMatrixEffect::CreateExplosion(pos, ExplosionRobotHit);
        }
    }
    else {
        if (attacker_side != 0 && !friendly_fire) {
            g_MatrixMap->GetSideById(attacker_side)->IncStatValue(STAT_TURRET_KILL);
        }

    inst_death:;

        CMatrixEffect::CreateExplosion(*(D3DXVECTOR3 *)&m_Core->m_Matrix._41, ExplosionRobotBoom, true);

        m_ShadowType = SHADOW_OFF;
        RChange(MR_ShadowProjGeom | MR_ShadowStencil);
        RNeed(MR_ShadowProjGeom | MR_ShadowStencil);
        m_CurrState = CANNON_DIP;

        ReleaseMe();

        for (int i = 0; i < m_WeaponCnt; ++i) {
            if (m_Weapons[i]) {
                m_Weapons[i]->Release();
                m_Weapons[i] = NULL;
            }
        }
        m_WeaponCnt = 0;

        /// bool cstay = FRND(1) < 0.5f;

        m_Unit[0].u1.s2.m_TTL = FRND(3000) + 2000;
        m_Unit[0].u1.s2.m_Pos.x = m_Unit[0].m_Matrix._41;
        m_Unit[0].u1.s2.m_Pos.y = m_Unit[0].m_Matrix._42;
        m_Unit[0].u1.s2.m_Pos.z = m_Unit[0].m_Matrix._43;

        // if (cstay)
        {
            m_Unit[0].u1.s2.m_dp = 0;
            m_Unit[0].u1.s2.m_dy = 0;
            m_Unit[0].u1.s2.m_dr = 0;
            m_Unit[0].u1.s2.m_Velocity = D3DXVECTOR3(0, 0, 0);
        }  // else
        //{
        // m_Unit[0].m_dp = FSRND(0.0005f);
        // m_Unit[0].m_dy = FSRND(0.0005f);
        // m_Unit[0].m_dr = FSRND(0.0005f);
        // m_Unit[0].m_Velocity = D3DXVECTOR3(0,0,0.1f);
        //}

        m_Unit[0].m_Smoke.effect = NULL;
        CMatrixEffect::CreateSmoke(
            &m_Unit[0].m_Smoke,
            m_Unit[0].u1.s2.m_Pos,
            m_Unit[0].u1.s2.m_TTL + 100000,
            1000,
            100,
            0xFF000000,
            false,
            1.0f / 30.0f);

        for (int i = 1; i < m_UnitCnt; ++i) {
            m_Unit[i].u1.s2.m_dp = FSRND(0.005f);
            m_Unit[i].u1.s2.m_dy = FSRND(0.005f);
            m_Unit[i].u1.s2.m_dr = FSRND(0.005f);
            m_Unit[i].u1.s2.m_Velocity = D3DXVECTOR3(FSRND(0.08f), FSRND(0.08f), 0.1f);
            m_Unit[i].u1.s2.m_TTL = FRND(3000) + 2000;
            m_Unit[i].u1.s2.m_Pos.x = m_Unit[i].m_Matrix._41;
            m_Unit[i].u1.s2.m_Pos.y = m_Unit[i].m_Matrix._42;
            m_Unit[i].u1.s2.m_Pos.z = m_Unit[i].m_Matrix._43;
            m_Unit[i].m_Smoke.effect = NULL;
            CMatrixEffect::CreateSmoke(
                &m_Unit[i].m_Smoke,
                m_Unit[i].u1.s2.m_Pos,
                m_Unit[i].u1.s2.m_TTL + 100000,
                1000,
                100,
                0xFF000000,
                false,
                1.0f / 30.0f);
        }

        return true;
    }
    return false;
}

void CMatrixCannon::ReleaseMe(void) {
    DTRACE();

    CMatrixSideUnit *s = NULL;
    if (m_ParentBuilding) {
        for (int i = 0; i < MAX_PLACES; i++) {
            if (m_ParentBuilding->m_TurretsPlaces[i].m_Coord.x == Float2Int(m_Pos.x / GLOBAL_SCALE_MOVE) &&
                m_ParentBuilding->m_TurretsPlaces[i].m_Coord.y == Float2Int(m_Pos.y / GLOBAL_SCALE_MOVE)) {
                m_ParentBuilding->m_TurretsPlaces[i].m_CannonType = -1;
            }
        }

        if (g_MatrixMap->GetPlayerSide()->m_ActiveObject == m_ParentBuilding) {
            g_IFaceList->CreateDynamicTurrets(m_ParentBuilding);
        }
    }

    if (m_ParentBuilding && m_ParentBuilding->m_TurretsHave) {
        m_ParentBuilding->m_TurretsHave--;

        int side = m_ParentBuilding->GetSide();

        if (side != 0) {
            CMatrixSideUnit *su = g_MatrixMap->GetSideById(side);
            if (su->m_CurrentAction == BUILDING_TURRET || FLAG(g_IFaceList->m_IfListFlags, PREORDER_BUILD_TURRET)) {
                m_ParentBuilding->CreatePlacesShow();
                m_ParentBuilding = NULL;
            }
        }
        CMatrixMapStatic *obj = CMatrixMapStatic::GetFirstLogic();
    }

    // old code!!!!!!! do not uncomment this!!!!!!
    // for(int c = 1; c <= g_MatrixMap->m_SideCnt; c++){
    //    s = g_MatrixMap->GetSideById(c);
    //    if(s != g_MatrixMap->GetPlayerSide() && s->m_GroupsList){
    //        CMatrixGroup* grps = s->m_GroupsList->m_FirstGroup;
    //        while(grps){
    //            if(grps->m_Tactics && grps->m_Tactics->GetTarget() == this){
    //                grps->DeInstallTactics();
    //            }
    //            grps = grps->m_NextGroup;
    //        }
    //    }else{
    //        if(s->m_CurGroup){
    //            CMatrixTactics* t = s->m_CurGroup->GetTactics();
    //            if(t && t->GetTarget() == this){
    //                s->m_CurGroup->DeInstallTactics();
    //            }
    //        }
    //    }
    //}

    CMatrixMapStatic *objects = CMatrixMapStatic::GetFirstLogic();

    while (objects) {
        if (objects->IsLiveRobot()) {
            objects->AsRobot()->GetEnv()->RemoveFromList(this);
        }
        else if (objects->IsLiveBuilding()) {
            objects->AsBuilding()->m_BS.DeleteItem(this);
        }
        objects = objects->GetNextLogic();
    }
}

bool CMatrixCannon::InRect(const CRect &rect) const {
    if (m_CurrState == CANNON_DIP)
        return false;

    D3DXVECTOR3 dir;
    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.left, rect.top), dir);
    if (Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, NULL))
        return true;

    D3DXMATRIX s, t;
    SEVH_data d;

    t = g_MatrixMap->m_Camera.GetViewMatrix() * g_MatrixMap->m_Camera.GetProjMatrix();
    D3DXMatrixScaling(&s, float(g_ScreenX / 2), float(-g_ScreenY / 2), 1);
    s._41 = s._11;
    s._42 = float(g_ScreenY / 2);
    t *= s;
    d.rect = &rect;

    for (int i = 0; i < m_UnitCnt; ++i) {
        if (m_Unit[i].m_Graph) {
            d.found = false;
            d.m = m_Unit[i].m_Matrix * t;
            m_Unit[i].m_Graph->EnumFrameVerts(EnumVertsHandler, (DWORD)&d);
            if (d.found)
                return true;
        }
    }

    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.left, rect.bottom), dir);
    if (Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, NULL))
        return true;

    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.right, rect.top), dir);
    if (Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, NULL))
        return true;

    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.right, rect.bottom), dir);
    if (Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, NULL))
        return true;

    return false;
}
