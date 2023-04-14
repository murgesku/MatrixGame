// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "CIFaceElement.h"
#include "CCounter.h"
#include "../MatrixSide.hpp"
#include "CConstructor.h"
#include "CInterface.h"

CIFaceCounter::CIFaceCounter() {
    m_Counter = 1;
    ZeroMemory(m_CounterImage, sizeof(m_CounterImage));
    m_ButtonDown = NULL;
    m_ButtonUp = NULL;
}

CIFaceCounter::~CIFaceCounter() {}

void CIFaceCounter::ManageButtons() {
    if ((m_Counter == UP_LIMIT) && m_ButtonUp) {
        m_ButtonUp->SetState(IFACE_DISABLED);
    }
    else if (m_ButtonUp) {
        m_ButtonUp->SetState(IFACE_NORMAL);
    }

    if (m_Counter == DOWN_LIMIT && m_ButtonDown) {
        m_ButtonDown->SetState(IFACE_DISABLED);
    }
    else if (m_ButtonDown) {
        m_ButtonDown->SetState(IFACE_NORMAL);
    }
}

void CIFaceCounter::MulRes() {
    g_IFaceList->CreateSummPrice(m_Counter);
    // CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
    // SPrice price;
    // ps->m_ConstructPanel->m_Configs[ps->m_ConstructPanel->m_CurrentConfig].CalcConfigPrice(&price);
    // for(int i = 0; i < MAX_RESOURCES; i++){
    //    ps->m_ConstructPanel->m_Configs[ps->m_ConstructPanel->m_CurrentConfig].m_TotalPrice.m_Resources[i] =
    //    price.m_Resources[i] * m_Counter;
    //}
}

void CIFaceCounter::DivRes() {
    g_IFaceList->CreateSummPrice(m_Counter);

    // CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
    // SPrice price;
    // ps->m_ConstructPanel->m_Configs[ps->m_ConstructPanel->m_CurrentConfig].CalcConfigPrice(&price);
    // for(int i = 0; i < MAX_RESOURCES; i++){
    //    ps->m_ConstructPanel->m_Configs[ps->m_ConstructPanel->m_CurrentConfig].m_TotalPrice.m_Resources[i] =
    //    price.m_Resources[i] * m_Counter;
    //}
}

void CIFaceCounter::CheckUp() {
    // check for robot_summ * m_Counter <= player_res

    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
    // SPrice price;
    // ps->m_ConstructPanel->m_Configs[ps->m_ConstructPanel->m_CurrentConfig].CalcConfigPrice(&price);
    int resources[MAX_RESOURCES];
    int res[MAX_RESOURCES], res2[MAX_RESOURCES];

    ps->m_Constructor->GetConstructionPrice(resources);

    for (int i = 0; i < MAX_RESOURCES; i++) {
        res[i] = resources[i] * (m_Counter + 1);
        res2[i] = resources[i] * (m_Counter);
    }
    if (!ps->IsEnoughResources(res2)) {
        if (m_ButtonUp)
            m_ButtonUp->SetState(IFACE_DISABLED);
        if (m_ButtonDown)
            m_ButtonUp->SetState(IFACE_DISABLED);
        m_Counter = 0;
    }
    if (!ps->IsEnoughResources(res) ||
        (ps->GetSideRobots() + ps->GetRobotsInStack() + m_Counter >= ps->GetMaxSideRobots())) {
        if (m_ButtonUp)
            m_ButtonUp->SetState(IFACE_DISABLED);
    }

    if (ps->m_ActiveObject && ps->m_ActiveObject->IsBuilding() && ps->m_ActiveObject->IsBase()) {
        CMatrixBuilding *base = (CMatrixBuilding *)ps->m_ActiveObject;
        //
        if ((base->m_BS.GetItemsCnt() + m_Counter + 1) > 6 /*max stack items*/) {
            if (m_ButtonUp)
                m_ButtonUp->SetState(IFACE_DISABLED);
        }
    }
}