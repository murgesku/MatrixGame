// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "d3d9.h"
#include "d3dx9tex.h"

#include <cstdint>

#define SOUND_ID_EMPTY (0xFFFFFFFF)

enum ESoundLayer {
    SL_ALL,
    SL_INTERFACE,
    SL_ELEVATORFIELD,  // its only one per game
    SL_SELECTION,
    SL_HULL,
    SL_CHASSIS,

    SL_ORDER,

    SL_ABLAZE0,
    SL_SHORTED0,

    SL_COUNT
};

// works only for non SL_ALL layer
enum ESoundInterruptFlag { SEF_INTERRUPT, SEF_SKIP };

enum ESound {

    // interface
    S_BCLICK,
    S_BENTER,
    S_BLEAVE,
    S_MAP_PLUS,
    S_MAP_MINUS,
    S_PRESET_CLICK,
    S_BUILD_CLICK,
    S_CANCEL_CLICK,

    // sounds of base
    S_DOORS_OPEN,
    S_DOORS_CLOSE,
    S_DOORS_CLOSE_STOP,
    S_PLATFORM_UP,
    S_PLATFORM_DOWN,
    S_PLATFORM_UP_STOP,
    //    S_BASE_AMBIENT,

    // other building ambients
    // S_TITAN_AMBIENT,
    // S_PLASMA_AMBIENT,
    // S_ELECTRONIC_AMBIENT,
    // S_ENERGY_AMBIENT,
    // S_REPAIR_AMBIENT,

    // effects explosions
    S_EXPLOSION_NORMAL,
    S_EXPLOSION_MISSILE,
    S_EXPLOSION_ROBOT_HIT,
    S_EXPLOSION_LASER_HIT,
    S_EXPLOSION_BUILDING_BOOM,   // small explosives
    S_EXPLOSION_BUILDING_BOOM2,  // small explosives 2
    S_EXPLOSION_BUILDING_BOOM3,  // non base buildings
    S_EXPLOSION_BUILDING_BOOM4,  // base buildings
    S_EXPLOSION_ROBOT_BOOM,
    S_EXPLOSION_ROBOT_BOOM_SMALL,
    S_EXPLOSION_BIGBOOM,
    S_EXPLOSION_OBJECT,

    S_SPLASH,

    S_EF_START,
    S_EF_CONTINUE,
    S_EF_END,

    // flyer
    S_FLYER_VINT_START,
    S_FLYER_VINT_CONTINUE,

    // chassis
    S_CHASSIS_PNEUMATIC_LOOP,
    S_CHASSIS_WHEEL_LOOP,
    S_CHASSIS_TRACK_LOOP,
    S_CHASSIS_HOVERCRAFT_LOOP,
    S_CHASSIS_ANTIGRAVITY_LOOP,

    // hulls
    S_HULL_PASSIVE,
    S_HULL_ACTIVE,
    S_HULL_FIREPROOF,
    S_HULL_PLASMIC,
    S_HULL_NUCLEAR,
    S_HULL_6,

    // weapons
    S_WEAPON_PLASMA,
    S_WEAPON_VOLCANO,
    S_WEAPON_HOMING_MISSILE,
    S_WEAPON_BOMB,
    S_WEAPON_FLAMETHROWER,
    S_WEAPON_BIGBOOM,
    S_WEAPON_LIGHTENING,
    S_WEAPON_LASER,
    S_WEAPON_GUN,
    S_WEAPON_REPAIR,

    S_WEAPON_CANNON0,
    S_WEAPON_CANNON1,
    S_WEAPON_CANNON2,
    S_WEAPON_CANNON3,

    S_WEAPON_HIT_PLASMA,
    S_WEAPON_HIT_VOLCANO,
    S_WEAPON_HIT_HOMING_MISSILE,
    S_WEAPON_HIT_BOMB,
    S_WEAPON_HIT_FLAMETHROWER,
    S_WEAPON_HIT_BIGBOOM,
    S_WEAPON_HIT_LIGHTENING,
    S_WEAPON_HIT_LASER,
    S_WEAPON_HIT_GUN,
    S_WEAPON_HIT_REPAIR,
    S_WEAPON_HIT_ABLAZE,
    S_WEAPON_HIT_SHORTED,
    S_WEAPON_HIT_DEBRIS,

    S_WEAPON_HIT_CANNON0,
    S_WEAPON_HIT_CANNON1,
    S_WEAPON_HIT_CANNON2,
    S_WEAPON_HIT_CANNON3,

    S_ROBOT_UPAL,
    //
    S_ROBOT_BUILD_END,
    S_ROBOT_BUILD_END_ALT,

