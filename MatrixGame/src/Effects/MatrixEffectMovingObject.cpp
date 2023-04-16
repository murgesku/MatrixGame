// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"
#include "../MatrixRenderPipeline.hpp"
#include <math.h>
#include "../MatrixFlyer.hpp"

#include "MatrixEffectMovingObject.hpp"
#include "MatrixEffectShleif.hpp"
#include "MatrixEffectPointLight.hpp"
#include "MatrixEffectExplosion.hpp"

void UnloadObject(CVectorObjectAnim *o, CHeap *heap);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CMatrixEffectMovingObject::CMatrixEffectMovingObject(const SMOProps &props, DWORD hitmask, CMatrixMapStatic *skip,
                                                     FIRE_END_HANDLER handler, DWORD user)
  : CMatrixEffect(), m_Props(props) {
    DTRACE();

    m_EffectType = EFFECT_MOVING_OBJECT;

    m_Props.time = 0;
    m_Props.endoflife = false;
    auto tmp = m_Props.target - m_Props.startpos;
    m_Props.distance = D3DXVec3Length(&tmp);
    // m_Props.curpos = m_Props.startpos;
    m_Props.endhandler = handler;
    m_Props.uservalue = user;
    m_Props.hitmask = hitmask;
    m_Props.skip = skip;

    memset(&m_Props.common, 0, sizeof(m_Props.common));
    if (props.handler == MO_Bomb_Takt) {
        m_Props.common.bomb.pos = props.common.bomb.pos;
        m_Props.common.bomb.trajectory = props.common.bomb.trajectory;
    }
    if (props.handler == MO_Gun_Takt || props.handler == MO_Gun_cannon_Takt) {
        m_Props.common.gun.maxdist = props.common.gun.maxdist;
    }

    if (m_Props.handler)
        m_Props.handler(m_Mat, m_Props, 0);
}

CMatrixEffectMovingObject::~CMatrixEffectMovingObject() {
    if (m_Props.object)
        UnloadObject(m_Props.object, m_Heap);
    if (m_Props.endhandler) {
        auto tmp = m_Props.curpos - m_Props.startpos;
        m_Dist2 = D3DXVec3LengthSq(&tmp);
        m_Props.endhandler(TRACE_STOP_NONE, m_Props.curpos, m_Props.uservalue, FEHF_LASTHIT);
    }

    // if (m_Props.attacker) m_Props.attacker->Release();

    if (m_Props.handler == MO_Homing_Missile_Takt) {
        if (m_Props.common.hm.target != NULL) {
            m_Props.common.hm.target->Release();
        }
    }
    else if (m_Props.handler == MO_Bomb_Takt) {
        if (m_Props.common.bomb.trajectory != NULL) {
            HDelete(CTrajectory, m_Props.common.bomb.trajectory, g_MatrixHeap);
        }
    }

    if (m_Props.shleif) {
        if (m_Props.shleif->effect)
            m_Props.shleif->Unconnect();
        HFree(m_Props.shleif, m_Heap);
    }
}

void CMatrixEffectMovingObject::BeforeDraw(void) {
#ifdef _DEBUG
    if (m_Props.object == NULL)
        debugbreak();
#endif
    m_Props.object->BeforeDraw();
}
void CMatrixEffectMovingObject::Draw(void) {
    DTRACE();

#ifdef _DEBUG
    if (m_Props.object == NULL)
        debugbreak();
#endif

    CVectorObject::DrawBegin();
    g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, g_MatrixMap->m_AmbientColorObj));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, TRUE));

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m_Mat));

    m_Props.object->Draw(0);

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, FALSE));
    CVectorObject::DrawEnd();
}
void CMatrixEffectMovingObject::Takt(float step) {
    DTRACE();
    m_Props.object->Takt(Float2Int(step));
    m_Props.time += step;
    m_Props.handler(m_Mat, m_Props, step);
    if (m_Props.endoflife) {
        UnloadObject(m_Props.object, m_Heap);
        m_Props.object = NULL;
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        g_MatrixMap->SubEffect(this);
#endif
    }
}
void CMatrixEffectMovingObject::Release(void) {
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectMovingObject, this, m_Heap);
}

