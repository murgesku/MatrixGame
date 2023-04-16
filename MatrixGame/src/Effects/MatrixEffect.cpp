// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>

#include "MatrixEffect.hpp"
#include "MatrixEffectShleif.hpp"
#include "MatrixEffectMoveTo.hpp"
#include "MatrixEffectElevatorField.hpp"
#include "MatrixEffectDust.hpp"
#include "MatrixEffectLightening.hpp"
#include "MatrixEffectZahvat.hpp"
#include "MatrixEffectBigBoom.hpp"
#include "MatrixEffectFlame.hpp"
#include "MatrixEffectPointLight.hpp"
#include "MatrixEffectSelection.hpp"
#include "MatrixEffectPath.hpp"
#include "MatrixEffectLandscapeSpot.hpp"
#include "MatrixEffectFirePlasma.hpp"
#include "MatrixEffectMovingObject.hpp"
#include "MatrixEffectExplosion.hpp"
#include "MatrixEffectSmokeAndFire.hpp"

#include "../MatrixMap.hpp"
#include "../MatrixObject.hpp"
#include "../MatrixRenderPipeline.hpp"
#include "../MatrixSkinManager.hpp"
#include <math.h>

#define M_PI 3.14159265358979323846

DWORD CMatrixEffect::m_before_draw_done;
CDebris *CMatrixEffect::m_Debris;
int CMatrixEffect::m_DebrisCnt;

SSpotProperties CMatrixEffect::m_SpotProperties[SPOT_TYPES_CNT];
SBillboardTextureArrayElement CMatrixEffect::m_BBTextures[BBT_LAST];

float CMatrixEffect::m_Dist2;  // used by movin objects (missiles cannons)

Base::CHeap *CMatrixEffect::m_Heap;

ELIST_DECLARE_OUTCLASS(EFFECT_EXPLOSION);
ELIST_DECLARE_OUTCLASS(EFFECT_POINT_LIGHT);
ELIST_DECLARE_OUTCLASS(EFFECT_SMOKE);
ELIST_DECLARE_OUTCLASS(EFFECT_LANDSCAPE_SPOT);
ELIST_DECLARE_OUTCLASS(EFFECT_MOVETO);
ELIST_DECLARE_OUTCLASS(EFFECT_FIREANIM);

static const SBillboardID2Texname bb2tn[] = {{BBT_SMOKE, BB_TEXTURE_NAME_SMOKE},
                                             {BBT_SMOKE_INTENSE, BB_TEXTURE_NAME_SMOKE_INTENSE},
                                             {BBT_FIRE, BB_TEXTURE_NAME_FIRE},
                                             {BBT_FIRE_INTENSE, BB_TEXTURE_NAME_FIRE_INTENSE},
                                             {BBT_POINTLIGHT, BB_TEXTURE_NAME_POINTLIGHT},
                                             {BBT_LASEREND, BB_TEXTURE_NAME_LASEREND},
                                             {BBT_SELDOT, BB_TEXTURE_NAME_SELDOT},
                                             {BBT_PATHDOT, BB_TEXTURE_NAME_PATHDOT},
                                             {BBT_GLOWBEAMEND, BB_TEXTURE_NAME_GLOWBEAMEND},
                                             {BBT_FLAME, BB_TEXTURE_NAME_FLAME},
                                             {BBT_PLASMA1, BB_TEXTURE_NAME_PLASMA1},
                                             {BBT_PLASMA2, BB_TEXTURE_NAME_PLASMA2},
                                             {BBT_INTENSE, BB_TEXTURE_NAME_INTENSE},
                                             {BBT_LASER, BB_TEXTURE_NAME_LASER},
                                             {BBT_GUNFIRE1, BB_TEXTURE_NAME_GUNFIRE1},
                                             {BBT_GUNFIRE2, BB_TEXTURE_NAME_GUNFIRE2},
                                             {BBT_GUNFIRE3, BB_TEXTURE_NAME_GUNFIRE3},
                                             {BBT_TRASSA, BB_TEXTURE_NAME_TRASSA},
                                             {BBT_SHLEIF, BB_TEXTURE_NAME_SHLEIF},
                                             {BBT_FIRESTREAM1, BB_TEXTURE_NAME_FIRESTREAM1},
                                             {BBT_FIRESTREAM2, BB_TEXTURE_NAME_FIRESTREAM2},
                                             {BBT_FIRESTREAM3, BB_TEXTURE_NAME_FIRESTREAM3},
                                             {BBT_FIRESTREAM4, BB_TEXTURE_NAME_FIRESTREAM4},
                                             {BBT_FIRESTREAM5, BB_TEXTURE_NAME_FIRESTREAM5},
                                             {BBT_GLOWBEAM, BB_TEXTURE_NAME_GLOWBEAM},
                                             {BBT_FLAMELINE, BB_TEXTURE_NAME_FLAMELINE},
                                             {BBT_SPARK, BB_TEXTURE_NAME_SPARK},
                                             {BBT_EFIELD, BB_TEXTURE_NAME_EFIELD},
                                             {BBT_FLAMEFRAME0, BB_TEXTURE_NAME_FLAMEFRAME0},
                                             {BBT_FLAMEFRAME1, BB_TEXTURE_NAME_FLAMEFRAME1},
                                             {BBT_FLAMEFRAME2, BB_TEXTURE_NAME_FLAMEFRAME2},
                                             {BBT_FLAMEFRAME3, BB_TEXTURE_NAME_FLAMEFRAME3},
                                             {BBT_FLAMEFRAME4, BB_TEXTURE_NAME_FLAMEFRAME4},
                                             {BBT_FLAMEFRAME5, BB_TEXTURE_NAME_FLAMEFRAME5},
                                             {BBT_FLAMEFRAME6, BB_TEXTURE_NAME_FLAMEFRAME6},
                                             {BBT_FLAMEFRAME7, BB_TEXTURE_NAME_FLAMEFRAME7},

                                             {BBT_REPGLOW, BB_TEXTURE_NAME_REPGLOW},
                                             {BBT_REPGLOWEND, BB_TEXTURE_NAME_REPGLOWEND},

                                             {BBT_SCOREPLUS, BB_TEXTURE_NAME_SCOREPLUS},
                                             {BBT_SCORE0, BB_TEXTURE_NAME_SCORE0},
                                             {BBT_SCORE1, BB_TEXTURE_NAME_SCORE1},
                                             {BBT_SCORE2, BB_TEXTURE_NAME_SCORE2},
                                             {BBT_SCORE3, BB_TEXTURE_NAME_SCORE3},
                                             {BBT_SCORE4, BB_TEXTURE_NAME_SCORE4},
                                             {BBT_SCORE5, BB_TEXTURE_NAME_SCORE5},
                                             {BBT_SCORE6, BB_TEXTURE_NAME_SCORE6},
                                             {BBT_SCORE7, BB_TEXTURE_NAME_SCORE7},
                                             {BBT_SCORE8, BB_TEXTURE_NAME_SCORE8},
                                             {BBT_SCORE9, BB_TEXTURE_NAME_SCORE9},
                                             {BBT_SCOREICON1, BB_TEXTURE_NAME_SCOREICON1},
                                             {BBT_SCOREICON2, BB_TEXTURE_NAME_SCOREICON2},
                                             {BBT_SCOREICON3, BB_TEXTURE_NAME_SCOREICON3},
                                             {BBT_SCOREICON4, BB_TEXTURE_NAME_SCOREICON4},
                                             {BBT_SCOREICONS, BB_TEXTURE_NAME_SCOREICONS},

#ifdef _DEBUG
                                             {BBT_LAST, NULL}
#endif
};

