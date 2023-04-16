// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixTactics.h"
#include "3g.hpp"

// Groups
#define DIF_GROUP_R    200
#define REGROUP_PERIOD 100
#define JUST_PERIOD    100

class CMatrixRobotAI;
class CMatrixSideUnit;
class CMatrixGroup;
class CMatrixTactics;

/*enum Team
{
    TEAM_UNDEF,
    SYSTEM,
    ALPHA,
    BRAVO,
    CHARLIE
};*/

//
class CMatrixGroupObject : public CMain {
public:
    int m_Team;
    CMatrixGroup *m_ParentGroup;
    CMatrixMapStatic *m_Object;
    CMatrixGroupObject *m_NextObject;
    CMatrixGroupObject *m_PrevObject;

public:
    void SetObject(CMatrixMapStatic *object) { m_Object = object; }

    CMatrixMapStatic *GetObject() { return m_Object; }

    CMatrixGroup *GetParentGroup() { return m_ParentGroup; }
    void SetParentGroup(CMatrixGroup *group) { m_ParentGroup = group; }

    void SetTeam(int team) { m_Team = team; }

    int GetTeam() { return m_Team; }

    CMatrixGroupObject() {
        m_Object = NULL;
        m_NextObject = NULL;
        m_PrevObject = NULL;
        m_ParentGroup = NULL;
        m_Team = -1;
    }
    ~CMatrixGroupObject();
};

class CMatrixGroup : public CMain {
public:
    int m_Team;
    int m_ObjectsCnt;
    int m_RobotsCnt;
    int m_FlyersCnt;
    int m_BuildingsCnt;
    // CMatrixTactics*     m_Tactics;
    D3DXVECTOR3 m_GroupPosition;
    int m_Id;
    float m_GroupSpeed;
    int m_SimpleTimer;

    CMatrixGroupObject *m_FirstObject;
    CMatrixGroupObject *m_LastObject;
    CMatrixGroup *m_NextGroup;
    CMatrixGroup *m_PrevGroup;

public:
    void AddObject(CMatrixMapStatic *object, int team);
    void RemoveObject(CMatrixMapStatic *object);
    bool RemoveObject(int num);
    void RemoveAll();
    void RemoveBuildings();
    void SortFlyers();

    CMatrixMapStatic *GetObjectByN(int num);

    int GetObjectsCnt() { return m_ObjectsCnt; }

    int GetRobotsCnt() { return m_RobotsCnt; }
    int GetFlyersCnt() { return m_FlyersCnt; }
    int GetBuildingsCnt() { return m_BuildingsCnt; }

    void SetTeam(int team) { m_Team = team; }
    int GetTeam() { return m_Team; }

    void FindNearObjects(CMatrixGroupObject *object);
    bool FindObject(CMatrixMapStatic *object);

    // void InstallTactics(TacticsType type, CBlockPar* par);
    // void DeInstallTactics();

    void LogicTakt(CMatrixSideUnit *side);
    void CalcGroupPosition();
    void CalcGroupSpeed();
    void SetGroupId(int id) { m_Id = id; }
    int GetGroupId() { return m_Id; }
    // CMatrixTactics* GetTactics()                                    { return m_Tactics; }
    D3DXVECTOR3 GetGroupPos() { return m_GroupPosition; }
    float GetGroupSpeed() { return m_GroupSpeed; }
    void SetGroupSpeed(float speed) { m_GroupSpeed = speed; }

    int GetBombersCnt();
    int GetRepairsCnt();

    CMatrixGroup();
    ~CMatrixGroup();
};

// class CMatrixGroupList : public CMain
//{
//    public:
//        int m_GroupsCnt;
//
//        int m_ReGroupPeriod;
//        CMatrixGroup*   m_FirstGroup;
//        CMatrixGroup*   m_LastGroup;
//
//    public:
//
//        int GetGroupsCnt()                                                  { return m_GroupsCnt; }
//
//        CMatrixGroup* AddNewGroup(int team, int id);
//        void RemoveGroup(CMatrixGroup* group);
//        void RemoveGroup(int team);
//
//        void ReGroup(CMatrixSideUnit* side);
//
//        //void AddRobotToTeam(Team team, CMatrixRobotAI* robot);
//        //void RemoveRobotFromTeam(Team team, CMatrixRobotAI* robot);
//
//        void AddObject(int team, int id, CMatrixMapStatic* object);
//        void RemoveObject(int team, int id, CMatrixMapStatic* object);
//
//        void LogicTakt(CMatrixSideUnit* side);
//
//        CMatrixGroupList()                                                  { m_GroupsCnt=0; m_FirstGroup=NULL;
//        m_LastGroup=NULL; m_ReGroupPeriod=0; } ~CMatrixGroupList();
//};
