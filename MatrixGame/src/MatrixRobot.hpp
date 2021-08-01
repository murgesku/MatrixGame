// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixMap.hpp"
#include "MatrixObject.hpp"
#include "MatrixObjectRobot.hpp"
#include "MatrixObjectBuilding.hpp"
#include "Logic/MatrixEnvironment.h"
#include "Logic/MatrixTactics.h"
#include "Logic/MatrixAIGroup.h"

#if defined _DEBUG || defined _TRACE
#define CHECK_ROBOT_POS() { if (m_MapX < 0 || m_MapX >= g_MatrixMap->m_SizeMove.x || m_MapY < 0 || m_MapY >= g_MatrixMap->m_SizeMove.y) _asm int 3 }
#else
#define CHECK_ROBOT_POS()
#endif

class CMatrixEffectSelection;

#define MAX_WEAPON_CNT			5
#define COLLIDE_FIELD_R			3
#define COLLIDE_BOT_R			18
#define COLLIDE_SPHERE_R        (GLOBAL_SCALE_MOVE/2.1f)
//5.2f
#define	BASE_DIST			    70		
#define MAX_HULL_ANGLE			GRAD2RAD(360)
#define BARREL_TO_SHOT_ANGLE    GRAD2RAD(15)
#define HULL_TO_ENEMY_ANGLE     GRAD2RAD(45)
#define MAX_ORDERS              5
#define ROBOT_FOV               GRAD2RAD(180)
#define GATHER_PERIOD           100
#define ZERO_VELOCITY           0.02f
#define DECELERATION_FORCE      0.9900990099009901f
#define MIN_ROT_DIST            20
#define GET_LOST_MIN            COLLIDE_BOT_R * 5
#define GET_LOST_MAX            COLLIDE_BOT_R * 10
#define ROBOT_SELECTION_HEIGHT  3
#define ROBOT_SELECTION_SIZE    20

#define HULL_SOUND_PERIOD       3000
#define HULL_ROT_TAKTS          30
#define HULL_ROT_S_ANGL         10

#define MAX_CAPTURE_CANDIDATES  4

struct SBotWeapon
{
private:
	CMatrixEffectWeapon*    m_Weapon;
public:
	SMatrixRobotUnit*       m_Unit;
    int                     m_Heat;
    int                     m_HeatPeriod;
    int                     m_HeatMod;//heat grows per WEAPON_HEAT_PERIOD
    int                     m_CoolDownPeriod;
    int                     m_CoolDownMod;

    DWORD                   m_On;
    SBotWeapon(void)        { m_On = true;m_Heat = 0;m_HeatMod = 0;m_HeatPeriod = 0;m_CoolDownMod = 0;m_CoolDownPeriod = 0;}

    void                    ModifyCoolDown(float addk) {m_Weapon->ModifyCoolDown(addk);}
    void                    ModifyDist(float addk) {m_Weapon->ModifyDist(addk);}
    bool                    GetWeaponPos(D3DXVECTOR3 &pos)          { if(m_Weapon){pos = m_Weapon->GetPos(); return true;} return false;}
    void                    SetArcadeCoefficient()      { if(m_Weapon)m_Weapon->SetArcadeCoefficient(); }
    void                    SetDefaultCoefficient()     { if(m_Weapon)m_Weapon->SetDefaultCoefficient(); }
    bool                    IsEffectPresent(void) const {return m_Weapon != NULL;}
    EWeapon                 GetWeaponType(void) const {return m_Weapon->GetWeaponType();}
    float                   GetWeaponDist(void) const {return m_Weapon->GetWeaponDist();}
    void                    Modify(const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &speed)
    {
        m_Weapon->Modify(pos, dir, speed);
    }
    void                    UpdateRepair(void)
    {
        if(m_Unit->m_WeaponRepairData) m_Unit->m_WeaponRepairData->Update(m_Unit);
    }

    void                    Takt(float t)
    {
        m_Weapon->Takt(t);
        if (m_Unit->m_WeaponRepairData) m_Unit->m_WeaponRepairData->Update(m_Unit);
    }
    bool IsFireWas(void) const {return m_Weapon->IsFireWas();}

    void SetOwner(CMatrixMapStatic *ms) {m_Weapon->SetOwner(ms);}

