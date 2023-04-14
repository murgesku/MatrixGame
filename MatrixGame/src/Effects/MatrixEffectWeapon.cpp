// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>

#include "../MatrixMap.hpp"
#include "../MatrixObject.hpp"
#include "../MatrixFlyer.hpp"
#include "../MatrixRobot.hpp"
#include <math.h>

#include "MatrixEffect.hpp"
#include "MatrixEffectLightening.hpp"
#include "MatrixEffectFlame.hpp"
#include "MatrixEffectPointLight.hpp"
#include "MatrixEffectMovingObject.hpp"
#include "MatrixEffectExplosion.hpp"

void CMatrixEffectWeapon::WeaponHit(CMatrixMapStatic *hiti, const D3DXVECTOR3 &pos, DWORD user, DWORD flags) {
    DTRACE();

    CMatrixEffectWeapon *w = (CMatrixEffectWeapon *)user;

    bool give_damage = true;
    if (w->m_Type == WEAPON_HOMING_MISSILE || w->m_Type == WEAPON_GUN || w->m_Type == WEAPON_CANNON0 ||
        w->m_Type == WEAPON_CANNON1 || w->m_Type == WEAPON_CANNON3) {
        if (POW2(w->m_WeaponDist * w->m_WeaponCoefficient) < m_Dist2)
            give_damage = false;
    }

    bool odead = false;

    DWORD flags_add = 0;

    if (give_damage && IS_TRACE_STOP_OBJECT(hiti)) {
        if (w->m_Type == WEAPON_BIGBOOM) {
            D3DXVECTOR3 dir(pos - w->m_Pos);
            D3DXVec3Normalize(&dir, &dir);
            odead = hiti->Damage(w->m_Type, pos, dir, w->GetSideStorage(), w->GetOwner());
        }
        else {
            odead = hiti->Damage(w->m_Type, pos, w->m_Dir, w->GetSideStorage(), w->GetOwner());
        }
        if (!odead) {
            if (hiti->IsRobot()) {
                flags_add = FEHF_DAMAGE_ROBOT;
            }
        }
    }
    else if (hiti == TRACE_STOP_WATER && w->m_Type != WEAPON_FLAMETHROWER && w->m_Type != WEAPON_BIGBOOM) {
        CMatrixEffect::CreateKonusSplash(pos, D3DXVECTOR3(0, 0, 1), 10, 5, FSRND(M_PI), 1000, true,
                                         (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_SPLASH));
    }

    if (odead)
        hiti = TRACE_STOP_NONE;

    if (w->m_Handler)
        w->m_Handler(hiti, pos, w->m_User, flags | flags_add);

    if (FLAG(flags, FEHF_LASTHIT)) {
        w->Release();
    }
}

CMatrixEffectWeapon::CMatrixEffectWeapon(const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, DWORD user,
                                         FIRE_END_HANDLER handler, EWeapon type, int cooldown)
  : CMatrixEffect(), m_Type(type), m_User(user), m_Handler(handler), m_Pos(pos), m_Dir(dir),
    m_CoolDown(cooldown ? float(cooldown) : ((float)(int)type)), m_Time(0), m_Volcano(NULL), m_Ref(1),
    m_Sound(SOUND_ID_EMPTY), m_Owner(NULL), m_SideStorage(0)
#ifdef _DEBUG
    ,
    m_Effect(DEBUG_CALL_INFO)
