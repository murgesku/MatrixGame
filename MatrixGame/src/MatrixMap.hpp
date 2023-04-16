// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_MAP_HPP
#define MATRIX_MAP_HPP

#include "Common.hpp"
#include "MatrixConfig.hpp"
#include "MatrixGame.h"

#include "CStorage.hpp"

#include <vector>

//#define DRAW_LANDSCAPE_SETKA 1

#define FREE_TIME_PERIOD (5000)  // in ms

#define GLOBAL_SCALE      (20.0f)
#define MOVE_CNT          (2)
#define GLOBAL_SCALE_MOVE (GLOBAL_SCALE / MOVE_CNT)

#define MAX_ALWAYS_DRAW_OBJ 16

#define MatrixPathMoveMax 256

#define ROBOT_WEAPONS_PER_ROBOT_CNT 10
#define ROBOT_MOVECELLS_PER_SIZE    4  // размер стороны квадрата робота в ячейках сетки проходимости

#define DRAW_SHADOWS_DISTANCE_SQ ((1024) * (1024))

#define TRACE_LANDSCAPE SETBIT(0)
#define TRACE_WATER     SETBIT(1)
#define TRACE_ROBOT     SETBIT(2)
#define TRACE_BUILDING  SETBIT(3)
#define TRACE_OBJECT    SETBIT(4)
#define TRACE_CANNON    SETBIT(5)
#define TRACE_FLYER     SETBIT(6)
#define TRACE_ANYOBJECT (TRACE_OBJECT | TRACE_BUILDING | TRACE_ROBOT | TRACE_CANNON | TRACE_FLYER)

#define TRACE_OBJECTSPHERE   SETBIT(10)  // trace object as speres
#define TRACE_SKIP_INVISIBLE SETBIT(11)  // skip objects with flag OBJECT_STATE_TRACE_INVISIBLE while tracing

#define TRACE_ALL       (((DWORD)(-1)) & (~TRACE_OBJECTSPHERE))
#define TRACE_NONOBJECT (TRACE_LANDSCAPE | TRACE_WATER)

#define TRACE_STOP_NONE      ((CMatrixMapStatic *)0)
#define TRACE_STOP_LANDSCAPE ((CMatrixMapStatic *)1)
#define TRACE_STOP_WATER     ((CMatrixMapStatic *)2)

#define IS_TRACE_STOP_OBJECT(o) (((uintptr_t)o > 100))
#define IS_TRACE_UNIT(o)        ((o->GetObjectType() == OBJECT_TYPE_ROBOTAI) || (o->GetObjectType() == OBJECT_TYPE_FLYER))

#define SHADER_PERC 20
#define SHADER_TIME 500

#include "MatrixCamera.hpp"
#include "VectorObject.hpp"
#include "MatrixSide.hpp"
#include "MatrixMapGroup.hpp"
#include "MatrixMapStatic.hpp"
#include "MatrixWater.hpp"
#include "MatrixMapTexture.hpp"
#include "Effects/MatrixEffect.hpp"
#include "StringConstants.hpp"
#include "MatrixMinimap.hpp"
#include "MatrixConfig.hpp"
#include "MatrixCursor.hpp"
#include "MatrixDebugInfo.hpp"
#include "Logic/MatrixRoadNetwork.hpp"
#include "DevConsole.hpp"
#include "MatrixObjectRobot.hpp"
#include "MatrixFlyer.hpp"
#include "MatrixTransition.hpp"

inline bool CMatrixMapStatic::FitToMask(DWORD mask) {
    if (IsLiveRobot())
        return (mask & TRACE_ROBOT) != 0;
    if (IsLiveCannon())
        return (mask & TRACE_CANNON) != 0;
    if (IsLiveBuilding())
        return (mask & TRACE_BUILDING) != 0;
    if (GetObjectType() == OBJECT_TYPE_MAPOBJECT)
        return (mask & TRACE_OBJECT) != 0;
    if (IsFlyer())
        return (mask & TRACE_FLYER) != 0;
    return false;
}