    void CreateEffect(DWORD user, FIRE_END_HANDLER handler, EWeapon type, int cooldown = 0)
    {
        m_Weapon = (CMatrixEffectWeapon*)CMatrixEffect::CreateWeapon(D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 1), user, handler, type, cooldown);
        if(type == WEAPON_REPAIR)
        {
            PrepareRepair();
        }
    }

    void FireBegin(const D3DXVECTOR3 &speed, CMatrixMapStatic *skip)
    {
        m_Weapon->FireBegin(speed, skip);
    }

    void FireEnd(void)
    {
        m_Weapon->FireEnd();
        m_Unit->m_Graph->SetAnimLooped(0);
    }

    void Draw(CMatrixRobotAI *robot);
    void Release(void)
    {
        m_Weapon->Release();
        if(m_Unit->m_WeaponRepairData) m_Unit->m_WeaponRepairData->Release();
        m_Unit->m_WeaponRepairData = NULL;
    }

    void PrepareRepair(void);
};

enum OrderType
{
	ROT_EMPTY_ORDER,
	ROT_MOVE_TO,
	ROT_MOVE_TO_BACK,
    ROT_MOVE_RETURN,          // Робот пропускает другого робота затем возвращается в эту позицию
    ROT_STOP_MOVE,
	ROT_FIRE,
	ROT_STOP_FIRE,
    ROT_CAPTURE_BUILDING,
    ROT_STOP_CAPTURE,

    OrderType_FORCE_DWORD = 0x7FFFFFFF
};

enum OrderPhase
{
    ROP_EMPTY_PHASE,
    ROP_WAITING_FOR_PARAMS,
    ROP_MOVING,
    ROP_FIRING,
    ROP_CAPTURE_MOVING,
    ROP_CAPTURE_IN_POSITION,
    ROP_CAPTURE_SETTING_UP,
    ROP_CAPTURING,
    ROP_GETING_LOST,

    OrderPhase_FORCE_DWORD = 0x7FFFFFFF
};

struct SCaptureCandidate
{
    SObjectCore *bcore;
    int          tbc;       // time before capture
};

struct SOrder 
{
	OrderType           m_OrderType;
    OrderPhase          m_OrderPhase;

	float               m_Param1, m_Param2, m_Param3;
    int                 m_Param4;
	//CMatrixRobotAI*     m_Robot;
    //void*               m_Object;
    SObjectCore         *m_ObjCore;
public:
    
    OrderType GetOrderType(void) const                                                  { return m_OrderType; }
    OrderPhase GetOrderPhase(void) const                                                { return m_OrderPhase; }

    void SetPhase(const OrderPhase &phase)                                              { m_OrderPhase = phase; }
    void GetParams(float *p1, float *p2, float *p3, int *p4 = NULL)
    {
        if(p1 != NULL)
            *p1 = m_Param1;
        if(p2 != NULL)
            *p2 = m_Param2;
        if(p3 != NULL)
            *p3 = m_Param3;
        if(p4 != NULL)
            *p4 = m_Param4;
    }

    CMatrixMapStatic *GetStatic(void)
    {
        if(m_ObjCore)
        {
            if(m_ObjCore->m_Object) return m_ObjCore->m_Object;
            m_ObjCore->Release();
            m_ObjCore = NULL;
        }
        return NULL;
    }

    void SetOrder(const OrderType &orderType, const float &p1, const float &p2, const float &p3, int p4 = 0)
    {
        Reset(); 
        m_OrderType = orderType;
        m_Param1 = p1;
        m_Param2 = p2;
        m_Param3 = p3;
        m_Param4 = p4;
    }
    void SetOrder(const OrderType &orderType, CMatrixMapStatic *obj)
    {
        Reset();
        m_OrderType = orderType;
        m_ObjCore = obj?obj->GetCore(DEBUG_CALL_INFO):NULL;
    }

    void Reset(void)
    {
        memset(this, 0, sizeof(SOrder));
    }

    void Release(void)
    {
        if(m_ObjCore)
        {
            if(m_ObjCore->m_Object && m_ObjCore->m_Object->IsBuilding())
            {
                m_ObjCore->m_Object->AsBuilding()->ResetCaptured();
            }

            m_ObjCore->Release();
            m_ObjCore = NULL;
        }
    }
};


