// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_EFFECT_REPAIR_INCLUDE
#define MATRIX_EFFECT_REPAIR_INCLUDE

#include "MatrixEffect.hpp"

#define REPAIR_BB_CNT 50

struct SRepairBBoard {
    CBillboardLine bb;
    float t;
    float dt;
};

class CMatrixEffectRepair : public CMatrixEffect {
    CTrajectory m_Kord;
    CTrajectory m_KordOnTarget;

    SRepairBBoard m_BBoards[REPAIR_BB_CNT];
    int m_BBCnt;

    CMatrixEffectRepair(const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, float seekradius, CMatrixMapStatic *skip);
    virtual ~CMatrixEffectRepair();

    SObjectCore *m_Target;
    CMatrixMapStatic *m_Skip;

    D3DXVECTOR3 m_Pos;
    D3DXVECTOR3 m_Dir;

    float m_OffTargetAmp;

    float m_ChangeTime;
    float m_SeekRadius;
    float m_NextSeekTime;
    float m_NextSpawnTime;
    float m_Time;

public:
    friend class CMatrixEffect;

    void UpdateData(const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir) {
        DTRACE();
        m_Pos = pos;
        m_Dir = dir;
        Takt(0);
    }

    CMatrixMapStatic *GetTarget(void);

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);

    virtual int Priority(void) { return MAX_EFFECT_PRIORITY; };
};

#endif
