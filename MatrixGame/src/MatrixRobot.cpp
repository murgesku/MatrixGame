// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>
#include <algorithm>

#include "MatrixRobot.hpp"
#include "MatrixObjectBuilding.hpp"
#include "Logic/MatrixRule.h"
#include "MatrixObjectCannon.hpp"
#include "MatrixFlyer.hpp"
#include "Interface/CInterface.h"

#include "Effects/MatrixEffectShleif.hpp"
#include "Effects/MatrixEffectElevatorField.hpp"
#include "Effects/MatrixEffectSelection.hpp"
#include "Effects/MatrixEffectExplosion.hpp"

void SWeaponRepairData::Release(void) {
    m_b0.Release();
    m_b1.Release();
    m_bl.Release();
    HFree(this, g_MatrixHeap);
}

void SWeaponRepairData::Draw(bool now) {
    if (!FLAG(m_Flags, CAN_BE_DRAWN))
        return;

    BYTE a = g_MatrixMap->IsPaused() ? 240 : (BYTE(FRND(128) + 128));

    m_bl.SetAlpha(a);
    m_b0.SetAlpha(a);
    m_b1.SetAlpha(a);

    D3DXVECTOR3 p0, p1;
    D3DXVECTOR3 tocam;
    tocam = (now ? g_MatrixMap->m_Camera.GetDrawNowFC() : g_MatrixMap->m_Camera.GetFrustumCenter()) - m_pos0;
    D3DXVec3Normalize(&tocam, &tocam);
    p0 = m_pos0 + tocam * 3;

    tocam = (now ? g_MatrixMap->m_Camera.GetDrawNowFC() : g_MatrixMap->m_Camera.GetFrustumCenter()) - m_pos1;
    D3DXVec3Normalize(&tocam, &tocam);
    p1 = m_pos1 + tocam * 3;

    m_bl.SetPos(p0, p1);
    m_b0.SetPos(p0);
    m_b1.SetPos(p1);

    if (now) {
        CVectorObject::DrawEnd();

        m_bl.DrawNow(g_MatrixMap->m_Camera.GetDrawNowFC());
        m_b0.DrawNow(g_MatrixMap->m_Camera.GetDrawNowIView());
        m_b1.DrawNow(g_MatrixMap->m_Camera.GetDrawNowIView());
    }
    else {
        m_bl.AddToDrawQueue();
        m_b0.Sort(g_MatrixMap->m_Camera.GetViewInversedMatrix());
        m_b1.Sort(g_MatrixMap->m_Camera.GetViewInversedMatrix());
    }
}

void SWeaponRepairData::Update(SMatrixRobotUnit *unit) {
    const D3DXMATRIX *m0 = unit->m_Graph->GetMatrixById(1);
    const D3DXMATRIX *m1 = unit->m_Graph->GetMatrixById(2);

    D3DXVec3TransformCoord(&m_pos0, (D3DXVECTOR3 *)&m0->_41, &unit->m_Matrix);
    D3DXVec3TransformCoord(&m_pos1, (D3DXVECTOR3 *)&m1->_41, &unit->m_Matrix);

    SETFLAG(m_Flags, SWeaponRepairData::CAN_BE_DRAWN);
}

SWeaponRepairData *SWeaponRepairData::Allocate(void) {
    SWeaponRepairData *r = (SWeaponRepairData *)HAlloc(sizeof(SWeaponRepairData), g_MatrixHeap);

    new(&r->m_b0) CBillboard(TRACE_PARAM_CALL D3DXVECTOR3(-1000, -1000, -1000), 5, 0, 0xFFFFFFFF,
                                   CMatrixEffect::GetBBTexI(BBT_REPGLOWEND));
    new(&r->m_b1) CBillboard(TRACE_PARAM_CALL D3DXVECTOR3(-1000, -1000, -1000), 10, 0, 0xFFFFFFFF,
                                   CMatrixEffect::GetBBTexI(BBT_REPGLOWEND));
    new(&r->m_bl) CBillboardLine(TRACE_PARAM_CALL D3DXVECTOR3(-1000, -1000, -1000),
                                           D3DXVECTOR3(-1000, -1000, -1000), 10, 0xFFFFFFFF,
                                           CMatrixEffect::GetBBTexI(BBT_REPGLOW));
    RESETFLAG(r->m_Flags, SWeaponRepairData::CAN_BE_DRAWN);
    return r;
}

void SBotWeapon::PrepareRepair(void) {
    if (m_Unit) {
        ASSERT(m_Unit->u1.s1.m_WeaponRepairData == NULL);
        m_Unit->u1.s1.m_WeaponRepairData = SWeaponRepairData::Allocate();
    }
}

CMatrixRobotAI::CMatrixRobotAI()
  : CMatrixRobot()
#ifdef _DEBUG
    ,
    m_Ablaze(DEBUG_CALL_INFO)
#endif
{
    m_ColsWeight = 0;
    m_ColsWeight2 = 0;

    m_HaveRepair = 0;

    m_Strength = 0;

    m_GroupSpeed = 0.0f;
    m_ColSpeed = 100.0f;

    m_MapX = 0;
    m_MapY = 0;

    m_ZoneCur = -1;

    //	m_LowOrder=0;

    m_DesX = 0;
    m_DesY = 0;

    m_ZoneDes = -1;
    m_ZonePathNext = -1;
    m_ZonePathCnt = 0;
    m_ZonePath = NULL;

    m_MovePathCnt = 0;
    m_MovePathCur = 0;

    m_defHitPoint = Float2Int(m_HitPoint);
    m_nLastCollideFrame = 0;

    ZeroMemory(m_Weapons, sizeof(m_Weapons));
    m_WeaponsCnt = 0;

    m_WeaponDir = D3DXVECTOR3(0, 0, 0);
    m_OrdersInPool = 0;
    m_CaptureCandidatesCnt = 0;
    //    m_GatherPeriod          = 0;

    // m_FireTarget            = NULL;

    m_Group = 0;
    m_Team = -1;
    m_GroupLogic = -1;

    m_MaxFireDist = 0;
    m_MinFireDist = 0;
    m_RepairDist = 0.0f;

    m_CollAvoid = D3DXVECTOR3(0, 0, 0);

    m_Cols = 0;

    m_Selection = NULL;
    m_SyncMul = 0;

    m_BigTexture = NULL;
    m_MedTexture = NULL;
#ifdef USE_SMALL_TEXTURE_IN_ROBOT_ICON
    m_SmallTexture = NULL;
#endif

    m_CtrlGroup = 0;

    m_SoundChassis = SOUND_ID_EMPTY;

    m_PrevTurnSign = 1;
    m_HullRotCnt = 0;

    m_LastDelayDamageSide = 0;
}

CMatrixRobotAI::~CMatrixRobotAI() {
    ReleaseMe();
}

void CMatrixRobotAI::DIPTakt(float ms) {
    DTRACE();

    D3DXVECTOR3 *pos;
    D3DXMATRIX *mat;

    bool del = true;
    for (int i = 0; i < m_UnitCnt; ++i) {
        if (m_Unit[i].u1.s2.m_TTL <= 0)
            continue;
        m_Unit[i].u1.s2.m_TTL -= ms;

        pos = &m_Unit[i].u1.s2.m_Pos;

        if (m_Unit[i].u1.s2.m_TTL <= 0) {
            // create explosive

            pos->z += 10;
            CMatrixEffect::CreateExplosion(*pos, ExplosionRobotBoomSmall, true);
            if (m_Unit[i].Smoke().effect) {
                ((CMatrixEffectSmoke *)m_Unit[i].Smoke().effect)->SetTTL(1000);
                m_Unit[i].Smoke().Unconnect();
            }

            continue;
        }

        del = false;

        if (IS_ZERO_VECTOR(m_Unit[i].u1.s2.m_Velocity)) {
            continue;
        }

        D3DXVECTOR3 oldpos = *pos;

        m_Unit[i].u1.s2.m_Velocity.z -= 0.0002f * ms;
        (*pos) += m_Unit[i].u1.s2.m_Velocity * ms;
        mat = &m_Unit[i].m_Matrix;

        D3DXVECTOR3 hitpos;
        CMatrixMapStatic *o = g_MatrixMap->Trace(&hitpos, oldpos, *pos, TRACE_ALL, this);
        if (o == TRACE_STOP_WATER) {
            // in water
            if (g_MatrixMap->GetZ(hitpos.x, hitpos.y) < WATER_LEVEL) {
                m_Unit[i].u1.s2.m_TTL = 0;
                CMatrixEffect::CreateKonusSplash(
                        hitpos, D3DXVECTOR3(0, 0, 1), 10, 5, FSRND(M_PI), 1000, true,
                        (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_SPLASH));
                if (m_Unit[i].Smoke().effect) {
                    ((CMatrixEffectSmoke *)m_Unit[i].Smoke().effect)->SetTTL(1000);
                    m_Unit[i].Smoke().Unconnect();
                }
            }
        }
        else if (o == TRACE_STOP_LANDSCAPE) {
            m_Unit[i].u1.s2.m_Velocity = D3DXVECTOR3(0, 0, 0);
            m_Unit[i].u1.s2.m_Pos = hitpos;

            SMatrixMapUnit *mu = g_MatrixMap->UnitGetTest(Float2Int(hitpos.x * INVERT(GLOBAL_SCALE)),
                                                          Float2Int(hitpos.y * INVERT(GLOBAL_SCALE)));
            if (mu == NULL || mu->m_Base != NULL) {
                m_Unit[i].u1.s2.m_TTL = 0;
            }
        }
        else if (IS_TRACE_STOP_OBJECT(o)) {
            o->Damage(WEAPON_DEBRIS, hitpos, m_Unit[i].u1.s2.m_Velocity, 0, NULL);
            m_Unit[i].u1.s2.m_TTL = 1;
        }

        // if (pos->z < 0) pos->z = 0;

        float time = float(g_MatrixMap->GetTime());

        D3DXMATRIX m0, m1(g_MatrixMap->GetIdentityMatrix()), m2(g_MatrixMap->GetIdentityMatrix());
        const D3DXVECTOR3 &pos1 = m_Unit[i].m_Graph->VO()->GetFrameGeoCenter(m_Unit[i].m_Graph->GetVOFrame());
        *(D3DXVECTOR3 *)&m1._41 = pos1;
        *(D3DXVECTOR3 *)&m2._41 = -pos1;
        D3DXMatrixRotationYawPitchRoll(&m0, m_Unit[i].u1.s2.m_dy * time, m_Unit[i].u1.s2.m_dp * time, m_Unit[i].u1.s2.m_dr * time);
        *mat = m2 * m0 * m1;

        mat->_41 = pos->x;
        mat->_42 = pos->y;
        mat->_43 = pos->z;

        if (m_Unit[i].Smoke().effect) {
            ((CMatrixEffectSmoke *)m_Unit[i].Smoke().effect)->SetPos(*pos);
        }
    }

    if (del) {
        g_MatrixMap->StaticDelete(this);
    }
}

struct RCData {
    D3DXVECTOR3 norm;
    CMatrixMapStatic *nearest;
    float neardist2;
};

static bool CollisionRobots(const D3DXVECTOR3 &center, CMatrixMapStatic *ms, DWORD user) {
    RCData *d = (RCData *)user;

    D3DXVECTOR3 v(center - ms->GetGeoCenter());
    float dist = D3DXVec3LengthSq(&v);
    if (dist < d->neardist2) {
        d->norm = v;
        d->neardist2 = dist;
        d->nearest = ms;
    }
    return true;
}

void CMatrixRobotAI::PauseTakt(int cms) {
    DTRACE();
    m_PB.Modify(100000.0f, 0);
    if (m_CurrState == ROBOT_DIP)
        return;
    if (m_ShowHitpointTime > 0) {
        m_ShowHitpointTime -= cms;
        if (m_ShowHitpointTime < 0)
            m_ShowHitpointTime = 0;
    }
}

void CMatrixRobotAI::LogicTakt(int ms) {
    DTRACE();

    if (0) {
    do_animation:;
        DCP();

        if (fabs(m_Speed) <= 0.01f) {
            SwitchAnimation(ANIMATION_STAY);
        }
        else {
            bool mt = FindOrderLikeThat(ROT_MOVE_TO);
            if (!mt)
                mt = FindOrderLikeThat(ROT_CAPTURE_FACTORY);

            if (mt) {
                SwitchAnimation(ANIMATION_MOVE);
            }
            else if (FindOrderLikeThat(ROT_MOVE_TO_BACK)) {
                SwitchAnimation(ANIMATION_MOVE_BACK);
            }
        }

        if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_PNEUMATIC) {
            DoAnimation(ms);
            LinkPneumatic();  // corrects pneumatic
        }
        return;
    }

    if (IsMustDie() && m_CurrState != ROBOT_DIP) {
        Damage(WEAPON_INSTANT_DEATH, GetGeoCenter(), D3DXVECTOR3(0, 0, 0), 0, NULL);
        return;
    }

    if (GetBase()) {
        m_TimeWithBase += ms;
        if (m_TimeWithBase > 10000) {
            GetBase()->Close();
            m_TimeWithBase = 0;
            Damage(WEAPON_INSTANT_DEATH, GetGeoCenter(), D3DXVECTOR3(0, 0, 0), 0, NULL);
            return;
        }
    }

    if (m_ColsWeight)
        m_ColsWeight = std::max(0, m_ColsWeight - ms);
    if (m_ColsWeight2)
        m_ColsWeight2 = std::max(0, m_ColsWeight2 - ms);
    else
        GetEnv()->m_BadCoordCnt = 0;

    DCP();

    if (!g_MatrixMap->GetPlayerSide()->FindObjectInSelection(this)) {
        UnSelect();
    }

    m_SyncMul = (float)ms / (float)LOGIC_TAKT_PERIOD;
    MoveSelection();

    DCP();

#ifdef _SUB_BUG
    if (this == g_MatrixMap->GetPlayerSide()->GetArcadedObject()) {
        g_MatrixMap->m_DI.T(L"ms", std::wstring(ms));
    }
