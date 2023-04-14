// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>
#include <algorithm>

#include "MatrixObjectBuilding.hpp"
#include "MatrixShadowManager.hpp"
#include "MatrixRenderPipeline.hpp"
#include "MatrixObject.hpp"
#include "MatrixFlyer.hpp"
#include "Logic/MatrixAIGroup.h"
#include "MatrixRobot.hpp"
#include "MatrixSkinManager.hpp"
#include "MatrixObjectCannon.hpp"
#include "Interface/CInterface.h"

#include "Effects/MatrixEffectSelection.hpp"
#include "Effects/MatrixEffectExplosion.hpp"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#pragma warning(disable : 4355)

CMatrixBuilding::CMatrixBuilding()
  : CMatrixMapStatic(), m_Name{L"FACTORY"}, m_BS(this), m_UnderAttackTime(0) {
    DTRACE();
    m_GGraph = NULL;
    //	m_ShadowStencil=NULL;
    m_ShadowProj = NULL;

    m_Core->m_Type = OBJECT_TYPE_BUILDING;

    m_Side = 0;
    m_Kind = BUILDING_BASE;

    m_Pos.x = m_Pos.y = 0;
    m_Angle = 0;

    m_ShadowType = SHADOW_STENCIL;
    m_ShadowSize = 128;

    m_BaseFloor = 0.2f;
    m_State = BASE_CLOSING;

    m_GGraph = HNew(g_MatrixHeap) CVectorObjectGroup();
    /*m_Busy = false;*/

    m_Capturer = NULL;
    m_capture = NULL;
    m_InCaptureTime = 0;
    m_CaptureSeekRobotNextTime = 0;

    m_ResourcePeriod = 0;
    m_TurretsHave = 0;

    m_defHitPoint = 0;

    m_TurretsPlacesCnt = 0;

    //    m_Places = (int*)HAlloc(sizeof(int)*(MAX_PLACES*2)+4, g_MatrixHeap);
    //    ZeroMemory(m_Places, sizeof(int)*(MAX_PLACES*2));

    m_PlacesShow = NULL;
    m_Selection = NULL;

    m_CaptureMeNextTime = 0;
    m_ShowHitpointTime = 0;
}

#pragma warning(default : 4355)

CMatrixBuilding::~CMatrixBuilding() {
    DTRACE();

    m_BS.ClearStack();
    UnSelect();

    int x = Float2Int(m_Pos.x * INVERT(GLOBAL_SCALE));
    int y = Float2Int(m_Pos.x * INVERT(GLOBAL_SCALE));

    for (int i = x - 4; i < (x + 4); ++i)
        for (int j = x - 4; j < (x + 4); ++j) {
            SMatrixMapUnit *mu = g_MatrixMap->UnitGetTest(i, j);
            if (mu == NULL)
                continue;
            if (mu->m_Base == this)
                mu->m_Base = NULL;
        }

    ReleaseMe();
    //	if(m_Graph) { HDelete(CVectorObjectAnim,m_Graph,g_MatrixHeap); m_Graph=NULL; }
    //	if(m_ShadowStencil) { HDelete(CVOShadowStencil,m_ShadowStencil,g_MatrixHeap); m_ShadowStencil=NULL; }
    if (m_GGraph) {
        HDelete(CVectorObjectGroup, m_GGraph, g_MatrixHeap);
    }
    if (m_ShadowProj) {
        HDelete(CMatrixShadowProj, m_ShadowProj, g_MatrixHeap);
        m_ShadowProj = NULL;
    }
#ifdef _DEBUG
    if (m_capture)
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, m_capture);
#else

    if (m_capture)
        g_MatrixMap->SubEffect(m_capture);
#endif

    /*    if(m_Places){
            HFree(m_Places, g_MatrixHeap);
        }*/

    DeletePlacesShow();
}

void CMatrixBuilding::RNeed(dword need) {
    DTRACE();
    if (need & m_RChange & (MR_Matrix)) {
        m_RChange &= ~MR_Matrix;

        // rotate

        switch (m_Angle) {
            case 0:
                m_Core->m_Matrix._11 = 1;
                m_Core->m_Matrix._12 = 0;
                m_Core->m_Matrix._21 = 0;
                m_Core->m_Matrix._22 = 1;
                break;
            case 1:
                m_Core->m_Matrix._11 = 0;
                m_Core->m_Matrix._12 = 1;
                m_Core->m_Matrix._21 = -1;
                m_Core->m_Matrix._22 = 0;
                break;
            case 2:
                m_Core->m_Matrix._11 = -1;
                m_Core->m_Matrix._12 = 0;
                m_Core->m_Matrix._21 = 0;
                m_Core->m_Matrix._22 = -1;
                break;
            case 3:
                m_Core->m_Matrix._11 = 0;
                m_Core->m_Matrix._12 = -1;
                m_Core->m_Matrix._21 = 1;
                m_Core->m_Matrix._22 = 0;
        }
        m_Core->m_Matrix._13 = 0;
        m_Core->m_Matrix._14 = 0;
        m_Core->m_Matrix._23 = 0;
        m_Core->m_Matrix._24 = 0;
        m_Core->m_Matrix._31 = 0;
        m_Core->m_Matrix._32 = 0;
        m_Core->m_Matrix._33 = 1;
        m_Core->m_Matrix._34 = 0;
        m_Core->m_Matrix._41 = m_Pos.x;
        m_Core->m_Matrix._42 = m_Pos.y;
        m_Core->m_Matrix._43 = m_BuildZ;
        m_Core->m_Matrix._44 = 1;

        m_GGraph->m_GroupToWorldMatrix = &m_Core->m_Matrix;

        D3DXMatrixInverse(&m_Core->m_IMatrix, NULL, &m_Core->m_Matrix);

        m_GGraph->RChange(VOUF_MATRIX);
        m_GGraph->RNeed(VOUF_MATRIX);
    }
    if (need & m_RChange & (MR_Graph)) {
        m_RChange &= ~MR_Graph;

        if (!m_GGraph->IsAlreadyLoaded()) {
            float hp = (float)g_Config.m_BuildingHitPoints[m_Kind];
            InitMaxHitpoint(hp);

            if (m_Kind == BUILDING_BASE)
                m_GGraph->Load(OBJECT_PATH_BASE_00, CMatrixEffect::GetBBTexI(BBT_POINTLIGHT), CSkinManager::GetSkin,
                               GSP_SIDE);
            else if (m_Kind == BUILDING_TITAN)
                m_GGraph->Load(OBJECT_PATH_BASE_01, CMatrixEffect::GetBBTexI(BBT_POINTLIGHT), CSkinManager::GetSkin,
                               GSP_SIDE);
            else if (m_Kind == BUILDING_PLASMA)
                m_GGraph->Load(OBJECT_PATH_BASE_02, CMatrixEffect::GetBBTexI(BBT_POINTLIGHT), CSkinManager::GetSkin,
                               GSP_SIDE);
            else if (m_Kind == BUILDING_ELECTRONIC)
                m_GGraph->Load(OBJECT_PATH_BASE_03, CMatrixEffect::GetBBTexI(BBT_POINTLIGHT), CSkinManager::GetSkin,
                               GSP_SIDE);
            else if (m_Kind == BUILDING_ENERGY)
                m_GGraph->Load(OBJECT_PATH_BASE_04, CMatrixEffect::GetBBTexI(BBT_POINTLIGHT), CSkinManager::GetSkin,
                               GSP_SIDE);
            else if (m_Kind == BUILDING_REPAIR)
                m_GGraph->Load(OBJECT_PATH_BASE_05, CMatrixEffect::GetBBTexI(BBT_POINTLIGHT), CSkinManager::GetSkin,
                               GSP_SIDE);
        }
    }
    if ((need & m_RChange & MR_ShadowStencil) && m_GGraph) {
        m_RChange &= ~MR_ShadowStencil;

        if (m_ShadowType != SHADOW_STENCIL) {
            m_GGraph->ShadowStencilOn(false);
        }
        else {
            m_GGraph->m_ShadowStencilLight = g_MatrixMap->m_LightMain;
            m_GGraph->m_GroundZ = (m_Kind == BUILDING_BASE) ? g_MatrixMap->m_GroundZBase : g_MatrixMap->m_GroundZ;
            m_GGraph->ShadowStencilOn(true);
            // m_GGraph->RChange(VOUF_SHADOWSTENCIL); // Takt will modifies this flag
            m_GGraph->RNeed(VOUF_MATRIX | VOUF_SHADOWSTENCIL);
        }

        /*		if(m_ShadowType!=SHADOW_STENCIL) {
                    if(m_ShadowStencil) { HDelete(CVOShadowStencil,m_ShadowStencil,g_MatrixHeap); m_ShadowStencil=NULL;
           } } else if(1) { ASSERT(m_Graph); ASSERT(m_Graph->VO());

                    if(!m_ShadowStencil) m_ShadowStencil=HNew(g_MatrixHeap) CVOShadowStencil;

                    if(!(m_Graph->VO()->EdgeExist())) m_Graph->VO()->EdgeBuild();

                    m_ShadowStencil->Build(*(m_Graph->VO()),m_Graph->FrameVO(),g_MatrixMap->m_LightMain,m_Matrix,g_MatrixMap->m_ShadowPlaneCut);
                }*/
    }
    if (need & m_RChange & MR_ShadowProjGeom) {
        m_RChange &= ~MR_ShadowProjGeom;

        // TO DO: see mapobject and robots

        /*		if(m_ShadowType!=SHADOW_PROJ_STATIC && m_ShadowType!=SHADOW_PROJ_DYNAMIC) {
                    if(m_ShadowProj) { HDelete(CVOShadowProj,m_ShadowProj,g_MatrixHeap); m_ShadowProj=NULL; }
                } else if(1) {
                    ASSERT(m_Graph);
                    ASSERT(m_Graph->VO());

                    if(!m_ShadowProj) m_ShadowProj=HNew(g_MatrixHeap) CVOShadowProj(g_MatrixHeap);

                    if(m_ShadowType==SHADOW_PROJ_DYNAMIC) {
                        CTexture * tex=m_ShadowProj->GetTexture();
                        m_ShadowProj->SetTexture(NULL);
                        if(tex) {
                            tex->RefDec();
                            ASSERT(tex->Ref()==0);
                        }
                        ShadowProjBuild(*m_ShadowProj,*(m_Graph->VO()),tex,m_Graph->FrameVO(),m_Matrix,g_MatrixMap->m_LightMain,10,m_ShadowSize,
        true); } else { CTexture * tex=m_ShadowProj->GetTexture(); m_ShadowProj->SetTexture(NULL); if(tex) {
                            tex->RefDec();
                            if(tex->Ref()<=0) CCache::Destroy(tex);
                        }
                        tex=NULL;
                        CMatrixMapStatic * so=g_MatrixMap->m_StaticFirst;
                        while(so)
                        {
                            if( so->GetObjectType()==OBJECT_TYPE_BUILDING &&
                                ((CMatrixBuilding *)so)->m_ShadowProj &&
                                ((CMatrixBuilding *)so)->m_ShadowProj->GetTexture() &&
                                ((CMatrixBuilding *)so)->m_ShadowType==m_ShadowType &&
                                ((CMatrixBuilding *)so)->m_ShadowSize==m_ShadowSize &&
                                ((CMatrixBuilding *)so)->m_Kind==m_Kind)
                            {
                                tex=((CMatrixBuilding *)so)->m_ShadowProj->GetTexture();
                                break;
                            }
                            so=so->m_Next;
                        }
        //				(std::wstring(L"SB_")+std::wstring(m_ShadowSize)+std::wstring(L"_")+std::wstring(m_Kind)).Get()
                        ShadowProjBuild(*m_ShadowProj,*(m_Graph->VO()),tex,0,m_Matrix,g_MatrixMap->m_LightMain,10,m_ShadowSize,false);
                    }
                    if(!(m_ShadowProj->IsProjected())) {
                        HDelete(CVOShadowProj,m_ShadowProj,g_MatrixHeap);
                        m_ShadowProj=NULL;
                    }
                }*/
    }
    if (need & m_RChange & (MR_MiniMap)) {
        m_RChange &= ~MR_MiniMap;
        g_MatrixMap->m_Minimap.RenderObjectToBackground(this);
    }
}

