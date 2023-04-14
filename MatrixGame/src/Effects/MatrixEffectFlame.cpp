// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>

#include "MatrixEffect.hpp"
#include "MatrixEffectShleif.hpp"
#include "../MatrixMap.hpp"
#include "../MatrixObject.hpp"
#include "../MatrixObjectRobot.hpp"
#include "../MatrixObjectCannon.hpp"
#include "../MatrixFlyer.hpp"
#include <math.h>

#include "MatrixEffectFlame.hpp"
#include "MatrixEffectPointLight.hpp"

CFlamePuff::CFlamePuff(CMatrixEffectFlame *owner, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir,
                       const D3DXVECTOR3 &speed)
  : m_Owner(owner), m_Pos(pos), m_Dir(dir), m_Speed(speed), m_Next(NULL), m_Prev(NULL), m_Shleif(NULL), m_CurAlpha(0),
    m_Break(0)
#ifdef _DEBUG
    ,
    m_Light(DEBUG_CALL_INFO)
#endif
{
    DTRACE();

    m_Time = 0;

    // static int n = 0;
    //++n;
    // if (n > 5)
    {
        CMatrixEffect::CreatePointLight(&m_Light, pos, 20, 0xFFFFFFFF, false);
        // n = 0;
    }  // else
    //{
    // m_Light = NULL;
    // m_TTL >>= 1;
    //    }

    DWORD color = (m_Owner->m_TTL < 1000) ? 0x10101090 : 0xFFFFFFFF;
    m_Alpha = float(color >> 24);

    for (int i = 0; i < FLAME_NUM_BILLS; ++i) {
        if (m_Owner->m_BBTextures[BBT_FLAME].IsIntense()) {
            new(&m_Flames[i].m_Flame) CBillboard(TRACE_PARAM_CALL m_Pos, 1, 0, 0xFFFFFFFF,
                                                       m_Owner->m_BBTextures[BBT_FLAME].tex);
        }
        else {
            new(&m_Flames[i].m_Flame) CBillboard(TRACE_PARAM_CALL m_Pos, 1, 0, 0xFFFFFFFF,
                                                       &m_Owner->m_BBTextures[BBT_FLAME].bbt);
        }
        m_Flames[i].m_Sign = (FRND(1) > 0.5f) ? 1.0f : -1.0f;
    }

    m_NextSeekTime = 0;
    m_NextSmokeTime = 0;
}

CFlamePuff::~CFlamePuff() {
    DTRACE();
#ifdef _DEBUG
    m_Light.Release(DEBUG_CALL_INFO);
#else
    m_Light.Release();
#endif

    // if (m_Next) m_Next->m_Prev = NULL;
    // if (m_Prev) m_Prev->m_Next = NULL;

    if (m_Shleif) {
        m_Shleif->Release();
        HFree(m_Shleif, m_Owner->m_Heap);
    }
}

void CFlamePuff::Release(void) {
    DTRACE();
    HDelete(CFlamePuff, this, m_Owner->m_Heap);
}

void CFlamePuff::Draw(void) {
    DTRACE();

    for (int i = 0; i < FLAME_NUM_BILLS; ++i) {
        m_Flames[i].m_Flame.Sort(g_MatrixMap->m_Camera.GetViewMatrix());
    }
    if (m_Shleif) {
        if (m_Prev) {
            BYTE a = BYTE(m_CurAlpha);
            m_Shleif->SetPos(m_Prev->m_Pos, m_Pos);
            auto tmp = m_Prev->m_Pos - m_Pos;
            float l = D3DXVec3Length(&tmp);
            if (l < (m_Scale * 2.0f)) {
                a = BYTE(float(m_CurAlpha) * (l * 0.5f / m_Scale));
            }
            m_Shleif->SetWidth(m_Scale * 2.0f);
            m_Shleif->SetAlpha(a);
            m_Shleif->AddToDrawQueue();
        }
    }
}

struct FlameData {
    D3DXVECTOR3 norm;
    FIRE_END_HANDLER handler;
    DWORD uvalue;
};

static bool FlameEnum(const D3DXVECTOR3 &center, CMatrixMapStatic *ms, DWORD user) {
    FlameData *fd = (FlameData *)user;

    if (ms->GetObjectType() != OBJECT_TYPE_ROBOTAI) {
        D3DXVECTOR3 anorm;
        auto tmp = center - ms->GetGeoCenter();
        D3DXVec3Normalize(&anorm, &tmp);
        fd->norm += anorm;
    }
    if (fd->handler)
        fd->handler(ms, center + fd->norm * ms->GetRadius(), fd->uvalue, 0);
    return true;
}

