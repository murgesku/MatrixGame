// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixLogicSlot.h"
#include "MatrixState.h"
#include "../MatrixMap.hpp"
#include "../MatrixRobot.hpp"

// CMatrixLogicSlot::~CMatrixLogicSlot()
//{
//	DTRACE();
//
//    CMatrixState *State = m_FirstState;
//	while(State != NULL){
//		if(State->m_NextState)
//			State = State->m_NextState;
//		else {
//			HDelete(CMatrixState, State, g_MatrixHeap);
//			State = NULL;
//			m_FirstState = NULL;
//			m_LastState = NULL;
//
//		}
//
//		if(State)
//			HDelete(CMatrixState, State->m_PrevState, g_MatrixHeap);
//	}
//
//	CLogicSlotRobot *bot = m_FirstRobot;
//
//	while(bot){
//		if(bot->m_NextRobot)
//			bot = bot->m_NextRobot;
//		else {
//			HDelete(CLogicSlotRobot, bot, g_MatrixHeap);
//			bot             = NULL;
//			m_FirstRobot    = bot;
//			m_LastRobot     = bot;
//
//		}
//
//		if(bot)
//			HDelete(CLogicSlotRobot, bot->m_PrevRobot, g_MatrixHeap);
//	}
//
//}
//
// void CMatrixLogicSlot::Load(CBlockPar &bp)
//{
//    CWStr filename;
//    //filename.Add(L"#LO_");
//    //filename.Add(GetName());
//    //filename.Add(L"_");
//    //filename.Add(((CMatrixTactics*)GetParent())->GetName());
//    //filename.Add(L".txt");
//    //bp.SaveInTextFile(filename);
//
//    m_Cells.m_MinCells = bp.ParNE(L"MinCells").GetInt();
//    m_Cells.m_MaxCells = bp.ParNE(L"MaxCells").GetInt();
//
//    CWStr DefStateName(L"");
//    DefStateName = bp.ParGetNE(L"DefState");
//
//     //
//    int states_cnt = bp.BlockCount();
//    for(int cnt = 0; cnt < states_cnt; cnt++){
//        CWStr blockname = bp.BlockGetName(cnt);
//        CMatrixState *state = HNew(g_MatrixHeap) CMatrixState();
//        state->SetName(blockname);
//        state->SetParent(this);
//        if(blockname == DefStateName){
//            m_ActiveState = state;
//        }
//        state->Load(*bp.BlockGet(cnt));
//        LIST_ADD(state, m_FirstState, m_LastState, m_PrevState, m_NextState);
//    }
//}
//
// void CMatrixLogicSlot::RemoveLogicRobotFromList(CMatrixRobotAI* robot)
//{
//    CLogicSlotRobot* ls_robot = m_FirstRobot;
//    while(ls_robot){
//        if(ls_robot->GetLogicRobot() == robot){
//            /*robot->BreakAllOrders();*/
//            LIST_DEL(ls_robot, m_FirstRobot, m_LastRobot, m_PrevRobot, m_NextRobot);
//            HDelete(CLogicSlotRobot, ls_robot, g_MatrixHeap);
//            m_RobotsCnt--;
//            return;
//        }
//        ls_robot = ls_robot->m_NextRobot;
//    }
//}
//
// void CMatrixLogicSlot::AddLogicRobotToList(CMatrixRobotAI* robot)
//{
//    CLogicSlotRobot* ls_robot = HNew(g_MatrixHeap) CLogicSlotRobot;
//    ls_robot->SetLogicRobot(robot);
//    LIST_ADD(ls_robot, m_FirstRobot, m_LastRobot, m_PrevRobot, m_NextRobot);
//    m_RobotsCnt++;
//}
//
// bool CMatrixLogicSlot::FindRobot(CMatrixRobotAI* robot)
//{
//   	CLogicSlotRobot *bots = m_FirstRobot;
//
//    while(bots){
//        if(bots->GetLogicRobot() == robot)
//            return true;
//        bots = bots->m_NextRobot;
//    }
//
//    return false;
//}
//