#endif

    m_PB.Modify(100000.0f, 0);
    if (m_CurrState == ROBOT_DIP) {
        DIPTakt(float(ms));
        return;
    }

    DCP();

    if (m_MiniMapFlashTime > 0) {
        m_MiniMapFlashTime -= ms;
    }

    if (m_ShowHitpointTime > 0) {
        m_ShowHitpointTime -= ms;
        if (m_ShowHitpointTime < 0)
            m_ShowHitpointTime = 0;
    }

    if (IsAblaze()) {
        DCP();

        if (m_Ablaze.effect == NULL) {
            CMatrixEffect::CreateShleif(&m_Ablaze);
        }

        if (m_Ablaze.effect != NULL) {
            //((CMatrixEffectShleif *)m_Ablaze.effect)->SetTTL(1000);

            while (g_MatrixMap->GetTime() > m_NextTimeAblaze) {
                m_NextTimeAblaze += OBJECT_ROBOT_ABLAZE_PERIOD_EFFECT;

                D3DXVECTOR3 dir, pos;
                float t;

                int cnt = 4;
                do {
                    pos.x = m_Core->m_Matrix._41 + FSRND(m_Core->m_Radius);
                    pos.y = m_Core->m_Matrix._42 + FSRND(m_Core->m_Radius);
                    pos.z = m_Core->m_Matrix._43 + FRND(m_Core->m_Radius * 2);
                    auto tmp = D3DXVECTOR3(m_Core->m_Matrix._41 - pos.x,
                                           m_Core->m_Matrix._42 - pos.y,
                                           m_Core->m_Matrix._43 - pos.z);
                    D3DXVec3Normalize(&dir, &tmp);
                }
                while (!PickFull(pos, dir, &t) && (--cnt > 0));

                if (cnt > 0) {
                    ((CMatrixEffectShleif *)m_Ablaze.effect)
                            ->AddFire(pos + dir * (t + 2), 100, 1500, 30, 2.5f, false, 1.0f / 35.0f);
                }

                for (int i = 0; i < OBJECT_ROBOT_ABLAZE_PERIOD_EFFECT; i += OBJECT_ROBOT_ABLAZE_PERIOD)
                    if (Damage(WEAPON_ABLAZE, pos, dir, m_LastDelayDamageSide, NULL))
                        return;
            }
        }
    }
    // else
    //{
    //    if (m_Ablaze.effect != NULL)
    //    {
    //        m_Ablaze.Release();
    //    }

    //}

    DCP();
    if (IsShorted()) {
        DCP();

        while (g_MatrixMap->GetTime() > m_NextTimeShorted) {
            m_NextTimeShorted += OBJECT_SHORTED_PERIOD;

            D3DXVECTOR3 d1, d2, dir, pos;
            float t;

            int cnt = 4;
            do {
                pos.x = m_Core->m_Matrix._41 + FSRND(m_Core->m_Radius);
                pos.y = m_Core->m_Matrix._42 + FSRND(m_Core->m_Radius);
                pos.z = m_Core->m_Matrix._43 + FRND(m_Core->m_Radius * 2);
                auto tmp = D3DXVECTOR3(m_Core->m_Matrix._41 - pos.x,
                                       m_Core->m_Matrix._42 - pos.y,
                                       m_Core->m_Matrix._43 - pos.z);
                D3DXVec3Normalize(&dir, &tmp);
            }
            while (!Pick(pos, dir, &t) && (--cnt > 0));
            if (cnt > 0) {
                d1 = pos + dir * t;
            }
            do {
                pos.x = m_Core->m_Matrix._41 + FSRND(m_Core->m_Radius);
                pos.y = m_Core->m_Matrix._42 + FSRND(m_Core->m_Radius);
                pos.z = m_Core->m_Matrix._43 + FRND(m_Core->m_Radius * 2);
                auto tmp = D3DXVECTOR3(m_Core->m_Matrix._41 - pos.x,
                                       m_Core->m_Matrix._42 - pos.y,
                                       m_Core->m_Matrix._43 - pos.z);
                D3DXVec3Normalize(&dir, &tmp);
            }
            while (!Pick(pos, dir, &t) && (--cnt > 0));
            if (cnt > 0) {
                d2 = pos + dir * t;
                CMatrixEffect::CreateShorted(d1, d2, FRND(400) + 100);
            }
            if (Damage(WEAPON_SHORTED, pos, dir, m_LastDelayDamageSide, NULL))
                return;
        }
    }

    DCP();

    if (IsCrazy()) {
        static int ppx = -1;
        static int ppy = -1;
        static int cnt = 0;
        if (ppx == m_MapX && ppy == m_MapY)
            cnt++;
        else {
            ppx = m_MapX;
            ppy = m_MapY;
        }

        if (cnt > 100 || (PLIsInPlace() && GetEnv()->GetEnemyCnt() <= 0)) {
            cnt = 0;
            ppx = m_MapX;
            ppy = m_MapY;
            // find new target :)

            CMatrixMapStatic *target = NULL;
            CMatrixMapStatic *ms = CMatrixMapStatic::GetFirstLogic();
            for (; ms; ms = ms->GetNextLogic()) {
                // if (ms->IsLiveBuilding() && !ms->IsBase())
                //{
                //    b = ms->AsBuilding();
                //    if (FRND(1) < 0.1f) break;
                //}
                if (ms->IsLiveRobot() && ms != this) {
                    target = ms;
                    if (ms->GetSide() == PLAYER_SIDE)
                        break;
                }
            }
            if (target == NULL) {
                MustDie();
                return;
            }

            SObjectCore *c = target->GetCore(DEBUG_CALL_INFO);
            CPoint pos(int(c->m_GeoCenter.x / GLOBAL_SCALE_MOVE), int(c->m_GeoCenter.y / GLOBAL_SCALE_MOVE));
            c->Release();

            SETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);

            CMatrixSideUnit *su = g_MatrixMap->GetSideById(m_Side);
            su->PGOrderAttack(su->RobotToLogicGroup(this), pos, target);

            RESETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);
        }
    }
    DCP();

    if (m_CurrState == ROBOT_FALLING) {
        DCP();

        float dtime = float(ms) * 0.013f;
        m_FallingSpeed += dtime;
        m_Core->m_Matrix._43 -= m_FallingSpeed * dtime;
        RChange(MR_Matrix | MR_ShadowProjGeom | MR_ShadowProjTex | MR_ShadowStencil);

        float z = Z_From_Pos();
        if (m_Core->m_Matrix._43 < z) {
            m_Core->m_Matrix._43 = z;
            m_CurrState = ROBOT_SUCCESSFULLY_BUILD;
            m_KeelWaterCount = 0;

            m_FallingSpeed = 0;

            CMatrixEffect::CreateDust(NULL, *(D3DXVECTOR2 *)&GetGeoCenter(), D3DXVECTOR2(0, 0), 3000);
            CMatrixEffect::CreateDust(NULL, *(D3DXVECTOR2 *)&GetGeoCenter(), D3DXVECTOR2(0, 0), 3000);
            CMatrixEffect::CreateDust(NULL, *(D3DXVECTOR2 *)&GetGeoCenter(), D3DXVECTOR2(0, 0), 3000);
            CMatrixEffect::CreateDust(NULL, *(D3DXVECTOR2 *)&GetGeoCenter(), D3DXVECTOR2(0, 0), 3000);
            JoinToGroup();

            if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_TRACK || m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_WHEEL)
                m_ChassisData.u1.s3.m_LastSolePos = GetGeoCenter();

            // int x0 = TruncFloat(m_PosX * INVERT(GLOBAL_SCALE_MOVE));
            // int y0 = TruncFloat(m_PosY * INVERT(GLOBAL_SCALE_MOVE));

            // AsRobot()->MoveTo(x0, y0);
            AsRobot()->MapPosCalc();

            CSound::AddSound(S_ROBOT_UPAL, GetGeoCenter());
        }

        return;
    }

    DCP();

    if (m_CurrState == ROBOT_CARRYING) {
        DCP();

        RCData data;

        float mul = (float)(1.0 - pow(CARRYING_SPEED, double(ms)));
        if (m_CargoFlyer->GetCarryData()->m_RobotElevatorField &&
            m_CargoFlyer->GetCarryData()->m_RobotElevatorField->m_Activated) {
            // m_Matrix._43 -= (0.01f * ms);
            m_CargoFlyer->GetCarryData()->m_RobotMassFactor += (0.0005f * ms);
            if (m_CargoFlyer->GetCarryData()->m_RobotMassFactor > 1.0f) {
                m_CargoFlyer->GetCarryData()->m_RobotMassFactor = 1.0f;
            }

            // if (FRND(1) < mul * 0.5f)
            //    g_MatrixMap->AddEffect(CMatrixEffect::CreateShorted(g_MatrixHeap, m_CargoFlyer->GetPos(),
            //    *(D3DXVECTOR3 *)&m_Matrix._41, FRND(400) + 100));
        }
        else {
        }

        m_FallingSpeed -= float(ms) * 0.013f;
        if (m_FallingSpeed < 0)
            m_FallingSpeed = 0;

        DCP();

        D3DXVECTOR3 move, delta(m_CargoFlyer->GetPos() - D3DXVECTOR3(0, 0, CARRYING_DISTANCE) - m_Core->m_GeoCenter);
        // float x = D3DXVec3Length(&delta);
        move = delta * mul * m_CargoFlyer->GetCarryData()->m_RobotMassFactor;
        move.z -= m_FallingSpeed * float(ms) * 0.013f;
        *(D3DXVECTOR3 *)&m_Core->m_Matrix._41 += move;

        float cz = g_MatrixMap->GetZ(m_Core->m_Matrix._41, m_Core->m_Matrix._42);
        if (m_Core->m_Matrix._43 < cz)
            m_Core->m_Matrix._43 = cz;

        m_CargoFlyer->GetCarryData()->m_RobotUp.x +=
                move.x * 0.03f + m_CargoFlyer->GetCarryData()->m_RobotUpBack.x * mul;
        m_CargoFlyer->GetCarryData()->m_RobotUp.y +=
                move.y * 0.03f + m_CargoFlyer->GetCarryData()->m_RobotUpBack.y * mul;

        Vec2Truncate(*(D3DXVECTOR2 *)&m_CargoFlyer->GetCarryData()->m_RobotUp, 1);

        m_CargoFlyer->GetCarryData()->m_RobotUpBack.x -= m_CargoFlyer->GetCarryData()->m_RobotUp.x * 0.08f;
        m_CargoFlyer->GetCarryData()->m_RobotUpBack.y -= m_CargoFlyer->GetCarryData()->m_RobotUp.y * 0.08f;
        ;
        m_CargoFlyer->GetCarryData()->m_RobotUpBack.z = 0;

        float mul2 = (float)pow(0.999, double(ms));
        (*(D3DXVECTOR2 *)&m_CargoFlyer->GetCarryData()->m_RobotUp) *= mul2;

        Vec2Truncate(*(D3DXVECTOR2 *)&m_CargoFlyer->GetCarryData()->m_RobotUpBack, 0.4f);

        float da = mul * mul * (float)AngleDist(m_CargoFlyer->GetAngle(), m_CargoFlyer->GetCarryData()->m_RobotAngle);
        m_CargoFlyer->GetCarryData()->m_RobotAngle -= da;

        *((D3DXVECTOR2 *)&m_Forward) = RotatePoint(*(D3DXVECTOR2 *)&m_CargoFlyer->GetCarryData()->m_RobotForward,
                                                   m_CargoFlyer->GetCarryData()->m_RobotAngle);
        m_Forward.z = m_CargoFlyer->GetCarryData()->m_RobotForward.z;

        RChange(MR_Matrix);
        m_CalcBoundsLastTime = g_MatrixMap->GetTime() - 10000;

        D3DXVECTOR3 vmin, vmax;
        if (CalcBounds(vmin, vmax))
            ERROR_E;
        m_Core->m_GeoCenter = (vmin + vmax) * 0.5f;

        data.neardist2 = 1e30f;
        data.nearest = NULL;

        bool hit = g_MatrixMap->FindObjects(m_Core->m_GeoCenter, m_Core->m_Radius * 0.5f, 0.5f, TRACE_ALL, NULL,
                                            CollisionRobots, (DWORD)&data);

        if (hit) {
            float dist = (float)sqrt(data.neardist2);
            if (dist >= 0.000001f)
                data.norm *= INVERT(dist);
            else
                D3DXVec3Normalize(&data.norm, &data.norm);

            *(D3DXVECTOR3 *)&m_Core->m_Matrix._41 +=
                    data.norm * (data.nearest->GetRadius() * 0.5f + m_Core->m_Radius * 0.5f - dist);
        }

        DWORD tc = g_MatrixMap->GetColor(m_Core->m_GeoCenter.x, m_Core->m_GeoCenter.y);
        // m_Core->m_TerainColor = LIC(tc,g_MatrixMap->m_Terrain2ObjectTargetColor,
        // g_MatrixMap->m_Terrain2ObjectInfluence);
        m_Core->m_TerainColor = 0xFFFFFFFF;

        // m_PosX = m_Matrix._41;
        // m_PosY = m_Matrix._42;
        m_PosX = m_Core->m_GeoCenter.x;
        m_PosY = m_Core->m_GeoCenter.y;

        RChange(MR_Matrix | MR_ShadowProjGeom | MR_ShadowProjTex | MR_ShadowStencil);

        // RNeed(MR_Matrix);
        if (m_CargoFlyer->GetCarryData()->m_RobotElevatorField == NULL) {
            m_CargoFlyer->GetCarryData()->m_RobotElevatorField =
                    (CMatrixEffectElevatorField *)CMatrixEffect::CreateElevatorField(
                            m_CargoFlyer->GetPos(), m_Core->m_GeoCenter, m_Core->m_Radius, m_Forward);
            if (!g_MatrixMap->AddEffect(m_CargoFlyer->GetCarryData()->m_RobotElevatorField))
                m_CargoFlyer->GetCarryData()->m_RobotElevatorField = NULL;
        }
        else {
            m_CargoFlyer->GetCarryData()->m_RobotElevatorField->UpdateData(
                    m_CargoFlyer->GetPos(), *(D3DXVECTOR3 *)&m_Core->m_Matrix._41, m_Core->m_Radius, m_Forward);
        }

        return;
    }
    if (IsShorted())
        return;

    DCP();

    // normal...

    {
        float mul = (float)(1.0 - pow(0.996, double(ms)));
        D3DXVECTOR3 up;
        g_MatrixMap->GetNormal(&up, m_PosX, m_PosY, true);
        *(D3DXVECTOR3 *)&m_Core->m_Matrix._31 = LERPVECTOR(mul, *(D3DXVECTOR3 *)&m_Core->m_Matrix._31, up);
    }
    DCP();

    // soles
    if (m_CurrState != ROBOT_IN_SPAWN) {
        DCP();

        if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_TRACK) {
            auto tmp = m_ChassisData.u1.s3.m_LastSolePos - m_Core->m_GeoCenter;
            float dist_sq = D3DXVec3LengthSq(&tmp);
            if (dist_sq > 100) {
                m_ChassisData.u1.s3.m_LastSolePos = m_Core->m_GeoCenter;
                int x = TruncFloat(m_Core->m_GeoCenter.x * INVERT(GLOBAL_SCALE));
                int y = TruncFloat(m_Core->m_GeoCenter.y * INVERT(GLOBAL_SCALE));
                SMatrixMapUnit *mu = g_MatrixMap->UnitGetTest(x, y);
                if (mu && mu->IsLand() && !mu->IsBridge()) {
                    float ang = (float)atan2(-m_Forward.x, m_Forward.y);
                    CMatrixEffect::CreateLandscapeSpot(NULL, *(D3DXVECTOR2 *)&m_Core->m_Matrix._41, ang, 2,
                                                       SPOT_SOLE_TRACK);
                }
            }
        }
        DCP();

        if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_WHEEL) {
            auto tmp = m_ChassisData.u1.s3.m_LastSolePos - m_Core->m_GeoCenter;
            float dist_sq = D3DXVec3LengthSq(&tmp);
            if (dist_sq > 100) {
                m_ChassisData.u1.s3.m_LastSolePos = m_Core->m_GeoCenter;
                int x = TruncFloat(m_Core->m_GeoCenter.x * INVERT(GLOBAL_SCALE));
                int y = TruncFloat(m_Core->m_GeoCenter.y * INVERT(GLOBAL_SCALE));
                SMatrixMapUnit *mu = g_MatrixMap->UnitGetTest(x, y);
                if (mu && mu->IsLand() && !mu->IsBridge()) {
                    float ang = (float)atan2(-m_Forward.x, m_Forward.y);
                    CMatrixEffect::CreateLandscapeSpot(NULL, *(D3DXVECTOR2 *)&m_Core->m_Matrix._41, ang, 3.0f,
                                                       SPOT_SOLE_WHEEL);
                }
            }
        }
    }
    DCP();

    if (m_CurrState == ROBOT_IN_SPAWN) {
        DCP();

        if (GetBase()) {
            if (GetBase()->m_State == BASE_OPENED) {
                m_CurrState = ROBOT_BASE_MOVEOUT;
            }
        }
    }
    else if (m_CurrState == ROBOT_BASE_MOVEOUT) {
        DCP();
        LowLevelMove(ms, m_Forward * 100, true, false);
        DCP();
        RChange(MR_Matrix | MR_ShadowProjGeom | MR_ShadowProjTex | MR_ShadowStencil);

        if (GetBase()) {
            CMatrixBuilding *base = GetBase();
            DCP();
            D3DXVECTOR2 dist = D3DXVECTOR2(m_PosX, m_PosY) - base->m_Pos;
            if (D3DXVec2LengthSq(&dist) >= BASE_DIST * BASE_DIST) {
                DCP();
                GetLost(m_Forward);
                D3DXVec2Normalize(&dist, &dist);
                m_CurrState = ROBOT_SUCCESSFULLY_BUILD;
                RESETFLAG(m_ObjectState, ROBOT_FLAG_DISABLE_MANUAL);

                DCP();

                if (m_OrdersInPool == 0)
                    LowLevelStop();
                g_MatrixMap->m_Minimap.AddEvent(m_Core->m_GeoCenter.x, m_Core->m_GeoCenter.y, 0xff00ff00, 0xff00ff00);

                base->ResetSpawningBot();
                base->Close();
                SetBase(NULL);
                DCP();
            }
        }
        else {
            DCP();
            MustDie();
            return;
        }
        DCP();

        goto do_animation;
    }
    DCP();

    ////////////////Real-deal logic starts here
    ///:)))/////////////////////////////////////////////////////////////////////////
    if (m_CurrState != ROBOT_SUCCESSFULLY_BUILD)
        RChange(MR_Matrix | MR_ShadowProjGeom | MR_ShadowProjTex | MR_ShadowStencil);
    if (m_CurrState != ROBOT_SUCCESSFULLY_BUILD && m_CurrState != ROBOT_BASE_CAPTURE) {
        goto do_animation;
    }
    DCP();

    //    GatherInfo(ms);
    //    m_Environment.LogicTakt();

    // if(this == (CMatrixRobotAI*)g_MatrixMap->GetPlayerSide()->GetArcadedObject())
    //    CDText::T("Velocity", m_Speed);
    DCP();

    if (m_Side == PLAYER_SIDE && this == g_MatrixMap->GetPlayerSide()->GetArcadedObject() &&
        m_CurrState == ROBOT_SUCCESSFULLY_BUILD) {
        if (g_IFaceList->m_InFocus != INTERFACE) {
            RotateHull(g_MatrixMap->m_TraceStopPos);
        }
    }
    DCP();

    // TODO : fire while capture here!

    if (this != g_MatrixMap->GetPlayerSide()->GetArcadedObject()) {
        if (m_Environment.m_Target && m_Environment.m_Target->IsRobot() &&
            m_Environment.m_Target->AsRobot()->m_CurrState == ROBOT_SUCCESSFULLY_BUILD) {
            DCP();
            bool capturing = FindOrderLikeThat(ROT_CAPTURE_FACTORY);

            CMatrixRobotAI *Enemy = (CMatrixRobotAI *)m_Environment.m_Target;
            D3DXVECTOR2 napr(0, 0);
            D3DXVECTOR3 enemy_pos(0, 0, 0);
            enemy_pos = D3DXVECTOR3(Enemy->m_PosX, Enemy->m_PosY, 0);
            napr = D3DXVECTOR2(Enemy->m_PosX, Enemy->m_PosY) - D3DXVECTOR2(m_PosX, m_PosY);
            D3DXVECTOR2 naprN;
            D3DXVec2Normalize(&naprN, &napr);

            float Cos = m_Forward.x * naprN.x + m_Forward.y * naprN.y;
            float needAngle = float(acos(Cos));
            //        RotateHull(D3DXVECTOR3(Enemy->m_PosX, Enemy->m_PosY, 0));
            if (fabs(needAngle) < MAX_HULL_ANGLE) {
                RotateHull(D3DXVECTOR3(Enemy->m_PosX, Enemy->m_PosY, 0));
            }
            else if (m_CollAvoid.x == 0 && m_CollAvoid.y == 0 && !capturing) {
                RotateRobot(D3DXVECTOR3(Enemy->m_PosX, Enemy->m_PosY, 0));
                RotateHull(D3DXVECTOR3(m_PosX + m_Forward.x, m_PosY + m_Forward.y, 0));
            }

            /*			Cos = m_Forward.x*m_HullForward.x + m_Forward.y*m_HullForward.y;
                        float realAngle = float(acos(Cos));

                        if(fabs(needAngle - realAngle) <= HULL_TO_ENEMY_ANGLE){
                            if(!Enemy->m_Environment.SearchEnemy(this)){
                                Enemy->m_Environment.AddToList(this);
                            }
                            m_FireTarget = Enemy;
                            Fire(Enemy->m_Core->m_GeoCenter + Enemy->m_Velocity);
                        }else{
                            StopFire();
                        }*/
        }
        else if (m_Environment.m_Target && m_Environment.m_Target->IsLiveActiveCannon()) {
            DCP();
            bool capturing = FindOrderLikeThat(ROT_CAPTURE_FACTORY);

            CMatrixCannon *Enemy = (CMatrixCannon *)m_Environment.m_Target;
            D3DXVECTOR2 napr(0, 0);
            D3DXVECTOR3 enemy_pos(0, 0, 0);
            enemy_pos = D3DXVECTOR3(Enemy->GetGeoCenter().x, Enemy->GetGeoCenter().y, 0);
            napr = D3DXVECTOR2(enemy_pos.x, enemy_pos.y) - D3DXVECTOR2(m_PosX, m_PosY);
            D3DXVECTOR2 naprN;
            D3DXVec2Normalize(&naprN, &napr);

            float Cos = m_Forward.x * naprN.x + m_Forward.y * naprN.y;
            float needAngle = float(acos(Cos));
            //        RotateHull(D3DXVECTOR3(Enemy->m_PosX, Enemy->m_PosY, 0));
            if (fabs(needAngle) < MAX_HULL_ANGLE) {
                RotateHull(enemy_pos);
            }
            else if (m_CollAvoid.x == 0 && m_CollAvoid.y == 0 && !capturing) {
                RotateRobot(enemy_pos);
                RotateHull(D3DXVECTOR3(m_PosX + m_Forward.x, m_PosY + m_Forward.y, 0));
            }

            /*			Cos = m_Forward.x*m_HullForward.x + m_Forward.y*m_HullForward.y;
                        float realAngle = float(acos(Cos));

                        if(fabs(needAngle - realAngle) <= HULL_TO_ENEMY_ANGLE){
                            m_FireTarget = Enemy;
                            Fire(Enemy->GetGeoCenter());
                        }else{
                            StopFire();
                        }*/
        }
        else if (m_Environment.m_Target && m_Environment.m_Target->IsLiveBuilding() &&
                 !FindOrderLikeThat(ROT_CAPTURE_FACTORY)) {
            DCP();

            //            CMatrixBuilding * Enemy = (CMatrixCannon*)m_Environment.m_Target;
            D3DXVECTOR2 napr(0, 0);
            D3DXVECTOR3 enemy_pos = m_WeaponDir;
            //            enemy_pos = D3DXVECTOR3(Enemy->GetGeoCenter().x, Enemy->GetGeoCenter().y, 0);
            napr = D3DXVECTOR2(enemy_pos.x, enemy_pos.y) - D3DXVECTOR2(m_PosX, m_PosY);
            D3DXVECTOR2 naprN;
            D3DXVec2Normalize(&naprN, &napr);

            float Cos = m_Forward.x * naprN.x + m_Forward.y * naprN.y;
            float needAngle = float(acos(Cos));
            if (fabs(needAngle) < MAX_HULL_ANGLE) {
                RotateHull(enemy_pos);
            }
            else if (m_CollAvoid.x == 0 && m_CollAvoid.y == 0) {
                RotateRobot(enemy_pos);
                RotateHull(D3DXVECTOR3(m_PosX + m_Forward.x, m_PosY + m_Forward.y, 0));
            }
        }
        else {
            if (this != (CMatrixRobotAI *)g_MatrixMap->GetPlayerSide()->GetArcadedObject()) {
                StopFire();
                RotateHull(D3DXVECTOR3(m_PosX + m_Forward.x, m_PosY + m_Forward.y, 0));
            }
        }
    }

    DCP();

    // ORDERS PROCESSING/////////////////////////////////////////////////////

    if (this == (CMatrixRobotAI *)g_MatrixMap->GetPlayerSide()
                        ->GetArcadedObject() /* && !((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0x8000)*/) {
        if (((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_LEFT]) & 0x8000) == 0x8000) ||
            ((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_LEFT_ALT]) & 0x8000) == 0x8000)) {
            RotateRobotLeft();
        }
        if (((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_RIGHT]) & 0x8000) == 0x8000) ||
            ((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_RIGHT_ALT]) & 0x8000) == 0x8000)) {
            RotateRobotRight();
        }
        if (((GetAsyncKeyState(g_Config.m_KeyActions[KA_FIRE]) & 0x8000) == 0x8000)) {
            if (!FindOrderLikeThat(ROT_FIRE) && g_IFaceList->m_InFocus != INTERFACE) {
                Fire(g_MatrixMap->m_TraceStopPos);
            }
        }
        else {
            if (FindOrderLikeThat(ROT_FIRE)) {
                StopFire();
            }
        }
    }
    DCP();

    if (FLAG(m_ObjectState, ROBOT_FLAG_ROT_LEFT | ROBOT_FLAG_ROT_RIGHT)) {
        D3DXVECTOR3 dest;
        D3DXMATRIX rot_mat;

        float ang = 0;

        if (FLAG(m_ObjectState, ROBOT_FLAG_ROT_LEFT)) {
            ang -= GRAD2RAD(90.0f);
        }
        if (FLAG(m_ObjectState, ROBOT_FLAG_ROT_RIGHT)) {
            ang += GRAD2RAD(90.0f);
        }

        dest = m_Forward * m_maxSpeed;
        D3DXMatrixRotationZ(&rot_mat, ang);
        D3DXVec3TransformCoord(&dest, &dest, &rot_mat);
        dest.x += m_PosX;
        dest.y += m_PosY;

        RotateRobot(dest);
        RESETFLAG(m_ObjectState, ROBOT_FLAG_ROT_LEFT | ROBOT_FLAG_ROT_RIGHT);
    }
    DCP();

    if (FLAG(g_MatrixMap->m_Flags, MMFLAG_AUTOMATIC_MODE) || m_Side != PLAYER_SIDE)
        TaktCaptureCandidate(ms);

    // if(this == g_MatrixMap->GetPlayerSide()->GetArcadedObject()){
    //    ASSERT(1);
    //}
    int cnt = 0;
    while (cnt < m_OrdersInPool) {
        float f1 = 0, f2 = 0, f3 = 0, x = 0, y = 0;
        D3DXVECTOR3 vvv;
        CMatrixBuilding *factory = NULL;
        int mx = 0, my = 0, d = 0;
        bool StillMoving = false;
        switch (m_OrdersList[0].GetOrderType()) {
            case ROT_MOVE_TO:
            case ROT_MOVE_TO_BACK: {
                DCP();
                if (this == (CMatrixRobotAI *)g_MatrixMap->GetPlayerSide()->GetArcadedObject()) {
                    if (((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_FORWARD]) & 0x8000) == 0x8000) ||
                        ((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_FORWARD_ALT]) & 0x8000) == 0x8000)) {
                        D3DXVECTOR3 dest(m_PosX, m_PosY, 0);
                        dest += m_Forward * m_maxSpeed;
                        LowLevelMove(ms, dest, true, true, false);
                    }
                    else if (((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_BACKWARD]) & 0x8000) == 0x8000) ||
                             ((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_BACKWARD_ALT]) & 0x8000) == 0x8000)) {
                        D3DXVECTOR3 dest(m_PosX, m_PosY, 0);
                        dest -= m_Forward * m_maxSpeed;
                        LowLevelMove(ms, dest, true, true, false, true);
                    }
                    else {
                        StopMoving();
                    }
                    MapPosCalc();
                    break;
                }

                int i;
                for (i = 0; i < m_OrdersInPool; i++) {
                    if (m_OrdersList[i].GetOrderType() == ROT_CAPTURE_FACTORY &&
                        m_OrdersList[i].GetOrderPhase() == ROP_CAPTURE_IN_POSITION) {
                        CMatrixBuilding *factory = (CMatrixBuilding *)m_OrdersList[i].GetStatic();
                        if (factory && factory->IsBase())
                            break;
                    }
                }
                if (i < m_OrdersInPool)
                    break;

                // int x0 = TruncFloat(robot_pos.x * INVERT(GLOBAL_SCALE_MOVE)) - COLLIDE_FIELD_R;
                // int y0 = TruncFloat(robot_pos.y * INVERT(GLOBAL_SCALE_MOVE)) - COLLIDE_FIELD_R;

                // g_MatrixMap->PlaceGet(m_Unit[0].u1.s1.m_Kind-1,m_PosX-20.0f,m_PosY-20.0f,&m_MapX,&m_MapY);
                MapPosCalc();

                CHECK_ROBOT_POS();
                //	            if(!(g_MatrixMap->PlaceFindNear(m_Unit[0].u1.s1.m_Kind-1,4,m_MapX,m_MapY,this))) ERROR_E;

                ZoneCurFind();

                if (m_ZoneCur < 0) {
                    if (m_MovePathCnt > 0)
                        MoveByMovePath(ms);

                    break;
                }

                /*                if(m_ZonePathNext>=0 && m_ZonePathNext<m_ZonePathCnt) {
                                    if(m_ZoneCur==m_ZonePath[m_ZonePathNext]) {
                                        m_ZonePathNext++;
                                        m_MovePathCur=0;
                                        m_MovePathCnt=0;
                                    }
                                }*/
                if (m_ZonePathNext >= 0 && m_ZonePathNext < m_ZonePathCnt) {
                    if (m_MovePathDistFollow > m_MovePathDist * 0.7) {
                        int i;
                        for (i = m_ZonePathNext; i < m_ZonePathCnt; i++) {
                            if (m_ZonePath[i] == m_ZoneCur) {
                                m_ZonePathNext = i + 1;
                                m_MovePathCur = 0;
                                m_MovePathCnt = 0;
                                break;
                            }
                        }
                        if (i >= m_ZonePathCnt) {
                            m_ZonePathCnt = 0;
                            m_MovePathCur = 0;
                            m_MovePathCnt = 0;
                        }
                    }
                }

                if (m_MovePathCnt > 0) {
                    MoveByMovePath(ms);
                    break;
                }
                if (m_ZoneDes < 0) {
                    m_ZoneDes = g_MatrixMap->ZoneFindNear(m_Unit[0].u1.s1.m_Kind - 1, m_DesX, m_DesY);
                    m_ZonePathCnt = 0;
                }

                if (m_ZonePathCnt <= 0) {
                    ZonePathCalc();
                }

                ZoneMoveCalc();
                if (m_MovePathCnt <= 0)
                    StopMoving();

                break;
            }
            case ROT_MOVE_RETURN: {
                DCP();
                if (!FindOrderLikeThat(ROT_MOVE_TO)) {
                    m_OrdersList[0].GetParams(&f1, &f2, NULL, NULL);
                    if (g_MatrixMap->PlaceIsEmpty(m_Unit[0].u1.s1.m_Kind - 1, 4, Float2Int(f1), Float2Int(f2), this)) {
                        RemoveOrderFromTop();
                        MoveTo(Float2Int(f1), Float2Int(f2));
                        continue;
                    }
                }
                break;
            }
            case ROT_STOP_MOVE: {
                DCP();
                if (0 /*this == (CMatrixRobotAI*)g_MatrixMap->GetPlayerSide()->GetArcadedObject()*/) {
                    LowLevelDecelerate(ms, true, true);
                    if (m_Speed <= ZERO_VELOCITY) {
                        LowLevelStop();
                        RemoveOrderFromTop();
                    }
                }
                else {
                    LowLevelStop();
                    RemoveOrderFromTop();
                }
                SETFLAG(m_ObjectState, ROBOT_FLAG_COLLISION);
                RChange(MR_Matrix | MR_ShadowProjTex | MR_ShadowStencil);
                continue;
            }
            case ROT_FIRE: {
                DCP();

                int type;

                m_OrdersList[0].GetParams(&f1, &f2, &f3, &type);

                if (g_MatrixMap->GetPlayerSide()->GetArcadedObject() == this) {
                    m_WeaponDir.x = g_MatrixMap->m_TraceStopPos.x;
                    m_WeaponDir.y = g_MatrixMap->m_TraceStopPos.y;
                    // if(IS_TRACE_STOP_OBJECT(g_MatrixMap->m_TraceStopObj))
                    //             m_WeaponDir.z = g_MatrixMap->m_TraceStopObj->GetGeoCenter().z;
                    // else
                    m_WeaponDir.z = g_MatrixMap->m_TraceStopPos.z;
                }
                else {
                    /*
                                        D3DXVECTOR2 napr = D3DXVECTOR2(f1, f2) - D3DXVECTOR2(m_PosX, m_PosY);
                                        D3DXVECTOR2 naprN;
                                        D3DXVec2Normalize(&naprN, &napr);

                                        float Cos = m_Forward.x*naprN.x + m_Forward.y*naprN.y;
                                        float needAngle = float(acos(Cos));

                                        if(fabs(needAngle) <= MAX_HULL_ANGLE)
                                            RotateHull(D3DXVECTOR3(f1, f2, 0));
                                        else
                                            RotateRobot(D3DXVECTOR3(f1, f2, 0));
                    */
                    m_WeaponDir.x = f1;
                    m_WeaponDir.y = f2;
                    m_WeaponDir.z = f3;
                }
                for (int nC = 0; nC < m_WeaponsCnt; ++nC) {
                    if (m_Weapons[nC].IsEffectPresent()) {
                        if (m_Weapons[nC].GetWeaponType() == WEAPON_REPAIR) {
                            if (type == 2)
                                m_Weapons[nC].FireBegin(m_Velocity * (1.0f / LOGIC_TAKT_PERIOD), this);
                            else
                                m_Weapons[nC].FireEnd();
                        }
                        else if (m_Weapons[nC].GetWeaponType() == WEAPON_BOMB) {
                            if (type == 0)
                                m_Weapons[nC].FireBegin(D3DXVECTOR3(f1, f2, f3), this);
                            else
                                m_Weapons[nC].FireEnd();
                        }
                        else {
                            if (type == 0)
                                m_Weapons[nC].FireBegin(m_Velocity * (1.0f / LOGIC_TAKT_PERIOD), this);
                            else
                                m_Weapons[nC].FireEnd();
                        }
                    }
                }
                break;
            }
            case ROT_STOP_FIRE: {
                LowLevelStopFire();
                RemoveOrderFromTop();
                continue;
            }
            case ROT_CAPTURE_FACTORY: {
                DCP();

                factory = (CMatrixBuilding *)m_OrdersList[0].GetStatic();
                if (factory == NULL)
                    break;
                factory->SetCapturedBy(this);

                D3DXVECTOR3 rotPos(0, 0, 0);

                if (m_OrdersList[0].GetOrderPhase() == ROP_EMPTY_PHASE) {
                    // kshhhhchk "Acknowledge" kshhhhchk
                    if (fabs(m_PosX - factory->m_Pos.x) < 2.0f && fabs(m_PosY - factory->m_Pos.y) < 2.0f) {
                        m_OrdersList[0].SetPhase(ROP_CAPTURE_SETTING_UP);
                        break;
                    }

                    m_OrdersList[0].SetPhase(ROP_CAPTURE_MOVING);
                    if (factory->IsBase()) {
                        D3DXVECTOR2 tgtpos(factory->m_Pos + (*(D3DXVECTOR2 *)&factory->GetMatrix()._21) * 100.0f);

                        mx = TruncFloat(tgtpos.x / GLOBAL_SCALE_MOVE - (ROBOT_MOVECELLS_PER_SIZE / 2));
                        my = TruncFloat(tgtpos.y / GLOBAL_SCALE_MOVE - (ROBOT_MOVECELLS_PER_SIZE / 2));

                        g_MatrixMap->PlaceFindNear(m_Unit[0].u1.s1.m_Kind - 1, 4, mx, my, this);
                    }
                    else {
                        mx = TruncFloat(factory->m_Pos.x / GLOBAL_SCALE_MOVE - (ROBOT_MOVECELLS_PER_SIZE / 2));
                        my = TruncFloat(factory->m_Pos.y / GLOBAL_SCALE_MOVE - (ROBOT_MOVECELLS_PER_SIZE / 2));
                        g_MatrixMap->PlaceFindNear(m_Unit[0].u1.s1.m_Kind - 1, 4, mx, my, this);
                    }
                    MoveTo(mx, my);
                    rotPos = D3DXVECTOR3(mx * GLOBAL_SCALE_MOVE, my * GLOBAL_SCALE_MOVE, 0);
                    m_OrdersList[0].SetPhase(ROP_CAPTURE_MOVING);
                    cnt--;
                }
                else if (m_OrdersList[0].GetOrderPhase() == ROP_CAPTURE_MOVING) {
                    // kshhhhchk "moving on" kshhhhchk
                    D3DXVECTOR2 tmp = factory->m_Pos - D3DXVECTOR2(m_PosX, m_PosY);
                    D3DXVECTOR3 dist = D3DXVECTOR3(tmp.x, tmp.y, 0);
                    if (factory->IsBase()) {
                        if (!FindOrderLikeThat(ROT_MOVE_TO, ROP_CAPTURE_MOVING) &&
                            D3DXVec3LengthSq(&dist) <= (BASE_DIST + 60) * (BASE_DIST + 60)) {
                            m_OrdersList[0].SetPhase(ROP_CAPTURE_IN_POSITION);
                        }
                    }
                    else {
                        if (!FindOrderLikeThat(ROT_MOVE_TO, ROP_CAPTURE_MOVING) && D3DXVec3LengthSq(&dist) <=
                                                                                           (BASE_DIST) * (BASE_DIST) /*(fabs(TruncFloat(m_PosX) - factory->m_Pos.x) < 5 && fabs(TruncFloat(m_PosY) - factory->m_Pos.y) < 5)*/) {
                            m_OrdersList[0].SetPhase(ROP_CAPTURE_IN_POSITION);
                        }
                    }
                }
                else if (m_OrdersList[0].GetOrderPhase() == ROP_CAPTURE_IN_POSITION) {
                    // kshhhhchk "i'm in position" kshhhhchk

                    if (factory->IsBase()) {
                        SetBase(factory);
                        if (factory->m_State != BASE_OPENED && factory->m_State != BASE_OPENING) {
                            factory->Open();
                            break;
                        }
                        else if (factory->m_State != BASE_OPENED) {
                            break;
                        }

                        SETFLAG(m_ObjectState, ROBOT_FLAG_DISABLE_MANUAL);

                        LowLevelMove(ms, D3DXVECTOR3(factory->m_Pos.x, factory->m_Pos.y, 0), false, false);
                    }
                    else {
                        LowLevelMove(ms, D3DXVECTOR3(factory->m_Pos.x, factory->m_Pos.y, 0), true, true);
                    }

                    if (fabs(m_PosX - factory->m_Pos.x) < 2.0f && fabs(m_PosY - factory->m_Pos.y) < 2.0f) {
                        m_OrdersList[0].SetPhase(ROP_CAPTURE_SETTING_UP);
                        break;
                    }
                }
                else if (m_OrdersList[0].GetOrderPhase() == ROP_CAPTURE_SETTING_UP) {
                    // kshhhhchk "setting up devices" kshhhhchk
                    if (RotateRobot(
                                D3DXVECTOR3(m_PosX + factory->GetMatrix()._21, m_PosY + factory->GetMatrix()._22, 0))) {
                        m_OrdersList[0].SetPhase(ROP_CAPTURING);
                        LowLevelStop();
                    }
                    RotateHull(D3DXVECTOR3(m_PosX + m_Forward.x, m_PosY + m_Forward.y, 0));
                }
                else if (m_OrdersList[0].GetOrderPhase() == ROP_CAPTURING) {
                    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
                    if (ps->m_CurrSel == BUILDING_SELECTED || ps->m_CurrSel == BASE_SELECTED) {
                        CMatrixBuilding *bld = ((CMatrixBuilding *)ps->m_ActiveObject);
                        if (bld == factory) {
                            ps->Select(NOTHING, NULL);
                            ps->PLDropAllActions();
                        }
                    }

                    // kshhhhchk "captureing factory" kshhhhchk
                    if (factory->IsBase()) {
                        if (factory->m_State != BASE_CLOSED && factory->m_State != BASE_CLOSING) {
                            m_CurrState = ROBOT_BASE_CAPTURE;
                            factory->Close();
                        }
                        else if (factory->m_State == BASE_CLOSED) {
                            if (m_Side == PLAYER_SIDE) {
                                CSound::Play(S_ENEMY_BASE_CAPTURED);
                            }
                            else {
                                if (factory->m_Side == PLAYER_SIDE)
                                    CSound::Play(S_PLAYER_BASE_CAPTURED);
                            }

                            factory->m_Side = m_Side;
                            factory->m_BS.ClearStack();
                            RemoveOrderFromTop();
                            g_MatrixMap->StaticDelete(this);
                            return;
                        }
                    }
                    else {
                        if (factory->Capture(this) == CAPTURE_DONE) {
                            RemoveOrderFromTop();
                            continue;
                        }
                    }
                }
            } break;

            case ROT_STOP_CAPTURE: {
                DCP();

                RemoveOrderFromTop();
                continue;
            }
        }
        // Process next order
        ProcessOrdersList();
        cnt++;
    }
    DCP();

    //{
    //    if(this == (CMatrixRobotAI*)g_MatrixMap->GetPlayerSide()->GetArcadedObject() && m_Speed == 0){
    //        CMatrixMapStatic* statics = CMatrixMapStatic::GetFirstLogic();
    //
    //        while(statics){
    //            if(statics->GetObjectType() == OBJECT_TYPE_BUILDING && ((CMatrixBuilding*)statics)->m_Side != m_Side){
    //                CMatrixBuilding* eve_factory = (CMatrixBuilding*)statics;
    //                if(!eve_factory->m_BusyFlag.IsBusy() || (eve_factory->m_BusyFlag.IsBusy() &&
    //                eve_factory->m_BusyFlag.GetBusyBy() != this)){
    //                   D3DXVECTOR3 res = m_Core->m_GeoCenter - D3DXVECTOR3(eve_factory->m_Pos.x, eve_factory->m_Pos.y,
    //                   0); if(D3DXVec3LengthSq(&res) < 800 && eve_factory->Capture(this) == CAPTURE_DONE)
    //                   {
    //                        eve_factory->m_BusyFlag.Reset();
    //                   }
    //                }
    //            }
    //            statics = statics->GetNextLogic();
    //        }
    //    }
    //}
    {
        for (int nC = 0; nC < m_WeaponsCnt; ++nC) {
            if (m_Weapons[nC].IsEffectPresent() &&
                (m_Weapons[nC].m_On || g_MatrixMap->GetPlayerSide()->GetArcadedObject() == this)) {
                //            m_WeaponDir.x = g_MatrixMap->m_TraceStopPos.x;
                //    		m_WeaponDir.y = g_MatrixMap->m_TraceStopPos.y;
                // m_WeaponDir.z = g_MatrixMap->m_TraceStopPos.z;

                int mod = 0;
                if (m_Weapons[nC].GetWeaponType() == WEAPON_HOMING_MISSILE) {
                    mod = 1;
                }
                else if (m_Weapons[nC].GetWeaponType() == WEAPON_BOMB) {
                    mod = 1;
                }
                else {
                    mod = 2;
                }

                if ((m_Weapons[nC].m_Heat + (m_Weapons[nC].m_HeatMod * mod) > WEAPON_MAX_HEAT)) {
                    m_Weapons[nC].FireEnd();
                    continue;
                }

                float fire_dist = m_Weapons[nC].GetWeaponDist();
                D3DXVECTOR3 vPos(0, 0, 0), vDir(0, 0, 0);
                vDir = m_WeaponDir;

                D3DXMATRIX m = (*m_Weapons[nC].m_Unit->m_Graph->GetMatrixById(1)) * m_Weapons[nC].m_Unit->m_Matrix;
                D3DXVec3TransformCoord(&vPos, &vPos, &m);

                D3DXVECTOR3 vWeapPos = D3DXVECTOR3(m._21, m._22, m._23);

                vDir.x -= vPos.x;
                vDir.y -= vPos.y;
                vDir.z -= vPos.z;

                D3DXVec3Normalize(&vDir, &vDir);

                // CHECKRIDE
                // Fire distance checkride
                if (this != g_MatrixMap->GetPlayerSide()->GetArcadedObject()) {
                    auto tmp = GetGeoCenter() - m_WeaponDir;
                    if (D3DXVec3LengthSq(&tmp) > fire_dist * fire_dist) {
                        m_Weapons[nC].FireEnd();
                        continue;
                    }
                }
                DCP();

                // Special weapons handler
                if (m_Weapons[nC].GetWeaponType() == WEAPON_BOMB) {
                    m_Weapons[nC].Modify(vPos, vWeapPos, m_WeaponDir);
                    m_Weapons[nC].Takt(float(ms));

                    if (m_Weapons[nC].IsFireWas()) {
                        // FIREINDAHOLE:
                        m_Weapons[nC].m_Heat += m_Weapons[nC].m_HeatMod;
                        if (m_Weapons[nC].m_Heat > WEAPON_MAX_HEAT) {
                            m_Weapons[nC].m_Heat = WEAPON_MAX_HEAT;
                        }

                        if (m_Weapons[nC].m_Unit->m_Graph->SetAnimByNameNoBegin(ANIMATION_NAME_FIRELOOP)) {
                            // not looped
                            m_Weapons[nC].m_Unit->m_Graph->SetAnimByName(ANIMATION_NAME_FIRE, 0);
                        }
                    }

                    continue;
                }
                else if (m_Weapons[nC].GetWeaponType() == WEAPON_BIGBOOM) {
                    // m_Weapons[nC].m_Weapon->Modify(vPos, vDir, m_Velocity * (1.0f/LOGIC_TAKT_PERIOD));
                    // m_Weapons[nC].m_Weapon->Takt(float(ms));
                    continue;
                }
                else if (m_Weapons[nC].GetWeaponType() == WEAPON_HOMING_MISSILE) {
                    // Special homing_missile handler
                    m_Weapons[nC].Modify(vPos, vDir, m_Velocity * (1.0f / LOGIC_TAKT_PERIOD));
                    m_Weapons[nC].Takt(float(ms));
                    if (m_Weapons[nC].IsFireWas()) {
                        // FIREINDAHOLE:
                        m_Weapons[nC].m_Heat += m_Weapons[nC].m_HeatMod;
                        if (m_Weapons[nC].m_Heat > WEAPON_MAX_HEAT) {
                            m_Weapons[nC].m_Heat = WEAPON_MAX_HEAT;
                        }
                    }
                    continue;
                }

                m_Weapons[nC].Modify(vPos, vDir, m_Velocity * (1.0f / LOGIC_TAKT_PERIOD));
                // Common weapons handler
                // Angle checkride

                // g_MatrixMap->m_DI.T(L"d",std::wstring(fire_dist));
                // g_MatrixMap->m_DI.T(L"d1",std::wstring(D3DXVec3Length(&vDir)));
                // g_MatrixMap->m_DI.T(L"d2",std::wstring(D3DXVec3Length(&D3DXVECTOR3(vPosNorm.x, vPosNorm.y, vDir.z))));

                // CHelper::Create(1, 0)->Line(vPos, vPos + vDir * (float)fire_dist, 0xffff0000, 0xffff0000);
                // CHelper::Create(1, 0)->Line(vPos, vPos + D3DXVECTOR3(vPosNorm.x, vPosNorm.y, vDir.z) *
                // (float)fire_dist);

                D3DXVECTOR2 dir1, dir2;
                D3DXVec2Normalize(&dir1, (D3DXVECTOR2 *)&vWeapPos);
                D3DXVec2Normalize(&dir2, (D3DXVECTOR2 *)&vDir);

                // float cos = vPosNorm.x*vDir.x + vPosNorm.y*vDir.y + /*vPosNorm.z*/vDir.z*vDir.z;

                float angle = (float)acos(D3DXVec2Dot(&dir1, &dir2));
                //            CDText::T("angle", angle);
                //            CDText::T("cos", cos);
                if (angle > BARREL_TO_SHOT_ANGLE && m_Weapons[nC].GetWeaponType() != WEAPON_REPAIR) {
                    m_Weapons[nC].FireEnd();
                    continue;
                }

                m_Weapons[nC].Takt(float(ms));
                if (m_Weapons[nC].IsFireWas()) {
                    // FIREINDAHOLE:
                    m_Weapons[nC].m_Heat += m_Weapons[nC].m_HeatMod;
                    if (m_Weapons[nC].m_Heat > WEAPON_MAX_HEAT) {
                        m_Weapons[nC].m_Heat = WEAPON_MAX_HEAT;
                    }

                    if (m_Weapons[nC].m_Unit->m_Graph->SetAnimByNameNoBegin(ANIMATION_NAME_FIRELOOP)) {
                        // not looped
                        m_Weapons[nC].m_Unit->m_Graph->SetAnimByName(ANIMATION_NAME_FIRE, 0);
                    }
                }
            }
        }
    }
    DCP();

    for (int cnt = 0; cnt < m_WeaponsCnt; ++cnt) {
        if (m_Weapons[cnt].IsEffectPresent() && m_Weapons[cnt].GetWeaponType() != WEAPON_BIGBOOM) {
            if (m_Weapons[cnt].m_Heat > 0)
                m_Weapons[cnt].m_CoolDownPeriod += ms;
            else
                m_Weapons[cnt].m_CoolDownPeriod = 0;

            m_Weapons[cnt].m_HeatPeriod = 0;
            int period = 0;
            switch (m_Weapons[cnt].GetWeaponType()) {
                case WEAPON_VOLCANO:
                    period = g_Config.m_Overheat[WCP_VOLCANO];
                    break;
                case WEAPON_GUN:
                    period = g_Config.m_Overheat[WCP_GUN];
                    break;
                case WEAPON_LASER:
                    period = g_Config.m_Overheat[WCP_LASER];
                    break;
                case WEAPON_PLASMA:
                    period = g_Config.m_Overheat[WCP_PLASMA];
                    break;
                case WEAPON_BOMB:
                    period = g_Config.m_Overheat[WCP_BOMB];
                    break;
                case WEAPON_FLAMETHROWER:
                    period = g_Config.m_Overheat[WCP_FLAMETHROWER];
                    break;
                case WEAPON_HOMING_MISSILE:
                    period = g_Config.m_Overheat[WCP_HOMING_MISSILE];
                    break;
                case WEAPON_LIGHTENING:
                    period = g_Config.m_Overheat[WCP_LIGHTENING];
                    break;
            }
            while (m_Weapons[cnt].m_Heat > 0 && m_Weapons[cnt].m_CoolDownPeriod >= period) {
                m_Weapons[cnt].m_Heat -= m_Weapons[cnt].m_CoolDownMod;
                if (m_Weapons[cnt].m_Heat < 0) {
                    m_Weapons[cnt].m_Heat = 0;
                }
                m_Weapons[cnt].m_CoolDownPeriod -= period;
            }
        }
    }

    DCP();

    goto do_animation;
}

