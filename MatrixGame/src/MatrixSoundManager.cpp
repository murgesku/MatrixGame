// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixSoundManager.hpp"
#include "MatrixGameDll.hpp"
#include "MatrixMap.hpp"

#include <utils.hpp>
#include <new>
#include <map>
#include <array>
#include <vector>
#include <fstream>

#define MAX_SOUNDS 16  // 16 mixed sound for SL_ALL

#define SOUND_FULL_VOLUME_DIST 200

#define SOUND_POS_DIVIDER (GLOBAL_SCALE + GLOBAL_SCALE)

namespace {

struct SSoundItem
{
    static const uint32_t LOOPED = SETBIT(0);
    static const uint32_t LOADED = SETBIT(1);
    static const uint32_t NOTINITED = SETBIT(2);

    float vol0{1};
    float vol1{1};
    float pan0{0};
    float pan1{0};
    uint32_t flags{NOTINITED};
    float attn;
    float radius;
    float ttl;       // valid only for looped pos sounds
    float fadetime;  // valid only for looped pos sounds
    std::wstring path;

    SSoundItem() = default;
    ~SSoundItem() = default;

    SSoundItem(const std::wstring& sndname)
    : flags{0}
    , path{sndname}
    {}
};

struct SPlayedSound
{
    uint32_t id_internal;  // used in Rangers engine
    uint32_t id;           // in robots. always uniq! there is no the same id's per game
    float curvol;
    float curpan;
};

struct SLID
{
    int index;
    uint32_t id;

    bool IsPlayed(void);
};

class CSoundArray
{
    struct SSndData
    {
        ESound snd;
        uint32_t id;
        float pan0, pan1;
        float vol0, vol1;
        float attn;
        float ttl, fade;
    };

    std::vector<SSndData> m_array;

public:
    CSoundArray() = default;

    void AddSound(ESound snd, const D3DXVECTOR3 &pos, ESoundLayer sl = SL_ALL, ESoundInterruptFlag ifl = SEF_INTERRUPT);
    void AddSound(const D3DXVECTOR3 &pos, float attn, float pan0, float pan1, float vol0, float vol1, const wchar_t *name)
    {
        uint32_t id = CSound::Play(pos, attn, pan0, pan1, vol0, vol1, name);
        if (id == SOUND_ID_EMPTY)
            return;

        m_array.emplace_back();
        auto& item = m_array.back();
        item.id = id;
        item.pan0 = pan0;
        item.pan1 = pan1;
        item.vol0 = vol0;
        item.vol1 = vol1;
        item.attn = attn;
        item.snd = S_UNDEF;
    }
    void SetSoundPos(const D3DXVECTOR3 &pos);
    void UpdateTimings(float ms);

    int size() const
    {
        return m_array.size() * sizeof(SSndData);
    }

    bool empty() const
    {
        return m_array.empty();
    }
};

std::array<SSoundItem, S_COUNT> m_Sounds;
SPlayedSound m_AllSounds[MAX_SOUNDS];
std::map<uint32_t, CSoundArray> m_PosSounds;
SLID m_LayersI[SL_COUNT];  // indices in m_AllSounds array
int m_LastGroup;
uint32_t m_LastID;

} // namespace

uint32_t PlayInternal(ESound snd, float vol, float pan, ESoundLayer sl, ESoundInterruptFlag interrupt);
void StopPlayInternal(int deli);
int FindSlotForSound(void);
int FindSoundSlot(uint32_t id);
int FindSoundSlotPlayedOnly(uint32_t id);
// void ExtraRemove(void);  // extra remove sound from SL_ALL layer.

///////////////////////////////////////////////////////////////////////////////
inline uint32_t snd_create(wchar_t *n, int i, int j) {
    DTRACE();
    return g_RangersInterface->m_SoundCreate(n, i, j);
}

inline void snd_destroy(uint32_t s) {
    DTRACE();
    g_RangersInterface->m_SoundDestroy(s);
}

inline void snd_pan(uint32_t s, float v) {
    DTRACE();
    g_RangersInterface->m_SoundPan(s, v);
}
inline void snd_vol(uint32_t s, float v) {
    DTRACE();
    g_RangersInterface->m_SoundVolume(s, v);
}

inline void snd_play(uint32_t s) {
    DTRACE();
    g_RangersInterface->m_SoundPlay(s);
}
///////////////////////////////////////////////////////////////////////////////

