// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once


// flame for flamethrower

#define FLAME_NUM_BILLS     10

#define FLAME_SCALE_FACTOR  3.6f //2.16f
//#define FLAME_DIST_FACTOR   0.3f
#define FLAME_DIR_SPEED     0.36f

#define FLAME_TIME_SEEK_PERIOD  20

struct SFlameBill
{
    CBillboard  m_Flame;
    float       m_Sign;
    ~SFlameBill()
    {
        m_Flame.Release();
    }
};

class CMatrixEffectFlame;

class CFlamePuff : public CMain
{
    CMatrixEffectFlame *m_Owner;

    SEffectHandler m_Light;

    float    m_Alpha;
    int      m_CurAlpha;

    SFlameBill  m_Flames[FLAME_NUM_BILLS];

    float m_Time;
    float m_NextSmokeTime;
    float m_NextSeekTime;

    D3DXVECTOR3 m_Pos;
    D3DXVECTOR3 m_Dir;
    D3DXVECTOR3 m_Speed;

    float       m_Scale;


    CBillboardLine *m_Shleif;
    DWORD           m_Break;

    CFlamePuff *m_Next;
    CFlamePuff *m_Prev;

    CFlamePuff(CMatrixEffectFlame *owner, const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &speed);
	~CFlamePuff();

public:
    friend class CMatrixEffectFlame;

    void Draw(void);
    void Takt(float step);
    void Release(void);

};

class CMatrixEffectWeapon;

class CMatrixEffectFlame : public CMatrixEffect
{

    CFlamePuff * m_First;
    CFlamePuff * m_Last;

    CFlamePuff * m_Stream;
    
    DWORD m_User;
    FIRE_END_HANDLER m_Handler;
    DWORD   m_hitmask;
    CMatrixMapStatic * m_skip;

    float     m_TTL;
    float   _m_TTL;

    SEffectHandler m_Smokes;

    CMatrixEffectFlame(float ttl, DWORD hitmask, CMatrixMapStatic * skip, DWORD user, FIRE_END_HANDLER handler);
	virtual ~CMatrixEffectFlame();

    void CreateSmoke(const D3DXVECTOR3 &pos);

public:

    friend class CMatrixEffect;
    friend class CFlamePuff;

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);

    virtual int  Priority(void) {return MAX_EFFECT_PRIORITY;};


    void AddPuff(const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &speed);
    void SubPuff(CFlamePuff *puf);
    void Break(void) {m_Stream = NULL;};

};