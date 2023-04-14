// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixEffect.hpp"
#include "MatrixEffectRepair.hpp"
#include "MatrixEffectKonus.hpp"

#include "../MatrixSoundManager.hpp"

#include "StringConstants.hpp"

// weapon
#define FLAME_PUFF_TTL 2000

#define LASER_WIDTH               10
#define VOLCANO_FIRE_LENGHT       15
#define VOLCANO_FIRE_WIDTH        10
#define VOLCANO_FIRE_KONUS_RADIUS 5
#define VOLCANO_FIRE_KONUS_LENGTH 5

#define ARCADEBOT_WEAPON_COEFF 1.2f
#define DEFBOT_WEAPON_COEFF    1.0f

//#define WD_PLASMA           300
//#define WD_VOLCANO          300
//#define WD_HOMING_MISSILE   500
//#define WD_BOMB             400
//#define WD_FLAMETHROWER     100
//#define WD_BIGBOOM          100
//#define WD_LIGHTENING       300
//#define WD_LASER            200
//#define WD_GUN              300

#define WEAPON_MAX_HEAT 1000

class CMatrixMapStatic;

enum EWeapon : unsigned int {
    WEAPON_NONE,

    WEAPON_PLASMA = 200,
    WEAPON_VOLCANO = 70,
    WEAPON_HOMING_MISSILE = 1000,
    WEAPON_BOMB = 2500,
    WEAPON_FLAMETHROWER = 60,
    WEAPON_BIGBOOM = 10000,
    WEAPON_LIGHTENING = 99,
    WEAPON_LASER = 98,
    WEAPON_GUN = 598,
    WEAPON_REPAIR = 57,

    WEAPON_CANNON0 = 300,
    WEAPON_CANNON1 = 998,
    WEAPON_CANNON2 = 97,
    WEAPON_CANNON3 = 1002,

    WEAPON_ABLAZE = 10000000,   // горим
    WEAPON_SHORTED = 10000001,  // замкнуло
    WEAPON_DEBRIS = 10000002,   // долбануло осколком

    WEAPON_COUNT = 17,
    WEAPON_INSTANT_DEATH = 0x7FFFFFFF - 1,

    EWeapon_FORCE_DWORD = 0x7FFFFFFF
};

inline int WeapName2Index(const std::wstring& w)
{
    if (w == PAR_SOURCE_DAMAGES_WEAPON_PLASMA)
        return 0;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_VOLCANO)
        return 1;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_HOMING_MISSILE)
        return 2;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_BOMB)
        return 3;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_FLAMETHROWER)
        return 4;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_BIGBOOM)
        return 5;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_LIGHTENING)
        return 6;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_LASER)
        return 7;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_GUN)
        return 8;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_ABLAZE)
        return 9;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_SHORTED)
        return 10;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_DEBRIS)
        return 11;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_REPAIR)
        return 12;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_CANNON0)
        return 13;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_CANNON1)
        return 14;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_CANNON2)
        return 15;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_CANNON3)
        return 16;
    return -1;
}

inline EWeapon WeapName2Weap(const std::wstring& w)
{
    if (w == PAR_SOURCE_DAMAGES_WEAPON_PLASMA)
        return WEAPON_PLASMA;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_VOLCANO)
        return WEAPON_VOLCANO;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_HOMING_MISSILE)
        return WEAPON_HOMING_MISSILE;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_BOMB)
        return WEAPON_BOMB;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_FLAMETHROWER)
        return WEAPON_FLAMETHROWER;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_BIGBOOM)
        return WEAPON_BIGBOOM;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_LIGHTENING)
        return WEAPON_LIGHTENING;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_LASER)
        return WEAPON_LASER;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_GUN)
        return WEAPON_GUN;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_REPAIR)
        return WEAPON_REPAIR;
    // if (w == PAR_SOURCE_DAMAGES_WEAPON_ABLAZE) return WEAPON_ABLAZE;
    // if (w == PAR_SOURCE_DAMAGES_WEAPON_SHORTED) return WEAPON_SHORTED;
    // if (w == PAR_SOURCE_DAMAGES_WEAPON_DEBRIS) return WEAPON_DEBRIS;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_CANNON0)
        return WEAPON_CANNON0;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_CANNON1)
        return WEAPON_CANNON1;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_CANNON2)
        return WEAPON_CANNON2;
    if (w == PAR_SOURCE_DAMAGES_WEAPON_CANNON3)
        return WEAPON_CANNON3;
    return WEAPON_NONE;
}

inline int Weap2Index(EWeapon w) {
    if (w == WEAPON_PLASMA)
        return 0;
    if (w == WEAPON_VOLCANO)
        return 1;
    if (w == WEAPON_HOMING_MISSILE)
        return 2;
    if (w == WEAPON_BOMB)
        return 3;
    if (w == WEAPON_FLAMETHROWER)
        return 4;
    if (w == WEAPON_BIGBOOM)
        return 5;
    if (w == WEAPON_LIGHTENING)
        return 6;
    if (w == WEAPON_LASER)
        return 7;
    if (w == WEAPON_GUN)
        return 8;
    if (w == WEAPON_ABLAZE)
        return 9;
    if (w == WEAPON_SHORTED)
        return 10;
    if (w == WEAPON_DEBRIS)
        return 11;
    if (w == WEAPON_REPAIR)
        return 12;
    if (w == WEAPON_CANNON0)
        return 13;
    if (w == WEAPON_CANNON1)
        return 14;
    if (w == WEAPON_CANNON2)
        return 15;
    if (w == WEAPON_CANNON3)
        return 16;
    return -1;
}