void CSound::Init(void) {
    DTRACE();

    m_LastGroup = 0;
    m_LastID = 0;
    for (int i = 0; i < SL_COUNT; ++i)
    {
        m_LayersI[i].index = -1;
    }

    for (int i = 0; i < MAX_SOUNDS; ++i)
    {
        m_AllSounds[i].id = SOUND_ID_EMPTY;
        m_AllSounds[i].id_internal = 0;
    }

    // sound initializatation
    m_Sounds[S_BCLICK] =       SSoundItem(L"bclick");
    m_Sounds[S_BENTER] =       SSoundItem(L"benter");
    m_Sounds[S_BLEAVE] =       SSoundItem(L"bleave");
    m_Sounds[S_MAP_PLUS] =     SSoundItem(L"map_plus");
    m_Sounds[S_MAP_MINUS] =    SSoundItem(L"map_minus");
    m_Sounds[S_PRESET_CLICK] = SSoundItem(L"preset_click");
    m_Sounds[S_BUILD_CLICK] =  SSoundItem(L"build_click");
    m_Sounds[S_CANCEL_CLICK] = SSoundItem(L"cancel_click");

    m_Sounds[S_DOORS_OPEN] =       SSoundItem(L"base_doors_open");
    m_Sounds[S_DOORS_CLOSE] =      SSoundItem(L"base_doors_close");
    m_Sounds[S_DOORS_CLOSE_STOP] = SSoundItem(L"base_doors_close_stop");
    m_Sounds[S_PLATFORM_UP] =      SSoundItem(L"base_platform_up");
    m_Sounds[S_PLATFORM_DOWN] =    SSoundItem(L"base_platform_down");
    m_Sounds[S_PLATFORM_UP_STOP] = SSoundItem(L"base_platform_up_stop");

    // m_Sounds[S_BASE_AMBIENT] = SSoundItem(L"base_amb");
    // m_Sounds[S_TITAN_AMBIENT] = SSoundItem(L"titan_amb");
    // m_Sounds[S_ELECTRONIC_AMBIENT] = SSoundItem(L"electronic_amb");
    // m_Sounds[S_ENERGY_AMBIENT] = SSoundItem(L"energy_amb");
    // m_Sounds[S_PLASMA_AMBIENT] = SSoundItem(L"plasma_amb");
    // m_Sounds[S_REPAIR_AMBIENT] = SSoundItem(L"repair_amb");

    m_Sounds[S_EXPLOSION_NORMAL] = SSoundItem(L"expl_norm");
    m_Sounds[S_EXPLOSION_MISSILE] = SSoundItem(L"expl_missile");
    m_Sounds[S_EXPLOSION_ROBOT_HIT] = SSoundItem(L"expl_rh");
    m_Sounds[S_EXPLOSION_LASER_HIT] = SSoundItem(L"expl_lh");
    m_Sounds[S_EXPLOSION_BUILDING_BOOM] = SSoundItem(L"expl_bb");
    m_Sounds[S_EXPLOSION_BUILDING_BOOM2] = SSoundItem(L"expl_bb2");
    m_Sounds[S_EXPLOSION_BUILDING_BOOM3] = SSoundItem(L"expl_bb3");
    m_Sounds[S_EXPLOSION_BUILDING_BOOM4] = SSoundItem(L"expl_bb4");
    m_Sounds[S_EXPLOSION_ROBOT_BOOM] = SSoundItem(L"expl_rb");
    m_Sounds[S_EXPLOSION_ROBOT_BOOM_SMALL] = SSoundItem(L"expl_rbs");
    m_Sounds[S_EXPLOSION_BIGBOOM] = SSoundItem(L"expl_bigboom");
    m_Sounds[S_EXPLOSION_OBJECT] = SSoundItem(L"expl_obj");

    m_Sounds[S_SPLASH] = SSoundItem(L"splash");

    m_Sounds[S_EF_START] = SSoundItem(L"ef_start");  // elevator fields
    m_Sounds[S_EF_CONTINUE] = SSoundItem(L"ef_cont");
    m_Sounds[S_EF_END] = SSoundItem(L"ef_end");

    m_Sounds[S_FLYER_VINT_START] = SSoundItem(L"fl_start");
    m_Sounds[S_FLYER_VINT_CONTINUE] = SSoundItem(L"fl_cont");

    m_Sounds[S_WEAPON_PLASMA] = SSoundItem(L"wplasma");
    m_Sounds[S_WEAPON_VOLCANO] = SSoundItem(L"wvolcano");
    m_Sounds[S_WEAPON_HOMING_MISSILE] = SSoundItem(L"wmissile");
    m_Sounds[S_WEAPON_BOMB] = SSoundItem(L"wbomb");
    m_Sounds[S_WEAPON_FLAMETHROWER] = SSoundItem(L"wflame");
    m_Sounds[S_WEAPON_BIGBOOM] = SSoundItem(L"wbigboom");
    m_Sounds[S_WEAPON_LIGHTENING] = SSoundItem(L"wlightening");
    m_Sounds[S_WEAPON_LASER] = SSoundItem(L"wlaser");
    m_Sounds[S_WEAPON_GUN] = SSoundItem(L"wgun");
    m_Sounds[S_WEAPON_REPAIR] = SSoundItem(L"wrepair");

    m_Sounds[S_WEAPON_CANNON0] = SSoundItem(L"wcannon0");
    m_Sounds[S_WEAPON_CANNON1] = SSoundItem(L"wcannon1");
    m_Sounds[S_WEAPON_CANNON2] = SSoundItem(L"wcannon2");
    m_Sounds[S_WEAPON_CANNON3] = SSoundItem(L"wcannon3");

    m_Sounds[S_WEAPON_HIT_PLASMA] = SSoundItem(L"whplasma");
    m_Sounds[S_WEAPON_HIT_VOLCANO] = SSoundItem(L"whvolcano");
    m_Sounds[S_WEAPON_HIT_HOMING_MISSILE] = SSoundItem(L"whmissile");
    m_Sounds[S_WEAPON_HIT_BOMB] = SSoundItem(L"whbomb");
    m_Sounds[S_WEAPON_HIT_FLAMETHROWER] = SSoundItem(L"whflame");
    m_Sounds[S_WEAPON_HIT_BIGBOOM] = SSoundItem(L"whbigboom");
    m_Sounds[S_WEAPON_HIT_LIGHTENING] = SSoundItem(L"whlightening");
    m_Sounds[S_WEAPON_HIT_LASER] = SSoundItem(L"whlaser");
    m_Sounds[S_WEAPON_HIT_GUN] = SSoundItem(L"whgun");
    m_Sounds[S_WEAPON_HIT_REPAIR] = SSoundItem(L"whrepair");
    m_Sounds[S_WEAPON_HIT_ABLAZE] = SSoundItem(L"whablaze");
    m_Sounds[S_WEAPON_HIT_SHORTED] = SSoundItem(L"whshorted");
    m_Sounds[S_WEAPON_HIT_DEBRIS] = SSoundItem(L"whdebris");

    m_Sounds[S_WEAPON_HIT_CANNON0] = SSoundItem(L"whcannon0");
    m_Sounds[S_WEAPON_HIT_CANNON1] = SSoundItem(L"whcannon1");
    m_Sounds[S_WEAPON_HIT_CANNON2] = SSoundItem(L"whcannon2");
    m_Sounds[S_WEAPON_HIT_CANNON3] = SSoundItem(L"whcannon3");

    m_Sounds[S_ROBOT_BUILD_END] = SSoundItem(L"r_build_e");
    m_Sounds[S_ROBOT_BUILD_END_ALT] = SSoundItem(L"r_build_ea");

    m_Sounds[S_TURRET_BUILD_START] = SSoundItem(L"t_build_s");
    m_Sounds[S_TURRET_BUILD_0] = SSoundItem(L"t_build_0");
    m_Sounds[S_TURRET_BUILD_1] = SSoundItem(L"t_build_1");
    m_Sounds[S_TURRET_BUILD_2] = SSoundItem(L"t_build_2");
    m_Sounds[S_TURRET_BUILD_3] = SSoundItem(L"t_build_3");

    m_Sounds[S_FLYER_BUILD_END] = SSoundItem(L"f_build_e");
    m_Sounds[S_FLYER_BUILD_END_ALT] = SSoundItem(L"f_build_ea");

    m_Sounds[S_YES_SIR_1] = SSoundItem(L"s_yes_1");
    m_Sounds[S_YES_SIR_2] = SSoundItem(L"s_yes_2");
    m_Sounds[S_YES_SIR_3] = SSoundItem(L"s_yes_3");
    m_Sounds[S_YES_SIR_4] = SSoundItem(L"s_yes_4");
    m_Sounds[S_YES_SIR_5] = SSoundItem(L"s_yes_5");

    m_Sounds[S_SELECTION_1] = SSoundItem(L"s_selection_1");
    m_Sounds[S_SELECTION_2] = SSoundItem(L"s_selection_2");
    m_Sounds[S_SELECTION_3] = SSoundItem(L"s_selection_3");
    m_Sounds[S_SELECTION_4] = SSoundItem(L"s_selection_4");
    m_Sounds[S_SELECTION_5] = SSoundItem(L"s_selection_5");
    m_Sounds[S_SELECTION_6] = SSoundItem(L"s_selection_6");
    m_Sounds[S_SELECTION_7] = SSoundItem(L"s_selection_7");

    m_Sounds[S_BUILDING_SEL] = SSoundItem(L"s_building_sel");
    m_Sounds[S_BASE_SEL] = SSoundItem(L"s_base_sel");

    m_Sounds[S_CHASSIS_PNEUMATIC_LOOP] = SSoundItem(L"s_chassis_pneumatic_l");
    m_Sounds[S_CHASSIS_WHEEL_LOOP] = SSoundItem(L"s_chassis_wheel_l");
    m_Sounds[S_CHASSIS_TRACK_LOOP] = SSoundItem(L"s_chassis_track_l");
    m_Sounds[S_CHASSIS_HOVERCRAFT_LOOP] = SSoundItem(L"s_chassis_hovercraft_l");
    m_Sounds[S_CHASSIS_ANTIGRAVITY_LOOP] = SSoundItem(L"s_chassis_antigravity_l");

    m_Sounds[S_HULL_PASSIVE] = SSoundItem(L"s_hull_passive");
    m_Sounds[S_HULL_ACTIVE] = SSoundItem(L"s_hull_active");
    m_Sounds[S_HULL_FIREPROOF] = SSoundItem(L"s_hull_fireproof");
    m_Sounds[S_HULL_PLASMIC] = SSoundItem(L"s_hull_plasmic");
    m_Sounds[S_HULL_NUCLEAR] = SSoundItem(L"s_hull_nuclear");
    m_Sounds[S_HULL_6] = SSoundItem(L"s_hull_6");

    m_Sounds[S_MAINTENANCE] = SSoundItem(L"s_maintenance");
    m_Sounds[S_MAINTENANCE_ON] = SSoundItem(L"s_maintenance_on");
    m_Sounds[S_RESINCOME] = SSoundItem(L"s_resincome");

    m_Sounds[S_SIDE_UNDER_ATTACK_1] = SSoundItem(L"s_side_attacked_1");
    m_Sounds[S_SIDE_UNDER_ATTACK_2] = SSoundItem(L"s_side_attacked_2");
    m_Sounds[S_SIDE_UNDER_ATTACK_3] = SSoundItem(L"s_side_attacked_3");

    m_Sounds[S_ENEMY_BASE_CAPTURED] = SSoundItem(L"s_eb_cap");
    m_Sounds[S_ENEMY_FACTORY_CAPTURED] = SSoundItem(L"s_ef_cap");
    m_Sounds[S_PLAYER_BASE_CAPTURED] = SSoundItem(L"s_pb_cap");
    m_Sounds[S_PLAYER_FACTORY_CAPTURED] = SSoundItem(L"s_pf_cap");

    m_Sounds[S_BASE_KILLED] = SSoundItem(L"s_base_dead");
    m_Sounds[S_FACTORY_KILLED] = SSoundItem(L"s_fa_dead");
    m_Sounds[S_BUILDING_KILLED] = SSoundItem(L"s_building_dead");

    m_Sounds[S_ORDER_INPROGRESS1] = SSoundItem(L"s_ord_inprogress1");
    m_Sounds[S_ORDER_INPROGRESS2] = SSoundItem(L"s_ord_inprogress2");

    m_Sounds[S_ORDER_ACCEPT] = SSoundItem(L"s_ord_accept");
    m_Sounds[S_ORDER_ATTACK] = SSoundItem(L"s_ord_attack");
    m_Sounds[S_ORDER_CAPTURE] = SSoundItem(L"s_ord_capture");
    m_Sounds[S_ORDER_CAPTURE_PUSH] = SSoundItem(L"s_ord_capture_push");
    m_Sounds[S_ORDER_REPAIR] = SSoundItem(L"s_ord_repair");

    m_Sounds[S_ORDER_AUTO_ATTACK] = SSoundItem(L"s_orda_attack");
    m_Sounds[S_ORDER_AUTO_CAPTURE] = SSoundItem(L"s_orda_capture");
    m_Sounds[S_ORDER_AUTO_DEFENCE] = SSoundItem(L"s_orda_defence");

    m_Sounds[S_TERRON_PAIN1] = SSoundItem(L"s_terron_pain1");
    m_Sounds[S_TERRON_PAIN2] = SSoundItem(L"s_terron_pain2");
    m_Sounds[S_TERRON_PAIN3] = SSoundItem(L"s_terron_pain3");
    m_Sounds[S_TERRON_PAIN4] = SSoundItem(L"s_terron_pain4");
    m_Sounds[S_TERRON_KILLED] = SSoundItem(L"s_terron_killed");

    m_Sounds[S_ORDER_CAPTURE_FUCK_OFF] = SSoundItem(L"s_ord_capoff");

    m_Sounds[S_ROBOT_UPAL] = SSoundItem(L"s_upal");

    m_Sounds[S_CANTBE] = SSoundItem(L"s_cantbe");

    m_Sounds[S_SPECIAL_SLOT] = SSoundItem(L"");

    for (int i = 0; i < S_COUNT; ++i)
    {
        if (FLAG(m_Sounds[i].flags, SSoundItem::NOTINITED))
        {
            throw std::runtime_error(utils::format("Sound %d not initialized!", i));
        }
    }
}

