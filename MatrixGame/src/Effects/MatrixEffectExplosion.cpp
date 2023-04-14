// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>

#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include "../MatrixRenderPipeline.hpp"
#include <math.h>

#include "MatrixEffectExplosion.hpp"
#include "MatrixEffectPointLight.hpp"

// explosions
const SExplosionProperties ExplosionNormal = {
        0,    // float min_speed;
        7,    // float max_speed;
        -15,  // float min_speed_z;
        15,   // float max_speed_z;

        10,    // int   sparks_deb_cnt;
        7,     // int   fire_deb_cnt;
        200,   // int   deb_cnt;
        1000,  // int   deb_min_ttl;
        4000,  // int   deb_max_ttl;

        6,  // int   intense_cnt;

        0,  // int   deb_type;

        false,  // bool  light;
        0,      // float light_radius1;
        0,      // float light_radius2;
        0,      // DWORD light_color1;
        0,      // DWORD light_color2;
        0,      // float   light_time1;
        0,      // float   light_time2;

        S_EXPLOSION_NORMAL,
        SPOT_VORONKA,  // ESpotType voronka;
        20,            // float     voronka_scale;
};

const SExplosionProperties ExplosionMissile = {
        0,    // float min_speed;
        7,    // float max_speed;
        -15,  // float min_speed_z;
        25,   // float max_speed_z;

        10,    // int   sparks_deb_cnt;
        2,     // int   fire_deb_cnt;
        0,     // int   deb_cnt;
        1000,  // int   deb_min_ttl;
        4000,  // int   deb_max_ttl;

        6,  // int   intense_cnt;

        1,  // int   deb_type;

        true,        // bool  light;
        10,          // float light_radius1;
        100,         // float light_radius2;
        0,           // DWORD light_color1;
        0xFFFF6F33,  // DWORD light_color2;
        1.5f,        // float   light_time1;
        8,           // float   light_time2;

        S_EXPLOSION_MISSILE,
        SPOT_TYPES_CNT,  // ESpotType voronka;
        0,               // float     voronka_scale;
};

const SExplosionProperties ExplosionRobotHit = {
        0,   // float min_speed;
        5,   // float max_speed;
        -5,  // float min_speed_z;
        5,   // float max_speed_z;

        5,     // int   sparks_deb_cnt;
        0,     // int   fire_deb_cnt;
        1,     // int   deb_cnt;
        1000,  // int   deb_min_ttl;
        2000,  // int   deb_max_ttl;
        0,     // int   intense_cnt;

        1,  // int   deb_type;

        false,       // bool  light;
        2,           // float light_radius1;
        30,          // float light_radius2;
        0xFFFFFFFF,  // DWORD light_color1;
        0x11FFFF11,  // DWORD light_color2;
        1.5f,        // float   light_time1;
        3,           // float   light_time2;

        S_EXPLOSION_ROBOT_HIT,
        SPOT_TYPES_CNT,  // ESpotType voronka;
        0,               // float     voronka_scale;
};

const SExplosionProperties ExplosionLaserHit = {
        0,   // float min_speed;
        5,   // float max_speed;
        -5,  // float min_speed_z;
        5,   // float max_speed_z;

        10,    // int   sparks_deb_cnt;
        0,     // int   fire_deb_cnt;
        0,     // int   deb_cnt;
        1000,  // int   deb_min_ttl;
        2000,  // int   deb_max_ttl;
        0,     // int   intense_cnt;

        0,  // int   deb_type;

        true,        // bool  light;
        2,           // float light_radius1;
        10,          // float light_radius2;
        0xFFFFFFFF,  // DWORD light_color1;
        0x11200505,  // DWORD light_color2;
        1.5f,        // float   light_time1;
        3,           // float   light_time2;

        S_EXPLOSION_LASER_HIT,
        SPOT_TYPES_CNT,  // ESpotType voronka;
        0,               // float     voronka_scale;
};