//////////////////////////////
//////////////////////////////

CEffectSpawner::CEffectSpawner(int minperiod, int maxperiod, int ttl, SpawnEffectProps *props)
  : CMain(), m_MIN_period(minperiod), m_MAX_period(maxperiod),
    m_NextTime(Double2Int(g_MatrixMap->GetTime() + (RND(minperiod, maxperiod)))), m_Under(TRACE_STOP_NONE),
    m_TTL((ttl > 1) ? (g_MatrixMap->GetTime() + ttl) : 0) {
    m_Props = (SpawnEffectProps *)HAlloc(props->m_Size, CMatrixEffect::m_Heap);
    memcpy(m_Props, props, props->m_Size);
}

void SpawnEffectLighteningDestructor(SpawnEffectProps *props) {
    SpawnEffectLightening *li = (SpawnEffectLightening *)props;
    if (li->m_Pair) {
        li->m_Pair->m_Pair = NULL;
#ifdef _DEBUG
        li->m_Effect.Release(DEBUG_CALL_INFO);
#else
        li->m_Effect.Release();
#endif
    }
}

CEffectSpawner::~CEffectSpawner() {
    if (m_Props->m_Destructor)
        m_Props->m_Destructor(m_Props);
    HFree(m_Props, CMatrixEffect::m_Heap);
}

struct FOwner {
    float dist;
    CMatrixMapStatic **target;
};

static bool FindOwner(const D3DXVECTOR3 &center, CMatrixMapStatic *ms, DWORD user) {
    FOwner *d = (FOwner *)user;

    D3DXVECTOR3 bmin, bmax;

    ms->CalcBounds(bmin, bmax);
    D3DXVECTOR3 pos = (bmin + bmax) * 0.5f;

    if (center.x < bmin.x)
        return true;
    if (center.y < bmin.y)
        return true;
    if (center.z < bmin.z)
        return true;

    if (center.x > bmax.x)
        return true;
    if (center.y > bmax.y)
        return true;
    if (center.z > bmax.z)
        return true;

    auto tmp = center - pos;
    float di = D3DXVec3LengthSq(&tmp);
    if (di < d->dist) {
        d->dist = di;
        *d->target = ms;
    }
    return true;
}

void CEffectSpawner::Takt(float) {
    if (m_Under == NULL) {
        FOwner data;
        data.dist = 100 * 100;
        data.target = &m_Under;

        g_MatrixMap->FindObjects(m_Props->m_Pos, 100, 1.0f, TRACE_BUILDING | TRACE_OBJECT, NULL, FindOwner,
                                 (DWORD)&data);
        if (m_Under == NULL) {
            m_Under = TRACE_STOP_LANDSCAPE;
        }
    }

    if (g_MatrixMap->GetTime() > m_NextTime) {
        m_NextTime = g_MatrixMap->GetTime() + Double2Int(RND(m_MIN_period, m_MAX_period));
        m_Props->m_Spawner(m_Props);
    }
}

bool CEffectSpawner::OutOfTime(void) const {
    return (m_TTL > 1) && (g_MatrixMap->GetTime() > m_TTL);
}

