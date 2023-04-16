// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixSide.hpp"
#include "MatrixMap.hpp"
#include "MatrixRobot.hpp"
#include "MatrixObjectBuilding.hpp"
#include "MatrixFormGame.hpp"
#include "MatrixMapStatic.hpp"
#include "Effects/MatrixEffectSelection.hpp"
#include "Interface/CConstructor.h"
#include "Interface/CIFaceElement.h"
#include "Logic/MatrixTactics.h"
#include "Logic/MatrixAIGroup.h"
#include "Interface/CConstructor.h"
#include "Interface/CInterface.h"
#include "Logic/MatrixTactics.h"
#include "Logic/MatrixAIGroup.h"
#include "MatrixObjectCannon.hpp"
#include "MatrixFlyer.hpp"
#include "Interface/CCounter.h"
#include "MatrixMultiSelection.hpp"

#include <algorithm>
#include <time.h>

// robot->GetEnv()->m_Place -   место куда робот идет или где стоит.
//                              <0 - роботу нужно назначить приказ
//                              >=0 - должен быть низкоуровневый приказ роботу. есть исключение: роботу не был отдан
//                              приказ так как предыдущий нельзя было прервать CanBreakOrder==false. в этом случае когда
//                              CanBreakOrder станет true выполнится кода: robot->GetEnv()->m_Place=-1

inline bool PrepareBreakOrder(CMatrixMapStatic *robot);
inline bool IsLiveUnit(CMatrixMapStatic *obj);
inline CPoint GetMapPos(CMatrixMapStatic *obj);
inline D3DXVECTOR2 GetWorldPos(CMatrixMapStatic *obj);
inline bool IsToPlace(CMatrixRobotAI *robot, int place);  // Движется ли робот к назначенному месту
inline bool IsInPlace(CMatrixRobotAI *robot, int place);  // Если робот стоит на месте
inline bool IsInPlace(CMatrixRobotAI *robot);             // Если робот стоит на месте
inline int RobotPlace(CMatrixRobotAI *robot);
inline int CannonPlace(CMatrixCannon *cannon);
inline int ObjPlace(CMatrixMapStatic *obj);
inline SMatrixPlace *ObjPlacePtr(CMatrixMapStatic *obj);
inline dword ObjPlaceData(CMatrixMapStatic *obj);
inline void ObjPlaceData(CMatrixMapStatic *obj, dword data);
inline SMatrixPlace *GetPlacePtr(int no);
inline bool CanMove(byte movetype, CMatrixRobotAI *robot);
inline int GetDesRegion(CMatrixRobotAI *robot);
inline int GetRegion(const CPoint &tp);
inline int GetRegion(int x, int y);
inline int GetRegion(CMatrixMapStatic *obj);
inline D3DXVECTOR3 PointOfAim(CMatrixMapStatic *obj);
inline int GetObjTeam(CMatrixMapStatic *robot) {
    return ((CMatrixRobotAI *)(robot))->GetTeam();
}
inline CInfo *GetEnv(CMatrixMapStatic *robot) {
    return ((CMatrixRobotAI *)(robot))->GetEnv();
}
inline int GetGroupLogic(CMatrixMapStatic *robot) {
    return ((CMatrixRobotAI *)(robot))->GetGroupLogic();
}
inline float Dist2(D3DXVECTOR2 p1, D3DXVECTOR2 p2) {
    return POW2(p1.x - p2.x) + POW2(p1.y - p2.y);
}
inline bool CanChangePlace(CMatrixRobotAI *robot) {
    return (g_MatrixMap->GetTime() - robot->GetEnv()->m_PlaceNotFound) > 2000;
}

inline bool PLIsToPlace(CMatrixRobotAI *robot);
inline CPoint PLPlacePos(CMatrixRobotAI *robot);

D3DXVECTOR3 CMatrixSideUnit::CorrectArcadedRobotArmorP(D3DXVECTOR3 &p, CMatrixRobot *r) {
    SObjectCore *core = r->GetCore(DEBUG_CALL_INFO);
    // D3DXVECTOR3 p1(r->m_PosX, r->m_PosY, core->m_Matrix._43);

    // static int  micnt = 0;
    // static float mid = 0;
    // static float prevdelta;

    D3DXVECTOR3 pp, ppos(r->m_PosX, r->m_PosY, core->m_Matrix._43);
    // D3DXVec3TransformNormal(&m_ArcadedP_ppos1, &ppos, &core->m_IMatrix);
    m_ArcadedP_ppos1 = ppos;

    float speed = r->m_Speed / ((CMatrixRobotAI *)r)->GetMaxSpeed();

    static D3DXVECTOR3 pf;

    if (m_ArcadedP_available && speed > 0) {
        // D3DXVec3TransformNormal(&ppos, &m_ArcadedP_ppos0, &core->m_Matrix);
        ppos = m_ArcadedP_ppos0;

        // CHelper::Create(1,0)->Line(ppos + D3DXVECTOR3(0,0,50), ppos + D3DXVECTOR3(0,0,100));

        D3DXVec3TransformCoord(&pp, &p, &core->m_Matrix);

        m_ArcadedP_prevrp = pp - ppos;

        pp = m_ArcadedP_cur + ppos;
        D3DXVec3TransformCoord(&pp, &pp, &core->m_IMatrix);

        core->Release();
        D3DXVECTOR3 rp = LERPVECTOR(m_ArcadedP_k, p, pp);

        if (pf.x != r->m_Forward.x) {
            m_ArcadedP_cur = m_ArcadedP_prevrp;
            m_ArcadedP_ppos0 = m_ArcadedP_ppos1;
            m_ArcadedP_k = 0;

            rp = p;
        }

        pf = r->m_Forward;

        return rp;
    }
    else {
        m_ArcadedP_available = 1;

        D3DXVec3TransformCoord(&pp, &p, &core->m_Matrix);

        m_ArcadedP_prevrp = pp - ppos;
        m_ArcadedP_cur = m_ArcadedP_prevrp;
        m_ArcadedP_ppos0 = m_ArcadedP_ppos1;

        m_ArcadedP_k = 0;

        pf = r->m_Forward;

        core->Release();
        return p;
    }
}

void CMatrixSideUnit::InterpolateArcadedRobotArmorP(int ms) {
    // return;
    if (m_ArcadedP_available == 0)
        return;

    D3DXVECTOR3 dir(m_ArcadedP_prevrp - m_ArcadedP_cur);

    float mul1 = (float)(1.0 - pow(0.996, double(ms)));
    float mul2 = (float)(1.0 - pow(0.994, double(ms)));
    // float mul = 1;

    m_ArcadedP_cur += dir * mul1;

    dir = (m_ArcadedP_ppos1 - m_ArcadedP_ppos0);
    m_ArcadedP_ppos0 += dir * mul2;

    m_ArcadedP_k += float(ms) * INVERT(2000);
    if (m_ArcadedP_k > 1.0f)
        m_ArcadedP_k = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CMatrixSideUnit::CMatrixSideUnit()
  : CMain(), m_Name{}, m_PlayerName(L"Player1"), m_SideStatus(SS_NONE) {
    DTRACE();

    m_TimeNextBomb = 60000;
    m_TimeLastBomb = 0;
    m_BraveMul = 0.5f;
    m_StrengthMul = 1.0f;
    m_DangerMul = 1.0f;
    m_WaitResMul = 1.0f;

    m_ArcadedP_available = 0;

    m_Id = 0;
    // m_Color=0;
    m_ColorTexture = NULL;

    m_Strength = 0.0f;
    m_WarSide = -1;
    // m_WarSideStrangeCancel=0.0f;
    m_NextWarSideCalcTime = 0;

    m_Constructor = HNew(g_MatrixHeap) CConstructor;
    m_RobotsCnt = 0;
    m_BuildRobotLast = -1;
    m_BuildRobotLast2 = -1;
    m_BuildRobotLast3 = -1;
    m_CurrSel = NOTHING_SELECTED;
    m_nCurrRobotPos = -1;
    m_ActiveObject = NULL;

    // m_GroupsList                = HNew(g_MatrixHeap) CMatrixGroupList;
    //    m_TacticsPar                = NULL;

    ZeroMemory(m_Robots, sizeof(m_Robots));

    m_TeamCnt = 3;

    for (int i = 0; i < MAX_TEAM_CNT; i++) {
        ZeroMemory(m_Team + i, sizeof(SMatrixTeam));
        m_Team[i].m_TargetRegion = -1;
        m_Team[i].m_RoadPath = HNew(g_MatrixHeap) CMatrixRoadRoute(&(g_MatrixMap->m_RN));
    }

    m_Region = NULL;
    m_RegionIndex = NULL;
    m_PlaceList = NULL;
    m_LastTaktHL = 0;
    m_LastTaktTL = 0;
    m_LastTaktUnderfire = 0;
    m_LastTeamChange = 0;
    ZeroMemory(m_LogicGroup, sizeof(SMatrixLogicGroup) * MAX_LOGIC_GROUP);
    ZeroMemory(m_PlayerGroup, sizeof(SMatrixPlayerGroup) * MAX_LOGIC_GROUP);

    for (int i = 0; i < MAX_LOGIC_GROUP; i++) {
        m_PlayerGroup[i].m_RoadPath = HNew(g_MatrixHeap) CMatrixRoadRoute(&(g_MatrixMap->m_RN));
    }

    m_Resources[TITAN] = 300;
    m_Resources[ELECTRONICS] = 300;
    m_Resources[ENERGY] = 300;
    m_Resources[PLASMA] = 300;

    m_CurrentAction = NOTHING_SPECIAL;
    m_Arcaded = NULL;
    m_ConstructPanel = NULL;

    m_CurSelNum = 0;

    m_FirstGroup = NULL;
    m_LastGroup = NULL;
    m_CurrentGroup = NULL;  //    SetCurGroup(NULL);

    m_CurSelGroup = NULL;

    m_WaitResForBuildRobot = -1;
}

CMatrixSideUnit::~CMatrixSideUnit() {
    DTRACE();

    if (m_Region != NULL) {
        HFree(m_Region, g_MatrixHeap);
        m_Region = NULL;
    }
    if (m_RegionIndex != NULL) {
        HFree(m_RegionIndex, g_MatrixHeap);
        m_RegionIndex = NULL;
    }
    if (m_PlaceList != NULL) {
        HFree(m_PlaceList, g_MatrixHeap);
        m_PlaceList = NULL;
    }

    if (m_Constructor != NULL)
        HDelete(CConstructor, m_Constructor, g_MatrixHeap);

    if (m_ColorTexture)
        CCache::Destroy(m_ColorTexture);

    // if(m_GroupsList){
    //    HDelete(CMatrixGroupList, m_GroupsList, g_MatrixHeap);
    //    m_GroupsList = NULL;
    //}

    if (m_CurSelGroup) {
        HDelete(CMatrixGroup, m_CurSelGroup, g_MatrixHeap);
        m_CurSelGroup = NULL;
    }

    /*m_CannonForBuild.Delete();*/
    if (m_CannonForBuild.m_Cannon) {
        HDelete(CMatrixCannon, m_CannonForBuild.m_Cannon, g_MatrixHeap);
        m_CannonForBuild.m_Cannon = NULL;
    }

    if (m_ConstructPanel)
        HDelete(CConstructorPanel, m_ConstructPanel, g_MatrixHeap);

    for (int i = 0; i < MAX_TEAM_CNT; i++) {
        if (m_Team[i].m_RoadPath) {
            HDelete(CMatrixRoadRoute, m_Team[i].m_RoadPath, g_MatrixHeap);
            m_Team[i].m_RoadPath = NULL;
        }
    }
    for (int i = 0; i < MAX_LOGIC_GROUP; i++) {
        if (m_PlayerGroup[i].m_RoadPath) {
            HDelete(CMatrixRoadRoute, m_PlayerGroup[i].m_RoadPath, g_MatrixHeap);
            m_PlayerGroup[i].m_RoadPath = NULL;
        }
    }

    CMatrixGroup *grps = m_FirstGroup;

    while (grps) {
        if (grps->m_NextGroup) {
            grps = grps->m_NextGroup;
        }
        else {
            HDelete(CMatrixGroup, grps, g_MatrixHeap);
            grps = NULL;
            m_FirstGroup = NULL;
            m_LastGroup = NULL;
        }

        if (grps)
            HDelete(CMatrixGroup, grps->m_PrevGroup, g_MatrixHeap);
    }
    m_CurrentGroup = NULL;  // SetCurGroup(NULL);

    //    if(m_CurGroup){
    //        HDelete(CMatrixGroup, m_CurGroup, g_MatrixHeap);
    //        m_CurGroup = NULL;
    //    }
}

void CMatrixSideUnit::GetResourceIncome(int &base_i, int &fa_i, ERes resource_type) {
    DTRACE();
    int base_cnt = 0;
    int fa_cnt = 0;

    EBuildingType type;
    ETimings tim;
    if (resource_type == TITAN) {
        type = BUILDING_TITAN;
        tim = RESOURCE_TITAN;
    }
    else if (resource_type == ELECTRONICS) {
        type = BUILDING_ELECTRONIC;
        tim = RESOURCE_ELECTRONICS;
    }
    else if (resource_type == PLASMA) {
        type = BUILDING_PLASMA;
        tim = RESOURCE_PLASMA;
    }
    else if (resource_type == ENERGY) {
        type = BUILDING_ENERGY;
        tim = RESOURCE_ENERGY;
    }

    CMatrixMapStatic *ms = CMatrixMapStatic::GetFirstLogic();
    for (; ms; ms = ms->GetNextLogic()) {
        if (ms->IsLiveBuilding() && ms->GetSide() == m_Id) {
            if (ms->IsBase())
                ++base_cnt;
            else if (ms->AsBuilding()->m_Kind == type) {
                ++fa_cnt;
            }
        }
    }
    int fu = GetResourceForceUp();

    fa_i = (fa_cnt * RESOURCES_INCOME) /* * 60000 / g_Config.m_Timings[tim]*/;
    base_i = (base_cnt * RESOURCES_INCOME_BASE * fu / 100) /* * 15000 / g_Config.m_Timings[RESOURCE_BASE]*/;
}

int CMatrixSideUnit::GetIncomePerTime(int building, int ms) {
    DTRACE();

    ETimings tim;
    if (building == int(BUILDING_TITAN)) {
        tim = RESOURCE_TITAN;
    }
    else if (building == int(BUILDING_ELECTRONIC)) {
        tim = RESOURCE_ELECTRONICS;
    }
    else if (building == int(BUILDING_PLASMA)) {
        tim = RESOURCE_PLASMA;
    }
    else if (building == int(BUILDING_ENERGY)) {
        tim = RESOURCE_ENERGY;
    }

    int fu = GetResourceForceUp();

    if (building != int(BUILDING_BASE))
        return (RESOURCES_INCOME) /* * ms / g_Config.m_Timings[tim]*/;

    return (RESOURCES_INCOME_BASE * fu / 100) /* * (ms/4) / g_Config.m_Timings[RESOURCE_BASE]*/;
}

void CMatrixSideUnit::BufPrepare() {
    DTRACE();
    if (!m_PlaceList)
        m_PlaceList = (int *)HAllocClear(sizeof(int) * g_MatrixMap->m_RN.m_PlaceCnt, g_MatrixHeap);
}

void CMatrixSideUnit::OrderFlyer(const D3DXVECTOR2 &to, EFlyerOrder order, float ang, int place, const CPoint &bpos,
                                 int botpar_i) {
    DTRACE();
    if (g_MatrixMap->m_AD_Obj_cnt >= MAX_ALWAYS_DRAW_OBJ)
        return;

    CMatrixFlyer *fl = g_MatrixMap->StaticAdd<CMatrixFlyer>(true);

    g_MatrixMap->m_AD_Obj[g_MatrixMap->m_AD_Obj_cnt] = fl;
    ++g_MatrixMap->m_AD_Obj_cnt;

    fl->ApplyOrder(to, m_Id, order, ang, place, bpos, botpar_i);
}

void CMatrixSideUnit::BuildCrazyBot(void) {
    DTRACE();
    SSpecialBot bot;
    ZeroMemory(&bot, sizeof(SSpecialBot));

    bot.m_Chassis.m_nKind = RUK_CHASSIS_ANTIGRAVITY;
    bot.m_Armor.m_Unit.m_nKind = RUK_ARMOR_NUCLEAR;
    bot.m_Head.m_nKind = RUK_HEAD_FIREWALL;

    bot.m_Weapon[0].m_Unit.m_nKind = RUK_WEAPON_CANNON;
    bot.m_Weapon[1].m_Unit.m_nKind = RUK_WEAPON_MISSILE;
    bot.m_Weapon[2].m_Unit.m_nKind = RUK_WEAPON_PLASMA;
    bot.m_Weapon[3].m_Unit.m_nKind = RUK_WEAPON_LASER;
    bot.m_Weapon[4].m_Unit.m_nKind = RUK_WEAPON_MORTAR;

    bot.m_Team = -1;

    m_Constructor->BuildSpecialBot(bot);
}

void CMatrixSideUnit::LogicTakt(int ms) {
    DTRACE();

    if (GetStatus() != SS_NONE && FLAG(g_Config.m_DIFlags, DI_SIDEINFO)) {
        g_MatrixMap->m_DI.T(utils::format(L"Side %d", m_Id).c_str(),
                            utils::format(L"Titan %d, Electronics %d, Plasma %d, Energy %d",
                                          m_Resources[TITAN],
                                          m_Resources[ELECTRONICS],
                                          m_Resources[PLASMA],
                                          m_Resources[ENERGY]).c_str());
    }

    if (GetStatus() != SS_NONE) {
        DWORD ctime = timeGetTime();
        DWORD v = (g_MatrixMap->GetStartTime() <= ctime) ? (ctime - g_MatrixMap->GetStartTime())
                                                         : (0xFFFFFFFF - g_MatrixMap->GetStartTime() + ctime);
        SetStatValue(STAT_TIME, v);
    }

    DCP();
    if (g_MatrixMap->GetPlayerSide() != this || FLAG(g_MatrixMap->m_Flags, MMFLAG_AUTOMATIC_MODE)) {
        if (m_Id == PLAYER_SIDE) {
            if (!g_MatrixMap->MaintenanceDisabled()) {
                if (g_MatrixMap->BeforeMaintenanceTime() == 0 && (FRND(1) < 0.05f)) {
                    CMatrixMapStatic *b = NULL;
                    CMatrixMapStatic *ms = CMatrixMapStatic::GetFirstLogic();
                    for (; ms; ms = ms->GetNextLogic()) {
                        if (ms->IsLiveBuilding() && ms->GetSide() == m_Id) {
                            b = ms;
                            if (FRND(1) < 0.05f)
                                break;
                        }
                    }
                    if (b)
                        b->AsBuilding()->Maintenance();
                }
            }
        }

        DCP();
        TaktHL();
        DCP();
        //        dword t2=timeGetTime();
        TaktTL();
        DCP();
        //        dword t3=timeGetTime();
        //        DM(L"TaktTL",std::wstring().Format(L"<i>",t3-t2).Get());
    }
    else {
        DCP();
        if (CMultiSelection::m_GameSelection) {
            SCallback cbs;
            cbs.mp = g_MatrixMap->m_Cursor.GetPos();
            cbs.calls = 0;

            CMultiSelection::m_GameSelection->Update(g_MatrixMap->m_Cursor.GetPos(),
                                                     TRACE_ROBOT /*|TRACE_FLYER*/ | TRACE_BUILDING,
                                                     SideSelectionCallBack, (DWORD)&cbs);
        }
        DCP();
        PumpGroups();
        DCP();
        if ((!GetCurGroup() || !GetCurGroup()->GetObjectsCnt()) &&
            (m_CurrSel == GROUP_SELECTED || m_CurrSel == ROBOT_SELECTED || m_CurrSel == FLYER_SELECTED)) {
            Select(NOTHING, NULL);
        }
        DCP();
        //        dword t1=timeGetTime();
        TaktPL();
        //        dword t2=timeGetTime();
        //        DM(L"TaktPL",std::wstring().Format(L"<i>",t2-t1).Get());
    }
    DCP();
    CalcMaxSpeed();
    DCP();

    CMatrixMapStatic *object = CMatrixMapStatic::GetFirstLogic();
    m_RobotsCnt = 0;
    for (int i = 0; i < m_TeamCnt; i++) {
        m_Team[i].m_RobotCnt = 0;
    }
    // m_EnergyCnt = 0;
    // m_TitanCnt = 0;
    // m_PlasmaCnt = 0;
    // m_ElectronicCnt = 0;

    DCP();
    if (this == g_MatrixMap->GetPlayerSide()) {
        if (m_CurrSel == BUILDING_SELECTED && m_ActiveObject) {
            if (((CMatrixBuilding *)m_ActiveObject)->m_Side != m_Id) {
                // Select(FLYER, NULL);
                m_CurrentAction = NOTHING_SPECIAL;
                m_CannonForBuild.m_CanBuildFlag = 0;
                m_CannonForBuild.m_Cannon = NULL;
                m_CannonForBuild.m_ParentBuilding = NULL;
            }
        }
        switch (m_CurrentAction) {
            case NOTHING_SPECIAL:
                g_MatrixMap->m_Cursor.SetVisible(true);
                break;
            case BUILDING_TURRET:
                if (m_CannonForBuild.m_Cannon) {
                    bool build_flag = (m_CannonForBuild.m_ParentBuilding->m_TurretsHave <
                                       m_CannonForBuild.m_ParentBuilding->m_TurretsMax);

                    if (g_IFaceList->m_InFocus == UNKNOWN) {
                        /*g_MatrixMap->m_Cursor.Select(CURSOR_EMPTY);*/
                        g_MatrixMap->m_Cursor.SetVisible(false);
                    }
                    else if (g_IFaceList->m_InFocus == INTERFACE) {
                        g_MatrixMap->m_Cursor.SetVisible(true);
                        g_MatrixMap->m_Cursor.Select(CURSOR_ARROW);
                    }

                    CPoint places[MAX_PLACES];
                    int places_cnt = m_CannonForBuild.m_ParentBuilding->GetPlacesForTurrets(places);
                    // places->x, places+1->y

                    D3DXVECTOR3 pos = g_MatrixMap->m_TraceStopPos;
                    g_MatrixMap->Trace(&pos, g_MatrixMap->m_Camera.GetFrustumCenter(),
                                       g_MatrixMap->m_Camera.GetFrustumCenter() + (g_MatrixMap->m_MouseDir * 10000.0f),
                                       TRACE_LANDSCAPE | TRACE_WATER);

                    int can_bld_flg = IsInPlaces(places, places_cnt, Float2Int(pos.x * INVERT(GLOBAL_SCALE_MOVE)),
                                                 Float2Int(pos.y * INVERT(GLOBAL_SCALE_MOVE)));

                    if (build_flag && can_bld_flg >= 0) {
                        m_CannonForBuild.m_Cannon->SetTerainColor(0xFF00FF00);
                        m_CannonForBuild.m_CanBuildFlag = 1;
                    }
                    else {
                        m_CannonForBuild.m_Cannon->SetTerainColor(0xFFFF0000);
                        m_CannonForBuild.m_CanBuildFlag = 0;
                    }
                    if (can_bld_flg >= 0) {
                        m_CannonForBuild.m_Cannon->m_Pos.x = places[can_bld_flg].x * GLOBAL_SCALE_MOVE;
                        m_CannonForBuild.m_Cannon->m_Pos.y = places[can_bld_flg].y * GLOBAL_SCALE_MOVE;
                        m_CannonForBuild.m_Cannon->m_Place = g_MatrixMap->m_RN.FindInPL(places[can_bld_flg]);
                    }
                    else {
                        m_CannonForBuild.m_Cannon->m_Pos.x = pos.x;
                        m_CannonForBuild.m_Cannon->m_Pos.y = pos.y;
                        m_CannonForBuild.m_Cannon->m_Place = -1;
                    }

                    if (can_bld_flg >= 0) {
                        for (int i = 0; i < m_CannonForBuild.m_ParentBuilding->m_TurretsMax; i++) {
                            if (m_CannonForBuild.m_ParentBuilding->m_TurretsPlaces[i].m_Coord == places[can_bld_flg]) {
                                float dang = (float)AngleDist(
                                        double(m_CannonForBuild.m_Cannon->m_Angle),
                                        float(m_CannonForBuild.m_ParentBuilding->m_TurretsPlaces[i].m_Angle));

                                m_CannonForBuild.m_Cannon->m_Angle += float(dang * (1.0 - pow(0.99, double(ms))));
                                m_CannonForBuild.m_Cannon->SetMustBeAngle(
                                        m_CannonForBuild.m_ParentBuilding->m_TurretsPlaces[i].m_Angle);
                                break;
                            }
                        }
                    }
                    m_CannonForBuild.m_Cannon->RChange(MR_Matrix);
                }
                break;
        }
    }
    DCP();
    while (object) {
        if (object->GetObjectType() == OBJECT_TYPE_ROBOTAI && ((CMatrixRobotAI *)object)->m_Side == m_Id &&
            ((CMatrixRobotAI *)object)->m_CurrState != ROBOT_DIP) {
            m_RobotsCnt++;
            if (((CMatrixRobotAI *)object)->GetTeam() >= 0) {
                ASSERT(((CMatrixRobotAI *)object)->GetTeam() >= 0 && ((CMatrixRobotAI *)object)->GetTeam() < m_TeamCnt);
                m_Team[((CMatrixRobotAI *)object)->GetTeam()].m_RobotCnt++;
            }
        }
        object = object->GetNextLogic();
    }

    if (IsRobotMode()) {
        m_Arcaded->AsRobot()->ShowHitpoint();
    }
    DCP();
}

void CMatrixSideUnit::OnMouseMove() {
    DTRACE();
    if (m_Id == PLAYER_SIDE && IsRobotMode() && g_IFaceList->m_InFocus != INTERFACE) {
        CMatrixRobotAI *robot = ((CMatrixRobotAI *)GetArcadedObject());
    }
}

void CMatrixSideUnit::OnLButtonDown(const CPoint &) {
    DTRACE();
    if (IsArcadeMode())
        return;

    CMatrixMapStatic *pObject = MouseToLand();

    if (pObject == TRACE_STOP_NONE)
        return;

    if(m_CurrentAction == BUILDING_TURRET && m_CannonForBuild.m_Cannon && m_CannonForBuild.m_CanBuildFlag/* && (m_CannonForBuild.m_ParentBuilding->m_TurretsHave < m_CannonForBuild.m_ParentBuilding->m_TurretsMax)*/){
        if (g_MatrixMap->IsPaused())
            return;
        CMatrixCannon *ca = g_MatrixMap->StaticAdd<CMatrixCannon>(true);
        ca->m_CurrState = CANNON_UNDER_CONSTRUCTION;

        ca->SetInvulnerability();
        ca->m_Pos.x = m_CannonForBuild.m_Cannon->m_Pos.x;  // g_MatrixMap->m_TraceStopPos.x;
        ca->m_Pos.y = m_CannonForBuild.m_Cannon->m_Pos.y;  // g_MatrixMap->m_TraceStopPos.y;
        ca->m_Place = m_CannonForBuild.m_Cannon->m_Place;
        ca->SetSide(m_Id);
        ca->UnitInit(m_CannonForBuild.m_Cannon->m_Num);

        ca->m_Angle = m_CannonForBuild.m_Cannon->GetMustBeAngle();
        ca->m_AddH = 0;

        ca->m_ShadowType = SHADOW_STENCIL;
        ca->m_ShadowSize = 128;

        ca->RNeed(MR_Matrix | MR_Graph);
        ca->m_ParentBuilding = (CMatrixBuilding *)m_ActiveObject;
        ca->JoinToGroup();

        m_CannonForBuild.m_ParentBuilding->m_TurretsHave++;
        ca->SetHitPoint(0);
        ((CMatrixBuilding *)m_ActiveObject)->m_BS.AddItem(ca);

        AddResourceAmount(TITAN, -g_Config.m_CannonsProps[m_CannonForBuild.m_Cannon->m_Num - 1].m_Resources[TITAN]);
        AddResourceAmount(ELECTRONICS,
                          -g_Config.m_CannonsProps[m_CannonForBuild.m_Cannon->m_Num - 1].m_Resources[ELECTRONICS]);
        AddResourceAmount(ENERGY, -g_Config.m_CannonsProps[m_CannonForBuild.m_Cannon->m_Num - 1].m_Resources[ENERGY]);
        AddResourceAmount(PLASMA, -g_Config.m_CannonsProps[m_CannonForBuild.m_Cannon->m_Num - 1].m_Resources[PLASMA]);

        m_CurrentAction = NOTHING_SPECIAL;
        m_CannonForBuild.Delete();
        CSound::Play(S_TURRET_BUILD_START, SL_ALL);
        g_IFaceList->ResetBuildCaMode();
        return;
    }

    if (m_CurrentAction == BUILDING_TURRET && m_CannonForBuild.m_Cannon) {
        return;
    }
    int mx = Float2Int(g_MatrixMap->m_TraceStopPos.x / GLOBAL_SCALE_MOVE);
    int my = Float2Int(g_MatrixMap->m_TraceStopPos.y / GLOBAL_SCALE_MOVE);
    D3DXVECTOR3 tpos = g_MatrixMap->m_TraceStopPos;

    if (g_IFaceList->m_InFocus == INTERFACE && g_IFaceList->m_FocusedInterface->m_strName == IF_MINI_MAP) {
        D3DXVECTOR2 tgt;
        if (g_MatrixMap->m_Minimap.CalcMinimap2World(tgt)) {
            pObject = TRACE_STOP_LANDSCAPE;
            mx = Float2Int(tgt.x / GLOBAL_SCALE_MOVE);
            my = Float2Int(tgt.y / GLOBAL_SCALE_MOVE);
            tpos = D3DXVECTOR3(tgt.x, tgt.y, tpos.z);
        }
    }

    if (IS_PREORDERING) {
        if (FLAG(g_IFaceList->m_IfListFlags, PREORDER_MOVE)) {
            // Move
            RESETFLAG(g_IFaceList->m_IfListFlags, PREORDER_MOVE | ORDERING_MODE);

            PGOrderMoveTo(SelGroupToLogicGroup(),
                          CPoint(mx - ROBOT_MOVECELLS_PER_SIZE / 2, my - ROBOT_MOVECELLS_PER_SIZE / 2));

            CMatrixGroup *group = GetCurGroup();
            CMatrixGroupObject *objs = group->m_FirstObject;
            while (objs) {
                if (objs->GetObject() && objs->GetObject()->GetObjectType() == OBJECT_TYPE_FLYER) {
                    int param = (group->GetObjectsCnt() - group->GetRobotsCnt()) - 1;
                    if (param > 4)
                        param = 4;
                    float x = (float)g_MatrixMap->RndFloat(g_MatrixMap->m_TraceStopPos.x - param * GLOBAL_SCALE_MOVE,
                                                           g_MatrixMap->m_TraceStopPos.x + param * GLOBAL_SCALE_MOVE);
                    float y = (float)g_MatrixMap->RndFloat(g_MatrixMap->m_TraceStopPos.y - param * GLOBAL_SCALE_MOVE,
                                                           g_MatrixMap->m_TraceStopPos.y + param * GLOBAL_SCALE_MOVE);
                    ((CMatrixFlyer *)objs->GetObject())->SetTarget(D3DXVECTOR2(x, y));
                }
                objs = objs->m_NextObject;
            }
        }
        else if (FLAG(g_IFaceList->m_IfListFlags, PREORDER_FIRE)) {
            // Fire
            if (IS_TRACE_STOP_OBJECT(pObject) && (pObject->IsLive() || pObject->IsSpecial())) {
                RESETFLAG(g_IFaceList->m_IfListFlags, PREORDER_FIRE | ORDERING_MODE);

                PGOrderAttack(SelGroupToLogicGroup(), GetMapPos(pObject), pObject);
            }
            else {
                RESETFLAG(g_IFaceList->m_IfListFlags, PREORDER_FIRE | ORDERING_MODE);

                PGOrderAttack(SelGroupToLogicGroup(),
                              CPoint(mx - ROBOT_MOVECELLS_PER_SIZE / 2, my - ROBOT_MOVECELLS_PER_SIZE / 2), NULL);
            }
        }
        else if (FLAG(g_IFaceList->m_IfListFlags, PREORDER_CAPTURE)) {
            // Capture
            if (IS_TRACE_STOP_OBJECT(pObject) && pObject->IsLiveBuilding() && pObject->GetSide() != PLAYER_SIDE) {
                RESETFLAG(g_IFaceList->m_IfListFlags, PREORDER_CAPTURE | ORDERING_MODE);

                PGOrderCapture(SelGroupToLogicGroup(), (CMatrixBuilding *)pObject);
            }
        }
        else if (FLAG(g_IFaceList->m_IfListFlags, PREORDER_PATROL)) {
            // Patrol
            RESETFLAG(g_IFaceList->m_IfListFlags, PREORDER_PATROL | ORDERING_MODE);
            PGOrderPatrol(SelGroupToLogicGroup(),
                          CPoint(mx - ROBOT_MOVECELLS_PER_SIZE / 2, my - ROBOT_MOVECELLS_PER_SIZE / 2));
        }
        else if (FLAG(g_IFaceList->m_IfListFlags, PREORDER_BOMB)) {
            // Nuclear BOMB!!! spasaisya kto mozhet!!! dab shas rvanet bombu!!!!
            if (IS_TRACE_STOP_OBJECT(pObject) && (pObject->IsLive() || pObject->IsSpecial())) {
                RESETFLAG(g_IFaceList->m_IfListFlags, PREORDER_BOMB | ORDERING_MODE);

                PGOrderBomb(SelGroupToLogicGroup(), GetMapPos(pObject), pObject);
            }
            else {
                RESETFLAG(g_IFaceList->m_IfListFlags, PREORDER_BOMB | ORDERING_MODE);

                PGOrderBomb(SelGroupToLogicGroup(),
                            CPoint(mx - ROBOT_MOVECELLS_PER_SIZE / 2, my - ROBOT_MOVECELLS_PER_SIZE / 2), NULL);
            }
        }
        else if (FLAG(g_IFaceList->m_IfListFlags, PREORDER_REPAIR)) {
            // Repair our robots please
            if (IS_TRACE_STOP_OBJECT(pObject) && pObject->IsLive() && pObject->GetSide() == PLAYER_SIDE) {
                RESETFLAG(g_IFaceList->m_IfListFlags, PREORDER_REPAIR | ORDERING_MODE);

                PGOrderRepair(SelGroupToLogicGroup(), (CMatrixBuilding *)pObject);
            }
        }
    }
}

void CMatrixSideUnit::OnLButtonDouble(const CPoint &mouse) {
    DTRACE();
    if (IsArcadeMode())
        return;

    CMatrixMapStatic *pObject = MouseToLand();

    if (pObject == TRACE_STOP_NONE ||
        !(IS_TRACE_STOP_OBJECT(pObject) && pObject->IsLiveRobot() && pObject->GetSide() == PLAYER_SIDE))
        return;

    if (IS_TRACE_STOP_OBJECT(pObject) && pObject->IsLiveRobot() && pObject->GetSide() == PLAYER_SIDE) {
        D3DXVECTOR3 o_pos = pObject->GetGeoCenter();
        CMatrixMapStatic *st = CMatrixMapStatic::GetFirstLogic();

        if (GetCurGroup()) {
            SelectedGroupUnselect();
            GetCurSelGroup()->RemoveAll();
        }

        while (st) {
            if (st->GetSide() == PLAYER_SIDE && st->IsLiveRobot()) {
                auto tmp = o_pos - st->GetGeoCenter();
                if (D3DXVec3LengthSq(&tmp) <=
                    FRIENDLY_SEARCH_RADIUS * FRIENDLY_SEARCH_RADIUS) {
                    GetCurSelGroup()->AddObject(st, -4);
                }
            }
            st = st->GetNextLogic();
        }
    }
    CreateGroupFromCurrent();
    if (GetCurGroup() && GetCurGroup()->GetObjectsCnt() == 1) {
        Select(ROBOT, NULL);
    }
    else if (GetCurGroup() && GetCurGroup()->GetObjectsCnt() > 1) {
        Select(GROUP, NULL);
    }
}

void CMatrixSideUnit::OnLButtonUp(const CPoint &) {
    DTRACE();
    if (IsArcadeMode())
        return;

    CMatrixMapStatic *pObject = MouseToLand();

    if (pObject == TRACE_STOP_NONE)
        return;

    if (IS_TRACE_STOP_OBJECT(pObject)) {}
}

void CMatrixSideUnit::OnRButtonDown(const CPoint &) {
    DTRACE();
    if (IsArcadeMode())
        return;
    DCP();
    if (IS_PREORDERING && m_CurrentAction == BUILDING_TURRET) {
        DCP();
        g_IFaceList->ResetOrderingMode();
        m_CannonForBuild.Delete();
        m_CurrentAction = NOTHING_SPECIAL;
        return;
    }
    DCP();

    CMatrixMapStatic *pObject = MouseToLand();
    DCP();

    int mx = Float2Int(g_MatrixMap->m_TraceStopPos.x / GLOBAL_SCALE_MOVE);
    int my = Float2Int(g_MatrixMap->m_TraceStopPos.y / GLOBAL_SCALE_MOVE);
    D3DXVECTOR3 tpos = g_MatrixMap->m_TraceStopPos;

    DCP();
    if (!IS_PREORDERING && GetCurGroup() && g_IFaceList->m_InFocus == INTERFACE &&
        g_IFaceList->m_FocusedInterface->m_strName == IF_MINI_MAP) {
        D3DXVECTOR2 tgt;
        if (g_MatrixMap->m_Minimap.CalcMinimap2World(tgt)) {
            pObject = TRACE_STOP_LANDSCAPE;
            mx = Float2Int(tgt.x / GLOBAL_SCALE_MOVE);
            my = Float2Int(tgt.y / GLOBAL_SCALE_MOVE);
            tpos = D3DXVECTOR3(tgt.x, tgt.y, tpos.z);
            g_MatrixMap->m_Minimap.AddEvent(tpos.x, tpos.y, 0xffff0000, 0xffff0000);
        }
    }

    if (pObject == TRACE_STOP_NONE)
        return;

    if (!IS_PREORDERING &&
        (m_CurrSel == GROUP_SELECTED || m_CurrSel == ROBOT_SELECTED || m_CurrSel == FLYER_SELECTED)) {
        if (IS_TRACE_STOP_OBJECT(pObject) && pObject->IsLiveBuilding() && pObject->GetSide() != m_Id) {
            // Capture
            PGOrderCapture(SelGroupToLogicGroup(), (CMatrixBuilding *)pObject);
        }
        else if (IS_TRACE_STOP_OBJECT(pObject) &&
                 ((IsLiveUnit(pObject) && pObject->GetSide() != m_Id) || pObject->IsSpecial())) {
            // Attack
            PGOrderAttack(SelGroupToLogicGroup(), GetMapPos(pObject), pObject);
        }
        else if (pObject == TRACE_STOP_LANDSCAPE || pObject == TRACE_STOP_WATER || (IS_TRACE_STOP_OBJECT(pObject))) {
            // MoveTo
            PGOrderMoveTo(SelGroupToLogicGroup(),
                          CPoint(mx - ROBOT_MOVECELLS_PER_SIZE / 2, my - ROBOT_MOVECELLS_PER_SIZE / 2));

            CMatrixGroupObject *objs = GetCurGroup()->m_FirstObject;
            while (objs) {
                if (objs->GetObject() && objs->GetObject()->GetObjectType() == OBJECT_TYPE_FLYER) {
                    int param = (GetCurGroup()->GetObjectsCnt() - GetCurGroup()->GetRobotsCnt()) - 1;
                    if (param > 4)
                        param = 4;
                    float x = (float)g_MatrixMap->RndFloat(g_MatrixMap->m_TraceStopPos.x - param * GLOBAL_SCALE_MOVE,
                                                           g_MatrixMap->m_TraceStopPos.x + param * GLOBAL_SCALE_MOVE);
                    float y = (float)g_MatrixMap->RndFloat(g_MatrixMap->m_TraceStopPos.y - param * GLOBAL_SCALE_MOVE,
                                                           g_MatrixMap->m_TraceStopPos.y + param * GLOBAL_SCALE_MOVE);
                    ((CMatrixFlyer *)objs->GetObject())->SetTarget(D3DXVECTOR2(x, y));
                }
                objs = objs->m_NextObject;
            }
        }
    }
}

void CMatrixSideUnit::OnRButtonUp(const CPoint &) {
    DTRACE();
    if (IsArcadeMode())
        return;
}

void CMatrixSideUnit::OnRButtonDouble(const CPoint &) {
    DTRACE();
    if (IsArcadeMode())
        return;

    //   if(m_CurrentAction == BUILDING_TURRET)
    //       return;

    //   CMatrixMapStatic* pObject = MouseToLand();
    //   if(pObject == TRACE_STOP_NONE) return;
    //
    // 	if(pObject == TRACE_STOP_LANDSCAPE){
    //       if(m_CurrentAction == CAPTURING_ROBOT || m_CurrentAction == GETING_IN_ROBOT){
    //           m_CurrentAction = NOTHING_SPECIAL;
    //       }
    //       if(m_CurrSel == BUILDING_SELECTED || m_CurrSel == BASE_SELECTED){
    //           Select(HELICOPTER, NULL);
    //           return;
    //       }
    //}else if(pObject == TRACE_STOP_WATER){
    //       if(m_CurrentAction == CAPTURING_ROBOT || m_CurrentAction == GETING_IN_ROBOT){
    //           m_CurrentAction = NOTHING_SPECIAL;
    //       }
    //	if(!(m_CurrSel == ROBOT_SELECTED)) Select(HELICOPTER, NULL);
    //       return;
    //}
}

void CMatrixSideUnit::OnForward(bool down) {
    DTRACE();
    if (!IsRobotMode() || !m_Arcaded)
        return;

    if (down && !m_Arcaded->AsRobot()->FindOrderLikeThat(ROT_MOVE_TO)) {
        D3DXVECTOR3 vel = m_Arcaded->AsRobot()->m_Forward * m_Arcaded->AsRobot()->GetMaxSpeed();
        float x = m_Arcaded->AsRobot()->m_PosX + vel.x;
        float y = m_Arcaded->AsRobot()->m_PosY + vel.y;
        m_Arcaded->AsRobot()->MoveTo(Float2Int(x / GLOBAL_SCALE_MOVE), Float2Int(y / GLOBAL_SCALE_MOVE));
    }
    else if (!down && m_Arcaded->AsRobot()->FindOrderLikeThat(ROT_MOVE_TO)) {
        m_Arcaded->AsRobot()->StopMoving();
    }
}

void CMatrixSideUnit::OnBackward(bool down) {
    DTRACE();
    if (!IsRobotMode() || !m_Arcaded)
        return;

    if (down && !m_Arcaded->AsRobot()->FindOrderLikeThat(ROT_MOVE_TO_BACK)) {
        D3DXVECTOR3 vel = m_Arcaded->AsRobot()->m_Forward * m_Arcaded->AsRobot()->GetMaxSpeed();
        float x = m_Arcaded->AsRobot()->m_PosX - vel.x;
        float y = m_Arcaded->AsRobot()->m_PosY - vel.y;
        m_Arcaded->AsRobot()->MoveToBack(Float2Int(x / GLOBAL_SCALE_MOVE), Float2Int(y / GLOBAL_SCALE_MOVE));
    }
    else if (!down && m_Arcaded->AsRobot()->FindOrderLikeThat(ROT_MOVE_TO_BACK)) {
        m_Arcaded->AsRobot()->StopMoving();
    }
}

void CMatrixSideUnit::OnLeft(bool down) {
    DTRACE();
    if (!IsRobotMode() || !m_Arcaded)
        return;
}

void CMatrixSideUnit::OnRight(bool down) {
    DTRACE();
    if (!IsRobotMode() || !m_Arcaded)
        return;
}

void CMatrixSideUnit::Select(ESelType type, CMatrixMapStatic *pObject) {
    DTRACE();

    if (m_CurrSel == BUILDING_SELECTED || m_CurrSel == BASE_SELECTED) {
        g_IFaceList->DeletePersonal();
        g_IFaceList->DeleteDynamicTurrets();
        // RESETFLAG(g_IFaceList->m_IfListFlags, TURRET_BUILD_MODE | FLYER_BUILD_MODE);
        g_IFaceList->ResetOrderingMode();

        ((CMatrixBuilding *)m_ActiveObject)->UnSelect();
    }

    /*m_CurrentAction = NOTHING_SPECIAL;*/
    m_ActiveObject = pObject;

    if ((m_CurrSel == FLYER_SELECTED || m_CurrSel == ROBOT_SELECTED || m_CurrSel == ARCADE_SELECTED) &&
        type != ARCADE) {
        RESETFLAG(g_IFaceList->m_IfListFlags, SINGLE_MODE);
        g_IFaceList->DeleteWeaponDynamicStatics();
        g_IFaceList->DeletePersonal();
        g_IFaceList->ResetOrderingMode();
    }

    if (m_CurrSel == GROUP_SELECTED) {
        g_IFaceList->DeleteProgressBars(NULL);
        g_IFaceList->DeleteGroupIcons();
        g_IFaceList->DeletePersonal();
        g_IFaceList->ResetOrderingMode();
    }

    if (type == GROUP || type == FLYER || type == ROBOT) {
        if (m_Id == PLAYER_SIDE) {
            int rnd = g_MatrixMap->Rnd(0, 6);
            if (!rnd) {
                CSound::Play(S_SELECTION_1, SL_SELECTION);
            }
            else if (rnd == 1) {
                CSound::Play(S_SELECTION_2, SL_SELECTION);
            }
            else if (rnd == 2) {
                CSound::Play(S_SELECTION_3, SL_SELECTION);
            }
            else if (rnd == 3) {
                CSound::Play(S_SELECTION_4, SL_SELECTION);
            }
            else if (rnd == 4) {
                CSound::Play(S_SELECTION_5, SL_SELECTION);
            }
            else if (rnd == 5) {
                CSound::Play(S_SELECTION_6, SL_SELECTION);
            }
            else if (rnd == 6) {
                CSound::Play(S_SELECTION_7, SL_SELECTION);
            }
        }
    }

    if (type == BUILDING && pObject) {
        m_nCurrRobotPos = -1;
        m_CurrSel = BUILDING_SELECTED;

        ((CMatrixBuilding *)pObject)->Select();
        g_IFaceList->CreateDynamicTurrets((CMatrixBuilding *)pObject);

        if (pObject->IsBase()) {
            m_CurrSel = BASE_SELECTED;
            m_Constructor->SetBase((CMatrixBuilding *)pObject);

            if (FLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_BASE_SEL_ENABLED))
                CSound::Play(S_BASE_SEL, SL_SELECTION);

            SETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_BASE_SEL_ENABLED);
        }
        else {
            CSound::Play(S_BUILDING_SEL, SL_SELECTION);
        }
    }
    else if (type == GROUP) {
        m_CurrSel = GROUP_SELECTED;

        SetCurSelNum(0);
        g_IFaceList->CreateGroupIcons();

        ShowOrderState();
    }
    else if ((type == FLYER || type == ROBOT)) {
        SETFLAG(g_IFaceList->m_IfListFlags, SINGLE_MODE);
        if (type == FLYER) {
            m_CurrSel = FLYER_SELECTED;
        }
        else {
            m_CurrSel = ROBOT_SELECTED;
        }
        SetCurSelNum(0);
        g_IFaceList->CreateWeaponDynamicStatics();

        ShowOrderState();
    }
    else if (type == ARCADE) {
    }
    else {
        m_nCurrRobotPos = -1;
        m_CurrSel = NOTHING_SELECTED;
        SelectedGroupUnselect();
    }
}

void CMatrixSideUnit::Reselect() {
    DTRACE();
    if (!GetCurGroup())
        return;

    CMatrixMapStatic *objs = CMatrixMapStatic::GetFirstLogic();
    while (objs) {
        if (objs->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
            if (!GetCurGroup()->FindObject(objs)) {
                ((CMatrixRobotAI *)objs)->UnSelect();
            }
            else {
                ((CMatrixRobotAI *)objs)->SelectByGroup();
            }
        }
        else if (objs->GetObjectType() == OBJECT_TYPE_FLYER) {
            if (!GetCurGroup()->FindObject(objs)) {
                ((CMatrixFlyer *)objs)->UnSelect();
            }
            else {
                ((CMatrixFlyer *)objs)->SelectByGroup();
            }
        }
        objs = objs->GetNextLogic();
    }
}

void CMatrixSideUnit::ShowOrderState() {
    bool auto_capture = false;
    bool auto_attack = false;
    bool auto_defence = false;

    CMatrixGroupObject *objs = GetCurGroup()->m_FirstObject;
    while (objs) {
        if (objs->GetObject() && objs->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
            CMatrixRobotAI *robot = (CMatrixRobotAI *)(objs->GetObject());
            if (robot->IsLiveRobot()) {
                if (robot->GetGroupLogic() >= 0 &&
                    g_MatrixMap->GetSideById(robot->GetSide())->m_PlayerGroup[robot->GetGroupLogic()].Order() ==
                            mpo_AutoCapture) {
                    auto_capture = true;
                }
                if (robot->GetGroupLogic() >= 0 &&
                    g_MatrixMap->GetSideById(robot->GetSide())->m_PlayerGroup[robot->GetGroupLogic()].Order() ==
                            mpo_AutoAttack) {
                    auto_attack = true;
                }
                if (robot->GetGroupLogic() >= 0 &&
                    g_MatrixMap->GetSideById(robot->GetSide())->m_PlayerGroup[robot->GetGroupLogic()].Order() ==
                            mpo_AutoDefence) {
                    auto_defence = true;
                }
            }
        }
        objs = objs->m_NextObject;
    }

    if (auto_capture)
        SETFLAG(g_IFaceList->m_IfListFlags, AUTO_CAPTURE_ON);
    else
        RESETFLAG(g_IFaceList->m_IfListFlags, AUTO_CAPTURE_ON);

    if (auto_attack)
        SETFLAG(g_IFaceList->m_IfListFlags, AUTO_FROBOT_ON);
    else
        RESETFLAG(g_IFaceList->m_IfListFlags, AUTO_FROBOT_ON);

    if (auto_defence)
        SETFLAG(g_IFaceList->m_IfListFlags, AUTO_PROTECT_ON);
    else
        RESETFLAG(g_IFaceList->m_IfListFlags, AUTO_PROTECT_ON);
}

bool CMatrixSideUnit::MouseToLand(const CPoint &, float *pWorldX, float *pWorldY, int *pMapX, int *pMapY) {
    DTRACE();
    if (g_MatrixMap->m_TraceStopObj) {
        *pMapX = int(g_MatrixMap->m_TraceStopPos.x / GLOBAL_SCALE);
        *pMapY = int(g_MatrixMap->m_TraceStopPos.y / GLOBAL_SCALE);
        *pWorldX = (float)*pMapX * GLOBAL_SCALE;
        *pWorldY = (float)*pMapY * GLOBAL_SCALE;
        return TRUE;
    }
    return FALSE;
}

CMatrixMapStatic *CMatrixSideUnit::MouseToLand() {
    return g_MatrixMap->m_TraceStopObj;
}

void CMatrixSideUnit::RobotStop(void *) {
    DTRACE();
    if (IsRobotMode()) {
        m_Arcaded->AsRobot()->BreakAllOrders();
    }
}

void __stdcall CMatrixSideUnit::PlayerAction(void *object) {
    DTRACE();
    CIFaceElement *element = (CIFaceElement *)object;

    if (element->m_strName == IF_BUILD_RO) {
        g_IFaceList->m_RCountControl->Reset();
        g_IFaceList->m_RCountControl->CheckUp();
        m_ConstructPanel->ActivateAndSelect();
    }
}

// void CMatrixSideUnit::GiveRandomOrder()
//{
//    //return;
//    //Attack
//    CMatrixGroup* groups = m_GroupsList->m_FirstGroup;
//    while(groups){
//
//        if(!groups->GetTactics()){
//            CMatrixGroupObject* gr_objects = groups->m_FirstObject;
//            while(gr_objects){
//                if(gr_objects->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI){
//                    if(((CMatrixRobotAI*)gr_objects->GetObject())->GetEnv()->GetEnemyCnt() > 0){
//                        groups->InstallTactics(ATTACK_TACTICS, m_TacticsPar);
//                        if(groups->GetTactics()){
//                            groups->GetTactics()->InitialiseTactics(groups, NULL, -1);
//                        }
//                    }
//                }
//                gr_objects = gr_objects->m_NextObject;
//            }
//        }
//
//        if(groups->GetTactics() != NULL && (groups->GetTactics()->GetType() == ATTACK_TACTICS ||
//        groups->GetTactics()->GetType() == JOIN_TACTICS)){
//            groups = groups->m_NextGroup;
//            continue;
//        }
//
//        if(groups->GetTactics() != NULL && groups->GetTactics()->GetType() == CAPTURE_TACTICS &&
//        groups->GetTactics()->GetTarget() && ((CMatrixBuilding*)groups->GetTactics()->GetTarget())->m_Side != m_Id){
//            groups = groups->m_NextGroup;
//            continue;
//        }
//
//
//
//        if(groups->GetGroupId() == 1){
//            //Capture
//            CMatrixMapStatic*   ms = CMatrixMapStatic::GetFirstLogic();
//            CMatrixMapStatic*   nearest = NULL;
//            float               nearest_l = 0;
//
//            while(ms) {
//                if(ms->GetObjectType()==OBJECT_TYPE_BUILDING && !((CMatrixBuilding*)ms)->IsBase() && ((CMatrixBuilding
//                *)ms)->m_Side != m_Id) {
//                    if(!((CMatrixBuilding *)ms)->m_BusyFlag.IsBusy()){
//                        D3DXVECTOR2 f_pos = ((CMatrixBuilding *)ms)->m_Pos;
//                        D3DXVECTOR2 my_pos = D3DXVECTOR2(groups->GetGroupPos().x, groups->GetGroupPos().y);
//                        float a = D3DXVec2LengthSq(&(f_pos - my_pos));
//                        if(a < nearest_l || nearest == NULL){
//                            nearest_l   = a;
//                            nearest     = ms;
//                        }
//                    }
//                }
//                ms = ms->GetNextLogic();
//            }
//            if(!nearest){
//                ms = CMatrixMapStatic::GetFirstLogic();
//                nearest_l = 0;
//                nearest = NULL;
//                while(ms) {
//		            if(ms->GetObjectType()==OBJECT_TYPE_BUILDING && ((CMatrixBuilding *)ms)->m_Side != m_Id &&
//((CMatrixBuilding*)ms)->IsBase()) {
//                        if(!((CMatrixBuilding*)ms)->m_BusyFlag.IsBusy() /*||
//                        ((((CMatrixBuilding*)ms)->m_BusyFlag.IsBusy() &&
//                        ((CMatrixBuilding*)ms)->m_BusyFlag.GetBusyBy() &&
//                        ((CMatrixBuilding*)ms)->m_BusyFlag.GetBusyBy()->m_Side != m_Id)) */){
//                            D3DXVECTOR2 f_pos = ((CMatrixBuilding *)ms)->m_Pos;
//                            D3DXVECTOR2 my_pos = D3DXVECTOR2(groups->GetGroupPos().x, groups->GetGroupPos().y);
//                            float a = D3DXVec2LengthSq(&(f_pos - my_pos));
//                            if(a < nearest_l || nearest == NULL){
//                                nearest_l   = a;
//                                nearest     = ms;
//                            }
//                        }
//                    }
//                    ms = ms->GetNextLogic();
//                }
//            }
//            if(nearest){
//                groups->InstallTactics(CAPTURE_TACTICS, m_TacticsPar);
//                groups->GetTactics()->InitialiseTactics(groups, nearest, -1);
//            }
//        }else if(groups->GetGroupId() > 1){
//            //Join to the main group
//
//            CMatrixMapStatic* target = NULL;
//            CMatrixGroup* main_grp = GetGroup(1, groups->GetTeam());
//            if(main_grp && main_grp->GetTactics())
//                target = main_grp->GetTactics()->GetTarget();
//
//            if(target && ((!((CMatrixBuilding*)target)->m_BusyFlag.IsBusy()) ||
//            (((CMatrixBuilding*)target)->m_BusyFlag.IsBusy() && ((CMatrixBuilding*)target)->m_BusyFlag.GetBusyBy() &&
//            ((CMatrixBuilding*)target)->m_BusyFlag.GetBusyBy()->m_Side != m_Id))){
//                groups->InstallTactics(CAPTURE_TACTICS, m_TacticsPar);
//                groups->GetTactics()->InitialiseTactics(groups, target, -1);
//
//            }else{
//                groups->InstallTactics(JOIN_TACTICS, m_TacticsPar);
//                if(groups->GetTactics() != NULL){
//                    groups->GetTactics()->InitialiseTactics(groups, NULL, -1);
//                }
//            }
//
//        }
//        groups = groups->m_NextGroup;
//    }
//
//}

// CMatrixGroup* CMatrixSideUnit::GetGroup(int id, int t)
//{
//    DTRACE();
//    CMatrixGroup* grps = m_GroupsList->m_FirstGroup, *main_grp = NULL;
//    while(grps){
//        if(grps->GetGroupId() == id && grps->GetTeam() == t){
//            main_grp = grps;
//            break;
//        }
//        grps = grps->m_NextGroup;
//    }
//    return main_grp;
//
//}
//

void SCannonForBuild::Delete(void) {
    DTRACE();
    if (m_Cannon) {
        HDelete(CMatrixCannon, m_Cannon, g_MatrixHeap);
        m_Cannon = NULL;
        m_ParentBuilding = NULL;
#ifdef _DEBUG
        m_ParentSpot.Release(DEBUG_CALL_INFO);
#else
        m_ParentSpot.Release();
#endif
    }
}

bool CMatrixSideUnit::IsRobotMode() {
    DTRACE();
    return (m_Arcaded && m_Arcaded->GetObjectType() == OBJECT_TYPE_ROBOTAI);
}

bool CMatrixSideUnit::IsFlyerMode() {
    return (m_Arcaded && m_Arcaded->GetObjectType() == OBJECT_TYPE_FLYER);
}

void CMatrixSideUnit::SetArcadedObject(CMatrixMapStatic *o) {
    DTRACE();
    if (o == NULL) {
        m_ArcadedP_available = 0;
    }

    if (o && o->GetSide() != PLAYER_SIDE) {
        return;
    }
    if (IsRobotMode()) {
        if (m_Arcaded->AsRobot()->m_SoundChassis) {
            CSound::StopPlay(m_Arcaded->AsRobot()->m_SoundChassis);
            m_Arcaded->AsRobot()->m_SoundChassis = SOUND_ID_EMPTY;
        }
    }

    if (IsRobotMode() && !m_Arcaded->IsDIP()) {
        SETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);
        m_Arcaded->AsRobot()->MapPosCalc();
        g_MatrixMap->GetSideById(m_Arcaded->AsRobot()->GetSide())
                ->PGOrderAttack(g_MatrixMap->GetSideById(m_Arcaded->AsRobot()->GetSide())
                                        ->RobotToLogicGroup(m_Arcaded->AsRobot()),
                                CPoint(m_Arcaded->AsRobot()->GetMapPosX(), m_Arcaded->AsRobot()->GetMapPosY()), NULL);
        RESETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);
    }

    if (IsRobotMode() && (o == NULL || o == m_Arcaded)) {
        m_Arcaded->AsRobot()->SetMaxSpeed(m_Arcaded->AsRobot()->GetMaxSpeed() / SPEED_BOOST);
        o = NULL;
        m_Arcaded->AsRobot()->UnSelect();
        if (g_IFaceList)
            g_IFaceList->DeleteWeaponDynamicStatics();
        Select(NOTHING, NULL);
    }
    else if (IsRobotMode() && o->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
        if (g_IFaceList)
            g_IFaceList->DeleteWeaponDynamicStatics();
        m_Arcaded->AsRobot()->UnSelect();
        m_Arcaded->AsRobot()->SetMaxSpeed(m_Arcaded->AsRobot()->GetMaxSpeed() / SPEED_BOOST);
        m_Arcaded->AsRobot()->SetWeaponToDefaultCoeff();
    }
    else if (IsRobotMode() && o->GetObjectType() == OBJECT_TYPE_FLYER) {
        if (g_IFaceList)
            g_IFaceList->DeleteWeaponDynamicStatics();
        m_Arcaded->AsRobot()->UnSelect();
        m_Arcaded->AsRobot()->SetMaxSpeed(m_Arcaded->AsRobot()->GetMaxSpeed() / SPEED_BOOST);
    }

    m_Arcaded = o;
    if (IsRobotMode()) {
        CMatrixRobotAI *robot = (CMatrixRobotAI *)o;
        if (robot->m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_PNEUMATIC) {
            robot->m_SoundChassis =
                    CSound::Play(robot->m_SoundChassis, S_CHASSIS_PNEUMATIC_LOOP, robot->GetGeoCenter(), SL_CHASSIS);
        }
        else if (robot->m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_WHEEL) {
            robot->m_SoundChassis =
                    CSound::Play(robot->m_SoundChassis, S_CHASSIS_WHEEL_LOOP, robot->GetGeoCenter(), SL_CHASSIS);
        }
        else if (robot->m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_TRACK) {
            robot->m_SoundChassis =
                    CSound::Play(robot->m_SoundChassis, S_CHASSIS_TRACK_LOOP, robot->GetGeoCenter(), SL_CHASSIS);
        }
        else if (robot->m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_HOVERCRAFT) {
            robot->m_SoundChassis =
                    CSound::Play(robot->m_SoundChassis, S_CHASSIS_HOVERCRAFT_LOOP, robot->GetGeoCenter(), SL_CHASSIS);
        }
        else if (robot->m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_ANTIGRAVITY) {
            robot->m_SoundChassis =
                    CSound::Play(robot->m_SoundChassis, S_CHASSIS_ANTIGRAVITY_LOOP, robot->GetGeoCenter(), SL_CHASSIS);
        }

        m_Arcaded->AsRobot()->SetMaxSpeed(m_Arcaded->AsRobot()->GetMaxSpeed() * SPEED_BOOST);
        robot->SetWeaponToArcadedCoeff();
        Select(ARCADE, o);
        m_Arcaded->AsRobot()->SelectArcade();
        if (g_IFaceList)
            g_IFaceList->CreateWeaponDynamicStatics();
        m_Arcaded->AsRobot()->BreakAllOrders();
    }
}

void CMatrixSideUnit::ResetSelection() {
    DTRACE();
    if (GetCurGroup()) {
        GetCurGroup()->RemoveAll();
        SetCurGroup(NULL);
    }
}

void CMatrixSideUnit::ResetSystemSelection() {
    DTRACE();
    if (m_CurSelGroup) {
        m_CurSelGroup->RemoveAll();
    }
}

void CMatrixSideUnit::SelectedGroupUnselect() {
    DTRACE();
    if (!GetCurGroup())
        return;

    CMatrixGroupObject *objs = GetCurGroup()->m_FirstObject;
    while (objs) {
        if (objs->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
            ((CMatrixRobotAI *)objs->GetObject())->UnSelect();
        }
        else if (objs->GetObject()->GetObjectType() == OBJECT_TYPE_FLYER) {
            ((CMatrixFlyer *)objs->GetObject())->UnSelect();
        }
        objs = objs->m_NextObject;
    }
    SetCurGroup(NULL);
}

void CMatrixSideUnit::GroupsUnselectSoft() {
    DTRACE();
    CMatrixGroup *grps = m_FirstGroup;

    while (grps) {
        CMatrixGroupObject *objs = grps->m_FirstObject;
        while (objs) {
            if (objs->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
                ((CMatrixRobotAI *)objs->GetObject())->UnSelect();
            }
            else if (objs->GetObject()->GetObjectType() == OBJECT_TYPE_FLYER) {
                ((CMatrixFlyer *)objs->GetObject())->UnSelect();
            }
            objs = objs->m_NextObject;
        }

        grps = grps->m_NextGroup;
    }
}

void CMatrixSideUnit::SelectedGroupBreakOrders() {
    CMatrixGroupObject *objs = GetCurGroup()->m_FirstObject;
    while (objs) {
        if (objs->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
            ((CMatrixRobotAI *)objs->GetObject())->BreakAllOrders();
        }
        else if (objs->GetObject()->GetObjectType() == OBJECT_TYPE_FLYER) {
        }
        objs = objs->m_NextObject;
    }
}

void CMatrixSideUnit::SetCurSelNum(int i) {
    DTRACE();
    g_IFaceList->DeletePersonal();
    if (i >= 0) {
        m_CurSelNum = i;
    }
    else {
        if (GetCurGroup() && m_CurSelNum >= GetCurGroup()->GetObjectsCnt()) {
            m_CurSelNum--;
        }
    }
    g_IFaceList->CreatePersonal();
}

CMatrixMapStatic *CMatrixSideUnit::GetCurSelObject() {
    DTRACE();
    if (!GetCurGroup())
        return NULL;

    CMatrixGroupObject *go = GetCurGroup()->m_FirstObject;
    for (int i = 0; i < m_CurSelNum && go; i++) {
        go = go->m_NextObject;
    }
    if (go) {
        return go->GetObject();
    }
    return NULL;
}

CMatrixGroup *CMatrixSideUnit::CreateGroupFromCurrent() {
    DTRACE();
    CMatrixGroup *ng = HNew(g_MatrixHeap) CMatrixGroup;

    CMatrixGroupObject *go = m_CurSelGroup->m_FirstObject;
    while (go) {
        ng->AddObject(go->GetObject(), -4);
        CMatrixGroup *grps = m_FirstGroup;
        while (grps) {
            grps->RemoveObject(go->GetObject());
            // if(grps->m_Tactics){
            //    grps->m_Tactics->RemoveObjectFromT(go->GetObject());
            //}
            grps = grps->m_NextGroup;
        }
        go = go->m_NextObject;
    }
    LIST_ADD(ng, m_FirstGroup, m_LastGroup, m_PrevGroup, m_NextGroup);
    m_CurSelGroup->RemoveAll();

    SetCurGroup(ng);
    Reselect();

    return ng;
}
void CMatrixSideUnit::CreateGroupFromCurrent(CMatrixMapStatic *obj) {
    DTRACE();
    CMatrixGroup *ng = HNew(g_MatrixHeap) CMatrixGroup;

    ng->AddObject(obj, -4);
    CMatrixGroup *grps = m_FirstGroup;
    while (grps) {
        grps->RemoveObject(obj);
        // if(grps->m_Tactics){
        //    grps->m_Tactics->RemoveObjectFromT(obj);
        //}
        grps = grps->m_NextGroup;
    }

    LIST_ADD(ng, m_FirstGroup, m_LastGroup, m_PrevGroup, m_NextGroup);
    m_CurSelGroup->RemoveAll();
    SetCurGroup(ng);
    Reselect();
}

void CMatrixSideUnit::AddToCurrentGroup() {
    DTRACE();
    if (GetCurGroup()) {
        CMatrixGroupObject *go = m_CurSelGroup->m_FirstObject;
        while (go) {
            if (GetCurGroup()->FindObject(go->GetObject())) {}
            else {
                if (GetCurGroup()->GetObjectsCnt() < 9)
                    GetCurGroup()->AddObject(go->GetObject(), -4);
            }
            go = go->m_NextObject;
        }
        m_CurSelGroup->RemoveAll();
        Reselect();
    }
}
void CMatrixSideUnit::PumpGroups() {
    DTRACE();
    CMatrixGroup *grps = m_FirstGroup;

    while (grps) {
        if (grps != GetCurGroup()) {
            CMatrixGroup *nxtgrp = grps->m_NextGroup;
            LIST_DEL(grps, m_FirstGroup, m_LastGroup, m_PrevGroup, m_NextGroup);
            HDelete(CMatrixGroup, grps, g_MatrixHeap);
            grps = nxtgrp;
            continue;
            /*            if(grps->m_Tactics == NULL){
                            CMatrixGroupObject* gos = grps->m_FirstObject;
                            bool canbefree = true;

                            while(gos){
                                if(gos->GetObject() && gos->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI){
                                    if(((CMatrixRobotAI*)gos->GetObject())->GetOrdersInPool()){
                                        canbefree = false;
                                        break;
                                    }
                                }else if(gos->GetObject() && gos->GetObject()->GetObjectType() == OBJECT_TYPE_FLYER){
                                }
                                gos = gos->m_NextObject;
                            }
                            if(canbefree){
                                CMatrixGroup* nxtgrp = grps->m_NextGroup;
                                LIST_DEL(grps, m_FirstGroup, m_LastGroup, m_PrevGroup, m_NextGroup);
                                HDelete(CMatrixGroup, grps, g_MatrixHeap);
                                grps = nxtgrp;
                                continue;
                            }
                        }*/
        }
        grps = grps->m_NextGroup;
    }
}

void CMatrixSideUnit::RemoveObjectFromSelectedGroup(CMatrixMapStatic *o) {
    DTRACE();
    if (o && GetCurGroup()) {
        if (o->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
            ((CMatrixRobotAI *)o)->DeleteProgressBarClone(PBC_CLONE1);
            ((CMatrixRobotAI *)o)->DeleteProgressBarClone(PBC_CLONE2);
            ((CMatrixRobotAI *)o)->UnSelect();
        }
        else if (o->GetObjectType() == OBJECT_TYPE_FLYER) {
            ((CMatrixFlyer *)o)->DeleteProgressBarClone(PBC_CLONE1);
            ((CMatrixFlyer *)o)->DeleteProgressBarClone(PBC_CLONE2);
            ((CMatrixFlyer *)o)->UnSelect();
        }
        GetCurGroup()->RemoveObject(o);
        m_CurSelGroup->RemoveObject(o);

        if (g_IFaceList) {
            g_IFaceList->DeleteProgressBars(o);
            g_IFaceList->CreateGroupIcons();
            SetCurSelNum(-1);
        }
        Reselect();
    }
}

void CMatrixSideUnit::RemoveFromSelection(CMatrixMapStatic *o) {
    if (CMultiSelection::m_GameSelection) {
        CMultiSelection::m_GameSelection->Remove(o);
    }
}

bool CMatrixSideUnit::FindObjectInSelection(CMatrixMapStatic *o) {
    DTRACE();
    if (GetCurGroup() && GetCurGroup()->FindObject(o)) {
        return true;
    }

    if (CMultiSelection::m_GameSelection && CMultiSelection::m_GameSelection->FindItem(o)) {
        return true;
    }

    return false;
}

void CMatrixSideUnit::InitPlayerSide() {
    DTRACE();
    m_ConstructPanel = HNew(g_MatrixHeap) CConstructorPanel;

    // m_Constructor->OperateUnit(MRT_CHASSIS, RUK_CHASSIS_PNEUMATIC);
    // m_Constructor->OperateUnit(MRT_ARMOR, RUK_ARMOR_6);
    // m_Constructor->OperateUnit(MRT_WEAPON, RUK_WEAPON_MACHINEGUN);

    // for(int i = 0; i < PRESETS; i++){
    //    m_ConstructPanel->m_Configs[i].m_Chassis.m_nKind = RUK_CHASSIS_PNEUMATIC;
    //    m_ConstructPanel->m_Configs[i].m_Hull.m_Unit.m_nKind = RUK_ARMOR_6;
    //    m_ConstructPanel->m_Configs[i].m_Weapon[0].m_nKind = RUK_WEAPON_MACHINEGUN;

    //    m_ConstructPanel->m_Configs[i].m_Chassis.m_Price.SetPrice(MRT_CHASSIS, RUK_CHASSIS_PNEUMATIC);
    //    m_ConstructPanel->m_Configs[i].m_Hull.m_Unit.m_Price.SetPrice(MRT_ARMOR, RUK_ARMOR_6);
    //    m_ConstructPanel->m_Configs[i].m_Weapon[0].m_Price.SetPrice(MRT_WEAPON, RUK_WEAPON_MACHINEGUN);

    //    m_ConstructPanel->m_Configs[i].m_Weight = g_MatrixMap->Rnd(0, 200);
    //    m_ConstructPanel->m_Configs[i].m_Structure = g_MatrixMap->Rnd(0, 200);
    //    m_ConstructPanel->m_Configs[i].m_Speed = g_MatrixMap->Rnd(0, 200);
    //}

    m_CurSelNum = 0;

    m_FirstGroup = NULL;
    m_LastGroup = NULL;
    m_CurrentGroup = NULL;  // SetCurGroup(NULL);

    m_CurSelGroup = HNew(g_MatrixHeap) CMatrixGroup;
}

int CMatrixSideUnit::IsInPlaces(const CPoint *places, int placescnt, int x, int y) {
    DTRACE();
    const CPoint *p_tmp = places;
    for (int i = 0; i < placescnt; i++, p_tmp++) {
        int dx = (x - p_tmp->x);
        int dy = (y - p_tmp->y);
        int rr = dx * dx + dy * dy;
        if (rr < 4) {
            return i;
        }
    }

    return -1;
}

int CMatrixSideUnit::GetMaxSideRobots() {
    DTRACE();
    CMatrixMapStatic *os = CMatrixMapStatic::GetFirstLogic();
    int bases = 0, factories = 0;
    while (os) {
        if (os->GetSide() == m_Id && os->GetObjectType() == OBJECT_TYPE_BUILDING) {
            if (((CMatrixBuilding *)os)->IsBase())
                bases++;
            else
                factories++;
        }
        os = os->GetNextLogic();
    }

    return (bases * ROBOTS_BY_BASE) + (bases == 0 ? 0 : ROBOTS_BY_MAIN) + factories /**ROBOT_BY_FACTORY*/;
}

int CMatrixSideUnit::GetRobotsInStack() {
    DTRACE();
    CMatrixMapStatic *os = CMatrixMapStatic::GetFirstLogic();
    int robots = 0;
    while (os) {
        if (os->GetSide() == m_Id && os->GetObjectType() == OBJECT_TYPE_BUILDING && ((CMatrixBuilding *)os)->IsBase()) {
            robots += ((CMatrixBuilding *)os)->GetStackRobots();
        }
        os = os->GetNextLogic();
    }

    return robots;
}

void CMatrixSideUnit::PLDropAllActions() {
    DTRACE();
    g_IFaceList->LiveRobot();
    m_CurrentAction = NOTHING_SPECIAL;
    m_CannonForBuild.Delete();
    g_IFaceList->ResetOrderingMode();
    if (m_ActiveObject && m_ActiveObject->IsBuilding()) {
        m_ActiveObject->AsBuilding()->m_BS.KillBar();
    }
    Select(NOTHING, NULL);
    RESETFLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE);
    m_ConstructPanel->ResetGroupNClose();
    g_MatrixMap->m_Cursor.SetVisible(true);
}

void CMatrixSideUnit::SetCurGroup(CMatrixGroup *group) {
    m_CurrentGroup = group;
    if (group == NULL) {
        //         debugbreak();
        g_IFaceList->ResetOrderingMode();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SideSelectionCallBack(CMatrixMapStatic *ms, DWORD param) {
    DTRACE();
    if (!ms ||
        (ms->GetObjectType() != OBJECT_TYPE_ROBOTAI && ms->GetObjectType() != OBJECT_TYPE_FLYER &&
         ms->GetObjectType() != OBJECT_TYPE_BUILDING) ||
        ms->GetSide() != PLAYER_SIDE)
        return;

    if (ms->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
        if (((CMatrixRobotAI *)ms)->m_CurrState == ROBOT_DIP)
            return;
    }
    else if (ms->GetObjectType() == OBJECT_TYPE_BUILDING) {
        if (((CMatrixBuilding *)ms)->State() == BUILDING_DIP ||
            ((CMatrixBuilding *)ms)->State() == BUILDING_DIP_EXPLODED)
            return;
    }

    CMatrixSideUnit *my_side = g_MatrixMap->GetPlayerSide();
    CMatrixGroup *cursel = my_side->GetCurSelGroup();

    static CPoint prev_mp(0, 0);

    SCallback *cbs = (SCallback *)param;

    CPoint mp = cbs->mp;
    cbs->calls++;

    if (mp.x == -1 && mp.y == -1) {
        // end
        if (prev_mp.x != mp.x || prev_mp.y != mp.y) {
            cursel->RemoveAll();
        }
        if ((cursel->GetRobotsCnt() + cursel->GetFlyersCnt()) == 9)
            return;

        cursel->AddObject(ms, -4);
        prev_mp = mp;
    }
    else {
        // Update
        // if(ms->GetObjectType() == OBJECT_TYPE_ROBOTAI && !((CMatrixRobotAI*)ms)->IsSelected()){
        //((CMatrixRobotAI*)ms)->SelectByGroup();
        //((CMatrixRobotAI*)ms)->GetSelection()->SetColor(SEL_COLOR_TMP);
        //}else if(ms->GetObjectType() == OBJECT_TYPE_FLYER && !((CMatrixFlyer*)ms)->IsSelected()){
        //((CMatrixFlyer*)ms)->SelectByGroup();
        //((CMatrixFlyer*)ms)->GetSelection()->SetColor(SEL_COLOR_TMP);
        //}
        cursel->AddObject(ms, -4);
        // if(tmp1 && tmp2 && 0){
        //    if(prev_mp.x != mp.x || prev_mp.y != mp.y){
        //        static int calles3;
        //        calles3++;
        //        CDText::T("ccc", calles3);
        //
        //        CMatrixGroupObject* o2 = tmp2->m_FirstObject;
        //        while(o2){
        //            if(!tmp1->FindObject(o2->GetObject()) && !cursel->FindObject(o2->GetObject()) &&
        //            ((!my_side->m_CurGroup) || (my_side->m_CurGroup &&
        //            !my_side->m_CurGroup->FindObject(o2->GetObject())))){
        //                if(o2->GetObject() && o2->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI){
        //                    static int calles2;
        //                    calles2++;
        //                    CDText::T("bbb", calles2);
        //
        //                    ((CMatrixRobotAI*)o2->GetObject())->UnSelect();
        //                }else if(ms->GetObjectType() == OBJECT_TYPE_FLYER){
        //                    ((CMatrixFlyer*)o2->GetObject())->UnSelect();
        //                }
        //            }
        //            o2 = o2->m_NextObject;
        //        }
        //        tmp2->RemoveAll();
        //        CMatrixGroupObject* o1 = tmp1->m_FirstObject;
        //        while(o1){
        //            tmp2->AddObject(o1->GetObject(), -4);
        //            if(o1->GetObject() && o1->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI){
        //                if(!((CMatrixRobotAI*)o1->GetObject())->IsSelected()){
        //                    static int calles;
        //                    calles++;
        //                    CDText::T("aaa", calles);
        //                    ((CMatrixRobotAI*)o1->GetObject())->SelectByGroup();
        //                    ((CMatrixRobotAI*)o1->GetObject())->GetSelection()->SetColor(SEL_COLOR_TMP);

        //                }
        //            }else if(ms->GetObjectType() == OBJECT_TYPE_FLYER){
        //                if(!((CMatrixFlyer*)o1->GetObject())->IsSelected()){
        //                    ((CMatrixFlyer*)o1->GetObject())->SelectByGroup();
        //                    ((CMatrixFlyer*)o1->GetObject())->m_Selection->SetColor(SEL_COLOR_TMP);
        //                }
        //            }

        //            o1 = o1->m_NextObject;
        //        }
        //        tmp1->RemoveAll();
        //    }
        //    tmp1->AddObject(ms, -4);
        //}
        prev_mp = mp;
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void CMatrixSideUnit::CalcStrength() {
    DTRACE();
    int c_base = 0;
    int c_building = 0;
    float s_cannon = 0;
    float s_robot = 0;

    CMatrixMapStatic *obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->GetSide() == m_Id) {
            if (obj->IsLiveBuilding()) {
                if (obj->AsBuilding()->IsBase())
                    c_base++;
                else
                    c_building++;
            }
            else if (obj->IsLiveActiveCannon())
                s_cannon += obj->AsCannon()->GetStrength();
            else if (obj->IsLiveRobot())
                s_robot += obj->AsRobot()->GetStrength();
        }
        obj = obj->GetNextLogic();
    }

    int res = 0;
    for (int r = 0; r < MAX_RESOURCES; r++)
        res += std::min(1000, m_Resources[r]);
    res /= MAX_RESOURCES;

    m_Strength = 5.0f * c_base + 1.0f * c_building + s_cannon / 2000.0f + s_robot / 1000.0f + float(res) / 100.0f;

    m_Strength *= m_StrengthMul;
}

void CMatrixSideUnit::Regroup() {
    DTRACE();
    CMatrixMapStatic *obj;
    int i, u, t, k;
    D3DXVECTOR2 v1, v2;

    CMatrixRobotAI *rl[MAX_ROBOTS];  // Список роботов на карте
    int subl[MAX_ROBOTS];            // В какую подгруппу входит робот
    int rlcnt = 0;                   // Кол-во роботов на карте
    int subcnt = 0;                  // Кол-во подгрупп

    int tl[MAX_ROBOTS];
    int tlcnt = 0, tlsme = 0;

    for (i = 0; i < MAX_ROBOTS; i++)
        subl[i] = -1;
    for (i = 0; i < MAX_LOGIC_GROUP; i++)
        m_LogicGroup[i].RobotsCnt(0);

    // Запоминаем всех роботов
    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->GetObjectType() == OBJECT_TYPE_ROBOTAI && obj->GetSide() == m_Id) {
            rl[rlcnt] = (CMatrixRobotAI *)obj;
            if (rl[rlcnt]->GetGroupLogic() >= 0 && rl[rlcnt]->GetGroupLogic() < MAX_LOGIC_GROUP) {
                m_LogicGroup[rl[rlcnt]->GetGroupLogic()].IncRobotsCnt();
                m_LogicGroup[rl[rlcnt]->GetGroupLogic()].m_Team = rl[rlcnt]->GetTeam();
            }
            if (rl[rlcnt]->GetTeam() >= 0)
                rlcnt++;
            ASSERT(rlcnt <= MAX_ROBOTS);
        }
        obj = obj->GetNextLogic();
    }

    // Делим на подгруппы для разделения. Растояние между роботами должно быть не более 400
    for (u = 0; u < rlcnt; u++) {
        if (subl[u] >= 0)
            continue;

        tl[0] = u;
        tlcnt = 1;
        tlsme = 0;
        subcnt++;
        subl[u] = subcnt - 1;

        while (tlsme < tlcnt) {
            v1.x = rl[tl[tlsme]]->m_PosX;
            v1.y = rl[tl[tlsme]]->m_PosY;

            for (i = u + 1; i < rlcnt; i++) {
                if (subl[i] >= 0)
                    continue;
                if (tl[tlsme] == i)
                    continue;
                if (rl[tl[tlsme]]->GetTeam() != rl[i]->GetTeam())
                    continue;
                if (rl[tl[tlsme]]->GetGroupLogic() != rl[i]->GetGroupLogic())
                    continue;
                v2.x = rl[i]->m_PosX;
                v2.y = rl[i]->m_PosY;
                if (((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y)) > POW2(400.0f))
                    continue;

                tl[tlcnt] = i;
                tlcnt++;
                subl[i] = subcnt - 1;
            }
            tlsme++;
        }
    }
    // Разделяем
    for (u = 0; u < MAX_LOGIC_GROUP; u++) {
        if (m_LogicGroup[u].RobotsCnt() < 0)
            continue;
        tlcnt = 0;
        for (i = 0; i < rlcnt; i++) {
            if (rl[i]->GetGroupLogic() != u)
                continue;
            for (t = 0; t < tlcnt; t++)
                if (tl[t] == subl[i])
                    break;
            if (t >= tlcnt) {
                tl[tlcnt] = subl[i];
                tlcnt++;
            }
        }
        for (i = 1; i < tlcnt; i++) {
            // Ищем пустую группу
            for (k = 0; k < MAX_LOGIC_GROUP; k++)
                if (m_LogicGroup[k].RobotsCnt() <= 0)
                    break;
            if (k >= MAX_LOGIC_GROUP)
                break;

            // Копируем группу
            for (t = 0; t < rlcnt; t++) {
                if (rl[t]->GetGroupLogic() != u)
                    continue;
                if (subl[t] == tl[0]) {
                    m_LogicGroup[k] = m_LogicGroup[rl[t]->GetGroupLogic()];
                    m_LogicGroup[k].RobotsCnt(0);
                    break;
                }
            }
            if (t >= rlcnt)
                break;

            // Переносим роботов в новую группу
            for (t = 0; t < rlcnt; t++) {
                if (rl[t]->GetGroupLogic() != u)
                    continue;
                if (subl[t] != tl[i])
                    continue;
                m_LogicGroup[rl[t]->GetGroupLogic()].IncRobotsCnt();
                rl[t]->SetGroupLogic(k);
                m_LogicGroup[k].IncRobotsCnt();
            }
        }
        if (i < tlcnt)
            break;
    }
    // Если робот не в группе то создаем для него новую группу
    for (t = 0; t < rlcnt; t++) {
        if (rl[t]->GetGroupLogic() >= 0 && rl[t]->GetGroupLogic() < MAX_LOGIC_GROUP)
            continue;

        for (i = 0; i < MAX_LOGIC_GROUP; i++)
            if (m_LogicGroup[i].RobotsCnt() <= 0)
                break;
        if (i >= MAX_LOGIC_GROUP)
            break;

        ZeroMemory(m_LogicGroup + i, sizeof(SMatrixLogicGroup));
        m_LogicGroup[i].m_Team = rl[t]->GetTeam();
        m_LogicGroup[i].RobotsCnt(1);
        m_LogicGroup[i].m_Action.m_Type = mlat_None;
        rl[t]->SetGroupLogic(i);
    }
    // Объеденяем группы если растояние между ними меньше 300
    for (u = 0; u < MAX_LOGIC_GROUP; u++) {
        if (m_LogicGroup[u].RobotsCnt() <= 0)
            continue;
        if (m_LogicGroup[u].m_Team < 0)
            continue;

        for (t = u + 1; t < MAX_LOGIC_GROUP; t++) {
            if (m_LogicGroup[t].RobotsCnt() <= 0)
                continue;
            if (m_LogicGroup[t].m_Team < 0)
                continue;

            if (m_LogicGroup[u].m_Team != m_LogicGroup[t].m_Team)
                continue;

            // Проверяем
            for (i = 0; i < rlcnt; i++) {
                if (rl[i]->GetGroupLogic() != u)
                    continue;

                for (k = 0; k < rlcnt; k++) {
                    if (rl[k]->GetGroupLogic() != t)
                        continue;

                    float d = (rl[i]->m_PosX - rl[k]->m_PosX) * (rl[i]->m_PosX - rl[k]->m_PosX) +
                              (rl[i]->m_PosY - rl[k]->m_PosY) * (rl[i]->m_PosY - rl[k]->m_PosY);
                    if (d < POW2(300.0f))
                        break;
                }
                if (k < rlcnt)
                    break;
            }

            // Объеденяем
            if (i < rlcnt) {
                if (m_LogicGroup[u].m_Action.m_Type == mlat_None) {  // Выбираем лучшую для которой приказ останется
                    SMatrixLogicGroup temp = m_LogicGroup[u];
                    m_LogicGroup[u] = m_LogicGroup[t];
                    m_LogicGroup[t] = temp;
                }

                m_LogicGroup[u].IncRobotsCnt(m_LogicGroup[t].RobotsCnt());

                for (k = 0; k < rlcnt; k++) {
                    if (rl[k]->GetGroupLogic() != t)
                        continue;
                    rl[k]->SetGroupLogic(u);
                }
            }
        }
    }

#if (defined _DEBUG) && !(defined _RELDEBUG) && !(defined _DISABLE_AI_HELPERS)
    // Отображаем хелперы
    dword colors[10] = {0xffff0000, 0xff00ff00, 0xff0000ff, 0xffffff00, 0xffff00ff,
                        0xff00ffff, 0xff800000, 0xff008000, 0xff000080, 0xff808000};
    //    CHelper::DestroyByGroup(101);
    // if(m_Id==PLAYER_SIDE)
    for (i = 0; i < rlcnt; i++) {
        float z = g_MatrixMap->GetZ(rl[i]->m_PosX, rl[i]->m_PosY);
        /*team*/ CHelper::Create(1)->Cone(D3DXVECTOR3(rl[i]->m_PosX, rl[i]->m_PosY, z + 10.0f),
                                          D3DXVECTOR3(rl[i]->m_PosX, rl[i]->m_PosY, z + 80.0f), 1.0f, 1.0f,
                                          colors[rl[i]->GetTeam() % 10], colors[rl[i]->GetTeam() % 10], 3);
        /*group*/ CHelper::Create(1)->Cone(D3DXVECTOR3(rl[i]->m_PosX, rl[i]->m_PosY, z + 80.0f),
                                           D3DXVECTOR3(rl[i]->m_PosX, rl[i]->m_PosY, z + 100.0f), 1.0f, 1.0f,
                                           colors[rl[i]->GetGroupLogic() % 10], colors[rl[i]->GetGroupLogic() % 10], 3);
    }
#endif
}

void CMatrixSideUnit::ClearTeam(int team) {
    DTRACE();
    m_Team[team].m_TargetRegion = -1;
    m_Team[team].m_Action.m_Type = mlat_None;
    m_Team[team].m_Action.m_RegionPathCnt = 0;
    m_Team[team].SetWar(false);
    m_Team[team].m_Brave = 0;
    m_Team[team].m_BraveStrangeCancel = 0;
    m_Team[team].SetRegroupOnlyAfterWar(false);
    m_Team[team].SetWaitUnion(false);
    m_Team[team].m_WaitUnionLast = 0;
}

int CMatrixSideUnit::ClacSpawnTeam(int region, int nsh) {
    DTRACE();
    for (int i = 0; i < m_TeamCnt; i++) {
        if (m_Team[i].m_RobotCnt <= 0) {
            ClearTeam(i);
            return i;
        }
    }

    if (!m_Region)
        return 0;

    for (int ct = 0; ct <= 2; ct++) {
        int u;
        int cnt = 0;
        int sme = 0;

        for (int i = 0; i < g_MatrixMap->m_RN.m_RegionCnt; i++)
            m_Region[i].m_Data = 0;

        m_RegionIndex[cnt] = region;
        cnt++;
        m_Region[region].m_Data = 1;

        int teamfind = -1;

        while (sme < cnt) {
            for (int i = 0; i < g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt; i++) {
                u = g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_Near[i];
                if (m_Region[u].m_Data)
                    continue;

                if (g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearMove[i] & (1 << nsh))
                    continue;

                if (ct == 0 && m_Region[u].m_EnemyRobotCnt > 0)
                    continue;

                if (m_Region[u].m_OurRobotCnt > 0) {
                    CMatrixMapStatic *ms = CMatrixMapStatic::GetFirstLogic();
                    while (ms) {
                        if (ms->IsLiveRobot() && ms->GetSide() == m_Id && ms->AsRobot()->GetTeam() >= 0) {
                            if (ct == 2 || ms->AsRobot()->GetEnv()->GetEnemyCnt() <= 0) {
                                if (teamfind < 0)
                                    teamfind = ms->AsRobot()->GetTeam();
                                else {
                                    if (m_Team[ms->AsRobot()->GetTeam()].m_RobotCnt < m_Team[teamfind].m_RobotCnt) {
                                        teamfind = ms->AsRobot()->GetTeam();
                                    }
                                }
                                //                                return ms->AsRobot()->GetTeam();
                            }
                        }
                        ms = ms->GetNextLogic();
                    }
                }

                m_RegionIndex[cnt] = u;
                cnt++;
                m_Region[u].m_Data = 1;
            }
            sme++;
        }

        if (teamfind >= 0)
            return teamfind;
    }
    return 0;
}

void CMatrixSideUnit::EscapeFromBomb() {
    DTRACE();
    CMatrixRobotAI *rb[MAX_ROBOTS * 3];
    float min_dist_enemy[MAX_ROBOTS * 3];
    int rbcnt = 0;
    float escape_radius = 250.0f * 1.2f;  // С запасом
    int escape_dist = Float2Int(escape_radius / GLOBAL_SCALE_MOVE) + 4;
    CPoint tp, tp2;
    int i;
    CEnemy *enemy;

    CMatrixMapStatic *ms = CMatrixMapStatic::GetFirstLogic();
    for (; ms; ms = ms->GetNextLogic()) {
        if (!ms->IsLiveRobot())
            continue;
        if (!ms->AsRobot()->HaveBomb())
            continue;

        if (ms->AsRobot()->GetEnv()->GetEnemyCnt() <= 0)
            continue;

        ASSERT(rbcnt < MAX_ROBOTS * 3);
        min_dist_enemy[rbcnt] = 1e20f;

        enemy = ms->AsRobot()->GetEnv()->m_FirstEnemy;
        for (; enemy; enemy = enemy->m_NextEnemy) {
            auto tmp = GetWorldPos(enemy->m_Enemy) - GetWorldPos(ms);
            min_dist_enemy[rbcnt] = std::min(min_dist_enemy[rbcnt], D3DXVec2LengthSq(&tmp));
        }

        rb[rbcnt] = (CMatrixRobotAI *)ms;
        rbcnt++;
    }
    if (rbcnt <= 0)
        return;

    CMatrixRobotAI *skip_normal = NULL;
    CMatrixRobotAI *skip_withbomb = NULL;

    float skip_normal_dist = 0.0f;
    float skip_withbomb_dist = 0.0f;

    ms = CMatrixMapStatic::GetFirstLogic();
    for (; ms; ms = ms->GetNextLogic()) {
        if (!ms->IsLiveRobot())
            continue;
        if (ms->GetSide() != m_Id)
            continue;
        CMatrixRobotAI *robot = ms->AsRobot();
        if (m_Id == PLAYER_SIDE && robot->GetGroupLogic() >= 0 &&
            m_PlayerGroup[robot->GetGroupLogic()].Order() < mpo_AutoCapture)
            continue;

        float mdist = 1e20f;
        for (i = 0; i < rbcnt; i++) {
            if (robot == rb[i])
                continue;
            if (rb[i]->GetSide() == m_Id)
                continue;

            float rpx = robot->m_PosX;
            float rpy = robot->m_PosY;

            float dist = POW2(rpx - rb[i]->m_PosX) + POW2(rpy - rb[i]->m_PosY);
            if (dist < POW2(escape_radius)) {
                if (dist < mdist)
                    mdist = dist;
            }
        }
        if (mdist > 1e19f)
            continue;

        if (!robot->HaveBomb()) {
            if (!skip_normal || mdist < skip_normal_dist) {
                skip_normal = robot;
                skip_normal_dist = mdist;
            }
        }
        else {
            if (!skip_withbomb || mdist < skip_withbomb_dist) {
                skip_withbomb = robot;
                skip_withbomb_dist = mdist;
            }
        }
    }

    if (skip_withbomb)
        skip_normal = NULL;

    g_MatrixMap->PrepareBuf();
    int *ind = g_MatrixMap->m_ZoneIndex;
    dword *data = g_MatrixMap->m_ZoneDataZero;

    ms = CMatrixMapStatic::GetFirstLogic();
    for (; ms; ms = ms->GetNextLogic()) {
        if (!ms->IsLiveRobot())
            continue;
        if (ms->GetSide() != m_Id)
            continue;
        if (ms == skip_normal || ms == skip_withbomb)
            continue;
        CMatrixRobotAI *robot = ms->AsRobot();
        if (m_Id == PLAYER_SIDE && robot->GetGroupLogic() >= 0 &&
            m_PlayerGroup[robot->GetGroupLogic()].Order() < mpo_AutoCapture)
            continue;

        if (robot->GetReturnCoords(tp))
            continue;
        float rpx = robot->m_PosX;
        float rpy = robot->m_PosY;

        float mde = 1e20f;
        enemy = ms->AsRobot()->GetEnv()->m_FirstEnemy;
        for (; enemy; enemy = enemy->m_NextEnemy) {
            auto tmp = GetWorldPos(enemy->m_Enemy) - GetWorldPos(ms);
            mde = std::min(mde, D3DXVec2LengthSq(&tmp));
        }

        for (i = 0; i < rbcnt; i++) {
            if (robot == rb[i])
                continue;
            if (rb[i]->GetSide() == m_Id && mde < min_dist_enemy[i])
                continue;  // Роботы впереди бомбы не отступают. Они прикрывают пока робот с бомбой дойдет до врага.
            float dist = POW2(rpx - rb[i]->m_PosX) + POW2(rpy - rb[i]->m_PosY);
            if (dist < POW2(escape_radius))
                break;
        }
        if (i >= rbcnt)
            continue;

        int rp = g_MatrixMap->FindNearPlace(1 << (robot->m_Unit[0].u1.s1.m_Kind - 1),
                                            CPoint(robot->GetMapPosX(), robot->GetMapPosY()));
        if (rp < 0)
            continue;

        int sme = 0;
        int cnt = 0;

        CMatrixMapStatic *ms2 = CMatrixMapStatic::GetFirstLogic();
        for (; ms2; ms2 = ms2->GetNextLogic()) {
            if (robot != ms2 && ms2->IsLiveRobot()) {
                if (ms2->AsRobot()->GetMoveToCoords(tp)) {
                    int np = g_MatrixMap->FindPlace(tp);
                    if (np >= 0) {
                        ind[cnt] = np;
                        data[np] = 2;
                        cnt++;
                    }
                }

                if (ms2->AsRobot()->GetEnv()->m_Place >= 0) {
                    ind[cnt] = ms2->AsRobot()->GetEnv()->m_Place;
                    data[ind[cnt]] = 2;
                    cnt++;
                }
            }
            else if (ms2->IsCannon()) {
                ind[cnt] = ms2->AsCannon()->m_Place;
                data[ind[cnt]] = 2;
                cnt++;
            }
        }

        sme = cnt;

        ind[cnt] = rp;
        data[rp] |= 1;
        cnt++;

        while (sme < cnt) {
            rp = ind[sme];
            tp = g_MatrixMap->m_RN.m_Place[rp].m_Pos;
            if (!(data[rp] & 2)) {
                for (i = 0; i < rbcnt; i++) {
                    if (robot == rb[i])
                        continue;
                    int dist = tp.Dist2(CPoint(rb[i]->GetMapPosX(), rb[i]->GetMapPosY()));
                    if (dist < POW2(escape_dist))
                        break;
                }
                if (i >= rbcnt) {
                    if (robot->GetMoveToCoords(tp2))
                        robot->MoveReturn(tp2.x, tp2.y);
                    else
                        robot->MoveReturn(robot->GetMapPosX(), robot->GetMapPosY());
                    robot->MoveTo(tp.x, tp.y);
                    break;
                }
            }
            for (i = 0; i < g_MatrixMap->m_RN.m_Place[rp].m_NearCnt; i++) {
                int np = g_MatrixMap->m_RN.m_Place[rp].m_Near[i];
                if (data[np] & 1)
                    continue;
                if (g_MatrixMap->m_RN.m_Place[rp].m_NearMove[i] & (1 << (robot->m_Unit[0].u1.s1.m_Kind - 1)))
                    continue;

                ind[cnt] = np;
                data[np] |= 1;
                cnt++;
            }

            sme++;
        }
        for (i = 0; i < cnt; i++)
            data[ind[i]] = 0;
    }
}

void CMatrixSideUnit::GroupNoTeamRobot() {
    DTRACE();
    CMatrixRobotAI *rl[MAX_ROBOTS];
    int il[MAX_ROBOTS];
    int rlcnt = 0;
    CMatrixMapStatic *ms;
    int g, i, u, cnt, sme;
    float cx, cy;

    if (m_Id == PLAYER_SIDE)
        return;

    for (i = 0; i < MAX_LOGIC_GROUP; i++)
        m_LogicGroup[i].RobotsCnt(0);

    ms = CMatrixMapStatic::GetFirstLogic();
    while (ms) {
        if (ms->IsRobot() && ms->GetSide() == m_Id) {
            if (ms->AsRobot()->GetGroupLogic() >= 0 && ms->AsRobot()->GetGroupLogic() < MAX_LOGIC_GROUP) {
                m_LogicGroup[ms->AsRobot()->GetGroupLogic()].IncRobotsCnt();
                m_LogicGroup[ms->AsRobot()->GetGroupLogic()].m_Team = ms->AsRobot()->GetTeam();
            }
        }
        ms = ms->GetNextLogic();
    }

    ms = CMatrixMapStatic::GetFirstLogic();
    for (; ms; ms = ms->GetNextLogic()) {
        if (!ms->IsRobot())
            continue;
        if (ms->GetSide() != m_Id)
            continue;
        if (ms->AsRobot()->GetTeam() >= 0)
            continue;

        ms->AsRobot()->SetGroupLogic(-1);

        ASSERT(rlcnt < MAX_ROBOTS);
        rl[rlcnt] = (CMatrixRobotAI *)ms;
        rlcnt++;
    }

    for (i = 0; i < rlcnt; i++) {
        if (rl[i]->GetGroupLogic() >= 0)
            continue;

        for (g = 0; g < MAX_LOGIC_GROUP; g++) {
            if (m_LogicGroup[g].RobotsCnt() <= 0)
                break;
        }
        ASSERT(g < MAX_LOGIC_GROUP);

        m_LogicGroup[g].m_Team = -1;
        m_LogicGroup[g].RobotsCnt(1);
        m_LogicGroup[g].m_Action.m_Type = mlat_Defence;
        m_LogicGroup[g].m_Action.m_RegionPathCnt = 0;
        m_LogicGroup[g].SetWar(false);

        rl[i]->SetGroupLogic(g);
        cx = rl[i]->m_PosX;
        cy = rl[i]->m_PosY;

        cnt = 0;
        sme = 0;
        il[cnt] = i;
        cnt++;

        while (sme < cnt) {
            for (u = i + 1; u < rlcnt; u++) {
                if (rl[i]->GetGroupLogic() >= 0)
                    continue;
                if ((POW2(rl[il[sme]]->m_PosX - rl[u]->m_PosX) + POW2(rl[il[sme]]->m_PosY - rl[u]->m_PosY)) <
                    POW2(200)) {
                    il[cnt] = u;
                    cnt++;

                    m_LogicGroup[g].IncRobotsCnt();
                    rl[u]->SetGroupLogic(g);
                    cx += rl[u]->m_PosX;
                    cy += rl[u]->m_PosY;
                }
            }
            sme++;
        }

        cx /= m_LogicGroup[g].RobotsCnt();
        cy /= m_LogicGroup[g].RobotsCnt();

        m_LogicGroup[g].m_Action.m_Region =
                g_MatrixMap->GetRegion(Float2Int(cx / GLOBAL_SCALE_MOVE), Float2Int(cy / GLOBAL_SCALE_MOVE));
        if (m_LogicGroup[g].m_Action.m_Region < 0)
            m_LogicGroup[g].m_Action.m_Region = g_MatrixMap->GetRegion(Float2Int(rl[i]->m_PosX / GLOBAL_SCALE_MOVE),
                                                                       Float2Int(rl[i]->m_PosY / GLOBAL_SCALE_MOVE));
        if (m_LogicGroup[g].m_Action.m_Region < 0)
            ERROR_S(L"Robot stands in prohibited area");
    }
}

void CMatrixSideUnit::CalcMaxSpeed() {
    DTRACE();
    CMatrixRobotAI *rl[MAX_ROBOTS];
    float pr[MAX_ROBOTS];
    int rlcnt;
    int i, u;
    CPoint tp;
    CMatrixMapStatic *ms;
    float d;

    ms = CMatrixMapStatic::GetFirstLogic();
    for (; ms; ms = ms->GetNextLogic()) {
        if (!ms->IsLiveRobot())
            continue;
        if (ms->GetSide() != m_Id)
            continue;

        ms->AsRobot()->m_GroupSpeed = ms->AsRobot()->GetMaxSpeed();
    }

    for (i = 0; i < MAX_LOGIC_GROUP; i++) {
        if (m_Id == PLAYER_SIDE && m_PlayerGroup[i].m_RobotCnt <= 0)
            continue;
        else if (m_Id != PLAYER_SIDE && m_LogicGroup[i].RobotsCnt() <= 0)
            continue;

        float cx = 0.0f;
        float cy = 0.0f;
        float dx = 0.0f;
        float dy = 0.0f;

        rlcnt = 0;

        ms = CMatrixMapStatic::GetFirstLogic();
        for (; ms; ms = ms->GetNextLogic()) {
            if (!ms->IsLiveRobot())
                continue;
            if (ms->GetSide() != m_Id)
                continue;
            if (ms->AsRobot()->GetGroupLogic() != i)
                continue;

            rl[rlcnt] = ms->AsRobot();
            cx += rl[rlcnt]->m_PosX;
            cy += rl[rlcnt]->m_PosY;

            if (rl[rlcnt]->GetReturnCoords(tp)) {
                dx += GLOBAL_SCALE_MOVE * tp.x + GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2;
                dy += GLOBAL_SCALE_MOVE * tp.y + GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2;
            }
            else if (rl[rlcnt]->GetMoveToCoords(tp)) {
                dx += GLOBAL_SCALE_MOVE * tp.x + GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2;
                dy += GLOBAL_SCALE_MOVE * tp.y + GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2;
            }
            else {
                dx += rl[rlcnt]->m_PosX;
                dy += rl[rlcnt]->m_PosY;
            }

            rlcnt++;
        }

        if (rlcnt <= 1)
            continue;

        d = 1.0f / rlcnt;
        cx *= d;
        cy *= d;
        dx = dx * d - cx;
        dy = dy * d - cy;
        d = sqrt(dx * dx + dy * dy);
        if (d < 100.0f)
            continue;
        dx /= d;
        dy /= d;
        float minpr = 1e20f;
        float maxpr = -1e20f;

        for (u = 0; u < rlcnt; u++) {
            float rx = rl[u]->m_PosX;
            float ry = rl[u]->m_PosY;

            pr[u] = dx * (rx - cx) + dy * (ry - cy);
            minpr = std::min(pr[u], minpr);
            maxpr = std::max(pr[u], maxpr);
        }

        for (u = 0; u < rlcnt - 1; u++) {
            for (int t = u + 1; t < rlcnt; t++) {
                if (pr[t] < pr[u]) {
                    d = pr[t];
                    pr[t] = pr[u];
                    pr[u] = d;
                    CMatrixRobotAI *robot = rl[t];
                    rl[t] = rl[u];
                    rl[u] = robot;
                }
            }
        }

        for (u = 0; u < rlcnt - 1; u++) {
            if ((pr[u + 1] - pr[u]) > COLLIDE_BOT_R * 7.0f)
                break;
        }
        minpr = pr[u];

        maxpr -= minpr;

        for (u = 0; u < rlcnt; u++) {
            if (rl[u]->GetReturnCoords(tp))
                continue;
            if (!rl[u]->GetMoveToCoords(tp))
                continue;
            if (rl[u]->GetEnv()->GetEnemyCnt())
                continue;

            pr[u] -= minpr;

            if (pr[u] < COLLIDE_BOT_R * 10.0f)
                continue;  // Отстающие, скорость не уменьшают

            rl[u]->m_GroupSpeed *= 0.6f + 0.4f * ((maxpr - pr[u])) / (maxpr - COLLIDE_BOT_R * 10.0f);
        }
    }
}

void CMatrixSideUnit::TaktHL() {
    DTRACE();

    // dword it1=timeGetTime();

    // DM(std::wstring().Format(L"Res1 Side=<i>",m_Id).Get(),std::wstring().Format(L"<i> <i> <i>
    // <i>",m_Resources[0],m_Resources[1],m_Resources[2],m_Resources[3]).Get());

    /*    if(g_MatrixMap->GetPlayerSide()->GetArcadedObject()) {
            if(g_MatrixMap->GetPlayerSide()->GetArcadedObject()->GetObjectType()==OBJECT_TYPE_ROBOTAI) {
                if(((CMatrixRobotAI *)g_MatrixMap->GetPlayerSide()->GetArcadedObject())->GetOrdersInPool()>0) {
                    ((CMatrixRobotAI *)g_MatrixMap->GetPlayerSide()->GetArcadedObject())->BreakAllOrders();
                }
            }
        }*/

    if (g_MatrixMap->m_RN.m_RegionCnt <= 0)
        return;

    Regroup();

    int i, u, t, k, p, team, cnt, sme, level, next, dist;
    //    int skipregion[MAX_ROBOTS];
    //    int skipregioncnt;
    SMatrixLogicAction *ac;
    CPoint tp;
    CMatrixMapStatic *ms;
    //    SMatrixRegion * mr;
    SMatrixPlace *place;
    CMatrixRobotAI *rl[MAX_ROBOTS];  // Список роботов на карте
    int rlcnt;                       // Кол-во роботов на карте
    int ourbasecnt = 0;

    if (m_Region == NULL)
        m_Region = (SMatrixLogicRegion *)HAllocClear(sizeof(SMatrixLogicRegion) * g_MatrixMap->m_RN.m_RegionCnt,
                                                     g_MatrixHeap);
    if (m_RegionIndex == NULL)
        m_RegionIndex = (int *)HAllocClear(sizeof(int) * g_MatrixMap->m_RN.m_RegionCnt, g_MatrixHeap);

    // Запускаем высшую логику раз в 100 тактов
    if (m_LastTaktHL != 0 && (g_MatrixMap->GetTime() - m_LastTaktHL) < 100)
        return;
    m_LastTaktHL = g_MatrixMap->GetTime();

    CalcStrength();

    EscapeFromBomb();

    // Расчет с кем воюем

    if (m_NextWarSideCalcTime < g_MatrixMap->GetTime()) {
        if (m_WarSide < 0 || g_MatrixMap->Rnd(0, 2)) {
            // Ищем сначала самого слабого, чтобы побыстрее забить его и нарастить силы.
            m_WarSide = -1;
            float mst = 1e20f;
            if (m_Strength > 0)
                for (i = 0; i < g_MatrixMap->m_SideCnt; i++) {
                    if (g_MatrixMap->m_Side[i].m_Id == m_Id)
                        continue;
                    if (g_MatrixMap->m_Side[i].GetStatus() == SS_NONE)
                        continue;
                    if (g_MatrixMap->m_Side[i].m_Strength <= 0)
                        continue;
                    if ((g_MatrixMap->m_Side[i].m_Strength < mst) &&
                        (g_MatrixMap->m_Side[i].m_Strength < m_Strength * 0.5f)) {
                        mst = g_MatrixMap->m_Side[i].m_Strength;
                        m_WarSide = g_MatrixMap->m_Side[i].m_Id;
                    }
                }

            // Если слабого нет то бем самого сильного, чтобы он не стал еще сильней.
            if (m_WarSide < 0) {
                mst = -1e20f;
                for (i = 0; i < g_MatrixMap->m_SideCnt; i++) {
                    if (g_MatrixMap->m_Side[i].m_Id == m_Id)
                        continue;
                    if (g_MatrixMap->m_Side[i].GetStatus() == SS_NONE)
                        continue;
                    if (g_MatrixMap->m_Side[i].m_Strength > mst) {
                        mst = g_MatrixMap->m_Side[i].m_Strength;
                        m_WarSide = g_MatrixMap->m_Side[i].m_Id;
                    }
                }
            }
        }
        else {
            int tries = 10;
            int idx = 0;
            do {
                idx = g_MatrixMap->Rnd(0, g_MatrixMap->m_SideCnt - 1);
                if (m_Id != g_MatrixMap->m_Side[idx].m_Id && g_MatrixMap->m_Side[idx].GetStatus() != SS_NONE)
                    break;
            }
            while (--tries > 0);

            if (tries < 1) {
                m_WarSide = -1;
            }
            else
                m_WarSide = g_MatrixMap->m_Side[idx].m_Id;
        }
        m_NextWarSideCalcTime = g_MatrixMap->GetTime() + 60000;
    }

    // Собираем статистику
    m_RobotsCnt = 0;

    for (i = 0; i < MAX_LOGIC_GROUP; i++) {
        m_LogicGroup[i].m_Strength = 0;
    }

    for (i = 0; i < m_TeamCnt; i++) {
        m_Team[i].m_RobotCnt = 0;
        m_Team[i].m_Strength = 0;
        m_Team[i].m_GroupCnt = 0;
        //        m_Team[i].m_WaitUnion=false;
        m_Team[i].SetStay(true);
        m_Team[i].SetWar(false);
        m_Team[i].m_CenterMass.x = 0;
        m_Team[i].m_CenterMass.y = 0;
        m_Team[i].m_RadiusMass = 0;
        m_Team[i].m_Rect = CRect(1000000000, 1000000000, 0, 0);
        m_Team[i].m_Center.x = 0;
        m_Team[i].m_Center.y = 0;
        m_Team[i].m_Radius = 0;
        m_Team[i].m_ActionCnt = 0;
        //        m_Team[i].m_RegionMass=-1;
        m_Team[i].m_RegionNearDanger = -1;
        m_Team[i].m_RegionFarDanger = -1;
        m_Team[i].m_RegionNearEnemy = -1;
        m_Team[i].m_RegionNearRetreat = -1;
        m_Team[i].m_RegionNearForward = -1;
        m_Team[i].m_RegionNerestBase = -1;
        m_Team[i].m_ActionPrev = m_Team[i].m_Action;
        m_Team[i].m_RegionNext = -1;
        m_Team[i].SetRobotInDesRegion(false);

        m_Team[i].Move(0);

        m_Team[i].m_RegionListCnt = 0;
    }

    for (i = 0; i < g_MatrixMap->m_RN.m_RegionCnt; i++) {
        m_Region[i].m_WarEnemyRobotCnt = 0;
        m_Region[i].m_WarEnemyCannonCnt = 0;
        m_Region[i].m_WarEnemyBuildingCnt = 0;
        m_Region[i].m_WarEnemyBaseCnt = 0;
        m_Region[i].m_EnemyRobotCnt = 0;
        m_Region[i].m_EnemyCannonCnt = 0;
        m_Region[i].m_EnemyBuildingCnt = 0;
        m_Region[i].m_EnemyBaseCnt = 0;
        m_Region[i].m_NeutralCannonCnt = 0;
        m_Region[i].m_NeutralBuildingCnt = 0;
        m_Region[i].m_NeutralBaseCnt = 0;
        m_Region[i].m_OurRobotCnt = 0;
        m_Region[i].m_OurCannonCnt = 0;
        m_Region[i].m_OurBuildingCnt = 0;
        m_Region[i].m_OurBaseCnt = 0;
        m_Region[i].m_EnemyRobotDist = -1;
        m_Region[i].m_EnemyBuildingDist = -1;
        m_Region[i].m_OurBaseDist = -1;
        m_Region[i].m_Danger = 0;
        m_Region[i].m_DangerAdd = 0;
    }

    ms = CMatrixMapStatic::GetFirstLogic();
    while (ms) {
        switch (ms->GetObjectType()) {
            case OBJECT_TYPE_BUILDING:
                i = GetRegion(int(((CMatrixBuilding *)ms)->m_Pos.x / GLOBAL_SCALE_MOVE),
                              int(((CMatrixBuilding *)ms)->m_Pos.y / GLOBAL_SCALE_MOVE));
                if (i >= 0) {
                    if (((CMatrixBuilding *)ms)->m_Side == 0)
                        m_Region[i].m_NeutralBuildingCnt++;
                    else if (((CMatrixBuilding *)ms)->m_Side != m_Id) {
                        m_Region[i].m_EnemyBuildingCnt++;
                        if (ms->GetSide() == m_WarSide)
                            m_Region[i].m_WarEnemyBuildingCnt++;
                    }
                    else
                        m_Region[i].m_OurBuildingCnt++;

                    if (((CMatrixBuilding *)ms)->m_Kind == 0) {
                        if (((CMatrixBuilding *)ms)->m_Side == 0)
                            m_Region[i].m_NeutralBaseCnt++;
                        else if (((CMatrixBuilding *)ms)->m_Side != m_Id) {
                            m_Region[i].m_EnemyBaseCnt++;
                            if (ms->GetSide() == m_WarSide)
                                m_Region[i].m_WarEnemyBaseCnt++;
                        }
                        else {
                            ourbasecnt++;
                            m_Region[i].m_OurBaseCnt++;
                        }
                    }
                }
                break;
            case OBJECT_TYPE_ROBOTAI:

                if (((CMatrixRobotAI *)ms)->m_CurrState != ROBOT_DIP) {
                    tp.x = int(ms->AsRobot()->m_PosX / GLOBAL_SCALE_MOVE);
                    tp.y = int(ms->AsRobot()->m_PosY / GLOBAL_SCALE_MOVE);
                    i = GetRegion(tp);

                    if (i >= 0) {
                        if (ms->AsRobot()->m_Side == 0)
                            ;
                        else if (ms->AsRobot()->m_Side != m_Id) {
                            m_Region[i].m_EnemyRobotCnt++;
                            if (ms->GetSide() == m_WarSide)
                                m_Region[i].m_WarEnemyRobotCnt++;
                            float d = ms->AsRobot()->GetStrength();
                            // if(ms->GetSide()!=PLAYER_SIDE) d*=1.5; // Завышаем опасность между компьютерами. Чтобы
                            // они воевали не межу собой, а против игрока.
                            m_Region[i].m_Danger += d * m_DangerMul;
                        }
                        else
                            m_Region[i].m_OurRobotCnt++;
                    }
                    if (ms->AsRobot()->m_Side == m_Id) {
                        m_RobotsCnt++;
                        team = ((CMatrixRobotAI *)ms)->GetTeam();
                        if (team >= 0 && team < m_TeamCnt) {
                            m_Team[team].m_RobotCnt++;
                            m_Team[team].m_Strength += ms->AsRobot()->GetStrength();
                            m_Team[team].m_CenterMass += tp;
                            m_Team[team].m_Rect.left = std::min(m_Team[team].m_Rect.left, tp.x);
                            m_Team[team].m_Rect.top = std::min(m_Team[team].m_Rect.top, tp.y);
                            m_Team[team].m_Rect.right = std::max(m_Team[team].m_Rect.right, tp.x);
                            m_Team[team].m_Rect.bottom = std::max(m_Team[team].m_Rect.bottom, tp.y);

                            m_Team[team].OrMove(1 << (ms->AsRobot()->m_Unit[0].u1.s1.m_Kind - 1));

                            if (i == m_Team[team].m_Action.m_Region)
                                m_Team[team].SetRobotInDesRegion(true);

                            for (t = 0; t < m_Team[team].m_RegionListCnt; t++)
                                if (m_Team[team].m_RegionList[t] == i)
                                    break;
                            if (t >= m_Team[team].m_RegionListCnt) {
                                m_Team[team].m_RegionListCnt++;
                                m_Team[team].m_RegionList[t] = i;
                                m_Team[team].m_RegionListRobots[t] = 1;
                            }
                            else {
                                m_Team[team].m_RegionListRobots[t]++;
                            }

                            if (!m_Team[team].IsWar())
                                m_Team[team].SetWar(ms->AsRobot()->GetEnv()->GetEnemyCnt() > 0);
                        }

                        if (ms->AsRobot()->FindOrderLikeThat(ROT_MOVE_TO) ||
                            ms->AsRobot()->FindOrderLikeThat(ROT_MOVE_RETURN))
                            m_Team[team].SetStay(false);

                        ASSERT(ms->AsRobot()->GetGroupLogic() >= 0 && ms->AsRobot()->GetGroupLogic() < MAX_LOGIC_GROUP);
                        m_LogicGroup[ms->AsRobot()->GetGroupLogic()].m_Strength += ms->AsRobot()->GetStrength();
                    }
                }
                break;

            case OBJECT_TYPE_CANNON:
                tp.x = TruncFloat(ms->AsCannon()->m_Pos.x * INVERT(GLOBAL_SCALE_MOVE));
                tp.y = TruncFloat(ms->AsCannon()->m_Pos.y * INVERT(GLOBAL_SCALE_MOVE));
                i = GetRegion(tp);
                if (i >= 0) {
                    if (ms->GetSide() == 0) {
                        m_Region[i].m_NeutralCannonCnt++;
                        m_Region[i].m_DangerAdd += ((CMatrixCannon *)ms)->GetStrength() * m_DangerMul;
                    }
                    else if (ms->GetSide() != m_Id) {
                        m_Region[i].m_EnemyCannonCnt++;
                        if (ms->GetSide() == m_WarSide)
                            m_Region[i].m_WarEnemyCannonCnt++;

                        float d = ((CMatrixCannon *)ms)->GetStrength();
                        // if(ms->GetSide()!=PLAYER_SIDE) d*=1.5; // Завышаем опасность между компьютерами. Чтобы они
                        // воевали не межу собой, а против игрока.

                        m_Region[i].m_DangerAdd += d * m_DangerMul;
                    }
                    else
                        m_Region[i].m_OurCannonCnt++;
                }
                break;
        }
        ms = ms->GetNextLogic();
    }

    // Выращиваем опасность
    for (i = 0; i < g_MatrixMap->m_RN.m_RegionCnt; i++) {
        if (m_Region[i].m_Danger <= 0)
            continue;

        SMatrixLogicRegion *lr = m_Region;
        for (u = 0; u < g_MatrixMap->m_RN.m_RegionCnt; u++, lr++)
            lr->m_Data = 0;

        for (u = 0; u < g_MatrixMap->m_RN.m_Region[i].m_NearCnt; u++) {
            t = g_MatrixMap->m_RN.m_Region[i].m_Near[u];
            if (m_Region[t].m_Data > 0)
                continue;

            m_Region[t].m_DangerAdd += m_Region[i].m_Danger;
            m_Region[t].m_Data = 1;
        }

        /*        cnt=0; sme=0; dist=0;
                m_RegionIndex[cnt]=i;
                m_Region[i].m_Data=3;
                cnt++;

                while(sme<cnt) {
                    for(u=0;u<g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt;u++) {
                        t=g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_Near[u];
                        if(m_Region[t].m_Data>0) continue;

                        m_Region[t].m_DangerAdd+=m_Region[i].m_Danger;

                        if(m_Region[t].m_Danger>0) {
                            m_RegionIndex[cnt]=t;
                            m_Region[t].m_Data=3;
                            cnt++;
                        } else {
                            m_Region[t].m_Data=m_Region[m_RegionIndex[sme]].m_Data-1;
                            if(m_Region[t].m_Data>=2) {
                                m_RegionIndex[cnt]=t;
                                cnt++;
                            }
                        }
                    }
                    sme++;
                }*/
    }
    for (i = 0; i < g_MatrixMap->m_RN.m_RegionCnt; i++)
        m_Region[i].m_Danger += m_Region[i].m_DangerAdd;

    for (i = 0; i < m_TeamCnt; i++) {
        if (m_Team[i].m_RobotCnt <= 0) {
            m_Team[i].m_RegionMass = -1;
            m_Team[i].m_RegionMassPrev = -1;
            m_Team[i].m_Brave = 0;
            m_Team[i].m_BraveStrangeCancel = 0;
            m_Team[i].SetWar(false);
            continue;
        }

        if (m_Team[i].IsWar())
            m_Team[i].SetRegroupOnlyAfterWar(false);

        //        if(!m_Team[i].m_WarCalc) m_Team[i].m_War=false;

        // Сортируем список по кол-во роботов
        for (t = 0; t < m_Team[i].m_RegionListCnt - 1; t++) {
            for (u = t + 1; u < m_Team[i].m_RegionListCnt; u++) {
                if (m_Team[i].m_RegionListRobots[u] > m_Team[i].m_RegionListRobots[t]) {
                    k = m_Team[i].m_RegionList[u];
                    m_Team[i].m_RegionList[u] = m_Team[i].m_RegionList[t];
                    m_Team[i].m_RegionList[t] = k;
                    k = m_Team[i].m_RegionListRobots[u];
                    m_Team[i].m_RegionListRobots[u] = m_Team[i].m_RegionListRobots[t];
                    m_Team[i].m_RegionListRobots[t] = k;
                }
            }
        }

        m_Team[i].m_CenterMass.x = m_Team[i].m_CenterMass.x / m_Team[i].m_RobotCnt;
        m_Team[i].m_CenterMass.y = m_Team[i].m_CenterMass.y / m_Team[i].m_RobotCnt;
        m_Team[i].m_Center.x = (m_Team[i].m_Rect.left + m_Team[i].m_Rect.right) / 2;
        m_Team[i].m_Center.y = (m_Team[i].m_Rect.top + m_Team[i].m_Rect.bottom) / 2;

        ASSERT(m_Team[i].m_RegionListCnt);

        int cr = -1;
        k = m_Team[i].m_RegionListRobots[0];
        if (m_Team[i].m_Action.m_Type != mlat_None) {
            for (t = 0; t < m_Team[i].m_RegionListCnt && k == m_Team[i].m_RegionListRobots[t]; t++) {
                if (m_Team[i].m_Action.m_Region == m_Team[i].m_RegionList[t]) {
                    cr = m_Team[i].m_Action.m_Region;
                    break;
                }
            }
        }
        if (cr < 0)
            cr = m_Team[i].m_RegionList[0];

        //        int cr=GetRegion(m_Team[i].m_CenterMass);
        // Если регион назначения занят и текущий регион возле региона назначения, то считаем что текущий регион равен
        // региону назначения
        if (m_Team[i].m_Action.m_Type != mlat_None && m_Team[i].m_Action.m_Region >= 0 &&
            m_Team[i].m_Action.m_Region != cr && g_MatrixMap->m_RN.IsNerestRegion(cr, m_Team[i].m_Action.m_Region)) {
            SMatrixRegion *region = g_MatrixMap->m_RN.GetRegion(m_Team[i].m_Action.m_Region);
            for (u = 0; u < region->m_PlaceCnt; u++)
                GetPlacePtr(region->m_Place[u])->m_Data = 0;

            // Находим роботов, текущей команды, места которые не в регионе назначения
            // + помечаем занетые места
            rlcnt = 0;
            ms = CMatrixMapStatic::GetFirstLogic();
            while (ms) {
                if (ms->IsLiveRobot() && ms->GetSide() == m_Id && ms->AsRobot()->GetTeam() == i) {
                    place = ObjPlacePtr(ms);
                    if (place == NULL || place->m_Region != m_Team[i].m_Action.m_Region) {
                        rl[rlcnt] = (CMatrixRobotAI *)ms;
                        rlcnt++;
                    }
                }
                if (IsLiveUnit(ms))
                    ObjPlaceData(ms, 1);
                ms = ms->GetNextLogic();
            }

            // Проверяем, можем ли разместить хоть одного работа в регионе назначения
            if (rlcnt > 0) {
                for (t = 0; t < rlcnt; t++) {
                    for (u = 0; u < region->m_PlaceCnt; u++) {
                        place = GetPlacePtr(region->m_Place[u]);
                        if (place->m_Data)
                            continue;
                        if (!CanMove(place->m_Move, rl[t]))
                            continue;

                        break;
                    }
                    if (u < region->m_PlaceCnt)
                        break;
                }
                if (t >= rlcnt)
                    cr = m_Team[i].m_Action.m_Region;
            }
        }
        if (m_Team[i].m_RegionMass != cr) {
            m_Team[i].m_RegionMassPrev = m_Team[i].m_RegionMass;
            m_Team[i].m_RegionMass = cr;
        }

        // Находим следующий регион в пути
        if (m_Team[i].m_Action.m_Type != mlat_None) {
            for (u = 0; u < m_Team[i].m_Action.m_RegionPathCnt; u++) {
                if (m_Team[i].m_Action.m_RegionPath[u] == m_Team[i].m_RegionMass) {
                    if ((u + 1) < m_Team[i].m_Action.m_RegionPathCnt)
                        m_Team[i].m_RegionNext = m_Team[i].m_Action.m_RegionPath[u + 1];
                    break;
                }
            }
            // Если текущий регион не в пути, то ищем ближайший к пути
            if (m_Team[i].m_Action.m_RegionPathCnt >= 2 && u >= m_Team[i].m_Action.m_RegionPathCnt) {
                for (u = 0; u < g_MatrixMap->m_RN.m_RegionCnt; u++) {
                    m_Region[u].m_Data = 0;
                }
                for (u = 0; u < m_Team[i].m_Action.m_RegionPathCnt; u++) {
                    m_Region[m_Team[i].m_Action.m_RegionPath[u]].m_Data = DWORD(-(u + 1));
                }

                sme = 0;
                cnt = 0;
                k = -1;
                level = 1;
                m_RegionIndex[cnt] = m_Team[i].m_RegionMass;
                m_Region[m_RegionIndex[cnt]].m_Data = level;
                cnt++;
                level++;

                next = cnt;
                while (sme < next) {
                    for (t = 0; t < g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt; t++) {
                        u = g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_Near[t];
                        if (int(m_Region[u].m_Data) > 0)
                            continue;

                        if (k >= 0) {
                            if (int(m_Region[k].m_Data) < int(m_Region[u].m_Data))
                                k = u;
                        }
                        else {
                            if (m_Region[u].m_Data == 0) {
                                m_RegionIndex[cnt] = u;
                                cnt++;
                                m_Region[u].m_Data = level;
                            }
                            else {
                                k = u;
                            }
                        }
                    }
                    sme++;
                    if (sme >= next) {
                        next = cnt;
                        if (k)
                            break;
                        level++;
                    }
                }
                if (k >= 0) {
                    while (true) {
                        p = -1;
                        for (t = 0; t < g_MatrixMap->m_RN.m_Region[k].m_NearCnt; t++) {
                            u = g_MatrixMap->m_RN.m_Region[k].m_Near[t];
                            if (int(m_Region[u].m_Data) <= 0)
                                continue;

                            if (int(m_Region[u].m_Data) < level) {
                                p = u;
                                break;
                            }
                        }
                        if (p < 0)
                            ERROR_E;
                        if (m_Region[p].m_Data <= 1) {
                            m_Team[i].m_RegionNext = k;
                            break;
                        }
                        else {
                            k = p;
                            level = m_Region[k].m_Data;
                        }
                    }
                }
            }
            //            g_MatrixMap->m_RN.FindPathInRegionInit();
            //            m_Team[i].m_RegionPathCnt=g_MatrixMap->m_RN.FindPathInRegionRun(m_Team[i].m_RegionMass,m_Team[i].m_Action.m_Region,m_Team[i].m_RegionPath,128);
            //            if(m_Team[i].m_RegionPathCnt>=2) m_Team[i].m_RegionNext=m_Team[i].m_RegionPath[1];
        }

        if (m_Team[i].m_RegionMass >= 0) {
            if (m_Region[m_Team[i].m_RegionMass].m_EnemyRobotCnt > 0) {
                m_Team[i].m_RegionNearEnemy = m_Team[i].m_RegionMass;
            }
            else {
                for (u = 0; u < g_MatrixMap->m_RN.m_Region[m_Team[i].m_RegionMass].m_NearCnt; u++) {
                    t = g_MatrixMap->m_RN.m_Region[m_Team[i].m_RegionMass].m_Near[u];
                    if (m_Region[t].m_EnemyRobotCnt > 0) {
                        m_Team[i].m_RegionNearEnemy = t;
                        break;
                    }
                }
            }

            float md = 0, md2 = 0;
            if (m_Region[m_Team[i].m_RegionMass].m_Danger > 0) {
                m_Team[i].m_RegionFarDanger = m_Team[i].m_RegionNearDanger = m_Team[i].m_RegionMass;
                md2 = md = m_Region[m_Team[i].m_RegionMass].m_Danger;
            }

            for (u = 0; u < g_MatrixMap->m_RN.m_Region[m_Team[i].m_RegionMass].m_NearCnt; u++) {
                t = g_MatrixMap->m_RN.m_Region[m_Team[i].m_RegionMass].m_Near[u];

                if (m_Region[t].m_Danger > md) {
                    md = m_Region[t].m_Danger;
                    m_Team[i].m_RegionNearDanger = t;
                }
                if (m_Region[t].m_Danger > md2) {
                    md2 = m_Region[t].m_Danger;
                    m_Team[i].m_RegionFarDanger = t;
                }

                for (k = 0; k < g_MatrixMap->m_RN.m_Region[t].m_NearCnt; k++) {
                    p = g_MatrixMap->m_RN.m_Region[t].m_Near[k];

                    if (m_Region[p].m_Danger > md2) {
                        md2 = m_Region[p].m_Danger;
                        m_Team[i].m_RegionFarDanger = p;
                    }
                }
            }

            /*            for(u=0;u<g_MatrixMap->m_RN.m_Region[m_Team[i].m_RegionMass].m_NearCnt;u++) {
                            t=g_MatrixMap->m_RN.m_Region[m_Team[i].m_RegionMass].m_Near[u];
                            if(m_Region[t].m_OurRobotCnt>0 && m_Region[t].m_EnemyRobotCnt<=0)
               m_Team[i].m_RegionNearRetreat=t;
                        }
                        if(m_Team[i].m_RegionNearRetreat<0) {
                            for(u=0;u<g_MatrixMap->m_RN.m_Region[m_Team[i].m_RegionMass].m_NearCnt;u++) {
                                t=g_MatrixMap->m_RN.m_Region[m_Team[i].m_RegionMass].m_Near[u];
                                if(m_Region[t].m_EnemyRobotCnt<=0) m_Team[i].m_RegionNearRetreat=t;
                            }
                        }*/
        }
    }
    ms = CMatrixMapStatic::GetFirstLogic();
    while (ms) {
        if (ms->IsLiveRobot() && ms->AsRobot()->m_Side == m_Id) {
            tp.x = int(ms->AsRobot()->m_PosX / GLOBAL_SCALE_MOVE);
            tp.y = int(ms->AsRobot()->m_PosY / GLOBAL_SCALE_MOVE);

            team = ((CMatrixRobotAI *)ms)->GetTeam();
            if (team >= 0 && team < m_TeamCnt) {
                m_Team[team].m_RadiusMass = std::max(m_Team[team].m_RadiusMass, m_Team[team].m_CenterMass.Dist2(tp));
                m_Team[team].m_Radius = std::max(m_Team[team].m_Radius, m_Team[team].m_Center.Dist2(tp));
            }
        }
        ms = ms->GetNextLogic();
    }
    for (i = 0; i < m_TeamCnt; i++) {
        m_Team[i].m_RadiusMass = int(sqrt(double(m_Team[i].m_RadiusMass)));
        m_Team[i].m_Radius = int(sqrt(double(m_Team[i].m_Radius)));
    }

    for (i = 0; i < m_TeamCnt; i++) {
        m_Team[i].m_GroupCnt = 0;
        for (u = 0; u < MAX_LOGIC_GROUP; u++) {
            if (m_LogicGroup[u].RobotsCnt() <= 0)
                continue;
            if (m_LogicGroup[u].m_Team != i)
                continue;
            m_Team[i].m_GroupCnt++;
        }
    }

    // Стоит ли ждать пока команда соберется
    for (i = 0; i < m_TeamCnt; i++) {
        if (m_Team[i].m_GroupCnt <= 1) {
            m_Team[i].SetWaitUnion(false);
            continue;
        }
        if (m_Team[i].m_RobotCnt <= 1) {
            m_Team[i].SetWaitUnion(false);
            continue;
        }

        int groupms = -1;
        for (u = 0; u < MAX_LOGIC_GROUP; u++) {
            if (m_LogicGroup[u].RobotsCnt() <= 0)
                continue;
            if (m_LogicGroup[u].m_Team != i)
                continue;

            if (groupms < 0)
                groupms = u;
            else {
                if (m_LogicGroup[u].m_Strength > m_LogicGroup[groupms].m_Strength)
                    groupms = u;
            }
        }
        if (groupms < 0) {
            m_Team[i].SetWaitUnion(false);
            continue;
        }

        if (g_MatrixMap->GetTime() - m_Team[i].m_WaitUnionLast < 5000)
            continue;

        m_Team[i].SetWaitUnion(!m_Team[i].IsStay() &&
                               (m_LogicGroup[groupms].m_Strength / m_Team[i].m_Strength) <= 0.8f);
        m_Team[i].m_WaitUnionLast = g_MatrixMap->GetTime();
    }

    /*    CMatrixGroup * mg=m_GroupsList->m_FirstGroup;
        while(mg) {
            m_Team[mg->m_Team].m_GroupCnt++;
            mg=mg->m_NextGroup;
        }*/

    // Как далеко роботы врага
    cnt = 0;
    sme = 0;
    dist = 0;

    for (i = 0; i < g_MatrixMap->m_RN.m_RegionCnt; i++) {
        if (m_Region[i].m_EnemyRobotCnt > 0) {
            m_RegionIndex[cnt] = i;
            cnt++;
            m_Region[i].m_EnemyRobotDist = dist;
        }
    }

    next = cnt;
    dist++;
    while (sme < cnt) {
        for (i = 0; i < g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt; i++) {
            u = g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_Near[i];
            if (m_Region[u].m_EnemyRobotDist >= 0)
                continue;

            m_RegionIndex[cnt] = u;
            cnt++;
            m_Region[u].m_EnemyRobotDist = dist;
        }
        sme++;
        if (sme >= next) {
            next = cnt;
            dist++;
        }
    }

    // Как далеко постройки врага
    cnt = 0;
    sme = 0;
    dist = 0;

    for (i = 0; i < g_MatrixMap->m_RN.m_RegionCnt; i++) {
        if (m_Region[i].m_EnemyBuildingCnt > 0) {
            m_RegionIndex[cnt] = i;
            cnt++;
            m_Region[i].m_EnemyBuildingDist = dist;
        }
    }

    next = cnt;
    dist++;
    while (sme < cnt) {
        for (i = 0; i < g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt; i++) {
            u = g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_Near[i];
            if (m_Region[u].m_EnemyBuildingDist >= 0)
                continue;

            m_RegionIndex[cnt] = u;
            cnt++;
            m_Region[u].m_EnemyBuildingDist = dist;
        }
        sme++;
        if (sme >= next) {
            next = cnt;
            dist++;
        }
    }

    // Как далеко от наших баз
    cnt = 0;
    sme = 0;
    dist = 0;

    for (i = 0; i < g_MatrixMap->m_RN.m_RegionCnt; i++) {
        if (m_Region[i].m_OurBaseCnt > 0) {
            m_RegionIndex[cnt] = i;
            cnt++;
            m_Region[i].m_OurBaseDist = dist;
        }
    }

    next = cnt;
    dist++;
    while (sme < cnt) {
        for (i = 0; i < g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt; i++) {
            u = g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_Near[i];
            if (m_Region[u].m_OurBaseDist >= 0)
                continue;

            m_RegionIndex[cnt] = u;
            cnt++;
            m_Region[u].m_OurBaseDist = dist;
        }
        sme++;
        if (sme >= next) {
            next = cnt;
            dist++;
        }
    }

    // Регион отступления
    for (i = 0; i < m_TeamCnt; i++) {
        if (m_Team[i].m_RobotCnt <= 0)
            continue;
        if (m_Team[i].m_RegionMass < 0)
            continue;
        if (m_Team[i].m_RegionNearDanger < 0)
            continue;

        float md = 1e30f;
        for (u = 0; u < g_MatrixMap->m_RN.m_Region[m_Team[i].m_RegionMass].m_NearCnt; u++) {
            t = g_MatrixMap->m_RN.m_Region[m_Team[i].m_RegionMass].m_Near[u];

            if (std::max(1.0, m_Team[i].m_Strength * 0.4) >= m_Region[t].m_Danger) {
                if (m_Region[t].m_Danger < md) {
                    md = m_Region[t].m_Danger;
                    m_Team[i].m_RegionNearRetreat = t;
                }
                else if (m_Region[t].m_Danger == md && m_Team[i].m_RegionNearRetreat >= 0 &&
                         m_Region[t].m_OurBaseDist < m_Region[m_Team[i].m_RegionNearRetreat].m_OurBaseDist) {
                    md = m_Region[t].m_Danger;
                    m_Team[i].m_RegionNearRetreat = t;
                }
            }
        }
    }

    // Находим ближайшие базы (нужно переписать с учетом непроходимости тоесть по m_OurBaseDist)
    for (i = 0; i < m_TeamCnt; i++) {
        if (m_Team[i].m_RegionMass < 0)
            continue;

        ms = CMatrixMapStatic::GetFirstLogic();
        while (ms) {
            if (ms->GetObjectType() == OBJECT_TYPE_BUILDING && ((CMatrixBuilding *)ms)->m_Kind == 0 &&
                ((CMatrixBuilding *)ms)->m_Side == m_Id) {
                u = GetRegion(int(((CMatrixBuilding *)ms)->m_Pos.x / GLOBAL_SCALE_MOVE),
                              int(((CMatrixBuilding *)ms)->m_Pos.y / GLOBAL_SCALE_MOVE));
                if (u < 0)
                    continue;
                t = g_MatrixMap->m_RN.m_Region[m_Team[i].m_RegionMass].m_Center.Dist2(
                        g_MatrixMap->m_RN.m_Region[u].m_Center);
                if (m_Team[i].m_RegionNerestBase < 0 || t < dist) {
                    m_Team[i].m_RegionNerestBase = u;
                    dist = t;
                    continue;
                }
            }
            ms = ms->GetNextLogic();
        }
    }

    // Строим робота
    BuildRobot();

    // Строим пушки
    BuildCannon();

    // Отвага
    for (i = 0; i < m_TeamCnt; i++) {
        if (m_Team[i].m_RobotCnt <= 0)
            continue;

        //        if(m_Team[i].m_RegionNearDanger<0) {
        if (m_Team[i].m_Brave) {
            if ((g_MatrixMap->GetTime() - m_Team[i].m_Brave) > 10000 &&
                m_Team[i].m_Strength < m_Team[i].m_BraveStrangeCancel) {
                m_Team[i].m_Brave = 0;
            }
        }
        //        }

        //        if(m_Team[i].m_RegionNearDanger>=0) {
        if (!m_Team[i].m_Brave) {
            //                if(m_Team[i].m_Strength>=2000 || ((m_Team[i].m_RobotCnt>=5) &&
            //                (m_Team[i].m_Action.m_Type==mlat_Defence) &&
            //                (g_MatrixMap->GetTime()-m_Team[i].m_ActionTime>3*60*1000))) {
            int bravecnt = GetMaxSideRobots();
            bravecnt = std::min(Float2Int(m_BraveMul * float(bravecnt)), bravecnt);
            if (bravecnt < 1)
                bravecnt = 1;

            if (m_Team[i].m_RobotCnt >= bravecnt) {
                m_Team[i].m_Brave = g_MatrixMap->GetTime();
                m_Team[i].m_BraveStrangeCancel = m_Team[i].m_Strength * 0.3f;
                if (m_Team[i].m_Action.m_Type == mlat_Retreat) {
                    m_Team[i].m_Action.m_Type = mlat_None;
                    m_Team[i].m_ActionTime = g_MatrixMap->GetTime();
                }
            }
        }
        //        }
    }

    // Регионы для продвижения вперед
    //    UpdateTargetRegion();
    //    CalcForwardRegion();

    // Проверяем корректна ли текущая задача
    for (i = 0; i < m_TeamCnt; i++) {
        if (m_Team[i].m_RobotCnt <= 0)
            continue;

        m_Team[i].SetlOk(true);

        if (m_Team[i].m_Action.m_Type == mlat_None) {
            m_Team[i].SetlOk(false);
        }
        else if (m_Team[i].m_Action.m_Type == mlat_Defence) {
            if (m_Team[i].IsWar())
                continue;
            if (m_Team[i].m_Brave && !m_Team[i].IsWaitUnion()) {
                DMTeam(i, m_Team[i].m_Action.m_Type, -1, L"m_Brave==true");
                m_Team[i].SetlOk(false);
                continue;
            }
            if (m_Team[i].m_RegionFarDanger < 0 && (g_MatrixMap->GetTime() - m_Team[i].m_ActionTime > 10000) &&
                !m_Team[i].IsWaitUnion()) {
                DMTeam(i, m_Team[i].m_Action.m_Type, -1, L"m_RegionFarDanger<<0");
                m_Team[i].SetlOk(false);
                continue;
            }
            if (m_Team[i].m_RegionFarDanger >= 0 && (g_MatrixMap->GetTime() - m_Team[i].m_ActionTime > 1000) &&
                m_Region[m_Team[i].m_RegionFarDanger].m_Danger < m_Team[i].m_Strength && !m_Team[i].IsWaitUnion()) {
                DMTeam(i, m_Team[i].m_Action.m_Type, -1, L"m_RegionFarDanger.m_Danger(<f>)<m_Strength(<f>)",
                       m_Region[m_Team[i].m_RegionFarDanger].m_Danger, m_Team[i].m_Strength);
                m_Team[i].SetlOk(false);
                continue;
            }
            //            if(m_Team[i].m_RegionMass>=0 && (m_Region[m_Team[i].m_RegionMass].m_NeutralBuildingCnt>0 ||
            //            m_Region[m_Team[i].m_RegionMass].m_EnemyBuildingCnt>0)) {
            if (m_Team[i].IsRobotInDesRegion() && (m_Region[m_Team[i].m_Action.m_Region].m_NeutralBuildingCnt > 0 ||
                                                   m_Region[m_Team[i].m_Action.m_Region].m_EnemyBuildingCnt > 0)) {
                m_Team[i].m_Action.m_Type = mlat_Capture;
                m_Team[i].m_ActionTime = g_MatrixMap->GetTime();
                DMTeam(i, m_Team[i].m_Action.m_Type, 1, L"From Defence");
                continue;
            }
        }
        else if (m_Team[i].m_Action.m_Type == mlat_Attack) {
            m_Team[i].SetlOk(m_Team[i].IsWar());
            if (!m_Team[i].IslOk())
                DMTeam(i, m_Team[i].m_Action.m_Type, -1, L"m_War==false");
        }
        else if (m_Team[i].m_Action.m_Type == mlat_Forward) {
            if (m_Team[i].IsWaitUnion()) {
                DMTeam(i, m_Team[i].m_Action.m_Type, -1, L"m_WaitUnion==true");
                m_Team[i].SetlOk(false);
                continue;
            }

            if (m_Team[i].m_RegionNext >= 0 &&
                m_Region[m_Team[i].m_RegionNext].m_Danger * 0.6f > m_Team[i].m_Strength) {
                DMTeam(i, m_Team[i].m_Action.m_Type, -1, L"m_RegionNext.m_Danger(<f>)*0.6f>m_Strength(<f>)",
                       m_Region[m_Team[i].m_RegionNext].m_Danger, m_Team[i].m_Strength);
                m_Team[i].SetlOk(false);
                continue;
            }

            m_Team[i].SetlOk(m_Team[i].m_RegionMass != m_Team[i].m_Action.m_Region);
            if (!m_Team[i].IslOk()) {
                DMTeam(i, m_Team[i].m_Action.m_Type, -1, L"m_RegionMass==m_Action.m_Region");
            }
            if (m_Team[i].IslOk() && m_Team[i].IsWar()) {
                DMTeam(i, m_Team[i].m_Action.m_Type, -1, L"m_War==true");
                m_Team[i].SetlOk(false);
            }

            // Если идем мимо завода, то захватываем его.
            if (m_Team[i].IslOk() && m_Team[i].m_RegionMass >= 0 &&
                (m_Region[m_Team[i].m_RegionMass].m_NeutralBuildingCnt > 0 ||
                 m_Region[m_Team[i].m_RegionMass].m_EnemyBuildingCnt > 0)) {
                for (u = 0; u < m_TeamCnt; u++) {
                    if (u == i)
                        continue;
                    if (((m_Team[u].m_Action.m_Type = mlat_Capture) || (m_Team[u].m_Action.m_Type = mlat_Forward)) &&
                        (m_Team[u].m_Action.m_Region == m_Team[i].m_RegionMass) &&
                        (m_Team[u].m_Action.m_Region == m_Team[u].m_RegionMass ||
                         g_MatrixMap->m_RN.IsNerestRegion(m_Team[u].m_Action.m_Region, m_Team[u].m_RegionMass)))
                        break;
                }
                if (u >= m_TeamCnt) {
                    for (u = 0; u < m_TeamCnt; u++) {
                        if (u == i)
                            continue;
                        if (((m_Team[u].m_Action.m_Type = mlat_Capture) ||
                             (m_Team[u].m_Action.m_Type = mlat_Forward)) &&
                            m_Team[u].m_Action.m_Region == m_Team[i].m_RegionMass) {
                            DMTeam(u, m_Team[u].m_Action.m_Type, -1, L"From Forward RegionMass");
                            m_Team[u].m_Action.m_Type = mlat_None;
                            m_Team[u].m_ActionTime = g_MatrixMap->GetTime();
                        }
                    }
                    m_Team[i].m_Action.m_Type = mlat_Capture;
                    m_Team[i].m_Action.m_Region = m_Team[i].m_RegionMass;
                    m_Team[i].m_ActionTime = g_MatrixMap->GetTime();
                    DMTeam(i, m_Team[i].m_Action.m_Type, 1, L"From Forward RegionMass");
                }
            }

            // Если не все роботы пришли в регион, но есть завод, который надо захватить, то меняем приказ на захват.
            if (m_Team[i].IslOk() && (m_Region[m_Team[i].m_Action.m_Region].m_NeutralBuildingCnt > 0 ||
                                      m_Region[m_Team[i].m_Action.m_Region].m_EnemyBuildingCnt > 0)) {
                ms = CMatrixMapStatic::GetFirstLogic();
                while (ms) {
                    if (ms->IsLiveRobot() && ms->GetSide() == m_Id && GetObjTeam(ms) == i) {
                        if (GetRegion(ms) == m_Team[i].m_Action.m_Region) {
                            m_Team[i].m_Action.m_Type = mlat_Capture;
                            m_Team[i].m_ActionTime = g_MatrixMap->GetTime();
                            DMTeam(i, m_Team[i].m_Action.m_Type, 1, L"From Forward");
                            break;
                        }
                    }
                    ms = ms->GetNextLogic();
                }
                if (ms) {
                    i--;
                    continue;
                }
            }
        }
        else if (m_Team[i].m_Action.m_Type == mlat_Retreat) {
            m_Team[i].SetlOk(!m_Team[i].IsWar());
            if (!m_Team[i].IslOk())
                DMTeam(i, m_Team[i].m_Action.m_Type, -1, L"m_War==true");

            //            if(m_Region[m_Team[i].m_RegionNearDanger].m_Danger>0) m_Team[i].m_lOk=false;

            //            if(m_Team[i].m_RegionNearDanger>=0 &&
            //            m_Team[i].m_Strength>=m_Region[m_Team[i].m_RegionNearDanger].m_Danger*0.9f) {
            //            m_Team[i].m_lOk=false; continue; }
            if (m_Team[i].IslOk() && m_Team[i].m_Action.m_Region == m_Team[i].m_RegionMass) {
                DMTeam(i, m_Team[i].m_Action.m_Type, -1, L"m_Action.m_Region==m_RegionMass");

                if (m_Team[i].m_RegionFarDanger >= 0) {
                    if (m_Team[i].m_Strength * 0.9 > m_Region[m_Team[i].m_RegionFarDanger].m_Danger &&
                        !m_Team[i].IsWaitUnion()) {
                        m_Team[i].m_Action.m_Type = mlat_Forward;
                        m_Team[i].m_Action.m_Region = m_Team[i].m_RegionFarDanger;
                        m_Team[i].m_ActionTime = g_MatrixMap->GetTime();

                        DMTeam(i, m_Team[i].m_Action.m_Type, 1,
                               L"From Retreat m_Strength(<f>)*0.9>m_RegionFarDanger.m_Danger(<f>)",
                               m_Team[i].m_Strength, m_Region[m_Team[i].m_RegionFarDanger].m_Danger);
                        continue;
                    }
                    else if (m_Team[i].m_Strength > m_Region[m_Team[i].m_RegionFarDanger].m_Danger * 0.6) {
                        m_Team[i].m_Action.m_Type = mlat_Defence;
                        m_Team[i].m_ActionTime = g_MatrixMap->GetTime();

                        DMTeam(i, m_Team[i].m_Action.m_Type, 1,
                               L"From Retreat m_Strength(<f>)>m_RegionFarDanger.m_Danger*0.6(<f>)",
                               m_Team[i].m_Strength, m_Region[m_Team[i].m_RegionFarDanger].m_Danger);
                        continue;
                    }
                }

                m_Team[i].SetlOk(false);
                continue;
            }

            //            if(m_Team[i].m_RegionNearDanger>=0) &&
            //            (m_Team[i].m_Strength<m_Region[m_Team[i].m_RegionNearDanger]*0.9);
        }
        else if (m_Team[i].m_Action.m_Type == mlat_Capture) {
            if (m_Team[i].m_RegionNext >= 0 &&
                m_Region[m_Team[i].m_RegionNext].m_Danger * 0.6f > m_Team[i].m_Strength) {
                DMTeam(i, m_Team[i].m_Action.m_Type, -1, L"m_RegionNext.m_Danger(<f>)*0.6f>m_Strength(<f>)",
                       m_Region[m_Team[i].m_RegionNext].m_Danger, m_Team[i].m_Strength);
                m_Team[i].SetlOk(false);
                continue;
            }

            m_Team[i].SetlOk(m_Region[m_Team[i].m_Action.m_Region].m_NeutralBuildingCnt > 0 ||
                             m_Region[m_Team[i].m_Action.m_Region].m_EnemyBuildingCnt > 0);
            if (!m_Team[i].IslOk())
                DMTeam(i, m_Team[i].m_Action.m_Type, -1, L"m_NeutralBuildingCnt<<=0 && m_EnemyBuildingCnt<<=0");
            if (m_Team[i].IslOk() && m_Team[i].IsWar()) {
                DMTeam(i, m_Team[i].m_Action.m_Type, -1, L"m_War==true");
                m_Team[i].SetlOk(false);
            }
        }
        else if (m_Team[i].m_Action.m_Type == mlat_Intercept) {
            m_Team[i].SetlOk(false);
        }
    }

    // Находим всевозможные варианты продвижения вперед.
    for (i = 0; i < m_TeamCnt; i++) {
        if (m_Team[i].IslOk())
            continue;
        if (m_Team[i].m_RobotCnt <= 0)
            continue;
        if (m_Team[i].IsWaitUnion())
            continue;

        cnt = 0;
        sme = 0;
        dist = 0;

        for (u = 0; u < g_MatrixMap->m_RN.m_RegionCnt; u++) {
            m_Region[u].m_Data = 0;
        }

        m_RegionIndex[cnt] = m_Team[i].m_RegionMass;
        m_Region[m_Team[i].m_RegionMass].m_Data = 1;
        cnt++;

        next = cnt;
        dist = 1;
        while (sme < cnt) {
            for (t = 0; t < g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt; t++) {
                u = g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_Near[t];
                if (m_Region[u].m_Data)
                    continue;

                if (g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearMove[t] & m_Team[i].Move())
                    continue;

                if (!m_Team[i].m_Brave && m_Team[i].m_Strength < m_Region[u].m_Danger * 0.7)
                    continue;  // Если регион слишком опасный, то пропускаем

                m_Region[u].m_Data = 1 + dist;
                m_RegionIndex[cnt] = u;
                cnt++;

                // Команды не должны идти в один регион (недоделанно)
                for (k = 0; k < m_TeamCnt; k++) {
                    if (k == i)
                        continue;
                    if (m_Team[k].m_RobotCnt <= 0)
                        continue;

                    if (m_Team[k].IslOk()) {
                        if (m_Team[k].m_Action.m_Type != mlat_None && m_Team[k].m_Action.m_Region == u)
                            break;
                    }
                    else {
                        if (m_Team[k].m_ActionCnt > 0 && m_Team[k].m_ActionList[0].m_Type != mlat_None &&
                            m_Team[k].m_ActionList[0].m_Region == u)
                            break;
                    }
                }
                if (k < m_TeamCnt)
                    continue;

                if (m_Region[u].m_WarEnemyBuildingCnt > 0 || (m_Region[u].m_EnemyBuildingCnt > 0 && dist <= 1 &&
                                                              m_Team[i].m_Strength * 0.33 > m_Region[u].m_Danger)) {
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_Type = mlat_Forward;
                    //                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPathCnt=dist+1;
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_Region = u;
                    CalcRegionPath(m_Team[i].m_ActionList + m_Team[i].m_ActionCnt, u, m_Team[i].Move());
                    // DMSide(L"CalcRegionPath Start=<i>(<i>) End=<i>(<i>) Cnt=<i> Dist=<i>\n",
                    //        m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPath[0],
                    //        m_Team[i].m_RegionMass,
                    //        m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPath[m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPathCnt-1],
                    //        u,
                    //        m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPathCnt,
                    //        dist);
                    m_Team[i].m_ActionCnt++;
                    LiveAction(i);
                }
                else if (m_Region[u].m_NeutralBuildingCnt > 0) {
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_Type = mlat_Forward;
                    //                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPathCnt=dist+1;
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_Region = u;
                    CalcRegionPath(m_Team[i].m_ActionList + m_Team[i].m_ActionCnt, u, m_Team[i].Move());
                    // DMSide(L"CalcRegionPath Start=<i>(<i>) End=<i>(<i>) Cnt=<i> Dist=<i>\n",
                    //        m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPath[0],
                    //        m_Team[i].m_RegionMass,
                    //        m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPath[m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPathCnt-1],
                    //        u,
                    //        m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPathCnt,
                    //        dist);
                    m_Team[i].m_ActionCnt++;
                    LiveAction(i);
                }
                else if (m_Region[u].m_WarEnemyRobotCnt > 0) {
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_Type = mlat_Forward;
                    //                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPathCnt=dist+1;
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_Region = u;
                    CalcRegionPath(m_Team[i].m_ActionList + m_Team[i].m_ActionCnt, u, m_Team[i].Move());
                    // DMSide(L"CalcRegionPath Start=<i>(<i>) End=<i>(<i>) Cnt=<i> Dist=<i>\n",
                    //        m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPath[0],
                    //        m_Team[i].m_RegionMass,
                    //        m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPath[m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPathCnt-1],
                    //        u,
                    //        m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPathCnt,
                    //        dist);
                    m_Team[i].m_ActionCnt++;
                    LiveAction(i);
                }
            }
            sme++;
            if (sme >= next) {
                next = cnt;
                dist++;
            }
        }
        BestAction(i);
    }
    // Если вариантов продвижения вперед нет. То создаем вариант защиты.
    for (i = 0; i < m_TeamCnt; i++) {
        if (m_Team[i].m_RobotCnt <= 0)
            continue;
        if (m_Team[i].IslOk())
            continue;
        if (m_Team[i].m_ActionCnt > 0)
            continue;
        if (m_Team[i].m_RegionMass < 0)
            continue;
        if (m_Team[i].IsWar())
            continue;

        ac = m_Team[i].m_ActionList + m_Team[i].m_ActionCnt;
        ac->m_Type = mlat_Defence;
        ac->m_Region = m_Team[i].m_RegionMass;
        ac->m_RegionPathCnt = 1;
        ac->m_RegionPath[0] = m_Team[i].m_RegionMass;
        m_Team[i].m_ActionCnt++;
    }

    /*    // Если вариантов продвижения вперед нет. То создаем вариант двигаться на вражескую базу или завод.
        skipregioncnt=0;
        for(i=0;i<m_TeamCnt;i++) {
            if(m_Team[i].m_RobotCnt<=0) continue;
            if(!m_Team[i].m_lOk) continue;
            if(m_Team[i].m_Action.m_Type==mlat_None) continue;

            skipregion[skipregioncnt]=m_Team[i].m_Action.m_Region;
            skipregioncnt++;
        }
        for(i=0;i<m_TeamCnt;i++) {
            if(m_Team[i].m_RobotCnt<=0) continue;
            if(m_Team[i].m_lOk) continue;
            if(m_Team[i].m_ActionCnt>0) continue;
            if(m_Team[i].m_RegionMass<0) continue;
            if(m_Team[i].m_War) continue;

            u=FindNearRegionWithUTR(m_Team[i].m_RegionMass,skipregion,skipregioncnt,2+4+8+16);
            if(u<0) continue;
            ac=m_Team[i].m_ActionList+m_Team[i].m_ActionCnt;
            ac->m_Type=mlat_Forward;
            ac->m_Region=u;
            g_MatrixMap->m_RN.FindPathInRegionInit();
            ac->m_RegionPathCnt=g_MatrixMap->m_RN.FindPathInRegionRun(m_Team[i].m_Move,m_Team[i].m_RegionMass,u,ac->m_RegionPath,REGION_PATH_MAX_CNT,true);
            m_Team[i].m_ActionCnt++;

            skipregion[skipregioncnt]=u;
            skipregioncnt++;
        }*/

    // Находим всевозможные варианты действий
    for (i = 0; i < m_TeamCnt; i++) {
        if (m_Team[i].IslOk())
            continue;
        if (m_Team[i].m_RobotCnt <= 0)
            continue;

        /*        SMatrixRegion * region=g_MatrixMap->m_RN.GetRegion(m_Team[i].m_RegionMass);
                for(int t=0;t<region->m_NearCnt;t++) {
                    region->m_Near[t];

                }*/

        if (m_Team[i].IsWar() && m_Team[i].m_ActionCnt < 16) {
            ac = m_Team[i].m_ActionList + m_Team[i].m_ActionCnt;
            ac->m_Type = mlat_Attack;
            ac->m_Region = m_Team[i].m_RegionMass;
            ac->m_RegionPathCnt = 1;
            ac->m_RegionPath[0] = m_Team[i].m_RegionMass;
            m_Team[i].m_ActionCnt++;
        }
        if (m_Team[i].IsWar() && m_Team[i].m_RegionNearEnemy >= 0 && m_Team[i].m_ActionCnt < 16) {
            ac = m_Team[i].m_ActionList + m_Team[i].m_ActionCnt;
            ac->m_Type = mlat_Attack;
            ac->m_Region = m_Team[i].m_RegionNearEnemy;
            ac->m_RegionPathCnt = 2;
            ac->m_RegionPath[0] = m_Team[i].m_RegionMass;
            ac->m_RegionPath[1] = ac->m_Region;
            m_Team[i].m_ActionCnt++;
        }
        if (m_Team[i].IsWar() && m_Team[i].m_ActionCnt < 16) {
            ac = m_Team[i].m_ActionList + m_Team[i].m_ActionCnt;
            ac->m_Type = mlat_Defence;
            ac->m_Region = m_Team[i].m_RegionMass;
            ac->m_RegionPathCnt = 1;
            ac->m_RegionPath[0] = m_Team[i].m_RegionMass;
            m_Team[i].m_ActionCnt++;
        }
        if (m_Team[i].IsWar() && m_Team[i].m_RegionNearDanger > 0 &&
            m_Region[m_Team[i].m_RegionNearDanger].m_Danger > m_Team[i].m_Strength && m_Team[i].m_ActionCnt < 16) {
            ac = m_Team[i].m_ActionList + m_Team[i].m_ActionCnt;
            ac->m_Type = mlat_Defence;
            ac->m_Region = m_Team[i].m_RegionMass;
            ac->m_RegionPathCnt = 1;
            ac->m_RegionPath[0] = m_Team[i].m_RegionMass;
            m_Team[i].m_ActionCnt++;
        }
        if (!m_Team[i].IsWar() && !m_Team[i].m_Brave && m_Team[i].m_RegionNearDanger > 0 &&
            m_Region[m_Team[i].m_RegionNearDanger].m_Danger * 0.8 > m_Team[i].m_Strength &&
            m_Team[i].m_RegionNearRetreat >= 0) {
            ac = m_Team[i].m_ActionList + m_Team[i].m_ActionCnt;
            ac->m_Type = mlat_Retreat;
            ac->m_Region = m_Team[i].m_RegionNearRetreat;
            ac->m_RegionPathCnt = 2;
            ac->m_RegionPath[0] = m_Team[i].m_RegionMass;
            ac->m_RegionPath[1] = ac->m_Region;
            m_Team[i].m_ActionCnt++;
        }
        if (!m_Team[i].IsWar() && m_Team[i].IsWaitUnion()) {
            ac = m_Team[i].m_ActionList + m_Team[i].m_ActionCnt;
            ac->m_Type = mlat_Defence;
            ac->m_Region = m_Team[i].m_RegionMass;
            ac->m_RegionPathCnt = 1;
            ac->m_RegionPath[0] = m_Team[i].m_RegionMass;
            m_Team[i].m_ActionCnt++;
        }

        /*        if(m_Team[i].m_War && m_Team[i].m_RegionNearRetreat>=0 && m_Team[i].m_ActionCnt<16) {
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_Type=mlat_Retreat;
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_Region=m_Team[i].m_RegionNearRetreat;
                    m_Team[i].m_ActionCnt++;
                }*/
        /*        if(m_Team[i].m_War && m_Team[i].m_RegionMass>=0 && m_Team[i].m_ActionCnt<16) {
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_Type=mlat_Defence;
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_Region=m_Team[i].m_RegionMass;
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPathCnt=0;
                    m_Team[i].m_ActionCnt++;
                }
                if(!m_Team[i].m_War && m_Team[i].m_RegionMass>=0 && m_Team[i].m_RadiusMass>50 &&
           m_Team[i].m_ActionCnt<16) { m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_Type=mlat_Defence;
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_Region=m_Team[i].m_RegionMass;
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_RegionPathCnt=0;
                    m_Team[i].m_ActionCnt++;
                }*/
        /*        if(!m_Team[i].m_War && m_Team[i].m_RegionNearForward>=0 && m_Team[i].m_ActionCnt<16) {
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_Type=mlat_Forward;
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_Region=m_Team[i].m_RegionNearForward;
                    m_Team[i].m_ActionCnt++;
                }*/
        /*        if(!m_Team[i].m_War && m_Team[i].m_TargetRegion>=0 && m_Team[i].m_ActionCnt<16) {
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_Type=mlat_Forward;
                    m_Team[i].m_ActionList[m_Team[i].m_ActionCnt].m_Region=m_Team[i].m_TargetRegion;
                    m_Team[i].m_ActionCnt++;
                }*/
        if (!m_Team[i].IsWar() && m_Team[i].m_RegionMass >= 0 &&
            //            m_Team[i].m_TargetRegion>=0 &&
            //            m_Team[i].m_TargetRegion==m_Team[i].m_RegionMass &&
            (m_Region[m_Team[i].m_RegionMass].m_NeutralBuildingCnt > 0 ||
             m_Region[m_Team[i].m_RegionMass].m_EnemyBuildingCnt > 0) &&
            m_Team[i].m_ActionCnt < 16) {
            // Если другая группа уже захватывает в этом регионе, то вариант не создаем
            for (u = 0; u < m_TeamCnt; u++) {
                if (u == i)
                    continue;
                if (!m_Team[u].IslOk())
                    continue;
                if (m_Team[u].m_RobotCnt <= 0)
                    continue;
                if (m_Team[u].m_Action.m_Type != mlat_Capture)
                    continue;
                if (m_Team[u].m_Action.m_Region != m_Team[i].m_RegionMass)
                    continue;
                break;
            }
            if (u >= m_TeamCnt) {
                ac = m_Team[i].m_ActionList + m_Team[i].m_ActionCnt;
                ac->m_Type = mlat_Capture;
                ac->m_Region = m_Team[i].m_RegionMass;
                ac->m_RegionPathCnt = 1;
                ac->m_RegionPath[0] = m_Team[i].m_RegionMass;
                m_Team[i].m_ActionCnt++;
            }
        }
    }

    // Находим лучший вариант
    for (i = 0; i < m_TeamCnt; i++) {
        if (m_Team[i].IslOk())
            continue;

        BestAction(i);
        if (m_Team[i].m_ActionCnt > 0) {
            m_Team[i].m_Action = m_Team[i].m_ActionList[0];
            m_Team[i].m_ActionTime = g_MatrixMap->GetTime();

            DMTeam(i, m_Team[i].m_Action.m_Type, 1,
                   L"m_Strange=<f> m_RegionMass.m_Danger=<f> m_RegionNearDanger.m_Danger=<f>", m_Team[i].m_Strength,
                   m_Team[i].m_RegionMass < 0 ? 0 : m_Region[m_Team[i].m_RegionMass].m_Danger,
                   m_Team[i].m_RegionNearDanger < 0 ? 0 : m_Region[m_Team[i].m_RegionNearDanger].m_Danger);

            m_Team[i].m_RoadPath->ClearFast();
            if (m_Team[i].m_Action.m_RegionPathCnt > 1) {
#if (defined _DEBUG) && !(defined _RELDEBUG)
                if (!g_TestLocal) {
                    bool test = false;
                    ms = CMatrixMapStatic::GetFirstLogic();
                    while (ms) {
                        if (ms->IsLiveRobot() && ms->GetSide() == m_Id && ms->AsRobot()->GetTeam() == i &&
                            ms == g_TestRobot) {
                            test = true;
                            break;
                        }
                        ms = ms->GetNextLogic();
                    }
                    g_MatrixMap->m_RN.FindPathFromRegionPath(m_Team[i].Move(), m_Team[i].m_Action.m_RegionPathCnt,
                                                             m_Team[i].m_Action.m_RegionPath, m_Team[i].m_RoadPath,
                                                             test);
                }
#else
                g_MatrixMap->m_RN.FindPathFromRegionPath(m_Team[i].Move(), m_Team[i].m_Action.m_RegionPathCnt,
                                                         m_Team[i].m_Action.m_RegionPath, m_Team[i].m_RoadPath);
#endif
            }

            //            g_MatrixMap->m_RN.FindPathInRegionInit();
            //            m_Team[i].m_Action.m_RegionPathCnt=g_MatrixMap->m_RN.FindPathInRegionRun(m_Team[i].m_RegionMass,m_Team[i].m_Action.m_Region,m_Team[i].m_Action.m_RegionPath,16);
        }
        else {
            m_Team[i].m_Action.m_Type = mlat_None;
            m_Team[i].m_ActionTime = g_MatrixMap->GetTime();
        }
    }

    // Перегруппируем один раз за такт, так как статистика становится не правильной.
    int changeok = false;

    // Если отступаем, то пытаемся найти подмогу
    if (!changeok)
        for (i = 0; i < m_TeamCnt; i++) {
            if (m_Team[i].IslOk())
                continue;
            if (m_Team[i].m_RobotCnt <= 0)
                continue;
            if (m_Team[i].m_Action.m_Type != mlat_Retreat)
                continue;
            if (m_Team[i].m_RegionFarDanger < 0)
                continue;

            // Создаем список групп, которые можно присоединить
            int gi[MAX_LOGIC_GROUP];
            int gid[MAX_LOGIC_GROUP];
            int gicnt = 0;

            for (u = 0; u < MAX_LOGIC_GROUP; u++) {
                if (m_LogicGroup[u].RobotsCnt() <= 0)
                    continue;
                if (m_LogicGroup[u].m_Team == i)
                    continue;
                if (m_LogicGroup[u].IsWar())
                    continue;
                if (m_LogicGroup[u].m_Action.m_Type == mlat_Attack)
                    continue;

                ms = CMatrixMapStatic::GetFirstLogic();
                while (ms) {
                    if (ms->IsLiveRobot() && ms->GetSide() == m_Id && GetGroupLogic(ms) == u)
                        break;
                    ms = ms->GetNextLogic();
                }
                if (!ms)
                    continue;

                int rr = GetRegion(ms);
                if (rr != m_Team[i].m_Action.m_Region) {
                    int path[8];
                    gid[gicnt] = g_MatrixMap->m_RN.FindPathInRegionRun(m_Team[i].Move(), rr,
                                                                       m_Team[i].m_Action.m_Region, path, 8, false);
                    if (gid[gicnt] <= 0)
                        continue;

                    for (t = 0; t < gid[gicnt]; t++) {  // Врагов на пути не должно быть
                        if (m_Region[path[t]].m_EnemyRobotCnt)
                            break;
                        if (t >= 4 && m_Region[path[t]].m_Danger > 0)
                            break;
                    }
                    if (t < gid[gicnt])
                        continue;
                }
                else
                    gid[gicnt] = 0;

                if (gid[gicnt] >= 5)
                    continue;  // Слишком далеко

                gi[gicnt] = u;
                gicnt++;
            }

            // Сортируем по дальности
            for (u = 0; u < gicnt - 1; u++) {
                for (t = u + 1; t < gicnt; t++) {
                    if (gid[t] < gid[u]) {
                        k = gid[t];
                        gid[t] = gid[u];
                        gid[u] = k;
                        k = gi[t];
                        gi[t] = gi[u];
                        gi[u] = k;
                    }
                }
            }

            // Объединяем
            for (u = 0; u < gicnt; u++) {
                ms = CMatrixMapStatic::GetFirstLogic();
                while (ms) {
                    if (ms->IsLiveRobot() && ms->GetSide() == m_Id && GetGroupLogic(ms) == gi[u]) {
                        DMSide(L"Robot=<b=16><u> Change team for defence <b=10><i>-><i>", DWORD(ms), k, i);

                        //                    m_Team[ms->AsRobot()->GetTeam()].m_RobotCnt--;
                        //                    m_Team[ms->AsRobot()->GetTeam()].m_Strength-=ms->AsRobot()->GetStrength();
                        ms->AsRobot()->SetGroupLogic(-1);
                        ms->AsRobot()->SetTeam(i);
                        m_Team[i].m_Strength += ms->AsRobot()->GetStrength();
                    }
                    ms = ms->GetNextLogic();
                }
                if (m_Team[i].m_Strength >= m_Team[i].m_RegionFarDanger)
                    break;
            }

            changeok = true;
            m_LastTeamChange = g_MatrixMap->GetTime();
            break;
        }

    // Обедняем, если группа дерется, сил у нее мало, а рядом группа, которая не воюет
    if (!changeok && (g_MatrixMap->GetTime() - m_LastTeamChange) > 3000)
        for (i = 0; i < m_TeamCnt; i++) {
            //        if(m_Team[i].m_lOk) continue;
            if (m_Team[i].m_RobotCnt <= 0)
                continue;
            if (!m_Team[i].IsWar())
                continue;
            if (m_Team[i].m_RegionMass < 0)
                continue;
            if (m_Team[i].m_RegionFarDanger < 0)
                continue;
            if (m_Region[m_Team[i].m_RegionFarDanger].m_Danger < m_Team[i].m_Strength * 0.7f)
                continue;

            for (u = 0; u < m_TeamCnt; u++) {
                if (i == u)
                    continue;
                //            if(m_Team[u].m_lOk) continue;
                if (m_Team[u].m_RobotCnt <= 0)
                    continue;
                if (m_Team[u].IsWar())
                    continue;
                if (m_Team[u].m_RegionMass < 0)
                    continue;
                if (m_Team[u].m_Action.m_Type == mlat_Capture)
                    continue;
                if (m_Team[i].m_RegionMass != m_Team[u].m_RegionMass &&
                    !g_MatrixMap->m_RN.IsNerestRegion(m_Team[i].m_RegionMass, m_Team[u].m_RegionMass))
                    continue;

                break;
            }
            if (u >= m_TeamCnt)
                continue;

            ms = CMatrixMapStatic::GetFirstLogic();
            while (ms) {
                if (ms->IsLiveRobot() && ms->GetSide() == m_Id && ms->AsRobot()->GetTeam() == u) {
                    DMSide(L"Robot=<b=16><u> Change team war <b=10><i>-><i>", DWORD(ms), k, i);
                    ms->AsRobot()->SetTeam(i);
                    ms->AsRobot()->SetGroupLogic(-1);
                }
                ms = ms->GetNextLogic();
            }

            changeok = true;
            m_LastTeamChange = g_MatrixMap->GetTime();
            break;
        }

    // Объединяем группы если они близко, и если есть опасность для обеих групп
    if (!changeok && (g_MatrixMap->GetTime() - m_LastTeamChange) > 3000)
        for (i = 0; i < m_TeamCnt; i++) {
            if (m_Team[i].m_RobotCnt <= 0)
                continue;
            if (m_Team[i].IsWar())
                continue;
            if (m_Team[i].m_RegionFarDanger < 0)
                continue;
            if (m_Team[i].m_RegionMass < 0)
                continue;
            if (m_Region[m_Team[i].m_RegionFarDanger].m_Danger < m_Team[i].m_Strength)
                continue;
            if (m_Team[i].m_Action.m_Type == mlat_Capture)
                continue;

            for (u = 0; u < m_TeamCnt; u++) {
                if (u == i)
                    continue;
                if (m_Team[u].m_RobotCnt <= 0)
                    continue;
                if (m_Team[u].IsWar())
                    continue;
                if (m_Team[u].m_RegionFarDanger < 0)
                    continue;
                if (m_Team[u].m_RegionMass < 0)
                    continue;
                if (m_Region[m_Team[u].m_RegionFarDanger].m_Danger < m_Team[u].m_Strength)
                    continue;
                if (m_Team[u].m_Action.m_Type == mlat_Capture)
                    continue;

                if (m_Team[i].m_RegionMass != m_Team[u].m_RegionMass &&
                    !g_MatrixMap->m_RN.IsNerestRegion(m_Team[i].m_RegionMass, m_Team[u].m_RegionMass))
                    continue;

                break;
            }
            if (u >= m_TeamCnt)
                continue;

            ms = CMatrixMapStatic::GetFirstLogic();
            while (ms) {
                if (ms->IsLiveRobot() && ms->GetSide() == m_Id && ms->AsRobot()->GetTeam() == u) {
                    DMSide(L"Robot=<b=16><u> Change team union group <b=10><i>-><i>", DWORD(ms), k, i);
                    ms->AsRobot()->SetTeam(i);
                    ms->AsRobot()->SetGroupLogic(-1);
                }
                ms = ms->GetNextLogic();
            }

            m_Team[i].m_Action.m_Type = mlat_None;
            m_Team[i].m_ActionTime = g_MatrixMap->GetTime();

            changeok = true;
            m_LastTeamChange = g_MatrixMap->GetTime();
            break;
        }

    // Перераспределяем роботов для пустой группы, если нет опасности
    if (!changeok && (g_MatrixMap->GetTime() - m_LastTeamChange) > 3000)
        for (i = 0; i < m_TeamCnt; i++) {
            if (m_Team[i].m_RobotCnt > 0)
                continue;

            // Находим подходящую команду с максимальным кол-во роботов
            k = -1;
            for (u = 0; u < m_TeamCnt; u++) {
                if (m_Team[u].m_RobotCnt < 2)
                    continue;  // Делить нечего
                if (m_Team[u].m_Action.m_Type != mlat_Forward && m_Team[u].m_Action.m_Type != mlat_Capture)
                    continue;  // Группа занята более важными делами
                if (m_Team[u].m_RegionFarDanger >= 0)
                    continue;
                if (m_Team[u].IsRegroupOnlyAfterWar())
                    continue;

                if (k < 0)
                    k = u;
                else if (m_Team[u].m_RobotCnt > m_Team[k].m_RobotCnt)
                    k = u;
            }
            if (k < 0)
                break;  // Делить нечего

            ClearTeam(i);

            if (m_Team[k].m_GroupCnt == 1) {  // Делим по роботам
                u = m_Team[k].m_RobotCnt / 2;

                ms = CMatrixMapStatic::GetFirstLogic();
                while (ms && u) {
                    if (ms->IsLiveRobot() && ms->GetSide() == m_Id && ms->AsRobot()->GetTeam() == k) {
                        DMSide(L"Robot=<b=16><u> Change team to empty <b=10><i>-><i>", DWORD(ms), k, i);
                        ms->AsRobot()->SetTeam(i);
                        ms->AsRobot()->SetGroupLogic(-1);
                        u--;
                    }
                    ms = ms->GetNextLogic();
                }
            }
            else {  // Делим по группам
                u = m_Team[k].m_GroupCnt / 2;

                for (t = 0; t < MAX_LOGIC_GROUP && u; t++) {
                    if (m_LogicGroup[t].RobotsCnt() <= 0)
                        continue;
                    if (m_LogicGroup[t].m_Team != k)
                        continue;

                    ms = CMatrixMapStatic::GetFirstLogic();
                    while (ms) {
                        if (ms->IsLiveRobot() && ms->GetSide() == m_Id && GetGroupLogic(ms) == t) {
                            DMSide(L"Robot=<b=16><u> Change team to empty <b=10><i>-><i>", DWORD(ms), k, i);
                            ms->AsRobot()->SetTeam(i);
                            ms->AsRobot()->SetGroupLogic(-1);
                        }
                        ms = ms->GetNextLogic();
                    }

                    u--;
                }
            }

            changeok = true;
            m_LastTeamChange = g_MatrixMap->GetTime();
            break;
        }

    // Перераспределяем если команды близко, но численность роботов не равная, и нет опасности
    if (!changeok && (g_MatrixMap->GetTime() - m_LastTeamChange) > 3000)
        for (i = 0; i < m_TeamCnt; i++) {
            if (m_Team[i].m_RobotCnt <= 0)
                continue;
            if (m_Team[i].m_Action.m_Type != mlat_Forward && m_Team[i].m_Action.m_Type != mlat_Capture)
                continue;
            if (m_Team[i].m_RegionMass < 0)
                continue;
            if (m_Team[i].m_RegionFarDanger >= 0)
                continue;
            if (m_Team[i].IsRegroupOnlyAfterWar())
                continue;

            k = -1;
            for (u = 0; u < m_TeamCnt; u++) {
                if (i == u)
                    continue;
                if (m_Team[u].m_RobotCnt <= 0)
                    continue;
                if (m_Team[u].m_RegionMass < 0)
                    continue;
                if (m_Team[u].m_Action.m_Type != mlat_Forward && m_Team[u].m_Action.m_Type != mlat_Capture)
                    continue;
                if (m_Team[u].m_RegionMass != m_Team[i].m_RegionMass &&
                    !g_MatrixMap->m_RN.IsNerestRegion(m_Team[u].m_RegionMass, m_Team[i].m_RegionMass))
                    continue;
                if (m_Team[u].m_RegionFarDanger >= 0)
                    continue;
                if (m_Team[u].IsRegroupOnlyAfterWar())
                    continue;

                if (k < 0)
                    k = u;
                else if (m_Team[u].m_RobotCnt > m_Team[k].m_RobotCnt)
                    k = u;
            }
            if (k < 0)
                continue;

            u = m_Team[u].m_RobotCnt - m_Team[i].m_RobotCnt;
            if (u < 2)
                continue;
            u = u / 2;

            ms = CMatrixMapStatic::GetFirstLogic();
            while (ms && u) {
                if (ms->IsLiveRobot() && ms->GetSide() == m_Id && ms->AsRobot()->GetTeam() == k) {
                    DMSide(L"Robot=<b=16><u> Change team if nerest <b=10><i>-><i>", DWORD(ms), k, i);
                    ms->AsRobot()->SetTeam(i);
                    ms->AsRobot()->SetGroupLogic(-1);
                    u--;
                }
                ms = ms->GetNextLogic();
            }

            m_Team[i].m_Action.m_Type = mlat_None;  // Отменяем приказ, так как он может существенно поменяться, так как
                                                    // место и положение роботов изменилось
            m_Team[i].m_ActionTime = g_MatrixMap->GetTime();

            changeok = true;
            m_LastTeamChange = g_MatrixMap->GetTime();
            break;
        }

    // Объединяем радом стоящие группы, которые находятся в защите. Разбить новую группу можно только после войны.
    if (!changeok && (g_MatrixMap->GetTime() - m_LastTeamChange) > 3000)
        for (i = 0; i < m_TeamCnt; i++) {
            if (m_Team[i].m_RobotCnt <= 0)
                continue;
            if (m_Team[i].IsWaitUnion())
                continue;
            if (m_Team[i].m_Action.m_Type != mlat_Defence)
                continue;
            if (m_Team[i].m_RegionMass < 0)
                continue;

            for (u = 0; u < m_TeamCnt; u++) {
                if (i == u)
                    continue;
                if (m_Team[u].m_RobotCnt <= 0)
                    continue;
                if (m_Team[u].IsWaitUnion())
                    continue;
                if (m_Team[u].m_Action.m_Type != mlat_Defence)
                    continue;
                if (m_Team[u].m_RegionMass < 0)
                    continue;
                if (m_Team[u].IsRegroupOnlyAfterWar())
                    continue;
                if (!(m_Team[i].m_Action.m_Region == m_Team[u].m_Action.m_Region ||
                      g_MatrixMap->m_RN.IsNerestRegion(m_Team[i].m_Action.m_Region, m_Team[u].m_Action.m_Region) ||
                      CanMoveNoEnemy(m_Team[i].Move() | m_Team[u].Move(), m_Team[i].m_Action.m_Region,
                                     m_Team[u].m_Action.m_Region)))
                    continue;

                m_Team[u].m_RobotCnt = 0;
                m_Team[i].SetRegroupOnlyAfterWar(true);

                ms = CMatrixMapStatic::GetFirstLogic();
                while (ms) {
                    if (ms->IsLiveRobot() && ms->GetSide() == m_Id && ms->AsRobot()->GetTeam() == u) {
                        DMSide(L"Robot=<b=16><u> Change team union defence group <b=10><i>-><i>", DWORD(ms), u, i);
                        ms->AsRobot()->SetTeam(i);
                        ms->AsRobot()->SetGroupLogic(-1);
                    }
                    ms = ms->GetNextLogic();
                }
            }
        }

        // dword it2=timeGetTime();
        // DM(L"TaktHL",std::wstring().Format(L"<i>",it2-it1).Get());

        //#if (defined _DEBUG) &&  !(defined _RELDEBUG)
        // if(m_RobotsCnt>0) DM(std::wstring().Format(L"Res2 Side=<i>",m_Id).Get(),std::wstring().Format(L"<i> <i> <i>
        // <i>",m_Resources[0],m_Resources[1],m_Resources[2],m_Resources[3]).Get()); #endif

        // Отображаем
#if (defined _DEBUG) && !(defined _RELDEBUG) && !(defined _DISABLE_AI_HELPERS)
    //    if(m_Id==PLAYER_SIDE)
    for (i = 0; i < m_TeamCnt; i++) {
        CHelper::DestroyByGroup(DWORD(m_Team + i));

        if (m_Team[i].m_Action.m_Type == mlat_None)
            continue;
        if (m_Team[i].m_RobotCnt <= 0)
            continue;

        DWORD colors[3] = {0xffff0000, 0xff00ff00, 0xff0000ff};

        CPoint tp = g_MatrixMap->m_RN.m_Region[m_Team[i].m_Action.m_Region].m_Center;
        float z = g_MatrixMap->GetZ(GLOBAL_SCALE_MOVE * tp.x, GLOBAL_SCALE_MOVE * tp.y);
        CHelper::Create(0, DWORD(m_Team + i))
                ->Cone(D3DXVECTOR3(GLOBAL_SCALE_MOVE * tp.x, GLOBAL_SCALE_MOVE * tp.y, z),
                       D3DXVECTOR3(GLOBAL_SCALE_MOVE * tp.x, GLOBAL_SCALE_MOVE * tp.y, z + 30 - 2.0f * i),
                       4.0f + 1.0f * i, 4.0f + 1.0f * i, colors[i], colors[i], 5);

        //// Отображаем путь по регионам
        // for(u=1;u<m_Team[i].m_Action.m_RegionPathCnt;u++) {
        //    tp=g_MatrixMap->m_RN.m_Region[m_Team[i].m_Action.m_RegionPath[u-1]].m_Center;
        //    D3DXVECTOR3
        //    v1=D3DXVECTOR3(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y,g_MatrixMap->GetZ(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y)+100.0f);
        //    tp=g_MatrixMap->m_RN.m_Region[m_Team[i].m_Action.m_RegionPath[u]].m_Center;
        //    D3DXVECTOR3
        //    v2=D3DXVECTOR3(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y,g_MatrixMap->GetZ(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y)+100.0f);

        //    CHelper::Create(0,DWORD(m_Team+i))->Line(v1,v2,colors[i],colors[i]);
        //}
        //// Отображаем путь по дорогам
        // for(u=1;u<m_Team[i].m_RoadPath->m_Header[0].m_Cnt;u++) {
        //    tp=m_Team[i].m_RoadPath->m_Units[u-1].m_Crotch->m_Center;
        //    D3DXVECTOR3
        //    v1=D3DXVECTOR3(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y,g_MatrixMap->GetZ(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y)+80.0f);
        //    tp=m_Team[i].m_RoadPath->m_Units[u].m_Crotch->m_Center;
        //    D3DXVECTOR3
        //    v2=D3DXVECTOR3(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y,g_MatrixMap->GetZ(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y)+80.0f);

        //    CHelper::Create(0,DWORD(m_Team+i))->Cone(v1,v2,1.0f,1.0f,colors[i],colors[i],5);
        //}
        //// Отображаем следующий регион
        // if(m_Team[i].m_RegionNext>=0) {
        //    tp=g_MatrixMap->m_RN.m_Region[m_Team[i].m_RegionNext].m_Center;
        //    D3DXVECTOR3
        //    v1=D3DXVECTOR3(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y,g_MatrixMap->GetZ(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y));

        //    CHelper::Create(0,DWORD(m_Team+i))->Line(v1,D3DXVECTOR3(v1.x,v1.y,v1.z+100.0f),colors[i],colors[i]);
        //}
    }
#endif
    /*#if (defined _DEBUG) &&  !(defined _RELDEBUG)
        CMatrixMapStatic * obj = CMatrixMapStatic::GetFirstLogic();
        while(obj) {
            if(obj->IsLiveRobot() && obj->GetSide()==m_Id) {
                tp=PLPlacePos(obj->AsRobot());
                if(tp.x>=0) {
                    D3DXVECTOR3 v1,v2,v3,v4;
                    v1.x=tp.x*GLOBAL_SCALE_MOVE; v1.y=tp.y*GLOBAL_SCALE_MOVE; v1.z=g_MatrixMap->GetZ(v1.x,v1.y)+1.0f;
                    v2.x=(tp.x+4)*GLOBAL_SCALE_MOVE; v2.y=tp.y*GLOBAL_SCALE_MOVE;
    v2.z=g_MatrixMap->GetZ(v2.x,v2.y)+1.0f; v3.x=(tp.x+4)*GLOBAL_SCALE_MOVE; v3.y=(tp.y+4)*GLOBAL_SCALE_MOVE;
    v3.z=g_MatrixMap->GetZ(v3.x,v3.y)+1.0f; v4.x=(tp.x)*GLOBAL_SCALE_MOVE; v4.y=(tp.y+4)*GLOBAL_SCALE_MOVE;
    v4.z=g_MatrixMap->GetZ(v4.x,v4.y)+1.0f;

                    CHelper::DestroyByGroup(DWORD(obj)+1);
                    CHelper::Create(10,DWORD(obj)+1)->Triangle(v1,v2,v3,0x8000ff00);
                    CHelper::Create(10,DWORD(obj)+1)->Triangle(v1,v3,v4,0x8000ff00);
                }
    //            D3DXVECTOR2 v=GetWorldPos(obj);
    //            CHelper::DestroyByGroup(DWORD(obj)+2);
    //
    CHelper::Create(10,DWORD(obj)+2)->Cone(D3DXVECTOR3(v.x,v.y,0),D3DXVECTOR3(v.x,v.y,40),float(obj->AsRobot()->GetMinFireDist()),float(obj->AsRobot()->GetMinFireDist()),0x80ffff00,0x80ffff00,20);
    //
    CHelper::Create(10,DWORD(obj)+2)->Cone(D3DXVECTOR3(v.x,v.y,0),D3DXVECTOR3(v.x,v.y,40),float(obj->AsRobot()->GetMaxFireDist()),float(obj->AsRobot()->GetMaxFireDist()),0x80ff0000,0x80ff0000,20);
            }
            obj = obj->GetNextLogic();
        }
    #endif*/

    //    GiveOrder();
}

int CMatrixSideUnit::FindNearRegionWithUTR(int from, int *exclude_list, int exclude_cnt,
                                           DWORD flags)  // 1-our 2-netral 4-enemy 8-base 16-building 32-robot 64-cannon
{
    int i, u, t, next, cnt = 0, sme = 0, level = 1;

    for (i = 0; i < g_MatrixMap->m_RN.m_RegionCnt; i++) {
        m_Region[i].m_Data = 0;
    }

    m_Region[from].m_Data = level;
    m_RegionIndex[cnt] = from;
    cnt++;

    next = cnt;
    level++;
    while (sme < cnt) {
        for (i = 0; i < g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt; i++) {
            u = g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_Near[i];
            if (m_Region[u].m_Data > 0)
                continue;

            for (t = 0; t < exclude_cnt; t++)
                if (u == exclude_list[t])
                    break;
            if (t >= exclude_cnt) {
                if (flags & 1) {
                    if ((flags & 8) && (m_Region[u].m_OurBaseCnt))
                        return u;
                    else if ((flags & 16) && (m_Region[u].m_OurBuildingCnt))
                        return u;
                    else if ((flags & 32) && (m_Region[u].m_OurRobotCnt))
                        return u;
                    else if ((flags & 64) && (m_Region[u].m_OurCannonCnt))
                        return u;
                }
                if (flags & 2) {
                    if ((flags & 8) && (m_Region[u].m_NeutralBaseCnt))
                        return u;
                    else if ((flags & 16) && (m_Region[u].m_NeutralBuildingCnt))
                        return u;
                    //                else if((flags & 32) && (m_Region[u].m_NeutralRobotCnt)) return u;
                    else if ((flags & 64) && (m_Region[u].m_NeutralCannonCnt))
                        return u;
                }
                if (flags & 4) {
                    if ((flags & 8) && (m_Region[u].m_EnemyBaseCnt))
                        return u;
                    else if ((flags & 16) && (m_Region[u].m_EnemyBuildingCnt))
                        return u;
                    else if ((flags & 32) && (m_Region[u].m_EnemyRobotCnt))
                        return u;
                    else if ((flags & 64) && (m_Region[u].m_EnemyCannonCnt))
                        return u;
                }
            }

            m_RegionIndex[cnt] = u;
            cnt++;
            m_Region[u].m_Data = level;
        }
        sme++;
        if (sme >= next) {
            next = cnt;
            level++;
        }
    }
    return -1;
}

int CMatrixSideUnit::CompareAction(int team, SMatrixLogicAction *a1, SMatrixLogicAction *a2) {
    DTRACE();

    int scale = 1;
    if (a1->m_Type > a2->m_Type) {
        SMatrixLogicAction *temp = a1;
        a1 = a2;
        a2 = temp;
        scale = -1;
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    if (a1->m_Type == mlat_Defence && a2->m_Type == mlat_Defence) {
        if (m_Region[a1->m_Region].m_Danger != m_Region[a2->m_Region].m_Danger) {
            if (m_Region[a1->m_Region].m_Danger < m_Region[a2->m_Region].m_Danger)
                return 1 * scale;
            else
                return -1 * scale;
        }
        return 0 * scale;
    }
    else if (a1->m_Type == mlat_Attack && a2->m_Type == mlat_Attack) {
        return 0 * scale;
    }
    else if (a1->m_Type == mlat_Forward && a2->m_Type == mlat_Forward) {
        if (a1->m_RegionPathCnt != a2->m_RegionPathCnt) {
            if (a1->m_RegionPathCnt > a2->m_RegionPathCnt)
                return -1 * scale;
            else
                return 1 * scale;
        }
        if (m_Region[a1->m_Region].m_EnemyBuildingCnt != m_Region[a2->m_Region].m_EnemyBuildingCnt) {
            if (m_Region[a1->m_Region].m_EnemyBuildingCnt < m_Region[a2->m_Region].m_EnemyBuildingCnt)
                return -1 * scale;
            else
                return 1 * scale;
        }
        if (m_Region[a1->m_Region].m_NeutralBuildingCnt != m_Region[a2->m_Region].m_NeutralBuildingCnt) {
            if (m_Region[a1->m_Region].m_NeutralBuildingCnt < m_Region[a2->m_Region].m_NeutralBuildingCnt)
                return -1 * scale;
            else
                return 1 * scale;
        }
        return 0 * scale;
    }
    else if (a1->m_Type == mlat_Retreat && a2->m_Type == mlat_Retreat) {
        return 0 * scale;
    }
    else if (a1->m_Type == mlat_Capture && a2->m_Type == mlat_Capture) {
        return 0 * scale;
    }
    else if (a1->m_Type == mlat_Intercept && a2->m_Type == mlat_Intercept) {
        return 0 * scale;

        ///////////////////////////////////////////////////////////////////////////////////////
    }
    else if (a1->m_Type == mlat_Defence && a2->m_Type == mlat_Attack) {
        if (!m_Team[team].IsWar() && m_Team[team].IsWaitUnion())
            return 1 * scale;

        if (m_Team[team].m_RegionNearDanger >= 0 &&
            m_Region[m_Team[team].m_RegionNearDanger].m_Danger > m_Team[team].m_Strength)
            return 1 * scale;
        return -1 * scale;
    }
    else if (a1->m_Type == mlat_Defence && a2->m_Type == mlat_Forward) {
        if (!m_Team[team].IsWar() && m_Team[team].IsWaitUnion())
            return 1 * scale;
        if (m_Team[team].IsWar())
            return -1 * scale;
        if (m_Region[a2->m_Region].m_Danger > 0) {
            if (m_Team[team].m_RegionNearDanger >= 0 &&
                m_Region[m_Team[team].m_RegionNearDanger].m_Danger > m_Team[team].m_Strength)
                return -1 * scale;
            return 1 * scale;
        }
        else {
            if (m_Team[team].m_RegionNearDanger < 0 ||
                m_Region[m_Team[team].m_RegionNearDanger].m_Danger * 0.5 > m_Team[team].m_Strength)
                return 1 * scale;
            return -1 * scale;
        }
    }
    else if (a1->m_Type == mlat_Defence && a2->m_Type == mlat_Retreat) {
        if (m_Team[team].IsWar())
            return -1 * scale;
        if (m_Team[team].m_RegionNearDanger >= 0 &&
            m_Region[m_Team[team].m_RegionNearDanger].m_Danger * 0.5f > m_Team[team].m_Strength)
            return 1 * scale;
        return -1 * scale;
    }
    else if (a1->m_Type == mlat_Defence && a2->m_Type == mlat_Capture) {
        if (!m_Team[team].IsWar() && m_Team[team].IsWaitUnion())
            return 1 * scale;

        if (m_Team[team].IsWar())
            return -1 * scale;
        if (m_Region[a2->m_Region].m_Danger > 0) {
            if (m_Team[team].m_RegionNearDanger >= 0 &&
                m_Region[m_Team[team].m_RegionNearDanger].m_Danger > m_Team[team].m_Strength)
                return 1 * scale;
            return -1 * scale;
        }
        else {
            if (m_Team[team].m_RegionNearDanger >= 0 &&
                m_Region[m_Team[team].m_RegionNearDanger].m_Danger * 0.5 > m_Team[team].m_Strength)
                return 1 * scale;
            return -1 * scale;
        }
    }
    else if (a1->m_Type == mlat_Defence && a2->m_Type == mlat_Intercept) {
        return -1 * scale;
    }
    else if (a1->m_Type == mlat_Attack && a2->m_Type == mlat_Forward) {
        return 1 * scale;
    }
    else if (a1->m_Type == mlat_Attack && a2->m_Type == mlat_Retreat) {
        if (m_Team[team].m_Strength >= m_Region[m_Team[team].m_RegionNearDanger].m_Danger * 0.8) {
            return 1 * scale;
        }
        else {
            return -1 * scale;
        }
    }
    else if (a1->m_Type == mlat_Attack && a2->m_Type == mlat_Capture) {
        return 1 * scale;
    }
    else if (a1->m_Type == mlat_Attack && a2->m_Type == mlat_Intercept) {
        return 1 * scale;
    }
    else if (a1->m_Type == mlat_Forward && a2->m_Type == mlat_Retreat) {
        if (m_Team[team].m_Strength >= m_Region[m_Team[team].m_RegionNearDanger].m_Danger * 0.8) {
            return 1 * scale;
        }
        else {
            return -1 * scale;
        }
    }
    else if (a1->m_Type == mlat_Forward && a2->m_Type == mlat_Capture) {
        return -1 * scale;
    }
    else if (a1->m_Type == mlat_Forward && a2->m_Type == mlat_Intercept) {
        return 1 * scale;
    }
    else if (a1->m_Type == mlat_Retreat && a2->m_Type == mlat_Capture) {
        return 1 * scale;
    }
    else if (a1->m_Type == mlat_Retreat && a2->m_Type == mlat_Intercept) {
        return 1 * scale;
    }
    else if (a1->m_Type == mlat_Capture && a2->m_Type == mlat_Intercept) {
        return 1 * scale;
    }

    return 0;
}

void CMatrixSideUnit::BestAction(int team) {
    if (m_Team[team].m_ActionCnt <= 1)
        return;

    int k = 0;
    for (int u = 1; u < m_Team[team].m_ActionCnt; u++) {
        if (CompareAction(team, m_Team[team].m_ActionList + k, m_Team[team].m_ActionList + u) < 0) {
            k = u;
        }
    }
    m_Team[team].m_ActionList[0] = m_Team[team].m_ActionList[k];
    m_Team[team].m_ActionCnt = 1;
}

void CMatrixSideUnit::LiveAction(int team) {
    if (m_Team[team].m_ActionCnt >= 16)
        BestAction(team);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void CMatrixSideUnit::TaktTL() {
    DTRACE();

    int i, u, x, y;
    CMatrixMapStatic *obj;
    CPoint tp;

    CMatrixRobotAI *rl[MAX_ROBOTS];  // Список роботов на карте
    int rlcnt;                       // Кол-во роботов на карте
    int team;

    if (m_LastTaktTL != 0 && (g_MatrixMap->GetTime() - m_LastTaktTL) < 10)
        return;
    m_LastTaktTL = g_MatrixMap->GetTime();

    // Для всех мест рассчитываем коэффициент вражеских объектов в зоне поражения
    if (m_LastTaktUnderfire == 0 || (g_MatrixMap->GetTime() - m_LastTaktUnderfire) > 500) {
        m_LastTaktUnderfire = g_MatrixMap->GetTime();

        SMatrixPlace *place = g_MatrixMap->m_RN.m_Place;
        for (i = 0; i < g_MatrixMap->m_RN.m_PlaceCnt; i++, place++)
            place->m_Underfire = 0;

        obj = CMatrixMapStatic::GetFirstLogic();
        while (obj) {
            if (IsLiveUnit(obj) && obj->GetSide() != m_Id) {
                tp = GetMapPos(obj);
                CRect rect(1000000000, 1000000000, -1000000000, -1000000000);
                rect.left = std::min(rect.left, tp.x);
                rect.top = std::min(rect.top, tp.y);
                rect.right = std::max(rect.right, tp.x + ROBOT_MOVECELLS_PER_SIZE);
                rect.bottom = std::max(rect.bottom, tp.y + ROBOT_MOVECELLS_PER_SIZE);

                tp.x += ROBOT_MOVECELLS_PER_SIZE >> 1;
                tp.y += ROBOT_MOVECELLS_PER_SIZE >> 1;

                int firedist = 0;
                int firedist2 = 0;
                if (obj->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
                    firedist = Float2Int(((CMatrixRobotAI *)(obj))->GetMaxFireDist() + GLOBAL_SCALE_MOVE);
                    firedist2 = Float2Int(((CMatrixRobotAI *)(obj))->GetMinFireDist() + GLOBAL_SCALE_MOVE);
                }
                else if (obj->GetObjectType() == OBJECT_TYPE_CANNON) {
                    firedist = Float2Int(((CMatrixCannon *)(obj))->GetFireRadius() + GLOBAL_SCALE_MOVE);
                    firedist2 = firedist;
                }
                firedist = firedist / int(GLOBAL_SCALE_MOVE);
                firedist2 = firedist2 / int(GLOBAL_SCALE_MOVE);

                CRect plr = g_MatrixMap->m_RN.CorrectRectPL(CRect(rect.left - firedist, rect.top - firedist,
                                                                  rect.right + firedist, rect.bottom + firedist));

                firedist *= firedist;
                firedist2 *= firedist2;

                SMatrixPlaceList *plist = g_MatrixMap->m_RN.m_PLList + plr.left + plr.top * g_MatrixMap->m_RN.m_PLSizeX;
                for (y = plr.top; y < plr.bottom; y++, plist += g_MatrixMap->m_RN.m_PLSizeX - (plr.right - plr.left)) {
                    for (x = plr.left; x < plr.right; x++, plist++) {
                        SMatrixPlace *place = g_MatrixMap->m_RN.m_Place + plist->m_Sme;
                        for (u = 0; u < plist->m_Cnt; u++, place++) {
                            int pcx = place->m_Pos.x + int(ROBOT_MOVECELLS_PER_SIZE / 2);  // Center place
                            int pcy = place->m_Pos.y + int(ROBOT_MOVECELLS_PER_SIZE / 2);

                            int d = (POW2(tp.x - pcx) + POW2(tp.y - pcy));
                            if (firedist >= d)
                                place->m_Underfire++;
                            if (firedist2 >= d)
                                place->m_Underfire++;
                        }
                    }
                }
            }
            obj = obj->GetNextLogic();
        }
    }

    // Проверяем завершился ли приказ который нельзя прервать
    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id) {
            if (GetEnv(obj)->m_OrderNoBreak && obj->AsRobot()->CanBreakOrder()) {
                GetEnv(obj)->m_OrderNoBreak = false;
                GetEnv(obj)->m_Place = -1;
            }

            if (GetEnv(obj)->m_Place >= 0 && obj->AsRobot()->CanBreakOrder() && !obj->AsRobot()->GetCaptureFactory() &&
                !IsToPlace(obj->AsRobot(),
                           GetEnv(obj)->m_Place)) {  // Если не синхронизировано место и то куда идем то место отчищаем
                GetEnv(obj)->m_Place = -1;
            }
        }
        obj = obj->GetNextLogic();
    }

    for (int g = 0; g < MAX_LOGIC_GROUP; g++) {
        if (m_LogicGroup[g].RobotsCnt() <= 0)
            continue;
        if (m_LogicGroup[g].IsWar())
            WarTL(g);
        else
            RepairTL(g);
    }

    for (int g = 0; g < MAX_LOGIC_GROUP; g++) {
        if (m_LogicGroup[g].RobotsCnt() <= 0)
            continue;

        // Создаем список роботов в группе
        rlcnt = 0;
        obj = CMatrixMapStatic::GetFirstLogic();
        while (obj) {
            if (obj->IsLiveRobot() && obj->GetSide() == m_Id && GetGroupLogic(obj) == g) {
                rl[rlcnt] = (CMatrixRobotAI *)obj;
                //                rl[rlcnt]->CalcStrength();
                rlcnt++;
            }
            obj = obj->GetNextLogic();
        }
        if (rlcnt <= 0)
            continue;

        team = rl[0]->GetTeam();
        m_LogicGroup[g].RobotsCnt(rlcnt);

        bool orderok = true;  // Приказ не изменился, данные корректны
        //        bool up_change_order=!((m_LogicGroup[g].m_Action.m_Type==m_Team[team].m_Action.m_Type) &&
        //        (m_LogicGroup[g].m_Action.m_Region==m_Team[team].m_Action.m_Region));

        // m_LogicGroup[g].m_Action=m_Team[team].m_Action;

        // Проверяем корректный ли текущий приказ
        while (true) {
            if (m_LogicGroup[g].m_Action.m_Type == mlat_None) {
                CopyOrder(team, g);
                if (m_LogicGroup[g].m_Action.m_Type != mlat_None) {
                    orderok = false;
                    continue;
                }
            }
            else if (m_LogicGroup[g].m_Action.m_Type == mlat_Capture) {
                if (!CmpOrder(team, g)) {
                    CopyOrder(team, g);
                    orderok = false;
                    continue;
                }  // Если приказ изменился, то повинуемся

                if (m_Region[m_LogicGroup[g].m_Action.m_Region].m_NeutralBuildingCnt <= 0 &&
                    m_Region[m_LogicGroup[g].m_Action.m_Region].m_EnemyBuildingCnt <=
                            0) {  // Если нечего захватывать, меняем приказ
                    if (m_LogicGroup[g].m_Action.m_Region != m_Team[team].m_RegionMass)
                        m_LogicGroup[g].m_Action.m_Type = mlat_Forward;
                    else
                        m_LogicGroup[g].m_Action.m_Type = mlat_None;
                    orderok = false;
                    break;
                }
                u = 0;
                obj = CMatrixMapStatic::GetFirstLogic();
                while (obj) {
                    if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetTeam() == team) {
                        if (obj->AsRobot()->FindOrderLikeThat(ROT_CAPTURE_FACTORY)) {
                            u++;
                            break;
                        }
                    }
                    obj = obj->GetNextLogic();
                }
                orderok = u > 0;
                if (orderok) {
                    for (i = 0; i < rlcnt; i++) {
                        if (rl[i]->FindOrderLikeThat(ROT_CAPTURE_FACTORY))
                            ;
                        else if (!PlaceInRegion(rl[i], rl[i]->GetEnv()->m_Place, m_LogicGroup[g].m_Action.m_Region)) {
                            if (CanChangePlace(rl[i])) {
                                orderok = false;
                                break;
                            }
                        }
                    }
                }
                else {
                    // Если все роботы группы заняты то приказ захвата считается успешным
                    for (i = 0; i < rlcnt; i++)
                        if (rl[i]->CanBreakOrder())
                            break;
                    if (i >= rlcnt)
                        orderok = true;

                    // Если другая команда захватывает завод и группа в регионе то приказ считаем успешным
                    if (!orderok) {
                        for (i = 0; i < rlcnt; i++) {
                            if (!PlaceInRegion(rl[i], rl[i]->GetEnv()->m_Place, m_LogicGroup[g].m_Action.m_Region))
                                break;
                        }
                        if (i >= rlcnt) {
                            obj = CMatrixMapStatic::GetFirstLogic();
                            while (obj) {
                                if (obj->IsLiveRobot() && obj->GetSide() == m_Id) {
                                    CMatrixBuilding *cf = obj->AsRobot()->GetCaptureFactory();
                                    if (cf && GetRegion(cf) == m_LogicGroup[g].m_Action.m_Region) {
                                        orderok = true;
                                        break;
                                    }
                                }
                                obj = obj->GetNextLogic();
                            }
                        }
                    }
                }
            }
            else if (m_LogicGroup[g].m_Action.m_Type == mlat_Defence) {
                if (!m_LogicGroup[g].IsWar()) {  // Если не в состоянии войны
                    for (i = 0; i < rlcnt; i++) {
                        if (rl[i]->GetEnv()->GetEnemyCnt())
                            break;
                    }
                    if (i < rlcnt && !m_LogicGroup[g].IsWar())
                        orderok = false;

                    if (orderok) {
                        if (team >= 0 && !CmpOrder(team, g)) {
                            CopyOrder(team, g);
                            orderok = false;
                            continue;
                        }  // Если приказ изменился и нет больше врагов, то повинуемся

                        for (i = 0; i < rlcnt; i++) {
                            if (CanChangePlace(rl[i]) && GetDesRegion(rl[i]) != m_LogicGroup[g].m_Action.m_Region) {
                                orderok = false;
                                break;
                            }
                        }
                    }
                }
                else {
                    for (i = 0; i < rlcnt; i++) {
                        if (rl[i]->GetEnv()->GetEnemyCnt())
                            break;
                    }
                    if (i >= rlcnt && m_LogicGroup[g].IsWar())
                        orderok = false;
                }
            }
            else if (m_LogicGroup[g].m_Action.m_Type == mlat_Attack) {
                u = 0;
                if (!m_LogicGroup[g].IsWar()) {  // Если не в состоянии войны
                    orderok = false;
                }
                else {
                    for (i = 0; i < rlcnt && orderok; i++) {
                        if (rl[i]->GetEnv()->GetEnemyCnt()) {  // Видит ли робот врага
                            u++;
                        }
                        else {
                            // Робот находится в регионе назночения или идет туда
                            if (CanChangePlace(rl[i]) && GetDesRegion(rl[i]) != m_LogicGroup[g].m_Action.m_Region) {
                                orderok = false;
                                break;
                            }
                        }
                    }
                }
                if (u == 0 &&
                    orderok) {  // Если нечего делать, но приказ остался, значит поблизости воюют - спешим им на помощь
                    obj = CMatrixMapStatic::GetFirstLogic();
                    while (obj) {
                        if (obj->IsLiveRobot() && obj->GetSide() == m_Id && GetObjTeam(obj) == team &&
                            GetGroupLogic(obj) != g) {
                            if (GetEnv(obj)->m_Target != NULL) {
                                m_LogicGroup[g].m_Action.m_Region = GetRegion(GetEnv(obj)->m_Target);
                                u = rlcnt;
                                orderok = false;
                                break;
                            }
                        }
                        obj = obj->GetNextLogic();
                    }
                }
                if (u == 0 && m_Team[team].m_Action.m_Type != mlat_Attack && !CmpOrder(team, g)) {
                    CopyOrder(team, g);
                    orderok = false;
                    continue;
                }  // Если приказ изменился и нет больше врагов, то повинуемся
            }
            else if (m_LogicGroup[g].m_Action.m_Type == mlat_Forward) {
                if (!CmpOrder(team, g)) {
                    CopyOrder(team, g);
                    orderok = false;
                    continue;
                }  // Если приказ изменился, то повинуемся
                   //                u=0;
                for (i = 0; i < rlcnt && orderok; i++) {
                    // Робот находится в регионе назночения или идет туда
                    if (CanChangePlace(rl[i]) && GetDesRegion(rl[i]) != m_LogicGroup[g].m_Action.m_Region) {
                        orderok = false;
                        break;
                    }

                    //                    if(IsInPlace(rl[i])) u++;
                }
                //                if(orderok && u==rlcnt) { // Если все пришли
                //                }
            }
            else if (m_LogicGroup[g].m_Action.m_Type == mlat_Retreat) {
                if (!CmpOrder(team, g)) {
                    CopyOrder(team, g);
                    orderok = false;
                    continue;
                }  // Если приказ изменился, то повинуемся

                for (i = 0; i < rlcnt && orderok; i++) {
                    if (CanChangePlace(rl[i]) && GetDesRegion(rl[i]) != m_LogicGroup[g].m_Action.m_Region) {
                        orderok = false;
                        break;
                    }
                }
            }
            break;
        }
        if (orderok)
            continue;

        if (m_LogicGroup[g].m_Action.m_Type == mlat_Attack && !m_LogicGroup[g].IsWar()) {
            if (!m_LogicGroup[g].IsWar()) {  // Если приказ только что установился то старые места не действительны
                for (i = 0; i < rlcnt; i++)
                    rl[i]->GetEnv()->m_Place = -1;
            }
        }

        m_LogicGroup[g].SetWar(false);

        // Назначаем новый приказ
        if (m_LogicGroup[g].m_Action.m_Type == mlat_Defence) {
            for (i = 0; i < rlcnt; i++) {
                if (rl[i]->GetEnv()->GetEnemyCnt())
                    break;
            }
            if (i < rlcnt) {
                m_LogicGroup[g].SetWar(true);
            }
            else {
                AssignPlace(g, m_LogicGroup[g].m_Action.m_Region);

                for (i = 0; i < rlcnt; i++) {
                    if (PrepareBreakOrder(rl[i])) {
                        // rl[i]->BreakAllOrders();
                        SMatrixPlace *place = GetPlacePtr(rl[i]->GetEnv()->m_Place);
                        if (place)
                            rl[i]->MoveToHigh(place->m_Pos.x, place->m_Pos.y);
                    }
                }
            }
        }
        else if (m_LogicGroup[g].m_Action.m_Type == mlat_Attack) {
            //            AssignPlace(g,m_LogicGroup[g].m_Action.m_Region);
            m_LogicGroup[g].SetWar(true);

            /*            for(i=0;i<rlcnt;i++) {
                            if(rl[i]->GetEnv()->GetEnemyCnt()<=0) { // Если робот не видит врага идем в регион
                                SMatrixPlace * place=GetPlacePtr(rl[i]->GetEnv()->m_Place);
                            }
                        }*/
        }
        else if (m_LogicGroup[g].m_Action.m_Type == mlat_Forward) {
            AssignPlace(g, m_LogicGroup[g].m_Action.m_Region);

            for (i = 0; i < rlcnt; i++) {
                if (PrepareBreakOrder(rl[i])) {
                    SMatrixPlace *place = GetPlacePtr(rl[i]->GetEnv()->m_Place);
                    // rl[i]->BreakAllOrders();
                    if (place)
                        rl[i]->MoveToHigh(place->m_Pos.x, place->m_Pos.y);
                }
            }
        }
        else if (m_LogicGroup[g].m_Action.m_Type == mlat_Retreat) {
            AssignPlace(g, m_LogicGroup[g].m_Action.m_Region);

            for (i = 0; i < rlcnt; i++) {
                if (PrepareBreakOrder(rl[i])) {
                    SMatrixPlace *place = GetPlacePtr(rl[i]->GetEnv()->m_Place);
                    if (place)
                        rl[i]->MoveToHigh(place->m_Pos.x, place->m_Pos.y);
                }
            }
        }
        else if (m_LogicGroup[g].m_Action.m_Type == mlat_Capture) {
            AssignPlace(g, m_LogicGroup[g].m_Action.m_Region);

            //            for(i=0;i<rlcnt;i++) rl[i]->BreakAllOrders();

            // Распределяем кто какие заводы захватывает
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveBuilding() && obj->GetSide() != m_Id && obj->AsBuilding()->CanBeCaptured()) {
                    i = GetRegion(GetMapPos(obj));
                    if (i == m_LogicGroup[g].m_Action.m_Region) {
                        CMatrixMapStatic *obj2 = CMatrixMapStatic::GetFirstLogic();
                        while (obj2) {
                            if (obj2->IsLiveRobot() && obj2->GetSide() == m_Id) {
                                if (obj2->AsRobot()->FindOrder(ROT_CAPTURE_FACTORY, obj))
                                    break;
                            }
                            obj2 = obj2->GetNextLogic();
                        }
                        if (obj2 == NULL) {
                            u = -1;
                            float mindist = 1e20f;
                            for (i = 0; i < rlcnt; i++) {
                                if (rl[i]->FindOrderLikeThat(ROT_CAPTURE_FACTORY))
                                    continue;
                                float d = Dist2(GetWorldPos(rl[i]), GetWorldPos(obj));
                                if (d < mindist) {
                                    mindist = d;
                                    u = i;
                                }
                            }
                            if (u >= 0) {
                                if (PrepareBreakOrder(rl[u])) {
                                    rl[u]->CaptureFactory((CMatrixBuilding *)obj);
                                }
                            }
                        }
                    }
                }
                obj = obj->GetNextLogic();
            }
            // Остальных растовляем по местам
            for (i = 0; i < rlcnt; i++) {
                if (rl[i]->FindOrderLikeThat(ROT_CAPTURE_FACTORY))
                    continue;

                if (PrepareBreakOrder(rl[i])) {
                    SMatrixPlace *place = GetPlacePtr(rl[i]->GetEnv()->m_Place);
                    if (place)
                        rl[i]->MoveToHigh(place->m_Pos.x, place->m_Pos.y);
                }
            }
        }
    }

#ifdef _DEBUG
/*    obj = CMatrixMapStatic::GetFirstLogic();
    while(obj) {
        if(obj->GetObjectType()==OBJECT_TYPE_ROBOTAI || obj->GetObjectType()==OBJECT_TYPE_CANNON) {
        }
        if(obj->IsLiveRobot() && obj->GetSide()==m_Id) {
            if(obj->AsRobot()->GetEnv()->m_Place>=0) {
                SMatrixPlace * place=g_MatrixMap->m_RN.GetPlace(obj->AsRobot()->GetEnv()->m_Place);

//
CHelper::Create(0,DWORD(place))->Cone(D3DXVECTOR3(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y,0),D3DXVECTOR3(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y,30-2.0f*i),4.0f+1.0f*i,4.0f+1.0f*i,colors[i],colors[i],5);

                D3DXVECTOR3 v1,v2,v3,v4;
                v1.x=place->m_Pos.x*GLOBAL_SCALE_MOVE; v1.y=place->m_Pos.y*GLOBAL_SCALE_MOVE;
v1.z=g_MatrixMap->GetZ(v1.x,v1.y)+1.0f; v2.x=(place->m_Pos.x+4)*GLOBAL_SCALE_MOVE;
v2.y=place->m_Pos.y*GLOBAL_SCALE_MOVE; v2.z=g_MatrixMap->GetZ(v2.x,v2.y)+1.0f;
                v3.x=(place->m_Pos.x+4)*GLOBAL_SCALE_MOVE; v3.y=(place->m_Pos.y+4)*GLOBAL_SCALE_MOVE;
v3.z=g_MatrixMap->GetZ(v3.x,v3.y)+1.0f; v4.x=(place->m_Pos.x)*GLOBAL_SCALE_MOVE;
v4.y=(place->m_Pos.y+4)*GLOBAL_SCALE_MOVE; v4.z=g_MatrixMap->GetZ(v4.x,v4.y)+1.0f;

                CHelper::DestroyByGroup(DWORD(obj)+1);
                CHelper::Create(10,DWORD(obj)+1)->Triangle(v1,v2,v3,0x8000ff00);
                CHelper::Create(10,DWORD(obj)+1)->Triangle(v1,v3,v4,0x8000ff00);
            }
            D3DXVECTOR2 v=GetWorldPos(obj);
            CHelper::DestroyByGroup(DWORD(obj)+2);
            CHelper::Create(10,DWORD(obj)+2)->Cone(D3DXVECTOR3(v.x,v.y,0),D3DXVECTOR3(v.x,v.y,40),float(obj->AsRobot()->GetMinFireDist()),float(obj->AsRobot()->GetMinFireDist()),0x80ffff00,0x80ffff00,20);
            CHelper::Create(10,DWORD(obj)+2)->Cone(D3DXVECTOR3(v.x,v.y,0),D3DXVECTOR3(v.x,v.y,40),float(obj->AsRobot()->GetMaxFireDist()),float(obj->AsRobot()->GetMaxFireDist()),0x80ff0000,0x80ff0000,20);
        }
        if(obj->IsLiveCannon()) {
            D3DXVECTOR2 v=GetWorldPos(obj);
            CHelper::DestroyByGroup(DWORD(obj)+2);
            CHelper::Create(10,DWORD(obj)+2)->Cone(D3DXVECTOR3(v.x,v.y,0),D3DXVECTOR3(v.x,v.y,40),((CMatrixCannon
*)obj)->GetSeekRadius(),((CMatrixCannon *)obj)->GetSeekRadius(),0x80ff0000,0x80ff0000,20);
        }
        obj = obj->GetNextLogic();
    }*/
/*    CHelper::DestroyByGroup(5001);
    for(i=0;i<g_MatrixMap->m_RN.m_RegionCnt;i++) {
        CPoint tp=g_MatrixMap->m_RN.m_Region[i].m_Center;
        CHelper::Create(10,5001)->Cone(D3DXVECTOR3(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y,0),D3DXVECTOR3(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y,20),1.0f,1.0f,g_MatrixMap->m_RN.m_Region[i].m_Color,g_MatrixMap->m_RN.m_Region[i].m_Color,5);
        for(u=0;u<i;u++) {
            CHelper::Create(10,5001)->Sphere(D3DXVECTOR3(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y,20.0f+8.0f*(u+1)),1.0f,3,g_MatrixMap->m_RN.m_Region[i].m_Color);
        }
        CHelper::Create(10,5001)->Cone(D3DXVECTOR3(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y,0),D3DXVECTOR3(GLOBAL_SCALE_MOVE*tp.x,GLOBAL_SCALE_MOVE*tp.y,40),GLOBAL_SCALE_MOVE*g_MatrixMap->m_RN.m_Region[i].m_RadiusPlace,GLOBAL_SCALE_MOVE*g_MatrixMap->m_RN.m_Region[i].m_RadiusPlace,g_MatrixMap->m_RN.m_Region[i].m_Color,g_MatrixMap->m_RN.m_Region[i].m_Color,20);
    }*/
#endif
}

void CMatrixSideUnit::WarTL(int group) {
    int i, u;  //,x,y;
    byte mm = 0;
    CMatrixMapStatic *obj;
    CMatrixRobotAI *rl[MAX_ROBOTS];  // Список роботов на карте
    int rlcnt = 0;                   // Кол-во роботов на карте
    bool rlokmove[MAX_ROBOTS];

    BufPrepare();

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id && GetGroupLogic(obj) == group) {
            rl[rlcnt] = (CMatrixRobotAI *)obj;
            mm |= 1 << (obj->AsRobot()->m_Unit[0].u1.s1.m_Kind - 1);
            rlcnt++;
        }
        obj = obj->GetNextLogic();
    }
    if (rlcnt < 0)
        return;

    // Находим врага для всей группы
    for (i = 0; i < rlcnt; i++) {
        if (rl[i]->GetEnv()->m_TargetAttack == rl[i])
            rl[i]->GetEnv()->m_TargetAttack = NULL;

        if (!rl[i]->GetEnv()->m_TargetAttack) {
            // Находим ближайшего незакрытого врага
            float mindist = 1e10f;
            CEnemy *enemyfind = NULL;
            CEnemy *enemy = rl[i]->GetEnv()->m_FirstEnemy;
            while (enemy) {
                if (IsLiveUnit(enemy->m_Enemy) && enemy->m_Enemy != rl[i]) {
                    float cd = Dist2(GetWorldPos(enemy->m_Enemy), GetWorldPos(rl[i]));
                    if (cd < mindist) {
                        // Проверяем не закрыт ли он своими
                        D3DXVECTOR3 des, from, dir, p;
                        float t, dist;

                        from = rl[i]->GetGeoCenter();
                        des = PointOfAim(enemy->m_Enemy);
                        dist = sqrt(POW2(from.x - des.x) + POW2(from.y - des.y) + POW2(from.z - des.z));
                        if (dist > 0.0f) {
                            t = 1.0f / dist;
                            dir.x = (des.x - from.x) * t;
                            dir.y = (des.y - from.y) * t;
                            dir.z = (des.z - from.z) * t;
                            obj = CMatrixMapStatic::GetFirstLogic();
                            while (obj) {
                                if (IsLiveUnit(obj) && obj->GetSide() == m_Id && rl[i] != obj) {
                                    p = PointOfAim(obj);

                                    if (IsIntersectSphere(p, 25.0f, from, dir, t)) {
                                        if (t >= 0.0f && t < dist)
                                            break;
                                    }
                                }
                                obj = obj->GetNextLogic();
                            }
                            if (!obj) {
                                mindist = cd;
                                enemyfind = enemy;
                            }
                        }
                    }
                }
                enemy = enemy->m_NextEnemy;
            }
            // Если не нашли открытого ищем закрытого
            if (!enemyfind) {
                enemy = rl[i]->GetEnv()->m_FirstEnemy;
                while (enemy) {
                    if (IsLiveUnit(enemy->m_Enemy) && enemy->m_Enemy != rl[i]) {
                        float cd = Dist2(GetWorldPos(enemy->m_Enemy), GetWorldPos(rl[i]));
                        if (cd < mindist) {
                            mindist = cd;
                            enemyfind = enemy;
                        }
                    }
                    enemy = enemy->m_NextEnemy;
                }
            }

            if (enemyfind) {
                rl[i]->GetEnv()->m_TargetAttack = enemyfind->m_Enemy;
                // Если новая цель пушка то меняем позицию
                if (rl[i]->GetEnv()->m_TargetAttack->IsLiveActiveCannon()) {
                    rl[i]->GetEnv()->m_Place = -1;
                }
            }
        }
    }

    // Проверяем правильно ли роботы идут
    bool moveok = true;
    for (i = 0; i < rlcnt; i++) {
        rlokmove[i] = true;

        if (!rl[i]->CanBreakOrder())
            continue;  // Пропускаем если робот не может прервать текущий приказ
        if (!rl[i]->GetEnv()->m_TargetAttack) {  // Если у робота нет цели то идем в регион назначения
            if (GetDesRegion(rl[i]) != m_LogicGroup[group].m_Action.m_Region) {
                if (!PlaceInRegion(rl[i], rl[i]->GetEnv()->m_Place, m_LogicGroup[group].m_Action.m_Region)) {
                    if (CanChangePlace(rl[i])) {
                        AssignPlace(rl[i], m_LogicGroup[group].m_Action.m_Region);

                        SMatrixPlace *place = ObjPlacePtr(rl[i]);
                        if (place && PrepareBreakOrder(rl[i])) {
                            rl[i]->MoveToHigh(place->m_Pos.x, place->m_Pos.y);
                        }
                    }
                }
            }
            continue;
        }
        if (rl[i]->GetEnv()->m_Place < 0) {
            if (CanChangePlace(rl[i])) {
                rlokmove[i] = false;
                moveok = false;
                continue;
            }
        }
        D3DXVECTOR2 tv = GetWorldPos(rl[i]->GetEnv()->m_TargetAttack);

        SMatrixPlace *place = GetPlacePtr(rl[i]->GetEnv()->m_Place);
        if (place == NULL) {
            if (CanChangePlace(rl[i])) {
                rl[i]->GetEnv()->m_Place = -1;
                rlokmove[i] = false;
                moveok = false;
                continue;
            }
        }
        else {
            float dist2 =
                    POW2(GLOBAL_SCALE_MOVE * place->m_Pos.x + GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2 - tv.x) +
                    POW2(GLOBAL_SCALE_MOVE * place->m_Pos.y + GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2 - tv.y);
            if (dist2 > POW2(rl[i]->GetMaxFireDist() - GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2)) {
                if (CanChangePlace(rl[i])) {
                    rl[i]->GetEnv()->m_Place = -1;
                    rlokmove[i] = false;
                    moveok = false;
                    continue;
                }
            }
        }
        /*        if(!IsToPlace(rl[i],rl[i]->GetEnv()->m_Place)) {
        //            IsToPlace(rl[i],rl[i]->GetEnv()->m_Place);
                    rlokmove[i]=false;
                    moveok=false;
                    continue;
                }*/
    }

    // Назначаем движение
    if (!moveok) {
        // Находим центр и радиус
        CPoint center, tp2, tp = CPoint(0, 0);
        int f = 0;
        for (i = 0; i < rlcnt; i++) {
            if (!rl[i]->GetEnv()->m_TargetAttack)
                continue;
            tp += GetMapPos(rl[i]->GetEnv()->m_TargetAttack);
            f++;
        }
        if (f <= 0)
            return;
        tp.x = tp.x / f;
        tp.y = tp.y / f;
        f = 1000000000;
        for (i = 0; i < rlcnt; i++) {
            if (!rl[i]->GetEnv()->m_TargetAttack)
                continue;
            tp2 = GetMapPos(rl[i]->GetEnv()->m_TargetAttack);
            int f2 = POW2(tp.x - tp2.x) + POW2(tp.y - tp2.y);
            if (f2 < f) {
                f = f2;
                center = tp2;
            }
        }
        int radius = 0;
        int radiusrobot = 0;
        for (i = 0; i < rlcnt; i++) {
            if (!rl[i]->GetEnv()->m_TargetAttack)
                continue;
            tp2 = GetMapPos(rl[i]->GetEnv()->m_TargetAttack);
            radiusrobot = std::max(radiusrobot, Float2Int(rl[i]->GetMaxFireDist() / GLOBAL_SCALE_MOVE));
            radius = std::max(radius, Float2Int(sqrt(float(POW2(center.x - tp2.x) + POW2(center.y - tp2.y))) +
                                           rl[i]->GetMaxFireDist() / GLOBAL_SCALE_MOVE + ROBOT_MOVECELLS_PER_SIZE));
        }

        // DM(L"RadiusSeek",std::wstring().Format(L"<i>   <i>",radius,radiusrobot).Get());

        bool cplr = true;

        // Находим место
        int listcnt;
        if (g_MatrixMap->PlaceList(mm, GetMapPos(rl[0]), center, radius, false, m_PlaceList, &listcnt) == 0) {
            for (i = 0; i < rlcnt; i++)
                rl[i]->GetEnv()->m_PlaceNotFound = g_MatrixMap->GetTime();
        }
        else {
            /*CHelper::DestroyByGroup(524234);
            D3DXVECTOR3 v1;
            v1.x=center.x*GLOBAL_SCALE_MOVE;
            v1.y=center.y*GLOBAL_SCALE_MOVE;
            v1.z=g_MatrixMap->GetZ(v1.x,v1.y);
            CHelper::Create(3000,524234)->Cone(v1,D3DXVECTOR3(v1.x,v1.y,v1.z+50.0f),3.0f,3.0f,0xffffffff,0xffffffff,3);

            for(i=0;i<listcnt;i++) {
            D3DXVECTOR3 v1;
            v1.x=g_MatrixMap->m_RN.m_Place[m_PlaceList[i]].m_Pos.x*GLOBAL_SCALE_MOVE;
            v1.y=g_MatrixMap->m_RN.m_Place[m_PlaceList[i]].m_Pos.y*GLOBAL_SCALE_MOVE;
            v1.z=g_MatrixMap->GetZ(v1.x,v1.y);
            CHelper::Create(3000,524234)->Cone(v1,D3DXVECTOR3(v1.x,v1.y,v1.z+30.0f),1.0f,1.0f,0xffffffff,0xffffffff,3);
            }*/

            // CRect rect(1000000000,1000000000,-1000000000,-1000000000);
            // int growsizemin=0;
            // int growsizemax=0;

            // for(i=0;i<rlcnt;i++) {
            //    growsizemin=std::max(growsizemin,int(rl[i]->GetMinFireDist()/GLOBAL_SCALE_MOVE));
            //    growsizemax=std::max(growsizemax,int(rl[i]->GetMaxFireDist()/GLOBAL_SCALE_MOVE));

            //    CEnemy * enemy=rl[i]->GetEnv()->m_FirstEnemy;
            //    while(enemy) {
            //        if(IsLiveUnit(enemy->m_Enemy)) {
            //            tp=GetMapPos(enemy->m_Enemy);
            //            rect.left=std::min(rect.left,tp.x);
            //            rect.top=std::min(rect.top,tp.y);
            //            rect.right=std::max(rect.right,tp.x+ROBOT_MOVECELLS_PER_SIZE);
            //            rect.bottom=std::max(rect.bottom,tp.y+ROBOT_MOVECELLS_PER_SIZE);
            //        }
            //        enemy=enemy->m_NextEnemy;
            //    }
            //}
            // if(!rect.IsEmpty()) {
            // Помечаем уже занетые места
            //            g_MatrixMap->m_RN.ActionDataPL(CRect(rect.left-growsizemax,rect.top-growsizemax,rect.right+growsizemax,rect.bottom+growsizemax),0);
            /*            for(i=0;i<rlcnt;i++) {
                            if(!rlokmove[i]) continue;
                            if(rl[i]->GetEnv()->m_Place<0) continue;
                            GetPlacePtr(rl[i]->GetEnv()->m_Place)->m_Data=1;
                        }*/

            for (i = 0; i < listcnt; i++)
                g_MatrixMap->m_RN.m_Place[m_PlaceList[i]].m_Data = 0;
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (IsLiveUnit(obj))
                    ObjPlaceData(obj, 1);
                obj = obj->GetNextLogic();
            }

            // Находим лучшее место для каждого робота
            //            CRect
            //            plr=g_MatrixMap->m_RN.CorrectRectPL(CRect(rect.left-growsizemax,rect.top-growsizemax,rect.right+growsizemax,rect.bottom+growsizemax));
            for (i = 0; i < rlcnt; i++) {
                if (rlokmove[i])
                    continue;
                if (!rl[i]->GetEnv()->m_TargetAttack)
                    continue;  // Если нет цели, то пропускаем

                bool havebomb = rl[i]->HaveBomb();

                int placebest = -1;
                float s_f1 = 0.0f;
                int s_underfire = 0;
                bool s_close = false;

                float tvx, tvy;  // To target
                int enemy_fire_dist;

                if (rl[i]->GetEnv()->m_TargetAttack->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
                    tvx = ((CMatrixRobotAI *)(rl[i]->GetEnv()->m_TargetAttack))->m_PosX - rl[i]->m_PosX;
                    tvy = ((CMatrixRobotAI *)(rl[i]->GetEnv()->m_TargetAttack))->m_PosY - rl[i]->m_PosY;
                    enemy_fire_dist =
                            Float2Int(((CMatrixRobotAI *)(rl[i]->GetEnv()->m_TargetAttack))->GetMaxFireDist());
                }
                else if (rl[i]->GetEnv()->m_TargetAttack->GetObjectType() == OBJECT_TYPE_CANNON) {
                    tvx = ((CMatrixCannon *)(rl[i]->GetEnv()->m_TargetAttack))->m_Pos.x - rl[i]->m_PosX;
                    tvy = ((CMatrixCannon *)(rl[i]->GetEnv()->m_TargetAttack))->m_Pos.y - rl[i]->m_PosY;
                    enemy_fire_dist = int(((CMatrixCannon *)(rl[i]->GetEnv()->m_TargetAttack))->GetFireRadius() +
                                          GLOBAL_SCALE_MOVE);
                }
                else
                    continue;
                float tsize2 = tvx * tvx + tvy * tvy;
                float tsize2o = 1.0f / tsize2;

                // SMatrixPlaceList * plist=g_MatrixMap->m_RN.m_PLList+plr.left+plr.top*g_MatrixMap->m_RN.m_PLSizeX;
                // for(y=plr.top;y<plr.bottom;y++,plist+=g_MatrixMap->m_RN.m_PLSizeX-(plr.right-plr.left)) {
                //    for(x=plr.left;x<plr.right;x++,plist++) {
                //        SMatrixPlace * place=g_MatrixMap->m_RN.m_Place+plist->m_Sme;
                //        for(u=0;u<plist->m_Cnt;u++,place++) {
                for (u = 0; u < listcnt; u++) {
                    int iplace = m_PlaceList[u];
                    SMatrixPlace *place = g_MatrixMap->m_RN.m_Place + iplace;

                    if (place->m_Data)
                        continue;  // Занетые места игнорируем
                    if (place->m_Move & (1 << (rl[i]->m_Unit[0].u1.s1.m_Kind - 1)))
                        continue;  // Если робот не может стоять на этом месте то пропускаем
                    if (rl[i]->GetEnv()->IsBadPlace(iplace))
                        continue;  // Плохое место пропускаем

                    float pcx = GLOBAL_SCALE_MOVE * place->m_Pos.x + (GLOBAL_SCALE_MOVE * 4.0f / 2.0f);  // Center place
                    float pcy = GLOBAL_SCALE_MOVE * place->m_Pos.y + (GLOBAL_SCALE_MOVE * 4.0f / 2.0f);

                    float pvx = pcx - rl[i]->m_PosX;  // To place
                    float pvy = pcy - rl[i]->m_PosY;

                    float k = (pvx * tvx + pvy * tvy) * tsize2o;
                    if (!havebomb && rl[i]->GetEnv()->m_TargetAttack->GetObjectType() == OBJECT_TYPE_CANNON) {
                        //                                if(k>1.5) continue; // Места за врагом не расматриваем
                    }
                    else if (!havebomb && rl[i]->GetEnv()->m_TargetAttack->GetObjectType() != OBJECT_TYPE_BUILDING) {
                        if (k > 0.95)
                            continue;  // Места за врагом не расматриваем
                    }
                    else if (!havebomb) {
                        if (k > 1.2)
                            continue;  // Места сильно за врагом не расматриваем
                    }
                    //                            if(k>0.95) continue; // Места за врагом не расматриваем
                    //                            if(k<0.0) continue; // Места за роботом игнорируем
                    float m = (-pvx * tvy + pvy * tvx) * tsize2o;
                    float distfrom2 = POW2(-m * tvy) + POW2(m * tvx);  // Дистанция отклонения
                    float distplace2 =
                            POW2(tvx - pcx /*pvx*/) + POW2(tvy - pcx /*pvy*/);  // Дистанция от места до врага
                    //                            if(distplace2>POW2(0.95*rl[i]->GetMaxFireDist()-GLOBAL_SCALE_MOVE*ROBOT_MOVECELLS_PER_SIZE/2))
                    //                            continue; // Робот должен достовать врага
                    if ((placebest < 0) || (rl[i]->GetEnv()->m_TargetAttack->GetObjectType() == OBJECT_TYPE_CANNON &&
                                            (rl[i]->GetMaxFireDist() - GLOBAL_SCALE_MOVE) > enemy_fire_dist)) {
                        if (distplace2 >
                            POW2(0.95 * rl[i]->GetMaxFireDist() - GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2))
                            continue;  // Робот должен достовать врага
                    }
                    else {
                        if (distplace2 >
                            POW2(0.95 * rl[i]->GetMinFireDist() - GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2))
                            continue;  // Робот должен достовать врага
                    }

                    if (!havebomb && rl[i]->GetEnv()->m_TargetAttack->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
                        if (distfrom2 > POW2(200 + 100))
                            continue;  // Робот не должен отклонится слишком далеко
                                       //                            } else
                        //                            if(rl[i]->GetEnv()->m_TargetAttack->GetObjectType()==OBJECT_TYPE_BUILDING)
                        //                            {
                        //                                if(distfrom2>POW2(300+100)) continue; // Робот не должен
                        //                                отклонится слишком далеко
                    }

                    int underfire = place->m_Underfire;
                    if (distplace2 <= POW2(enemy_fire_dist))
                        underfire++;

                    CMatrixMapStatic *trace_res = g_MatrixMap->Trace(
                            NULL, D3DXVECTOR3(pcx, pcy, g_MatrixMap->GetZ(pcx, pcy) + 20.0f) /*rl[i]->GetGeoCenter()*/,
                            PointOfAim(rl[i]->GetEnv()->m_TargetAttack),
                            TRACE_OBJECT | TRACE_NONOBJECT | TRACE_OBJECTSPHERE | TRACE_SKIP_INVISIBLE, rl[i]);
                    bool close =
                            (IS_TRACE_STOP_OBJECT(trace_res) && trace_res->GetObjectType() == OBJECT_TYPE_MAPOBJECT) ||
                            (trace_res == TRACE_STOP_WATER) || (trace_res == TRACE_STOP_LANDSCAPE);

                    if (placebest >= 0) {  // Если уже найдено место то выбираем наилучшее
                        if (havebomb) {
                            if (distplace2 > s_f1)
                                continue;  // Место дальше предыдущего пропускаем
                        }
                        else if (close != s_close) {
                            if (close)
                                continue;
                        }
                        else if (!underfire && s_underfire)
                            ;
                        else if (underfire && !s_underfire)
                            continue;
                        else if (underfire) {  // Если под обстрелом
                            if (underfire > s_underfire)
                                continue;  // Место более обстреливоемое пропускаем
                            if (distplace2 < s_f1)
                                continue;  // Место ближе предыдущего пропускаем
                        }
                        else {  // Если вне радиуса поражения противника
                            if (distplace2 > s_f1)
                                continue;  // Место дальше предыдущего пропускаем
                        }
                    }

                    s_close = close;
                    s_f1 = distplace2;
                    s_underfire = underfire;
                    placebest = iplace;
                }
                //                    }
                //                }

                if (placebest >= 0) {
                    cplr = false;

                    SMatrixPlace *place = GetPlacePtr(placebest);
                    place->m_Data = 1;
                    rl[i]->GetEnv()->m_Place = placebest;
                    if (PrepareBreakOrder(rl[i])) {
                        rl[i]->MoveToHigh(place->m_Pos.x, place->m_Pos.y);
                    }
                }
                else if (cplr) {
                    cplr = false;
                    if (g_MatrixMap->PlaceList(mm, GetMapPos(rl[0]), center, radiusrobot, false, m_PlaceList,
                                               &listcnt) == 0) {
                        for (u = 0; u < rlcnt; u++)
                            rl[u]->GetEnv()->m_PlaceNotFound = g_MatrixMap->GetTime();
                        break;
                    }
                    else {
                        for (u = 0; u < listcnt; u++)
                            g_MatrixMap->m_RN.m_Place[m_PlaceList[u]].m_Data = 0;
                        obj = CMatrixMapStatic::GetFirstLogic();
                        while (obj) {
                            if (IsLiveUnit(obj))
                                ObjPlaceData(obj, 1);
                            obj = obj->GetNextLogic();
                        }
                        i = -1;
                        continue;
                    }
                }
                else {  // Не нашли
                    rl[i]->GetEnv()->m_PlaceNotFound = g_MatrixMap->GetTime();

                    int iplace;
                    SMatrixPlace *place;

                    for (u = 0; u < listcnt; u++) {
                        iplace = m_PlaceList[u];
                        place = g_MatrixMap->m_RN.m_Place + iplace;

                        if (place->m_Data)
                            continue;  // Занетые места игнорируем
                        if (place->m_Move & (1 << (rl[i]->m_Unit[0].u1.s1.m_Kind - 1)))
                            continue;  // Если робот не может стоять на этом месте то пропускаем
                        break;
                    }
                    if (u < listcnt) {
                        place->m_Data = 1;
                        rl[i]->GetEnv()->m_Place = iplace;
                        if (PrepareBreakOrder(rl[i])) {
                            rl[i]->MoveToHigh(place->m_Pos.x, place->m_Pos.y);
                        }
                    }
                    else {  // Расширяем
                        if (g_MatrixMap->PlaceListGrow(mm, m_PlaceList, &listcnt, rlcnt) <= 0)
                            continue;

                        for (u = 0; u < listcnt; u++)
                            g_MatrixMap->m_RN.m_Place[m_PlaceList[u]].m_Data = 0;
                        obj = CMatrixMapStatic::GetFirstLogic();
                        while (obj) {
                            if (IsLiveUnit(obj))
                                ObjPlaceData(obj, 1);
                            obj = obj->GetNextLogic();
                        }
                    }
                }
            }
        }
    }

    // Корректируем точку выстрела
    D3DXVECTOR3 des, from, dir, p;
    float t, dist;

    int curTime = g_MatrixMap->GetTime();

    for (i = 0; i < rlcnt; i++) {
        if (rl[i]->GetEnv()->m_TargetAttack) {
            if (!IsLiveUnit(rl[i]->GetEnv()->m_TargetAttack)) {
                rl[i]->StopFire();
                continue;
            }

            des = PointOfAim(rl[i]->GetEnv()->m_TargetAttack);

            // Не стрелять из прямого оружия, если на пути к цели свои
            from = rl[i]->GetGeoCenter();
            dist = sqrt(POW2(from.x - des.x) + POW2(from.y - des.y) + POW2(from.z - des.z));

            bool fireline = rl[i]->HaveRepair() != 2;

            if (fireline && dist > 0.0f) {
                t = 1.0f / dist;
                dir.x = (des.x - from.x) * t;
                dir.y = (des.y - from.y) * t;
                dir.z = (des.z - from.z) * t;

                obj = CMatrixMapStatic::GetFirstLogic();
                while (obj) {
                    if (IsLiveUnit(obj) && obj->GetSide() == m_Id && rl[i] != obj) {
                        p = PointOfAim(obj);

                        if (IsIntersectSphere(p, 25.0f, from, dir, t)) {
                            if (t >= 0.0f && t < dist) {
                                // CHelper::DestroyByGroup(DWORD(this)+4);
                                // CHelper::Create(10,DWORD(this)+4)->Cone(from,des,1.0f,1.0f,0xffffffff,0xffffffff,3);
                                // CHelper::Create(10,DWORD(this)+4)->Sphere(D3DXVECTOR3(from.x+dir.x*t,from.y+dir.y*t,from.z+dir.z*t),2,5,0xffff0000);

                                fireline = false;
                                break;
                            }
                        }
                    }
                    obj = obj->GetNextLogic();
                }
            }

            //            des.x+=(float)g_MatrixMap->RndFloat(-5.0f,+5.0f);
            //            des.y+=(float)g_MatrixMap->RndFloat(-5.0f,+5.0f);
            //            des.z+=(float)g_MatrixMap->RndFloat(-5.0f,+5.0f);

            CInfo *env = GetEnv(rl[i]);
            if (env->m_TargetAttack != env->m_TargetLast) {
                env->m_TargetLast = env->m_TargetAttack;
                env->m_TargetChange = curTime;
            }

            if (fireline) {
                D3DXVECTOR3 v1 = rl[i]->GetGeoCenter();
                D3DXVECTOR3 v2 = PointOfAim(rl[i]->GetEnv()->m_TargetAttack);

                auto tmp = v2 - v1;
                fireline = D3DXVec3LengthSq(&tmp) <= POW2(rl[i]->GetMaxFireDist());

                if (fireline) {
                    CMatrixMapStatic *trace_res =
                            g_MatrixMap->Trace(NULL, v1, v2, TRACE_OBJECT | TRACE_NONOBJECT, rl[i]);
                    fireline = !(
                            (IS_TRACE_STOP_OBJECT(trace_res) && trace_res->GetObjectType() == OBJECT_TYPE_MAPOBJECT) ||
                            (trace_res == TRACE_STOP_WATER) || (trace_res == TRACE_STOP_LANDSCAPE));
                }
            }

            if (fireline) {
                // Если у цели голова Firewall то в нее сложнее попасть
                /*if(env->m_TargetAttack->IsRobot() && env->m_TargetAttack->AsRobot()->m_AimProtect>0) {
                    if(env->m_Target!=env->m_TargetAttack || fabs(env->m_TargetAngle)<=1.0f*1.1f*ToRad) {
                        env->m_TargetAngle=0.0f;

                        env->m_TargetAngle=std::min(30.0f*ToRad,(float)atan(env->m_TargetAttack->AsRobot()->m_AimProtect/sqrt(POW2(des.x-rl[i]->m_PosX)+POW2(des.y-rl[i]->m_PosY))));
                        if(g_MatrixMap->Rnd(0,9)<5) env->m_TargetAngle=-env->m_TargetAngle;
                    }
                    else if(env->m_TargetAngle>0) env->m_TargetAngle-=1.0f*ToRad;
                    else env->m_TargetAngle+=1.0f*ToRad;*/

                if (env->m_TargetAttack->IsRobot() && env->m_TargetAttack->AsRobot()->m_AimProtect > 0) {
                    if (env->m_Target != env->m_TargetAttack ||
                        fabs(env->m_TargetAngle) <=
                                1.3f * ToRad) {  //>=15.0f*env->m_TargetAttack->AsRobot()->m_AimProtect*ToRad) {
                        env->m_TargetAngle = 0.0f;

                        env->m_TargetAngle =
                                std::min(8.0f * ToRad, (float)g_MatrixMap->Rnd(1, 100) / 100.0f * 16.0f *
                                                          env->m_TargetAttack->AsRobot()->m_AimProtect * ToRad);
                        if (g_MatrixMap->Rnd(0, 9) < 5)
                            env->m_TargetAngle = -env->m_TargetAngle;
                    }
                    // else if(env->m_TargetAngle>0) env->m_TargetAngle+=1.0f*ToRad;
                    // else if(env->m_TargetAngle<0) env->m_TargetAngle-=1.0f*ToRad;
                    // else if(fabs(env->m_TargetAngle)>1.0f) env->m_TargetAngle*=0.7f;
                    // else env->m_TargetAngle=(2*g_MatrixMap->Rnd(0,1)-1)*ToRad;
                    else
                        env->m_TargetAngle *= 0.75f;

                    if (env->m_TargetAngle != 0.0f) {
                        float vx = des.x - rl[i]->m_PosX;
                        float vy = des.y - rl[i]->m_PosY;
                        float sa, ca;
                        SinCos(env->m_TargetAngle, &sa, &ca);
                        des.x = (ca * vx + sa * vy) + rl[i]->m_PosX;
                        des.y = (-sa * vx + ca * vy) + rl[i]->m_PosY;
                    }
                }

                env->m_Target = env->m_TargetAttack;
                env->m_LastFire = curTime;

                rl[i]->Fire(des);

                // CHelper::DestroyByGroup(DWORD(this)+6);
                // CHelper::Create(10,DWORD(this)+6)->Cone(rl[i]->GetGeoCenter(),des,1.0f,1.0f,0xffffffff,0xffffffff,3);

                // Если стоим на месте
                if (IsInPlace(rl[i])) {
                    // Если несколько врагов и в цель не попадаем в течении долгого времени, то переназначаем цель
                    if (env->m_EnemyCnt > 1 && (curTime - env->m_TargetChange) > 4000 &&
                        (curTime - env->m_LastHitTarget) > 4000) {
                        env->m_TargetAttack = NULL;
                    }
                    // Если один враг и в цель не попадаем в течении долгого времени и стоим на месте, то переназначаем
                    // место
                    if (env->m_EnemyCnt == 1 && (curTime - env->m_TargetChange) > 4000 &&
                        (curTime - env->m_LastHitTarget) > 4000) {
                        env->AddBadPlace(env->m_Place);
                        env->m_Place = -1;
                    }
                    // Если очень долго не попадаем в цель, то меняем позицию
                    if ((curTime - env->m_TargetChange) > 2000 && (curTime - env->m_LastHitTarget) > 10000) {
                        env->AddBadPlace(env->m_Place);
                        env->m_Place = -1;
                    }
                }
                else
                    env->m_TargetChange = curTime;
            }
            else {
                if (rl[i]->HaveRepair() && (g_MatrixMap->GetTime() - rl[i]->GetEnv()->m_TargetChangeRepair) >
                                                   1000) {  // Ищем робота для починки
                    D3DXVECTOR2 v, v2;

                    if (rl[i]->GetEnv()->m_Target && IsLiveUnit(rl[i]->GetEnv()->m_Target) &&
                        rl[i]->GetEnv()->m_Target->GetSide() == m_Id && rl[i]->GetEnv()->m_Target->NeedRepair()) {
                        v = GetWorldPos(rl[i]);
                        v2 = GetWorldPos(rl[i]->GetEnv()->m_Target);
                        if ((POW2(v.x - v2.x) + POW2(v.y - v2.y)) > POW2(rl[i]->GetRepairDist()))
                            rl[i]->GetEnv()->m_Target = NULL;
                    }
                    else
                        rl[i]->GetEnv()->m_Target = NULL;

                    if (!rl[i]->GetEnv()->m_Target) {
                        obj = CMatrixMapStatic::GetFirstLogic();
                        while (obj) {
                            if (obj != rl[i] && IsLiveUnit(obj) && obj->GetSide() == m_Id && obj->NeedRepair()) {
                                v = GetWorldPos(rl[i]);
                                v2 = GetWorldPos(obj);
                                if ((POW2(v.x - v2.x) + POW2(v.y - v2.y)) < POW2(rl[i]->GetRepairDist())) {
                                    rl[i]->GetEnv()->m_Target = obj;
                                    rl[i]->GetEnv()->m_TargetChangeRepair = g_MatrixMap->GetTime();
                                    break;
                                }
                            }
                            obj = obj->GetNextLogic();
                        }
                    }
                }

                if (rl[i]->GetEnv()->TargetType() == 2)
                    rl[i]->Fire(PointOfAim(rl[i]->GetEnv()->m_Target), 2);
                else
                    rl[i]->StopFire();

                // Если стоим на месте
                if (IsInPlace(rl[i])) {
                    // Если несколько врагов, а текущий долго закрыт своими, то переназначаем цель
                    if (env->m_EnemyCnt > 1 && (curTime - env->m_TargetChange) > 4000 &&
                        (curTime - env->m_LastFire) > 4000) {
                        env->m_TargetAttack = NULL;
                    }
                    // Если долго закрыт своими, то меняем позицию
                    if ((curTime - env->m_TargetChange) > 4000 && (curTime - env->m_LastFire) > 6000) {
                        if (CanChangePlace(rl[i])) {
                            env->AddBadPlace(env->m_Place);
                            env->m_Place = -1;
                        }
                    }
                }
                else
                    env->m_TargetChange = curTime;
            }
        }
    }
}

void CMatrixSideUnit::RepairTL(int group) {
    int i, rlcnt = 0;
    CMatrixMapStatic *obj;
    CMatrixRobotAI *rl[MAX_ROBOTS];  // Список роботов на карте
    D3DXVECTOR2 v, v2;

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id && GetGroupLogic(obj) == group) {
            rl[rlcnt] = obj->AsRobot();
            rlcnt++;
        }
        obj = obj->GetNextLogic();
    }
    if (rlcnt <= 0)
        return;

    // Если нет цели для починки, то ищем ее
    for (i = 0; i < rlcnt; i++) {
        if (rl[i]->GetRepairDist() <= 0)
            continue;
        if (rl[i]->GetEnv()->m_Target && rl[i]->GetEnv()->m_Target->IsLive() &&
            rl[i]->GetEnv()->m_Target->NeedRepair()) {
            v = GetWorldPos(rl[i]);
            v2 = GetWorldPos(rl[i]->GetEnv()->m_Target);
            if ((POW2(v.x - v2.x) + POW2(v.y - v2.y)) < POW2(rl[i]->GetRepairDist()))
                continue;
        }

        rl[i]->GetEnv()->m_Target = NULL;

        obj = CMatrixMapStatic::GetFirstLogic();
        while (obj) {
            if (obj != rl[i] && obj->IsLive() && obj->GetSide() == m_Id && obj->NeedRepair()) {
                v = GetWorldPos(rl[i]);
                v2 = GetWorldPos(obj);
                if ((POW2(v.x - v2.x) + POW2(v.y - v2.y)) < POW2(rl[i]->GetRepairDist())) {
                    rl[i]->GetEnv()->m_Target = obj;
                    break;
                }
            }
            obj = obj->GetNextLogic();
        }
    }

    // Корректируем точку выстрела
    for (i = 0; i < rlcnt; i++) {
        if (!rl[i]->GetEnv()->m_Target)
            continue;

        D3DXVECTOR3 des = PointOfAim(rl[i]->GetEnv()->m_Target);

        rl[i]->Fire(des, 2);
    }
}

// Назначаем место в регионе или место близкие к этому региону
void CMatrixSideUnit::AssignPlace(CMatrixRobotAI *robot, int region) {
    int i, u;
    CMatrixMapStatic *obj;
    SMatrixPlace *place;

    // Если текущее место в регионе, то оно нас устраивает
    if (PlaceInRegion(robot, robot->GetEnv()->m_Place, region))
        return;

    // В текущем регионе помечаем все места как пустые
    SMatrixRegion *uregion = g_MatrixMap->m_RN.GetRegion(region);
    for (i = 0; i < uregion->m_PlaceAllCnt; i++)
        GetPlacePtr(uregion->m_PlaceAll[i])->m_Data = 0;

    // Помечаем занятые места кроме текущего робота
    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (IsLiveUnit(obj) && obj != robot)
            ObjPlaceData(obj, 1);
        obj = obj->GetNextLogic();
    }

    // Находим пустое место
    for (u = 0; u < uregion->m_PlaceAllCnt; u++) {
        place = GetPlacePtr(uregion->m_PlaceAll[u]);
        if (place->m_Data)
            continue;
        if (!CanMove(place->m_Move, robot))
            continue;  // Если робот не может стоять на этом месте, то пропускаем

        robot->GetEnv()->m_Place = uregion->m_PlaceAll[u];
        place->m_Data = 1;
        break;
    }
}

// Назначаем места в регионе или места близкие к этому региону
void CMatrixSideUnit::AssignPlace(int group, int region) {
    float f;
    CPoint tp, tp2;
    int i, u, t;
    CMatrixMapStatic *obj;
    SMatrixPlace *place;
    byte mm = 0;

    CMatrixRobotAI *rl[MAX_ROBOTS];  // Список роботов на карте
    int rlcnt = 0;                   // Кол-во роботов на карте

    BufPrepare();

    // В текущем регионе помечаем все места как пустые
    SMatrixRegion *uregion = g_MatrixMap->m_RN.GetRegion(region);
    for (i = 0; i < uregion->m_PlaceAllCnt; i++)
        GetPlacePtr(uregion->m_PlaceAll[i])->m_Data = 0;

    // Помечаем занятые места кроме роботов текущей группы
    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot()) {
            if (obj->GetSide() == m_Id && GetGroupLogic(obj) == group) {
                rl[rlcnt] = obj->AsRobot();
                mm |= 1 << (obj->AsRobot()->m_Unit[0].u1.s1.m_Kind - 1);
                rlcnt++;
            }
            if (obj->GetSide() != m_Id || GetGroupLogic(obj) != group)
                ObjPlaceData(obj, 1);
        }
        else if (obj->IsLiveActiveCannon())
            ObjPlaceData(obj, 1);
        obj = obj->GetNextLogic();
    }
    if (rlcnt <= 0)
        return;

    SortRobotList(rl, rlcnt);

    // Рассчитываем вектор на врага
    D3DXVECTOR2 venemy;
    int cr = rl[0]->GetRegion();
    int r = FindNearRegionWithUTR(cr, NULL, 0, 4 + 32 + 64);
    if (r >= 0 && r != rl[0]->GetRegion()) {
        tp = g_MatrixMap->m_RN.m_Region[r].m_Center;
        tp2 = g_MatrixMap->m_RN.m_Region[cr].m_Center;
    }
    else if (m_LogicGroup[group].m_Team >= 0 && m_Team[m_LogicGroup[group].m_Team].m_RegionMassPrev != r) {
        tp = g_MatrixMap->m_RN.m_Region[cr].m_Center;
        tp2 = g_MatrixMap->m_RN.m_Region[m_Team[m_LogicGroup[group].m_Team].m_RegionMassPrev].m_Center;
    }
    else {
        tp = CPoint(0, 0);
        tp2 = CPoint(1, 1);
    }
    venemy.x = float(tp.x - tp2.x);
    venemy.y = float(tp.y - tp2.y);
    f = 1.0f / sqrt(POW2(venemy.x) + POW2(venemy.y));
    venemy.x *= f;
    venemy.y *= f;

    D3DXVECTOR2 vcenter;
    tp = g_MatrixMap->m_RN.m_Region[region].m_Center;
    vcenter.x = float(tp.x);
    vcenter.y = float(tp.y);

    // Создаем список пустых мест
    int listcnt = 0;
    SMatrixRegion *pregion = g_MatrixMap->m_RN.m_Region + region;
    for (i = 0; i < pregion->m_PlaceCnt; i++) {
        if (g_MatrixMap->m_RN.m_Place[pregion->m_Place[i]].m_Data)
            continue;
        m_PlaceList[listcnt] = pregion->m_Place[i];
        listcnt++;
    }
    for (i = 0; i < listcnt - 1; i++) {  // Сортеруем по дальности
        place = g_MatrixMap->m_RN.m_Place + m_PlaceList[i];
        float pr1 = venemy.x * (place->m_Pos.x - vcenter.x) + venemy.y * (place->m_Pos.y - vcenter.y);
        for (u = i + 1; u < listcnt; u++) {
            place = g_MatrixMap->m_RN.m_Place + m_PlaceList[u];
            float pr2 = venemy.x * (place->m_Pos.x - vcenter.x) + venemy.y * (place->m_Pos.y - vcenter.y);
            if (pr2 > pr1) {
                int temp = m_PlaceList[u];
                m_PlaceList[u] = m_PlaceList[i];
                m_PlaceList[i] = temp;
                pr1 = pr2;
            }
        }
    }

    // Назначаем места
    for (t = 0; t < rlcnt; t++) {
        for (i = 0; i < listcnt; i++) {
            place = g_MatrixMap->m_RN.m_Place + m_PlaceList[i];
            if (place->m_Data)
                continue;
            if (!CanMove(place->m_Move, rl[t]))
                continue;
            break;
        }
        if (i < listcnt) {
            place->m_Data = 1;
            rl[t]->GetEnv()->m_Place = m_PlaceList[i];
        }
        else {  // Если не нашли то расширяем список
            for (i = 0; i < rlcnt; i++)
                rl[i]->GetEnv()->m_PlaceNotFound = g_MatrixMap->GetTime();

            if (g_MatrixMap->PlaceListGrow(mm, m_PlaceList, &listcnt, rlcnt) <= 0)
                continue;

            for (i = 0; i < listcnt - 1; i++) {  // Сортеруем по дальности
                place = g_MatrixMap->m_RN.m_Place + m_PlaceList[i];
                float pr1 = venemy.x * (place->m_Pos.x - vcenter.x) + venemy.y * (place->m_Pos.y - vcenter.y);
                for (u = i + 1; u < listcnt; u++) {
                    place = g_MatrixMap->m_RN.m_Place + m_PlaceList[u];
                    float pr2 = venemy.x * (place->m_Pos.x - vcenter.x) + venemy.y * (place->m_Pos.y - vcenter.y);
                    if (pr2 > pr1) {
                        int temp = m_PlaceList[u];
                        m_PlaceList[u] = m_PlaceList[i];
                        m_PlaceList[i] = temp;
                        pr1 = pr2;
                    }
                }
            }
            for (i = 0; i < listcnt; i++) {
                place = g_MatrixMap->m_RN.m_Place + m_PlaceList[i];
                place->m_Data = 0;
            }
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot()) {
                    if (obj->GetSide() != m_Id || GetGroupLogic(obj) != group)
                        ObjPlaceData(obj, 1);
                }
                else if (obj->IsLiveActiveCannon())
                    ObjPlaceData(obj, 1);
                obj = obj->GetNextLogic();
            }
            t = -1;
            continue;
        }
    }

    /*    // Помечаем места которые устраивают
        t=0;
        for(i=0;i<rlcnt;i++) {
            if(rl[i]->GetEnv()->m_Place>=0 && GetPlacePtr(rl[i]->GetEnv()->m_Place)->m_Region==region &&
       GetPlacePtr(rl[i]->GetEnv()->m_Place)->m_Data==0) { GetPlacePtr(rl[i]->GetEnv()->m_Place)->m_Data=1; t++; } else
       rl[i]->GetEnv()->m_Place=-1;
        }
        if(t==rlcnt) return;

        // Назначаем остальные места
        for(i=0;i<rlcnt;i++) {
            if(rl[i]->GetEnv()->m_Place<0) {
                for(u=0;u<uregion->m_PlaceAllCnt;u++) {
                    place=GetPlacePtr(uregion->m_PlaceAll[u]);
                    if(place->m_Data) continue;
                    if(!CanMove(place->m_Move,rl[i])) continue; // Если робот не может стоять на этом месте то,
       пропускаем

                    rl[i]->GetEnv()->m_Place=uregion->m_PlaceAll[u];
                    place->m_Data=1;
                    break;
                }
                if(u>=uregion->m_PlaceAllCnt) {
                    ERROR_E;
                }
            }
        }*/
}

void CMatrixSideUnit::SortRobotList(CMatrixRobotAI **rl, int rlcnt) {
    int i, u;

    if (rlcnt <= 1)
        return;

    CMatrixRobotAI *rln[MAX_ROBOTS];
    CMatrixRobotAI *rlr[MAX_ROBOTS];
    CMatrixRobotAI *rlb[MAX_ROBOTS];
    int rlncnt = 0;
    int rlrcnt = 0;
    int rlbcnt = 0;

    // Сортируем по силе
    for (i = 0; i < rlcnt - 1; i++) {
        for (u = i + 1; u < rlcnt; u++) {
            if (rl[u]->GetStrength() < rl[i]->GetStrength()) {
                CMatrixRobotAI *temp = rl[u];
                rl[u] = rl[i];
                rl[i] = temp;
            }
        }
    }

    // Роботы с чинилкой, бомбой, и без, в разных списках
    for (i = 0; i < rlcnt; i++) {
        if (rl[i]->HaveBomb()) {
            rlb[rlbcnt] = rl[i];
            rlbcnt++;
        }
        else if (rl[i]->HaveRepair()) {
            rlr[rlrcnt] = rl[i];
            rlrcnt++;
        }
        else {
            rln[rlncnt] = rl[i];
            rlncnt++;
        }
    }

    // Обедняем списки. Каждый 2 робот с бомбой. Каждый 3 робот с чинилкой, начиная от роботы с бомбой.
    rlcnt = 0;
    int s_normal = 0, s_bomb = 0, s_repair = 0;
    int i_bomb = 0, i_repair = 0;
    while ((s_normal < rlncnt) || (s_repair < rlrcnt) || (s_bomb < rlbcnt)) {
        if (i_bomb >= 1 && s_bomb < rlbcnt) {
            rl[rlcnt] = rlb[s_bomb];
            s_bomb++;
            rlcnt++;

            i_bomb = 0;
            i_repair = 0;
        }
        else if (i_repair >= 2 && s_repair < rlrcnt) {
            rl[rlcnt] = rlr[s_repair];
            s_repair++;
            rlcnt++;

            i_repair = 0;
        }
        else if (s_normal < rlncnt) {
            rl[rlcnt] = rln[s_normal];
            s_normal++;
            rlcnt++;

            i_bomb++;
            i_repair++;
        }
        else {
            i_bomb++;
            i_repair++;
        }
    }

    /*#if (defined _DEBUG) &&  !(defined _RELDEBUG)
        std::wstring tstr;
        for(i=0;i<rlcnt;i++) {
            if(rl[i]->HaveBomb()) tstr+=L"B";
            else if(rl[i]->HaveRepair()) tstr+=L"R";
            else tstr+=L"N";
        }
        tstr+=L"    ";
        for(i=0;i<rlcnt;i++) {
            tstr+=std::wstring().Format(L"(<i>)",Float2Int(rl[i]->GetStrength()));
        }
        DM(L"SortRobot",tstr.Get());
    #endif*/
}

bool CMatrixSideUnit::PlaceInRegion(CMatrixRobotAI *robot, int place, int region) {
    int i;

    if (place < 0)
        return false;

    // Место в регионе
    SMatrixPlace *pl = GetPlacePtr(place);
    if (pl->m_Region == region)
        return true;

    // Если свободное место в регионе то возращаем false
    SMatrixRegion *uregion = g_MatrixMap->m_RN.GetRegion(region);

    for (i = 0; i < uregion->m_PlaceCnt; i++) {
        pl = GetPlacePtr(uregion->m_Place[i]);
        pl->m_Data = 0;
    }

    CMatrixMapStatic *obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (IsLiveUnit(obj) && obj != robot) {
            pl = ObjPlacePtr(obj);
            if (pl)
                pl->m_Data = 1;
        }
        obj = obj->GetNextLogic();
    }
    for (i = 0; i < uregion->m_PlaceCnt; i++) {
        pl = GetPlacePtr(uregion->m_Place[i]);
        if (pl->m_Data)
            continue;
        if (!CanMove(pl->m_Move, robot))
            continue;  // Если робот не может стоять на этом месте то пропускаем
        return false;
    }

    // Ближайшее место к региону
    for (i = uregion->m_PlaceCnt; i < uregion->m_PlaceAllCnt; i++) {
        if (uregion->m_PlaceAll[i] == place)
            return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
float CMatrixSideUnit::BuildRobotMinStrange(CMatrixBuilding *base) {
    int i;
    // Расчитываем минимальную силу робота для постройки. (Учитывается поблизости силу вражеских роботов)
    int baseregion = GetRegion(base);
    float minstrange = 0.0f;
    CMatrixMapStatic *mps = CMatrixMapStatic::GetFirstLogic();
    while (mps) {
        if (mps->IsLiveRobot()) {
            CMatrixRobotAI *robot = mps->AsRobot();

            i = GetRegion(robot);
            if (i == baseregion || g_MatrixMap->m_RN.IsNerestRegion(i, baseregion)) {
                if (robot->GetSide() == m_Id) {
                    minstrange -= robot->GetStrength();
                }
                else {
                    minstrange += robot->GetStrength();
                }
            }
        }
        else if (mps->IsLiveActiveCannon() && mps->GetSide() == m_Id) {
            minstrange -= mps->AsCannon()->GetStrength();
        }
        mps = mps->GetNextLogic();
    }
    return std::max(0.0f, minstrange * 0.7f);  // Занижаем минимальную силу
}

void CMatrixSideUnit::BuildRobot(void) {
    int i, k, r, u, cnt, lwcnt, ik, uk;
    CMatrixBuilding *base = NULL;
    float minstrange = 0;

    int basecnt = 0;
    int wr[MAX_RESOURCES];
    for (r = 0; r < MAX_RESOURCES; r++)
        wr[r] = 0;

    // Собираем информацию
    // Выбираем базу для рождения
    cnt = 0;
    CMatrixMapStatic *mps = CMatrixMapStatic::GetFirstLogic();
    while (mps) {
        if (mps->GetSide() == m_Id && mps->IsLiveBuilding() && mps->IsBase()) {
            basecnt++;
            cnt += mps->AsBuilding()->m_BS.GetItemsCnt();

            while (true) {
                if (!base) {
                    base = (CMatrixBuilding *)mps;
                    minstrange = BuildRobotMinStrange(mps->AsBuilding());
                }
                else {
                    i = GetRegion(mps);
                    k = GetRegion(base);

                    float istr = BuildRobotMinStrange(mps->AsBuilding());
                    if (istr != minstrange) {
                        if (istr < minstrange) {
                            base = (CMatrixBuilding *)mps;
                            minstrange = istr;
                        }
                        break;
                    }
                    if (m_Region[i].m_EnemyRobotDist != m_Region[k].m_EnemyRobotDist) {
                        if (m_Region[k].m_EnemyRobotDist < 0 ||
                            (m_Region[i].m_EnemyRobotDist >= 0 &&
                             m_Region[i].m_EnemyRobotDist < m_Region[k].m_EnemyRobotDist)) {
                            base = mps->AsBuilding();
                            minstrange = istr;
                        }
                        break;
                    }
                    if (m_Region[i].m_EnemyBuildingDist != m_Region[k].m_EnemyBuildingDist) {
                        if (m_Region[k].m_EnemyBuildingDist < 0 ||
                            (m_Region[i].m_EnemyBuildingDist >= 0 &&
                             m_Region[i].m_EnemyBuildingDist < m_Region[k].m_EnemyBuildingDist)) {
                            base = mps->AsBuilding();
                            minstrange = istr;
                        }
                        break;
                    }
                }
                break;
            }
        }
        else if (mps->GetSide() == m_Id && mps->IsLiveBuilding() && !mps->IsBase()) {
            if (mps->AsBuilding()->m_Kind == BUILDING_TITAN)
                wr[TITAN]++;
            else if (mps->AsBuilding()->m_Kind == BUILDING_PLASMA)
                wr[PLASMA]++;
            else if (mps->AsBuilding()->m_Kind == BUILDING_ELECTRONIC)
                wr[ELECTRONICS]++;
            else if (mps->AsBuilding()->m_Kind == BUILDING_ENERGY)
                wr[ENERGY]++;
        }
        else if (mps->GetSide() == m_Id && mps->IsLiveRobot() && mps->AsRobot()->GetTeam() >= 0) {
            cnt++;
        }
        mps = mps->GetNextLogic();
    }
    if (cnt >= GetMaxSideRobots())
        return;
    if (!base)
        return;

    if (base->m_BS.GetItemsCnt() > 0)
        return;  // Если в очереди есть робот то больше пока не строим

    // Время сколько можно подождать
    int waittime = 10000;
    if (m_Region[GetRegion(base)].m_EnemyRobotDist >= 0)
        waittime = 10000 * std::max(0, m_Region[GetRegion(base)].m_EnemyRobotDist - 1);
    else if (m_Region[GetRegion(base)].m_EnemyBuildingDist >= 0)
        waittime = 10000 * std::max(0, m_Region[GetRegion(base)].m_EnemyBuildingDist - 1);

    waittime = Float2Int(m_WaitResMul * float(std::min(waittime, 40000)));  // Нет смысла долго ждять.

    // Сколько нужно ждать до запланированного робота
    int waitend = -1;
    if (m_WaitResForBuildRobot >= 0) {
        waitend = 0;
        for (r = 0; r < MAX_RESOURCES; r++) {
            k = (SSpecialBot::m_AIRobotTypeList[m_WaitResForBuildRobot].m_Resources[r] - m_Resources[r]);
            if (k > 0) {
                if (wr[r] <= 0 && basecnt <= 0) {
                    waitend = 1000000000;
                    break;
                }
                waitend =
                        std::max(waitend, Float2Int(float(k * g_Config.m_Timings[r]) /
                                               float(wr[r] * RESOURCES_INCOME +
                                                     (RESOURCES_INCOME_BASE * GetResourceForceUp() / 100 * basecnt))));
            }
        }
    }

    // Сколько будет ресурсов, если немного подождать
    int mr = 0;
    for (r = 0; r < MAX_RESOURCES; r++)
        mr += std::min(2000, m_Resources[r]);
    mr = Float2Int(float(mr / MAX_RESOURCES) * 0.6f);

    for (r = 0; r < MAX_RESOURCES; r++) {
        //        if(m_Resources[r]<mr) {  // Не ждем то чего слишком мало
        //            wr[r]=m_Resources[r];
        //        } else {
        wr[r] = (RESOURCES_INCOME * wr[r] + (RESOURCES_INCOME_BASE * GetResourceForceUp() / 100 * basecnt)) *
                        (waittime / g_Config.m_Timings[r]) +
                m_Resources[r];
        //        }
    }

    // Создаем список роботов, которые можем построить сразу, и список, если подождем немного
    int *list = (int *)HAlloc(SSpecialBot::m_AIRobotTypeCnt * sizeof(int) * 2, g_MatrixHeap);
    int *lwait = list + SSpecialBot::m_AIRobotTypeCnt;
    cnt = 0;
    lwcnt = 0;

    bool nobomb = ((m_BuildRobotLast >= 0) && SSpecialBot::m_AIRobotTypeList[m_BuildRobotLast].m_HaveBomb) ||
                  ((m_BuildRobotLast2 >= 0) && SSpecialBot::m_AIRobotTypeList[m_BuildRobotLast2].m_HaveBomb) ||
                  ((m_BuildRobotLast3 >= 0) && SSpecialBot::m_AIRobotTypeList[m_BuildRobotLast3].m_HaveBomb);

    if (!nobomb) {
        if ((g_MatrixMap->GetTime() - m_TimeLastBomb) < m_TimeNextBomb)
            nobomb = true;
    }

    bool norepair = ((m_BuildRobotLast >= 0) && SSpecialBot::m_AIRobotTypeList[m_BuildRobotLast].m_HaveRepair) ||
                    ((m_BuildRobotLast2 >= 0) && SSpecialBot::m_AIRobotTypeList[m_BuildRobotLast2].m_HaveRepair) ||
                    ((m_BuildRobotLast3 >= 0) && SSpecialBot::m_AIRobotTypeList[m_BuildRobotLast3].m_HaveRepair);

    //    for(int bt=0;bt<=1;bt++) {
    for (i = 0; i < SSpecialBot::m_AIRobotTypeCnt; i++) {
        if (nobomb && SSpecialBot::m_AIRobotTypeList[i].m_HaveBomb)
            continue;
        if (norepair && SSpecialBot::m_AIRobotTypeList[i].m_HaveRepair)
            continue;
        if (!norepair && !SSpecialBot::m_AIRobotTypeList[i].m_HaveRepair)
            continue;
        if (SSpecialBot::m_AIRobotTypeList[i].m_Strength < minstrange)
            continue;  // Сила должна быть больше минимальной

        for (r = 0; r < MAX_RESOURCES; r++)
            if (m_Resources[r] < SSpecialBot::m_AIRobotTypeList[i].m_Resources[r])
                break;

        if (r >= MAX_RESOURCES) {
            // Одинаковых роботов не строим, так как не разнообразно и не красиво.
            if (/*bt==0 &&*/ i != m_WaitResForBuildRobot && m_BuildRobotLast >= 0 &&
                SSpecialBot::m_AIRobotTypeList[i].DifWeapon(SSpecialBot::m_AIRobotTypeList[m_BuildRobotLast]) > 0.6)
                ;
            else if (/*bt==0 &&*/ i != m_WaitResForBuildRobot && m_BuildRobotLast2 >= 0 &&
                     SSpecialBot::m_AIRobotTypeList[i].DifWeapon(SSpecialBot::m_AIRobotTypeList[m_BuildRobotLast2]) >
                             0.8)
                ;
            //                else if(/*bt==0 &&*/ i!=m_WaitResForBuildRobot && m_BuildRobotLast3>=0 &&
            //                SSpecialBot::m_AIRobotTypeList[i].DifWeapon(SSpecialBot::m_AIRobotTypeList[m_BuildRobotLast3])>0.9);
            else {
                list[cnt] = i;
                cnt++;
            }
        }
        else {
            for (r = 0; r < MAX_RESOURCES; r++)
                if (wr[r] < SSpecialBot::m_AIRobotTypeList[i].m_Resources[r])
                    break;
            if (r >= MAX_RESOURCES) {
                // Одинаковых роботов не строим, так как не разнообразно и не красиво.
                if (/*bt==0 &&*/ i != m_WaitResForBuildRobot && m_BuildRobotLast >= 0 &&
                    SSpecialBot::m_AIRobotTypeList[i].DifWeapon(SSpecialBot::m_AIRobotTypeList[m_BuildRobotLast]) > 0.6)
                    ;
                else if (/*bt==0 &&*/ i != m_WaitResForBuildRobot && m_BuildRobotLast2 >= 0 &&
                         SSpecialBot::m_AIRobotTypeList[i].DifWeapon(
                                 SSpecialBot::m_AIRobotTypeList[m_BuildRobotLast2]) > 0.8)
                    ;
                //                    else if(/*bt==0 &&*/ i!=m_WaitResForBuildRobot && m_BuildRobotLast3>=0 &&
                //                    SSpecialBot::m_AIRobotTypeList[i].DifWeapon(SSpecialBot::m_AIRobotTypeList[m_BuildRobotLast3])>0.9);
                else {
                    lwait[lwcnt] = i;
                    lwcnt++;
                }
            }
        }
    }
    //        if(cnt+lwcnt>0) break;
    //    }
    if (cnt + lwcnt <= 0) {
        HFree(list, g_MatrixHeap);
        return;
    }

    // Если дождались ожидаемого робота, то строим его
    if (m_WaitResForBuildRobot >= 0) {
        for (i = 0; i < cnt; i++)
            if (list[i] == m_WaitResForBuildRobot)
                break;
        if (i < cnt) {
            if (base->m_BS.GetItemsCnt() < 6) {
                for (r = 0; r < MAX_RESOURCES; r++)
                    m_Resources[r] = std::max(0, m_Resources[r] - SSpecialBot::m_AIRobotTypeList[list[i]].m_Resources[r]);
                m_Constructor->SetBase(base);
                m_Constructor->BuildSpecialBot(SSpecialBot::m_AIRobotTypeList[list[i]]);

                if (SSpecialBot::m_AIRobotTypeList[list[i]].m_HaveBomb)
                    m_TimeLastBomb = g_MatrixMap->GetTime();

                m_BuildRobotLast3 = m_BuildRobotLast2;
                m_BuildRobotLast2 = m_BuildRobotLast;
                m_BuildRobotLast = list[i];

                DMSide(L"BuildRobot BuildWaitRobotStrange=<f>", SSpecialBot::m_AIRobotTypeList[list[i]].m_Strength);
                HFree(list, g_MatrixHeap);
            }
            m_WaitResForBuildRobot = -1;
            return;
        }
    }

    // Если периуд слишко большой, то сбрасываем ожидаемого робота
    if (waitend >= 0 && waitend > waittime) {
        DMSide(L"BuildRobot CancelWaitRobotStrange=<f>",
               SSpecialBot::m_AIRobotTypeList[m_WaitResForBuildRobot].m_Strength);
        m_WaitResForBuildRobot = -1;
    }

    // Ожидаем пока накопятся ресурсы
    if (m_WaitResForBuildRobot >= 0) {
        HFree(list, g_MatrixHeap);
        return;
    }

    DMSide(L"BuildRobot MinStrange=<f>", minstrange);
    DMSide(L"BuildRobot ImmediatelyCnt=<i> WaitCnt=<i>", cnt, lwcnt);

    // Стоит ли ожидать
    if (cnt > 0 && lwcnt > 0 &&
        SSpecialBot::m_AIRobotTypeList[lwait[0]].m_Strength * 0.6 >
                SSpecialBot::m_AIRobotTypeList[list[0]].m_Strength) {
        // выбираем из лучших, случайно по приоритету
        for (i = 1; i < lwcnt; i++)
            if (SSpecialBot::m_AIRobotTypeList[lwait[i]].m_Strength <
                0.7 * SSpecialBot::m_AIRobotTypeList[lwait[0]].m_Strength)
                break;
        lwcnt = i;

        for (r = 0; r < MAX_RESOURCES; r++)
            if (m_Resources[r] < mr)
                break;
        if (r < MAX_RESOURCES) {  // Сортируем по ресурсам которых мало

            for (i = 0; i < lwcnt - 1; i++) {
                ik = 0;
                for (r = 0; r < MAX_RESOURCES; r++)
                    if (m_Resources[r] < mr)
                        ik += SSpecialBot::m_AIRobotTypeList[lwait[i]].m_Resources[r];

                for (u = i + 1; u < lwcnt; u++) {
                    uk = 0;
                    for (r = 0; r < MAX_RESOURCES; r++)
                        if (m_Resources[r] < mr)
                            uk += SSpecialBot::m_AIRobotTypeList[lwait[u]].m_Resources[r];

                    if (uk < ik) {
                        int temp = lwait[u];
                        lwait[u] = lwait[i];
                        lwait[i] = temp;
                        temp = uk;
                        uk = ik;
                        ik = temp;
                    }
                }
            }

            ik = 0;
            for (r = 0; r < MAX_RESOURCES; r++)
                if (m_Resources[r] < mr)
                    ik += SSpecialBot::m_AIRobotTypeList[lwait[0]].m_Resources[r];
            ik += ik / 10;
            for (i = 1; i < lwcnt; i++) {
                uk = 0;
                for (r = 0; r < MAX_RESOURCES; r++)
                    if (m_Resources[r] < mr)
                        uk += SSpecialBot::m_AIRobotTypeList[lwait[i]].m_Resources[r];
                if (uk > ik)
                    break;
            }
            lwcnt = i;
        }

        k = 0;
        for (i = 0; i < lwcnt; i++)
            k += SSpecialBot::m_AIRobotTypeList[lwait[i]].m_Pripor;
        k = g_MatrixMap->Rnd(0, k - 1);

        for (i = 0; i < lwcnt; i++) {
            k -= SSpecialBot::m_AIRobotTypeList[lwait[i]].m_Pripor;
            if (k < 0)
                break;
        }
        if (i >= lwcnt)
            ERROR_E;

        m_WaitResForBuildRobot = lwait[i];
        DMSide(L"BuildRobot WaitRobotStrange=<f>", SSpecialBot::m_AIRobotTypeList[m_WaitResForBuildRobot].m_Strength);
    }
    else if (cnt > 0) {  // Строим сразу
        // выбираем из лучших, случайно по приоритету
        for (i = 1; i < cnt; i++)
            if (SSpecialBot::m_AIRobotTypeList[list[i]].m_Strength <
                0.7 * SSpecialBot::m_AIRobotTypeList[list[0]].m_Strength)
                break;
        cnt = i;

        for (r = 0; r < MAX_RESOURCES; r++)
            if (m_Resources[r] < mr)
                break;
        if (r < MAX_RESOURCES) {  // Сортируем по ресурсам которых мало

            for (i = 0; i < cnt - 1; i++) {
                ik = 0;
                for (r = 0; r < MAX_RESOURCES; r++)
                    if (m_Resources[r] < mr)
                        ik += SSpecialBot::m_AIRobotTypeList[list[i]].m_Resources[r];

                for (u = i + 1; u < cnt; u++) {
                    uk = 0;
                    for (r = 0; r < MAX_RESOURCES; r++)
                        if (m_Resources[r] < mr)
                            uk += SSpecialBot::m_AIRobotTypeList[list[u]].m_Resources[r];

                    if (uk < ik) {
                        int temp = list[u];
                        list[u] = list[i];
                        list[i] = temp;
                        temp = uk;
                        uk = ik;
                        ik = temp;
                    }
                }
            }

            ik = 0;
            for (r = 0; r < MAX_RESOURCES; r++)
                if (m_Resources[r] < mr)
                    ik += SSpecialBot::m_AIRobotTypeList[list[0]].m_Resources[r];
            ik += ik / 10;
            for (i = 1; i < cnt; i++) {
                uk = 0;
                for (r = 0; r < MAX_RESOURCES; r++)
                    if (m_Resources[r] < mr)
                        uk += SSpecialBot::m_AIRobotTypeList[list[i]].m_Resources[r];
                if (uk > ik)
                    break;
            }
            cnt = i;
        }

        k = 0;
        for (i = 0; i < cnt; i++)
            k += SSpecialBot::m_AIRobotTypeList[list[i]].m_Pripor;
        k = g_MatrixMap->Rnd(0, k - 1);

        for (i = 0; i < cnt; i++) {
            k -= SSpecialBot::m_AIRobotTypeList[list[i]].m_Pripor;
            if (k < 0)
                break;
        }
        if (i >= cnt)
            ERROR_E;

        if (base->m_BS.GetItemsCnt() < 6) {
            for (r = 0; r < MAX_RESOURCES; r++)
                m_Resources[r] = std::max(0, m_Resources[r] - SSpecialBot::m_AIRobotTypeList[list[i]].m_Resources[r]);

            m_Constructor->SetBase(base);
            m_Constructor->BuildSpecialBot(SSpecialBot::m_AIRobotTypeList[list[i]]);

            if (SSpecialBot::m_AIRobotTypeList[list[i]].m_HaveBomb)
                m_TimeLastBomb = g_MatrixMap->GetTime();

            m_BuildRobotLast3 = m_BuildRobotLast2;
            m_BuildRobotLast2 = m_BuildRobotLast;
            m_BuildRobotLast = list[i];

            DMSide(L"BuildRobot BuildRobotStrange=<f>", SSpecialBot::m_AIRobotTypeList[list[i]].m_Strength);
        }
    }

    HFree(list, g_MatrixHeap);
}

void CMatrixSideUnit::BuildCannon(void) {
    CMatrixBuilding *building = NULL;
    int mdist = 0;

    int maxrobot = GetMaxSideRobots();
    int curcannoncnt = 0;

    CMatrixMapStatic *mps = CMatrixMapStatic::GetFirstLogic();
    while (mps) {
        if (mps->IsLiveActiveCannon() && mps->GetSide() == m_Id)
            curcannoncnt++;
        mps = mps->GetNextLogic();
    }

    if (m_RobotsCnt < maxrobot && curcannoncnt >= m_RobotsCnt)
        return;

    // Ищем здание для постройки пушки
    mps = CMatrixMapStatic::GetFirstLogic();
    while (mps) {
        if (mps->GetSide() == m_Id && mps->IsLiveBuilding()) {
            CMatrixBuilding *cb = mps->AsBuilding();

            while (true) {
                if (cb->HaveMaxTurrets())
                    break;  //  Пропускаем если все места заняты
                if (BuildRobotMinStrange(cb) > 0)
                    break;  // Пропускаем если слишком опасно

                int r = GetRegion(mps);

                if (building) {
                    if (m_Region[r].m_EnemyRobotDist >= mdist)
                        break;  // Здание наиболее близкое к фронту
                }

                mdist = m_Region[r].m_EnemyRobotDist;
                building = cb;
            }
        }
        mps = mps->GetNextLogic();
    }
    if (!building)
        return;

    int ct[CANNON_TYPE_CNT];
    memset(ct, 0, sizeof(ct));

    mps = CMatrixMapStatic::GetFirstLogic();
    while (mps) {
        if (mps->IsLiveActiveCannon() && mps->AsCannon()->m_ParentBuilding == building) {
            ASSERT(mps->AsCannon()->m_Num >= 1 && mps->AsCannon()->m_Num <= CANNON_TYPE_CNT);
            ct[mps->AsCannon()->m_Num - 1]++;
        }
        mps = mps->GetNextLogic();
    }
    mps = building->m_BS.GetTopItem();
    while (mps) {
        if (mps->GetObjectType() == OBJECT_TYPE_CANNON) {
            ASSERT(((CMatrixCannon *)mps)->m_Num >= 1 && ((CMatrixCannon *)mps)->m_Num <= CANNON_TYPE_CNT);
            ct[((CMatrixCannon *)mps)->m_Num - 1]++;
        }
        mps = mps->m_NextStackItem;
    }

    int vmin = ct[0];
    for (int i = 1; i < CANNON_TYPE_CNT; i++)
        vmin = std::min(ct[i], vmin);

    int curtype = 0;
    while (true) {
        curtype = g_MatrixMap->Rnd(0, CANNON_TYPE_CNT - 1);
        if (vmin == ct[curtype])
            break;
    }

    int r;
    for (r = 0; r < MAX_RESOURCES; r++)
        if (g_Config.m_CannonsProps[curtype].m_Resources[r] > m_Resources[r])
            break;
    if (r < MAX_RESOURCES)
        return;  // Нехватает ресурсов

    if (building->m_BS.GetItemsCnt() > 0)
        return;  // Если уже что-то строится, то не строим

    CPoint plist[MAX_PLACES];
    int plistcnt = building->GetPlacesForTurrets(plist);
    if (!plistcnt)
        return;

    for (r = 0; r < MAX_RESOURCES; r++)
        m_Resources[r] = std::max(0, m_Resources[r] - g_Config.m_CannonsProps[curtype].m_Resources[r]);

    CPoint pcannon = plist[g_MatrixMap->Rnd(0, plistcnt - 1)];

    CMatrixCannon *cannon = g_MatrixMap->StaticAdd<CMatrixCannon>(true);
    cannon->m_CurrState = CANNON_UNDER_CONSTRUCTION;
    cannon->SetInvulnerability();

    cannon->m_Pos.x = pcannon.x * GLOBAL_SCALE_MOVE;
    cannon->m_Pos.y = pcannon.y * GLOBAL_SCALE_MOVE;

    cannon->m_Angle = 0;
    for (int i = 0; i < building->m_TurretsMax; i++) {
        if (building->m_TurretsPlaces[i].m_Coord == pcannon) {
            cannon->m_Angle = building->m_TurretsPlaces[i].m_Angle;
            cannon->m_AddH = 0;  // building->m_TurretsPlaces[i].m_AddH;
            break;
        }
    }

    cannon->m_Place = g_MatrixMap->m_RN.FindInPL(pcannon);
    cannon->SetSide(m_Id);
    cannon->UnitInit(curtype + 1);

    cannon->m_ShadowType = SHADOW_STENCIL;
    cannon->m_ShadowSize = 128;

    cannon->RNeed(MR_Matrix | MR_Graph);
    cannon->m_ParentBuilding = building;
    cannon->JoinToGroup();

    cannon->m_ParentBuilding->m_TurretsHave++;
    cannon->SetHitPoint(0);
    building->m_BS.AddItem(cannon);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void CMatrixSideUnit::TaktPL(int onlygroup) {
    DTRACE();

    int i, u, t, x, y;
    byte mm = 0;
    CPoint tp;
    CMatrixMapStatic *obj;
    CMatrixBuilding *building;
    CMatrixRobotAI *robot, *robot2;
    bool orderok[MAX_LOGIC_GROUP];

    // Запускаем логику раз в 100 тактов
    if (m_LastTaktHL != 0 && (g_MatrixMap->GetTime() - m_LastTaktHL) < 100)
        return;
    m_LastTaktHL = g_MatrixMap->GetTime();

    CalcStrength();

    EscapeFromBomb();

    // Для всех мест рассчитываем коэффициент вражеских объектов в зоне поражения
    if (m_LastTaktUnderfire == 0 || (g_MatrixMap->GetTime() - m_LastTaktUnderfire) > 500) {
        m_LastTaktUnderfire = g_MatrixMap->GetTime();

        SMatrixPlace *place = g_MatrixMap->m_RN.m_Place;
        for (i = 0; i < g_MatrixMap->m_RN.m_PlaceCnt; i++, place++)
            place->m_Underfire = 0;

        obj = CMatrixMapStatic::GetFirstLogic();
        while (obj) {
            if (IsLiveUnit(obj) && obj->GetSide() != m_Id) {
                tp = GetMapPos(obj);
                CRect rect(1000000000, 1000000000, -1000000000, -1000000000);
                rect.left = std::min(rect.left, tp.x);
                rect.top = std::min(rect.top, tp.y);
                rect.right = std::max(rect.right, tp.x + ROBOT_MOVECELLS_PER_SIZE);
                rect.bottom = std::max(rect.bottom, tp.y + ROBOT_MOVECELLS_PER_SIZE);

                tp.x += ROBOT_MOVECELLS_PER_SIZE >> 1;
                tp.y += ROBOT_MOVECELLS_PER_SIZE >> 1;

                int firedist = 0;
                int firedist2 = 0;
                if (obj->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
                    firedist = Float2Int(((CMatrixRobotAI *)(obj))->GetMaxFireDist() + GLOBAL_SCALE_MOVE);
                    firedist2 = Float2Int(((CMatrixRobotAI *)(obj))->GetMinFireDist() + GLOBAL_SCALE_MOVE);
                }
                else if (obj->GetObjectType() == OBJECT_TYPE_CANNON) {
                    firedist = Float2Int(((CMatrixCannon *)(obj))->GetFireRadius() + GLOBAL_SCALE_MOVE);
                    firedist2 = firedist;
                }
                firedist = firedist / int(GLOBAL_SCALE_MOVE);
                firedist2 = firedist2 / int(GLOBAL_SCALE_MOVE);

                CRect plr = g_MatrixMap->m_RN.CorrectRectPL(CRect(rect.left - firedist, rect.top - firedist,
                                                                  rect.right + firedist, rect.bottom + firedist));

                firedist *= firedist;
                firedist2 *= firedist2;

                SMatrixPlaceList *plist = g_MatrixMap->m_RN.m_PLList + plr.left + plr.top * g_MatrixMap->m_RN.m_PLSizeX;
                for (y = plr.top; y < plr.bottom; y++, plist += g_MatrixMap->m_RN.m_PLSizeX - (plr.right - plr.left)) {
                    for (x = plr.left; x < plr.right; x++, plist++) {
                        SMatrixPlace *place = g_MatrixMap->m_RN.m_Place + plist->m_Sme;
                        for (u = 0; u < plist->m_Cnt; u++, place++) {
                            int pcx = place->m_Pos.x + int(ROBOT_MOVECELLS_PER_SIZE / 2);  // Center place
                            int pcy = place->m_Pos.y + int(ROBOT_MOVECELLS_PER_SIZE / 2);

                            int d = (POW2(tp.x - pcx) + POW2(tp.y - pcy));
                            if (firedist >= d)
                                place->m_Underfire++;
                            if (firedist2 >= d)
                                place->m_Underfire++;
                        }
                    }
                }
            }
            obj = obj->GetNextLogic();
        }
    }

    // Собираем статистику
    for (i = 0; i < MAX_LOGIC_GROUP; i++)
        m_PlayerGroup[i].m_RobotCnt = 0;

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id) {
            if (GetEnv(obj)->m_OrderNoBreak && obj->AsRobot()->CanBreakOrder()) {
                GetEnv(obj)->m_OrderNoBreak = false;
                if (obj->AsRobot()->GetGroupLogic() < 0 ||
                    m_PlayerGroup[obj->AsRobot()->GetGroupLogic()].Order() != mpo_MoveTo) {
                    GetEnv(obj)->m_Place = -1;
                    GetEnv(obj)->m_PlaceAdd = CPoint(-1, -1);
                }
            }
            if (obj->AsRobot()->GetGroupLogic() >= 0 && obj->AsRobot()->GetGroupLogic() < MAX_LOGIC_GROUP) {
                m_PlayerGroup[obj->AsRobot()->GetGroupLogic()].m_RobotCnt++;
            }
        }
        obj = obj->GetNextLogic();
    }

    // Проверяем коректна ли цель
    for (i = 0; i < MAX_LOGIC_GROUP; i++) {
        if (m_PlayerGroup[i].m_RobotCnt <= 0)
            continue;
        if (!m_PlayerGroup[i].m_Obj)
            continue;

        obj = CMatrixMapStatic::GetFirstLogic();
        while (obj) {
            if (obj == m_PlayerGroup[i].m_Obj)
                break;
            obj = obj->GetNextLogic();
        }
        if (!obj || !obj->IsLive())
            m_PlayerGroup[i].m_Obj = NULL;

        // Если цель атаки близко, то заносим во врагов
        if ((m_PlayerGroup[i].Order() == mpo_Attack || m_PlayerGroup[i].Order() == mpo_AutoAttack ||
             m_PlayerGroup[i].Order() == mpo_AutoDefence) &&
            m_PlayerGroup[i].m_Obj && m_PlayerGroup[i].m_Obj->IsLive()) {
            tp = GetMapPos(m_PlayerGroup[i].m_Obj);

            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i) {
                    robot = (CMatrixRobotAI *)obj;

                    if (tp.Dist2(GetMapPos(robot)) < POW2(30)) {
                        if (!robot->GetEnv()->SearchEnemy(m_PlayerGroup[i].m_Obj)) {
                            robot->GetEnv()->AddToList(m_PlayerGroup[i].m_Obj);
                        }
                    }
                }
                obj = obj->GetNextLogic();
            }
        }
    }

    for (i = 0; i < MAX_LOGIC_GROUP; i++) {
        if (m_PlayerGroup[i].m_RobotCnt <= 0)
            continue;
        if (m_PlayerGroup[i].Order() == mpo_Repair)
            RepairPL(i);
        else if (m_PlayerGroup[i].IsWar())
            WarPL(i);
        else if (!FirePL(i))
            RepairPL(i);
    }

    // Успешно ли выполняется текущий приказ
    for (i = 0; i < MAX_LOGIC_GROUP; i++) {
        if (onlygroup >= 0 && i != onlygroup)
            continue;
        if (m_PlayerGroup[i].m_RobotCnt <= 0)
            continue;
        orderok[i] = true;

        bool prevwar = m_PlayerGroup[i].IsWar();
        m_PlayerGroup[i].SetWar(false);

        if (m_PlayerGroup[i].Order() == mpo_Stop) {
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i) {
                    robot = (CMatrixRobotAI *)obj;
                    if (!PLIsToPlace(robot)) {
                        orderok[i] = false;
                        break;
                    }
                }
                obj = obj->GetNextLogic();
            }
        }
        else if (m_PlayerGroup[i].Order() == mpo_MoveTo) {
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i) {
                    robot = (CMatrixRobotAI *)obj;
                    if (!PLIsToPlace(robot)) {
                        if (CanChangePlace(robot)) {
                            orderok[i] = false;
                            break;
                        }
                    }
                }
                obj = obj->GetNextLogic();
            }
        }
        else if (m_PlayerGroup[i].Order() == mpo_Patrol) {
            t = 1;
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i) {
                    robot = obj->AsRobot();
                    if (robot->GetEnv()->GetEnemyCnt()) {
                        orderok[i] = true;
                        m_PlayerGroup[i].SetWar(true);
                        if (!prevwar)
                            PGPlaceClear(i);
                        t = 0;
                        break;
                    }
                    if (!PLIsToPlace(robot)) {
                        if (CanChangePlace(robot)) {
                            orderok[i] = false;
                            break;
                        }
                    }
                    if (!robot->PLIsInPlace()) {
                        t = 0;
                    }
                }
                obj = obj->GetNextLogic();
            }
            if (t) {
                orderok[i] = false;
                m_PlayerGroup[i].SetPatrolReturn(!m_PlayerGroup[i].IsPatrolReturn());
            }
        }
        else if (m_PlayerGroup[i].Order() == mpo_Repair) {
            if (m_PlayerGroup[i].m_Obj == NULL || !m_PlayerGroup[i].m_Obj->NeedRepair()) {
                PGOrderStop(i);
            }
        }
        else if (m_PlayerGroup[i].Order() == mpo_Capture) {
            u = 0;
            t = 0;
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id) {
                    building = obj->AsRobot()->GetCaptureFactory();
                    if (building && building == m_PlayerGroup[i].m_Obj) {
                        u++;
                        break;
                    }
                }
                if (m_PlayerGroup[i].m_Obj == obj && obj->IsLiveBuilding() && obj->GetSide() != m_Id)
                    t = 1;
                obj = obj->GetNextLogic();
            }
            if (!t) {  // Нечего захватывать
                PGOrderStop(i);
                continue;
            }
            orderok[i] = (u > 0);
            // Если приказ захватить базу, но есть пушки, то атаковать пушки
            if (m_PlayerGroup[i].m_Obj->IsBase() && m_PlayerGroup[i].m_Obj->AsBuilding()->m_TurretsHave) {
                obj = CMatrixMapStatic::GetFirstLogic();
                while (obj) {
                    if (obj->IsLiveActiveCannon() && obj->AsCannon()->m_ParentBuilding == m_PlayerGroup[i].m_Obj) {
                        CMatrixMapStatic *obj2 = CMatrixMapStatic::GetFirstLogic();
                        while (obj2) {
                            if (obj2->IsLiveRobot() && obj2->GetSide() == m_Id &&
                                obj2->AsRobot()->GetGroupLogic() == i) {
                                robot = obj2->AsRobot();
                                if (robot->GetEnv()->SearchEnemy(obj))
                                    break;
                            }
                            obj2 = obj2->GetNextLogic();
                        }
                        if (obj2)
                            break;
                    }
                    obj = obj->GetNextLogic();
                }
                if (obj) {
                    CMatrixMapStatic *obj2 = CMatrixMapStatic::GetFirstLogic();
                    while (obj2) {
                        if (obj2->IsLiveRobot() && obj2->GetSide() == m_Id &&
                            ((CMatrixRobotAI *)obj2)->GetGroupLogic() == i) {
                            robot = (CMatrixRobotAI *)obj2;
                            if (robot->GetCaptureFactory() && robot->CanBreakOrder()) {
                                robot->BreakAllOrders();
                            }
                        }
                        obj2 = obj2->GetNextLogic();
                    }
                    m_PlayerGroup[i].SetWar(true);

                    if (FLAG(g_MatrixMap->m_Flags, MMFLAG_ENABLE_CAPTURE_FUCKOFF_SOUND)) {
                        CSound::Play(S_ORDER_CAPTURE_FUCK_OFF);
                        RESETFLAG(g_MatrixMap->m_Flags, MMFLAG_ENABLE_CAPTURE_FUCKOFF_SOUND);
                    }

                    if (!prevwar)
                        PGPlaceClear(i);
                    orderok[i] = true;
                    continue;
                }
            }
            if (orderok[i]) {  // Проверяем у всех ли правильно назначено место
                obj = CMatrixMapStatic::GetFirstLogic();
                while (obj) {
                    if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i) {
                        robot = (CMatrixRobotAI *)obj;

                        if (robot->GetEnv()->m_Place < 0 && robot->GetEnv()->m_PlaceAdd.x < 0)
                            break;
                    }
                    obj = obj->GetNextLogic();
                }
                if (obj) {
                    if (CanChangePlace(robot)) {
                        orderok[i] = false;
                    }
                }
            }
        }
        else if (m_PlayerGroup[i].Order() == mpo_Attack) {
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i) {
                    robot = (CMatrixRobotAI *)obj;
                    if (m_PlayerGroup[i].m_Obj && robot->GetEnv()->SearchEnemy(m_PlayerGroup[i].m_Obj)) {
                        //                        if(robot->GetEnv()->m_Target!=m_PlayerGroup[i].m_Obj)
                        //                        robot->GetEnv()->m_Target=m_PlayerGroup[i].m_Obj;
                        orderok[i] = true;
                        m_PlayerGroup[i].SetWar(true);
                        if (!prevwar)
                            PGPlaceClear(i);
                        break;
                    }
                    else if (!m_PlayerGroup[i].m_Obj && robot->GetEnv()->GetEnemyCnt()) {
                        orderok[i] = true;
                        m_PlayerGroup[i].SetWar(true);
                        if (!prevwar)
                            PGPlaceClear(i);
                        break;
                    }
                    if (!PLIsToPlace(robot)) {
                        if (CanChangePlace(robot)) {
                            orderok[i] = false;
                            break;
                        }
                    }
                }
                obj = obj->GetNextLogic();
            }
            if (!m_PlayerGroup[i].IsWar() &&
                prevwar) {  // Если до этого находились в состоянии войны, то заново назначаем маршрут
                orderok[i] = false;
                continue;
            }
            if (!m_PlayerGroup[i].IsWar() && orderok[i]) {  // Если нет войны, и правильно идем по места, проверяем,
                                                            // сильно ли изменила цель свою позицию
                tp = m_PlayerGroup[i].m_To;
                if (m_PlayerGroup[i].m_Obj)
                    tp = GetMapPos(m_PlayerGroup[i].m_Obj);

                obj = CMatrixMapStatic::GetFirstLogic();
                while (obj) {
                    if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i) {
                        robot = (CMatrixRobotAI *)obj;

                        if (tp.Dist2(PLPlacePos(robot)) < POW2(15))
                            break;
                    }
                    obj = obj->GetNextLogic();
                }
                if (!obj) {
                    orderok[i] = false;
                    continue;
                }
            }
        }
        else if (m_PlayerGroup[i].Order() == mpo_Bomb) {
            t = 0;
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i) {
                    robot = (CMatrixRobotAI *)obj;
                    if (robot->HaveBomb()) {
                        t++;
                        auto tmp = GetWorldPos(robot) - GetWorldPos(m_PlayerGroup[i].m_Obj);
                        if (robot->PLIsInPlace()) {
                            robot->BigBoom();
                            t--;
                        }
                        else if (PLPlacePos(robot).Dist2(GetMapPos(robot)) < POW2(2)) {
                            robot->BigBoom();
                            t--;
                        }
                        else if (m_PlayerGroup[i].m_Obj && m_PlayerGroup[i].m_Obj->IsLive() &&
                                 D3DXVec2LengthSq(&tmp) < POW2(150)) {
                            robot->BigBoom();
                            t--;
                        }
                    }
                    if (!PLIsToPlace(robot)) {
                        if (CanChangePlace(robot)) {
                            orderok[i] = false;
                        }
                    }
                }
                obj = obj->GetNextLogic();
            }
            if (t <= 0) {
                PGOrderStop(i);
                continue;
            }
            if (orderok[i]) {  // и правильно идем по места, проверяем, сильно ли изменила цель свою позицию
                tp = m_PlayerGroup[i].m_To;
                if (m_PlayerGroup[i].m_Obj)
                    tp = GetMapPos(m_PlayerGroup[i].m_Obj);

                obj = CMatrixMapStatic::GetFirstLogic();
                while (obj) {
                    if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i) {
                        robot = (CMatrixRobotAI *)obj;

                        if (tp.Dist2(PLPlacePos(robot)) < POW2(10))
                            break;
                    }
                    obj = obj->GetNextLogic();
                }
                if (!obj) {
                    orderok[i] = false;
                    continue;
                }
            }
        }
        else if (m_PlayerGroup[i].Order() == mpo_AutoCapture) {
            float strange = 0.0f;
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i) {
                    strange += obj->AsRobot()->GetStrength();
                }
                obj = obj->GetNextLogic();
            }

            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i) {
                    robot = (CMatrixRobotAI *)obj;
                    if (robot->GetEnv()->GetEnemyCnt()) {
                        orderok[i] = true;
                        if (strange >= 1.0f)
                            m_PlayerGroup[i].SetWar(true);
                        if (!prevwar)
                            PGPlaceClear(i);
                        break;
                    }
                }
                obj = obj->GetNextLogic();
            }
            if (m_PlayerGroup[i].IsWar())
                continue;

            if (!m_PlayerGroup[i].m_Obj || !m_PlayerGroup[i].m_Obj->IsLiveBuilding() ||
                m_PlayerGroup[i].m_Obj->GetSide() == m_Id) {
                m_PlayerGroup[i].m_Obj = NULL;
                orderok[i] = false;
                continue;
            }
            u = 0;
            t = 0;
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id) {
                    building = obj->AsRobot()->GetCaptureFactory();
                    if (building && building == m_PlayerGroup[i].m_Obj) {
                        u++;
                        break;
                    }
                }
                if (m_PlayerGroup[i].m_Obj == obj && obj->IsLiveBuilding() && obj->GetSide() != m_Id)
                    t = 1;
                obj = obj->GetNextLogic();
            }
            if (!t) {  // Нечего захватывать
                orderok[i] = false;
                continue;
            }
            orderok[i] = (u > 0);
            if (orderok[i]) {  // Проверяем у всех ли правильно назначено место
                obj = CMatrixMapStatic::GetFirstLogic();
                while (obj) {
                    if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i) {
                        robot = (CMatrixRobotAI *)obj;

                        if (robot->GetEnv()->m_Place < 0 && robot->GetEnv()->m_PlaceAdd.x < 0)
                            break;
                    }
                    obj = obj->GetNextLogic();
                }
                if (obj) {
                    if (CanChangePlace(robot)) {
                        orderok[i] = false;
                    }
                }
            }
        }
        else if (m_PlayerGroup[i].Order() == mpo_AutoAttack) {
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i) {
                    robot = (CMatrixRobotAI *)obj;
                    if (robot->GetEnv()->GetEnemyCnt()) {
                        orderok[i] = true;
                        m_PlayerGroup[i].SetWar(true);
                        if (!prevwar)
                            PGPlaceClear(i);
                        break;
                    }
                    if (!PLIsToPlace(robot)) {
                        if (CanChangePlace(robot)) {
                            orderok[i] = false;
                            break;
                        }
                    }
                }
                obj = obj->GetNextLogic();
            }
            if (m_PlayerGroup[i].IsWar())
                continue;

            if (!m_PlayerGroup[i].m_Obj || !m_PlayerGroup[i].m_Obj->IsLive()) {
                m_PlayerGroup[i].m_Obj = NULL;
                orderok[i] = false;
                continue;
            }
            if (!m_PlayerGroup[i].IsWar() && orderok[i]) {  // Если нет войны, и правильно идем по места, проверяем,
                                                            // сильно ли изменила цель свою позицию
                tp = m_PlayerGroup[i].m_To;
                if (m_PlayerGroup[i].m_Obj)
                    tp = GetMapPos(m_PlayerGroup[i].m_Obj);

                obj = CMatrixMapStatic::GetFirstLogic();
                while (obj) {
                    if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i) {
                        robot = (CMatrixRobotAI *)obj;

                        if (tp.Dist2(PLPlacePos(robot)) < POW2(15))
                            break;
                    }
                    obj = obj->GetNextLogic();
                }
                if (!obj) {
                    orderok[i] = false;
                    continue;
                }
            }
        }
        else if (m_PlayerGroup[i].Order() == mpo_AutoDefence) {
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i) {
                    robot = (CMatrixRobotAI *)obj;
                    if (robot->GetEnv()->GetEnemyCnt()) {
                        orderok[i] = true;
                        m_PlayerGroup[i].SetWar(true);
                        if (!prevwar)
                            PGPlaceClear(i);
                        break;
                    }
                    if (!PLIsToPlace(robot)) {
                        orderok[i] = false;
                        break;
                    }
                }
                obj = obj->GetNextLogic();
            }
            if (m_PlayerGroup[i].IsWar())
                continue;

            if (!m_PlayerGroup[i].m_Obj || !m_PlayerGroup[i].m_Obj->IsLiveRobot() || m_PlayerGroup[i].m_Region < 0) {
                m_PlayerGroup[i].m_Obj = NULL;
                orderok[i] = false;
                continue;
            }
            if (!m_PlayerGroup[i].IsWar() && orderok[i]) {  // Если нет войны, и правильно идем по места, проверяем,
                                                            // идет ли цель в регион назначения.
                if (!m_PlayerGroup[i].m_Obj->AsRobot()->GetMoveToCoords(tp))
                    tp = GetMapPos(m_PlayerGroup[i].m_Obj);
                int reg = g_MatrixMap->GetRegion(tp);

                if (m_PlayerGroup[i].m_Region != reg) {
                    if (CanChangePlace(robot)) {
                        m_PlayerGroup[i].m_Obj = NULL;
                        orderok[i] = false;
                        continue;
                    }
                }
            }
        }
    }

    // Применяем приказ
    for (i = 0; i < MAX_LOGIC_GROUP; i++) {
        if (onlygroup >= 0 && i != onlygroup)
            continue;
        if (m_PlayerGroup[i].m_RobotCnt <= 0)
            continue;
        if (orderok[i])
            continue;

        if (m_PlayerGroup[i].Order() == mpo_Stop) {
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i &&
                    !obj->AsRobot()->IsDisableManual()) {
                    robot = (CMatrixRobotAI *)obj;

                    tp = PLPlacePos(robot);
                    if (tp.x >= 0 && PrepareBreakOrder(robot))
                        robot->MoveToHigh(tp.x, tp.y);
                }
                obj = obj->GetNextLogic();
            }
            if (m_PlayerGroup[i].IsShowPlace())
                PGShowPlace(i);
        }
        else if (m_PlayerGroup[i].Order() == mpo_MoveTo) {
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i &&
                    !obj->AsRobot()->IsDisableManual()) {
                    robot = (CMatrixRobotAI *)obj;

                    tp = PLPlacePos(robot);
                    if (tp.x >= 0 && PrepareBreakOrder(robot))
                        robot->MoveToHigh(tp.x, tp.y);
                }
                obj = obj->GetNextLogic();
            }
            if (m_PlayerGroup[i].IsShowPlace())
                PGShowPlace(i);
        }
        else if (m_PlayerGroup[i].Order() == mpo_Patrol) {
            if (!m_PlayerGroup[i].IsPatrolReturn())
                PGAssignPlace(i, m_PlayerGroup[i].m_From);
            else
                PGAssignPlace(i, m_PlayerGroup[i].m_To);

            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i &&
                    !obj->AsRobot()->IsDisableManual()) {
                    robot = (CMatrixRobotAI *)obj;

                    tp = PLPlacePos(robot);
                    if (tp.x >= 0 && PrepareBreakOrder(robot))
                        robot->MoveToHigh(tp.x, tp.y);
                }
                obj = obj->GetNextLogic();
            }
            if (m_PlayerGroup[i].IsShowPlace())
                PGShowPlace(i);
        }
        else if (m_PlayerGroup[i].Order() == mpo_Capture) {
            robot2 = NULL;
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && !obj->AsRobot()->IsDisableManual()) {
                    // if(obj->IsLiveRobot() && obj->GetSide()==m_Id && obj->AsRobot()->GetGroupLogic()==i &&
                    // !obj->AsRobot()->IsDisableManual()) {
                    robot = (CMatrixRobotAI *)obj;
                    building = robot->GetCaptureFactory();
                    if (building && building == m_PlayerGroup[i].m_Obj) {
                        robot2 = robot;
                        break;
                    }
                }
                obj = obj->GetNextLogic();
            }
            if (robot2 == NULL) {
                t = 1000000000;
                obj = CMatrixMapStatic::GetFirstLogic();
                while (obj) {
                    // if(obj->IsLiveRobot() && obj->GetSide()==m_Id && obj->AsRobot()->GetGroupLogic()==i) {
                    if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i &&
                        !obj->AsRobot()->IsDisableManual()) {
                        robot = (CMatrixRobotAI *)obj;

                        if (robot->CanBreakOrder()) {
                            u = GetMapPos(robot).Dist2(GetMapPos(m_PlayerGroup[i].m_Obj));
                            if (u < t) {
                                t = u;
                                robot2 = robot;
                            }
                        }
                    }
                    obj = obj->GetNextLogic();
                }
                if (robot2) {
                    if (PrepareBreakOrder(robot2)) {
                        SoundCapture(i);
                        robot2->CaptureFactory((CMatrixBuilding *)m_PlayerGroup[i].m_Obj);
                    }
                }
            }
            PGAssignPlace(i, GetMapPos(m_PlayerGroup[i].m_Obj));
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                // if(obj->IsLiveRobot() && obj->GetSide()==m_Id && obj->AsRobot()->GetGroupLogic()==i && obj!=robot2) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i &&
                    obj != robot2 && !obj->AsRobot()->IsDisableManual()) {
                    robot = (CMatrixRobotAI *)obj;

                    tp = PLPlacePos(robot);
                    if (tp.x >= 0 && PrepareBreakOrder(robot))
                        robot->MoveToHigh(tp.x, tp.y);
                }
                obj = obj->GetNextLogic();
            }
            if (m_PlayerGroup[i].IsShowPlace())
                PGShowPlace(i);
        }
        else if (m_PlayerGroup[i].Order() == mpo_Attack) {
            if (m_PlayerGroup[i].m_Obj)
                PGAssignPlacePlayer(i, GetMapPos(m_PlayerGroup[i].m_Obj));
            else
                PGAssignPlacePlayer(i, m_PlayerGroup[i].m_To);

            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                // if(obj->IsLiveRobot() && obj->GetSide()==m_Id && obj->AsRobot()->GetGroupLogic()==i) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i &&
                    !obj->AsRobot()->IsDisableManual()) {
                    robot = (CMatrixRobotAI *)obj;

                    tp = PLPlacePos(robot);
                    if (tp.x >= 0 && PrepareBreakOrder(robot))
                        robot->MoveToHigh(tp.x, tp.y);
                }
                obj = obj->GetNextLogic();
            }

            if (m_PlayerGroup[i].IsShowPlace())
                PGShowPlace(i);
        }
        else if (m_PlayerGroup[i].Order() == mpo_Bomb) {
            if (m_PlayerGroup[i].m_Obj)
                PGAssignPlacePlayer(i, GetMapPos(m_PlayerGroup[i].m_Obj));
            else
                PGAssignPlacePlayer(i, m_PlayerGroup[i].m_To);

            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                // if(obj->IsLiveRobot() && obj->GetSide()==m_Id && obj->AsRobot()->GetGroupLogic()==i) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i &&
                    !obj->AsRobot()->IsDisableManual()) {
                    robot = (CMatrixRobotAI *)obj;

                    tp = PLPlacePos(robot);
                    if (tp.x >= 0 && PrepareBreakOrder(robot))
                        robot->MoveToHigh(tp.x, tp.y);
                }
                obj = obj->GetNextLogic();
            }

            if (m_PlayerGroup[i].IsShowPlace())
                PGShowPlace(i);
        }
        else if (m_PlayerGroup[i].Order() == mpo_AutoCapture) {
            if (!m_PlayerGroup[i].m_Obj)
                PGFindCaptureFactory(i);

            if (!m_PlayerGroup[i].m_Obj) {
                PGOrderStop(i);
                continue;
            }

            robot2 = NULL;
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id) {
                    robot = (CMatrixRobotAI *)obj;
                    building = robot->GetCaptureFactory();
                    if (building && building == m_PlayerGroup[i].m_Obj) {
                        robot2 = robot;
                        break;
                    }
                }
                obj = obj->GetNextLogic();
            }
            if (robot2 == NULL) {
                t = 1000000000;
                obj = CMatrixMapStatic::GetFirstLogic();
                while (obj) {
                    // if(obj->IsLiveRobot() && obj->GetSide()==m_Id && obj->AsRobot()->GetGroupLogic()==i) {
                    if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i &&
                        !obj->AsRobot()->IsDisableManual()) {
                        robot = (CMatrixRobotAI *)obj;

                        if (robot->CanBreakOrder()) {
                            u = GetMapPos(robot).Dist2(GetMapPos(m_PlayerGroup[i].m_Obj));
                            if (u < t) {
                                t = u;
                                robot2 = robot;
                            }
                        }
                    }
                    obj = obj->GetNextLogic();
                }
                if (robot2) {
                    if (PrepareBreakOrder(robot2)) {
                        SoundCapture(i);
                        robot2->CaptureFactory((CMatrixBuilding *)m_PlayerGroup[i].m_Obj);
                    }
                }
            }
            PGAssignPlace(i, GetMapPos(m_PlayerGroup[i].m_Obj));
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                // if(obj->IsLiveRobot() && obj->GetSide()==m_Id && obj->AsRobot()->GetGroupLogic()==i && obj!=robot2) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i &&
                    obj != robot2 && !obj->AsRobot()->IsDisableManual()) {
                    robot = (CMatrixRobotAI *)obj;

                    tp = PLPlacePos(robot);
                    if (tp.x >= 0 && PrepareBreakOrder(robot))
                        robot->MoveToHigh(tp.x, tp.y);
                }
                obj = obj->GetNextLogic();
            }
        }
        else if (m_PlayerGroup[i].Order() == mpo_AutoAttack) {
            if (!m_PlayerGroup[i].m_Obj)
                PGFindAttackTarget(i);

            if (!m_PlayerGroup[i].m_Obj) {
                continue;
            }

            if (m_PlayerGroup[i].m_Obj)
                PGAssignPlacePlayer(i, GetMapPos(m_PlayerGroup[i].m_Obj));
            else
                PGAssignPlacePlayer(i, m_PlayerGroup[i].m_To);

            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                // if(obj->IsLiveRobot() && obj->GetSide()==m_Id && obj->AsRobot()->GetGroupLogic()==i) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i &&
                    !obj->AsRobot()->IsDisableManual()) {
                    robot = (CMatrixRobotAI *)obj;

                    tp = PLPlacePos(robot);
                    if (tp.x >= 0 && PrepareBreakOrder(robot))
                        robot->MoveToHigh(tp.x, tp.y);
                }
                obj = obj->GetNextLogic();
            }
        }
        else if (m_PlayerGroup[i].Order() == mpo_AutoDefence) {
            if (!m_PlayerGroup[i].m_Obj)
                PGFindDefenceTarget(i);

            if (!m_PlayerGroup[i].m_Obj || m_PlayerGroup[i].m_Region < 0) {
                continue;
            }

            PGAssignPlace(i, g_MatrixMap->m_RN.GetRegion(m_PlayerGroup[i].m_Region)->m_Center);

            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                // if(obj->IsLiveRobot() && obj->GetSide()==m_Id && obj->AsRobot()->GetGroupLogic()==i) {
                if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == i &&
                    !obj->AsRobot()->IsDisableManual()) {
                    robot = (CMatrixRobotAI *)obj;

                    tp = PLPlacePos(robot);
                    if (tp.x >= 0 && PrepareBreakOrder(robot))
                        robot->MoveToHigh(tp.x, tp.y);
                }
                obj = obj->GetNextLogic();
            }
        }
    }

#if (defined _DEBUG) && !(defined _RELDEBUG) && !(defined _DISABLE_AI_HELPERS)
    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id) {
            tp = PLPlacePos(obj->AsRobot());
            if (tp.x >= 0) {
                D3DXVECTOR3 v1, v2, v3, v4;
                v1.x = tp.x * GLOBAL_SCALE_MOVE;
                v1.y = tp.y * GLOBAL_SCALE_MOVE;
                v1.z = g_MatrixMap->GetZ(v1.x, v1.y) + 1.0f;
                v2.x = (tp.x + 4) * GLOBAL_SCALE_MOVE;
                v2.y = tp.y * GLOBAL_SCALE_MOVE;
                v2.z = g_MatrixMap->GetZ(v2.x, v2.y) + 1.0f;
                v3.x = (tp.x + 4) * GLOBAL_SCALE_MOVE;
                v3.y = (tp.y + 4) * GLOBAL_SCALE_MOVE;
                v3.z = g_MatrixMap->GetZ(v3.x, v3.y) + 1.0f;
                v4.x = (tp.x) * GLOBAL_SCALE_MOVE;
                v4.y = (tp.y + 4) * GLOBAL_SCALE_MOVE;
                v4.z = g_MatrixMap->GetZ(v4.x, v4.y) + 1.0f;

                CHelper::DestroyByGroup(DWORD(obj) + 1);
                CHelper::Create(10, DWORD(obj) + 1)->Triangle(v1, v2, v3, 0x8000ff00);
                CHelper::Create(10, DWORD(obj) + 1)->Triangle(v1, v3, v4, 0x8000ff00);
            }
            //            D3DXVECTOR2 v=GetWorldPos(obj);
            //            CHelper::DestroyByGroup(DWORD(obj)+2);
            //            CHelper::Create(10,DWORD(obj)+2)->Cone(D3DXVECTOR3(v.x,v.y,0),D3DXVECTOR3(v.x,v.y,40),float(obj->AsRobot()->GetMinFireDist()),float(obj->AsRobot()->GetMinFireDist()),0x80ffff00,0x80ffff00,20);
            //            CHelper::Create(10,DWORD(obj)+2)->Cone(D3DXVECTOR3(v.x,v.y,0),D3DXVECTOR3(v.x,v.y,40),float(obj->AsRobot()->GetMaxFireDist()),float(obj->AsRobot()->GetMaxFireDist()),0x80ff0000,0x80ff0000,20);
        }
        obj = obj->GetNextLogic();
    }
#endif
}

void CMatrixSideUnit::RepairPL(int group) {
    int i, u, t;
    CPoint tp;
    CMatrixMapStatic *obj;
    CMatrixRobotAI *rl[MAX_ROBOTS];  // Список роботов на карте
    bool rlok[MAX_ROBOTS];
    bool ok;
    int rlcnt = 0;  // Кол-во роботов на карте
    byte mm = 0;
    int listcnt;
    SMatrixPlace *place;
    D3DXVECTOR2 v, v2;

    BufPrepare();

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id && GetGroupLogic(obj) == group) {
            mm |= 1 << (obj->AsRobot()->m_Unit[0].u1.s1.m_Kind - 1);
            rl[rlcnt] = obj->AsRobot();
            rlok[rlcnt] = true;
            rlcnt++;
        }
        obj = obj->GetNextLogic();
    }
    if (rlcnt <= 0)
        return;

    if (m_PlayerGroup[group].Order() == mpo_Repair && m_PlayerGroup[group].m_Obj &&
        m_PlayerGroup[group].m_Obj->IsLive()) {
        for (i = 0; i < rlcnt; i++) {
            if (m_PlayerGroup[group].m_Obj == rl[i])
                rl[i]->GetEnv()->m_Target = NULL;
            else
                rl[i]->GetEnv()->m_Target = m_PlayerGroup[group].m_Obj;
        }

        // Проверяем достает ли ченилка
        ok = true;
        for (i = 0; i < rlcnt; i++) {
            if (rl[i]->GetRepairDist() <= 0)
                continue;
            tp = PLPlacePos(rl[i]);
            D3DXVECTOR2 v;
            v.x = GLOBAL_SCALE_MOVE * tp.x + GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2.0f;
            v.y = GLOBAL_SCALE_MOVE * tp.y + GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2.0f;

            v2 = GetWorldPos(m_PlayerGroup[group].m_Obj);

            if ((POW2(v2.x - v.x) + POW2(v2.y - v.y)) > POW2(rl[i]->GetRepairDist())) {
                if ((g_MatrixMap->GetTime() - rl[i]->GetEnv()->m_PlaceNotFound) > 2000) {
                    rlok[i] = false;
                    ok = false;
                }
            }
        }

        // Назначаем новые места, если кто-нибудь не достает
        if (!ok) {
            if (g_MatrixMap->PlaceList(mm, GetMapPos(rl[0]), GetMapPos(m_PlayerGroup[group].m_Obj),
                                       Float2Int(rl[0]->GetRepairDist() * INVERT(GLOBAL_SCALE_MOVE)), false,
                                       m_PlaceList, &listcnt) == 0) {
                for (i = 0; i < rlcnt; i++)
                    rl[i]->GetEnv()->m_PlaceNotFound = g_MatrixMap->GetTime();
                return;
            }

            // Помечаем занятые места
            for (t = 0; t < listcnt; t++) {
                g_MatrixMap->m_RN.m_Place[m_PlaceList[t]].m_Data = 0;
            }
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (IsLiveUnit(obj))
                    ObjPlaceData(obj, 1);
                obj = obj->GetNextLogic();
            }

            // Находим места
            u = 0;
            for (i = 0; i < rlcnt; i++) {
                if (rlok[i])
                    continue;

                place = NULL;
                for (u = 0; u < listcnt; u++) {
                    place = g_MatrixMap->m_RN.m_Place + m_PlaceList[u];
                    if (place->m_Data)
                        continue;

                    v.x = GLOBAL_SCALE_MOVE * place->m_Pos.x + GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2.0f;
                    v.y = GLOBAL_SCALE_MOVE * place->m_Pos.y + GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2.0f;

                    v2 = GetWorldPos(m_PlayerGroup[group].m_Obj);
                    if ((POW2(v.x - v2.x) + POW2(v.y - v2.y)) < POW2(rl[i]->GetRepairDist()))
                        break;
                }
                if (u >= listcnt) {  // Если роботы не влазят
                    for (u = i; u < rlcnt; u++) {
                        rl[u]->GetEnv()->m_Place = -1;
                        rl[u]->GetEnv()->m_PlaceAdd = CPoint(-1, -1);
                    }
                    u = 0;
                    for (; i < rlcnt; i++) {
                        rl[i]->GetEnv()->m_PlaceNotFound = g_MatrixMap->GetTime();

                        for (; u < listcnt; u++) {
                            place = g_MatrixMap->m_RN.m_Place + m_PlaceList[u];
                            if (place->m_Data)
                                continue;
                            break;
                        }
                        if (u >= listcnt) {
                            // Выращиваем список
                            if (g_MatrixMap->PlaceListGrow(mm, m_PlaceList, &listcnt, rlcnt) <= 0)
                                continue;
                            // Помечаем занятые места
                            for (t = 0; t < listcnt; t++) {
                                g_MatrixMap->m_RN.m_Place[m_PlaceList[t]].m_Data = 0;
                            }
                            obj = CMatrixMapStatic::GetFirstLogic();
                            while (obj) {
                                if (IsLiveUnit(obj))
                                    ObjPlaceData(obj, 1);
                                obj = obj->GetNextLogic();
                            }
                            i--;
                            continue;
                        }
                        else {
                            place->m_Data = 1;
                            rl[i]->GetEnv()->m_Place = m_PlaceList[u];
                            rl[i]->GetEnv()->m_PlaceAdd = CPoint(-1, -1);

                            if (PrepareBreakOrder(rl[i])) {
                                rl[i]->MoveToHigh(place->m_Pos.x, place->m_Pos.y);
                            }
                        }
                    }

                    break;
                }
                place->m_Data = 1;
                rl[i]->GetEnv()->m_Place = m_PlaceList[u];
                rl[i]->GetEnv()->m_PlaceAdd = CPoint(-1, -1);

                if (PrepareBreakOrder(rl[i])) {
                    rl[i]->MoveToHigh(place->m_Pos.x, place->m_Pos.y);
                }
            }
        }
    }
    else {
        // Если нет цели для починки, то ищем ее
        for (i = 0; i < rlcnt; i++) {
            if (rl[i]->GetRepairDist() <= 0)
                continue;
            if (rl[i]->GetEnv()->m_Target && rl[i]->GetEnv()->m_Target->IsLive() &&
                rl[i]->GetEnv()->m_Target->NeedRepair()) {
                v = GetWorldPos(rl[i]);
                v2 = GetWorldPos(rl[i]->GetEnv()->m_Target);
                if ((POW2(v.x - v2.x) + POW2(v.y - v2.y)) < POW2(rl[i]->GetRepairDist()))
                    continue;
            }

            rl[i]->GetEnv()->m_Target = NULL;

            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (obj != rl[i] && obj->IsLive() && obj->GetSide() == m_Id && obj->NeedRepair()) {
                    v = GetWorldPos(rl[i]);
                    v2 = GetWorldPos(obj);
                    if ((POW2(v.x - v2.x) + POW2(v.y - v2.y)) < POW2(rl[i]->GetRepairDist())) {
                        rl[i]->GetEnv()->m_Target = obj;
                        break;
                    }
                }
                obj = obj->GetNextLogic();
            }
        }
    }

    // Корректируем точку выстрела
    for (i = 0; i < rlcnt; i++) {
        if (!rl[i]->GetEnv()->m_Target)
            continue;

        D3DXVECTOR3 des = PointOfAim(rl[i]->GetEnv()->m_Target);

        rl[i]->Fire(des, 2);
    }
}

bool CMatrixSideUnit::FirePL(int group) {
    bool rv = false;
    int i;
    CMatrixMapStatic *obj;
    CMatrixRobotAI *rl[MAX_ROBOTS];  // Список роботов на карте
    int rlcnt = 0;                   // Кол-во роботов на карте

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id && GetGroupLogic(obj) == group) {
            rl[rlcnt] = (CMatrixRobotAI *)obj;
            rlcnt++;
        }
        obj = obj->GetNextLogic();
    }
    if (rlcnt <= 0)
        return false;

    // Находим врага для всей группы
    for (i = 0; i < rlcnt; i++) {
        if (rl[i]->GetEnv()->m_TargetAttack && rl[i]->GetEnv()->SearchEnemy(rl[i]->GetEnv()->m_TargetAttack)) {
            float cd = Dist2(GetWorldPos(rl[i]->GetEnv()->m_TargetAttack), GetWorldPos(rl[i]));
            if (cd > POW2(rl[i]->GetMaxFireDist()))
                rl[i]->GetEnv()->m_TargetAttack = NULL;
        }
        if (!(rl[i]->GetEnv()->m_TargetAttack && rl[i]->GetEnv()->SearchEnemy(rl[i]->GetEnv()->m_TargetAttack))) {
            float mindist = 1e10f;
            CEnemy *enemyfind = NULL;
            CEnemy *enemy = NULL;
            // Находим ближайшего незакрытого врага
            enemy = rl[i]->GetEnv()->m_FirstEnemy;
            while (enemy) {
                if (IsLiveUnit(enemy->m_Enemy) && enemy->m_Enemy != rl[i]) {
                    float cd = Dist2(GetWorldPos(enemy->m_Enemy), GetWorldPos(rl[i]));
                    if (cd < mindist) {
                        // Проверяем не закрыт ли он своими
                        D3DXVECTOR3 des, from, dir, p;
                        float t, dist;

                        from = rl[i]->GetGeoCenter();
                        des = PointOfAim(enemy->m_Enemy);
                        dist = sqrt(POW2(from.x - des.x) + POW2(from.y - des.y) + POW2(from.z - des.z));
                        if (dist > 0.0f) {
                            t = 1.0f / dist;
                            dir.x = (des.x - from.x) * t;
                            dir.y = (des.y - from.y) * t;
                            dir.z = (des.z - from.z) * t;
                            obj = CMatrixMapStatic::GetFirstLogic();
                            while (obj) {
                                if (IsLiveUnit(obj) && obj->GetSide() == m_Id && rl[i] != obj &&
                                    rl[i]->GetEnv()->m_TargetAttack != obj) {
                                    p = PointOfAim(obj);

                                    if (IsIntersectSphere(p, 25.0f, from, dir, t)) {
                                        if (t >= 0.0f && t < dist)
                                            break;
                                    }
                                }
                                obj = obj->GetNextLogic();
                            }
                            if (!obj) {
                                mindist = cd;
                                enemyfind = enemy;
                            }
                        }
                    }
                }
                enemy = enemy->m_NextEnemy;
            }
            // Если не нашли открытого ищем закрытого
            if (!enemyfind) {
                enemy = rl[i]->GetEnv()->m_FirstEnemy;
                while (enemy) {
                    if (IsLiveUnit(enemy->m_Enemy) && enemy->m_Enemy != rl[i]) {
                        float cd = Dist2(GetWorldPos(enemy->m_Enemy), GetWorldPos(rl[i]));
                        if (cd < mindist) {
                            mindist = cd;
                            enemyfind = enemy;
                        }
                    }
                    enemy = enemy->m_NextEnemy;
                }
            }

            if (enemyfind) {
                rl[i]->GetEnv()->m_TargetAttack = enemyfind->m_Enemy;
            }
        }
    }

    // Корректируем точку выстрела
    D3DXVECTOR3 des, from, dir, p;
    float t, dist;

    for (i = 0; i < rlcnt; i++) {
        if (rl[i]->GetEnv()->m_TargetAttack && rl[i]->GetEnv()->SearchEnemy(rl[i]->GetEnv()->m_TargetAttack)) {
            if (!rl[i]->GetEnv()->m_TargetAttack->IsLive()) {
                rl[i]->StopFire();
                continue;
            }

            des = PointOfAim(rl[i]->GetEnv()->m_TargetAttack);

            // Не стрелять из прямого оружия, если на пути к цели свои
            from = rl[i]->GetGeoCenter();
            dist = sqrt(POW2(from.x - des.x) + POW2(from.y - des.y) + POW2(from.z - des.z));

            bool fireline = rl[i]->HaveRepair() != 2;

            if (fireline && dist > 0.0f) {
                t = 1.0f / dist;
                dir.x = (des.x - from.x) * t;
                dir.y = (des.y - from.y) * t;
                dir.z = (des.z - from.z) * t;

                obj = CMatrixMapStatic::GetFirstLogic();
                while (obj) {
                    if (IsLiveUnit(obj) && obj->GetSide() == m_Id && rl[i] != obj &&
                        rl[i]->GetEnv()->m_TargetAttack != obj) {
                        p = PointOfAim(obj);

                        if (IsIntersectSphere(p, 25.0f, from, dir, t)) {
                            if (t >= 0.0f && t < dist) {
                                // CHelper::DestroyByGroup(DWORD(this)+4);
                                // CHelper::Create(10,DWORD(this)+4)->Cone(from,des,1.0f,1.0f,0xffffffff,0xffffffff,3);
                                // CHelper::Create(10,DWORD(this)+4)->Sphere(D3DXVECTOR3(from.x+dir.x*t,from.y+dir.y*t,from.z+dir.z*t),2,5,0xffff0000);

                                fireline = false;
                                break;
                            }
                        }
                    }
                    obj = obj->GetNextLogic();
                }
            }

            //            des.x+=(float)g_MatrixMap->RndFloat(-5.0f,+5.0f);
            //            des.y+=(float)g_MatrixMap->RndFloat(-5.0f,+5.0f);
            //            des.z+=(float)g_MatrixMap->RndFloat(-5.0f,+5.0f);

            CInfo *env = GetEnv(rl[i]);
            if (env->m_TargetAttack != env->m_TargetLast) {
                env->m_TargetLast = env->m_TargetAttack;
            }

            if (fireline) {
                D3DXVECTOR3 v1 = rl[i]->GetGeoCenter();
                D3DXVECTOR3 v2 = PointOfAim(rl[i]->GetEnv()->m_TargetAttack);

                auto tmp = v2 - v1;
                fireline = D3DXVec3LengthSq(&tmp) <= POW2(rl[i]->GetMaxFireDist());

                if (fireline) {
                    CMatrixMapStatic *trace_res =
                            g_MatrixMap->Trace(NULL, v1, v2, TRACE_OBJECT | TRACE_NONOBJECT, rl[i]);
                    fireline = !(
                            (IS_TRACE_STOP_OBJECT(trace_res) && trace_res->GetObjectType() == OBJECT_TYPE_MAPOBJECT) ||
                            (trace_res == TRACE_STOP_WATER) || (trace_res == TRACE_STOP_LANDSCAPE));
                }
            }

            if (fireline) {
                rv = true;

                // Если у цели голова Firewall то в нее сложнее попасть
                /*if(env->m_TargetAttack->IsRobot() && env->m_TargetAttack->AsRobot()->m_AimProtect>0) {
                    if(env->m_Target!=env->m_TargetAttack || fabs(env->m_TargetAngle)<=1.0f*1.1f*ToRad) {
                        env->m_TargetAngle=0.0f;

                        env->m_TargetAngle=std::min(30.0f*ToRad,(float)atan(env->m_TargetAttack->AsRobot()->m_AimProtect/sqrt(POW2(des.x-rl[i]->m_PosX)+POW2(des.y-rl[i]->m_PosY))));
                        if(g_MatrixMap->Rnd(0,9)<5) env->m_TargetAngle=-env->m_TargetAngle;
                    }
                    else if(env->m_TargetAngle>0) env->m_TargetAngle-=1.0f*ToRad;
                    else env->m_TargetAngle+=1.0f*ToRad;*/

                if (env->m_TargetAttack->IsRobot() && env->m_TargetAttack->AsRobot()->m_AimProtect > 0) {
                    if (env->m_Target != env->m_TargetAttack ||
                        fabs(env->m_TargetAngle) <=
                                1.3f * ToRad) {  //>=15.0f*env->m_TargetAttack->AsRobot()->m_AimProtect*ToRad) {
                        env->m_TargetAngle = 0.0f;

                        env->m_TargetAngle =
                                std::min(8.0f * ToRad, (float)g_MatrixMap->Rnd(1, 100) / 100.0f * 16.0f *
                                                          env->m_TargetAttack->AsRobot()->m_AimProtect * ToRad);
                        if (g_MatrixMap->Rnd(0, 9) < 5)
                            env->m_TargetAngle = -env->m_TargetAngle;
                    }
                    // else if(env->m_TargetAngle>0) env->m_TargetAngle+=1.0f*ToRad;
                    // else if(env->m_TargetAngle<0) env->m_TargetAngle-=1.0f*ToRad;
                    // else if(fabs(env->m_TargetAngle)>1.0f) env->m_TargetAngle*=0.7f;
                    // else env->m_TargetAngle=(2*g_MatrixMap->Rnd(0,1)-1)*ToRad;
                    else
                        env->m_TargetAngle *= 0.75f;

                    if (env->m_TargetAngle != 0.0f) {
                        float vx = des.x - rl[i]->m_PosX;
                        float vy = des.y - rl[i]->m_PosY;
                        float sa, ca;
                        SinCos(env->m_TargetAngle, &sa, &ca);
                        des.x = (ca * vx + sa * vy) + rl[i]->m_PosX;
                        des.y = (-sa * vx + ca * vy) + rl[i]->m_PosY;
                    }
                }

                env->m_Target = env->m_TargetAttack;
                rl[i]->Fire(des);
            }
            else {
                if (rl[i]->HaveRepair() && (g_MatrixMap->GetTime() - rl[i]->GetEnv()->m_TargetChangeRepair) >
                                                   1000) {  // Ищем робота для починки
                    D3DXVECTOR2 v, v2;

                    if (rl[i]->GetEnv()->m_Target && IsLiveUnit(rl[i]->GetEnv()->m_Target) &&
                        rl[i]->GetEnv()->m_Target->GetSide() == m_Id && rl[i]->GetEnv()->m_Target->NeedRepair()) {
                        v = GetWorldPos(rl[i]);
                        v2 = GetWorldPos(rl[i]->GetEnv()->m_Target);
                        if ((POW2(v.x - v2.x) + POW2(v.y - v2.y)) > POW2(rl[i]->GetRepairDist()))
                            rl[i]->GetEnv()->m_Target = NULL;
                    }
                    else
                        rl[i]->GetEnv()->m_Target = NULL;

                    if (!rl[i]->GetEnv()->m_Target) {
                        obj = CMatrixMapStatic::GetFirstLogic();
                        while (obj) {
                            if (obj != rl[i] && IsLiveUnit(obj) && obj->GetSide() == m_Id && obj->NeedRepair()) {
                                v = GetWorldPos(rl[i]);
                                v2 = GetWorldPos(obj);
                                if ((POW2(v.x - v2.x) + POW2(v.y - v2.y)) < POW2(rl[i]->GetRepairDist())) {
                                    rl[i]->GetEnv()->m_Target = obj;
                                    rl[i]->GetEnv()->m_TargetChangeRepair = g_MatrixMap->GetTime();
                                    break;
                                }
                            }
                            obj = obj->GetNextLogic();
                        }
                    }
                }

                if (rl[i]->GetEnv()->TargetType() == 2)
                    rl[i]->Fire(PointOfAim(rl[i]->GetEnv()->m_Target), 2);
                else
                    rl[i]->StopFire();

                //                env->m_Target=NULL;
                //                rl[i]->StopFire();
            }
        }
    }
    return rv;
}

void CMatrixSideUnit::WarPL(int group) {
    int i, u;  //,x,y;
    byte mm = 0;
    CMatrixMapStatic *obj;
    CMatrixRobotAI *rl[MAX_ROBOTS];  // Список роботов на карте
    int rlcnt = 0;                   // Кол-во роботов на карте
    bool rlokmove[MAX_ROBOTS];

    BufPrepare();

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id && GetGroupLogic(obj) == group) {
            rl[rlcnt] = (CMatrixRobotAI *)obj;
            mm |= 1 << (obj->AsRobot()->m_Unit[0].u1.s1.m_Kind - 1);
            rlcnt++;
        }
        obj = obj->GetNextLogic();
    }
    if (rlcnt < 0)
        return;

    // Находим врага для всей группы
    for (i = 0; i < rlcnt; i++) {
        if (m_PlayerGroup[group].Order() == mpo_Attack && m_PlayerGroup[group].m_Obj &&
            m_PlayerGroup[group].m_Obj != rl[i] && m_PlayerGroup[group].m_Obj->IsLive() &&
            m_PlayerGroup[group].m_Obj != rl[i]->GetEnv()->m_TargetAttack) {
            if (rl[i]->GetEnv()->SearchEnemy(m_PlayerGroup[group].m_Obj)) {
                rl[i]->GetEnv()->m_TargetAttack = m_PlayerGroup[group].m_Obj;
            }
        }
        if (m_PlayerGroup[group].Order() == mpo_Capture && m_PlayerGroup[group].m_Obj &&
            m_PlayerGroup[group].m_Obj->IsLive() && rl[i]->GetEnv()->m_TargetAttack &&
            rl[i]->GetEnv()->m_TargetAttack->IsLive() &&
            (rl[i]->GetEnv()->m_TargetAttack->GetObjectType() != OBJECT_TYPE_CANNON ||
             ((CMatrixCannon *)(rl[i]->GetEnv()->m_TargetAttack))->m_ParentBuilding != m_PlayerGroup[group].m_Obj)) {
            float mindist = 1e10f;
            CEnemy *enemyfind = NULL;
            CEnemy *enemy = rl[i]->GetEnv()->m_FirstEnemy;
            while (enemy) {
                if (enemy->m_Enemy->IsLiveActiveCannon() &&
                    enemy->m_Enemy->AsCannon()->m_ParentBuilding == m_PlayerGroup[group].m_Obj) {
                    while (true) {
                        float cd = Dist2(GetWorldPos(enemy->m_Enemy), GetWorldPos(rl[i]));
                        if (enemyfind) {
                            if (mindist < cd)
                                break;
                        }

                        mindist = cd;
                        enemyfind = enemy;
                        break;
                    }
                }
                enemy = enemy->m_NextEnemy;
            }
            if (enemyfind) {
                rl[i]->GetEnv()->m_TargetAttack = enemyfind->m_Enemy;
                rl[i]->GetEnv()->m_Place = -1;
            }
        }

        if (!rl[i]->GetEnv()->m_TargetAttack) {
            float mindist = 1e10f;
            CEnemy *enemyfind = NULL;
            CEnemy *enemy = NULL;
            // Находим цель которую указал игрок
            if (m_PlayerGroup[group].m_Obj && m_PlayerGroup[group].m_Obj != rl[i]) {
                enemyfind = rl[i]->GetEnv()->SearchEnemy(m_PlayerGroup[group].m_Obj);
            }
            // Находим ближайшего незакрытого врага
            if (!enemyfind) {
                enemy = rl[i]->GetEnv()->m_FirstEnemy;
                while (enemy) {
                    if (IsLiveUnit(enemy->m_Enemy) && enemy->m_Enemy != rl[i]) {
                        float cd = Dist2(GetWorldPos(enemy->m_Enemy), GetWorldPos(rl[i]));
                        if (cd < mindist) {
                            // Проверяем не закрыт ли он своими
                            D3DXVECTOR3 des, from, dir, p;
                            float t, dist;

                            from = rl[i]->GetGeoCenter();
                            des = PointOfAim(enemy->m_Enemy);
                            dist = sqrt(POW2(from.x - des.x) + POW2(from.y - des.y) + POW2(from.z - des.z));
                            if (dist > 0.0f) {
                                t = 1.0f / dist;
                                dir.x = (des.x - from.x) * t;
                                dir.y = (des.y - from.y) * t;
                                dir.z = (des.z - from.z) * t;
                                obj = CMatrixMapStatic::GetFirstLogic();
                                while (obj) {
                                    if (IsLiveUnit(obj) && obj->GetSide() == m_Id && rl[i] != obj &&
                                        rl[i]->GetEnv()->m_TargetAttack != obj) {
                                        p = PointOfAim(obj);

                                        if (IsIntersectSphere(p, 25.0f, from, dir, t)) {
                                            if (t >= 0.0f && t < dist)
                                                break;
                                        }
                                    }
                                    obj = obj->GetNextLogic();
                                }
                                if (!obj) {
                                    mindist = cd;
                                    enemyfind = enemy;
                                }
                            }
                        }
                    }
                    enemy = enemy->m_NextEnemy;
                }
            }
            // Если не нашли открытого ищем закрытого
            if (!enemyfind) {
                enemy = rl[i]->GetEnv()->m_FirstEnemy;
                while (enemy) {
                    if (IsLiveUnit(enemy->m_Enemy) && enemy->m_Enemy != rl[i]) {
                        float cd = Dist2(GetWorldPos(enemy->m_Enemy), GetWorldPos(rl[i]));
                        if (cd < mindist) {
                            mindist = cd;
                            enemyfind = enemy;
                        }
                    }
                    enemy = enemy->m_NextEnemy;
                }
            }

            if (enemyfind) {
                rl[i]->GetEnv()->m_TargetAttack = enemyfind->m_Enemy;
                // Если новая цель пушка или завод, то меняем позицию
                if (rl[i]->GetEnv()->m_TargetAttack->IsLiveActiveCannon() ||
                    rl[i]->GetEnv()->m_TargetAttack->IsLiveBuilding()) {
                    rl[i]->GetEnv()->m_Place = -1;
                }
            }
        }
    }

    // Проверяем правильно ли роботы идут
    bool moveok = true;
    for (i = 0; i < rlcnt; i++) {
        rlokmove[i] = true;

        if (!rl[i]->CanBreakOrder())
            continue;  // Пропускаем если робот не может прервать текущий приказ
        if (!rl[i]->GetEnv()->m_TargetAttack) {  // Если у робота нет цели то ждем завершение состояния войныы
            /*            if(GetDesRegion(rl[i])!=m_LogicGroup[group].m_Action.m_Region) {
                            if(!PlaceInRegion(rl[i],rl[i]->GetEnv()->m_Place,m_LogicGroup[group].m_Action.m_Region)) {
                                if(CanChangePlace(rl[i])) {
                                AssignPlace(rl[i],m_LogicGroup[group].m_Action.m_Region);

                                SMatrixPlace * place=ObjPlacePtr(rl[i]);
                                if(place && PrepareBreakOrder(rl[i])) {
                                    rl[i]->MoveToHigh(place->m_Pos.x,place->m_Pos.y);
                                }
                            }
                            }
                        }*/
            continue;
        }
        if (rl[i]->GetEnv()->m_Place < 0) {
            if (CanChangePlace(rl[i])) {
                rlokmove[i] = false;
                moveok = false;
                continue;
            }
        }
        D3DXVECTOR2 tv = GetWorldPos(rl[i]->GetEnv()->m_TargetAttack);

        SMatrixPlace *place = GetPlacePtr(rl[i]->GetEnv()->m_Place);
        if (place == NULL) {
            if (CanChangePlace(rl[i])) {
                rl[i]->GetEnv()->m_Place = -1;
                rlokmove[i] = false;
                moveok = false;
                continue;
            }
        }
        else {
            float dist2 =
                    POW2(GLOBAL_SCALE_MOVE * place->m_Pos.x + GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2 - tv.x) +
                    POW2(GLOBAL_SCALE_MOVE * place->m_Pos.y + GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2 - tv.y);
            if (dist2 > POW2(rl[i]->GetMaxFireDist() - GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2)) {
                if (CanChangePlace(rl[i])) {
                    rl[i]->GetEnv()->m_Place = -1;
                    rlokmove[i] = false;
                    moveok = false;
                    continue;
                }
            }
        }
        /*        if(!IsToPlace(rl[i],rl[i]->GetEnv()->m_Place)) {
        //            IsToPlace(rl[i],rl[i]->GetEnv()->m_Place);
                    rlokmove[i]=false;
                    moveok=false;
                    continue;
                }*/
    }

    // Назначаем движение
    if (!moveok) {
        // Находим место
        // Находим центр и радиус
        CPoint center, tp2, tp = CPoint(0, 0);
        int f = 0;
        for (i = 0; i < rlcnt; i++) {
            if (!rl[i]->GetEnv()->m_TargetAttack)
                continue;
            tp += GetMapPos(rl[i]->GetEnv()->m_TargetAttack);
            f++;
        }
        if (f <= 0)
            return;
        tp.x = tp.x / f;
        tp.y = tp.y / f;
        f = 1000000000;
        for (i = 0; i < rlcnt; i++) {
            if (!rl[i]->GetEnv()->m_TargetAttack)
                continue;
            tp2 = GetMapPos(rl[i]->GetEnv()->m_TargetAttack);
            int f2 = POW2(tp.x - tp2.x) + POW2(tp.y - tp2.y);
            if (f2 < f) {
                f = f2;
                center = tp2;
            }
        }
        int radius = 0;
        int radiusrobot = 0;
        for (i = 0; i < rlcnt; i++) {
            if (!rl[i]->GetEnv()->m_TargetAttack)
                continue;
            tp2 = GetMapPos(rl[i]->GetEnv()->m_TargetAttack);
            radiusrobot = std::max(radiusrobot, Float2Int(rl[i]->GetMaxFireDist() / GLOBAL_SCALE_MOVE));
            radius = std::max(radius, Float2Int(sqrt(float(POW2(center.x - tp2.x) + POW2(center.y - tp2.y))) +
                                           rl[i]->GetMaxFireDist() / GLOBAL_SCALE_MOVE + ROBOT_MOVECELLS_PER_SIZE));
        }

        // DM(L"RadiusSeek",std::wstring().Format(L"<i>   <i>",radius,radiusrobot).Get());

        bool cplr = true;

        // Находим место
        int listcnt;
        if (g_MatrixMap->PlaceList(mm, GetMapPos(rl[0]), center, radius, false, m_PlaceList, &listcnt) == 0) {
            for (i = 0; i < rlcnt; i++)
                rl[i]->GetEnv()->m_PlaceNotFound = g_MatrixMap->GetTime();
        }
        else {
            /*CHelper::DestroyByGroup(524234);
            D3DXVECTOR3 v1;
            v1.x=center.x*GLOBAL_SCALE_MOVE;
            v1.y=center.y*GLOBAL_SCALE_MOVE;
            v1.z=g_MatrixMap->GetZ(v1.x,v1.y);
            CHelper::Create(3000,524234)->Cone(v1,D3DXVECTOR3(v1.x,v1.y,v1.z+50.0f),3.0f,3.0f,0xffffffff,0xffffffff,3);

            for(i=0;i<listcnt;i++) {
            D3DXVECTOR3 v1;
            v1.x=g_MatrixMap->m_RN.m_Place[m_PlaceList[i]].m_Pos.x*GLOBAL_SCALE_MOVE;
            v1.y=g_MatrixMap->m_RN.m_Place[m_PlaceList[i]].m_Pos.y*GLOBAL_SCALE_MOVE;
            v1.z=g_MatrixMap->GetZ(v1.x,v1.y);
            CHelper::Create(3000,524234)->Cone(v1,D3DXVECTOR3(v1.x,v1.y,v1.z+30.0f),1.0f,1.0f,0xffffffff,0xffffffff,3);
            }*/

            // CPoint tp;
            // CRect rect(1000000000,1000000000,-1000000000,-1000000000);
            // int growsizemin=0;
            // int growsizemax=0;

            // for(i=0;i<rlcnt;i++) {
            //    growsizemin=std::max(growsizemin,int(rl[i]->GetMinFireDist()/GLOBAL_SCALE_MOVE));
            //    growsizemax=std::max(growsizemax,int(rl[i]->GetMaxFireDist()/GLOBAL_SCALE_MOVE));

            //    CEnemy * enemy=rl[i]->GetEnv()->m_FirstEnemy;
            //    while(enemy) {
            //        if(IsLive(enemy->m_Enemy)) {
            //            tp=GetMapPos(enemy->m_Enemy);
            //            rect.left=std::min(rect.left,tp.x);
            //            rect.top=std::min(rect.top,tp.y);
            //            rect.right=std::max(rect.right,tp.x+ROBOT_MOVECELLS_PER_SIZE);
            //            rect.bottom=std::max(rect.bottom,tp.y+ROBOT_MOVECELLS_PER_SIZE);
            //        }
            //        enemy=enemy->m_NextEnemy;
            //    }
            //}
            // if(!rect.IsEmpty()) {
            // Помечаем уже занетые места
            //            g_MatrixMap->m_RN.ActionDataPL(CRect(rect.left-growsizemax,rect.top-growsizemax,rect.right+growsizemax,rect.bottom+growsizemax),0);
            /*            for(i=0;i<rlcnt;i++) {
                            if(!rlokmove[i]) continue;
                            if(rl[i]->GetEnv()->m_Place<0) continue;
                            GetPlacePtr(rl[i]->GetEnv()->m_Place)->m_Data=1;
                        }*/
            for (i = 0; i < listcnt; i++)
                g_MatrixMap->m_RN.m_Place[m_PlaceList[i]].m_Data = 0;
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (IsLiveUnit(obj))
                    ObjPlaceData(obj, 1);
                obj = obj->GetNextLogic();
            }

            // Находим лучшее место для каждого робота
            //            CRect
            //            plr=g_MatrixMap->m_RN.CorrectRectPL(CRect(rect.left-growsizemax,rect.top-growsizemax,rect.right+growsizemax,rect.bottom+growsizemax));
            for (i = 0; i < rlcnt; i++) {
                if (rlokmove[i])
                    continue;
                if (!rl[i]->GetEnv()->m_TargetAttack)
                    continue;  // Если нет цели, то пропускаем

                bool havebomb = rl[i]->HaveBomb();

                int placebest = -1;
                float s_f1 = 0.0f;
                int s_underfire = 0;
                bool s_close = false;

                float tvx, tvy;  // To target
                int enemy_fire_dist;

                if (rl[i]->GetEnv()->m_TargetAttack->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
                    tvx = ((CMatrixRobotAI *)(rl[i]->GetEnv()->m_TargetAttack))->m_PosX - rl[i]->m_PosX;
                    tvy = ((CMatrixRobotAI *)(rl[i]->GetEnv()->m_TargetAttack))->m_PosY - rl[i]->m_PosY;
                    enemy_fire_dist =
                            Float2Int(((CMatrixRobotAI *)(rl[i]->GetEnv()->m_TargetAttack))->GetMaxFireDist());
                }
                else if (rl[i]->GetEnv()->m_TargetAttack->GetObjectType() == OBJECT_TYPE_CANNON) {
                    tvx = ((CMatrixCannon *)(rl[i]->GetEnv()->m_TargetAttack))->m_Pos.x - rl[i]->m_PosX;
                    tvy = ((CMatrixCannon *)(rl[i]->GetEnv()->m_TargetAttack))->m_Pos.y - rl[i]->m_PosY;
                    enemy_fire_dist = int(((CMatrixCannon *)(rl[i]->GetEnv()->m_TargetAttack))->GetFireRadius() +
                                          GLOBAL_SCALE_MOVE);
                }
                else if (rl[i]->GetEnv()->m_TargetAttack->GetObjectType() == OBJECT_TYPE_BUILDING) {
                    tvx = ((CMatrixBuilding *)(rl[i]->GetEnv()->m_TargetAttack))->m_Pos.x - rl[i]->m_PosX;
                    tvy = ((CMatrixBuilding *)(rl[i]->GetEnv()->m_TargetAttack))->m_Pos.y - rl[i]->m_PosY;
                    enemy_fire_dist = 150;  // int(GLOBAL_SCALE_MOVE*12.0f);
                }
                else
                    continue;
                float tsize2 = tvx * tvx + tvy * tvy;
                float tsize2o = 1.0f / tsize2;

                // SMatrixPlaceList * plist=g_MatrixMap->m_RN.m_PLList+plr.left+plr.top*g_MatrixMap->m_RN.m_PLSizeX;
                // for(y=plr.top;y<plr.bottom;y++,plist+=g_MatrixMap->m_RN.m_PLSizeX-(plr.right-plr.left)) {
                //    for(x=plr.left;x<plr.right;x++,plist++) {
                //        SMatrixPlace * place=g_MatrixMap->m_RN.m_Place+plist->m_Sme;
                //        for(u=0;u<plist->m_Cnt;u++,place++) {
                for (u = 0; u < listcnt; u++) {
                    int iplace = m_PlaceList[u];
                    SMatrixPlace *place = g_MatrixMap->m_RN.m_Place + iplace;

                    if (place->m_Data)
                        continue;  // Занетые места игнорируем
                    if (place->m_Move & (1 << (rl[i]->m_Unit[0].u1.s1.m_Kind - 1)))
                        continue;  // Если робот не может стоять на этом месте то пропускаем
                    if (rl[i]->GetEnv()->IsBadPlace(iplace))
                        continue;  // Плохое место пропускаем

                    float pcx = GLOBAL_SCALE_MOVE * place->m_Pos.x + (GLOBAL_SCALE_MOVE * 4.0f / 2.0f);  // Center place
                    float pcy = GLOBAL_SCALE_MOVE * place->m_Pos.y + (GLOBAL_SCALE_MOVE * 4.0f / 2.0f);

                    float pvx = pcx - rl[i]->m_PosX;  // To place
                    float pvy = pcy - rl[i]->m_PosY;

                    float k = (pvx * tvx + pvy * tvy) * tsize2o;
                    if (!havebomb && rl[i]->GetEnv()->m_TargetAttack->GetObjectType() == OBJECT_TYPE_CANNON) {
                        //                                if(k>1.5) continue; // Места за врагом не расматриваем
                    }
                    else if (!havebomb && rl[i]->GetEnv()->m_TargetAttack->GetObjectType() != OBJECT_TYPE_BUILDING) {
                        if (k > 0.95)
                            continue;  // Места за врагом не расматриваем
                    }
                    else if (!havebomb) {
                        if (k > 1.2)
                            continue;  // Места сильно за врагом не расматриваем
                    }
                    //                            if(k<0.0) continue; // Места за роботом игнорируем
                    float m = (-pvx * tvy + pvy * tvx) * tsize2o;
                    float distfrom2 = POW2(-m * tvy) + POW2(m * tvx);  // Дистанция отклонения
                    float distplace2 =
                            POW2(tvx - pcx /*pvx*/) + POW2(tvy - pcx /*pvy*/);  // Дистанция от места до врага
                    if ((placebest < 0) || (rl[i]->GetEnv()->m_TargetAttack->GetObjectType() == OBJECT_TYPE_CANNON &&
                                            (rl[i]->GetMaxFireDist() - GLOBAL_SCALE_MOVE) > enemy_fire_dist)) {
                        if (distplace2 >
                            POW2(0.95 * rl[i]->GetMaxFireDist() - GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2))
                            continue;  // Робот должен достовать врага
                    }
                    else {
                        if (distplace2 >
                            POW2(0.95 * rl[i]->GetMinFireDist() - GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2))
                            continue;  // Робот должен достовать врага
                    }

                    if (!havebomb && rl[i]->GetEnv()->m_TargetAttack->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
                        if (distfrom2 > POW2(200 + 100))
                            continue;  // Робот не должен отклонится слишком далеко
                                       //                            } else {
                        //                                if(distfrom2>POW2(300+100)) continue; // Робот не должен
                        //                                отклонится слишком далеко
                    }

                    //// Если место слишком близко к постройке, то игнорируем
                    // if(rl[i]->GetEnv()->m_TargetAttack->GetObjectType()==OBJECT_TYPE_BUILDING) {
                    //    if(distplace2<POW2(200)) continue;
                    //}

                    int underfire = place->m_Underfire;
                    if (distplace2 <= POW2(enemy_fire_dist))
                        underfire++;

                    CMatrixMapStatic *trace_res = g_MatrixMap->Trace(
                            NULL, D3DXVECTOR3(pcx, pcy, g_MatrixMap->GetZ(pcx, pcy) + 20.0f) /*rl[i]->GetGeoCenter()*/,
                            PointOfAim(rl[i]->GetEnv()->m_TargetAttack),
                            TRACE_OBJECT | TRACE_NONOBJECT | TRACE_OBJECTSPHERE | TRACE_SKIP_INVISIBLE, rl[i]);
                    bool close =
                            (IS_TRACE_STOP_OBJECT(trace_res) && trace_res->GetObjectType() == OBJECT_TYPE_MAPOBJECT) ||
                            (trace_res == TRACE_STOP_WATER) || (trace_res == TRACE_STOP_LANDSCAPE);

                    if (placebest >= 0) {  // Если уже найдено место то выбираем наилучшее
                        if (havebomb) {
                            if (distplace2 > s_f1)
                                continue;  // Место дальше предыдущего пропускаем
                        }
                        else if (close != s_close) {
                            if (close)
                                continue;
                        }
                        else if (!underfire && s_underfire)
                            ;
                        else if (underfire && !s_underfire)
                            continue;
                        else if (underfire) {  // Если под обстрелом
                            if (underfire > s_underfire)
                                continue;  // Место более обстреливоемое пропускаем
                            if (distplace2 < s_f1)
                                continue;  // Место ближе предыдущего пропускаем
                        }
                        else {  // Если вне радиуса поражения противника
                            if (distplace2 > s_f1)
                                continue;  // Место дальше предыдущего пропускаем
                        }
                    }

                    s_close = close;
                    s_f1 = distplace2;
                    s_underfire = underfire;
                    placebest = iplace;
                }
                //    }
                //}

                if (placebest >= 0) {
                    cplr = false;
                    SMatrixPlace *place = GetPlacePtr(placebest);
                    place->m_Data = 1;
                    rl[i]->GetEnv()->m_Place = placebest;
                    if (PrepareBreakOrder(rl[i])) {
                        rl[i]->MoveToHigh(place->m_Pos.x, place->m_Pos.y);
                    }
                }
                else if (cplr) {
                    cplr = false;
                    if (g_MatrixMap->PlaceList(mm, GetMapPos(rl[0]), center, radiusrobot, false, m_PlaceList,
                                               &listcnt) == 0) {
                        for (u = 0; u < rlcnt; u++)
                            rl[u]->GetEnv()->m_PlaceNotFound = g_MatrixMap->GetTime();
                        break;
                    }
                    else {
                        for (u = 0; u < listcnt; u++)
                            g_MatrixMap->m_RN.m_Place[m_PlaceList[u]].m_Data = 0;
                        obj = CMatrixMapStatic::GetFirstLogic();
                        while (obj) {
                            if (IsLiveUnit(obj))
                                ObjPlaceData(obj, 1);
                            obj = obj->GetNextLogic();
                        }
                        i = -1;
                        continue;
                    }
                }
                else {  // Не нашли
                    rl[i]->GetEnv()->m_PlaceNotFound = g_MatrixMap->GetTime();

                    int iplace;
                    SMatrixPlace *place;

                    for (u = 0; u < listcnt; u++) {
                        iplace = m_PlaceList[u];
                        place = g_MatrixMap->m_RN.m_Place + iplace;

                        if (place->m_Data)
                            continue;  // Занетые места игнорируем
                        if (place->m_Move & (1 << (rl[i]->m_Unit[0].u1.s1.m_Kind - 1)))
                            continue;  // Если робот не может стоять на этом месте то пропускаем
                        break;
                    }
                    if (u < listcnt) {
                        place->m_Data = 1;
                        rl[i]->GetEnv()->m_Place = iplace;
                        if (PrepareBreakOrder(rl[i])) {
                            rl[i]->MoveToHigh(place->m_Pos.x, place->m_Pos.y);
                        }
                    }
                    else {  // Расширяем
                        if (g_MatrixMap->PlaceListGrow(mm, m_PlaceList, &listcnt, rlcnt) <= 0)
                            continue;

                        for (u = 0; u < listcnt; u++)
                            g_MatrixMap->m_RN.m_Place[m_PlaceList[u]].m_Data = 0;
                        obj = CMatrixMapStatic::GetFirstLogic();
                        while (obj) {
                            if (IsLiveUnit(obj))
                                ObjPlaceData(obj, 1);
                            obj = obj->GetNextLogic();
                        }
                    }
                }
            }
        }
    }

    // Корректируем точку выстрела
    D3DXVECTOR3 des, from, dir, p;
    float t, dist;

    int curTime = g_MatrixMap->GetTime();

    for (i = 0; i < rlcnt; i++) {
        if (rl[i]->GetEnv()->m_TargetAttack) {
            if (!rl[i]->GetEnv()->m_TargetAttack->IsLive()) {
                rl[i]->StopFire();
                continue;
            }

            des = PointOfAim(rl[i]->GetEnv()->m_TargetAttack);

            // Не стрелять из прямого оружия, если на пути к цели свои
            from = rl[i]->GetGeoCenter();
            dist = sqrt(POW2(from.x - des.x) + POW2(from.y - des.y) + POW2(from.z - des.z));

            bool fireline = rl[i]->HaveRepair() != 2;

            if (fireline && dist > 0.0f) {
                t = 1.0f / dist;
                dir.x = (des.x - from.x) * t;
                dir.y = (des.y - from.y) * t;
                dir.z = (des.z - from.z) * t;

                obj = CMatrixMapStatic::GetFirstLogic();
                while (obj) {
                    if (IsLiveUnit(obj) && obj->GetSide() == m_Id && rl[i] != obj &&
                        rl[i]->GetEnv()->m_TargetAttack != obj) {
                        p = PointOfAim(obj);

                        if (IsIntersectSphere(p, 25.0f, from, dir, t)) {
                            if (t >= 0.0f && t < dist) {
                                // CHelper::DestroyByGroup(DWORD(this)+4);
                                // CHelper::Create(10,DWORD(this)+4)->Cone(from,des,1.0f,1.0f,0xffffffff,0xffffffff,3);
                                // CHelper::Create(10,DWORD(this)+4)->Sphere(D3DXVECTOR3(from.x+dir.x*t,from.y+dir.y*t,from.z+dir.z*t),2,5,0xffff0000);

                                fireline = false;
                                break;
                            }
                        }
                    }
                    obj = obj->GetNextLogic();
                }
            }

            //            des.x+=(float)g_MatrixMap->RndFloat(-5.0f,+5.0f);
            //            des.y+=(float)g_MatrixMap->RndFloat(-5.0f,+5.0f);
            //            des.z+=(float)g_MatrixMap->RndFloat(-5.0f,+5.0f);

            CInfo *env = GetEnv(rl[i]);
            if (env->m_TargetAttack != env->m_TargetLast) {
                env->m_TargetLast = env->m_TargetAttack;
                env->m_TargetChange = curTime;
            }

            if (fireline) {
                D3DXVECTOR3 v1 = rl[i]->GetGeoCenter();
                D3DXVECTOR3 v2 = PointOfAim(rl[i]->GetEnv()->m_TargetAttack);

                auto tmp = v2 - v1;
                fireline = D3DXVec3LengthSq(&tmp) <= POW2(rl[i]->GetMaxFireDist());

                if (fireline) {
                    CMatrixMapStatic *trace_res =
                            g_MatrixMap->Trace(NULL, v1, v2, TRACE_OBJECT | TRACE_NONOBJECT, rl[i]);
                    fireline = !(
                            (IS_TRACE_STOP_OBJECT(trace_res) && trace_res->GetObjectType() == OBJECT_TYPE_MAPOBJECT) ||
                            (trace_res == TRACE_STOP_WATER) || (trace_res == TRACE_STOP_LANDSCAPE));
                }
            }

            if (fireline) {
                // Если у цели голова Firewall то в нее сложнее попасть
                /*if(env->m_TargetAttack->IsRobot() && env->m_TargetAttack->AsRobot()->m_AimProtect>0) {
                    if(env->m_Target!=env->m_TargetAttack || fabs(env->m_TargetAngle)<=1.0f*1.1f*ToRad) {
                        env->m_TargetAngle=0.0f;

                        env->m_TargetAngle=std::min(30.0f*ToRad,(float)atan(env->m_TargetAttack->AsRobot()->m_AimProtect/sqrt(POW2(des.x-rl[i]->m_PosX)+POW2(des.y-rl[i]->m_PosY))));
                        if(g_MatrixMap->Rnd(0,9)<5) env->m_TargetAngle=-env->m_TargetAngle;
                    }
                    else if(env->m_TargetAngle>0) env->m_TargetAngle-=1.0f*ToRad;
                    else env->m_TargetAngle+=1.0f*ToRad;*/

                if (env->m_TargetAttack->IsRobot() && env->m_TargetAttack->AsRobot()->m_AimProtect > 0) {
                    if (env->m_Target != env->m_TargetAttack ||
                        fabs(env->m_TargetAngle) <=
                                1.3f * ToRad) {  //>=15.0f*env->m_TargetAttack->AsRobot()->m_AimProtect*ToRad) {
                        env->m_TargetAngle = 0.0f;

                        env->m_TargetAngle =
                                std::min(8.0f * ToRad, (float)g_MatrixMap->Rnd(1, 100) / 100.0f * 16.0f *
                                                          env->m_TargetAttack->AsRobot()->m_AimProtect * ToRad);
                        if (g_MatrixMap->Rnd(0, 9) < 5)
                            env->m_TargetAngle = -env->m_TargetAngle;
                    }
                    // else if(env->m_TargetAngle>0) env->m_TargetAngle+=1.0f*ToRad;
                    // else if(env->m_TargetAngle<0) env->m_TargetAngle-=1.0f*ToRad;
                    // else if(fabs(env->m_TargetAngle)>1.0f) env->m_TargetAngle*=0.7f;
                    // else env->m_TargetAngle=(2*g_MatrixMap->Rnd(0,1)-1)*ToRad;
                    else
                        env->m_TargetAngle *= 0.75f;

                    if (env->m_TargetAngle != 0.0f) {
                        float vx = des.x - rl[i]->m_PosX;
                        float vy = des.y - rl[i]->m_PosY;
                        float sa, ca;
                        SinCos(env->m_TargetAngle, &sa, &ca);
                        des.x = (ca * vx + sa * vy) + rl[i]->m_PosX;
                        des.y = (-sa * vx + ca * vy) + rl[i]->m_PosY;
                    }
                }

                env->m_Target = env->m_TargetAttack;
                env->m_LastFire = curTime;
                rl[i]->Fire(des);

                // Если стоим на месте
                if (IsInPlace(rl[i])) {
                    // Если несколько врагов и в цель не попадаем в течении долгого времени, то переназначаем цель
                    if (env->m_EnemyCnt > 1 && (curTime - env->m_TargetChange) > 4000 &&
                        (curTime - env->m_LastHitTarget) > 4000) {
                        env->m_TargetAttack = NULL;
                    }
                    // Если один враг и в цель не попадаем в течении долгого времени и стоим на месте, то переназначаем
                    // место
                    if (env->m_EnemyCnt == 1 && (curTime - env->m_TargetChange) > 4000 &&
                        (curTime - env->m_LastHitTarget) > 4000) {
                        env->AddBadPlace(env->m_Place);
                        env->m_Place = -1;
                    }
                    // Если очень долго не попадаем в цель, то меняем позицию
                    if ((curTime - env->m_TargetChange) > 2000 && (curTime - env->m_LastHitTarget) > 10000) {
                        env->AddBadPlace(env->m_Place);
                        env->m_Place = -1;
                    }
                }
                else
                    env->m_TargetChange = curTime;
            }
            else {
                if (rl[i]->HaveRepair() && (g_MatrixMap->GetTime() - rl[i]->GetEnv()->m_TargetChangeRepair) >
                                                   1000) {  // Ищем робота для починки
                    D3DXVECTOR2 v, v2;

                    if (rl[i]->GetEnv()->m_Target && IsLiveUnit(rl[i]->GetEnv()->m_Target) &&
                        rl[i]->GetEnv()->m_Target->GetSide() == m_Id && rl[i]->GetEnv()->m_Target->NeedRepair()) {
                        v = GetWorldPos(rl[i]);
                        v2 = GetWorldPos(rl[i]->GetEnv()->m_Target);
                        if ((POW2(v.x - v2.x) + POW2(v.y - v2.y)) > POW2(rl[i]->GetRepairDist()))
                            rl[i]->GetEnv()->m_Target = NULL;
                    }
                    else
                        rl[i]->GetEnv()->m_Target = NULL;

                    if (!rl[i]->GetEnv()->m_Target) {
                        obj = CMatrixMapStatic::GetFirstLogic();
                        while (obj) {
                            if (obj != rl[i] && IsLiveUnit(obj) && obj->GetSide() == m_Id && obj->NeedRepair()) {
                                v = GetWorldPos(rl[i]);
                                v2 = GetWorldPos(obj);
                                if ((POW2(v.x - v2.x) + POW2(v.y - v2.y)) < POW2(rl[i]->GetRepairDist())) {
                                    rl[i]->GetEnv()->m_Target = obj;
                                    rl[i]->GetEnv()->m_TargetChangeRepair = g_MatrixMap->GetTime();
                                    break;
                                }
                            }
                            obj = obj->GetNextLogic();
                        }
                    }
                }

                if (rl[i]->GetEnv()->TargetType() == 2)
                    rl[i]->Fire(PointOfAim(rl[i]->GetEnv()->m_Target), 2);
                else
                    rl[i]->StopFire();

                // Если стоим на месте
                if (IsInPlace(rl[i])) {
                    // Если несколько врагов, а текущий долго закрыт своими, то переназначаем цель
                    if (env->m_EnemyCnt > 1 && (curTime - env->m_TargetChange) > 4000 &&
                        (curTime - env->m_LastFire) > 4000) {
                        env->m_TargetAttack = NULL;
                    }
                    // Если долго закрыт своими, то меняем позицию
                    if ((curTime - env->m_TargetChange) > 4000 && (curTime - env->m_LastFire) > 6000) {
                        if (CanChangePlace(rl[i])) {
                            env->AddBadPlace(env->m_Place);
                            env->m_Place = -1;
                        }
                    }
                }
                else
                    env->m_TargetChange = curTime;
            }
        }
    }
}

int CMatrixSideUnit::SelGroupToLogicGroup() {
    CMatrixMapStatic *obj;
    int i, no;

    for (i = 0; i < MAX_LOGIC_GROUP; i++)
        m_PlayerGroup[i].m_RobotCnt = 0;

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id) {
            if (obj->AsRobot()->GetGroupLogic() >= 0 && obj->AsRobot()->GetGroupLogic() < MAX_LOGIC_GROUP) {
                m_PlayerGroup[obj->AsRobot()->GetGroupLogic()].m_RobotCnt++;
            }
        }
        obj = obj->GetNextLogic();
    }

    for (no = 0; no < MAX_LOGIC_GROUP; no++) {
        if (m_PlayerGroup[no].m_RobotCnt <= 0)
            break;
    }
    ASSERT(no < MAX_LOGIC_GROUP);

    m_PlayerGroup[no].Order(mpo_Stop);
    m_PlayerGroup[no].m_Obj = NULL;
    m_PlayerGroup[no].SetWar(false);
    m_PlayerGroup[no].m_RoadPath->Clear();

    CMatrixGroupObject *objs = GetCurGroup()->m_FirstObject;
    while (objs) {
        if (objs->GetObject() && objs->GetObject()->IsLiveRobot()) {
            m_PlayerGroup[no].m_RobotCnt++;
            objs->GetObject()->AsRobot()->SetGroupLogic(no);
        }
        objs = objs->m_NextObject;
    }

    return no;
}

int CMatrixSideUnit::RobotToLogicGroup(CMatrixRobotAI *robot) {
    CMatrixMapStatic *obj;
    int i, no;

    for (i = 0; i < MAX_LOGIC_GROUP; i++)
        m_PlayerGroup[i].m_RobotCnt = 0;

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id) {
            if (obj->AsRobot()->GetGroupLogic() >= 0 && obj->AsRobot()->GetGroupLogic() < MAX_LOGIC_GROUP) {
                m_PlayerGroup[obj->AsRobot()->GetGroupLogic()].m_RobotCnt++;
            }
        }
        obj = obj->GetNextLogic();
    }

    for (no = 0; no < MAX_LOGIC_GROUP; no++) {
        if (m_PlayerGroup[no].m_RobotCnt <= 0)
            break;
    }
    ASSERT(no < MAX_LOGIC_GROUP);

    m_PlayerGroup[no].Order(mpo_Stop);
    m_PlayerGroup[no].m_Obj = NULL;
    m_PlayerGroup[no].SetWar(false);
    m_PlayerGroup[no].m_RoadPath->Clear();

    m_PlayerGroup[no].m_RobotCnt++;
    robot->SetGroupLogic(no);

    return no;
}

void CMatrixSideUnit::PGOrderStop(int no) {
    if (m_PlayerGroup[no].m_RobotCnt <= 0)
        return;

    m_PlayerGroup[no].Order(mpo_Stop);
    m_PlayerGroup[no].m_To = PGCalcCenterGroup(no);
    m_PlayerGroup[no].m_Obj = NULL;
    m_PlayerGroup[no].SetShowPlace(true);
    m_PlayerGroup[no].m_RoadPath->Clear();

    PGRemoveAllPassive(no, m_PlayerGroup[no].m_Obj);

    if (m_PlayerGroup[no].m_To.Dist2(PGCalcPlaceCenter(no)) > POW2(10)) {
        PGAssignPlacePlayer(no, m_PlayerGroup[no].m_To);
    }
    else {
        PGShowPlace(no);
    }

    m_LastTaktHL = -1000;
    TaktPL();
    PGShowPlace(no);
}

void CMatrixSideUnit::PGOrderMoveTo(int no, const CPoint &tp) {
    if (m_PlayerGroup[no].m_RobotCnt <= 0)
        return;

    if (m_PlayerGroup[no].Order() == mpo_MoveTo) {
        // in progress
        int o = IRND(2);
        if (o == 0)
            CSound::Play(S_ORDER_INPROGRESS1, SL_ORDER);
        else
            CSound::Play(S_ORDER_INPROGRESS2, SL_ORDER);
    }
    else {
        if (GetCurGroup()) {
            if (GetCurGroup()->GetObjectByN(GetCurSelNum())->IsLiveRobot()) {
                int chassis = (int)GetCurGroup()->GetObjectByN(GetCurSelNum())->AsRobot()->m_Unit[0].u1.s1.m_Kind;

                CBlockPar *rs = g_MatrixData->BlockGet(PAR_SOURCE_CHARS)
                                        ->BlockGet(L"ChassisSounds")
                                        ->BlockGetNE(utils::format(L"%d", chassis));
                if (rs) {
                    int cnt = rs->ParCount(L"MoveTo");
                    CSound::Play(rs->ParGet(L"MoveTo", IRND(cnt)).c_str(), SL_ORDER);
                }
            }
        }

        // int o = IRND(4);
        // if (o == 0)
        //    CSound::Play(S_ORDER_ACCEPT,SL_ORDER);
        // else if (o == 1)
        //    CSound::Play(S_ORDER_MOVE_TO_1,SL_ORDER);
        // else if (o == 2)
        //    CSound::Play(S_ORDER_MOVE_TO_2,SL_ORDER);
        // else if (o == 3)
        //    CSound::Play(S_ORDER_MOVE_TO_3,SL_ORDER);
    }

    m_PlayerGroup[no].Order(mpo_MoveTo);
    m_PlayerGroup[no].m_To = tp;
    m_PlayerGroup[no].m_Obj = NULL;
    m_PlayerGroup[no].SetShowPlace(true);
    m_PlayerGroup[no].m_RoadPath->Clear();

    PGRemoveAllPassive(no, m_PlayerGroup[no].m_Obj);

    PGAssignPlacePlayer(no, tp);

    m_LastTaktHL = -1000;
    TaktPL();
    PGShowPlace(no);
}

void CMatrixSideUnit::SoundCapture(int pg) {
    if (FLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_CAPTURE_ENABLED)) {
        if (m_PlayerGroup[pg].Order() == mpo_Capture) {
            // in progress
            int o = IRND(2);
            if (o == 0)
                CSound::Play(S_ORDER_INPROGRESS1, SL_ORDER);
            else
                CSound::Play(S_ORDER_INPROGRESS2, SL_ORDER);
        }
        else {
            CSound::Play(S_ORDER_CAPTURE, SL_ORDER);
        }
    }
    RESETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_CAPTURE_ENABLED);
}

void CMatrixSideUnit::PGOrderCapture(int no, CMatrixBuilding *building) {
    if (m_PlayerGroup[no].m_RobotCnt <= 0)
        return;

    SETFLAG(g_MatrixMap->m_Flags, MMFLAG_ENABLE_CAPTURE_FUCKOFF_SOUND);
    SETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_CAPTURE_ENABLED);

    if (m_PlayerGroup[no].Order() == mpo_Capture) {
        // in progress
        int o = IRND(2);
        if (o == 0)
            CSound::Play(S_ORDER_INPROGRESS1, SL_ORDER);
        else
            CSound::Play(S_ORDER_INPROGRESS2, SL_ORDER);
    }
    else {
        // int o = IRND(4);
        // if (o == 0)
        //    CSound::Play(S_ORDER_ACCEPT,SL_ORDER);
        // else if (o == 1)
        //    CSound::Play(S_ORDER_MOVE_TO_1,SL_ORDER);
        // else if (o == 2)
        //    CSound::Play(S_ORDER_MOVE_TO_2,SL_ORDER);
        // else if (o == 3)
        CSound::Play(S_ORDER_CAPTURE_PUSH, SL_ORDER);
    }

    m_PlayerGroup[no].Order(mpo_Capture);
    m_PlayerGroup[no].m_To = GetMapPos(building);
    m_PlayerGroup[no].m_Obj = building;
    m_PlayerGroup[no].SetShowPlace(true);
    m_PlayerGroup[no].m_RoadPath->Clear();

    PGRemoveAllPassive(no, m_PlayerGroup[no].m_Obj);

    CMatrixMapStatic *obj;
    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id) {
            CMatrixBuilding *b2 = obj->AsRobot()->GetCaptureFactory();
            if (obj->AsRobot()->GetGroupLogic() == no) {
                if (b2 && b2 != building) {
                    if (obj->AsRobot()->CanBreakOrder()) {
                        obj->AsRobot()->StopCapture();
                        obj->AsRobot()->GetEnv()->m_Place = -1;
                    }
                }
            }
            else {
                if (b2 && b2 == building) {
                    if (obj->AsRobot()->CanBreakOrder()) {
                        obj->AsRobot()->StopCapture();
                        obj->AsRobot()->GetEnv()->m_Place = -1;
                    }
                }
            }
        }
        obj = obj->GetNextLogic();
    }

    m_LastTaktHL = -1000;
    TaktPL(no);
    PGShowPlace(no);
}

void CMatrixSideUnit::PGOrderAttack(int no, const CPoint &tp, CMatrixMapStatic *terget_obj) {
    if (m_PlayerGroup[no].m_RobotCnt <= 0)
        return;

    if (!FLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE)) {
        if (m_PlayerGroup[no].Order() == mpo_Attack) {
            // in progress
            int o = IRND(2);
            if (o == 0)
                CSound::Play(S_ORDER_INPROGRESS1, SL_ORDER);
            else
                CSound::Play(S_ORDER_INPROGRESS2, SL_ORDER);
        }
        else {
            CSound::Play(S_ORDER_ATTACK, SL_ORDER);
        }
    }

    m_PlayerGroup[no].Order(mpo_Attack);
    m_PlayerGroup[no].m_To = tp;
    m_PlayerGroup[no].m_Obj = terget_obj;
    m_PlayerGroup[no].SetShowPlace(true);
    m_PlayerGroup[no].m_RoadPath->Clear();

    PGRemoveAllPassive(no, m_PlayerGroup[no].m_Obj);

    CMatrixMapStatic *obj;
    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == no) {
            CMatrixRobotAI *robot = (CMatrixRobotAI *)obj;
            robot->GetEnv()->m_Place = -1;
            robot->GetEnv()->m_PlaceAdd = CPoint(-1, -1);
        }
        obj = obj->GetNextLogic();
    }

    if (terget_obj) {
        // Точка сбора подальше от базы а то могут вместе с ней зарватся
        if (terget_obj->GetObjectType() == OBJECT_TYPE_BUILDING /*&& ((CMatrixBuilding *)terget_obj)->m_Kind==0*/) {
            switch (((CMatrixBuilding *)terget_obj)->m_Angle) {
                case 0:
                    m_PlayerGroup[no].m_To.y += 20;
                    break;
                case 1:
                    m_PlayerGroup[no].m_To.x -= 20;
                    break;
                case 2:
                    m_PlayerGroup[no].m_To.y -= 20;
                    break;
                case 3:
                    m_PlayerGroup[no].m_To.x += 20;
                    break;
            }
        }

        CMatrixMapStatic *obj;
        obj = CMatrixMapStatic::GetFirstLogic();
        while (obj) {
            if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == no) {
                CMatrixRobotAI *robot = (CMatrixRobotAI *)obj;

                if (robot->GetEnv()->SearchEnemy(terget_obj)) {
                    robot->GetEnv()->m_Target = terget_obj;
                }
                else {
                    //                    if(IsLiveBuilding(terget_obj)) {
                    //                        robot->GetEnv()->AddToList(terget_obj);
                    //                        robot->GetEnv()->m_Target=terget_obj;
                    //                    }
                }
            }
            obj = obj->GetNextLogic();
        }
    }

    m_LastTaktHL = -1000;
    TaktPL();
    PGShowPlace(no);
}

void CMatrixSideUnit::PGOrderPatrol(int no, const CPoint &tp) {
    if (m_PlayerGroup[no].m_RobotCnt <= 0)
        return;

    if (m_PlayerGroup[no].Order() == mpo_Patrol) {
        // in progress
        int o = IRND(2);
        if (o == 0)
            CSound::Play(S_ORDER_INPROGRESS1, SL_ORDER);
        else
            CSound::Play(S_ORDER_INPROGRESS2, SL_ORDER);
    }
    else {
        if (GetCurGroup()) {
            if (GetCurGroup()->GetObjectByN(GetCurSelNum())->IsLiveRobot()) {
                int chassis = (int)GetCurGroup()->GetObjectByN(GetCurSelNum())->AsRobot()->m_Unit[0].u1.s1.m_Kind;

                CBlockPar *rs = g_MatrixData->BlockGet(PAR_SOURCE_CHARS)
                                        ->BlockGet(L"ChassisSounds")
                                        ->BlockGetNE(utils::format(L"%d", chassis));
                if (rs) {
                    int cnt = rs->ParCount(L"Patrol");
                    CSound::Play(rs->ParGet(L"Patrol", IRND(cnt)).c_str(), SL_ORDER);
                }
            }
        }
    }

    m_PlayerGroup[no].Order(mpo_Patrol);
    m_PlayerGroup[no].m_To = tp;
    m_PlayerGroup[no].m_From = PGCalcPlaceCenter(no);
    m_PlayerGroup[no].m_Obj = NULL;
    m_PlayerGroup[no].SetPatrolReturn(false);
    m_PlayerGroup[no].SetShowPlace(true);
    m_PlayerGroup[no].m_RoadPath->Clear();

    PGRemoveAllPassive(no, m_PlayerGroup[no].m_Obj);

    PGAssignPlacePlayer(no, tp);

    m_LastTaktHL = -1000;
    TaktPL();
    PGShowPlace(no);
}

void CMatrixSideUnit::PGOrderRepair(int no, CMatrixMapStatic *terget_obj) {
    if (m_PlayerGroup[no].m_RobotCnt <= 0)
        return;

    if (m_PlayerGroup[no].Order() == mpo_Repair) {
        // in progress
        int o = IRND(2);
        if (o == 0)
            CSound::Play(S_ORDER_INPROGRESS1, SL_ORDER);
        else
            CSound::Play(S_ORDER_INPROGRESS2, SL_ORDER);
    }
    else {
        int o = IRND(2);
        if (o == 0)
            CSound::Play(S_ORDER_ACCEPT, SL_ORDER);
        else
            CSound::Play(S_ORDER_REPAIR, SL_ORDER);
    }

    m_PlayerGroup[no].Order(mpo_Repair);
    m_PlayerGroup[no].m_To = GetMapPos(terget_obj);
    m_PlayerGroup[no].m_From = CPoint(-1, -1);
    m_PlayerGroup[no].m_Obj = terget_obj;
    m_PlayerGroup[no].SetShowPlace(true);
    m_PlayerGroup[no].m_RoadPath->Clear();

    PGRemoveAllPassive(no, m_PlayerGroup[no].m_Obj);

    // PGAssignPlacePlayer(no,m_PlayerGroup[no].m_To);

    m_LastTaktHL = -1000;
    TaktPL();
    PGShowPlace(no);
}

void CMatrixSideUnit::PGOrderBomb(int no, const CPoint &tp, CMatrixMapStatic *terget_obj) {
    if (m_PlayerGroup[no].m_RobotCnt <= 0)
        return;

    m_PlayerGroup[no].Order(mpo_Bomb);
    m_PlayerGroup[no].m_To = tp;
    m_PlayerGroup[no].m_Obj = terget_obj;
    m_PlayerGroup[no].SetShowPlace(true);
    m_PlayerGroup[no].m_RoadPath->Clear();

    PGRemoveAllPassive(no, m_PlayerGroup[no].m_Obj);

    CMatrixMapStatic *obj;
    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == no) {
            CMatrixRobotAI *robot = (CMatrixRobotAI *)obj;
            robot->GetEnv()->m_Place = -1;
            robot->GetEnv()->m_PlaceAdd = CPoint(-1, -1);
        }
        obj = obj->GetNextLogic();
    }

    m_LastTaktHL = -1000;
    TaktPL();
    PGShowPlace(no);
}

void CMatrixSideUnit::PGOrderAutoCapture(int no) {
    if (m_PlayerGroup[no].m_RobotCnt <= 0)
        return;

    if (m_PlayerGroup[no].Order() == mpo_AutoCapture) {
        // in progress
        int o = IRND(2);
        if (o == 0)
            CSound::Play(S_ORDER_INPROGRESS1, SL_ORDER);
        else
            CSound::Play(S_ORDER_INPROGRESS2, SL_ORDER);
    }
    else {
        CSound::Play(S_ORDER_AUTO_CAPTURE, SL_ORDER);
    }

    m_PlayerGroup[no].Order(mpo_AutoCapture);
    m_PlayerGroup[no].m_To = CPoint(-1, -1);
    m_PlayerGroup[no].m_From = CPoint(-1, -1);
    m_PlayerGroup[no].m_Obj = NULL;
    m_PlayerGroup[no].SetShowPlace(false);
    m_PlayerGroup[no].m_RoadPath->Clear();

    PGRemoveAllPassive(no, m_PlayerGroup[no].m_Obj);
}

void CMatrixSideUnit::PGOrderAutoAttack(int no) {
    if (m_PlayerGroup[no].m_RobotCnt <= 0)
        return;

    if (m_PlayerGroup[no].Order() == mpo_AutoAttack) {
        // in progress
        int o = IRND(2);
        if (o == 0)
            CSound::Play(S_ORDER_INPROGRESS1, SL_ORDER);
        else
            CSound::Play(S_ORDER_INPROGRESS2, SL_ORDER);
    }
    else {
        CSound::Play(S_ORDER_AUTO_ATTACK, SL_ORDER);
    }

    m_PlayerGroup[no].Order(mpo_AutoAttack);
    m_PlayerGroup[no].m_To = CPoint(-1, -1);
    m_PlayerGroup[no].m_From = CPoint(-1, -1);
    m_PlayerGroup[no].m_Obj = NULL;
    m_PlayerGroup[no].SetShowPlace(false);
    m_PlayerGroup[no].m_Region = -1;
    m_PlayerGroup[no].m_RoadPath->Clear();

    PGRemoveAllPassive(no, m_PlayerGroup[no].m_Obj);

    CMatrixMapStatic *obj;
    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id) {
            if (obj->AsRobot()->GetGroupLogic() == no) {
                if (obj->AsRobot()->GetCaptureFactory()) {
                    if (obj->AsRobot()->CanBreakOrder()) {
                        obj->AsRobot()->StopCapture();
                        obj->AsRobot()->GetEnv()->m_Place = -1;
                        obj->AsRobot()->GetEnv()->m_PlaceNotFound = -10000;
                    }
                }
            }
        }
        obj = obj->GetNextLogic();
    }
}

void CMatrixSideUnit::PGOrderAutoDefence(int no) {
    if (m_PlayerGroup[no].m_RobotCnt <= 0)
        return;

    if (m_PlayerGroup[no].Order() == mpo_AutoDefence) {
        // in progress
        int o = IRND(2);
        if (o == 0)
            CSound::Play(S_ORDER_INPROGRESS1, SL_ORDER);
        else
            CSound::Play(S_ORDER_INPROGRESS2, SL_ORDER);
    }
    else {
        CSound::Play(S_ORDER_AUTO_DEFENCE, SL_ORDER);
    }

    CPoint tp(0, 0);
    int cnt = 0;
    CMatrixMapStatic *obj;
    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id) {
            if (obj->AsRobot()->GetGroupLogic() == no) {
                tp.x += obj->AsRobot()->GetMapPosX();
                tp.y += obj->AsRobot()->GetMapPosY();
                cnt++;
                if (obj->AsRobot()->GetCaptureFactory()) {
                    if (obj->AsRobot()->CanBreakOrder()) {
                        obj->AsRobot()->StopCapture();
                        obj->AsRobot()->GetEnv()->m_Place = -1;
                        obj->AsRobot()->GetEnv()->m_PlaceNotFound = -10000;
                    }
                }
            }
        }
        obj = obj->GetNextLogic();
    }

    m_PlayerGroup[no].Order(mpo_AutoDefence);
    m_PlayerGroup[no].m_To = CPoint(-1, -1);
    m_PlayerGroup[no].m_From = CPoint(-1, -1);
    m_PlayerGroup[no].m_Obj = NULL;
    m_PlayerGroup[no].SetShowPlace(false);
    m_PlayerGroup[no].m_Region = g_MatrixMap->GetRegion(tp.x / cnt, tp.y / cnt);
    m_PlayerGroup[no].m_RoadPath->Clear();

    PGRemoveAllPassive(no, m_PlayerGroup[no].m_Obj);

    PGAssignPlace(no, g_MatrixMap->m_RN.GetRegion(m_PlayerGroup[no].m_Region)->m_Center);

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == no) {
            CMatrixRobotAI *robot = (CMatrixRobotAI *)obj;

            tp = PLPlacePos(robot);
            if (tp.x >= 0 && PrepareBreakOrder(robot))
                robot->MoveToHigh(tp.x, tp.y);
        }
        obj = obj->GetNextLogic();
    }
}

void CMatrixSideUnit::PGRemoveAllPassive(int no, CMatrixMapStatic *skip) {
#define IsPassive(obj) \
    (((obj)->GetObjectType() == OBJECT_TYPE_BUILDING) || ((obj)->IsLive() && (obj)->GetSide() == m_Id))

    CMatrixMapStatic *obj;
    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == no) {
            CMatrixRobotAI *robot = (CMatrixRobotAI *)obj;
            CInfo *env = robot->GetEnv();

            if (env->m_Target && env->m_Target != skip && IsPassive(env->m_Target))
                env->m_Target = NULL;
            CEnemy *enemie = env->m_FirstEnemy;
            while (enemie) {
                CEnemy *e2 = enemie;
                enemie = enemie->m_NextEnemy;
                if (e2->GetEnemy() != skip && IsPassive(e2->GetEnemy()))
                    env->RemoveFromList(e2);
            }
        }
        obj = obj->GetNextLogic();
    }
#undef IsPassive
}

void CMatrixSideUnit::PGSetPlace(CMatrixRobotAI *robot, const CPoint &p) {
    int x, y, u;
    CRect plr = g_MatrixMap->m_RN.CorrectRectPL(CRect(p.x - 4, p.y - 4, p.x + 4, p.y + 4));

    SMatrixPlaceList *plist = g_MatrixMap->m_RN.m_PLList + plr.left + plr.top * g_MatrixMap->m_RN.m_PLSizeX;
    for (y = plr.top; y < plr.bottom; y++, plist += g_MatrixMap->m_RN.m_PLSizeX - (plr.right - plr.left)) {
        for (x = plr.left; x < plr.right; x++, plist++) {
            SMatrixPlace *place = g_MatrixMap->m_RN.m_Place + plist->m_Sme;
            for (u = 0; u < plist->m_Cnt; u++, place++) {
                if (!(p.x >= (place->m_Pos.x + 4) || (p.x + 4) <= place->m_Pos.x) &&
                    !(p.y >= (place->m_Pos.y + 4) || (p.y + 4) <= place->m_Pos.y)) {
                    robot->GetEnv()->m_Place = plist->m_Sme + u;
                    robot->GetEnv()->m_PlaceAdd = CPoint(-1, -1);
                    return;
                }
            }
        }
    }
    robot->GetEnv()->m_Place = -1;
    robot->GetEnv()->m_PlaceAdd = p;
}

CPoint CMatrixSideUnit::PGCalcCenterGroup(int no) {
    CMatrixMapStatic *obj;

    CPoint tp(0, 0);
    int cnt = 0;

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && (obj->AsRobot()->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == no)) {
            CMatrixRobotAI *robot = (CMatrixRobotAI *)obj;

            tp.x += robot->GetMapPosX();
            tp.y += robot->GetMapPosY();
            cnt++;
        }
        obj = obj->GetNextLogic();
    }
    return CPoint(tp.x / cnt, tp.y / cnt);
}

void CMatrixSideUnit::PGPlaceClear(int no) {
    CMatrixMapStatic *obj;

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && (obj->AsRobot()->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == no)) {
            CMatrixRobotAI *robot = (CMatrixRobotAI *)obj;

            robot->GetEnv()->m_Place = -1;
            robot->GetEnv()->m_PlaceAdd = CPoint(-1, -1);
        }
        obj = obj->GetNextLogic();
    }
}

CPoint CMatrixSideUnit::PGCalcPlaceCenter(int no) {
    CMatrixMapStatic *obj;

    CPoint tp(0, 0);
    int cnt = 0;

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && (obj->AsRobot()->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == no)) {
            CMatrixRobotAI *robot = (CMatrixRobotAI *)obj;

            CPoint tp2 = PLPlacePos(robot);
            if (tp2.x >= 0) {
                tp.x += tp2.x;
                tp.y += tp2.y;
                cnt++;
            }
        }
        obj = obj->GetNextLogic();
    }
    if (cnt <= 0)
        return PGCalcCenterGroup(no);
    else
        return CPoint(tp.x / cnt, tp.y / cnt);
}

void CMatrixSideUnit::PGShowPlace(int no) {
    CMatrixMapStatic *obj;

    CMatrixEffect::DeleteAllMoveto();

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && (obj->AsRobot()->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == no)) {
            CMatrixRobotAI *robot = (CMatrixRobotAI *)obj;

            CMatrixBuilding *cf = robot->GetCaptureFactory();
            if (cf) {
                D3DXVECTOR2 v;
                v = GetWorldPos(cf);
                CMatrixEffect::CreateMoveto(D3DXVECTOR3(v.x, v.y, g_MatrixMap->GetZ(v.x, v.y) + 2.0f));
            }
            else {
                CPoint tp = PLPlacePos(robot);
                if (tp.x >= 0) {
                    D3DXVECTOR3 v;
                    v.x = GLOBAL_SCALE_MOVE * tp.x + GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2;
                    v.y = GLOBAL_SCALE_MOVE * tp.y + GLOBAL_SCALE_MOVE * ROBOT_MOVECELLS_PER_SIZE / 2;
                    v.z = g_MatrixMap->GetZ(v.x, v.y);
                    CMatrixEffect::CreateMoveto(v);
                }
            }
        }
        obj = obj->GetNextLogic();
    }
    m_PlayerGroup[no].SetShowPlace(false);
}

void CMatrixSideUnit::PGAssignPlace(int no, CPoint center) {
    int i, u;
    byte mm = 0;
    CMatrixMapStatic *obj;
    SMatrixPlace *place;
    CMatrixRobotAI *rl[MAX_ROBOTS];
    int rlcnt = 0;
    int listcnt = 0;

    BufPrepare();

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot()) {
            if (obj->GetSide() == m_Id && GetGroupLogic(obj) == no) {
                rl[rlcnt] = (CMatrixRobotAI *)obj;
                mm |= 1 << (obj->AsRobot()->m_Unit[0].u1.s1.m_Kind - 1);
                rlcnt++;
            }
        }
        obj = obj->GetNextLogic();
    }
    if (rlcnt <= 0)
        return;

    int x = center.x, y = center.y;
    if (g_MatrixMap->PlaceFindNear(rl[0]->m_Unit[0].u1.s1.m_Kind - 1, 4, x, y, 0, NULL, NULL)) {
        center.x = x;
        center.y = y;
    }
    m_PlaceList[listcnt] = g_MatrixMap->FindNearPlace(mm, center);
    if (m_PlaceList[listcnt] < 0) {
        for (i = 0; i < rlcnt; i++)
            rl[i]->GetEnv()->m_PlaceNotFound = g_MatrixMap->GetTime();
        return;
    }
    listcnt++;
    if (rlcnt > 1 && g_MatrixMap->PlaceListGrow(mm, m_PlaceList, &listcnt, rlcnt - 1) <= 0) {
        for (i = 0; i < rlcnt; i++)
            rl[i]->GetEnv()->m_PlaceNotFound = g_MatrixMap->GetTime();
        return;
    }

    for (i = 0; i < listcnt; i++)
        g_MatrixMap->m_RN.m_Place[m_PlaceList[i]].m_Data = 0;
    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (IsLiveUnit(obj))
            ObjPlaceData(obj, 1);
        obj = obj->GetNextLogic();
    }

    for (i = 0; i < rlcnt; i++) {
        for (u = 0; u < listcnt; u++) {
            place = g_MatrixMap->m_RN.m_Place + m_PlaceList[u];
            if (place->m_Data)
                continue;
            if (!CanMove(place->m_Move, rl[i]))
                continue;
            break;
        }
        if (u < listcnt) {
            place->m_Data = 1;
            rl[i]->GetEnv()->m_Place = m_PlaceList[u];
            rl[i]->GetEnv()->m_PlaceAdd = CPoint(-1, -1);
        }
        else {
            if (g_MatrixMap->PlaceListGrow(mm, m_PlaceList, &listcnt, rlcnt) <= 0) {
                for (; i < rlcnt; i++)
                    rl[i]->GetEnv()->m_PlaceNotFound = g_MatrixMap->GetTime();
                return;
            }
            for (u = 0; u < listcnt; u++)
                g_MatrixMap->m_RN.m_Place[m_PlaceList[u]].m_Data = 0;
            obj = CMatrixMapStatic::GetFirstLogic();
            while (obj) {
                if (IsLiveUnit(obj))
                    ObjPlaceData(obj, 1);
                obj = obj->GetNextLogic();
            }
            i--;
            continue;
        }
    }

    /*    int u,x,y;
        CMatrixMapStatic * obj;

        int growsizemax=50;
        g_MatrixMap->m_RN.ActionDataPL(CRect(center.x-growsizemax,center.y-growsizemax,center.x+growsizemax,center.y+growsizemax),0);

        CRect
       plr=g_MatrixMap->m_RN.CorrectRectPL(CRect(center.x-growsizemax,center.y-growsizemax,center.x+growsizemax,center.y+growsizemax));

        obj = CMatrixMapStatic::GetFirstLogic();
        while(obj) {
            if(obj->IsLiveActiveCannon()) ObjPlaceData(obj,1);
            else if(obj->IsLiveRobot() && (obj->AsRobot()->GetSide()!=m_Id || obj->AsRobot()->GetGroupLogic()!=no))
       ObjPlaceData(obj,1); obj = obj->GetNextLogic();
        }

        obj = CMatrixMapStatic::GetFirstLogic();
        while(obj) {
            if(obj->IsLiveRobot() && (obj->AsRobot()->GetSide()==m_Id && obj->AsRobot()->GetGroupLogic()==no)) {
                CMatrixRobotAI * robot=(CMatrixRobotAI*)obj;
                int mindist=1000000000;
                robot->GetEnv()->m_Place=-1;

                CPoint tp=GetMapPos(obj);

                SMatrixPlaceList * plist=g_MatrixMap->m_RN.m_PLList+plr.left+plr.top*g_MatrixMap->m_RN.m_PLSizeX;
                for(y=plr.top;y<plr.bottom;y++,plist+=g_MatrixMap->m_RN.m_PLSizeX-(plr.right-plr.left)) {
                    for(x=plr.left;x<plr.right;x++,plist++) {
                        SMatrixPlace * place=g_MatrixMap->m_RN.m_Place+plist->m_Sme;
                        for(u=0;u<plist->m_Cnt;u++,place++) {
                            if(place->m_Data) continue; // Занетые места игнорируем
                            if(!CanMove(place->m_Move,robot)) continue; // Если робот не может стоять на этом месте, то
       пропускаем

                            int t=POW2(place->m_Pos.x-center.x)+POW2(place->m_Pos.y-center.y);
                            if(t<mindist) {
                                mindist=t;
                                robot->GetEnv()->m_Place=plist->m_Sme+u;
                            }
                        }
                    }
                }

                if(robot->GetEnv()->m_Place>=0) {
                    g_MatrixMap->m_RN.GetPlace(robot->GetEnv()->m_Place)->m_Data=1;
                }
            }

            obj = obj->GetNextLogic();
        }*/
}

void CMatrixSideUnit::PGAssignPlacePlayer(int no, const CPoint &center) {
    int other_cnt = 0;
    int other_size[200];
    CPoint other_des[200];

    CMatrixMapStatic *obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot()) {
            CMatrixRobotAI *r = (CMatrixRobotAI *)obj;
            if (r->GetSide() != m_Id || r->GetGroupLogic() != no) {
                if (r->GetEnv()->m_Place >= 0) {
                    ASSERT(other_cnt < 200);

                    other_size[other_cnt] = 4;
                    other_des[other_cnt] = g_MatrixMap->m_RN.GetPlace(r->GetEnv()->m_Place)->m_Pos;
                    other_cnt++;
                }
                else if (r->GetEnv()->m_PlaceAdd.x >= 0) {
                    ASSERT(other_cnt < 200);

                    other_size[other_cnt] = 4;
                    other_des[other_cnt] = r->GetEnv()->m_PlaceAdd;
                    other_cnt++;
                }
            }
        }
        else if (obj->IsLiveCannon()) {
            ASSERT(other_cnt < 200);

            other_size[other_cnt] = 4;
            other_des[other_cnt] = g_MatrixMap->m_RN.GetPlace(obj->AsCannon()->m_Place)->m_Pos;
            other_cnt++;
        }
        obj = obj->GetNextLogic();
    }

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj->GetSide() == m_Id && GetGroupLogic(obj) == no) {
            CMatrixRobotAI *r = (CMatrixRobotAI *)obj;
            int mx = center.x;
            int my = center.y;
            g_MatrixMap->PlaceFindNear(r->m_Unit[0].u1.s1.m_Kind - 1, 4, mx, my, other_cnt, other_size, other_des);
            PGSetPlace(r, CPoint(mx, my));

            if (r->GetEnv()->m_Place >= 0) {
                ASSERT(other_cnt < 200);

                other_size[other_cnt] = 4;
                other_des[other_cnt] = g_MatrixMap->m_RN.GetPlace(r->GetEnv()->m_Place)->m_Pos;
                other_cnt++;
            }
            else if (r->GetEnv()->m_PlaceAdd.x >= 0) {
                ASSERT(other_cnt < 200);

                other_size[other_cnt] = 4;
                other_des[other_cnt] = r->GetEnv()->m_PlaceAdd;
                other_cnt++;
            }

            //            obj->AsRobot()->MoveTo(mx, my);
            //            obj->AsRobot()->GetEnv()->m_Place=-1;
        }
        obj = obj->GetNextLogic();
    }
}

void CMatrixSideUnit::PGCalcStat() {
    CMatrixMapStatic *ms;
    int i, u, t;  //,cnt,sme,dist;
    CPoint tp;

    if (m_Region == NULL)
        m_Region = (SMatrixLogicRegion *)HAllocClear(sizeof(SMatrixLogicRegion) * g_MatrixMap->m_RN.m_RegionCnt,
                                                     g_MatrixHeap);
    if (m_RegionIndex == NULL)
        m_RegionIndex = (int *)HAllocClear(sizeof(int) * g_MatrixMap->m_RN.m_RegionCnt, g_MatrixHeap);

    for (i = 0; i < g_MatrixMap->m_RN.m_RegionCnt; i++) {
        m_Region[i].m_EnemyRobotCnt = 0;
        m_Region[i].m_EnemyCannonCnt = 0;
        m_Region[i].m_EnemyBuildingCnt = 0;
        m_Region[i].m_EnemyBaseCnt = 0;
        m_Region[i].m_NeutralCannonCnt = 0;
        m_Region[i].m_NeutralBuildingCnt = 0;
        m_Region[i].m_NeutralBaseCnt = 0;
        m_Region[i].m_OurRobotCnt = 0;
        m_Region[i].m_OurCannonCnt = 0;
        m_Region[i].m_OurBuildingCnt = 0;
        m_Region[i].m_OurBaseCnt = 0;
        m_Region[i].m_EnemyRobotDist = -1;
        m_Region[i].m_EnemyBuildingDist = -1;
        m_Region[i].m_OurBaseDist = -1;
        m_Region[i].m_Danger = 0;
        m_Region[i].m_DangerAdd = 0;
    }

    ms = CMatrixMapStatic::GetFirstLogic();
    while (ms) {
        switch (ms->GetObjectType()) {
            case OBJECT_TYPE_BUILDING:
                i = GetRegion(int(((CMatrixBuilding *)ms)->m_Pos.x / GLOBAL_SCALE_MOVE),
                              int(((CMatrixBuilding *)ms)->m_Pos.y / GLOBAL_SCALE_MOVE));
                if (i >= 0) {
                    if (((CMatrixBuilding *)ms)->m_Side == 0)
                        m_Region[i].m_NeutralBuildingCnt++;
                    else if (((CMatrixBuilding *)ms)->m_Side != m_Id)
                        m_Region[i].m_EnemyBuildingCnt++;
                    else
                        m_Region[i].m_OurBuildingCnt++;

                    if (((CMatrixBuilding *)ms)->m_Kind == 0) {
                        if (((CMatrixBuilding *)ms)->m_Side == 0)
                            m_Region[i].m_NeutralBaseCnt++;
                        else if (((CMatrixBuilding *)ms)->m_Side != m_Id)
                            m_Region[i].m_EnemyBaseCnt++;
                        else
                            m_Region[i].m_OurBaseCnt++;
                    }
                }
                break;
            case OBJECT_TYPE_ROBOTAI:

                if (((CMatrixRobotAI *)ms)->m_CurrState != ROBOT_DIP) {
                    tp.x = int(ms->AsRobot()->m_PosX / GLOBAL_SCALE_MOVE);
                    tp.y = int(ms->AsRobot()->m_PosY / GLOBAL_SCALE_MOVE);
                    i = GetRegion(tp);

                    if (i >= 0) {
                        if (ms->AsRobot()->m_Side == 0)
                            ;
                        else if (ms->AsRobot()->m_Side != m_Id) {
                            m_Region[i].m_EnemyRobotCnt++;
                            float d = ms->AsRobot()->GetStrength();
                            m_Region[i].m_Danger += d;
                        }
                        else
                            m_Region[i].m_OurRobotCnt++;
                    }
                }
                break;

            case OBJECT_TYPE_CANNON:
                tp.x = int(((CMatrixCannon *)ms)->m_Pos.x / GLOBAL_SCALE_MOVE);
                tp.y = int(((CMatrixCannon *)ms)->m_Pos.y / GLOBAL_SCALE_MOVE);
                i = GetRegion(tp);
                if (i >= 0) {
                    if (ms->GetSide() == 0) {
                        m_Region[i].m_NeutralCannonCnt++;
                        m_Region[i].m_DangerAdd += ((CMatrixCannon *)ms)->GetStrength();
                    }
                    else if (ms->GetSide() != m_Id) {
                        m_Region[i].m_EnemyCannonCnt++;
                        m_Region[i].m_DangerAdd += ((CMatrixCannon *)ms)->GetStrength();
                    }
                    else
                        m_Region[i].m_OurCannonCnt++;
                }
                break;
        }
        ms = ms->GetNextLogic();
    }

    // Выращиваем опасность
    for (i = 0; i < g_MatrixMap->m_RN.m_RegionCnt; i++) {
        if (m_Region[i].m_Danger <= 0)
            continue;

        SMatrixLogicRegion *lr = m_Region;
        for (u = 0; u < g_MatrixMap->m_RN.m_RegionCnt; u++, lr++)
            lr->m_Data = 0;

        for (u = 0; u < g_MatrixMap->m_RN.m_Region[i].m_NearCnt; u++) {
            t = g_MatrixMap->m_RN.m_Region[i].m_Near[u];
            if (m_Region[t].m_Data > 0)
                continue;

            m_Region[t].m_DangerAdd += m_Region[i].m_Danger;
            m_Region[t].m_Data = 1;
        }

        /*        cnt=0; sme=0; dist=0;
                m_RegionIndex[cnt]=i;
                m_Region[i].m_Data=3;
                cnt++;

                while(sme<cnt) {
                    for(u=0;u<g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt;u++) {
                        t=g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_Near[u];
                        if(m_Region[t].m_Data>0) continue;

                        m_Region[t].m_DangerAdd+=m_Region[i].m_Danger;

                        if(m_Region[t].m_Danger>0) {
                            m_RegionIndex[cnt]=t;
                            m_Region[t].m_Data=3;
                            cnt++;
                        } else {
                            m_Region[t].m_Data=m_Region[m_RegionIndex[sme]].m_Data-1;
                            if(m_Region[t].m_Data>=2) {
                                m_RegionIndex[cnt]=t;
                                cnt++;
                            }
                        }
                    }
                    sme++;
                }*/
    }
    for (i = 0; i < g_MatrixMap->m_RN.m_RegionCnt; i++)
        m_Region[i].m_Danger += m_Region[i].m_DangerAdd;
}

void CMatrixSideUnit::PGFindCaptureFactory(int no) {
    PGCalcStat();

    int u, t, k, cnt, sme, dist, next;
    CMatrixMapStatic *obj;
    byte mm = 0;

    cnt = 0;
    sme = 0;
    dist = 0;

    int regionmass = g_MatrixMap->GetRegion(PGCalcCenterGroup(no));
    float strength = 0;

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && (obj->AsRobot()->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == no)) {
            CMatrixRobotAI *robot = (CMatrixRobotAI *)obj;

            mm |= 1 << (robot->m_Unit[0].u1.s1.m_Kind - 1);

            //            robot->CalcStrength();
            strength += robot->GetStrength();
            if (regionmass < 0)
                regionmass = g_MatrixMap->GetRegion(GetMapPos(robot));
        }
        obj = obj->GetNextLogic();
    }
    if (regionmass < 0)
        return;

    // В текущем регионе
    if (strength >= m_Region[regionmass].m_Danger * 1.0) {  // Если регион слишком опасный, то пропускаем
        obj = CMatrixMapStatic::GetFirstLogic();
        while (obj) {
            if (obj->IsLiveBuilding() && (obj->GetSide() != m_Id) && GetRegion(obj) == regionmass) {
                m_PlayerGroup[no].m_Obj = obj;

                m_PlayerGroup[no].m_RegionPathCnt = 0;
                m_PlayerGroup[no].m_RoadPath->ClearFast();

                return;
            }
            obj = obj->GetNextLogic();
        }
    }

    // В других регионах (безопасных/опасных)
    for (int type = 0; type <= 1; type++) {
        for (u = 0; u < g_MatrixMap->m_RN.m_RegionCnt; u++) {
            m_Region[u].m_Data = 0;
        }

        cnt = 0;
        sme = 0;
        m_RegionIndex[cnt] = regionmass;
        m_Region[regionmass].m_Data = 1;
        cnt++;

        next = cnt;
        dist = 1;
        while (sme < cnt) {
            for (t = 0; t < g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt; t++) {
                u = g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_Near[t];
                if (m_Region[u].m_Data)
                    continue;

                if (g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearMove[t] & mm)
                    continue;

                if (type == 0 && strength < m_Region[u].m_Danger * 1.0)
                    continue;  // Если регион слишком опасный, то пропускаем

                m_Region[u].m_Data = 1 + dist;
                m_RegionIndex[cnt] = u;
                cnt++;

                // Группы не должны идти в один регион
                for (k = 0; k < MAX_LOGIC_GROUP; k++) {
                    if (k == no)
                        continue;
                    if (m_PlayerGroup[k].m_RobotCnt <= 0)
                        continue;
                    if (m_PlayerGroup[k].Order() != mpo_Capture && m_PlayerGroup[k].Order() != mpo_AutoCapture)
                        continue;
                    if (!m_PlayerGroup[k].m_Obj)
                        continue;
                    if (!m_PlayerGroup[k].m_Obj->IsLiveBuilding())
                        continue;

                    if (u != GetRegion(m_PlayerGroup[k].m_Obj))
                        continue;
                    break;
                }
                if (k < MAX_LOGIC_GROUP)
                    continue;

                if (m_Region[u].m_EnemyBuildingCnt > 0 || m_Region[u].m_NeutralBuildingCnt > 0) {
                    obj = CMatrixMapStatic::GetFirstLogic();
                    while (obj) {
                        if (obj->IsLiveBuilding() && (obj->GetSide() != m_Id) && GetRegion(obj) == u) {
                            m_PlayerGroup[no].m_Obj = obj;
                            PGCalcRegionPath(m_PlayerGroup + no, u, mm);

                            m_PlayerGroup[no].m_RoadPath->ClearFast();
                            if (m_PlayerGroup[no].m_RegionPathCnt > 1) {
                                g_MatrixMap->m_RN.FindPathFromRegionPath(mm, m_PlayerGroup[no].m_RegionPathCnt,
                                                                         m_PlayerGroup[no].m_RegionPath,
                                                                         m_PlayerGroup[no].m_RoadPath);
                            }

                            return;
                        }
                        obj = obj->GetNextLogic();
                    }
                }
            }
            sme++;
            if (sme >= next) {
                next = cnt;
                dist++;
            }
        }
    }
}

void CMatrixSideUnit::PGFindAttackTarget(int no) {
    PGCalcStat();

    int u, t, k, cnt, sme, dist, next;
    CMatrixMapStatic *obj;
    byte mm = 0;

    cnt = 0;
    sme = 0;
    dist = 0;

    int regionmass = g_MatrixMap->GetRegion(PGCalcCenterGroup(no));
    float strength = 0;

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && (obj->AsRobot()->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == no)) {
            CMatrixRobotAI *robot = (CMatrixRobotAI *)obj;

            mm |= 1 << (robot->m_Unit[0].u1.s1.m_Kind - 1);

            //            robot->CalcStrength();
            strength += robot->GetStrength();
            if (regionmass < 0)
                regionmass = g_MatrixMap->GetRegion(GetMapPos(robot));
        }
        obj = obj->GetNextLogic();
    }
    if (regionmass < 0)
        return;

    // В текущем регионе
    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (IsLiveUnit(obj) && (obj->GetSide() != m_Id) && GetRegion(obj) == regionmass) {
            m_PlayerGroup[no].m_Obj = obj;

            m_PlayerGroup[no].m_RegionPathCnt = 0;
            m_PlayerGroup[no].m_RoadPath->ClearFast();

            return;
        }
        obj = obj->GetNextLogic();
    }

    // В других регионах
    for (int withourgroup = 0; withourgroup <= 1; withourgroup++) {  // (с учетом/без учета своих групп)
        for (u = 0; u < g_MatrixMap->m_RN.m_RegionCnt; u++) {
            m_Region[u].m_Data = 0;
        }

        cnt = 0;
        sme = 0;
        m_RegionIndex[cnt] = regionmass;
        m_Region[regionmass].m_Data = 1;
        cnt++;

        next = cnt;
        dist = 1;
        while (sme < cnt) {
            for (t = 0; t < g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt; t++) {
                u = g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_Near[t];
                if (m_Region[u].m_Data)
                    continue;

                if (g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearMove[t] & mm)
                    continue;

                m_Region[u].m_Data = 1 + dist;
                m_RegionIndex[cnt] = u;
                cnt++;

                if (withourgroup == 0) {
                    // Группы не должны идти в один регион
                    for (k = 0; k < MAX_LOGIC_GROUP; k++) {
                        if (k == no)
                            continue;
                        if (m_PlayerGroup[k].m_RobotCnt <= 0)
                            continue;
                        if (m_PlayerGroup[k].Order() != mpo_Attack && m_PlayerGroup[k].Order() != mpo_AutoAttack)
                            continue;
                        if (!m_PlayerGroup[k].m_Obj)
                            continue;
                        if (!IsLiveUnit(m_PlayerGroup[k].m_Obj))
                            continue;

                        if (u != GetRegion(m_PlayerGroup[k].m_Obj))
                            continue;
                        break;
                    }
                    if (k < MAX_LOGIC_GROUP)
                        continue;
                }

                if (m_Region[u].m_EnemyRobotCnt > 0 || m_Region[u].m_EnemyCannonCnt > 0 ||
                    m_Region[u].m_NeutralCannonCnt > 0) {
                    obj = CMatrixMapStatic::GetFirstLogic();
                    while (obj) {
                        if (IsLiveUnit(obj) && (obj->GetSide() != m_Id) && GetRegion(obj) == u) {
                            m_PlayerGroup[no].m_Obj = obj;
                            PGCalcRegionPath(m_PlayerGroup + no, u, mm);

                            m_PlayerGroup[no].m_RoadPath->ClearFast();
                            if (m_PlayerGroup[no].m_RegionPathCnt > 1) {
                                g_MatrixMap->m_RN.FindPathFromRegionPath(mm, m_PlayerGroup[no].m_RegionPathCnt,
                                                                         m_PlayerGroup[no].m_RegionPath,
                                                                         m_PlayerGroup[no].m_RoadPath);
                            }

                            return;
                        }
                        obj = obj->GetNextLogic();
                    }
                }
            }
            sme++;
            if (sme >= next) {
                next = cnt;
                dist++;
            }
        }
    }

    // Если нет врагов идем уничтожать вражескую базу
    for (u = 0; u < g_MatrixMap->m_RN.m_RegionCnt; u++) {
        m_Region[u].m_Data = 0;
    }

    cnt = 0;
    sme = 0;
    m_RegionIndex[cnt] = regionmass;
    m_Region[regionmass].m_Data = 1;
    cnt++;

    next = cnt;
    dist = 1;
    while (sme < cnt) {
        for (t = 0; t < g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt; t++) {
            u = g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_Near[t];
            if (m_Region[u].m_Data)
                continue;

            if (g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearMove[t] & mm)
                continue;

            m_Region[u].m_Data = 1 + dist;
            m_RegionIndex[cnt] = u;
            cnt++;

            if (m_Region[u].m_EnemyBaseCnt > 0) {
                obj = CMatrixMapStatic::GetFirstLogic();
                while (obj) {
                    if (obj->IsLiveBuilding() && obj->IsBase() && (obj->GetSide() != m_Id) && GetRegion(obj) == u) {
                        m_PlayerGroup[no].m_Obj = obj;
                        PGCalcRegionPath(m_PlayerGroup + no, u, mm);

                        m_PlayerGroup[no].m_RoadPath->ClearFast();
                        if (m_PlayerGroup[no].m_RegionPathCnt > 1) {
                            g_MatrixMap->m_RN.FindPathFromRegionPath(mm, m_PlayerGroup[no].m_RegionPathCnt,
                                                                     m_PlayerGroup[no].m_RegionPath,
                                                                     m_PlayerGroup[no].m_RoadPath);
                        }

                        return;
                    }
                    obj = obj->GetNextLogic();
                }
            }
        }
        sme++;
        if (sme >= next) {
            next = cnt;
            dist++;
        }
    }
}

void CMatrixSideUnit::PGFindDefenceTarget(int no) {
    PGCalcStat();

    int u, t, k, cnt, sme, dist, next;
    CMatrixMapStatic *obj;
    byte mm;

    cnt = 0;
    sme = 0;
    dist = 0;

    int regionmass = g_MatrixMap->GetRegion(PGCalcCenterGroup(no));
    float strength = 0;

    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && (obj->AsRobot()->GetSide() == m_Id && obj->AsRobot()->GetGroupLogic() == no)) {
            CMatrixRobotAI *robot = (CMatrixRobotAI *)obj;

            mm |= 1 << (robot->m_Unit[0].u1.s1.m_Kind - 1);

            //            robot->CalcStrength();
            strength += robot->GetStrength();
            if (regionmass < 0)
                regionmass = g_MatrixMap->GetRegion(GetMapPos(robot));
        }
        obj = obj->GetNextLogic();
    }
    if (regionmass < 0)
        return;

    // В текущем регионе
    obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (IsLiveUnit(obj) && (obj->GetSide() != m_Id) && GetRegion(obj) == regionmass) {
            m_PlayerGroup[no].m_Obj = obj;
            m_PlayerGroup[no].m_Region = regionmass;

            m_PlayerGroup[no].m_RegionPathCnt = 0;
            m_PlayerGroup[no].m_RoadPath->ClearFast();

            return;
        }
        obj = obj->GetNextLogic();
    }

    // От наших баз/От текущего места
    // с учетом/без учета своих групп
    for (int type = 0; type <= 3; type++) {
        cnt = 0;
        sme = 0;
        if (type == 0 || type == 1) {
            for (u = 0; u < g_MatrixMap->m_RN.m_RegionCnt; u++) {
                m_Region[u].m_Data = 0;

                if (m_Region[u].m_OurBaseCnt > 0) {
                    m_RegionIndex[cnt] = u;
                    m_Region[u].m_Data = 1;
                    cnt++;
                }
            }
        }
        else if (type == 2 || type == 3) {
            for (u = 0; u < g_MatrixMap->m_RN.m_RegionCnt; u++) {
                m_Region[u].m_Data = 0;
            }

            m_RegionIndex[cnt] = regionmass;
            m_Region[regionmass].m_Data = 1;
            cnt++;
        }

        next = cnt;
        dist = 1;
        while (sme < cnt) {
            for (t = 0; t < g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt; t++) {
                u = g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_Near[t];
                if (m_Region[u].m_Data)
                    continue;

                if (g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearMove[t] & mm)
                    continue;

                if (m_Region[u].m_EnemyBuildingCnt > 0)
                    continue;
                if (m_Region[u].m_EnemyCannonCnt > 0)
                    continue;

                m_Region[u].m_Data = 1 + dist;
                m_RegionIndex[cnt] = u;
                cnt++;

                if (type == 0 || type == 2) {
                    // Группы не должны идти в один регион
                    for (k = 0; k < MAX_LOGIC_GROUP; k++) {
                        if (k == no)
                            continue;
                        if (m_PlayerGroup[k].m_RobotCnt <= 0)
                            continue;
                        if (m_PlayerGroup[k].Order() != mpo_Attack && m_PlayerGroup[k].Order() != mpo_AutoAttack)
                            continue;
                        if (!m_PlayerGroup[k].m_Obj)
                            continue;
                        if (!IsLiveUnit(m_PlayerGroup[k].m_Obj))
                            continue;

                        if (u != GetRegion(m_PlayerGroup[k].m_Obj))
                            continue;
                        break;
                    }
                    if (k < MAX_LOGIC_GROUP)
                        continue;
                }

                if (m_Region[u].m_OurBuildingCnt > 0 || m_Region[u].m_OurCannonCnt > 0) {
                    obj = CMatrixMapStatic::GetFirstLogic();
                    while (obj) {
                        if (obj->IsLiveRobot() && (obj->GetSide() != m_Id)) {
                            CPoint tp;
                            if (!obj->AsRobot()->GetMoveToCoords(tp))
                                tp = GetMapPos(obj);
                            int reg = g_MatrixMap->GetRegion(tp);

                            if(reg>=0 && reg==u /*&& (GetRegion(obj)==reg || g_MatrixMap->m_RN.IsNerestRegion(reg,GetRegion(obj)))*/) {
                                m_PlayerGroup[no].m_Region = reg;
                                m_PlayerGroup[no].m_Obj = obj;
                                PGCalcRegionPath(m_PlayerGroup + no, u, mm);

                                m_PlayerGroup[no].m_RoadPath->ClearFast();
                                if (m_PlayerGroup[no].m_RegionPathCnt > 1) {
                                    g_MatrixMap->m_RN.FindPathFromRegionPath(mm, m_PlayerGroup[no].m_RegionPathCnt,
                                                                             m_PlayerGroup[no].m_RegionPath,
                                                                             m_PlayerGroup[no].m_RoadPath);
                                }

                                return;
                            }
                        }
                        obj = obj->GetNextLogic();
                    }
                }
            }
            sme++;
            if (sme >= next) {
                next = cnt;
                dist++;
            }
        }
    }
}

void CMatrixSideUnit::PGCalcRegionPath(SMatrixPlayerGroup *pg, int rend, byte mm) {
    int t, u, i;

    pg->m_RegionPathCnt = 0;
    pg->m_RegionPath[REGION_PATH_MAX_CNT - 1 - pg->m_RegionPathCnt] = rend;
    pg->m_RegionPathCnt++;

    DWORD level = m_Region[rend].m_Data;

    while (true) {
        i = -1;
        for (t = 0; t < g_MatrixMap->m_RN.m_Region[rend].m_NearCnt; t++) {
            u = g_MatrixMap->m_RN.m_Region[rend].m_Near[t];
            if (!m_Region[u].m_Data)
                continue;
            if (m_Region[u].m_Data >= level)
                continue;

            if (g_MatrixMap->m_RN.m_Region[rend].m_NearMove[t] & mm)
                continue;

            i = u;
            break;
        }
        if (i < 0)
            ERROR_E;

        ASSERT(pg->m_RegionPathCnt < REGION_PATH_MAX_CNT);

        pg->m_RegionPath[REGION_PATH_MAX_CNT - 1 - pg->m_RegionPathCnt] = i;
        pg->m_RegionPathCnt++;

        rend = i;
        level = m_Region[rend].m_Data;

        if (level <= 1)
            break;
    }

    if (pg->m_RegionPathCnt < REGION_PATH_MAX_CNT)
        MoveMemory(pg->m_RegionPath, pg->m_RegionPath + REGION_PATH_MAX_CNT - pg->m_RegionPathCnt,
                   pg->m_RegionPathCnt * sizeof(int));

    // CHelper::DestroyByGroup(101);
    // for(i=1;i<pg->m_RegionPathCnt;i++) {
    //    D3DXVECTOR3 v1,v2;
    //    v1.x=g_MatrixMap->m_RN.m_Region[pg->m_RegionPath[i-1]].m_Center.x*GLOBAL_SCALE_MOVE;
    //    v1.y=g_MatrixMap->m_RN.m_Region[pg->m_RegionPath[i-1]].m_Center.y*GLOBAL_SCALE_MOVE;
    //    v1.z=g_MatrixMap->GetZ(v1.x,v1.y)+150.0f;
    //    v2.x=g_MatrixMap->m_RN.m_Region[pg->m_RegionPath[i]].m_Center.x*GLOBAL_SCALE_MOVE;
    //    v2.y=g_MatrixMap->m_RN.m_Region[pg->m_RegionPath[i]].m_Center.y*GLOBAL_SCALE_MOVE;
    //    v2.z=g_MatrixMap->GetZ(v2.x,v2.y)+150.0f;
    //    CHelper::Create(0,101)->Cone(v1,v2,1.0f,1.0f,0xff00ff00,0xff00ff00,3);
    //}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

inline bool PrepareBreakOrder(CMatrixMapStatic *robot) {
    return !(((CMatrixRobotAI *)robot)->GetEnv()->m_OrderNoBreak = !robot->AsRobot()->CanBreakOrder());
}

inline bool IsLiveUnit(CMatrixMapStatic *obj) {
    if (obj->GetObjectType() == OBJECT_TYPE_ROBOTAI)
        return obj->AsRobot()->m_CurrState !=
               ROBOT_DIP;  // && (obj->AsRobot()->GetSide()!=PLAYER_SIDE || !obj->AsRobot()->IsSelected()) &&
                           // (g_MatrixMap->GetPlayerSide()->GetArcadedObject() != obj);
    else if (obj->GetObjectType() == OBJECT_TYPE_CANNON)
        return obj->AsCannon()->m_CurrState != CANNON_DIP && obj->AsCannon()->m_CurrState != CANNON_UNDER_CONSTRUCTION;
    else
        return false;
}

inline CPoint GetMapPos(CMatrixMapStatic *obj) {
    switch (obj->GetObjectType()) {
        case OBJECT_TYPE_ROBOTAI:
            return CPoint(obj->AsRobot()->GetMapPosX(), obj->AsRobot()->GetMapPosY());
        case OBJECT_TYPE_CANNON:
            return CPoint(int(((CMatrixCannon *)obj)->m_Pos.x / GLOBAL_SCALE_MOVE),
                          int(((CMatrixCannon *)obj)->m_Pos.y / GLOBAL_SCALE_MOVE));
        case OBJECT_TYPE_BUILDING:
            return CPoint(int(((CMatrixBuilding *)obj)->m_Pos.x / GLOBAL_SCALE_MOVE),
                          int(((CMatrixBuilding *)obj)->m_Pos.y / GLOBAL_SCALE_MOVE));
    }
    if (obj->GetObjectType() == OBJECT_TYPE_MAPOBJECT && obj->IsSpecial()) {
        return CPoint(int(((CMatrixMapObject *)obj)->GetGeoCenter().x / GLOBAL_SCALE_MOVE),
                      int(((CMatrixBuilding *)obj)->GetGeoCenter().y / GLOBAL_SCALE_MOVE));
    }
    ERROR_E;
}

inline D3DXVECTOR2 GetWorldPos(CMatrixMapStatic *obj) {
    switch (obj->GetObjectType()) {
        case OBJECT_TYPE_ROBOTAI:
            return D3DXVECTOR2(obj->AsRobot()->m_PosX, obj->AsRobot()->m_PosY);
        case OBJECT_TYPE_CANNON:
            return obj->AsCannon()->m_Pos;
        case OBJECT_TYPE_BUILDING:
            return obj->AsBuilding()->m_Pos;
    }
    ERROR_E;
}

inline bool IsToPlace(CMatrixRobotAI *robot, int place) {
    if (place < 0)
        return false;

    SMatrixPlace *pl = GetPlacePtr(place);

    CPoint tp;
    if (robot->GetMoveToCoords(tp)) {
        if (robot->FindOrderLikeThat(ROT_CAPTURE_FACTORY))
            return false;
        else if ((pl->m_Pos.x == tp.x) && (pl->m_Pos.y == tp.y))
            return true;
        else if (robot->GetReturnCoords(tp) && (pl->m_Pos.x == tp.x) && (pl->m_Pos.y == tp.y))
            return true;
        else
            return false;
    }
    else {
        if (robot->GetReturnCoords(tp) && (pl->m_Pos.x == tp.x) && (pl->m_Pos.y == tp.y))
            return true;

        return (robot->GetMapPosX() == pl->m_Pos.x) && (robot->GetMapPosY() == pl->m_Pos.y);
        // return fabs(robot->m_PosX-GLOBAL_SCALE_MOVE*(ROBOT_MOVECELLS_PER_SIZE>>1)-GLOBAL_SCALE_MOVE*pl->m_Pos.x)<0.9f
        // && fabs(robot->m_PosY-GLOBAL_SCALE_MOVE*(ROBOT_MOVECELLS_PER_SIZE>>1)-GLOBAL_SCALE_MOVE*pl->m_Pos.y)<0.9f;
    }
}

inline bool IsInPlace(CMatrixRobotAI *robot, int place) {
    SMatrixPlace *pl = GetPlacePtr(place);

    if (pl == NULL)
        return false;

    if (robot->FindOrderLikeThat(ROT_MOVE_TO))
        return false;

    return (robot->GetMapPosX() == pl->m_Pos.x) && (robot->GetMapPosY() == pl->m_Pos.y);
}

inline bool IsInPlace(CMatrixRobotAI *robot) {
    return IsInPlace(robot, RobotPlace(robot));
}

inline int RobotPlace(CMatrixRobotAI *robot) {
    return robot->GetEnv()->m_Place;
    //    if(IsToPlace(robot,robot->GetEnv()->m_Place)) return robot->GetEnv()->m_Place;
    //    else return -1;
}

inline int CannonPlace(CMatrixCannon *cannon) {
    return cannon->m_Place;
}

inline int ObjPlace(CMatrixMapStatic *obj) {
    if (obj->IsRobot())
        return RobotPlace(obj->AsRobot());
    else if (obj->IsCannon())
        return CannonPlace(obj->AsCannon());
    ERROR_E;
}

inline SMatrixPlace *ObjPlacePtr(CMatrixMapStatic *obj) {
    int i = ObjPlace(obj);
    if (i >= 0)
        return GetPlacePtr(i);
    else
        return NULL;
}

inline dword ObjPlaceData(CMatrixMapStatic *obj) {
    int i = ObjPlace(obj);
    if (i >= 0)
        return GetPlacePtr(i)->m_Data;
    else
        return 0;
}

inline void ObjPlaceData(CMatrixMapStatic *obj, dword data) {
    int i = ObjPlace(obj);
    if (i >= 0)
        GetPlacePtr(i)->m_Data = data;
}

inline bool CanMove(byte movetype, CMatrixRobotAI *robot) {
    return !(movetype & (1 << (robot->m_Unit[0].u1.s1.m_Kind - 1)));
}

inline int GetDesRegion(CMatrixRobotAI *robot) {
    SMatrixPlace *pl = ObjPlacePtr(robot);
    if (!pl)
        return -1;
    return pl->m_Region;
    /*    CPoint tp;

        if(robot->GetMoveToCoords(tp)) return GetRegion(tp);
        else return GetRegion(robot->GetMapPosX(),robot->GetMapPosY());*/
}

inline int GetRegion(const CPoint &tp) {
    return g_MatrixMap->GetRegion(tp);
}

inline int GetRegion(int x, int y) {
    return g_MatrixMap->GetRegion(x, y);
}

inline int GetRegion(CMatrixMapStatic *obj) {
    return GetRegion(GetMapPos(obj));
}

inline D3DXVECTOR3 PointOfAim(CMatrixMapStatic *obj) {
    D3DXVECTOR3 p;

    if (obj->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
        p = obj->GetGeoCenter();
        p.z += 5.0f;
    }
    else if (obj->GetObjectType() == OBJECT_TYPE_CANNON) {
        p = obj->GetGeoCenter();
        p.z += 5.0f;

        /*        p.x=((CMatrixCannon *)(obj))->m_Pos.x;
                p.y=((CMatrixCannon *)(obj))->m_Pos.y;
                p.z=g_MatrixMap->GetZ(p.x,p.y)+25.0f;*/
    }
    else if (obj->GetObjectType() == OBJECT_TYPE_BUILDING) {
        p = obj->GetGeoCenter();
        p.z = g_MatrixMap->GetZ(p.x, p.y) + 20.0f;
    }
    else
        ASSERT(0);

    return p;
}

inline bool PLIsToPlace(CMatrixRobotAI *robot) {
    CPoint tp, ptp;
    if (robot->GetEnv()->m_Place >= 0) {
        ptp = GetPlacePtr(robot->GetEnv()->m_Place)->m_Pos;
    }
    else if (robot->GetEnv()->m_PlaceAdd.x >= 0) {
        ptp = robot->GetEnv()->m_PlaceAdd;
    }
    else
        return false;

    if (robot->GetMoveToCoords(tp)) {
        if (robot->FindOrderLikeThat(ROT_CAPTURE_FACTORY))
            return false;
        else if ((ptp.x == tp.x) && (ptp.y == tp.y))
            return true;
        else if (robot->GetReturnCoords(tp) && (ptp.x == tp.x) && (ptp.y == tp.y))
            return true;
        else
            return false;
    }
    else {
        if (robot->GetReturnCoords(tp) && (ptp.x == tp.x) && (ptp.y == tp.y))
            return true;

        return (robot->GetMapPosX() == ptp.x) && (robot->GetMapPosY() == ptp.y);
    }
}

inline CPoint PLPlacePos(CMatrixRobotAI *robot) {
    if (robot->GetEnv()->m_Place >= 0) {
        return GetPlacePtr(robot->GetEnv()->m_Place)->m_Pos;
    }
    else if (robot->GetEnv()->m_PlaceAdd.x >= 0) {
        return robot->GetEnv()->m_PlaceAdd;
    }
    else
        return CPoint(-1, -1);
}

void CMatrixSideUnit::CalcRegionPath(SMatrixLogicAction *ac, int rend, byte mm) {
    int t, u, i;

    ac->m_RegionPathCnt = 0;
    ac->m_RegionPath[REGION_PATH_MAX_CNT - 1 - ac->m_RegionPathCnt] = rend;
    ac->m_RegionPathCnt++;

    DWORD level = m_Region[rend].m_Data;

    while (true) {
        i = -1;
        for (t = 0; t < g_MatrixMap->m_RN.m_Region[rend].m_NearCnt; t++) {
            u = g_MatrixMap->m_RN.m_Region[rend].m_Near[t];
            if (!m_Region[u].m_Data)
                continue;
            if (m_Region[u].m_Data >= level)
                continue;

            if (g_MatrixMap->m_RN.m_Region[rend].m_NearMove[t] & mm)
                continue;

            i = u;
            break;
        }
        if (i < 0)
            ERROR_E;

        ASSERT(ac->m_RegionPathCnt < REGION_PATH_MAX_CNT);

        ac->m_RegionPath[REGION_PATH_MAX_CNT - 1 - ac->m_RegionPathCnt] = i;
        ac->m_RegionPathCnt++;

        rend = i;
        level = m_Region[rend].m_Data;

        if (level <= 1)
            break;
    }

    if (ac->m_RegionPathCnt < REGION_PATH_MAX_CNT)
        MoveMemory(ac->m_RegionPath, ac->m_RegionPath + REGION_PATH_MAX_CNT - ac->m_RegionPathCnt,
                   ac->m_RegionPathCnt * sizeof(int));
}

bool CMatrixSideUnit::CanMoveNoEnemy(byte mm, int r1, int r2) {
    int u, t, sme, cnt, dist, dist2, next;
    for (u = 0; u < g_MatrixMap->m_RN.m_RegionCnt; u++)
        m_Region[u].m_Data = 0;

    sme = 0;
    cnt = 0;
    dist = 1;
    m_RegionIndex[cnt] = r1;
    m_Region[r1].m_Data = dist;
    cnt++;
    dist++;

    next = cnt;

    while (sme < cnt) {
        for (t = 0; t < g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt; t++) {
            u = g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_Near[t];
            if (m_Region[u].m_Data)
                continue;

            if (g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearMove[t] & mm)
                continue;

            if (u == r2)
                break;

            m_RegionIndex[cnt] = u;
            m_Region[u].m_Data = dist;
            cnt++;
        }
        if (t < g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt)
            break;

        sme++;
        if (sme >= next) {
            next = cnt;
            dist++;
        }
    }
    if (sme >= cnt)
        return false;

    for (u = 0; u < g_MatrixMap->m_RN.m_RegionCnt; u++)
        m_Region[u].m_Data = 0;

    sme = 0;
    cnt = 0;
    dist2 = 1;
    m_RegionIndex[cnt] = r1;
    m_Region[r1].m_Data = dist2;
    cnt++;
    dist2++;

    next = cnt;

    while (sme < cnt) {
        for (t = 0; t < g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt; t++) {
            u = g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_Near[t];
            if (m_Region[u].m_Data)
                continue;

            if (g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearMove[t] & mm)
                continue;

            if (u == r2)
                break;

            if (m_Region[u].m_EnemyRobotCnt > 0 || m_Region[u].m_EnemyCannonCnt > 0)
                continue;

            m_RegionIndex[cnt] = u;
            m_Region[u].m_Data = dist2;
            cnt++;
        }
        if (t < g_MatrixMap->m_RN.m_Region[m_RegionIndex[sme]].m_NearCnt)
            break;

        sme++;
        if (sme >= next) {
            next = cnt;
            dist2++;
        }
    }
    if (sme >= cnt)
        return false;

    return dist2 <= Float2Int(1.3f * dist);
}

void CMatrixSideUnit::DMTeam(int team, EMatrixLogicActionType ot, int state, const wchar *format, ...) {
    const wchar *ots[] = {L"mlat_None",    L"mlat_Defence", L"mlat_Attack",   L"mlat_Forward",
                    L"mlat_Retreat", L"mlat_Capture", L"mlat_Intercept"};
    const wchar *sstate;
    if (state < 0)
        sstate = L" Cancel";
    else if (state > 0)
        sstate = L" Accept";
    else
        sstate = L"";

    va_list marker;
    va_start(marker, format);
    // DM( std::wstring().Format(L"Size.<i>.Team.<i>",m_Id,team).c_str(),
    //    std::wstring().Format(L"[<s><s>] <s>",ots[ot],sstate,std::wstring().Format(format,marker).c_str()).c_str()
    //    );
}

void CMatrixSideUnit::DMSide(const wchar *format, ...) {
    va_list marker;
    va_start(marker, format);
    // DM( std::wstring().Format(L"Size.<i>",m_Id).c_str(),
    //    std::wstring().Format(format,marker).c_str()
    //    );
}