bool CMatrixBuilding::Damage(EWeapon weap, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &, int attacker_side,
                             CMatrixMapStatic *attaker) {
    DTRACE();

    if (m_State == BUILDING_DIP || m_State == BUILDING_DIP_EXPLODED)
        return true;

    bool friendly_fire = (attacker_side != 0) && (attacker_side == m_Side);
    float damagek =
            (friendly_fire || m_Side != PLAYER_SIDE) ? 1.0f : g_MatrixMap->m_Difficulty.k_damage_enemy_to_player;
    if (friendly_fire && m_Side == PLAYER_SIDE)
        damagek = damagek * g_MatrixMap->m_Difficulty.k_friendly_fire;

    int idx = Weap2Index(weap);
    if (weap == WEAPON_REPAIR) {
        m_HitPoint +=
                friendly_fire ? g_Config.m_BuildingDamages[idx].friend_damage : g_Config.m_BuildingDamages[idx].damage;
        if (m_HitPoint > m_HitPointMax) {
            m_HitPoint = m_HitPointMax;
        }
        m_PB.Modify(m_HitPoint * m_MaxHitPointInversed);

        return false;
    }

    CMatrixEffectWeapon::SoundHit(weap, pos);

    if (m_HitPoint > g_Config.m_BuildingDamages[idx].mindamage) {
        m_HitPoint -= damagek * (friendly_fire ? g_Config.m_BuildingDamages[idx].friend_damage
                                               : g_Config.m_BuildingDamages[idx].damage);

        if (m_HitPoint >= 0) {
            m_PB.Modify(m_HitPoint * m_MaxHitPointInversed);
        }
        else {
            m_PB.Modify(0);
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

    if (m_HitPoint <= 0) {
        if (m_Side == PLAYER_SIDE) {
            bool bu = FRND(1) < 0.7f;
            if (bu) {
                if (m_Kind == BUILDING_BASE) {
                    CSound::Play(S_BASE_KILLED);
                }
                else {
                    CSound::Play(S_FACTORY_KILLED);
                }
            }
            else {
                CSound::Play(S_BUILDING_KILLED);
            }
        }

        ReleaseMe();
        m_HitPoint = -1;
        m_State = BUILDING_DIP;

        if (attacker_side != 0 && !friendly_fire) {
            g_MatrixMap->GetSideById(attacker_side)->IncStatValue(STAT_BUILDING_KILL);
        }

        m_NextExplosionTime = g_MatrixMap->GetTime();
        m_NextExplosionTimeSound = g_MatrixMap->GetTime();
        g_MatrixMap->RemoveEffectSpawnerByObject(this);
        return true;
    }

    return false;
}

void CMatrixBuilding::OnOutScreen(void) {}

void CMatrixBuilding::Takt(int cms) {
    DTRACE();

    if (m_Side == PLAYER_SIDE && FLAG(m_ObjectState, BUILDING_NEW_INCOME)) {
        RESETFLAG(m_ObjectState, BUILDING_NEW_INCOME);
        switch (m_Kind) {
            case BUILDING_TITAN:
                CMatrixEffect::CreateBillboardScore(
                        L"t10", D3DXVECTOR3(GetGeoCenter().x, GetGeoCenter().y, GetGeoCenter().z + 40.0f), 0xFFFFFFFF);
                break;
            case BUILDING_PLASMA:
                CMatrixEffect::CreateBillboardScore(
                        L"p10", D3DXVECTOR3(GetGeoCenter().x, GetGeoCenter().y, GetGeoCenter().z + 40.0f), 0xFFFFFFFF);
                break;
            case BUILDING_ELECTRONIC:
                CMatrixEffect::CreateBillboardScore(
                        L"e10", D3DXVECTOR3(GetGeoCenter().x, GetGeoCenter().y, GetGeoCenter().z + 40.0f), 0xFFFFFFFF);
                break;
            case BUILDING_ENERGY:
                CMatrixEffect::CreateBillboardScore(
                        L"b10", D3DXVECTOR3(GetGeoCenter().x, GetGeoCenter().y, GetGeoCenter().z + 40.0f), 0xFFFFFFFF);
                break;
            case BUILDING_BASE: {
                int fu = g_MatrixMap->GetPlayerSide()->GetResourceForceUp();
                int prihod = RESOURCES_INCOME_BASE * fu / 100;
                CMatrixEffect::CreateBillboardScore(utils::format(L"a%d", prihod).c_str(), m_TopPoint, 0xFFFFFFFF);
                // if(m_BaseRCycle == 0){
                //    CMatrixEffect::CreateBillboardScore(L"t" + std::wstring(prihod, g_CacheHeap), m_TopPoint, 0xFFFFFF00);
                //}else if(m_BaseRCycle == 1){
                //    CMatrixEffect::CreateBillboardScore(L"e" + std::wstring(prihod, g_CacheHeap), m_TopPoint, 0xFFFFFF00);
                //}else if(m_BaseRCycle == 2){
                //    CMatrixEffect::CreateBillboardScore(L"b" + std::wstring(prihod, g_CacheHeap), m_TopPoint, 0xFFFFFF00);
                //}else if(m_BaseRCycle == 3){
                //    CMatrixEffect::CreateBillboardScore(L"p" + std::wstring(prihod, g_CacheHeap), m_TopPoint, 0xFFFFFF00);
                //}

            } break;
        }
    }

    if (m_capture) {
        m_capture->UpdateData(m_TrueColor.m_Color, m_TrueColor.m_ColoredCnt);
    }
    if (m_GGraph) {
        if (m_GGraph->Takt(cms)) {
            if (m_ShadowType == SHADOW_STENCIL) {
                RChange(MR_ShadowStencil);
                // m_GGraph->RChange(MR_ShadowStencil);
            }
            else if (m_ShadowType == SHADOW_PROJ_DYNAMIC) {
                RChange(MR_ShadowProjTex);
                // m_GGraph->RChange(MR_ShadowProj);
            }
        }
    }
    if (!m_capture && !IsBase()) {
        if (m_Core->m_Matrix._22 > 0) {
            m_capture = (CMatrixEffectZahvat *)CMatrixEffect::CreateZahvat(
                    D3DXVECTOR3(m_Pos.x + m_Core->m_Matrix._21 * 2.7f, m_Pos.y + m_Core->m_Matrix._22 * 2.7f,
                                m_Core->m_Matrix._43 + 0.8f),
                    24, GRAD2RAD(102), MAX_ZAHVAT_POINTS);
        }
        else if (m_Core->m_Matrix._21 > 0) {
            m_capture = (CMatrixEffectZahvat *)CMatrixEffect::CreateZahvat(
                    D3DXVECTOR3(m_Pos.x + m_Core->m_Matrix._21 * 2.7f, m_Pos.y + m_Core->m_Matrix._22 * 2.7f,
                                m_Core->m_Matrix._43 + 0.8f),
                    24, GRAD2RAD(12), MAX_ZAHVAT_POINTS);
        }
        else if (m_Core->m_Matrix._21 < 0) {
            m_capture = (CMatrixEffectZahvat *)CMatrixEffect::CreateZahvat(
                    D3DXVECTOR3(m_Pos.x + m_Core->m_Matrix._21 * 2.7f, m_Pos.y + m_Core->m_Matrix._22 * 2.7f,
                                m_Core->m_Matrix._43 + 0.8f),
                    24, GRAD2RAD(192), MAX_ZAHVAT_POINTS);
        }
        else if (m_Core->m_Matrix._22 < 0) {
            m_capture = (CMatrixEffectZahvat *)CMatrixEffect::CreateZahvat(
                    D3DXVECTOR3(m_Pos.x + m_Core->m_Matrix._21 * 2.7f, m_Pos.y + m_Core->m_Matrix._22 * 2.7f,
                                m_Core->m_Matrix._43 + 0.8f),
                    24, GRAD2RAD(-79), MAX_ZAHVAT_POINTS);
        }

        if (!g_MatrixMap->AddEffect(m_capture)) {
            m_capture = NULL;
        }
        else {
            if (m_Side == 0) {
                m_TrueColor.m_Color = 0;
                m_TrueColor.m_ColoredCnt = 0;
            }
            else {
                m_TrueColor.m_Color = (0xFF000000) | g_MatrixMap->GetSideColor(m_Side);
                m_TrueColor.m_ColoredCnt = MAX_ZAHVAT_POINTS;
            }
        }
    }
}

struct SFindRobotForCaptureAny {
    CMatrixRobotAI *found;
    float dist2;
};

static bool FindCaptureMe(const D3DXVECTOR2 &center, CMatrixMapStatic *ms, DWORD user) {
    DTRACE();

    CMatrixBuilding *b = (CMatrixBuilding *)user;

    if (ms->AsRobot()->m_Side != b->m_Side)
        ms->AsRobot()->AddCaptureCandidate(b);
    return true;
}

static bool FindRobotForCaptureAny(const D3DXVECTOR2 &center, CMatrixMapStatic *ms, DWORD user) {
    DTRACE();

    SFindRobotForCaptureAny *data = (SFindRobotForCaptureAny *)user;

    auto tmp = center - *(D3DXVECTOR2 *)&ms->GetGeoCenter();
    float dist2 = D3DXVec2LengthSq(&tmp);
    if (dist2 < data->dist2) {
        data->found = (CMatrixRobotAI *)ms;
        data->dist2 = dist2;
    }

    return true;
}

void CMatrixBuilding::PauseTakt(int cms) {
    DTRACE();
    m_PB.Modify(100000.0f, 0);

    if (m_State != BUILDING_DIP && m_State != BUILDING_DIP_EXPLODED) {
        m_ShowHitpointTime -= cms;
        if (m_ShowHitpointTime < 0) {
            m_ShowHitpointTime = 0;
        }
    }
}

void CMatrixBuilding::LogicTakt(int cms) {
    DTRACE();

    m_PB.Modify(100000.0f, 0);

    if (m_State != BUILDING_DIP && m_State != BUILDING_DIP_EXPLODED) {
        if (m_CaptureMeNextTime < g_MatrixMap->GetTime() && m_Capturer == NULL && !(IsBase() && m_TurretsHave > 0)) {
            m_CaptureMeNextTime = g_MatrixMap->GetTime() + 1002;

            CMatrixMapStatic *r = CMatrixMapStatic::GetFirstLogic();
            for (; r; r = r->GetNextLogic()) {
                if (r->IsRobot())
                    r->AsRobot()->UnMarkCaptureInformed();
            }

            g_MatrixMap->FindObjects(m_Pos, DISTANCE_CAPTURE_ME, 1, TRACE_ROBOT, m_Capturer, FindCaptureMe,
                                     (DWORD)this);

            // should not be captured...
            r = CMatrixMapStatic::GetFirstLogic();
            for (; r; r = r->GetNextLogic()) {
                if (r->IsRobot() && !r->AsRobot()->IsCaptureInformed()) {
                    r->AsRobot()->RemoveCaptureCandidate(this);
                }
            }
        }

        m_UnderAttackTime -= cms;
        if (m_UnderAttackTime < 0)
            m_UnderAttackTime = 0;

        if (m_ShowHitpointTime > 0) {
            m_ShowHitpointTime -= cms;
            if (m_ShowHitpointTime < 0)
                m_ShowHitpointTime = 0;
        }

        if (m_Kind != BUILDING_BASE) {
            if (m_CaptureSeekRobotNextTime < g_MatrixMap->GetTime()) {
                SFindRobotForCaptureAny data;
                data.found = NULL;
                data.dist2 = CAPTURE_RADIUS * CAPTURE_RADIUS;

                // CHelper::Create(300,0)->Line(D3DXVECTOR3(m_Pos.x, m_Pos.y, 0), D3DXVECTOR3(m_Pos.x, m_Pos.y, 1000));

                g_MatrixMap->FindObjects(m_Pos, CAPTURE_RADIUS, 1, TRACE_ROBOT, NULL, FindRobotForCaptureAny,
                                         (DWORD)&data);

                if (data.found && m_Side != data.found->GetSide()) {
                    Capture(data.found);

                    int nt = 100;
                    if (nt > g_Config.m_CaptureTimeErase)
                        nt = g_Config.m_CaptureTimeErase;
                    if (nt > g_Config.m_CaptureTimePaint)
                        nt = g_Config.m_CaptureTimePaint;

                    m_CaptureSeekRobotNextTime = g_MatrixMap->GetTime() + nt;
                }
                else {
                    m_CaptureSeekRobotNextTime = g_MatrixMap->GetTime() + CAPTURE_SEEK_ROBOT_PERIOD;
                }
            }
            if (m_InCaptureTime > 0) {
                m_InCaptureTime -= cms;
                if (m_InCaptureTime <= 0) {
                    m_InCaptureTime = 0;
                    m_CaptureNextTimeRollback = g_MatrixMap->GetTime();
                }
            }
            else {
                // rollback
                if (m_Side == 0 && m_TrueColor.m_ColoredCnt > 0) {
                    while (m_CaptureNextTimeRollback < g_MatrixMap->GetTime() && m_TrueColor.m_ColoredCnt > 0) {
                        m_CaptureNextTimeRollback += g_Config.m_CaptureTimeRolback;
                        --m_TrueColor.m_ColoredCnt;
                    }
                }
                else if (m_Side != 0 && m_TrueColor.m_ColoredCnt < MAX_ZAHVAT_POINTS) {
                    while (m_CaptureNextTimeRollback < g_MatrixMap->GetTime() &&
                           m_TrueColor.m_ColoredCnt < MAX_ZAHVAT_POINTS) {
                        m_CaptureNextTimeRollback += g_Config.m_CaptureTimeRolback;
                        ++m_TrueColor.m_ColoredCnt;
                    }
                }
            }
        }

        CMatrixSideUnit *su = NULL;
        if (m_Side)
            su = g_MatrixMap->GetSideById(m_Side);

        if (su) {
            m_BS.TickTimer(cms);
            m_ResourcePeriod += cms;

            switch (m_Kind) {
                case BUILDING_TITAN:
                    if (m_ResourcePeriod >= g_Config.m_Timings[RESOURCE_TITAN]) {
                        // m_ResourceAmount += RESOURCES_INCOME;
                        SETFLAG(m_ObjectState, BUILDING_NEW_INCOME);
                        m_ResourcePeriod = 0;
                        su->AddResourceAmount(TITAN, RESOURCES_INCOME);
                    }
                    break;
                case BUILDING_PLASMA:
                    if (m_ResourcePeriod >= g_Config.m_Timings[RESOURCE_PLASMA]) {
                        // m_ResourceAmount += RESOURCES_INCOME;
                        SETFLAG(m_ObjectState, BUILDING_NEW_INCOME);
                        m_ResourcePeriod = 0;
                        su->AddResourceAmount(PLASMA, RESOURCES_INCOME);
                    }
                    break;
                case BUILDING_ELECTRONIC:
                    if (m_ResourcePeriod >= g_Config.m_Timings[RESOURCE_ELECTRONICS]) {
                        // m_ResourceAmount += RESOURCES_INCOME;
                        SETFLAG(m_ObjectState, BUILDING_NEW_INCOME);
                        m_ResourcePeriod = 0;
                        su->AddResourceAmount(ELECTRONICS, RESOURCES_INCOME);
                    }
                    break;
                case BUILDING_ENERGY:
                    if (m_ResourcePeriod >= g_Config.m_Timings[RESOURCE_ENERGY]) {
                        // m_ResourceAmount += RESOURCES_INCOME;
                        SETFLAG(m_ObjectState, BUILDING_NEW_INCOME);
                        m_ResourcePeriod = 0;
                        su->AddResourceAmount(ENERGY, RESOURCES_INCOME);
                    }
                    break;
                case BUILDING_BASE:
                    if (m_ResourcePeriod >= g_Config.m_Timings[RESOURCE_BASE]) {
                        // m_BaseRCycle++;
                        // if(m_BaseRCycle > 3) m_BaseRCycle = 0;
                        int fu = su->GetResourceForceUp();
                        int ra = RESOURCES_INCOME_BASE * fu / 100;
                        SETFLAG(m_ObjectState, BUILDING_NEW_INCOME);
                        m_ResourcePeriod = 0;

                        su->AddResourceAmount(TITAN, ra);
                        su->AddResourceAmount(ELECTRONICS, ra);
                        su->AddResourceAmount(ENERGY, ra);
                        su->AddResourceAmount(PLASMA, ra);

                        // if(m_BaseRCycle == 0){
                        //    su->AddTitan(RESOURCES_INCOME * fu / 100);
                        //}else if(m_BaseRCycle == 1){
                        //    su->AddElectronics(RESOURCES_INCOME * fu / 100);
                        //}else if(m_BaseRCycle == 2){
                        //    su->AddEnergy(RESOURCES_INCOME * fu / 100);
                        //}else if(m_BaseRCycle == 3){
                        //    su->AddPlasma(RESOURCES_INCOME * fu / 100);
                        //}
                    }
                    break;
            }
        }
    }
    // if (m_Kind==BUILDING_BASE) CDText::T("hp",(float)m_HitPoint);

    int downtime = -BUILDING_EXPLOSION_TIME;
    if (m_Kind == BUILDING_BASE) {
        downtime -= BUILDING_BASE_EXPLOSION_TIME;

        if ((m_State == BUILDING_DIP) && (m_HitPoint < downtime + 100)) {
            CSound::AddSound(S_EXPLOSION_BUILDING_BOOM4, GetGeoCenter());
            // DCNT("boom");
            CMatrixEffectWeapon *e = (CMatrixEffectWeapon *)CMatrixEffect::CreateWeapon(
                    GetGeoCenter(), D3DXVECTOR3(0, 0, 1), 0, NULL, WEAPON_BIGBOOM);
            e->SetOwner(this);
            e->FireBegin(D3DXVECTOR3(0, 0, 0), this);
            e->Takt(1);
            e->FireEnd();
            e->Release();
            m_State = BUILDING_DIP_EXPLODED;
        }
    }

    if (m_HitPoint < downtime) {
        if (m_Kind != BUILDING_BASE) {
            CSound::AddSound(S_EXPLOSION_BUILDING_BOOM3, GetGeoCenter());
        }

        if (m_GGraph) {
            m_ShadowType = SHADOW_OFF;
            RChange(MR_ShadowStencil | MR_ShadowProjGeom);
            RNeed(MR_ShadowStencil | MR_ShadowProjGeom);

            HDelete(CVectorObjectGroup, m_GGraph, g_MatrixHeap);
            m_GGraph = NULL;

            // replace geometry
            int n = (int)m_Kind;

            std::wstring namet(OBJECT_PATH_BUILDINGS_RUINS);
            namet += L"b";
            namet += utils::format(L"%d", n);
            std::wstring namev(namet);
            namev += L".vo";

            CMatrixMapObject *mo = g_MatrixMap->StaticAdd<CMatrixMapObject>(false);
            mo->InitAsBaseRuins(m_Pos, m_Angle, namev, namet, true);

            if (n != 0) {
                namev = namet + L"p.vo";
                namet += L"?Trans";
                CMatrixMapObject *momo = g_MatrixMap->StaticAdd<CMatrixMapObject>(false);
                momo->InitAsBaseRuins(m_Pos, m_Angle, namev, namet, false);
            }

            n = IRND(30) + 20;
            for (int i = 0; i < n; ++i) {
                std::wstring type(L"1,1000,5000,");
                type += utils::format(L"%d", IRND(500) + 700);
                type += L",2400,60,false,0.03,78000000";

                D3DXVECTOR3 dir, pos;
                float t;

                int cnt = 30;
                do {
                    pos.x = mo->GetGeoCenter().x + FSRND(mo->GetRadius());
                    pos.y = mo->GetGeoCenter().y + FSRND(mo->GetRadius());
                    pos.z = mo->GetMatrix()._43 + FRND(mo->GetRadius() * 2);
                    auto tmp = D3DXVECTOR3(mo->GetMatrix()._41 - pos.x,
                                           mo->GetMatrix()._42 - pos.y,
                                           mo->GetMatrix()._43 - pos.z);
                    D3DXVec3Normalize(&dir, &tmp);
                }
                while (!mo->PickFull(pos, dir, &t) && (--cnt > 0));

                if (cnt > 0) {
                    pos += dir * (t + 2);

                    g_MatrixMap->AddEffectSpawner(pos.x, pos.y, pos.z, Float2Int(FRND(15000) + 5000), type);
                }
            }

            g_MatrixMap->StaticDelete(this);
            return;
        }
        m_HitPoint = -10000000;
    }
    if (m_HitPoint < 0) {
        if (m_GGraph) {
            m_HitPoint -= cms;
            // explosions
            while (g_MatrixMap->GetTime() > m_NextExplosionTimeSound) {
                m_NextExplosionTimeSound += IRND(BUILDING_EXPLOSION_PERIOD_SND_2 - BUILDING_EXPLOSION_PERIOD_SND_1) +
                                            BUILDING_EXPLOSION_PERIOD_SND_1;
                CSound::AddSound(S_EXPLOSION_BUILDING_BOOM, GetGeoCenter());
            }
            while (g_MatrixMap->GetTime() > m_NextExplosionTime) {
                m_NextExplosionTime += BUILDING_EXPLOSION_PERIOD;

                D3DXVECTOR3 dir, pos, pos0;
                float t;

                int cnt = 4;
                do {
                    pos0.x = m_Pos.x - m_Core->m_Matrix._21 * 60;
                    pos0.y = m_Pos.y - m_Core->m_Matrix._22 * 60;
                    pos0.z = m_Core->m_Matrix._43;

                    pos = pos0 + D3DXVECTOR3(FSRND(GetRadius()), FSRND(GetRadius()), FRND(2 * GetRadius()));

                    auto tmp = pos0 - pos;
                    D3DXVec3Normalize(&dir, &tmp);
                }
                while (!Pick(pos, dir, &t) && (--cnt > 0));
                if (cnt > 0) {
                    // CHelper::Create(1,0)->Line(pos, pos+dir * 10);

                    if (FRND(1) < 0.04f)
                        CMatrixEffect::CreateExplosion(pos + dir * (t + 2), ExplosionBuildingBoom2);
                    else
                        CMatrixEffect::CreateExplosion(pos + dir * (t + 2), ExplosionBuildingBoom);
                }

                // if (Damage(WEAPON_ABLAZE, pos, dir)) return;
            }
        }

        return;
    }

    if (m_Kind == BUILDING_BASE) {
        float oldf = m_BaseFloor;
        if (m_State == BASE_OPENING) {
            m_BaseFloor += BASE_FLOOR_SPEED * cms;
            if (m_BaseFloor > 1.0f) {
                m_BaseFloor = 1.0f;
                m_State = BASE_OPENED;

                CSound::AddSound(S_PLATFORM_UP_STOP, GetGeoCenter());
            }
        }
        if (m_State == BASE_CLOSING) {
            m_BaseFloor -= BASE_FLOOR_SPEED * cms;
            if (m_BaseFloor < 0.0f) {
                m_BaseFloor = 0.0f;
                m_State = BASE_CLOSED;
                CSound::AddSound(S_DOORS_CLOSE_STOP, GetGeoCenter());
            }
        }

        if (m_BaseFloor != oldf) {
            CVectorObjectGroupUnit *gu;

            gu = m_GGraph->GetById(1);
            SETFLAG(gu->m_Flags, VOUF_MATRIX_USE);
            D3DXMatrixTranslation(&(gu->m_Matrix), 0, 0, -(1.0f - m_BaseFloor) * 63.0f - 3.0f);

            gu = m_GGraph->GetById(2);
            SETFLAG(gu->m_Flags, VOUF_MATRIX_USE);
            D3DXMatrixTranslation(&(gu->m_Matrix), std::min(m_BaseFloor * 2, 1.0f) * 25.0f, 0, 0);

            gu = m_GGraph->GetById(3);
            SETFLAG(gu->m_Flags, VOUF_MATRIX_USE);
            D3DXMatrixTranslation(&(gu->m_Matrix), -std::min(m_BaseFloor * 2, 1.0f) * 25.0f, 0, 0);

            RChange(MR_Matrix);
        }
    }

    // ESound snd = S_BASE_AMBIENT;
    // if (m_Kind == BUILDING_TITAN)
    //{
    //    snd = S_TITAN_AMBIENT;
    //} else
    // if (m_Kind == BUILDING_ELECTRONIC)
    //{
    //    snd = S_ELECTRONIC_AMBIENT;
    //} else
    // if (m_Kind == BUILDING_ENERGY)
    //{
    //    snd = S_ENERGY_AMBIENT;
    //} else
    // if (m_Kind == BUILDING_REPAIR)
    //{
    //    snd = S_REPAIR_AMBIENT;
    //} else
    // if (m_Kind == BUILDING_PLASMA)
    //{
    //    snd = S_PLASMA_AMBIENT;
    //}

    // float dist2 = D3DXVec3LengthSq(&(g_MatrixMap->m_Camera.GetFrustumCenter() - GetGeoCenter()));
    // if (dist2 > CSound::GetSoundMaxDistSQ(snd))
    //{
    //    CSound::StopPlay(m_AmbientSound);
    //    m_AmbientSound = SOUND_ID_EMPTY;
    //} else
    //{
    //    m_AmbientSound = CSound::Play(m_AmbientSound, snd, GetGeoCenter());
    //}
}

bool CMatrixBuilding::Pick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, float *outt) const {
    DTRACE();
    if (!m_GGraph)
        return false;

    return m_GGraph->Pick(orig, dir, outt) != NULL;
}

void CMatrixBuilding::FreeDynamicResources(void) {
    DTRACE();

    if (m_ShadowProj) {
        m_ShadowProj->DX_Free();
    }
    if (m_ShadowType == SHADOW_STENCIL) {
        m_GGraph->ShadowStencil_DX_Free();
    }
}

void CMatrixBuilding::BeforeDraw(void) {
    DTRACE();
    DWORD sh = (g_Config.m_ShowProjShadows ? (MR_ShadowProjGeom | MR_ShadowProjTex) : 0) |
               (g_Config.m_ShowStencilShadows ? MR_ShadowStencil : 0);
    RNeed(MR_Matrix | MR_Graph | MR_MiniMap | sh);

    // static float pp = 0.3f;
    // if (g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_PGUP) {g_MatrixMap->m_KeyDown = false; pp += 0.01f;}
    // if (g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_PGDN) {g_MatrixMap->m_KeyDown = false; pp -= 0.01f;}
    // if (pp > 1.0f) pp = 1.0f;
    // if (pp < 0) pp = 0;

    if (m_ShowHitpointTime > 0 && m_HitPoint > 0) {
        D3DXVECTOR3 pos;
        float r = GetRadius();
        if (m_Kind == BUILDING_BASE) {
            pos = GetGeoCenter() - *(D3DXVECTOR3 *)&m_Core->m_Matrix._21 * 20;
            r *= 0.7f;
            pos.z = g_MatrixMap->GetZ(pos.x, pos.y) + 90;
        }
        else {
            pos = GetGeoCenter();
            pos.z = g_MatrixMap->GetZ(pos.x, pos.y) + 80;
        }

        if (TRACE_STOP_NONE ==
            g_MatrixMap->Trace(NULL, g_MatrixMap->m_Camera.GetFrustumCenter(), pos, TRACE_LANDSCAPE, NULL)) {
            D3DXVECTOR2 p = g_MatrixMap->m_Camera.Project(pos, g_MatrixMap->GetIdentityMatrix());
            m_PB.Modify(p.x - r, p.y, m_HitPoint * m_MaxHitPointInversed);
        }
    }

    if (m_GGraph)
        m_GGraph->BeforeDraw(g_Config.m_ShowStencilShadows);
}

void CMatrixBuilding::Draw(void) {
    DTRACE();

    if (!m_GGraph)
        return;

    for (int i = 0; i < 4; i++) {
        ASSERT_DX(g_D3DD->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&g_MatrixMap->m_BiasBuildings))));
    }

    //	ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD,&m_Matrix));
    //	m_Graph->DrawSide(map.SideToColor(m_Side),flags);
    DWORD coltex = (DWORD)g_MatrixMap->GetSideColorTexture(m_Side)->Tex();
    g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, m_Core->m_TerainColor);
    //	m_Graph->Draw(flags);
    m_GGraph->Draw(coltex);

    if (!FLAG(g_MatrixMap->m_Flags, MMFLAG_DISABLE_DRAW_OBJECT_LIGHTS)) {
        m_GGraph->DrawLights();
    }
}