#endif
{
    DTRACE();

    m_EffectType = EFFECT_WEAPON;

    // Zak sorry for this shit. Sub.
    // its ok. I'v added some crap to this shit :). Zak

    int widx = Weap2Index(m_Type);

    m_WeaponDist = g_Config.m_WeaponRadius[widx];
    m_WeaponCoefficient = DEFBOT_WEAPON_COEFF;
    if (g_Config.m_WeaponCooldown[widx] > 0)
        m_CoolDown = (float)g_Config.m_WeaponCooldown[widx];

    switch (m_Type) {
        case WEAPON_PLASMA:
            m_SoundType = S_WEAPON_PLASMA;
            RESETFLAG(m_Flags, WEAPFLAGS_SND_OFF);
            RESETFLAG(m_Flags, WEAPFLAGS_SND_SKIP);
            break;
        case WEAPON_VOLCANO:
            m_SoundType = S_WEAPON_VOLCANO;
            SETFLAG(m_Flags, WEAPFLAGS_SND_OFF);
            SETFLAG(m_Flags, WEAPFLAGS_SND_SKIP);
            break;
        case WEAPON_HOMING_MISSILE:
            m_SoundType = S_WEAPON_HOMING_MISSILE;
            RESETFLAG(m_Flags, WEAPFLAGS_SND_OFF);
            RESETFLAG(m_Flags, WEAPFLAGS_SND_SKIP);
            break;
        case WEAPON_BOMB:
            m_SoundType = S_WEAPON_BOMB;
            RESETFLAG(m_Flags, WEAPFLAGS_SND_OFF);
            RESETFLAG(m_Flags, WEAPFLAGS_SND_SKIP);
            break;
        case WEAPON_FLAMETHROWER:
            m_SoundType = S_WEAPON_FLAMETHROWER;
            SETFLAG(m_Flags, WEAPFLAGS_SND_OFF);
            SETFLAG(m_Flags, WEAPFLAGS_SND_SKIP);
            break;
        case WEAPON_BIGBOOM:
            m_SoundType = S_WEAPON_BIGBOOM;
            RESETFLAG(m_Flags, WEAPFLAGS_SND_OFF);
            RESETFLAG(m_Flags, WEAPFLAGS_SND_SKIP);
            break;
        case WEAPON_LIGHTENING:
            m_SoundType = S_WEAPON_LIGHTENING;
            SETFLAG(m_Flags, WEAPFLAGS_SND_OFF);
            SETFLAG(m_Flags, WEAPFLAGS_SND_SKIP);
            break;
        case WEAPON_LASER:
            m_SoundType = S_WEAPON_LASER;
            SETFLAG(m_Flags, WEAPFLAGS_SND_OFF);
            SETFLAG(m_Flags, WEAPFLAGS_SND_SKIP);
            break;
        case WEAPON_GUN:
            m_SoundType = S_WEAPON_GUN;
            RESETFLAG(m_Flags, WEAPFLAGS_SND_OFF);
            RESETFLAG(m_Flags, WEAPFLAGS_SND_SKIP);
            break;
        case WEAPON_REPAIR:
            m_SoundType = S_WEAPON_REPAIR;
            SETFLAG(m_Flags, WEAPFLAGS_SND_OFF);
            SETFLAG(m_Flags, WEAPFLAGS_SND_SKIP);
            break;
        case WEAPON_CANNON0:
            m_SoundType = S_WEAPON_CANNON0;
            RESETFLAG(m_Flags, WEAPFLAGS_SND_OFF);
            RESETFLAG(m_Flags, WEAPFLAGS_SND_SKIP);
            break;
        case WEAPON_CANNON1:
            m_SoundType = S_WEAPON_CANNON1;
            RESETFLAG(m_Flags, WEAPFLAGS_SND_OFF);
            RESETFLAG(m_Flags, WEAPFLAGS_SND_SKIP);
            break;
        case WEAPON_CANNON2:
            m_SoundType = S_WEAPON_CANNON2;
            SETFLAG(m_Flags, WEAPFLAGS_SND_OFF);
            SETFLAG(m_Flags, WEAPFLAGS_SND_SKIP);
            break;
        case WEAPON_CANNON3:
            m_SoundType = S_WEAPON_CANNON3;
            RESETFLAG(m_Flags, WEAPFLAGS_SND_OFF);
            RESETFLAG(m_Flags, WEAPFLAGS_SND_SKIP);
            break;
        default:
            m_SoundType = S_NONE;
    }
    // EndOfShit
}

CMatrixEffectWeapon::~CMatrixEffectWeapon() {
    DTRACE();
    FireEnd();

    if (m_Type == WEAPON_FLAMETHROWER) {
        if (m_Effect.effect)
            m_Effect.Unconnect();
    }

#ifdef _DEBUG
    m_Effect.Release(DEBUG_CALL_INFO);
#else
    m_Effect.Release();
#endif

    if (m_Owner)
        m_Owner->Release();
}

void CMatrixEffectWeapon::Release(void) {
    DTRACE();
    --m_Ref;
#ifdef _DEBUG
    if (m_Ref < 0)
        debugbreak();
#endif
    if (m_Ref <= 0) {
        SetDIP();
        HDelete(CMatrixEffectWeapon, this, m_Heap);
    }
}

void CMatrixEffectWeapon::Takt(float step) {
    DTRACE();

    if (m_Time < 0)
        m_Time += step;
    if (m_Time > 0 && !IsFire())
        m_Time = 0;

    if (m_Sound != SOUND_ID_EMPTY) {
        m_Sound = CSound::ChangePos(m_Sound, m_SoundType, m_Pos);
    }

    if (IsFire()) {
        while (m_Time >= 0) {
            Fire();
            SETFLAG(m_Flags, WEAPFLAGS_FIREWAS);
            m_Time -= m_CoolDown;
        }
    }
}

