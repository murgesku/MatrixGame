// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>

#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include <math.h>

#include "MatrixEffectSmokeAndFire.hpp"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CMatrixEffectSmoke::CMatrixEffectSmoke(const D3DXVECTOR3 &pos, float ttl, float puffttl, float spawntime, DWORD color,
                                       bool intense, float speed)
  : CMatrixEffect(), m_Color(color), m_Speed(speed) {
    DTRACE();

    m_EffectType = intense ? EFFECT_SMOKE_INTENSE : EFFECT_SMOKE;
    ELIST_ADD(EFFECT_SMOKE);

    m_PuffCnt = 0;

    m_TTL = ttl;
    m_PuffTTL = puffttl;
    m_Spawn = spawntime;

    m_Time = 0;
    m_NextSpawnTime = 0;

    m_Pos = pos;
    m_Mins = pos;
    m_Maxs = pos;

    m_Puffs = (SSmokePuff *)HAlloc(sizeof(SSmokePuff) * MAX_PUFF_COUNT, m_Heap);
}

CMatrixEffectSmoke::~CMatrixEffectSmoke() {
    DTRACE();
    while (m_PuffCnt > 0) {
        m_Puffs[--m_PuffCnt].m_Puff.Release();
    }

    HFree(m_Puffs, m_Heap);

    ELIST_DEL(EFFECT_SMOKE);
}

void CMatrixEffectSmoke::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectSmoke, this, m_Heap);
}

void CMatrixEffectSmoke::BeforeDraw(void) {}

void CMatrixEffectSmoke::Draw(void) {
    DTRACE();

    if (!(g_MatrixMap->m_Camera.IsInFrustum(m_Mins, m_Maxs)))
        return;

    // CHelper::Create(1,0)->BoundBox(m_Mins, m_Maxs);

    int i;

    i = 0;
    while (i < m_PuffCnt) {
        m_Puffs[i].m_Puff.Sort(g_MatrixMap->m_Camera.GetViewMatrix());
        ++i;
    }
}

void CMatrixEffectSmoke::Takt(float step) {
    DTRACE();

    float dtime = (m_Speed * step);

    m_Time += step;
    m_TTL -= step;
    while ((m_Time > m_NextSpawnTime) && m_TTL > 0) {
        SpawnPuff();
        m_NextSpawnTime += m_Spawn;
    }

    if ((m_PuffCnt == 0) && (m_TTL < 0)) {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        g_MatrixMap->SubEffect(this);
#endif
        return;
    }

    int i;

    i = 0;
    while (i < m_PuffCnt) {
        m_Puffs[i].m_PuffTTL -= step;
        if (m_Puffs[i].m_PuffTTL < 0) {
            m_Puffs[i].m_Puff.Release();
            m_Puffs[i] = m_Puffs[--m_PuffCnt];
            continue;
        }

        ++i;
    }

    i = 0;
    while (i < m_PuffCnt) {
        float life = m_Puffs[i].m_PuffTTL / m_PuffTTL;

        m_Puffs[i].m_Puff.m_Pos.z += dtime * m_Speed * (5 + (m_Puffs[i].m_Puff.m_Pos.z - m_Puffs[i].m_PuffOrig.z));
        // if (m_Puff[i].m_Pos.z > m_MaxZ) m_MaxZ = m_Puff[i].m_Pos.z;
        m_Puffs[i].m_Puff.SetScale(LERPFLOAT(life, PUFF_SCALE2, PUFF_SCALE1));
        m_Puffs[i].m_Puff.SetAlpha((BYTE)Float2Int((m_Color >> 24) * life));
        float a = M_PI_MUL(life * m_Speed * 25);
        SET_SIGN_FLOAT(a, (i & 1));
        m_Puffs[i].m_Puff.SetAngle(m_Puffs[i].m_PuffAngle + a, 0, 0);

        ++i;
    }

    // update bbox
    if (m_PuffCnt > 0) {
        m_Mins = m_Puffs[0].m_Puff.m_Pos;
        m_Maxs = m_Mins;
        i = 0;
        while (i < m_PuffCnt) {
            AddSphereToBox(m_Mins, m_Maxs, m_Puffs[i].m_Puff.m_Pos, m_Puffs[i].m_Puff.GetScale());
            ++i;
        }
    }
    else {
        m_Mins = m_Pos;
        m_Maxs = m_Pos;
    }
}

