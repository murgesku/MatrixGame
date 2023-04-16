// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixAIGroup.h"
#include "../MatrixMap.hpp"
#include "../MatrixRobot.hpp"
#include "../MatrixSide.hpp"
#include "MatrixLogicSlot.h"
#include "MatrixRule.h"
#include "MatrixState.h"
#include "../MatrixFlyer.hpp"
//#include "MatrixTactics.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_________CMatrixGroupRobot_____________________PLease stand up______________________________________________________//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMatrixGroupObject::~CMatrixGroupObject() {}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//___________CMatrixGroup________________________May i have your ATTENTION please!____________________________________//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMatrixGroup::CMatrixGroup() {
    m_NextGroup = NULL;
    m_PrevGroup = NULL;
    m_FirstObject = NULL;
    m_LastObject = NULL;
    m_ObjectsCnt = 0;
    m_RobotsCnt = 0;
    m_FlyersCnt = 0;
    m_BuildingsCnt = 0;
    m_Team = -1;
    // m_Tactics       = NULL;
    m_Id = 0;
    m_GroupPosition = D3DXVECTOR3(0, 0, 0);
    m_GroupSpeed = 0;
    m_SimpleTimer = -1;
}

CMatrixGroup::~CMatrixGroup() {
    DTRACE();
    RemoveAll();
    // if(m_Tactics)
    //    HDelete(CMatrixTactics, m_Tactics, g_MatrixHeap);
}

void CMatrixGroup::AddObject(CMatrixMapStatic *object, int team) {
    CMatrixGroupObject *objects = m_FirstObject;
    while (objects) {
        if (objects->GetObject() == object)
            return;
        objects = objects->m_NextObject;
    }
    CMatrixGroupObject *g_object = HNew(g_MatrixHeap) CMatrixGroupObject;
    g_object->SetObject(object);
    g_object->SetParentGroup(this);
    g_object->SetTeam(team);
    LIST_ADD(g_object, m_FirstObject, m_LastObject, m_PrevObject, m_NextObject);
    if (object->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
        m_RobotsCnt++;
    }
    else if (object->GetObjectType() == OBJECT_TYPE_FLYER) {
        m_FlyersCnt++;
    }
    else if (object->GetObjectType() == OBJECT_TYPE_BUILDING) {
        m_BuildingsCnt++;
    }
    m_ObjectsCnt++;
    CalcGroupPosition();
    CalcGroupSpeed();
}

void CMatrixGroup::RemoveObject(CMatrixMapStatic *object) {
    CMatrixGroupObject *g_object = m_FirstObject;
    while (g_object) {
        if (g_object->GetObject() == object) {
            LIST_DEL(g_object, m_FirstObject, m_LastObject, m_PrevObject, m_NextObject);
            HDelete(CMatrixGroupObject, g_object, g_MatrixHeap);
            if (object->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
                m_RobotsCnt--;
            }
            else if (object->GetObjectType() == OBJECT_TYPE_FLYER) {
                m_FlyersCnt--;
            }
            else if (object->GetObjectType() == OBJECT_TYPE_BUILDING) {
                m_BuildingsCnt--;
            }
            m_ObjectsCnt--;
            CalcGroupPosition();
            CalcGroupSpeed();

            return;
        }
        g_object = g_object->m_NextObject;
    }
}

void CMatrixGroup::RemoveAll() {
    DTRACE();
    CMatrixGroupObject *Objects = m_FirstObject;

    while (Objects != NULL) {
        if (Objects->m_NextObject)
            Objects = Objects->m_NextObject;
        else {
            HDelete(CMatrixGroupObject, Objects, g_MatrixHeap);
            Objects = NULL;
            m_FirstObject = NULL;
            m_LastObject = NULL;
        }

        if (Objects)
            HDelete(CMatrixGroupObject, Objects->m_PrevObject, g_MatrixHeap);
    }

    m_RobotsCnt = 0;
    m_FlyersCnt = 0;
    m_BuildingsCnt = 0;
    m_ObjectsCnt = 0;
}

