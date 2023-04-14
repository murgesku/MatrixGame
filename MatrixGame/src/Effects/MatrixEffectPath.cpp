// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>

#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include <math.h>

#include "MatrixEffectPath.hpp"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CMatrixEffectPath::CMatrixEffectPath(const D3DXVECTOR3 *pos, int cnt)
  : CMatrixEffect(), m_Kill(false), m_Cnt(cnt), m_Takt(0),
    m_NextTakt(0), m_Dots(NULL), m_DotsCnt(0), m_First(NULL), m_Last(NULL), m_Angle(0), m_Barier(PATH_HIDE_DISTANCE) {
    DTRACE();

    m_EffectType = EFFECT_PATH;

    ASSERT(cnt >= 2);

    for (int i = 0; i < cnt; ++i) {
        m_Points.Add<D3DXVECTOR3>(pos[i]);
    }
    UpdateData();
}

void CMatrixEffectPath::AddPos(const D3DXVECTOR3 &pos) {
    DTRACE();
    m_Points.Add<D3DXVECTOR3>(pos);
    ++m_Cnt;
    UpdateData();
}

CMatrixEffectPath::~CMatrixEffectPath() {
    DTRACE();
    if (m_Dots) {
        while (m_DotsCnt > 0) {
            m_Dots[--m_DotsCnt].dot.Release();
        }

        HFree(m_Dots, m_Heap);
    }
}

void CMatrixEffectPath::UpdateData(void) {
    DTRACE();
    m_Dirs.Clear();
    m_Lens.Clear();

    D3DXVECTOR3 *pts = (D3DXVECTOR3 *)m_Points.Get();

    m_Len = 0;

    for (int i = 0; i < (m_Cnt - 1); ++i) {
        D3DXVECTOR3 dir(*(pts + 1) - (*pts));
        float l = D3DXVec3Length(&dir);
        float _l = (l != 0) ? 1.0f / l : 1.0f;
        m_Dirs.Add<D3DXVECTOR3>(dir * _l);
        m_Lens.Add<float>(l);

        m_Len += l;
        ++pts;
    }

    _m_Len = 1.0f / m_Len;

    int m_DotsMax1 = TruncFloat(m_Len * INVERT(PATH_DOT_DISTANCE)) + 2;
    // if (m_DotsMax1 == 0) m_DotsMax1 = 1;

    while (m_DotsMax1 < m_DotsCnt) {
        m_Dots[--m_DotsCnt].dot.Release();
        LIST_DEL((m_Dots + m_DotsCnt), m_First, m_Last, prev, next);
    }
    m_DotsMax = m_DotsMax1;

    BYTE *old_base = (BYTE *)m_Dots;
    m_Dots = (SPathDot *)HAllocEx(m_Dots, sizeof(SPathDot) * m_DotsMax, m_Heap);

#define REBASE(ptr) ptr = (SPathDot *)(((BYTE *)ptr) + rebase)

    if (old_base != NULL) {
        BYTE *new_base = (BYTE *)m_Dots;
        if (old_base != new_base) {
            // rebasing list
            DWORD rebase = new_base - old_base;
            REBASE(m_First);
            REBASE(m_Last);
            SPathDot *temp = m_First;
            while (temp) {
                if (temp->next)
                    REBASE(temp->next);
                if (temp->prev)
                    REBASE(temp->prev);
                temp = temp->next;
            }
        }
    }
#undef REBASE
}

void CMatrixEffectPath::GetPos(D3DXVECTOR3 *out, float t) {
    DTRACE();
    if (t < 0)
        t += (TruncFloat(t) + 1);
    t = t - float(TruncFloat(t));

    t *= m_Len;

    float *lens = (float *)m_Lens.Get();

    float tt0;
    float tt1 = 0;
    int i;
    for (i = 0; i < (m_Cnt - 1); ++i) {
        tt0 = tt1;
        tt1 += *lens;

        if (t <= tt1) {
            D3DXVECTOR3 *p = (((D3DXVECTOR3 *)m_Points.Get()) + i);
            D3DXVECTOR3 *d = (((D3DXVECTOR3 *)m_Dirs.Get()) + i);

            *out = *p + *d * (t - tt0);
            return;
        }
        ++lens;
    }
}

void CMatrixEffectPath::BeforeDraw(void) {}