const SExplosionProperties ExplosionBuildingBoom = {
        0,   // float min_speed;
        10,  // float max_speed;
        -5,  // float min_speed_z;
        10,  // float max_speed_z;

        2,     // int   sparks_deb_cnt;
        0,     // int   fire_deb_cnt;
        1,     // int   deb_cnt;
        1000,  // int   deb_min_ttl;
        2000,  // int   deb_max_ttl;
        4,     // int   intense_cnt;
        1,     // int   deb_type;

        false,       // bool  light;
        2,           // float light_radius1;
        10,          // float light_radius2;
        0xFFFFFFFF,  // DWORD light_color1;
        0x11FF3111,  // DWORD light_color2;
        1.5f,        // float   light_time1;
        3,           // float   light_time2;

        S_NONE,
        SPOT_TYPES_CNT,  // ESpotType voronka;
        0,               // float     voronka_scale;
};

const SExplosionProperties ExplosionBuildingBoom2 = {
        10,  // float min_speed;
        15,  // float max_speed;
        -5,  // float min_speed_z;
        15,  // float max_speed_z;

        20,    // int   sparks_deb_cnt;
        1,     // int   fire_deb_cnt;
        10,    // int   deb_cnt;
        1000,  // int   deb_min_ttl;
        2000,  // int   deb_max_ttl;
        3,     // int   intense_cnt;
        1,     // int   deb_type;

        false,       // bool  light;
        2,           // float light_radius1;
        10,          // float light_radius2;
        0xFFFFFFFF,  // DWORD light_color1;
        0x11FF3111,  // DWORD light_color2;
        1.5f,        // float   light_time1;
        3,           // float   light_time2;

        S_EXPLOSION_BUILDING_BOOM2,
        SPOT_TYPES_CNT,  // ESpotType voronka;
        0,               // float     voronka_scale;
};

const SExplosionProperties ExplosionRobotBoom = {
        0,   // float min_speed;
        7,   // float max_speed;
        0,   // float min_speed_z;
        10,  // float max_speed_z;

        50,    // int   sparks_deb_cnt;
        3,     // int   fire_deb_cnt;
        30,    // int   deb_cnt;
        1000,  // int   deb_min_ttl;
        4000,  // int   deb_max_ttl;

        6,  // int   intense_cnt;
        1,  // int   deb_type;

        true,        // bool  light;
        10,          // float light_radius1;
        70,          // float light_radius2;
        0,           // DWORD light_color1;
        0xFFFF6F33,  // DWORD light_color2;
        1.5f,        // float   light_time1;
        8,           // float   light_time2;

        S_EXPLOSION_ROBOT_BOOM,
        SPOT_TYPES_CNT,  // ESpotType voronka;
        0,               // float     voronka_scale;
};

const SExplosionProperties ExplosionRobotBoomSmall = {
        0,   // float min_speed;
        7,   // float max_speed;
        0,   // float min_speed_z;
        10,  // float max_speed_z;

        20,    // int   sparks_deb_cnt;
        2,     // int   fire_deb_cnt;
        10,    // int   deb_cnt;
        800,   // int   deb_min_ttl;
        2000,  // int   deb_max_ttl;

        4,  // int   intense_cnt;
        1,  // int   deb_type;

        true,        // bool  light;
        10,          // float light_radius1;
        50,          // float light_radius2;
        0,           // DWORD light_color1;
        0xFFFF6F33,  // DWORD light_color2;
        1.5f,        // float   light_time1;
        8,           // float   light_time2;

        S_EXPLOSION_ROBOT_BOOM_SMALL,
        SPOT_TYPES_CNT,  // ESpotType voronka;
        0,               // float     voronka_scale;
};

