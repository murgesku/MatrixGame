// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "VectorObject.hpp"
#include "Effects/MatrixEffect.hpp"
#include "Logic/MatrixRoadNetwork.hpp"
#include "MatrixConfig.hpp"
#include "MatrixFlyer.hpp"

//#define MAX_ROBOTS_BUILD        10
#define MAX_ROBOTS      60
#define MAX_FACTORIES   10
#define PLAYER_SIDE     1
#define MAX_LOGIC_GROUP (MAX_ROBOTS + 1)

#define MAX_TEAM_CNT         3
#define MAX_SELECTION_GROUPS 10

#define HELI_PARAM 3

#define ROBOTS_BY_BASE    3
#define ROBOTS_BY_MAIN    4
#define ROBOTS_BY_FACTORY 1

#define SPEED_BOOST 1.1f

#define FRIENDLY_SEARCH_RADIUS 400

class CMatrixEffectWeapon;
class CMatrixMapStatic;
class CConstructor;
class CMatrixEffectSelection;
class CMatrixRobotAI;
class CMatrixRobot;
class CMatrixBuilding;
class CMatrixTactics;
class CMatrixTacticsList;
class CMatrixGroupList;
class CMatrixEffectElevatorField;
class CMatrixGroup;
class CMatrixCannon;
class CMatrixEffectLandscapeSpot;
class CMatrixFlyer;
class CConstructorPanel;

#define MAX_STATISTICS 6
enum EStat {
    STAT_ROBOT_BUILD,
    STAT_ROBOT_KILL,
    STAT_TURRET_BUILD,
    STAT_TURRET_KILL,
    STAT_BUILDING_KILL,
    STAT_TIME,

    EStat_FORCE_DWORD = 0x7FFFFFFF
};

enum ESelection {
    FLYER_SELECTED = 0,
    ROBOT_SELECTED = 1,
    BUILDING_SELECTED = 2,
    NOTHING_SELECTED = 4,
    BASE_SELECTED = 5,
    GROUP_SELECTED = 6,
    ARCADE_SELECTED = 7,

    ESelection_FORCE_DWORD = 0x7FFFFFFF
};

enum ESelType {
    ROBOT = 0,
    FAR_ROBOT = 1,
    FLYER = 2,
    BUILDING = 3,
    NOTHING = 4,
    GROUP = 5,
    ARCADE = 6,

    ESelType_FORCE_DWORD = 0x7FFFFFFF
};

struct SRobot {
    CMatrixRobotAI *m_Robot;
    D3DXVECTOR3 m_Mark;
    DWORD m_CrossCatched;  // this is bool variable
    SRobot() { m_CrossCatched = false; }
};

enum ESideStatus {
    SS_NONE,       // side absent
    SS_ACTIVE,     // active side
    SS_JUST_DEAD,  // just dead side. switch this status to SS_NONE, after you get it
    SS_JUST_WIN,   // valid only for player side

    ESideStatus_FORCE_DWORD = 0x7FFFFFFF
};

enum EPlayerActions {
    NOTHING_SPECIAL,
    CAPTURING_ROBOT,
    TRANSPORTING_ROBOT,
    DROPING_ROBOT,
    GETING_IN_ROBOT,
    BUILDING_TURRET,

    EPlayerActions_FORCE_DWORD = 0x7FFFFFFF
};

struct SCannonForBuild {
    CMatrixCannon *m_Cannon;
    CMatrixBuilding *m_ParentBuilding;
    SEffectHandler m_ParentSpot;
    int m_CanBuildFlag;

    void Delete();
    SCannonForBuild()
      :
#ifdef _DEBUG
        m_ParentSpot(DEBUG_CALL_INFO)
#else
        m_ParentSpot()
#endif
    {
        m_Cannon = NULL;
        m_ParentBuilding = NULL;
        m_CanBuildFlag = 0;
    }
};

struct SCallback {
    CPoint mp;
    int calls;
};

