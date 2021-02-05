// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "../stdafx.h"
#include "MatrixEffect.hpp"
#include "../MatrixMap.hpp"

//Параметры визуализации вейпоинта
#define MOVETO_TTL  400
#define GATHPNT_TTL 800
#define MOVETO_S    2
#define MOVETO_R    20
#define MOVETO_Z    10

void SPointMoveTo::Init(CMatrixEffectMoveto* host, int i)
{
    D3DXMatrixScaling(&m, MOVETO_S, MOVETO_S, MOVETO_S);

    Change(host, i, 1);
}

void SPointMoveTo::Change(CMatrixEffectMoveto* host, int i, float k)
{

    float a = M_PI_MUL((i & (~1)) * INVERT(3.0));

    float s, c, d, z;

    d = MOVETO_R * 0.3f * (float)sin(M_PI_MUL(k));
    z = (float)sin(M_PI_MUL(k / 2.0f));
    SET_SIGN_FLOAT(d, i & 1);

    SinCos(a, &s, &c);
    m._41 = MOVETO_R * s * k + d * c;
    m._42 = MOVETO_R * c * k - d * s;


    *(D3DXVECTOR2*)&m._41 += *(D3DXVECTOR2*)&host->m_Pos;

    float lz = g_MatrixMap->GetZ(m._41, m._42);
    if(lz < host->m_Pos.z) lz = host->m_Pos.z;

    m._43 = lz + MOVETO_Z * z;
}

void SPointMoveTo::Draw(CMatrixEffectMoveto *host)
{
    CMatrixEffectBillboard::Draw(m, 0xFFFFFFFF, host->m_Tex, false);
}

CTextureManaged*       CMatrixEffectMoveto::m_Tex;
int                    CMatrixEffectMoveto::m_RefCnt;

CMatrixEffectMoveto::CMatrixEffectMoveto(const D3DXVECTOR3& pos, int type) :CMatrixEffect(), m_Pos(pos)
{
    m_EffectType = EFFECT_MOVETO;

    for(int i = 0; i < 6; ++i)
    {
        m_Pts[i].Init(this, i);
    }

    if(type == 1)
    {
        m_TTL = MOVETO_TTL;
        if(!m_RefCnt)
        {
            //Подгружаем основную текстуру вейпоинта (зелёный кружок 32x32)
            m_Tex = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_MOVETO);
            m_Tex->RefInc();
        }
        ++m_RefCnt;
    }
    else
    {
        m_TTL = GATHPNT_TTL;
        if(!m_RefCnt)
        {
            //Подгружаем альтернативную текстуру вейпоинта для точки сбора
            m_Tex = (CTextureManaged*)g_Cache->Get(cc_TextureManaged, TEXTURE_PATH_GATHER_POINT);
            m_Tex->RefInc();
        }
        ++m_RefCnt;
    }

    CMatrixEffectBillboard::CreateGeometry();
    ELIST_ADD(EFFECT_MOVETO);
}

CMatrixEffectMoveto::~CMatrixEffectMoveto()
{
    ELIST_DEL(EFFECT_MOVETO);

    if((--m_RefCnt) <= 0)
    {
        m_Tex->RefDecUnload();
        m_Tex = NULL;
    }
    CMatrixEffectBillboard::ReleaseGeometry();
}

void CMatrixEffectMoveto::Takt(float step)
{
    DTRACE();
    m_TTL -= step;
    if(m_TTL < 0)
    {
#ifdef _DEBUG
        g_MatrixMap->SubEffect(DEBUG_CALL_INFO, this);
#else
        g_MatrixMap->SubEffect(this);
#endif
        return;
    }

    float k = m_TTL * INVERT(MOVETO_TTL);
    for(int i = 0; i < 6; ++i)
    {
        m_Pts[i].Change(this, i, k);
    }
}

void CMatrixEffectMoveto::Draw(void)
{
    DTRACE();

    if(!IS_VB(CMatrixEffectBillboard::m_VB)) return;

    RESETFLAG(m_Flags, MOVETOF_PREPARED);

    for(int i = 0; i < 6; ++i)
    {
        m_Pts[i].Draw(this);
    }
}

void CMatrixEffectMoveto::Release(void)
{
    DTRACE();
    SetDIP();
    HDelete(CMatrixEffectMoveto, this, m_Heap);
}

void CMatrixEffectMoveto::BeforeDraw(void)
{
    CMatrixEffectBillboard::PrepareDX();

    if(!FLAG(m_Flags, MOVETOF_PREPARED))
    {
        m_Tex->Preload();
        SETFLAG(m_Flags, MOVETOF_PREPARED);
    }
}