void CMatrixBuilding::DrawShadowStencil(void) {
    DTRACE();
    if (m_ShadowType != SHADOW_STENCIL)
        return;
    m_GGraph->ShadowStencilDraw();
}

void CMatrixBuilding::DrawShadowProj() {
    // DTRACE();

    // if(m_ShadowType!=SHADOW_PROJ_DYNAMIC || m_ShadowType!=SHADOW_PROJ_STATIC) return;

    //    m_ShadowProj->Render();

    //   D3DXMATRIX m = g_MatrixMap->GetIdentityMatrix();
    //   m._41 = m_ShadowProj->GetDX();
    //   m._42 = m_ShadowProj->GetDY();
    // ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD,&m));
}

float CMatrixBuilding::GetFloorZ(void) {
    return m_BuildZ + (1.0f - m_BaseFloor) * BASE_FLOOR_Z - 3.0f + 2.7f;
}

void CMatrixBuilding::OnLoad(void) {
    DTRACE();
    m_PB.Modify(1000000, 0, PB_BUILDING_WIDTH, 1);

    /*    int* tmp = m_Places;

        int b_x = (int)(m_Pos.x / GLOBAL_SCALE_MOVE);
        int b_y = (int)(m_Pos.y / GLOBAL_SCALE_MOVE);

        *tmp = b_x - 10;
        *(++tmp) = b_y - 10;

        *(++tmp) = b_x + 10;
        *(++tmp) = b_y + 10;

        *(++tmp) = b_x - 10;
        *(++tmp) = b_y + 10;

        *(++tmp) = b_x + 10;
        *(++tmp) = b_y - 10;*/

    m_BuildZ = g_MatrixMap->GetZ(m_Pos.x, m_Pos.y);
    if (m_BuildZ < WATER_LEVEL) {
        D3DXVECTOR3 pos(0, 0, WATER_LEVEL);
        g_MatrixMap->Trace(&pos, D3DXVECTOR3(m_Pos.x, m_Pos.y, 1000), D3DXVECTOR3(m_Pos.x, m_Pos.y, -1000), TRACE_ALL,
                           this);
        m_BuildZ = pos.z;
    }

    switch (m_Kind) {
        case BUILDING_BASE:
            m_TurretsMax = BASE_TURRETS;
            m_Name = L"BASE";
            break;
        case BUILDING_TITAN:
            m_TurretsMax = TITAN_TURRETS;
            m_Name = L"TITAN";
            break;
        case BUILDING_PLASMA:
            m_TurretsMax = PLASMA_TURRETS;
            m_Name = L"PLASMA";
            break;
        case BUILDING_ELECTRONIC:
            m_TurretsMax = ELECTRONIC_TURRETS;
            m_Name = L"ELECTRONICS";
            break;
        case BUILDING_ENERGY:
            m_TurretsMax = ENERGY_TURRETS;
            m_Name = L"ENERGY";
            break;
        case BUILDING_REPAIR:
            m_TurretsMax = REPAIR_TURRETS;
            m_Name = L"REPAIR";
            break;
    }

    if (m_Kind == BUILDING_BASE) {
        int px = Float2Int((m_Pos.x - (GLOBAL_SCALE / 2)) * INVERT(GLOBAL_SCALE)) - 1;
        int py = Float2Int((m_Pos.y - (GLOBAL_SCALE / 2)) * INVERT(GLOBAL_SCALE)) - 1;

        for (int x = 0; x < 3; ++x)
            for (int y = 0; y < 3; ++y) {
                SMatrixMapUnit *mu = g_MatrixMap->UnitGet(x + px, y + py);
                mu->m_Base = this;
            }
    }

    m_ShowHitpointTime = 0;
    m_defHitPoint = Float2Int(m_HitPoint);
}

