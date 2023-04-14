// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixTactics.h"
#include "CMain.hpp"

class CMatrixState;
class CMatrixRobotAI;
class CMatrixRule;

typedef struct _c {
    int m_MinCells;
    int m_MaxCells;
    _c() {
        m_MinCells = 0;
        m_MaxCells = 0;
    }
} Cells;

class CLogicSlotRobot : public CMain {
    CMatrixRobotAI *m_Robot;
    CMatrixRule *m_Rule;

public:
    CLogicSlotRobot *m_NextRobot;
    CLogicSlotRobot *m_PrevRobot;

    void SetLogicRobot(CMatrixRobotAI *robot) { m_Robot = robot; }
    CMatrixRobotAI *GetLogicRobot() { return m_Robot; }

    void SetRule(CMatrixRule *rule) { m_Rule = rule; }
    CMatrixRule *GetRule() { return m_Rule; }

    CLogicSlotRobot() {
        m_NextRobot = nullptr;
        m_PrevRobot = nullptr;
        m_Rule = nullptr;
    }
    ~CLogicSlotRobot() {}
};

// class CMatrixLogicSlot : public CBaseTactics
//{
//    CMatrixState*       m_ActiveState;
//    Cells               m_Cells;
//    int                 m_RobotsCnt;
// public:
//    CLogicSlotRobot*    m_FirstRobot;
//    CLogicSlotRobot*    m_LastRobot;
//
//    CMatrixState*       m_FirstState;
//    CMatrixState*       m_LastState;
//
//    CMatrixLogicSlot*   m_PrevSlot;
//    CMatrixLogicSlot*   m_NextSlot;
//
//    void Reset()                                                                    { m_RobotsCnt = 0; }
//
//    void SetActiveState(CMatrixState* state)                                        { if(state) m_ActiveState = state;
//    } CMatrixState* GetActiveState()                                                  { return m_ActiveState; }
//
//    Cells GetCellsCnt()                                                             { return m_Cells; }
//    void Load(CBlockPar &bp);
//
//    int GetLogicRobotsCnt()                                                         { return m_RobotsCnt; }
//
//
//    void RemoveLogicRobotFromList(CMatrixRobotAI* robot);
//    void AddLogicRobotToList(CMatrixRobotAI* robot);
//    bool FindRobot(CMatrixRobotAI* robot);
//
//    CMatrixLogicSlot()
//    {
//        m_ActiveState   = NULL;
//        m_FirstState    = NULL;
//        m_LastState     = NULL;
//        m_PrevSlot      = NULL;
//        m_NextSlot      = NULL;
//        m_FirstRobot    = NULL;
//        m_LastRobot     = NULL;
//        m_RobotsCnt     = 0;
//    }
//    ~CMatrixLogicSlot();
//};