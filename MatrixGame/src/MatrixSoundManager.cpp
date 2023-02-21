// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>

#include "stdafx.h"

#include "MatrixSoundManager.hpp"
#include "MatrixGameDll.hpp"
#include "MatrixMap.hpp"

#include <utils.hpp>

CSound::SSoundItem CSound::m_Sounds[S_COUNT];
CSound::SLID CSound::m_LayersI[SL_COUNT];
int CSound::m_LastGroup;
DWORD CSound::m_LastID;
// CBuf                *CSound::m_AllSounds;
CSound::SPlayedSound CSound::m_AllSounds[MAX_SOUNDS];
CDWORDMap *CSound::m_PosSounds;

inline DWORD snd_create(wchar *n, int i, int j) {
    DTRACE();
    return g_RangersInterface->m_SoundCreate(n, i, j);
}

inline void snd_destroy(DWORD s) {
    DTRACE();
    g_RangersInterface->m_SoundDestroy(s);
}

inline void snd_pan(DWORD s, float v) {
    DTRACE();
    g_RangersInterface->m_SoundPan(s, v);
}
inline void snd_vol(DWORD s, float v) {
    DTRACE();
    g_RangersInterface->m_SoundVolume(s, v);
}

inline void snd_play(DWORD s) {
    DTRACE();
    g_RangersInterface->m_SoundPlay(s);
}

CSound::SSoundItem::SSoundItem(const wchar *sndname) : vol0(1), vol1(1), pan0(0), pan1(0), flags(0) {
    new(&Path()) CWStr(sndname, g_MatrixHeap);
}

