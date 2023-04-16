// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include <math.h>

#include "MatrixEffectFirePlasma.hpp"
#include "MatrixEffectPointLight.hpp"

CMatrixEffectFirePlasma::CMatrixEffectFirePlasma(const D3DXVECTOR3 &start, const D3DXVECTOR3 &end, float speed,
                                                 DWORD hitmask, CMatrixMapStatic *skip, FIRE_END_HANDLER handler,
                                                 DWORD user)
  : CMatrixEffect(), m_Pos(start), m_End(end), m_Speed(speed), m_UserData(user), m_Handler(handler), m_HitMask(hitmask),
    m_Skip(skip), m_BBLine(TRACE_PARAM_CALL start, start, FIRE_PLASMA_W * 1.4f, 0xFFFFFFFF, GetBBTexI(BBT_PLASMA1)),
    m_BB1(TRACE_PARAM_CALL start, FIRE_PLASMA_W, 0, 0xFFFFFFFF, GetBBTexI(BBT_PLASMA2)),
    m_BB2(TRACE_PARAM_CALL start, FIRE_PLASMA_W, 0, 0xFFFFFFFF, GetBBTexI(BBT_PLASMA2)),
    m_BB3(TRACE_PARAM_CALL start, FIRE_PLASMA_W, 0, 0xFFFFFFFF, GetBBTexI(BBT_PLASMA2)),
    m_BB4(TRACE_PARAM_CALL start, FIRE_PLASMA_W, 0, 0xFFFFFFFF, GetBBTexI(BBT_PLASMA2))
#ifdef _DEBUG
    , m_Light(DEBUG_CALL_INFO)
#endif
{

    DTRACE();

    m_EffectType = EFFECT_FIRE_PLASMA;

    CMatrixEffect::CreatePointLight(&m_Light, start, 20, 0x80202030, true);
    // CMatrixEffect::CreatePointLight(&m_Light, start,20, 0xFFFFFFFF, true);

    const auto tmp = m_End - m_Pos;
    D3DXVec3Normalize(&m_Dir, &tmp);
    m_Prevdist = D3DXVec3Length(&tmp);
}

CMatrixEffectFirePlasma::~CMatrixEffectFirePlasma() {
    DTRACE();

    m_BBLine.Release();
    m_BB1.Release();
    m_BB2.Release();
    m_BB3.Release();
    m_BB4.Release();

#ifdef _DEBUG
    m_Light.Release(DEBUG_CALL_INFO);
#else
    m_Light.Release();
#endif

    if (m_Handler)
        m_Handler(TRACE_STOP_NONE, m_Pos, m_UserData, FEHF_LASTHIT);
}

void CMatrixEffectFirePlasma::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectFirePlasma, this, m_Heap);
}

void CMatrixEffectFirePlasma::Draw(void) {
    DTRACE();
    m_BBLine.AddToDrawQueue();
    m_BB1.Sort(g_MatrixMap->m_Camera.GetViewInversedMatrix());
    m_BB2.Sort(g_MatrixMap->m_Camera.GetViewInversedMatrix());
    m_BB3.Sort(g_MatrixMap->m_Camera.GetViewInversedMatrix());
    m_BB4.Sort(g_MatrixMap->m_Camera.GetViewInversedMatrix());
}

void CMatrixEffectFirePlasma::Takt(float step) {
    DTRACE();

    bool hit = false;
    D3DXVECTOR3 hitpos;

    float dtime = (m_Speed * step);

    D3DXVECTOR3 newpos(m_Pos + m_Dir * dtime);

    CMatrixMapStatic *hito = g_MatrixMap->Trace(&hitpos, m_Pos, newpos, m_HitMask, m_Skip);
    if (hito != TRACE_STOP_NONE) {
        hit = true;
    }

    m_Pos = newpos;
    if (m_Light.effect)
        ((CMatrixEffectPointLight *)m_Light.effect)->SetPos(m_Pos);

    const auto tmp = m_End - m_Pos;
    float newdist = D3DXVec3Length(&tmp);
    if (newdist > m_Prevdist) {
        hit = true;
        hitpos = m_End;
    }

    m_Prevdist = newdist;

    if (hit) {
        // finish
        if (m_Handler)
            m_Handler(hito, hitpos, m_UserData, FEHF_LASTHIT);
        m_Handler = NULL;

        if (hito == TRACE_STOP_LANDSCAPE) {
            CMatrixEffect::CreateLandscapeSpot(NULL, D3DXVECTOR2(hitpos.x, hitpos.y), FSRND(M_PI), FRND(1) + 0.5f,
                                               SPOT_PLASMA_HIT);
            CMatrixEffect::CreateSmoke(NULL, hitpos, 100, 1400, 41, 0xFF909090);
        }
        else {
            if (hito == TRACE_STOP_WATER) {
                // CMatrixEffect::CreateFire(hitpos, 10000, 2000, 20, 1);
                CMatrixEffect::CreateSmoke(NULL, hitpos, 100, 1400, 41, 0xFFFFFFFF);
            }
            else {
                CMatrixEffect::CreateSmoke(NULL, hitpos, 100, 1400, 41, 0xFF3F2F2F);
            }
        }
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        g_MatrixMap->SubEffect(this);
#endif
        return;
    }

    m_BBLine.SetPos(m_Pos - m_Dir * FIRE_PLASMA_L * 0.7f, m_Pos + m_Dir * FIRE_PLASMA_L * 0.7f);
    m_BB1.m_Pos = (m_Pos - m_Dir * FIRE_PLASMA_L * 0.5f);
    m_BB2.m_Pos = (m_Pos - m_Dir * FIRE_PLASMA_L * 0.166f);
    m_BB3.m_Pos = (m_Pos + m_Dir * FIRE_PLASMA_L * 0.166f);
    m_BB4.m_Pos = (m_Pos + m_Dir * FIRE_PLASMA_L * 0.5f);
}