class CMatrixRobotAI : public CMatrixRobot
{
    CTextureManaged    *m_BigTexture;
    CTextureManaged    *m_MedTexture;
#ifdef USE_SMALL_TEXTURE_IN_ROBOT_ICON
    CTextureManaged    *m_SmallTexture;   // hm, may be its no needed //may be, who cares :)
#endif

    SEffectHandler      m_Ablaze;

    int                 m_ColsWeight;
    int                 m_ColsWeight2;
    int                 m_Cols;
    int                 m_Team;
    int                 m_GroupLogic; // dab. В какой логической группе находится робот
    int                 m_Group;
    int                 m_OrdersInPool;
    SOrder              m_OrdersList[MAX_ORDERS];

    SCaptureCandidate   m_CaptureCandidates[MAX_CAPTURE_CANDIDATES];
    int                 m_CaptureCandidatesCnt;

    int                 m_MapX, m_MapY;                     //
    int                 m_ZoneCur;				            // Зона в которой находится робот
    int                 m_DesX, m_DesY;		                // Точка в которую хочет встать робот
    int                 m_ZoneDes;				            // Зона в которую стремится робот
    int                 m_ZonePathNext;			            // Следующая зона в пути 
    int                 m_ZonePathCnt;			            // Количество зон (включительно начальную и конечную) до m_ZoneDes
    int                 m_ZoneNear;				            // Ближайшая зона от той в которой находится робот по направлению к m_ZoneDes
    int                 m_MovePathCnt;			            // Количество точек в пути движения
    int                 m_MovePathCur;			            // Текущая точка в пути движения
    int                *m_ZonePath;			                // Список зон до m_ZoneDes
    CPoint              m_MovePath[MatrixPathMoveMax];      // Путь движения
    float               m_MovePathDist;                     // Длина расчитанного пути
    float               m_MovePathDistFollow;               // Сколько робот прошёл по пути
    D3DXVECTOR2         m_MoveTestPos;
    int                 m_MoveTestChange;

    float               m_Strength;                         // Сила робота

    int                 m_NextTimeAblaze;
    int                 m_NextTimeShorted;
    D3DXVECTOR3         m_CollAvoid;

    float               m_SpeedWaterCorr;                   // коррекция скорости при движении по воде
    float               m_SpeedSlopeCorrDown;               // коррекция скорости при движении с горы
    float               m_SpeedSlopeCorrUp;                 // коррекция скорости при движении в гору

	float               m_maxSpeed;		                    // максимально развиваемая скорость
    float               m_maxHullSpeed;                     // скорость поворота корпуса
    float               m_maxRotationSpeed;                 // скорость вращения робота на месте
    float               m_MaxFireDist;
    float               m_MinFireDist;
    float               m_RepairDist;
    float               m_SyncMul;
    int                 m_CtrlGroup;
    SBotWeapon          m_Weapons[MAX_WEAPON_CNT];          // установленное оружие
    int                 m_WeaponsCnt;

    //Маркер, указывающий, что робот был выведен из ручного контроля игроком, никаких приказов на себе не имеет,
    //а потому не трогай его, сука, со своими перенаправлениями в ближайший треугольник сетки!
    bool                m_AfterManualControl;

    //CMatrixMapStatic*   m_FireTarget;
//    int                 m_GatherPeriod;
    CInfo               m_Environment;
    int                 m_HaveRepair;                       // 0-нет 1-есть чинилка 2-все оружия это чинилка (не учитывается бомба)

    D3DXVECTOR3         m_WeaponDir;
    DWORD               m_nLastCollideFrame;
    DWORD               m_PrevTurnSign;
    DWORD               m_HullRotCnt;
    CMatrixEffectSelection *m_Selection;

    int                 m_LastDelayDamageSide;
public:
    DWORD               m_SoundChassis;

    float               m_RadarRadius;
    float               m_LightProtect;     // 0 .. 1   (0 - no protection, 1 - full protection)
    float               m_BombProtect;      // 0 .. 1   (0 - no protection, 1 - full protection)
    float               m_AimProtect;       // 0 .. 1   (0 - no protection, 1 - full protection)

    float               m_GroupSpeed;                       // скорость робота в группе
    float               m_ColSpeed;                         // скорость робота если впереди другой робот