const SExplosionProperties ExplosionBigBoom = {
        0,    // float min_speed_x;
        25,   // float max_speed_x;
        -25,  // float min_speed_z;
        25,   // float max_speed_z;

        1000,  // int   sparks_deb_cnt;
        0,     // int   fire_deb_cnt;
        400,   // int   deb_cnt;
        1000,  // int   deb_min_ttl;
        4000,  // int   deb_max_ttl;

        16,  // int   intense_cnt;
        1,   // int   deb_type;

        true,        // bool  light;
        10,          // float light_radius1;
        70,          // float light_radius2;
        0,           // DWORD light_color1;
        0xFFFF6F33,  // DWORD light_color2;
        1.5f,        // float   light_time1;
        8,           // float   light_time2;

        S_EXPLOSION_BIGBOOM,
        SPOT_VORONKA,  // ESpotType voronka;
        12,            // float     voronka_scale;
};

const SExplosionProperties ExplosionObject = {
        0,   // float min_speed;
        7,   // float max_speed;
        0,   // float min_speed_z;
        10,  // float max_speed_z;

        20,    // int   sparks_deb_cnt;
        2,     // int   fire_deb_cnt;
        20,    // int   deb_cnt;
        1000,  // int   deb_min_ttl;
        2000,  // int   deb_max_ttl;

        8,  // int   intense_cnt;
        1,  // int   deb_type;

        true,        // bool  light;
        10,          // float light_radius1;
        70,          // float light_radius2;
        0,           // DWORD light_color1;
        0xFFFF6F33,  // DWORD light_color2;
        1.5f,        // float   light_time1;
        8,           // float   light_time2;

        S_EXPLOSION_OBJECT,
        SPOT_VORONKA,  // ESpotType voronka;
        10,            // float     voronka_scale;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CMatrixEffectExplosion::CMatrixEffectExplosion(const D3DXVECTOR3 &pos, const SExplosionProperties &props)
  : CMatrixEffect()
#ifdef _DEBUG
    ,
    m_Light(DEBUG_CALL_INFO)
#endif
{
    DTRACE();

    m_EffectType = EFFECT_EXPLOSION;
    ELIST_ADD(EFFECT_EXPLOSION);

    m_Props = &props;

    m_Time = 0;

    int dt_cnt = 0;
    int dt_idx = -1;
    for (int ii = 0; ii < m_DebrisCnt; ++ii) {
        if (m_Debris[ii].GetDebType() == m_Props->deb_type) {
            ++dt_cnt;
            if (dt_idx == -1)
                dt_idx = ii;
        }
    }

    if (dt_idx < 0)
        ERROR_E;

    m_Light.effect = NULL;
    if (m_Props->light) {
        CMatrixEffect::CreatePointLight(&m_Light, pos, m_Props->light_radius1, m_Props->light_color1, false);
    }

    m_DebCnt = m_Props->deb_cnt + m_Props->fire_deb_cnt + m_Props->intense_cnt + m_Props->sparks_deb_cnt;
    m_Deb = (SDebris *)HAlloc(sizeof(SDebris) * m_DebCnt, m_Heap);

    int fire_cnt = m_Props->fire_deb_cnt;
    int intense_cnt = m_Props->intense_cnt;
    int sparks_cnt = m_Props->sparks_deb_cnt;

    // creating fires

    int i = 0;

    for (; fire_cnt > 0; ++i) {
        m_Deb[i].u1.s3.fire.effect = NULL;
        m_Deb[i].u1.s3.light.effect = NULL;

        CMatrixEffect::CreateFire(&m_Deb[i].u1.s3.fire, pos, 1000000, 1000, 50, 5, false);
        CMatrixEffect::CreatePointLight(&m_Deb[i].u1.s3.light, pos, 20, 0x22222202, false);

        if (m_Deb[i].u1.s3.light.effect == NULL || m_Deb[i].u1.s3.fire.effect == NULL) {
#ifdef _DEBUG
            m_Deb[i].u1.s3.light.Release(DEBUG_CALL_INFO);
            m_Deb[i].u1.s3.fire.Release(DEBUG_CALL_INFO);
#else
            m_Deb[i].u1.s3.light.Release();
            m_Deb[i].u1.s3.fire.Release();
#endif
            m_DebCnt -= fire_cnt;  // no more slots for effects
            fire_cnt = 0;
            break;
        }
        --fire_cnt;

        m_Deb[i].ttl = (float)RND(m_Props->deb_min_ttl, m_Props->deb_max_ttl);
        m_Deb[i].type = DEB_FIRE;

        float r = (float)RND(m_Props->min_speed, m_Props->max_speed);
        float a = FSRND(M_PI);
        m_Deb[i].v.x = TableSin(a) * r * 0.5f;
        m_Deb[i].v.y = TableCos(a) * r * 0.5f;
        m_Deb[i].v.z = (float)RND(m_Props->min_speed_z * 0.5f, m_Props->max_speed_z * 0.5f);
    }

    if (i >= m_DebCnt)
        return;

    // creating intenses

    for (; intense_cnt > 0; ++i) {
        --intense_cnt;
        m_Deb[i].type = DEB_INTENSE;
        m_Deb[i].u1.s2.billboard = (CBillboard *)HAlloc(sizeof(CBillboard), m_Heap);
        new(m_Deb[i].u1.s2.billboard) CBillboard(TRACE_PARAM_CALL pos, INTENSE_INIT_SIZE, FSRND(M_PI), 0x1F807E1B,
                                                   m_BBTextures[BBT_INTENSE].tex);
        m_Deb[i].u1.s2.ttm = FRND(200);

        float sn, cs;
        SinCos(FSRND(M_PI), &sn, &cs);
        m_Deb[i].v = D3DXVECTOR3(float(INTENSE_DISPLACE_RADIUS * cs), float(INTENSE_DISPLACE_RADIUS * sn), 0);
        m_Deb[i].ttl = INTENSE_TTL;
    }

    if (i >= m_DebCnt)
        return;

    // creating sparks

    for (; sparks_cnt > 0; ++i) {
        --sparks_cnt;
        m_Deb[i].type = DEB_SPARK;
        m_Deb[i].u1.s1.pos = pos;
        m_Deb[i].u1.s1.u2.s5.prepos = pos;
        // m_Deb[i].len = 20;

        float r = (float)RND(m_Props->min_speed, m_Props->max_speed);
        SinCos(FSRND(M_PI), &m_Deb[i].v.x, &m_Deb[i].v.y);
        m_Deb[i].v.x *= r;
        m_Deb[i].v.y *= r;
        m_Deb[i].v.z = (float)RND(m_Props->min_speed_z, m_Props->max_speed_z);

        m_Deb[i].u1.s1.u2.s5.bline = (CBillboardLine *)HAlloc(sizeof(CBillboardLine), m_Heap);
        new(m_Deb[i].u1.s1.u2.s5.bline) CBillboardLine(TRACE_PARAM_CALL pos, pos + m_Deb[i].v * (1.0f / 20.0f),
                                                       EXPLOSION_SPARK_WIDTH, 0xFFFFFFFF, GetBBTexI(BBT_SPARK));
        m_Deb[i].ttl = (float)RND(m_Props->deb_min_ttl, m_Props->deb_max_ttl);
        m_Deb[i].u1.s1.u2.s5.unttl = 1.0f / m_Deb[i].ttl;
    }

    if (i >= m_DebCnt)
        return;

    // creating debrises

    for (; i < m_DebCnt; ++i) {
        m_Deb[i].u1.s1.pos = pos;
        m_Deb[i].u1.s1.u2.s4.alpha = 1.0f;
        m_Deb[i].u1.s1.u2.s4.scale = 1.0f;

        m_Deb[i].u1.s1.u2.s4.angley = FSRND(1);
        m_Deb[i].u1.s1.u2.s4.anglep = FSRND(1);
        m_Deb[i].u1.s1.u2.s4.angler = FSRND(1);
        m_Deb[i].index = dt_idx + (rand() % dt_cnt);

        ASSERT(m_Deb[i].index >= 0);

        float r = (float)RND(m_Props->min_speed, m_Props->max_speed);
        SinCos(FSRND(M_PI), &m_Deb[i].v.x, &m_Deb[i].v.y);
        m_Deb[i].v.x *= r;
        m_Deb[i].v.y *= r;
        m_Deb[i].v.z = (float)RND(m_Props->min_speed_z, m_Props->max_speed_z);
        m_Deb[i].ttl = (float)RND(m_Props->deb_min_ttl, m_Props->deb_max_ttl);
    }
}

void CMatrixEffectExplosion::RemoveDebris(int debi) {
    DTRACE();

    ASSERT(debi < m_DebCnt && debi >= 0);

    SDebris *deb = m_Deb + debi;

#if defined _DEBUG || defined _TRACE
    DCP();
    if (deb->type == DEB_DEAD)
        debugbreak();
    DCP();
#endif

    if (deb->type == DEB_INTENSE) {
        deb->u1.s2.billboard->Release();
        HFree(deb->u1.s2.billboard, m_Heap);
    }
    else if (deb->type == DEB_FIRE) {
        if (deb->u1.s3.fire.effect) {
            ((CMatrixEffectFire *)deb->u1.s3.fire.effect)->SetTTL(1000);
            deb->u1.s3.fire.Unconnect();
        }
        if (deb->u1.s3.light.effect) {
            ((CMatrixEffectPointLight *)deb->u1.s3.light.effect)->Kill(3000);
            deb->u1.s3.light.Unconnect();
        }
    }
    else if (deb->type == DEB_SPARK) {
        deb->u1.s1.u2.s5.bline->Release();
        HFree(deb->u1.s1.u2.s5.bline, m_Heap);
    }

    --m_DebCnt;
    if (debi < m_DebCnt) {
        // non last debris

        memcpy(deb, m_Deb + m_DebCnt, sizeof(SDebris));

        if (deb->type == DEB_FIRE) {
            deb->u1.s3.fire.Rebase();
            deb->u1.s3.light.Rebase();
        }

#if defined _DEBUG || defined _TRACE
        m_Deb[m_DebCnt].type = DEB_DEAD;
#endif
    }
    else {
        // it was last debris
#if defined _DEBUG || defined _TRACE
        deb->type = DEB_DEAD;
#endif
    }
}

CMatrixEffectExplosion::~CMatrixEffectExplosion() {
    DTRACE();

    while (m_DebCnt > 0) {
        RemoveDebris(m_DebCnt - 1);
    }
    HFree(m_Deb, m_Heap);
    ELIST_DEL(EFFECT_EXPLOSION);
}

void CMatrixEffectExplosion::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectExplosion, this, m_Heap);
}