typedef bool (*ENUM_OBJECTS)(const D3DXVECTOR3 &center, CMatrixMapStatic *o, DWORD user);
typedef bool (*ENUM_OBJECTS2D)(const D3DXVECTOR2 &center, CMatrixMapStatic *o, DWORD user);

class CMatrixBuilding;

struct SMatrixMapPoint {
    float z;
    float z_land;
    D3DXVECTOR3 n;
    DWORD color;
    int lum_r, lum_g, lum_b;
};

/*struct SMatrixMapZone {
    int m_BeginX,m_BeginY;			// Начало роста
    int m_CenterX,m_CenterY;		// Центр масс
    bool m_Building;				// В зоне есть здание
    int m_Size;						// Cnt in unit
    int m_Perim;					// Периметр
    CRect m_Rect;					// Bound zone

    int m_NearZoneCnt;				// Кол-во ближайших зон
    int m_NearZone[8];				// Ближайшие зоны
    int m_NearZoneConnectSize[8];	// Длина соединения с ближайшими зонами

    int m_FPLevel;
    int m_FPWt;
    int m_FPWtp;
};*/

struct SMatrixMapUnit {
private:
    DWORD m_TypeBits;

public:
    bool IsWater(void) const { return FLAG(m_TypeBits, CELLFLAG_WATER); }
    bool IsLand(void) const { return FLAG(m_TypeBits, CELLFLAG_LAND); }
    void SetType(DWORD t) { m_TypeBits = t; }
    bool IsFlat(void) const { return FLAG(m_TypeBits, CELLFLAG_FLAT); }
    bool IsBridge(void) const { return FLAG(m_TypeBits, CELLFLAG_BRIDGE); }
    bool IsInshore(void) const { return FLAG(m_TypeBits, CELLFLAG_INSHORE); }
    void ResetInshore(void) { RESETFLAG(m_TypeBits, CELLFLAG_INSHORE); }
    void ResetFlat(void) { RESETFLAG(m_TypeBits, CELLFLAG_FLAT); }
    void SetFlat(void) { SETFLAG(m_TypeBits, CELLFLAG_FLAT); }
    void SetBridge(void) { SETFLAG(m_TypeBits, CELLFLAG_BRIDGE); }

    bool IsDown(void) const { return FLAG(m_TypeBits, CELLFLAG_DOWN); }

    CMatrixBuilding *m_Base;

    // koefs for z calc (with bridge bridge)
    float a1, b1, c1;
    float a2, b2, c2;
};

struct SMatrixMapMove {
    int m_Zone;
    DWORD m_Sphere;
    DWORD m_Zubchik;

    int m_Find;
    int m_Weight;
    DWORD m_Stop;  // (1-нельзя пройти) 1-Shasi1(Пневматика) 2-Shasi2(Колеса) 4-Shasi3(Гусеницы) 8-Shasi4(Подушка)
                   // 16-Shasi5(Крылья)
                   // <<0-size 1       <<6-size 2       <<12-size 3       <<18-size 4        <<24-size 5

    BYTE GetType(int nsh) const {
        byte rv = 0;

        if (!(this->m_Stop & (1 << nsh)))
            return 0xff;

        if (this->m_Sphere & ((1 << nsh) << 0))
            rv |= 1;
        if (this->m_Sphere & ((1 << nsh) << 8))
            rv |= 2;
        if (this->m_Sphere & ((1 << nsh) << 16))
            rv |= 4;
        if (this->m_Sphere & ((1 << nsh) << 24))
            rv |= 8;

        if (this->m_Zubchik & ((1 << nsh) << 0))
            rv |= 16;
        if (this->m_Zubchik & ((1 << nsh) << 8))
            rv |= 32;
        if (this->m_Zubchik & ((1 << nsh) << 16))
            rv |= 64;
        if (this->m_Zubchik & ((1 << nsh) << 24))
            rv |= 128;

        return rv;
    }
};

