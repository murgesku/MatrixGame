// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "CReminder.hpp"
#include "VectorObject.hpp"

#include <utils.hpp>

class CMatrixMapGroup;
typedef CMatrixMapGroup *PCMatrixMapGroup;

//#define SHOW_ASSIGNED_GROUPS    6
//#define SHOW_ASSIGNED_GROUPS    3

// #include "Effects/MatrixEffect.hpp"
// #include "Effects/MatrixEffectWeapon.hpp"

// Ресурс объекта
#define MR_Graph         SETBIT(0)  // Графика
#define MR_Matrix        SETBIT(1)  // Матрица
#define MR_Pos           SETBIT(2)  // Положение
#define MR_Rotate        SETBIT(3)  // Ориентация
#define MR_ShadowStencil SETBIT(4)  // Стенсильные тени
//#define MR_ShadowProj           SETBIT(5)       // Проекционные тени
#define MR_ShadowProjGeom SETBIT(6)  // Проекционные тени: геометрия
#define MR_ShadowProjTex  SETBIT(7)  // Проекционные тени: текстуры
#define MR_MiniMap        SETBIT(8)  // Присутствие на миникарте

//#define MR_GraphSort          SETBIT(1)          // Сортировка графики

typedef enum {
    OBJECT_TYPE_EMPTY = 0,
    OBJECT_TYPE_MAPOBJECT = 2,
    OBJECT_TYPE_ROBOTAI = 3,
    OBJECT_TYPE_BUILDING = 4,
    OBJECT_TYPE_CANNON = 5,
    OBJECT_TYPE_FLYER = 6,

    EObjectType_FORCE_DWORD = 0x7FFFFFFF
} EObjectType;

#define MAX_OBJECTS_PER_SCREEN 2560

#define UNDER_ATTACK_IDLE_TIME 120000

#define OBJECT_STATE_ABLAZE    SETBIT(0)  // горит
#define OBJECT_STATE_SHORTED   SETBIT(1)  // закорочен
#define OBJECT_STATE_INVISIBLE SETBIT(2)  // невидимый
#define OBJECT_STATE_INTERFACE SETBIT(3)  // рисуется в интерфейсе

#define OBJECT_STATE_INVULNERABLE    SETBIT(3)
#define OBJECT_STATE_SHADOW_SPECIAL  SETBIT(4)  // параметры для русчета тени загружаются
#define OBJECT_STATE_TRACE_INVISIBLE SETBIT(5)  // объект невидим для TRACE_SKIP_INVISIBLE объектов
#define OBJECT_STATE_DIP             SETBIT(6)  // используется в StaticDelete

// comon flags

// only mesh flags
#define OBJECT_STATE_BURNED           SETBIT(10)  // сгоревший (для мешей)
#define OBJECT_STATE_EXPLOSIVE        SETBIT(11)  // ломается со взрывом (для мешей)
#define OBJECT_STATE_NORMALIZENORMALS SETBIT(12)  // нормализовывать нормали (для мешей)
#define OBJECT_STATE_SPECIAL          SETBIT(13)  // специальный объект: смерть требуется для победы
#define OBJECT_STATE_TERRON_EXPL      SETBIT(14)  // террон взрывается
#define OBJECT_STATE_TERRON_EXPL1     SETBIT(15)  // террон взрывается 1
#define OBJECT_STATE_TERRON_EXPL2     SETBIT(16)  // террон взрывается 2

// only cannon flags. use its values for other objects
#define OBJECT_CANNON_REF_PROTECTION     SETBIT(10)
#define OBJECT_CANNON_REF_PROTECTION_HIT SETBIT(11)

// only robots flags. use its values for other objects
#define ROBOT_FLAG_MOVING_BACK    SETBIT(10)
#define ROBOT_FLAG_COLLISION      SETBIT(11)  // if collision, pneumatic chassis does not steps well
#define ROBOT_FLAG_SGROUP         SETBIT(12)
#define ROBOT_FLAG_SARCADE        SETBIT(13)
#define ROBOT_FLAG_ONWATER        SETBIT(14)
#define ROBOT_FLAG_LINKED         SETBIT(15)
#define ROBOT_FLAG_DISABLE_MANUAL SETBIT(16)

#define ROBOT_FLAG_ROT_LEFT    SETBIT(17)
#define ROBOT_FLAG_ROT_RIGHT   SETBIT(18)
#define ROBOT_CRAZY            SETBIT(19)  // easter egg :) crazy bot
#define ROBOT_FLAG_INPOSITION  SETBIT(20)  // easter egg
#define ROBOT_MUST_DIE_FLAG    SETBIT(21)
#define ROBOT_CAPTURE_INFORMED SETBIT(22)