void CMatrixEffectWeapon::Modify(const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &speed) {
    DTRACE();

    m_Pos = pos;
    m_Dir = dir;
    m_Speed = speed;
    if (m_Type == WEAPON_PLASMA) {
        if (m_Effect.effect && m_Effect.effect->GetType() == EFFECT_KONUS) {
            ((CMatrixEffectKonus *)m_Effect.effect)->Modify(pos, dir);
        }
    }
    else if (m_Type == WEAPON_VOLCANO) {
        if (m_Volcano) {
            m_Volcano->SetPos(pos, pos + dir * VOLCANO_FIRE_LENGHT, dir);
        }
    }
    else if (m_Type == WEAPON_LIGHTENING) {
        if (m_Effect.effect && m_Effect.effect->GetType() == EFFECT_LIGHTENING) {
            D3DXVECTOR3 hitpos(m_Pos + m_Dir * m_WeaponDist * m_WeaponCoefficient);
            g_MatrixMap->Trace(&hitpos, m_Pos, hitpos, TRACE_ALL, m_Skip);
            ((CMatrixEffectLightening *)m_Effect.effect)->SetPos(m_Pos, hitpos);
        }
    }
    else if (m_Type == WEAPON_LASER || m_Type == WEAPON_CANNON2) {
        if (m_Laser) {
            D3DXVECTOR3 hitpos(m_Pos + m_Dir * m_WeaponDist * m_WeaponCoefficient);
            g_MatrixMap->Trace(&hitpos, m_Pos, hitpos, TRACE_ALL, m_Skip);

            m_Laser->SetPos(m_Pos, hitpos);
        }
    }
    else if (m_Type == WEAPON_REPAIR) {
        if (m_Repair) {
            m_Repair->UpdateData(pos, dir);
        }
    }
}

