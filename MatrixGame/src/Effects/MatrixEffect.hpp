// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#define MAX_EFFECTS_COUNT   1280
#define MAX_EFFECT_PRIORITY 10000

#include "VectorObject.hpp"
#include "CBillboard.hpp"

// #include "MatrixEffectWeapon.hpp"
enum EWeapon : unsigned int;

#define MAX_EFFECT_DISTANCE_SQ ((3000) * (3000))

// billboards

enum EBillboardTextureSort {
    BBT_SMOKE,
    BBT_SMOKE_INTENSE,
    BBT_FIRE,
    BBT_FIRE_INTENSE,
    BBT_POINTLIGHT,
    BBT_LASEREND,
    BBT_SELDOT,
    BBT_PATHDOT,
    BBT_GLOWBEAMEND,
    BBT_FLAME,
    BBT_PLASMA1,
    BBT_PLASMA2,
    BBT_INTENSE,
    BBT_LASER,
    BBT_GUNFIRE1,
    BBT_GUNFIRE2,
    BBT_GUNFIRE3,
    BBT_TRASSA,
    BBT_SHLEIF,
    BBT_FIRESTREAM1,
    BBT_FIRESTREAM2,
    BBT_FIRESTREAM3,
    BBT_FIRESTREAM4,
    BBT_FIRESTREAM5,
    BBT_GLOWBEAM,
    BBT_FLAMELINE,
    BBT_SPARK,
    BBT_EFIELD,
    BBT_FLAMEFRAME0,
    BBT_FLAMEFRAME1,
    BBT_FLAMEFRAME2,
    BBT_FLAMEFRAME3,
    BBT_FLAMEFRAME4,
    BBT_FLAMEFRAME5,
    BBT_FLAMEFRAME6,
    BBT_FLAMEFRAME7,

    BBT_REPGLOWEND,
    BBT_REPGLOW,

    BBT_SCOREPLUS,
    BBT_SCORE0,
    BBT_SCORE1,
    BBT_SCORE2,
    BBT_SCORE3,
    BBT_SCORE4,
    BBT_SCORE5,
    BBT_SCORE6,
    BBT_SCORE7,
    BBT_SCORE8,
    BBT_SCORE9,
    BBT_SCOREICON1,
    BBT_SCOREICON2,
    BBT_SCOREICON3,
    BBT_SCOREICON4,
    BBT_SCOREICONS,

    BBT_LAST,
    EBillboardTextureSort_FORCE_DWORD = 0x7FFFFFFF
};

struct SBillboardID2Texname {
    EBillboardTextureSort id;
    const wchar *name;
};

#define BBT_FLAG_INTENSE SETBIT(0)

struct SBillboardTextureArrayElement {
    DWORD flags;
    union {
        SBillboardTexture bbt;  // for sorted
        CTextureManaged *tex;   // for intense
    };

    bool IsIntense(void) const { return FLAG(flags, BBT_FLAG_INTENSE); };
};

// classes

class CDebris : public CVectorObjectAnim {
    int m_DebrisType;

public:
    void SetDebType(int type) { m_DebrisType = type; };
    int GetDebType(void) { return m_DebrisType; }
};

class CMatrixEffectExplosion;
class CMatrixEffectFire;
class CMatrixEffectFirePlasma;
class CMatrixEffectMovingObject;
class CMatrixEffectPointLight;
class CMatrixEffectLandscapeSpot;
class CMatrixMapStatic;
class CMatrixEffectBillboard;
class CMatrixEffectFireStream;
class CMatrixEffectFlame;
class CMatrixEffectFireAnim;

struct SExplosionProperties;
struct SMOProps;

typedef void (*ADD_TAKT)(CMatrixEffectBillboard *bb, float ms);

#define FEHF_LASTHIT      SETBIT(0)
#define FEHF_DAMAGE_ROBOT SETBIT(1)

typedef void (*FIRE_END_HANDLER)(CMatrixMapStatic *hit, const D3DXVECTOR3 &pos, DWORD user, DWORD flags);

