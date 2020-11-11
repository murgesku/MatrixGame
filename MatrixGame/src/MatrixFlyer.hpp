// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_FLYER_INCLUDE
#define MATRIX_FLYER_INCLUDE

#include "MatrixMapStatic.hpp"
#include "MatrixProgressBar.hpp"

class CMatrixEffectSelection;

#define FLYER_ALT_EMPTY     70
#define FLYER_ALT_CARRYING  110
#define FLYER_ALT_MIN       20

#define FLYER_MOUSE_DEAD_AREA_TOP 0.3f            // 0.2 of screen width
#define FLYER_MOUSE_DEAD_AREA_BOTTOM 0.05f            // 0.0 of screen width

#define FLYER_SEEKBASE_MATCH_RADIUS   10
#define FLYER_TARGET_MATCH_RADIUS   50
#define FLYER_FIRETARGET_MATCH_RADIUS   20

#define FLYER_SPINUP_TIME   3000

//#define FLYER_MIN_SPEED 0.002f
#define FLYER_MAX_SPEED 0.2f
#define FLYER_MAX_BACK_SPEED 0.10f
#define FLYER_MAX_STRIFE_SPEED 0.12f

#define FLYER_MAX_FIRE_SPEED 0.8f
//#define FLYER_MAX_ALT_CHANGE_SPEED  0.1f

#define FLYER_RADIUS    15


#define ENGINE_ANGLE_BREAK  GRAD2RAD(179.9)
#define ENGINE_ANGLE_STAY   GRAD2RAD(90)
#define ENGINE_ANGLE_MOVE   GRAD2RAD(0)

#define YAW_ANGLE_BREAK  GRAD2RAD(70)
#define YAW_ANGLE_STAY   GRAD2RAD(0)
#define YAW_ANGLE_MOVE   GRAD2RAD(-20)

//#define FLYER_WEAPON1_HFOV  GRAD2RAD(90)
//#define FLYER_WEAPON1_UP_ANGLE  GRAD2RAD(0)
//#define FLYER_WEAPON1_DOWN_ANGLE  GRAD2RAD(85)

#define FLYER_SELECTION_HEIGHT 2
#define FLYER_SELECTION_SIZE 20
class CMatrixSideUnit;
class CMatrixEffectWeapon;
class CMatrixRobot;
class CMatrixBuilding;
class CMatrixEffectElevatorField;
class CMatrixEffectFireStream;

enum EWeapon;

enum EFlyerUnitType
{
    FLYER_UNIT_BODY = 0,
    FLYER_UNIT_VINT = 1,
    FLYER_UNIT_ENGINE = 2,
    FLYER_UNIT_WEAPON = 3,
    FLYER_UNIT_WEAPON_HOLLOW = 4
};


enum EFlyerKind
{
    FLYER_SPEED = 0,
    FLYER_ATTACK = 1,
    FLYER_TRANSPORT = 2,
    FLYER_BOMB = 3,
};

class CMatrixFlyer;

struct SMatrixFlyerUnit
{
    EFlyerUnitType      m_Type;
	CVectorObjectAnim * m_Graph;
	CVOShadowStencil *  m_ShadowStencil;
    D3DXMATRIX          m_Matrix;
    D3DXMATRIX          m_IMatrix;

    union
    {
        struct
        {
            int   m_MatrixID; // matrix id
            DWORD m_Inversed;
	        float m_Angle;
            float m_AngleSpeedMax;
            D3DMATRIX  m_VintMatrix;
	        CTextureManaged *m_Tex;

            int m_CollapsedCountDown;
        } m_Vint;
        struct
        {
            CMatrixEffectWeapon * m_Weapon;
            int   m_MatrixID; // matrix id
            union
            {
                struct
                {
                    DWORD m_Inversed;
	                float m_AngleZ;
	                float m_AngleX;
                    float m_HFOV;
                    float m_UpAngle;
                    float m_DownAngle;
                };
                struct
                {
                    int m_Unit;

                };
            };

        } m_Weapon;
        struct
        {
            int   m_MatrixID; // matrix id
            DWORD m_Inversed;
	        float m_Angle;
        } m_Engine;

    };


    void    Release(void);
    bool    Takt(CMatrixFlyer *owner, float ms);
};

#define MF_TARGETMOVE   SETBIT(0)
#define MF_TARGETFIRE   SETBIT(1)
//#define MF_SEEKBASE     SETBIT(2)
//#define MF_SEEKBASEOK   SETBIT(3)
//#define MF_SEEKBASEFOUND  SETBIT(4)

#define FLYER_ACTION_MOVE_FORWARD    SETBIT(5)
#define FLYER_ACTION_MOVE_BACKARD    SETBIT(6)
#define FLYER_ACTION_MOVE_LEFT       SETBIT(7)
#define FLYER_ACTION_MOVE_RIGHT      SETBIT(8)
#define FLYER_ACTION_ROT_LEFT        SETBIT(9)
#define FLYER_ACTION_ROT_RIGHT       SETBIT(10)