enum EMatrixLogicActionType {
    mlat_None,
    mlat_Defence,
    mlat_Attack,
    mlat_Forward,
    mlat_Retreat,
    mlat_Capture,
    mlat_Intercept,

    EMatrixLogicActionType_FORCE_DWORD = 0x7FFFFFFF
};

enum EFlyerOrder : unsigned int;

#define MLRG_CAPTURE 1
#define MLRG_DEFENCE 2

#define REGION_PATH_MAX_CNT 32

struct SMatrixLogicAction {
    EMatrixLogicActionType m_Type;
    int m_Region;
    int m_RegionPathCnt;                    // Кол-во регионов в пути
    int m_RegionPath[REGION_PATH_MAX_CNT];  // Путь по регионам
};

struct SMatrixLogicGroup {
private:
    DWORD m_Bits;
    // int m_RobotCnt;
    // bool m_War;                             // Группа вступила в бой
public:
    int RobotsCnt(void) const { return m_Bits & 0xFFFF; }
    void RobotsCnt(int a) { m_Bits = (m_Bits & 0xFFFF0000) | a; }
    inline void IncRobotsCnt(int cc = 1) { m_Bits += cc; }

    bool IsWar(void) const { return FLAG(m_Bits, SETBIT(31)); }
    void SetWar(bool w) { INITFLAG(m_Bits, SETBIT(31), w); }

    SMatrixLogicAction m_Action;
    int m_Team;
    float m_Strength;
};

enum EMatrixPlayerOrder {
    mpo_Stop,
    mpo_MoveTo,
    mpo_Capture,
    mpo_Attack,
    mpo_Patrol,
    mpo_Repair,
    mpo_Bomb,
    mpo_AutoCapture,
    mpo_AutoAttack,
    mpo_AutoDefence,

    EMatrixPlayerOrder_FORCE_DWORD = 0x7FFFFFFF
};

struct SMatrixPlayerGroup {
private:
    DWORD m_Bits;
    // bool m_War;                                     // Группа вступила в бой
    // bool m_ShowPlace;
    // bool m_PatrolReturn;
    // EMatrixPlayerOrder m_Order;
public:
    EMatrixPlayerOrder Order(void) const { return EMatrixPlayerOrder(m_Bits & 0xFF); }
    void Order(EMatrixPlayerOrder o) { m_Bits = (m_Bits & 0xFFFFFF00) | o; }

    bool IsWar(void) const { return FLAG(m_Bits, SETBIT(31)); }
    void SetWar(bool w) { INITFLAG(m_Bits, SETBIT(31), w); }

    bool IsShowPlace(void) const { return FLAG(m_Bits, SETBIT(30)); }
    void SetShowPlace(bool w) { INITFLAG(m_Bits, SETBIT(30), w); }

    bool IsPatrolReturn(void) const { return FLAG(m_Bits, SETBIT(29)); }
    void SetPatrolReturn(bool w) { INITFLAG(m_Bits, SETBIT(29), w); }

    int m_RobotCnt;
    CPoint m_From;
    CPoint m_To;
    CMatrixMapStatic *m_Obj;
    int m_Region;
    int m_RegionPathCnt;                    // Кол-во регионов в пути
    int m_RegionPath[REGION_PATH_MAX_CNT];  // Путь по регионам
    CMatrixRoadRoute *m_RoadPath;
};

struct SMatrixTeam {
private:
    DWORD m_Bits;

    // byte m_Move;
    // bool m_War;
    // bool m_Stay;
    // bool m_WaitUnion;
    // bool m_RobotInDesRegion;
    // bool m_RegroupOnlyAfterWar;

    // bool m_lOk;
public:
    BYTE Move(void) const { return BYTE(m_Bits & 0xFF); }
    void Move(BYTE m) { m_Bits = (m_Bits & 0xFFFFFF00) | m; }
    void OrMove(BYTE m) { m_Bits |= m; }

