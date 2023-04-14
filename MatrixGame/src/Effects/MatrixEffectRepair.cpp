// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>

#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include <math.h>

#define REPAIR_CHANGE_TIME 500

#define REPAIR_SEEK_TARGET_PERIOD 100
#define REPAIR_SPAWN_PERIOD       13

CMatrixEffectRepair::CMatrixEffectRepair(const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, float seekradius,
                                         CMatrixMapStatic *skip)
  : CMatrixEffect(), m_BBCnt(0), m_Time(0), m_NextSpawnTime(0), m_SeekRadius(seekradius), m_Target(NULL),
    m_OffTargetAmp(0), m_Skip(skip), m_Pos(pos), m_Dir(dir), m_NextSeekTime(0), m_ChangeTime(0), m_KordOnTarget(m_Heap),
    m_Kord(m_Heap) {
    DTRACE();
    m_EffectType = EFFECT_REPAIR;

    m_Flags = ERF_OFF_TARGET;

    D3DXVECTOR3 p[4];
    p[0] = pos;
    p[1] = pos + dir * (seekradius * INVERT(3.0f));
    p[2] = pos + dir * (seekradius * 2 * INVERT(3.0f));
    p[3] = pos + dir * seekradius;

    m_Kord.Init1(p, 4);

    UpdateData(pos, dir);
}
CMatrixEffectRepair::~CMatrixEffectRepair() {
    DTRACE();
    int i;
    for (i = 0; i < m_BBCnt; ++i) {
        m_BBoards[i].bb.Release();
    }
#ifdef _DEBUG
    for (i = m_BBCnt; i < REPAIR_BB_CNT; ++i)
        m_BBoards[i].bb.release_called = true;
#endif

    if (m_Target)
        m_Target->Release();
}

void CMatrixEffectRepair::BeforeDraw(void) {}

void CMatrixEffectRepair::Draw(void) {
    DTRACE();

    for (int i = 0; i < m_BBCnt; ++i) {
        m_BBoards[i].bb.AddToDrawQueue();
    }

    //#ifdef _DEBUG
    //    CHelper::DestroyByGroup(123);
    //
    //    D3DXVECTOR3 pp;
    //    m_Kord.CalcPoint(pp,0);
    //    for (float t = 0.01f; t <= 1.0f; t += 0.01f)
    //    {
    //        D3DXVECTOR3 ppp;
    //        m_Kord.CalcPoint(ppp,t);
    //        CHelper::Create(100000,123)->Line(pp,ppp);
    //
    //        pp = ppp;
    //    }
    //#endif
}

struct SFindPatientData {
    CMatrixMapStatic *tgt;
    float dist;
    D3DXVECTOR3 *wpos;
    float wdist2;
    int side_of_owner;
};

static bool FindPatient(const D3DXVECTOR3 &fpos, CMatrixMapStatic *ms, DWORD user) {
    if (!ms->NeedRepair())
        return true;
    SFindPatientData *data = (SFindPatientData *)user;
    if (ms->GetSide() != data->side_of_owner)
        return true;

    auto tmp1 = *data->wpos - ms->GetGeoCenter();
    float dist = D3DXVec3LengthSq(&tmp1);
    if (dist > data->wdist2)
        return true;

    auto tmp2 = fpos - ms->GetGeoCenter();
    dist = D3DXVec3LengthSq(&tmp2);
    if (dist < data->dist) {
        if (ms->IsRobot()) {
            data->tgt = ms;
            data->dist = dist;
        }
        else if (data->tgt) {
            if (!data->tgt->IsRobot()) {
                data->tgt = ms;
                data->dist = dist;
            }
        }
        else {
            data->tgt = ms;
            data->dist = dist;
        }
    }
    return true;
}