void CMatrixEffectExplosion::BeforeDraw(void) {
    if (FLAG(m_before_draw_done, BDDF_EXPLOSION))
        return;

    DTRACE();

    for (int i = 0; i < m_DebrisCnt; ++i) {
        m_Debris[i].BeforeDraw();
    }

    SETFLAG(m_before_draw_done, BDDF_EXPLOSION);
}

void CMatrixEffectExplosion::Draw(void) {
    DTRACE();

    CVectorObject::DrawBegin();
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, 0xFFFFFFFF));

    D3DXMATRIX mat;

    int i = 0;
    while (i < m_DebCnt) {
        if (m_Deb[i].type == DEB_FIRE) {
            ++i;
            continue;
        }
        if (m_Deb[i].type == DEB_INTENSE) {
            if (m_Deb[i].u1.s2.ttm < 0) {
                m_Deb[i].u1.s2.billboard->Sort(g_MatrixMap->m_Camera.GetViewMatrix());
            }
            ++i;
            continue;
        }
        if (m_Deb[i].type == DEB_SPARK) {
            m_Deb[i].u1.s1.u2.s5.bline->AddToDrawQueue();
            ++i;
            continue;
        }

#if defined _DEBUG || defined _TRACE
        DCP();
        if (m_Deb[i].type == DEB_DEAD)
            debugbreak();
        DCP();
#endif

        float k = float(m_Deb[i].ttl) / 3500.0f;
        if (k < 0)
            k = 0;
        if (k > 1)
            k = 1;
        BYTE cc = BYTE(k * 255.0f);

        g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFF000000 | (cc << 16) | (cc << 8) | (cc << 0));

        D3DXMatrixRotationYawPitchRoll(
            &mat,
            m_Deb[i].u1.s1.u2.s4.angley * m_Time,
            m_Deb[i].u1.s1.u2.s4.anglep * m_Time,
            m_Deb[i].u1.s1.u2.s4.angler * m_Time);

        mat._11 *= m_Deb[i].u1.s1.u2.s4.scale;
        mat._12 *= m_Deb[i].u1.s1.u2.s4.scale;
        mat._13 *= m_Deb[i].u1.s1.u2.s4.scale;
        mat._21 *= m_Deb[i].u1.s1.u2.s4.scale;
        mat._22 *= m_Deb[i].u1.s1.u2.s4.scale;
        mat._23 *= m_Deb[i].u1.s1.u2.s4.scale;
        mat._31 *= m_Deb[i].u1.s1.u2.s4.scale;
        mat._32 *= m_Deb[i].u1.s1.u2.s4.scale;
        mat._33 *= m_Deb[i].u1.s1.u2.s4.scale;

        mat._41 = m_Deb[i].u1.s1.pos.x;
        mat._42 = m_Deb[i].u1.s1.pos.y;
        mat._43 = m_Deb[i].u1.s1.pos.z;

        ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &mat));
        m_Debris[m_Deb[i].index].Draw(0);
        ++i;
    }

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, FALSE));
    CVectorObject::DrawEnd();
}