void CMatrixEffectPath::Draw(void) {
    DTRACE();

    /*
    int i;

    for (i = 0; i<(m_Cnt-1); ++i)
    {
        D3DXVECTOR3 * p0 = (((D3DXVECTOR3 *)m_Points.Get()) + i);
        D3DXVECTOR3 * p1 = (((D3DXVECTOR3 *)m_Points.Get()) + i + 1);

        CHelper::Create(1,0)->Line(*p0,*p1);
    }
    */

    //   D3DXVECTOR3 timepos;
    //    GetPos(&timepos, m_Time);

    // CDText::T("Time", m_Time);

    // CHelper::Create(1,0)->Line(timepos-D3DXVECTOR3(0,0,10),timepos+D3DXVECTOR3(0,0,10));

    // int cnt = 0;

    SPathDot *temp = m_First;
    while (temp) {
        //++cnt;
        temp->dot.Sort(g_MatrixMap->m_Camera.GetViewMatrix());
        temp = temp->next;
    }

    // CDText::T("P", CStr(cnt) + "/" + CStr(m_DotsCnt));
}
void CMatrixEffectPath::Takt(float step) {
    DTRACE();

    float dtime = (0.05f * step);
    if (dtime > 0.9f)
        dtime = 0.9f;

    m_Angle += 0.05f * dtime;
    if (m_Angle > (2 * M_PI))
        m_Angle -= float(2 * M_PI);

    m_Takt += step;
    if (m_Takt > m_NextTakt || m_DotsCnt < m_DotsMax) {
        m_NextTakt += PATH_DOT_RATE;

        while (m_DotsCnt < m_DotsMax) {
            LIST_INSERT(m_First, (m_Dots + m_DotsCnt), m_First, m_Last, prev, next);
            m_Dots[m_DotsCnt].pos = 0;
            if (m_BBTextures[BBT_PATHDOT].IsIntense()) {
                new(&m_Dots[m_DotsCnt].dot) CBillboard(TRACE_PARAM_CALL * (D3DXVECTOR3 *)m_Points.Get(),
                                                             PATH_SIZE, 0, PATH_COLOR, m_BBTextures[BBT_PATHDOT].tex);
            }
            else {
                new(&m_Dots[m_DotsCnt].dot) CBillboard(TRACE_PARAM_CALL * (D3DXVECTOR3 *)m_Points.Get(),
                                                             PATH_SIZE, 0, PATH_COLOR, &m_BBTextures[BBT_PATHDOT].bbt);
            }
            ++m_DotsCnt;
        }
    }

    if (m_Kill) {
        m_Barier += step * 0.002f * m_Len;
        if (m_Barier > (2 * m_Len)) {
#ifdef _DEBUG
            g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
            g_MatrixMap->SubEffect(this);
#endif
            return;
        }
    }

    float prevpos = -PATH_DOT_DISTANCE;
    SPathDot *temp = m_First;
    while (temp) {
        float tgtpos = prevpos + PATH_DOT_DISTANCE;
        float d = (tgtpos - temp->pos);
        // if (d != 0)
        { temp->pos += d * dtime; }
        // temp->pos += (PATH_DOT_SPEED * step);

        if (temp->pos > m_Len) {
            temp->pos = 0;
            temp->dot.SetPos(*(D3DXVECTOR3 *)m_Points.Get());
            temp->dot.SetAlpha(0);
            SPathDot *temp1 = temp->next;
            LIST_DEL(temp, m_First, m_Last, prev, next);
            LIST_INSERT(m_First, temp, m_First, m_Last, prev, next);
            temp = temp1;
            continue;
        }
        prevpos = temp->pos;

        float normpos = temp->pos * _m_Len;

        D3DXVECTOR3 pos;
        GetPos(&pos, normpos);
        temp->dot.SetPos(pos);

        temp->dot.SetAngle(m_Angle + temp->pos * 0.1f, 0, 0 /*sin(m_Angle + (temp->pos * 0.5f)) */);

        float alpha = 1;
        if (temp->pos < m_Barier - PATH_HIDE_DISTANCE) {
            alpha = 0;
        }
        else if (temp->pos < m_Barier) {
            alpha *= 1.0f - (m_Barier - temp->pos) * INVERT(PATH_HIDE_DISTANCE);
        }
        if (temp->pos > (m_Len - PATH_HIDE_DISTANCE)) {
            float alpha1 = (m_Len - temp->pos) * INVERT(PATH_HIDE_DISTANCE);
            if (alpha1 < 0)
                alpha1 = 0;
            alpha *= alpha1;
        }

        temp->dot.SetAlpha(BYTE(alpha * float(PATH_COLOR >> 24)));

        temp = temp->next;
    }

    // m_Time += dtime * PATH_SPEED * _m_Len;
    // if(m_Time > 1) m_Time -= 1;

    /*
    if (m_Kill)
    {
        m_Radius -= dtime;
        if (m_Radius < 0 )
        {
            g_MatrixMap->SubEffect(this);
            return;
        }

        for (int i = 0; i<SEL_CNT; ++i)
            for (int j = 0; j<SEL_BLUR_CNT; ++j)
                m_Points[i].m_Blur[j].SetAlpha( BYTE((m_Radius/m_InitRadius) * float(SEL_COLOR_1 >> 24)));

    }
    */
}
void CMatrixEffectPath::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectPath, this, m_Heap);
}
