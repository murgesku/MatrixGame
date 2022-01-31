// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixMap.hpp"
#include "MatrixProgressBar.hpp"
#include "MatrixSoundManager.hpp"

#define BASE_FLOOR_Z        (-63.0f) //Высота подъёма готового юнита из недр базы (на визуальную глубину залегания подъемника не влияет)
#define BASE_FLOOR_SPEED    0.0008f  //Скорость воспроизведения подъёма лифта и юнита на нём (так можно ускорить спавн)
#define MAX_ZAHVAT_POINTS   14

#define BUILDING_EXPLOSION_PERIOD_SND_1    100
#define BUILDING_EXPLOSION_PERIOD_SND_2    500
#define BUILDING_EXPLOSION_PERIOD          10
#define BUILDING_EXPLOSION_TIME            1000
#define BUILDING_BASE_EXPLOSION_TIME       2000

#define RESOURCES_INCOME            10
#define RESOURCES_INCOME_BASE       3

#define GATHERING_POINT_SHOW_TIME   1650

#define BUILDING_SELECTION_SIZE     50

#define CAPTURE_RADIUS              50
#define CAPTURE_SEEK_ROBOT_PERIOD   500

#define DISTANCE_CAPTURE_ME         300

#define MAX_PLACES                  4

//#define RES_TITAN_PERIOD       10000
//#define RES_PLASMA_PERIOD      5000
//#define RES_ENERGY_PERIOD      15000
//#define RES_ELECTRO_PERIOD     8500
//
//#define ROBOT_BUILD_TIME       5000
//#define FLYER_BUILD_TIME       5000
//#define TURRET_BUILD_TIME      5000

#define MAX_STACK_UNITS         6

enum EFlyerKind;

enum EBuildingTurrets
{
    BASE_TURRETS = 4,
    TITAN_TURRETS = 4,
    PLASMA_TURRETS = 4,
    ELECTRONIC_TURRETS = 4,
    ENERGY_TURRETS = 4,
    REPAIR_TURRETS = 4,

    EBuildingTurrets_FORCE_DWORD = 0x7FFFFFFF 
};

enum EBuildingType
{
    BUILDING_BASE = 0,
    BUILDING_TITAN = 1,
    BUILDING_PLASMA = 2,
    BUILDING_ELECTRONIC = 3,
    BUILDING_ENERGY = 4,
    BUILDING_REPAIR = 5,

    EBuildingType_FORCE_DWORD = 0x7FFFFFFF 
};

enum EBaseState
{
	BASE_CLOSED,
	BASE_OPENING,
    BASE_OPENED,
	BASE_CLOSING,

    BUILDING_DIP,
    BUILDING_DIP_EXPLODED,

    EBaseState_FORCE_DWORD = 0x7FFFFFFF 
};

enum ECaptureStatus
{
    CAPTURE_DONE,       // захват данным цветом завершен
    CAPTURE_IN_PROGRESS,// в процессе
    CAPTURE_TOO_FAR,    // робот далеко. подъедь ближе
    CAPTURE_BUSY,       // база занята делами. обратитесь позже
};

struct STrueColor
{
    int m_ColoredCnt;
    DWORD m_Color;
    STrueColor()
    {
        m_ColoredCnt = 0;
        m_Color = 0;
    }
};

struct SResource
{
    EBuildingType   m_Type;
    int             m_Amount;
    //int           m_BaseRCycle;
};

class CBuildStack : public CMain
{
    int                 m_Items;
    int                 m_Timer;
    CMatrixMapStatic   *m_Top;
    CMatrixMapStatic   *m_Bottom;
    CMatrixBuilding    *m_ParentBase;
    CMatrixProgressBar  m_PB;
public:
    void AddItem(CMatrixMapStatic *item);
    int DeleteItem(int no);
    void DeleteItem(CMatrixMapStatic *item);
    void ClearStack();
    CMatrixMapStatic *GetTopItem()                                  { return m_Top; }
    void TickTimer(int ms);                                          
    int GetItemsCnt(void) const                                     { return m_Items; }

    //void SetParentBase(CMatrixBuilding* base)                       { m_ParentBase = base; }
    CMatrixBuilding* GetParentBase()                                { return m_ParentBase; }
    bool IsMaxItems()                                               { return m_Items >= MAX_STACK_UNITS; }

    void ReturnRobotResources(CMatrixRobotAI* robot);
    void ReturnTurretResources(CMatrixCannon* turret);

    int GetRobotsCnt(void) const;
    void KillBar(void);
    
    CBuildStack(CMatrixBuilding *base):m_ParentBase(base)           { m_Top = NULL; m_Bottom = NULL; m_Items = 0; m_Timer = 0;}
    ~CBuildStack();
};

