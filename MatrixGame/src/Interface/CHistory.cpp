// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "CHistory.h"
#include "../MatrixGame.h"
#include "../MatrixMap.hpp"

CHistory *g_ConfigHistory;

CHistory::CHistory() {
    m_FirstConfig = NULL;
    m_LastConfig = NULL;
    m_CurrentConfig = NULL;
}

CHistory::~CHistory() {
    SRobotConfig *cfgs = m_FirstConfig;

    while (cfgs) {
        SRobotConfig *next = cfgs->m_NextConfig;
        HDelete(SRobotConfig, cfgs, g_MatrixHeap);
        cfgs = next;
    }
    m_FirstConfig = NULL;
    m_LastConfig = NULL;
}

void CHistory::AddConfig(SRobotConfig *config) {
    DTRACE();
    DelConfig(config);
    SRobotConfig *new_cfg = HNew(g_MatrixHeap) SRobotConfig;
    memcpy(new_cfg, config, sizeof(SRobotConfig));
    m_CurrentConfig = new_cfg;
    LIST_ADD(new_cfg, m_FirstConfig, m_LastConfig, m_PrevConfig, m_NextConfig);
}

void CHistory::DelConfig(SRobotConfig *config) {
    DTRACE();
    SRobotConfig *confs = m_FirstConfig;
    while (confs) {
        bool gotcha = false;
        if (confs->m_Head.m_nKind == config->m_Head.m_nKind && confs->m_Chassis.m_nKind == config->m_Chassis.m_nKind &&
            confs->m_Hull.m_Unit.m_nKind == config->m_Hull.m_Unit.m_nKind) {
            gotcha = true;
        }

        if (gotcha) {
            for (int i = 0; i < MAX_WEAPON_CNT; i++) {
                if (confs->m_Weapon[i].m_nKind != config->m_Weapon[i].m_nKind) {
                    gotcha = false;
                    break;
                }
            }
        }

        if (gotcha) {
            SRobotConfig *next = confs->m_NextConfig;
            LIST_DEL(confs, m_FirstConfig, m_LastConfig, m_PrevConfig, m_NextConfig);
            HDelete(SRobotConfig, confs, g_MatrixHeap);
            confs = next;
            continue;
        }
        confs = confs->m_NextConfig;
    }
}

void CHistory::LoadCurrentConfigToConstructor() {
    DTRACE();
    if (m_CurrentConfig) {
        CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
        ps->m_Constructor->SuperDjeans(MRT_HEAD, m_CurrentConfig->m_Head.m_nKind, 0, true);
        ps->m_Constructor->SuperDjeans(MRT_ARMOR, m_CurrentConfig->m_Hull.m_Unit.m_nKind, 0, true);
        for (int i = 0; i < MAX_WEAPON_CNT; i++) {
            if (m_CurrentConfig->m_Weapon[i].m_nKind)
                ps->m_Constructor->SuperDjeans(MRT_WEAPON, m_CurrentConfig->m_Weapon[i].m_nKind, i, true);
        }
        ps->m_Constructor->SuperDjeans(MRT_CHASSIS, m_CurrentConfig->m_Chassis.m_nKind, 0, true);
    }
}

void __stdcall CHistory::PrevConfig(void *object) {
    DTRACE();
    if (m_CurrentConfig && m_CurrentConfig->m_PrevConfig) {
        m_CurrentConfig = m_CurrentConfig->m_PrevConfig;
        LoadCurrentConfigToConstructor();
    }
}

void __stdcall CHistory::NextConfig(void *object) {
    DTRACE();
    if (m_CurrentConfig && m_CurrentConfig->m_NextConfig) {
        m_CurrentConfig = m_CurrentConfig->m_NextConfig;
        LoadCurrentConfigToConstructor();
    }
}