bool CMatrixBuilding::CalcBounds(D3DXVECTOR3 &omin, D3DXVECTOR3 &omax) {
    DTRACE();

    if (!(m_GGraph) || !(m_GGraph->m_First))
        return true;

    m_GGraph->BoundGetAllFrame(omin, omax);

    return false;

    /*    SVOFrame *f = m_GGraph->m_First->m_Obj->VO()->FrameGet(0);
        min.x = f->m_MinX;
        min.y = f->m_MinY;
        max.x = f->m_MaxX;
        max.y = f->m_MaxY;

        int cnt = m_GGraph->m_First->m_Obj->VO()->Header()->m_FrameCnt;
        for (int i = 1; i<cnt; ++i)
        {
            SVOFrame *f = m_GGraph->m_First->m_Obj->VO()->FrameGet(i);

            min.x = min(min.x,f->m_MinX);
            min.y = min(min.y,f->m_MinY);
            max.x = max(max.x,f->m_MaxX);
            max.y = max(max.y,f->m_MaxY);
        }

        return false;*/
}

struct SFindRobotForCapture {
    CMatrixRobotAI *by;
    CMatrixRobotAI *found;
    float dist2;
};

static bool FindRobotForCapture(const D3DXVECTOR2 &center, CMatrixMapStatic *ms, DWORD user) {
    SFindRobotForCapture *data = (SFindRobotForCapture *)user;

    auto tmp = center - *(D3DXVECTOR2 *)&ms->GetGeoCenter();
    float dist2 = D3DXVec2LengthSq(&tmp);
    if (dist2 < data->dist2) {
        if (data->by == data->found) {
            data->by = NULL;
            return false;
        }

        data->found = (CMatrixRobotAI *)ms;
        data->dist2 = dist2;
    }

    return true;
}