// only buildings flags. use its values for other objects
#define BUILDING_NEW_INCOME          SETBIT(10)
#define BUILDING_SPAWNBOT            SETBIT(11)  // opening for bot spawn
#define BUILDING_CAPTURE_IN_PROGRESS SETBIT(12)

#define OBJECT_ABLAZE_PERIOD              90
#define OBJECT_ROBOT_ABLAZE_PERIOD        10
#define OBJECT_ROBOT_ABLAZE_PERIOD_EFFECT 90
#define OBJECT_SHORTED_PERIOD             50

#define HITPOINT_SHOW_TIME 1000

typedef enum {
    SHADOW_OFF = 0,
    SHADOW_PROJ_STATIC = 1,
    SHADOW_PROJ_DYNAMIC = 2,
    SHADOW_STENCIL = 3,

    EShadowType_FORCE_DWORD = 0x7FFFFFFF
} EShadowType;

#define MAX_GROUPS_PER_OBJECT 36

class CTemporaryLoadData;

enum {
    OBJ_RENDER_ORDINAL,
    OBJ_RENDER_ORDINAL_GLOSS,
    OBJ_RENDER_SIDE,
    OBJ_RENDER_SIDE_GLOSS,

    OBJ_RENDER_TYPES_CNT
};

class CMatrixMapStatic;

#ifdef _DEBUG
#include "stdio.h"
#endif

struct SObjectCore {
#ifdef _DEBUG
    SDebugCallInfo m_dci;
#endif

    D3DXMATRIX m_Matrix;
    D3DXMATRIX m_IMatrix;  // inversed matrix
    float m_Radius;
    D3DXVECTOR3 m_GeoCenter;
    EObjectType m_Type;  // 0-empty 2-CMatrixMapObject 3-CMatrixRobotAI 4-CMatrixBuilding 5-CMatrixCannon
    DWORD m_TerainColor;
    int m_Ref;

    CMatrixMapStatic *m_Object;

    static SObjectCore *Create(CMatrixMapStatic *obj) {
        SObjectCore *c = (SObjectCore *)HAlloc(sizeof(SObjectCore), g_MatrixHeap);
#ifdef _DEBUG
        c->m_dci._file = NULL;
        c->m_dci._line = -1;
#endif
        c->m_Object = obj;
        c->m_Ref = 1;
        D3DXMatrixIdentity(&c->m_Matrix);

        //#ifdef _DEBUG
        //
        //        FILE *f = fopen("Errors\\"+CStr(int(c))+".log","a");
        //        fwrite("create\n", strlen("create\n"), 1, f);
        //        fclose(f);
        //
        //#endif

        return c;
    }

    void RefInc(void) { ++m_Ref; }
    void RefDec(void) {
        //#ifdef _DEBUG
        //
        //        FILE *f = fopen("Errors\\"+CStr(int(this))+".log","a");
        //        fwrite("release\n", strlen("release\n"), 1, f);
        //        fclose(f);
        //
        //#endif
        --m_Ref;
        if (m_Ref <= 0) {
#ifdef _DEBUG
            if (m_Ref < 0)
                debugbreak();

            DeleteFile(utils::format("Errors\\%x.log", this).c_str());
#endif
            HFree(this, g_MatrixHeap);
        }
    }
    void Release(void) { RefDec(); }
};

bool FreeObjResources(uintptr_t user);

struct SRenderTexture {
    ETexSize ts;
    CTextureManaged *tex;
};

enum EWeapon : unsigned int;

class CMatrixRobotAI;
class CMatrixCannon;
class CMatrixBuilding;
class CMatrixFlyer;

class CMatrixMapStatic;
typedef CMatrixMapStatic *PCMatrixMapStatic;
class CMatrixMapStatic : public CMain {
    SRemindCore m_RemindCore;

    int m_ObjectStateTTLAblaze;
    int m_ObjectStateTTLShorted;

    int m_Z;

    static PCMatrixMapStatic objects[MAX_OBJECTS_PER_SCREEN];
    static int objects_left;
    static int objects_rite;

    static CMatrixMapStatic *m_FirstVisNew;
    static CMatrixMapStatic *m_LastVisNew;

    static CMatrixMapStatic *m_FirstVisOld;
    static CMatrixMapStatic *m_LastVisOld;