void CMatrixEffectWeapon::Fire(void) {
    DTRACE();

    ++m_FireCount;

    if (m_SoundType != S_NONE) {
        if (FLAG(m_Flags, WEAPFLAGS_SND_SKIP)) {
            m_Sound = CSound::Play(m_Sound, m_SoundType, m_Pos);
        }
        else {
            m_Sound = CSound::Play(m_SoundType, m_Pos);
        }
    }

    DCP();

    switch (m_Type) {
        case WEAPON_PLASMA: {
            DCP();
            float ang = float(2 * M_PI / 4096.0) * (g_MatrixMap->GetTime() & 4095);
            CMatrixEffect::CreateKonus(&m_Effect, m_Pos, m_Dir, 10, 10, ang, 300, true, NULL);
            m_Ref++;
            CMatrixEffect::CreateFirePlasma(m_Pos, m_Pos + (m_Dir * m_WeaponDist * m_WeaponCoefficient), 0.5f,
                                            TRACE_ALL, m_Skip, WeaponHit, (DWORD)this);
            break;
        }
        case WEAPON_VOLCANO: {
            DCP();
            if (m_Volcano) {
                // g_MatrixMap->SubEffect(m_Konus);
                float ang = float(2 * M_PI / 4096.0) * (g_MatrixMap->GetTime() & 4095);
                m_Volcano->SetPos(m_Pos, m_Pos + m_Dir * VOLCANO_FIRE_LENGHT, m_Dir, ang);
            }
            else {
                float ang = float(2 * M_PI / 4096.0) * (g_MatrixMap->GetTime() & 4095);
                m_Volcano = HNew(m_Heap) CVolcano(m_Pos, m_Dir, ang);
                if (!g_MatrixMap->AddEffect(m_Volcano))
                    m_Volcano = NULL;
            }

            DWORD flags_add = FEHF_LASTHIT;

            D3DXVECTOR3 hitpos;
            D3DXVECTOR3 splash;
            CMatrixMapStatic *s = g_MatrixMap->Trace(&hitpos, m_Pos, m_Pos + m_Dir * m_WeaponDist * m_WeaponCoefficient,
                                                     TRACE_ALL, m_Skip);
            if (s == TRACE_STOP_NONE)
                break;
            if (IS_TRACE_STOP_OBJECT(s)) {
                bool dead = s->Damage(m_Type, hitpos, m_Dir, GetSideStorage(), GetOwner());
                SETFLAG(m_Flags, WEAPFLAGS_HITWAS);
                if (dead) {
                    s = TRACE_STOP_NONE;
                }
                splash = -m_Dir;
            }
            else if (s == TRACE_STOP_WATER) {
                splash = D3DXVECTOR3(0, 0, 1);
                CMatrixEffect::CreateKonusSplash(
                        hitpos, splash, 10, 5, FSRND(M_PI), 1000, true,
                        (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_SPLASH));
            }
            else {
                g_MatrixMap->GetNormal(&splash, hitpos.x, hitpos.y);
                CMatrixEffect::CreateKonus(
                        NULL, hitpos, splash, 5, 10, FSRND(M_PI), 300, true,
                        (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_GUN_BULLETS1));
                CMatrixEffect::CreateKonus(
                        NULL, hitpos, splash, 5, 5, FSRND(M_PI), 300, true,
                        (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_GUN_BULLETS2));
            }

            if (FRND(1) < 0.1f) {
                // CHelper::Create(1,1)->Line(m_Pos, hitpos, 0x80808080, 0x80808080);
                CMatrixEffect::CreateBillboardLine(NULL, m_Pos, hitpos, 0.5f, 0x80FFFFFF, 0, 100,
                                                   m_BBTextures[BBT_TRASSA].tex);
            }

            if (m_Handler)
                m_Handler(s, hitpos, m_User, FEHF_LASTHIT);

            break;
        }
        case WEAPON_HOMING_MISSILE:
        case WEAPON_CANNON3: {
            DCP();
            SMOProps mo;
            mo.startpos = m_Pos;
            mo.target = m_Pos + m_Dir * 1500;
            mo.curpos = mo.startpos;
            mo.velocity = m_Dir;
            mo.object = LoadObject(OBJECT_PATH_ROCKET, m_Heap);
            mo.handler = MO_Homing_Missile_Takt;

            mo.side = m_SideStorage;
            // mo.attacker = m_Owner;
            // if (mo.attacker) mo.attacker->RefInc();

            mo.shleif = (SEffectHandler *)HAlloc(sizeof(SEffectHandler), m_Heap);
            mo.shleif->effect = NULL;
            CMatrixEffect::CreateShleif(mo.shleif);
            m_Ref++;
            CMatrixEffect::CreateMovingObject(NULL, mo, TRACE_ALL, m_Skip, WeaponHit, (DWORD)this);

            break;
        }
        case WEAPON_BOMB: {
            DCP();
            SMOProps mo;
            mo.startpos = m_Pos;
            mo.curpos = mo.startpos;

            // CHelper::Create(100,0)->Line(m_Speed, m_Speed + D3DXVECTOR3(0,0,100));

            auto tmp = m_Pos - m_Speed;
            float len = D3DXVec3Length(&tmp);
            D3DXVECTOR3 dir((m_Pos - m_Speed) * (1.0f / len));
            if (len > m_WeaponDist * m_WeaponCoefficient)
                len = m_WeaponDist * m_WeaponCoefficient;

            mo.target = m_Pos - dir * len;
            D3DXVECTOR3 mid((m_Pos + mo.target) * 0.5f);

            // SPlane pl;
            // SPlane::BuildFromPointNormal(pl, mid, dir);
            // float ttt;
            // pl.FindIntersect(m_Pos, m_Dir, ttt);

            int pcnt = 5;
            D3DXVECTOR3 pts[5];

            pts[0] = m_Pos;

            if (m_Dir.z < 0) {
                // on flyer bombomet
                pts[1] = m_Pos + m_Dir * 70 - D3DXVECTOR3(0, 0, 25);
                pts[2] = m_Pos + m_Dir * 110 - D3DXVECTOR3(0, 0, 100);
                pts[3] = m_Pos + m_Dir * 130 - D3DXVECTOR3(0, 0, 300);

                pcnt = 4;

                mo.velocity.x = 0.0006f;
            }
            else {
                pts[1] = mid + dir * len * 0.15f + D3DXVECTOR3(0, 0, len * 0.5f);
                pts[2] = mid - dir * len * 0.15f + D3DXVECTOR3(0, 0, len * 0.5f);
                pts[3] = mo.target;
                // more beautiful trajectory
                pts[4] = mo.target - dir * len * 0.35f - D3DXVECTOR3(0, 0, len * 0.5f);

                mo.velocity.x = 0.0005f;
            }

            // CHelper::Create(1,0)->Line(pts[0], pts[1]);
            // CHelper::Create(1,0)->Line(pts[1], pts[2]);
            // CHelper::Create(1,0)->Line(pts[2], pts[3]);

            mo.common.bomb.pos = 0;
            mo.common.bomb.trajectory = HNew(g_MatrixHeap) CTrajectory(g_MatrixHeap, pts, pcnt);

            // CHelper::DestroyByGroup(123);

            // D3DXVECTOR3 pp;
            // mo.common.bomb.trajectory->CalcPoint(pp,0);
            // for (float t = 0.01f; t <= 1.0f; t += 0.01f)
            //{
            //    D3DXVECTOR3 ppp;
            //    mo.common.bomb.trajectory->CalcPoint(ppp,t);
            //    CHelper::Create(100000,123)->Line(pp,ppp);

            //    pp = ppp;
            //}

            // float speed = D3DXVec3Length(&(m_Pos-m_Speed)) / 100;
            // if (speed < 1) speed = 1;
            // if (speed > 3) speed = 3;

            // speed -= (speed - 1.2f) * 0.3f;

            // mo.velocity = m_Dir * speed * 1.5f;
            mo.object = LoadObject(OBJECT_PATH_MINA, m_Heap);
            mo.handler = MO_Bomb_Takt;

            mo.side = m_SideStorage;
            // mo.attacker = m_Owner;
            // if (mo.attacker) mo.attacker->RefInc();

            mo.shleif = (SEffectHandler *)HAlloc(sizeof(SEffectHandler), m_Heap);
            mo.shleif->effect = NULL;
            CMatrixEffect::CreateShleif(mo.shleif);

            m_Ref++;
            CMatrixEffect::CreateMovingObject(NULL, mo, TRACE_ALL, m_Skip, WeaponHit, (DWORD)this);

            break;
        }
        case WEAPON_GUN:
        case WEAPON_CANNON1: {
            DCP();
            SMOProps mo;
            mo.common.gun.maxdist = m_WeaponDist * m_WeaponCoefficient;
            mo.startpos = m_Pos;
            mo.target = m_Speed;
            mo.curpos = mo.startpos;

            mo.side = m_SideStorage;
            // mo.attacker = m_Owner;
            // if (mo.attacker) mo.attacker->RefInc();

            mo.velocity = m_Dir * 22;
            mo.object = LoadObject(OBJECT_PATH_GUN, m_Heap);
            mo.handler = MO_Gun_Takt;
            m_Ref++;
            CMatrixEffect::CreateMovingObject(NULL, mo, TRACE_ALL, m_Skip, WeaponHit, (DWORD)this);

#ifdef _DEBUG
            SEffectHandler eh(DEBUG_CALL_INFO);
#else
            SEffectHandler eh;
#endif
            CMatrixEffect::CreatePointLight(&eh, m_Pos, 60, 0xFF303030, false);
            if (eh.effect) {
                ((CMatrixEffectPointLight *)eh.effect)->Kill(1000);
                eh.Unconnect();
            }

            int f = IRND(3);
            if (f == 0)
                CMatrixEffect::CreateBillboardLine(NULL, m_Pos, m_Pos + m_Dir * 50, 30, 0xFFFFFFFF, 0, 1000,
                                                   m_BBTextures[BBT_GUNFIRE1].tex);
            else if (f == 1)
                CMatrixEffect::CreateBillboardLine(NULL, m_Pos, m_Pos + m_Dir * 50, 30, 0xFFFFFFFF, 0, 1000,
                                                   m_BBTextures[BBT_GUNFIRE2].tex);
            else
                CMatrixEffect::CreateBillboardLine(NULL, m_Pos, m_Pos + m_Dir * 50, 30, 0xFFFFFFFF, 0, 1000,
                                                   m_BBTextures[BBT_GUNFIRE3].tex);

            break;
        }
        case WEAPON_LIGHTENING: {
            DCP();
            D3DXVECTOR3 hitpos = m_Pos + m_Dir * m_WeaponDist * m_WeaponCoefficient;
            CMatrixMapStatic *s = g_MatrixMap->Trace(&hitpos, m_Pos, hitpos, TRACE_ALL, m_Skip);

            // float l = D3DXVec3Length(&(m_Pos- hitpos));
            // CDText::T("len", l);

            // CHelper::Create(10,0)->Line(m_Pos, hitpos);
            // CHelper::Create(10,0)->Line(hitpos, hitpos + D3DXVECTOR3(0,0,100));

            if (IS_TRACE_STOP_OBJECT(s)) {
                bool dead = s->Damage(m_Type, hitpos, m_Dir, GetSideStorage(), GetOwner());
                SETFLAG(m_Flags, WEAPFLAGS_HITWAS);
                if (dead) {
                    s = TRACE_STOP_NONE;
                }
                else {
                    if (!s->IsFlyer()) {
                        D3DXVECTOR3 pos1(hitpos.x + FSRND(20), hitpos.y + FSRND(20), 0);
                        pos1.z = g_MatrixMap->GetZ(pos1.x, pos1.y);
                        CMatrixEffect::CreateShorted(hitpos, pos1, FRND(500) + 400);
                    }
                }
            }
            else if (s == TRACE_STOP_LANDSCAPE) {
                CMatrixEffect::CreateLandscapeSpot(NULL, D3DXVECTOR2(hitpos.x, hitpos.y), FSRND(M_PI), FRND(1) + 0.5f,
                                                   SPOT_PLASMA_HIT);

                // e = CMatrixEffect::CreateShorted(hitpos + D3DXVECTOR3(FSRND(5),FSRND(5),0),
                //                                 hitpos + D3DXVECTOR3(FSRND(5),FSRND(5),0), int(FRND(500) + 400));
                // g_MatrixMap->AddEffect(e);
            }

            if (m_Effect.effect && m_Effect.effect->GetType() == EFFECT_LIGHTENING) {
                ((CMatrixEffectLightening *)m_Effect.effect)->SetPos(m_Pos, hitpos);
            }
            else {
                CMatrixEffect::CreateLightening(&m_Effect, m_Pos, hitpos, 1000000, 3, LIGHTENING_WIDTH);
            }

            if (m_Handler)
                m_Handler(s, hitpos, m_User, FEHF_LASTHIT);

            break;
        }
        case WEAPON_LASER:
        case WEAPON_CANNON2: {
            DCP();
            D3DXVECTOR3 hitpos = m_Pos + m_Dir * m_WeaponDist * m_WeaponCoefficient;
            CMatrixMapStatic *s = g_MatrixMap->Trace(&hitpos, m_Pos, hitpos, TRACE_ALL, m_Skip);

            if (IS_TRACE_STOP_OBJECT(s)) {
                bool dead = s->Damage(m_Type, hitpos, m_Dir, GetSideStorage(), GetOwner());
                SETFLAG(m_Flags, WEAPFLAGS_HITWAS);
                if (dead) {
                    s = TRACE_STOP_NONE;
                }

                /*
                D3DXVECTOR3 pos1(hitpos.x + FSRND(20), hitpos.y + FSRND(20), 0);
                pos1.z = g_MatrixMap->GetZ(pos1.x, pos1.y);
                e = CMatrixEffect::CreateShorted(hitpos, pos1,int(FRND(500) + 400));
                g_MatrixMap->AddEffect(e);
                */

                CMatrixEffect::CreateExplosion(hitpos, ExplosionLaserHit);

                if (m_Handler)
                    m_Handler(s, hitpos, m_User, FEHF_LASTHIT);
            }
            else if (s == TRACE_STOP_LANDSCAPE) {
                CMatrixEffect::CreateLandscapeSpot(NULL, D3DXVECTOR2(hitpos.x, hitpos.y), FSRND(M_PI), FRND(1) + 0.5f,
                                                   SPOT_PLASMA_HIT);
                CMatrixEffect::CreateExplosion(hitpos, ExplosionLaserHit);
            }
            else if (s == TRACE_STOP_WATER) {
                CMatrixEffect::CreateSmoke(NULL, hitpos, 100, 1400, 10, 0xFFFFFF);
            }

            if (m_Laser) {
                m_Laser->SetPos(m_Pos, hitpos);
            }
            else {
                m_Laser = HNew(m_Heap) CLaser(m_Pos, hitpos);
                if (!g_MatrixMap->AddEffect(m_Laser))
                    m_Laser = NULL;
            }

            if (m_Handler)
                m_Handler(s, hitpos, m_User, FEHF_LASTHIT);

            break;
        }
        case WEAPON_FLAMETHROWER: {
            DCP();

            if (m_Effect.effect == NULL) {
                float ttl = m_WeaponDist * m_WeaponCoefficient * 8.333f;
                CMatrixEffect::CreateFlame(&m_Effect, ttl, TRACE_ALL, m_Skip, (DWORD)this, WeaponHit);
                if (m_Effect.effect == NULL)
                    break;
                ++m_Ref;
            }
            ((CMatrixEffectFlame *)m_Effect.effect)->AddPuff(m_Pos, m_Dir, m_Speed);
            break;
        }
        case WEAPON_BIGBOOM: {
            DCP();

            m_Ref++;
            CMatrixEffect::CreateBigBoom(m_Pos, m_WeaponDist * m_WeaponCoefficient, 300, TRACE_ALL, NULL /*m_Skip*/,
                                         (DWORD)this, WeaponHit);
            CMatrixEffect::CreateBigBoom(m_Pos, m_WeaponDist, 350, 0, 0, 0, 0);
            CMatrixEffect::CreateBigBoom(m_Pos, m_WeaponDist, 400, 0, 0, 0, 0, 0xFFAFAF40);
            CMatrixEffect::CreateExplosion(m_Pos, ExplosionBigBoom, true);

            break;
        }
        case WEAPON_REPAIR: {
            DCP();
            if (m_Repair) {
                m_Repair->UpdateData(m_Pos, m_Dir);
            }
            else {
                m_Repair =
                        (CMatrixEffectRepair *)CreateRepair(m_Pos, m_Dir, m_WeaponDist * m_WeaponCoefficient, m_Skip);
                if (!g_MatrixMap->AddEffect(m_Laser))
                    m_Repair = NULL;
            }

            if (m_Repair) {
                CMatrixMapStatic *ms = m_Repair->GetTarget();
                if (ms) {
                    if (m_Handler)
                        m_Handler(ms, ms->GetGeoCenter(), m_User, FEHF_LASTHIT);
                    SETFLAG(m_Flags, WEAPFLAGS_HITWAS);
                    ms->Damage(m_Type, ms->GetGeoCenter(), m_Dir, GetSideStorage(), GetOwner());
                }
                else {
                    if (m_Handler)
                        m_Handler(TRACE_STOP_NONE, m_Pos, m_User, FEHF_LASTHIT);
                }
            }

            break;
        }

        case WEAPON_CANNON0: {
            DCP();
            SMOProps mo;
            mo.common.gun.maxdist = m_WeaponDist * m_WeaponCoefficient;
            mo.startpos = m_Pos;
            mo.target = m_Speed;
            mo.curpos = mo.startpos;

            mo.side = m_SideStorage;
            // mo.attacker = m_Owner;
            // if (mo.attacker) mo.attacker->RefInc();

            mo.velocity = m_Dir * 27;
            mo.object = LoadObject(OBJECT_PATH_GUN, m_Heap);
            mo.handler = MO_Gun_cannon_Takt;
            m_Ref++;
            CMatrixEffect::CreateMovingObject(NULL, mo, TRACE_ALL, m_Skip, WeaponHit, (DWORD)this);

#ifdef _DEBUG
            SEffectHandler eh(DEBUG_CALL_INFO);
#else
            SEffectHandler eh;
#endif
            CMatrixEffect::CreatePointLight(&eh, m_Pos, 40, 0xFF303030, false);
            if (eh.effect) {
                ((CMatrixEffectPointLight *)eh.effect)->Kill(1000);
                eh.Unconnect();
            }

            int f = IRND(3);
            if (f == 0)
                CMatrixEffect::CreateBillboardLine(NULL, m_Pos, m_Pos + m_Dir * 35, 25, 0xFFFFFFFF, 0, 1000,
                                                   m_BBTextures[BBT_GUNFIRE1].tex);
            else if (f == 1)
                CMatrixEffect::CreateBillboardLine(NULL, m_Pos, m_Pos + m_Dir * 35, 25, 0xFFFFFFFF, 0, 1000,
                                                   m_BBTextures[BBT_GUNFIRE2].tex);
            else
                CMatrixEffect::CreateBillboardLine(NULL, m_Pos, m_Pos + m_Dir * 35, 25, 0xFFFFFFFF, 0, 1000,
                                                   m_BBTextures[BBT_GUNFIRE3].tex);

            break;
        }

        default:
            CMatrixEffect::CreateExplosion(m_Pos, ExplosionRobotBoom, true);
    }
}