void CMatrixGroup::FindNearObjects(CMatrixGroupObject *fn_object) {
    CMatrixGroupObject *gr_objects = m_FirstObject;

    while (gr_objects) {
        CMatrixMapStatic *ai_object = gr_objects->GetObject();

        if (ai_object->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
            if (((CMatrixRobotAI *)ai_object)->GetGroup() != 0 || (fn_object->GetTeam() != gr_objects->GetTeam())) {
                gr_objects = gr_objects->m_NextObject;
                continue;
            }
        }
        else if (ai_object->GetObjectType() == OBJECT_TYPE_FLYER) {
            if (((CMatrixFlyer *)ai_object)->m_Group != 0 || (fn_object->GetTeam() != gr_objects->GetTeam())) {
                gr_objects = gr_objects->m_NextObject;
                continue;
            }
        }

        // check r

        D3DXVECTOR2 fr(0, 0);
        D3DXVECTOR2 ar(0, 0);
        int *ai_gr = NULL, *fn_gr = NULL;

        if (fn_object->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
            fn_gr = ((CMatrixRobotAI *)fn_object->GetObject())->GetGroupP();
            fr = D3DXVECTOR2(((CMatrixRobotAI *)fn_object->GetObject())->m_PosX,
                             ((CMatrixRobotAI *)fn_object->GetObject())->m_PosY);
        }
        else if (fn_object->GetObject()->GetObjectType() == OBJECT_TYPE_FLYER) {
            fn_gr = &((CMatrixFlyer *)fn_object->GetObject())->m_Group;
            fr = D3DXVECTOR2(((CMatrixFlyer *)fn_object->GetObject())->GetPos().x,
                             ((CMatrixFlyer *)fn_object->GetObject())->GetPos().y);
        }

        if (ai_object->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
            ai_gr = ((CMatrixRobotAI *)ai_object)->GetGroupP();
            ar = D3DXVECTOR2(((CMatrixRobotAI *)ai_object)->m_PosX, ((CMatrixRobotAI *)ai_object)->m_PosY);
        }
        else if (ai_object->GetObjectType() == OBJECT_TYPE_FLYER) {
            ai_gr = &((CMatrixFlyer *)ai_object)->m_Group;
            ar = D3DXVECTOR2(((CMatrixFlyer *)ai_object)->GetPos().x, ((CMatrixFlyer *)ai_object)->GetPos().y);
        }

        const auto tmp = ar - fr;
        if (D3DXVec2LengthSq(&tmp) > 0 && D3DXVec2LengthSq(&tmp) <= DIF_GROUP_R * DIF_GROUP_R) {
            *ai_gr = *fn_gr;
            // ai_object->m_Group = fn_object->GetObject()->m_Group;

            FindNearObjects(gr_objects);
        }
        //
        gr_objects = gr_objects->m_NextObject;
    }
}

// void CMatrixGroup::InstallTactics(TacticsType type, CBlockPar* par)
//{
//    if(par == NULL)
//        return;
//    if(m_Tactics != NULL)
//        DeInstallTactics();
//
//    CWStr stype;
//    switch(type){
//        case ATTACK_TARGET_TACTICS:
//            stype = L"AttackTarget";
//            break;
//        case ATTACK_TACTICS:
//            stype = L"Attack";
//            break;
//        case CAPTURE_TACTICS:
//            stype = L"Capture";
//            break;
//        case PROTECTION_TACTICS:
//            stype = L"Protection";
//            break;
//        case JOIN_TACTICS:
//            stype = L"Join";
//            break;
//        case MOVE_TACTICS:
//            stype = L"Move";
//            break;
//        default:
//            return;
//
//    }
//
//    int tactics_cnt = par->BlockCount();
//    if(tactics_cnt <= 0)
//        return;
//////Tactics loading
//    for(int cnt = 0;cnt < tactics_cnt; cnt++){
//        if(par->BlockGet(cnt)->ParGetNE(L"Type") == stype){
//            m_Tactics = HNew(g_MatrixHeap) CMatrixTactics;
//            m_Tactics->SetName(par->BlockGetName(cnt));
//            m_Tactics->SetParent(NULL);
//            m_Tactics->Load(*par->BlockGet(cnt));
//            break;
//        }
//    }
//}
//
// void CMatrixGroup::DeInstallTactics()
//{
//    if(m_Tactics == NULL)
//        return;
//
//    CMatrixLogicSlot*   slots = m_Tactics->m_FirstSlot;
//
//    while(slots){
//        CLogicSlotRobot* robots = slots->m_FirstRobot;
//        while(robots){
//            robots->GetLogicRobot()->BreakAllOrders();
//            robots = robots->m_NextRobot;
//        }
//        slots = slots->m_NextSlot;
//    }
//
//    m_Tactics->Reset();
//    //ZeroMemory(m_Tactics, sizeof(CMatrixTactics));
//    HDelete(CMatrixTactics, m_Tactics, g_MatrixHeap);
//    m_Tactics = NULL;
//}

