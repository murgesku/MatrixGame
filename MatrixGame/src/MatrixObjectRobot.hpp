// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixMap.hpp"
#include "MatrixConfig.hpp"
#include "MatrixProgressBar.hpp"
#include "Effects/MatrixEffect.hpp"



class CMatrixFlyer;

enum ERobotState
{
	ROBOT_IN_SPAWN,
	ROBOT_BASE_MOVEOUT,
	ROBOT_SUCCESSFULLY_BUILD,
    ROBOT_CARRYING,
    ROBOT_FALLING,
    ROBOT_DIP,  // death in progress
    ROBOT_BASE_CAPTURE,
    ROBOT_EMBRYO,

    ERobotState_FORCE_DWORD = 0x7FFFFFFF
};

enum EAnimation
{
    ANIMATION_OFF,
    ANIMATION_STAY,
    ANIMATION_MOVE,
    ANIMATION_BEGINMOVE,
    ANIMATION_ENDMOVE,
    ANIMATION_ROTATE,
    ANIMATION_MOVE_BACK,
    ANIMATION_BEGINMOVE_BACK,
    ANIMATION_ENDMOVE_BACK,

    EAnimation_FORCE_DWORD = 0x7FFFFFFF
};


enum ERobotUnitType
{
    MRT_EMPTY   = 0,
    MRT_CHASSIS	= 1,
    MRT_WEAPON	= 2,
    MRT_ARMOR   = 3,
    MRT_HEAD    = 4,

    ERobotUnitType_FORCE_DWORD = 0x7FFFFFFF
};


#define ANIMSPEED_CHAISIS_TRACK 0.20f
#define ANIMSPEED_CHAISIS_WHEEL 0.36f
#define ANIMSPEED_CHAISIS_PNEUMATIC 0.155f

#define CARRYING_DISTANCE   20.0f
#define CARRYING_SPEED      0.996

#define KEELWATER_SPAWN_FACTOR  0.01f
#define DUST_SPAWN_FACTOR  0.007f

#define MR_MAXUNIT		9

struct SMatrixRobotUnit;

struct SWeaponRepairData
{
    CBillboard m_b0;
    CBillboard m_b1;
    CBillboardLine m_bl;
    D3DXVECTOR3 m_pos0,m_pos1;

    DWORD   m_Flags;
    static const DWORD CAN_BE_DRAWN = SETBIT(0);

    void Release(void);
    void Update(SMatrixRobotUnit *unit);
    void Draw(bool now);

    static SWeaponRepairData *Allocate(void);
};



struct SChassisData
{
    union
    {
        struct
        {
            // for antigrav

            CMatrixEffectFireStream *m_LStream;
            CMatrixEffectFireStream *m_RStream;
            float                    m_StreamLen;
        }; 
        struct
        {
            float       m_DustCount;    // for aircraft
        };
        struct
        {
            D3DXVECTOR3 m_LastSolePos;  // for track
        };
        struct
        {
            D3DXVECTOR2 m_LinkPos;  // for pneumatic
            int         m_LinkPrevFrame;
        };
    };

    SChassisData() {};
};

struct SMatrixRobotUnit
{
	ERobotUnitType m_Type;		// 0-empty 1-Шасси 2-Оружие 3-Броня 4-голова

	CVectorObjectAnim * m_Graph;
	D3DXMATRIX m_Matrix;

    union
    {
        struct
        {
            SWeaponRepairData * m_WeaponRepairData;
	        CVOShadowStencil  * m_ShadowStencil;
	        ERobotUnitKind      m_Kind;
            float               m_NextAnimTime;
	        float   m_Angle;
            int     m_LinkMatrix;
            D3DXMATRIX m_IMatrix;
	        DWORD  m_Invert;
        };
        struct
        {
            D3DXVECTOR3 m_Pos;
            D3DXVECTOR3 m_Velocity;
            float       m_TTL;
            float       m_dp, m_dr, m_dy;
            
            BYTE        m_SmokeEffect[sizeof(SEffectHandler)];
        };
    };
    
    SMatrixRobotUnit() {};

    SEffectHandler & Smoke(void) {return *(SEffectHandler *)&m_SmokeEffect;}

    void PrepareForDIP(void);
};


struct SPneumaticData
{
    D3DXVECTOR2 foot;
    D3DXVECTOR2 other_foot; // if relink occurs, this contained new foot (relink coord)
    DWORD       newlink;
};

class CMatrixRobot : public CMatrixMapStatic {
        EAnimation  m_Animation;
		CMatrixBuilding *m_Base; //база из который вышел робот

protected:
        // hitpoint
        CMatrixProgressBar m_PB;
        int         m_ShowHitpointTime;
        float       m_HitPoint;
	    float       m_HitPointMax;  // Максимальное кол-во здоровья
        float       m_MaxHitPointInversed; // for normalized calcs


        static      SPneumaticData * m_Pneumaic;

        //DWORD       m_RobotFlags; // m_ObjectState used instead. do not uncomment!
public:
		
	    EShadowType     m_ShadowType; // 0-off 1-proj 2-proj with anim 3-stencil
        int             m_ShadowSize; // texture size for proj

        CWStr m_Name;
        int m_defHitPoint;

        float m_Speed;
        float m_RotSpeed;
        float m_PosX,m_PosY;

		int m_Side;		// 1-8