    bool IsWar(void) const { return FLAG(m_Bits, SETBIT(31)); }
    void SetWar(bool w) { INITFLAG(m_Bits, SETBIT(31), w); }

    bool IsStay(void) const { return FLAG(m_Bits, SETBIT(30)); }
    void SetStay(bool w) { INITFLAG(m_Bits, SETBIT(30), w); }

    bool IsWaitUnion(void) const { return FLAG(m_Bits, SETBIT(29)); }
    void SetWaitUnion(bool w) { INITFLAG(m_Bits, SETBIT(29), w); }

    bool IsRobotInDesRegion(void) const { return FLAG(m_Bits, SETBIT(28)); }
    void SetRobotInDesRegion(bool w) { INITFLAG(m_Bits, SETBIT(28), w); }

    bool IsRegroupOnlyAfterWar(void) const { return FLAG(m_Bits, SETBIT(27)); }
    void SetRegroupOnlyAfterWar(bool w) { INITFLAG(m_Bits, SETBIT(27), w); }

    bool IslOk(void) const { return FLAG(m_Bits, SETBIT(26)); }
    void SetlOk(bool w) { INITFLAG(m_Bits, SETBIT(26), w); }

    int m_RobotCnt;  // Кол-во роботов в команде
    int m_GroupCnt;

    int m_WaitUnionLast;

    float m_Strength;

    int m_TargetRegion;
    int m_TargetRegionGoal;

    CPoint m_CenterMass;
    int m_RadiusMass;
    CRect m_Rect;
    CPoint m_Center;
    int m_Radius;
    int m_RegionMassPrev;
    int m_RegionMass;
    int m_RegionNearDanger;  // Самый опасный регион, включая текущий
    int m_RegionFarDanger;
    int m_RegionNearEnemy;
    int m_RegionNearRetreat;
    int m_RegionNearForward;
    int m_RegionNerestBase;

    int m_Brave;
    float m_BraveStrangeCancel;

    int m_ActionCnt;
    SMatrixLogicAction m_ActionList[16];
    SMatrixLogicAction m_Action;
    SMatrixLogicAction m_ActionPrev;
    int m_ActionTime;
    int m_RegionNext;
    CMatrixRoadRoute *m_RoadPath;

    int m_RegionListCnt;           // Список регионов, в которых находятся роботы
    int m_RegionList[MAX_ROBOTS];  // Регионы
    int m_RegionListRobots[MAX_ROBOTS];  // Кол-во роботов
};

struct SMatrixLogicRegion {
    int m_WarEnemyRobotCnt;
    int m_WarEnemyCannonCnt;
    int m_WarEnemyBuildingCnt;
    int m_WarEnemyBaseCnt;

    int m_EnemyRobotCnt;
    int m_EnemyCannonCnt;
    int m_EnemyBuildingCnt;
    int m_EnemyBaseCnt;

    float m_Danger;     // Кофициент опасности
    float m_DangerAdd;  // Выращенная опасность

    int m_NeutralCannonCnt;
    int m_NeutralBuildingCnt;
    int m_NeutralBaseCnt;

    int m_OurRobotCnt;
    int m_OurCannonCnt;
    int m_OurBuildingCnt;
    int m_OurBaseCnt;

    int m_EnemyRobotDist;
    int m_EnemyBuildingDist;
    int m_OurBaseDist;

    DWORD m_Data;
};

class CMatrixSideUnit : public CMain {
public:
    // In map options
    int m_TimeNextBomb;
    float m_StrengthMul;
    float m_BraveMul;
    int m_TeamCnt;
    float m_DangerMul;
    float m_WaitResMul;

private:
    int m_TimeLastBomb;

    ESideStatus m_SideStatus;
    int m_RobotsCnt;

    float m_Strength;  // Сила стороны
    int m_WarSide;     // На какую сторону стараемся напасть
    // float m_WarSideStrangeCancel;           // Перерассчитываем сторону с которой воюем когда сила стороны упадет
    // ниже критической

