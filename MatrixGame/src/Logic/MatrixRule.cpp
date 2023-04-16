// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixRule.h"
#include "MatrixLogicSlot.h"
#include "MatrixState.h"
#include "MatrixTactics.h"
#include "../MatrixMap.hpp"
#include "../MatrixRobot.hpp"

// CMatrixRule::~CMatrixRule()
//{
//}
//
// void CMatrixRule::Load(CBlockPar &bp)
//{
//    CWStr filename;
//    //filename.Add(L"#RU_");
//    //filename.Add(GetName());
//    //filename.Add(L"_");
//    //filename.Add(((CMatrixState*)GetParent())->GetName());
//    //filename.Add(L"_");
//    //filename.Add(((CMatrixLogicSlot*)((CMatrixState*)GetParent())->GetParent())->GetName());
//    //filename.Add(L"_");
//    //filename.Add(((CMatrixTactics*)((CMatrixLogicSlot*)((CMatrixState*)GetParent())->GetParent())->GetParent())->GetName());
//    //filename.Add(L".txt");
//    //bp.SaveInTextFile(filename);
//
//    CBlockPar *bp_if = bp.BlockGetNE(L"If");
//    CBlockPar *bp_do = bp.BlockGetNE(L"Do");
//
//    CMatrixTactics* parent_tactics =
//    ((CMatrixTactics*)((CMatrixLogicSlot*)((CMatrixState*)GetParent())->GetParent())->GetParent());
//
////If
//    if(bp_if){
//        //Target
//        if(bp_if->ParNE(L"EnemySpoted").Find(L"Nearest") >= 0){
//            m_Condition.m_EnemySpoted |= ENEMY_NEAREST;
//        }
//        if(bp_if->ParNE(L"EnemySpoted").Find(L"Strongest") >= 0){
//            m_Condition.m_EnemySpoted |= ENEMY_STRONGEST;
//        }
//        if(bp_if->ParNE(L"EnemySpoted").Find(L"MinHealth") >= 0){
//            m_Condition.m_EnemySpoted |= ENEMY_MINHEALTH;
//        }
//        if(bp_if->ParNE(L"EnemySpoted").Find(L"MaxHealth") >= 0){
//            m_Condition.m_EnemySpoted |= ENEMY_MAXHEALTH;
//        }
//        if(bp_if->ParNE(L"EnemySpoted").Find(L"Dangerous") >= 0){
//            m_Condition.m_EnemySpoted |= ENEMY_DANGEROUS;
//        }
//        if(bp_if->ParNE(L"EnemySpoted").Find(L"None") >= 0){
//            m_Condition.m_EnemySpoted |= ENEMY_NONE;
//        }
//
//        if(bp_if->ParNE(L"EnemySpoted").Find(L"Any") >= 0){
//            m_Condition.m_EnemySpoted = ENEMY_ANY;
//        }
//
//        //Destroyed
//        CWStr IfDestroyedSlot(bp_if->ParNE(L"Destroyed"));
//
//        if(IfDestroyedSlot != L""){
//            CMatrixLogicSlot *tmp_slot = parent_tactics->m_FirstSlot;
//            while(tmp_slot){
//                if(tmp_slot->GetName() == IfDestroyedSlot){
//                    m_Condition.m_DestroyedSlot = tmp_slot;
//                    break;
//                }
//                tmp_slot = tmp_slot->m_NextSlot;
//            }
//        }
//
//        //Position
//        if(bp_if->ParNE(L"Position").Find(L"NearestToTarget") >= 0){
//            m_Condition.m_Position = POSITION_NEAREST_TO_TARGET;
//        }
//
//    }
////Do
//    if(bp_do){
//        //PursueEnemy
//        if(bp_do->ParGetNE(L"PursueEnemy") == L"True"){
//            m_Action.m_PursueEnemy = PURSUE;
//        }else if(bp_do->ParGetNE(L"PursueEnemy") == L"False"){
//            m_Action.m_PursueEnemy = NOT_PURSUE;
//        }
//
//        //FleeFromEnemy
//        if(bp_do->ParGetNE(L"FleeFromEnemy") == L"True"){
//            m_Action.m_FleeFromEnemy = FLEE;
//        }else if(bp_do->ParGetNE(L"FleeFromEnemy") == L"False"){
//            m_Action.m_FleeFromEnemy = NOT_FLEE;
//        }
//
//        //ChangeEnemy
//        if(bp_do->ParGetNE(L"ChangeEnemy") == L"True"){
//            m_Action.m_ChangeEnemy = CHANGE;
//        }else if(bp_do->ParGetNE(L"ChangeEnemy") == L"False"){
//            m_Action.m_ChangeEnemy = NOT_CHANGE;
//        }
//
//        //Attack
//        if(bp_do->ParGetNE(L"AttackEnemy") == L"True"){
//            m_Action.m_AttackEnemy = ATTACK;
//        }else if(bp_do->ParGetNE(L"AttackEnemy") == L"False"){
//            m_Action.m_AttackEnemy = NOT_ATTACK;
//        }
//
//        //MoveTo
//        if(bp_do->ParNE(L"MoveTo").Find(L"Protected") >= 0){
//            m_Action.m_MoveTo |= MOVETO_PROTECTED;
//        }
//        if(bp_do->ParNE(L"MoveTo").Find(L"NearEntry") >= 0){
//            m_Action.m_MoveTo |= MOVETO_NEAR_ENTRY;
//        }
//        if(bp_do->ParNE(L"MoveTo").Find(L"FarEntry") >= 0){
//            m_Action.m_MoveTo |= MOVETO_FAR_ENTRY;
//        }
//        if(bp_do->ParNE(L"MoveTo").Find(L"BlockEntry") >= 0){
//            m_Action.m_MoveTo |= MOVETO_BLOCK_ENTRY;
//        }
//        if(bp_do->ParNE(L"MoveTo").Find(L"TargetZone") >= 0){
//            m_Action.m_MoveTo |= MOVETO_TARGETZONE;
//        }
//        if(bp_do->ParNE(L"MoveTo").Find(L"MainGroup") >= 0){
//            m_Action.m_MoveTo |= MOVETO_MAINGROUP;
//        }
//        if(bp_do->ParNE(L"MoveTo").Find(L"AroundEnemy") >= 0){
//            m_Action.m_MoveTo |= MOVETO_AROUNDENEMY;
//        }
//        if(bp_do->ParNE(L"MoveTo").Find(L"FarManeuver") >= 0){
//            m_Action.m_MoveTo |= MOVETO_FAR_MANEUVER;
//        }
//        if(bp_do->ParNE(L"MoveTo").Find(L"NearManeuver") >= 0){
//            m_Action.m_MoveTo |= MOVETO_NEAR_MANEUVER;
//        }
//        if(bp_do->ParNE(L"MoveTo").Find(L"Region") >= 0){
//            m_Action.m_MoveTo |= MOVETO_REGION;
//        }
//
//        if(bp_do->ParNE(L"MoveTo").Find(L"Any") >= 0){
//            m_Action.m_MoveTo = MOVETO_ANY;
//        }
//
//        //Patrol
//        if(bp_do->ParNE(L"Patrol").Find(L"Protected") >= 0){
//            m_Action.m_Patrol |= MOVETO_PROTECTED;
//        }
//        if(bp_do->ParNE(L"Patrol").Find(L"NearEntry") >= 0){
//            m_Action.m_Patrol |= MOVETO_NEAR_ENTRY;
//        }
//        if(bp_do->ParNE(L"Patrol").Find(L"FarEntry") >= 0){
//            m_Action.m_Patrol |= MOVETO_FAR_ENTRY;
//        }
//        if(bp_do->ParNE(L"Patrol").Find(L"BlockEntry") >= 0){
//            m_Action.m_Patrol |= MOVETO_BLOCK_ENTRY;
//        }
//        if(bp_do->ParNE(L"Patrol").Find(L"Any") >= 0){
//            m_Action.m_Patrol = MOVETO_ANY;
//        }
//
//
//        //GoTo
//        CWStr GoTo(bp_do->ParNE(L"GoTo"));
//
//        if(GoTo != L""){
//            CMatrixState *state_list = ((CMatrixLogicSlot*)((CMatrixState*)GetParent())->GetParent())->m_FirstState;
//            while(state_list){
//                if(state_list->GetName() == GoTo){
//                    m_Action.m_GoTo = state_list;
//                    break;
//                }
//                state_list = state_list->m_NextState;
//            }
//        }
//
//        //Exit
//        if(bp_do->ParNE(L"Exit").Find(L"True") >= 0){
//            m_Action.m_Exit = EXIT;
//        }else if(bp_do->ParNE(L"Exit").Find(L"False") >= 0){
//            m_Action.m_Exit = NOT_EXIT;
//        }
//
//        //Capture
//        if(bp_do->ParNE(L"Capture").Find(L"True") >= 0){
//            m_Action.m_Capture = CAPTURE;
//        }else if(bp_do->ParNE(L"Capture").Find(L"False") >= 0){
//            m_Action.m_Capture = NOT_CAPTURE;
//        }
//
//    }
//}
//
// bool CMatrixRule::If(CMatrixRobotAI *robot, CMatrixGroup* group)
//{
//    DTRACE();
//    m_ContextEnemy = NULL;
//    if(robot->m_CurrState != ROBOT_SUCCESSFULLY_BUILD)
//        return false;
//    if(robot == (CMatrixRobotAI*)g_MatrixMap->GetPlayerSide()->GetArcadedObject())
//        return false;
//
//    CMatrixTactics* parent_tactics =
//    ((CMatrixTactics*)((CMatrixLogicSlot*)((CMatrixState*)GetParent())->GetParent())->GetParent()); CMatrixLogicSlot*
//    parent_slot = ((CMatrixLogicSlot*)((CMatrixState*)GetParent())->GetParent()); if(m_Condition.m_DestroyedSlot !=
//    NULL){
//        return false;
//    }
//
//    if(m_Condition.m_EnemySpoted != ENEMY_UNDEF){
//        if((m_Condition.m_EnemySpoted & ENEMY_ANY) == ENEMY_ANY){
//            if(robot->GetEnv()->GetEnemyCnt() == 0){
//                return false;
//            }else{
//                m_ContextEnemy = robot->GetEnv()->GetEnemyByKind(ENEMY_ANY);
//            }
//        }else if((m_Condition.m_EnemySpoted & ENEMY_NONE) == ENEMY_NONE){
//            if(parent_slot->GetName() == L"ExitFromTactics"){
//                CMatrixGroupObject* gr_objects = group->m_FirstObject;
//                while(gr_objects){
//                    if(gr_objects->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI){
//                        if(((CMatrixRobotAI*)gr_objects->GetObject())->GetEnv()->GetEnemyCnt() > 0)
//                            return false;
//                    }
//                    gr_objects = gr_objects->m_NextObject;
//                }
//            }else{
//                if(robot->GetEnv()->GetEnemyCnt() > 0){
//                    return false;
//                }
//            }
//        }
//    }
//    if(m_Condition.m_Position != POSITION_UNDEF){
//        if(m_Condition.m_Position == POSITION_NEAREST_TO_TARGET && parent_tactics->GetNearestRobotAI() != robot){
//            return false;
//        }
//    }
//
//    return true;
//}
//
// void CMatrixRule::Do(CMatrixRobotAI *robot, CMatrixGroup* group)
//{
//    DTRACE();
//    if(robot->m_CurrState != ROBOT_SUCCESSFULLY_BUILD)
//        return;
//    CMatrixTactics* parent_tactics =
//    ((CMatrixTactics*)((CMatrixLogicSlot*)((CMatrixState*)GetParent())->GetParent())->GetParent());
//
//    if(m_Action.m_AttackEnemy == ATTACK){
//        if(parent_tactics->GetType() == ATTACK_TARGET_TACTICS){
//            robot->SetEnvTarget(parent_tactics->GetTarget());
//        }else{
//            robot->SetEnvTarget(m_ContextEnemy);
//        }
//
//        //if((m_Condition.m_EnemySpoted & ENEMY_ANY) == ENEMY_ANY)
//        //    robot->m_Environment.KillEnemyByKind(ENEMY_ANY);
//        //else
//        //    robot->m_Environment.KillEnemyByKind(ENEMY_UNDEF);
//
//    }else if(m_Action.m_AttackEnemy == NOT_ATTACK){
//        robot->SetEnvTarget(NULL);
//    }
//    if(m_Action.m_ChangeEnemy == CHANGE){
//    }
//    if(m_Action.m_Exit == EXIT){
//    }
//    if(m_Action.m_FleeFromEnemy == FLEE){
//    }
//    if(m_Action.m_GoTo != NULL){
//    }
//    if(m_Action.m_MoveTo != MOVETO_UNDEF){
//        if((m_Action.m_MoveTo & MOVETO_TARGETZONE) == MOVETO_TARGETZONE && parent_tactics->GetTarget() != NULL){
//            D3DXVECTOR3 mypos = D3DXVECTOR3(robot->m_PosX, robot->m_PosY, 0);
//            D3DXVECTOR3 tpos = D3DXVECTOR3(((CMatrixBuilding*)parent_tactics->GetTarget())->m_Pos.x,
//            ((CMatrixBuilding*)parent_tactics->GetTarget())->m_Pos.y, 0);
//
//            if(!robot->FindOrderLikeThat(ROT_MOVE_TO) && D3DXVec3LengthSq(&(mypos - tpos)) >
//            (TARGETZONE_R*TARGETZONE_R)){
//
//                int mx = 0, my = 0;
//                do{
//                    mx = int(g_MatrixMap->Rnd((int)tpos.x - TARGETZONE_R, (int)tpos.x + TARGETZONE_R) /
//                    GLOBAL_SCALE_MOVE);
//                }while(fabs(mx * GLOBAL_SCALE_MOVE - tpos.x) < TARGETZONE_MINR);
//
//                do{
//                    my = int(g_MatrixMap->Rnd((int)tpos.y - TARGETZONE_R, (int)tpos.y + TARGETZONE_R) /
//                    GLOBAL_SCALE_MOVE);
//                }while(fabs(my * GLOBAL_SCALE_MOVE - tpos.y) < TARGETZONE_MINR);
//
//                g_MatrixMap->PlaceFindNear(robot->m_Unit[0].m_Kind, 4, mx, my, robot);
//                robot->MoveTo(mx, my);
//            }
//        }else if((m_Action.m_MoveTo & MOVETO_MAINGROUP) == MOVETO_MAINGROUP){
//            CMatrixSideUnit*    my_side = g_MatrixMap->GetSideById(robot->m_Side);
//            CMatrixGroup*       groups = my_side->m_GroupsList->m_FirstGroup;
//            while(groups){
//                if(groups->GetTeam() == robot->GetTeam() && groups->GetGroupId() == 1){
//                    if(!robot->FindOrderLikeThat(ROT_MOVE_TO)){
//                        D3DXVECTOR3 gpos = groups->GetGroupPos();
//                        int mx = int(g_MatrixMap->Rnd((int)gpos.x - GROUPZONE_R, (int)gpos.x + GROUPZONE_R) /
//                        GLOBAL_SCALE_MOVE); int my = int(g_MatrixMap->Rnd((int)gpos.y - GROUPZONE_R, (int)gpos.y +
//                        GROUPZONE_R) / GLOBAL_SCALE_MOVE); g_MatrixMap->PlaceFindNear(robot->m_Unit[0].m_Kind, 4, mx,
//                        my, robot); robot->MoveTo(mx, my);
//                    }else{
//                        D3DXVECTOR3 gpos = groups->GetGroupPos();
//                        int mx = int(g_MatrixMap->Rnd((int)gpos.x - GROUPZONE_R, (int)gpos.x + GROUPZONE_R) /
//                        GLOBAL_SCALE_MOVE); int my = int(g_MatrixMap->Rnd((int)gpos.y - GROUPZONE_R, (int)gpos.y +
//                        GROUPZONE_R) / GLOBAL_SCALE_MOVE); g_MatrixMap->PlaceFindNear(robot->m_Unit[0].m_Kind, 4, mx,
//                        my, robot); robot->UpdateOrder_MoveTo(mx, my);
//                    }
//                    break;
//                }
//                groups = groups->m_NextGroup;
//            }
//
//        }else if((m_Action.m_MoveTo & MOVETO_AROUNDENEMY) == MOVETO_AROUNDENEMY){
//            if((m_AroundPeriod >= AROUNDENEMY_PERIOD || m_AroundPeriod == 0) && robot->GetEnv()->m_Target &&
//            robot->GetEnv()->m_Target->GetObjectType() == OBJECT_TYPE_ROBOTAI &&
//            !robot->FindOrderLikeThat(ROT_MOVE_TO)){
//                m_AroundPeriod = 0;
//
//                float ex = 0, ey = 0;
//                CMatrixRobotAI* target = (CMatrixRobotAI*)robot->GetEnv()->m_Target;
//                ex = target->m_PosX;
//                ey = target->m_PosY;
//
//
//                int mx = 0, my = 0;
//                do{
//                    mx = int(g_MatrixMap->Rnd((int)ex - AROUNDENEMY_R, (int)ex + AROUNDENEMY_R) / GLOBAL_SCALE_MOVE);
//                }while(fabs(mx * GLOBAL_SCALE_MOVE - ex) < AROUNDENEMY_MINR);
//
//                do{
//                    my = int(g_MatrixMap->Rnd((int)ey - AROUNDENEMY_R, (int)ey + AROUNDENEMY_R) / GLOBAL_SCALE_MOVE);
//                }while(fabs(my * GLOBAL_SCALE_MOVE - ey) < AROUNDENEMY_MINR);
//
//                g_MatrixMap->PlaceFindNear(robot->m_Unit[0].m_Kind, 4, mx, my, robot);
//                robot->MoveTo(mx, my);
//
//            }
//            m_AroundPeriod++;
//        }else if((m_Action.m_MoveTo & MOVETO_FAR_MANEUVER) == MOVETO_FAR_MANEUVER){
//            if(!robot->FindOrderLikeThat(ROBOT_FAR_MANEUVER) && m_ContextEnemy){
//                robot->FarManeuver(m_ContextEnemy);
//            }
//        }else if((m_Action.m_MoveTo & MOVETO_NEAR_MANEUVER) == MOVETO_NEAR_MANEUVER){
//            if(!robot->FindOrderLikeThat(ROBOT_NEAR_MANEUVER)){
//                if(m_ContextEnemy){
//                    robot->NearManeuver(m_ContextEnemy);
//                }else if(parent_tactics->GetType() == ATTACK_TARGET_TACTICS){
//                    robot->NearManeuver(parent_tactics->GetTarget());
//                }
//            }
//        }else if((m_Action.m_MoveTo & MOVETO_REGION) == MOVETO_REGION){
//
//            CPoint tp;
//            if(!robot->GetMoveToCoords(tp) || g_MatrixMap->m_RN.FindNerestRegion(tp)!=parent_tactics->GetRegion()) {
//                tp=g_MatrixMap->m_RN.m_Region[parent_tactics->GetRegion()].m_Center;
//                robot->MoveTo(tp.x,tp.y);
//            }
//        }
//    }
//    if(m_Action.m_Patrol != MOVETO_UNDEF){
//    }
//    if(m_Action.m_PursueEnemy == PURSUE){
//    }
//    if(m_Action.m_Capture == CAPTURE){
//        if(!robot->FindOrderLikeThat(ROT_CAPTURE_FACTORY)){
//            if(parent_tactics->GetTarget() != NULL)
//            robot->CaptureFactory((CMatrixBuilding*)parent_tactics->GetTarget());
//        }
//    }
//    if(m_Action.m_Exit == EXIT){
//    }
//}
//
// void CMatrixRule::LogicTakt()
//{
//
//}
//
