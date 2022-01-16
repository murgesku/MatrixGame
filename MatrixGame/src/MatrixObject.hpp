// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixMap.hpp"

enum EBehFlag {
    BEHF_STATIC,   // не реагирует на окружающую действительность
    BEHF_BURN,     // горит (и сгорает)
    BEHF_BREAK,    // ломается
    BEHF_ANIM,     // меняется анимация
    BEHF_SENS,     // чувствительность к приближению роботов
    BEHF_SPAWNER,  // робот - spawner
    BEHF_TERRON,   // босс
    BEHF_PORTRET,

    BEHF_FORCE_DWORD = 0x7fffffff
};

struct SMatrixSkin;

class CMatrixMapObject : public CMatrixMapStatic {
    struct SObjectShadowTexture {
        CTextureManaged *tex;
        CTextureManaged *tex_burn;

        SObjectShadowTexture(void) : tex(NULL), tex_burn(NULL) {}
    };

public:
    float m_AngleZ;
    float m_AngleX;
    float m_AngleY;
    float m_Scale;
    float m_TexBias;

    int m_Type;

    int m_UID;

    D3DXVECTOR3 m_ShCampos;
    D3DXVECTOR2 m_ShDim;

    EShadowType m_ShadowType;

    static SObjectShadowTexture *m_ShadowTextures;
    static int m_ShadowTexturesCount;

    EBehFlag m_BehFlag;
    union {
        struct {
            int m_NextTime;
            int m_BurnTimeTotal;  // increases

            SMatrixSkin *m_BurnSkin;
            int m_BurnSkinVis;  // 0-255
        };
        struct {
            int m_BreakHitPoint;
            int m_AnimState;  // for BEHF_ANIM
            CMatrixProgressBar *m_PB;
            int m_BreakHitPointMax;
            int m_NextExplosionTime;
            int m_NextExplosionTimeSound;
        };
        struct {
            // -1 - not inited
            // 0 - default (idle)
            int m_PrevStateRobotsInRadius;

            SObjectCore *m_SpawnRobotCore;
            float m_SensRadius;
            int m_Photo;
            int m_PhotoTime;
        };
    };

    CVectorObjectAnim *m_Graph;
    CVOShadowStencil *m_ShadowStencil;
    CMatrixShadowProj *m_ShadowProj;

    void FreeShadowTexture(void);

public:
    CMatrixMapObject(void);
    ~CMatrixMapObject();

    static void StaticInit(void) {
        m_ShadowTextures = NULL;
        m_ShadowTexturesCount = 0;
    }
#ifdef _DEBUG
    static void ValidateAfterReset(void) {
        if (m_ShadowTextures || m_ShadowTexturesCount)
            _asm int 3
    }
#endif

    static void InitTextures(int n);
    static void ClearTextures(void);
    static void ClearTexture(CBaseTexture *tex);

    void SetupMatricesForShadowTextureCalc(void);
    void MarkSpecialShadow(void) { SETFLAG(m_ObjectState, OBJECT_STATE_SHADOW_SPECIAL); }
    void Init(int ids);
    // void         InitAsBaseRuins(CMatrixBuilding *b, const CWStr &namev, const CWStr &namet, bool shadow);
    void InitAsBaseRuins(const D3DXVECTOR2 &pos, int angle, const CWStr &namev, const CWStr &namet, bool shadow);

    virtual void RNeed(dword need);

    virtual void Takt(int cms);
    virtual void LogicTakt(int);
    void PauseTakt(int cms);

    void ApplyAnimState(int anims);

    virtual bool Pick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, float *outt) const;
    bool PickFull(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, float *outt) const;

    virtual bool Damage(EWeapon weap, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, int attacker_side,
                        CMatrixMapStatic *attaker);

    virtual void BeforeDraw(void);
    virtual void Draw(void);
    virtual void DrawShadowStencil(void);
    virtual void DrawShadowProj(void);

    virtual void FreeDynamicResources(void);

    void OnLoad(void);

    virtual bool CalcBounds(D3DXVECTOR3 &omin, D3DXVECTOR3 &omax);

    virtual bool FitToMask(DWORD mask) {
        if (mask & TRACE_OBJECT)
            return true;
        return false;
    }

    virtual int GetSide(void) const { return 0; };
    virtual bool NeedRepair(void) const { return false; }

    virtual bool InRect(const CRect &rect) const;

    void OnOutScreen(void){};
};