    void                CreateTextures();
    CTextureManaged*    GetBigTexture()                            { return m_BigTexture; }
    CTextureManaged*    GetMedTexture()                            { return m_MedTexture; }
#ifdef USE_SMALL_TEXTURE_IN_ROBOT_ICON
    CTextureManaged*    GetSmallTexture()                          { return m_SmallTexture; }
#endif
    
    int         GetCols()                                           { return m_Cols; }
    void        IncCols()                                           { ++m_Cols; }
    int         GetColsWeight()                                     { return m_ColsWeight; }
    void        IncColsWeight(int val = 1)                          { m_ColsWeight += val; }
    void        SetColsWeight(int w)                                { m_ColsWeight = w; }
    int         GetColsWeight2()                                    { return m_ColsWeight2; }
    void        IncColsWeight2(int val = 1)                         { m_ColsWeight2 += val; }
    void        SetColsWeight2(int w)                               { m_ColsWeight2 = w; }
    int         GetMapPosX(void) const                              { return m_MapX; }
    int         GetMapPosY(void) const                              { return m_MapY; }
    CInfo      *GetEnv()                                            { return &m_Environment; }
    void        SetEnvTarget(CMatrixMapStatic *t)                   { m_Environment.m_Target = t; }
    int         GetTeam()                                           { return m_Team; }
    void        SetTeam(int t)                                      { m_Team = t; }
    int         GetGroup()                                          { return m_Group; }
    void        SetGroup(int g)                                     { m_Group = g; }
    int*        GetGroupP()                                         { return &m_Group; }
    float       GetMaxSpeed()                                       { return m_maxSpeed; }
    void        SetMaxSpeed(float s)                                { m_maxSpeed = s; }
    int         GetOrdersInPool()                                   { return m_OrdersInPool; }
    SOrder*     GetOrder(int no)                                    { return m_OrdersList + no; }
    CMatrixEffectSelection* GetSelection()                          { return m_Selection; }
    //CWStr      &GetName()                                           { return m_Name; }
    const SBotWeapon &GetWeapon(int i) const                        { return m_Weapons[i]; }
    float       GetMaxFireDist()                                    { return m_MaxFireDist; }
    float       GetMinFireDist()                                    { return m_MinFireDist; }
    float       GetRepairDist()                                     { return m_RepairDist; }
    int         GetGroupLogic()                                     { return m_GroupLogic; }
    void        SetGroupLogic(int gl)                               { m_GroupLogic = gl; }
    int         GetRegion(void)                                     { return g_MatrixMap->GetRegion(m_MapX, m_MapY); }
    int         GetCtrlGroup()                                      { return m_CtrlGroup; }
    void        SetCtrlGroup(int group)                             { m_CtrlGroup = group; }

    void        MapPosCalc() { g_MatrixMap->PlaceGet(m_Unit[0].m_Kind - 1, m_PosX - 20.0f, m_PosY - 20.0f, &m_MapX, &m_MapY); }

    bool        IsDisableManual()                                   { return FLAG(m_ObjectState, ROBOT_FLAG_DISABLE_MANUAL); }
    void        SetWeaponToArcadedCoeff();
    void        SetWeaponToDefaultCoeff();

    float       GetStrength(void)                                   { return m_Strength; } // Сила робота
    void        CalcStrength(void);                                 // Расчитываем силу робота

    bool        PLIsInPlace(void) const;

    void        CreateProgressBarClone(float x, float y, float width, EPBCoord clone_type);
    void        DeleteProgressBarClone(EPBCoord clone_type);

    bool        SelectByGroup();
    bool        SelectArcade();
    void        UnSelect();
    bool        IsSelected();
    bool        CreateSelection();
    void        KillSelection();
    void        MoveSelection();

    //Маркер, указывающий, что робот был выведен из ручного контроля игроком, никаких приказов на себе не имеет,
    //а потому не трогай его, сука, со своими перенаправлениями в ближайший треугольник сетки!
    bool        IsAfterManual() { return m_AfterManualControl; }
    void        SetAfterManual(bool value) { m_AfterManualControl = value; }

    void        PlayHullSound();
    bool        CheckFireDist(const D3DXVECTOR3 &point);