void CMatrixEffectRepair::Takt(float t) {
    DTRACE();

    m_Time += t;

    if (FLAG(m_Flags, ERF_FOUND_TARGET)) {
        m_ChangeTime += t;
        if (m_ChangeTime > REPAIR_CHANGE_TIME) {
            m_ChangeTime = REPAIR_CHANGE_TIME;

            RESETFLAG(m_Flags, ERF_FOUND_TARGET);
            SETFLAG(m_Flags, ERF_WITH_TARGET);
        }
    }
    if (FLAG(m_Flags, ERF_LOST_TARGET)) {
        m_ChangeTime -= t;
        if (m_ChangeTime < 0) {
            m_ChangeTime = 0;

            RESETFLAG(m_Flags, ERF_LOST_TARGET);
            SETFLAG(m_Flags, ERF_OFF_TARGET);
        }
    }

    // CDText::T("ct", m_ChangeTime);

    if (!FLAG(m_Flags, ERF_MAX_AMP)) {
        m_OffTargetAmp += 0.001f * t;
        float lim = m_SeekRadius * 0.2f;
        if (m_OffTargetAmp > lim) {
            m_OffTargetAmp = lim;
            SETFLAG(m_Flags, ERF_MAX_AMP);
        }
    }

    // seek target

    if (m_Time > m_NextSeekTime) {
        m_NextSeekTime = m_Time + REPAIR_SEEK_TARGET_PERIOD;

        SFindPatientData data;
        data.tgt = NULL;
        data.dist = 1E30f;
        data.wpos = &m_Pos;
        data.wdist2 = m_SeekRadius * m_SeekRadius;
        data.side_of_owner = 0;
        if (m_Skip)
            data.side_of_owner = m_Skip->GetSide();

        // g_MatrixMap->m_DI.T(L"ishem...",L"",1000);
        g_MatrixMap->FindObjects(m_Pos + m_Dir * m_SeekRadius * 0.5f, m_SeekRadius * 0.5f, 1.0f,
                                 TRACE_ROBOT | TRACE_BUILDING | TRACE_CANNON, m_Skip, FindPatient, (DWORD)&data);

        if (data.tgt) {
            // ok

            // g_MatrixMap->m_DI.T(L"i nahodim...",L"",1000);

            SObjectCore *core = data.tgt->GetCore(DEBUG_CALL_INFO);

            if (m_Target == core) {
                core->Release();
            }
            else {
                if (m_Target) {
                    m_Target->Release();
                    m_Target = NULL;
                    RESETFLAG(m_Flags, ERF_OFF_TARGET);
                    RESETFLAG(m_Flags, ERF_WITH_TARGET);
                    SETFLAG(m_Flags, ERF_LOST_TARGET);
                    RESETFLAG(m_Flags, ERF_FOUND_TARGET);

                    core->Release();
                }
                else {
                    m_Target = core;

                    RESETFLAG(m_Flags, ERF_OFF_TARGET);
                    RESETFLAG(m_Flags, ERF_WITH_TARGET);
                    RESETFLAG(m_Flags, ERF_LOST_TARGET);
                    SETFLAG(m_Flags, ERF_FOUND_TARGET);
                }
            }
        }
        else {
            RESETFLAG(m_Flags, ERF_WITH_TARGET);
            RESETFLAG(m_Flags, ERF_FOUND_TARGET);
            if (m_Target) {
                m_Target->Release();
                m_Target = NULL;
                RESETFLAG(m_Flags, ERF_OFF_TARGET);
                SETFLAG(m_Flags, ERF_LOST_TARGET);
            }
            else {
            }
        }
    }

    if (m_Target) {
        if (m_Target->m_Object) {}
        else {
            RESETFLAG(m_Flags, ERF_WITH_TARGET);
            RESETFLAG(m_Flags, ERF_FOUND_TARGET);
            SETFLAG(m_Flags, ERF_LOST_TARGET);
            m_Target->Release();
            m_Target = NULL;
        }
    }

    float sins[10] = {(float)sin(m_Time * 0.005f + 0.9f),  (float)sin(m_Time * 0.007f + 0.2f),
                      (float)sin(m_Time * 0.0073f + 0.3f), (float)sin(m_Time * 0.0013f),
                      (float)sin(m_Time * 0.0022f + 0.2f), (float)sin(m_Time * 0.0017f + 0.8f),
                      (float)sin(m_Time * 0.0028f + 0.3f), (float)sin(m_Time * 0.0051f + 0.9f),
                      (float)sin(m_Time * 0.003f + 0.01f), (float)sin(m_Time * 0.001f + 0.93f)};

    D3DXVECTOR3 p4[4];

    if (FLAG(m_Flags, ERF_OFF_TARGET | ERF_FOUND_TARGET | ERF_LOST_TARGET) && !FLAG(m_Flags, ERF_WITH_TARGET)) {
        // build p4;
        p4[0] = m_Pos;
        p4[1] = m_Pos + m_Dir * (m_SeekRadius * INVERT(3.0f));

        float dx2 = 0.4f * m_OffTargetAmp * sins[3];
        float dy2 = 0.4f * m_OffTargetAmp * sins[7];
        float dz2 = 0.4f * m_OffTargetAmp * sins[9];

        p4[2] = m_Pos + m_Dir * (m_SeekRadius * 2 * INVERT(3.0f)) + D3DXVECTOR3(dx2, dy2, dz2);

        float dx1 = m_OffTargetAmp * sins[1];
        float dy1 = m_OffTargetAmp * sins[4];
        float dz1 = m_OffTargetAmp * sins[8];

        p4[3] = m_Pos + m_Dir * m_SeekRadius + D3DXVECTOR3(dx1, dy1, dz1);
    }

    D3DXVECTOR3 p9[9];
    if (FLAG(m_Flags, ERF_WITH_TARGET | ERF_FOUND_TARGET) && !FLAG(m_Flags, ERF_OFF_TARGET)) {
        // build p9
        ASSERT(m_Target && m_Target->m_Object);

        auto tmp = m_Pos - m_Target->m_GeoCenter;
        float len = D3DXVec3Length(&tmp);

        float disp = len * 0.01f;

        p9[0] = m_Pos;
        p9[1] = m_Pos + m_Dir * (len * INVERT(3.0f));

        D3DXVECTOR3 td(p9[1] - m_Target->m_GeoCenter);

        D3DXVec3Normalize(&td, &td);
        D3DXVECTOR3 side;
        D3DXVec3Cross(&side, &td, (D3DXVECTOR3 *)&m_Target->m_Matrix._31);
        D3DXVec3Normalize(&side, &side);

        float dx1 = disp * sins[0];
        float dy1 = disp * sins[1];
        float dz1 = disp * sins[2];

        float dx2 = disp * sins[2];
        float dy2 = disp * sins[3];
        float dz2 = disp * sins[4];

        float dx3 = disp * sins[4];
        float dy3 = disp * sins[5];
        float dz3 = disp * sins[6];

        float dx4 = disp * sins[6];
        float dy4 = disp * sins[7];
        float dz4 = disp * sins[8];

        float dx5 = disp * sins[3];
        float dy5 = disp * sins[6];
        float dz5 = disp * sins[9];

        p9[2] = D3DXVECTOR3(dx1, dy1, dz1) + m_Target->m_GeoCenter + td * m_Target->m_Radius * 1.4f;
        p9[3] = D3DXVECTOR3(dx2, dy2, dz2) + m_Target->m_GeoCenter + side * m_Target->m_Radius;
        p9[4] = D3DXVECTOR3(dx3, dy3, dz3) + m_Target->m_GeoCenter - td * m_Target->m_Radius;
        p9[5] = D3DXVECTOR3(dx4, dy4, dz4) + m_Target->m_GeoCenter - side * m_Target->m_Radius;
        p9[6] = D3DXVECTOR3(dx5, dy5, dz5) + m_Target->m_GeoCenter + td * m_Target->m_Radius * 1.4f;
        p9[7] = p9[1];
        p9[8] = p9[0];

        m_KordOnTarget.Init2(p9, 9);
    }

    if (FLAG(m_Flags, ERF_FOUND_TARGET | ERF_LOST_TARGET)) {
        float k = m_ChangeTime * INVERT(REPAIR_CHANGE_TIME);

        CTrajectory tr(m_Heap, p4, 4);

        D3DXVECTOR3 p0;
        D3DXVECTOR3 p1;

        D3DXVECTOR3 newkord[9];

        for (int i = 0; i < 9; ++i) {
            float t = i * INVERT(8.0f);

            tr.CalcPoint(p0, t);

            m_KordOnTarget.CalcPoint(p1, t * (k * 0.5f + 0.5f));

            newkord[i] = LERPVECTOR(k, p0, p1);
        }

        m_Kord.Init1(newkord, 9);
    }
    else {
        if (FLAG(m_Flags, ERF_WITH_TARGET)) {
            m_Kord.Init1(p9, 9);
        }
        else {
            m_Kord.Init1(p4, 4);
        }
    }

    float dt = 0.06f;
    if (!m_Target)
        dt = 0.1f;
    int i = 0;
    while (i < m_BBCnt) {
        m_BBoards[i].t += m_BBoards[i].dt;
        D3DXVECTOR3 newpos0, newpos1;
        m_Kord.CalcPoint(newpos0, m_BBoards[i].t - dt);
        m_Kord.CalcPoint(newpos1, m_BBoards[i].t);
        m_BBoards[i].bb.SetPos(newpos0, newpos1);

        if (m_BBoards[i].t > 1) {
            //        m_Activated = true;
            m_BBoards[i].bb.Release();
            m_BBoards[i] = m_BBoards[--m_BBCnt];
            continue;
        }
        else {
            BYTE a = BYTE(KSCALE(m_BBoards[i].t, 0, 0.2f) * 255.0);
            m_BBoards[i].bb.SetAlpha(a);
        }
        ++i;
    }

    while (m_NextSpawnTime < m_Time) {
        m_NextSpawnTime += REPAIR_SPAWN_PERIOD;

        // spawn bill
        if (m_BBCnt < REPAIR_BB_CNT) {
            D3DXVECTOR3 p;
            m_Kord.CalcPoint(p, 0);

            new(&m_BBoards[m_BBCnt].bb) CBillboardLine(TRACE_PARAM_CALL p, p, 4, 0x00FFFFFF,
                                                                 GetBBTexI(BBT_REPGLOWEND));
            m_BBoards[m_BBCnt].dt = FRND(0.03f) + 0.006f;
            m_BBoards[m_BBCnt].t = 0;
            ++m_BBCnt;
        }
    }

    ////D3DXVec3TransformCoordArray(&m_BBoards[0].bb.m_Pos, sizeof(SElevatorFieldBBoard), &m_BBoards[0].bb.m_Pos,
    ///sizeof(SElevatorFieldBBoard), &m_Rot, m_AllBBCnt);
}
void CMatrixEffectRepair::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectRepair, this, m_Heap);
}

CMatrixMapStatic *CMatrixEffectRepair::GetTarget(void) {
    if (m_Target)
        return m_Target->m_Object;
    return NULL;
}