void CSound::Init(void) {
    DTRACE();

    m_LastGroup = 0;
    m_LastID = 0;
    for (int i = 0; i < SL_COUNT; ++i) {
        m_LayersI[i].index = -1;
    }
#ifdef _DEBUG
    for (int i = 0; i < S_COUNT; ++i) {
        SETFLAG(m_Sounds[i].flags, SSoundItem::NOTINITED);
    }
#endif
    for (int i = 0; i < MAX_SOUNDS; ++i) {
        m_AllSounds[i].id = SOUND_ID_EMPTY;
        m_AllSounds[i].id_internal = 0;
    }
    m_PosSounds = HNew(g_MatrixHeap) CDWORDMap(g_MatrixHeap);

    // sound initializatation

    new(&m_Sounds[S_BCLICK])       SSoundItem(L"bclick");
    new(&m_Sounds[S_BENTER])       SSoundItem(L"benter");
    new(&m_Sounds[S_BLEAVE])       SSoundItem(L"bleave");
    new(&m_Sounds[S_MAP_PLUS])     SSoundItem(L"map_plus");
    new(&m_Sounds[S_MAP_MINUS])    SSoundItem(L"map_minus");
    new(&m_Sounds[S_PRESET_CLICK]) SSoundItem(L"preset_click");
    new(&m_Sounds[S_BUILD_CLICK])  SSoundItem(L"build_click");
    new(&m_Sounds[S_CANCEL_CLICK]) SSoundItem(L"cancel_click");

    new(&m_Sounds[S_DOORS_OPEN])       SSoundItem(L"base_doors_open");
    new(&m_Sounds[S_DOORS_CLOSE])      SSoundItem(L"base_doors_close");
    new(&m_Sounds[S_DOORS_CLOSE_STOP]) SSoundItem(L"base_doors_close_stop");
    new(&m_Sounds[S_PLATFORM_UP])      SSoundItem(L"base_platform_up");
    new(&m_Sounds[S_PLATFORM_DOWN])    SSoundItem(L"base_platform_down");
    new(&m_Sounds[S_PLATFORM_UP_STOP]) SSoundItem(L"base_platform_up_stop");
    //    m_Sounds[S_BASE_AMBIENT     ].SSoundItem::SSoundItem(L"base_amb");

    // m_Sounds[S_TITAN_AMBIENT    ].SSoundItem::SSoundItem(L"titan_amb");
    // m_Sounds[S_ELECTRONIC_AMBIENT].SSoundItem::SSoundItem(L"electronic_amb");
    // m_Sounds[S_ENERGY_AMBIENT].SSoundItem::SSoundItem(L"energy_amb");
    // m_Sounds[S_PLASMA_AMBIENT].SSoundItem::SSoundItem(L"plasma_amb");
    // m_Sounds[S_REPAIR_AMBIENT].SSoundItem::SSoundItem(L"repair_amb");

    new(&m_Sounds[S_EXPLOSION_NORMAL]) SSoundItem(L"expl_norm");
    new(&m_Sounds[S_EXPLOSION_MISSILE]) SSoundItem(L"expl_missile");
    new(&m_Sounds[S_EXPLOSION_ROBOT_HIT]) SSoundItem(L"expl_rh");
    new(&m_Sounds[S_EXPLOSION_LASER_HIT]) SSoundItem(L"expl_lh");
    new(&m_Sounds[S_EXPLOSION_BUILDING_BOOM]) SSoundItem(L"expl_bb");
    new(&m_Sounds[S_EXPLOSION_BUILDING_BOOM2]) SSoundItem(L"expl_bb2");
    new(&m_Sounds[S_EXPLOSION_BUILDING_BOOM3]) SSoundItem(L"expl_bb3");
    new(&m_Sounds[S_EXPLOSION_BUILDING_BOOM4]) SSoundItem(L"expl_bb4");
    new(&m_Sounds[S_EXPLOSION_ROBOT_BOOM]) SSoundItem(L"expl_rb");
    new(&m_Sounds[S_EXPLOSION_ROBOT_BOOM_SMALL]) SSoundItem(L"expl_rbs");
    new(&m_Sounds[S_EXPLOSION_BIGBOOM]) SSoundItem(L"expl_bigboom");
    new(&m_Sounds[S_EXPLOSION_OBJECT]) SSoundItem(L"expl_obj");

    new(&m_Sounds[S_SPLASH]) SSoundItem(L"splash");

    new(&m_Sounds[S_EF_START]) SSoundItem(L"ef_start");  // elevator fields
    new(&m_Sounds[S_EF_CONTINUE]) SSoundItem(L"ef_cont");
    new(&m_Sounds[S_EF_END]) SSoundItem(L"ef_end");

    new(&m_Sounds[S_FLYER_VINT_START]) SSoundItem(L"fl_start");
    new(&m_Sounds[S_FLYER_VINT_CONTINUE]) SSoundItem(L"fl_cont");

    new(&m_Sounds[S_WEAPON_PLASMA]) SSoundItem(L"wplasma");
    new(&m_Sounds[S_WEAPON_VOLCANO]) SSoundItem(L"wvolcano");
    new(&m_Sounds[S_WEAPON_HOMING_MISSILE]) SSoundItem(L"wmissile");
    new(&m_Sounds[S_WEAPON_BOMB]) SSoundItem(L"wbomb");
    new(&m_Sounds[S_WEAPON_FLAMETHROWER]) SSoundItem(L"wflame");
    new(&m_Sounds[S_WEAPON_BIGBOOM]) SSoundItem(L"wbigboom");
    new(&m_Sounds[S_WEAPON_LIGHTENING]) SSoundItem(L"wlightening");
    new(&m_Sounds[S_WEAPON_LASER]) SSoundItem(L"wlaser");
    new(&m_Sounds[S_WEAPON_GUN]) SSoundItem(L"wgun");
    new(&m_Sounds[S_WEAPON_REPAIR]) SSoundItem(L"wrepair");

    new(&m_Sounds[S_WEAPON_CANNON0]) SSoundItem(L"wcannon0");
    new(&m_Sounds[S_WEAPON_CANNON1]) SSoundItem(L"wcannon1");
    new(&m_Sounds[S_WEAPON_CANNON2]) SSoundItem(L"wcannon2");
    new(&m_Sounds[S_WEAPON_CANNON3]) SSoundItem(L"wcannon3");

    new(&m_Sounds[S_WEAPON_HIT_PLASMA]) SSoundItem(L"whplasma");
    new(&m_Sounds[S_WEAPON_HIT_VOLCANO]) SSoundItem(L"whvolcano");
    new(&m_Sounds[S_WEAPON_HIT_HOMING_MISSILE]) SSoundItem(L"whmissile");
    new(&m_Sounds[S_WEAPON_HIT_BOMB]) SSoundItem(L"whbomb");
    new(&m_Sounds[S_WEAPON_HIT_FLAMETHROWER]) SSoundItem(L"whflame");
    new(&m_Sounds[S_WEAPON_HIT_BIGBOOM]) SSoundItem(L"whbigboom");
    new(&m_Sounds[S_WEAPON_HIT_LIGHTENING]) SSoundItem(L"whlightening");
    new(&m_Sounds[S_WEAPON_HIT_LASER]) SSoundItem(L"whlaser");
    new(&m_Sounds[S_WEAPON_HIT_GUN]) SSoundItem(L"whgun");
    new(&m_Sounds[S_WEAPON_HIT_REPAIR]) SSoundItem(L"whrepair");
    new(&m_Sounds[S_WEAPON_HIT_ABLAZE]) SSoundItem(L"whablaze");
    new(&m_Sounds[S_WEAPON_HIT_SHORTED]) SSoundItem(L"whshorted");
    new(&m_Sounds[S_WEAPON_HIT_DEBRIS]) SSoundItem(L"whdebris");

    new(&m_Sounds[S_WEAPON_HIT_CANNON0]) SSoundItem(L"whcannon0");
    new(&m_Sounds[S_WEAPON_HIT_CANNON1]) SSoundItem(L"whcannon1");
    new(&m_Sounds[S_WEAPON_HIT_CANNON2]) SSoundItem(L"whcannon2");
    new(&m_Sounds[S_WEAPON_HIT_CANNON3]) SSoundItem(L"whcannon3");

    new(&m_Sounds[S_ROBOT_BUILD_END]) SSoundItem(L"r_build_e");
    new(&m_Sounds[S_ROBOT_BUILD_END_ALT]) SSoundItem(L"r_build_ea");

    new(&m_Sounds[S_TURRET_BUILD_START]) SSoundItem(L"t_build_s");
    new(&m_Sounds[S_TURRET_BUILD_0]) SSoundItem(L"t_build_0");
    new(&m_Sounds[S_TURRET_BUILD_1]) SSoundItem(L"t_build_1");
    new(&m_Sounds[S_TURRET_BUILD_2]) SSoundItem(L"t_build_2");
    new(&m_Sounds[S_TURRET_BUILD_3]) SSoundItem(L"t_build_3");

    new(&m_Sounds[S_FLYER_BUILD_END]) SSoundItem(L"f_build_e");
    new(&m_Sounds[S_FLYER_BUILD_END_ALT]) SSoundItem(L"f_build_ea");

    new(&m_Sounds[S_YES_SIR_1]) SSoundItem(L"s_yes_1");
    new(&m_Sounds[S_YES_SIR_2]) SSoundItem(L"s_yes_2");
    new(&m_Sounds[S_YES_SIR_3]) SSoundItem(L"s_yes_3");
    new(&m_Sounds[S_YES_SIR_4]) SSoundItem(L"s_yes_4");
    new(&m_Sounds[S_YES_SIR_5]) SSoundItem(L"s_yes_5");

    new(&m_Sounds[S_SELECTION_1]) SSoundItem(L"s_selection_1");
    new(&m_Sounds[S_SELECTION_2]) SSoundItem(L"s_selection_2");
    new(&m_Sounds[S_SELECTION_3]) SSoundItem(L"s_selection_3");
    new(&m_Sounds[S_SELECTION_4]) SSoundItem(L"s_selection_4");
    new(&m_Sounds[S_SELECTION_5]) SSoundItem(L"s_selection_5");
    new(&m_Sounds[S_SELECTION_6]) SSoundItem(L"s_selection_6");
    new(&m_Sounds[S_SELECTION_7]) SSoundItem(L"s_selection_7");

    new(&m_Sounds[S_BUILDING_SEL]) SSoundItem(L"s_building_sel");
    new(&m_Sounds[S_BASE_SEL]) SSoundItem(L"s_base_sel");

    new(&m_Sounds[S_CHASSIS_PNEUMATIC_LOOP]) SSoundItem(L"s_chassis_pneumatic_l");
    new(&m_Sounds[S_CHASSIS_WHEEL_LOOP]) SSoundItem(L"s_chassis_wheel_l");
    new(&m_Sounds[S_CHASSIS_TRACK_LOOP]) SSoundItem(L"s_chassis_track_l");
    new(&m_Sounds[S_CHASSIS_HOVERCRAFT_LOOP]) SSoundItem(L"s_chassis_hovercraft_l");
    new(&m_Sounds[S_CHASSIS_ANTIGRAVITY_LOOP]) SSoundItem(L"s_chassis_antigravity_l");

    new(&m_Sounds[S_HULL_PASSIVE]) SSoundItem(L"s_hull_passive");
    new(&m_Sounds[S_HULL_ACTIVE]) SSoundItem(L"s_hull_active");
    new(&m_Sounds[S_HULL_FIREPROOF]) SSoundItem(L"s_hull_fireproof");
    new(&m_Sounds[S_HULL_PLASMIC]) SSoundItem(L"s_hull_plasmic");
    new(&m_Sounds[S_HULL_NUCLEAR]) SSoundItem(L"s_hull_nuclear");
    new(&m_Sounds[S_HULL_6]) SSoundItem(L"s_hull_6");

    new(&m_Sounds[S_MAINTENANCE]) SSoundItem(L"s_maintenance");
    new(&m_Sounds[S_MAINTENANCE_ON]) SSoundItem(L"s_maintenance_on");
    new(&m_Sounds[S_RESINCOME]) SSoundItem(L"s_resincome");

    new(&m_Sounds[S_SIDE_UNDER_ATTACK_1]) SSoundItem(L"s_side_attacked_1");
    new(&m_Sounds[S_SIDE_UNDER_ATTACK_2]) SSoundItem(L"s_side_attacked_2");
    new(&m_Sounds[S_SIDE_UNDER_ATTACK_3]) SSoundItem(L"s_side_attacked_3");

    new(&m_Sounds[S_ENEMY_BASE_CAPTURED]) SSoundItem(L"s_eb_cap");
    new(&m_Sounds[S_ENEMY_FACTORY_CAPTURED]) SSoundItem(L"s_ef_cap");
    new(&m_Sounds[S_PLAYER_BASE_CAPTURED]) SSoundItem(L"s_pb_cap");
    new(&m_Sounds[S_PLAYER_FACTORY_CAPTURED]) SSoundItem(L"s_pf_cap");

    new(&m_Sounds[S_BASE_KILLED]) SSoundItem(L"s_base_dead");
    new(&m_Sounds[S_FACTORY_KILLED]) SSoundItem(L"s_fa_dead");
    new(&m_Sounds[S_BUILDING_KILLED]) SSoundItem(L"s_building_dead");

    new(&m_Sounds[S_ORDER_INPROGRESS1]) SSoundItem(L"s_ord_inprogress1");
    new(&m_Sounds[S_ORDER_INPROGRESS2]) SSoundItem(L"s_ord_inprogress2");

    new(&m_Sounds[S_ORDER_ACCEPT]) SSoundItem(L"s_ord_accept");
    new(&m_Sounds[S_ORDER_ATTACK]) SSoundItem(L"s_ord_attack");
    new(&m_Sounds[S_ORDER_CAPTURE]) SSoundItem(L"s_ord_capture");
    new(&m_Sounds[S_ORDER_CAPTURE_PUSH]) SSoundItem(L"s_ord_capture_push");
    new(&m_Sounds[S_ORDER_REPAIR]) SSoundItem(L"s_ord_repair");

    new(&m_Sounds[S_ORDER_AUTO_ATTACK]) SSoundItem(L"s_orda_attack");
    new(&m_Sounds[S_ORDER_AUTO_CAPTURE]) SSoundItem(L"s_orda_capture");
    new(&m_Sounds[S_ORDER_AUTO_DEFENCE]) SSoundItem(L"s_orda_defence");

    new(&m_Sounds[S_TERRON_PAIN1]) SSoundItem(L"s_terron_pain1");
    new(&m_Sounds[S_TERRON_PAIN2]) SSoundItem(L"s_terron_pain2");
    new(&m_Sounds[S_TERRON_PAIN3]) SSoundItem(L"s_terron_pain3");
    new(&m_Sounds[S_TERRON_PAIN4]) SSoundItem(L"s_terron_pain4");
    new(&m_Sounds[S_TERRON_KILLED]) SSoundItem(L"s_terron_killed");

    new(&m_Sounds[S_ORDER_CAPTURE_FUCK_OFF]) SSoundItem(L"s_ord_capoff");

    new(&m_Sounds[S_ROBOT_UPAL]) SSoundItem(L"s_upal");

    new(&m_Sounds[S_CANTBE]) SSoundItem(L"s_cantbe");

    new(&m_Sounds[S_SPECIAL_SLOT]) SSoundItem(L"");

#ifdef _DEBUG
    for (int i = 0; i < S_COUNT; ++i) {
        if (FLAG(m_Sounds[i].flags, SSoundItem::NOTINITED)) {
            // ERROR_S((CWStr(L"Sound ") + i + L" not initialized!").Get());
            MessageBoxW(NULL, (CWStr(L"Sound ") + i + L" not initialized!").Get(), L"Error", MB_ICONERROR);
            debugbreak();
        }
    }
#endif
}