void CMatrixBuilding::SetNeutral(void) {
    m_TrueColor.m_ColoredCnt = 0;
    m_TrueColor.m_Color = 0;
    m_Side = 0;
    m_BS.ClearStack();
    RChange(MR_MiniMap);
}

ECaptureStatus CMatrixBuilding::Capture(CMatrixRobotAI *by) {
    if (m_InCaptureTime <= 0) {
        m_InCaptureTime = g_Config.m_CaptureTimeErase + g_Config.m_CaptureTimePaint;
        m_InCaptureNextTimeErase = g_MatrixMap->GetTime();
        m_InCaptureNextTimePaint = g_MatrixMap->GetTime();
    }

    if (m_InCaptureNextTimeErase >= g_MatrixMap->GetTime() || m_InCaptureNextTimePaint >= g_MatrixMap->GetTime())
        return CAPTURE_BUSY;

    SFindRobotForCapture data;
    data.by = by;
    data.found = NULL;
    data.dist2 = CAPTURE_RADIUS * CAPTURE_RADIUS;

    g_MatrixMap->FindObjects(m_Pos, CAPTURE_RADIUS, 1, TRACE_ROBOT, NULL, FindRobotForCapture, (DWORD)&data);

    if (data.by == NULL || data.by != data.found)
        return CAPTURE_TOOFAR;

    DWORD captureer_color = 0xFF000000 | g_MatrixMap->GetSideColor(by->GetSide());

    m_InCaptureTime = g_Config.m_CaptureTimeErase + g_Config.m_CaptureTimePaint;

    if (m_Side == 0) {
        if (m_TrueColor.m_Color == 0 || m_TrueColor.m_Color == captureer_color) {
            m_TrueColor.m_Color = captureer_color;
            while (m_InCaptureNextTimePaint < g_MatrixMap->GetTime()) {
                m_InCaptureNextTimePaint += g_Config.m_CaptureTimePaint;
                m_InCaptureNextTimeErase = g_MatrixMap->GetTime();

                if (m_TrueColor.m_ColoredCnt == MAX_ZAHVAT_POINTS) {
                    int side = by->GetSide();
                    if (side == PLAYER_SIDE)
                        CSound::Play(S_ENEMY_FACTORY_CAPTURED);
                    m_Side = side;
                    m_BS.ClearStack();
                    RChange(MR_MiniMap);
                    return CAPTURE_DONE;
                }
                ++m_TrueColor.m_ColoredCnt;
            }
        }
        else {
            while (m_InCaptureNextTimeErase < g_MatrixMap->GetTime()) {
                m_InCaptureNextTimeErase += g_Config.m_CaptureTimeErase;
                m_InCaptureNextTimePaint = g_MatrixMap->GetTime();

                if (m_TrueColor.m_ColoredCnt == 0) {
                    m_TrueColor.m_Color = 0;
                    return CAPTURE_INPROGRESS;
                }
                --m_TrueColor.m_ColoredCnt;
            }
        }
    }
    else {
        if (m_TrueColor.m_Color == captureer_color) {
            while (m_InCaptureNextTimePaint < g_MatrixMap->GetTime()) {
                m_InCaptureNextTimePaint += g_Config.m_CaptureTimePaint;
                m_InCaptureNextTimeErase = g_MatrixMap->GetTime();

                if (m_TrueColor.m_ColoredCnt == MAX_ZAHVAT_POINTS) {
                    // дозахват
                    int side = by->GetSide();
                    // if (side == PLAYER_SIDE)
                    //    CSound::Play(S_ENEMY_FACTORY_CAPTURED);
                    m_Side = side;
                    RChange(MR_MiniMap);

                    return CAPTURE_DONE;
                }
                ++m_TrueColor.m_ColoredCnt;
            }
        }
        else {
            while (m_InCaptureNextTimeErase < g_MatrixMap->GetTime()) {
                m_InCaptureNextTimeErase += g_Config.m_CaptureTimeErase;
                m_InCaptureNextTimePaint = g_MatrixMap->GetTime() + g_Config.m_CaptureTimePaint;

                if (m_TrueColor.m_ColoredCnt == 0) {
                    if (m_Side == PLAYER_SIDE)
                        CSound::Play(S_PLAYER_FACTORY_CAPTURED);

                    m_TrueColor.m_Color = 0;
                    m_Side = 0;
                    RChange(MR_MiniMap);
                    m_BS.ClearStack();
                    return CAPTURE_INPROGRESS;
                }
                --m_TrueColor.m_ColoredCnt;
            }
        }
    }

    return CAPTURE_INPROGRESS;
}