#define SMOKE_SPEED (1.0f / 25.0f)
// fire
#define FIRE_SPEED (0.5f / 25.0f)

// selection
#define SEL_COLOR_DEFAULT 0xFF00FF00
#define SEL_COLOR_TMP     0xFF305010
enum EBuoyType {
    BUOY_RED,
    BUOY_GREEN,
    BUOY_BLUE,

    EBuoyType_FORCE_DWORD = 0x7FFFFFFF
};

enum ESpotType {
    SPOT_CONSTANT = 0,
    SPOT_SELECTION = 1,
    SPOT_PLASMA_HIT = 2,
    // SPOT_MOVE_TO = 3,
    SPOT_POINTLIGHT = 4,
    SPOT_VORONKA = 5,
    SPOT_TURRET = 6,
    SPOT_SOLE_TRACK = 7,      // следы гусениц
    SPOT_SOLE_WHEEL = 8,      // следы колес
    SPOT_SOLE_PNEUMATIC = 9,  // следы ног

    SPOT_TYPES_CNT,
    ESpotType_FORCE_DWORD = 0x7FFFFFFF
};

typedef void (*SPOT_TAKT_FUNC)(CMatrixEffectLandscapeSpot *spot, float ms);

#define LSFLAG_INTENSE         SETBIT(0)
#define LSFLAG_SCALE_BY_NORMAL SETBIT(1)

typedef struct {
    SPOT_TAKT_FUNC func;
    CTextureManaged *texture;
    DWORD color;
    float ttl;
    DWORD flags;

} SSpotProperties;

#define MAX_EFFECTS_EXPLOSIONS   50
#define MAX_EFFECTS_POINT_LIGHTS 50
#define MAX_EFFECTS_SMOKEFIRES   100
#define MAX_EFFECTS_LANDSPOTS    100
#define MAX_EFFECTS_FIREANIM     50

// common
enum EEffectType {
    /* 0  */ EFFECT_UNDEFINED,
    /* 1  */ EFFECT_EXPLOSION,
    /* 2  */ EFFECT_SMOKE,
    /* 3  */ EFFECT_SMOKE_INTENSE,
    /* 4  */ EFFECT_FIRE_PLASMA,
    /* 5  */ EFFECT_LANDSCAPE_SPOT,
    /* 6  */ EFFECT_FIRE,
    /* 7  */ EFFECT_FIRE_INTENSE,
    /* 8  */ EFFECT_MOVING_OBJECT,
    /* 9  */ EFFECT_SELECTION,
    /* 10 */ EFFECT_SELECTION_LAND,
    /* 11 */ EFFECT_MOVETO,
    /* 12 */ EFFECT_PATH,
    /* 13 */ EFFECT_ZAHVAT,
    /* 14 */ EFFECT_ELEVATORFIELD,
    /* 15 */ EFFECT_POINT_LIGHT,
    /* 16 */ EFFECT_KONUS,
    /* 17 */ EFFECT_BILLBOARD,
    /* 18 */ EFFECT_BILLBOARDLINE,
    /* 19 */ EFFECT_FLAME,
    /* 20 */ EFFECT_BIG_BOOM,
    /* 21 */ EFFECT_LIGHTENING,
    /* 22 */ EFFECT_SHORTED,
    /* 23 */ EFFECT_LASER,
    /* 24 */ EFFECT_VOLCANO,
    /* 25 */ EFFECT_FIRESTREAM,
    /* 26 */ EFFECT_DUST,
    /* 27 */ EFFECT_SMOKESHLEIF,
    /* 28 */ EFFECT_FIREANIM,
    /* 29 */ EFFECT_SCORE,
    /* 30 */ EFFECT_REPAIR,
    /* 31 */ EFFECT_WEAPON,
    /* 32 */ EFFECT_SPLASH,

    EFFECT_TYPE_COUNT,
    EEffectType_FORCE_DWORD = 0x7FFFFFFF

};

