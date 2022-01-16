// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"

#include "MatrixSoundManager.hpp"
#include "MatrixGameDll.hpp"
#include "Matrixmap.hpp"

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
    Path().CWStr::CWStr(sndname, g_MatrixHeap);
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

    m_Sounds[S_BCLICK].SSoundItem::SSoundItem(L"bclick");
    m_Sounds[S_BENTER].SSoundItem::SSoundItem(L"benter");
    m_Sounds[S_BLEAVE].SSoundItem::SSoundItem(L"bleave");
    m_Sounds[S_MAP_PLUS].SSoundItem::SSoundItem(L"map_plus");
    m_Sounds[S_MAP_MINUS].SSoundItem::SSoundItem(L"map_minus");
    m_Sounds[S_PRESET_CLICK].SSoundItem::SSoundItem(L"preset_click");
    m_Sounds[S_BUILD_CLICK].SSoundItem::SSoundItem(L"build_click");
    m_Sounds[S_CANCEL_CLICK].SSoundItem::SSoundItem(L"cancel_click");

    m_Sounds[S_DOORS_OPEN].SSoundItem::SSoundItem(L"base_doors_open");
    m_Sounds[S_DOORS_CLOSE].SSoundItem::SSoundItem(L"base_doors_close");
    m_Sounds[S_DOORS_CLOSE_STOP].SSoundItem::SSoundItem(L"base_doors_close_stop");
    m_Sounds[S_PLATFORM_UP].SSoundItem::SSoundItem(L"base_platform_up");
    m_Sounds[S_PLATFORM_DOWN].SSoundItem::SSoundItem(L"base_platform_down");
    m_Sounds[S_PLATFORM_UP_STOP].SSoundItem::SSoundItem(L"base_platform_up_stop");
    //    m_Sounds[S_BASE_AMBIENT     ].SSoundItem::SSoundItem(L"base_amb");

    // m_Sounds[S_TITAN_AMBIENT    ].SSoundItem::SSoundItem(L"titan_amb");
    // m_Sounds[S_ELECTRONIC_AMBIENT].SSoundItem::SSoundItem(L"electronic_amb");
    // m_Sounds[S_ENERGY_AMBIENT].SSoundItem::SSoundItem(L"energy_amb");
    // m_Sounds[S_PLASMA_AMBIENT].SSoundItem::SSoundItem(L"plasma_amb");
    // m_Sounds[S_REPAIR_AMBIENT].SSoundItem::SSoundItem(L"repair_amb");

    m_Sounds[S_EXPLOSION_NORMAL].SSoundItem::SSoundItem(L"expl_norm");
    m_Sounds[S_EXPLOSION_MISSILE].SSoundItem::SSoundItem(L"expl_missile");
    m_Sounds[S_EXPLOSION_ROBOT_HIT].SSoundItem::SSoundItem(L"expl_rh");
    m_Sounds[S_EXPLOSION_LASER_HIT].SSoundItem::SSoundItem(L"expl_lh");
    m_Sounds[S_EXPLOSION_BUILDING_BOOM].SSoundItem::SSoundItem(L"expl_bb");
    m_Sounds[S_EXPLOSION_BUILDING_BOOM2].SSoundItem::SSoundItem(L"expl_bb2");
    m_Sounds[S_EXPLOSION_BUILDING_BOOM3].SSoundItem::SSoundItem(L"expl_bb3");
    m_Sounds[S_EXPLOSION_BUILDING_BOOM4].SSoundItem::SSoundItem(L"expl_bb4");
    m_Sounds[S_EXPLOSION_ROBOT_BOOM].SSoundItem::SSoundItem(L"expl_rb");
    m_Sounds[S_EXPLOSION_ROBOT_BOOM_SMALL].SSoundItem::SSoundItem(L"expl_rbs");
    m_Sounds[S_EXPLOSION_BIGBOOM].SSoundItem::SSoundItem(L"expl_bigboom");
    m_Sounds[S_EXPLOSION_OBJECT].SSoundItem::SSoundItem(L"expl_obj");

    m_Sounds[S_SPLASH].SSoundItem::SSoundItem(L"splash");

    m_Sounds[S_EF_START].SSoundItem::SSoundItem(L"ef_start");  // elevator fields
    m_Sounds[S_EF_CONTINUE].SSoundItem::SSoundItem(L"ef_cont");
    m_Sounds[S_EF_END].SSoundItem::SSoundItem(L"ef_end");

    m_Sounds[S_FLYER_VINT_START].SSoundItem::SSoundItem(L"fl_start");
    m_Sounds[S_FLYER_VINT_CONTINUE].SSoundItem::SSoundItem(L"fl_cont");

    m_Sounds[S_WEAPON_PLASMA].SSoundItem::SSoundItem(L"wplasma");
    m_Sounds[S_WEAPON_VOLCANO].SSoundItem::SSoundItem(L"wvolcano");
    m_Sounds[S_WEAPON_HOMING_MISSILE].SSoundItem::SSoundItem(L"wmissile");
    m_Sounds[S_WEAPON_BOMB].SSoundItem::SSoundItem(L"wbomb");
    m_Sounds[S_WEAPON_FLAMETHROWER].SSoundItem::SSoundItem(L"wflame");
    m_Sounds[S_WEAPON_BIGBOOM].SSoundItem::SSoundItem(L"wbigboom");
    m_Sounds[S_WEAPON_LIGHTENING].SSoundItem::SSoundItem(L"wlightening");
    m_Sounds[S_WEAPON_LASER].SSoundItem::SSoundItem(L"wlaser");
    m_Sounds[S_WEAPON_GUN].SSoundItem::SSoundItem(L"wgun");
    m_Sounds[S_WEAPON_REPAIR].SSoundItem::SSoundItem(L"wrepair");

    m_Sounds[S_WEAPON_CANNON0].SSoundItem::SSoundItem(L"wcannon0");
    m_Sounds[S_WEAPON_CANNON1].SSoundItem::SSoundItem(L"wcannon1");
    m_Sounds[S_WEAPON_CANNON2].SSoundItem::SSoundItem(L"wcannon2");
    m_Sounds[S_WEAPON_CANNON3].SSoundItem::SSoundItem(L"wcannon3");

    m_Sounds[S_WEAPON_HIT_PLASMA].SSoundItem::SSoundItem(L"whplasma");
    m_Sounds[S_WEAPON_HIT_VOLCANO].SSoundItem::SSoundItem(L"whvolcano");
    m_Sounds[S_WEAPON_HIT_HOMING_MISSILE].SSoundItem::SSoundItem(L"whmissile");
    m_Sounds[S_WEAPON_HIT_BOMB].SSoundItem::SSoundItem(L"whbomb");
    m_Sounds[S_WEAPON_HIT_FLAMETHROWER].SSoundItem::SSoundItem(L"whflame");
    m_Sounds[S_WEAPON_HIT_BIGBOOM].SSoundItem::SSoundItem(L"whbigboom");
    m_Sounds[S_WEAPON_HIT_LIGHTENING].SSoundItem::SSoundItem(L"whlightening");
    m_Sounds[S_WEAPON_HIT_LASER].SSoundItem::SSoundItem(L"whlaser");
    m_Sounds[S_WEAPON_HIT_GUN].SSoundItem::SSoundItem(L"whgun");
    m_Sounds[S_WEAPON_HIT_REPAIR].SSoundItem::SSoundItem(L"whrepair");
    m_Sounds[S_WEAPON_HIT_ABLAZE].SSoundItem::SSoundItem(L"whablaze");
    m_Sounds[S_WEAPON_HIT_SHORTED].SSoundItem::SSoundItem(L"whshorted");
    m_Sounds[S_WEAPON_HIT_DEBRIS].SSoundItem::SSoundItem(L"whdebris");

    m_Sounds[S_WEAPON_HIT_CANNON0].SSoundItem::SSoundItem(L"whcannon0");
    m_Sounds[S_WEAPON_HIT_CANNON1].SSoundItem::SSoundItem(L"whcannon1");
    m_Sounds[S_WEAPON_HIT_CANNON2].SSoundItem::SSoundItem(L"whcannon2");
    m_Sounds[S_WEAPON_HIT_CANNON3].SSoundItem::SSoundItem(L"whcannon3");

    m_Sounds[S_ROBOT_BUILD_END].SSoundItem::SSoundItem(L"r_build_e");
    m_Sounds[S_ROBOT_BUILD_END_ALT].SSoundItem::SSoundItem(L"r_build_ea");

    m_Sounds[S_TURRET_BUILD_START].SSoundItem::SSoundItem(L"t_build_s");
    m_Sounds[S_TURRET_BUILD_0].SSoundItem::SSoundItem(L"t_build_0");
    m_Sounds[S_TURRET_BUILD_1].SSoundItem::SSoundItem(L"t_build_1");
    m_Sounds[S_TURRET_BUILD_2].SSoundItem::SSoundItem(L"t_build_2");
    m_Sounds[S_TURRET_BUILD_3].SSoundItem::SSoundItem(L"t_build_3");

    m_Sounds[S_FLYER_BUILD_END].SSoundItem::SSoundItem(L"f_build_e");
    m_Sounds[S_FLYER_BUILD_END_ALT].SSoundItem::SSoundItem(L"f_build_ea");

    m_Sounds[S_YES_SIR_1].SSoundItem::SSoundItem(L"s_yes_1");
    m_Sounds[S_YES_SIR_2].SSoundItem::SSoundItem(L"s_yes_2");
    m_Sounds[S_YES_SIR_3].SSoundItem::SSoundItem(L"s_yes_3");
    m_Sounds[S_YES_SIR_4].SSoundItem::SSoundItem(L"s_yes_4");
    m_Sounds[S_YES_SIR_5].SSoundItem::SSoundItem(L"s_yes_5");

    m_Sounds[S_SELECTION_1].SSoundItem::SSoundItem(L"s_selection_1");
    m_Sounds[S_SELECTION_2].SSoundItem::SSoundItem(L"s_selection_2");
    m_Sounds[S_SELECTION_3].SSoundItem::SSoundItem(L"s_selection_3");
    m_Sounds[S_SELECTION_4].SSoundItem::SSoundItem(L"s_selection_4");
    m_Sounds[S_SELECTION_5].SSoundItem::SSoundItem(L"s_selection_5");
    m_Sounds[S_SELECTION_6].SSoundItem::SSoundItem(L"s_selection_6");
    m_Sounds[S_SELECTION_7].SSoundItem::SSoundItem(L"s_selection_7");

    m_Sounds[S_BUILDING_SEL].SSoundItem::SSoundItem(L"s_building_sel");
    m_Sounds[S_BASE_SEL].SSoundItem::SSoundItem(L"s_base_sel");

    m_Sounds[S_CHASSIS_PNEUMATIC_LOOP].SSoundItem::SSoundItem(L"s_chassis_pneumatic_l");
    m_Sounds[S_CHASSIS_WHEEL_LOOP].SSoundItem::SSoundItem(L"s_chassis_wheel_l");
    m_Sounds[S_CHASSIS_TRACK_LOOP].SSoundItem::SSoundItem(L"s_chassis_track_l");
    m_Sounds[S_CHASSIS_HOVERCRAFT_LOOP].SSoundItem::SSoundItem(L"s_chassis_hovercraft_l");
    m_Sounds[S_CHASSIS_ANTIGRAVITY_LOOP].SSoundItem::SSoundItem(L"s_chassis_antigravity_l");

    m_Sounds[S_HULL_PASSIVE].SSoundItem::SSoundItem(L"s_hull_passive");
    m_Sounds[S_HULL_ACTIVE].SSoundItem::SSoundItem(L"s_hull_active");
    m_Sounds[S_HULL_FIREPROOF].SSoundItem::SSoundItem(L"s_hull_fireproof");
    m_Sounds[S_HULL_PLASMIC].SSoundItem::SSoundItem(L"s_hull_plasmic");
    m_Sounds[S_HULL_NUCLEAR].SSoundItem::SSoundItem(L"s_hull_nuclear");
    m_Sounds[S_HULL_6].SSoundItem::SSoundItem(L"s_hull_6");

    m_Sounds[S_MAINTENANCE].SSoundItem::SSoundItem(L"s_maintenance");
    m_Sounds[S_MAINTENANCE_ON].SSoundItem::SSoundItem(L"s_maintenance_on");
    m_Sounds[S_RESINCOME].SSoundItem::SSoundItem(L"s_resincome");

    m_Sounds[S_SIDE_UNDER_ATTACK_1].SSoundItem::SSoundItem(L"s_side_attacked_1");
    m_Sounds[S_SIDE_UNDER_ATTACK_2].SSoundItem::SSoundItem(L"s_side_attacked_2");
    m_Sounds[S_SIDE_UNDER_ATTACK_3].SSoundItem::SSoundItem(L"s_side_attacked_3");

    m_Sounds[S_ENEMY_BASE_CAPTURED].SSoundItem::SSoundItem(L"s_eb_cap");
    m_Sounds[S_ENEMY_FACTORY_CAPTURED].SSoundItem::SSoundItem(L"s_ef_cap");
    m_Sounds[S_PLAYER_BASE_CAPTURED].SSoundItem::SSoundItem(L"s_pb_cap");
    m_Sounds[S_PLAYER_FACTORY_CAPTURED].SSoundItem::SSoundItem(L"s_pf_cap");

    m_Sounds[S_BASE_KILLED].SSoundItem::SSoundItem(L"s_base_dead");
    m_Sounds[S_FACTORY_KILLED].SSoundItem::SSoundItem(L"s_fa_dead");
    m_Sounds[S_BUILDING_KILLED].SSoundItem::SSoundItem(L"s_building_dead");

    m_Sounds[S_ORDER_INPROGRESS1].SSoundItem::SSoundItem(L"s_ord_inprogress1");
    m_Sounds[S_ORDER_INPROGRESS2].SSoundItem::SSoundItem(L"s_ord_inprogress2");

    m_Sounds[S_ORDER_ACCEPT].SSoundItem::SSoundItem(L"s_ord_accept");
    m_Sounds[S_ORDER_ATTACK].SSoundItem::SSoundItem(L"s_ord_attack");
    m_Sounds[S_ORDER_CAPTURE].SSoundItem::SSoundItem(L"s_ord_capture");
    m_Sounds[S_ORDER_CAPTURE_PUSH].SSoundItem::SSoundItem(L"s_ord_capture_push");
    m_Sounds[S_ORDER_REPAIR].SSoundItem::SSoundItem(L"s_ord_repair");

    m_Sounds[S_ORDER_AUTO_ATTACK].SSoundItem::SSoundItem(L"s_orda_attack");
    m_Sounds[S_ORDER_AUTO_CAPTURE].SSoundItem::SSoundItem(L"s_orda_capture");
    m_Sounds[S_ORDER_AUTO_DEFENCE].SSoundItem::SSoundItem(L"s_orda_defence");

    m_Sounds[S_TERRON_PAIN1].SSoundItem::SSoundItem(L"s_terron_pain1");
    m_Sounds[S_TERRON_PAIN2].SSoundItem::SSoundItem(L"s_terron_pain2");
    m_Sounds[S_TERRON_PAIN3].SSoundItem::SSoundItem(L"s_terron_pain3");
    m_Sounds[S_TERRON_PAIN4].SSoundItem::SSoundItem(L"s_terron_pain4");
    m_Sounds[S_TERRON_KILLED].SSoundItem::SSoundItem(L"s_terron_killed");

    m_Sounds[S_ORDER_CAPTURE_FUCK_OFF].SSoundItem::SSoundItem(L"s_ord_capoff");

    m_Sounds[S_ROBOT_UPAL].SSoundItem::SSoundItem(L"s_upal");

    m_Sounds[S_CANTBE].SSoundItem::SSoundItem(L"s_cantbe");

    m_Sounds[S_SPECIAL_SLOT].SSoundItem::SSoundItem(L"");