void CMatrixEffectWeapon::FireEnd(void) {
    if (!IsFire())
        return;
    RESETFLAG(m_Flags, WEAPFLAGS_FIRE);

    if (m_SoundType != S_NONE) {
        if (FLAG(m_Flags, WEAPFLAGS_SND_OFF)) {
            CSound::StopPlay(m_Sound);
            m_Sound = SOUND_ID_EMPTY;
        }
    }

    if (m_Type == WEAPON_LIGHTENING) {
#ifdef _DEBUG
        m_Effect.Release(DEBUG_CALL_INFO);
#else
        m_Effect.Release();
#endif
    }
    else if (m_Type == WEAPON_LASER || m_Type == WEAPON_CANNON2) {
        if (m_Laser) {
#ifdef _DEBUG
            g_MatrixMap->SubEffect(DEBUG_CALL_INFO, m_Laser);
#else
            g_MatrixMap->SubEffect(m_Laser);
#endif
            m_Laser = NULL;
        }
    }
    else if (m_Type == WEAPON_VOLCANO) {
        if (m_Volcano) {
#ifdef _DEBUG
            g_MatrixMap->SubEffect(DEBUG_CALL_INFO, m_Volcano);
#else
            g_MatrixMap->SubEffect(m_Volcano);
#endif
            m_Volcano = NULL;
        }
    }
    else if (m_Type == WEAPON_REPAIR) {
        if (m_Repair) {
            // CMatrixMapStatic *ms = m_Repair->GetTarget();
            // if (ms && m_Handler) m_Handler(ms, ms->GetGeoCenter(), m_User, FEHF_LASTHIT);
#ifdef _DEBUG
            g_MatrixMap->SubEffect(DEBUG_CALL_INFO, m_Repair);
#else
            g_MatrixMap->SubEffect(m_Repair);
#endif
            m_Repair = NULL;
        }
    }
    else if (m_Type == WEAPON_FLAMETHROWER) {
        if (m_Effect.effect)
            ((CMatrixEffectFlame *)m_Effect.effect)->Break();
    }
}

