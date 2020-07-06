// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixEffectSmokeAndFire.hpp"
#include "MatrixSoundManager.hpp"

// explosion


#define EXPLOSION_TIME_PERIOD   10
#define DEBRIS_SPEED        (1.0f/100.0f)
#define INTENSE_INIT_SIZE   4
#define INTENSE_END_SIZE    60
#define INTENSE_DISPLACE_RADIUS  10
#define INTENSE_TTL         1300

#define EXPLOSION_SPARK_WIDTH 3

struct SExplosionProperties
{
    float min_speed;
    float max_speed;
    float min_speed_z;
    float max_speed_z;

    int   sparks_deb_cnt;
    int   fire_deb_cnt;
    int   deb_cnt;
    int   deb_min_ttl;
    int   deb_max_ttl;

    int   intense_cnt;

    int   deb_type;

    DWORD  light;   // bool
    float light_radius1;
    float light_radius2;
    DWORD light_color1;
    DWORD light_color2;
    float   light_time1;
    float   light_time2;

    ESound  sound;

    ESpotType voronka;
    float     voronka_scale;
};

extern const SExplosionProperties ExplosionNormal;
extern const SExplosionProperties ExplosionMissile;
extern const SExplosionProperties ExplosionRobotHit;
extern const SExplosionProperties ExplosionRobotBoom;
extern const SExplosionProperties ExplosionRobotBoomSmall;
extern const SExplosionProperties ExplosionBigBoom;
extern const SExplosionProperties ExplosionLaserHit;
extern const SExplosionProperties ExplosionBuildingBoom;
extern const SExplosionProperties ExplosionBuildingBoom2;
extern const SExplosionProperties ExplosionObject;


enum EDebType
{
    DEB_FIRE = -1,
    DEB_INTENSE = -2,
    DEB_SPARK = -3,
    
#if defined _DEBUG || defined _TRACE
    DEB_DEAD = -4, // special flag
#endif

    EDebType_FORCE_DWORD = 0x7FFFFFFF
};

typedef struct _SDebris
{
    D3DXVECTOR3 v;
    union
    {
        int         index;        // if < 0 then fire
        EDebType    type;
    };
    float         ttl;    // time to live
    union
    {
        struct // debris
        {
            D3DXVECTOR3 pos;
            union
            {
                struct
                {
                    float       alpha;  // 1.0 - visible
                    float       scale;
                    float       angley;
                    float       anglep;
                    float       angler;
                };
                struct // sparks
                {
                    D3DXVECTOR3            prepos; // distance between prepos and pos cannot be larger then len
                    CBillboardLine        *bline;
                    //float                  len;
                    float                  unttl;
                };
            };
        };
        struct // intense
        {
            float                 ttm;
            CBillboard        *billboard;
        };
        struct // fire
        {
            SEffectHandler fire;
            SEffectHandler light;
        };

    };

} SDebris;


class CMatrixEffectExplosion : public CMatrixEffect
{

    SDebris *m_Deb;
    int      m_DebCnt;
    float    m_Time;

    const SExplosionProperties *m_Props;

    SEffectHandler  m_Light;

    CMatrixEffectExplosion(const D3DXVECTOR3 &pos, const SExplosionProperties &props);
	virtual ~CMatrixEffectExplosion();

    void RemoveDebris(int deb);
public:
    friend class CMatrixEffect;


    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);

    virtual int  Priority(void) {return m_DebCnt?MAX_EFFECT_PRIORITY:0;};

};