void CMatrixBuilding::Maintenance(void) {
    if (m_Side == 0)
        return;
    if (g_MatrixMap->MaintenanceDisabled())
        return;
    if (g_MatrixMap->BeforeMaintenanceTime() > 0)
        return;

    int cx = Float2Int(m_Pos.x * INVERT(GLOBAL_SCALE_MOVE));  // - ROBOT_MOVECELLS_PER_SIZE/2;
    int cy = Float2Int(m_Pos.y * INVERT(GLOBAL_SCALE_MOVE));  // - ROBOT_MOVECELLS_PER_SIZE/2;

    if (m_Kind == 0) {
        switch (m_Angle) {
            case 0:
                cy += 5 + 1;
                break;
            case 1:
                cx -= 6 + 1;
                break;
            case 2:
                cy -= 6 + 1;
                break;
            case 3:
                cx += 5 + 1;
                break;
        }
    }

    // m_Pos.x*GLOBAL_SACLE_MOVE+ROBOT_MOVECELLS_PER_SIZE*GLOBAL_SCALE_MOVE/2

    g_MatrixMap->PlaceFindNear(0, 4, cx, cy, 0, NULL, NULL);

    CMatrixSideUnit *su = g_MatrixMap->GetSideById(m_Side);

    int listcnt;
    su->BufPrepare();
    int ret = g_MatrixMap->PlaceList(1 + 2 + 4 + 8 + 16, CPoint(cx, cy), CPoint(cx, cy), 100, false, su->m_PlaceList,
                                     &listcnt);
    if (ret == 0)
        return;
    g_MatrixMap->InitMaintenanceTime();
    CSound::Play(S_MAINTENANCE);
    // g_MatrixMap->PlaceListGrow(1+2+4+8+16, g_MatrixMap->GetPlayerSide()->m_PlaceList, &listcnt, 2);

    for (int i = 0; i < listcnt; i++) {
        g_MatrixMap->m_RN.m_Place[su->m_PlaceList[i]].m_Data = 0;
    }
    CMatrixMapStatic *obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveCannon()) {
            g_MatrixMap->m_RN.m_Place[obj->AsCannon()->m_Place].m_Data = 1;
        }
        else if (obj->IsLiveRobot() && obj->AsRobot()->GetEnv()->m_Place >= 0) {
            g_MatrixMap->m_RN.m_Place[obj->AsRobot()->GetEnv()->m_Place].m_Data = 1;
        }
        obj = obj->GetNextLogic();
    }

    float angle = FSRND(M_PI);

    CBlockPar *bp = g_MatrixData->BlockGet(PAR_SOURCE_FLYER_ORDERS)->BlockGet(PAR_SOURCE_FLYER_ORDERS_GIVE_BOT);

    int score = 0;

    int pli = 0;
    for (; score < 100;) {
        if (pli >= listcnt)
            break;

        int place = su->m_PlaceList[pli++];
        if (g_MatrixMap->m_RN.m_Place[place].m_Data)
            continue;
        if (g_MatrixMap->m_RN.m_Place[place].m_Move & (1 + 2 + 4 + 8 + 16))
            continue;
        const CPoint &pos = g_MatrixMap->m_RN.m_Place[place].m_Pos;

        int botpar_i;

        int cnt = 10;
        int sc = 0;
        for (; cnt > 0; --cnt) {
            botpar_i = g_MatrixMap->Rnd(0, bp->BlockCount() - 1);
            sc = bp->BlockGetName(botpar_i).GetInt();
            if (score + sc > 130)
                continue;
            break;
        }
        score += sc;
        if (sc > 130)
            break;

        CPoint bpos(Float2Int((m_Pos.x + m_Core->m_Matrix._21 * 50) * INVERT(GLOBAL_SCALE_MOVE)) -
                            ROBOT_MOVECELLS_PER_SIZE / 2,
                    Float2Int((m_Pos.y + m_Core->m_Matrix._22 * 50) * INVERT(GLOBAL_SCALE_MOVE)) -
                            ROBOT_MOVECELLS_PER_SIZE / 2);

        su->OrderFlyer(D3DXVECTOR2(pos.x * GLOBAL_SCALE_MOVE + ROBOT_MOVECELLS_PER_SIZE * GLOBAL_SCALE_MOVE / 2,
                                   pos.y * GLOBAL_SCALE_MOVE + ROBOT_MOVECELLS_PER_SIZE * GLOBAL_SCALE_MOVE / 2),
                       FO_GIVE_BOT, angle, place, bpos, botpar_i);

        // CHelper::Create(100,0)->Cone(D3DXVECTOR3(pos.x*GLOBAL_SCALE_MOVE,pos.y*GLOBAL_SCALE_MOVE,0),
        //    D3DXVECTOR3(pos.x*GLOBAL_SCALE_MOVE,pos.y*GLOBAL_SCALE_MOVE,100),10,10,0xFFFFFFFF,0,10);
    }
}

bool CMatrixBuilding::BuildFlyer(EFlyerKind kind) {
    CMatrixFlyer *fl = g_MatrixMap->StaticAdd<CMatrixFlyer>();
    fl->m_FlyerKind = kind;

    fl->Begin(this);

    return true;
}

void CMatrixBuilding::ReleaseMe(void) {
    DTRACE();

    DeletePlacesShow();

    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
    if (GetSide() == PLAYER_SIDE) {
        if (ps->m_ActiveObject == this) {
            ps->PLDropAllActions();
        }
        ps->RemoveFromSelection(this);
    }

    // for(int c = 1; c <= g_MatrixMap->m_SideCnt; c++){
    //    s = g_MatrixMap->GetSideById(c);
    //    if(s->m_Id == PLAYER_SIDE && s->m_ActiveObject == this){
    //        s->Select(NOTHING, NULL);
    //    }
    // if(s != g_MatrixMap->GetPlayerSide() && s->m_GroupsList){
    //    CMatrixGroup* grps = s->m_GroupsList->m_FirstGroup;
    //    while(grps){
    //        if(grps->m_Tactics && grps->m_Tactics->GetTarget() == this){
    //            grps->DeInstallTactics();
    //        }
    //        grps = grps->m_NextGroup;
    //    }
    //}else{
    //    if(s->m_CurGroup){
    //        CMatrixTactics* t = s->m_CurGroup->GetTactics();
    //        if(t && t->GetTarget() == this){
    //            s->m_CurGroup->DeInstallTactics();
    //        }
    //    }
    //}
    //}

    CMatrixMapStatic *objects = CMatrixMapStatic::GetFirstLogic();

    while (objects) {
        if (objects->IsLiveRobot()) {
            if (objects->AsRobot()->GetBase() == this) {
                objects->AsRobot()->SetBase(NULL);
                objects->AsRobot()->MustDie();
            }

            objects->AsRobot()->RemoveCaptureCandidate(this);
            objects->AsRobot()->GetEnv()->RemoveFromList(this);
            if (objects->AsRobot()->GetCaptureFactory() == this) {
                objects->AsRobot()->StopCapture();
            }
        }
        else if (objects->IsCannon() && objects->AsCannon()->m_ParentBuilding == this) {
            objects->AsCannon()->m_ParentBuilding = NULL;
        }
        objects = objects->GetNextLogic();
    }
}

bool CMatrixBuilding::InRect(const CRect &rect) const {
    D3DXVECTOR3 dir;
    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.left, rect.top), dir);
    if (Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, NULL))
        return true;

    D3DXMATRIX s;
    SEVH_data d;

    d.m = g_MatrixMap->m_Camera.GetViewMatrix() * g_MatrixMap->m_Camera.GetProjMatrix();
    D3DXMatrixScaling(&s, float(g_ScreenX / 2), float(-g_ScreenY / 2), 1);
    s._41 = s._11;
    s._42 = float(g_ScreenY / 2);
    d.m *= s;
    d.found = false;
    d.rect = &rect;

    m_GGraph->EnumFrameVerts_transform_position(EnumVertsHandler, (DWORD)&d);
    if (d.found)
        return true;

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

bool CMatrixBuilding::Select(void) {
    D3DXVECTOR3 pos;

    pos.x = m_Pos.x - m_Core->m_Matrix._21 * 60;
    pos.y = m_Pos.y - m_Core->m_Matrix._22 * 60;
    pos.z = m_Core->m_Matrix._43 + 5;

    float r = BUILDING_SELECTION_SIZE;

    if (m_Kind == BUILDING_BASE) {
        r = BUILDING_SELECTION_SIZE + 24;
        pos.x -= m_Core->m_Matrix._11 * 7;
        pos.y -= m_Core->m_Matrix._12 * 7;

        pos.x += m_Core->m_Matrix._21 * 16;
        pos.y += m_Core->m_Matrix._22 * 16;
    }
    if (m_Kind == BUILDING_ENERGY) {
        r = BUILDING_SELECTION_SIZE + 10;
        pos.x -= m_Core->m_Matrix._21 * 13;
        pos.y -= m_Core->m_Matrix._22 * 13;
    }
    if (m_Kind == BUILDING_PLASMA) {
        r = BUILDING_SELECTION_SIZE + 15;
        pos.x -= m_Core->m_Matrix._21 * 17;
        pos.y -= m_Core->m_Matrix._22 * 17;
    }
    if (m_Kind == BUILDING_TITAN) {
        r = BUILDING_SELECTION_SIZE + 15;
        pos.x -= m_Core->m_Matrix._21 * 17;
        pos.y -= m_Core->m_Matrix._22 * 17;
    }
    if (m_Kind == BUILDING_ELECTRONIC) {
        r = BUILDING_SELECTION_SIZE + 17;
        pos.x -= m_Core->m_Matrix._21 * 17;
        pos.y -= m_Core->m_Matrix._22 * 17;
    }

    m_Selection = (CMatrixEffectSelection *)CMatrixEffect::CreateSelection(pos, r);
    if (!g_MatrixMap->AddEffect(m_Selection)) {
        m_Selection = NULL;
        return false;
    }
    return true;
}

