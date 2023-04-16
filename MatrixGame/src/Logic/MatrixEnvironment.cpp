// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixEnvironment.h"
#include "MatrixRule.h"
#include "../MatrixMap.hpp"
#include "../MatrixRobot.hpp"
#include "../MatrixObjectCannon.hpp"

CEnemy::CEnemy() {
    m_Enemy = NULL;
    m_EnemyKind = ENEMY_UNDEF;
    m_PrevEnemy = NULL;
    m_NextEnemy = NULL;
    m_DelSlowly = 0;
}

CEnemy::~CEnemy() {}

void CEnemy::ClassifyEnemy(CMatrixMapStatic *relTo) {
    m_EnemyKind = ENEMY_ANY;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CInfo::CInfo() {
    m_EnemyCnt = 0;
    m_FirstEnemy = NULL;
    m_LastEnemy = NULL;
    m_EnemyKind = 0;
    m_Target = NULL;
    m_TargetAttack = NULL;
    m_TargetLast = NULL;
    m_TargetChange = 0;
    m_TargetChangeRepair = 0;
    m_TargetAngle = 0.0f;

    m_Place = -1;
    m_PlaceAdd = CPoint(-1, -1);
    m_PlaceNotFound = -1000000;
    m_OrderNoBreak = false;
    m_LastFire = 0;
    m_LastHitTarget = 0;
    m_LastHitEnemy = 0;
    m_LastHitFriendly = 0;

    m_BadPlaceCnt = 0;
    m_BadCoordCnt = 0;
    m_IgnoreCnt = 0;
}

CInfo::~CInfo() {
    Clear();
}

void CInfo::Clear(void) {
    DTRACE();
    CEnemy *enemies = m_FirstEnemy;

    while (enemies) {
        CEnemy *next = enemies->m_NextEnemy;
        HDelete(CEnemy, enemies, g_MatrixHeap);
        enemies = next;
    }
    m_FirstEnemy = NULL;
    m_LastEnemy = NULL;
}

void CInfo::RemoveAllBuilding(CMatrixMapStatic *skip) {
    if (m_Target != skip && m_Target && m_Target->GetObjectType() == OBJECT_TYPE_BUILDING)
        m_Target = NULL;
    if (m_TargetAttack != skip && m_TargetAttack && m_TargetAttack->GetObjectType() == OBJECT_TYPE_BUILDING)
        m_TargetAttack = NULL;

    CEnemy *enemie = m_FirstEnemy;
    while (enemie) {
        CEnemy *e2 = enemie;
        enemie = enemie->m_NextEnemy;
        if (e2->GetEnemy() != skip && e2->GetEnemy()->GetObjectType() == OBJECT_TYPE_BUILDING) {
            LIST_DEL(e2, m_FirstEnemy, m_LastEnemy, m_PrevEnemy, m_NextEnemy);
            HDelete(CEnemy, e2, g_MatrixHeap);
            m_EnemyCnt--;
        }
    }
}

void CInfo::RemoveAllSlowely() {
    CEnemy *enemie = m_FirstEnemy;
    while (enemie) {
        CEnemy *e2 = enemie;
        enemie = enemie->m_NextEnemy;
        if (e2->m_DelSlowly) {
            if (e2->GetEnemy() == m_Target)
                m_Target = NULL;
            if (e2->GetEnemy() == m_TargetAttack)
                m_TargetAttack = NULL;

            LIST_DEL(e2, m_FirstEnemy, m_LastEnemy, m_PrevEnemy, m_NextEnemy);
            HDelete(CEnemy, e2, g_MatrixHeap);
            m_EnemyCnt--;
        }
    }
}

void CInfo::RemoveFromList(CMatrixMapStatic *ms) {
    if (m_Target == ms)
        m_Target = NULL;
    if (m_TargetAttack == ms)
        m_TargetAttack = NULL;

    CEnemy *enemy = SearchEnemy(ms);
    if (!enemy)
        return;

    RemoveFromList(enemy);
}

void CInfo::RemoveFromList(CEnemy *enemy) {
    LIST_DEL(enemy, m_FirstEnemy, m_LastEnemy, m_PrevEnemy, m_NextEnemy);
    HDelete(CEnemy, enemy, g_MatrixHeap);
    m_EnemyCnt--;
}

void CInfo::RemoveFromListSlowly(CMatrixMapStatic *ms) {
    CEnemy *enemy = SearchEnemy(ms);
    if (!enemy) {
        if (m_Target == ms)
            m_Target = NULL;
        if (m_TargetAttack == ms)
            m_TargetAttack = NULL;
        return;
    }

    enemy->m_DelSlowly++;
    if (enemy->m_DelSlowly >= 3) {
        if (m_Target == ms)
            m_Target = NULL;
        if (m_TargetAttack == ms)
            m_TargetAttack = NULL;

        LIST_DEL(enemy, m_FirstEnemy, m_LastEnemy, m_PrevEnemy, m_NextEnemy);
        HDelete(CEnemy, enemy, g_MatrixHeap);
        m_EnemyCnt--;
    }
}

void CInfo::AddToList(CMatrixMapStatic *ms) {
    DelIgnore(ms);

    CEnemy *enemy = HNew(g_MatrixHeap) CEnemy;
    enemy->SetEnemy(ms);
    enemy->ClassifyEnemy(ms);
    LIST_ADD(enemy, m_FirstEnemy, m_LastEnemy, m_PrevEnemy, m_NextEnemy);
    m_EnemyCnt++;
}

void CInfo::AddToListSlowly(CMatrixMapStatic *ms) {
    DelIgnore(ms);

    CEnemy *enemy = SearchEnemy(ms);
    if (enemy) {
        enemy->m_DelSlowly = 0;
        return;
    }

    enemy = HNew(g_MatrixHeap) CEnemy;
    enemy->SetEnemy(ms);
    enemy->ClassifyEnemy(ms);
    LIST_ADD(enemy, m_FirstEnemy, m_LastEnemy, m_PrevEnemy, m_NextEnemy);
    m_EnemyCnt++;
}

void CInfo::Reset() {
    m_EnemyKind = ENEMY_UNDEF;
    m_Target = NULL;
    m_TargetAttack = NULL;
}

CEnemy *CInfo::SearchEnemy(CMatrixMapStatic *ms) {
    CEnemy *enemie = m_FirstEnemy;
    while (enemie) {
        if (enemie->GetEnemy() == ms)
            return enemie;
        enemie = enemie->m_NextEnemy;
    }
    return NULL;
}

CMatrixMapStatic *CInfo::GetEnemyByKind(uint32_t kind) {
    CEnemy *enemies = m_FirstEnemy;
    while (enemies) {
        if (enemies->GetKind() == kind)
            return enemies->GetEnemy();
        enemies = enemies->m_NextEnemy;
    }
    return NULL;
}
void CInfo::AddBadPlace(int place) {
    if (IsBadPlace(place))
        return;

    if (m_BadPlaceCnt < 8) {
        m_BadPlace[m_BadPlaceCnt] = place;
        m_BadPlaceCnt++;
    }
    else {
        CopyMemory(m_BadPlace, m_BadPlace + 1, (8 - 1) * sizeof(int));
        m_BadPlace[m_BadPlaceCnt - 1] = place;
    }
}

bool CInfo::IsBadPlace(int place) {
    for (int i = 0; i < m_BadPlaceCnt; i++) {
        if (m_BadPlace[i] == place)
            return true;
    }
    return false;
}

void CInfo::AddBadCoord(const CPoint &coord) {
    if (IsBadCoord(coord))
        return;

    if (m_BadCoordCnt < 16) {
        m_BadCoord[m_BadCoordCnt] = coord;
        m_BadCoordCnt++;
    }
    else {
        CopyMemory(m_BadCoord, m_BadCoord + 1, (16 - 1) * sizeof(int));
        m_BadCoord[m_BadCoordCnt - 1] = coord;
    }
}

bool CInfo::IsBadCoord(const CPoint &coord) {
    for (int i = 0; i < m_BadCoordCnt; i++) {
        if (m_BadCoord[i] == coord)
            return true;
    }
    return false;
}

void CInfo::AddIgnore(CMatrixMapStatic *ms) {
    int empty = -1;
    for (int i = 0; i < m_IgnoreCnt; i++) {
        if (m_Ignore[i] == ms) {
            m_IgnoreTime[i] = g_MatrixMap->GetTime();
            return;
        }
        else if (empty < 0 && (m_Ignore[i] == NULL || (g_MatrixMap->GetTime() - m_IgnoreTime[i]) >= 5000)) {
            empty = i;
        }
    }

    if (empty >= 0) {
        m_Ignore[empty] = ms;
        m_IgnoreTime[empty] = g_MatrixMap->GetTime();
    }
    else if (m_IgnoreCnt < 16) {
        m_Ignore[m_IgnoreCnt] = ms;
        m_IgnoreTime[m_IgnoreCnt] = g_MatrixMap->GetTime();
        m_IgnoreCnt++;
    }
    else {
        CopyMemory(m_Ignore, m_Ignore + 1, (16 - 1) * sizeof(CMatrixMapStatic *));
        CopyMemory(m_IgnoreTime, m_IgnoreTime + 1, (16 - 1) * sizeof(int));
        m_Ignore[m_IgnoreCnt - 1] = ms;
    }
}

bool CInfo::IsIgnore(CMatrixMapStatic *ms) {
    for (int i = 0; i < m_IgnoreCnt; i++) {
        if (m_Ignore[i] == ms && (g_MatrixMap->GetTime() - m_IgnoreTime[i]) < 5000) {
            return true;
        }
    }
    return false;
}

void CInfo::DelIgnore(CMatrixMapStatic *ms) {
    for (int i = 0; i < m_IgnoreCnt; i++) {
        if (m_Ignore[i] == ms) {
            m_Ignore[i] = NULL;
        }
    }
}