// before draw done flags
#define BDDF_EXPLOSION SETBIT(0)
#define BDDF_DUST      SETBIT(1)
//#define BDDF_FIRE       SETBIT(2)
//#define BDDF_SMOKE      SETBIT(3)
//#define BDDF_PLASMA     SETBIT(4)
//#define BDDF_FLAME      SETBIT(5)
//#define BDDF_LIGHTENING SETBIT(6)
#define BDDF_PLIGHT SETBIT(7)
//#define BDDF_FIRESTREAM SETBIT(8)

// m_Flags

#define EFFF_DIP SETBIT(0)  // effect is under destruction

// only weapon flags
#define WEAPFLAGS_FIRE     SETBIT(1)
#define WEAPFLAGS_FIREWAS  SETBIT(2)
#define WEAPFLAGS_HITWAS   SETBIT(3)  // only for instant weapons (LASER, VOLCANO, LIGHTENING)
#define WEAPFLAGS_SND_OFF  SETBIT(4)  // выключать при FireEnd (использовать на зацикленных звуках (Laser, lightening))
#define WEAPFLAGS_SND_SKIP SETBIT(5)  // если звук уже звучит, пропускать (volcano)

// only repair flags
#define ERF_LOST_TARGET  SETBIT(1)
#define ERF_OFF_TARGET   SETBIT(2)
#define ERF_FOUND_TARGET SETBIT(3)
#define ERF_WITH_TARGET  SETBIT(4)
#define ERF_MAX_AMP      SETBIT(5)

// only PointLight flags
#define PLF_KIP SETBIT(1)

// only Selection flags
#define SELF_KIP SETBIT(1)

// only BigBoom flags
#define BIGBOOMF_PREPARED SETBIT(1)

// only MoveTo flags
#define MOVETOF_PREPARED SETBIT(1)

typedef struct {
    float c;
    float t;
} SGradient;

inline float CalcGradient(float t, const SGradient *grad) {
    int i = 1;
    for (;; ++i) {
        if (t >= grad[i].t) {
            continue;
        }
        return (grad[i].c - grad[i - 1].c) * (t - grad[i - 1].t) / (grad[i].t - grad[i - 1].t) + grad[i - 1].c;
    }
}

class CMatrixEffect;

struct SEffectHandler {
#ifdef _DEBUG
    SDebugCallInfo from;
#endif
    CMatrixEffect *effect;

#ifdef _DEBUG
    SEffectHandler(void) : from("", -1), effect(NULL) {}
    SEffectHandler(const SDebugCallInfo &f) : from(f), effect(NULL) {}
    ~SEffectHandler() { Release(from); }
    void Release(const SDebugCallInfo &from);
#else
    SEffectHandler(void) : effect(NULL) {}
    ~SEffectHandler() { Release(); }
    void Release(void);
#endif
    void Unconnect(void);
    void Rebase(void);

#ifdef _DEBUG
    SEffectHandler &operator=(const SEffectHandler &h)
    {
        debugbreak();
        return *this;
    }
#endif
};

#define ELIST_NAME(n, x) m_EffectsList##n##x
#define ELIST_FIRST(n)   ELIST_NAME(n, _First)
#define ELIST_LAST(n)    ELIST_NAME(n, _Last)
#define ELIST_CNT(n)     ELIST_NAME(n, _Count)
#define ELIST_DECLARE_INCLASS(n)          \
    static CMatrixEffect *ELIST_FIRST(n); \
    static CMatrixEffect *ELIST_LAST(n);  \
    static int ELIST_CNT(n)
#define ELIST_DECLARE_OUTCLASS(n)                 \
    CMatrixEffect *CMatrixEffect::ELIST_FIRST(n); \
    CMatrixEffect *CMatrixEffect::ELIST_LAST(n);  \
    int CMatrixEffect::ELIST_CNT(n)
#define ELIST_INIT(n)                     \
    CMatrixEffect::ELIST_FIRST(n) = NULL; \
    CMatrixEffect::ELIST_LAST(n) = NULL;  \
    CMatrixEffect::ELIST_CNT(n) = 0
