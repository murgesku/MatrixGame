// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>

#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include <math.h>

#include "MatrixEffectZahvat.hpp"
#include "MatrixEffectBillboard.hpp"

CMatrixEffectZahvat::CMatrixEffectZahvat(const D3DXVECTOR3 &pos, float radius, float angle, int cnt)
  : CMatrixEffect(), m_Count(cnt) {
    DTRACE();
    m_EffectType = EFFECT_ZAHVAT;

    m_BBoards = (CMatrixEffectBillboard *)HAlloc(sizeof(CMatrixEffectBillboard) * cnt, m_Heap);

    float da = float(2 * M_PI / double(cnt));
    float s, c;

    for (int i = 0; i < m_Count; ++i) {
        SinCos(angle, &s, &c);
        new(&m_BBoards[i]) CMatrixEffectBillboard(
                pos + D3DXVECTOR3(c * radius, s * radius, 0), ZAHVAT_SPOT_SIZE, ZAHVAT_SPOT_SIZE, ZAHVAT_SPOT_GRAY1,
                ZAHVAT_SPOT_GRAY2, ZAHVAT_FLASH_PERIOD, 0, TEXTURE_PATH_ZAHVAT, D3DXVECTOR3(1, 0, 0));
        m_BBoards[i].m_Intense = false;
        angle += da;
    }

    UpdateData(0, 0);
}
CMatrixEffectZahvat::~CMatrixEffectZahvat() {
    DTRACE();
    for (int i = 0; i < m_Count; ++i)
        m_BBoards[i].~CMatrixEffectBillboard();
    HFree(m_BBoards, m_Heap);
}

void CMatrixEffectZahvat::UpdateData(DWORD color, int count) {
    DTRACE();
    ASSERT(count <= m_Count);
    int count1 = m_Count - count;
    int i = 0;
    while (count-- > 0) {
        m_BBoards[i++].SetColor(color, ZAHVAT_SPOT_GRAY2);
    }
    while (count1-- > 0) {
        m_BBoards[i++].SetColor(ZAHVAT_SPOT_GRAY1, ZAHVAT_SPOT_GRAY2);
    }
}

void CMatrixEffectZahvat::BeforeDraw(void) {
    DTRACE();

    if (m_Count > 0)
        m_BBoards[0].BeforeDraw();
}

void CMatrixEffectZahvat::Draw(void) {
    DTRACE();

    for (int i = 0; i < m_Count; ++i)
        m_BBoards[i].Draw();
}
void CMatrixEffectZahvat::Takt(float) {
    DTRACE();
    // for (int i=0;i<m_Count; ++i) m_BBoards[i].Takt2(step);
}
void CMatrixEffectZahvat::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectZahvat, this, m_Heap);
}
