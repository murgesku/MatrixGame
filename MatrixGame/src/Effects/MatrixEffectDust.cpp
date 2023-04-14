// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>

#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include <math.h>

#include "MatrixEffectDust.hpp"

#define DUST_R1      5
#define DUST_R1_VAR  3
#define DUST_R2      25
#define DUST_R2_VAR  7
#define DUST_C1      0x4F888888
#define DUST_C2      0x00777777
#define DUST_SPEED_K 1.5f

CMatrixEffectDust::CMatrixEffectDust(const D3DXVECTOR2 &pos, const D3DXVECTOR2 &adddir, float ttl)
  : CMatrixEffect(), m_TTL(ttl), m_AddSpeed(adddir) {
    DTRACE();
    m_EffectType = EFFECT_DUST;

    for (int i = 0; i < DUST_BILLS_CNT; ++i) {
        D3DXVECTOR2 dir(FSRND(1), FSRND(1));
        D3DXVec2Normalize(m_Dirs + i, &dir);
        new(&m_BBoards[i])
            CMatrixEffectBillboard(
                D3DXVECTOR3(pos.x, pos.y, 0),
                DUST_R1 + FSRND(DUST_R1_VAR),
                DUST_R2 + FSRND(DUST_R2_VAR),
                DUST_C1,
                DUST_C2,
                ttl,
                0.0f,
                TEXTURE_PATH_DUST,
                D3DXVECTOR3(m_Dirs[i].x, m_Dirs[i].y, 0),
                NULL);

        m_BBoards[i].m_Intense = false;
        m_Dirs[i] *= DUST_SPEED_K;
    }
}
CMatrixEffectDust::~CMatrixEffectDust() {
    DTRACE();
}

void CMatrixEffectDust::BeforeDraw(void) {
    DTRACE();

    if (FLAG(m_before_draw_done, BDDF_DUST))
        return;

    m_BBoards[0].BeforeDraw();

    SETFLAG(m_before_draw_done, BDDF_DUST);

    // for (int i=0; i<DUST_BILLS_CNT; ++i)
    //{
    //    m_BBoards[i].BeforeDraw();
    //}
}

void CMatrixEffectDust::Draw(void) {
    DTRACE();

    for (int i = 0; i < DUST_BILLS_CNT; ++i) {
        m_BBoards[i].Draw();
    }
}
void CMatrixEffectDust::Takt(float time) {
    DTRACE();
    float mul = (float)(pow(0.996, double(time)));
    int bz = 0;
    for (int i = 0; i < DUST_BILLS_CNT; ++i) {
        *(D3DXVECTOR2 *)&m_BBoards[i].m_Mat._41 += m_Dirs[i] + m_AddSpeed;
        m_BBoards[i].m_Mat._43 = g_MatrixMap->GetZ(m_BBoards[i].m_Mat._41, m_BBoards[i].m_Mat._42) + 0.1f;

        m_Dirs[i] *= mul;
        m_BBoards[i].m_TTL -= time;
        if (m_BBoards[i].m_TTL < 0)
            m_BBoards[i].m_TTL = 0.0f;
        if (m_BBoards[i].m_TTL == 0.0f) {
            ++bz;
        }
        else {
            m_BBoards[i].UpdateData();
        }
    }
    if (bz == DUST_BILLS_CNT) {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        g_MatrixMap->SubEffect(this);
#endif
    }
}
void CMatrixEffectDust::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectDust, this, m_Heap);
}