#define FLYER_MANUAL                SETBIT(11)
#define FLYER_BODY_MATRIX_DONE      SETBIT(12)
#define FLYER_IN_SPAWN              SETBIT(13)
#define FLYER_BREAKING              SETBIT(14)  // тормозит (в стратегическом режиме сход с траектории)
#define FLYER_SGROUP                SETBIT(15)
#define FLYER_SARCADE               SETBIT(16)
#define FLYER_IN_SPAWN_SPINUP       SETBIT(17) // раскручивает лопасти


struct SFlyerTaktData;

enum EFlyerOrder
{
    FO_GIVE_BOT,
    FO_FIRE,
};


struct SFireStream
{
    union
    {
        CMatrixEffectFireStream *effect;
        CBlockPar *bp;
    };
    int                      matrix;    // matrix id
    int                      unit;      // unit index

};

class CMatrixFlyer : public CMatrixMapStatic
{

    static D3D_VB               m_VB;
    static int                  m_VB_ref;

    int                 m_Side;

    SMatrixFlyerUnit   *m_Units;
    int                 m_UnitCnt;
    int                 m_EngineUnit;

    SFireStream             *m_Streams;
    int                      m_StreamsCount;
    float                    m_StreamLen;

    DWORD       m_Flags;
	D3DXVECTOR2 m_Target;
    D3DXVECTOR3 m_FireTarget;

    DWORD       m_Sound;

    union
    {
	    //float       m_TargetAlt;
        CMatrixBuilding *m_Base;
    };

	D3DXVECTOR3 m_Pos; // origin

    float m_DAngle;
    float m_AngleZ;
    float m_AngleZSin;
    float m_AngleZCos;

    float m_RotZSpeed;
	float m_MoveSpeed;
    float m_StrifeSpeed; // <0 - left, >0 rite

	float m_Pitch;
	float m_Yaw;


    float m_TargetEngineAngle;
    float m_TargetYawAngle;
    float m_TargetPitchAngle;

    //float          m_BaseLandAngle;
    //D3DXVECTOR3    m_BaseLandPos;
    CTrajectory *m_Trajectory;
    union
    {
        struct {D3DXVECTOR3 m_StoreTarget;}; // in breaking mode
        struct
        {
            float        m_TrajectoryPos; // [0..1]
            float        m_TrajectoryLen; 
            float        m_TrajectoryLenRev; 
            float        m_TrajectoryTargetAngle;
        };
    };

    int m_TgtUpdateCount;

    CWStr m_Name;

    // hitpoint
    CMatrixProgressBar m_PB;
    int         m_ShowHitpointTime;
    float       m_HitPoint;
	float       m_HitPointMax;  // Максимальное кол-во здоровья
    float       m_MaxHitPointInversed; // for normalized calcs

    // carrying

    struct SCarryData
    {

        CMatrixRobot               *m_Robot;
        D3DXVECTOR3                 m_RobotForward;
        D3DXVECTOR3                 m_RobotUp;
        D3DXVECTOR3                 m_RobotUpBack;
        float                       m_RobotAngle;
        float                       m_RobotMassFactor;
        CMatrixEffectElevatorField *m_RobotElevatorField;
    } m_CarryData;

    CTextureManaged*    m_BigTexture;
    CTextureManaged*    m_MedTexture;
    CTextureManaged*    m_SmallTexture;

    void    CalcMatrix(void);
    void    CalcBodyMatrix(void);
    void    LogicTaktArcade(SFlyerTaktData &td);
    void    LogicTaktStrategy(SFlyerTaktData &td);
    bool    LogicTaktOrder(SFlyerTaktData &td);


    void    CalcCollisionDisplace(SFlyerTaktData &td);

    //void CalcTrajectory(const D3DXVECTOR3 &target, const D3DXVECTOR3 &dir);
    void CalcTrajectory(const D3DXVECTOR3 &target);
    void ProceedTrajectory(SFlyerTaktData &td);
    void CancelTrajectory(void);
    bool IsTrajectoryEnd(void) const {return m_TrajectoryPos >= 0.99f;}

    float   CalcFlyerZInPoint(float x, float y);
    CMatrixEffectSelection *m_Selection;
    int m_CtrlGroup;
public:

    friend struct SMatrixFlyerUnit;

    int         m_Team;
    int         m_Group;

    

    bool        SelectByGroup();
    bool        SelectArcade();
    void        UnSelect();
    bool        IsSelected();
    CMatrixEffectSelection* GetSelection()                                      { return m_Selection; }
    bool        CreateSelection();
    void        KillSelection();
    void        MoveSelection();
    CWStr       GetName()                                                       { return m_Name; }
    void        SetName(const CWStr &name)                                      { m_Name = name; }
    int         GetCtrlGroup()                                                  { return m_CtrlGroup; } 
    void        SetCtrlGroup(int group)                                         { m_CtrlGroup = group; }