struct SRobotWeaponMatrix {
    int cnt;
    int common;
    int extra;
    struct SW {
        int id;
        DWORD access_invert;  // high bit is invert flag
    } list[ROBOT_WEAPONS_PER_ROBOT_CNT];
};

struct SDifficulty {
    float k_damage_enemy_to_player;
    float k_time_before_maintenance;
    float k_friendly_fire;
};

struct SGroupVisibility {
    PCMatrixMapGroup *vis;
    int vis_cnt;
    float z_from;
    int *levels;
    int levels_cnt;

    void Release(void);
};

#define MMFLAG_OBJECTS_DRAWN SETBIT(0)
#define MMFLAG_FOG_ON        SETBIT(1)
#define MMFLAG_SKY_ON        SETBIT(2)
//#define MMFLAG_EFF_TAKT         SETBIT(3)   // effect takts loop is active. it causes SubEffect to do not delete
//effect immediately
#define MMFLAG_PAUSE                        SETBIT(4)
#define MMFLAG_DIALOG_MODE                  SETBIT(5)
#define MMFLAG_MOUSECAM                     SETBIT(6)
#define MMFLAG_NEEDRECALCTER                SETBIT(7)
#define MMFLAG_STAT_DIALOG                  SETBIT(8)
#define MMFLAG_STAT_DIALOG_D                SETBIT(9)
#define MMFLAG_ENABLE_CAPTURE_FUCKOFF_SOUND SETBIT(10)
#define MMFLAG_SOUND_BASE_SEL_ENABLED       SETBIT(11)
#define MMFLAG_SOUND_ORDER_ATTACK_DISABLE   SETBIT(12)
#define MMFLAG_SOUND_ORDER_CAPTURE_ENABLED  SETBIT(13)
#define MMFLAG_DISABLE_DRAW_OBJECT_LIGHTS   SETBIT(14)
#define MMFLAG_WIN                          SETBIT(15)
#define MMFLAG_MUSIC_VOL_CHANGING           SETBIT(16)
#define MMFLAG_TRANSITION                   SETBIT(17)
#define MMFLAG_VIDEO_RESOURCES_READY        SETBIT(18)  // VB's, IB's and Non managed textures

#define MMFLAG_MEGABUSTALREADY   SETBIT(19)  // easter egg
#define MMFLAG_ROBOT_IN_POSITION SETBIT(20)  // easter egg
#define MMFLAG_SHOWPORTRETS      SETBIT(21)  // easter egg

#define MMFLAG_DISABLEINSHORE_BUILD SETBIT(22)  // inshores can be enabled, but cannot be build in WaterBuild function
#define MMFLAG_AUTOMATIC_MODE       SETBIT(23)
#define MMFLAG_FLYCAM               SETBIT(24)
#define MMFLAG_FULLAUTO             SETBIT(25)

#define MMFLAG_TERRON_DEAD  SETBIT(26)
#define MMFLAG_TERRON_ONMAP SETBIT(27)

#define MMFLAG_SPECIAL_BROKEN SETBIT(28)

struct SSkyTex {
    CTextureManaged *tex;
    float u0, v0, u1, v1;
};

enum {
    SKY_FORE,
    SKY_BACK,
    SKY_LEFT,
    SKY_RITE,
};

class CMatrixHint;

class CMatrixMap : public CMain {
    std::vector<CMatrixMapStatic*> m_AllObjects;

protected:
    ~CMatrixMap();

private:
    struct SShadowRectVertex {
        D3DXVECTOR4 p;
        DWORD color;
    };

    enum EReloadStep {
        RS_SIDEAI,
        RS_RESOURCES,
        RS_MAPOBJECTS,
        RS_BUILDINGS,
        RS_CANNONS,
        RS_ROBOTS,
        RS_EFFECTS,
        RS_CAMPOS,

    };

public:
    DWORD m_Flags;

    CMatrixHint *m_PauseHint;