    CMatrixMapStatic *m_NextVis;
    CMatrixMapStatic *m_PrevVis;

protected:
    struct SEVH_data {
        D3DXMATRIX m;
        const CRect *rect;
        bool found;
    };

    DWORD m_ObjectState;  // битовый набор.
    DWORD m_RChange;  // Какой ресурс объекта изменился. При созданнии класса устанавливается в 0xffffffff

    SObjectCore *m_Core;

    void SetShortedTTL(int ttl) { m_ObjectStateTTLShorted = ttl; };
    void SetAblazeTTL(int ttl) { m_ObjectStateTTLAblaze = ttl; };
    int GetShortedTTL(void) { return m_ObjectStateTTLShorted; }
    int GetAblazeTTL(void) { return m_ObjectStateTTLAblaze; }
    bool IsAblaze(void) const { return FLAG(m_ObjectState, OBJECT_STATE_ABLAZE); }
    bool IsShorted(void) const { return FLAG(m_ObjectState, OBJECT_STATE_SHORTED); }
    void MarkAblaze(void) { SETFLAG(m_ObjectState, OBJECT_STATE_ABLAZE); }
    void MarkShorted(void) { SETFLAG(m_ObjectState, OBJECT_STATE_SHORTED); }
    void UnmarkAblaze(void) { RESETFLAG(m_ObjectState, OBJECT_STATE_ABLAZE); }
    void UnmarkShorted(void) { RESETFLAG(m_ObjectState, OBJECT_STATE_SHORTED); }

    static CMatrixMapStatic *m_FirstLogicTemp;
    static CMatrixMapStatic *m_LastLogicTemp;
    CMatrixMapStatic *m_PrevLogicTemp;
    CMatrixMapStatic *m_NextLogicTemp;

    PCMatrixMapGroup m_InGroups[MAX_GROUPS_PER_OBJECT];  // max size of object is MAX_GROUPS_PER_OBJECT groups
    int m_InCnt;

    float m_CamDistSq;  // квадрат растояния до камеры. вычисляется только в графическом такте.

    int m_NearBaseCnt;

    // for visibility
    void WillDraw(void) {
        // TODO : доделать!!!
        return;
        // remove from previous list of visibility
        LIST_DEL(this, m_FirstVisOld, m_LastVisOld, m_PrevVis, m_NextVis);
        // add to new list of visibility
        LIST_ADD(this, m_FirstVisNew, m_LastVisNew, m_PrevVis, m_NextVis);
    }

public:
    CMatrixMapStatic *m_NextStackItem;
    CMatrixMapStatic *m_PrevStackItem;

    bool IsNotOnMinimap(void) const { return FLAG(m_RChange, MR_MiniMap); }
    void SetInvulnerability(void) { SETFLAG(m_ObjectState, OBJECT_STATE_INVULNERABLE); }
    void ResetInvulnerability(void) { RESETFLAG(m_ObjectState, OBJECT_STATE_INVULNERABLE); }
    bool IsInvulnerable(void) const { return FLAG(m_ObjectState, OBJECT_STATE_INVULNERABLE); }
    bool IsTraceInvisible(void) const { return FLAG(m_ObjectState, OBJECT_STATE_TRACE_INVISIBLE); }
    bool IsSpecial(void) const { return FLAG(m_ObjectState, OBJECT_STATE_SPECIAL); }

    bool IsDIP(void) const { return FLAG(m_ObjectState, OBJECT_STATE_DIP); }
    void SetDIP(void) { SETFLAG(m_ObjectState, OBJECT_STATE_DIP); }

    static void StaticInit(void) {
        m_FirstLogicTemp = NULL;
        m_LastLogicTemp = NULL;
        objects_left = MAX_OBJECTS_PER_SCREEN >> 1;
        objects_rite = MAX_OBJECTS_PER_SCREEN >> 1;

        m_FirstVisNew = NULL;
        m_LastVisNew = NULL;

        m_FirstVisOld = NULL;
        m_LastVisOld = NULL;
    }
#ifdef _DEBUG
    static void ValidateAfterReset(void) {
        if (m_FirstLogicTemp || m_LastLogicTemp)
            debugbreak();
        if (objects_left != objects_rite)
            debugbreak();
        if (m_FirstVisNew || m_LastVisNew)
            debugbreak();
        if (m_FirstVisOld || m_LastVisOld)
            debugbreak();
    }
#endif