void EffectSpawnerFire(SpawnEffectProps *props) {
    SpawnEffectFire *fire = (SpawnEffectFire *)props;
    CMatrixEffect::CreateFire(NULL, fire->m_Pos, fire->m_ttl, fire->m_puffttl, fire->m_spawntime, fire->m_dispfactor,
                              fire->m_intense, fire->m_speed);
}
void EffectSpawnerSmoke(SpawnEffectProps *props) {
    SpawnEffectSmoke *smoke = (SpawnEffectSmoke *)props;
    CMatrixEffect::CreateSmoke(NULL, smoke->m_Pos, smoke->m_ttl, smoke->m_puffttl, smoke->m_spawntime, smoke->m_color,
                               smoke->m_intense, smoke->m_speed);
}
void EffectSpawnerSound(SpawnEffectProps *props) {
    SpawnEffectSound *sound = (SpawnEffectSound *)props;
    CSound::AddSound(sound->m_Pos, sound->m_attn * 0.002f, sound->m_pan0, sound->m_pan1, sound->m_vol0, sound->m_vol1,
                     sound->m_name);
}
void EffectSpawnerLightening(SpawnEffectProps *props) {
    SpawnEffectLightening *li = (SpawnEffectLightening *)props;

    ASSERT(li->m_Width > 0);  // make sure it has pair (editor must save this effect correctly)

    if (li->m_Pair && li->m_Dispers >= 0) {
        // ok, effect has pair and its host

        SpawnEffectLightening *l = (FRND(1) < 0.5f) ? li : li->m_Pair;

        CMatrixEffect::CreateLightening(&li->m_Effect, li->m_Pos, li->m_Pair->m_Pos, l->m_ttl, li->m_Dispers,
                                        l->m_Width, l->m_Color);
    }
}

//////////////////////////////
//////////////////////////////

void CMatrixEffect::ClearEffects(void) {
    DTRACE();
    if (m_DebrisCnt > 0) {
        for (int i = 0; i < m_DebrisCnt; ++i) {
            m_Debris[i].~CDebris();
        }
    }

    if (m_Debris)
        HFree(m_Debris, m_Heap);
    m_DebrisCnt = 0;
    m_Debris = 0;

    CBillboard::Deinit();
    CMatrixEffectPointLight::ClearAll();

    if (m_Heap) {
        HDelete(CHeap, m_Heap, NULL);
        m_Heap = NULL;
    }
}

void CMatrixEffect::CreatePoolDefaultResources(void) {
    DTRACE();
    CBillboard::Init();
}

void CMatrixEffect::ReleasePoolDefaultResources(void) {
    DTRACE();
    CBillboard::Deinit();

    CMatrixEffectBigBoom::MarkAllBuffersNoNeed();
    CMatrixEffectBillboard::MarkAllBuffersNoNeed();
    CMatrixEffectKonus::MarkAllBuffersNoNeed();
    CMatrixEffectLandscapeSpot::MarkAllBuffersNoNeed();
    CMatrixEffectPointLight::MarkAllBuffersNoNeed();
}