    std::vector<CMatrixHint*> m_DialogModeHints;
    const wchar *m_DialogModeName;

    int m_TexUnionDim;
    int m_TexUnionSize;

    float m_SkyHeight;

    CPoint m_Size;
    CPoint m_SizeMove;
    SMatrixMapUnit *m_Unit;
    SMatrixMapPoint *m_Point;
    SMatrixMapMove *m_Move;

    CMatrixRoadNetwork m_RN;

    CPoint m_GroupSize;
    CMatrixMapGroup **m_Group;
    SGroupVisibility *m_GroupVis;

    CDevConsole m_Console;
    CMatrixDebugInfo m_DI;

    SRobotWeaponMatrix m_RobotWeaponMatrix[ROBOT_ARMOR_CNT];

    int m_BeforeWinCount;

    float m_GroundZ;
    float m_GroundZBase;
    float m_GroundZBaseMiddle;
    float m_GroundZBaseMax;
    // D3DXPLANE m_ShadowPlaneCut;
    // D3DXPLANE m_ShadowPlaneCutBase;
    D3DXVECTOR3 m_LightMain;

    CMatrixWater *m_Water;

    int m_IdsCnt;
    std::wstring *m_Ids;

    CMatrixSideUnit *m_PlayerSide;
    CMatrixSideUnit *m_Side;
    int m_SideCnt;

    PCMatrixEffect m_EffectsFirst;
    PCMatrixEffect m_EffectsLast;
    PCMatrixEffect m_EffectsNextTakt;
    int m_EffectsCnt;
    // CDWORDMap         m_Effects;

    CEffectSpawner *m_EffectSpawners;
    int m_EffectSpawnersCnt;

    CMatrixMapStatic *m_NextLogicObject;

    std::wstring m_WaterName;

    float m_BiasCannons;
    float m_BiasRobots;
    float m_BiasBuildings;
    float m_BiasTer;
    float m_BiasWater;

    float m_CameraAngle;
    float m_WaterNormalLen;

    float m_Terrain2ObjectInfluence;
    DWORD m_Terrain2ObjectTargetColor;
    DWORD m_WaterColor;
    DWORD m_SkyColor;
    DWORD m_InshorewaveColor;
    DWORD m_AmbientColor;
    DWORD m_AmbientColorObj;
    DWORD m_LightMainColor;
    DWORD m_LightMainColorObj;
    DWORD m_ShadowColor;
    float m_LightMainAngleZ, m_LightMainAngleX;

    int m_MacrotextureSize;
    CTextureManaged *m_Macrotexture;

    CTextureManaged *m_Reflection;

    SSkyTex m_SkyTex[4];
    float m_SkyAngle;
    float m_SkyDeltaAngle;

    CTransition m_Transition;

    CMinimap m_Minimap;

    int m_KeyDown;
    int m_KeyScan;

    CMatrixCursor m_Cursor;

    int m_IntersectFlagTracer;
    int m_IntersectFlagFindObjects;

    D3DXVECTOR3 m_MouseDir;  // world direction to mouse cursor
    // trace stop!
    D3DXVECTOR3 m_TraceStopPos;
    CMatrixMapStatic *m_TraceStopObj;

    bool IsTraceNonPlayerObj();
    PCMatrixMapStatic m_AD_Obj[MAX_ALWAYS_DRAW_OBJ];  // out of land object // flyer
    int m_AD_Obj_cnt;

    CMatrixCamera m_Camera;

    CDeviceState *m_DeviceState;

    SDifficulty m_Difficulty;

protected:
    D3D_VB m_ShadowVB;

    DWORD m_NeutralSideColor;
    DWORD m_NeutralSideColorMM;
    CTexture *m_NeutralSideColorTexture;

    D3DXMATRIX m_Identity;

    float m_minz;
    float m_maxz;

    std::vector<D3DXVECTOR2> m_VisWater;

    CMatrixMapGroup **m_VisibleGroups;
    int m_VisibleGroupsCount;