struct SDS {
    DWORD key;
    CSoundArray *sa;
    float ms;
};
static bool update_positions(DWORD key, DWORD val, DWORD user) {
    DTRACE();

    SDS *kk = (SDS *)user;
    CSoundArray *sa = (CSoundArray *)val;
    if (sa->Len() == 0) {
        kk->key = key;
        kk->sa = sa;
        return true;
    }

    int x = (key & 0x1F) | ((key & 0x1FC00) >> 5);  // 00000000000000111111110000011111 bits
    if ((key & 0x020000) != 0)
        x = -x;

    int y = ((key & 0x3E0) >> 5) | ((key & 0x1FC0000) >> (8 + 5));  // 00000011111111000000001111100000 bits
    if ((key & 0x02000000) != 0)
        y = -y;

    DWORD z = (key >> 26);  // only positive
    D3DXVECTOR3 pos(float(x * SOUND_POS_DIVIDER), float(y * SOUND_POS_DIVIDER), float(z * SOUND_POS_DIVIDER));
    sa->UpdateTimings(kk->ms);
    sa->SetSoundPos(pos);
    return true;
}

void CSound::Takt(void) {
    DTRACE();

    static int nextsoundtakt_1;
    int delta = nextsoundtakt_1 - g_MatrixMap->GetTime();
    if (delta < 0 || delta > 100) {
        nextsoundtakt_1 = g_MatrixMap->GetTime() + 100;

        if (delta < 0)
            delta = 100;
        if (delta > 1000)
            delta = 1000;

        SDS sds;
        sds.ms = float(delta);
        sds.sa = NULL;
        m_PosSounds->Enum(update_positions, (DWORD)&sds);
        if (sds.sa != NULL) {
            HDelete(CSoundArray, sds.sa, g_MatrixHeap);
            m_PosSounds->Del(sds.key);
        }
    }

    static int nextsoundtakt;
    delta = nextsoundtakt - g_MatrixMap->GetTime();
    if (delta < 0 || delta > 1000) {
        nextsoundtakt = 1000 + g_MatrixMap->GetTime();

        if (g_RangersInterface) {
            // for (int i=1;i<SL_COUNT; ++i)
            //{
            //    if (m_Layers[i] != SOUND_ID_EMPTY)
            //    {
            //        if (!g_RangersInterface->m_SoundIsPlay(m_Layers[i]))
            //        {
            //            snd_destroy(m_Layers[i]);
            //            m_Layers[i] = SOUND_ID_EMPTY;
            //        }
            //    }
            //}

            for (int i = 0; i < MAX_SOUNDS; ++i) {
                if (m_AllSounds[i].id_internal != 0) {
                    if (!g_RangersInterface->m_SoundIsPlay(m_AllSounds[i].id_internal)) {
                        snd_destroy(m_AllSounds[i].id_internal);
                        m_AllSounds[i].id_internal = 0;
                        m_AllSounds[i].id = SOUND_ID_EMPTY;
                    }
                }
            }
        }

        if (FLAG(g_Config.m_DIFlags, DI_ACTIVESOUNDS)) {
            int sc = 0;
            for (int i = 0; i < MAX_SOUNDS; ++i) {
                if (m_AllSounds[i].id != SOUND_ID_EMPTY) {
                    ++sc;
                }
            }
            // CDText::T("SND: ", sc);
            g_MatrixMap->m_DI.T(L"Active sounds: ", CWStr(sc).c_str());
        }
    }
}

