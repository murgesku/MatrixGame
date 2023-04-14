// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>

#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include <math.h>

#include "MatrixEffectLightening.hpp"

CMatrixEffectLightening::CMatrixEffectLightening(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float ttl,
                                                 float dispers, float width, DWORD color, bool bp)
  : CMatrixEffect(), m_Pos0(pos0), m_Pos1(pos1), m_TTL(ttl), m_Color(color), m_End0(NULL), m_Dispersion(dispers),
    m_Width(width) {
    DTRACE();

    m_EffectType = EFFECT_LIGHTENING;

    if (bp) {
        m_End0 = (CBillboard *)HAlloc(sizeof(CBillboard), m_Heap);
        if (m_BBTextures[BBT_GLOWBEAMEND].IsIntense()) {
            new(m_End0) CBillboard(TRACE_PARAM_CALL m_Pos0, width / 2, 0, color,
                                           m_BBTextures[BBT_GLOWBEAMEND].tex);
        }
        else {
            new(m_End0) CBillboard(TRACE_PARAM_CALL m_Pos0, width / 2, 0, color,
                                           &m_BBTextures[BBT_GLOWBEAMEND].bbt);
        }
    }

    m_BL_cnt = 0;
    m_BL = NULL;
}

void CMatrixEffectLightening::SetPos(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1) {
    DTRACE();

    m_Pos0 = pos0;
    m_Pos1 = pos1;

    if (m_End0)
        m_End0->m_Pos = pos0;

    m_Dir = (pos1 - pos0);
    float len = D3DXVec3Length(&m_Dir);
    float _len = 1.0f / len;
    m_Dir *= _len;
    if (m_Dir.x == 0 && m_Dir.y == 0) {
        auto tmp = D3DXVECTOR3(0, 1, 0);
        D3DXVec3Cross(&m_Perp, &m_Dir, &tmp);
    }
    else {
        auto tmp = D3DXVECTOR3(0, 0, 1);
        D3DXVec3Cross(&m_Perp, &m_Dir, &tmp);
    }

    int cnt = TruncFloat(len * INVERT(LIGHTENING_SEGMENT_LENGTH)) + 1;

    float m_LastSegLen;
    m_LastSegLen = len - (cnt - 1) * LIGHTENING_SEGMENT_LENGTH;
    if (m_LastSegLen < (LIGHTENING_SEGMENT_LENGTH / 2) && (cnt > 1)) {
        m_LastSegLen += LIGHTENING_SEGMENT_LENGTH;
        --cnt;
    }
    m_SegLen = len / float(cnt);

    if (cnt < m_BL_cnt) {
        while (m_BL_cnt > cnt) {
            m_BL[--m_BL_cnt].Release();
        }
        m_BL = (CBillboardLine *)HAllocEx(m_BL, sizeof(CBillboardLine) * m_BL_cnt, m_Heap);
    }
    else {
        m_BL = (CBillboardLine *)HAllocEx(m_BL, sizeof(CBillboardLine) * cnt, m_Heap);
        while (m_BL_cnt < cnt) {
            new(&m_BL[m_BL_cnt]) CBillboardLine(TRACE_PARAM_CALL pos0, pos1, m_Width, m_Color,
                                                          GetBBTexI(BBT_GLOWBEAM));
            ++m_BL_cnt;
        }
    }
    UpdateData();
}

void CMatrixEffectLightening::UpdateData(void) {
    DTRACE();

    D3DXMATRIX m;

    D3DXVECTOR3 pos0, pos1(m_Pos0), posx(m_Pos0);
    for (int i = 0; i < (m_BL_cnt - 1); ++i) {
        pos0 = pos1;
        posx += m_Dir * m_SegLen;
        pos1 = posx + m_Dir * FSRND(m_SegLen * 0.5f) + (m_Perp * FRND(m_Dispersion));

        BuildRotateMatrix(m, m_Pos0, m_Dir, FSRND(M_PI));
        D3DXVec3TransformCoord(&pos1, &pos1, &m);

        m_BL[i].SetPos(pos0, pos1);
    }
    m_BL[m_BL_cnt - 1].SetPos(pos1, m_Pos1);
}

CMatrixEffectLightening::~CMatrixEffectLightening() {
    DTRACE();
    if (m_End0) {
        m_End0->Release();
        HFree(m_End0, m_Heap);
        m_End0 = NULL;
    }
    for (int i = 0; i < m_BL_cnt; ++i) {
        m_BL[i].Release();
    }
    HFree(m_BL, m_Heap);
}

void CMatrixEffectLightening::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectLightening, this, m_Heap);
}

void CMatrixEffectLightening::BeforeDraw(void) {}