struct SDS {
    uint32_t key;
    CSoundArray *sa;
    float ms;
};
static bool update_positions(uint32_t key, CSoundArray& sa, SDS& kk)
{
    DTRACE();

    if (sa.empty())
    {
        kk.key = key;
        kk.sa = &sa;
        return true;
    }

    int x = (key & 0x1F) | ((key & 0x1FC00) >> 5);  // 00000000000000111111110000011111 bits
    if ((key & 0x020000) != 0)
        x = -x;

    int y = ((key & 0x3E0) >> 5) | ((key & 0x1FC0000) >> (8 + 5));  // 00000011111111000000001111100000 bits
    if ((key & 0x02000000) != 0)
        y = -y;

    uint32_t z = (key >> 26);  // only positive
    D3DXVECTOR3 pos(float(x * SOUND_POS_DIVIDER), float(y * SOUND_POS_DIVIDER), float(z * SOUND_POS_DIVIDER));
    sa.UpdateTimings(kk.ms);
    sa.SetSoundPos(pos);
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

        for (auto& [key, val] : m_PosSounds)
        {
            update_positions(key, val, sds);
        }

        if (sds.sa != NULL)
        {
            m_PosSounds.erase(sds.key);
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
            g_MatrixMap->m_DI.T(L"Active sounds: ", utils::format(L"%d", sc).c_str());
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

            CBlockPar *bp = bps->BlockGetNE(m_Sounds[snd].path);
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
                m_Sounds[snd].pan0 = (float)bp->ParGet(L"pan").GetStrPar(0, L",").GetDouble();
                m_Sounds[snd].pan1 = (float)bp->ParGet(L"pan").GetStrPar(1, L",").GetDouble();
            }
            if (bp->ParCount(L"vol")) {
                m_Sounds[snd].vol0 = (float)bp->ParGet(L"vol").GetStrPar(0, L",").GetDouble();
                m_Sounds[snd].vol1 = (float)bp->ParGet(L"vol").GetStrPar(1, L",").GetDouble();
            }
            if (bp->ParCount(L"looped")) {
                bool looped = 0 != bp->ParGet(L"looped").GetInt();
                INITFLAG(m_Sounds[snd].flags, SSoundItem::LOOPED, looped);
            }
            else {
                RESETFLAG(m_Sounds[snd].flags, SSoundItem::LOOPED);
            }
            if (bp->ParCount(L"ttl")) {
                m_Sounds[snd].ttl = (float)bp->ParGet(L"ttl").GetStrPar(0, L",").GetDouble();
                m_Sounds[snd].fadetime = (float)bp->ParGet(L"ttl").GetStrPar(1, L",").GetDouble();
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

            m_Sounds[snd].path= bp->ParGet(L"path");
            SETFLAG(m_Sounds[snd].flags, SSoundItem::LOADED);
        }
    }
}