void CSound::LayerOff(ESoundLayer sl) {
    DTRACE();

    ASSERT(g_RangersInterface);
    if (m_LayersI[sl].index >= 0 && m_LayersI[sl].index < MAX_SOUNDS) {
        if (m_LayersI[sl].id == m_AllSounds[m_LayersI[sl].index].id) {
            StopPlayInternal(m_LayersI[sl].index);
        }
        m_AllSounds[m_LayersI[sl].index].id = SOUND_ID_EMPTY;
        m_AllSounds[m_LayersI[sl].index].id_internal = 0;
    }
}

void CSound::SureLoaded(ESound snd) {
    DTRACE();

    if (g_RangersInterface) {
        if (!FLAG(m_Sounds[snd].flags, SSoundItem::LOADED)) {
            // load sound
            CBlockPar *bps = g_MatrixData->BlockGet(L"Sounds");

            CWStr temp(m_Sounds[snd].Path().Get(), g_CacheHeap);
            CBlockPar *bp = bps->BlockGetNE(temp);
            if (bp == NULL) {
                bp = bps->BlockGetNE(L"dummy");
            }

            m_Sounds[snd].ttl = 1E30f;
            m_Sounds[snd].fadetime = 1000;
            m_Sounds[snd].pan0 = 0;
            m_Sounds[snd].pan1 = 0;
            m_Sounds[snd].vol0 = 1;
            m_Sounds[snd].vol1 = 1;
            if (bp->ParCount(L"pan")) {
                m_Sounds[snd].pan0 = (float)bp->ParGet(L"pan").GetDoublePar(0, L",");
                m_Sounds[snd].pan1 = (float)bp->ParGet(L"pan").GetDoublePar(1, L",");
            }
            if (bp->ParCount(L"vol")) {
                m_Sounds[snd].vol0 = (float)bp->ParGet(L"vol").GetDoublePar(0, L",");
                m_Sounds[snd].vol1 = (float)bp->ParGet(L"vol").GetDoublePar(1, L",");
            }
            if (bp->ParCount(L"looped")) {
                bool looped = 0 != bp->ParGet(L"looped").GetInt();
                INITFLAG(m_Sounds[snd].flags, SSoundItem::LOOPED, looped);
            }
            else {
                RESETFLAG(m_Sounds[snd].flags, SSoundItem::LOOPED);
            }
            if (bp->ParCount(L"ttl")) {
                m_Sounds[snd].ttl = (float)bp->ParGet(L"ttl").GetDoublePar(0, L",");
                m_Sounds[snd].fadetime = (float)bp->ParGet(L"ttl").GetDoublePar(1, L",");
            }
            if (bp->ParCount(L"attn")) {
                m_Sounds[snd].attn = (float)(0.002 * bp->ParGet(L"attn").GetDouble());
                if (m_Sounds[snd].attn == 0) {
                    m_Sounds[snd].radius = 1E15f;
                }
                else {
                    m_Sounds[snd].radius = 1.0f / m_Sounds[snd].attn;
                }
            }
            else {
                m_Sounds[snd].attn = 0.002f;
                m_Sounds[snd].radius = 1.0f / 0.002f;
            }

            m_Sounds[snd].Path() = bp->ParGet(L"path");
            SETFLAG(m_Sounds[snd].flags, SSoundItem::LOADED);
        }
    }
}