    bool        IsAutomaticMode(void) const
    {
        return m_CurrState == ROBOT_IN_SPAWN || m_CurrState == ROBOT_BASE_MOVEOUT || m_CurrState == ROBOT_BASE_CAPTURE;
    }
    bool CanBreakOrder(void)
    {
        if(m_Side != PLAYER_SIDE || FLAG(g_MatrixMap->m_Flags, MMFLAG_FULLAUTO))
        {
            CMatrixBuilding *cf = GetCaptureBuilding();
            if(cf) 
            {
                return false; //DO NOT BREAK CAPTURING!!!!!!!!!!!!!!!!!!!!!!!! NEVER!!!!!!!!!!
                //if(cf->IsBase()) return false;
                //if(cf->GetSide() != robot->GetSide())
                //{
                //    if((float(cf->m_TrueColor.m_ColoredCnt)/MAX_ZAHVAT_POINTS) > (1.0-(robot->AsRobot()->GetHitPoint()*1.1f)/robot->AsRobot()->GetMaxHitPoint())) return false;
                //}
            }
        }

        return !IsAutomaticMode() && ((m_Side != PLAYER_SIDE) || (g_MatrixMap->GetPlayerSide()->GetArcadedObject() != this));
    }

    void        OBBToAABBCollision(int nHeight, int nWidth);
	D3DXVECTOR3 LineToAABBIntersection(const D3DXVECTOR2 &s,const D3DXVECTOR2 &e, const D3DXVECTOR2 &vLu, const D3DXVECTOR2 &vLd, const D3DXVECTOR2 &vRu, const D3DXVECTOR2 &vRd, bool revers_x, bool revers_y);
	D3DXVECTOR3 CornerLineToAABBIntersection(const D3DXVECTOR2 &s, const D3DXVECTOR2 &e, const D3DXVECTOR2 &vLu, const D3DXVECTOR2 &vLd, const D3DXVECTOR2 &vRu, const D3DXVECTOR2 &vRd);
    D3DXVECTOR3 SphereRobotToAABBObstacleCollision( D3DXVECTOR3 &corr, const D3DXVECTOR3 &vel);
	//D3DXVECTOR3 SphereToAABBIntersection(const D3DXVECTOR2 &pos,float r, const D3DXVECTOR2 &vLu,const D3DXVECTOR2 &vLd,const D3DXVECTOR2 &vRu,const D3DXVECTOR2 &vRd, bool revers_x, bool revers_y);
    D3DXVECTOR3 RobotToObjectCollision(const D3DXVECTOR3 &vel, int ms);
    void        WallAvoid(const D3DXVECTOR3 &obstacle, const D3DXVECTOR3 &dest);
    static bool SphereToAABBCheck(const D3DXVECTOR2 &sphere_p, const D3DXVECTOR2 &vMin, const D3DXVECTOR2 &vMax, float &d, float &dx, float &dy);
    D3DXVECTOR3 SphereToAABB(const D3DXVECTOR2 &pos, const SMatrixMapMove *smm, const CPoint &cell, BYTE corner); //, bool revers_x, bool revers_y);

    void ZoneCurFind(void);		                    // Найти зону в которой находится робот (Out: m_ZoneCur)
	void ZonePathCalc(void);	                    // Рассчитать путь до m_ZoneDes (In: m_ZoneCur, m_ZoneDes) (Out: m_ZonePathCnt, m_ZonePath)
	void ZoneMoveCalc(void);                        // Рассчитать путь движения до ближайшей зоны (In: m_ZoneCur, m_ZoneNear) (Out: m_MovePathCnt, m_MovePathCur, m_MovePath)
    float CalcPathLength(void);
    //void ZoneMoveCalcTo(void);	                // Рассчитать путь в нутрии текущей зоны до точки назначения (In: m_DesX, m_DesY) (Out: m_MovePathCnt, m_MovePathCur, m_MovePath)
	void MoveByMovePath(int ms);	                // Двигаться по пути движения
    void MoveToRndBuilding();


    void CalcRobotMass();                           // вычисляет массу, скорость, силу робота
    bool Seek(const D3DXVECTOR3 &dest, bool &rotate, bool end_path, bool back = false); // поиск вектора смещения робота
	void RotateHull(const D3DXVECTOR3 &direction);  // поворот башни
    bool RotateRobot(const D3DXVECTOR3 &dest, float *rotateangle = NULL);      // поворот робота
	void Decelerate();                              // замедление
    void RobotWeaponInit();                         // инициализация оружия
    void HitTo(CMatrixMapStatic *hit, const D3DXVECTOR3 &pos);
    