void CMatrixRobotAI::ZoneCurFind() {
    SMatrixMapMove *smm = g_MatrixMap->MoveGet(m_MapX, m_MapY);
    if (smm && smm->m_Zone >= 0) {
        m_ZoneCur = smm->m_Zone;
        return;
    }

    /*	mu=g_MatrixMap->UnitGet(m_MapX+1,m_MapY+1);
        if(mu && mu->m_Zone[m_Unit[0].m_Kind-1]>=0) { m_ZoneCur=mu->m_Zone[m_Unit[0].m_Kind-1]; return;	}

        mu=g_MatrixMap->UnitGet(m_MapX+1,m_MapY);
        if(mu && mu->m_Zone[m_Unit[0].m_Kind-1]>=0) { m_ZoneCur=mu->m_Zone[m_Unit[0].m_Kind-1]; return;	}

        mu=g_MatrixMap->UnitGet(m_MapX,m_MapY+1);
        if(mu && mu->m_Zone[m_Unit[0].m_Kind-1]>=0) { m_ZoneCur=mu->m_Zone[m_Unit[0].m_Kind-1]; return;	}*/

    // ERROR_E;
    //		m_ZoneCur=g_MatrixMap->ZoneFindNear(m_Unit[0].m_Kind-1,m_MapX,m_MapY);

    /*	//m_MovePathCnt
        int m_MovePathCnt=g_MatrixMap->ZoneMoveFindNear(m_Unit[0].m_Kind-1,m_MapX,m_MapY,m_MovePath);
        if(newcnt<=0) {
            SMatrixMapUnit * mu=g_MatrixMap->UnitGet(m_MapX,m_MapY);
            if(mu && mu->m_Zone[m_Unit[0].m_Kind-1]>=0) m_ZoneCur=mu->m_Zone[m_Unit[0].m_Kind-1];
            else m_ZoneCur=-1;
        } else m_ZoneCur=-1;*/
}

void CMatrixRobotAI::ZonePathCalc() {
    if (m_ZoneCur < 0)
        return;
    ASSERT(m_ZoneCur >= 0);

    if (!m_ZonePath)
        m_ZonePath = (int *)HAlloc(g_MatrixMap->m_RN.m_ZoneCnt * sizeof(int), g_MatrixHeap);
    //	m_ZonePathCnt=g_MatrixMap->ZoneFindPath(m_Unit[0].u1.s1.m_Kind-1,m_ZoneCur,m_ZoneDes,m_ZonePath);

    CMatrixSideUnit *side = g_MatrixMap->GetSideById(GetSide());
    if (GetSide() == PLAYER_SIDE && GetGroupLogic() >= 0 &&
        side->m_PlayerGroup[GetGroupLogic()].m_RoadPath->m_ListCnt > 0) {
        m_ZonePathCnt = g_MatrixMap->FindPathInZone(m_Unit[0].u1.s1.m_Kind - 1, m_ZoneCur, m_ZoneDes,
                                                    side->m_PlayerGroup[GetGroupLogic()].m_RoadPath, 0, m_ZonePath,
                                                    g_TestRobot == this);
    }
    else if (GetSide() != PLAYER_SIDE && GetTeam() >= 0 && side->m_Team[GetTeam()].m_RoadPath->m_ListCnt > 0) {
        m_ZonePathCnt =
                g_MatrixMap->FindPathInZone(m_Unit[0].u1.s1.m_Kind - 1, m_ZoneCur, m_ZoneDes,
                                            side->m_Team[GetTeam()].m_RoadPath, 0, m_ZonePath, g_TestRobot == this);
    }
    else {
        m_ZonePathCnt = g_MatrixMap->FindPathInZone(m_Unit[0].u1.s1.m_Kind - 1, m_ZoneCur, m_ZoneDes, NULL, 0, m_ZonePath,
                                                    g_TestRobot == this);
    }
    if (m_ZonePathCnt > 0) {
        ASSERT(m_ZonePathCnt >= 2);
        m_ZonePathNext = 1;
    }
    else
        m_ZonePathNext = -1;

    if (GetSide() != PLAYER_SIDE && m_ZoneCur != m_ZoneDes &&
        m_ZonePathCnt <= 0) {  // Если дойти не можем, то меняем команду
        SetTeam(g_MatrixMap->GetSideById(GetSide())->ClacSpawnTeam(GetRegion(), m_Unit[0].u1.s1.m_Kind - 1));
        SetGroupLogic(-1);
    }
}

void CMatrixRobotAI::ZoneMoveCalc() {
    if (m_ZoneCur < 0)
        return;
    ASSERT(m_ZoneCur >= 0);
    //	ASSERT(m_ZoneNear>=0);

    int other_cnt = 0;
    int other_size[200];
    CPoint *other_path_list[200];
    int other_path_cnt[200];
    CPoint other_des[200];

    m_MoveTestChange = g_MatrixMap->GetTime();

    CMatrixMapStatic *obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (obj->IsLiveRobot() && obj != this) {
            CMatrixRobotAI *r = (CMatrixRobotAI *)obj;
            CPoint tp;
            if (r->GetMoveToCoords(tp)) {
                ASSERT(other_cnt < 200);
                other_size[other_cnt] = 4;
                if (r->m_MovePath && r->m_MovePathCur < r->m_MovePathCnt) {
                    other_path_list[other_cnt] = r->m_MovePath + r->m_MovePathCur;
                    other_path_cnt[other_cnt] = r->m_MovePathCnt - r->m_MovePathCur;
                }
                else {
                    other_path_list[other_cnt] = NULL;
                    other_path_cnt[other_cnt] = 0;
                }
                other_des[other_cnt] = CPoint(r->m_DesX, r->m_DesY);
                other_cnt++;
            }
            else {
                ASSERT(other_cnt < 200);
                other_size[other_cnt] = 4;
                other_path_list[other_cnt] = NULL;
                other_path_cnt[other_cnt] = 0;
                other_des[other_cnt] = CPoint(r->m_MapX, r->m_MapY);
                other_cnt++;
            }
        }
        else if (obj->IsLiveCannon()) {
            ASSERT(other_cnt < 200);
            other_size[other_cnt] = 4;
            other_path_list[other_cnt] = NULL;
            other_path_cnt[other_cnt] = 0;
            other_des[other_cnt] =
                    CPoint(Float2Int(obj->AsCannon()->m_Pos.x / GLOBAL_SCALE_MOVE) - ROBOT_MOVECELLS_PER_SIZE / 2,
                           Float2Int(obj->AsCannon()->m_Pos.y / GLOBAL_SCALE_MOVE) - ROBOT_MOVECELLS_PER_SIZE / 2);
            other_cnt++;
        }
        obj = obj->GetNextLogic();
    }

    if (m_ZonePathNext >= 0) {
        m_MovePathCnt =
                g_MatrixMap->FindLocalPath(m_Unit[0].u1.s1.m_Kind - 1, 4, m_MapX, m_MapY, m_ZonePath + m_ZonePathNext - 1,
                                           m_ZonePathCnt - (m_ZonePathNext - 1), m_DesX, m_DesY, m_MovePath, other_cnt,
                                           other_size, other_path_list, other_path_cnt, other_des, g_TestRobot == this);
    }
    else {
        m_MovePathCnt = g_MatrixMap->FindLocalPath(m_Unit[0].u1.s1.m_Kind - 1, 4, m_MapX, m_MapY, &m_ZoneCur, 1, m_DesX,
                                                   m_DesY, m_MovePath, other_cnt, other_size, other_path_list,
                                                   other_path_cnt, other_des, g_TestRobot == this);
    }

    // int zonesou1=m_ZoneCur;
    // int zonesou2=m_ZoneCur;
    // int zonesou3=m_ZoneCur;
    // if(m_ZonePathNext>=0 && m_ZonePathNext<m_ZonePathCnt) {
    //	zonesou1=zonesou2=zonesou3=m_ZonePath[m_ZonePathNext];
    //	if((m_ZonePathNext+1)<m_ZonePathCnt) zonesou2=zonesou3=m_ZonePath[m_ZonePathNext+1];
    //}
    //   if((m_ZonePathNext+2)>=m_ZonePathCnt) {
    //    m_MovePathCnt=g_MatrixMap->ZoneMoveFind(m_Unit[0].m_Kind-1,4,m_MapX,m_MapY,m_ZoneCur,zonesou1,-1,zonesou3,m_DesX,m_DesY,m_MovePath);
    //   } else if(/*m_MovePathCnt<3 &&*/ (m_ZonePathNext+2)<m_ZonePathCnt) {
    //	zonesou3=m_ZonePath[m_ZonePathNext+2];
    //	m_MovePathCnt=g_MatrixMap->ZoneMoveFind(m_Unit[0].m_Kind-1,4,m_MapX,m_MapY,m_ZoneCur,zonesou1,zonesou2,zonesou3,m_DesX,m_DesY,m_MovePath);
    //}

    m_MovePathCnt = g_MatrixMap->OptimizeMovePath(m_Unit[0].u1.s1.m_Kind - 1, 4, m_MovePathCnt, m_MovePath);
    m_MovePathCur = 0;

    m_MovePathDist = 0.0f;
    m_MovePathDistFollow = 0.0f;
    for (int i = 1; i < m_MovePathCnt; i++) {
        m_MovePathDist += GLOBAL_SCALE_MOVE * sqrt(float(m_MovePath[i - 1].Dist2(m_MovePath[i])));
    }
}

/*void CMatrixRobotAI::ZoneMoveCalcTo()
{
    ASSERT(m_ZoneCur>=0);
    m_MovePathCnt=g_MatrixMap->ZoneMoveIn(m_Unit[0].m_Kind-1,4,m_MapX,m_MapY,m_DesX,m_DesY,m_MovePath);
    m_MovePathCnt=g_MatrixMap->OptimizeMovePath(m_Unit[0].m_Kind-1,m_MovePathCnt,m_MovePath);
    m_MovePathCur=0;
}*/

float CMatrixRobotAI::CalcPathLength() {
    float dist = 0.0f;
    for (int i = m_MovePathCur; i < m_MovePathCnt - 1; i++) {
        dist += (float)sqrt(float(POW2(GLOBAL_SCALE_MOVE * m_MovePath[i].x - GLOBAL_SCALE_MOVE * m_MovePath[i + 1].x) +
                                  POW2(GLOBAL_SCALE_MOVE * m_MovePath[i].y - GLOBAL_SCALE_MOVE * m_MovePath[i + 1].y)));
    }
    return dist;
}

void CMatrixRobotAI::MoveByMovePath(int ms) {
    ASSERT(m_MovePathCnt > 0);
    ASSERT(m_MovePathCur >= 0 || m_MovePathCur < m_MovePathCnt - 1);

    float sou_x = GLOBAL_SCALE_MOVE * m_MovePath[m_MovePathCur].x + GLOBAL_SCALE_MOVE * 2.0f;  //+GLOBAL_SCALE;//+5.0f;
    float sou_y = GLOBAL_SCALE_MOVE * m_MovePath[m_MovePathCur].y + GLOBAL_SCALE_MOVE * 2.0f;  //+GLOBAL_SCALE;//+5.0f;
    float des_x =
            GLOBAL_SCALE_MOVE * m_MovePath[m_MovePathCur + 1].x + GLOBAL_SCALE_MOVE * 2.0f;  //+GLOBAL_SCALE;//+5.0f;
    float des_y =
            GLOBAL_SCALE_MOVE * m_MovePath[m_MovePathCur + 1].y + GLOBAL_SCALE_MOVE * 2.0f;  //+GLOBAL_SCALE;//+5.0f;

    // float v_x=des_x-sou_x;
    // float v_y=des_y-sou_y;

    bool globalend = (m_DesX == m_MovePath[m_MovePathCur + 1].x) && (m_DesY == m_MovePath[m_MovePathCur + 1].y);

    LowLevelMove(ms, D3DXVECTOR3(des_x, des_y, 0), true, true, globalend && (m_MovePathCur + 1) == (m_MovePathCnt - 1));

    D3DXVECTOR2 vMe = D3DXVECTOR2(m_PosX, m_PosY) - D3DXVECTOR2(sou_x, sou_y);
    D3DXVECTOR2 vPath = D3DXVECTOR2(des_x, des_y) - D3DXVECTOR2(sou_x, sou_y);

    D3DXVECTOR3 vMeProj = Vec3Projection(D3DXVECTOR3(vPath.x, vPath.y, 0), D3DXVECTOR3(vMe.x, vMe.y, 0));

    float lengthMeProjSq = D3DXVec3LengthSq(&vMeProj);
    float lengthPathSq = D3DXVec2LengthSq(&vPath);

    if ((!globalend && (lengthMeProjSq >= lengthPathSq)) ||
        (globalend && (POW2(m_PosX - des_x) + POW2(m_PosY - des_y) < 0.2f))) {
        m_MovePathCur++;
        if (m_MovePathCur >= m_MovePathCnt - 1) {
            m_ZonePathNext++;
            if (m_ZonePathNext < m_ZonePathCnt) {
                m_MovePathCur = 0;
                m_MovePathCnt = 0;
            }
            else {
                m_PosX = des_x;
                m_PosY = des_y;
                StopMoving();
            }
        }
    }

    // Если долго стоим на месте, то перерассчитать маршрут
    if ((POW2(m_MoveTestPos.x - m_PosX) + POW2(m_MoveTestPos.y - m_PosY)) > POW2(5.0f)) {
        m_MoveTestPos.x = m_PosX;
        m_MoveTestPos.y = m_PosY;
        m_MoveTestChange = g_MatrixMap->GetTime();
    }
    else if ((g_MatrixMap->GetTime() - m_MoveTestChange) > 2000) {
        m_ZonePathCnt = 0;
        m_MovePathCur = 0;
        m_MovePathCnt = 0;
    }

    RChange(MR_Matrix | MR_Rotate | MR_Pos | MR_ShadowStencil | MR_ShadowProjGeom | MR_ShadowProjTex);
}

// void CMatrixRobotAI::LowOrderStop()
//{
//	m_LowOrder=0;
//	m_ZoneDes=-1;
//	m_ZonePathCnt=0;
//	m_ZonePathNext=-1;
////	m_ZoneNear=-1;
//	m_MovePathCnt=0;
//	m_MovePathCur=0;
//}

// void CMatrixRobotAI::LowOrderMoveTo(int mx,int my)
//{
//	LowOrderStop();
//	m_DesX=mx; m_DesY=my;
//	m_LowOrder=1;
//}

// void CMatrixRobotAI::MoveToRndBuilding()
//{
//	CMatrixMapStatic * ms=g_MatrixMap->m_StaticFirstNT;
//	while(ms) {
//		if(ms->GetObjectType()==OBJECT_TYPE_BUILDING && (g_MatrixMap->Rnd(0,10)<=5) /*&& ((CMatrixBuilding*)ms)->m_Kind !=
//0*/) {
//            if(((CMatrixBuilding *)ms)->m_Side != m_Side && !((CMatrixBuilding *)ms)->m_Busy){
//                CaptureFactory(((CMatrixBuilding *)ms));
//            }
///*
//			float x=GLOBAL_SCALE*(((CMatrixBuilding *)ms)->m_MapPos.x);
//			float y=GLOBAL_SCALE*(((CMatrixBuilding *)ms)->m_MapPos.y+3);
//
//            if(((x-m_PosX)*(x-m_PosX)+(y-m_PosY)*(y-m_PosY))>100*100) {
//				int mx=int(x/GLOBAL_SCALE);
//				int my=int(y/GLOBAL_SCALE);
//
//				g_MatrixMap->PlaceFindNear(m_Unit[0].m_Kind-1,4,mx,my);
//
//                MoveTo(mx, my);
//				break;
//			}
//*/
//        }else if(ms != this && ms->GetObjectType()==OBJECT_TYPE_ROBOTAI && (g_MatrixMap->Rnd(0,10)<=5)){
//			float x=(ms->AsRobot()->m_PosX);
//			float y=(ms->AsRobot()->m_PosY+30);
//
//          if(ms->AsRobot()->m_CurrState == ROBOT_SUCCESSFULLY_BUILD &&
//          ((x-m_PosX)*(x-m_PosX)+(y-m_PosY)*(y-m_PosY))>100*100)
//          {
//				int mx=int(x/GLOBAL_SCALE);
//				int my=int(y/GLOBAL_SCALE);
//
//				g_MatrixMap->PlaceFindNear(m_Unit[0].m_Kind-1,4,mx,my);
//
//                MoveTo(mx, my);
//				break;
//			}
//
//        }
//		ms=ms->m_NextNT;
//	}
//}

bool CMatrixRobotAI::Damage(EWeapon weap, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, int attacker_side,
                            CMatrixMapStatic *attaker) {
    DTRACE();

    ASSERT(this);

    if (m_CurrState == ROBOT_DIP)
        return true;
    DCP();

    bool friendly_fire = false;
    float damagek = 0.0;
    int idx = 0;

    if (weap == WEAPON_INSTANT_DEATH)
        goto inst_death;

    friendly_fire = (attacker_side != 0) && (attacker_side == m_Side);

    damagek =
            (friendly_fire || m_Side != PLAYER_SIDE) ? 1.0f : g_MatrixMap->m_Difficulty.k_damage_enemy_to_player;
    if (friendly_fire && m_Side == PLAYER_SIDE)
        damagek = damagek * g_MatrixMap->m_Difficulty.k_friendly_fire;

    idx = Weap2Index(weap);
    if (weap == WEAPON_REPAIR) {
        m_HitPoint += friendly_fire ? g_Config.m_RobotDamages[idx].friend_damage : g_Config.m_RobotDamages[idx].damage;
        if (m_HitPoint > m_HitPointMax) {
            m_HitPoint = m_HitPointMax;
        }
        m_PB.Modify(m_HitPoint * m_MaxHitPointInversed);

        return false;
    }

#if (defined _DEBUG) && !(defined _RELDEBUG)
    if (attaker != NULL) {
        CMatrixMapStatic *ms = CMatrixMapStatic::GetFirstLogic();
        while (ms) {
            if (ms == attaker)
                break;
            ms = ms->GetNextLogic();
        }
        if (!ms)
            debugbreak();
    }

#endif
    if (!friendly_fire && attaker != NULL && attaker->IsLiveCannon() && attaker->AsCannon()->GetSide() != GetSide()) {
        if (!GetEnv()->SearchEnemy(attaker))
            GetEnv()->AddToList(attaker);

        if ((!GetEnv()->m_TargetAttack || GetEnv()->m_TargetAttack->IsCannon()) &&
            (g_MatrixMap->GetTime() - GetEnv()->m_LastHitTarget) > 4000 &&
            (g_MatrixMap->GetTime() - GetEnv()->m_TargetChange) > 1000) {
            GetEnv()->m_TargetLast = GetEnv()->m_TargetAttack;
            GetEnv()->m_TargetAttack = attaker;
            GetEnv()->m_TargetChange = g_MatrixMap->GetTime();
        }
    }

    CMatrixEffectWeapon::SoundHit(weap, pos);

    if (m_HitPoint > g_Config.m_RobotDamages[idx].mindamage) {
        float damage = damagek * float(friendly_fire ? g_Config.m_RobotDamages[idx].friend_damage
                                                     : g_Config.m_RobotDamages[idx].damage);
        if (weap == WEAPON_BIGBOOM)
            damage -= damage * m_BombProtect;
        m_HitPoint -= damage;

        if (m_HitPoint >= 0) {
            m_PB.Modify(m_HitPoint * m_MaxHitPointInversed);
        }
        else {
            m_PB.Modify(0);
        }
        if (!friendly_fire)
            m_MiniMapFlashTime = FLASH_PERIOD;

        if (FLAG(g_MatrixMap->m_Flags, MMFLAG_FLYCAM)) {
            if (attaker)
                g_MatrixMap->m_Camera.AddWarPair(this, attaker);
        }
    }

    if (weap == WEAPON_FLAMETHROWER) {
        MarkAblaze();
        m_LastDelayDamageSide = attacker_side;

        int ttl = GetAblazeTTL();
        ttl += 300;
        if (ttl > 5000)
            ttl = 5000;
        SetAblazeTTL(ttl);

        m_NextTimeAblaze = g_MatrixMap->GetTime();
    }
    else if (weap == WEAPON_LIGHTENING) {
        LowLevelStopFire();

        SwitchAnimation(ANIMATION_OFF);
        MarkShorted();
        m_LastDelayDamageSide = attacker_side;

        int ttl = GetShortedTTL();
        ttl += 500 - Float2Int(500.0f * m_LightProtect);
        int maxl = 3000 - Float2Int(3000.0f * m_LightProtect);
        if (ttl > maxl)
            ttl = maxl;

        SetShortedTTL(ttl);

        m_NextTimeShorted = g_MatrixMap->GetTime();
    }
    else {
        m_LastDelayDamageSide = 0;
    }

    if (m_HitPoint > 50) {
        if (weap != WEAPON_ABLAZE && weap != WEAPON_SHORTED && weap != WEAPON_LIGHTENING &&
            weap != WEAPON_FLAMETHROWER) {
            CMatrixEffect::CreateExplosion(pos, ExplosionRobotHit);
        }
    }
    else if (m_HitPoint > 0) {
    }
    else {  // if(m_HitPoint > 0){

        if (attacker_side != 0 && !friendly_fire) {
            g_MatrixMap->GetSideById(attacker_side)->IncStatValue(STAT_ROBOT_KILL);
        }

    inst_death:;

        if (IsInPosition()) {
            g_MatrixMap->ShowPortrets();
        }

        for (int nC = 0; nC < m_WeaponsCnt; ++nC) {
            if (m_Weapons[nC].IsEffectPresent() && m_Weapons[nC].GetWeaponType() == WEAPON_BIGBOOM) {
                if (GetSide() == PLAYER_SIDE) {
                    //                    BigBoom(nC);
                }
                else {
                    float danager = 0.0f;
                    CMatrixMapStatic *ms = CMatrixMapStatic::GetFirstLogic();
                    for (; ms; ms = ms->GetNextLogic()) {
                        if (ms != this && ms->IsLiveRobot()) {
                            if ((POW2(ms->AsRobot()->m_PosX - m_PosX) + POW2(ms->AsRobot()->m_PosY - m_PosY)) <
                                POW2(m_Weapons[nC].GetWeaponDist() * 1.0f)) {
                                if (ms->GetSide() == GetSide())
                                    danager -= ms->AsRobot()->GetStrength();
                                else
                                    danager += ms->AsRobot()->GetStrength();
                            }
                        }
                        else if (ms->IsLiveCannon() && ms->AsCannon()->m_CurrState != CANNON_UNDER_CONSTRUCTION) {
                            if ((POW2(ms->AsCannon()->m_Pos.x - m_PosX) + POW2(ms->AsCannon()->m_Pos.y - m_PosY)) <
                                POW2(m_Weapons[nC].GetWeaponDist() * 1.0f)) {
                                if (ms->GetSide() == GetSide())
                                    danager -= ms->AsCannon()->GetStrength();
                                else
                                    danager += ms->AsCannon()->GetStrength();
                            }
                        }
                    }

                    if (danager > 0.0f) {
                        BigBoom(nC);
                    }
                }
                break;
            }
        }

        ResetMustDie();  // to avoid MUST_DIE flag checking... robot already dieing...

        //    } else {
        DCP();

        ReleaseMe();
        DCP();
        CMatrixEffect::CreateExplosion(GetGeoCenter(), ExplosionRobotBoom, true);
        CMatrixEffect::CreateLandscapeSpot(NULL, *(D3DXVECTOR2 *)&GetGeoCenter(), FSRND(M_PI), 4 + FRND(2),
                                           SPOT_VORONKA);

        DCP();
        ClearSelection();

        DCP();
        m_ShadowType = SHADOW_OFF;
        RChange(MR_ShadowProjGeom | MR_ShadowStencil);
        RNeed(MR_ShadowProjGeom | MR_ShadowStencil);
        DCP();
        for (int i = 0; i < m_UnitCnt; ++i) {
            m_Unit[i].PrepareForDIP();
        }

        DCP();

        if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_ANTIGRAVITY && m_Unit[0].m_Type == MRT_CHASSIS) {
            m_ChassisData.u1.s1.m_LStream->Release();
            m_ChassisData.u1.s1.m_RStream->Release();
        }
        DCP();

        bool cstay = FRND(1) < 0.5f;
        if (IsDisableManual()) {
            if (GetBase()) {
                if (m_CurrState == ROBOT_BASE_MOVEOUT || m_CurrState == ROBOT_IN_SPAWN)
                    GetBase()->ResetSpawningBot();  // killing bo always close base
                GetBase()->Close();
            }
            cstay = false;
        }

        if (FLAG(m_ObjectState, ROBOT_FLAG_ONWATER))
            cstay = false;

        DCP();
        SwitchAnimation(ANIMATION_OFF);
        m_CurrState = ROBOT_DIP;

        bool onair = false;

        float z = g_MatrixMap->GetZ(m_Unit[0].m_Matrix._41, m_Unit[0].m_Matrix._42);
        if (z + GetRadius() < m_Unit[0].m_Matrix._43) {
            // on air
            onair = true;
        }
        DCP();

        m_Unit[0].u1.s2.m_TTL = FRND(3000) + 2000;
        m_Unit[0].u1.s2.m_Pos.x = m_Unit[0].m_Matrix._41;
        m_Unit[0].u1.s2.m_Pos.y = m_Unit[0].m_Matrix._42;
        m_Unit[0].u1.s2.m_Pos.z = m_Unit[0].m_Matrix._43;
        DCP();

        if (cstay && !onair) {
            m_Unit[0].u1.s2.m_dp = 0;
            m_Unit[0].u1.s2.m_dy = 0;
            m_Unit[0].u1.s2.m_dr = 0;
            m_Unit[0].u1.s2.m_Velocity = D3DXVECTOR3(0, 0, 0);
        }
        else {
            m_Unit[0].u1.s2.m_dp = FSRND(0.0005f);
            m_Unit[0].u1.s2.m_dy = FSRND(0.0005f);
            m_Unit[0].u1.s2.m_dr = FSRND(0.0005f);
            if (onair) {
                m_Unit[0].u1.s2.m_Velocity = D3DXVECTOR3(FSRND(0.08f), FSRND(0.08f), FSRND(0.1f));
            }
            else {
                m_Unit[0].u1.s2.m_Velocity = D3DXVECTOR3(0, 0, 0.1f);
            }
        }

        DCP();
        m_Unit[0].Smoke().effect = NULL;
        CMatrixEffect::CreateSmoke(&m_Unit[0].Smoke(), m_Unit[0].u1.s2.m_Pos, m_Unit[0].u1.s2.m_TTL + 100000, 1600, 80, 0xCF303030,
                                   false, 1.0f / 30.0f);

        for (int i = 1; i < m_UnitCnt; ++i) {
            m_Unit[i].Smoke().effect = NULL;
            if (m_Unit[i].m_Type == MRT_ARMOR) {
                m_Unit[i].u1.s2.m_TTL = 0;
                continue;
            }

            m_Unit[i].u1.s2.m_dp = FSRND(0.005f);
            m_Unit[i].u1.s2.m_dy = FSRND(0.005f);
            m_Unit[i].u1.s2.m_dr = FSRND(0.005f);
            if (onair) {
                m_Unit[i].u1.s2.m_Velocity = D3DXVECTOR3(FSRND(0.08f), FSRND(0.08f), FSRND(0.1f));
            }
            else {
                m_Unit[i].u1.s2.m_Velocity = D3DXVECTOR3(FSRND(0.08f), FSRND(0.08f), 0.1f);
            }

            m_Unit[i].u1.s2.m_TTL = FRND(3000) + 2000;
            m_Unit[i].u1.s2.m_Pos.x = m_Unit[i].m_Matrix._41;
            m_Unit[i].u1.s2.m_Pos.y = m_Unit[i].m_Matrix._42;
            m_Unit[i].u1.s2.m_Pos.z = m_Unit[i].m_Matrix._43;

            CMatrixEffect::CreateSmoke(&m_Unit[i].Smoke(), m_Unit[i].u1.s2.m_Pos, m_Unit[i].u1.s2.m_TTL + 100000, 1600, 80,
                                       0xCF303030, false, 1.0f / 30.0f);
        }

        DCP();
        return true;
    }
    return false;
}