void CFlamePuff::Takt(float step) {
    DTRACE();

    m_Time += step;

    if (m_Time > m_Owner->m_TTL) {
        m_Owner->SubPuff(this);
        return;
    }
    float k = m_Time * m_Owner->_m_TTL;
    if (k > 0.3f) {
        while (m_Time > m_NextSmokeTime) {
            m_NextSmokeTime += 100;
            m_Owner->CreateSmoke(m_Pos);
        }
        // CMatrixEffect::CreateFire(m_Pos + m_Dir * FSRND(5), 100, int(k * 700.0f), 20, 4, true);
    }
    else {
        m_NextSmokeTime = m_Time;
    }
    // if (k > 0.3f)
    //{
    //}
    float scale = (k * FLAME_SCALE_FACTOR) + 1.5f;
    // float _scale = FLAME_DIST_FACTOR / scale;
    float mk = 1.0f - k;

    D3DXVECTOR3 oldpos = m_Pos;
    m_Pos += (m_Speed * float(step)) + (float(step) * m_Dir * mk * mk * FLAME_DIR_SPEED);

    m_Scale = scale * scale;

    // CHelper::Create(1,0)->Line(m_Pos-D3DXVECTOR3(scale,scale,0)*2,m_Pos-D3DXVECTOR3(-scale,scale,0)*2);
    // CHelper::Create(1,0)->Line(m_Pos-D3DXVECTOR3(-scale,scale,0)*2,m_Pos-D3DXVECTOR3(-scale,-scale,0)*2);
    // CHelper::Create(1,0)->Line(m_Pos-D3DXVECTOR3(-scale,-scale,0)*2,m_Pos-D3DXVECTOR3(scale,-scale,0)*2);
    // CHelper::Create(1,0)->Line(m_Pos-D3DXVECTOR3(scale,-scale,0)*2,m_Pos-D3DXVECTOR3(scale,scale,0)*2);

    FlameData data;
    data.norm = D3DXVECTOR3(0, 0, 0);
    data.handler = m_Owner->m_Handler;
    data.uvalue = m_Owner->m_User;
    bool hit = false;
    if (m_Owner->m_hitmask & TRACE_WATER) {
        if ((m_Pos.z - m_Scale) <= WATER_LEVEL) {
            if (m_Owner->m_Handler)
                m_Owner->m_Handler(TRACE_STOP_WATER, D3DXVECTOR3(m_Pos.x, m_Pos.y, WATER_LEVEL), m_Owner->m_User, 0);
            data.norm.z += 1;
            hit = true;
        }
    }
    if (m_Owner->m_hitmask & TRACE_LANDSCAPE) {
        float z = g_MatrixMap->GetZ(m_Pos.x, m_Pos.y);
        if ((m_Pos.z - m_Scale) <= z) {
            auto tmp1 = m_Pos - oldpos;
            float l = D3DXVec3Length(&tmp1);
            m_Pos.z = z + m_Scale;

            D3DXVECTOR3 n;
            auto tmp2 = m_Pos - oldpos;
            D3DXVec3Normalize(&n, &tmp2);
            m_Pos = oldpos + n * l;
        }
    }

    if (m_Time > m_NextSeekTime) {
        m_NextSeekTime = m_Time + FLAME_TIME_SEEK_PERIOD;
        if (m_Owner->m_hitmask & TRACE_ANYOBJECT) {
            hit = g_MatrixMap->FindObjects(m_Pos, m_Scale, 0.7f, m_Owner->m_hitmask, m_Owner->m_skip, FlameEnum,
                                           (DWORD)&data);
        }
    }

    if (hit) {
        D3DXVec3Normalize(&data.norm, &data.norm);
        m_Pos += data.norm * scale * (0.02f * step);
    }

    float k1 = 1.0f - KSCALE(k, 0.5f, 1.0f);
    m_CurAlpha = BYTE(m_Alpha * k1);
    if (m_Light.effect) {
        BYTE alpha2 = BYTE(m_Alpha * k1 * 0.3f);
        BYTE alpha3 = BYTE(m_Alpha * k1 * 0.1f);
        ((CMatrixEffectPointLight *)m_Light.effect)
                ->SetColor((alpha2 << 24) | (alpha2 << 16) | (alpha2 << 8) | (alpha3));
        ((CMatrixEffectPointLight *)m_Light.effect)->SetPosAndRadius(m_Pos, m_Scale * 3);
    }

    for (int i = 0; i < FLAME_NUM_BILLS; ++i) {
        float angle = float(2 * M_PI / 1024.0) * (Float2Int(m_Time) & 1023) * m_Flames[i].m_Sign;

        // float lscale = 1.0f / (float(i)* 0.1f + 1.0f);

        m_Flames[i].m_Flame.SetScale(m_Scale);
        m_Flames[i].m_Flame.SetAngle(angle, 0, 0);

        if (i == 0) {
            m_Flames[0].m_Flame.SetAlpha(BYTE(m_CurAlpha));
            m_Flames[0].m_Flame.m_Pos = m_Pos;
        }
        else {
            m_CurAlpha = BYTE(m_Alpha * k1 * k1);
            float kk = (1.0f - float(i) / FLAME_NUM_BILLS);
            BYTE rg = BYTE(kk * 128.0f);
            int b = BYTE(kk * 512.0f);
            if (b > 255)
                b = 255;
            m_Flames[i].m_Flame.SetColor(m_CurAlpha << 24 | (rg << 16) | (rg << 8) | b);

            D3DXVECTOR3 delta((m_Flames[i - 1].m_Flame.m_Pos - m_Flames[i].m_Flame.m_Pos));
            float x = D3DXVec3Length(&delta);
            if (x > scale * k1) {
                m_Flames[i].m_Flame.m_Pos += delta * (x - scale * k1) / x;
            }
        }
    }

    if (m_Shleif) {
        if (m_Prev == NULL) {
            m_Shleif->Release();
            HFree(m_Shleif, m_Owner->m_Heap);
            m_Shleif = NULL;
        }
    }
    else {
        if (m_Prev && (!m_Break)) {
            auto tmp = m_Prev->m_Pos - m_Pos;
            float l = D3DXVec3Length(&tmp);
            BYTE a = BYTE(m_CurAlpha);
            if (l < (m_Scale * 2.0f)) {
                a = BYTE(float(m_CurAlpha) * (l * 0.5f / m_Scale));
            }
            m_Shleif = (CBillboardLine *)HAlloc(sizeof(CBillboardLine), m_Owner->m_Heap);
            new(m_Shleif) CBillboardLine(TRACE_PARAM_CALL m_Prev->m_Pos, m_Pos, m_Scale * 2.0f,
                                                     (a << 24) | 0xFFFFFF, m_Owner->GetBBTexI(BBT_FLAMELINE));
        }
    }
}