    DWORD m_CurFrame;

    int m_Time;
    int m_PrevTimeCheckStatus;
    int m_BeforeWinLooseDialogCount;
    DWORD m_StartTime;

    // helicopter explosion
    int m_ShadeTime;
    int m_ShadeTimeCurrent;
    bool m_ShadeOn;
    D3DXVECTOR3 m_ShadePosFrom;
    D3DXVECTOR3 m_ShadePosTo;
    DWORD m_ShadeInterfaceColor;

    int m_MaintenanceTime;
    int m_MaintenancePRC;

    float m_StoreCurrentMusicVolume;
    float m_TargetMusicVolume;

public:
    CMatrixMap(void);
    void Clear(void);

    void IdsClear(void);
    inline const std::wstring &MapName(void) { return m_Ids[m_IdsCnt - 1]; }
    inline ParamParser IdsGet(int no) { return m_Ids[no]; }
    inline int IdsGetCount(void) const { return m_IdsCnt; }

    void UnitClear(void);

    void UnitInit(int sx, int sy);
    void PointCalcNormals(int x, int y);

    void CalcVis(void);  // non realtime function! its updates map data
    // void CalcVisTemp(int from, int to, const D3DXVECTOR3 &ptfrom); // non realtime function! its updates map data

    inline SMatrixMapUnit *UnitGet(int x, int y) { return m_Unit + y * m_Size.x + x; }
    inline SMatrixMapUnit *UnitGetTest(int x, int y) {
        return (x >= 0 && x < m_Size.x && y >= 0 && y < m_Size.y) ? (m_Unit + y * m_Size.x + x) : NULL;
    }

    inline SMatrixMapMove *MoveGet(int x, int y) { return m_Move + y * m_SizeMove.x + x; }
    inline SMatrixMapMove *MoveGetTest(int x, int y) {
        return (x >= 0 && x < m_SizeMove.x && y >= 0 && y < m_SizeMove.y) ? (m_Move + y * m_SizeMove.x + x) : NULL;
    }

    inline SMatrixMapPoint *PointGet(int x, int y) { return m_Point + x + y * (m_Size.x + 1); }
    inline SMatrixMapPoint *PointGetTest(int x, int y) {
        return (x >= 0 && x <= m_Size.x && y >= 0 && y <= m_Size.y) ? (m_Point + x + y * (m_Size.x + 1)) : NULL;
    }

#if defined _TRACE || defined _DEBUG
    void ResetMaintenanceTime(void) { m_MaintenanceTime = 0; };
#endif
    int BeforeMaintenanceTime(void) const { return m_MaintenanceTime; };
    float BeforMaintenanceTimeT(void) const {
        return 1.0f - float(m_MaintenanceTime) / (m_Difficulty.k_time_before_maintenance *
                                                  float(g_Config.m_MaintenanceTime * m_MaintenancePRC / 100));
    };
    void InitMaintenanceTime(void) {
        m_MaintenanceTime = Float2Int(m_Difficulty.k_time_before_maintenance * float(g_Config.m_MaintenanceTime) *
                                      float(m_MaintenancePRC) / 100.0f);
    };
    bool MaintenanceDisabled(void) const { return m_MaintenancePRC == 0; }
    // int     GetMaintenancePrc()                             { return m_MaintenancePRC; }
    void SetMaintenanceTime(int time) { m_MaintenanceTime = time; }

    float GetZLand(double wx, double wy);
    float GetZ(float wx, float wy);
    float GetZInterpolatedLand(float wx, float wy);
    float GetZInterpolatedObj(float wx, float wy);
    float GetZInterpolatedObjRobots(float wx, float wy);
    void GetNormal(D3DXVECTOR3 *out, float wx, float wy, bool check_water = false);
    DWORD GetColor(float wx, float wy);

    void CalcMoveSpherePlace(void);

    void ClearGroupVis(void);
    void GroupClear(void);
    void GroupBuild(CStorage &stor);