#ifdef _DEBUG
void CMatrixRobotAI::Draw(void) {
    // if(m_CurrState != ROBOT_DIP) {
    //    CPoint tp;
    //    if(!GetMoveToCoords(tp)) {
    //        tp.x=m_MapX; tp.y=m_MapY;
    //    }

    //    D3DXVECTOR3 v1,v2,v3,v4;
    //    v1.x=tp.x*GLOBAL_SCALE_MOVE; v1.y=tp.y*GLOBAL_SCALE_MOVE; v1.z=g_MatrixMap->GetZ(v1.x,v1.y)+1.0f;
    //    v2.x=(tp.x+4)*GLOBAL_SCALE_MOVE; v2.y=tp.y*GLOBAL_SCALE_MOVE; v2.z=g_MatrixMap->GetZ(v2.x,v2.y)+1.0f;
    //    v3.x=(tp.x+4)*GLOBAL_SCALE_MOVE; v3.y=(tp.y+4)*GLOBAL_SCALE_MOVE; v3.z=g_MatrixMap->GetZ(v3.x,v3.y)+1.0f;
    //    v4.x=(tp.x)*GLOBAL_SCALE_MOVE; v4.y=(tp.y+4)*GLOBAL_SCALE_MOVE; v4.z=g_MatrixMap->GetZ(v4.x,v4.y)+1.0f;

    //    CHelper::Create(1)->Triangle(v1,v2,v3,0x8000ff00);
    //    CHelper::Create(1)->Triangle(v1,v3,v4,0x8000ff00);
    //}

    // if(m_CurrState != ROBOT_DIP) {
    // if(m_ZonePathCnt>0) {
    //  for(int i=1;i<m_ZonePathCnt;i++) {
    //   D3DXVECTOR3 vfrom,vto;
    //   vfrom.x=GLOBAL_SCALE_MOVE*float(g_MatrixMap->m_RN.m_Zone[m_ZonePath[i-1]].m_Center.x)+GLOBAL_SCALE_MOVE/2;
    //   vfrom.y=GLOBAL_SCALE_MOVE*float(g_MatrixMap->m_RN.m_Zone[m_ZonePath[i-1]].m_Center.y)+GLOBAL_SCALE_MOVE/2;
    //   vfrom.z=g_MatrixMap->GetZ(vfrom.x,vfrom.y)+50.0f;

    //   vto.x=GLOBAL_SCALE_MOVE*float(g_MatrixMap->m_RN.m_Zone[m_ZonePath[i]].m_Center.x)+GLOBAL_SCALE_MOVE/2;
    //   vto.y=GLOBAL_SCALE_MOVE*float(g_MatrixMap->m_RN.m_Zone[m_ZonePath[i]].m_Center.y)+GLOBAL_SCALE_MOVE/2;
    //   vto.z=g_MatrixMap->GetZ(vto.x,vto.y)+50.0f;

    //            CHelper::Create(1)->Cone(vfrom,vto,0.5f,0.5f,0xffff0000,0xffff0000,6);
    //   CHelper::Create(1)->Cone(vto+(vfrom-vto)*0.1f,vto,1.5f,0.5f,0xffff0000,0xffffff00,6);
    //   CHelper::Create(1)->Cone(vto,vto-D3DXVECTOR3(0.0f,0.0f,50.0f),0.5f,0.5f,0xffff0000,0xffff0000,6);
    //  }
    // }

    // if(m_MovePathCnt>0) {
    //  for(int i=1;i<m_MovePathCnt;i++) {
    //   D3DXVECTOR3 vfrom,vto;
    //   vfrom.x=GLOBAL_SCALE_MOVE*m_MovePath[i-1].x+GLOBAL_SCALE_MOVE/2;
    //   vfrom.y=GLOBAL_SCALE_MOVE*m_MovePath[i-1].y+GLOBAL_SCALE_MOVE/2;
    //   vfrom.z=g_MatrixMap->GetZ(vfrom.x,vfrom.y)+2.0f;//+GLOBAL_SCALE_MOVE;

    //   vto.x=GLOBAL_SCALE_MOVE*m_MovePath[i].x+GLOBAL_SCALE_MOVE/2;
    //   vto.y=GLOBAL_SCALE_MOVE*m_MovePath[i].y+GLOBAL_SCALE_MOVE/2;
    //   vto.z=g_MatrixMap->GetZ(vto.x,vto.y)+2.0f;//+GLOBAL_SCALE_MOVE;

    //   CHelper::Create(1)->Cone(vfrom,vto,0.5f,0.5f,0xffffffff,0xffff0000,6);
    //  }
    // }

    //}
    CMatrixRobot::Draw();
}
#endif

void CMatrixRobotAI::RobotSpawn(CMatrixBuilding *pBase) {
    DTRACE();

    SetBase(pBase);
    pBase->SetSpawningBot();

    CMatrixSideUnit *side = g_MatrixMap->GetSideById(pBase->m_Side);

    SETFLAG(m_ObjectState, ROBOT_FLAG_DISABLE_MANUAL);
    if (IsCrazy()) {
        InitMaxHitpoint(1000000.0f);
        m_Team = 0;

        SETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);

        side->AssignPlace(this, g_MatrixMap->GetRegion(CPoint(int(pBase->m_Pos.x / GLOBAL_SCALE_MOVE),
                                                              int(pBase->m_Pos.y / GLOBAL_SCALE_MOVE))));
        if (GetEnv()->m_Place < 0)
            side->PGOrderAttack(
                    side->RobotToLogicGroup(this),
                    CPoint(int(pBase->m_Pos.x / GLOBAL_SCALE_MOVE), int(pBase->m_Pos.y / GLOBAL_SCALE_MOVE)), NULL);
        else
            side->PGOrderAttack(side->RobotToLogicGroup(this), g_MatrixMap->m_RN.GetPlace(GetEnv()->m_Place)->m_Pos,
                                NULL);

        RESETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);
    }
    else {
        if (side->m_Id != PLAYER_SIDE) {
            m_Team = side->ClacSpawnTeam(g_MatrixMap->GetRegion(CPoint(Float2Int(pBase->m_Pos.x / GLOBAL_SCALE_MOVE),
                                                                       Float2Int(pBase->m_Pos.x / GLOBAL_SCALE_MOVE))),
                                         m_Unit[0].u1.s1.m_Kind - 1);
            /*        int minr=side->m_Team[m_Team].m_RobotCnt;
                for(int i=1;i<side->m_TeamCnt;i++) {
                    if(side->m_Team[i].m_RobotCnt<minr) {
                        minr=side->m_Team[i].m_RobotCnt;
                        m_Team=i;
                    }
                    }*/
        }
        else {
            m_Team = 0;

            SETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);

            side->AssignPlace(this, g_MatrixMap->GetRegion(CPoint(int(pBase->m_Pos.x / GLOBAL_SCALE_MOVE),
                                                                  int(pBase->m_Pos.y / GLOBAL_SCALE_MOVE))));
            if (GetEnv()->m_Place < 0)
                side->PGOrderAttack(
                        side->RobotToLogicGroup(this),
                        CPoint(int(pBase->m_Pos.x / GLOBAL_SCALE_MOVE), int(pBase->m_Pos.y / GLOBAL_SCALE_MOVE)), NULL);
            else
                side->PGOrderAttack(side->RobotToLogicGroup(this), g_MatrixMap->m_RN.GetPlace(GetEnv()->m_Place)->m_Pos,
                                    NULL);

            RESETFLAG(g_MatrixMap->m_Flags, MMFLAG_SOUND_ORDER_ATTACK_DISABLE);
        }
        if (m_Side != 0)
            g_MatrixMap->GetSideById(m_Side)->IncStatValue(STAT_ROBOT_BUILD);
    }

    m_CurrState = ROBOT_IN_SPAWN;
    pBase->Open();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrixRobotAI::RotateHull(const D3DXVECTOR3 &dest) {
    D3DXVECTOR3 destDirN(0, 0, 0), destDir(0, 0, 0);

    destDir = dest - D3DXVECTOR3(m_PosX, m_PosY, 0);
    if (this == (CMatrixRobotAI *)g_MatrixMap->GetPlayerSide()->GetArcadedObject() &&
        D3DXVec3LengthSq(&destDir) < MIN_ROT_DIST * MIN_ROT_DIST) {
        destDirN = m_HullForward;
    }
    else {
        destDir.z = 0;
        D3DXVec3Normalize(&destDirN, &destDir);
    }
    float cos3 = m_HullForward.x * destDirN.x + m_HullForward.y * destDirN.y;

    float angle3 = (float)acos(cos3);

    float cos1 = m_HullForward.x * m_Forward.x + m_HullForward.y * m_Forward.y;
    float angle1 = (float)acos(cos1);

    D3DXVECTOR3 vec(m_Unit[0].m_Matrix._11, m_Unit[0].m_Matrix._12, 0);
    D3DXVec3Normalize(&vec, &vec);
    float cos2 = m_HullForward.x * vec.x + m_HullForward.y * vec.y;

    if (this == (CMatrixRobotAI *)g_MatrixMap->GetPlayerSide()->GetArcadedObject()) {
        bool dchanged = false;
        if (fabs(angle3) >= GRAD2RAD(HULL_ROT_S_ANGL)) {
            float cosmos = destDirN.x * (-m_HullForward.y) + destDirN.y * (m_HullForward.x);
            if (cosmos < 0 && m_PrevTurnSign == 1) {
                m_PrevTurnSign = 0;
                dchanged = true;
            }
            else if (cosmos > 0 && m_PrevTurnSign == 0) {
                m_PrevTurnSign = 1;
                dchanged = true;
            }
        }

        m_HullRotCnt++;

        if (dchanged && m_HullRotCnt >= HULL_ROT_TAKTS) {
            PlayHullSound();
            m_HullRotCnt = 0;
        }
    }

    if (fabs(angle3) >= GRAD2RAD(160)) {
        auto tmp = m_HullForward + Vec3Truncate((destDirN - m_HullForward), m_maxHullSpeed * 3);
        D3DXVec3Normalize(&m_HullForward, &tmp);
    }
    else {
        auto tmp = m_HullForward + Vec3Truncate((destDirN - m_HullForward), m_maxHullSpeed);
        D3DXVec3Normalize(&m_HullForward, &tmp);
    }

    if (fabs(angle1) >= MAX_HULL_ANGLE) {
        D3DXMATRIX rotMat;
        if (cos2 > 0) {
            D3DXMatrixRotationZ(&rotMat, -(MAX_HULL_ANGLE));
        }
        else {
            D3DXMatrixRotationZ(&rotMat, MAX_HULL_ANGLE);
        }
        D3DXVec3TransformCoord(&m_HullForward, &m_Forward, &rotMat);
    }

    RChange(MR_Matrix | MR_ShadowProjTex | MR_ShadowStencil);
}

bool CMatrixRobotAI::RotateRobot(const D3DXVECTOR3 &dest, float *rotateangle) {
    RChange(MR_Matrix | MR_ShadowProjTex | MR_ShadowStencil);

    // SwitchAnimation(ANIMATION_ROTATE);
    // m_RotSpeed = m_GroupSpeed;

    D3DXVECTOR3 destDirN(0, 0, 0), destDir(0, 0, 0), forward(0, 0, 0);

    float rot_speed = m_maxRotationSpeed * m_SyncMul;

    destDir = dest - D3DXVECTOR3(m_PosX, m_PosY, 0);
    destDir.z = 0;
    D3DXVec3Normalize(&destDirN, &destDir);
    forward = m_Forward;
    forward.z = 0;
    D3DXVec3Normalize(&forward, &forward);

    float cos1 = forward.x * destDirN.x + forward.y * destDirN.y;
    if (cos1 > 1.0f)
        cos1 = 1.0f;
    else if (cos1 < -1.0f)
        cos1 = -1.0f;
    float angle1 = (float)acos(cos1);

    // DM(L"Rotate",std::wstring().Format(L"cos1=<f> angle1=<f>",cos1,angle1).Get());

    if (rotateangle)
        *rotateangle = angle1;

    //    D3DXVECTOR3 vec(m_Unit[0].m_Matrix._11, m_Unit[0].m_Matrix._12, 0);
    D3DXVECTOR3 vec(forward.y, -forward.x, 0);
    D3DXVec3Normalize(&vec, &vec);
    float rotDir = destDirN.x * vec.x + destDirN.y * vec.y;

    // TruncFloat(
    if (angle1 /**ToGrad*/ <= rot_speed /*&& rotDir < 1*/) {
        m_Forward = destDirN;
        m_Velocity = m_Forward * D3DXVec3Length(&m_Velocity);
        return true;
    }

    if (fabs(angle1 - rot_speed) < 0.001) {
        forward = destDirN;
    }
    else if (rotDir > 0) {
        D3DXMATRIX rotMat;
        D3DXMatrixRotationZ(&rotMat, -rot_speed);
        D3DXVec3TransformCoord(&forward, &forward, &rotMat);
    }
    else {
        D3DXMATRIX rotMat;
        D3DXMatrixRotationZ(&rotMat, rot_speed);
        D3DXVec3TransformCoord(&forward, &forward, &rotMat);
    }

    /*    if(fabs(angle1) > GRAD2RAD(100)){
            D3DXMATRIX rotMat;
            if(rotDir > 0){
                D3DXMatrixRotationZ(&rotMat, -m_maxRotationSpeed);
            }else{
                D3DXMatrixRotationZ(&rotMat, m_maxRotationSpeed);
            }
            D3DXVec3TransformCoord(&forward, &forward, &rotMat);
        }else{
            D3DXVec3Normalize(&forward, &(forward + Vec3Truncate((destDirN - forward), m_maxRotationSpeed)));
        }*/
    m_Forward = forward;

    // CHelper::Create(1, 0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 0), D3DXVECTOR3(m_PosX, m_PosY, 0)+ destDirN*100,
    // 0xffff0000, 0xffff0000); CHelper::Create(1, 0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 0), D3DXVECTOR3(m_PosX, m_PosY,
    // 0)+ m_Forward*100, 0xffffffff, 0xffffffff);

    //    cos1 = forward.x * destDirN.x + forward.y * destDirN.y;
    //    if(cos1>1.0f) cos1=1.0f;
    //    else if(cos1<-1.0f) cos1=-1.0f;
    //    angle1 = (float)acos(cos1);

    m_Velocity = forward * D3DXVec3Length(&m_Velocity);

    // static float a = 360;
    // if(TruncFloat(angle1*ToGrad) < a)
    //    a = TruncFloat(angle1*ToGrad);

    // if(a == 0)
    //    a = 360;

    // g_MatrixMap->m_DI.T(L"angle", std::wstring(a));

    //    if(angle1*ToGrad <= 1/* && rotDir < 1*/){
    //        return true;
    //	}

    return false;
}

bool CMatrixRobotAI::Seek(const D3DXVECTOR3 &dest, bool &rotate, bool end_path, bool back) {
    float rangle = 0.0;
    rotate = true;
    if (m_CurrState != ROBOT_BASE_MOVEOUT && !back) {
        if (RotateRobot(dest, &rangle)) {
            rotate = false;
            rangle = 0.0f;
        }
    }
    else
        rotate = false;

    D3DXVECTOR3 forward = m_Forward;
    forward.z = 0;
    D3DXVec3Normalize(&forward, &forward);

    if (back)
        forward = -forward;

    D3DXVECTOR3 destDir = dest - D3DXVECTOR3(m_PosX, m_PosY, 0);
    float destLength = D3DXVec3Length(&destDir);

    if (m_CollAvoid.x != 0.0f || m_CollAvoid.y != 0.0f) {
        ASSERT(1);
    }

    if (m_GroupSpeed <= 0.001f)
        m_GroupSpeed = m_maxSpeed;

    float k = FLAG(m_ObjectState, ROBOT_FLAG_ONWATER) ? m_SpeedWaterCorr : 1.0f;

    float slope;
    if (back) {
        D3DXVECTOR3 f(-(*(D3DXVECTOR3 *)&m_Core->m_Matrix._21));
        auto tmp = D3DXVECTOR3(0, 0, 1);
        slope = D3DXVec3Dot(&tmp, &f);
    }
    else {
        auto tmp = D3DXVECTOR3(0, 0, 1);
        slope = D3DXVec3Dot(&tmp, (D3DXVECTOR3 *)&m_Core->m_Matrix._21);
    }

    if (slope >= 0) {
        if (slope >= m_SpeedSlopeCorrUp)
            k = 0;
        else {
            k *= LERPFLOAT(slope / m_SpeedSlopeCorrUp, 1.0, 0.0f);
        }
    }
    else if (slope < 0) {
        k *= LERPFLOAT(-slope, 1.0f, m_SpeedSlopeCorrDown);
    }

    if ((destLength - std::min(m_GroupSpeed, m_ColSpeed)) < 0.001f) {
        m_Velocity = destDir * k;
        m_Speed = destLength * k;
    }
    else {
        m_Speed = k * std::min(m_GroupSpeed, m_ColSpeed);
        if (end_path) {
            float t = std::min(1.0f, destLength / 20.0f);
            t *= std::min(1.0f, (1.0f - rangle / pi_f));
            m_Speed = m_Speed * t;
        }
        m_Velocity = forward * m_Speed;
    }
    return true;
}

// bool CMatrixRobotAI::Seek(const D3DXVECTOR3 &dest,bool end_path)
//{
//	if(m_GroupSpeed == 0)
//        m_GroupSpeed = m_maxSpeed;
//    //D3DXVECTOR3 oldv = D3DXVECTOR3(m_PosX, m_PosY, 0) + m_Velocity;
//    //D3DXVECTOR3 ovel = m_Velocity;
//
//    D3DXVECTOR3 desired_velocity(0,0,0), SteeringN(0,0,0), destDir(0,0,0), destDirN(0,0,0), destRot(0,0,0);
//	bool accelerating = false;
//
//	destDir = dest - D3DXVECTOR3(m_PosX, m_PosY, 0);
//    float destLength=D3DXVec3Length(&destDir);
//
//    if(m_CollAvoid.x != 0 || m_CollAvoid.y != 0){
//        destDirN = m_CollAvoid;
//        //CHelper::Create(1, 0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50), D3DXVECTOR3(m_PosX, m_PosY, 0) +
//        D3DXVECTOR3(m_CollAvoid.x*100.0f, m_CollAvoid.y*100.0f, 50),0xffffffff, 0xffff0000);
//    }else{
//        D3DXVec3Normalize(&destDirN, &destDir);
//}
//
//
//    destRot = D3DXVECTOR3(m_PosX, m_PosY, 0) + destDirN * destLength;
//    if(m_CurrState == ROBOT_BASE_MOVEOUT){
//		desired_velocity = m_Forward;
//		accelerating = true;
//    }else{
//		desired_velocity = destDirN;
//		accelerating = false;
//    }
//
//    m_GroupSpeed=m_maxSpeed;
//
//    desired_velocity *= /*m_maxSpeed*/m_GroupSpeed;
//    desired_velocity.z = 0;
//
//    D3DXVECTOR3 steering = desired_velocity - m_Velocity;
//	D3DXVECTOR3 accel = Vec3Truncate(steering, m_maxForce) / m_RobotMass;
//
//	D3DXVec3Normalize(&SteeringN, &(m_Velocity + accel));
//
//    float rangle=0.0f;
//
//	m_Velocity.z = 0;
//    if(m_CurrState == ROBOT_BASE_MOVEOUT/* || RotateRobot(destRot)*/){
//        m_Velocity = Vec3Truncate(m_Velocity + accel, /*m_maxSpeed*/m_GroupSpeed);
//    }else{
//        float mspeed=m_GroupSpeed;
//
//        bool rot = false;
//        float len = CalcPathLength();//D3DXVec3Length(&destDir);
//        if(len > COLLIDE_BOT_R*3){
//            if((m_CollAvoid.x == 0 && m_CollAvoid.y == 0)){
//                if(RotateRobot(dest,&rangle)){
//                    rot = true;
//                }else{
//                    m_Velocity = Vec3Truncate(m_Velocity + Vec3Truncate((m_Forward - m_Velocity),
//                    m_maxForce)/m_RobotMass, m_GroupSpeed); if(end_path) m_Velocity = Vec3Truncate(m_Velocity,
//                    max(0.01f,m_GroupSpeed*min(1.0f,destLength/(GLOBAL_SCALE_MOVE*1.5f))));
//                }
//            }
//        }else if(len > COLLIDE_BOT_R*0.5f){
//            rot = RotateRobot(dest);
//            if(!rot){
//                return false;
//            }
//        }else{
//            rot = true;
//        }
//
//        if(rot){
//            if(!end_path) m_Velocity = Vec3Truncate(m_Velocity + accel, /*m_maxSpeed*/m_GroupSpeed);
//            else {
////if(rangle<-pi || rangle>pi) {
////    ASSERT(1);
////}
////DM(L"Velocity",std::wstring().Format(L"mul=<f> Vel=<f>,<f>,<f>",rangle,m_Velocity.x,m_Velocity.y,m_Velocity.z).Get());
//                rangle=1.0f+(fabs(rangle)/pi)*2.0f;
//                m_Velocity = Vec3Truncate(m_Velocity + accel,
//                max(0.1f,m_GroupSpeed*min(1.0f,destLength/(GLOBAL_SCALE_MOVE*2.0f*rangle))));
//            }
//        }
//    }
////
//    //D3DXVECTOR3 newv = D3DXVECTOR3(m_PosX, m_PosY, 0) + m_Velocity;
//
//    //D3DXVECTOR3 dv = newv - oldv;
//    //dv *= m_SyncMul;
//    //m_Velocity = ovel + dv;
//
//
////
//    m_Speed = D3DXVec3Length(&m_Velocity);
//    D3DXVECTOR3 vvv(0,0,0);
//    D3DXVec3Normalize(&vvv, &m_Velocity);
//
//
//    if (!IS_ZERO_VECTOR(vvv)){
//	    m_Forward = vvv;
//    }
//
//    return true;
//
//}

void CMatrixRobotAI::LowLevelMove(int ms, const D3DXVECTOR3 &dest, bool robot_coll, bool obst_coll, bool end_path,
                                  bool back) {
    bool rotate = false;
    bool vel = Seek(dest, rotate, end_path, back);

    m_Cols = 0;
    D3DXVECTOR3 r(0, 0, 0), o(0, 0, 0), result_coll(0, 0, 0), tmp_vel(0, 0, 0);

    if (!vel) {
        tmp_vel = m_Velocity;
        m_Velocity = D3DXVECTOR3(0, 0, 0);
    }

    D3DXVECTOR3 genetic_mutated_velocity = m_Velocity * m_SyncMul;

    if (robot_coll) {
        r = RobotToObjectCollision(genetic_mutated_velocity, ms);
#ifdef _SUB_BUG
        CHelper::Create(1, 0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50),
                                    D3DXVECTOR3(m_PosX, m_PosY, 0) + D3DXVECTOR3(r.x * 100, r.y * 100, 50), 0xffffffff,
                                    0xff00ff00);
#endif
        result_coll = r;
    }
    if (obst_coll) {
        o = SphereRobotToAABBObstacleCollision(r, genetic_mutated_velocity);
        if (!m_Cols) {
            WallAvoid(o, dest);
        }
#ifdef _SUB_BUG
        CHelper::Create(1, 0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50),
                                    D3DXVECTOR3(m_PosX, m_PosY, 0) + D3DXVECTOR3(o.x * 100, o.y * 100, 50), 0xffffffff,
                                    0xff0000ff);
#endif
        result_coll = r + o;
    }

#ifdef _SUB_BUG
    if (this == g_MatrixMap->GetPlayerSide()->GetArcadedObject()) {
        g_MatrixMap->m_DI.T(L"pos_x", std::wstring(m_PosX));
        g_MatrixMap->m_DI.T(L"pos_y", std::wstring(m_PosY));
        g_MatrixMap->m_DI.T(L"geo_x", std::wstring(m_Core->m_GeoCenter.x));
        g_MatrixMap->m_DI.T(L"geo_y", std::wstring(m_Core->m_GeoCenter.y));
        g_MatrixMap->m_DI.T(L"m_Velocity", std::wstring(D3DXVec3Length(&m_Velocity)));
        //        g_MatrixMap->m_DI.T(L"genetic_mutated_velocity", std::wstring(D3DXVec3Length(&genetic_mutated_velocity)));
        g_MatrixMap->m_DI.T(L"result_coll", std::wstring(D3DXVec3Length(&result_coll)));
    }
    CHelper::Create(1, 0)->Line(
            D3DXVECTOR3(m_PosX, m_PosY, 50),
            D3DXVECTOR3(m_PosX, m_PosY, 0) + D3DXVECTOR3(m_Velocity.x * 100, m_Velocity.y * 100, 50), 0xffffffff,
            0xffffff00);
// CHelper::Create(1,0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50), D3DXVECTOR3(m_PosX, m_PosY, 0) + D3DXVECTOR3(100,
// 0,50),0xffffffff, 0xff00ff00); CHelper::Create(1,0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50), D3DXVECTOR3(m_PosX, m_PosY,
// 0) + D3DXVECTOR3(0, 100,50),0xffffffff, 0xffff00ff); CHelper::Create(1,0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50),
// D3DXVECTOR3(m_PosX, m_PosY, 0) + D3DXVECTOR3(result_coll.x*100, result_coll.y*100,50),0xffffffff, 0xffff0000);
#endif

    m_MovePathDistFollow += sqrt(
            float(POW2(genetic_mutated_velocity.x + result_coll.x) + POW2(genetic_mutated_velocity.y + result_coll.y)));

    RChange(MR_Matrix | MR_ShadowProjGeom);

    m_PosX += genetic_mutated_velocity.x + result_coll.x;
    m_PosY += genetic_mutated_velocity.y + result_coll.y;

    if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_PNEUMATIC) {
        if (rotate || result_coll.x != 0 || result_coll.y != 0 || GetColsWeight2() ||
            (end_path && (POW2(dest.x - m_PosX) + POW2(dest.y - m_PosY)) < POW2(GLOBAL_SCALE_MOVE))) {
            if (!FLAG(m_ObjectState, ROBOT_FLAG_COLLISION)) {
                SETFLAG(m_ObjectState, ROBOT_FLAG_COLLISION);
            }
        }
        else {
            if (FLAG(m_ObjectState, ROBOT_FLAG_COLLISION)) {
                RESETFLAG(m_ObjectState, ROBOT_FLAG_COLLISION);
                FirstLinkPneumatic();
            }
        }
    }

    // DM(L"RC",std::wstring().Format(L"rotate=<i> result_coll=<i> GetColsWeight2=<i> end_path=<i> ROBOT_FLAG_COLLISION=<i>",
    //        int(rotate),
    //        int(result_coll.x != 0 || result_coll.y != 0),
    //        int(GetColsWeight2()),
    //        (end_path && (POW2(dest.x-m_PosX)+POW2(dest.y-m_PosY))<POW2(GLOBAL_SCALE_MOVE)),
    //        FLAG(m_RobotFlags,ROBOT_FLAG_COLLISION)
    //        ).Get());

    //    m_MovePathDistFollow+=sqrt(float(POW2(genetic_mutated_velocity.x +
    //    result_coll.x)+POW2(genetic_mutated_velocity.y + result_coll.y)));

    //    m_PosX += genetic_mutated_velocity.x + result_coll.x;
    //    m_PosY += genetic_mutated_velocity.y + result_coll.y;

    if (!vel) {
        m_Velocity = tmp_vel;
    }
    JoinToGroup();
}

void CMatrixRobotAI::LowLevelDecelerate(int ms, bool robot_coll, bool obst_coll) {
    DTRACE();

    Decelerate();
    m_Cols = 0;
    D3DXVECTOR3 r(0, 0, 0), o(0, 0, 0), result_coll(0, 0, 0);

    D3DXVECTOR3 genetic_mutated_velocity = m_Velocity * m_SyncMul;

    if (robot_coll) {
        r = RobotToObjectCollision(genetic_mutated_velocity, ms);
        result_coll = r;
    }
    if (obst_coll) {
        o = SphereRobotToAABBObstacleCollision(r, genetic_mutated_velocity);
        result_coll = r + o;
    }

    if (result_coll.x != 0 || result_coll.y != 0) {
        SETFLAG(m_ObjectState, ROBOT_FLAG_COLLISION);
    }
    else {
        RESETFLAG(m_ObjectState, ROBOT_FLAG_COLLISION);
        if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_PNEUMATIC) {
            FirstLinkPneumatic();
        }
    }

    m_PosX += genetic_mutated_velocity.x + result_coll.x;
    m_PosY += genetic_mutated_velocity.y + result_coll.y;
    JoinToGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrixRobotAI::Decelerate() {
    m_Velocity *= DECELERATION_FORCE;
    m_Speed = D3DXVec3Length(&m_Velocity);
    D3DXVECTOR3 vvv(0, 0, 0);
    D3DXVec3Normalize(&vvv, &m_Velocity);
    if (!IS_ZERO_VECTOR(vvv))
        m_Forward = vvv;
}

#if (defined _DEBUG) && !(defined _RELDEBUG)
void HelperT(D3DXVECTOR2 from, D3DXVECTOR2 to) {
    return;
    CHelper::Create(1, 0)->Line(D3DXVECTOR3(from.x, from.y, 2), D3DXVECTOR3(to.x, to.y, 2), 0xffffff00, 0xffffff00);

    auto tmp = to - from;
    D3DXVECTOR3 n{tmp.x, tmp.y, 0};
    D3DXVec3Normalize(&n, &n);

    CHelper::Create(1, 0)->Line(D3DXVECTOR3(to.x - n.y * 1.5f, to.y + n.x * 1.5f, 2),
                                D3DXVECTOR3(to.x + n.y * 1.5f, to.y - n.x * 1.5f, 2), 0xffffff00, 0xffffff00);
}
#endif