void CMatrixEffectSmoke::SpawnPuff(void) {
    DTRACE();
    if (m_PuffCnt >= MAX_PUFF_COUNT)
        return;
    m_Puffs[m_PuffCnt].m_PuffAngle = FRND(M_PI);

    EBillboardTextureSort tex = (m_EffectType == EFFECT_FIRE)
                                        ? BBT_FIRE
                                        : ((m_EffectType == EFFECT_SMOKE)          ? BBT_SMOKE
                                           : (m_EffectType == EFFECT_FIRE_INTENSE) ? BBT_FIRE_INTENSE
                                                                                   : BBT_SMOKE_INTENSE);

    if (m_BBTextures[tex].IsIntense()) {
        new(&m_Puffs[m_PuffCnt].m_Puff) CBillboard(
                TRACE_PARAM_CALL m_Pos, 5 - FRND(2), m_Puffs[m_PuffCnt].m_PuffAngle, m_Color, m_BBTextures[tex].tex);
    }
    else {
        new(&m_Puffs[m_PuffCnt].m_Puff) CBillboard(
                TRACE_PARAM_CALL m_Pos, 5 - FRND(2), m_Puffs[m_PuffCnt].m_PuffAngle, m_Color, &m_BBTextures[tex].bbt);
    }

    m_Puffs[m_PuffCnt].m_PuffOrig = m_Pos;
    m_Puffs[m_PuffCnt].m_PuffTTL = m_PuffTTL;
    ++m_PuffCnt;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CMatrixEffectFire::CMatrixEffectFire(const D3DXVECTOR3 &pos, float ttl, float puffttl, float spawntime,
                                     float dispfactor, bool intense, float speed)
  : CMatrixEffect(), m_Speed(speed), m_DispFactor(dispfactor) {
    // m_EffectType = intense?EFFECT_SMOKE_INTENSE:EFFECT_SMOKE;
    m_EffectType = intense ? EFFECT_FIRE_INTENSE : EFFECT_FIRE;
    ELIST_ADD(EFFECT_SMOKE);

    m_PuffCnt = 0;

    m_TTL = ttl;
    m_PuffTTL = puffttl;
    m_Spawn = spawntime;

    m_Time = 0;
    m_NextSpawnTime = 0;

    m_Pos = pos;
    m_Mins = pos;
    m_Maxs = pos;

    m_Puffs = (SFirePuff *)HAlloc(sizeof(SFirePuff) * MAX_PUFF_COUNT, m_Heap);
}

CMatrixEffectFire::~CMatrixEffectFire() {
    DTRACE();
    while (m_PuffCnt > 0) {
        m_Puffs[--m_PuffCnt].m_Puff.Release();
    }

    HFree(m_Puffs, m_Heap);

    ELIST_DEL(EFFECT_SMOKE);
}

void CMatrixEffectFire::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectFire, this, m_Heap);
}

void CMatrixEffectFire::BeforeDraw(void) {}

void CMatrixEffectFire::Draw(void) {
    DTRACE();

    if (!(g_MatrixMap->m_Camera.IsInFrustum(m_Mins, m_Maxs)))
        return;

    // CHelper::Create(1,0)->BoundBox(m_Mins, m_Maxs);

    int i;

    i = 0;
    while (i < m_PuffCnt) {
        m_Puffs[i].m_Puff.Sort(g_MatrixMap->m_Camera.GetViewMatrix());
        ++i;
    }
}

void CMatrixEffectFire::SpawnPuff(void) {
    DTRACE();
    if (m_PuffCnt >= MAX_PUFF_COUNT)
        return;
    m_Puffs[m_PuffCnt].m_PuffAngle = FRND(M_PI);

    EBillboardTextureSort tex = (m_EffectType == EFFECT_FIRE)
                                        ? BBT_FIRE
                                        : ((m_EffectType == EFFECT_SMOKE)          ? BBT_SMOKE
                                           : (m_EffectType == EFFECT_FIRE_INTENSE) ? BBT_FIRE_INTENSE
                                                                                   : BBT_SMOKE_INTENSE);

    if (m_BBTextures[tex].IsIntense()) {
        new(&m_Puffs[m_PuffCnt].m_Puff) CBillboard(
                TRACE_PARAM_CALL m_Pos, 5 - FRND(2), m_Puffs[m_PuffCnt].m_PuffAngle, 0xFFFFFFFF, m_BBTextures[tex].tex);
    }
    else {
        new(&m_Puffs[m_PuffCnt].m_Puff) CBillboard(TRACE_PARAM_CALL m_Pos, 5 - FRND(2),
                                                         m_Puffs[m_PuffCnt].m_PuffAngle, 0xFFFFFFFF,
                                                         &m_BBTextures[tex].bbt);
    }

    m_Puffs[m_PuffCnt].m_PuffOrig = m_Pos;
    m_Puffs[m_PuffCnt].m_PuffTTL = m_PuffTTL;
    m_Puffs[m_PuffCnt].m_PuffDir = D3DXVECTOR2(0, 0);
    ++m_PuffCnt;
}

