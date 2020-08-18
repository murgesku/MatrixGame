// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixEffect.hpp"

// smoke

#define MAX_PUFF_COUNT      256
#define PUFF_SCALE1  4.0f
#define PUFF_SCALE2  16.0f

#define PUFF_FIRE_SCALE  3.5f


#define  FIRE_STREAM_WIDTH  5

#define  FIREFRAME_ANIM_PERIOD  100
#define  FIREFRAME_TTL_POROG    0.9f
#define  FIREFRAME_W_DEAD       0
#define  FIREFRAME_H_DEAD       0

//___________________________________________________________________________________________________
/////////////////////////////////////////////////////////////////////////////////////////////////////

struct SSmokePuff
{
    CBillboard    m_Puff;
    float         m_PuffAngle;
    float         m_PuffTTL;
    D3DXVECTOR3   m_PuffOrig;

};

struct SFirePuff : public SSmokePuff
{
    D3DXVECTOR2 m_PuffDir;
};


class CMatrixEffectSmoke : public CMatrixEffect
{
    SSmokePuff   *m_Puffs;
    int           m_PuffCnt;

    D3DXVECTOR3 m_Pos, m_Mins, m_Maxs;

    float    m_TTL;
    float    m_PuffTTL;
    float    m_Spawn;

    float    m_Time;
    float    m_NextSpawnTime;

    float    m_Speed;

    DWORD    m_Color;

    CMatrixEffectSmoke(const D3DXVECTOR3 &pos, float ttl, float puffttl, float spawntime, DWORD color, bool intense, float speed);
	virtual ~CMatrixEffectSmoke();

    void  SpawnPuff(void);

public:
    friend class CMatrixEffect;
    friend class CMatrixEffectFire;
    friend class CMatrixEffectExplosion;
    friend class CMatrixEffectShleif;

    void    SetPos(const D3DXVECTOR3 &pos) {m_Pos = pos;}
    void    SetTTL(float ttl) {m_TTL = ttl;}

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);
    virtual int  Priority(void) {return 30;};


};

class CMatrixEffectFire : public CMatrixEffect
{
    SFirePuff    *m_Puffs;
    int           m_PuffCnt;

    D3DXVECTOR3 m_Pos, m_Mins, m_Maxs;

    float    m_TTL;
    float    m_PuffTTL;
    float    m_Spawn;

    float    m_Time;
    float    m_NextSpawnTime;

    float    m_Speed;
    float m_DispFactor;

    CMatrixEffectFire(const D3DXVECTOR3 &pos, float ttl, float puffttl, float spawntime, float dispfactor, bool intense,  float speed = SMOKE_SPEED);
    

    void  SpawnPuff(void);

public:

    ~CMatrixEffectFire();

    friend class CMatrixEffect;
    friend class CMatrixEffectSmoke;
    friend class CMatrixEffectExplosion;
    friend class CMatrixEffectShleif;

    void    SetPos(const D3DXVECTOR3 &pos) {m_Pos = pos;}
    void    SetTTL(float ttl) {m_TTL = ttl;}

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void Takt(float step);
    virtual void Release(void);
    virtual int  Priority(void) {return 30;};
};

class CMatrixEffectFireStream : public CMatrixEffect
{
    CBillboardLine      m_bl1,m_bl2,m_bl3,m_bl4,m_bl5;

    virtual ~CMatrixEffectFireStream()
    {
        m_bl1.Release(); m_bl2.Release(); m_bl3.Release(); m_bl4.Release(); m_bl5.Release();
    }

public:
    CMatrixEffectFireStream(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1);

    virtual void BeforeDraw(void);
    virtual void Draw(void) {Draw(false);};
    void Draw(bool now);
    virtual void Takt(float) {};
    virtual void Release(void)
    {
        HDelete(CMatrixEffectFireStream, this, m_Heap);
    };

    virtual int  Priority(void) {return MAX_EFFECT_PRIORITY;};

    void SetPos(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1)
    {
        m_bl1.SetPos(pos0,pos1); m_bl2.SetPos(pos0,pos1); m_bl3.SetPos(pos0,pos1); m_bl4.SetPos(pos0,pos1); m_bl5.SetPos(pos0,pos1);
    }
};


class CMatrixEffectFireAnim : public CMatrixEffect
{
    CBillboardLine      m_bl[8];

    float m_TTLcurr;
    float m_TTL;
    int m_Frame;
    int m_NextTime;
    float   m_Height;
    float   m_Width;
    float   m_HeightCurr;
    float   m_WidthCurr;
    D3DXVECTOR3 m_Pos;

    virtual ~CMatrixEffectFireAnim()
    {
        ELIST_DEL(EFFECT_FIREANIM);

        for (int i=0; i<8;++i)
        {
            m_bl[i].Release();
        }
    }
    void Update(void)
    {
        for (int i=0; i<8;++i)
        {
            m_bl[i].SetPos(m_Pos,m_Pos + D3DXVECTOR3(0,0,m_HeightCurr));
            m_bl[i].SetWidth(m_WidthCurr);
        }

    }

public:
    CMatrixEffectFireAnim(const D3DXVECTOR3 &pos, float w, float h, int ttl);

    virtual void BeforeDraw(void);
    virtual void Draw(void) {Draw(false);};
    void Draw(bool now);
    virtual void Takt(float);
    virtual void Release(void)
    {
        HDelete(CMatrixEffectFireAnim, this, m_Heap);
    };

    virtual int  Priority(void) {return 10;};

    void SetPos(const D3DXVECTOR3 &pos)
    {
        m_Pos = pos;
        Update();
    }
    void SetWH(float w, float h)
    {
        m_WidthCurr = w; m_Width = w;
        m_HeightCurr = h; m_Height = h;
        Update();
    }
};