#define ELIST_ADD(n)                                                       \
    LIST_ADD(this, ELIST_FIRST(n), ELIST_LAST(n), m_TypePrev, m_TypeNext); \
    ++ELIST_CNT(n)
#define ELIST_DEL(n)                                                       \
    LIST_DEL(this, ELIST_FIRST(n), ELIST_LAST(n), m_TypePrev, m_TypeNext); \
    --ELIST_CNT(n)

#ifdef _DEBUG
#define ELSIT_REMOVE_OLDEST(n) \
    { g_MatrixMap->SubEffect(DEBUG_CALL_INFO, ELIST_FIRST(n)); }
#define ELSIT_REMOVE_OLDEST_PRIORITY(n, p)             \
    {                                                  \
        CMatrixEffect *ef = ELIST_FIRST(n);            \
        CMatrixEffect *ef2d = ef;                      \
        for (; ef; ef = ef->m_TypeNext) {              \
            if (ef->Priority() < (p)) {                \
                ef2d = ef;                             \
                break;                                 \
            }                                          \
        }                                              \
        {                                              \
            if (ef2d == ELIST_LAST(n)) {               \
                ef2d = ELIST_FIRST(n);                 \
            }                                          \
        }                                              \
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, ef2d); \
    }
#else
#define ELSIT_REMOVE_OLDEST(n) \
    { g_MatrixMap->SubEffect(ELIST_FIRST(n)); }
#define ELSIT_REMOVE_OLDEST_PRIORITY(n, p)  \
    {                                       \
        CMatrixEffect *ef = ELIST_FIRST(n); \
        CMatrixEffect *ef2d = ef;           \
        for (; ef; ef = ef->m_TypeNext) {   \
            if (ef->Priority() < (p)) {     \
                ef2d = ef;                  \
                break;                      \
            }                               \
        }                                   \
        {                                   \
            if (ef2d == ELIST_LAST(n)) {    \
                ef2d = ELIST_FIRST(n);      \
            }                               \
        }                                   \
        g_MatrixMap->SubEffect(ef2d);       \
    }
#endif

class CMatrixEffect : public CMain {
protected:
    // DPTR_MEM_SIGNATURE_DECLARE();

    EEffectType m_EffectType;
    SEffectHandler *m_EffectHandler;
    DWORD m_Flags;

    CMatrixEffect *m_TypePrev;
    CMatrixEffect *m_TypeNext;

    static DWORD m_before_draw_done;
    static CDebris *m_Debris;
    static int m_DebrisCnt;
    static SSpotProperties m_SpotProperties[SPOT_TYPES_CNT];
    static SBillboardTextureArrayElement m_BBTextures[BBT_LAST];

    ELIST_DECLARE_INCLASS(EFFECT_EXPLOSION);
    ELIST_DECLARE_INCLASS(EFFECT_POINT_LIGHT);
    ELIST_DECLARE_INCLASS(EFFECT_SMOKE);
    ELIST_DECLARE_INCLASS(EFFECT_LANDSCAPE_SPOT);
    ELIST_DECLARE_INCLASS(EFFECT_MOVETO);
    ELIST_DECLARE_INCLASS(EFFECT_FIREANIM);

    void SetDIP(void) { SETFLAG(m_Flags, EFFF_DIP); }

    CMatrixEffect(void)
      : CMain(), m_EffectHandler(NULL), m_TypePrev(NULL), m_TypeNext(NULL), m_Prev(NULL), m_Next(NULL), m_Flags(0)
#ifdef _DEBUG
        ,
        m_EffectType(EFFECT_UNDEFINED)
#endif
    {
        // DPTR_MEM_SIGNATURE_INIT(sizeof(CMatrixEffect));
        DCS_INCONSTRUCTOR();
    };
    ~CMatrixEffect() {
        DTRACE();
        if (m_EffectHandler)
            m_EffectHandler->effect = NULL;
    };

public:
    static Base::CHeap *m_Heap;
    CMatrixEffect *m_Prev;
    CMatrixEffect *m_Next;
    static float m_Dist2;  // used by movin objects (missiles cannons)

