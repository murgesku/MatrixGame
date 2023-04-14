// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>

#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include <math.h>

#include "MatrixEffectElevatorField.hpp"

CMatrixEffectElevatorField::CMatrixEffectElevatorField(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float radius,
                                                       const D3DXVECTOR3 &fwd)
  : CMatrixEffect(), m_AllBBCnt(0), m_Time(0), m_NextTime(0), m_Activated(false), m_Angle(0.0f) {
    DTRACE();
    m_EffectType = EFFECT_ELEVATORFIELD;

    memset(&m_BBCnt, 0, sizeof(m_BBCnt));

    for (int i = 0; i < ELEVATORFIELD_CNT; ++i) {
        new(&m_Kords[i]) CTrajectory(m_Heap);
    }

    UpdateData(pos0, pos1, radius, fwd);

    m_Sound = CSound::Play(S_EF_START, m_Pos, SL_ELEVATORFIELD);
}
CMatrixEffectElevatorField::~CMatrixEffectElevatorField() {
    DTRACE();
    int i;
    for (i = 0; i < m_AllBBCnt; ++i) {
        m_BBoards[i].bb.Release();
    }
#ifdef _DEBUG
    for (i = m_AllBBCnt; i < ELEVATORFIELD_BB_CNT; ++i)
        m_BBoards[i].bb.release_called = true;
#endif

    CSound::AddSound(S_EF_END, m_Pos, SL_ELEVATORFIELD);
}

void CMatrixEffectElevatorField::UpdateData(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float r,
                                            const D3DXVECTOR3 &fwd) {
    DTRACE();

    m_Pos = pos1;
    auto tmp = pos0 - pos1;
    D3DXVec3Normalize(&m_Dir, &tmp);
    D3DXVECTOR3 perp;
    D3DXVec3Normalize(&perp, D3DXVec3Cross(&perp, &fwd, &m_Dir));

    D3DXMATRIX m;

    D3DXVECTOR3 p[6];
    D3DXVECTOR3 pn[6];

    static float koefs[5][2] = {
            {0.9f, 0.0f}, {0.0f, -0.67f}, {-0.2f, 0.2f}, {0.32f, 0.65f}, {0.81f, 0.3f},
    };

    /*
    {
        static i1 = 0;
        if (g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_PGDN) {g_MatrixMap->m_KeyDown = false; i1--; if (i1
    < 0) i1 = 4;} if (g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_PGUP) {g_MatrixMap->m_KeyDown = false;
    i1++; if (i1 > 4) i1 = 0;}

        if (g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_HOME) {koefs[i1][0] += 0.01f;}
        if (g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_END)  {koefs[i1][0] -= 0.01f;}

        if (g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_INSERT) {koefs[i1][1] += 0.01f;}
        if (g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_DELETE)  {koefs[i1][1] -= 0.01f;}

        CDText::T("info", CStr(i1) + "," + CStr(koefs[i1][0])+"," + CStr(koefs[i1][1]));
    }
    */

    for (int i = 0; i < 5; ++i) {
        p[i] = pos1 + m_Dir * r * koefs[i][0] + perp * r * koefs[i][1];
    }

    p[5] = pos0;

    float a = m_Angle;
    float da = M_PI_MUL(2.0 / ELEVATORFIELD_CNT);
    for (int i = 0; i < ELEVATORFIELD_CNT; ++i) {
        BuildRotateMatrix(m, pos1, m_Dir, a);
        a += da;

        D3DXVec3TransformCoordArray(pn, sizeof(D3DXVECTOR3), p, sizeof(D3DXVECTOR3), &m, 6);

        m_Kords[i].Init2(pn, 6);
    }
}

void CMatrixEffectElevatorField::BeforeDraw(void) {}