void CMatrixEffectWeapon::SoundHit(EWeapon w, const D3DXVECTOR3 &pos) {
    const ESound snds[17] = {
            S_WEAPON_HIT_PLASMA,       S_WEAPON_HIT_VOLCANO, S_WEAPON_HIT_HOMING_MISSILE, S_WEAPON_HIT_BOMB,
            S_WEAPON_HIT_FLAMETHROWER, S_WEAPON_HIT_BIGBOOM, S_WEAPON_HIT_LIGHTENING,     S_WEAPON_HIT_LASER,
            S_WEAPON_HIT_GUN,          S_WEAPON_HIT_ABLAZE,  S_WEAPON_HIT_SHORTED,        S_WEAPON_HIT_DEBRIS,
            S_WEAPON_HIT_REPAIR,       S_WEAPON_HIT_CANNON0, S_WEAPON_HIT_CANNON1,        S_WEAPON_HIT_CANNON2,
            S_WEAPON_HIT_CANNON3,

    };

    ESound snd = snds[Weap2Index(w)];
    // ESoundLayer l = SL_ALL;
    // if (snd == S_WEAPON_HIT_ABLAZE) l = SL_ABLAZE0;
    // if (snd == S_WEAPON_HIT_SHORTED) l = SL_SHORTED0;

    CSound::AddSound(snd, pos, SL_ALL, SEF_SKIP);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

CLaser::CLaser(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1)
  : CMatrixEffect(), m_bl(TRACE_PARAM_CALL pos0, pos1, LASER_WIDTH, 0xFFFFFFFF, m_BBTextures[BBT_LASER].tex) {
    m_EffectType = EFFECT_LASER;

    if (m_BBTextures[BBT_LASEREND].IsIntense()) {
        new(&m_end) CBillboard(TRACE_PARAM_CALL pos0, LASER_WIDTH * 0.5f, 0, 0xFFFFFFFF,
                                     m_BBTextures[BBT_LASEREND].tex);
    }
    else {
        new(&m_end) CBillboard(TRACE_PARAM_CALL pos0, LASER_WIDTH * 0.5f, 0, 0xFFFFFFFF,
                                     &m_BBTextures[BBT_LASEREND].bbt);
    }
}

void CLaser::Draw(void) {
    DTRACE();

    BYTE a = g_MatrixMap->IsPaused() ? 240 : (BYTE(FRND(128) + 128));

    m_bl.SetAlpha(a);
    m_bl.AddToDrawQueue();

    m_end.SetAlpha(a);
    m_end.Sort(g_MatrixMap->m_Camera.GetViewMatrix());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

CVolcano::CVolcano(const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir, float angle)
  : CMatrixEffect(), m_Konus(start, dir, VOLCANO_FIRE_KONUS_RADIUS, VOLCANO_FIRE_KONUS_LENGTH, angle, 300, true, NULL),
    m_bl1(TRACE_PARAM_CALL start, start + dir * VOLCANO_FIRE_LENGHT, VOLCANO_FIRE_WIDTH, 0xFFFFFFFF,
          m_BBTextures[BBT_GUNFIRE1].tex),
    m_bl2(TRACE_PARAM_CALL start, start + dir * VOLCANO_FIRE_LENGHT, VOLCANO_FIRE_WIDTH, 0xFFFFFFFF,
          m_BBTextures[BBT_GUNFIRE2].tex),
    m_bl3(TRACE_PARAM_CALL start, start + dir * VOLCANO_FIRE_LENGHT, VOLCANO_FIRE_WIDTH, 0xFFFFFFFF,
          m_BBTextures[BBT_GUNFIRE3].tex) {
    m_EffectType = EFFECT_VOLCANO;
}

void CVolcano::Draw(void) {
    m_Konus.Draw();

    int idx = g_MatrixMap->IsPaused() ? 0 : (IRND(3));

    if (idx == 0)
        m_bl1.AddToDrawQueue();
    else if (idx == 1)
        m_bl2.AddToDrawQueue();
    else if (idx == 2)
        m_bl3.AddToDrawQueue();
}

void CMatrixEffectWeapon::SetOwner(CMatrixMapStatic *ms) {
    m_Owner = ms->GetCore(DEBUG_CALL_INFO);
    m_SideStorage = ms->GetSide();
}
CMatrixMapStatic *CMatrixEffectWeapon::GetOwner(void) {
    if (m_Owner)
        return m_Owner->m_Object;
    return NULL;
}