void CMatrixEffect::InitEffects(CBlockPar &bp_in) {
    DTRACE();

    // Static Init

    CMatrixEffectBigBoom::StaticInit();
    CMatrixEffectBillboard::StaticInit();
    CMatrixEffectKonus::StaticInit();
    CMatrixEffectLandscapeSpot::StaticInit();
    CMatrixEffectMoveto::StaticInit();

    CMatrixEffectPointLight::StaticInit();

    ELIST_INIT(EFFECT_EXPLOSION);
    ELIST_INIT(EFFECT_POINT_LIGHT);
    ELIST_INIT(EFFECT_SMOKE);
    ELIST_INIT(EFFECT_LANDSCAPE_SPOT);
    ELIST_INIT(EFFECT_MOVETO);
    ELIST_INIT(EFFECT_FIREANIM);

    m_Heap = HNew(NULL) CHeap();

    // init debris

    CBlockPar &bp = *bp_in.BlockGet(L"Models")->BlockGet(L"Debris");

    m_DebrisCnt = bp.ParCount();
    m_Debris = (CDebris *)HAlloc(sizeof(CDebris) * m_DebrisCnt, m_Heap);

    for (int i = 0; i < m_DebrisCnt; ++i) {
        new(&m_Debris[i]) CDebris();

        CVectorObject *vo = (CVectorObject *)g_Cache->Get(cc_VO, bp.ParGet(i).c_str());
        vo->PrepareSpecial(OLF_AUTO, CSkinManager::GetSkin, GSP_ORDINAL);
        m_Debris[i].Init(vo, NULL);

        int n = bp.ParGetName(i).GetStrPar(0, L",").GetInt();
        m_Debris[i].SetDebType(n);
    }

    // init spots

    m_SpotProperties[SPOT_CONSTANT].color = 0xFF000000;
    m_SpotProperties[SPOT_CONSTANT].func = SpotTaktConstant;
    m_SpotProperties[SPOT_CONSTANT].texture = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_SPOT);
    m_SpotProperties[SPOT_CONSTANT].ttl = 15000;
    m_SpotProperties[SPOT_CONSTANT].flags = 0;

    m_SpotProperties[SPOT_SELECTION].color = 0x80808080;
    m_SpotProperties[SPOT_SELECTION].func = SpotTaktAlways;
    m_SpotProperties[SPOT_SELECTION].texture = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_OBJSEL);
    m_SpotProperties[SPOT_SELECTION].texture->MipmapOff();
    m_SpotProperties[SPOT_SELECTION].ttl = 0;
    m_SpotProperties[SPOT_SELECTION].flags = 0;

    m_SpotProperties[SPOT_PLASMA_HIT].color = 0xFFFFFFFF;
    m_SpotProperties[SPOT_PLASMA_HIT].func = SpotTaktPlasmaHit;
    m_SpotProperties[SPOT_PLASMA_HIT].texture = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_HIT);
    m_SpotProperties[SPOT_PLASMA_HIT].ttl = 3000;
    m_SpotProperties[SPOT_PLASMA_HIT].flags = LSFLAG_SCALE_BY_NORMAL;

    // m_SpotProperties[SPOT_MOVE_TO].color = 0x80808080;
    // m_SpotProperties[SPOT_MOVE_TO].func = SpotTaktMoveTo;
    // m_SpotProperties[SPOT_MOVE_TO].texture = (CTextureManaged *)g_Cache->Get(cc_TextureManaged,TEXTURE_PATH_MOVETO);
    // m_SpotProperties[SPOT_MOVE_TO].ttl = 1500;
    // m_SpotProperties[SPOT_MOVE_TO].flags = LSFLAG_SCALE_BY_NORMAL;

    m_SpotProperties[SPOT_POINTLIGHT].color = 0xFFFFFFFF;
    m_SpotProperties[SPOT_POINTLIGHT].func = SpotTaktPointlight;
    m_SpotProperties[SPOT_POINTLIGHT].texture =
            (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_POINTLIGHT);
    m_SpotProperties[SPOT_POINTLIGHT].ttl = 1500;
    m_SpotProperties[SPOT_POINTLIGHT].flags = LSFLAG_INTENSE;

    m_SpotProperties[SPOT_VORONKA].color = 0x00FFFFFF;
    m_SpotProperties[SPOT_VORONKA].func = SpotTaktVoronka;
    m_SpotProperties[SPOT_VORONKA].texture = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_VORONKA);
    m_SpotProperties[SPOT_VORONKA].ttl = 30000;
    m_SpotProperties[SPOT_VORONKA].flags = 0;

    m_SpotProperties[SPOT_TURRET].color = 0x80FFFFFF;
    m_SpotProperties[SPOT_TURRET].func = SpotTaktAlways;
    m_SpotProperties[SPOT_TURRET].texture =
            (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_TURRET_RADIUS);
    m_SpotProperties[SPOT_TURRET].texture->MipmapOff();
    m_SpotProperties[SPOT_TURRET].ttl = 0;
    m_SpotProperties[SPOT_TURRET].flags = 0;

    m_SpotProperties[SPOT_SOLE_TRACK].color = 0xFFFFFFFF;
    m_SpotProperties[SPOT_SOLE_TRACK].func = SpotTaktConstant;
    m_SpotProperties[SPOT_SOLE_TRACK].texture =
            (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_SOLE_TRACK);
    m_SpotProperties[SPOT_SOLE_TRACK].ttl = 30000;
    m_SpotProperties[SPOT_SOLE_TRACK].flags = 0;

    m_SpotProperties[SPOT_SOLE_WHEEL].color = 0xFFFFFFFF;
    m_SpotProperties[SPOT_SOLE_WHEEL].func = SpotTaktConstant;
    m_SpotProperties[SPOT_SOLE_WHEEL].texture =
            (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_SOLE_WHEEL);
    m_SpotProperties[SPOT_SOLE_WHEEL].ttl = 30000;
    m_SpotProperties[SPOT_SOLE_WHEEL].flags = 0;

    m_SpotProperties[SPOT_SOLE_PNEUMATIC].color = 0xFFFFFFFF;
    m_SpotProperties[SPOT_SOLE_PNEUMATIC].func = SpotTaktConstant;
    m_SpotProperties[SPOT_SOLE_PNEUMATIC].texture =
            (CTextureManaged *)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_SOLE_PNEUMATIC);
    m_SpotProperties[SPOT_SOLE_PNEUMATIC].ttl = 30000;
    m_SpotProperties[SPOT_SOLE_PNEUMATIC].flags = 0;

    // init billboards

    CBlockPar &bb = *bp_in.BlockGet(PAR_SOURCE_BILLBOARDS);
    CBlockPar &tex = *bb.BlockGet(PAR_SOURCE_BILLBOARDS_TEXTURES);

    CTextureManaged *ts = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, bb.ParGet(PAR_SOURCE_BILLBOARDS_TEXSORT).c_str());
    // CTextureManaged *ti = (CTextureManaged *)g_Cache->Get(cc_TextureManaged,
    // bb.ParGet(PAR_SOURCE_BILLBOARDS_TEXINTENSE));
    ts->Preload();
    // ti->Preload();

    CBillboard::SetSortTexture(ts);

    for (int i = 0; i < BBT_LAST; ++i) {
        auto texp = tex.ParGet(bb2tn[i].name);
        SBillboardTextureArrayElement *e = m_BBTextures + bb2tn[i].id;
        if (texp.GetStrPar(0, L",").GetInt() == 0) {
            RESETFLAG(e->flags, BBT_FLAG_INTENSE);

            int x0, y0, sx, sy;
            x0 = texp.GetStrPar(1, L",").GetInt();
            y0 = texp.GetStrPar(2, L",").GetInt();
            sx = texp.GetStrPar(3, L",").GetInt();
            sy = texp.GetStrPar(4, L",").GetInt();

            e->bbt.tu0 = float(x0) / float(ts->GetSizeX());
            e->bbt.tv0 = float(y0) / float(ts->GetSizeY());

            e->bbt.tu1 = float(x0 + sx) / float(ts->GetSizeX());
            e->bbt.tv1 = float(y0 + sy) / float(ts->GetSizeY());
        }
        else {
            SETFLAG(e->flags, BBT_FLAG_INTENSE);

            e->tex = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, texp.GetStrPar(1, L",").c_str());
            // e->tex->Preload();
        }
    }

    m_before_draw_done = 0;
}