////////////////

CMatrixEffectFlame::CMatrixEffectFlame(float ttl, DWORD hitmask, CMatrixMapStatic *skip, DWORD user,
                                       FIRE_END_HANDLER handler)
  : CMatrixEffect(), m_User(user), m_Handler(handler), m_hitmask(hitmask), m_skip(skip), m_TTL(ttl),
    _m_TTL(INVERT(ttl)), m_First(NULL), m_Last(NULL)
#ifdef _DEBUG
    ,
    m_Smokes(DEBUG_CALL_INFO)
#endif
{
    DTRACE();

    m_EffectType = EFFECT_FLAME;
}

CMatrixEffectFlame::~CMatrixEffectFlame() {
    while (m_First)
        SubPuff(m_First);

#ifdef _DEBUG
    m_Smokes.Release(DEBUG_CALL_INFO);
#else
    m_Smokes.Release();
#endif

    if (m_Handler)
        m_Handler(TRACE_STOP_NONE, D3DXVECTOR3(0, 0, 0), m_User, FEHF_LASTHIT);
    // m_Owner->Release();
}

void CMatrixEffectFlame::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectFlame, this, m_Heap);
}

void CMatrixEffectFlame::AddPuff(const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &speed) {
    CFlamePuff *p = HNew(m_Heap) CFlamePuff(this, start, dir, speed);

    p->m_Next = m_First;
    if (m_First) {
        m_First->m_Prev = p;
    }
    else {
        m_Last = p;
    }
    p->m_Prev = NULL;
    m_First = p;
    if (m_Stream == NULL && p->m_Next) {
        p->m_Next->m_Break = 1;
    }

    m_Stream = p;
}

void CMatrixEffectFlame::SubPuff(CFlamePuff *puf) {
    // if (m_Handler && m_EffectHandler) m_Handler(TRACE_STOP_NONE, puf->m_Pos, m_User,FEHF_LASTHIT);
    if (m_Stream == puf)
        m_Stream = NULL;
    LIST_DEL(puf, m_First, m_Last, m_Prev, m_Next);
    puf->Release();
}

void CMatrixEffectFlame::BeforeDraw(void) {}

void CMatrixEffectFlame::Draw(void) {
    for (CFlamePuff *p = m_First; p; p = p->m_Next) {
        p->Draw();
    }
}

void CMatrixEffectFlame::Takt(float ms) {
    bool no_more_puffs = true;
    for (CFlamePuff *p = m_First; p;) {
        no_more_puffs = false;

        CFlamePuff *next = p->m_Next;

        p->Takt(ms);

        p = next;
    }

    if (no_more_puffs) {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        g_MatrixMap->SubEffect(this);
#endif
    }
}

void CMatrixEffectFlame::CreateSmoke(const D3DXVECTOR3 &pos) {
    if (m_Smokes.effect == NULL) {
        CMatrixEffect::CreateShleif(&m_Smokes);
    }
    if (m_Smokes.effect != NULL) {
        ((CMatrixEffectShleif *)m_Smokes.effect)->AddSmoke(pos, 300, 700, 400, 0x2F303030, false, 0);
    }
}
