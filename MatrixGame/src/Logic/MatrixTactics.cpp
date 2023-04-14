// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "../MatrixMap.hpp"
#include "MatrixTactics.h"
#include "MatrixLogicSlot.h"
#include "../MatrixSide.hpp"
#include "../MatrixRobot.hpp"
#include "MatrixState.h"
#include "MatrixRule.h"
#include "MatrixAIGroup.h"
#include "../MatrixObjectCannon.hpp"

// CMatrixTactics::~CMatrixTactics()
//{
//	DTRACE();
//	CMatrixLogicSlot *Slot = m_FirstSlot;
//
//	while(Slot != NULL){
//		if(Slot->m_NextSlot)
//			Slot = Slot->m_NextSlot;
//		else {
//			HDelete(CMatrixLogicSlot, Slot, g_MatrixHeap);
//			Slot = NULL;
//			m_FirstSlot = NULL;
//			m_LastSlot = NULL;
//
//		}
//
//		if(Slot)
//			HDelete(CMatrixLogicSlot, Slot->m_PrevSlot, g_MatrixHeap);
//	}
//}
//
//
// void CMatrixTactics::Load(CBlockPar &bp)
//{
//    DTRACE();
//    //CWStr filename;
//    //filename.Add(L"#TA_");
//    //filename.Add(GetName());
//    //filename.Add(L".txt");
//    //bp.SaveInTextFile(filename);
//
//    //Type
//    if(bp.ParGetNE(L"Type") == L"Protection"){
//        m_Type = PROTECTION_TACTICS;
//    }else if(bp.ParGetNE(L"Type") == L"Attack"){
//        m_Type = ATTACK_TACTICS;
//    }else if(bp.ParGetNE(L"Type") == L"Capture"){
//        m_Type = CAPTURE_TACTICS;
//    }else if(bp.ParGetNE(L"Type") == L"Join"){
//        m_Type = JOIN_TACTICS;
//    }else if(bp.ParGetNE(L"Type") == L"Move"){
//        m_Type = MOVE_TACTICS;
//    }else if(bp.ParGetNE(L"Type") == L"AttackTarget"){
//        m_Type = ATTACK_TARGET_TACTICS;
//    }
//
//    int logic_cnt = bp.BlockCount();
//    for(int cnt = 0; cnt < logic_cnt; cnt++){
//        CMatrixLogicSlot *slot = HNew(g_MatrixHeap) CMatrixLogicSlot;
//        slot->SetName(bp.BlockGetName(cnt));
//        slot->SetParent(this);
//        slot->Load(*bp.BlockGet(cnt));
//        LIST_ADD(slot, m_FirstSlot, m_LastSlot, m_PrevSlot, m_NextSlot);
//    }
//}
//
// bool CMatrixTactics::TryTactics(CMatrixGroup* group)
//{
//    DTRACE();
//    CMatrixLogicSlot* tmpsl = m_FirstSlot;
//    int min_cells  = 0;
//    int max_cells  = 0;
//
//    while(tmpsl){
//        min_cells += tmpsl->GetCellsCnt().m_MinCells;
//        max_cells += tmpsl->GetCellsCnt().m_MaxCells;
//
//        tmpsl = tmpsl->m_NextSlot;
//    }
//
//    if(group->GetRobotsCnt() < min_cells || group->GetRobotsCnt() > max_cells)
//        return false;
//
//    return true;
//}
//
// void CMatrixTactics::InitialiseTactics(CMatrixGroup* group, CMatrixMapStatic* target, int region)
//{
//    DTRACE();
//
//    m_Target =          target;
//    m_Region =          region;
//    CMatrixLogicSlot*   tmpsl = m_FirstSlot;
//    CMatrixGroupObject*  group_objects = group->m_FirstObject;
//
//
//    int Robots  = group->GetRobotsCnt();
//    int cnt     = 0;
//
//    //if(Robots > 1)
//    //    ASSERT(1);
//    while(tmpsl && Robots > 0){
//        cnt = tmpsl->GetLogicRobotsCnt();
//        Robots -= cnt;
//
//        if(tmpsl->GetName() == L"ExitFromTactics"){
//            tmpsl = tmpsl->m_NextSlot;
//            continue;
//        }
//        while(group_objects && cnt < tmpsl->GetCellsCnt().m_MaxCells && Robots > 0){
//            if(group_objects->GetObject()->GetObjectType() != OBJECT_TYPE_ROBOTAI){
//                group_objects = group_objects->m_NextObject;
//                continue;
//            }
//            if(!FindRobot((CMatrixRobotAI*)group_objects->GetObject())/*!tmpsl->FindRobot(group_robots->GetRobot())*/){
//                tmpsl->AddLogicRobotToList((CMatrixRobotAI*)group_objects->GetObject());
//                cnt++;
//                Robots--;
//            }
//            group_objects = group_objects->m_NextObject;
//        }
//        tmpsl = tmpsl->m_NextSlot;
//    }
//}
//
// void CMatrixTactics::RemoveRobotFromT(CMatrixRobotAI* robot)
//{
//    DTRACE();
//    CMatrixLogicSlot* tmpsl = m_FirstSlot;
//
//    while(tmpsl){
//        tmpsl->RemoveLogicRobotFromList(robot);
//        tmpsl = tmpsl->m_NextSlot;
//    }
//}
//
// void CMatrixTactics::RemoveObjectFromT(CMatrixMapStatic* object){
//    DTRACE();
//    if(object->GetObjectType() != OBJECT_TYPE_ROBOTAI)
//        return;
//
//    CMatrixLogicSlot* tmpsl = m_FirstSlot;
//
//    while(tmpsl){
//        tmpsl->RemoveLogicRobotFromList((CMatrixRobotAI*)object);
//        tmpsl = tmpsl->m_NextSlot;
//    }
//
//}
//
// void CMatrixTactics::LogicTakt(CMatrixSideUnit* side, CMatrixGroup* group)
//{
//    DTRACE();
//
//    if(m_Type == CAPTURE_TACTICS){
//        if(!m_Target){
//            group->DeInstallTactics();
//        }else{
//            if(m_Target->GetObjectType() == OBJECT_TYPE_BUILDING && (((CMatrixBuilding*)m_Target)->m_State ==
//            BUILDING_DIP || ((CMatrixBuilding*)m_Target)->m_State == BUILDING_DIP_EXPLODED)){
//                group->DeInstallTactics();
//            }
//        }
//    }else if(m_Type == ATTACK_TARGET_TACTICS){
//        if(!m_Target){
//            group->DeInstallTactics();
//        }else{
//            if(m_Target->GetObjectType() == OBJECT_TYPE_ROBOTAI && ((CMatrixRobotAI*)m_Target)->m_CurrState ==
//            ROBOT_DIP){
//                group->DeInstallTactics();
//            }else if(m_Target->GetObjectType() == OBJECT_TYPE_CANNON && ((CMatrixCannon*)m_Target)->m_CurrState ==
//            CANNON_DIP){
//                group->DeInstallTactics();
//            }
//        }
//    }
//    CalcNearestToTargetBot();
//
//    if(m_Type == CAPTURE_TACTICS && m_Target && m_Target->GetObjectType() == OBJECT_TYPE_BUILDING &&
//    ((CMatrixBuilding*)m_Target)->m_Side == side->m_Id && ((CMatrixBuilding*)m_Target)->IsBase()){
//        CMatrixGroupObject*  gr_objects = group->m_FirstObject;
//
//        while(gr_objects){
//            if(gr_objects->GetObject() && gr_objects->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI){
//                CMatrixRobotAI* ai_bot = (CMatrixRobotAI* )gr_objects->GetObject();
//                if(ai_bot->FindOrderLikeThat(ROBOT_CAPTURE_FACTORY)){
//                    ai_bot->BreakAllOrders();
//                }
//            }
//            gr_objects = gr_objects->m_NextObject;
//        }
//        group->DeInstallTactics();
//        return;
//
//    }
//
//    CMatrixLogicSlot* tmpsl = m_FirstSlot;
//    while(tmpsl){
//        if(tmpsl->GetName() == L"ExitFromTactics"){
//            CMatrixGroupObject*  gr_objects = group->m_FirstObject;
//            while(gr_objects){
//                if(gr_objects->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI){
//                    if(side->m_Id != PLAYER_SIDE){
//                        if(tmpsl->GetActiveState()->m_FirstRule->If((CMatrixRobotAI*)gr_objects->GetObject(), group)){
//                            tmpsl->GetActiveState()->m_FirstRule->Do((CMatrixRobotAI*)gr_objects->GetObject(), group);
//                            group->DeInstallTactics();
//                            return;
//                        }
//                    }
//                }
//                gr_objects = gr_objects->m_NextObject;
//            }
//            tmpsl = tmpsl->m_NextSlot;
//            continue;
//        }
//
//        CLogicSlotRobot* logic_bot = tmpsl->m_FirstRobot;
//        while(logic_bot){
//            CMatrixRule*  rule = tmpsl->GetActiveState()->m_FirstRule;
//            while(rule && !(g_MatrixMap->GetPlayerSide()->GetArcadedObject() ==
//            (CMatrixMapStatic*)(logic_bot->GetLogicRobot()))){
//                rule->LogicTakt();
//                if(rule->If(logic_bot->GetLogicRobot(), group)){
//                    if(logic_bot->GetRule() != rule){
//                        if(logic_bot->GetLogicRobot()->FindOrderLikeThat(ROBOT_CAPTURE_FACTORY, ROBOT_CAPTURING))
//                            break;
//
//                        logic_bot->GetLogicRobot()->BreakAllOrders();
//                        logic_bot->SetRule(rule);
//                    }
//
//                    rule->Do(logic_bot->GetLogicRobot(), group);
//                    break;
//                }
//                rule = rule->m_NextRule;
//            }
//            logic_bot = logic_bot->m_NextRobot;
//        }
//
//        tmpsl = tmpsl->m_NextSlot;
//    }
//}
//
// void CMatrixTactics::Reset()
//{
//    DTRACE();
//    CMatrixLogicSlot* tmpsl = m_FirstSlot;
//
//    while(tmpsl){
//
//   	    CLogicSlotRobot *bot = tmpsl->m_FirstRobot;
//
//	    while(bot){
//		    if(bot->m_NextRobot)
//			    bot = bot->m_NextRobot;
//		    else {
//			    HDelete(CLogicSlotRobot, bot, g_MatrixHeap);
//			    bot                      = NULL;
//			    tmpsl->m_FirstRobot      = bot;
//			    tmpsl->m_LastRobot       = bot;
//
//		    }
//
//		    if(bot)
//			    HDelete(CLogicSlotRobot, bot->m_PrevRobot, g_MatrixHeap);
//	    }
//
//        tmpsl->Reset();
//        tmpsl = tmpsl->m_NextSlot;
//    }
//
//}
//
// bool CMatrixTactics::FindRobot(CMatrixRobotAI* robot)
//{
//    CMatrixLogicSlot*    slots = m_FirstSlot;
//    while(slots){
//        CLogicSlotRobot* bots = slots->m_FirstRobot;
//        while(bots){
//            if(bots->GetLogicRobot() == robot)
//                return true;
//            bots = bots->m_NextRobot;
//        }
//        slots = slots->m_NextSlot;
//    }
//    return false;
//}
//
// void CMatrixTactics::CalcNearestToTargetBot()
//{
//    if(!m_Target)
//        return;
//
//    CMatrixLogicSlot*   slots = m_FirstSlot;
//    CLogicSlotRobot*    nearestBot = NULL;
//    float               nearestDist = 0;
//    D3DXVECTOR3         targ_pos(0,0,0);
//
//    if(m_Target->GetObjectType() == OBJECT_TYPE_BUILDING){
//        targ_pos = D3DXVECTOR3(((CMatrixBuilding*)m_Target)->m_Pos.x, ((CMatrixBuilding*)m_Target)->m_Pos.y, 0);
//    }else if(m_Target->GetObjectType() == OBJECT_TYPE_ROBOTAI){
//        targ_pos = D3DXVECTOR3(((CMatrixRobotAI*)m_Target)->m_PosX, ((CMatrixRobotAI*)m_Target)->m_PosY, 0);
//    }else if(m_Target->GetObjectType() == OBJECT_TYPE_CANNON){
//        targ_pos = D3DXVECTOR3(((CMatrixCannon*)m_Target)->m_Pos.x, ((CMatrixCannon*)m_Target)->m_Pos.x, 0);
//    }
//
//    while(slots){
//        CLogicSlotRobot* robots = slots->m_FirstRobot;
//        while(robots){
//            D3DXVECTOR3 bot_pos = D3DXVECTOR3(robots->GetLogicRobot()->m_PosX, robots->GetLogicRobot()->m_PosY,0);
//            float a = D3DXVec3LengthSq(&(bot_pos - targ_pos));
//            if(a < nearestDist || nearestBot == NULL){
//                nearestDist = a;
//                nearestBot = robots;
//            }
//            robots = robots->m_NextRobot;
//        }
//        slots = slots->m_NextSlot;
//    }
//    m_NearestToTarget = nearestBot;
//}
//
// CMatrixRobotAI* CMatrixTactics::GetNearestRobotAI()
//{
//        return m_NearestToTarget->GetLogicRobot();
//}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CMatrixTacticsList::~CMatrixTacticsList()
//{
//    DTRACE();
//    CMatrixTactics *Tactics = m_FirstTactics;
//
//	while(Tactics != NULL){
//		if(Tactics->m_NextTactics)
//			Tactics = Tactics->m_NextTactics;
//		else {
//			HDelete(CMatrixTactics, Tactics, g_MatrixHeap);
//			Tactics = NULL;
//			m_FirstTactics = NULL;
//			m_LastTactics = NULL;
//
//		}
//
//		if(Tactics)
//			HDelete(CMatrixTactics, Tactics->m_PrevTactics, g_MatrixHeap);
//	}
//}
//
// CMatrixTactics* CMatrixTacticsList::FindTactics(CMatrixGroup *group, TacticsType tt)
//{
//    CMatrixTactics* tmpt = m_FirstTactics;
//    while(tmpt){
//        if(tmpt->GetType() == tt){
//            if(tmpt->TryTactics(group))
//                return tmpt;
//        }
//        tmpt = tmpt->m_NextTactics;
//    }
//    return NULL;
//}
//