#ifdef _DEBUG
void SEffectHandler::Release(const SDebugCallInfo &from)
#else
void SEffectHandler::Release(void)
#endif
{
    if (effect) {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(from, effect);
#else
        g_MatrixMap->SubEffect(effect);
#endif
        effect = NULL;
    }
}

void CMatrixEffect::CreateExplosion(const D3DXVECTOR3 &pos, const SExplosionProperties &props, bool fire) {
    DTRACE();

    auto tmp = pos - g_MatrixMap->m_Camera.GetFrustumCenter();
    if (MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&tmp))
        return;

    if (ELIST_CNT(EFFECT_EXPLOSION) >= MAX_EFFECTS_EXPLOSIONS) {
        // remove oldest

#ifdef _DEBUG
        // DM("aaaaaaa", "Remove oldest explosion");
#endif

        ELSIT_REMOVE_OLDEST(EFFECT_EXPLOSION);
    }

    CMatrixEffectExplosion *e;

    float z = g_MatrixMap->GetZ(pos.x, pos.y);
    if (pos.z < (z + 10)) {
        e = HNew(m_Heap) CMatrixEffectExplosion(D3DXVECTOR3(pos.x, pos.y, z + 10), props);
    }
    else {
        e = HNew(m_Heap) CMatrixEffectExplosion(pos, props);
    }
    if (!g_MatrixMap->AddEffect(e))
        return;

    if (props.sound != S_NONE)
        CSound::AddSound(props.sound, pos);
    // CMatrixEffect *ee = CMatrixEffect::CreateLandscapeSpot(m_Heap, D3DXVECTOR2(pos.x,pos.y), FSRND(3.1415), FRND(2) +
    // 0.5f, SPOT_CONSTANT);

    if (fire) {
        // CMatrixEffect::CreateFire(NULL, pos, 1000, 1500, 50, 10, false);

        CMatrixEffect::CreateFireAnim(NULL, D3DXVECTOR3(pos.x, pos.y, z), 35, 60, 2000);
        CMatrixEffect::CreateSmoke(NULL, pos, 1500, 1500, 100, 0xFF000000);
    }
    if (props.voronka != SPOT_TYPES_CNT) {
        int x = TruncFloat(pos.x * INVERT(GLOBAL_SCALE * MAP_GROUP_SIZE));
        int y = TruncFloat(pos.y * INVERT(GLOBAL_SCALE * MAP_GROUP_SIZE));
        CMatrixMapGroup *g = g_MatrixMap->GetGroupByIndexTest(x, y);
        if (g && !g->IsBaseOn()) {
            CMatrixEffect::CreateLandscapeSpot(NULL, D3DXVECTOR2(pos.x, pos.y), FSRND(M_PI), props.voronka_scale,
                                               props.voronka);
        }
    }
}