    void RobotPreload(void);

    const D3DXMATRIX &GetIdentityMatrix(void) const { return m_Identity; }

    bool CalcVectorToLandscape(const D3DXVECTOR2 &pos, D3DXVECTOR2 &dir);

    inline void RemoveFromAD(CMatrixMapStatic *ms) {
        for (int i = 0; i < m_AD_Obj_cnt; ++i) {
            if (m_AD_Obj[i] == ms) {
                m_AD_Obj[i] = m_AD_Obj[--m_AD_Obj_cnt];
                return;
            }
        }
    }

    bool UnitPick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, int *ox, int *oy, float *ot = NULL) {
        return UnitPick(orig, dir, CRect(0, 0, m_Size.x, m_Size.y), ox, oy, ot);
    }
    bool UnitPick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, const CRect &ar, int *ox, int *oy, float *ot = NULL);
    bool PointPick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, int *ox, int *oy) {
        return PointPick(orig, dir, CRect(0, 0, m_Size.x, m_Size.y), ox, oy);
    }
    bool PointPick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, const CRect &ar, int *ox, int *oy);
    bool UnitPickGrid(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, int *ox, int *oy);
    bool UnitPickWorld(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, float *ox, float *oy);

    void RestoreMusicVolume(void);
    void SetMusicVolume(float vol);

    void StaticClear(void);
    void StaticDelete(CMatrixMapStatic *ms);

    // CMatrixMapStatic * StaticAdd(EObjectType type, bool add_to_logic = true);

    inline void AddObject(CMatrixMapStatic *ms, bool add_to_logic) {
        m_AllObjects.push_back(ms);
        if (add_to_logic)
            ms->AddLT();
    }
    template <class O>
    inline O *StaticAdd(bool add_to_logic = true) {
        O *o = HNew(g_MatrixHeap) O();
        AddObject(o, add_to_logic && o->GetObjectType() != OBJECT_TYPE_MAPOBJECT);
        return o;
    }

    void AddEffectSpawner(float x, float y, float z, int ttl, const std::wstring &type);
    void RemoveEffectSpawnerByObject(CMatrixMapStatic *ms);
    void RemoveEffectSpawnerByTime(void);

    void MacrotextureClear(void);
    void MacrotextureInit(const std::wstring &path);

    // side funcs
    DWORD GetSideColor(int id);
    DWORD GetSideColorMM(int id);
    CTexture *GetSideColorTexture(int id);
    inline CTexture *GetPlayerSideColorTexture(void) { return m_PlayerSide->m_ColorTexture; };
    void ClearSide(void);

    void LoadSide(CBlockPar &bp);
    // void LoadTactics(CBlockPar & bp);

    CMatrixSideUnit *GetSideById(int id);
    CMatrixSideUnit *GetPlayerSide(void) { return m_PlayerSide; };

    void WaterClear(void);
    void WaterInit(void);

    void BeforeDraw(void);
    void Draw(void);

    void MarkNeedRecalcTerainColor(void) { SETFLAG(m_Flags, MMFLAG_NEEDRECALCTER); }

    bool AddEffect(CMatrixEffect *ef);
#ifdef _DEBUG
    void SubEffect(const SDebugCallInfo &from, PCMatrixEffect e);
#else
    void SubEffect(PCMatrixEffect e);