D3DXVECTOR3 CMatrixRobotAI::SphereRobotToAABBObstacleCollision(D3DXVECTOR3 &corr, const D3DXVECTOR3 &vel) {
    D3DXVECTOR2 robot_pos;
    D3DXVECTOR3 vCorrTotal(0, 0, 0);

    robot_pos.x = m_PosX + corr.x + vel.x;
    robot_pos.y = m_PosY + corr.y + vel.y;

    D3DXVECTOR3 oldpos = D3DXVECTOR3(robot_pos.x, robot_pos.y, 0);

    CPoint check_cell;

    int calc_for_x = -COLLIDE_FIELD_R * 2;
    int calc_for_y = -COLLIDE_FIELD_R * 2;
    BYTE corners[COLLIDE_FIELD_R + COLLIDE_FIELD_R][COLLIDE_FIELD_R + COLLIDE_FIELD_R];
    memset(corners, -1, sizeof(corners));

    for (int cnt = 0; cnt < 4; cnt++) {
        //        robot_pos.x += vCorrTotal.x;
        //        robot_pos.y += vCorrTotal.y;
        //        vCorrTotal*=0.0f;

        int col_cnt = 0;

        int x0 = TruncFloat(robot_pos.x * INVERT(GLOBAL_SCALE_MOVE)) - COLLIDE_FIELD_R;
        int y0 = TruncFloat(robot_pos.y * INVERT(GLOBAL_SCALE_MOVE)) - COLLIDE_FIELD_R;

        int x1 = x0 + (COLLIDE_FIELD_R * 2);
        int y1 = y0 + (COLLIDE_FIELD_R * 2);

        if (x1 < 0)
            break;
        if (y1 < 0)
            break;
        if (x0 > g_MatrixMap->m_SizeMove.x)
            break;
        if (y0 > g_MatrixMap->m_SizeMove.y)
            break;

        if (x0 < 0)
            x0 = 0;
        if (y0 < 0)
            y0 = 0;
        if (x1 > g_MatrixMap->m_SizeMove.x)
            x1 = g_MatrixMap->m_SizeMove.x;
        if (y1 > g_MatrixMap->m_SizeMove.y)
            y1 = g_MatrixMap->m_SizeMove.y;

        SMatrixMapMove *smm = g_MatrixMap->MoveGet(x0, y0);

        if (calc_for_x != x0 || calc_for_y != y0) {
            // recalc corners

            SMatrixMapMove *smm0 = smm;

            for (int y = y0; y < y1; ++y, smm0 += g_MatrixMap->m_SizeMove.x - (x1 - x0)) {
                for (int x = x0; x < x1; ++x, ++smm0) {
                    corners[x - x0][y - y0] = smm0->GetType(m_Unit[0].u1.s1.m_Kind - 1);
                }
            }

            calc_for_x = x0;
            calc_for_y = y0;
        }

        for (int y = y0; y < y1; ++y, smm += g_MatrixMap->m_SizeMove.x - (x1 - x0)) {
            for (int x = x0; x < x1; ++x, ++smm) {
                BYTE corner = corners[x - x0][y - y0];
                if (corner == 0xFF)
                    continue;

                D3DXVECTOR3 col = SphereToAABB(robot_pos, smm, CPoint(x, y),
                                               corner);  //, x >= (x1-COLLIDE_FIELD_R), y >= (y1-COLLIDE_FIELD_R));

                if (!IS_ZERO_VECTOR(col)) {
                    col.z = 0;

                    robot_pos.x += col.x;
                    robot_pos.y += col.y;

                    col_cnt++;
                }
            }
        }
    }

    //#ifdef _SUB_BUG
    // if(this == g_MatrixMap->GetPlayerSide()->GetArcadedObject()){
    //    g_MatrixMap->m_DI.T(L"col_cnt", std::wstring(col_cnt + m_Cols));
    //}
    //#endif

    // if(m_Cols+col_cnt > 2){
    //    m_Velocity *= 0;
    //    corr *= 0;
    //    vCorrTotal *= 0;
    //    return vCorrTotal;
    //}

    // float ct_len/*_sq*/ = D3DXVec3Length/*Sq*/(&vCorrTotal);
    // float c_len/*_sq*/ = D3DXVec3Length/*Sq*/(&corr);
    // float vel_len/*_sq*/ = D3DXVec3Length/*Sq*/(&m_Velocity);
    // float ctc_len/*_sq*/ = D3DXVec3Length/*Sq*/(&(vCorrTotal + corr));
    // float velc_len/*_sq*/ = D3DXVec3Length/*Sq*/(&(m_Velocity + corr));
    // float velct_len/*_sq*/ = D3DXVec3Length/*Sq*/(&(vCorrTotal + m_Velocity));

    // if(ct_len > vel_len){
    //    D3DXVec3Normalize(&vCorrTotal, &vCorrTotal);
    //    vCorrTotal *= vel_len;
    //}

    // if(velc_len > ct_len){
    //    D3DXVec3Normalize(&vCorrTotal, &vCorrTotal);
    //    vCorrTotal *= velc_len;
    //}

    // if(corr.x || corr.y){
    //    if(vel_len > ctc_len){
    //        D3DXVec3Normalize(&m_Velocity, &m_Velocity);
    //        m_Velocity *= ctc_len;
    //    }else if(velc_len > ctc_len){
    //        m_Velocity *= 0;
    //        corr *= 0;
    //        vCorrTotal *= 0;
    //    }else if(velct_len > c_len){
    //        m_Velocity *= 0;
    //        corr *= 0;
    //        vCorrTotal *= 0;
    //    }
    //}

    // if(ct_len > vel_len){
    //    //D3DXVec3Normalize(&vCorrTotal, &vCorrTotal);
    //    //vCorrTotal *= vel_len;
    //    if(corr.x || corr.y){
    //        if(vel_len > ctc_len){
    //            D3DXVec3Normalize(&m_Velocity, &m_Velocity);
    //            m_Velocity *= ctc_len;
    //        }else if(D3DXVec3Length(&(m_Velocity + corr)) > ct_len){
    //            m_Velocity *= 0;
    //            corr *= 0;
    //            vCorrTotal *= 0;
    //        }
    //    }
    //}

    // if(ct_len > vel_len){
    //    D3DXVec3Normalize(&vCorrTotal, &vCorrTotal);
    //    vCorrTotal *= vel_len;
    //}
    // if(corr.x || corr.y){
    //    ct_len = D3DXVec3Length/*Sq*/(&vCorrTotal);
    //    D3DXVec3Normalize(&vCorrTotal, &vCorrTotal);
    //    vCorrTotal *= ct_len + c_len;
    //}

    return D3DXVECTOR3(robot_pos.x, robot_pos.y, 0) - oldpos;  // vCorrTotal;
}

struct CollisionData {
    D3DXVECTOR3 result;
    CMatrixRobotAI *robot;
    D3DXVECTOR3 vel;
    bool stop;
    int ms;
    bool far_col;
};

static bool CollisionCallback(const D3DXVECTOR3 &fpos, CMatrixMapStatic *pObject, DWORD user) {
    CollisionData *data = (CollisionData *)user;

    const int tm = 2;

    if (pObject->IsRobot() && !pObject->AsRobot()->IsAutomaticMode()) {
        CMatrixRobotAI *pCurrBot = pObject->AsRobot();
        D3DXVECTOR2 my_pos =
                D3DXVECTOR2(data->robot->m_PosX, data->robot->m_PosY) + D3DXVECTOR2(data->vel.x, data->vel.y);
        D3DXVECTOR2 collide_pos = D3DXVECTOR2(
                pCurrBot->m_PosX, pCurrBot->m_PosY) /*+ D3DXVECTOR2(pCurrBot->m_Velocity.x, pCurrBot->m_Velocity.y)*/;
        D3DXVECTOR2 vDist = my_pos - collide_pos;
        float dist = D3DXVec2Length(&vDist) /*D3DXVec2LengthSq(&vDist)*/;

        if (dist < (COLLIDE_BOT_R + COLLIDE_BOT_R) /**(COLLIDE_BOT_R + COLLIDE_BOT_R)*/) {
            while (true) {
                float vd = POW2(data->vel.x) + POW2(data->vel.y);
                if (fabs(vd) < POW2(0.0001f))
                    break;

                float d = 1.0f / (float)sqrt(vd);
                float vx = data->vel.x * d;
                float vy = data->vel.y * d;

                data->robot->IncColsWeight(data->ms * tm);
                data->robot->IncColsWeight2(data->ms * tm);
                pCurrBot->IncColsWeight(data->ms * tm);
                pCurrBot->IncColsWeight2(data->ms * tm);
                if (data->robot->GetColsWeight2() > 500 * tm)
                    data->robot->SetColsWeight2(500 * tm);

                if (!pCurrBot->IsAutomaticMode()) {
                    if (data->robot->GetColsWeight2() < 200 * tm) {  // Если недавно столкнулись
                        float vd2 = POW2(pCurrBot->m_Velocity.x) + POW2(pCurrBot->m_Velocity.y);
                        if (vd2 > POW2(0.0001f)) {                      // Если робот движется
                            if ((vx * -vDist.x + vy * -vDist.y) > 0) {  // И он находится впереди
                                if (vx * pCurrBot->m_Velocity.x + vy * pCurrBot->m_Velocity.y >
                                    0) {  // И движется приблезительно в одну сторону

                                    if ((pCurrBot->m_Velocity.x * vDist.x + pCurrBot->m_Velocity.y * vDist.y) > 0 &&
                                        DWORD(pCurrBot) < DWORD(data->robot))
                                        ;  // Только один из двух роботов моежт двигатся
                                    else {
                                        data->stop = true;
                                        data->far_col = true;
                                        data->robot->m_ColSpeed =
                                                std::min(data->robot->m_GroupSpeed, pCurrBot->m_GroupSpeed * 0.5f);
                                    }
                                }
                            }
                        }
                    }
                }

#ifdef _SUB_BUG
                g_MatrixMap->m_DI.T(
                        L"ColsWeight",
                        std::wstring().Format(L"<i>    <i>", data->robot->GetColsWeight(), data->robot->GetColsWeight2())
                                .Get(),
                        1000);
#endif

                if (data->robot->GetColsWeight() < 500 * tm)
                    break;
                data->robot->SetColsWeight(500 * tm);

                d = 1.0f / dist;
                float dx = -vDist.x * d, dy = -vDist.y * d;

                if (!((vx * dx + vy * dy) > cos(90 * ToRad)))
                    break;

                d = POW2(pCurrBot->m_Velocity.x) + POW2(pCurrBot->m_Velocity.y);
                if (d > 0) {
                    d = 1.0f / (float)sqrt(d);
                    float dvx = pCurrBot->m_Velocity.x * d;
                    float dvy = pCurrBot->m_Velocity.y * d;

                    if (!((vx * dvx + vy * dvy) < -cos(45 * ToRad))) {
                        data->robot->SetColsWeight(0);
                        break;
                    }
                }

                if (!data->robot->FindOrderLikeThat(ROT_MOVE_RETURN) && data->robot->m_Side == pCurrBot->m_Side) {
                    if (!pCurrBot->IsAutomaticMode()) {
                        if (!pCurrBot->FindOrderLikeThat(ROT_MOVE_RETURN)) {
                            CPoint tp;
                            if (pCurrBot->GetMoveToCoords(tp))
                                pCurrBot->MoveReturn(tp.x, tp.y);
                            else
                                pCurrBot->MoveReturn(pCurrBot->GetMapPosX(), pCurrBot->GetMapPosY());
                        }

                        int tpx = pCurrBot->GetMapPosX();
                        int tpy = pCurrBot->GetMapPosY();
                        if (g_MatrixMap->PlaceFindNearReturn(pCurrBot->m_Unit[0].u1.s1.m_Kind - 1, 4, tpx, tpy, pCurrBot)) {
                            pCurrBot->MoveTo(tpx, tpy);
                            pCurrBot->GetEnv()->AddBadCoord(CPoint(tpx, tpy));
                        }
                    }
                }
                data->robot->SetColsWeight(0);

                break;
            }

            /*            if(data->robot->m_CurrState == ROBOT_BASE_MOVEOUT ||
               data->robot->FindOrderLikeThat(ROT_MOVE_TO, ROT_GETING_LOST)){ pCurrBot->GetLost(data->robot->m_Forward);
                            float vel_len1 = D3DXVec3LengthSq(&pCurrBot->m_Velocity);
                            float vel_len2 = D3DXVec3LengthSq(&data->robot->m_Velocity);

                            if(vel_len1 > ZERO_VELOCITY*ZERO_VELOCITY && vel_len1 < vel_len2){
                                D3DXVec3Normalize(&data->robot->m_Velocity, &data->robot->m_Velocity);
                                data->robot->m_Velocity *= (float)sqrt(vel_len1);
                            }

                        }*/
            float correction = float(COLLIDE_BOT_R + COLLIDE_BOT_R - /*sqrt*/ (dist)) * 0.5f;
            D3DXVec2Normalize(&vDist, &vDist);
            vDist *= correction;
            data->result += D3DXVECTOR3(vDist.x, vDist.y, 0);
            data->robot->IncCols();
        }
        else if (dist < (COLLIDE_BOT_R * 4.0f) /**(COLLIDE_BOT_R + COLLIDE_BOT_R)*/) {
            while (true) {
                float vd = POW2(data->vel.x) + POW2(data->vel.y);
                if (fabs(vd) < POW2(0.0001f))
                    break;

                float d = 1.0f / (float)sqrt(vd);
                float vx = data->vel.x * d;
                float vy = data->vel.y * d;

                if (!pCurrBot->IsAutomaticMode()) {
                    float vd2 = POW2(pCurrBot->m_Velocity.x) + POW2(pCurrBot->m_Velocity.y);
                    if (vd2 > POW2(0.0001f)) {                      // Если робот движется
                        if ((vx * -vDist.x + vy * -vDist.y) > 0) {  // И он находится впереди
                            if (vx * pCurrBot->m_Velocity.x + vy * pCurrBot->m_Velocity.y >
                                0) {  // И движется приблезительно в одну сторону

                                if ((pCurrBot->m_Velocity.x * vDist.x + pCurrBot->m_Velocity.y * vDist.y) > 0 &&
                                    DWORD(pCurrBot) < DWORD(data->robot))
                                    ;  // Только один из двух роботов моежт двигатся
                                else {
                                    data->far_col = true;
                                    data->robot->m_ColSpeed =
                                            std::min(data->robot->m_GroupSpeed, pCurrBot->m_GroupSpeed * 0.5f);
                                }
                            }
                        }
                    }
                }
                break;
            }
        }
    }
    else if (pObject->GetObjectType() == OBJECT_TYPE_CANNON) {
        CMatrixCannon *cannon = ((CMatrixCannon *)pObject);
        D3DXVECTOR2 my_pos =
                D3DXVECTOR2(data->robot->m_PosX, data->robot->m_PosY) + D3DXVECTOR2(data->vel.x, data->vel.y);
        D3DXVECTOR2 collide_pos = cannon->m_Pos;
        D3DXVECTOR2 vDist = my_pos - collide_pos;
        float dist = D3DXVec2Length(&vDist) /*D3DXVec2LengthSq(&vDist)*/;
        //#ifdef _SUB_BUG
        //                CHelper::Create(1,1)->Cone(D3DXVECTOR3(cannon->m_Pos.x, cannon->m_Pos.y, 5.9f),
        //                D3DXVECTOR3(cannon->m_Pos.x, cannon->m_Pos.y, 7),  CANNON_COLLIDE_R,  CANNON_COLLIDE_R,
        //                0xffffffff, 0xffffffff,  12); g_MatrixMap->m_DI.T(L"CAN_DIST", std::wstring(dist));
        //#endif

        if (dist < (COLLIDE_BOT_R + CANNON_COLLIDE_R) /**(COLLIDE_BOT_R + CANNON_COLLIDE_R)*/) {
            float correction = float(COLLIDE_BOT_R + CANNON_COLLIDE_R - /*sqrt*/ (dist));
            //#ifdef _SUB_BUG
            //                    g_MatrixMap->m_DI.T(L"correction", std::wstring(correction));
            //#endif
            D3DXVec2Normalize(&vDist, &vDist);
            vDist *= correction;
            data->result += D3DXVECTOR3(vDist.x, vDist.y, 0);
            data->robot->IncCols();
        }
    }
    return true;
}

D3DXVECTOR3 CMatrixRobotAI::RobotToObjectCollision(const D3DXVECTOR3 &vel, int ms) {
    CollisionData data;
    data.result = D3DXVECTOR3(0, 0, 0);
    data.robot = this;
    data.vel = vel;
    data.stop = false;
    data.ms = ms;
    data.far_col = false;
    g_MatrixMap->FindObjects(D3DXVECTOR3(m_PosX, m_PosY, m_Core->m_GeoCenter.z), COLLIDE_BOT_R * 3, 1,
                             TRACE_ROBOT | TRACE_CANNON, this, CollisionCallback, (DWORD)&data);

    if (!data.far_col)
        m_ColSpeed = 100.0f;

    if (data.stop)
        return -vel;
    else
        return data.result;
}

void CMatrixRobotAI::WallAvoid(const D3DXVECTOR3 &o, const D3DXVECTOR3 &dest) {
    D3DXVECTOR3 prev_coll = m_CollAvoid;
    m_CollAvoid = D3DXVECTOR3(0, 0, 0);
    return;
    if (o.x != 0 || o.y != 0) {
        D3DXVECTOR3 true_forward(1, 0, 0);
        float sign1 = m_Forward.x * true_forward.x + m_Forward.y * true_forward.y;
        true_forward = D3DXVECTOR3(0, 1, 0);
        float sign2 = m_Forward.x * true_forward.x + m_Forward.y * true_forward.y;
        // HelperCreate(1, 0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 30), D3DXVECTOR3(m_PosX, m_PosY, 0) +
        // D3DXVECTOR3(true_forward.x * 100, true_forward.y * 100, 30), 0xff00ff00, 0xff00ff00); HelperCreate(1,
        // 0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50), D3DXVECTOR3(m_PosX, m_PosY, 0) + D3DXVECTOR3(m_Forward.x * 100,
        // m_Forward.y * 100, 50), 0xffffff00, 0xffffff00); if(sign < 0)
        //    CDText::T("SIGN", "<<<<");
        // else
        //    CDText::T("SIGN", ">>>>");

        if (o.x != 0 && o.y != 0) {
            D3DXVECTOR3 o_norm(0, 0, 0);
            D3DXVec3Normalize(&o_norm, &o);

            float cos1 = m_Forward.x * o_norm.x;
            float angle1 = (float)acos(cos1);

            float cos2 = m_Forward.y * o_norm.y;
            float angle2 = (float)acos(cos2);

            if (fabs(angle1) > GRAD2RAD(120)) {
                // we are perp to Y
                if (o.y < 0)
                    m_CollAvoid = D3DXVECTOR3(0, -1, 0);
                else
                    m_CollAvoid = D3DXVECTOR3(0, 1, 0);
            }
            else if (fabs(angle2) > GRAD2RAD(120)) {
                // we are perp to X
                if (o.x < 0)
                    m_CollAvoid = D3DXVECTOR3(-1, 0, 0);
                else
                    m_CollAvoid = D3DXVECTOR3(1, 0, 0);
            }
            // if(prev_coll.y == -1){
            //    if(o.x < 0)
            //        m_CollAvoid = D3DXVECTOR3(-1, 0, 0);
            //    else
            //        m_CollAvoid = D3DXVECTOR3(1, 0, 0);
            //}
        }
        else if (o.x != 0 /* || ((o.x != 0 && o.y != 0) && fabs(o.x) > fabs(o.y))*/) {
            //это условие избыточно, оно зарезервировано для возможных изменений

            if (o.x > 0) {
                if (sign2 < 0) {
                    m_CollAvoid += D3DXVECTOR3(0, -1, 0);
                }
                else {
                    m_CollAvoid += D3DXVECTOR3(0, 1, 0);
                }
            }
            else {
                if (sign2 < 0) {
                    m_CollAvoid += D3DXVECTOR3(0, -1, 0);
                }
                else {
                    m_CollAvoid += D3DXVECTOR3(0, 1, 0);
                }
            }
        }
        else if (o.y != 0 /* || ((o.x != 0 && o.y != 0) && fabs(o.y) > fabs(o.x))*/) {
            //это условие избыточно, оно зарезервировано для возможных изменений
            if (o.y > 0) {
                if (sign1 < 0) {
                    m_CollAvoid += D3DXVECTOR3(-1, 0, 0);
                }
                else {
                    m_CollAvoid += D3DXVECTOR3(1, 0, 0);
                }
            }
            else {
                if (sign1 < 0) {
                    m_CollAvoid += D3DXVECTOR3(-1, 0, 0);
                }
                else {
                    m_CollAvoid += D3DXVECTOR3(1, 0, 0);
                }
            }
        }
    }

    if (m_CollAvoid.x || m_CollAvoid.y) {
        D3DXVECTOR3 dest_n = dest - D3DXVECTOR3(m_PosX, m_PosY, 0);
        D3DXVec3Normalize(&dest_n, &dest_n);

        float cos = dest_n.x * m_CollAvoid.x + dest_n.y * m_CollAvoid.y;
        float angle = (float)acos(cos);

        if (this == g_MatrixMap->GetPlayerSide()->GetArcadedObject()) {
            if (((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_LEFT]) & 0x8000) == 0x8000) ||
                ((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_RIGHT]) & 0x8000) == 0x8000)) {
                ZeroMemory(&m_CollAvoid, sizeof(D3DXVECTOR3));
            }
        }

        if (angle >= GRAD2RAD(90)) {
            ZeroMemory(&m_CollAvoid, sizeof(D3DXVECTOR3));
        }
    }
}

D3DXVECTOR3 CMatrixRobotAI::SphereToAABB(const D3DXVECTOR2 &pos, const SMatrixMapMove *smm, const CPoint &cell,
                                         BYTE corner)  //, bool revers_x, bool revers_y)
{
    // byte corner = smm->GetType(m_Unit[0].u1.s1.m_Kind-1);

    // if (corner != 0xff)
    //{

    float dcol = 0, dsx = 0, dsy = 0;

    D3DXVECTOR2 lu(cell.x * GLOBAL_SCALE_MOVE, cell.y * GLOBAL_SCALE_MOVE);
    D3DXVECTOR2 rd(lu.x + GLOBAL_SCALE_MOVE, lu.y + GLOBAL_SCALE_MOVE);

    if (SphereToAABBCheck(pos, lu, rd, dcol, dsx, dsy)) {
        //        #ifdef _SUB_BUG
        //        CHelper::Create(1,1)->Cone(D3DXVECTOR3(pos.x, pos.y, 0.9f), D3DXVECTOR3(pos.x, pos.y, 3.0f),
        //        COLLIDE_BOT_R,  COLLIDE_BOT_R, 0xffffffff, 0xffffffff,  12);
        ////line1(-)top
        //        CHelper::Create(1,0)->Line(D3DXVECTOR3(vLu.x, vLu.y, 1),
        //						D3DXVECTOR3(vRu.x, vRu.y, 1),
        //						0xffff0000,0xffff0000);
        //
        ////line2(|)left
        //        CHelper::Create(1,0)->Line(D3DXVECTOR3(vLu.x, vLu.y, 1),
        //						D3DXVECTOR3(vLd.x, vLd.y, 1),
        //						0xffff0000,0xffff0000);
        //
        ////line3(|)right
        //        CHelper::Create(1,0)->Line(D3DXVECTOR3(vRu.x, vRu.y, 1),
        //						D3DXVECTOR3(vRd.x, vRd.y, 1),
        //						0xffff0000,0xffff0000);
        ////line4(-)bottom
        //        CHelper::Create(1,0)->Line(D3DXVECTOR3(vLd.x, vLd.y, 1),
        //						D3DXVECTOR3(vRd.x, vRd.y, 1),
        //						0xffff0000,0xffff0000);
        //        #endif

        // place for spherical and triangle corners
        if (corner > 0) {
            //#ifdef _SUB_BUG
            // CHelper::Create(1,1)->Cone(D3DXVECTOR3(vLu.x+COLLIDE_SPHERE_R, vLu.y+COLLIDE_SPHERE_R, 1),
            // D3DXVECTOR3(vLu.x+COLLIDE_SPHERE_R, vLu.y+COLLIDE_SPHERE_R, 2),  COLLIDE_SPHERE_R,  COLLIDE_SPHERE_R,
            // 0xffffffff, 0xffffffff,  12); #endif check for spherical corner or zubchik

            // byte c1 = (corner & 1);
            // byte c2 = (corner & 2);
            // byte c4 = (corner & 4);
            // byte c8 = (corner & 8);

            // byte z1 = (corner & 16);
            // byte z2 = (corner & 32);
            // byte z4 = (corner & 64);
            // byte z8 = (corner & 128);

            byte cr = 0, zb = 0;

            if (pos.x > lu.x + GLOBAL_SCALE_MOVE * 0.5f) {
                if (pos.y < lu.y + GLOBAL_SCALE_MOVE * 0.5f) {
                    // 1
                    if (corner & 16)
                        zb = (corner & 16);
                    else
                        cr = (corner & 1);
                }
                else {
                    // 2
                    if (corner & 32)
                        zb = (corner & 32);
                    else
                        cr = (corner & 2);
                }
            }
            else {
                if (pos.y < lu.y + GLOBAL_SCALE_MOVE * 0.5f) {
                    // 8
                    if (corner & 128)
                        zb = (corner & 128);
                    else
                        cr = (corner & 8);
                }
                else {
                    // 4
                    if (corner & 64)
                        zb = (corner & 64);
                    else
                        cr = (corner & 4);
                }
            }

            if (zb) {
                if ((corner & 16) || (corner & 64)) {
                    D3DXVECTOR3 v = D3DXVECTOR3(pos.x, pos.y, 0) - D3DXVECTOR3(lu.x, lu.y, 0);
                    D3DXVECTOR3 v2 = D3DXVECTOR3(GLOBAL_SCALE_MOVE, GLOBAL_SCALE_MOVE, 0);
                    D3DXVECTOR3 proj = Vec3Projection(v2, v);
                    D3DXVECTOR2 point = lu + D3DXVECTOR2(proj.x, proj.y);

                    D3DXVECTOR2 res = pos - point;
                    float res_len = D3DXVec2Length(&res);

                    if ((point.x >= lu.x && point.x <= rd.x && point.y >= lu.y && point.y <= rd.y) &&
                        res_len < COLLIDE_BOT_R) {
                        //      #ifdef _SUB_BUG
                        //      CHelper::Create(1,0)->Line(D3DXVECTOR3(vLu.x, vLu.y, 1),
                        // D3DXVECTOR3(vRd.x, vRd.y, 1),
                        // 0xffff0000,0xffff0000);
                        //      #endif

                        float cor = COLLIDE_BOT_R - res_len;
                        D3DXVec2Normalize(&res, &res);
                        res *= cor;
                        return D3DXVECTOR3(res.x, res.y, -1);
                    }
                }
                else if ((corner & 32) || (corner & 128)) {
                    D3DXVECTOR3 v = D3DXVECTOR3(pos.x, pos.y, 0) - D3DXVECTOR3(rd.x, lu.y, 0);
                    // D3DXVECTOR3 v2 = D3DXVECTOR3(vLd.x, vLd.y, 0) - D3DXVECTOR3(vRu.x, vRu.y, 0);
                    D3DXVECTOR3 v2 = D3DXVECTOR3(-GLOBAL_SCALE_MOVE, GLOBAL_SCALE_MOVE, 0);

                    D3DXVECTOR3 proj = Vec3Projection(v2, v);
                    D3DXVECTOR2 point(rd.x + proj.x, lu.y + proj.y);  //= vRu + D3DXVECTOR2(proj.x, );

                    D3DXVECTOR2 res = pos - point;
                    float res_len = D3DXVec2Length(&res);

                    // if((point.x >= vLd.x && point.x <= vRu.x && point.y >= vRu.y && point.y <= vLd.y) && res_len <
                    // COLLIDE_BOT_R){
                    if ((point.x >= lu.x && point.x <= rd.x && point.y >= lu.y && point.y <= rd.y) &&
                        res_len < COLLIDE_BOT_R) {
                        //      #ifdef _SUB_BUG
                        //      CHelper::Create(1,0)->Line(D3DXVECTOR3(vRu.x, vRu.y, 1),
                        // D3DXVECTOR3(vLd.x, vLd.y, 1),
                        // 0xffff0000,0xffff0000);
                        //      #endif
                        float cor = COLLIDE_BOT_R - res_len;
                        D3DXVec2Normalize(&res, &res);
                        res *= cor;

                        return D3DXVECTOR3(res.x, res.y, -1);
                    }
                }
                return D3DXVECTOR3(0, 0, 0);
            }
            else if (cr) {
                D3DXVECTOR3 vDist = D3DXVECTOR3(pos.x, pos.y, 0) -
                                    D3DXVECTOR3(lu.x + GLOBAL_SCALE_MOVE * 0.5f, lu.y + GLOBAL_SCALE_MOVE * 0.5f, 0);
                float dist = D3DXVec3Length /*Sq*/ (&vDist);
                if (dist < (COLLIDE_BOT_R + COLLIDE_SPHERE_R) /**(COLLIDE_BOT_R + COLLIDE_SPHERE_R)*/) {
                    float correction = float(COLLIDE_BOT_R + COLLIDE_SPHERE_R - /*sqrt*/ (dist));
                    D3DXVec3Normalize(&vDist, &vDist);
                    vDist *= correction;
                    return vDist;
                }
                return D3DXVECTOR3(0, 0, 0);
            }

            if (corner > 15)
                return D3DXVECTOR3(0, 0, 0);
        }

        //

        D3DXVECTOR2 mind_corner = lu;
        auto tmp1 = pos - lu;
        float prev_min_dist = D3DXVec2LengthSq(&tmp1);
        int angle = 8;

        auto tmp2 = pos - D3DXVECTOR2(rd.x, lu.y);
        float a = D3DXVec2LengthSq(&tmp2);
        if (a < prev_min_dist) {
            prev_min_dist = a;
            mind_corner = D3DXVECTOR2(rd.x, lu.y);
            angle = 1;
        }

        auto tmp3 = pos - rd;
        a = D3DXVec2LengthSq(&tmp3);
        if (a < prev_min_dist) {
            prev_min_dist = a;
            mind_corner = rd;
            angle = 2;
        }

        auto tmp4 = pos - D3DXVECTOR2(lu.x, rd.y);
        a = D3DXVec2LengthSq(&tmp4);

        if (a < prev_min_dist) {
            prev_min_dist = a;
            mind_corner = D3DXVECTOR2(lu.x, rd.y);
            angle = 4;
        }

        bool _GOTCHA_ = false;
        if (angle == 1) {
            // r
            // u
            BYTE a1 = 255;
            BYTE a2 = 255;
            if (cell.x < (g_MatrixMap->m_SizeMove.x - 1))
                a1 = (smm + 1)->GetType(m_Unit[0].u1.s1.m_Kind - 1);
            if (cell.y > 0)
                a2 = (smm - g_MatrixMap->m_SizeMove.x)->GetType(m_Unit[0].u1.s1.m_Kind - 1);

            // byte a1 = g_MatrixMap->GetCellMoveType(m_Unit[0].u1.s1.m_Kind-1, int(vLu.x+GLOBAL_SCALE_MOVE) /
            // int(GLOBAL_SCALE_MOVE), int(vLu.y) / int(GLOBAL_SCALE_MOVE)); byte a2 =
            // g_MatrixMap->GetCellMoveType(m_Unit[0].u1.s1.m_Kind-1, int(vLu.x) / int(GLOBAL_SCALE_MOVE),
            // int(vLu.y-GLOBAL_SCALE_MOVE) / int(GLOBAL_SCALE_MOVE));

            if ((a1 > 15 && a1 != 255) && (a2 > 15 && a2 != 255)) {
                return D3DXVECTOR3(0, 0, 0);
            }
            else if ((a1 > 15 && a1 != 255) || (a2 > 15 && a2 != 255)) {
                _GOTCHA_ = true;
            }
        }
        else if (angle == 2) {
            // r
            // d
            BYTE a1 = 255;
            BYTE a2 = 255;
            if (cell.x < (g_MatrixMap->m_SizeMove.x - 1))
                a1 = (smm + 1)->GetType(m_Unit[0].u1.s1.m_Kind - 1);
            if (cell.y < (g_MatrixMap->m_SizeMove.y - 1))
                a2 = (smm + g_MatrixMap->m_SizeMove.x)->GetType(m_Unit[0].u1.s1.m_Kind - 1);

            // byte a1 = g_MatrixMap->GetCellMoveType(m_Unit[0].u1.s1.m_Kind-1, int(vLu.x+GLOBAL_SCALE_MOVE) /
            // int(GLOBAL_SCALE_MOVE), int(vLu.y) / int(GLOBAL_SCALE_MOVE)); byte a2 =
            // g_MatrixMap->GetCellMoveType(m_Unit[0].u1.s1.m_Kind-1, int(vLu.x) / int(GLOBAL_SCALE_MOVE),
            // int(vLu.y+GLOBAL_SCALE_MOVE) / int(GLOBAL_SCALE_MOVE));

            if ((a1 > 15 && a1 != 255) && (a2 > 15 && a2 != 255)) {
                return D3DXVECTOR3(0, 0, 0);
            }
            else if ((a1 > 15 && a1 != 255) || (a2 > 15 && a2 != 255)) {
                _GOTCHA_ = true;
            }
        }
        else if (angle == 4) {
            // l
            // d
            BYTE a1 = 255;
            BYTE a2 = 255;
            if (cell.x > 0)
                a1 = (smm - 1)->GetType(m_Unit[0].u1.s1.m_Kind - 1);
            if (cell.y < (g_MatrixMap->m_SizeMove.y - 1))
                a2 = (smm + g_MatrixMap->m_SizeMove.x)->GetType(m_Unit[0].u1.s1.m_Kind - 1);

            // byte a1 = g_MatrixMap->GetCellMoveType(m_Unit[0].u1.s1.m_Kind-1, int(vLu.x-GLOBAL_SCALE_MOVE) /
            // int(GLOBAL_SCALE_MOVE), int(vLu.y) / int(GLOBAL_SCALE_MOVE)); byte a2 =
            // g_MatrixMap->GetCellMoveType(m_Unit[0].u1.s1.m_Kind-1, int(vLu.x) / int(GLOBAL_SCALE_MOVE),
            // int(vLu.y+GLOBAL_SCALE_MOVE) / int(GLOBAL_SCALE_MOVE));

            if ((a1 > 15 && a1 != 255) && (a2 > 15 && a2 != 255)) {
                return D3DXVECTOR3(0, 0, 0);
            }
            else if ((a1 > 15 && a1 != 255) || (a2 > 15 && a2 != 255)) {
                _GOTCHA_ = true;
            }
        }
        else if (angle == 8) {
            // l
            // u
            BYTE a1 = 255;
            BYTE a2 = 255;
            if (cell.x > 0)
                a1 = (smm - 1)->GetType(m_Unit[0].u1.s1.m_Kind - 1);
            if (cell.y > 0)
                a2 = (smm - g_MatrixMap->m_SizeMove.x)->GetType(m_Unit[0].u1.s1.m_Kind - 1);

            // byte a1 = g_MatrixMap->GetCellMoveType(m_Unit[0].u1.s1.m_Kind-1, int(vLu.x-GLOBAL_SCALE_MOVE) /
            // int(GLOBAL_SCALE_MOVE), int(vLu.y) / int(GLOBAL_SCALE_MOVE)); byte a2 =
            // g_MatrixMap->GetCellMoveType(m_Unit[0].u1.s1.m_Kind-1, int(vLu.x) / int(GLOBAL_SCALE_MOVE),
            // int(vLu.y-GLOBAL_SCALE_MOVE) / int(GLOBAL_SCALE_MOVE));

            if ((a1 > 15 && a1 != 255) && (a2 > 15 && a2 != 255)) {
                return D3DXVECTOR3(0, 0, 0);
            }
            else if ((a1 > 15 && a1 != 255) || (a2 > 15 && a2 != 255)) {
                _GOTCHA_ = true;
            }
        }

        float dx = pos.x - mind_corner.x;
        dx *= dx;

        float dy = pos.y - mind_corner.y;
        dy *= dy;

        D3DXVECTOR3 result(0, 0, 0), result2(0, 0, 0);

        if (dx > dy) {
            result = D3DXVECTOR3(pos.x, 0, 0) - D3DXVECTOR3(mind_corner.x, 0, 0);
            result2 = D3DXVECTOR3(pos.x, pos.y, 0) - D3DXVECTOR3(mind_corner.x, mind_corner.y, 0);
            D3DXVECTOR3 gotcha;
            D3DXVec3Normalize(&gotcha, &result2);

            float cos1 = gotcha.x * 1 + gotcha.y * 0;
            float cos2 = gotcha.x * 0 + gotcha.y * 1;
            if (_GOTCHA_) {
                if (angle == 1 && (cos1 > 0 && cos2 < 0)) {
                    result = result2;
                }
                else if (angle == 2 && (cos1 > 0 && cos2 > 0)) {
                    result = result2;
                }
                else if (angle == 4 && (cos1 < 0 && cos2 > 0)) {
                    result = result2;
                }
                else if (angle == 8 && (cos1 < 0 && cos2 < 0)) {
                    result = result2;
                }
            }
            D3DXVec3Normalize(&result, &result);
            result *= COLLIDE_BOT_R - (float)sqrt(dcol);
        }
        else if (dx < dy) {
            result = D3DXVECTOR3(0, pos.y, 0) - D3DXVECTOR3(0, mind_corner.y, 0);
            result2 = D3DXVECTOR3(pos.x, pos.y, 0) - D3DXVECTOR3(mind_corner.x, mind_corner.y, 0);
            D3DXVECTOR3 gotcha;
            D3DXVec3Normalize(&gotcha, &result2);
            float cos1 = gotcha.x * 1 + gotcha.y * 0;
            float cos2 = gotcha.x * 0 + gotcha.y * 1;

            if (_GOTCHA_) {
                if (angle == 1 && (cos1 > 0 && cos2 < 0)) {
                    result = result2;
                }
                else if (angle == 2 && (cos1 > 0 && cos2 > 0)) {
                    result = result2;
                }
                else if (angle == 4 && (cos1 < 0 && cos2 > 0)) {
                    result = result2;
                }
                else if (angle == 8 && (cos1 < 0 && cos2 < 0)) {
                    result = result2;
                }
            }
            D3DXVec3Normalize(&result, &result);
            result *= COLLIDE_BOT_R - (float)sqrt(dcol);
        }

        return result;
    }
    //}
    return D3DXVECTOR3(0, 0, 0);
}