    void        SetSide(int side)                                               { m_Side = side; }

    EFlyerKind  m_FlyerKind;

    static void StaticInit(void)
    {
        m_VB = NULL;
        m_VB_ref = 0;
    }

    static void     MarkAllBuffersNoNeed(void);
    static void     InitBuffers(void);



    CMatrixFlyer(void);
    ~CMatrixFlyer(void);

    void    ApplyOrder(const D3DXVECTOR2 &pos, int side, EFlyerOrder order, float ang, int place, const CPoint &bpos, int botpar_i);

    // carry robot
    bool CarryingRobot(void) const {return m_CarryData.m_Robot != NULL;}
    CMatrixRobot* GetCarryingRobot(void) {return m_CarryData.m_Robot;}
    CMatrixFlyer::SCarryData *GetCarryData(void) {return &m_CarryData;};

    void    ShowHitpoint(void) {m_ShowHitpointTime = HITPOINT_SHOW_TIME;}
    float   GetHitPoint(void) const {return m_HitPoint;}
    float   GetMaxHitPoint() { return m_HitPointMax; }
    void    InitMaxHitpoint(float hp) {m_HitPoint = hp; m_HitPointMax = hp; m_MaxHitPointInversed = 1.0f / hp;}
#ifdef _DEBUG
    void    SetHitpoint(float hp) {m_HitPoint = hp;}
#endif

    const D3DXVECTOR3  & GetPos(void) const {return m_Pos;}
    const D3DXVECTOR3   GetPos(float pered) const {return GetPos() + D3DXVECTOR3(-m_AngleZSin, m_AngleZCos,0) * pered;}

    const D3DXVECTOR2   GetTarget() const {return m_Target;}
    //void                SetAlt(float alt) {m_TargetAlt = alt;}
    
    float               GetAngle(void) const {return m_AngleZ;};
    void                SetAngle(float a)
    {
        a = (float)AngleNorm(a);
        if (a!=m_AngleZ)
        {
            m_AngleZ = a; SinCos(a, &m_AngleZSin, &m_AngleZCos);
        }
    }

    void                SetTarget(const D3DXVECTOR2 &tgt);
    void                SetFireTarget(const D3DXVECTOR3 &tgt)
    {
        //if (!FLAG(m_Flags, MF_SEEKBASE))
        {
            m_FireTarget = tgt;
            RESETFLAG(m_Flags, MF_TARGETFIRE);
        }
    };

    float               GetSpeed(void) const {return m_MoveSpeed;}
    float               GetSpeedNorm(void) const {return m_MoveSpeed / FLYER_MAX_SPEED ;}

    void    RotLeft(void) {SETFLAG(m_Flags, FLYER_ACTION_ROT_LEFT);}
    void    RotRight(void) {SETFLAG(m_Flags, FLYER_ACTION_ROT_RIGHT);}
    void    MoveLeft(void) {SETFLAG(m_Flags, FLYER_ACTION_MOVE_LEFT);}
    void    MoveRight(void) {SETFLAG(m_Flags, FLYER_ACTION_MOVE_RIGHT);}
    void    MoveForward(void) {SETFLAG(m_Flags, FLYER_ACTION_MOVE_FORWARD);}
    void    MoveBackward(void) {SETFLAG(m_Flags, FLYER_ACTION_MOVE_BACKARD);}

    void FireBegin(void);
    void FireEnd(void);


    //void DownToBase(CMatrixBuilding *building);

    void Begin(CMatrixBuilding *b);

    void DrawPropeller(void);


    void ReleaseMe();

    void CreateTextures();
    
    CTextureManaged* GetBigTexture()                            { return m_BigTexture; }
    CTextureManaged* GetMedTexture()                            { return m_MedTexture; }
    CTextureManaged* GetSmallTexture()                          { return m_SmallTexture; }

    void    CreateProgressBarClone(float x, float y, float width, EPBCoord clone_type);
    void    DeleteProgressBarClone(EPBCoord clone_type);


    virtual bool Damage(EWeapon weap, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, int attacker_side, CMatrixMapStatic* attaker);
	virtual void RNeed(dword need);

	virtual void Takt(int cms);
    virtual void LogicTakt(int cms);

	virtual bool Pick(const D3DXVECTOR3 & orig, const D3DXVECTOR3 & dir,float * outt) const;

	virtual void BeforeDraw(void);
	virtual void Draw(void);
	virtual void DrawShadowStencil(void);
    virtual void DrawShadowProj(void) {};

    virtual void FreeDynamicResources(void);

    virtual void Load(CBuf & buf, CTemporaryLoadData *td) {};

    virtual bool CalcBounds(D3DXVECTOR3 &omin, D3DXVECTOR3 &omax);
    virtual int  GetSide(void) const {return m_Side;};
    virtual bool  NeedRepair(void) const {return m_HitPoint < m_HitPointMax;}

    virtual bool InRect(const CRect &rect)const;

    void    OnOutScreen(void) {};
};



#endif