#endif
    // bool IsEffect(PCMatrixEffect e);
    int GetEffectCount(void) { return m_EffectsCnt; }
    // void SubLowestEffect(void);

    void Takt(int step);

    // DWORD Load(CBuf & b, CBlockPar & bp, bool &surface_macro);
    void Restart(void);  // boo!

    int ReloadDynamics(CStorage &stor, EReloadStep step, void* robots = NULL);

    int PrepareMap(CStorage &stor, const std::wstring &mapname);
    void StaticPrepare(int n, bool skip_progress = false);
    void StaticPrepare2(void* robots);

    void InitObjectsLights(void);  // must be called only after CreatePoolDefaultResources
    void ReleasePoolDefaultResources(void);
    void CreatePoolDefaultResources(bool loading);  // false - restoring resources
    bool CheckLostDevice(void);                     // if true, device is lost

    // zakker
    void ShowPortrets(void);

    void EnterDialogMode(const wchar *hint);
    void LeaveDialogMode(void);

    void BeginDieSequence(void);
    bool DieSequenceInProgress(void) { return m_ShadeOn; }

    CTextureManaged *GetReflectionTexture(void) { return m_Reflection; };
    int GetTime(void) const { return m_Time; }
    DWORD GetStartTime(void) const { return m_StartTime; }

    bool CatchPoint(const D3DXVECTOR3 &from, const D3DXVECTOR3 &to);
    bool TraceLand(D3DXVECTOR3 *out, const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir);
    CMatrixMapStatic *Trace(D3DXVECTOR3 *out, const D3DXVECTOR3 &start, const D3DXVECTOR3 &end, DWORD mask,
                            CMatrixMapStatic *skip = NULL);
    bool FindObjects(const D3DXVECTOR3 &pos, float radius, float oscale, DWORD mask, CMatrixMapStatic *skip,
                     ENUM_OBJECTS callback, DWORD user);
    bool FindObjects(const D3DXVECTOR2 &pos, float radius, float oscale, DWORD mask, CMatrixMapStatic *skip,
                     ENUM_OBJECTS2D callback, DWORD user);

    // CMatrixMapGroup * GetGroupByCell(int x, int y) { return m_Group[(x/MATRIX_MAP_GROUP_SIZE) +
    // (y/MATRIX_MAP_GROUP_SIZE) * m_GroupSize.x ];  }
    inline CMatrixMapGroup *GetGroupByIndex(int x, int y) { return m_Group[x + y * m_GroupSize.x]; }
    inline CMatrixMapGroup *GetGroupByIndex(int i) { return m_Group[i]; }
    inline CMatrixMapGroup *GetGroupByIndexTest(int x, int y) {
        return (x < 0 || y < 0 || x >= m_GroupSize.x || y >= m_GroupSize.x) ? NULL : m_Group[x + y * m_GroupSize.x];
    }

    float GetGroupMaxZLand(int x, int y);
    float GetGroupMaxZObj(int x, int y);
    float GetGroupMaxZObjRobots(int x, int y);

    void CalcMapGroupVisibility(void);

    struct SCalcVisRuntime;

    void CheckCandidate(SCalcVisRuntime &cvr, CMatrixMapGroup *g);

    DWORD GetCurrentFrame(void) { return m_CurFrame; }

    inline void Pause(bool p) {
        if (FLAG(m_Flags, MMFLAG_DIALOG_MODE))
            return;  // disable pasue/unpause in dialog mode
        INITFLAG(m_Flags, MMFLAG_PAUSE, p);
        if (p)
            CSound::StopPlayAllSounds();
    }
    inline bool IsPaused(void) { return FLAG(m_Flags, MMFLAG_PAUSE); }

    inline void MouseCam(bool p) { INITFLAG(m_Flags, MMFLAG_MOUSECAM, p); }
    inline bool IsMouseCam(void) { return FLAG(m_Flags, MMFLAG_MOUSECAM); }

    // draw functions

    void BeforeDrawLandscape(bool all = false);
    void BeforeDrawLandscapeSurfaces(bool all = false);
    void DrawLandscape(bool all = false);
    void DrawLandscapeSurfaces(bool all = false);
    void DrawObjects(void);
    void DrawWater(void);
    void DrawShadowsProjFast(void);
    void DrawShadows(void);
    void DrawEffects(void);
    void DrawSky(void);

private:
    enum EScanResult {
        SR_NONE,
        SR_BREAK,
        SR_FOUND

    };
    EScanResult ScanLandscapeGroup(void *data, int gx, int gy, const D3DXVECTOR3 &start, const D3DXVECTOR3 &end);
    EScanResult ScanLandscapeGroupForLand(void *data, int gx, int gy, const D3DXVECTOR3 &start, const D3DXVECTOR3 &end);
};