    void SetTerainColor(DWORD color) { m_Core->m_TerainColor = color; }

    void SetVisible(bool flag) { INITFLAG(m_ObjectState, OBJECT_STATE_INVISIBLE, !flag); }
    bool IsVisible(void) const { return !FLAG(m_ObjectState, OBJECT_STATE_INVISIBLE); }
    void SetInterfaceDraw(bool flag) { INITFLAG(m_ObjectState, OBJECT_STATE_INTERFACE, flag); }
    bool IsInterfaceDraw(void) const { return FLAG(m_ObjectState, OBJECT_STATE_INTERFACE); }

    PCMatrixMapGroup GetGroup(int n) { return m_InGroups[n]; }
    int GetGroupCnt(void) { return m_InCnt; }

    D3DXVECTOR3 m_AdditionalPoint;  // to check visibility with shadows

    DWORD m_LastVisFrame;
    int m_IntersectFlagTracer;
    int m_IntersectFlagFindObjects;

#pragma warning(disable : 4355)
    CMatrixMapStatic(void)
      : CMain(), m_RChange(0xffffffff), m_LastVisFrame(0xFFFFFFFF), m_NearBaseCnt(0), m_IntersectFlagTracer(0xFFFFFFFF),
        m_IntersectFlagFindObjects(0xFFFFFFFF), m_ObjectState(0), m_ObjectStateTTLAblaze(0), m_ObjectStateTTLShorted(0),
        m_InCnt(0), m_PrevLogicTemp(NULL), m_NextLogicTemp(NULL), m_RemindCore(FreeObjResources, reinterpret_cast<uintptr_t>(this)) {
        m_Core = SObjectCore::Create(this);

        m_Core->m_TerainColor = 0xFFFFFFFF;
        m_Core->m_Type = OBJECT_TYPE_EMPTY;

        memset(m_InGroups, 0, sizeof(m_InGroups));

        m_NextStackItem = NULL;
        m_PrevStackItem = NULL;

        m_NextVis = NULL;
        m_PrevVis = NULL;

        m_CamDistSq = 10000.0f;
    }
#pragma warning(default : 4355)

    ~CMatrixMapStatic();

    static CMatrixMapStatic *GetFirstLogic(void) { return m_FirstLogicTemp; }
    static CMatrixMapStatic *GetLastLogic(void) { return m_LastLogicTemp; }
    CMatrixMapStatic *GetNextLogic(void) { return m_NextLogicTemp; }
    CMatrixMapStatic *GetPrevLogic(void) { return m_PrevLogicTemp; }

    bool IsBase(void) const;
    inline bool IsRobot(void) const { return GetObjectType() == OBJECT_TYPE_ROBOTAI; };
    bool IsLiveRobot(void) const;
    inline bool IsBuilding(void) const { return GetObjectType() == OBJECT_TYPE_BUILDING; };
    bool IsLiveBuilding(void) const;
    inline bool IsCannon(void) const { return GetObjectType() == OBJECT_TYPE_CANNON; };
    bool IsLiveCannon(void) const;
    bool IsLiveActiveCannon(void) const;

    inline bool IsFlyer(void) const { return GetObjectType() == OBJECT_TYPE_FLYER; };
    inline bool IsUnit(void) const { return IsRobot() || IsCannon(); }

    inline bool IsLive(void) const { return IsLiveRobot() || IsLiveCannon() || IsLiveBuilding(); }
    //{
    //    if(obj->GetObjectType()==OBJECT_TYPE_ROBOTAI) return obj->AsRobot()->m_CurrState!=ROBOT_DIP;// &&
    //    (obj->AsRobot()->GetSide()!=PLAYER_SIDE || !obj->AsRobot()->IsSelected()) &&
    //    (g_MatrixMap->GetPlayerSide()->GetArcadedObject() != obj); else if(obj->GetObjectType()==OBJECT_TYPE_CANNON)
    //    return obj->AsCannon()->m_CurrState!=CANNON_DIP && obj->AsCannon()->m_CurrState!=CANNON_UNDER_CONSTRUCTION;
    //    else if(obj->GetObjectType()==OBJECT_TYPE_BUILDING) return (obj->AsBuilding()->m_State!=BUILDING_DIP) &&
    //    (obj->AsBuilding()->m_State!=BUILDING_DIP_EXPLODED); else return false;
    //}

    bool FitToMask(DWORD mask);