struct HMData {
    D3DXVECTOR3 dir;
    D3DXVECTOR3 to_tgt;
    // CMatrixMapStatic *target;
    float maxcos;
    SMOProps *props;
    bool found;
};

static bool HMEnum(const D3DXVECTOR3 &fpos, CMatrixMapStatic *ms, DWORD user) {
    D3DXVECTOR3 to_dir;
    const D3DXVECTOR3 *p;
    bool selectnew;
    HMData *hmd = (HMData *)user;

    // int side = 0;
    // if (hmd->props->attacker && hmd->props->attacker->m_Object) side = hmd->props->attacker->m_Object->GetSide();

    if (hmd->props->side == ms->GetSide())
        return true;  // no need friendly object
    if (hmd->props->common.hm.target != NULL &&
        ((hmd->props->common.hm.target->m_Type == OBJECT_TYPE_FLYER) ||
         (hmd->props->common.hm.target->m_Type == OBJECT_TYPE_ROBOTAI) ||
         (hmd->props->common.hm.target->m_Type == OBJECT_TYPE_CANNON)) &&
        (ms->GetObjectType() == OBJECT_TYPE_BUILDING)) {
        return true;  // no need building if robot located
    }

    selectnew = (hmd->props->common.hm.target != NULL) &&
                (hmd->props->common.hm.target->m_Type == OBJECT_TYPE_BUILDING) &&
                ((ms->GetObjectType() == OBJECT_TYPE_FLYER) || (ms->GetObjectType() == OBJECT_TYPE_ROBOTAI) ||
                 (ms->GetObjectType() == OBJECT_TYPE_CANNON));

    p = &ms->GetGeoCenter();

    auto tmp = *p - hmd->props->curpos;
    float cc = D3DXVec3Dot(&hmd->dir, D3DXVec3Normalize(&to_dir, &tmp));

    SObjectCore *oc = ms->GetCore(DEBUG_CALL_INFO);

    if ((cc > 0.98480775301220806f) && ((cc > hmd->maxcos) || selectnew || (oc == hmd->props->common.hm.target))) {
        CMatrixMapStatic *t = g_MatrixMap->Trace(NULL, hmd->props->curpos, *p + (*p - hmd->props->curpos) * 0.1f,
                                                 TRACE_ALL, hmd->props->skip);
        if ((t == TRACE_STOP_NONE) || (t == ms)) {
            if (hmd->props->common.hm.target != NULL)
                hmd->props->common.hm.target->Release();
            hmd->props->common.hm.target = oc;
            oc->RefInc();

            hmd->to_tgt = to_dir;
            hmd->maxcos = cc;
            hmd->found = true;
        }
    }
    oc->Release();
    return true;
}

static bool MOEnum(const D3DXVECTOR3 &center, CMatrixMapStatic *ms, DWORD user) {
    SMOProps *props = (SMOProps *)user;
    if (props->endhandler) {
        auto tmp = center - props->startpos;
        CMatrixEffect::m_Dist2 = D3DXVec3LengthSq(&tmp);
        props->endhandler(ms, center, props->uservalue, 0);
    }
    return true;
}