void CMatrixEffectExplosion::Takt(float step) {
    DTRACE();

    float dtime = (DEBRIS_SPEED * step);

    m_Time += dtime;

    int i;

    if (m_Light.effect) {
        if (m_Time < m_Props->light_time1) {
            float k = (m_Time / m_Props->light_time1);

            ((CMatrixEffectPointLight *)m_Light.effect)->SetColor(LIC(m_Props->light_color1, m_Props->light_color2, k));
            ((CMatrixEffectPointLight *)m_Light.effect)
                    ->SetRadius(LERPFLOAT(k, m_Props->light_radius1, m_Props->light_radius2));
        }
        else {
            float kill_time = float(m_Props->light_time2 - m_Props->light_time1) / DEBRIS_SPEED;
            ((CMatrixEffectPointLight *)m_Light.effect)->Kill(kill_time);
            m_Light.Unconnect();
        }
    }

    /*
    for (i = 0; i<m_DebrisCnt; ++i)
    {
        m_Debris[i].Takt(step);
    }
    */

    // update debrises lives

    i = 0;
    while (i < m_DebCnt) {
        if (m_Deb[i].type == DEB_INTENSE) {
            if (m_Deb[i].u1.s2.ttm >= 0) {
                m_Deb[i].u1.s2.ttm -= step;
            }
            else {
                if ((m_Deb[i].ttl -= step) < 0) {
                    RemoveDebris(i);
                    continue;
                }
            }
        }
        else if ((m_Deb[i].ttl -= step) < 0) {
            RemoveDebris(i);
            continue;
        }
        ++i;
    }

    if (m_DebCnt == 0) {
        if (m_Light.effect)
            return;  // light not yet finished
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        g_MatrixMap->SubEffect(this);
#endif
        return;
    }

    for (i = 0; i < m_DebCnt; ++i) {
        SDebris *deb = m_Deb + i;

#if defined _DEBUG || defined _TRACE
        DCP();
        if (deb->type == DEB_DEAD)
            debugbreak();
        DCP();
#endif

        if (deb->type == DEB_INTENSE) {
            if (deb->u1.s2.ttm < 0) {
                float t = 1.0f - ((float)deb->ttl * INVERT(INTENSE_TTL));

                float displace_factor = (t > 0.05f) ? 1 : t * 20;

                m_Deb[i].u1.s2.billboard->DisplaceTo(D3DXVECTOR2(deb->v.x * displace_factor, deb->v.y * displace_factor));
                m_Deb[i].u1.s2.billboard->SetScale(1.0f + (INTENSE_END_SIZE / INTENSE_INIT_SIZE - 1.0f) * displace_factor);

                const SGradient R[3] = {{150, 0}, {100, 0.5f}, {0, 1.01f}};
                const SGradient G[4] = {{34, 0}, {106, 0.25f}, {29, 0.5f}, {0, 1.01f}};
                const SGradient B[4] = {{23, 0}, {29, 0.25f}, {20, 0.5f}, {0, 1.01f}};

                BYTE a = (BYTE)Float2Int((1.0f - KSCALE(t, 0.0f, 1.0f)) * 255);
                BYTE r = (BYTE)Float2Int(CalcGradient(t, R));
                BYTE g = (BYTE)Float2Int(CalcGradient(t, G));
                BYTE b = (BYTE)Float2Int(CalcGradient(t, B));
                deb->u1.s2.billboard->SetColor((a << 24) | (r << 16) | (g << 8) | b);
            }
            continue;
        }
        else if (m_Deb[i].type == DEB_FIRE) {
            if (deb->u1.s3.light.effect == NULL || deb->u1.s3.fire.effect == NULL) {
                // эффект скончался.
                deb->ttl = 1;
                continue;
            }
        }

        D3DXVECTOR3 *pos = (deb->type == DEB_FIRE) ? &((CMatrixEffectFire *)deb->u1.s3.fire.effect)->m_Pos : &deb->u1.s1.pos;

        deb->v.z -= 1.1f * dtime;

        *pos += deb->v * dtime;

        float z = g_MatrixMap->GetZ(pos->x, pos->y);
        if (pos->z < WATER_LEVEL) {
            if (z <= WATER_LEVEL) {
                // in water
                deb->ttl = 1;
                CMatrixEffect::CreateKonusSplash(
                        *pos, D3DXVECTOR3(0, 0, 1), 10, 5, FSRND(M_PI), 1000, true,
                        (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_SPLASH));
                continue;
            }
        }

        if (z > pos->z) {
            float vl = D3DXVec3Length(&deb->v);
            D3DXVECTOR3 n;
            g_MatrixMap->GetNormal(&n, pos->x, pos->y);

            deb->v += n * vl;
            deb->v *= 0.7f;

            SMatrixMapUnit *mu = g_MatrixMap->UnitGetTest(Float2Int(pos->x * INVERT(GLOBAL_SCALE)),
                                                          Float2Int(pos->y * INVERT(GLOBAL_SCALE)));
            if (mu == NULL || mu->m_Base != NULL) {
                deb->ttl = 1;
            }
        }

        if (deb->type == DEB_FIRE) {
            ASSERT(deb->u1.s3.light.effect);
            ((CMatrixEffectPointLight *)deb->u1.s3.light.effect)->SetPos(*pos);
        }
        else if (deb->type == DEB_SPARK) {
            float k = deb->ttl * deb->u1.s1.u2.s5.unttl;
            float len = 5 * k * k + 1;

            D3DXVECTOR3 delta(*pos - deb->u1.s1.u2.s5.prepos);
            float x = D3DXVec3Length(&delta);
            if (x > len) {
                deb->u1.s1.u2.s5.prepos += delta * (x - len) / x;
            }
            deb->u1.s1.u2.s5.bline->SetPos(deb->u1.s1.u2.s5.prepos, *pos);
        }
    }
}