    friend class CMatrixEffectExplosion;
    friend class CMatrixEffectSmoke;
    friend class CMatrixEffectFire;
    friend class CMatrixEffectFirePlasma;
    friend class CMatrixEffectLandscapeSpot;
    friend class CMatrixEffectPointLight;
    friend class CMatrixEffectMoveto;
    friend class CMatrixEffectFireAnim;

    void SetHandler(SEffectHandler *eh) { m_EffectHandler = eh; }
    SEffectHandler *GetHandler(void) { return m_EffectHandler; }
    void Unconnect(void) {
        if (m_EffectHandler)
            m_EffectHandler->effect = NULL;
    }
    EEffectType GetType(void) { return m_EffectType; }

    static void CreatePoolDefaultResources(void);
    static void ReleasePoolDefaultResources(void);

    static void InitEffects(CBlockPar &bp);
    static void ClearEffects(void);

    static const SBillboardTexture *GetBBTex(EBillboardTextureSort t) { return &m_BBTextures[t].bbt; };
    static CTextureManaged *GetBBTexI(EBillboardTextureSort t) { return m_BBTextures[t].tex; };

    static void CreateExplosion(const D3DXVECTOR3 &pos, const SExplosionProperties &props,
                                bool fire = false);  // automaticaly adds to Effects list; can return NULL
    static void CreateSmoke(SEffectHandler *eh, const D3DXVECTOR3 &pos, float ttl, float puffttl, float spawntime,
                            DWORD color, bool intense = false,
                            float speed = SMOKE_SPEED);  // automaticaly adds to Effects list; can return NULL
    static void CreateFire(SEffectHandler *eh, const D3DXVECTOR3 &pos, float ttl, float puffttl, float spawntime,
                           float dispfactor, bool intense = true,
                           float speed = FIRE_SPEED);  // automaticaly adds to Effects list; can return NULL
    static void CreateFireAnim(SEffectHandler *eh, const D3DXVECTOR3 &pos, float w, float h,
                               int ttl);  // automaticaly adds to Effects list; can return NULL
    static void CreateFirePlasma(const D3DXVECTOR3 &start, const D3DXVECTOR3 &end, float speed, DWORD hitmask,
                                 CMatrixMapStatic *skip, FIRE_END_HANDLER handler,
                                 DWORD user);  // automaticaly adds to Effects list; can return NULL
    static void CreateLandscapeSpot(
            SEffectHandler *eh, const D3DXVECTOR2 &pos, float angle, float scale,
            ESpotType type = SPOT_CONSTANT);  // automaticaly adds to Effects list; can return NULL
    static void CreateMovingObject(SEffectHandler *eh, const SMOProps &props, DWORD hitmask, CMatrixMapStatic *skip,
                                   FIRE_END_HANDLER = NULL,
                                   DWORD user = 0);  // automaticaly adds to Effects list; can return NULL
    static void CreateBuoy(SEffectHandler *eh, const D3DXVECTOR3 &pos, EBuoyType bt);
    static void CreateMoveto(void);
    static void CreateMoveto(const D3DXVECTOR3 &pos);
    static void DeleteAllMoveto(void);
    static CMatrixEffect *CreateSelection(const D3DXVECTOR3 &pos, float r, DWORD color = SEL_COLOR_DEFAULT);
    static CMatrixEffect *CreatePath(const D3DXVECTOR3 *pos, int cnt);
    static void CreatePointLight(SEffectHandler *eh, const D3DXVECTOR3 &pos, float r, DWORD color,
                                 bool drawbill = false);  // automaticaly adds to Effects list; can return NULL
    static void CreateKonus(SEffectHandler *eh, const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir, float radius,
                            float height, float angle, float ttl, bool intense, CTextureManaged *tex = NULL);
    static void CreateKonusSplash(const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir, float radius, float height,
                                  float angle, float ttl, bool intense, CTextureManaged *tex = NULL);
    static CMatrixEffect *CreateWeapon(const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir, DWORD user,
                                       FIRE_END_HANDLER handler, EWeapon type, int cooldown = 0);
    static void CreateFlame(SEffectHandler *eh, float ttl, DWORD hitmask, CMatrixMapStatic *skip, DWORD user,
                            FIRE_END_HANDLER handler);
    static void CreateBigBoom(const D3DXVECTOR3 &pos, float radius, float ttl, DWORD hitmask, CMatrixMapStatic *skip,
                              DWORD user, FIRE_END_HANDLER handler,
                              DWORD light = 0);  // automaticaly adds to Effects list; can return NULL
    static void CreateLightening(SEffectHandler *eh, const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float ttl,
                                 float dispers, float width,
                                 DWORD color = 0xFFFFFFFF);  // automaticaly adds to Effects list; can return NULL
    static void CreateShorted(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float ttl,
                              DWORD color = 0xFFFFFFFF);  // automaticaly adds to Effects list; can return NULL
    static void CreateBillboard(SEffectHandler *eh, const D3DXVECTOR3 &pos, float radius1, float radius2, DWORD c1,
                                DWORD c2, float ttl, float delay, const wchar *tex, const D3DXVECTOR3 &dir,
                                ADD_TAKT addtakt = NULL);  // automaticaly adds to Effects list; can return NULL
    static void CreateBillboardScore(const wchar *n, const D3DXVECTOR3 &pos, DWORD color);
    static void CreateBillboardLine(SEffectHandler *eh, const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float width,
                                    DWORD c1, DWORD c2, float ttl,
                                    CTextureManaged *tex);  // automaticaly adds to Effects list; can return NULL
    static CMatrixEffect *CreateZahvat(const D3DXVECTOR3 &pos, float radius, float angle, int cnt);
    static CMatrixEffect *CreateElevatorField(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float radius,
                                              const D3DXVECTOR3 &fwd);
    static void CreateDust(SEffectHandler *eh, const D3DXVECTOR2 &pos, const D3DXVECTOR2 &adddir,
                           float ttl);  // automaticaly adds to Effects list; can return NULL
    static CMatrixEffectFireStream *CreateFireStream(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1);
    static void CreateShleif(SEffectHandler *eh);
    static CMatrixEffect *CreateRepair(const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, float seekradius,
                                       CMatrixMapStatic *skip);