void CMatrixEffectLightening::Draw(void) {
    DTRACE();

    if (m_End0)
        m_End0->Sort(g_MatrixMap->m_Camera.GetViewMatrix());

    /*
    D3DXMATRIX m;
    float sign = (FRND(1) < 0.5f)? 1.0f : -1.0f;
    m._31 = g_MatrixMap->GetViewInversedMatrix()._31 * sign;
    m._32 = g_MatrixMap->GetViewInversedMatrix()._32 * sign;
    m._33 = g_MatrixMap->GetViewInversedMatrix()._33 * sign;
    */

    for (int i = 0; i < m_BL_cnt; ++i) {
        m_BL[i].AddToDrawQueue();

        // CHelper::Create(1,0)->Line(m_BL[i].GetPos0(), m_BL[i].GetPos1());
    }
}

void CMatrixEffectLightening::Takt(float step) {
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

    UpdateData();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

CMatrixEffectShorted::CMatrixEffectShorted(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float ttl, DWORD color)
  : CMatrixEffectLightening(pos0, pos1, ttl, 3, SHORTED_WIDTH, color, false), _m_TTL(1.0f / float(ttl)) {
    m_EffectType = EFFECT_SHORTED;
}

void CMatrixEffectShorted::UpdateData(void) {
    DTRACE();

    D3DXMATRIX m;

    float h = m_Len * 0.3f;
    // float h = m_Len;
    D3DXVECTOR3 pos0, pos1(m_Pos0), posx(m_Pos0), up((m_Pos0 + m_Pos1) * 0.5f + m_Perp * h);

    float dang = float(M_PI * _m_BL_cnt);
    float ang = float(M_PI - dang);

    for (int i = 0; i < (m_BL_cnt - 1); ++i) {
        pos0 = pos1;
        // posx += m_Dir * m_SegLen;
        // pos1 = posx + m_Dir * FSRND(m_SegLen * 0.5f) + (m_Perp *  FRND(1));

        float dx = float(cos(ang)) * m_Len * 0.5f + m_Len * 0.5f;
        float dy = float(sin(ang)) * h;

        pos1 = m_Pos0 + m_Dir * dx + m_Perp * dy;
        ang -= dang;

        pos1.x += FSRND(m_SegLen * 0.5f);
        pos1.y += FSRND(m_SegLen * 0.5f);
        pos1.z += FSRND(m_SegLen * 0.5f);

        BuildRotateMatrix(m, m_Pos0, m_Dir, float(-(m_TTL * _m_TTL) * M_PI));
        D3DXVec3TransformCoord(&pos1, &pos1, &m);

        m_BL[i].SetPos(pos0, pos1);
    }
    m_BL[m_BL_cnt - 1].SetPos(pos1, m_Pos1);
}

void CMatrixEffectShorted::SetPos(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1) {
    DTRACE();

    m_Pos0 = pos0;
    m_Pos1 = pos1;

    if (m_End0)
        m_End0->m_Pos = pos0;

    m_Dir = (pos1 - pos0);
    m_Len = D3DXVec3Length(&m_Dir);
    float _len = 1.0f / m_Len;
    m_Dir *= _len;
    if (m_Dir.x == 0 && m_Dir.y == 0) {
        auto tmp = D3DXVECTOR3(0, 1, 0);
        D3DXVec3Cross(&m_Perp, &m_Dir, &tmp);
    }
    else {
        auto tmp = D3DXVECTOR3(0, 0, 1);
        D3DXVec3Cross(&m_Perp, &m_Dir, &tmp);
    }

    int cnt = TruncFloat(m_Len * INVERT(SHORTED_SEGMENT_LENGTH)) + 1;

    float m_LastSegLen;
    m_LastSegLen = m_Len - (cnt - 1) * SHORTED_SEGMENT_LENGTH;
    if (m_LastSegLen < (SHORTED_SEGMENT_LENGTH / 2) && (cnt > 1)) {
        m_LastSegLen += SHORTED_SEGMENT_LENGTH;
        --cnt;
    }
    _m_BL_cnt = 1.0f / float(cnt);
    m_SegLen = m_Len * _m_BL_cnt;

    if (cnt < m_BL_cnt) {
        while (m_BL_cnt > cnt) {
            m_BL[--m_BL_cnt].Release();
        }
        m_BL = (CBillboardLine *)HAllocEx(m_BL, sizeof(CBillboardLine) * m_BL_cnt, m_Heap);
    }
    else {
        m_BL = (CBillboardLine *)HAllocEx(m_BL, sizeof(CBillboardLine) * cnt, m_Heap);
        while (m_BL_cnt < cnt) {
            new(&m_BL[m_BL_cnt]) CBillboardLine(TRACE_PARAM_CALL pos0, pos1, SHORTED_WIDTH, m_Color,
                                                          GetBBTexI(BBT_GLOWBEAM));
            ++m_BL_cnt;
        }
    }
    UpdateData();
}