        int m_CalcBoundsLastTime;   // need for calculation of bound only 10 times per second
        D3DXVECTOR3 m_CalcBoundMin;
        D3DXVECTOR3 m_CalcBoundMax;


		int m_UnitCnt;
		SMatrixRobotUnit m_Unit[MR_MAXUNIT];

        SChassisData    m_ChassisData;

//temporary
        int              m_TimeWithBase;
		float m_HullRotAngle;
		float m_BotRotAngle;
		D3DXVECTOR3 m_Forward;
		D3DXVECTOR3 m_HullForward;
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
//Steering behavior's try
		D3DXVECTOR3 m_Velocity;//вектор скорости, длина равна скорости, направление движения

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

		CVOShadowProj * m_ShadowProj;

		ERobotState m_CurrState;

        float       m_FallingSpeed;
        union
        {
            CMatrixFlyer   *m_CargoFlyer;
            float           m_KeelWaterCount;
        };

        int m_MiniMapFlashTime;

	public:
		CMatrixRobot(void);
		~CMatrixRobot();


        void    ShowHitpoint(void) {m_ShowHitpointTime = HITPOINT_SHOW_TIME;}
        float   GetHitPoint(void) const {return m_HitPoint/10;}
        float   GetMaxHitPoint() { return m_HitPointMax/10; }
        void    InitMaxHitpoint(float hp) {m_HitPoint = hp; m_HitPointMax = hp; m_MaxHitPointInversed = 1.0f / hp;}


        void    MarkCrazy(void) {SETFLAG(m_ObjectState, ROBOT_CRAZY);}
        void    UnMarkCrazy(void) {RESETFLAG(m_ObjectState, ROBOT_CRAZY);}
        bool    IsCrazy(void) const {return FLAG(m_ObjectState, ROBOT_CRAZY);}

        void    MarkInPosition(void) {SETFLAG(m_ObjectState, ROBOT_FLAG_INPOSITION);}
        void    UnMarkInPosition(void) {RESETFLAG(m_ObjectState, ROBOT_FLAG_INPOSITION);}
        bool    IsInPosition(void) const {return FLAG(m_ObjectState, ROBOT_FLAG_INPOSITION);}

        bool    IsMustDie(void) const {return FLAG(m_ObjectState, ROBOT_MUST_DIE_FLAG);}
        void    MustDie(void)
        {
            SETFLAG(m_ObjectState, ROBOT_MUST_DIE_FLAG);
        }
        void    ResetMustDie(void) { RESETFLAG(m_ObjectState, ROBOT_MUST_DIE_FLAG);}

        void    MarkCaptureInformed(void) {SETFLAG(m_ObjectState, ROBOT_CAPTURE_INFORMED);}
        void    UnMarkCaptureInformed(void) {RESETFLAG(m_ObjectState, ROBOT_CAPTURE_INFORMED);}
        bool    IsCaptureInformed(void) const {return FLAG(m_ObjectState, ROBOT_CAPTURE_INFORMED);}
        


        void    SetBase(CMatrixBuilding *b) {m_Base = b; m_TimeWithBase = 0;}
        CMatrixBuilding *GetBase(void) const {return m_Base;}


        void SwitchAnimation(EAnimation a);

#ifdef _DEBUG        
        EAnimation GetAnimation(){ return m_Animation;}
#endif

        bool Carry(CMatrixFlyer *cargo, bool quick_connect = false); // NULL to off
        void ClearSelection(void);
	

        static void BuildPneumaticData(CVectorObject *vo);
        static void DestroyPneumaticData(void);
        void LinkPneumatic(void);
        void FirstLinkPneumatic(void);

        float GetChassisHeight(void) const;

        float Z_From_Pos(void);

        void    ApplyNaklon(const D3DXVECTOR3 &dir);

		void UnitInsert(int beforeunit, ERobotUnitType type, ERobotUnitKind kind);
		void WeaponInsert(int beforeunit, ERobotUnitType type, ERobotUnitKind kind, int hullno, int pilon);
        void UnitDelete(int nounit);
		void UnitClear(void);

		void BoundGet(D3DXVECTOR3 & bmin,D3DXVECTOR3 & bmax);

        void WeaponSelectMatrix(void);

        void DoAnimation(int cms);

        virtual bool Damage(EWeapon weap, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, int attacker_side, CMatrixMapStatic* attaker) = 0;
		virtual void RNeed(dword need);

		virtual void Takt(int cms);
        virtual void LogicTakt(int cms) = 0;

		virtual bool Pick(const D3DXVECTOR3 & orig, const D3DXVECTOR3 & dir,float * outt)  const;
        bool PickFull(const D3DXVECTOR3 & orig, const D3DXVECTOR3 & dir,float * outt)  const;

		virtual void BeforeDraw(void);
		virtual void Draw(void);
		virtual void DrawShadowStencil(void);
		virtual void DrawShadowProj(void);

        virtual void FreeDynamicResources(void);

        void OnLoad(void) {};

        virtual bool CalcBounds(D3DXVECTOR3 &omin, D3DXVECTOR3 &omax);
        virtual int  GetSide(void) const {return m_Side;};
        virtual bool  NeedRepair(void) const {return m_HitPoint < m_HitPointMax;}
        virtual bool InRect(const CRect &rect)const;

        void    OnOutScreen(void) {};
};

__forceinline bool CMatrixMapStatic::IsLiveRobot(void) const
{
    return IsRobot() && ((CMatrixRobot*)this)->m_CurrState!=ROBOT_DIP;
}