void MO_Homing_Missile_Takt(D3DXMATRIX &m, SMOProps &props, float takt) {
    DTRACE();

    float dtime = 0.1f * float(takt);

    // calc target

    HMData data;

    D3DXVec3Normalize(&data.dir, &props.velocity);
    D3DXVECTOR3 seekcenter;
    if (props.common.hm.target) {
        auto tmp = props.target - props.curpos;
        data.maxcos = D3DXVec3Dot(&data.dir, D3DXVec3Normalize(&seekcenter, &tmp));
        // seekcenter = props.target;
    }
    else {
        data.maxcos = 0;
    }
    seekcenter = (props.curpos + data.dir * HOMING_RADIUS);
    D3DXVECTOR2 vmin(seekcenter.x - HOMING_RADIUS, seekcenter.y - HOMING_RADIUS),
            vmax(seekcenter.x + HOMING_RADIUS, seekcenter.y + HOMING_RADIUS);

    data.to_tgt = data.dir;

    if (props.time > props.common.hm.next_seek_time) {
        props.common.hm.next_seek_time = HM_SEEK_TIME_PERIOD + props.time;
    }
    else {
        goto skip_obj_seek;
    }

    data.props = &props;
    data.found = false;

    g_MatrixMap->FindObjects(seekcenter, HOMING_RADIUS, 1, TRACE_BUILDING | TRACE_ROBOT | TRACE_CANNON | TRACE_FLYER,
                             props.skip, HMEnum, (DWORD)&data);

    if (data.found == false) {
        if (props.common.hm.target)
            props.common.hm.target->Release();
        props.common.hm.target = NULL;
    }
    else if (props.common.hm.target != NULL) {
        props.target = props.common.hm.target->m_GeoCenter;
    }

skip_obj_seek:

    props.velocity += data.to_tgt * 0.1f * dtime;

    float vel = D3DXVec3Length(&props.velocity);
    float k = float(1.0 / vel);
    if (vel > 1000)
        vel *= k * 1000;

    D3DXVECTOR3 newpos = props.curpos + props.velocity * dtime;

    if (g_MatrixMap->m_Camera.IsInFrustum(newpos)) {
        if (props.common.hm.in_frustum_count <= 0) {
            props.common.hm.next_fire_time = props.time;
        }
        props.common.hm.in_frustum_count = 10;
    }
    else {
        --props.common.hm.in_frustum_count;
    }

    if (props.common.hm.in_frustum_count > 0) {
        float t, dt;
        if (props.time > props.common.hm.next_fire_time) {
            t = 0;
            dt = MISSILE_FIRE_PERIOD / float(props.time - props.common.hm.next_fire_time);
            while (props.time > props.common.hm.next_fire_time) {
                props.common.hm.next_fire_time += MISSILE_FIRE_PERIOD;

                D3DXVECTOR3 p = (newpos - props.curpos) * t + props.curpos;
                t += dt;

                if (props.time < 200) {
                    if (props.shleif->effect) {
                        ((CMatrixEffectShleif *)props.shleif->effect)->AddSmoke(p, 300, 300, 400, 0xFFFFFFFF, true, 0);
                    }
                }
                else {
                    if (props.shleif->effect) {
                        ((CMatrixEffectShleif *)props.shleif->effect)->AddSmoke(p, 300, 600, 800, 0xFFFFFFFF, true, 0);

                        ((CMatrixEffectShleif *)props.shleif->effect)->AddFire(p, 300, 300, 800, 1, true, 0);
                    }
                }
            }
        }
    }

    D3DXVECTOR3 hitpos = newpos;
    bool hit = false;
    CMatrixMapStatic *hito = g_MatrixMap->Trace(&hitpos, props.curpos, newpos, props.hitmask, props.skip);
    if (hito != TRACE_STOP_NONE) {
        hit = true;
    }

    props.curpos = hitpos;

    VecToMatrixY(m, props.curpos, props.velocity * k);

    auto tmp = props.curpos - props.target;
    float distance = D3DXVec3LengthSq(&tmp);
    if (hit || distance < MISSILE_IMPACT_RADIUS_SQ) {
        if (hito == TRACE_STOP_NONE && props.common.hm.target != NULL && props.common.hm.target->m_Object != NULL) {
            hito = props.common.hm.target->m_Object;
        }

        // seek objects

        hit |= g_MatrixMap->FindObjects(props.curpos, MISSILE_IMPACT_RADIUS, 1, props.hitmask, props.skip, MOEnum,
                                        (DWORD)&props);

        if (hit) {
            bool fire = false;
            if (hito == TRACE_STOP_LANDSCAPE) {
                CMatrixEffect::CreateLandscapeSpot(NULL, D3DXVECTOR2(hitpos.x, hitpos.y), FSRND(M_PI), FRND(3) + 6,
                                                   SPOT_VORONKA);
                hitpos.z = g_MatrixMap->GetZ(hitpos.x, hitpos.y) + 10;
                fire = true;
            }
            CMatrixEffect::CreateExplosion(hitpos, ExplosionMissile, fire);
        }

        // impact
        props.endoflife = true;
        if (props.endhandler) {
            auto tmp = props.curpos - props.startpos;
            CMatrixEffect::m_Dist2 = D3DXVec3LengthSq(&tmp);
            props.endhandler(TRACE_STOP_NONE, props.curpos, props.uservalue, FEHF_LASTHIT);
            props.endhandler = NULL;
        }
        if (props.common.hm.target) {
            props.common.hm.target->Release();
            props.common.hm.target = NULL;
        }
    }
    else if (props.time > 10000) {
        // time to die...
        props.endoflife = true;
        if (props.endhandler) {
            auto tmp = props.curpos - props.startpos;
            CMatrixEffect::m_Dist2 = D3DXVec3LengthSq(&tmp);
            props.endhandler(hito, hit ? hitpos : props.curpos, props.uservalue, FEHF_LASTHIT);
            props.endhandler = NULL;
        }
        if (props.common.hm.target) {
            props.common.hm.target->Release();
            props.common.hm.target = NULL;
        }
    }
}