void CMatrixGroup::LogicTakt(CMatrixSideUnit *side) {
    if (m_SimpleTimer >= JUST_PERIOD || m_SimpleTimer == -1) {
        CalcGroupPosition();
        CalcGroupSpeed();
        m_SimpleTimer = 0;
    }

    m_SimpleTimer++;

    // if(m_Tactics){
    //    m_Tactics->LogicTakt(side, this);
    //}
}

void CMatrixGroup::CalcGroupPosition() {
    CMatrixGroupObject *objects = m_FirstObject;

    D3DXVECTOR3 pos(0, 0, 0);
    while (objects) {
        CMatrixMapStatic *ai_object = objects->GetObject();
        if (ai_object->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
            pos.x += ((CMatrixRobotAI *)ai_object)->m_PosX;
            pos.y += ((CMatrixRobotAI *)ai_object)->m_PosY;
        }
        else if (ai_object->GetObjectType() == OBJECT_TYPE_FLYER) {
            pos.x += ((CMatrixFlyer *)ai_object)->GetPos().x;
            pos.y += ((CMatrixFlyer *)ai_object)->GetPos().y;
        }

        objects = objects->m_NextObject;
    }
    if (m_ObjectsCnt > 1) {
        pos.x /= m_ObjectsCnt;
        pos.y /= m_ObjectsCnt;
    }
    m_GroupPosition = pos;
}

void CMatrixGroup::CalcGroupSpeed() {
    CMatrixGroupObject *objects = m_FirstObject;
    float lowest_speed = 0;

    while (objects) {
        CMatrixMapStatic *ai_object = objects->GetObject();
        if (ai_object->GetObjectType() == OBJECT_TYPE_ROBOTAI &&
            ((CMatrixRobotAI *)ai_object)->GetMaxSpeed() < lowest_speed &&
            ((CMatrixRobotAI *)ai_object)->m_CurrState == ROBOT_SUCCESSFULLY_BUILD /* || lowest_speed == 0*/) {
            lowest_speed = ((CMatrixRobotAI *)ai_object)->GetMaxSpeed();
        }
        else if (ai_object->GetObjectType() == OBJECT_TYPE_FLYER &&
                 ((CMatrixFlyer *)ai_object)->GetSpeed() < lowest_speed) {
            lowest_speed = ((CMatrixFlyer *)ai_object)->GetSpeed();
        }

        objects = objects->m_NextObject;
    }
    m_GroupSpeed = lowest_speed;
}

void CMatrixGroup::RemoveBuildings() {
    if (m_BuildingsCnt) {
        CMatrixGroupObject *go = m_FirstObject;

        while (go) {
            if (go->GetObject()->GetObjectType() == OBJECT_TYPE_BUILDING) {
                CMatrixGroupObject *go2 = go->m_NextObject;
                LIST_DEL(go, m_FirstObject, m_LastObject, m_PrevObject, m_NextObject);
                HDelete(CMatrixGroupObject, go, g_MatrixHeap);
                m_BuildingsCnt--;
                m_ObjectsCnt--;
                go = go2;
                continue;
            }

            go = go->m_NextObject;
        }
    }
}

void CMatrixGroup::SortFlyers() {
    CMatrixGroupObject *go = m_FirstObject;

    while (go->GetObject()->GetObjectType() == OBJECT_TYPE_FLYER) {
        LIST_DEL(go, m_FirstObject, m_LastObject, m_PrevObject, m_NextObject);
        LIST_ADD(go, m_FirstObject, m_LastObject, m_PrevObject, m_NextObject);
        go = m_FirstObject;
    }
}

CMatrixMapStatic *CMatrixGroup::GetObjectByN(int num) {
    if (num > m_ObjectsCnt) {
        return NULL;
    }
    CMatrixGroupObject *go = m_FirstObject;

    while (go && num--) {
        go = go->m_NextObject;
    }

    if (go) {
        return go->GetObject();
    }
    return NULL;
}

bool CMatrixGroup::RemoveObject(int num) {
    CMatrixMapStatic *o = GetObjectByN(num);
    if (o) {
        RemoveObject(o);
        return true;
    }
    return false;
}

bool CMatrixGroup::FindObject(CMatrixMapStatic *object) {
    DTRACE();
    CMatrixGroupObject *objects = m_FirstObject;

    while (objects) {
        if (objects->GetObject() == object)
            return true;
        objects = objects->m_NextObject;
    }
    return false;
}