struct STurretPlace
{
    CPoint  m_Coord;
    float   m_Angle;
    ///float   m_AddH; // no need this

    // dynamic members
    int     m_CannonType;
};

struct SGatheringPoint
{
    SGatheringPoint() : IsSet(false) {}

    bool    IsSet;

    int     CoordX;
    int     CoordY;

    CPoint  Coords;
};

class CMatrixBuilding : public CMatrixMapStatic 
{
    union
    {
        struct //dip
        {
            int m_NextExplosionTime;
            int m_NextExplosionTimeSound;
        };
        struct 
        {
            int m_ResourcePeriod;
            SEffectHandler *m_PlacesShow;
        };
    };

    CMatrixEffectSelection *m_Selection;
    int m_UnderAttackTime;
    int m_CaptureMeNextTime;
    int m_CtrlGroup;

public:
    D3DXVECTOR3     m_TopPoint;
    CWStr           m_Name;
    int             m_defHitPoint;
    CBuildStack     m_BS;

    //Точка сбора для базы
    SGatheringPoint m_GatheringPoint;
    int m_ShowGatheringPointTime;

    bool GatheringPointIsSet()
    {
        return m_GatheringPoint.IsSet;
    }
    void SetGatheringPoint(int x, int y)
    {
        m_GatheringPoint.IsSet = true;
        //Чистые X и Y здесь нужны для расчёта точки отрисовки сборочного пункта
        m_GatheringPoint.CoordX = x;
        m_GatheringPoint.CoordY = y;
        //А в координаты построенным роботам передаётся уже вот это значение
        //Так запоминаются координаты для AssignPlace > GetRegion
        m_GatheringPoint.Coords = CPoint(x / GLOBAL_SCALE_MOVE, y / GLOBAL_SCALE_MOVE);
        //А так запоминаются координаты для PGOrderMoveTo
        //m_GatheringPoint.Coords = CPoint(x - ROBOT_MOVECELLS_PER_SIZE / 2, y - ROBOT_MOVECELLS_PER_SIZE / 2);
        m_ShowGatheringPointTime = g_MatrixMap->GetTime();
        //Заносим указатель на здание в массив для перебора и отрисовки всех установленных точек сбора
        g_MatrixMap->AddGPoint(this);
    }
    CPoint GetGatheringPoint()
    {
        if(GatheringPointIsSet()) return m_GatheringPoint.Coords;
    }
    void ClearGatheringPoint()
    {
        //Убираем маркер наличия точки сбора у здания
        m_GatheringPoint.IsSet = false;
        //И удаляем указатель на это здание из массива, перебирающего все здания со сборными точками для их отрисовки
        g_MatrixMap->RemoveGPoint(this);
    }
    void CMatrixBuilding::ShowGatheringPointTakt(int cms);

    D3DXVECTOR2 m_Pos;
	int m_Angle;


	int m_Side;		// 1-8

	EBuildingType m_Kind;
    EBuildingTurrets    m_TurretsMax;
    int                 m_TurretsHave;
    STurretPlace        m_TurretsPlaces[MAX_PLACES];
    int                 m_TurretsPlacesCnt;


    bool HaveMaxTurrets(void)  const          { return m_TurretsHave >= (int)m_TurretsMax; }

    bool CanBeCaptured(void) const {return !FLAG(m_ObjectState, BUILDING_CAPTURE_IN_PROGRESS);}
    bool IsCaptured(void) const {return FLAG(m_ObjectState, BUILDING_CAPTURE_IN_PROGRESS);}
    void SetCapturedBy(CMatrixMapStatic* ms)
    {
        SETFLAG(m_ObjectState, BUILDING_CAPTURE_IN_PROGRESS);
        m_Capturer = ms;
    }
    void ResetCaptured(void)
    {
        RESETFLAG(m_ObjectState, BUILDING_CAPTURE_IN_PROGRESS);
        m_Capturer = NULL;
    }

    int  GetCtrlGroup() { return m_CtrlGroup; }
    void SetCtrlGroup(int group) { m_CtrlGroup = group; }

    float m_BaseFloor;
    float m_BuildZ;

	CVectorObjectGroup *m_GGraph;
	CMatrixShadowProj *m_ShadowProj;

    EBaseState m_State; 

    CMatrixEffectZahvat *m_capture;
    STrueColor m_TrueColor;
    ECaptureStatus Capture(CMatrixRobotAI *by);

    int m_InCaptureTime;
    union
    {
        int m_InCaptureNextTimeErase;
        int m_CaptureNextTimeRollback;
    };
    int                 m_InCaptureNextTimePaint;
    int                 m_CaptureSeekRobotNextTime;
    CMatrixMapStatic   *m_Capturer; // used only for check