void CMatrixEffectElevatorField::Draw(void) {
    DTRACE();

    for (int i = 0; i < m_AllBBCnt; ++i) {
        // m_BBoards[i].bb.Sort(g_MatrixMap->GetViewMatrix());
        m_BBoards[i].bb.AddToDrawQueue();
    }

    /*
    for (int i = 0; i < ELEVATORFIELD_CNT; ++i)
    {
        D3DXVECTOR3 p0,p1;
        m_Kords[i].CalcPoint(p1, 0);
        float t = 0.01f;
        while (t < 1)
        {
            p0 = p1;
            m_Kords[i].CalcPoint(p1, t);

            CHelper::Create(1,0)->Line(p0,p1);

            t += 0.01f;
        }



    }
    */
}
void CMatrixEffectElevatorField::Takt(float t) {
    DTRACE();

    m_Angle += t * 0.001f;
    if (m_Angle > M_PI_MUL(2)) {
        m_Angle -= M_PI_MUL(2);
    }
    // BuildRotateMatrix(m_Rot, m_Pos, m_Dir, m_Angle);

    if (m_Activated) {
        m_Sound = CSound::Play(m_Sound, S_EF_CONTINUE, m_Pos, SL_ELEVATORFIELD);
    }
    else {
        m_Sound = CSound::Play(m_Sound, S_EF_START, m_Pos, SL_ELEVATORFIELD);
    }

    int i = 0;
    while (i < m_AllBBCnt) {
        m_BBoards[i].t += m_BBoards[i].dt;
        D3DXVECTOR3 newpos0, newpos1;
        // m_Kords[m_BBoards[i].kord].CalcPoint(m_BBoards[i].bb.m_Pos, m_BBoards[i].t);
        m_Kords[m_BBoards[i].kord].CalcPoint(newpos0, m_BBoards[i].t - 0.1f);
        m_Kords[m_BBoards[i].kord].CalcPoint(newpos1, m_BBoards[i].t);
        m_BBoards[i].bb.SetPos(newpos0, newpos1);

        if (m_BBoards[i].t > 1) {
            m_Activated = true;
            m_BBoards[i].bb.Release();
            --m_AllBBCnt;
            --m_BBCnt[m_BBoards[i].kord];
            m_BBoards[i] = m_BBoards[m_AllBBCnt];
            continue;
        }
        else {
            BYTE a = BYTE(KSCALE(m_BBoards[i].t, 0, 0.2f) * 255.0);
            m_BBoards[i].bb.SetAlpha(a);
        }
        ++i;
    }

    m_Time += t;
    while (m_NextTime < m_Time) {
        m_NextTime += ELEVATORFIELD_SPAWN_PERIOD;

        // spawn bill
        if (m_AllBBCnt < ELEVATORFIELD_BB_CNT) {
            int m = m_BBCnt[0];
            int idx = 0;
            for (int i = 1; i < ELEVATORFIELD_CNT; ++i) {
                if (m > m_BBCnt[i]) {
                    idx = i;
                    m = m_BBCnt[i];
                }
            }
            D3DXVECTOR3 p;
            m_Kords[idx].CalcPoint(p, 0);

            new(&m_BBoards[m_AllBBCnt].bb) CBillboardLine(TRACE_PARAM_CALL p, p, ELEVATORFIELD_BB_SIZE,
                                                                    0x00FFFFFF, GetBBTexI(BBT_EFIELD));
            m_BBoards[m_AllBBCnt].dt = FRND(0.01f) + 0.001f;
            m_BBoards[m_AllBBCnt].t = 0;
            m_BBoards[m_AllBBCnt].kord = idx;
            ++m_AllBBCnt;
            ++m_BBCnt[idx];
        }
    }

    // D3DXVec3TransformCoordArray(&m_BBoards[0].bb.m_Pos, sizeof(SElevatorFieldBBoard), &m_BBoards[0].bb.m_Pos,
    // sizeof(SElevatorFieldBBoard), &m_Rot, m_AllBBCnt);
}
void CMatrixEffectElevatorField::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectElevatorField, this, m_Heap);
}
