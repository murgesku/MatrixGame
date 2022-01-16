// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

// Tactic's engine
// class CMatrixLogicSlot;
// class CMatrixTactics;
class CMatrixSideUnit;
class CMatrixMapStatic;
class CMatrixRobotAI;
class CMatrixGroup;
// class CLogicSlotRobot;

// typedef enum
//{
//    EMPTY_TACTICS,
//    ATTACK_TACTICS,
//    PROTECTION_TACTICS,
//    CAPTURE_TACTICS,
//    JOIN_TACTICS,
//    MOVE_TACTICS,
//    ATTACK_TARGET_TACTICS
//}TacticsType;
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class CBaseTactics : public CMain
//{
// protected:
//    CWStr m_Name;
//    void* m_Parent;
// public:
//
//    void SetName(const CWStr &name)                                             { m_Name = name; }
//    CWStr GetName()                                                             { return m_Name; }
//
//    void SetParent(const void *parent)                                          { m_Parent = (void *)parent; }
//    void* GetParent()                                                           { return m_Parent; }
//
//    virtual void Load(CBlockPar &bp) = 0;
//
//    CBaseTactics()
//    {
//        m_Name          = L"";
//        m_Parent = NULL;
//    }
//    ~CBaseTactics()                                                             {}
//};
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class CMatrixTactics: public CBaseTactics
//{
//    //Тип тактики
//    TacticsType         m_Type;
//    CMatrixMapStatic*   m_Target;
//    int                 m_Region;
//    CLogicSlotRobot*    m_NearestToTarget;
//
// public:
//
//    //список слотов с логикой
//    CMatrixLogicSlot *m_FirstSlot;
//    CMatrixLogicSlot *m_LastSlot;
//
//    //указатели на соседние элементы
//    CMatrixTactics *m_PrevTactics;
//    CMatrixTactics *m_NextTactics;
//
//    TacticsType GetType()                       { return m_Type; }
//    CMatrixMapStatic* GetTarget()               { return m_Target; }
//    void SetTarget(CMatrixMapStatic* target)    { m_Target = target; }
//    int GetRegion()                             { return m_Region; }
//    void SetRegion(int region)                  { m_Region = region; }
//
//    bool TryTactics(CMatrixGroup* group);
//    void Load(CBlockPar &bp);
//    CWStr GetTacticsName()                      { return m_Name; }
//
//    void InitialiseTactics(CMatrixGroup* group, CMatrixMapStatic* target, int region);
//    void RemoveRobotFromT(CMatrixRobotAI* robot);
//
//    void RemoveObjectFromT(CMatrixMapStatic* object);
//
//    void Reset();
//    void LogicTakt(CMatrixSideUnit* side, CMatrixGroup* group);
//
//    bool FindRobot(CMatrixRobotAI* robot);
//
//    void CalcNearestToTargetBot();
//    CMatrixRobotAI* GetNearestRobotAI();
//
//    CMatrixTactics()
//    {
//        m_FirstSlot         = NULL;
//        m_LastSlot          = NULL;
//        m_PrevTactics       = NULL;
//        m_NextTactics       = NULL;
//
//        m_Type              = EMPTY_TACTICS;
//        m_Target            = NULL;
//        m_NearestToTarget   = NULL;
//    }
//    ~CMatrixTactics();
//};
//
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class CMatrixTacticsList : public CMain
//{
// public:
//    CMatrixTactics *m_FirstTactics;
//    CMatrixTactics *m_LastTactics;
//
//    CMatrixTactics* FindTactics(CMatrixGroup *group, TacticsType tt);
//
//    CMatrixTacticsList()
//    {
//        m_FirstTactics = NULL;
//        m_LastTactics = NULL;
//    }
//
//    ~CMatrixTacticsList();
//
//};
//