    // hitpoint
    CMatrixProgressBar m_PB;
    int         m_ShowHitpointTime;
    float       m_HitPoint;
    float       m_HitPointMax;
    float       m_MaxHitPointInversed; // for normalized calcs


    //void SetResourceAmount(int amount)
    //{
    //    m_ResourceAmount = amount;
    //}

    int GetStackRobots(void) const        { return m_BS.GetRobotsCnt();}

	EShadowType     m_ShadowType; // 0-off 1-proj 2-proj with anim 3-stencil
    int             m_ShadowSize; // texture size for proj


	CMatrixBuilding(void);
	~CMatrixBuilding();
    
    //Вызов подкрепления
    void Maintenance(void);

    //Постройка вертолёта
    bool BuildFlyer(EFlyerKind kind);
    
    //Открывается сборочная камера и поднимается подъёмник с готовым роботом/вертолётом
    void Open(void)
    {
        if(m_State == BUILDING_DIP || m_State == BUILDING_DIP_EXPLODED) return;
        m_State = BASE_OPENING;
        CSound::AddSound(S_DOORS_OPEN, GetGeoCenter());
        CSound::AddSound(S_PLATFORM_UP, GetGeoCenter());
    }
    //Сборочная камера закрывается, подъёмник опускается
    void Close(void)
    {
        if(m_State == BUILDING_DIP || m_State == BUILDING_DIP_EXPLODED) return;
        if(FLAG(m_ObjectState, BUILDING_SPAWNBOT)) return; // cannot close while spawn
        m_State = BASE_CLOSING;
        CSound::AddSound(S_DOORS_CLOSE, GetGeoCenter());
        CSound::AddSound(S_PLATFORM_DOWN, GetGeoCenter());
    }
    EBaseState State(void) const
    {
        return m_State;
    }

    bool IsSpawningBot(void) const { return FLAG(m_ObjectState, BUILDING_SPAWNBOT); };
    void ResetSpawningBot(void) { RESETFLAG(m_ObjectState, BUILDING_SPAWNBOT); };
    void SetSpawningBot(void) { SETFLAG(m_ObjectState, BUILDING_SPAWNBOT); };

    void ShowHitpoint(void)
    {
        m_ShowHitpointTime = HITPOINT_SHOW_TIME;
    }
    void InitMaxHitpoint(float hp)
    {
        m_HitPoint = hp;
        m_HitPointMax = hp;
        m_MaxHitPointInversed = 1.0f / hp;
    }
    float GetMaxHitPoint()
    {
        return m_HitPointMax / 10;
    }
    float GetHitPoint()
    {
        return m_HitPoint / 10;
    }
    void ReleaseMe();

    void SetNeutral(void);

    D3DXVECTOR3 GetPlacePos(void) const { return m_GGraph->GetPosByName(MATRIX_BASE_PLACE); }

	virtual void RNeed(dword need); // Запрашиваем нужные ресурсы объекта

	virtual void Takt(int cms);

	virtual void LogicTakt(int cms);
    void PauseTakt(int cms);

    float GetFloorZ(void);

    void CreateProgressBarClone(float x, float y, float width, EPBCoord clone_type);
    void DeleteProgressBarClone(EPBCoord clone_type);

    bool Select(void);
    void UnSelect(void);

	virtual bool Pick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, float *outt) const;

    virtual bool Damage(EWeapon weap, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, int attacker_side, CMatrixMapStatic* attaker);
	virtual void BeforeDraw(void);
	virtual void Draw(void);
	virtual void DrawShadowStencil(void);
	virtual void DrawShadowProj(void);

    virtual void FreeDynamicResources(void);

	void OnLoad(void);

    virtual bool CalcBounds(D3DXVECTOR3 &omin, D3DXVECTOR3 &omax);

    virtual int  GetSide(void) const {return m_Side;};
    virtual bool NeedRepair(void) const {return m_HitPoint < m_HitPointMax;}
    virtual bool InRect(const CRect &rect) const;

    void    OnOutScreen(void);

    // STUB:
    int  GetPlacesForTurrets(CPoint *places);
    void CreatePlacesShow();
    void DeletePlacesShow();
};

__forceinline bool CMatrixMapStatic::IsBase(void) const
{
    if(GetObjectType() == OBJECT_TYPE_BUILDING)
    {
        if(((CMatrixBuilding*)this)->m_Kind == BUILDING_BASE) return true;
    }
    return false;
}

__forceinline bool CMatrixMapStatic::IsBuildingAlive(void) const
{
    return IsBuilding() && ((CMatrixBuilding*)this)->m_State!=BUILDING_DIP && ((CMatrixBuilding*)this)->m_State != BUILDING_DIP_EXPLODED;
}