int CMatrixGroup::GetBombersCnt() {
    DTRACE();

    int cnt = 0;
    CMatrixGroupObject *objects = m_FirstObject;

    while (objects) {
        if (objects->GetObject()->IsLiveRobot()) {
            if (objects->GetObject()->AsRobot()->FindWeapon(WEAPON_BIGBOOM)) {
                cnt++;
            }
        }
        objects = objects->m_NextObject;
    }

    return cnt;
}

int CMatrixGroup::GetRepairsCnt() {
    DTRACE();

    int cnt = 0;
    CMatrixGroupObject *objects = m_FirstObject;

    while (objects) {
        if (objects->GetObject()->IsLiveRobot()) {
            if (objects->GetObject()->AsRobot()->FindWeapon(WEAPON_REPAIR)) {
                cnt++;
            }
        }
        objects = objects->m_NextObject;
    }

    return cnt;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_______CMatrixGroupList___PAPA BEAR TO ALPHA TEAM: Next you'll see new class implementation.. out.__________________//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CMatrixGroupList::~CMatrixGroupList()
//{
//    DTRACE();
//    CMatrixGroup *groups = m_FirstGroup;
//
//	while(groups != NULL){
//        if(groups->m_NextGroup){
//			groups = groups->m_NextGroup;
//        } else {
//			HDelete(CMatrixGroup, groups, g_MatrixHeap);
//			groups = NULL;
//			m_FirstGroup = NULL;
//			m_LastGroup = NULL;
//
//		}
//
//		if(groups)
//			HDelete(CMatrixGroup, groups->m_PrevGroup, g_MatrixHeap);
//	}
//}
//
// CMatrixGroup* CMatrixGroupList::AddNewGroup(int team, int id)
//{
//    CMatrixGroup* group = HNew(g_MatrixHeap) CMatrixGroup;
//    group->SetTeam(team);
//    group->SetGroupId(id);
//    LIST_ADD(group, m_FirstGroup, m_LastGroup, m_PrevGroup, m_NextGroup);
//    m_GroupsCnt++;
//    return group;
//}
//
// void CMatrixGroupList::RemoveGroup(CMatrixGroup* group)
//{
//    CMatrixGroup* groups = m_FirstGroup;
//    while(groups){
//        if(groups == group){
//            LIST_DEL(groups, m_FirstGroup, m_LastGroup, m_PrevGroup, m_NextGroup);
//            HDelete(CMatrixGroup, groups, g_MatrixHeap);
//            m_GroupsCnt--;
//            return;
//        }
//        groups = groups->m_NextGroup;
//    }
//}
//
// void CMatrixGroupList::RemoveGroup(int team)
//{
//    CMatrixGroup* groups = m_FirstGroup;
//    while(groups){
//        if(groups->GetTeam() == team){
//            LIST_DEL(groups, m_FirstGroup, m_LastGroup, m_PrevGroup, m_NextGroup);
//            HDelete(CMatrixGroup, groups, g_MatrixHeap);
//            m_GroupsCnt--;
//            return;
//        }
//        groups = groups->m_NextGroup;
//    }
//}
//
// void CMatrixGroupList::ReGroup(CMatrixSideUnit* side)
//{
//    if(m_GroupsCnt > 0 && m_ReGroupPeriod < REGROUP_PERIOD)
//        return;
//
//    m_ReGroupPeriod = 0;
//    CMatrixMapStatic* objects = CMatrixMapStatic::GetFirstLogic();
//
//    CMatrixGroup* side_objects = AddNewGroup(-2, 0);
//    while(objects){
//        if(objects->GetObjectType() == OBJECT_TYPE_ROBOTAI && ((CMatrixRobotAI*)objects)->m_Side == side->m_Id){
//            ((CMatrixRobotAI*)objects)->SetGroup(0);
//            side_objects->AddObject(objects, ((CMatrixRobotAI*)objects)->GetTeam());
//        }else if(objects->GetObjectType() == OBJECT_TYPE_FLYER && ((CMatrixFlyer*)objects)->GetSide() == side->m_Id){
//            ((CMatrixFlyer*)objects)->m_Group = 0;
//            side_objects->AddObject(objects, ((CMatrixFlyer*)objects)->m_Team);
//        }
//        objects = objects->GetNextLogic();
//    }
//    if(side_objects->GetObjectsCnt() <= 0){
//        RemoveGroup(side_objects);
//        return;
//    }
//
//    for(int cnt = 0; cnt < 3; cnt++){
//        CMatrixGroupObject* gr_objects = side_objects->m_FirstObject;
//        int group = 1;
//        int team = cnt;
//        while(gr_objects){
//            if(gr_objects->GetTeam() == team){
//                CMatrixMapStatic* ai_object = gr_objects->GetObject();
//                int ai_group = 0;
//                if(ai_object->GetObjectType() == OBJECT_TYPE_ROBOTAI && ((CMatrixRobotAI*)ai_object)->GetGroup() == 0)
//                {
//                    ((CMatrixRobotAI*)ai_object)->SetGroup(group);
//                    ai_group = group;
//                    group++;
//                }else if(ai_object->GetObjectType() == OBJECT_TYPE_FLYER && ((CMatrixFlyer*)ai_object)->m_Group == 0){
//                    ((CMatrixFlyer*)ai_object)->m_Group = group;
//                    ai_group = group;
//                    group++;
//                }
//                AddObject(team, ai_group, ai_object);
//                side_objects->FindNearObjects(gr_objects);
//            }
//            gr_objects = gr_objects->m_NextObject;
//        }
//
//    }
//    RemoveGroup(side_objects);
//
//    CMatrixGroup*   groups = m_FirstGroup;
//
//    while(groups){
//        CMatrixGroupObject* objects = groups->m_FirstObject;
//        while(objects){
//            int ob_group = 0;
//            if(objects->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI){
//                ob_group = ((CMatrixRobotAI*)objects->GetObject())->GetGroup();
//            }else if(objects->GetObject()->GetObjectType() == OBJECT_TYPE_FLYER){
//                ob_group = ((CMatrixFlyer*)objects->GetObject())->m_Group;
//            }
//
//            if(ob_group != groups->GetGroupId()){
//                CMatrixGroupObject* rb = objects;
//                objects = objects->m_NextObject;
//                if(groups->GetTactics()){
//                    groups->GetTactics()->RemoveObjectFromT(rb->GetObject());
//                }
//
//                groups->RemoveObject(rb->GetObject());
//                continue;
//            }
//            objects = objects->m_NextObject;
//        }
//        if(groups->GetObjectsCnt() <= 0){
//            CMatrixGroup* gr = groups;
//            groups = groups->m_NextGroup;
//            RemoveGroup(gr);
//            continue;
//        }
//        if(groups->GetTactics()){
//            groups->GetTactics()->InitialiseTactics(groups,groups->GetTactics()->GetTarget(),groups->GetTactics()->GetRegion());
//        }
//
//        groups = groups->m_NextGroup;
//    }
//}
//
// void CMatrixGroupList::LogicTakt(CMatrixSideUnit* side)
//{
//    m_ReGroupPeriod++;
//    if(m_GroupsCnt == 0)
//        return;
//
//    CMatrixGroup* groups = m_FirstGroup;
//
//    while(groups){
//        groups->LogicTakt(side);
//        groups = groups->m_NextGroup;
//    }
//}
//
// void CMatrixGroupList::AddObject(int team, int id, CMatrixMapStatic* object)
//{
//    CMatrixGroup* grouppies = m_FirstGroup, *group = NULL;
//    while(grouppies){
//        if(grouppies->GetGroupId() == id && grouppies->GetTeam() == team){
//            group = grouppies;
//            break;
//        }
//        grouppies = grouppies->m_NextGroup;
//    }
//
//    if(group == NULL)
//        group = AddNewGroup(team, id);
//
//    group->AddObject(object, team);
//}
//
// void CMatrixGroupList::RemoveObject(int team, int id, CMatrixMapStatic* object)
//{
//    CMatrixGroup* groups = m_FirstGroup;
//    while(groups){
//        if(groups->GetTeam() == team && groups->GetGroupId() == id){
//            CMatrixTactics* t = groups->GetTactics();
//            if(t) t->RemoveObjectFromT(object);
//            groups->RemoveObject(object);
//            return;
//        }
//        groups = groups->m_NextGroup;
//    }
//}

// void CMatrixGroupList::AddRobotToTeam(Team team, CMatrixRobotAI* robot)
//{
//    CMatrixGroup* groups = m_FirstGroup;
//    while(groups){
//        if(groups->GetTeam() == team){
//            groups->AddRobot(robot, team);
//            return;
//        }
//        groups = groups->m_NextGroup;
//    }
//
//}
//
// void CMatrixGroupList::RemoveRobotFromTeam(Team team, CMatrixRobotAI* robot)
//{
//    CMatrixGroup* groups = m_FirstGroup;
//    while(groups){
//        if(groups->GetTeam() == team){
//            groups->RemoveRobot(robot);
//            return;
//        }
//        groups = groups->m_NextGroup;
//    }
//
//}
//