// void CSound::ExtraRemove(void)
//{
//    float minv = 100;
//    int deli = -1;
//    for(int i=0; i<MAX_SOUNDS; ++i)
//    {
//        if (!g_RangersInterface->m_SoundIsPlay(m_AllSounds[i].id_internal))
//        {
//            snd_destroy(m_AllSounds[i].id_internal);
//            m_AllSounds[i].id_internal = 0;
//            m_AllSounds[i].id = SOUND_ID_EMPTY;
//            return;
//        } else
//        {
//            if (m_AllSounds[i].curvol < minv)
//            {
//                minv = m_AllSounds[i].curvol;
//                deli = i;
//            }
//        }
//    }
//
//    StopPlayInternal(deli);
//
//}

void CSound::StopPlayInternal(int deli) {
    DTRACE();
    // snd_vol(m_AllSounds[deli].id_internal,0);
    snd_destroy(m_AllSounds[deli].id_internal);
    m_AllSounds[deli].id_internal = 0;
    m_AllSounds[deli].id = SOUND_ID_EMPTY;
}

int CSound::FindSoundSlot(DWORD id) {
    DTRACE();
    for (int i = 0; i < MAX_SOUNDS; ++i) {
        if (m_AllSounds[i].id == id) {
            return i;
        }
    }
    return -1;
}

int CSound::FindSoundSlotPlayedOnly(DWORD id) {
    DTRACE();
    int i = FindSoundSlot(id);
    if (i >= 0) {
        if (g_RangersInterface->m_SoundIsPlay(m_AllSounds[i].id_internal))
            return i;
        snd_destroy(m_AllSounds[i].id_internal);

        m_AllSounds[i].id_internal = 0;
        m_AllSounds[i].id = SOUND_ID_EMPTY;
    }
    return -1;
}

int CSound::FindSlotForSound(void) {
    DTRACE();
    float minv = 100;
    int deli = -1;
    for (int i = 0; i < MAX_SOUNDS; ++i) {
        if (m_AllSounds[i].id_internal == 0) {
            return i;
        }
        if (m_AllSounds[i].id == SOUND_ID_EMPTY) {
            StopPlayInternal(i);
            return i;
        }
        if (!g_RangersInterface->m_SoundIsPlay(m_AllSounds[i].id_internal)) {
            snd_destroy(m_AllSounds[i].id_internal);
            m_AllSounds[i].id_internal = 0;
            m_AllSounds[i].id = SOUND_ID_EMPTY;
            return i;
        }
        else {
            if (m_AllSounds[i].curvol < minv) {
                minv = m_AllSounds[i].curvol;
                deli = i;
            }
        }
    }
    StopPlayInternal(deli);
    return deli;
}

DWORD CSound::Play(const wchar *name, const D3DXVECTOR3 &pos, ESoundLayer sl, ESoundInterruptFlag interrupt) {
    DTRACE();

    if (!g_RangersInterface)
        return SOUND_ID_EMPTY;

    RESETFLAG(m_Sounds[S_SPECIAL_SLOT].flags, SSoundItem::LOADED);
    m_Sounds[S_SPECIAL_SLOT].Path().Set(name);

    return Play(S_SPECIAL_SLOT, pos, sl, interrupt);
}

DWORD CSound::Play(const wchar *name, ESoundLayer sl, ESoundInterruptFlag interrupt) {
    DTRACE();

    if (!g_RangersInterface)
        return SOUND_ID_EMPTY;

    RESETFLAG(m_Sounds[S_SPECIAL_SLOT].flags, SSoundItem::LOADED);
    m_Sounds[S_SPECIAL_SLOT].Path().Set(name);

    return Play(S_SPECIAL_SLOT, sl, interrupt);
}

DWORD CSound::Play(const D3DXVECTOR3 &pos, float attn, float pan0, float pan1, float vol0, float vol1, wchar *name) {
    DTRACE();

    if (!g_RangersInterface)
        return SOUND_ID_EMPTY;

    int si = FindSlotForSound();

    float pan, vol;
    CalcPanVol(pos, attn, pan0, pan1, vol0, vol1, &pan, &vol);

    if (vol < 0.00001f)
        return SOUND_ID_EMPTY;

    m_AllSounds[si].id_internal = snd_create(name, m_LastGroup++, 0);
    m_AllSounds[si].id = m_LastID++;

    snd_pan(m_AllSounds[si].id_internal, pan);
    snd_vol(m_AllSounds[si].id_internal, vol);

#if defined _TRACE || defined _DEBUG
    try {
#endif

        snd_play(m_AllSounds[si].id_internal);

#if defined _TRACE || defined _DEBUG
    }
    catch (...) {
        ERROR_S2(L"Problem with sound: ", CWStr(name).c_str());
    }
#endif

    m_AllSounds[si].curpan = pan;
    m_AllSounds[si].curvol = vol;

    return m_AllSounds[si].id;
}