bool CMatrixRobotAI::SphereToAABBCheck(const D3DXVECTOR2 &p, const D3DXVECTOR2 &vMin, const D3DXVECTOR2 &vMax, float &d,
                                       float &dsx, float &dsy) {
    float distance = 0;

    if (p.x < vMin.x) {
        float dx = p.x - vMin.x;
        distance += dx * dx;
        dsx += dx * dx;
    }
    else if (p.x > vMax.x) {
        float dx = p.x - vMax.x;
        distance += dx * dx;
        dsx += dx * dx;
    }

    if (p.y < vMin.y) {
        float dy = p.y - vMin.y;
        distance += dy * dy;
        dsy += dy * dy;
    }
    else if (p.y > vMax.y) {
        float dy = p.y - vMax.y;
        distance += dy * dy;
        dsy += dy * dy;
    }

    d = distance;
    return distance <= (COLLIDE_BOT_R * COLLIDE_BOT_R);
}
// void CMatrixRobotAI::OBBToAABBCollision(int nHeight, int nWidth)
//{
//
//	D3DXVECTOR2 curBlock, checkingBlock, point_tmp, s_field(0,0), s_me(0,0);
//	D3DXVECTOR2 botDirFN(m_Unit[0].m_Matrix._21, m_Unit[0].m_Matrix._22),
//		botDirUN(m_Unit[0].m_Matrix._11, m_Unit[0].m_Matrix._12),
//		botDirBN, botDirDN, worldDirFN(0, 1), worldDirUN(1, 0), worldDirBN(0, -1),worldDirDN(-1, 0);
//
//	D3DXVec2Normalize(&botDirFN, &botDirFN);
//	D3DXVec2Normalize(&botDirUN, &botDirUN);
//
//	botDirBN = botDirFN * (-1);
//	botDirDN = botDirUN * (-1);
//
//	D3DXVECTOR2 future_pos;
//	D3DXVECTOR2 vLu;
//	D3DXVECTOR2 vRu;
//	D3DXVECTOR2 vLd;
//	D3DXVECTOR2 vRd;
//
//	D3DXVECTOR3 Velocity(m_Velocity);
//
//	for(int nE = 1; nE <= 1; nE++){
//		D3DXVECTOR3 vRight(0, 0, 0), vRightPrev(0,0, 0);
//		D3DXVECTOR3 vLeft(0, 0, 0), vLeftPrev(0, 0, 0);
//		D3DXVECTOR3 vForward(0, 0, 0), vForwardPrev(0, 0, 0);
//		D3DXVECTOR3 vBackward(0, 0, 0), vBackwardPrev(0, 0, 0);
//		D3DXVECTOR3 vLuCor(0, 0, 0), vLuCorPrev(0, 0, 0);
//		D3DXVECTOR3 vRuCor(0, 0, 0), vRuCorPrev(0, 0, 0);
//		D3DXVECTOR3 vLdCor(0, 0, 0), vLdCorPrev(0, 0, 0);
//		D3DXVECTOR3 vRdCor(0, 0, 0), vRdCorPrev(0, 0, 0);
//
//		future_pos = D3DXVECTOR2(m_PosX, m_PosY) + D3DXVECTOR2(Velocity.x, Velocity.y);
//		s_me = future_pos + ((botDirFN * (float)(nHeight * int(GLOBAL_SCALE)) / 2) + (botDirUN * (float)(nWidth *
//int(GLOBAL_SCALE)) / 2));
//
//		s_field.y = float( (int(future_pos.y) / int(GLOBAL_SCALE)) - COLLIDE_FIELD_R);
//		s_field.x = float( (int(future_pos.x) / int(GLOBAL_SCALE)) - COLLIDE_FIELD_R);
//
//		vLu = s_me;
//		vRu = s_me + botDirDN * (float)(nWidth * int(GLOBAL_SCALE));
//		vLd = s_me + botDirBN * (float)(nHeight * int(GLOBAL_SCALE));
//		vRd = s_me + botDirBN * (float)(nHeight * int(GLOBAL_SCALE)) + botDirDN * (float)(nWidth * int(GLOBAL_SCALE));
//
//
///*
//		for(int i = 0; i < 7; i++) {
//			D3DXVECTOR3 v1=D3DXVECTOR3(s_field.x*GLOBAL_SCALE,s_field.y*GLOBAL_SCALE+GLOBAL_SCALE*i,1);
//			D3DXVECTOR3 v2=D3DXVECTOR3(s_field.x*GLOBAL_SCALE+GLOBAL_SCALE*7,s_field.y*GLOBAL_SCALE+GLOBAL_SCALE*i,1);
//
//			CHelper::Create(1,0)->Line(v1,v2,0xffff0000,0xffff0000);
//
//			v1=D3DXVECTOR3(s_field.x*GLOBAL_SCALE+GLOBAL_SCALE*i,s_field.y*GLOBAL_SCALE,1);
//			v2=D3DXVECTOR3(s_field.x*GLOBAL_SCALE+GLOBAL_SCALE*i,s_field.y*GLOBAL_SCALE+GLOBAL_SCALE*7,1);
//
//			CHelper::Create(1,0)->Line(v1,v2,0xffff0000,0xffff0000);
//		}
//*/
//		for(int outerLoop = 0; outerLoop <= 6; outerLoop++){
//			checkingBlock.y = (s_field.y + outerLoop);
//			for(int innerLoop = 0; innerLoop <= 6; innerLoop++){
//				checkingBlock.x = (s_field.x + innerLoop);
//
//				D3DXVECTOR2 vCellLu, vCellRu, vCellLd, vCellRd;
//				bool bReversX, bReversY;
//				bReversX = innerLoop>=3;
//				bReversY = outerLoop>=3;
//
//				vCellLu.x = checkingBlock.x*int(GLOBAL_SCALE);
//				vCellLu.y = checkingBlock.y*int(GLOBAL_SCALE);
//
//				vCellRu.x = checkingBlock.x*int(GLOBAL_SCALE) + int(GLOBAL_SCALE);
//				vCellRu.y = checkingBlock.y*int(GLOBAL_SCALE);
//
//				vCellLd.x = checkingBlock.x*int(GLOBAL_SCALE);
//				vCellLd.y = checkingBlock.y*int(GLOBAL_SCALE) + int(GLOBAL_SCALE);
//
//				vCellRd.x = checkingBlock.x*int(GLOBAL_SCALE) + int(GLOBAL_SCALE);
//				vCellRd.y = checkingBlock.y*int(GLOBAL_SCALE) + int(GLOBAL_SCALE);
//
//
//				if(PointToAABB(vCellLu, vLu, (int)GLOBAL_SCALE, (int)GLOBAL_SCALE))
//				{
//				//Угол принадлежащий vLu лежит внутри ячейки
//					D3DXVECTOR3 vTmp	= CornerLineToAABBIntersection(vLu, vRu, vCellLu, vCellLd, vCellRu, vCellRd);
//					D3DXVECTOR3 vTmp2	= CornerLineToAABBIntersection(vLu, vLd, vCellLu, vCellLd, vCellRu, vCellRd);
//					if(D3DXVec3LengthSq(&vTmp) < D3DXVec3LengthSq(&vTmp2))
//						vLuCor = vTmp;
//					else
//						vLuCor = vTmp2;
//
//					if(D3DXVec3LengthSq(&vLuCor) > D3DXVec3LengthSq(&vLuCorPrev))
//						vLuCorPrev = vLuCor;
//
//				}else if(PointToAABB(vCellLu, vRu, (int)GLOBAL_SCALE, (int)GLOBAL_SCALE)){
//				//Угол принадлежащий vRu лежит внутри ячейки
//					D3DXVECTOR3 vTmp	= CornerLineToAABBIntersection(vRu, vLu, vCellLu, vCellLd, vCellRu, vCellRd);
//					D3DXVECTOR3 vTmp2	= CornerLineToAABBIntersection(vRu, vRd, vCellLu, vCellLd, vCellRu, vCellRd);
//
//					if(D3DXVec3LengthSq(&vTmp) < D3DXVec3LengthSq(&vTmp2))
//						vRuCor = vTmp;
//					else
//						vRuCor = vTmp2;
//
//					if(D3DXVec3LengthSq(&vRuCor) > D3DXVec3LengthSq(&vRuCorPrev))
//						vRuCorPrev = vRuCor;
//
//				} else if (PointToAABB(vCellLu, vLd, (int)GLOBAL_SCALE, (int)GLOBAL_SCALE)){
//				//Угол принадлежащий vLd лежит внутри ячейки
//					D3DXVECTOR3 vTmp	= CornerLineToAABBIntersection(vLd, vLu, vCellLu, vCellLd, vCellRu, vCellRd);
//					D3DXVECTOR3 vTmp2	= CornerLineToAABBIntersection(vLd, vRd, vCellLu, vCellLd, vCellRu, vCellRd);
//
//					if(D3DXVec3LengthSq(&vTmp) < D3DXVec3LengthSq(&vTmp2))
//						vLdCor = vTmp;
//					else
//						vLdCor = vTmp2;
//
//					if(D3DXVec3LengthSq(&vLdCor) > D3DXVec3LengthSq(&vLdCorPrev))
//						vLdCorPrev = vLdCor;
//
//				} else if (PointToAABB(vCellLu, vRd, (int)GLOBAL_SCALE, (int)GLOBAL_SCALE)){
//				//Угол принадлежащий vRd лежит внутри ячейки
//					D3DXVECTOR3 vTmp	= CornerLineToAABBIntersection(vRd, vRu, vCellLu, vCellLd, vCellRu, vCellRd);
//					D3DXVECTOR3 vTmp2	= CornerLineToAABBIntersection(vRd, vLd, vCellLu, vCellLd, vCellRu, vCellRd);
//
//					if(D3DXVec3LengthSq(&vTmp) < D3DXVec3LengthSq(&vTmp2))
//						vLuCor = vTmp;
//					else
//						vLuCor = vTmp2;
//
//					if(D3DXVec3LengthSq(&vRdCor) > D3DXVec3LengthSq(&vRdCorPrev))
//						vRdCorPrev = vRdCor;
//
//				}else{
////Right
//					vRight = LineToAABBIntersection(vRu, vRd, vCellLu, vCellLd, vCellRu, vCellRd, bReversX, bReversY);
//					if(D3DXVec3LengthSq(&vRight) > D3DXVec3LengthSq(&vRightPrev)){
//						vRightPrev = vRight;
//					}
//
////Left
//					vLeft = LineToAABBIntersection(vLu, vLd, vCellLu, vCellLd, vCellRu, vCellRd, bReversX, bReversY);
//					if(D3DXVec3LengthSq(&vLeft) > D3DXVec3LengthSq(&vLeftPrev)){
//						vLeftPrev = vLeft;
//					}
//
//
////Backward
//					vBackward = LineToAABBIntersection(vLd, vRd, vCellLu, vCellLd, vCellRu, vCellRd, bReversX,
//bReversY); 					if(D3DXVec3LengthSq(&vBackward) > D3DXVec3LengthSq(&vBackwardPrev)){ 						vBackwardPrev = vBackward;
//					}
//
//
////Forward
//					vForward = LineToAABBIntersection(vLu, vRu, vCellLu, vCellLd, vCellRu, vCellRd, bReversX, bReversY);
//					if(D3DXVec3LengthSq(&vForward) > D3DXVec3LengthSq(&vForwardPrev)){
//						vForwardPrev = vForward;
//					}
//				}
//			}
//		}
////		CHelper::Create(1,0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50),
////			D3DXVECTOR3(m_PosX, m_PosY, 50) + vRightPrev,
////			0xffff0000,0xffff0000);
////		CHelper::Create(1,0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50),
////			D3DXVECTOR3(m_PosX, m_PosY, 50) + vLeftPrev,
////			0xffff0000,0xffff0000);
////		CHelper::Create(1,0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50),
////			D3DXVECTOR3(m_PosX, m_PosY, 50) + vForwardPrev,
////			0xffff0000,0xffff0000);
////		CHelper::Create(1,0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50),
////			D3DXVECTOR3(m_PosX, m_PosY, 50) + vBackwardPrev,
////			0xffff0000,0xffff0000);
//
//		Velocity += vRightPrev + vLeftPrev + vForwardPrev + vBackwardPrev + vLuCorPrev + vRuCorPrev + vLdCorPrev
//+vRdCorPrev;
//
////		CHelper::Create(1,0)->Line(D3DXVECTOR3(m_PosX, m_PosY, 50),
////			D3DXVECTOR3(m_PosX, m_PosY, 50) + Velocity*10,
////			0xffffffff,0xffff0000);
//
//	}
//
//
////	Velocity = Vec3Truncate(Velocity, m_maxSpeed);
//	m_PosX += Velocity.x;
//	m_PosY += Velocity.y;
//
//}

// D3DXVECTOR3 CMatrixRobotAI::LineToAABBIntersection(const D3DXVECTOR2 &s,const D3DXVECTOR2 &e, const D3DXVECTOR2
// &vLu,const D3DXVECTOR2 &vLd,const D3DXVECTOR2 &vRu,const D3DXVECTOR2 &vRd, bool revers_x, bool revers_y)
//{
//	if(!g_MatrixMap->PlaceIsEmpty(m_Unit[0].m_Kind-1, 1, int(vLu.x / int(GLOBAL_SCALE_MOVE)), int(vLu.y /
//int(GLOBAL_SCALE_MOVE)))){ 		int line = 0;
//
//		float u1 = 0,u2 = 0,u4 = 0,u8 = 0, t1 = 0, t2 = 0, t4 = 0, t8 = 0;
//
//		line = 0;
////line1(-)top
//		if(IntersectLine(s, e,	vLu, vRu, &t1, &u1) && t1 >= 0.0f && t1 <= 1.0 && u1 >= 0.0f && u1 <= 1.0f){
//			line |= 1;
//		}
////line2(|)left
//		if(IntersectLine(s, e,	vLu, vLd, &t2, &u2) && t2 >= 0.0f && t2 <= 1.0 && u2 >= 0.0f && u2 <= 1.0f){
//			line |= 2;
//		}
//
////line3(|)right
//		if(IntersectLine(s, e, vRu, vRd, &t4, &u4) && t4 >= 0.0f && t4 <= 1.0 && u4 >= 0.0f && u4 <= 1.0f){
//			line |= 4;
//		}
//
////line4(-)bottom
//		if(IntersectLine(s, e,	vLd, vRd, &t8, &u8) && t8 >= 0.0f && t8 <= 1.0 && u8 >= 0.0f && u8 <= 1.0f){
//			line |= 8;
//		}
//
//		if(line != 0){
//	/*CHelper::Create(1,0)->Line(D3DXVECTOR3(s.x, s.y, 1), D3DXVECTOR3(e.x, e.y, 1),0xffffffff,0xffffffff);*/
///*
//
//
//	CHelper::Create(1,0)->Line(D3DXVECTOR3(vLu.x, vLu.y, 1),
//						 D3DXVECTOR3(vRu.x, vRu.y, 1),
//						 0xffff0000,0xffff0000);
//	CHelper::Create(1,0)->Line(D3DXVECTOR3(vLu.x, vLu.y, 1),
//						 D3DXVECTOR3(vLd.x, vLd.y, 1),
//						 0xffff0000,0xffff0000);
//	CHelper::Create(1,0)->Line(D3DXVECTOR3(vRu.x, vRu.y, 1),
//						 D3DXVECTOR3(vRd.x, vRd.y, 1),
//						 0xffff0000,0xffff0000);
//	CHelper::Create(1,0)->Line(D3DXVECTOR3(vLd.x, vLd.y, 1),
//						 D3DXVECTOR3(vRd.x, vRd.y, 1),
//						 0xffff0000,0xffff0000);
//*/
//		}
//		D3DXVECTOR2 r, r2;
//		if(line==(2|4)) {
//			if(!revers_y) {
//				r=D3DXVECTOR2(0.0f,min(1.0f-u2,1.0f-u4)*GLOBAL_SCALE_MOVE);
//
////				if((1.0f-u2)<(1.0f-u4)) HelperT(vLd-r,vLd);
////				else HelperT(vRd-r,vRd);
//
//				return D3DXVECTOR3(r.x, r.y, 0);
//			} else {
//				r=D3DXVECTOR2(0.0f,(min(u2,u4))*GLOBAL_SCALE_MOVE);
//
////				if((u2)<(u4)) HelperT(vLu+r,vLu);
////				else HelperT(vRu+r,vRu);
//
//				return D3DXVECTOR3(-r.x, -r.y, 0);
//			}
//
//		} else if(line==(1|8)) {
//			if(!revers_x) {
//				r=D3DXVECTOR2(min(1.0f-u1,1.0f-u8)*GLOBAL_SCALE_MOVE,0.0f);
//
////				if((1.0f-u2)<(1.0f-u4)) HelperT(vRu-r,vRu);
////				else HelperT(vRd-r,vRd);
//
//				return D3DXVECTOR3(r.x, r.y, 0);
//			} else {
//				r=D3DXVECTOR2((min(u1,u8))*GLOBAL_SCALE_MOVE,0.0f);
//
////				if((u2)<(u4)) HelperT(vLu+r,vLu);
////				else HelperT(vLd+r,vLd);
//
//				return D3DXVECTOR3(-r.x, -r.y, 0);
//			}
//		} else if(line == (1|4)) {
//			r=D3DXVECTOR2((1.0f - u1)*GLOBAL_SCALE_MOVE, 0.0f);
//			r2=D3DXVECTOR2(0.0f, u4*GLOBAL_SCALE_MOVE);
//
//	//		HelperT(vRu - r,vRu+r2);
//	//		HelperT(vRu - r,vRu);
//	//		HelperT(vRu,vRu+r2);
//
//			if((1.0f - u1) < u4){
//				return D3DXVECTOR3(r.x, r.y, 0);
//			}
//			else{
//				return D3DXVECTOR3(-r2.x, -r2.y, 0);
//
//			}
//		} else if(line == (8|4)) {
//			r=D3DXVECTOR2((1.0f - u8)*GLOBAL_SCALE_MOVE, 0.0f);
//			r2=D3DXVECTOR2(0.0f, (1.0f - u4)*GLOBAL_SCALE_MOVE);
//
//	//		HelperT(vRd - r,vRd-r2);
//	//		HelperT(vRd - r,vRd);
//	//		HelperT(vRd,vRd-r2);
//
//
//			if((1.0f - u8) < (1.0f - u4)){
//				return D3DXVECTOR3(r.x, r.y, 0);
//			}
//			else{
//				return D3DXVECTOR3(r2.x, r2.y, 0);
//
//			}
//		} else if(line == (1|2)) {
//			r=D3DXVECTOR2(u1*GLOBAL_SCALE_MOVE, 0.0f);
//			r2=D3DXVECTOR2(0.0f, u2*GLOBAL_SCALE_MOVE);
//
//	//		HelperT(vLu, vLu + r);
//	//		HelperT(vLu, vLu + r2);
//	//		HelperT(vLu + r2, vLu + r);
//
//			if(u1 < u2){
//				return D3DXVECTOR3(-r.x, -r.y, 0);
//			}
//			else{
//				return D3DXVECTOR3(-r2.x, -r2.y, 0);
//			}
//		} else if(line == (8|2)) {
//			r=D3DXVECTOR2(u8*GLOBAL_SCALE_MOVE, 0.0f);
//			r2=D3DXVECTOR2(0.0f, (1.0f - u2)*GLOBAL_SCALE_MOVE);
//
//	//		HelperT(vLd, vLd + r);
//	//		HelperT(vLd, vLd - r2);
//	//		HelperT(vLd - r2,vLd + r);
//
//			if(u8 < (1.0f - u2)){
//				return D3DXVECTOR3(-r.x, -r.y, 0);
//			}
//			else{
//				return D3DXVECTOR3(r2.x, r2.y, 0);
//			}
//		}
//		else return D3DXVECTOR3(0,0,0);
//	}
//	return D3DXVECTOR3(0,0,0);
//}

// D3DXVECTOR3 CMatrixRobotAI::CornerLineToAABBIntersection(const D3DXVECTOR2 &s,const D3DXVECTOR2 &e, const D3DXVECTOR2
// &vLu,const D3DXVECTOR2 &vLd,const D3DXVECTOR2 &vRu,const D3DXVECTOR2 &vRd)
//{
//	if(!g_MatrixMap->PlaceIsEmpty(m_Unit[0].m_Kind-1, 1, int(vLu.x / int(GLOBAL_SCALE_MOVE)), int(vLu.y /
//int(GLOBAL_SCALE_MOVE)))){ 		int line = 0;
//
//		float u1 = 0,u2 = 0,u4 = 0,u8 = 0, t1 = 0, t2 = 0, t4 = 0, t8 = 0;
//
////line1(-)top
//		if(IntersectLine(s, e,	vLu, vRu, &t1, &u1) && t1 >= 0.0f && t1 <= 1.0 && u1 >= 0.0f && u1 <= 1.0f){
//			line = 1;
//		}
////line2(|)left
//		if(IntersectLine(s, e,	vLu, vLd, &t2, &u2) && t2 >= 0.0f && t2 <= 1.0 && u2 >= 0.0f && u2 <= 1.0f){
//			line = 2;
//		}
//
////line3(|)right
//		if(IntersectLine(s, e, vRu, vRd, &t4, &u4) && t4 >= 0.0f && t4 <= 1.0 && u4 >= 0.0f && u4 <= 1.0f){
//			line = 4;
//		}
//
////line4(-)bottom
//		if(IntersectLine(s, e,	vLd, vRd, &t8, &u8) && t8 >= 0.0f && t8 <= 1.0 && u8 >= 0.0f && u8 <= 1.0f){
//			line = 8;
//		}
//
//
//		if(line != 0){
////CHelper::Create(1,0)->Line(D3DXVECTOR3(s.x, s.y, 1), D3DXVECTOR3(e.x, e.y, 1),0xffffffff,0xffffffff);
///*
// CHelper::Create(1,0)->Line(D3DXVECTOR3(vLu.x, vLu.y, 1),
//						 D3DXVECTOR3(vRu.x, vRu.y, 1),
//						 0xffff0000,0xffff0000);
// CHelper::Create(1,0)->Line(D3DXVECTOR3(vLu.x, vLu.y, 1),
//						 D3DXVECTOR3(vLd.x, vLd.y, 1),
//						 0xffff0000,0xffff0000);
// CHelper::Create(1,0)->Line(D3DXVECTOR3(vRu.x, vRu.y, 1),
//						 D3DXVECTOR3(vRd.x, vRd.y, 1),
//						 0xffff0000,0xffff0000);
// CHelper::Create(1,0)->Line(D3DXVECTOR3(vLd.x, vLd.y, 1),
//						 D3DXVECTOR3(vRd.x, vRd.y, 1),
//						 0xffff0000,0xffff0000);
//*/
//		}
//
//		if(line == 1){
//			D3DXVECTOR2  napr = e - s;
//			napr *= t1;
////			HelperT(s, s + napr);
//			return D3DXVECTOR3(napr.x, napr.y, 0);
//		} else if(line == 2){
//			D3DXVECTOR2  napr = e - s;
//			napr *= t2;
////			HelperT(s, s + napr);
//			return D3DXVECTOR3(napr.x, napr.y, 0);
//
//		} else if(line == 4){
//			D3DXVECTOR2  napr = e - s;
//			napr *= t4;
////			HelperT(s, s + napr);
//			return D3DXVECTOR3(napr.x, napr.y, 0);
//
//		} else if(line == 8){
//			D3DXVECTOR2  napr = e - s;
//			napr *= t8;
////			HelperT(s, s + napr);
//			return D3DXVECTOR3(napr.x, napr.y, 0);
//		}
//		else return D3DXVECTOR3(0,0,0);
//	}
//	return D3DXVECTOR3(0,0,0);
//}

// void CMatrixRobotAI::Load(CBuf & buf, CTemporaryLoadData *)
//{
//    DTRACE();
//
//
//	m_PosX=float(buf.Int());
//	m_PosY=float(buf.Int());
//
//	m_PosX=GLOBAL_SCALE*(m_PosX+buf.Float());
//    m_PosY=GLOBAL_SCALE*(m_PosY+buf.Float());
//
//	m_Side=buf.Byte();
//
//    bool arm = false;
//
//	int cnt=buf.Byte();
//	for(int i=0;i<cnt;i++)
//    {
//		ERobotUnitType type = (ERobotUnitType)buf.Byte();
//		ERobotUnitKind kind = (ERobotUnitKind)buf.Byte();
//		UnitInsert(i,type,kind);
//        float a = buf.Float();
//
//        if (type == MRT_ARMOR) arm = true;
//
//        if (type == MRT_CHASSIS)
//        {
//            m_Forward.z = 0;
//            SinCos(a, &m_Forward.x, &m_Forward.y);
//            (*(DWORD *)&m_Forward.x) ^= 0x80000000;
//            m_Unit[i].m_Angle = 0;
//            m_HullForward = m_Forward;
//        } else
//        {
//            m_Unit[i].m_Angle = a;
//        }
//
//        D3DXMatrixIdentity(&m_Unit[i].m_Matrix);
//	}
//
//    D3DXMatrixIdentity(&m_Core->m_Matrix);
//
//	m_Core->m_ShadowType = (EShadowType)buf.Byte();
////m_ShadowType=0;
//	m_Core->m_ShadowSize=buf.Word();
//
//    m_CurrState = ROBOT_SUCCESSFULLY_BUILD;
//
//    if (!arm)
//    {
//        g_MatrixMap->StaticDelete(this);
//        return;
//    }
//
//    WeaponSelectMatrix();
//    RobotWeaponInit();
//	CalcRobotMass();
//
//    RChange(MR_Graph);
//    RNeed(MR_Graph);
//
//    SwitchAnimation(ANIMATION_STAY);
//}