void MO_Bomb_Takt(D3DXMATRIX &m, SMOProps &props, float takt) {
    DTRACE();

    float dtime = props.velocity.x * float(takt);

    // D3DXVECTOR3 to_tgt(props.target - props.curpos);
    // props.velocity.z -= 0.035f * dtime;
    // float vel = D3DXVec3Length(&props.velocity);

    bool hit = false;

    // D3DXVECTOR3 newpos = props.curpos + props.velocity * dtime;

    props.common.bomb.pos += dtime;

    D3DXVECTOR3 newpos, dir;
    props.common.bomb.trajectory->CalcPoint(newpos, props.common.bomb.pos);

    auto tmp = newpos - props.curpos;
    D3DXVec3Normalize(&dir, &tmp);

    float t, dt;
    if (props.time > props.common.bomb.next_fire_time) {
        t = 0;
        dt = BOMB_FIRE_PERIOD / float(props.time - props.common.bomb.next_fire_time);
        while (props.time > props.common.bomb.next_fire_time) {
            props.common.bomb.next_fire_time += BOMB_FIRE_PERIOD;

            D3DXVECTOR3 p = (newpos - props.curpos) * t + props.curpos;
            t += dt;

            if (props.time < 200) {
                if (props.shleif->effect) {
                    ((CMatrixEffectShleif *)props.shleif->effect)->AddSmoke(p, 300, 300, 300, 0xFFFFFFFF, true, 0);
                }
            }
            else {
                // CMatrixEffect::CreateFire(p, 300, 300, 300, 1, true, 0);
                if (props.shleif->effect) {
                    ((CMatrixEffectShleif *)props.shleif->effect)->AddSmoke(p, 300, 600, 400, 0xFFFFFFFF, true, 0);
                }
            }
        }
    }

    // CMatrixMapStatic *target = NULL;

    if ((newpos.z - g_MatrixMap->GetZ(newpos.x, newpos.y)) < (BOMB_DAMAGE_RADIUS + 10)) {
        hit = g_MatrixMap->FindObjects(newpos, BOMB_DAMAGE_RADIUS, 1, props.hitmask, props.skip, MOEnum, (DWORD)&props);
    }
    D3DXVECTOR3 hitpos = newpos;
    // DDVECT("bla0", hitpos);
    CMatrixMapStatic *hito = g_MatrixMap->Trace(&hitpos, props.curpos, newpos, props.hitmask, props.skip);
    // DDVECT("bla1", hitpos);

    if (hito != TRACE_STOP_NONE) {
        hit = true;
    }

    // check if bomb is on last trajectory point, then explode if true
    D3DXVECTOR3 last;
    props.common.bomb.trajectory->CalcPoint(last, 1.0);

    if (IsVec3Equal(last, newpos, 0.001f)) {
        hit = true;
    }

    props.curpos = newpos;

    VecToMatrixY(m, props.curpos, dir);

    if (hit) {
        // if (hito == TRACE_STOP_NONE && target != NULL) hito = target;

        if (hito != TRACE_STOP_WATER) {
            bool fire = false;
            if (hito == TRACE_STOP_LANDSCAPE) {
                CMatrixEffect::CreateLandscapeSpot(NULL, D3DXVECTOR2(hitpos.x, hitpos.y), FSRND(M_PI), FRND(3) + 6,
                                                   SPOT_VORONKA);
                hitpos.z = g_MatrixMap->GetZ(hitpos.x, hitpos.y) + 10;
                fire = true;
            }
            CMatrixEffect::CreateExplosion(hitpos, ExplosionMissile, fire);
        }
        else {
            float z = g_MatrixMap->GetZ(hitpos.x, hitpos.y);
            if (z > WATER_LEVEL) {
                CMatrixEffect::CreateLandscapeSpot(NULL, D3DXVECTOR2(hitpos.x, hitpos.y), FSRND(M_PI), FRND(3) + 6,
                                                   SPOT_VORONKA);
                hitpos.z = z + 10;
                CMatrixEffect::CreateExplosion(hitpos, ExplosionMissile, true);
            }
        }

        // impact
        props.endoflife = true;
        if (props.endhandler) {
            auto tmp = hitpos - props.startpos;
            CMatrixEffect::m_Dist2 = D3DXVec3LengthSq(&tmp);
            props.endhandler(hito, hitpos, props.uservalue, FEHF_LASTHIT);
            props.endhandler = NULL;
        }
    }
}