    static void DrawBegin(void);
    static void DrawEnd(void);

    virtual void BeforeDraw(void) = 0;
    virtual void Draw(void) = 0;
    virtual void Takt(float step) = 0;
    virtual void Release(void) = 0;

    virtual int Priority(void) = 0;  // priority of effect

    bool IsDIP(void) const { return FLAG(m_Flags, EFFF_DIP); }
};
typedef CMatrixEffect *PCMatrixEffect;

inline void SEffectHandler::Rebase(void) {
    if (effect)
        effect->SetHandler(this);
}

inline void SEffectHandler::Unconnect(void) {
    DTRACE();
#ifdef _DEBUG
    ASSERT(effect);
    if (effect->GetHandler() == this) {
        effect->SetHandler(NULL);
    }
    else {
        debugbreak();
    }
    effect = NULL;
#else
    if (effect) {
        effect->SetHandler(NULL);
        effect = NULL;
    }
#endif
}

enum EEffectSpawnerType {
    EST_SMOKE = 1,
    EST_FIRE = 2,
    EST_SOUND = 3,
    EST_LIGHTENING = 4,

    EEffectSpawnerType_FORCE_DWORD = 0x7FFFFFFF
};

struct SpawnEffectProps;

void EffectSpawnerFire(SpawnEffectProps *props);
void EffectSpawnerSmoke(SpawnEffectProps *props);
void EffectSpawnerSound(SpawnEffectProps *props);
void EffectSpawnerLightening(SpawnEffectProps *props);

void SpawnEffectLighteningDestructor(SpawnEffectProps *props);

struct SpawnEffectProps {
    typedef void (*SEP_DESTRUCTOR)(SpawnEffectProps *props);
    typedef void (*EFFECT_SPAWNER)(SpawnEffectProps *props);

