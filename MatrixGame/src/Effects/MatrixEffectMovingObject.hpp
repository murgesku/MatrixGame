// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

//#define MAX_EFFECTS_COUNT   512

#define MISSILE_FIRE_PERIOD 20
#define BOMB_FIRE_PERIOD    15
#define GUN_FIRE_PERIOD     10

#define HOMING_RADIUS 1000

#define MISSILE_IMPACT_RADIUS    18
#define MISSILE_IMPACT_RADIUS_SQ (MISSILE_IMPACT_RADIUS * MISSILE_IMPACT_RADIUS)
#define HM_SEEK_TIME_PERIOD      50

#define BOMB_DAMAGE_RADIUS 20
#define GUN_DAMAGE_RADIUS  5

// moving object

struct SObjectCore;
struct SMOProps;
typedef void (*MO_MOVE_HANDLER)(D3DXMATRIX &m, SMOProps &pops,
                                float takt);  // function to build matrix for moving object
struct SMOProps {
    SMOProps(void) { memset(this, 0, sizeof(SMOProps)); }

    D3DXVECTOR3 startpos;
    D3DXVECTOR3 curpos;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 target;
    MO_MOVE_HANDLER handler;

    CVectorObjectAnim *object;

    // runtime members!
    float time;      // its automaticaly increases
    int endoflife;   // set this flag to initiate death of object
    float distance;  // temporary. use it for your own purpose. initialy it is and distance between start and end pos
    FIRE_END_HANDLER endhandler;
    DWORD uservalue;
    // SObjectCore        *attacker;  // attacker    // ха, вроде не нужно :)
    int side;  // side of attacker (if it dead)
    union {
        DWORD hitmask;
        EBuoyType buoytype;
    };

    CMatrixMapStatic *skip;

    SEffectHandler *shleif;
    union {
        struct {
            float next_fire_time;
            CTrajectory *trajectory;
            float pos;

        } bomb;
        struct {
            float next_fire_time;
            float next_seek_time;
            SObjectCore *target;
            int in_frustum_count;

        } hm;
        struct {
            float maxdist;
            float dist;
            int in_frustum_count;

        } gun;
        struct {
            float angle;

        } buoy;

    } common;
};

class CMatrixEffectMovingObject : public CMatrixEffect {
    SMOProps m_Props;
    D3DXMATRIX m_Mat;

    CMatrixEffectMovingObject(const SMOProps &props, DWORD hitmask, CMatrixMapStatic *skip, FIRE_END_HANDLER handler,
                              DWORD user);
    virtual ~CMatrixEffectMovingObject();

public:
    friend class CMatrixEffect;
    friend class CMatrixEffectBuoy;
    friend class CMatrixEffectMoveto;

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);
    virtual int Priority(void) { return MAX_EFFECT_PRIORITY; };
};

class CMatrixEffectBuoy : public CMatrixEffectMovingObject {
    CMatrixEffectBuoy(const SMOProps &props)
      : CMatrixEffectMovingObject(props, 0, 0, 0, 0), m_Kill(false), m_KillTime(1000),
#ifdef _DEBUG
        m_Light(DEBUG_CALL_INFO)
#else
        m_Light()
#endif

    {
        m_Props.buoytype = props.buoytype;  // m_Z = props.curpos.z;
        m_BuoyColor = 0x00FF0000;
        if (m_Props.buoytype == BUOY_BLUE) {
            m_BuoyColor = 0x000000FF;
        }
        else if (m_Props.buoytype == BUOY_GREEN) {
            m_BuoyColor = 0x0000FF00;
        }
        Takt(0);
    }

    bool m_Kill;
    float m_KillTime;
    DWORD m_BuoyColor;

    SEffectHandler m_Light;
    // float   m_Z;
public:
    friend class CMatrixEffect;

    virtual void Draw(void);
    virtual void Takt(float);

    void Kill(void);
};

void MO_Homing_Missile_Takt(D3DXMATRIX &m, SMOProps &pops, float takt);
void MO_Bomb_Takt(D3DXMATRIX &m, SMOProps &pops, float takt);
void MO_Gun_Takt(D3DXMATRIX &m, SMOProps &pops, float takt);
void MO_Gun_cannon_Takt(D3DXMATRIX &m, SMOProps &pops, float takt);