void robot_weapon_hit(CMatrixMapStatic *hit, const D3DXVECTOR3 &pos, DWORD user, DWORD flags) {
    CMatrixMapStatic *obj = CMatrixMapStatic::GetFirstLogic();
    while (obj) {
        if (user == DWORD(obj)) {
            if (obj->AsRobot()->m_CurrState != ROBOT_DIP) {
                obj->AsRobot()->HitTo(hit, pos);
            }
            return;
        }
        obj = obj->GetNextLogic();
    }
}

void CMatrixRobotAI::HitTo(CMatrixMapStatic *hit, const D3DXVECTOR3 &pos) {
    if (IS_TRACE_STOP_OBJECT(hit)) {
        if (GetEnv()->m_Target == hit)
            GetEnv()->m_LastHitEnemy = GetEnv()->m_LastHitTarget = g_MatrixMap->GetTime();
        else if (hit->GetObjectType() == OBJECT_TYPE_BUILDING)
            ;
        else if (hit->GetSide() != GetSide())
            GetEnv()->m_LastHitEnemy = g_MatrixMap->GetTime();
        else
            GetEnv()->m_LastHitFriendly = g_MatrixMap->GetTime();

        // if(GetSide()!=PLAYER_SIDE) {
        //    CMatrixSideUnit * side=g_MatrixMap->GetSideById(GetSide());
        //    side->m_Team[GetTeam()].m_War=true;
        //}

        if (hit->GetObjectType() ==
            OBJECT_TYPE_ROBOTAI) {  // Если робот стреляет в пушку и мы в него попали то робот переключается на нас.
            CMatrixRobotAI *robot = (CMatrixRobotAI *)hit;

            if (robot != this && robot->GetSide() != GetSide() && !robot->m_Environment.SearchEnemy(this))
                robot->m_Environment.AddToList(this);

            if (robot->GetEnv()->m_TargetAttack != NULL &&
                robot->GetEnv()->m_TargetAttack->GetObjectType() == OBJECT_TYPE_CANNON &&
                robot->GetEnv()->SearchEnemy(this) && (g_MatrixMap->GetTime() - GetEnv()->m_TargetChange) > 1000) {
                GetEnv()->m_TargetLast = GetEnv()->m_TargetAttack;
                robot->GetEnv()->m_TargetAttack = this;
                GetEnv()->m_TargetChange = g_MatrixMap->GetTime();
            }
        }
    }
}

void CMatrixRobotAI::RobotWeaponInit() {
    int Weap = 0;
    for (int C = 0; C < m_UnitCnt; C++) {
        if (m_Unit[C].m_Type == MRT_WEAPON) {
            switch (m_Unit[C].u1.s1.m_Kind) {
                case RUK_WEAPON_MACHINEGUN:
                    m_Weapons[Weap].CreateEffect((DWORD)this, &robot_weapon_hit, WEAPON_VOLCANO);
                    m_Weapons[Weap].m_Unit = &m_Unit[C];
                    m_Weapons[Weap].m_HeatMod = g_Config.m_Overheat[WEAPON_VOLCANO_HEAT];
                    m_Weapons[Weap].m_CoolDownMod = g_Config.m_Overheat[WEAPON_VOLCANO_COOL];
                    break;
                case RUK_WEAPON_CANNON:
                    m_Weapons[Weap].CreateEffect((DWORD)this, &robot_weapon_hit, WEAPON_GUN);
                    m_Weapons[Weap].m_Unit = &m_Unit[C];
                    m_Weapons[Weap].m_HeatMod = g_Config.m_Overheat[WEAPON_GUN_HEAT];
                    m_Weapons[Weap].m_CoolDownMod = g_Config.m_Overheat[WEAPON_GUN_COOL];

                    break;
                case RUK_WEAPON_MISSILE:
                    m_Weapons[Weap].CreateEffect((DWORD)this, &robot_weapon_hit, WEAPON_HOMING_MISSILE);
                    m_Weapons[Weap].m_Unit = &m_Unit[C];
                    m_Weapons[Weap].m_HeatMod = g_Config.m_Overheat[WEAPON_HOMING_MISSILE_HEAT];
                    m_Weapons[Weap].m_CoolDownMod = g_Config.m_Overheat[WEAPON_HOMING_MISSILE_COOL];

                    break;
                case RUK_WEAPON_FLAMETHROWER:
                    m_Weapons[Weap].CreateEffect((DWORD)this, &robot_weapon_hit, WEAPON_FLAMETHROWER);
                    m_Weapons[Weap].m_Unit = &m_Unit[C];
                    m_Weapons[Weap].m_HeatMod = g_Config.m_Overheat[WEAPON_FLAMETHROWER_HEAT];
                    m_Weapons[Weap].m_CoolDownMod = g_Config.m_Overheat[WEAPON_FLAMETHROWER_COOL];

                    break;
                case RUK_WEAPON_MORTAR:
                    m_Weapons[Weap].CreateEffect((DWORD)this, &robot_weapon_hit, WEAPON_BOMB);
                    m_Weapons[Weap].m_Unit = &m_Unit[C];
                    m_Weapons[Weap].m_HeatMod = g_Config.m_Overheat[WEAPON_BOMB_HEAT];
                    m_Weapons[Weap].m_CoolDownMod = g_Config.m_Overheat[WEAPON_BOMB_COOL];

                    break;
                case RUK_WEAPON_LASER:
                    m_Weapons[Weap].CreateEffect((DWORD)this, &robot_weapon_hit, WEAPON_LASER);
                    m_Weapons[Weap].m_Unit = &m_Unit[C];
                    m_Weapons[Weap].m_HeatMod = g_Config.m_Overheat[WEAPON_LASER_HEAT];
                    m_Weapons[Weap].m_CoolDownMod = g_Config.m_Overheat[WEAPON_LASER_COOL];

                    break;
                case RUK_WEAPON_BOMB:
                    m_Weapons[Weap].CreateEffect((DWORD)this, &robot_weapon_hit, WEAPON_BIGBOOM);
                    m_Weapons[Weap].m_Unit = &m_Unit[C];
                    m_Weapons[Weap].m_HeatMod = 10;
                    m_Weapons[Weap].m_CoolDownMod = 10;

                    break;
                case RUK_WEAPON_PLASMA:
                    m_Weapons[Weap].CreateEffect((DWORD)this, &robot_weapon_hit, WEAPON_PLASMA);
                    m_Weapons[Weap].m_Unit = &m_Unit[C];
                    m_Weapons[Weap].m_HeatMod = g_Config.m_Overheat[WEAPON_PLASMA_HEAT];
                    m_Weapons[Weap].m_CoolDownMod = g_Config.m_Overheat[WEAPON_PLASMA_COOL];

                    break;
                case RUK_WEAPON_ELECTRIC:
                    m_Weapons[Weap].CreateEffect((DWORD)this, NULL, WEAPON_LIGHTENING);
                    m_Weapons[Weap].m_Unit = &m_Unit[C];
                    m_Weapons[Weap].m_HeatMod = g_Config.m_Overheat[WEAPON_LIGHTENING_HEAT];
                    m_Weapons[Weap].m_CoolDownMod = g_Config.m_Overheat[WEAPON_LIGHTENING_COOL];

                    break;
                case RUK_WEAPON_REPAIR:
                    m_Weapons[Weap].m_Unit = m_Unit + C;
                    m_Weapons[Weap].m_HeatMod = 10;
                    m_Weapons[Weap].m_CoolDownMod = 10;
                    m_Weapons[Weap].CreateEffect((DWORD)this, &robot_weapon_hit, WEAPON_REPAIR);

                    break;

                default:
                    break;
            }
            m_Weapons[Weap].SetOwner(this);
            m_Weapons[Weap].m_CoolDownPeriod = 0;
            m_Weapons[Weap].m_HeatPeriod = 0;
            m_Weapons[Weap].m_Heat = 0;
            m_Weapons[Weap].m_On = true;
            Weap++;
        }
    }
    m_WeaponsCnt = Weap;
}

void CMatrixRobotAI::CalcRobotMass() {
    m_HaveRepair = 0;

    bool normalweapon = false;

    float hp = 0;
    for (int nC = 0; nC <= m_UnitCnt; nC++) {
        if (m_Unit[nC].m_Type == MRT_CHASSIS) {
            switch (m_Unit[nC].u1.s1.m_Kind) {
                case RUK_CHASSIS_PNEUMATIC:
                    m_SpeedWaterCorr = g_Config.m_ItemChars[CHASSIS1_MOVE_WATER_CORR];
                    m_SpeedSlopeCorrDown = g_Config.m_ItemChars[CHASSIS1_MOVE_SLOPE_CORR_DOWN];
                    m_SpeedSlopeCorrUp = g_Config.m_ItemChars[CHASSIS1_MOVE_SLOPE_CORR_UP];
                    m_maxSpeed = g_Config.m_ItemChars[CHASSIS1_MOVE_SPEED];
                    m_maxRotationSpeed = g_Config.m_ItemChars[CHASSIS1_ROTATION_SPEED];
                    hp += g_Config.m_ItemChars[CHASSIS1_STRUCTURE];
                    break;
                case RUK_CHASSIS_WHEEL:
                    m_SpeedWaterCorr = g_Config.m_ItemChars[CHASSIS2_MOVE_WATER_CORR];
                    m_SpeedSlopeCorrDown = g_Config.m_ItemChars[CHASSIS2_MOVE_SLOPE_CORR_DOWN];
                    m_SpeedSlopeCorrUp = g_Config.m_ItemChars[CHASSIS2_MOVE_SLOPE_CORR_UP];
                    m_maxSpeed = g_Config.m_ItemChars[CHASSIS2_MOVE_SPEED];
                    m_maxRotationSpeed = g_Config.m_ItemChars[CHASSIS2_ROTATION_SPEED];
                    hp += g_Config.m_ItemChars[CHASSIS2_STRUCTURE];
                    break;
                case RUK_CHASSIS_TRACK:
                    m_SpeedWaterCorr = g_Config.m_ItemChars[CHASSIS3_MOVE_WATER_CORR];
                    m_SpeedSlopeCorrDown = g_Config.m_ItemChars[CHASSIS3_MOVE_SLOPE_CORR_DOWN];
                    m_SpeedSlopeCorrUp = g_Config.m_ItemChars[CHASSIS3_MOVE_SLOPE_CORR_UP];
                    m_maxSpeed = g_Config.m_ItemChars[CHASSIS3_MOVE_SPEED];
                    m_maxRotationSpeed = g_Config.m_ItemChars[CHASSIS3_ROTATION_SPEED];
                    hp += g_Config.m_ItemChars[CHASSIS3_STRUCTURE];
                    break;
                case RUK_CHASSIS_HOVERCRAFT:
                    m_SpeedWaterCorr = g_Config.m_ItemChars[CHASSIS4_MOVE_WATER_CORR];
                    m_SpeedSlopeCorrDown = g_Config.m_ItemChars[CHASSIS4_MOVE_SLOPE_CORR_DOWN];
                    m_SpeedSlopeCorrUp = g_Config.m_ItemChars[CHASSIS4_MOVE_SLOPE_CORR_UP];
                    m_maxSpeed = g_Config.m_ItemChars[CHASSIS4_MOVE_SPEED];
                    m_maxRotationSpeed = g_Config.m_ItemChars[CHASSIS4_ROTATION_SPEED];
                    hp += g_Config.m_ItemChars[CHASSIS4_STRUCTURE];
                    break;
                case RUK_CHASSIS_ANTIGRAVITY:
                    m_SpeedWaterCorr = g_Config.m_ItemChars[CHASSIS5_MOVE_WATER_CORR];
                    m_SpeedSlopeCorrDown = g_Config.m_ItemChars[CHASSIS5_MOVE_SLOPE_CORR_DOWN];
                    m_SpeedSlopeCorrUp = g_Config.m_ItemChars[CHASSIS5_MOVE_SLOPE_CORR_UP];
                    m_maxSpeed = g_Config.m_ItemChars[CHASSIS5_MOVE_SPEED];
                    m_maxRotationSpeed = g_Config.m_ItemChars[CHASSIS5_ROTATION_SPEED];
                    hp += g_Config.m_ItemChars[CHASSIS5_STRUCTURE];
                    break;
            }
        }
        if (m_Unit[nC].m_Type == MRT_ARMOR) {
            switch (m_Unit[nC].u1.s1.m_Kind) {
                case RUK_ARMOR_PASSIVE:
                    m_maxHullSpeed = g_Config.m_ItemChars[ARMOR1_ROTATION_SPEED];
                    hp += g_Config.m_ItemChars[ARMOR1_STRUCTURE];
                    break;
                case RUK_ARMOR_ACTIVE:
                    m_maxHullSpeed = g_Config.m_ItemChars[ARMOR2_ROTATION_SPEED];
                    hp += g_Config.m_ItemChars[ARMOR2_STRUCTURE];
                    break;
                case RUK_ARMOR_FIREPROOF:
                    m_maxHullSpeed = g_Config.m_ItemChars[ARMOR3_ROTATION_SPEED];
                    hp += g_Config.m_ItemChars[ARMOR3_STRUCTURE];
                    break;
                case RUK_ARMOR_PLASMIC:
                    m_maxHullSpeed = g_Config.m_ItemChars[ARMOR4_ROTATION_SPEED];
                    hp += g_Config.m_ItemChars[ARMOR4_STRUCTURE];
                    break;
                case RUK_ARMOR_NUCLEAR:
                    m_maxHullSpeed = g_Config.m_ItemChars[ARMOR5_ROTATION_SPEED];
                    hp += g_Config.m_ItemChars[ARMOR5_STRUCTURE];
                    break;
                case RUK_ARMOR_6:
                    m_maxHullSpeed = g_Config.m_ItemChars[ARMOR6_ROTATION_SPEED];
                    hp += g_Config.m_ItemChars[ARMOR6_STRUCTURE];
                    break;
            }
        }
        if (m_Unit[nC].m_Type == MRT_WEAPON) {
            if (m_Unit[nC].u1.s1.m_Kind == RUK_WEAPON_REPAIR)
                m_HaveRepair = 1;
            else if (m_Unit[nC].u1.s1.m_Kind != RUK_WEAPON_BOMB)
                normalweapon = true;
        }
    }
    if (m_HaveRepair && !normalweapon)
        m_HaveRepair = 2;

    CBlockPar *bp = g_MatrixData->BlockGet(PAR_SOURCE_CHARS)->BlockGet(L"Heads");
    const wchar *hn = L"";

    for (int nC = m_UnitCnt - 1; nC >= 0; --nC) {
        if (m_Unit[nC].m_Type == MRT_HEAD) {
            if (m_Unit[nC].u1.s1.m_Kind == RUK_HEAD_BLOCKER) {
                hn = L"S";
            }
            else if (m_Unit[nC].u1.s1.m_Kind == RUK_HEAD_DYNAMO) {
                hn = L"D";
            }
            else if (m_Unit[nC].u1.s1.m_Kind == RUK_HEAD_LOCKATOR) {
                hn = L"L";
            }
            else if (m_Unit[nC].u1.s1.m_Kind == RUK_HEAD_FIREWALL) {
                hn = L"F";
            }
            break;
        }
    }

    float cool_down = 0;
    float fire_dist = 0;
    float overheat = 0;

    m_LightProtect = 0;
    m_BombProtect = 0;
    m_AimProtect = 0;
    m_RadarRadius = g_Config.m_RobotRadarR;

    bp = bp->BlockGetNE(hn);
    if (bp) {
        hp += bp->ParGetNE(L"HIT_POINT_ADD").GetInt();

        cool_down = float(bp->ParGetNE(L"COOL_DOWN").GetDouble() / 100.0f);
        if (cool_down < -1.0f)
            cool_down = -1.0f;

        fire_dist = float(bp->ParGetNE(L"FIRE_DISTANCE").GetDouble() / 100.0f);
        if (fire_dist < -1.0f)
            fire_dist = -1.0f;

        overheat = float(bp->ParGetNE(L"OVERHEAT").GetDouble() / 100.0f);
        if (overheat < -1.0f)
            overheat = -1.0f;

        float up = float(bp->ParGetNE(L"CHASSIS_SPEED").GetDouble() / 100.0f);
        if (up < -1.0f)
            up = -1.0f;
        m_maxSpeed += m_maxSpeed * up;
        m_maxRotationSpeed += m_maxRotationSpeed * up;

        up = float(bp->ParGetNE(L"RADAR_DISTANCE").GetDouble() / 100.0f);
        if (up < -1.0f)
            up = -1.0f;
        m_RadarRadius += m_RadarRadius * up;

        m_LightProtect = float(bp->ParGetNE(L"LIGHT_PROTECT").GetDouble() / 100.0f);
        if (m_LightProtect < 0)
            m_LightProtect = 0;
        else if (m_LightProtect > 1)
            m_LightProtect = 1;

        m_BombProtect = float(bp->ParGetNE(L"BOMB_PROTECT").GetDouble() / 100.0f);
        if (m_BombProtect < 0)
            m_BombProtect = 0;
        else if (m_BombProtect > 1)
            m_BombProtect = 1;

        m_AimProtect = float(bp->ParGetNE(L"AIM_PROTECT").GetDouble() / 100.0f);
        if (m_AimProtect < 0)
            m_AimProtect = 0;
        else if (m_AimProtect > 1)
            m_AimProtect = 1;
    }

    InitMaxHitpoint(hp);
    float tmp_min = 1E30f, tmp_max = -1E30f, r_min = 1e30f;

    int cnt = 0;
    while (cnt < m_WeaponsCnt) {
        m_Weapons[cnt].ModifyCoolDown(cool_down);
        m_Weapons[cnt].ModifyDist(fire_dist);

        m_Weapons[cnt].m_HeatMod =
                Float2Int(float(m_Weapons[cnt].m_HeatMod) + float(m_Weapons[cnt].m_HeatMod) * overheat);

        if (m_Weapons[cnt].GetWeaponType() == WEAPON_BIGBOOM) {
            cnt++;
            continue;
        }
        if (m_Weapons[cnt].GetWeaponType() == WEAPON_REPAIR) {
            r_min = std::min(r_min, m_Weapons[cnt].GetWeaponDist());
            cnt++;
            continue;
        }
        if (m_Weapons[cnt].GetWeaponDist() < tmp_min) {
            tmp_min = m_Weapons[cnt].GetWeaponDist();
        }
        if (m_Weapons[cnt].GetWeaponDist() > tmp_max) {
            tmp_max = m_Weapons[cnt].GetWeaponDist();
        }
        cnt++;
    }
    if (tmp_max < 0.0f) {
        m_MaxFireDist = m_MinFireDist = 0.0f;
    }
    else {
        m_MaxFireDist = tmp_max;
        m_MinFireDist = tmp_min;
    }
    if (r_min > 1e20f)
        m_RepairDist = 0;
    else
        m_RepairDist = r_min;
}

void CMatrixRobotAI::GatherInfo(int type) {
    DTRACE();

    CalcStrength();
    DCP();

    //    m_GatherPeriod += ms;

    /*    CMatrixGroup * my_group = g_MatrixMap->GetSideById(m_Side)->GetGroup(m_Group, m_Team);
        if(my_group) {
            m_GroupSpeed = my_group->GetGroupSpeed();
        }*/

    //    if(m_GatherPeriod >= GATHER_PERIOD) m_GatherPeriod = 0;
    //    else return;

    CMatrixMapStatic *obj = CMatrixMapStatic::GetFirstLogic();
    DCP();

    if (type == 0) {
        // Look
        while (obj) {
            DCP();
            if (obj->IsLiveRobot() && obj != this && obj->GetSide() != m_Side) {
                CMatrixRobotAI *robot = (CMatrixRobotAI *)obj;
                D3DXVECTOR3 enemy_napr = D3DXVECTOR3(robot->m_PosX, robot->m_PosY, 0) - D3DXVECTOR3(m_PosX, m_PosY, 0);

                D3DXVECTOR3 en_norm(0, 0, 0);
                D3DXVec3Normalize(&en_norm, &enemy_napr);
                float cos = m_HullForward.x * en_norm.x + m_HullForward.y * en_norm.y;
                float angle_rad = (float)acos(cos);

                float dist_enemy = D3DXVec3LengthSq(&enemy_napr);
                DCP();

                if (dist_enemy <= POW2(std::max(robot->m_MaxFireDist, m_MaxFireDist) *
                                       1.1) /*(D3DXVec3LengthSq(&enemy_napr) <= m_MinFireDist*m_MinFireDist) ||
                                               (D3DXVec3LengthSq(&enemy_napr) <= m_MaxFireDist*m_MaxFireDist &&
                                               angle_rad <= ROBOT_FOV)*/
                    && robot->m_CurrState != ROBOT_DIP) {
                    if (g_MatrixMap->IsLogicVisible(this, robot, 0.0f)) {
                        DCP();
                        if (!m_Environment.SearchEnemy(robot) && !m_Environment.IsIgnore(robot)) {
                            DCP();

                            CMatrixSideUnit *side = g_MatrixMap->GetSideById(GetSide());
                            int listcnt = 0;
                            int dist = 0;
                            CPoint p_from(m_MapX, m_MapY);
                            CPoint p_to(robot->m_MapX, robot->m_MapY);

                            side->BufPrepare();

                            float d = sqrt(float(p_from.Dist2(p_to)));
                            float z = fabs(g_MatrixMap->GetZ(robot->m_PosX, robot->m_PosY) -
                                           g_MatrixMap->GetZ(m_PosX, m_PosY));

                            /*if((z/(d*GLOBAL_SCALE_MOVE))>=tan(BARREL_TO_SHOT_ANGLE)) {
                                m_Environment.AddIgnore(robot);
                            } else*/
                            if (g_MatrixMap->PlaceList(
                                        m_Unit[0].u1.s1.m_Kind - 1, p_from, p_to,
                                        Float2Int(GetMaxFireDist() / GLOBAL_SCALE_MOVE /*+ROBOT_MOVECELLS_PER_SIZE*/),
                                        false, side->m_PlaceList, &listcnt, &dist)) {
                                if (POW2(dist / 4) < p_from.Dist2(p_to)) {
                                    m_Environment.AddToList(robot);
                                }
                                else {
                                    m_Environment.AddIgnore(robot);
                                }
                            }
                            else
                                m_Environment.AddIgnore(robot);
                        }
                    }
                    DCP();
                    /*                D3DXVECTOR3 rem;
                                    CMatrixMapStatic* trace_res = NULL;
                                    trace_res = g_MatrixMap->Trace(&rem, GetGeoCenter(), robot->GetGeoCenter(),
                       TRACE_ANYOBJECT|TRACE_NONOBJECT|TRACE_OBJECTSPHERE|TRACE_SKIP_INVISIBLE, this);
                                    if((IS_TRACE_STOP_OBJECT(trace_res) && trace_res == robot) &&
                       !m_Environment.SearchEnemy(robot)){ m_Environment.AddToList(obj); } else { D3DXVECTOR3
                       v1=GetGeoCenter(); v1.z+=50.0f; trace_res = g_MatrixMap->Trace(&rem, v1, robot->GetGeoCenter(),
                       TRACE_ANYOBJECT|TRACE_NONOBJECT|TRACE_OBJECTSPHERE|TRACE_SKIP_INVISIBLE, this);
                                        if((IS_TRACE_STOP_OBJECT(trace_res) && trace_res == robot) &&
                       !m_Environment.SearchEnemy(robot)){ m_Environment.AddToList(obj);
                                        }
                                    }*/
                }
                else {
                    DCP();
                    if (/*robot->m_FireTarget != this && */ dist_enemy >
                        POW2(std::max(robot->m_MaxFireDist, m_MaxFireDist) * 1.4))
                        m_Environment.RemoveFromListSlowly(obj);
                }
                DCP();
            }
            else if (obj->IsLiveCannon() && obj->AsCannon()->m_CurrState != CANNON_UNDER_CONSTRUCTION &&
                     obj->GetSide() != m_Side) {
                CMatrixCannon *cannon = (CMatrixCannon *)obj;
                D3DXVECTOR3 enemy_napr = cannon->GetGeoCenter() - D3DXVECTOR3(m_PosX, m_PosY, 0);

                D3DXVECTOR3 en_norm(0, 0, 0);
                D3DXVec3Normalize(&en_norm, &enemy_napr);
                float cos = m_HullForward.x * en_norm.x + m_HullForward.y * en_norm.y;
                float angle_rad = (float)acos(cos);

                float dist_enemy = D3DXVec3LengthSq(&enemy_napr);

                DCP();
                if (dist_enemy <=
                            POW2(std::max(cannon->GetFireRadius() * 1.01,
                                     m_MaxFireDist *
                                             1.1)) /*(D3DXVec3LengthSq(&enemy_napr) <= m_MinFireDist*m_MinFireDist)*/
                    /* || (D3DXVec3LengthSq(&enemy_napr) <= m_MaxFireDist*m_MaxFireDist && angle_rad <= ROBOT_FOV) */
                    && cannon->m_CurrState != CANNON_DIP) {
                    if (g_MatrixMap->IsLogicVisible(this, cannon, 0.0f)) {
                        if (!m_Environment.SearchEnemy(cannon) && !m_Environment.IsIgnore(cannon)) {
                            DCP();

                            CMatrixSideUnit *side = g_MatrixMap->GetSideById(GetSide());
                            int listcnt = 0;
                            int dist = 0;
                            CPoint p_from(m_MapX, m_MapY);
                            CPoint p_to(Float2Int(cannon->m_Pos.x / GLOBAL_SCALE_MOVE),
                                        Float2Int(cannon->m_Pos.y / GLOBAL_SCALE_MOVE));

                            side->BufPrepare();

                            float d = sqrt(float(p_from.Dist2(p_to)));
                            // float
                            // z=fabs(g_MatrixMap->GetZ(cannon->m_Pos.x,cannon->m_Pos.y)-g_MatrixMap->GetZ(m_PosX,m_PosY));
                            float z = fabs(cannon->GetGeoCenter().z - GetGeoCenter().z);

                            if ((z / (d * GLOBAL_SCALE_MOVE)) >= tan(BARREL_TO_SHOT_ANGLE)) {
                                m_Environment.AddIgnore(cannon);
                            }
                            else if (g_MatrixMap->PlaceList(m_Unit[0].u1.s1.m_Kind - 1, p_from, p_to,
                                                            Float2Int(GetMaxFireDist() /
                                                                      GLOBAL_SCALE_MOVE /*+ROBOT_MOVECELLS_PER_SIZE*/),
                                                            false, side->m_PlaceList, &listcnt, &dist)) {
                                if (POW2(dist / 4) < p_from.Dist2(p_to)) {
                                    m_Environment.AddToList(cannon);
                                }
                                else {
                                    m_Environment.AddIgnore(cannon);
                                }
                            }
                            else
                                m_Environment.AddIgnore(cannon);
                        }
                    }
                    /*                D3DXVECTOR3 rem;

                                    CMatrixMapStatic* trace_res = NULL;
                                    trace_res = g_MatrixMap->Trace(&rem, GetGeoCenter(), cannon->GetGeoCenter(),
                       TRACE_ANYOBJECT|TRACE_NONOBJECT|TRACE_OBJECTSPHERE|TRACE_SKIP_INVISIBLE, this);
                                    if((IS_TRACE_STOP_OBJECT(trace_res) && trace_res == cannon) &&
                       !m_Environment.SearchEnemy(cannon)){ m_Environment.AddToList(obj); } else { D3DXVECTOR3
                       v1=GetGeoCenter(); v1.z+=50.0f; trace_res = g_MatrixMap->Trace(&rem, v1, cannon->GetGeoCenter(),
                       TRACE_ANYOBJECT|TRACE_NONOBJECT|TRACE_OBJECTSPHERE|TRACE_SKIP_INVISIBLE, this);
                                        if((IS_TRACE_STOP_OBJECT(trace_res) && trace_res == cannon) &&
                       !m_Environment.SearchEnemy(cannon)){ m_Environment.AddToList(obj);
                                        }
                                    }*/
                }
                else {
                    DCP();
                    if (cannon->m_TargetCore != this->m_Core &&
                        dist_enemy > POW2(std::max(cannon->GetFireRadius(), m_MaxFireDist) * 1.5))
                        m_Environment.RemoveFromListSlowly(obj);
                    DCP();
                }
            }
            DCP();
            obj = obj->GetNextLogic();
            DCP();
        }
    }
    else if (type == 1) {
        DCP();

        int r = GetRegion();
        DCP();

        // Get info about enemies by radio
        obj = CMatrixMapStatic::GetFirstLogic();
        DCP();
        while (obj) {
            DCP();
            if (this != obj && obj->GetObjectType() == OBJECT_TYPE_ROBOTAI && obj->GetSide() == GetSide()) {
                DCP();
                if (obj->AsRobot()->m_GroupLogic == m_GroupLogic) {  // Если водной группе
                    DCP();
                    CEnemy *enemie = obj->AsRobot()->m_Environment.m_FirstEnemy;
                    while (enemie) {
                        DCP();
                        if (enemie->GetEnemy()->GetSide() != GetSide()) {
                            if (!enemie->m_DelSlowly)
                                m_Environment.AddToListSlowly(enemie->GetEnemy());
                        }
                        enemie = enemie->m_NextEnemy;
                    }
                }
                else if (obj->AsRobot()->GetTeam() == GetTeam() &&
                         obj->AsRobot()->GetRegion() == r) {  // Если в одной команде и в одном регионе
                    CEnemy *enemie = obj->AsRobot()->m_Environment.m_FirstEnemy;
                    while (enemie) {
                        DCP();
                        if (enemie->GetEnemy()->GetSide() != GetSide()) {
                            if (!enemie->m_DelSlowly)
                                m_Environment.AddToListSlowly(enemie->GetEnemy());
                        }
                        enemie = enemie->m_NextEnemy;
                        DCP();
                    }
                }
                DCP();
            }
            DCP();
            obj = obj->GetNextLogic();
            DCP();
        }

        //    } else {
        //        m_Environment.RemoveAllSlowely();
    }

    /*    CMatrixGroupObject* gobj = my_group->m_FirstObject;

        while(gobj){
            if(gobj->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI){
                CMatrixRobotAI* bot = (CMatrixRobotAI*)gobj->GetObject();
                if(bot->m_Environment.GetEnemyCnt() > 0){
                    CEnemy* enemies = bot->m_Environment.m_FirstEnemy;
                    while(enemies){
                        if(!m_Environment.SearchEnemy(enemies->GetEnemy()))
                            m_Environment.AddToList(enemies->GetEnemy());
                        enemies = enemies->m_NextEnemy;
                    }
                }
            }
            gobj = gobj->m_NextObject;
        }*/
    // Classify all enemies
    CEnemy *enemies = m_Environment.m_FirstEnemy;
    DCP();
    while (enemies) {
        DCP();
        enemies->ClassifyEnemy(this);
        enemies = enemies->m_NextEnemy;
        DCP();
    }
    DCP();
}