class CLaser : public CMatrixEffect {
    CBillboardLine m_bl;
    CBillboard m_end;
    virtual ~CLaser() {
        m_bl.Release();
        m_end.Release();
    }

public:
    CLaser(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1);

    virtual void BeforeDraw(void){};
    virtual void Draw(void);
    virtual void Takt(float){};
    virtual void Release(void) { HDelete(CLaser, this, m_Heap); };
    virtual int Priority(void) { return MAX_EFFECT_PRIORITY; };

    void SetPos(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1) {
        m_bl.SetPos(pos0, pos1);
        m_end.m_Pos = pos0;
    }
};

class CVolcano : public CMatrixEffect {
    CMatrixEffectKonus m_Konus;
    CBillboardLine m_bl1;
    CBillboardLine m_bl2;
    CBillboardLine m_bl3;

    virtual ~CVolcano() {
        m_bl1.Release();
        m_bl2.Release();
        m_bl3.Release();
    }

public:
    CVolcano(const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir, float angle);

    virtual void BeforeDraw(void) { m_Konus.BeforeDraw(); };
    virtual void Draw(void);
    virtual void Takt(float){};
    virtual void Release(void) { HDelete(CVolcano, this, m_Heap); };

    virtual int Priority(void) { return MAX_EFFECT_PRIORITY; };

    void SetPos(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, const D3DXVECTOR3 &dir) {
        m_bl1.SetPos(pos0, pos1);
        m_bl2.SetPos(pos0, pos1);
        m_bl3.SetPos(pos0, pos1);
        m_Konus.Modify(pos0, dir);
    }
    void SetPos(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, const D3DXVECTOR3 &dir, float angle) {
        m_bl1.SetPos(pos0, pos1);
        m_bl2.SetPos(pos0, pos1);
        m_bl3.SetPos(pos0, pos1);
        m_Konus.Modify(pos0, dir, VOLCANO_FIRE_KONUS_RADIUS, VOLCANO_FIRE_KONUS_LENGTH, angle);
    }
};

class CMatrixEffectWeapon : public CMatrixEffect {
    EWeapon m_Type;
    float m_WeaponDist;
    float m_WeaponCoefficient;

    DWORD m_Sound;
    ESound m_SoundType;

    DWORD m_User;
    FIRE_END_HANDLER m_Handler;
    int m_Ref;

    D3DXVECTOR3 m_Pos;
    D3DXVECTOR3 m_Dir;
    D3DXVECTOR3 m_Speed;

    float m_Time;
    float m_CoolDown;
    CMatrixMapStatic *m_Skip;

    int m_FireCount;

    SEffectHandler m_Effect;
    union {
        CLaser *m_Laser;
        CVolcano *m_Volcano;
        CMatrixEffectRepair *m_Repair;
    };

    SObjectCore *m_Owner;
    int m_SideStorage;  // side storage (if owner will be killed)

    CMatrixEffectWeapon(const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir, DWORD user, FIRE_END_HANDLER handler,
                        EWeapon type, int cooldown);
    virtual ~CMatrixEffectWeapon();
    void Fire(void);

public:
    D3DXVECTOR3 GetPos() { return m_Pos; }
    void SetDefaultCoefficient() { m_WeaponCoefficient = DEFBOT_WEAPON_COEFF; }
    void SetArcadeCoefficient() { m_WeaponCoefficient = ARCADEBOT_WEAPON_COEFF; }
    EWeapon GetWeaponType(void) const { return m_Type; }
    float GetWeaponDist(void) const { return m_WeaponDist * m_WeaponCoefficient; }
    friend class CMatrixEffect;

    static void WeaponHit(CMatrixMapStatic *hiti, const D3DXVECTOR3 &pos, DWORD user, DWORD flags);

    void SetOwner(CMatrixMapStatic *ms);
    int GetSideStorage(void) const { return m_SideStorage; };
    CMatrixMapStatic *GetOwner(void);

    virtual void BeforeDraw(void){};
    virtual void Draw(void){};
    virtual void Takt(float step);
    virtual void Release(void);

    virtual int Priority(void) { return MAX_EFFECT_PRIORITY; };

    void Modify(const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &speed);

    bool IsFire(void) { return FLAG(m_Flags, WEAPFLAGS_FIRE); }
    bool IsFireWas(void) {
        bool r = FLAG(m_Flags, WEAPFLAGS_FIREWAS);
        RESETFLAG(m_Flags, WEAPFLAGS_FIREWAS);
        return r;
    }
    bool IsHitWas(void) {
        bool r = FLAG(m_Flags, WEAPFLAGS_HITWAS);
        RESETFLAG(m_Flags, WEAPFLAGS_HITWAS);
        return r;
    }

    void ResetFireCount(void) { m_FireCount = 0; }
    int GetFireCount(void) const { return m_FireCount; }

    void ModifyCoolDown(float addk) { m_CoolDown += m_CoolDown * addk; }
    void ModifyDist(float addk) { m_WeaponDist += m_WeaponDist * addk; }

    void FireBegin(const D3DXVECTOR3 &speed, CMatrixMapStatic *skip) {
        if (IsFire())
            return;
        m_Speed = speed;
        SETFLAG(m_Flags, WEAPFLAGS_FIRE);
        RESETFLAG(m_Flags, WEAPFLAGS_FIREWAS);
        RESETFLAG(m_Flags, WEAPFLAGS_HITWAS);
        m_Skip = skip;
        // CHelper::Create(10000,0)->Line(m_Pos, m_Pos+m_Dir*100);
    }
    void FireEnd(void);

    static void SoundHit(EWeapon w, const D3DXVECTOR3 &pos);
};