    SMatrixLogicRegion *m_Region;
    int *m_RegionIndex;
    int m_LastTaktHL;
    int m_LastTaktTL;
    int m_LastTeamChange;
    int m_LastTaktUnderfire;

    int m_NextWarSideCalcTime;

    //    int m_TitanCnt;
    //    int m_ElectronicCnt;
    //    int m_EnergyCnt;
    //    int m_PlasmaCnt;

    int m_BaseResForce;
    int m_Resources[MAX_RESOURCES];
    int m_CurSelNum;

    int m_Statistic[MAX_STATISTICS];

    CMatrixMapStatic *m_Arcaded;
    DWORD m_ArcadedP_available;
    D3DXVECTOR3 m_ArcadedP_cur;
    D3DXVECTOR3 m_ArcadedP_prevrp;

    D3DXVECTOR3 m_ArcadedP_ppos0;
    D3DXVECTOR3 m_ArcadedP_ppos1;
    float m_ArcadedP_k;

    CMatrixGroup *m_CurSelGroup;
    CMatrixGroup *m_FirstGroup;
    CMatrixGroup *m_LastGroup;

public:
    int *m_PlaceList;

    SMatrixLogicGroup m_LogicGroup[MAX_LOGIC_GROUP];
    SMatrixPlayerGroup m_PlayerGroup[MAX_LOGIC_GROUP];
    int m_WaitResForBuildRobot;

    SMatrixTeam m_Team[MAX_TEAM_CNT];

public:
    void BufPrepare(void);

    void SetStatus(ESideStatus s) { m_SideStatus = s; }
    ESideStatus GetStatus(void) const { return m_SideStatus; }

    void ClearStatistics(void) { memset(m_Statistic, 0, sizeof(m_Statistic)); }
    int GetStatValue(EStat stat) const { return m_Statistic[stat]; }
    void SetStatValue(EStat stat, int v) { m_Statistic[stat] = v; }
    void IncStatValue(EStat stat, int v = 1) { m_Statistic[stat] += v; }

    int GetRobotsInStack();
    int GetAlphaCnt() { return m_Team[0].m_RobotCnt; }
    int GetBravoCnt() { return m_Team[1].m_RobotCnt; }
    int GetCharlieCnt() { return m_Team[2].m_RobotCnt; }
    int GetRobotsCnt() { return m_RobotsCnt; }

    int GetResourcesAmount(ERes res) const { return m_Resources[res]; }
    void AddResourceAmount(ERes res, int amount) {
        m_Resources[res] += amount;
        if (m_Resources[res] > 9000)
            m_Resources[res] = 9000;
        if (m_Resources[res] < 0)
            m_Resources[res] = 0;
    }
    void SetResourceAmount(ERes res, int amount) { m_Resources[res] = amount; }
    void SetResourceForceUp(int fu) { m_BaseResForce = fu; }
    int GetResourceForceUp(void) { return m_BaseResForce; }
    bool IsEnoughResources(const int *resources) {
        if (m_Resources[0] >= resources[0] && m_Resources[1] >= resources[1] && m_Resources[2] >= resources[2] &&
            m_Resources[3] >= resources[3])
            return true;
        else
            return false;
    }

    void GetResourceIncome(int &base_i, int &fa_i, ERes resource_type);
    int GetIncomePerTime(int building, int ms);
    void PLDropAllActions();
    SMatrixTeam *GetTeam(int no) { return m_Team + no; }

    int m_Id;
    std::wstring m_Name;

    DWORD m_Color;
    DWORD m_ColorMM;
    CTexture *m_ColorTexture;

    CConstructor *m_Constructor;

    int m_BuildRobotLast;
    int m_BuildRobotLast2;
    int m_BuildRobotLast3;

    int GetSideRobots() { return m_RobotsCnt; }
    int GetMaxSideRobots();