void CMatrixEffect::CreateSmoke(SEffectHandler *eh, const D3DXVECTOR3 &pos, float ttl, float puffttl, float spawntime,
                                DWORD color, bool intense, float speed) {
    DTRACE();

    auto tmp = pos - g_MatrixMap->m_Camera.GetFrustumCenter();
    if (MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&tmp))
        return;

    if (ELIST_CNT(EFFECT_SMOKE) >= MAX_EFFECTS_SMOKEFIRES) {
        // remove oldest
        ELSIT_REMOVE_OLDEST(EFFECT_SMOKE);
    }

    CMatrixEffectSmoke *e = HNew(m_Heap) CMatrixEffectSmoke(pos, ttl, puffttl, spawntime, color, intense, speed);
    if (!g_MatrixMap->AddEffect(e))
        e = NULL;
    if (eh && e) {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateFire(SEffectHandler *eh, const D3DXVECTOR3 &pos, float ttl, float puffttl, float spawntime,
                               float dispfactor, bool intense, float speed) {
    DTRACE();

    auto tmp = pos - g_MatrixMap->m_Camera.GetFrustumCenter();
    if (MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&tmp))
        return;

    if (ELIST_CNT(EFFECT_SMOKE) >= MAX_EFFECTS_SMOKEFIRES) {
        // remove oldest
        ELSIT_REMOVE_OLDEST(EFFECT_SMOKE);
    }

    CMatrixEffectFire *e = HNew(m_Heap) CMatrixEffectFire(pos, ttl, puffttl, spawntime, dispfactor, intense, speed);
    if (!g_MatrixMap->AddEffect(e))
        e = NULL;
    if (eh && e) {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateFireAnim(SEffectHandler *eh, const D3DXVECTOR3 &pos, float w, float h, int ttl) {
    auto tmp = pos - g_MatrixMap->m_Camera.GetFrustumCenter();
    if (MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&tmp))
        return;

    if (ELIST_CNT(EFFECT_FIREANIM) >= MAX_EFFECTS_FIREANIM) {
        // remove oldest
        ELSIT_REMOVE_OLDEST(EFFECT_FIREANIM);
    }

    CMatrixEffectFireAnim *e = HNew(m_Heap) CMatrixEffectFireAnim(pos, w, h, ttl);
    if (!g_MatrixMap->AddEffect(e))
        e = NULL;
    if (eh && e) {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateFirePlasma(const D3DXVECTOR3 &start, const D3DXVECTOR3 &end, float speed, DWORD hitmask,
                                     CMatrixMapStatic *skip, FIRE_END_HANDLER handler, DWORD user) {
    DTRACE();

    CMatrixEffectFirePlasma *e = HNew(m_Heap) CMatrixEffectFirePlasma(start, end, speed, hitmask, skip, handler, user);
    g_MatrixMap->AddEffect(e);
}
#ifdef _DEBUG
#include "stdio.h"
#endif
void CMatrixEffect::CreateLandscapeSpot(SEffectHandler *eh, const D3DXVECTOR2 &pos, float angle, float scale,
                                        ESpotType type) {
    DTRACE();

    auto tmp = pos - *(D3DXVECTOR2 *)&g_MatrixMap->m_Camera.GetFrustumCenter();
    if (type != SPOT_TURRET) {
        if (MAX_EFFECT_DISTANCE_SQ < D3DXVec2LengthSq(&tmp))
            return;
    }

    CMatrixEffectLandscapeSpot *e = HNew(m_Heap) CMatrixEffectLandscapeSpot(pos, angle, scale, type);

    if (ELIST_CNT(EFFECT_LANDSCAPE_SPOT) >= MAX_EFFECTS_LANDSPOTS) {
        // remove oldest
        //#ifdef _DEBUG
        //
        //        {
        //            CMatrixEffect *ef = ELIST_FIRST(EFFECT_LANDSCAPE_SPOT);
        //
        //            CStr log;
        //            DM("ef", "begin...........");
        //
        //            for (;ef;ef = ef->m_TypeNext)
        //            {
        //                std::wstring hex;
        //                hex.AddHex(BYTE((DWORD(ef) >> 24) & 0xFF));
        //                hex.AddHex(BYTE((DWORD(ef) >> 16) & 0xFF));
        //                hex.AddHex(BYTE((DWORD(ef) >> 8) & 0xFF));
        //                hex.AddHex(BYTE((DWORD(ef) >> 0) & 0xFF));
        //
        //                log += (int)ef->GetType();
        //                log += ":";
        //                log += utils::from_wstring(hex.Get()).c_str();
        //                log += ":";
        //                log += ef->Priority();
        //                //log += "\n";
        //
        //                DM("ef", log);
        //                log = "";
        //            }
        //
        //            log += "effect:";
        //            log += e->Priority();
        //            DM("ef", log);
        //            //log += "\n";
        //
        //            //SLOG("ls.txt", log.Get());
        //        }
        //#endif
        int p = e->Priority() + 1;
        ELSIT_REMOVE_OLDEST_PRIORITY(EFFECT_LANDSCAPE_SPOT, p);
    }

    if (!g_MatrixMap->AddEffect(e))
        e = NULL;
    if (eh && e) {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateMovingObject(SEffectHandler *eh, const SMOProps &props, DWORD hitmask, CMatrixMapStatic *skip,
                                       FIRE_END_HANDLER handler, DWORD user) {
    DTRACE();
    CMatrixEffectMovingObject *e = HNew(m_Heap) CMatrixEffectMovingObject(props, hitmask, skip, handler, user);
    if (!g_MatrixMap->AddEffect(e))
        e = NULL;
    if (eh && e) {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateMoveto(void) {
    DTRACE();

    CreateMoveto(g_MatrixMap->m_TraceStopPos);
}

void CMatrixEffect::CreateMoveto(const D3DXVECTOR3 &pos) {
    DTRACE();

    D3DXVECTOR3 tp = g_MatrixMap->m_Camera.GetFrustumCenter() - pos;
    D3DXVec3Normalize(&tp, &tp);

    CMatrixEffectMoveto *e = HNew(m_Heap) CMatrixEffectMoveto(pos + tp * 10);

    g_MatrixMap->AddEffect(e);
}

void CMatrixEffect::DeleteAllMoveto() {
    while (ELIST_FIRST(EFFECT_MOVETO))
        ELSIT_REMOVE_OLDEST(EFFECT_MOVETO);
}

void CMatrixEffect::CreateBuoy(SEffectHandler *eh, const D3DXVECTOR3 &pos, EBuoyType bt) {
    DTRACE();

    SMOProps mo;
    memset(&mo, 0, sizeof(mo));
    mo.curpos = pos;
    mo.buoytype = bt;
    // mo.velocity = D3DXVECTOR3(1,1,1);

    const wchar *name = OBJECT_PATH_LOGO_RED;

    if (bt == BUOY_BLUE)
        name = OBJECT_PATH_LOGO_BLUE;
    else if (bt == BUOY_GREEN)
        name = OBJECT_PATH_LOGO_GREEN;

    mo.object = LoadObject(name, m_Heap);

    CMatrixEffectBuoy *e = HNew(m_Heap) CMatrixEffectBuoy(mo);
    if (!g_MatrixMap->AddEffect(e))
        e = NULL;
    if (eh && e) {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

CMatrixEffect *CMatrixEffect::CreateSelection(const D3DXVECTOR3 &pos, float r, DWORD color) {
    DTRACE();

    CMatrixEffectSelection *e = HNew(m_Heap) CMatrixEffectSelection(pos, r, color);
    return e;
}

CMatrixEffect *CMatrixEffect::CreatePath(const D3DXVECTOR3 *pos, int cnt) {
    DTRACE();
    CMatrixEffectPath *e = HNew(m_Heap) CMatrixEffectPath(pos, cnt);
    return e;
}

void CMatrixEffect::CreatePointLight(SEffectHandler *eh, const D3DXVECTOR3 &pos, float r, DWORD color, bool drawbill) {
    DTRACE();

    auto tmp = pos - g_MatrixMap->m_Camera.GetFrustumCenter();
    if (MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&tmp))
        return;

    if (ELIST_CNT(EFFECT_POINT_LIGHT) >= MAX_EFFECTS_POINT_LIGHTS) {
        // remove oldest
        ELSIT_REMOVE_OLDEST(EFFECT_POINT_LIGHT);
    }

    CMatrixEffectPointLight *e = HNew(m_Heap) CMatrixEffectPointLight(pos, r, color, drawbill);
    if (!g_MatrixMap->AddEffect(e))
        e = NULL;
    if (eh && e) {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateKonus(SEffectHandler *eh, const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir, float radius,
                                float height, float angle, float ttl, bool intense, CTextureManaged *tex) {
    DTRACE();

    auto tmp = start - g_MatrixMap->m_Camera.GetFrustumCenter();
    if (MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&tmp))
        return;

    CMatrixEffectKonus *e = HNew(m_Heap) CMatrixEffectKonus(start, dir, radius, height, angle, ttl, intense, tex);
    if (!g_MatrixMap->AddEffect(e))
        e = NULL;
    if (eh && e) {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateKonusSplash(const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir, float radius, float height,
                                      float angle, float ttl, bool intense, CTextureManaged *tex) {
    DTRACE();

    auto tmp = start - g_MatrixMap->m_Camera.GetFrustumCenter();
    if (MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&tmp))
        return;

    CMatrixEffectKonusSplash *e =
            HNew(m_Heap) CMatrixEffectKonusSplash(start, dir, radius, height, angle, ttl, intense, tex);

    if (!g_MatrixMap->AddEffect(e))
        e = NULL;
    if (e) {
        CSound::AddSound(S_SPLASH, start);
    }
}

CMatrixEffect *CMatrixEffect::CreateWeapon(const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir, DWORD user,
                                           FIRE_END_HANDLER handler, EWeapon type, int cooldown) {
    DTRACE();
    CMatrixEffectWeapon *e = HNew(m_Heap) CMatrixEffectWeapon(start, dir, user, handler, type, cooldown);
    return e;
}

void CMatrixEffect::CreateFlame(SEffectHandler *eh, float ttl, DWORD hitmask, CMatrixMapStatic *skip, DWORD user,
                                FIRE_END_HANDLER handler) {
    DTRACE();
    CMatrixEffectFlame *e = HNew(m_Heap) CMatrixEffectFlame(ttl, hitmask, skip, user, handler);
    if (!g_MatrixMap->AddEffect(e))
        e = NULL;
    if (eh && e) {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateBigBoom(const D3DXVECTOR3 &pos, float radius, float ttl, DWORD hitmask,
                                  CMatrixMapStatic *skip, DWORD user, FIRE_END_HANDLER handler, DWORD light) {
    DTRACE();
    CMatrixEffectBigBoom *e = HNew(m_Heap) CMatrixEffectBigBoom(pos, radius, ttl, hitmask, skip, user, handler, light);
    g_MatrixMap->AddEffect(e);
}

void CMatrixEffect::CreateLightening(SEffectHandler *eh, const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float ttl,
                                     float dispers, float width, DWORD color) {
    DTRACE();

    auto tmp = pos0 - g_MatrixMap->m_Camera.GetFrustumCenter();
    if (MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&tmp))
        return;

    CMatrixEffectLightening *e = HNew(m_Heap) CMatrixEffectLightening(pos0, pos1, ttl, dispers, width, color);
    e->SetPos(pos0, pos1);
    if (!g_MatrixMap->AddEffect(e))
        e = NULL;
    if (eh && e) {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateShorted(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float ttl, DWORD color) {
    DTRACE();
    CMatrixEffectShorted *e = HNew(m_Heap) CMatrixEffectShorted(pos0, pos1, ttl, color);
    e->SetPos(pos0, pos1);
    g_MatrixMap->AddEffect(e);
}

void CMatrixEffect::CreateBillboard(SEffectHandler *eh, const D3DXVECTOR3 &pos, float radius1, float radius2, DWORD c1,
                                    DWORD c2, float ttl, float delay, const wchar *tex, const D3DXVECTOR3 &dir,
                                    ADD_TAKT addtakt) {
    DTRACE();
    CMatrixEffectBillboard *e =
            HNew(m_Heap) CMatrixEffectBillboard(pos, radius1, radius2, c1, c2, ttl, delay, tex, dir, addtakt);
    if (!g_MatrixMap->AddEffect(e))
        e = NULL;
    if (eh && e) {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateBillboardScore(const wchar *n, const D3DXVECTOR3 &pos, DWORD color) {
    DTRACE();

    CSound::AddSound(S_RESINCOME, pos);

    CMatrixEffectBillboardScore *e = HNew(m_Heap) CMatrixEffectBillboardScore(n, pos, color);
    g_MatrixMap->AddEffect(e);
}

void CMatrixEffect::CreateBillboardLine(SEffectHandler *eh, const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1,
                                        float width, DWORD c1, DWORD c2, float ttl, CTextureManaged *tex) {
    DTRACE();

    auto tmp = pos0 - g_MatrixMap->m_Camera.GetFrustumCenter();
    if (MAX_EFFECT_DISTANCE_SQ < D3DXVec3LengthSq(&tmp))
        return;

    CMatrixEffectBillboardLine *e = HNew(m_Heap) CMatrixEffectBillboardLine(pos0, pos1, width, c1, c2, ttl, tex);
    if (!g_MatrixMap->AddEffect(e))
        e = NULL;
    if (eh && e) {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

CMatrixEffect *CMatrixEffect::CreateZahvat(const D3DXVECTOR3 &pos, float radius, float angle, int cnt) {
    DTRACE();
    CMatrixEffectZahvat *e = HNew(m_Heap) CMatrixEffectZahvat(pos, radius, angle, cnt);
    return e;
}

CMatrixEffect *CMatrixEffect::CreateElevatorField(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float radius,
                                                  const D3DXVECTOR3 &fwd) {
    DTRACE();
    CMatrixEffectElevatorField *e = HNew(m_Heap) CMatrixEffectElevatorField(pos0, pos1, radius, fwd);
    return e;
}

void CMatrixEffect::CreateDust(SEffectHandler *eh, const D3DXVECTOR2 &pos, const D3DXVECTOR2 &adddir, float ttl) {
    DTRACE();

    auto tmp = pos - *(D3DXVECTOR2 *)&g_MatrixMap->m_Camera.GetFrustumCenter();
    if (MAX_EFFECT_DISTANCE_SQ < D3DXVec2LengthSq(&tmp))
        return;

    CMatrixEffectDust *e = HNew(m_Heap) CMatrixEffectDust(pos, adddir, ttl);
    if (!g_MatrixMap->AddEffect(e))
        e = NULL;
    if (eh && e) {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

void CMatrixEffect::CreateShleif(SEffectHandler *eh) {
    DTRACE();
    CMatrixEffectShleif *e = HNew(m_Heap) CMatrixEffectShleif();
    if (!g_MatrixMap->AddEffect(e))
        e = NULL;
    if (eh && e) {
#ifdef _DEBUG
        eh->Release(DEBUG_CALL_INFO);
#else
        eh->Release();
#endif
        eh->effect = e;
        e->SetHandler(eh);
    }
}

CMatrixEffect *CMatrixEffect::CreateRepair(const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, float seekradius,
                                           CMatrixMapStatic *skip) {
    DTRACE();
    CMatrixEffectRepair *e = HNew(m_Heap) CMatrixEffectRepair(pos, dir, seekradius, skip);
    return e;
}

CMatrixEffectFireStream *CMatrixEffect::CreateFireStream(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1) {
    DTRACE();
    CMatrixEffectFireStream *e = HNew(m_Heap) CMatrixEffectFireStream(pos0, pos1);
    return e;
}

void CMatrixEffect::DrawBegin(void) {
    DTRACE();

    float fBias = -1.0f;
    for (int i = 0; i < 4; i++) {
        ASSERT_DX(g_D3DD->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&(fBias)))));
    }

    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
    ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
    ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));

    // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE,	FALSE));

    // DCPO_START();
}

void CMatrixEffect::DrawEnd(void) {
    DTRACE();
    // ASSERT_DX(g_D3DD->SetRenderState( D3DRS_ALPHABLENDENABLE,   FALSE ));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));

    // restore view matrix

    // ASSERT_DX(g_D3DD->SetTransform(D3DTS_VIEW,&g_MatrixMap->GetViewMatrix()));

    m_before_draw_done = 0;

    //#ifdef _TRACE
    //    int pri[16];
    //    int pric[16];
    //    memset(pri,-1,sizeof(pri));
    //    memset(pric,0,sizeof(pri));
    //
    //    CMatrixEffectLandscapeSpot *ls = (CMatrixEffectLandscapeSpot *)ELIST_FIRST(EFFECT_LANDSCAPE_SPOT);
    //    for (;ls;ls = (CMatrixEffectLandscapeSpot *)ls->m_TypeNext)
    //    {
    //        int pr = ls->Priority();
    //        for (int i=0;i<16;++i)
    //        {
    //            if (pri[i] == -1 || pri[i] == pr)
    //            {
    //                pri[i] = pr;
    //                ++pric[i];
    //                break;
    //            }
    //        }
    //    }
    //
    //    for (int i=0; i<16;++i)
    //    {
    //        if (pri[i] == -1) break;
    //        g_MatrixMap->m_DI.T(L"ls:" + std::wstring(pri[i]), std::wstring(pric[i]), 1000);
    //
    //
    //    }
    //
    //#endif
}