SOrder *CMatrixRobotAI::AllocPlaceForOrderOnTop(void) {
    if (m_OrdersInPool >= MAX_ORDERS)
        return NULL;

    if (GetBase() && !FLAG(m_ObjectState, ROBOT_FLAG_DISABLE_MANUAL)) {
        // base capturing
        GetBase()->Close();
        SetBase(NULL);
    }

    if (m_OrdersInPool > 0) {
        MoveMemory(&m_OrdersList[1], &m_OrdersList[0], sizeof(SOrder) * m_OrdersInPool);
    }
    m_OrdersInPool++;
    return m_OrdersList;
}

// void CMatrixRobotAI::AddOrderToEnd(const SOrder &order)
//{
//    if(m_OrdersInPool > 0){
//        MoveMemory(&m_OrdersList[0], &m_OrdersList[1], sizeof(SOrder) * m_OrdersInPool-1);
//    }
//    m_OrdersList[m_OrdersInPool-1] = order;
//    m_OrdersInPool++;
//
//}

void CMatrixRobotAI::RemoveOrder(int pos) {
    if (pos < 0 || pos >= m_OrdersInPool)
        return;
    m_OrdersList[pos].Release();

    --m_OrdersInPool;
    memcpy(m_OrdersList + pos, m_OrdersList + pos + 1, sizeof(SOrder) * (m_OrdersInPool - pos));
}

void CMatrixRobotAI::RemoveOrder(OrderType order) {
    for (int i = 0; i < m_OrdersInPool;) {
        if (m_OrdersList[i].GetOrderType() == order) {
            RemoveOrder(i);
            continue;
        }
        ++i;
    }
}

void CMatrixRobotAI::ProcessOrdersList() {
    if (m_OrdersInPool <= 1)
        return;

    SOrder tmp_order;
    tmp_order = m_OrdersList[0];

    MoveMemory(&m_OrdersList[0], &m_OrdersList[1], sizeof(SOrder) * m_OrdersInPool - 1);

    m_OrdersList[m_OrdersInPool - 1] = tmp_order;
}

bool CMatrixRobotAI::HaveBomb(void) const {
    for (int nC = 0; nC < m_WeaponsCnt; ++nC) {
        if (m_Weapons[nC].IsEffectPresent() && m_Weapons[nC].GetWeaponType() == WEAPON_BIGBOOM)
            return true;
    }
    return false;
}

void CMatrixRobotAI::MoveTo(int mx, int my) {
    // if(this == g_MatrixMap->GetPlayerSide()->GetArcadedObject()){
    //    ASSERT(1);
    //}

    DTRACE();

    RemoveOrder(ROT_MOVE_TO);
    RemoveOrder(ROT_MOVE_TO_BACK);
    RemoveOrder(ROT_STOP_MOVE);

    SOrder *order = AllocPlaceForOrderOnTop();
    if (order == NULL)
        return;

    order->SetOrder(ROT_MOVE_TO, (float)mx, (float)my, 0, 0);

    m_ZoneDes = -1;
    m_ZonePathCnt = 0;
    m_ZonePathNext = -1;
    // m_ZoneNear = -1;
    m_MovePathCnt = 0;
    m_MovePathCur = 0;
    m_DesX = mx;
    m_DesY = my;

#if (defined _DEBUG) && !(defined _RELDEBUG)
    if (m_DesX < 0 || m_DesX >= g_MatrixMap->m_SizeMove.x)
        debugbreak();
    if (m_DesY < 0 || m_DesY >= g_MatrixMap->m_SizeMove.y)
        debugbreak();
#endif
}

void CMatrixRobotAI::MoveToBack(int mx, int my) {
    DTRACE();

    RemoveOrder(ROT_MOVE_TO);
    RemoveOrder(ROT_MOVE_TO_BACK);
    RemoveOrder(ROT_STOP_MOVE);

    SOrder *order = AllocPlaceForOrderOnTop();
    if (order == NULL)
        return;
    order->SetOrder(ROT_MOVE_TO_BACK, (float)mx, (float)my, 0, 0);

    m_ZoneDes = -1;
    m_ZonePathCnt = 0;
    m_ZonePathNext = -1;
    // m_ZoneNear = -1;
    m_MovePathCnt = 0;
    m_MovePathCur = 0;
    m_DesX = mx;
    m_DesY = my;

#if (defined _DEBUG) && !(defined _RELDEBUG)
    if (m_DesX < 0 || m_DesX >= g_MatrixMap->m_SizeMove.x)
        debugbreak();
    if (m_DesY < 0 || m_DesY >= g_MatrixMap->m_SizeMove.y)
        debugbreak();
#endif
}

void CMatrixRobotAI::MoveReturn(int mx, int my) {
    for (int cnt = 0; cnt < m_OrdersInPool; cnt++) {
        if (m_OrdersList[cnt].GetOrderType() == ROT_MOVE_RETURN) {
            m_OrdersList[cnt].SetOrder(ROT_MOVE_TO, (float)mx, (float)my, 0, 0);
            return;
        }
    }
    SOrder *order = AllocPlaceForOrderOnTop();
    if (order == NULL)
        return;
    order->SetOrder(ROT_MOVE_RETURN, (float)mx, (float)my, 0, 0);
}

void CMatrixRobotAI::MoveToHigh(int mx, int my) {
    DTRACE();

    // if (FindOrderLikeThat(ROT_CAPTURE_FACTORY, ROP_CAPTURE_IN_POSITION) || FindOrderLikeThat(ROT_CAPTURE_FACTORY,
    // ROP_CAPTURE_SETTING_UP))
    //{
    //    return;
    //}

    StopCapture();
    RemoveOrder(ROT_MOVE_RETURN);
    MoveTo(mx, my);
}

void CMatrixRobotAI::StopMoving(void) {
    DTRACE();

    for (int cnt = 0; cnt < m_OrdersInPool; cnt++) {
        if (m_OrdersList[cnt].GetOrderType() == ROT_MOVE_TO || m_OrdersList[cnt].GetOrderType() == ROT_MOVE_TO_BACK) {
            m_OrdersList[cnt].SetOrder(ROT_STOP_MOVE, 0, 0, 0, 0);
        }
    }
    m_ZoneDes = -1;
    m_ZonePathCnt = 0;
    m_ZonePathNext = -1;
    // m_ZoneNear = -1;
    m_MovePathCnt = 0;
    m_MovePathCur = 0;

    // LowLevelStop();
}

void CMatrixRobotAI::Fire(const D3DXVECTOR3 &fire_pos, int type) {
    DTRACE();

    RemoveOrder(ROT_FIRE);
    SOrder *order = AllocPlaceForOrderOnTop();
    if (order == NULL)
        return;
    order->SetOrder(ROT_FIRE, fire_pos.x, fire_pos.y, fire_pos.z, type);
}

void CMatrixRobotAI::StopFire(void) {
    DTRACE();
    // m_FireTarget = NULL;
    for (int cnt = 0; cnt < m_OrdersInPool; cnt++) {
        if (m_OrdersList[cnt].GetOrderType() == ROT_FIRE) {
            m_OrdersList[cnt].SetOrder(ROT_STOP_FIRE, 0, 0, 0);
        }
    }
}

void CMatrixRobotAI::BigBoom(int nc) {
    if (nc < 0) {
        for (nc = 0; nc < m_WeaponsCnt; ++nc) {
            if (m_Weapons[nc].IsEffectPresent() && m_Weapons[nc].GetWeaponType() == WEAPON_BIGBOOM)
                break;
        }
        if (nc >= m_WeaponsCnt)
            return;
    }
    if (nc >= 0) {
        m_Weapons[nc].Modify(GetGeoCenter(), D3DXVECTOR3(m_PosX, m_PosY, 0), m_Velocity * (1.0f / LOGIC_TAKT_PERIOD));
        m_Weapons[nc].FireBegin(m_Velocity * (1.0f / LOGIC_TAKT_PERIOD), this);
        m_Weapons[nc].Takt(1);
    }

    MustDie();
}

struct SSeekCaptureMeB {
    CMatrixBuilding *b;
    CMatrixRobotAI *r;
    float dist2;
};

void CMatrixRobotAI::CaptureFactory(CMatrixBuilding *factory) {
    DTRACE();

    RemoveOrder(ROT_CAPTURE_FACTORY);

    SOrder *capture_order = AllocPlaceForOrderOnTop();
    if (capture_order == NULL)
        return;
    capture_order->SetOrder(ROT_CAPTURE_FACTORY, factory);
}

CMatrixBuilding *CMatrixRobotAI::GetCaptureFactory(void) {
    for (int cnt = 0; cnt < m_OrdersInPool; cnt++) {
        if (m_OrdersList[cnt].GetOrderType() == ROT_CAPTURE_FACTORY) {
            return (CMatrixBuilding *)m_OrdersList[cnt].GetStatic();
        }
    }
    return NULL;
}

void CMatrixRobotAI::StopCapture(void) {
    DTRACE();

    //#ifdef _DEBUG
    //    if (FindOrderLikeThat(ROT_CAPTURE_FACTORY, ROP_CAPTURE_IN_POSITION) || FindOrderLikeThat(ROT_CAPTURE_FACTORY,
    //    ROP_CAPTURE_SETTING_UP))
    //    {
    //        _asm int 3
    //    }
    //#endif

    for (int cnt = 0; cnt < m_OrdersInPool; cnt++) {
        if (m_OrdersList[cnt].GetOrderType() == ROT_CAPTURE_FACTORY) {
            m_OrdersList[cnt].Release();
            CMatrixMapStatic *ms = m_OrdersList[cnt].GetStatic();
            m_OrdersList[cnt].SetOrder(ROT_STOP_CAPTURE, ms);
        }
        else if (m_OrdersList[cnt].GetOrderType() == ROT_MOVE_TO &&
                 m_OrdersList[cnt].GetOrderPhase() == ROP_CAPTURE_MOVING) {
            m_OrdersList[cnt].Release();
            m_OrdersList[cnt].SetOrder(ROT_STOP_MOVE, 0, 0, 0, 0);
        }
    }
}

void CMatrixRobotAI::TaktCaptureCandidate(int ms) {
    DTRACE();

    CMatrixBuilding *bc = NULL;

    for (int i = 0; i < m_CaptureCandidatesCnt;) {
        m_CaptureCandidates[i].tbc -= ms;
        if (m_CaptureCandidates[i].tbc < 0) {
            CMatrixBuilding *b = (CMatrixBuilding *)m_CaptureCandidates[i].bcore->m_Object;
            if (b != NULL && b->m_Side != m_Side) {
                if (b->m_Capturer == NULL) {
                    if (bc) {
                        if (bc->m_TurretsHave > b->m_TurretsHave)
                            bc = b;
                        else if (bc->m_TurretsHave == b->m_TurretsHave) {
                            if (b->m_Side == 0)
                                bc = b;  // neutral buildings are more prioritized
                        }
                    }
                    else {
                        bc = b;
                    }
                }
            }
            else {
                m_CaptureCandidates[i].bcore->Release();
                m_CaptureCandidates[i] = m_CaptureCandidates[--m_CaptureCandidatesCnt];
                continue;
            }
        }
        ++i;
    }

    if (bc != NULL) {
        CaptureFactory(bc);
        ClearCaptureCandidates();
    }
}

void CMatrixRobotAI::ClearCaptureCandidates(void) {
    DTRACE();
    for (int i = 0; i < m_CaptureCandidatesCnt; ++i) {
        m_CaptureCandidates[i].bcore->Release();
    }
    m_CaptureCandidatesCnt = 0;
}

void CMatrixRobotAI::AddCaptureCandidate(CMatrixBuilding *b) {
    DTRACE();

    if (FindOrderLikeThat(ROT_CAPTURE_FACTORY))
        return;

    MarkCaptureInformed();

    if (m_CaptureCandidatesCnt >= MAX_CAPTURE_CANDIDATES)
        return;

    int i;
    for (i = 0; i < m_CaptureCandidatesCnt; ++i) {
        if (m_CaptureCandidates[i].bcore->m_Object == b)
            return;
    }
    m_CaptureCandidates[i].bcore = b->GetCore(DEBUG_CALL_INFO);
    m_CaptureCandidates[i].tbc = g_MatrixMap->Rnd(100, 5000);
    ++m_CaptureCandidatesCnt;
}

void CMatrixRobotAI::RemoveCaptureCandidate(CMatrixBuilding *b) {
    DTRACE();
    for (int i = 0; i < m_CaptureCandidatesCnt; ++i) {
        if (m_CaptureCandidates[i].bcore->m_Object == b) {
            m_CaptureCandidates[i].bcore->Release();
            m_CaptureCandidates[i] = m_CaptureCandidates[--m_CaptureCandidatesCnt];
            return;
        }
    }
}

bool CMatrixRobotAI::FindOrder(OrderType findOrder, CMatrixMapStatic *obj) {
    DTRACE();
    void *param = NULL;
    for (int cnt = 0; cnt < m_OrdersInPool; cnt++) {
        if (m_OrdersList[cnt].GetOrderType() == findOrder && m_OrdersList[cnt].GetStatic() == obj) {
            return true;
        }
    }
    return false;
}

void CMatrixRobotAI::BreakAllOrders(void) {
    DTRACE();
    LowLevelStop();

    DCP();

    m_Environment.Reset();

    DCP();
    LowLevelStopFire();

    DCP();
    for (int cnt = 0; cnt < m_OrdersInPool; cnt++) {
        if (m_OrdersList[cnt].GetOrderType() == ROT_CAPTURE_FACTORY) {
            CMatrixBuilding *factory = (CMatrixBuilding *)m_OrdersList[cnt].GetStatic();
            if (factory && factory->IsCaptured() && factory->m_Capturer == this) {
                factory->ResetCaptured();

                if (!FLAG(m_ObjectState, ROBOT_FLAG_DISABLE_MANUAL)) {
                    if (factory->IsBase() && factory->m_State != BASE_CLOSED)
                        factory->Close();
                }
                else {
                    MustDie();
                }
            }
        }
    }
    DCP();

    while (m_OrdersInPool > 0) {
        RemoveOrder(m_OrdersInPool - 1);
    }
}

bool CMatrixRobotAI::FindOrderLikeThat(OrderType order) const {
    for (int cnt = 0; cnt < m_OrdersInPool; ++cnt) {
        if (m_OrdersList[cnt].GetOrderType() == order)
            return true;
    }
    return false;
}

bool CMatrixRobotAI::FindOrderLikeThat(OrderType order, OrderPhase phase) {
    for (int cnt = 0; cnt < m_OrdersInPool; cnt++) {
        if (m_OrdersList[cnt].GetOrderType() == order && m_OrdersList[cnt].GetOrderPhase() == phase)
            return true;
    }
    return false;
}

void CMatrixRobotAI::UpdateOrder_MoveTo(int mx, int my) {
    for (int cnt = 0; cnt < m_OrdersInPool; cnt++) {
        if (m_OrdersList[cnt].GetOrderType() == ROT_MOVE_TO) {
            // g_MatrixMap->PlaceFindNear(m_Unit[0].u1.s1.m_Kind-1,4,mx,my);
            m_OrdersList[cnt].Release();
            m_OrdersList[cnt].SetOrder(ROT_MOVE_TO, (float)mx, (float)my, 0);
            return;
        }
    }
}

bool CMatrixRobotAI::GetMoveToCoords(CPoint &pos) {
    float f1, f2;
    for (int cnt = 0; cnt < m_OrdersInPool; cnt++) {
        if (m_OrdersList[cnt].GetOrderType() == ROT_MOVE_TO) {
            m_OrdersList[cnt].GetParams(&f1, &f2, NULL);
            pos.x = Float2Int(f1);
            pos.y = Float2Int(f2);
            return true;
        }
    }
    return false;
}

bool CMatrixRobotAI::GetReturnCoords(CPoint &pos) {
    float f1, f2;
    for (int cnt = 0; cnt < m_OrdersInPool; cnt++) {
        if (m_OrdersList[cnt].GetOrderType() == ROT_MOVE_RETURN) {
            m_OrdersList[cnt].GetParams(&f1, &f2, NULL);
            pos.x = Float2Int(f1);
            pos.y = Float2Int(f2);
            return true;
        }
    }
    return false;
}

void CMatrixRobotAI::LowLevelStopFire(void) {
    for (int nC = 0; nC < m_WeaponsCnt; nC++) {
        if (m_Weapons[nC].IsEffectPresent()) {
            m_Weapons[nC].FireEnd();
        }
    }
}

void CMatrixRobotAI::LowLevelStop(void) {
    m_Speed = 0;
    m_Velocity = D3DXVECTOR3(0, 0, 0);
}

bool CMatrixRobotAI::FindWeapon(EWeapon type) {
    for (int nC = 0; nC < m_WeaponsCnt; ++nC) {
        if (m_Weapons[nC].IsEffectPresent() && m_Weapons[nC].GetWeaponType() == type) {
            return true;
        }
    }
    return false;
}

void CMatrixRobotAI::ReleaseMe(void) {
    DTRACE();

    UnSelect();
    BreakAllOrders();
    ClearCaptureCandidates();

    DCP();

    for (int nC = 0; nC < m_WeaponsCnt; nC++) {
        if (m_Weapons[nC].IsEffectPresent()) {
            m_Weapons[nC].FireEnd();
            m_Weapons[nC].Release();
        }
    }
    m_WeaponsCnt = 0;

    DCP();

    if (GetSide() == PLAYER_SIDE) {
        CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();

        ps->RemoveFromSelection(this);

        int pos = 0;

        if (ps->IsArcadeMode() && this == ps->GetArcadedObject() && g_IFaceList) {
            CInterface *ifs = g_IFaceList->m_First;
            while (ifs) {
                if (ifs->m_strName == IF_MAIN) {
                    ifs->m_xPos = float(g_ScreenX - (1024 - 447));
                    ifs->ReCalcElementsPos();
                    break;
                }
                ifs = ifs->m_NextInterface;
            }
        }
        if (ps->GetCurGroup()) {
            CMatrixGroupObject *go = ps->GetCurGroup()->m_FirstObject;
            while (go) {
                pos++;
                if (go->GetObject() == this) {
                    break;
                }
                go = go->m_NextObject;
            }

            if (ps->GetCurGroup()->FindObject(this)) {
                ps->GetCurGroup()->RemoveObject(this);
                if (!ps->GetCurGroup()->GetObjectsCnt()) {
                    ps->PLDropAllActions();
                }
            }
        }

        if (ps->GetCurSelNum() == pos) {
            pos = 0;
        }
        else {
            pos = -1;
        }

        DeleteProgressBarClone(PBC_CLONE1);
        DeleteProgressBarClone(PBC_CLONE2);
        if (g_IFaceList) {
            g_IFaceList->DeleteProgressBars(this);
            if (ps->m_CurrSel == GROUP_SELECTED) {
                g_IFaceList->CreateGroupIcons();
            }
            ps->SetCurSelNum(pos);
        }
    }

    CMatrixMapStatic *objects = CMatrixMapStatic::GetFirstLogic();
    while (objects) {
        if (objects->IsLiveRobot() && objects->AsRobot() != this) {
            objects->AsRobot()->m_Environment.RemoveFromList(this);
        }
        objects = objects->GetNextLogic();
    }

    m_Environment.Clear();

    if (m_CurrState == ROBOT_CARRYING) {
        Carry(NULL);
    }

    if (m_BigTexture) {
        g_Cache->Destroy(m_BigTexture);
        m_BigTexture = NULL;
    }
    if (m_MedTexture) {
        g_Cache->Destroy(m_MedTexture);
        m_MedTexture = NULL;
    }
#ifdef USE_SMALL_TEXTURE_IN_ROBOT_ICON
    if (m_SmallTexture) {
        g_Cache->Destroy(m_SmallTexture);
        m_SmallTexture = NULL;
    }
#endif

    if (m_ZonePath) {
        HFree(m_ZonePath, g_MatrixHeap);
        m_ZonePath = NULL;
    }

    if (this == g_MatrixMap->GetPlayerSide()->GetArcadedObject()) {
        g_MatrixMap->GetPlayerSide()->SetArcadedObject(NULL);
    }
}

void CMatrixRobotAI::GetLost(const D3DXVECTOR3 &v) {
    if (this == g_MatrixMap->GetPlayerSide()->GetArcadedObject())
        return;

    D3DXVECTOR3 f1, f2;

    D3DXVec3Normalize(&f1, &m_Forward);
    D3DXVec3Normalize(&f2, &v);

    float cos1 = f1.x * f2.x + f1.y * f2.y;
    float angle = (float)acos(cos1);

    if (angle > GRAD2RAD(70)) {
        RotateRobot((D3DXVECTOR3(m_PosX, m_PosY, 0) + v));
        return;
    }

    D3DXVECTOR3 vLeft(0, 0, 0), vRight(0, 0, 0);
    float lost_len = 0, lost_param = 0, v_param = 0;

    vLeft.x = v.y;
    vLeft.y = -v.x;

    vRight = -vLeft;

    v_param = (float)g_MatrixMap->RndFloat(0, 1);
    lost_param = (float)g_MatrixMap->RndFloat(0, 1);

    D3DXVECTOR3 vLost = LERPVECTOR(v_param, vLeft, vRight);
    lost_len = LERPFLOAT(lost_param, GET_LOST_MIN, GET_LOST_MAX);

    vLost *= lost_len;
    vLost += v;

    vLost += D3DXVECTOR3(m_PosX, m_PosY, 0);
    int mx = Float2Int(vLost.x / GLOBAL_SCALE_MOVE);
    int my = Float2Int(vLost.y / GLOBAL_SCALE_MOVE);

    g_MatrixMap->PlaceFindNear(m_Unit[0].u1.s1.m_Kind - 1, 4, mx, my, this);
    if (!FindOrderLikeThat(ROT_MOVE_TO, ROP_GETING_LOST)) {
        MoveTo(mx, my);
        for (int cnt = 0; cnt < m_OrdersInPool; ++cnt) {
            if (m_OrdersList[cnt].GetOrderType() == ROT_MOVE_TO) {
                m_OrdersList[cnt].SetPhase(ROP_GETING_LOST);
                break;
            }
        }
    }
}

bool CMatrixRobotAI::SelectByGroup() {
    // UnSelect();
    RESETFLAG(m_ObjectState, ROBOT_FLAG_SARCADE);

    if ((!FLAG(m_ObjectState, ROBOT_FLAG_SGROUP) && !m_Selection) && CreateSelection()) {
        SETFLAG(m_ObjectState, ROBOT_FLAG_SGROUP);
        return true;
    }
    else if (!m_Selection) {
        RESETFLAG(m_ObjectState, ROBOT_FLAG_SGROUP);
        return false;
    }
    return true;
}

bool CMatrixRobotAI::SelectArcade() {
    // UnSelect();
    RESETFLAG(m_ObjectState, ROBOT_FLAG_SGROUP);

    if ((!FLAG(m_ObjectState, ROBOT_FLAG_SARCADE) && !m_Selection) && CreateSelection()) {
        SETFLAG(m_ObjectState, ROBOT_FLAG_SARCADE);
        return true;
    }
    else if (!m_Selection) {
        RESETFLAG(m_ObjectState, ROBOT_FLAG_SARCADE);
        return false;
    }
    return true;
}

void CMatrixRobotAI::UnSelect() {
    KillSelection();

    RESETFLAG(m_ObjectState, ROBOT_FLAG_SGROUP);
    RESETFLAG(m_ObjectState, ROBOT_FLAG_SARCADE);
}

bool CMatrixRobotAI::CreateSelection() {
    m_Selection = (CMatrixEffectSelection *)CMatrixEffect::CreateSelection(
            D3DXVECTOR3(m_PosX, m_PosY, m_Core->m_Matrix._43 + 3 /*ROBOT_SELECTION_HEIGHT*/), ROBOT_SELECTION_SIZE);
    if (!g_MatrixMap->AddEffect(m_Selection)) {
        m_Selection = NULL;
        return false;
    }
    return true;
}

void CMatrixRobotAI::KillSelection() {
    if (m_Selection) {
        m_Selection->Kill();
        m_Selection = NULL;
    }
}

void CMatrixRobotAI::MoveSelection() {
    if (m_Selection) {
        m_Selection->SetPos(D3DXVECTOR3(m_PosX, m_PosY, m_Core->m_Matrix._43 + 3 /*ROBOT_SELECTION_HEIGHT*/));
    }
}

bool CMatrixRobotAI::IsSelected() {
    if (FLAG(m_ObjectState, ROBOT_FLAG_SGROUP) || FLAG(m_ObjectState, ROBOT_FLAG_SARCADE)) {
        return true;
    }
    return false;
}

void CMatrixRobotAI::CalcStrength()  // Расчитываем силу робота
{
    m_Strength = 0;

    for (int i = 0; i < m_WeaponsCnt; ++i) {
        switch (m_Weapons[i].GetWeaponType()) {
            case WEAPON_PLASMA:
                m_Strength += g_Config.m_WeaponStrengthAI[RUK_WEAPON_PLASMA];
                break;
            case WEAPON_VOLCANO:
                m_Strength += g_Config.m_WeaponStrengthAI[RUK_WEAPON_MACHINEGUN];
                break;
            case WEAPON_HOMING_MISSILE:
                m_Strength += g_Config.m_WeaponStrengthAI[RUK_WEAPON_MISSILE];
                break;
            case WEAPON_BOMB:
                m_Strength += g_Config.m_WeaponStrengthAI[RUK_WEAPON_MORTAR];
                break;
            case WEAPON_FLAMETHROWER:
                m_Strength += g_Config.m_WeaponStrengthAI[RUK_WEAPON_FLAMETHROWER];
                break;
            case WEAPON_BIGBOOM:
                m_Strength += g_Config.m_WeaponStrengthAI[RUK_WEAPON_BOMB];
                break;
            case WEAPON_LIGHTENING:
                m_Strength += g_Config.m_WeaponStrengthAI[RUK_WEAPON_ELECTRIC];
                break;
            case WEAPON_LASER:
                m_Strength += g_Config.m_WeaponStrengthAI[RUK_WEAPON_LASER];
                break;
            case WEAPON_GUN:
                m_Strength += g_Config.m_WeaponStrengthAI[RUK_WEAPON_CANNON];
                break;
        }
    }

    m_Strength = m_Strength * (0.4f + 0.6f * (m_HitPoint / m_HitPointMax));
}

void CMatrixRobotAI::CreateProgressBarClone(float x, float y, float width, EPBCoord clone_type) {
    m_PB.CreateClone(clone_type, x, y, width);
}

void CMatrixRobotAI::DeleteProgressBarClone(EPBCoord clone_type) {
    m_PB.KillClone(clone_type);
}

void CMatrixRobotAI::CreateTextures() {
#ifdef USE_SMALL_TEXTURE_IN_ROBOT_ICON
    SRenderTexture rt[3];
    rt[0].ts = TEXSIZE_256;
    rt[1].ts = TEXSIZE_64;
    rt[2].ts = TEXSIZE_32;

    if (RenderToTexture(rt, 3)) {
        m_BigTexture = rt[0].tex;
        m_MedTexture = rt[1].tex;
        m_SmallTexture = rt[2].tex;
    }
    else {
        m_BigTexture = NULL;
        m_MedTexture = NULL;
        m_SmallTexture = NULL;
    }
#else
    SRenderTexture rt[2];
    rt[0].ts = TEXSIZE_256;
    rt[1].ts = TEXSIZE_64;

    if (RenderToTexture(rt, 2)) {
        m_BigTexture = rt[0].tex;
        m_MedTexture = rt[1].tex;
    }
    else {
        m_BigTexture = NULL;
        m_MedTexture = NULL;
    }
#endif
}

void CMatrixRobotAI::PlayHullSound() {
    if (m_Unit[1].u1.s1.m_Kind == RUK_ARMOR_PASSIVE) {
        CSound::Play(S_HULL_PASSIVE, GetGeoCenter(), SL_HULL);
    }
    else if (m_Unit[1].u1.s1.m_Kind == RUK_ARMOR_ACTIVE) {
        CSound::Play(S_HULL_ACTIVE, GetGeoCenter(), SL_HULL);
    }
    else if (m_Unit[1].u1.s1.m_Kind == RUK_ARMOR_FIREPROOF) {
        CSound::Play(S_HULL_FIREPROOF, GetGeoCenter(), SL_HULL);
    }
    else if (m_Unit[1].u1.s1.m_Kind == RUK_ARMOR_PLASMIC) {
        CSound::Play(S_HULL_PLASMIC, GetGeoCenter(), SL_HULL);
    }
    else if (m_Unit[1].u1.s1.m_Kind == RUK_ARMOR_NUCLEAR) {
        CSound::Play(S_HULL_NUCLEAR, GetGeoCenter(), SL_HULL);
    }
    else if (m_Unit[1].u1.s1.m_Kind == RUK_ARMOR_6) {
        CSound::Play(S_HULL_6, GetGeoCenter(), SL_HULL);
    }
}

void CMatrixRobotAI::SetWeaponToArcadedCoeff() {
    for (int i = 0; i < m_WeaponsCnt; ++i) {
        m_Weapons[i].SetArcadeCoefficient();
    }
}

void CMatrixRobotAI::SetWeaponToDefaultCoeff() {
    for (int i = 0; i < m_WeaponsCnt; ++i) {
        m_Weapons[i].SetDefaultCoefficient();
    }
}

bool CMatrixRobotAI::CheckFireDist(const D3DXVECTOR3 &point) {
    // CHelper::DestroyByGroup(0);
    for (int i = 0; i < m_WeaponsCnt; ++i) {
        if (m_Weapons[i].IsEffectPresent()) {
            D3DXVECTOR3 wpos(0, 0, 0);
            if (m_Weapons[i].GetWeaponPos(wpos)) {
                D3DXVECTOR3 hitpos(0, 0, 0);
                D3DXVECTOR3 out(0, 0, 0);
                auto tmp = point - wpos;
                D3DXVec3Normalize(&out, &tmp);
                out *= (m_Weapons[i].GetWeaponDist());
                // CHelper::Create(1,0)->Line(wpos, wpos+out);
                CMatrixMapStatic *hito = g_MatrixMap->Trace(&hitpos, wpos, wpos + out, TRACE_ALL, this);
                if (hito == g_MatrixMap->m_TraceStopObj) {
                    // if(POW2(m_Weapons[i].GetWeaponDist()) > D3DXVec3LengthSq(&(hitpos-wpos))){
                    return true;
                    //}
                }
            }
        }
    }
    return false;
}

// void SOrder::LogicTakt()
//{
////    CDText::T("orders", m_OrdersInPool);
//    switch(m_OrderType){
//        case ROT_MOVE_TO:
//            if(m_OrderPhase != ROBOT_MOVING)
//                m_OrderPhase = ROBOT_MOVING;
//            break;
//        case ROBOT_FIRE:
//            break;
//        case ROT_CAPTURE_FACTORY:
//            break;
//        default:
//            break;
//    }
//
//}