void MO_Gun_Takt(D3DXMATRIX &m, SMOProps &props, float takt) {
    DTRACE();

    float dtime = 0.1f * float(takt);

    D3DXVECTOR3 to_tgt(props.target - props.curpos);

    float vel = D3DXVec3Length(&props.velocity);
    D3DXVECTOR3 dir(props.velocity * (1.0f / vel));

    bool hit = false;

    D3DXVECTOR3 newpos = props.curpos + props.velocity * dtime;

    props.common.gun.dist += vel * dtime;

    if ((newpos.z - g_MatrixMap->GetZ(newpos.x, newpos.y)) < (GUN_DAMAGE_RADIUS + 10)) {
        hit = g_MatrixMap->FindObjects(newpos, GUN_DAMAGE_RADIUS, 1, props.hitmask, props.skip, MOEnum, (DWORD)&props);
    }

    D3DXVECTOR3 hitpos = newpos;
    CMatrixMapStatic *hito = g_MatrixMap->Trace(&hitpos, props.curpos, newpos, props.hitmask, props.skip);
    if (hito != TRACE_STOP_NONE) {
        hit = true;
    }

    if (g_MatrixMap->m_Camera.IsInFrustum(newpos)) {
        props.common.gun.in_frustum_count = 100;
    }
    else {
        --props.common.gun.in_frustum_count;
    }

    if (props.common.gun.in_frustum_count > 0) {
        CMatrixEffect::CreateBillboardLine(NULL, props.curpos, hitpos, 6, 0x8FFFFFFF, 0, 1000,
                                           CMatrixEffect::GetBBTexI(BBT_SHLEIF));
        // DM("cc", CStr(props.curpos.x)  + ","+CStr(props.curpos.y) + ","+ CStr(props.curpos.z) + ","+ CStr(hitpos.x) +
        // ","+ CStr(hitpos.x) + ","+  CStr(hitpos.x));
    }

    props.curpos = hitpos;

    VecToMatrixY(m, props.curpos, dir);

    if (hit) {
        // if (hito == TRACE_STOP_NONE && target != NULL) hito = target;
        if (hito != TRACE_STOP_WATER) {
            bool fire = false;
            if (hito == TRACE_STOP_LANDSCAPE) {
                CMatrixEffect::CreateLandscapeSpot(NULL, D3DXVECTOR2(hitpos.x, hitpos.y), FSRND(M_PI), FRND(3) + 6,
                                                   SPOT_VORONKA);
                hitpos.z = g_MatrixMap->GetZ(hitpos.x, hitpos.y) + 10;
                fire = true;
            }
            CMatrixEffect::CreateExplosion(hitpos, ExplosionMissile, fire);
        }

        // impact
        props.endoflife = true;
        if (props.endhandler) {
            auto tmp = hitpos - props.startpos;
            CMatrixEffect::m_Dist2 = D3DXVec3LengthSq(&tmp);
            props.endhandler(hito, hitpos, props.uservalue, FEHF_LASTHIT);
            props.endhandler = NULL;
        }
    }
    else if (props.common.gun.dist > props.common.gun.maxdist) {
        props.endoflife = true;
        if (props.endhandler) {
            auto tmp = hitpos - props.startpos;
            CMatrixEffect::m_Dist2 = D3DXVec3LengthSq(&tmp);
            props.endhandler(TRACE_STOP_NONE, hitpos, props.uservalue, FEHF_LASTHIT);
            props.endhandler = NULL;
        }
    }
}