void CMatrixEffectFire::Takt(float step) {
    DTRACE();

    float dtime = (SMOKE_SPEED * step);

    m_Time += step;
    m_TTL -= step;
    while ((m_Time > m_NextSpawnTime) && m_TTL > 0) {
        SpawnPuff();
        m_NextSpawnTime += m_Spawn;
    }

    if ((m_PuffCnt == 0) && (m_TTL < 0)) {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        g_MatrixMap->SubEffect(this);
#endif
        return;
    }

    int i;
    i = 0;
    while (i < m_PuffCnt) {
        m_Puffs[i].m_PuffTTL -= step;
        if (m_Puffs[i].m_PuffTTL < 0) {
            m_Puffs[i].m_Puff.Release();
            m_Puffs[i] = m_Puffs[--m_PuffCnt];
            // m_MaxZ = m_Pos.z;
            continue;
        }

        ++i;
    }

    i = 0;
    while (i < m_PuffCnt) {
        float life = 1.0f - (m_Puffs[i].m_PuffTTL / m_PuffTTL);

        m_Puffs[i].m_Puff.m_Pos.z += dtime * m_Speed * (10 + (m_Puffs[i].m_Puff.m_Pos.z - m_Puffs[i].m_PuffOrig.z));
        // if (m_Puff[i].m_Pos.z > m_MaxZ) m_MaxZ = m_Puff[i].m_Pos.z;
        m_Puffs[i].m_Puff.SetScale(PUFF_FIRE_SCALE);  //(1.0f - life) * (END_OF_LIFE_SCALE - 1.0f) + 1.0f;
        // m_Puff[i].SetAlpha((BYTE)(255 * life));

        byte a = (BYTE)Float2Int(255.0f * (KSCALE(life, 0.0f, 0.3f) - (KSCALE(life, 0.3f, 1.0f))));
        // byte a = (BYTE)( 255.0f * (KSCALE(life,0.0f, 0.2f) ));
        byte r = (BYTE)Float2Int((1.0f - KSCALE(life, 0.5f, 1.0f)) * 255);
        byte g = (BYTE)Float2Int((1.0f - KSCALE(life, 0.0f, 1.0f)) * 255);
        byte b = 10 + (BYTE)Float2Int((1.0f - KSCALE(life, 0.0f, 0.1f)) * 245);
        m_Puffs[i].m_Puff.SetColor((a << 24) | (r << 16) | (g << 8) | b);

        float xl = -m_DispFactor, xr = m_DispFactor;
        float yl = -m_DispFactor, yr = m_DispFactor;

        float dx = (m_Puffs[i].m_Puff.m_Pos.x - m_Puffs[i].m_PuffOrig.x);
        float dy = (m_Puffs[i].m_Puff.m_Pos.y - m_Puffs[i].m_PuffOrig.y);

        if (dx > m_DispFactor) {
            xr = m_DispFactor / dx;
        }
        else if (dx < -m_DispFactor) {
            xl = m_DispFactor / dx;
        }
        if (dy > m_DispFactor) {
            yr = m_DispFactor / dy;
        }
        else if (dy < -m_DispFactor) {
            yl = m_DispFactor / dy;
        }

        m_Puffs[i].m_PuffDir.x += float(0.01 * dtime * RND(xl, xr));
        m_Puffs[i].m_PuffDir.y += float(0.01 * dtime * RND(yl, yr));
        m_Puffs[i].m_Puff.m_Pos.x += m_Puffs[i].m_PuffDir.x;
        m_Puffs[i].m_Puff.m_Pos.y += m_Puffs[i].m_PuffDir.y;

        ++i;
    }

    // update bbox
    if (m_PuffCnt > 0) {
        m_Mins = m_Puffs[0].m_Puff.m_Pos;
        m_Maxs = m_Mins;
        i = 0;
        while (i < m_PuffCnt) {
            AddSphereToBox(m_Mins, m_Maxs, m_Puffs[i].m_Puff.m_Pos, m_Puffs[i].m_Puff.GetScale());
            ++i;
        }
    }
    else {
        m_Mins = m_Pos;
        m_Maxs = m_Pos;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

CMatrixEffectFireStream::CMatrixEffectFireStream(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1)
  : CMatrixEffect(),
    m_bl1(TRACE_PARAM_CALL pos0, pos1, FIRE_STREAM_WIDTH, 0xFFFFFFFF, m_BBTextures[BBT_FIRESTREAM1].tex),
    m_bl2(TRACE_PARAM_CALL pos0, pos1, FIRE_STREAM_WIDTH, 0xFFFFFFFF, m_BBTextures[BBT_FIRESTREAM2].tex),
    m_bl3(TRACE_PARAM_CALL pos0, pos1, FIRE_STREAM_WIDTH, 0xFFFFFFFF, m_BBTextures[BBT_FIRESTREAM3].tex),
    m_bl4(TRACE_PARAM_CALL pos0, pos1, FIRE_STREAM_WIDTH, 0xFFFFFFFF, m_BBTextures[BBT_FIRESTREAM4].tex),
    m_bl5(TRACE_PARAM_CALL pos0, pos1, FIRE_STREAM_WIDTH, 0xFFFFFFFF, m_BBTextures[BBT_FIRESTREAM5].tex) {
    m_EffectType = EFFECT_FIRESTREAM;
}

void CMatrixEffectFireStream::BeforeDraw(void) {}

void CMatrixEffectFireStream::Draw(bool now) {
    int idx = g_MatrixMap->IsPaused() ? 2 : (IRND(5));
    if (now) {
        if (idx == 0)
            m_bl1.DrawNow(g_MatrixMap->m_Camera.GetDrawNowFC());
        else if (idx == 1)
            m_bl2.DrawNow(g_MatrixMap->m_Camera.GetDrawNowFC());
        else if (idx == 2)
            m_bl3.DrawNow(g_MatrixMap->m_Camera.GetDrawNowFC());
        else if (idx == 3)
            m_bl4.DrawNow(g_MatrixMap->m_Camera.GetDrawNowFC());
        else if (idx == 4)
            m_bl5.DrawNow(g_MatrixMap->m_Camera.GetDrawNowFC());
    }
    else {
        if (idx == 0)
            m_bl1.AddToDrawQueue();
        else if (idx == 1)
            m_bl2.AddToDrawQueue();
        else if (idx == 2)
            m_bl3.AddToDrawQueue();
        else if (idx == 3)
            m_bl4.AddToDrawQueue();
        else if (idx == 4)
            m_bl5.AddToDrawQueue();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

CMatrixEffectFireAnim::CMatrixEffectFireAnim(const D3DXVECTOR3 &pos, float w, float h, int ttl)
  : CMatrixEffect(), m_Height(h), m_Width(w), m_HeightCurr(0), m_WidthCurr(0), m_Pos(pos) {
    m_EffectType = EFFECT_FIREANIM;
    ELIST_ADD(EFFECT_FIREANIM);
    m_NextTime = g_MatrixMap->GetTime();

    m_TTLcurr = float(ttl);
    m_TTL = 1.0f / m_TTLcurr;

    m_Frame = IRND(8);

#ifdef _DEBUG
    if (m_Frame == 8)
        debugbreak();
#endif

    for (int i = 0; i < 8; ++i) {
        new(&m_bl[i]) CBillboardLine(TRACE_PARAM_CALL pos, pos + D3DXVECTOR3(0, 0, 0), 0, 0xFFFFFFFF,
                                               m_BBTextures[BBT_FLAMEFRAME0 + i].tex);
    }
}

void CMatrixEffectFireAnim::BeforeDraw(void) {}

void CMatrixEffectFireAnim::Takt(float t) {
    m_TTLcurr -= t;
    if (m_TTLcurr <= 0) {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        g_MatrixMap->SubEffect(this);
#endif
        return;
    }

    float k = m_TTLcurr * m_TTL;
    if (k < FIREFRAME_TTL_POROG) {
        k *= INVERT(FIREFRAME_TTL_POROG);
        m_HeightCurr = LERPFLOAT(k, m_Height * FIREFRAME_H_DEAD, m_Height);
        m_WidthCurr = LERPFLOAT(k, m_Width * FIREFRAME_W_DEAD, m_Width);
    }
    else {
        k = (k - FIREFRAME_TTL_POROG) * INVERT(1 - FIREFRAME_TTL_POROG);
        m_HeightCurr = LERPFLOAT(k, m_Height, m_Height * FIREFRAME_H_DEAD);
        m_WidthCurr = LERPFLOAT(k, m_Width, m_Width * FIREFRAME_W_DEAD);
    }

    Update();

    while (g_MatrixMap->GetTime() > m_NextTime) {
        m_NextTime += FIREFRAME_ANIM_PERIOD;
        ++m_Frame;
        if (m_Frame >= 8)
            m_Frame = 0;
    }
}

void CMatrixEffectFireAnim::Draw(bool now) {
    if (now) {
        m_bl[m_Frame].DrawNow(g_MatrixMap->m_Camera.GetDrawNowFC());
    }
    else {
        m_bl[m_Frame].AddToDrawQueue();
    }
}
