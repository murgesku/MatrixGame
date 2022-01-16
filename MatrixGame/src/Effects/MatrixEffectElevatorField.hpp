// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#define ELEVATORFIELD_CNT          7
#define ELEVATORFIELD_BB_CNT       300
#define ELEVATORFIELD_SPAWN_PERIOD 10
#define ELEVATORFIELD_BB_SIZE      3

struct SElevatorFieldBBoard {
    // CBillboard  bb;
    CBillboardLine bb;
    int kord;
    float t;
    float dt;
};

class CMatrixEffectElevatorField : public CMatrixEffect {
    CTrajectory m_Kords[ELEVATORFIELD_CNT];

    int m_BBCnt[ELEVATORFIELD_CNT];
    SElevatorFieldBBoard m_BBoards[ELEVATORFIELD_BB_CNT];
    int m_AllBBCnt;

    D3DXMATRIX m_Rot;
    float m_Angle;
    D3DXVECTOR3 m_Pos;
    D3DXVECTOR3 m_Dir;

    CMatrixEffectElevatorField(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float radius, const D3DXVECTOR3 &fwd);
    virtual ~CMatrixEffectElevatorField();

    float m_NextTime;
    float m_Time;

    DWORD m_Sound;

public:
    DWORD m_Activated;

    friend class CMatrixEffect;

    void UpdateData(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float r, const D3DXVECTOR3 &fwd);

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);

    virtual int Priority(void) { return MAX_EFFECT_PRIORITY; };
};