void MO_Gun_cannon_Takt(D3DXMATRIX &m, SMOProps &props, float takt) {
    DTRACE();

    float dtime = 0.1f * float(takt);

    D3DXVECTOR3 to_tgt(props.target - props.curpos);

    float vel = D3DXVec3Length(&props.velocity);
    D3DXVECTOR3 dir(props.velocity * (1.0f / vel));

    bool hit = false;

    D3DXVECTOR3 newpos = props.curpos + props.velocity * dtime;

    props.common.gun.dist += vel * dtime;

    if ((newpos.z - g_MatrixMap->GetZ(newpos.x, newpos.y)) < (GUN_DAMAGE_RADIUS + 10)) {
        hit = g_MatrixMap->FindObjects(newpos, GUN_DAMAGE_RADIUS, 1, props.hitmask, props.skip, MOEnum, (DWORD)&props);
    }

    D3DXVECTOR3 hitpos = newpos;
    CMatrixMapStatic *hito = g_MatrixMap->Trace(&hitpos, props.curpos, newpos, props.hitmask, props.skip);
    if (hito != TRACE_STOP_NONE) {
        hit = true;
    }

    if (g_MatrixMap->m_Camera.IsInFrustum(newpos)) {
        props.common.gun.in_frustum_count = 100;
    }
    else {
        --props.common.gun.in_frustum_count;
    }

    if (props.common.gun.in_frustum_count > 0) {
        CMatrixEffect::CreateBillboardLine(NULL, props.curpos, hitpos, 4, 0x6FFFFFFF, 0, 500,
                                           CMatrixEffect::GetBBTexI(BBT_SHLEIF));
        // DM("cc", CStr(props.curpos.x)  + ","+CStr(props.curpos.y) + ","+ CStr(props.curpos.z) + ","+ CStr(hitpos.x) +
        // ","+ CStr(hitpos.x) + ","+  CStr(hitpos.x));
    }

    props.curpos = hitpos;

    VecToMatrixY(m, props.curpos, dir);

    if (hit) {
        // if (hito == TRACE_STOP_NONE && target != NULL) hito = target;
        if (hito != TRACE_STOP_WATER) {
            bool fire = false;
            if (hito == TRACE_STOP_LANDSCAPE) {
                CMatrixEffect::CreateLandscapeSpot(NULL, D3DXVECTOR2(hitpos.x, hitpos.y), FSRND(M_PI), FRND(3) + 6,
                                                   SPOT_VORONKA);
                hitpos.z = g_MatrixMap->GetZ(hitpos.x, hitpos.y) + 10;
                fire = true;
            }
            CMatrixEffect::CreateExplosion(hitpos, ExplosionMissile, fire);
        }

        // impact
        props.endoflife = true;
        if (props.endhandler) {
            auto tmp = hitpos - props.startpos;
            CMatrixEffect::m_Dist2 = D3DXVec3LengthSq(&tmp);
            props.endhandler(hito, hitpos, props.uservalue, FEHF_LASTHIT);
            props.endhandler = NULL;
        }
    }
    else if (props.common.gun.dist > props.common.gun.maxdist) {
        props.endoflife = true;
        if (props.endhandler) {
            auto tmp = hitpos - props.startpos;
            CMatrixEffect::m_Dist2 = D3DXVec3LengthSq(&tmp);
            props.endhandler(TRACE_STOP_NONE, hitpos, props.uservalue, FEHF_LASTHIT);
            props.endhandler = NULL;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMatrixEffectBuoy::Kill(void) {
    m_Kill = true;
    if (m_Light.effect) {
        ((CMatrixEffectPointLight *)m_Light.effect)->Kill(1100);
        m_Light.Unconnect();
    }
}

void CMatrixEffectBuoy::Takt(float step) {
    DTRACE();

    m_Props.object->Takt(Float2Int(step));
    m_Props.time += step;

    D3DXMATRIX mr, mr1;

    m_Props.common.buoy.angle += float(step * 0.001);
    if (m_Props.common.buoy.angle > M_PI_MUL(2))
        m_Props.common.buoy.angle -= M_PI_MUL(2);
    D3DXMatrixRotationYawPitchRoll(&mr, m_Props.startpos.x, m_Props.startpos.y,
                                   m_Props.startpos.z + m_Props.common.buoy.angle);

    D3DXMatrixScaling(&m_Mat, m_Props.velocity.x, m_Props.velocity.y, m_Props.velocity.z);

    m_Mat *= mr;

    m_Mat._41 = m_Props.curpos.x;
    m_Mat._42 = m_Props.curpos.y;
    // m_Mat._43 = m_Z + sin(m_Props.any0f) * 20;
    m_Mat._43 = m_Props.curpos.z;

    float mul = (float)pow(0.995, double(step));

    m_Props.velocity.x = (m_Props.velocity.x - 1) * mul + 1;
    m_Props.velocity.y = (m_Props.velocity.y - 1) * mul + 1;
    m_Props.velocity.z = (m_Props.velocity.z - 1) * mul + 1;

    m_Props.startpos *= mul;

    // CDText::T("mul", mul);

    if (IRND(7000) < step) {
        ((float *)&m_Props.velocity)[IRND(3)] *= 2;

        CMatrixEffect::CreateBillboard(NULL, m_Props.curpos, 1, 30, 0xFF000000 | m_BuoyColor, m_BuoyColor, 1000, 0,
                                       TEXTURE_PATH_WAVE, D3DXVECTOR3(1, 0, 0));
        CMatrixEffect::CreateBillboard(NULL, m_Props.curpos, 1, 30, 0xFF000000 | m_BuoyColor, m_BuoyColor, 1000, 300,
                                       TEXTURE_PATH_WAVE, D3DXVECTOR3(1, 0, 0));
        CMatrixEffect::CreateBillboard(NULL, m_Props.curpos, 1, 30, 0xFF000000 | m_BuoyColor, m_BuoyColor, 1000, 600,
                                       TEXTURE_PATH_WAVE, D3DXVECTOR3(1, 0, 0));
    }
    // if (IRND(7000) < takt)
    {
        //((float *)&m_Props.startpos)[IRND(3)] = M_PI_MUL(2);
    }

    if (m_Kill) {
        m_KillTime -= step;
        if (m_KillTime < 0)
            goto endoflife;
    }
    if (m_Props.endoflife) {
    endoflife:
        UnloadObject(m_Props.object, m_Heap);
        m_Props.object = NULL;

#ifdef _DEBUG
        m_Light.Release(DEBUG_CALL_INFO);
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        m_Light.Release();
        g_MatrixMap->SubEffect(this);
#endif
        return;
    }

    DWORD lcol = LIC(0, m_BuoyColor, 0.2f);
    lcol = LIC(0, lcol, m_Props.velocity.x);

    if (m_Light.effect) {
        ((CMatrixEffectPointLight *)m_Light.effect)->SetColor(lcol);
    }
    else {
        CMatrixEffect::CreatePointLight(&m_Light, *(D3DXVECTOR3 *)&m_Mat._41, 30, lcol, true);
    }
}

void BuoySetupTex(SVOSurface *vo, DWORD user_param, int) {
    //    ASSERT_DX(g_D3DD->SetTexture(0, vo->m_Tex->Tex()));
    return;
}

bool BuoySetupStages(DWORD user_param, int) {
    DTRACE();

    ASSERT_DX(g_D3DD->SetFVF(VO_FVF));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));

    // g_D3DD->SetRenderState(D3DRS_DESTBLEND,  D3DBLEND_ONE  );

    ASSERT_DX(g_D3DD->SetTexture(1, g_MatrixMap->GetReflectionTexture()->Tex()));
    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR));

    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);

    SetColorOpAnyOrder(1, D3DTOP_ADD, D3DTA_TEXTURE, D3DTA_CURRENT);
    SetAlphaOpSelect(1, D3DTA_CURRENT);

    SetColorOpDisable(2);

    return false;
}

void CMatrixEffectBuoy::Draw(void) {
    DTRACE();

    BYTE a = 255;

    if (m_Kill) {
        a = BYTE((m_KillTime / 1000.0f) * 255.0f);
    }

    g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFF | (a << 24));

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m_Mat));

    m_Props.object->Draw(0);

    // ASSERT_DX(g_D3DD->SetRenderState( D3DRS_ALPHABLENDENABLE,   FALSE ));
    // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW ));
    // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE,	TRUE));

    // g_D3DD->SetRenderState(D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA  );
    ASSERT_DX(g_D3DD->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
}