class CMatrixMapLogic;
class CMatrixTacticsList;

#include "MatrixLogic.hpp"

inline bool CMatrixMap::AddEffect(CMatrixEffect *ef) {
    DTRACE();
#ifdef _DEBUG
    if (ef->GetType() == EFFECT_UNDEFINED) {
        // int a = 1;
        ERROR_S(L"Undefined effect!");
    }
#endif

    if (m_EffectsCnt >= MAX_EFFECTS_COUNT) {
        PCMatrixEffect ef2del = NULL;
        int pri = ef->Priority();
        for (PCMatrixEffect e = m_EffectsFirst; e != NULL; e = e->m_Next) {
            if (e->IsDIP())
                continue;
            int p = e->Priority();
            if (p < MAX_EFFECT_PRIORITY) {
                ef2del = e;
                pri = p;
            }
        }
        if (ef2del == NULL) {
            ef->Release();
            return false;
        }
        else {
            if (m_EffectsNextTakt == ef2del)
                m_EffectsNextTakt = ef2del->m_Next;
            LIST_DEL(ef2del, m_EffectsFirst, m_EffectsLast, m_Prev, m_Next);
            ef2del->Release();
            --m_EffectsCnt;
        }
    }
    LIST_ADD(ef, m_EffectsFirst, m_EffectsLast, m_Prev, m_Next);
    ++m_EffectsCnt;
#ifdef _DEBUG
    CDText::T("E", m_EffectsCnt);
#endif
    return true;
}

inline CMatrixSideUnit *CMatrixMap::GetSideById(int id) {
    DTRACE();
    for (int i = 0; i < m_SideCnt; i++) {
        if (m_Side[i].m_Id == id)
            return &m_Side[i];
    }
#ifdef _DEBUG
    debugbreak();
#endif
    ERROR_E;
}

inline DWORD CMatrixMap::GetSideColor(int id) {
    DTRACE();
    if (id == 0)
        return m_NeutralSideColor;
    return GetSideById(id)->m_Color;
}

inline DWORD CMatrixMap::GetSideColorMM(int id) {
    DTRACE();
    if (id == 0)
        return m_NeutralSideColorMM;
    return GetSideById(id)->m_ColorMM;
}

inline CTexture *CMatrixMap::GetSideColorTexture(int id) {
    DTRACE();
    if (id == 0)
        return m_NeutralSideColorTexture;
    return GetSideById(id)->m_ColorTexture;
}

inline float CMatrixMap::GetGroupMaxZLand(int x, int y) {
    if (x < 0 || x >= m_GroupSize.x || y < 0 || y >= m_GroupSize.y)
        return 0;
    CMatrixMapGroup *g = GetGroupByIndex(x, y);
    if (g == NULL)
        return 0;
    float z = GetGroupByIndex(x, y)->GetMaxZLand();
    if (z < 0)
        return 0;
    return z;
}
inline float CMatrixMap::GetGroupMaxZObj(int x, int y) {
    if (x < 0 || x >= m_GroupSize.x || y < 0 || y >= m_GroupSize.y)
        return 0;
    CMatrixMapGroup *g = GetGroupByIndex(x, y);
    if (g == NULL)
        return 0;
    float z = GetGroupByIndex(x, y)->GetMaxZObj();
    if (z < 0)
        return 0;
    return z;
}
inline float CMatrixMap::GetGroupMaxZObjRobots(int x, int y) {
    if (x < 0 || x >= m_GroupSize.x || y < 0 || y >= m_GroupSize.y)
        return 0;
    CMatrixMapGroup *g = GetGroupByIndex(x, y);
    if (g == NULL)
        return 0;
    float z = GetGroupByIndex(x, y)->GetMaxZObjRobots();
    if (z < 0)
        return 0;
    return z;
}

#endif