bool CSound::IsSoundPlay(DWORD id) {
    DTRACE();

    if (g_RangersInterface) {
        for (int i = 0; i < MAX_SOUNDS; ++i) {
            if (m_AllSounds[i].id == id) {
                if (g_RangersInterface->m_SoundIsPlay(m_AllSounds[i].id_internal))
                    return true;
                // snd_vol(m_AllSounds[i].id_internal, 0);
                snd_destroy(m_AllSounds[i].id_internal);
                m_AllSounds[i].id = SOUND_ID_EMPTY;
                m_AllSounds[i].id_internal = 0;
                return false;
            }
        }
    }
    return false;
}

bool CSound::SLID::IsPlayed(void) {
    DTRACE();

    if (index >= 0 && index < MAX_SOUNDS) {
        if (CSound::m_AllSounds[index].id == id) {
            return g_RangersInterface->m_SoundIsPlay(CSound::m_AllSounds[index].id_internal) != 0;
        }
    }
    return false;
}

DWORD CSound::PlayInternal(ESound snd, float vol, float pan, ESoundLayer sl, ESoundInterruptFlag interrupt) {
    DTRACE();

    if (g_RangersInterface) {
        if (vol < 0.00001f)
            return SOUND_ID_EMPTY;

        //#if defined _TRACE || defined _DEBUG
        //    try
        //    {
        //#endif

        int si;

        DWORD newid = m_LastID++;

        if (sl != SL_ALL) {
            if (interrupt == SEF_SKIP && m_LayersI[sl].IsPlayed()) {
                return SOUND_ID_EMPTY;
            }
            LayerOff(sl);
            m_LayersI[sl].id = newid;
        }

        si = FindSlotForSound();
        m_AllSounds[si].id_internal =
                snd_create(m_Sounds[snd].Path().GetBuf(), m_LastGroup++, FLAG(m_Sounds[snd].flags, SSoundItem::LOOPED));
        m_AllSounds[si].id = newid;

        m_AllSounds[si].curpan = pan;
        m_AllSounds[si].curvol = vol;

        snd_pan(m_AllSounds[si].id_internal, pan);
        snd_vol(m_AllSounds[si].id_internal, vol);

        snd_play(m_AllSounds[si].id_internal);

        return newid;

        //#if defined _TRACE || defined _DEBUG
        //    } catch (...)
        //    {
        //        ERROR_S(L"Problem with sound: " + m_Sounds[snd].Path());
        //    }
        //#endif
    }
    else {
        return SOUND_ID_EMPTY;
    }
}

DWORD CSound::Play(ESound snd, ESoundLayer sl, ESoundInterruptFlag interrupt) {
    DTRACE();

    if (g_RangersInterface) {
        SureLoaded(snd);
        return PlayInternal(snd, (float)RND(m_Sounds[snd].vol0, m_Sounds[snd].vol1),
                            (float)RND(m_Sounds[snd].pan0, m_Sounds[snd].pan1), sl, interrupt);
    }
    else {
        return SOUND_ID_EMPTY;
    }
}

DWORD CSound::Play(ESound snd, const D3DXVECTOR3 &pos, ESoundLayer sl, ESoundInterruptFlag interrupt) {
    DTRACE();

    if (g_RangersInterface) {
        SureLoaded(snd);

        float pan, vol;
        CalcPanVol(pos, m_Sounds[snd].attn, m_Sounds[snd].pan0, m_Sounds[snd].pan1, m_Sounds[snd].vol0,
                   m_Sounds[snd].vol1, &pan, &vol);

        return PlayInternal(snd, vol, pan, sl, interrupt);
    }
    else {
        return SOUND_ID_EMPTY;
    }
}

void CSound::CalcPanVol(const D3DXVECTOR3 &pos, float attn, float pan0, float pan1, float vol0, float vol1, float *pan,
                        float *vol) {
    DTRACE();

    D3DXVECTOR3 dir(pos - g_MatrixMap->m_Camera.GetFrustumCenter());
    float dist = D3DXVec3Length(&dir);
    if (dist != 0.0f)
        dir *= (1.0f / dist);
    dist -= SOUND_FULL_VOLUME_DIST;
    if (dist < 0)
        dist = 0;  // close enough to be at full volume
    dist *= attn;  // different attenuation levels

    if (pan) {
        float dot = D3DXVec3Dot(&g_MatrixMap->m_Camera.GetRight(), &dir);
        *pan = LERPFLOAT((dot + 1) / 2.0f, pan0, pan1);
    }
    if (vol) {
        float k = (1.0f - dist);
        if (k < 0)
            k = 0;
        *vol = LERPFLOAT(k, vol0, vol1);
    }
}

DWORD CSound::Play(DWORD id, ESound snd, const D3DXVECTOR3 &pos, ESoundLayer sl, ESoundInterruptFlag interrupt) {
    DTRACE();

    if (g_RangersInterface) {
        SureLoaded(snd);

        float pan, vol;
        CalcPanVol(pos, m_Sounds[snd].attn, m_Sounds[snd].pan0, m_Sounds[snd].pan1, m_Sounds[snd].vol0,
                   m_Sounds[snd].vol1, &pan, &vol);

        int idx = FindSoundSlotPlayedOnly(id);
        if (idx >= 0) {
            // already played
            snd_pan(m_AllSounds[idx].id_internal, pan);
            snd_vol(m_AllSounds[idx].id_internal, vol);

            m_AllSounds[idx].curpan = pan;
            m_AllSounds[idx].curvol = vol;
        }
        else {
            id = PlayInternal(snd, vol, pan, sl, interrupt);
        }
        return id;
    }
    else {
        return SOUND_ID_EMPTY;
    }
}