    void RotateRobotLeft() { SETFLAG(m_ObjectState, ROBOT_FLAG_ROT_LEFT);  }
    void RotateRobotRight() { SETFLAG(m_ObjectState, ROBOT_FLAG_ROT_RIGHT); }


//Orders stack processing
    SOrder *AllocPlaceForOrderOnTop(void);  // make sure that order will bi initialized after this call
    //void AddOrderToEnd(const SOrder &order);

    void RemoveOrderFromTop(void)
    {
        RemoveOrder(0);
    }

    void RemoveOrder(int pos);
    void RemoveOrder(OrderType order);
    
    void ProcessOrdersList();

    bool HaveBomb(void) const;
    int HaveRepair(void) const { return m_HaveRepair; }

    //High orders
    void MoveToHigh(int mx, int my);

    //Orders
    void MoveTo(int mx, int my);
    void MoveToBack(int mx, int my);
    void MoveReturn(int mx, int my);
    void StopMoving();

    void Fire(const D3DXVECTOR3 &fire_pos, int type = 0); // type 0-fire 2-repair
    void StopFire(void);
    void BigBoom(int nc = -1);

    void CaptureBuilding(CMatrixBuilding *building);
    CMatrixBuilding *GetCaptureBuilding(void);
    void StopCapture();

    void TaktCaptureCandidate(int ms);
    void AddCaptureCandidate(CMatrixBuilding *b);
    void RemoveCaptureCandidate(CMatrixBuilding *b);
    void ClearCaptureCandidates(void);

    bool FindOrder(OrderType findOrder, CMatrixMapStatic *obj);
    bool FindOrderLikeThat(OrderType order) const;
    bool FindOrderLikeThat(OrderType order, OrderPhase phase);

    void BreakAllOrders();
    void UpdateOrder_MoveTo(int mx, int my);
    bool GetMoveToCoords(CPoint &pos);
    bool GetReturnCoords(CPoint &pos);

    void LowLevelStopFire(void);
    void LowLevelStop();
    bool FindWeapon(EWeapon type);

    void LowLevelMove(int ms, const D3DXVECTOR3 &dest, bool robot_coll, bool obst_coll, bool end_path = true, bool back = false);
    void LowLevelDecelerate(int ms, bool robot_coll, bool obst_coll);

    void ReleaseMe();
    void GatherInfo(int ms);

    void GetLost(const D3DXVECTOR3 &v);
//

    void RobotSpawn(CMatrixBuilding *pBase);    // spawn робота		
    void DIPTakt(float ms);                     // death in progress takt

	virtual void LogicTakt(int cms);
    void PauseTakt(int cms);

#ifdef _DEBUG
    virtual void Draw(void);
#endif
    virtual bool Damage(EWeapon weap, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, int attacker_side, CMatrixMapStatic *attaker);

    friend class CMatrixRobot;

	CMatrixRobotAI(void);
	~CMatrixRobotAI();
};

__forceinline void SBotWeapon::Draw(CMatrixRobotAI *robot)
{
    if(m_Unit->m_WeaponRepairData) m_Unit->m_WeaponRepairData->Draw(robot->IsInterfaceDraw());
}

inline SMatrixPlace *GetPlacePtr(int no)
{
    if(no < 0) return NULL;
    return g_MatrixMap->m_RN.GetPlace(no);
}

__forceinline bool CMatrixRobotAI::PLIsInPlace(void) const
{
    CPoint ptp;

    if(FindOrderLikeThat(ROT_MOVE_TO)) return false;
    if(FindOrderLikeThat(ROT_MOVE_RETURN)) return false;

    if(m_Environment.m_Place >= 0)
    {
        ptp = GetPlacePtr(m_Environment.m_Place)->m_Pos;
    }
    else if(m_Environment.m_PlaceAdd.x >= 0)
    {
        ptp = m_Environment.m_PlaceAdd;
    }
    else return false;

    return (GetMapPosX() == ptp.x) && (GetMapPosY() == ptp.y);
}