    void LogicTakt(int ms);

    // Player
    CConstructorPanel *m_ConstructPanel;
    void InitPlayerSide();
    std::wstring m_PlayerName;
    EPlayerActions m_CurrentAction;
    int m_nCurrRobotPos;
    ESelection m_CurrSel;

private:
    CMatrixGroup *m_CurrentGroup;  // CMatrixGroup* m_CurGroup;
public:
    CMatrixGroup *GetCurGroup() { return m_CurrentGroup; }
    void SetCurGroup(CMatrixGroup *group);

    void RemoveObjectFromSelectedGroup(CMatrixMapStatic *o);

    CMatrixMapStatic *m_ActiveObject;
    SRobot m_Robots[MAX_ROBOTS];
    SCannonForBuild m_CannonForBuild;

    int GetCurSelNum() { return m_CurSelNum; }
    void SetCurSelNum(int i);
    void ResetSelection();
    CMatrixMapStatic *GetCurSelObject();

    CMatrixGroup *CreateGroupFromCurrent();
    void CreateGroupFromCurrent(CMatrixMapStatic *obj);
    void AddToCurrentGroup();
    void SelectedGroupUnselect();
    void GroupsUnselectSoft();
    void SelectedGroupBreakOrders();
    void SelectedGroupMoveTo(const D3DXVECTOR2 &pos);
    void SelectedGroupAttack(CMatrixMapStatic *victim);
    void SelectedGroupCapture(CMatrixMapStatic *building);
    void PumpGroups();

    CMatrixGroup *GetFirstGroup() { return m_FirstGroup; }
    CMatrixGroup *GetLastGroup() { return m_LastGroup; }
    CMatrixGroup *GetCurSelGroup() { return m_CurSelGroup; }

    void RemoveFromSelection(CMatrixMapStatic *o);
    bool FindObjectInSelection(CMatrixMapStatic *o);
    void ResetSystemSelection();
    void __stdcall PlayerAction(void *object);

    void RobotStop(void *pObject);
    void Select(ESelType type, CMatrixMapStatic *object);
    void Reselect();
    void ShowOrderState(void);
    bool MouseToLand(const CPoint &mouse, float *pWorldX, float *pWorldY, int *pMapX, int *pMapY);
    CMatrixMapStatic *MouseToLand();
    void OnRButtonDown(const CPoint &mouse);
    void OnRButtonDouble(const CPoint &mouse);
    void OnLButtonDown(const CPoint &mouse);
    void OnLButtonDouble(const CPoint &mouse);
    void OnRButtonUp(const CPoint &mouse);
    void OnLButtonUp(const CPoint &mouse);
    void OnForward(bool down);
    void OnBackward(bool down);
    void OnLeft(bool down);
    void OnRight(bool down);
    void OnMouseMove();

    // Tactics
    // void GiveRandomOrder();
    // CBlockPar *m_TacticsPar;

    // Groups
    // CMatrixGroupList*   m_GroupsList;
    // CMatrixGroup* GetGroup(int id, int t);

    bool IsArcadeMode(void) const { return m_Arcaded != NULL; }
    bool IsRobotMode();
    bool IsFlyerMode();
    CMatrixMapStatic *GetArcadedObject(void) { return m_Arcaded; }
    void SetArcadedObject(CMatrixMapStatic *o);

    D3DXVECTOR3 CorrectArcadedRobotArmorP(D3DXVECTOR3 &p, CMatrixRobot *robot);
    // void SetArcadedRobotArmorP(const D3DXVECTOR3 &p);
    void InterpolateArcadedRobotArmorP(int ms);

    void OrderFlyer(const D3DXVECTOR2 &to, EFlyerOrder order, float ang, int place, const CPoint &bpos, int bpi);

    CMatrixSideUnit(void);
    ~CMatrixSideUnit();

    // STUB:
    int IsInPlaces(const CPoint *places, int placescnt, int x, int y);

