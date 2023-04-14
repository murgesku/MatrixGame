// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixEffectShleif.hpp"
#include "MatrixEffectSmokeAndFire.hpp"

#include "../MatrixMap.hpp"

CMatrixEffectShleif::CMatrixEffectShleif(void) {
    m_EffectType = EFFECT_SMOKESHLEIF;
    m_SmokesCnt = 0;
    m_TTL = SHLEIF_TTL;

    m_Smokes = (SEffectHandler *)HAlloc(sizeof(SEffectHandler) * SHLEIF_MAX_SMOKES, m_Heap);
#ifdef _DEBUG
    for (int i = 0; i < SHLEIF_MAX_SMOKES; ++i) {
        new (&m_Smokes[i]) SEffectHandler(DEBUG_CALL_INFO);
    }
#else
    memset(m_Smokes, 0, sizeof(SEffectHandler) * SHLEIF_MAX_SMOKES);
#endif
}

CMatrixEffectShleif::~CMatrixEffectShleif() {
    for (int i = 0; i < m_SmokesCnt; ++i) {
        if (m_Smokes[i].effect) {
#ifdef _DEBUG
            m_Smokes[i].Release(DEBUG_CALL_INFO);
#else
            m_Smokes[i].Release();
#endif
            // m_Smokes[i].Unconnect();
        }
    }
    HFree(m_Smokes, m_Heap);
}

void CMatrixEffectShleif::Takt(float step) {
    DTRACE();
    m_TTL -= step;
    if (m_TTL < 0) {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        g_MatrixMap->SubEffect(this);
#endif
        return;
    }
    for (int i = 0; i < m_SmokesCnt; ++i) {
        if (m_Smokes[i].effect) {
            m_Smokes[i].effect->Takt(step);
        }
    }
}

void CMatrixEffectShleif::BeforeDraw(void) {
    DTRACE();

    for (int i = 0; i < m_SmokesCnt; ++i) {
        if (m_Smokes[i].effect) {
            m_Smokes[i].effect->BeforeDraw();
        }
        else {
            while (i < (m_SmokesCnt - 1)) {
                --m_SmokesCnt;
                memcpy(m_Smokes + i, m_Smokes + m_SmokesCnt, sizeof(SEffectHandler));
                m_Smokes[m_SmokesCnt].effect = NULL;
                m_Smokes[i].Rebase();

                if (m_Smokes[i].effect)
                    break;
            }
        }
    }
}

void CMatrixEffectShleif::Draw(void) {
    DTRACE();

    for (int i = 0; i < m_SmokesCnt; ++i) {
        if (m_Smokes[i].effect)
            m_Smokes[i].effect->Draw();
    }
}

void CMatrixEffectShleif::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectShleif, this, m_Heap);
}

void CMatrixEffectShleif::AddSmoke(const D3DXVECTOR3 &pos, float ttl, float puffttl, float spawntime, DWORD color,
                                   bool intense, float speed) {
    if (m_SmokesCnt < SHLEIF_MAX_SMOKES) {
        CMatrixEffectSmoke *e = HNew(m_Heap) CMatrixEffectSmoke(pos, ttl, puffttl, spawntime, color, intense, speed);

        m_Smokes[m_SmokesCnt].effect = e;
        e->SetHandler(m_Smokes + m_SmokesCnt);

        m_TTL = SHLEIF_TTL;
        ++m_SmokesCnt;
    }
}

void CMatrixEffectShleif::AddFire(const D3DXVECTOR3 &pos, float ttl, float puffttl, float spawntime, float dispfactor,
                                  bool intense, float speed) {
    if (m_SmokesCnt < SHLEIF_MAX_SMOKES) {
        CMatrixEffectFire *e = HNew(m_Heap) CMatrixEffectFire(pos, ttl, puffttl, spawntime, dispfactor, intense, speed);

        m_Smokes[m_SmokesCnt].effect = e;
        e->SetHandler(m_Smokes + m_SmokesCnt);

        m_TTL = SHLEIF_TTL;
        ++m_SmokesCnt;
    }
}

// void CMatrixEffectShleif::AddFlame(const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &speed, float
// ttl, DWORD hitmask, CMatrixMapStatic * skip, DWORD user, FIRE_END_HANDLER handler)
//{
//    if (m_SmokesCnt < SHLEIF_MAX_SMOKES)
//    {
//        CMatrixEffectFire *e = HNew(m_Heap) CMatrixEffectFire(pos,ttl,puffttl,spawntime,dispfactor, intense, speed);
//
//        m_Smokes[m_SmokesCnt].effect = e;
//        e->SetHandler(m_Smokes+m_SmokesCnt);
//
//        m_TTL = SHLEIF_TTL;
//        ++m_SmokesCnt;
//    }
//
//}
//