DWORD CSound::ChangePos(DWORD id, ESound snd, const D3DXVECTOR3 &pos) {
    DTRACE();

    if (g_RangersInterface) {
        SureLoaded(snd);

        float pan, vol;
        CalcPanVol(pos, m_Sounds[snd].attn, m_Sounds[snd].pan0, m_Sounds[snd].pan1, m_Sounds[snd].vol0,
                   m_Sounds[snd].vol1, &pan, &vol);

        int idx = FindSoundSlotPlayedOnly(id);
        if (idx >= 0) {
            // already played
            snd_pan(m_AllSounds[idx].id_internal, pan);
            snd_vol(m_AllSounds[idx].id_internal, vol);

            m_AllSounds[idx].curpan = pan;
            m_AllSounds[idx].curvol = vol;
        }
        else {
            id = SOUND_ID_EMPTY;
        }
        return id;
    }
    else {
        return SOUND_ID_EMPTY;
    }
}

void CSound::StopPlayAllSounds(void) {
    DTRACE();

    if (g_RangersInterface) {
        for (int i = 0; i < MAX_SOUNDS; ++i) {
            StopPlayInternal(i);
        }
    }
}

void CSound::StopPlay(DWORD id) {
    DTRACE();

    if (id == SOUND_ID_EMPTY)
        return;
    if (g_RangersInterface) {
        // g_MatrixMap->m_DI.T(CWStr(L"sndoff") + (int)id, L"");

        int idx = FindSoundSlotPlayedOnly(id);
        if (idx >= 0) {
            StopPlayInternal(idx);
        }
    }
}

inline DWORD CSound::Pos2Key(const D3DXVECTOR3 &pos) {
    DTRACE();

    int x = Float2Int(pos.x / SOUND_POS_DIVIDER);
    int y = Float2Int(pos.y / SOUND_POS_DIVIDER);
    int z = Float2Int(pos.z / SOUND_POS_DIVIDER);

    DWORD key = 0;

    if (x < 0) {
        x = -x;
        key |= 0x020000;
    }
    if (x > 4095)
        x = 4095;

    if (y < 0) {
        y = -y;
        key |= 0x02000000;
    }
    if (y > 4095)
        y = 4095;

    if (z < 0)
        z = 0;
    else if (z > 63)
        z = 63;

    key |= (x & 31) | ((x & 4064) << 5);
    key |= ((y & 31) << 5) | ((y & 4064) << (8 + 5));
    key |= ((DWORD)z) << 26;

    return key;
}

void CSound::AddSound(ESound snd, const D3DXVECTOR3 &pos, ESoundLayer sl,
                      ESoundInterruptFlag ifl)  // automatic position
{
    DTRACE();

    if (!g_RangersInterface)
        return;
    DWORD key = Pos2Key(pos);

    CSoundArray *sa;
    if (!m_PosSounds->Get(key, (DWORD *)&sa)) {
        sa = HNew(g_MatrixHeap) CSoundArray(g_MatrixHeap);
        m_PosSounds->Set(key, (DWORD)sa);
    }
    sa->AddSound(snd, pos, sl, ifl);
}

void CSound::AddSound(const wchar *name, const D3DXVECTOR3 &pos) {
    DTRACE();
    if (!g_RangersInterface)
        return;

    RESETFLAG(m_Sounds[S_SPECIAL_SLOT].flags, SSoundItem::LOADED);
    m_Sounds[S_SPECIAL_SLOT].Path().Set(name);
    SureLoaded(S_SPECIAL_SLOT);

    AddSound(pos, m_Sounds[S_SPECIAL_SLOT].attn, m_Sounds[S_SPECIAL_SLOT].pan0, m_Sounds[S_SPECIAL_SLOT].pan1,
             m_Sounds[S_SPECIAL_SLOT].vol0, m_Sounds[S_SPECIAL_SLOT].vol1, m_Sounds[S_SPECIAL_SLOT].Path().GetBuf());
}

void CSound::AddSound(const D3DXVECTOR3 &pos, float attn, float pan0, float pan1, float vol0, float vol1, wchar *name) {
    DTRACE();

    if (!g_RangersInterface)
        return;
    DWORD key = Pos2Key(pos);

    CSoundArray *sa;
    if (!m_PosSounds->Get(key, (DWORD *)&sa)) {
        sa = HNew(g_MatrixHeap) CSoundArray(g_MatrixHeap);
        m_PosSounds->Set(key, (DWORD)sa);
    }
    sa->AddSound(pos, attn, pan0, pan1, vol0, vol1, name);
}

static bool delete_arrays(DWORD key, DWORD val, DWORD user) {
    DTRACE();

    CSoundArray *sa = (CSoundArray *)val;
    HDelete(CSoundArray, sa, g_MatrixHeap);
    return true;
}

void CSound::Clear(void) {
    DTRACE();

    if (g_RangersInterface) {
        for (int i = 0; i < MAX_SOUNDS; ++i) {
            if (m_AllSounds[i].id_internal != 0) {
                // snd_vol(m_AllSounds[i].id_internal, 0);
                snd_destroy(m_AllSounds[i].id_internal);
            }
        }
    }
    m_PosSounds->Enum(delete_arrays, 0);

    for (int i = 0; i < S_COUNT; ++i) {
        m_Sounds[i].Release();
    }

    HDelete(CDWORDMap, m_PosSounds, g_MatrixHeap);
}