void CMatrixBuilding::UnSelect(void) {
    if (m_Selection) {
        m_Selection->Kill();
        m_Selection = NULL;
    }
}

void CMatrixBuilding::CreateProgressBarClone(float x, float y, float width, EPBCoord clone_type) {
    m_PB.CreateClone(clone_type, x, y, width);
}

void CMatrixBuilding::DeleteProgressBarClone(EPBCoord clone_type) {
    m_PB.KillClone(clone_type);
}

int CMatrixBuilding::GetPlacesForTurrets(CPoint *places) {
    int cx = Float2Int(m_Pos.x / GLOBAL_SCALE_MOVE);
    int cy = Float2Int(m_Pos.y / GLOBAL_SCALE_MOVE);

    int cnt = 0;
    //    int dist[MAX_PLACES];

    SMatrixPlace *place = g_MatrixMap->m_RN.m_Place;
    for (int i = 0; i < g_MatrixMap->m_RN.m_PlaceCnt; i++, place++)
        place->m_Data = 0;

    CMatrixMapStatic *obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveCannon()) {
            if (obj->AsCannon()->m_Place >= 0) {
                place = g_MatrixMap->m_RN.GetPlace(obj->AsCannon()->m_Place);
                place->m_Data = 1;
            }
        }
        if (obj->IsLiveBuilding()) {
            CMatrixMapStatic *bi = obj->AsBuilding()->m_BS.GetTopItem();
            while (bi) {
                if (bi->IsCannon()) {
                    place = g_MatrixMap->m_RN.GetPlace(bi->AsCannon()->m_Place);
                    place->m_Data = 1;
                }
                bi = bi->m_NextStackItem;
            }
        }
        obj = obj->GetNextLogic();
    }

    for (int i = 0; i < m_TurretsPlacesCnt; i++) {
        place = g_MatrixMap->m_RN.GetPlace(g_MatrixMap->m_RN.FindInPL(m_TurretsPlaces[i].m_Coord));
        if (place->m_Data)
            continue;

        places[cnt] = m_TurretsPlaces[i].m_Coord;
        cnt++;
    }

    /*    CRect plr=g_MatrixMap->m_RN.CorrectRectPL(CRect(cx-16*2,cy-16*2,cx+16*2,cy+16*2));
        SMatrixPlaceList * plist=g_MatrixMap->m_RN.m_PLList+plr.left+plr.top*g_MatrixMap->m_RN.m_PLSizeX;
        for(int y=plr.top;y<plr.bottom;y++,plist+=g_MatrixMap->m_RN.m_PLSizeX-(plr.right-plr.left)) {
            for(int x=plr.left;x<plr.right;x++,plist++) {
                place=g_MatrixMap->m_RN.m_Place+plist->m_Sme;
                for(int u=0;u<plist->m_Cnt;u++,place++) {
                    if(place->m_Data) continue;

                    for(i=0;i<m_TurretsPlaceCnt;i++) {
                        if(m_TurretsPlace[i]==place->m_Pos) break;
                    }
                    if(i>=m_TurretsPlaceCnt) continue;

                    int cd=POW2(place->m_Pos.x-cx)+POW2(place->m_Pos.y-cy);
                    if(cd>POW2(40)) continue;

                    for(i=0;i<cnt;i++) if(cd<dist[i]) break;
                    if(i>=MAX_PLACES) continue;
                    else if(i>=cnt) {
                        cnt++;
                    } else if(cnt<MAX_PLACES) {
                        CopyMemory(dist+i+1,dist+i,(cnt-i)*sizeof(int));
                        CopyMemory(places+i+1,places+i,(cnt-i)*sizeof(CPoint));
                        cnt++;
                    } else {
                        CopyMemory(dist+i+1,dist+i,(cnt-i-1)*sizeof(int));
                        CopyMemory(places+i+1,places+i,(cnt-i-1)*sizeof(CPoint));
                    }
                    dist[i]=cd;
                    places[i]=place->m_Pos;
                    places[i].x+=ROBOT_MOVECELLS_PER_SIZE/2;
                    places[i].y+=ROBOT_MOVECELLS_PER_SIZE/2;

                }
            }
        }*/
    return cnt;
}

void CMatrixBuilding::CreatePlacesShow(void) {
    DTRACE();

    DeletePlacesShow();

    CPoint pl[MAX_PLACES];
    int cnt = GetPlacesForTurrets(pl);

    m_PlacesShow = (SEffectHandler *)HAlloc(sizeof(SEffectHandler) * MAX_PLACES, g_MatrixHeap);
    for (int i = 0; i < MAX_PLACES; ++i) {
#ifdef _DEBUG
        new(&m_PlacesShow[i]) SEffectHandler(DEBUG_CALL_INFO);
#else
        new(&m_PlacesShow[i]) SEffectHandler();
#endif
    }

    for (int i = 0; i < cnt; ++i) {
        float x = (float)pl[i].x;
        float y = (float)pl[i].y;
        CMatrixEffect::CreateLandscapeSpot(m_PlacesShow + i, D3DXVECTOR2(x * GLOBAL_SCALE_MOVE, y * GLOBAL_SCALE_MOVE),
                                           0, 6, SPOT_TURRET);
    }
}

