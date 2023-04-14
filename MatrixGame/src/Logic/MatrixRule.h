// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixTactics.h"

#include <cstdint>

class CMatrixRobotAI;
class CMatrixLogicSlot;
class CMatrixState;
class CMatrixBuilding;
class CMatrixMapStatic;

#define ENEMY_UNDEF     0
#define ENEMY_ANY       1
#define ENEMY_NEAREST   2
#define ENEMY_DANGEROUS 4
#define ENEMY_STRONGEST 8
#define ENEMY_MAXHEALTH 16
#define ENEMY_MINHEALTH 32
#define ENEMY_NONE      64

#define MOVETO_UNDEF         0
#define MOVETO_ANY           1
#define MOVETO_PROTECTED     2
#define MOVETO_NEAR_ENTRY    4
#define MOVETO_FAR_ENTRY     8
#define MOVETO_BLOCK_ENTRY   16
#define MOVETO_TARGETZONE    32
#define MOVETO_MAINGROUP     64
#define MOVETO_AROUNDENEMY   128
#define MOVETO_FAR_MANEUVER  256
#define MOVETO_NEAR_MANEUVER 512
#define MOVETO_REGION        1024

#define TARGETZONE_R       150
#define TARGETZONE_MINR    80
#define GROUPZONE_R        200
#define AROUNDENEMY_PERIOD 1
#define AROUNDENEMY_R      300
#define AROUNDENEMY_MINR   70

typedef enum {
    CHANGE_UNDEF,
    CHANGE,
    NOT_CHANGE,
} ChangeEnemy;

typedef enum {
    ATTACK_UNDEF,
    ATTACK,
    NOT_ATTACK,
} AttackEnemy;

typedef enum { PURSUE_UNDEF, PURSUE, NOT_PURSUE } PursueEnemy;

typedef enum { FLEE_UNDEF, FLEE, NOT_FLEE } FleeFromEnemy;

typedef enum { EXIT_UNDEF, EXIT, NOT_EXIT } ExitFromTactics;

typedef enum { CAPTURE_UNDEF, CAPTURE, NOT_CAPTURE } CaptureTarget;

typedef enum { POSITION_UNDEF, POSITION_NEAREST_TO_TARGET } Position;

typedef struct _cond {
    uint32_t m_EnemySpoted;
    CMatrixLogicSlot *m_DestroyedSlot;
    Position m_Position;
    _cond() {
        m_EnemySpoted = ENEMY_UNDEF;
        m_DestroyedSlot = nullptr;
        m_Position = POSITION_UNDEF;
    }
} Condition;

typedef struct _act {
    uint32_t m_MoveTo;
    uint32_t m_Patrol;
    ChangeEnemy m_ChangeEnemy;
    AttackEnemy m_AttackEnemy;
    PursueEnemy m_PursueEnemy;
    FleeFromEnemy m_FleeFromEnemy;
    CMatrixState *m_GoTo;
    ExitFromTactics m_Exit;
    CaptureTarget m_Capture;
    _act() {
        m_MoveTo = MOVETO_UNDEF;
        m_Patrol = MOVETO_UNDEF;
        m_ChangeEnemy = CHANGE_UNDEF;
        m_AttackEnemy = ATTACK_UNDEF;
        m_PursueEnemy = PURSUE_UNDEF;
        m_FleeFromEnemy = FLEE_UNDEF;
        m_GoTo = nullptr;
        m_Exit = EXIT_UNDEF;
        m_Capture = CAPTURE_UNDEF;
    }
} Action;

// class CMatrixRule : public CBaseTactics
//{
//    CMatrixMapStatic*       m_ContextEnemy;
// public:
//    int                     m_AroundPeriod;
//    CMatrixRule*            m_PrevRule;
//    CMatrixRule*            m_NextRule;
//
//    Condition               m_Condition;
//    Action                  m_Action;
//
//    bool If(CMatrixRobotAI *robot, CMatrixGroup* group);
//    void Do(CMatrixRobotAI *robot, CMatrixGroup* group);
//
//    //Actions
//
//    void Load(CBlockPar &bp);
//    void LogicTakt();
//
//    CMatrixRule()
//    {
//        m_PrevRule      = nullptr;
//        m_NextRule      = nullptr;
//        m_ContextEnemy  = nullptr;
//
//        m_AroundPeriod  = AROUNDENEMY_PERIOD + 10;
//    }
//    ~CMatrixRule();
//};