    // High logic
    void CalcStrength(void);
    void Regroup(void);
    void ClearTeam(int team);
    int ClacSpawnTeam(int region, int nsh);
    void EscapeFromBomb(void);
    void GroupNoTeamRobot(void);
    void CalcMaxSpeed(void);
    void TaktHL(void);
    int FindNearRegionWithUTR(int from, int *exclude_list, int exclude_cnt,
                              DWORD flags);  // 1-our 2-netral 4-enemy 8-base 16-building 32-robot 64-cannon
    int CompareRegionForward(int team, int r1, int r2);
    int CompareAction(int team, SMatrixLogicAction *a1, SMatrixLogicAction *a2);
    void BestAction(int team);
    void LiveAction(int team);
    float BuildRobotMinStrange(CMatrixBuilding *base);
    void BuildRobot(void);
    void BuildCannon(void);

    // Theam logic
    void TaktTL(void);
    void WarTL(int group);
    void RepairTL(int group);
    void AssignPlace(CMatrixRobotAI *robot, int region);
    void AssignPlace(int group, int region);
    void SortRobotList(CMatrixRobotAI **rl, int rlcnt);
    bool CmpOrder(int team, int group) {
        ASSERT(team >= 0 && team < m_TeamCnt);
        return m_LogicGroup[group].m_Action.m_Type == m_Team[team].m_Action.m_Type &&
               m_LogicGroup[group].m_Action.m_Region == m_Team[team].m_Action.m_Region;
    }  // Путь не сравнивается
    void CopyOrder(int team, int group) {
        ASSERT(team >= 0 && team < m_TeamCnt);
        m_LogicGroup[group].m_Action = m_Team[team].m_Action;
    }

    bool PlaceInRegion(CMatrixRobotAI *robot, int place, int region);

    void CalcRegionPath(SMatrixLogicAction *ac, int rend, byte mm);

    bool CanMoveNoEnemy(byte mm, int r1, int r2);

    // Player logic

    void SoundCapture(int pg);

    void TaktPL(int onlygroup = -1);
    bool FirePL(int group);
    void RepairPL(int group);
    void WarPL(int group);
    int SelGroupToLogicGroup(void);
    int RobotToLogicGroup(CMatrixRobotAI *robot);
    void PGOrderStop(int no);
    void PGOrderMoveTo(int no, const CPoint &tp);
    void PGOrderCapture(int no, CMatrixBuilding *building);
    void PGOrderAttack(int no, const CPoint &tp, CMatrixMapStatic *terget_obj);
    void PGOrderPatrol(int no, const CPoint &tp);
    void PGOrderRepair(int no, CMatrixMapStatic *terget_obj);
    void PGOrderBomb(int no, const CPoint &tp, CMatrixMapStatic *terget_obj);
    void PGOrderAutoCapture(int no);
    void PGOrderAutoAttack(int no);
    void PGOrderAutoDefence(int no);
    void PGRemoveAllPassive(int no, CMatrixMapStatic *skip);
    void PGAssignPlace(int no, CPoint center);
    void PGAssignPlacePlayer(int no, const CPoint &center);
    void PGSetPlace(CMatrixRobotAI *robot, const CPoint &p);
    void PGPlaceClear(int no);
    CPoint PGCalcCenterGroup(int no);
    CPoint PGCalcPlaceCenter(int no);
    void PGShowPlace(int no);
    void PGCalcStat(void);
    void PGFindCaptureFactory(int no);
    void PGFindAttackTarget(int no);
    void PGFindDefenceTarget(int no);
    void PGCalcRegionPath(SMatrixPlayerGroup *pg, int rend, byte mm);

    void BuildCrazyBot(void);

    void DMTeam(int team, EMatrixLogicActionType ot, int state, const wchar *format, ...);
    void DMSide(const wchar *format, ...);
};

void SideSelectionCallBack(CMatrixMapStatic *ms, DWORD param);