void CMatrixBuilding::DeletePlacesShow() {
    DTRACE();
    if (IsLiveBuilding() && m_PlacesShow) {
        for (int i = 0; i < MAX_PLACES; i++) {
#ifdef _DEBUG
            m_PlacesShow[i].Release(DEBUG_CALL_INFO);
#else
            m_PlacesShow[i].Release();
#endif
        }
        HFree(m_PlacesShow, g_MatrixHeap);
        m_PlacesShow = NULL;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CBuildStack::TickTimer(int ms) {
    if (!m_Items /*|| (m_ParentBase->IsBase() && m_ParentBase->m_State != BASE_CLOSED)*/) {
        m_Timer = 0;
        return;
    }

    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
    m_Timer += ms;

    if (m_Items && m_Top->IsRobot()) {
        float x = g_IFaceList->GetMainX() + 283;
        float y = g_IFaceList->GetMainY() + 71;

        m_PB.Modify(100000.0f, 0);
        if (m_Timer <= g_Config.m_Timings[UNIT_ROBOT]) {
            m_PB.Modify(float(m_Timer) / float(g_Config.m_Timings[UNIT_ROBOT]));
        }

        if (ps->m_CurrSel == BASE_SELECTED && ps->m_ActiveObject == m_ParentBase) {
            m_PB.CreateClone(PBC_CLONE1, x, y, 87);
        }
        else {
            m_PB.KillClone(PBC_CLONE1);
        }
        if (m_Timer >= g_Config.m_Timings[UNIT_ROBOT] && m_ParentBase->m_State == BASE_CLOSED) {
            if (m_Top->GetSide() == PLAYER_SIDE && !FLAG(g_MatrixMap->m_Flags, MMFLAG_AUTOMATIC_MODE)) {
                if (g_MatrixMap->Rnd(0, 1)) {
                    CSound::Play(S_ROBOT_BUILD_END, SL_ALL);
                }
                else {
                    CSound::Play(S_ROBOT_BUILD_END_ALT, SL_ALL);
                }
            }

            m_Timer = 0;
            m_PB.KillClone(PBC_CLONE1);

            // produce robot, del from stack
            // STUB:
            if (m_ParentBase->GetSide() == PLAYER_SIDE) {
                g_IFaceList->DeleteStackIcon(1, m_ParentBase);
            }
            // m_ParentBase->m_BusyFlag.SetBusy((CMatrixRobotAI*)m_Top); // do not busy flag while building robot

            m_Top->AsRobot()->RobotSpawn(((CMatrixRobotAI *)m_Top)->GetBase());
            m_Top->AsRobot()->JoinToGroup();

            g_MatrixMap->AddObject(m_Top, true);

            LIST_DEL(m_Top, m_Top, m_Bottom, m_PrevStackItem, m_NextStackItem);
            m_Items--;
        }
    } /*else if(m_Items && m_Top->GetObjectType() == OBJECT_TYPE_FLYER){
         float x = g_IFaceList->GetMainX()+283;
         float y = g_IFaceList->GetMainY()+71;

         m_PB.Modify(100000.0f, 0);
         if(m_Timer <= g_Config.m_Timings[UNIT_FLYER]){
             m_PB.Modify(m_Timer * 1.0f/g_Config.m_Timings[UNIT_FLYER]);
         }


         if(ps->m_CurrSel == BASE_SELECTED && ps->m_ActiveObject == m_ParentBase){
             m_PB.CreateClone(PBC_CLONE1, x, y, 87);
         }else{
             m_PB.KillClone(PBC_CLONE1);
         }

         if(m_Timer >= g_Config.m_Timings[UNIT_FLYER] && m_ParentBase->m_State == BASE_CLOSED){
             if(m_Top->GetSide() == PLAYER_SIDE){
                 if(g_MatrixMap->Rnd(0,1)){
                     CSound::Play(S_FLYER_BUILD_END, SL_ALL);
                 }else{
                     CSound::Play(S_FLYER_BUILD_END_ALT, SL_ALL);
                 }
             }

             m_Timer = 0;
             m_PB.KillClone(PBC_CLONE1);
             //produce flyer, del from stack
             //STUB:
             if(m_ParentBase->GetSide() == PLAYER_SIDE){
                 g_IFaceList->DeleteStackIcon(1, m_ParentBase);
             }


             g_MatrixMap->AddObject(m_Top, true);

             m_Top->AddLT();
             ((CMatrixFlyer*)m_Top)->Begin(m_ParentBase);

             LIST_DEL(m_Top, m_Top, m_Bottom, m_PrevStackItem, m_NextStackItem);
             m_Items--;

         }
     }*/
    else if (m_Items && m_Top->IsCannon()) {
        float x = g_IFaceList->GetMainX() + 283;
        float y = g_IFaceList->GetMainY() + 71;
        float percent_done = float(m_Timer) / float(g_Config.m_Timings[UNIT_TURRET]);
        m_PB.Modify(100000.0f, 0);
        m_PB.Modify(percent_done);

        //((CMatrixCannon*)m_Top)->SetPBOutOfScreen();
        m_Top->AsCannon()->SetHitPoint(m_Top->AsCannon()->GetMaxHitPoint() * percent_done);

        if ((ps->m_CurrSel == BASE_SELECTED || ps->m_CurrSel == BUILDING_SELECTED) &&
            ps->m_ActiveObject == m_ParentBase) {
            m_PB.CreateClone(PBC_CLONE1, x, y, 87);
        }
        else {
            m_PB.KillClone(PBC_CLONE1);
        }
        if (m_Timer >= g_Config.m_Timings[UNIT_TURRET]) {
            if (m_Top->GetSide() == PLAYER_SIDE) {
                if (m_Top->AsCannon()->m_Num == 1) {
                    CSound::Play(S_TURRET_BUILD_0, SL_ALL);
                }
                else if (m_Top->AsCannon()->m_Num == 2) {
                    CSound::Play(S_TURRET_BUILD_1, SL_ALL);
                }
                else if (m_Top->AsCannon()->m_Num == 3) {
                    CSound::Play(S_TURRET_BUILD_2, SL_ALL);
                }
                else if (m_Top->AsCannon()->m_Num == 4) {
                    CSound::Play(S_TURRET_BUILD_3, SL_ALL);
                }
            }

            m_Timer = 0;
            m_PB.KillClone(PBC_CLONE1);

            for (int i = 0; i < MAX_PLACES; i++) {
                if (m_ParentBase->m_TurretsPlaces[i].m_Coord.x ==
                            Float2Int(m_Top->AsCannon()->m_Pos.x / GLOBAL_SCALE_MOVE) &&
                    m_ParentBase->m_TurretsPlaces[i].m_Coord.y ==
                            Float2Int(m_Top->AsCannon()->m_Pos.y / GLOBAL_SCALE_MOVE)) {
                    m_ParentBase->m_TurretsPlaces[i].m_CannonType = m_Top->AsCannon()->m_Num;
                }
            }

            // STUB:
            if (m_ParentBase->GetSide() == PLAYER_SIDE) {
                g_IFaceList->DeleteStackIcon(1, m_ParentBase);
                if (g_MatrixMap->GetPlayerSide()->m_ActiveObject == m_ParentBase) {
                    g_IFaceList->CreateDynamicTurrets(m_ParentBase);
                }
            }

            g_MatrixMap->m_Minimap.AddEvent(m_Top->GetGeoCenter().x, m_Top->GetGeoCenter().y, 0xffffff00, 0xffffff00);
            m_Top->AsCannon()->m_CurrState = CANNON_IDLE;
            int ss = m_Top->GetSide();
            if (ss != 0)
                g_MatrixMap->GetSideById(ss)->IncStatValue(STAT_TURRET_BUILD);

            m_Top->ResetInvulnerability();
            LIST_DEL(m_Top, m_Top, m_Bottom, m_PrevStackItem, m_NextStackItem);

            m_Items--;
        }
    }

    // if(m_Top){
    //    CMatrixMapStatic* o = m_Top->m_NextStackItem;
    //    while(o){
    //        if(o->GetObjectType() == OBJECT_TYPE_CANNON){
    //            ((CMatrixCannon*)o)->SetTerainColor(0xFF00FF00);
    //        }
    //        o = o->m_NextStackItem;
    //    }
    //}
}

void CBuildStack::AddItem(CMatrixMapStatic *item) {
    if (item && m_Items < MAX_STACK_UNITS) {
        LIST_ADD(item, m_Top, m_Bottom, m_PrevStackItem, m_NextStackItem);
        m_Items++;
        // STUB:
        if (item->GetSide() == PLAYER_SIDE) {
            g_IFaceList->CreateStackIcon(m_Items, m_ParentBase, item);
        }
    }
}

int CBuildStack::DeleteItem(int no) {
    if (m_Items) {
        if (no == 1) {
            m_Timer = 0;
            m_PB.KillClone(PBC_CLONE1);
        }
        // STUB:
        if (m_ParentBase->GetSide() == PLAYER_SIDE) {
            g_IFaceList->DeleteStackIcon(no, m_ParentBase);
        }

        CMatrixMapStatic *items = m_Top;
        int i = 0;
        while (items) {
            i++;
            if (i == no) {
                LIST_DEL(items, m_Top, m_Bottom, m_PrevStackItem, m_NextStackItem);
                if (items->IsRobot()) {
                    ReturnRobotResources(items->AsRobot());
                    HDelete(CMatrixRobotAI, (CMatrixRobotAI *)items, g_MatrixHeap);
                }
                else if (items->IsCannon()) {
                    ReturnTurretResources(items->AsCannon());
                    items->UnjoinGroup();
                    g_MatrixMap->StaticDelete(items);
                }
                else if (items->IsFlyer()) {
                    HDelete(CMatrixFlyer, (CMatrixFlyer *)items, g_MatrixHeap);
                }
                m_Items--;
                return m_Items;
            }
            items = items->m_NextStackItem;
        }
    }
    return 0;
}

CBuildStack::~CBuildStack() {
    if (m_Items) {
        CMatrixMapStatic *items = m_Top;

        while (items) {
            if (items->m_NextStackItem) {
                items = items->m_NextStackItem;
            }
            else {
                if (items->IsRobot()) {
                    HDelete(CMatrixRobotAI, (CMatrixRobotAI *)items, g_MatrixHeap);
                }
                else if (items->IsFlyer()) {
                    HDelete(CMatrixFlyer, (CMatrixFlyer *)items, g_MatrixHeap);
                }
                else if (items->IsCannon()) {
                    items->Damage(WEAPON_INSTANT_DEATH, D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 0), 0, NULL);
                }
                items = NULL;
                m_Top = NULL;
                m_Bottom = NULL;
            }

            if (items && items->m_PrevStackItem) {
                if (items->m_PrevStackItem->IsRobot()) {
                    HDelete(CMatrixRobotAI, (CMatrixRobotAI *)items->m_PrevStackItem, g_MatrixHeap);
                }
                else if (items->m_PrevStackItem->IsFlyer()) {
                    HDelete(CMatrixFlyer, (CMatrixFlyer *)items->m_PrevStackItem, g_MatrixHeap);
                }
                else if (items->m_PrevStackItem->IsCannon()) {
                    items->m_PrevStackItem->Damage(WEAPON_INSTANT_DEATH, D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 0), 0,
                                                   NULL);
                }
            }
        }
    }
}

void CBuildStack::DeleteItem(CMatrixMapStatic *item) {
    if (m_Items) {
        CMatrixMapStatic *i = m_Top;
        int cnt = 0;
        while (i) {
            cnt++;
            if (i == item) {
                DeleteItem(cnt);
                break;
            }
            i = i->m_NextStackItem;
        }
    }
}

int CBuildStack::GetRobotsCnt(void) const {
    CMatrixMapStatic *objects = m_Top;
    int robots = 0;
    while (objects) {
        if (objects->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
            robots++;
        }
        objects = objects->m_NextStackItem;
    }

    return robots;
}

void CBuildStack::ClearStack() {
    while (DeleteItem(1)) {}
    if (m_ParentBase->m_Side == PLAYER_SIDE) {
        CInterface *ifs = g_IFaceList->m_First;
        while (ifs) {
            if (ifs->m_strName == IF_MAIN) {
                CIFaceElement *els = ifs->m_FirstElement;
                while (els) {
                    if (IS_STACK_ICON(els->m_nId) && els->m_iParam == int(m_ParentBase)) {
                        els = ifs->DelElement(els);
                        continue;
                    }
                    els = els->m_NextElement;
                }
                break;
            }
            ifs = ifs->m_NextInterface;
        }
    }
}

void CBuildStack::ReturnRobotResources(CMatrixRobotAI *robot) {
    CMatrixSideUnit *s = g_MatrixMap->GetSideById(robot->GetSide());

    int titan_cost = 0, elec_cost = 0, energy_cost = 0, plasm_cost = 0;

    for (int i = 0; i < robot->m_UnitCnt; i++) {
        int *res_point = NULL;
        switch (robot->m_Unit[i].m_Type) {
            case MRT_HEAD:
                res_point = &g_Config.m_Price[HEAD1_TITAN + (robot->m_Unit[i].u1.s1.m_Kind - 1) * 4];
                break;
            case MRT_ARMOR:
                res_point = &g_Config.m_Price[ARMOR1_TITAN + (robot->m_Unit[i].u1.s1.m_Kind - 1) * 4];
                break;
            case MRT_CHASSIS:
                res_point = &g_Config.m_Price[CHASSIS1_TITAN + (robot->m_Unit[i].u1.s1.m_Kind - 1) * 4];
                break;
            case MRT_WEAPON:
                res_point = &g_Config.m_Price[WEAPON1_TITAN + (robot->m_Unit[i].u1.s1.m_Kind - 1) * 4];
                break;
            default:
                break;
        }
        if (res_point) {
            titan_cost += res_point[0];
            elec_cost += res_point[1];
            energy_cost += res_point[2];
            plasm_cost += res_point[3];
        }
    }
    s->AddResourceAmount(TITAN, titan_cost);
    s->AddResourceAmount(ELECTRONICS, elec_cost);
    s->AddResourceAmount(ENERGY, energy_cost);
    s->AddResourceAmount(PLASMA, plasm_cost);
}

void CBuildStack::ReturnTurretResources(CMatrixCannon *turret) {
    CMatrixSideUnit *s = g_MatrixMap->GetSideById(turret->GetSide());

    s->AddResourceAmount(TITAN, g_Config.m_CannonsProps[turret->m_Num - 1].m_Resources[0]);
    s->AddResourceAmount(ELECTRONICS, g_Config.m_CannonsProps[turret->m_Num - 1].m_Resources[1]);
    s->AddResourceAmount(ENERGY, g_Config.m_CannonsProps[turret->m_Num - 1].m_Resources[2]);
    s->AddResourceAmount(PLASMA, g_Config.m_CannonsProps[turret->m_Num - 1].m_Resources[3]);
}

void CBuildStack::KillBar(void) {
    m_PB.KillClone(PBC_CLONE1);
}