// void ExtraRemove(void)
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

void StopPlayInternal(int deli) {
    DTRACE();
    // snd_vol(m_AllSounds[deli].id_internal,0);
    snd_destroy(m_AllSounds[deli].id_internal);
    m_AllSounds[deli].id_internal = 0;
    m_AllSounds[deli].id = SOUND_ID_EMPTY;
}

int FindSoundSlot(uint32_t id) {
    DTRACE();
    for (int i = 0; i < MAX_SOUNDS; ++i) {
        if (m_AllSounds[i].id == id) {
            return i;
        }
    }
    return -1;
}

int FindSoundSlotPlayedOnly(uint32_t id) {
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

int FindSlotForSound(void) {
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

uint32_t CSound::Play(const wchar_t *name, const D3DXVECTOR3 &pos, ESoundLayer sl, ESoundInterruptFlag interrupt) {
    DTRACE();

    if (!g_RangersInterface)
        return SOUND_ID_EMPTY;

    RESETFLAG(m_Sounds[S_SPECIAL_SLOT].flags, SSoundItem::LOADED);
    m_Sounds[S_SPECIAL_SLOT].path = name;

    return Play(S_SPECIAL_SLOT, pos, sl, interrupt);
}

uint32_t CSound::Play(ESound snd, float vol, float pan, ESoundLayer sl, ESoundInterruptFlag interrupt)
{
    SureLoaded(snd);
    return PlayInternal(snd, vol, pan, sl, interrupt);
}

uint32_t CSound::Play(const wchar_t *name, ESoundLayer sl, ESoundInterruptFlag interrupt) {
    DTRACE();

    if (!g_RangersInterface)
        return SOUND_ID_EMPTY;

    RESETFLAG(m_Sounds[S_SPECIAL_SLOT].flags, SSoundItem::LOADED);
    m_Sounds[S_SPECIAL_SLOT].path = name;

    return Play(S_SPECIAL_SLOT, sl, interrupt);
}

uint32_t CSound::Play(const D3DXVECTOR3 &pos, float attn, float pan0, float pan1, float vol0, float vol1, const wchar_t *name) {
    DTRACE();

    if (!g_RangersInterface)
        return SOUND_ID_EMPTY;

    int si = FindSlotForSound();

    float pan, vol;
    CalcPanVol(pos, attn, pan0, pan1, vol0, vol1, &pan, &vol);

    if (vol < 0.00001f)
        return SOUND_ID_EMPTY;

    // TODO: non-const pointer is required here for no reason
    wchar_t* raw_name = const_cast<wchar_t*>(name);
    m_AllSounds[si].id_internal = snd_create(raw_name, m_LastGroup++, 0);
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
        ERROR_S2(L"Problem with sound: ", std::wstring(name).c_str());
    }
#endif

    m_AllSounds[si].curpan = pan;
    m_AllSounds[si].curvol = vol;

    return m_AllSounds[si].id;
}

bool CSound::IsSoundPlay(uint32_t id) {
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

bool SLID::IsPlayed(void) {
    DTRACE();

    if (index >= 0 && index < MAX_SOUNDS) {
        if (m_AllSounds[index].id == id) {
            return g_RangersInterface->m_SoundIsPlay(m_AllSounds[index].id_internal) != 0;
        }
    }
    return false;
}

uint32_t PlayInternal(ESound snd, float vol, float pan, ESoundLayer sl, ESoundInterruptFlag interrupt) {
    DTRACE();

    if (g_RangersInterface) {
        if (vol < 0.00001f)
            return SOUND_ID_EMPTY;

        //#if defined _TRACE || defined _DEBUG
        //    try
        //    {
        //#endif

        int si;

        uint32_t newid = m_LastID++;

        if (sl != SL_ALL) {
            if (interrupt == SEF_SKIP && m_LayersI[sl].IsPlayed()) {
                return SOUND_ID_EMPTY;
            }
            CSound::LayerOff(sl);
            m_LayersI[sl].id = newid;
        }

        si = FindSlotForSound();
        // TODO: non-const pointer is required here for no reason
        wchar_t* path = const_cast<wchar_t*>(m_Sounds[snd].path.c_str());
        m_AllSounds[si].id_internal =
                snd_create(path, m_LastGroup++, FLAG(m_Sounds[snd].flags, SSoundItem::LOOPED));
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
        //        ERROR_S(L"Problem with sound: " + m_Sounds[snd].path);
        //    }
        //#endif
    }
    else {
        return SOUND_ID_EMPTY;
    }
}

uint32_t CSound::Play(ESound snd, ESoundLayer sl, ESoundInterruptFlag interrupt) {
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

uint32_t CSound::Play(ESound snd, const D3DXVECTOR3 &pos, ESoundLayer sl, ESoundInterruptFlag interrupt) {
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

uint32_t CSound::Play(uint32_t id, ESound snd, const D3DXVECTOR3 &pos, ESoundLayer sl, ESoundInterruptFlag interrupt) {
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

uint32_t CSound::ChangePos(uint32_t id, ESound snd, const D3DXVECTOR3 &pos) {
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

void CSound::StopPlay(uint32_t id) {
    DTRACE();

    if (id == SOUND_ID_EMPTY)
        return;
    if (g_RangersInterface) {
        // g_MatrixMap->m_DI.T(std::wstring(L"sndoff") + (int)id, L"");

        int idx = FindSoundSlotPlayedOnly(id);
        if (idx >= 0) {
            StopPlayInternal(idx);
        }
    }
}

float CSound::GetSoundMaxDistSQ(ESound snd)
{
    SureLoaded(snd);
    return m_Sounds[snd].radius * m_Sounds[snd].radius;
}

inline uint32_t CSound::Pos2Key(const D3DXVECTOR3 &pos) {
    DTRACE();

    int x = Float2Int(pos.x / SOUND_POS_DIVIDER);
    int y = Float2Int(pos.y / SOUND_POS_DIVIDER);
    int z = Float2Int(pos.z / SOUND_POS_DIVIDER);

    uint32_t key = 0;

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
    key |= ((uint32_t)z) << 26;

    return key;
}

void CSound::AddSound(ESound snd, const D3DXVECTOR3 &pos, ESoundLayer sl,
                      ESoundInterruptFlag ifl)  // automatic position
{
    DTRACE();

    if (!g_RangersInterface)
        return;
    uint32_t key = Pos2Key(pos);

    if (!m_PosSounds.contains(key))
    {
        m_PosSounds.emplace(key, CSoundArray{});
    }
    m_PosSounds[key].AddSound(snd, pos, sl, ifl);
}

void CSound::AddSound(const wchar_t *name, const D3DXVECTOR3 &pos) {
    DTRACE();
    if (!g_RangersInterface)
        return;

    RESETFLAG(m_Sounds[S_SPECIAL_SLOT].flags, SSoundItem::LOADED);
    m_Sounds[S_SPECIAL_SLOT].path = name;
    SureLoaded(S_SPECIAL_SLOT);

    AddSound(pos, m_Sounds[S_SPECIAL_SLOT].attn, m_Sounds[S_SPECIAL_SLOT].pan0, m_Sounds[S_SPECIAL_SLOT].pan1,
             m_Sounds[S_SPECIAL_SLOT].vol0, m_Sounds[S_SPECIAL_SLOT].vol1, m_Sounds[S_SPECIAL_SLOT].path.c_str());
}

void CSound::AddSound(const D3DXVECTOR3 &pos, float attn, float pan0, float pan1, float vol0, float vol1, const wchar_t *name) {
    DTRACE();

    if (!g_RangersInterface)
        return;
    uint32_t key = Pos2Key(pos);

    if (!m_PosSounds.contains(key))
    {
        m_PosSounds.emplace(key, CSoundArray{});
    }
    m_PosSounds[key].AddSound(pos, attn, pan0, pan1, vol0, vol1, name);
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

    m_PosSounds.clear();
}

void CSoundArray::UpdateTimings(float ms)
{
    DTRACE();

    if (!g_RangersInterface)
    {
        m_array.clear();
        return;
    }

    for (auto iter = m_array.begin(); iter < m_array.end();)
    {
        if (iter->snd == S_UNDEF)
        {
            ++iter;
            continue;
        }

        int idx = FindSoundSlotPlayedOnly(iter->id);
        if (idx < 0)
        {
            iter = m_array.erase(iter);
            continue;
        }

        if (iter->ttl < 0)
        {
            if (iter->fade < 0)
            {
                if (iter->id == 1)
                {
                    debugbreak();
                }

                StopPlayInternal(idx);

                iter = m_array.erase(iter);
                continue;
            }

            iter->fade -= ms;
        }
        else
        {
            iter->ttl -= ms;
            if (iter->ttl < 0)
            {
                iter->ttl = -(iter->fade);
            }
        }

        ++iter;
    }
}

void CSoundArray::SetSoundPos(const D3DXVECTOR3 &pos)
{
    DTRACE();

    if (!g_RangersInterface)
    {
        m_array.clear();
        return;
    }

    for (auto iter = m_array.begin(); iter < m_array.end();)
    {
        DCP();
        int idx = FindSoundSlotPlayedOnly(iter->id);
        if (idx < 0)
        {
            iter = m_array.erase(iter);
            continue;
        }

        DCP();
        float k = 1.0f;
        if (iter->ttl < 0 && iter->snd != S_UNDEF) {
            k = -iter->fade / iter->ttl;
        }

        DCP();
        float pan, vol;
        CSound::CalcPanVol(pos, iter->attn, iter->pan0, iter->pan1, iter->vol0, iter->vol1, &pan, &vol);

        DCP();
        vol *= k;
        DCP();
        if (vol < 0.00001f) {
            DCP();
            StopPlayInternal(idx);
            DCP();

            iter = m_array.erase(iter);
            continue;
        }
#if defined _TRACE || defined _DEBUG
        try {
#endif
            DCP();

            snd_pan(m_AllSounds[idx].id_internal, pan);
            DCP();
            snd_vol(m_AllSounds[idx].id_internal, vol);
            DCP();

#if defined _TRACE || defined _DEBUG
        }
        catch (...) {
            if (iter->snd < S_COUNT && (int)iter->snd >= 0) {
                ERROR_S2(L"Problem with sound: ", m_Sounds[iter->snd].path.c_str());
            }
            else {
                ERROR_S2(L"Problem with sound: ", utils::format(L"%d", (int)iter->snd).c_str());
            }
        }
#endif

        m_AllSounds[idx].curpan = pan;
        m_AllSounds[idx].curvol = vol;

        ++iter;
    }
}

void CSoundArray::AddSound(ESound snd, const D3DXVECTOR3 &pos, ESoundLayer sl, ESoundInterruptFlag ifl)
{
    DTRACE();

    for (auto iter = m_array.begin(); iter < m_array.end(); ++iter)
    {
        if (iter->snd == snd)
        {
            if (ifl == SEF_INTERRUPT)
            {
                CSound::StopPlay(iter->id);
                m_array.erase(iter);
                break;
            }
            if (!CSound::IsSoundPlay(iter->id))
            {
                m_array.erase(iter);
                break;
            }

            // oops. the same sound
            // only set ttl and fade
            iter->ttl = m_Sounds[snd].ttl;
            iter->fade = m_Sounds[snd].fadetime;
            return;
        }
    }

    uint32_t id = CSound::Play(snd, pos, sl, ifl);
    if (id == SOUND_ID_EMPTY)
        return;

    m_array.emplace_back();
    auto& item = m_array.back();

    item.snd = snd;
    item.id = id;
    item.pan0 = m_Sounds[snd].pan0;
    item.pan1 = m_Sounds[snd].pan1;
    item.vol0 = m_Sounds[snd].vol0;
    item.vol1 = m_Sounds[snd].vol1;
    item.attn = m_Sounds[snd].attn;
    item.ttl = m_Sounds[snd].ttl;
    item.fade = m_Sounds[snd].fadetime;
    return;
}

void CSound::SaveSoundLog()
{
    DTRACE();

    std::ofstream out("log_sounds.txt");
    out << "Sounds:\n";

    for (int i = 0; i < MAX_SOUNDS; ++i)
    {
        out <<
            utils::format(
                "%d - id:%d, idi:%d, vol:%.8f, pan:%.8f, rvol:%.8f, is_play:%d\n",
                i,
                int(m_AllSounds[i].id),
                int(m_AllSounds[i].id_internal),
                m_AllSounds[i].curvol,
                m_AllSounds[i].curpan,
                g_RangersInterface->m_SoundGetVolume(m_AllSounds[i].id_internal),
                g_RangersInterface->m_SoundIsPlay(m_AllSounds[i].id_internal));
    }
}