    inline CMatrixRobotAI *AsRobot(void) { return (CMatrixRobotAI *)this; }
    inline CMatrixCannon *AsCannon(void) { return (CMatrixCannon *)this; }
    inline CMatrixBuilding *AsBuilding(void) { return (CMatrixBuilding *)this; }
    inline CMatrixFlyer *AsFlyer(void) { return (CMatrixFlyer *)this; }

    inline bool IsNearBase(void) const { return m_NearBaseCnt != 0; }

    void RecalcTerainColor(void);
    inline DWORD GetTerrainColor(void) const { return m_Core->m_TerainColor; }

    void Sort(const D3DXMATRIX &sort);
    static void SortBegin(void);
    static void OnEndOfDraw(void);  // this must be called before any sorting stuf

    static void SortEndRecalcTerainColor(void);
    static void SortEndDraw(void);
    static void SortEndBeforeDraw(void);
    static void SortEndDrawShadowProj(void);
    static void SortEndDrawShadowStencil(void);
    static void SortEndGraphicTakt(int step);

    static void CalcDistances(void);

    static void RemoveFromSorted(CMatrixMapStatic *ms);

    static int GetVisObjCnt(void);
    static CMatrixMapStatic *GetVisObj(int i);

    inline EObjectType GetObjectType(void) const { return m_Core->m_Type; }
    inline const D3DXVECTOR3 &GetGeoCenter(void) const { return m_Core->m_GeoCenter; }
    inline const D3DXMATRIX &GetMatrix(void) const { return m_Core->m_Matrix; }
    inline float GetRadius(void) const { return m_Core->m_Radius; }

#ifdef _DEBUG
    SObjectCore *GetCore(const SDebugCallInfo &dci);
#else
#ifdef _TRACE
    inline SObjectCore *GetCore(const SDebugCallInfo&) {
        m_Core->RefInc();
        return m_Core;
    }
#else
    inline SObjectCore *GetCore(void) {
        m_Core->RefInc();
        return m_Core;
    }
#endif
#endif

    // logic temp: list of static objects, у которых временно вызывается логический такт
    inline bool InLT(void) {
        return (m_PrevLogicTemp != NULL) || (m_NextLogicTemp != NULL) || (this == m_FirstLogicTemp);
    }
    inline void AddLT(void) {
        if (!InLT()) {
            LIST_ADD(this, m_FirstLogicTemp, m_LastLogicTemp, m_PrevLogicTemp, m_NextLogicTemp);
        }
    }
    inline void DelLT(void) {
        if (InLT()) {
            LIST_DEL_CLEAR(this, m_FirstLogicTemp, m_LastLogicTemp, m_PrevLogicTemp, m_NextLogicTemp);
        }
    }

    static void ProceedLogic(int ms);

    inline void RChange(dword zn) { m_RChange |= zn; }
    inline void RNoNeed(dword zn) { m_RChange &= (~zn); }

    void StaticTakt(int ms);

    virtual void RNeed(dword need) = 0;  // Запрашиваем нужные ресурсы объекта

    virtual void Takt(int cms) = 0;
    virtual void LogicTakt(int cms) = 0;

    virtual bool Pick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, float *outt) const = 0;

    virtual bool Damage(EWeapon weap, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, int attacker_side,
                        CMatrixMapStatic *attaker) = 0;

    virtual void BeforeDraw(void) = 0;
    virtual void Draw(void) = 0;
    virtual void DrawShadowStencil(void) = 0;
    virtual void DrawShadowProj(void) = 0;
    virtual void FreeDynamicResources(void) = 0;

    void OnLoad(void);
    void Init(int ids);

    virtual bool CalcBounds(D3DXVECTOR3 &omin, D3DXVECTOR3 &omax) = 0;

    virtual int GetSide(void) const = 0;
    virtual bool NeedRepair(void) const = 0;

    static bool EnumVertsHandler(const SVOVertex &v, DWORD data);
    virtual bool InRect(const CRect &rect) const = 0;

    void JoinToGroup(void);
    void UnjoinGroup(void);

    bool RenderToTexture(SRenderTexture *rt, int n, /*float *fff=NULL,*/ float anglez = GRAD2RAD(30),
                         float anglex = GRAD2RAD(30), float fov = GRAD2RAD(60));

#ifdef SHOW_ASSIGNED_GROUPS
    void ShowGroups(void);
#endif
};

CVectorObjectAnim *LoadObject(const wchar *name, CHeap *heap, bool side = false, const wchar *texname = NULL);
void UnloadObject(CVectorObjectAnim *o, CHeap *heap);