    S_TURRET_BUILD_START,
    S_TURRET_BUILD_0,
    S_TURRET_BUILD_1,
    S_TURRET_BUILD_2,
    S_TURRET_BUILD_3,

    S_FLYER_BUILD_END,
    S_FLYER_BUILD_END_ALT,

    S_YES_SIR_1,
    S_YES_SIR_2,
    S_YES_SIR_3,
    S_YES_SIR_4,
    S_YES_SIR_5,

    S_SELECTION_1,
    S_SELECTION_2,
    S_SELECTION_3,
    S_SELECTION_4,
    S_SELECTION_5,
    S_SELECTION_6,
    S_SELECTION_7,

    S_BUILDING_SEL,
    S_BASE_SEL,

    S_SIDE_UNDER_ATTACK_1,
    S_SIDE_UNDER_ATTACK_2,
    S_SIDE_UNDER_ATTACK_3,

    S_ENEMY_BASE_CAPTURED,
    S_ENEMY_FACTORY_CAPTURED,

    S_PLAYER_BASE_CAPTURED,
    S_PLAYER_FACTORY_CAPTURED,

    S_BASE_KILLED,
    S_FACTORY_KILLED,
    S_BUILDING_KILLED,

    S_MAINTENANCE,
    S_MAINTENANCE_ON,

    S_RESINCOME,

    S_TERRON_PAIN1,
    S_TERRON_PAIN2,
    S_TERRON_PAIN3,
    S_TERRON_PAIN4,
    S_TERRON_KILLED,

    // orders

    S_ORDER_INPROGRESS1,
    S_ORDER_INPROGRESS2,

    S_ORDER_ACCEPT,

    S_ORDER_ATTACK,
    S_ORDER_CAPTURE,
    S_ORDER_CAPTURE_PUSH,
    S_ORDER_REPAIR,

    S_ORDER_AUTO_ATTACK,
    S_ORDER_AUTO_CAPTURE,
    S_ORDER_AUTO_DEFENCE,

    S_ORDER_CAPTURE_FUCK_OFF,

    //
    S_CANTBE,

    // special slot

    S_SPECIAL_SLOT,

    S_COUNT,
    S_UNDEF,
    S_NONE = -1
};

class CSound
{
public:
    static bool IsSoundPlay(uint32_t id);

    static void Init();
    static void Clear();
    static void LayerOff(ESoundLayer sl);
    static void SureLoaded(ESound snd);

    static uint32_t Play(ESound snd, ESoundLayer sl = SL_ALL, ESoundInterruptFlag interrupt = SEF_INTERRUPT);
    static uint32_t Play(ESound snd, float vol, float pan, ESoundLayer sl = SL_ALL, ESoundInterruptFlag interrupt = SEF_INTERRUPT);

    static uint32_t Play(const wchar_t *name, ESoundLayer sl = SL_ALL,
                      ESoundInterruptFlag interrupt = SEF_INTERRUPT);  // name - robots sound
    static uint32_t Play(const wchar_t *name, const D3DXVECTOR3 &pos, ESoundLayer sl = SL_ALL,
                      ESoundInterruptFlag interrupt = SEF_INTERRUPT);  // name - robots sound
    static uint32_t Play(const D3DXVECTOR3 &pos, float attn, float pan0, float pan1, float vol0, float vol1, const wchar_t *name);
    static uint32_t Play(ESound snd, const D3DXVECTOR3 &pos, ESoundLayer sl = SL_ALL,
                      ESoundInterruptFlag interrupt = SEF_INTERRUPT);
    static uint32_t Play(uint32_t id, ESound snd, const D3DXVECTOR3 &pos, ESoundLayer sl = SL_ALL,
                      ESoundInterruptFlag interrupt = SEF_INTERRUPT);  // use only for ambient
    static void StopPlay(uint32_t id);
    static void StopPlayAllSounds();
    static uint32_t ChangePos(uint32_t id, ESound snd, const D3DXVECTOR3 &pos);
    static float GetSoundMaxDistSQ(ESound snd);

    static void CalcPanVol(const D3DXVECTOR3 &pos, float attn, float pan0, float pan1, float vol0, float vol1,
                           float *pan, float *vol);

    static void Takt();

    static void AddSound(ESound snd, const D3DXVECTOR3 &pos, ESoundLayer sl = SL_ALL,
                         ESoundInterruptFlag ifl = SEF_INTERRUPT);  // automatic position

    static void AddSound(const wchar_t *name, const D3DXVECTOR3 &pos);  // automatic position
    static void AddSound(const D3DXVECTOR3 &pos, float attn, float pan0, float pan1, float vol0, float vol1,
                         const wchar_t *name);

    static inline uint32_t Pos2Key(const D3DXVECTOR3 &pos);

    static void SaveSoundLog();
};