void CSoundArray::UpdateTimings(float ms) {
    DTRACE();

    if (g_RangersInterface) {
        SSndData *sb = Buff<SSndData>();
        SSndData *se = BuffEnd<SSndData>();
        for (; sb < se;) {
            if (sb->snd == S_UNDEF) {
                ++sb;
                continue;
            }
            int idx = CSound::FindSoundSlotPlayedOnly(sb->id);
            if (idx >= 0) {
                if (sb->ttl < 0) {
                    if (sb->fade < 0) {
                        if (sb->id == 1)
                            debugbreak();

                        CSound::StopPlayInternal(idx);
                        goto del;
                    }

                    sb->fade -= ms;
                }
                else {
                    sb->ttl -= ms;
                    if (sb->ttl < 0) {
                        sb->ttl = -sb->fade;
                    }
                }
            }
            else {
            del:
                *sb = *(--se);
                SetLenNoShrink(Len() - sizeof(SSndData));
                continue;
            }

            ++sb;
        }
    }
    else {
        SetLenNoShrink(0);
    }
}

void CSoundArray::SetSoundPos(const D3DXVECTOR3 &pos) {
    DTRACE();

    if (g_RangersInterface) {
        DCP();
        SSndData *sb = Buff<SSndData>();
        SSndData *se = BuffEnd<SSndData>();
        for (; sb < se;) {
            DCP();
            int idx = CSound::FindSoundSlotPlayedOnly(sb->id);
            if (idx >= 0) {
                DCP();
                float k = 1.0f;
                if (sb->ttl < 0 && sb->snd != S_UNDEF) {
                    k = -sb->fade / sb->ttl;
                }

                DCP();
                float pan, vol;
                CSound::CalcPanVol(pos, sb->attn, sb->pan0, sb->pan1, sb->vol0, sb->vol1, &pan, &vol);

                DCP();
                vol *= k;
                DCP();
                if (vol < 0.00001f) {
                    DCP();
                    CSound::StopPlayInternal(idx);
                    DCP();
                    goto dele;
                }
#if defined _TRACE || defined _DEBUG
                try {
#endif
                    DCP();

                    snd_pan(CSound::m_AllSounds[idx].id_internal, pan);
                    DCP();
                    snd_vol(CSound::m_AllSounds[idx].id_internal, vol);
                    DCP();

#if defined _TRACE || defined _DEBUG
                }
                catch (...) {
                    if (sb->snd < S_COUNT && (int)sb->snd >= 0) {
                        ERROR_S2(L"Problem with sound: ", CSound::m_Sounds[sb->snd].Path().c_str());
                    }
                    else {
                        ERROR_S2(L"Problem with sound: ", CWStr((int)sb->snd).c_str());
                    }
                }
#endif
                DCP();

                CSound::m_AllSounds[idx].curpan = pan;
                CSound::m_AllSounds[idx].curvol = vol;

                // g_MatrixMap->m_DI.T(CWStr(vol).Get(), L"1212");
            }
            else {
            dele:;
                DCP();

                *sb = *(--se);
                SetLenNoShrink(Len() - sizeof(SSndData));
                continue;
            }

            ++sb;
        }
    }
    else {
        DCP();

        SetLenNoShrink(0);
    }
    DCP();
}

void CSoundArray::AddSound(ESound snd, const D3DXVECTOR3 &pos, ESoundLayer sl, ESoundInterruptFlag ifl) {
    DTRACE();

    SSndData *sb = Buff<SSndData>();
    SSndData *se = BuffEnd<SSndData>();
    for (; sb < se; ++sb) {
        if (sb->snd == snd) {
            if (ifl == SEF_INTERRUPT) {
                CSound::StopPlay(sb->id);
                (*sb) = *(--se);
                SetLenNoShrink(Len() - sizeof(SSndData));
                break;
            }
            if (!CSound::IsSoundPlay(sb->id)) {
                (*sb) = *(--se);
                SetLenNoShrink(Len() - sizeof(SSndData));
                break;
            }

            // oops. the same sound
            // only set ttl and fade
            sb->ttl = CSound::m_Sounds[snd].ttl;
            sb->fade = CSound::m_Sounds[snd].fadetime;
            return;
        }
    }

    DWORD id = CSound::Play(snd, pos, sl, ifl);
    if (id == SOUND_ID_EMPTY)
        return;

    //#ifdef _DEBUG
    //    if (snd == S_WEAPON_HIT_ABLAZE)
    //    {
    //        DCNT("Ablaze!");
    //
    //    }
    //
    //#endif

    Expand(sizeof(SSndData));

    (BuffEnd<SSndData>() - 1)->snd = snd;
    (BuffEnd<SSndData>() - 1)->id = id;
    (BuffEnd<SSndData>() - 1)->pan0 = CSound::m_Sounds[snd].pan0;
    (BuffEnd<SSndData>() - 1)->pan1 = CSound::m_Sounds[snd].pan1;
    (BuffEnd<SSndData>() - 1)->vol0 = CSound::m_Sounds[snd].vol0;
    (BuffEnd<SSndData>() - 1)->vol1 = CSound::m_Sounds[snd].vol1;
    (BuffEnd<SSndData>() - 1)->attn = CSound::m_Sounds[snd].attn;
    (BuffEnd<SSndData>() - 1)->ttl = CSound::m_Sounds[snd].ttl;
    (BuffEnd<SSndData>() - 1)->fade = CSound::m_Sounds[snd].fadetime;
    return;
}

void CSound::SaveSoundLog(void) {
    DTRACE();

    CBuf b(g_CacheHeap);
    b.StrNZ("Sounds:\n");

    for (int i = 0; i < MAX_SOUNDS; ++i) {
        auto ss =
            utils::format(
                "%d - id:%d, idi:%d, vol:%.8f, pan:%.8f, rvol:%.8f, is_play:%d\n",
                i,
                int(m_AllSounds[i].id),
                int(m_AllSounds[i].id_internal),
                m_AllSounds[i].curvol,
                m_AllSounds[i].curpan,
                g_RangersInterface->m_SoundGetVolume(m_AllSounds[i].id_internal),
                g_RangersInterface->m_SoundIsPlay(m_AllSounds[i].id_internal));

        b.StrNZ(ss);
    }

    b.SaveInFile(L"log_sounds.txt");
}