    int m_Size;          // structure size
    EEffectType m_Type;  // effect type
    D3DXVECTOR3 m_Pos;
    SEP_DESTRUCTOR m_Destructor;
    EFFECT_SPAWNER m_Spawner;
};

struct SpawnEffectSmoke : public SpawnEffectProps {
    SpawnEffectSmoke(void) {
        memset(this, 0, sizeof(SpawnEffectSmoke));
        m_Size = sizeof(SpawnEffectSmoke);
        m_Spawner = EffectSpawnerSmoke;
    }
    float m_ttl;
    float m_puffttl;
    float m_spawntime;
    DWORD m_color;
    float m_speed;
    bool m_intense;
};

struct SpawnEffectFire : public SpawnEffectProps {
    SpawnEffectFire(void) {
        memset(this, 0, sizeof(SpawnEffectFire));
        m_Size = sizeof(SpawnEffectFire);
        m_Spawner = EffectSpawnerFire;
    }
    float m_ttl;
    float m_puffttl;
    float m_spawntime;
    float m_dispfactor;
    float m_speed;
    bool m_intense;
};

struct SpawnEffectSound : public SpawnEffectProps {
    SpawnEffectSound(void) {
        memset(this, 0, sizeof(SpawnEffectSound));
        m_Size = sizeof(SpawnEffectSound);
        m_Spawner = EffectSpawnerSound;
    }
    float m_pan0, m_pan1;
    float m_vol0, m_vol1;
    float m_attn;
    wchar m_name[32];
    // bool        m_looped;
};

struct SpawnEffectLightening : public SpawnEffectProps {
#ifdef _DEBUG
    SpawnEffectLightening()
      : m_Effect(DEBUG_CALL_INFO)
#else
    SpawnEffectLightening(void)
#endif

    {
        memset(this, 0, sizeof(SpawnEffectLightening));
        m_Size = sizeof(SpawnEffectLightening);
        m_Spawner = EffectSpawnerLightening;
        m_Destructor = SpawnEffectLighteningDestructor;
    }

    union {
        SpawnEffectLightening *m_Pair;
        std::wstring *m_Tag;
    };

    float m_ttl;
    float m_Width;
    float m_Dispers;  // if < 0 then non host effect. not produced
    DWORD m_Color;

    SEffectHandler m_Effect;
};

class CEffectSpawner : public CMain {
    SpawnEffectProps *m_Props;
    int m_NextTime;
    int m_TTL;
    int m_MIN_period;
    int m_MAX_period;

    CMatrixMapStatic *m_Under;

public:
    CEffectSpawner(int minperiod, int maxperiod, int ttl, SpawnEffectProps *props);
    ~CEffectSpawner();

    SpawnEffectProps *Props(void) { return m_Props; }

    CMatrixMapStatic *GetUnder(void) { return m_Under; };

    bool OutOfTime(void) const;

    void Takt(float ms);
};

typedef CEffectSpawner *PCEffectSpawner;

// #include "MatrixEffectExplosion.hpp"
// #include "MatrixEffectSmokeAndFire.hpp"

// #include "MatrixEffectFirePlasma.hpp"
// #include "MatrixEffectLandscapeSpot.hpp"
// #include "MatrixEffectMovingObject.hpp"

// #include "MatrixEffectSelection.hpp"
// #include "MatrixEffectPath.hpp"
// #include "MatrixEffectPointLight.hpp"

// #include "MatrixEffectKonus.hpp"
// #include "MatrixEffectFlame.hpp"
// #include "MatrixEffectBigBoom.hpp"

// #include "MatrixEffectLightening.hpp"
// #include "MatrixEffectBillboard.hpp"
// #include "MatrixEffectZahvat.hpp"

// #include "MatrixEffectElevatorField.hpp"
// #include "MatrixEffectDust.hpp"
// #include "MatrixEffectMoveTo.hpp"

// #include "MatrixEffectShleif.hpp"
// #include "MatrixEffectRepair.hpp"
// #include "MatrixEffectWeapon.hpp"