#ifdef _DEBUG
    for (int i = 0; i < S_COUNT; ++i) {
        if (FLAG(m_Sounds[i].flags, SSoundItem::NOTINITED)) {
            // ERROR_S((CWStr(L"Sound ") + i + L" not initialized!").Get());
            MessageBoxW(NULL, (CWStr(L"Sound ") + i + L" not initialized!").Get(), L"Error", MB_ICONERROR);
            _asm int 3
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
            // CDText::T("SND: ", CStr(sc));
            g_MatrixMap->m_DI.T(L"Active sounds: ", CWStr(sc));
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
        ERROR_S(L"Problem with sound: " + CWStr(name));
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
                            _asm int 3

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
                        ERROR_S(L"Problem with sound: " + CSound::m_Sounds[sb->snd].Path());
                    }
                    else {
                        ERROR_S(L"Problem with sound: " + CWStr((int)sb->snd));
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
        CStr ss(g_CacheHeap);

        ss = i;
        ss += " - id:";
        ss += int(m_AllSounds[i].id);

        ss += ", idi:";
        ss += int(m_AllSounds[i].id_internal);

        ss += ", vol:";
        ss += m_AllSounds[i].curvol;

        ss += ", pan:";
        ss += m_AllSounds[i].curpan;

        ss += ", rvol:";
        ss += g_RangersInterface->m_SoundGetVolume(m_AllSounds[i].id_internal);

        ss += ", is_play:";
        ss += g_RangersInterface->m_SoundIsPlay(m_AllSounds[i].id_internal);
        ss += "\n";

        b.StrNZ(ss);
    }

    b.SaveInFile(L"log_sounds.txt");
}
