// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>
#include <algorithm>

#include <windows.h>
#include "MatrixMap.hpp"
#include "MatrixMapStatic.hpp"
#include "MatrixObject.hpp"
#include "MatrixObjectBuilding.hpp"
#include "MatrixObjectCannon.hpp"
#include "MatrixRobot.hpp"
#include "MatrixRenderPipeline.hpp"
#include "MatrixFlyer.hpp"
#include "MatrixProgressBar.hpp"
#include "ShadowStencil.hpp"
#include "MatrixLoadProgress.hpp"
#include "MatrixTerSurface.hpp"
#include "MatrixSkinManager.hpp"
#include "MatrixSoundManager.hpp"
#include "Interface/MatrixHint.hpp"
#include "MatrixInstantDraw.hpp"

#include "Interface/CInterface.h"
#include "Logic/MatrixTactics.h"
#include "MatrixGameDll.hpp"
#include "MatrixSampleStateManager.hpp"
#include "MatrixMultiSelection.hpp"

#include "Effects/MatrixEffectLandscapeSpot.hpp"

#include "CFile.hpp"

#define RENDER_PROJ_SHADOWS_IN_STENCIL_PASS

// Globals
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CMatrixMap::CMatrixMap()
  : CMain(), m_Console(), m_Camera(), m_CurFrame(0), m_IntersectFlagTracer(0), m_IntersectFlagFindObjects(0),
    m_RN(g_MatrixHeap), m_EffectsFirst(NULL), m_EffectsLast(NULL),
    m_EffectsNextTakt(NULL), m_Flags(0), m_WaterName{}, m_SkyAngle(0), m_SkyDeltaAngle(0),
    m_PrevTimeCheckStatus(-1500), m_Time(0), m_BeforeWinCount(0), m_PauseHint(NULL),
    m_DialogModeName(NULL), m_BeforeWinLooseDialogCount(0) {
    DTRACE();

    m_Reflection = NULL;

    m_Size.x = 0;
    m_Size.y = 0;
    m_SizeMove.x = 0;
    m_SizeMove.y = 0;
    m_Unit = NULL;
    m_Point = NULL;
    m_Move = NULL;

    m_GroupSize.x = 0;
    m_GroupSize.y = 0;
    m_Group = NULL;
    m_GroupVis = NULL;

    m_AD_Obj_cnt = 0;

    m_GroundZ = WATER_LEVEL;
    m_GroundZBase = WATER_LEVEL - 64;

    m_TraceStopObj = NULL;

    m_LightMain = D3DXVECTOR3(250.0f, -50.0f, -250.0f);
    D3DXVec3Normalize(&m_LightMain, &m_LightMain);

    m_Water = NULL;
    m_VisibleGroupsCount = 0;

    m_IdsCnt = 0;
    m_Ids = NULL;

    m_Side = NULL;
    m_SideCnt = 0;

    m_EffectsCnt = 0;

    m_MacrotextureSize = 1;
    m_Macrotexture = NULL;

    // zakker

    m_minz = 10000.0;
    m_maxz = -10000.0;
    D3DXMatrixIdentity(&m_Identity);

    m_EffectSpawners = NULL;
    m_EffectSpawnersCnt = 0;

    m_ShadowVB = NULL;

    m_ShadeTime = 0;
    m_ShadeOn = false;

    m_NeutralSideColorTexture = NULL;

    // m_Reflection->Tex()->SetPriority(0xFFFFFFFF);

    CSound::Init();

    m_NextLogicObject = NULL;

    memset(m_SkyTex, 0, sizeof(m_SkyTex));

    m_CameraAngle = 0;
    m_WaterNormalLen = 1;

    m_TargetMusicVolume = 1.0f;
    m_StoreCurrentMusicVolume = 1.0f;
    m_DeviceState = NULL;

    // init dificulty
    m_Difficulty.k_damage_enemy_to_player = 1.0f;
    m_Difficulty.k_time_before_maintenance = 1.0f;
    m_Difficulty.k_friendly_fire = 1.0f;

    CBlockPar *repl = g_MatrixData->BlockGetNE(PAR_REPLACE);
    if (repl) {
        auto t1(repl->ParGetNE(PAR_REPLACE_DIFFICULTY));

        if (!t1.empty()) {
            CBlockPar *dif = g_MatrixData->BlockGetNE(PAR_SOURCE_DIFFICULTY);
            if (dif) {
                auto t2 = dif->ParGetNE(t1);
                if (!t2.empty()) {
                    m_Difficulty.k_damage_enemy_to_player = 1.0f + float(t2.GetStrPar(0, L",").GetDouble() / 100.0);
                    m_Difficulty.k_time_before_maintenance = 1.0f + float(t2.GetStrPar(1, L",").GetDouble() / 100.0);
                    m_Difficulty.k_friendly_fire = 1.0f + float(t2.GetStrPar(2, L",").GetDouble() / 100.0);
                    if (m_Difficulty.k_damage_enemy_to_player < 0.01f)
                        m_Difficulty.k_damage_enemy_to_player = 0.01f;
                    if (m_Difficulty.k_time_before_maintenance < 0.01f)
                        m_Difficulty.k_time_before_maintenance = 0.01f;
                    if (m_Difficulty.k_friendly_fire < 0.01f)
                        m_Difficulty.k_friendly_fire = 0.01f;
                }
            }
        }
    }
}

CMatrixMap::~CMatrixMap() {
    DTRACE();

    // Clear(); do not call Clear method from this destructor!!!!!! it will be called from CMatrixMapLogic Clear
}

void CMatrixMap::Clear(void) {
    DTRACE();

    m_RN.Clear();

    m_Minimap.Clear();
    StaticClear();

    // clear effects

    while (m_EffectsFirst) {
#ifdef _DEBUG
        SubEffect(DEBUG_CALL_INFO, m_EffectsFirst);
#else
        SubEffect(m_EffectsFirst);
#endif
    }

    if (m_EffectSpawners) {
        for (int i = 0; i < m_EffectSpawnersCnt; ++i) {
            m_EffectSpawners[i].~CEffectSpawner();
        }
        HFree(m_EffectSpawners, g_MatrixHeap);
    }
    m_EffectSpawnersCnt = 0;
    m_EffectSpawners = NULL;
    CMatrixEffect::ClearEffects();

    MacrotextureClear();
    WaterClear();
    GroupClear();

    UnitClear();
    IdsClear();
    ClearSide();

    CTerSurface::ClearSurfaces();
    CBottomTextureUnion::Clear();
    CSkinManager::Clear();
    CSound::Clear();

    if (IS_VB(m_ShadowVB))
        DESTROY_VB(m_ShadowVB);

    m_Cursor.Clear();
    m_DI.Clear();

    if (m_DeviceState != NULL) {
        HDelete(CDeviceState, m_DeviceState, g_MatrixHeap);
        m_DeviceState = NULL;
    }
}

void CMatrixMap::IdsClear() {
    DTRACE();

    if (m_Ids) {
        for (int i = 0; i < m_IdsCnt; i++) {
            m_Ids[i].std::wstring::~wstring();
        }
        HFree(m_Ids, g_MatrixHeap);
        m_Ids = NULL;
    }
    m_IdsCnt = 0;
}

void CMatrixMap::RobotPreload(void) {
    g_LoadProgress->SetCurLP(LP_PRELOADROBOTS);
    g_LoadProgress->InitCurLP(ROBOT_HEAD_CNT + ROBOT_CHASSIS_CNT + ROBOT_WEAPON_CNT + ROBOT_ARMOR_CNT);
    int curlp = 0;

    ZeroMemory(m_RobotWeaponMatrix, sizeof(m_RobotWeaponMatrix));

    std::wstring tstr, tstr2;

    for (int i = 1; i <= ROBOT_HEAD_CNT; i++) {
        CVectorObject *vo =
                (CVectorObject *)g_Cache->Get(cc_VO, utils::format(L"%ls%d.vo", OBJECT_PATH_ROBOT_HEAD, i).c_str());

        vo->PrepareSpecial(OLF_MULTIMATERIAL_ONLY, CSkinManager::GetSkin, GSP_SIDE);

        g_LoadProgress->SetCurLPPos(curlp++);
    }
    for (int i = 1; i <= ROBOT_CHASSIS_CNT; i++) {
        CVectorObject *vo =
                (CVectorObject *)g_Cache->Get(cc_VO, utils::format(L"%ls%d.vo", OBJECT_PATH_ROBOT_CHASSIS, i).c_str());
        vo->PrepareSpecial(OLF_MULTIMATERIAL_ONLY, CSkinManager::GetSkin, GSP_SIDE);

        if (i == RUK_CHASSIS_PNEUMATIC) {
            CMatrixRobot::BuildPneumaticData(vo);
        }

        g_LoadProgress->SetCurLPPos(curlp++);
    }
    for (int i = 1; i <= ROBOT_WEAPON_CNT; i++) {
        CVectorObject *vo =
                (CVectorObject *)g_Cache->Get(cc_VO, utils::format(L"%ls%d.vo", OBJECT_PATH_ROBOT_WEAPON, i).c_str());
        vo->PrepareSpecial(OLF_MULTIMATERIAL_ONLY, CSkinManager::GetSkin, GSP_SIDE);

        g_LoadProgress->SetCurLPPos(curlp++);
    }

    for (int i = 0; i < ROBOT_ARMOR_CNT; i++) {
        CVectorObject *vo =
                (CVectorObject *)g_Cache->Get(cc_VO, utils::format(L"%ls%d.vo", OBJECT_PATH_ROBOT_ARMOR, i + 1).c_str());
        vo->PrepareSpecial(OLF_MULTIMATERIAL_ONLY, CSkinManager::GetSkin, GSP_SIDE);
        g_LoadProgress->SetCurLPPos(curlp++);

        int n = vo->GetMatrixCount();

        for (int u = 0; u < n; u++) {
            if (vo->GetMatrixId(u) < 20)
                continue;
            tstr = vo->GetMatrixName(u);

            int cntp = ParamParser{tstr}.GetCountPar(L",");
            for (int p = 0; p < cntp; p++) {
                ParamParser tstr2 = utils::trim(ParamParser{tstr}.GetStrPar(p, L","));
                if (p == 0 && tstr2 != L"W")
                    break;
                else if (p == 0) {
                    if (m_RobotWeaponMatrix[i].cnt >= ROBOT_WEAPONS_PER_ROBOT_CNT)
                        break;
                    m_RobotWeaponMatrix[i].list[m_RobotWeaponMatrix[i].cnt].id = vo->GetMatrixId(u);
                    ++m_RobotWeaponMatrix[i].cnt;
                    continue;
                }

                if (tstr2.IsOnlyInt()) {
                    if ((m_RobotWeaponMatrix[i].list[m_RobotWeaponMatrix[i].cnt - 1].access_invert & (~SETBIT(31))) ==
                        0) {
                        if (tstr2.GetInt() == RUK_WEAPON_BOMB || tstr2.GetInt() == RUK_WEAPON_MORTAR)
                            m_RobotWeaponMatrix[i].extra++;
                        else
                            m_RobotWeaponMatrix[i].common++;
                    }
                    m_RobotWeaponMatrix[i].list[m_RobotWeaponMatrix[i].cnt - 1].access_invert |=
                            (DWORD(1) << (tstr2.GetInt() - 1));
                }
                else if (tstr2 == L"I") {
                    m_RobotWeaponMatrix[i].list[m_RobotWeaponMatrix[i].cnt - 1].access_invert |= SETBIT(31);
                }
            }
        }

        // sort by id
        for (int u = 0; u < m_RobotWeaponMatrix[i].cnt - 1; u++) {
            for (int t = u + 1; t < m_RobotWeaponMatrix[i].cnt; t++) {
                if (m_RobotWeaponMatrix[i].list[u].id > m_RobotWeaponMatrix[i].list[t].id) {
                    SRobotWeaponMatrix::SW temp = m_RobotWeaponMatrix[i].list[u];
                    m_RobotWeaponMatrix[i].list[u] = m_RobotWeaponMatrix[i].list[t];
                    m_RobotWeaponMatrix[i].list[t] = temp;
                }
            }
        }
    }
}

void CMatrixMap::UnitClear(void) {
    DTRACE();

    if (m_Unit != NULL) {
        HFree(m_Unit, g_MatrixHeap);
        m_Unit = NULL;
    }
    if (m_Point != NULL) {
        HFree(m_Point, g_MatrixHeap);
        m_Point = NULL;
    }
    if (m_Move != NULL) {
        HFree(m_Move, g_MatrixHeap);
        m_Move = NULL;
    }
}

void CMatrixMap::UnitInit(int sx, int sy) {
    DTRACE();

    UnitClear();

    m_Size.x = sx;
    m_Size.y = sy;
    m_SizeMove.x = sx * MOVE_CNT;
    m_SizeMove.y = sy * MOVE_CNT;
    int cnt = sx * sy;

    m_Unit = (SMatrixMapUnit *)HAllocClear(sizeof(SMatrixMapUnit) * cnt, g_MatrixHeap);

    m_Point = (SMatrixMapPoint *)HAllocClear(sizeof(SMatrixMapPoint) * (sx + 1) * (sy + 1), g_MatrixHeap);

    m_Move = (SMatrixMapMove *)HAllocClear(sizeof(SMatrixMapMove) * m_SizeMove.x * m_SizeMove.y, g_MatrixHeap);

    /*
    SMatrixMapUnit * un=m_Unit;
    while(cnt>0) {
        un->m_TextureTop=-1;
        un++;
        cnt--;
    }
    */
}

DWORD CMatrixMap::GetColor(float wx, float wy) {
    DTRACE();

    // static int ccc = 0;
    // CDText::T("COLOR", ccc++);

    float scaledx = wx * INVERT(GLOBAL_SCALE);
    float scaledy = wy * INVERT(GLOBAL_SCALE);

    int x = TruncFloat(scaledx);
    int y = TruncFloat(scaledy);

    SMatrixMapUnit *un = UnitGetTest(x, y);
    if ((un == NULL) || (un->IsWater()))
        return m_AmbientColorObj;

    float kx = scaledx - float(x);
    float ky = scaledy - float(y);

    SMatrixMapPoint *mp = PointGet(x, y);
    int temp0r, temp0g, temp0b;
    int temp1r, temp1g, temp1b;
    int temp2r, temp2g, temp2b;
    int temp3r, temp3g, temp3b;

    // color 0;
    temp0r = ((mp->color >> 16) & 255) + mp->lum_r;
    temp0g = ((mp->color >> 8) & 255) + mp->lum_g;
    temp0b = ((mp->color >> 0) & 255) + mp->lum_b;

    // color 1;
    SMatrixMapPoint *mp0 = mp + 1;
    temp1r = ((mp0->color >> 16) & 255) + mp0->lum_r;
    temp1g = ((mp0->color >> 8) & 255) + mp0->lum_g;
    temp1b = ((mp0->color >> 0) & 255) + mp0->lum_b;

    // color 2;
    mp0 = (mp + m_Size.x + 1);
    temp2r = ((mp0->color >> 16) & 255) + mp0->lum_r;
    temp2g = ((mp0->color >> 8) & 255) + mp0->lum_g;
    temp2b = ((mp0->color >> 0) & 255) + mp0->lum_b;

    // color 2;
    mp0 = (mp + m_Size.x + 2);
    temp3r = ((mp0->color >> 16) & 255) + mp0->lum_r;
    temp3g = ((mp0->color >> 8) & 255) + mp0->lum_g;
    temp3b = ((mp0->color >> 0) & 255) + mp0->lum_b;

    int r = Float2Int(LERPFLOAT(ky, LERPFLOAT(kx, temp0r, temp1r), LERPFLOAT(kx, temp2r, temp3r)));
    int g = Float2Int(LERPFLOAT(ky, LERPFLOAT(kx, temp0g, temp1g), LERPFLOAT(kx, temp2g, temp3g)));
    int b = Float2Int(LERPFLOAT(ky, LERPFLOAT(kx, temp0b, temp1b), LERPFLOAT(kx, temp2b, temp3b)));
    if (r > 255)
        r = 255;
    if (g > 255)
        g = 255;
    if (b > 255)
        b = 255;

    return (r << 16) | (g << 8) | (b);
}
float CMatrixMap::GetZInterpolatedLand(float wx, float wy) {
    DTRACE();

    int x = Float2Int((float)floor(wx * INVERT(GLOBAL_SCALE * MAP_GROUP_SIZE)));
    int y = Float2Int((float)floor(wy * INVERT(GLOBAL_SCALE * MAP_GROUP_SIZE)));

    D3DXVECTOR3 xx[4], yy[4], ox, oy;
    float fx(x * (GLOBAL_SCALE * MAP_GROUP_SIZE)), fy(y * (GLOBAL_SCALE * MAP_GROUP_SIZE)), z;

    float tx = (wx - fx) * INVERT(GLOBAL_SCALE * MAP_GROUP_SIZE);
    float ty = (wy - fy) * INVERT(GLOBAL_SCALE * MAP_GROUP_SIZE);
    if (tx < 0)
        tx = 1.0f + tx;
    if (ty < 0)
        ty = 1.0f + ty;

    for (int j = 0; j <= 3; ++j) {
        int cy = y + j - 1;
        float prex_z = GetGroupMaxZLand(x - 2, cy);
        if (prex_z < m_GroundZBaseMiddle)
            prex_z = m_GroundZBaseMiddle;
        else if (prex_z > m_GroundZBaseMax)
            prex_z = m_GroundZBaseMax;
        for (int i = 0; i <= 3; ++i) {
            int cx = x + i - 1;

            // along x
            xx[i].x = cx * (GLOBAL_SCALE * MAP_GROUP_SIZE);
            xx[i].y = cy * (GLOBAL_SCALE * MAP_GROUP_SIZE);
            z = GetGroupMaxZLand(cx, cy);
            if (z < m_GroundZBaseMiddle)
                z = m_GroundZBaseMiddle;
            else if (z > m_GroundZBaseMax)
                z = m_GroundZBaseMax;
            xx[i].z = std::max(prex_z, z);  //(prex_z + z) * 0.5f;
            prex_z = z;
        }
        SBSplineKoefs kx;
        CalcBSplineKoefs2(kx, xx[0], xx[1], xx[2], xx[3]);
        CalcBSplinePoint(kx, yy[j], tx);
    }

    SBSplineKoefs ky;
    CalcBSplineKoefs2(ky, yy[0], yy[1], yy[2], yy[3]);
    CalcBSplinePoint(ky, oy, ty);
    return oy.z;
}

float CMatrixMap::GetZInterpolatedObj(float wx, float wy) {
    DTRACE();

    int x = Float2Int((float)floor(wx * INVERT(GLOBAL_SCALE * MAP_GROUP_SIZE)));
    int y = Float2Int((float)floor(wy * INVERT(GLOBAL_SCALE * MAP_GROUP_SIZE)));

    D3DXVECTOR3 xx[4], yy[4], ox, oy;
    float fx(x * (GLOBAL_SCALE * MAP_GROUP_SIZE)), fy(y * (GLOBAL_SCALE * MAP_GROUP_SIZE)), z;

    float tx = (wx - fx) * INVERT(GLOBAL_SCALE * MAP_GROUP_SIZE);
    float ty = (wy - fy) * INVERT(GLOBAL_SCALE * MAP_GROUP_SIZE);
    if (tx < 0)
        tx = 1.0f + tx;
    if (ty < 0)
        ty = 1.0f + ty;

    for (int j = 0; j <= 3; ++j) {
        int cy = y + j - 1;
        float prex_z = GetGroupMaxZObj(x - 2, cy);
        for (int i = 0; i <= 3; ++i) {
            int cx = x + i - 1;

            // along x
            xx[i].x = cx * (GLOBAL_SCALE * MAP_GROUP_SIZE);
            xx[i].y = cy * (GLOBAL_SCALE * MAP_GROUP_SIZE);
            z = GetGroupMaxZObj(cx, cy);
            xx[i].z = std::max(prex_z, z);  //(prex_z + z) * 0.5f;
            prex_z = z;
        }
        SBSplineKoefs kx;
        CalcBSplineKoefs2(kx, xx[0], xx[1], xx[2], xx[3]);
        CalcBSplinePoint(kx, yy[j], tx);
    }

    SBSplineKoefs ky;
    CalcBSplineKoefs2(ky, yy[0], yy[1], yy[2], yy[3]);
    CalcBSplinePoint(ky, oy, ty);
    return oy.z;
}

float CMatrixMap::GetZInterpolatedObjRobots(float wx, float wy) {
    DTRACE();

    int x = Float2Int((float)floor(wx * INVERT(GLOBAL_SCALE * MAP_GROUP_SIZE)));
    int y = Float2Int((float)floor(wy * INVERT(GLOBAL_SCALE * MAP_GROUP_SIZE)));

    D3DXVECTOR3 xx[4], yy[4], ox, oy;
    float fx(x * (GLOBAL_SCALE * MAP_GROUP_SIZE)), fy(y * (GLOBAL_SCALE * MAP_GROUP_SIZE)), z;

    float tx = (wx - fx) * INVERT(GLOBAL_SCALE * MAP_GROUP_SIZE);
    float ty = (wy - fy) * INVERT(GLOBAL_SCALE * MAP_GROUP_SIZE);
    if (tx < 0)
        tx = 1.0f + tx;
    if (ty < 0)
        ty = 1.0f + ty;

    for (int j = 0; j <= 3; ++j) {
        int cy = y + j - 1;
        float prex_z = GetGroupMaxZObjRobots(x - 2, cy);
        for (int i = 0; i <= 3; ++i) {
            int cx = x + i - 1;

            // along x
            xx[i].x = cx * (GLOBAL_SCALE * MAP_GROUP_SIZE);
            xx[i].y = cy * (GLOBAL_SCALE * MAP_GROUP_SIZE);
            z = GetGroupMaxZObjRobots(cx, cy);
            xx[i].z = std::max(prex_z, z);  //(prex_z + z) * 0.5f;
            prex_z = z;
        }
        SBSplineKoefs kx;
        CalcBSplineKoefs2(kx, xx[0], xx[1], xx[2], xx[3]);
        CalcBSplinePoint(kx, yy[j], tx);
    }

    SBSplineKoefs ky;
    CalcBSplineKoefs2(ky, yy[0], yy[1], yy[2], yy[3]);
    CalcBSplinePoint(ky, oy, ty);
    return oy.z;
}

float CMatrixMap::GetZLand(double wx, double wy) {
    DTRACE();

    int x = TruncDouble(wx * INVERT(GLOBAL_SCALE));
    int y = TruncDouble(wy * INVERT(GLOBAL_SCALE));

    SMatrixMapUnit *un = UnitGetTest(x, y);
    if (un == NULL || !un->IsLand())
        return 0;

    wx -= x * GLOBAL_SCALE;
    wy -= y * GLOBAL_SCALE;

    SMatrixMapPoint *mp = PointGet(x, y);

    D3DXVECTOR3 p0, p2;
    p0.x = 0;
    p0.y = 0;
    p0.z = mp->z;
    p2.x = GLOBAL_SCALE;
    p2.y = GLOBAL_SCALE;
    p2.z = (mp + m_Size.x + 2)->z;

    if (wy < wx) {
        D3DXVECTOR3 p1;
        D3DXPLANE pl;

        p1.x = GLOBAL_SCALE;
        p1.y = 0;
        p1.z = (mp + 1)->z;

        D3DXPlaneFromPoints(&pl, &p0, &p1, &p2);
        float cc = -1.0f / pl.c;
        float a1 = pl.a * cc;
        float b1 = pl.b * cc;
        float c1 = pl.d * cc;

        return float(a1 * wx + b1 * wy + c1);
    }
    else {
        D3DXVECTOR3 p3;
        D3DXPLANE pl;
        p3.x = 0;
        p3.y = GLOBAL_SCALE;
        p3.z = (mp + m_Size.x + 1)->z;

        D3DXPlaneFromPoints(&pl, &p0, &p2, &p3);
        float cc = -1.0f / pl.c;
        float a2 = pl.a * cc;
        float b2 = pl.b * cc;
        float c2 = pl.d * cc;

        return float(a2 * wx + b2 * wy + c2);
    }
}

float CMatrixMap::GetZ(float wx, float wy) {
    DTRACE();

    int x = TruncFloat(wx * INVERT(GLOBAL_SCALE));
    int y = TruncFloat(wy * INVERT(GLOBAL_SCALE));

    SMatrixMapUnit *un = UnitGetTest(x, y);
    if (un == NULL)
        return -1000.0f;
    if (!un->IsBridge()) {
        if (un->IsWater())
            return -1000.0f;
    }
    if (un->IsFlat())
        return un->a1;

    wx -= x * GLOBAL_SCALE;
    wy -= y * GLOBAL_SCALE;

    if (wy < wx) {
        return un->a1 * wx + un->b1 * wy + un->c1;
    }
    else {
        return un->a2 * wx + un->b2 * wy + un->c2;
    }
}

void CMatrixMap::GetNormal(D3DXVECTOR3 *out, float wx, float wy, bool check_water) {
    DTRACE();

    float scaledx = wx / GLOBAL_SCALE;
    float scaledy = wy / GLOBAL_SCALE;

    int x = TruncFloat(scaledx);
    int y = TruncFloat(scaledy);

    SMatrixMapUnit *un = UnitGetTest(x, y);

    if ((un == NULL) || un->IsFlat()) {
    water:
        out->x = 0;
        out->y = 0;
        out->z = 1;
        return;
    }

    if (un->IsBridge()) {
        D3DXVECTOR3 norm(GetZ(wx - (GLOBAL_SCALE * 0.5f), wy) - GetZ(wx + (GLOBAL_SCALE * 0.5f), wy),
                         GetZ(wx, wy - (GLOBAL_SCALE * 0.5f)) - GetZ(wx, wy + (GLOBAL_SCALE * 0.5f)), GLOBAL_SCALE);
        D3DXVec3Normalize(out, &norm);
        return;
    }
    else if (un->IsWater()) {
        goto water;
    }

    float kx = scaledx - float(x);
    float ky = scaledy - float(y);

    SMatrixMapPoint *mp0 = PointGet(x, y);
    SMatrixMapPoint *mp1 = mp0 + 1;
    SMatrixMapPoint *mp2 = (mp0 + m_Size.x + 1);
    SMatrixMapPoint *mp3 = (mp0 + m_Size.x + 2);

    if (check_water && ((mp0->z < 0) || (mp1->z < 0) || (mp2->z < 0) || (mp3->z < 0)))
        goto water;

    D3DXVECTOR3 v1 = LERPVECTOR(kx, mp0->n, mp1->n);
    D3DXVECTOR3 v2 = LERPVECTOR(kx, mp2->n, mp3->n);

    auto tmp = LERPVECTOR(ky, v1, v2);
    D3DXVec3Normalize(out, &tmp);
}

bool CMatrixMap::UnitPick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, const CRect &ar, int *ox, int *oy,
                          float *ot) {
    DTRACE();

    int x, y;
    D3DXVECTOR3 p0, p1, p2, p3;
    SMatrixMapUnit *un;
    float t, m_Unit, v;

    float mt = 1e30f;
    bool rv = false;

    for (y = std::max(0, ar.top); y < std::min(m_Size.y, ar.bottom); y++) {
        for (x = std::max(0, ar.left); x < std::min(m_Size.x, ar.right); x++) {
            un = UnitGet(x, y);
            if (!un->IsLand())
                continue;

            p0.x = GLOBAL_SCALE * (x);
            p0.y = GLOBAL_SCALE * (y);
            p0.z = PointGet(x, y)->z;
            p1.x = GLOBAL_SCALE * (x + 1);
            p1.y = GLOBAL_SCALE * (y);
            p1.z = PointGet(x + 1, y)->z;
            p2.x = GLOBAL_SCALE * (x + 1);
            p2.y = GLOBAL_SCALE * (y + 1);
            p2.z = PointGet(x + 1, y + 1)->z;
            p3.x = GLOBAL_SCALE * (x);
            p3.y = GLOBAL_SCALE * (y + 1);
            p3.z = PointGet(x, y + 1)->z;

            if (IntersectTriangle(orig, dir, p0, p1, p2, t, m_Unit, v) ||
                IntersectTriangle(orig, dir, p0, p2, p3, t, m_Unit, v)) {
                if (t < mt) {
                    mt = t;
                    if (ox != NULL)
                        *ox = x;
                    if (oy != NULL)
                        *oy = y;
                    if (ot != NULL)
                        *ot = mt;
                    rv = true;
                }
            }
        }
    }
    return rv;
}

bool CMatrixMap::PointPick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, const CRect &ar, int *ox, int *oy) {
    DTRACE();

    int x, y;
    if (!UnitPick(orig, dir, ar, &x, &y))
        return false;

    D3DXVECTOR3 p;

    p.x = GLOBAL_SCALE * (x);
    p.y = GLOBAL_SCALE * (y);
    p.z = PointGet(x, y)->z;
    float d0 = DistLinePoint(orig, orig + dir, p);

    p.x = GLOBAL_SCALE * (x + 1);
    p.y = GLOBAL_SCALE * (y);
    p.z = PointGet(x + 1, y)->z;
    float d1 = DistLinePoint(orig, orig + dir, p);

    p.x = GLOBAL_SCALE * (x + 1);
    p.y = GLOBAL_SCALE * (y + 1);
    p.z = PointGet(x + 1, y + 1)->z;
    float d2 = DistLinePoint(orig, orig + dir, p);

    p.x = GLOBAL_SCALE * (x);
    p.y = GLOBAL_SCALE * (y + 1);
    p.z = PointGet(x, y + 1)->z;
    float d3 = DistLinePoint(orig, orig + dir, p);

    if (d1 <= d0 && d1 <= d2 && d1 <= d3)
        x++;
    else if (d2 <= d0 && d2 <= d1 && d2 <= d3) {
        x++;
        y++;
    }
    else if (d3 <= d0 && d3 <= d1 && d3 <= d2)
        y++;

    if (ox != NULL)
        *ox = x;
    if (oy != NULL)
        *oy = y;

    return true;
}

bool CMatrixMap::UnitPickGrid(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, int *ox, int *oy) {
    DTRACE();

    D3DXPLANE pl;
    auto tmp1 = D3DXVECTOR3(0, 0, 0);
    auto tmp2 = D3DXVECTOR3(1.0f, 0, 0);
    auto tmp3 = D3DXVECTOR3(0, 1.0f, 0);
    D3DXPlaneFromPoints(&pl, &tmp1, &tmp2, &tmp3);
    //	D3DXPlaneNormalize(&pl,&pl);

    D3DXVECTOR3 v;
    auto tmp4 = orig + dir * 1000000.0f;
    if (D3DXPlaneIntersectLine(&v, &pl, &orig, &tmp4) == NULL)
        return false;

    if (ox != NULL)
        *ox = TruncFloat(v.x / GLOBAL_SCALE);
    if (oy != NULL)
        *oy = TruncFloat(v.y / GLOBAL_SCALE);

    return true;
}

bool CMatrixMap::UnitPickWorld(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, float *ox, float *oy) {
    D3DXPLANE pl;
    auto tmp1 = D3DXVECTOR3(0, 0, 0);
    auto tmp2 = D3DXVECTOR3(1.0f, 0, 0);
    auto tmp3 = D3DXVECTOR3(0, 1.0f, 0);
    D3DXPlaneFromPoints(&pl, &tmp1, &tmp2, &tmp3);
    //	D3DXPlaneNormalize(&pl,&pl);

    D3DXVECTOR3 v;
    auto tmp4 = orig + dir * 1000000.0f;
    if (D3DXPlaneIntersectLine(&v, &pl, &orig, &tmp4) == NULL)
        return false;

    if (ox != NULL)
        *ox = v.x;
    if (oy != NULL)
        *oy = v.y;

    return true;
}

void CMatrixMap::StaticClear(void) {
    DTRACE();

    while (!m_AllObjects.empty())
    {
        StaticDelete(m_AllObjects.front());
    }

    CMatrixMapObject::ClearTextures();
}

#ifdef _DEBUG
#include "stdio.h"
#endif

void CMatrixMap::StaticDelete(CMatrixMapStatic *ms) {
    DTRACE();

    if (ms->IsDIP())
        return;  // already deleted
    ms->SetDIP();

    if (m_NextLogicObject == ms) {
        m_NextLogicObject = ms->GetNextLogic();
    }

    CMatrixMapStatic::RemoveFromSorted(ms);

    auto sb = m_AllObjects.begin();
    auto se = m_AllObjects.end();
    auto ses = se - 1;

    if (ms->InLT()) {
        ms->DelLT();
        // remove this object from end of array
        while (sb < se) {
            --se;
            if (*se == ms && ses != se) {
                *se = *ses;
            }
        }
    }
    else {
        // remove this object from begin of array
        while (sb < se) {
            if (*sb == ms && ses != sb) {
                *sb = *ses;
            }
            ++sb;
        }
    }
    m_AllObjects.erase(m_AllObjects.end() - 1);

    //#ifdef _DEBUG
    //    std::wstring c(L"Del obj ");
    //    if (ms->GetObjectType()==OBJECT_TYPE_MAPOBJECT) c+=L" mapobj: ";
    //	else if(ms->GetObjectType()==OBJECT_TYPE_ROBOTAI) c+=L" robot: ";
    //	else if(ms->GetObjectType()==OBJECT_TYPE_BUILDING) c+=L" build: ";
    //    else if(ms->GetObjectType()==OBJECT_TYPE_CANNON) c+=L" cannon: ";
    //    else if(ms->GetObjectType()==OBJECT_TYPE_FLYER) c+=L" flyer: ";
    //    c.AddHex(ms);
    //    c += L"\n";
    //    SLOG("objlog.txt", c.Get());
    //#endif

    if (ms->GetObjectType() == OBJECT_TYPE_MAPOBJECT) {
        HDelete(CMatrixMapObject, (CMatrixMapObject *)ms, g_MatrixHeap);
    }
    else if (ms->IsRobot()) {
        HDelete(CMatrixRobotAI, (CMatrixRobotAI *)ms, g_MatrixHeap);
    }
    else if (ms->IsCannon()) {
        HDelete(CMatrixCannon, (CMatrixCannon *)ms, g_MatrixHeap);
    }
    else if (ms->IsFlyer()) {
        HDelete(CMatrixFlyer, (CMatrixFlyer *)ms, g_MatrixHeap);
    }
    else if (ms->IsBuilding()) {
        HDelete(CMatrixBuilding, (CMatrixBuilding *)ms, g_MatrixHeap);
    }
    else
        ERROR_E;
}

// CMatrixMapStatic * CMatrixMap::StaticAdd(EObjectType type, bool add_to_logic)
//{
//    DTRACE();
//
//	CMatrixMapStatic * ms;
//	if (type==OBJECT_TYPE_MAPOBJECT) ms=HNew(g_MatrixHeap) CMatrixMapObject();
//	else if(type==OBJECT_TYPE_ROBOTAI) ms=HNew(g_MatrixHeap) CMatrixRobotAI();
//	else if(type==OBJECT_TYPE_BUILDING) ms=HNew(g_MatrixHeap) CMatrixBuilding();
//    else if(type==OBJECT_TYPE_CANNON) ms=HNew(g_MatrixHeap) CMatrixCannon();
//    else if(type==OBJECT_TYPE_FLYER) ms=HNew(g_MatrixHeap) CMatrixFlyer();
//	else ERROR_E;
//
////#ifdef _DEBUG
////    std::wstring c(L"New obj ");
////    if (type==OBJECT_TYPE_MAPOBJECT) c+=L" mapobj: ";
////	else if(type==OBJECT_TYPE_ROBOTAI) c+=L" robot: ";
////	else if(type==OBJECT_TYPE_BUILDING) c+=L" build: ";
////    else if(type==OBJECT_TYPE_CANNON) c+=L" cannon: ";
////    else if(type==OBJECT_TYPE_FLYER) c+=L" flyer: ";
////    c.AddHex(ms);
////    c += L"\n";
////    SLOG("objlog.txt", c.Get());
////#endif
//
//    m_AllObjects.push_back(ms);
//
//
//	if(add_to_logic && type!=OBJECT_TYPE_MAPOBJECT)
//    {
//		ms->AddLT();
//	}
//
//	return ms;
//}

void CMatrixMap::MacrotextureClear() {
    DTRACE();

    if (m_Macrotexture) {
        m_Macrotexture = NULL;
    }
}

void CMatrixMap::MacrotextureInit(const std::wstring &path) {
    DTRACE();

    MacrotextureClear();

    m_Macrotexture = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, path.c_str());

    int cnt = ParamParser{path}.GetCountPar(L"?");
    for (int i = 1; i < cnt; i++) {
        ParamParser op = utils::trim(ParamParser{path}.GetStrPar(i, L"?"));
        if (utils::starts_with(op, L"SIM"))
            m_MacrotextureSize = op.GetInt();
    }
}

void CMatrixMap::ClearSide() {
    DTRACE();

    if (m_Side) {
        for (int i = 0; i < m_SideCnt; i++) {
            m_Side[i].CMatrixSideUnit::~CMatrixSideUnit();
        }
        HFree(m_Side, g_MatrixHeap);
        m_Side = NULL;
    }
    m_PlayerSide = NULL;
    if (m_NeutralSideColorTexture) {
        CCache::Destroy(m_NeutralSideColorTexture);
        m_NeutralSideColorTexture = NULL;
    }
    m_SideCnt = 0;
}

// void CMatrixMap::LoadTactics(CBlockPar & bp)
//{
//    DTRACE();
//    //bp.SaveInTextFile(L"#tactics.txt");
//    for(int sides = 1; sides <= 3; sides++){
//        CMatrixSideUnit* side = GetSideById(sides);
//        side->m_TacticsPar = bp.BlockPathGet(L"Tactics");
//    }
//}

void CMatrixMap::LoadSide(CBlockPar &bp) {
    ClearSide();

    int cnt = bp.ParCount();
    ASSERT(cnt == 5);  // there are 4 sides + one neutral. please updated data.txt

    m_SideCnt = cnt - 1;
    m_Side = (CMatrixSideUnit *)HAllocClear(m_SideCnt * sizeof(CMatrixSideUnit), g_MatrixHeap);
    for (int i = 0; i < m_SideCnt; i++)
        new(&m_Side[i]) CMatrixSideUnit();

    int idx = 0;
    for (int i = 0; i < cnt; i++) {
        int id = bp.ParGetName(i).GetInt();
        const auto name = bp.ParGet(i);
        DWORD color = (DWORD(name.GetStrPar(1, L",").GetInt() & 255) << 16) | (DWORD(name.GetStrPar(2, L",").GetInt() & 255) << 8) |
                      DWORD(name.GetStrPar(3, L",").GetInt() & 255);
        DWORD colorMM = (DWORD(name.GetStrPar(5, L",").GetInt() & 255) << 16) | (DWORD(name.GetStrPar(6, L",").GetInt() & 255) << 8) |
                        DWORD(name.GetStrPar(7, L",").GetInt() & 255);

        if (id == 0) {
            m_NeutralSideColor = color;
            m_NeutralSideColorMM = colorMM;
            continue;
        }

        m_Side[idx].m_Id = id;

        m_Side[idx].m_Constructor->SetSide(id);
        m_Side[idx].m_Color = color;
        m_Side[idx].m_ColorMM = colorMM;
        m_Side[idx].m_ColorTexture = NULL;
        m_Side[idx].m_Name = name.GetStrPar(0, L",");
        ++idx;

        if (id == PLAYER_SIDE) {
            m_PlayerSide = m_Side;
            m_Side->InitPlayerSide();
        }
    }

    /*m_PlayerSide = GetSideById(PLAYER_SIDE);*/
}

void CMatrixMap::WaterClear() {
    DTRACE();

    if (m_Water) {
        HDelete(CMatrixWater, m_Water, g_MatrixHeap);
        m_Water = NULL;
    }
    m_VisWater.clear();

    SInshorewave::MarkAllBuffersNoNeed();
}

void CMatrixMap::WaterInit() {
    DTRACE();

    if (!m_Water) {
        m_Water = HNew(g_MatrixHeap) CMatrixWater;
    }
    else {
        m_Water->Clear();
    }

    m_VisWater.clear();
    m_Water->Init();
}

void CMatrixMap::BeforeDraw(void) {
    DTRACE();

    ++m_CurFrame;  // uniq number per ~500 days

    m_Camera.BeforeDraw();

    CPoint p(m_Cursor.GetPos());
    // ScreenToClient(g_Wnd,&p);
    m_Camera.CalcPickVector(p, m_MouseDir);

    //{
    //    static bool start = false;
    //    static bool end = false;
    //    static D3DXVECTOR3 s,e;

    //    if (m_KeyDown && m_KeyScan == KEY_F5)
    //    {
    //        start = true;
    //        s = m_Camera.GetFrustumCenter();
    //        m_KeyDown = false;

    //        //s = D3DXVECTOR3(2946.17f, 1301.01f, 142.9f);

    //    }
    //    if (m_KeyDown && m_KeyScan == KEY_F6)
    //    {

    //        end = true;
    //        e = s + m_MouseDir * 1000;
    //        m_KeyDown = false;
    //        //e = D3DXVECTOR3(3827.65f, 1731.25f, -51.7776f);

    //    }
    //    if (start && end)
    //    {
    //        CHelper::Create(1,0)->Line(s,e);

    //        D3DXVECTOR3 hit = e;
    //        CMatrixMapStatic *ms = Trace(&hit, s, e, TRACE_ALL, NULL);

    //        if (IS_TRACE_STOP_OBJECT(ms))
    //            ms->SetTerainColor(0xFF00FF00);
    //
    //        CHelper::Create(1,0)->Line(hit-D3DXVECTOR3(0,0,100),hit+D3DXVECTOR3(0,0,100));
    //
    //        //CMatrixCannon *ca = (CMatrixCannon *)0x04d58d60;
    //        //CVectorObjectAnim * o = ca->m_Unit[1].m_Graph;
    //        //
    //        //SVOGeometrySimple *gs = o->VO()->GetGS();

    //        //D3DXVECTOR3 gsc = gs->m_AsIs.m_Center;
    //        //D3DXVec3TransformCoord(&gsc, &gsc, &ca->m_Unit[1].m_Matrix);

    //        //CHelper::Create(1,0)->Line(gsc-D3DXVECTOR3(0,0,100),gsc+D3DXVECTOR3(0,0,100));
    //        //CHelper::Create(1,0)->Line(gsc-D3DXVECTOR3(0,100,0),gsc+D3DXVECTOR3(0,100,0));
    //        //CHelper::Create(1,0)->Line(gsc-D3DXVECTOR3(100,0,0),gsc+D3DXVECTOR3(100, 0,0));

    //    }
    //}

    // TAKT_BEGIN();
    m_TraceStopObj =
            Trace(&m_TraceStopPos, m_Camera.GetFrustumCenter(), m_Camera.GetFrustumCenter() + (m_MouseDir * 10000.0f),
                  TRACE_ALL, g_MatrixMap->GetPlayerSide()->GetArcadedObject());
    if (IS_TRACE_STOP_OBJECT(m_TraceStopObj)) {
        if (m_TraceStopObj->GetObjectType() == OBJECT_TYPE_BUILDING) {
            ((CMatrixBuilding *)m_TraceStopObj)->ShowHitpoint();
        }
        else if (m_TraceStopObj->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
            ((CMatrixRobotAI *)m_TraceStopObj)->ShowHitpoint();
        }
        else if (m_TraceStopObj->GetObjectType() == OBJECT_TYPE_CANNON) {
            ((CMatrixCannon *)m_TraceStopObj)->ShowHitpoint();

            // for (int i=0;i<m_TraceStopObj->GetGroupCnt();++i)
            //{
            //    CMatrixMapGroup *g =  m_TraceStopObj->GetGroup(i);
            //    g->DrawBBox();

            //    m_DI.T(L"Group"+std::wstring(i), std::wstring(g->GetPos0().x) + L", " + std::wstring(g->GetPos0().y));
            //}
        }
        else if (m_TraceStopObj->GetObjectType() == OBJECT_TYPE_FLYER) {
            ((CMatrixFlyer *)m_TraceStopObj)->ShowHitpoint();
        }

#ifdef _DEBUG
        if (m_TraceStopObj->GetObjectType() == OBJECT_TYPE_MAPOBJECT)
            m_DI.T(L"Under cursor", L"Mesh", 1000);
        else if (m_TraceStopObj->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
            m_DI.T(L"Under cursor",
                   utils::format(L"Robot %x   S%d T%d G%d",
                                 DWORD(m_TraceStopObj),
                                 m_TraceStopObj->GetSide(),
                                 ((CMatrixRobotAI *)m_TraceStopObj)->GetTeam(),
                                 ((CMatrixRobotAI *)m_TraceStopObj)->GetGroupLogic())
                           .c_str(),
                   1000);
        }
        else if (m_TraceStopObj->GetObjectType() == OBJECT_TYPE_CANNON)
            m_DI.T(L"Under cursor", L"Cannon", 1000);
        else if (m_TraceStopObj->GetObjectType() == OBJECT_TYPE_BUILDING) {
            m_DI.T(L"Under cursor", utils::format(L"Building: 0x%X", reinterpret_cast<dword>((void*)m_TraceStopObj)).c_str(), 1000);
        }
        else if (m_TraceStopObj->GetObjectType() == OBJECT_TYPE_FLYER)
            m_DI.T(L"Under cursor", L"Flyer", 1000);
#endif
    }

    // flyer
    CMatrixSideUnit *player_side = GetPlayerSide();

    // CDText::T("sel", player_side->m_CurrSel);
    // if(player_side->IsArcadeMode() &&  player_side->GetArcadedObject()->GetObjectType() == OBJECT_TYPE_FLYER &&
    // (GetAsyncKeyState(g_Config.m_KeyActions[KA_AUTO]) & 0x8000)==0x8000 && g_IFaceList->m_InFocus != INTERFACE)
    //{
    //    CMatrixFlyer * fl = (CMatrixFlyer *)player_side->GetArcadedObject();

    //    SPlane hp;
    //    hp.BuildFromPointNormal(hp, fl->GetPos(), D3DXVECTOR3(0,0,1));

    //    float t = 0;
    //    bool hit = hp.FindIntersect(m_Camera.GetFrustumCenter(), vdir, t);
    //    if (hit)
    //    {
    //        fl->SetTarget(D3DXVECTOR2(m_Camera.GetFrustumCenter().x + vdir.x * t,  m_Camera.GetFrustumCenter().y +
    //        vdir.y * t));

    //        CMatrixSideUnit *player_side = GetPlayerSide();
    //        //if (player_side->HasFlyer()) player_side->Select(HELICOPTER, NULL);

    //        //D3DXVECTOR3 p = m_Camera.GetFrustumCenter() + vdir * t;
    //        //CHelper::Create(1,0)->Line(p, p + D3DXVECTOR3(0,0,-100));
    //    }

    //    //player_side->GetFlyer()->SetTarget(D3DXVECTOR2(m_TraceStopPos.x, m_TraceStopPos.y));
    //}

#ifdef _DEBUG

    if (player_side->m_ActiveObject != m_TraceStopObj && player_side->m_ActiveObject &&
        player_side->m_ActiveObject->GetObjectType() == OBJECT_TYPE_FLYER &&
        (GetAsyncKeyState(g_Config.m_KeyActions[KA_AUTO]) & 0x8000) == 0x8000 && g_IFaceList->m_InFocus != INTERFACE) {
        CMatrixFlyer *fl = (CMatrixFlyer *)player_side->m_ActiveObject;

        SPlane hp;
        hp.BuildFromPointNormal(hp, fl->GetPos(), D3DXVECTOR3(0, 0, 1));

        float t = 0;
        bool hit = hp.FindIntersect(m_Camera.GetFrustumCenter(), m_MouseDir, t);
        if (hit) {
            fl->SetTarget(D3DXVECTOR2(m_Camera.GetFrustumCenter().x + m_MouseDir.x * t,
                                      m_Camera.GetFrustumCenter().y + m_MouseDir.y * t));

            CMatrixSideUnit *player_side = GetPlayerSide();
            // if (player_side->HasFlyer()) player_side->Select(HELICOPTER, NULL);

            // D3DXVECTOR3 p = m_Camera.GetFrustumCenter() + vdir * t;
            // CHelper::Create(1,0)->Line(p, p + D3DXVECTOR3(0,0,-100));
        }

        // player_side->GetFlyer()->SetTarget(D3DXVECTOR2(m_TraceStopPos.x, m_TraceStopPos.y));
    }

#endif

    // CDText::T("t", int(m_TraceStopObj));

    // if (m_KeyDown && m_KeyScan == KEY_PGDN) {m_KeyDown = false; m_Minimap.SetOutParams(m_Minimap.GetScale() * 0.8f);}
    // if (m_KeyDown && m_KeyScan == KEY_PGUP) {m_KeyDown = false; m_Minimap.SetOutParams(m_Minimap.GetScale()
    // * 1.25f);}

    // static float angle = 0;
    // if (m_KeyDown && m_KeyScan == KEY_PGDN) {m_KeyDown = false; angle += 0.01f;}
    // if (m_KeyDown && m_KeyScan == KEY_PGUP) {m_KeyDown = false; angle -= 0.01f;}

    // m_Minimap.SetAngle(angle);

    m_Minimap.SetOutParams(D3DXVECTOR2(m_Camera.GetXYStrategy().x, m_Camera.GetXYStrategy().y));
    //#ifdef MINIMAP_SUPPORT_ROTATION
    //    m_Minimap.SetAngle(-m_Camera.GetAngleZ());
    //#endif

    //#ifdef _DEBUG
    //    if (IS_TRACE_STOP_OBJECT(m_TraceStopObj) && m_TraceStopObj->GetObjectType() == OBJECT_TYPE_MAPOBJECT)
    //    {
    //        int t = ((CMatrixMapObject *)m_TraceStopObj)->m_BurnTexVis;
    //        m_TraceStopObj->Damage(WEAPON_PLASMA, D3DXVECTOR3(0,0,0), D3DXVECTOR3(0,0,0));
    //
    //        if (m_KeyDown && m_KeyScan == KEY_PGDN) {m_KeyDown = false; t -= 1;}
    //        if (m_KeyDown && m_KeyScan == KEY_PGUP) {m_KeyDown = false; t += 1;}
    //
    //        CDText::T("v", t);
    //
    //        ((CMatrixMapObject *)m_TraceStopObj)->m_BurnTexVis = (BYTE)(t&255);
    //
    //    }
    //#endif

    // D3DXVECTOR3 out;
    // if (TraceLand(&out, m_Camera.GetFrustumCenter() + D3DXVECTOR3(0,100,0), m_MouseDir))
    //{
    //    CHelper::Create(1,0)->Line(m_Camera.GetFrustumCenter() + D3DXVECTOR3(0,100,0),out);
    //    CHelper::Create(1,0)->Line(out - D3DXVECTOR3(0,0,100),out + D3DXVECTOR3(0,0,100));

    //}

    // D3DXVECTOR3 p0, p1;
    // p0 = m_TraceStopPos;
    // p0.z = GetZ(p0.x, p0.y);
    // GetNormal(&p1, p0.x, p0.y);
    ////p1 = p0 + p1*50;
    // p1 = p0 + p1*100;

    // D3DXVECTOR3 p0 = m_TraceStopPos;
    // CHelper::Create(1,0)->Line(p0 - D3DXVECTOR3(0,0,100),p0 + D3DXVECTOR3(0,0,100));
    // CHelper::Create(1,0)->Line(p0 - D3DXVECTOR3(0,100,0),p0 + D3DXVECTOR3(0,100,0));
    // CHelper::Create(1,0)->Line(p0 - D3DXVECTOR3(100,0,0),p0 + D3DXVECTOR3(100,0,0));

    // static CMatrixEffectPointLight *pl = NULL;
    // if (pl == NULL)
    //{
    //    pl = (CMatrixEffectPointLight *)CMatrixEffect::CreatePointLight(p1, 100, 0xFFFFFF00, true);
    //} else
    //{
    //    pl->SetPos(p1);
    //}

    // int ux = int(p0.x / GLOBAL_SCALE);
    // int uy = int(p0.y / GLOBAL_SCALE);
    // SMatrixMapUnit *mu = UnitGetTest(ux,uy);

    // CHelper::Create(1,0)->BoundBox(D3DXVECTOR3(ux*GLOBAL_SCALE,uy*GLOBAL_SCALE,0),
    // D3DXVECTOR3((ux+1)*GLOBAL_SCALE,(uy+1)*GLOBAL_SCALE,10));

    // if (mu)
    //{
    //    std::wstring str;
    //    if (mu->IsBridge()) str += L"Bridge,";
    //    if (mu->IsFlat()) str += L"Flat,";
    //    if (mu->IsInshore()) str += L"Inshore,";
    //    if (mu->IsWater()) str += L"Water,";
    //    if (mu->IsLand()) str += L"Land,";
    //    str += L"ok";
    //    m_DI.T(L"Unit type", str.Get());

    //}

    // CDText::T("tracez", p0.z);

    // TAKT_END("Trace");

    // int gx = int(m_TraceStopPos.x / (GLOBAL_SCALE*MATRIX_MAP_GROUP_SIZE));
    // int gy = int(m_TraceStopPos.y / (GLOBAL_SCALE*MATRIX_MAP_GROUP_SIZE));

    // m_DI.T(L"group", std::wstring(gx) + L"," + std::wstring(gy));
    // GetGroupByIndex(gx,gy)->DrawBBox();

    /*
        D3DXVECTOR3 p0 = GetPlayerSide()->m_HelicopterPos;
        D3DXVECTOR3 p1;
        Trace(&p1, p0, p0 + ((m_TraceStopPos-p0)*5), TRACE_ALL, TRACE_STOP_FLYER);

        CHelper::Create(1,0)->Line(p0, p1);
        CHelper::Create(1,0)->Line(p1, p1 + D3DXVECTOR3(0,0,100));
    */

    CalcMapGroupVisibility();

    BeforeDrawLandscape();
    if (CTerSurface::IsSurfacesPresent())
        BeforeDrawLandscapeSurfaces();

    // build objects sort array

    CMatrixMapStatic::SortBegin();
    // CMatrixMapStatic::OnEndOfDraw(); // this will call OnOutScreen for all

    int cnt = m_VisibleGroupsCount;
    CMatrixMapGroup **md = m_VisibleGroups;
    while ((cnt--) > 0) {
        if (*(md) != NULL) {
            (*(md))->SortObjects(m_Camera.GetViewMatrix());
            (*(md))->BeforeDrawSurfaces();
        }
        ++md;
    }

    for (int od = 0; od < m_AD_Obj_cnt; ++od) {
        m_AD_Obj[od]->Sort(m_Camera.GetViewMatrix());
        if (m_AD_Obj[od]->GetObjectType() == OBJECT_TYPE_FLYER) {
            if (((CMatrixFlyer *)m_AD_Obj[od])->CarryingRobot()) {
                ((CMatrixFlyer *)m_AD_Obj[od])->GetCarryingRobot()->Sort(m_Camera.GetViewMatrix());
            }
        }
    }

    if (FLAG(m_Flags, MMFLAG_NEEDRECALCTER)) {
        CMatrixMapStatic::SortEndRecalcTerainColor();
        RESETFLAG(m_Flags, MMFLAG_NEEDRECALCTER);
    }

    if (FLAG(g_Config.m_DIFlags, DI_VISOBJ))
        m_DI.T(L"Visible objects", utils::format(L"%d", CMatrixMapStatic::GetVisObjCnt()).c_str());

    CMatrixMapStatic::SortEndBeforeDraw();

    // cannon for build before draw
    // player_side = GetPlayerSide();
    if (player_side->m_CannonForBuild.m_Cannon) {
        if (g_IFaceList->m_InFocus == UNKNOWN) {
            player_side->m_CannonForBuild.m_Cannon->SetVisible(true);
            player_side->m_CannonForBuild.m_Cannon->BeforeDraw();
        }
        else {
            player_side->m_CannonForBuild.m_Cannon->SetVisible(false);
        }
    }

    // CVectorObject::m_VB->PrepareAll();

    m_Minimap.BeforeDraw();

    CBillboard::BeforeDraw();
    for (PCMatrixEffect e = m_EffectsFirst; e != NULL; e = e->m_Next) {
        e->BeforeDraw();
    }
    CMatrixProgressBar::BeforeDrawAll();
    m_Water->BeforeDraw();

    g_IFaceList->BeforeRender();
}

bool CMatrixMap::CalcVectorToLandscape(const D3DXVECTOR2 &pos, D3DXVECTOR2 &dir) {
    const float maxrad = GLOBAL_SCALE * 5;
    const float minrad = GLOBAL_SCALE * 5;
    const float radstep = GLOBAL_SCALE;

    dir.x = 0;
    dir.y = 0;

    const float up_level = 0.0f;
    const float down_level = -20.1f;

    float zz;

    for (float r = minrad; r <= maxrad; r += radstep) {
        float astep = float(INVERT(r * GLOBAL_SCALE));
        for (float a = 0; a < M_PI_MUL(2); a += astep) {
            float ca = TableCos(a);
            float sa = TableSin(a);

            float x = pos.x + r * ca;
            float y = pos.y + r * sa;

            float z = GetZ(x, y);

            if (z < down_level)
                zz = 0;
            else if (z > up_level)
                zz = 1;
            else
                zz = ((z - down_level) / (up_level - down_level));

            dir += zz * D3DXVECTOR2(ca, sa);
        }
    }

    if (dir.x == 0 && dir.y == 0) {
        return false;
    }

    D3DXVec2Normalize(&dir, &dir);
    return true;
}

//#include <stdio.h>

void CMatrixMap::BeforeDrawLandscape(bool all) {
    DTRACE();

    CMatrixMapGroup::BeforeDrawAll();
    if (m_Macrotexture)
        m_Macrotexture->Prepare();

    int cnt;
    CMatrixMapGroup **md;

    if (all) {
        cnt = m_GroupSize.x * m_GroupSize.y;
        md = m_Group;
    }
    else {
        cnt = m_VisibleGroupsCount;
        md = m_VisibleGroups;
    }

    while (cnt > 0) {
        if (*md)
            (*md)->BeforeDraw();
        md++;
        cnt--;
    }
}

void CMatrixMap::BeforeDrawLandscapeSurfaces(bool all) {
    DTRACE();

    CTerSurface::BeforeDrawAll();
    if (m_Macrotexture)
        m_Macrotexture->Prepare();

    int cnt;
    CMatrixMapGroup **md;

    if (all) {
        cnt = m_GroupSize.x * m_GroupSize.y;
        md = m_Group;
    }
    else {
        cnt = m_VisibleGroupsCount;
        md = m_VisibleGroups;
    }

    while (cnt > 0) {
        if ((*md))
            (*md)->BeforeDrawSurfaces();
        md++;
        cnt--;
    }
}

void CMatrixMap::DrawLandscapeSurfaces(bool all) {
    DTRACE();

    for (int i = 0; i < 8; i++) {
        ASSERT_DX(g_D3DD->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&m_BiasTer))));
        ASSERT_DX(g_Sampler.SetState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
        ASSERT_DX(g_Sampler.SetState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
        ASSERT_DX(g_Sampler.SetState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));
    }

    // ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD,&GetIdentityMatrix()));

    // int cnt;
    // CMatrixMapGroup * * md;

    //   if (all)
    //   {
    //    cnt = m_GroupSize.x*m_GroupSize.y;
    //    md = m_Group;
    //   } else
    //   {
    //    cnt = m_VisibleGroupsCount;
    //    md = m_VisibleGroups;
    //   }
    // while(cnt>0)
    //   {
    //       if (*md) (*md)->DrawSurfaces();
    //       md++;
    //	cnt--;
    //   }

    CTerSurface::DrawAll();
}

void CMatrixMap::DrawLandscape(bool all) {
    DTRACE();

#ifdef _DEBUG
    CDText::T("land_dp_calls", CMatrixMapGroup::m_DPCalls);
    CMatrixMapGroup::m_DPCalls = 0;
#endif

    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&m_BiasTer))));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));

    // ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD,&GetIdentityMatrix()));

    ASSERT_DX(g_D3DD->SetFVF(MATRIX_MAP_BOTTOM_VERTEX_FORMAT));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, FALSE));

    int cnt;
    CMatrixMapGroup **md;

    if (all) {
        cnt = m_GroupSize.x * m_GroupSize.y;
        md = m_Group;
        while (cnt > 0) {
            if ((*md))
                (*md)->Draw();
            md++;
            cnt--;
        }
    }
    else {
        cnt = m_VisibleGroupsCount;
        md = m_VisibleGroups;
        while (cnt > 0) {
            (*md)->Draw();

            ///*
#if DRAW_LANDSCAPE_SETKA == 1
            CHelper::Create(1, 0)->Line(D3DXVECTOR3((*md)->GetPos0().x, (*md)->GetPos0().y, 10.0f),
                                        D3DXVECTOR3((*md)->GetPos0().x, (*md)->GetPos1().y, 10.0f), 0xFFFF0000,
                                        0xFFFF0000);

            CHelper::Create(1, 0)->Line(D3DXVECTOR3((*md)->GetPos0().x, (*md)->GetPos1().y, 10.0f),
                                        D3DXVECTOR3((*md)->GetPos1().x, (*md)->GetPos1().y, 10.0f), 0xFFFF0000,
                                        0xFFFF0000);

            CHelper::Create(1, 0)->Line(D3DXVECTOR3((*md)->GetPos1().x, (*md)->GetPos1().y, 10.0f),
                                        D3DXVECTOR3((*md)->GetPos1().x, (*md)->GetPos0().y, 10.0f), 0xFFFF0000,
                                        0xFFFF0000);

            CHelper::Create(1, 0)->Line(D3DXVECTOR3((*md)->GetPos1().x, (*md)->GetPos0().y, 10.0f),
                                        D3DXVECTOR3((*md)->GetPos0().x, (*md)->GetPos0().y, 10.0f), 0xFFFF0000,
                                        0xFFFF0000);

            //*/
#endif

            md++;
            cnt--;
        }
    }

    /*
        for (int x = 0; x < m_Size.x; ++x)
            for (int y = 0; y < m_Size.y; ++y)
            {
                D3DXVECTOR3 p0(x * GLOBAL_SCALE, y*GLOBAL_SCALE, 30);
                D3DXVECTOR3 p1((x+1) * GLOBAL_SCALE, y*GLOBAL_SCALE, 30);
                D3DXVECTOR3 p2((x+1) * GLOBAL_SCALE, (y+1)*GLOBAL_SCALE, 30);
                SMatrixMapUnit *mu = UnitGet(x,y);
                if (mu->IsBridge())
                {
                    CHelper::Create(1,0)->Triangle(p0,p1,p2,0xFFFF0000);

                } else
                {
                    //CHelper::Create(1,0)->Triangle(p0,p1,p2,0xFF0000FF);
                }
            }
    */
}

void CMatrixMap::DrawObjects(void) {
    DTRACE();

    // ASSERT_DX(g_D3DD->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX,0));
    // ASSERT_DX(g_D3DD->SetTextureStageState(2,D3DTSS_COLOROP,D3DTOP_DISABLE));

    CVectorObject::DrawBegin();

    // if (m_KeyScan != KEY_L || m_KeyDown == false)

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, TRUE));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, m_AmbientColorObj));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF));

    //   ASSERT_DX(g_D3DD->SetRenderState( D3DRS_TEXTUREFACTOR, m_AmbientColorObj ));
    // ASSERT_DX(g_D3DD->SetRenderState( D3DRS_AMBIENT, 0xFFFFFFFF));

    for (int i = 0; i < 8; i++) {
        ASSERT_DX(g_D3DD->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&m_BiasRobots))));
        ASSERT_DX(g_Sampler.SetState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
        ASSERT_DX(g_Sampler.SetState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
        ASSERT_DX(g_Sampler.SetState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));
    }

    // draw all objects
    CMatrixMapStatic::SortEndDraw();

    // draw cannon for build

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_LIGHTING, FALSE));

    CMatrixSideUnit *player_side = GetPlayerSide();
    if (player_side->m_CannonForBuild.m_Cannon) {
        if (player_side->m_CannonForBuild.m_Cannon->IsVisible())
            player_side->m_CannonForBuild.m_Cannon->Draw();
    }

    CVectorObject::DrawEnd();
}

void CMatrixMap::DrawWater(void) {
    DTRACE();

    if (!m_Water->IsReadyForDraw())
        return;

    // g_D3DD->SetRenderState( D3DRS_NORMALIZENORMALS,  TRUE );

    // g_D3DD->SetMaterial(&mtrl );
    // g_D3DD->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
    // g_D3DD->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR1);

    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));

    for (int i = 0; i < 4; i++) {
        ASSERT_DX(g_D3DD->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&m_BiasWater))));

        ASSERT_DX(g_D3DD->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
        ASSERT_DX(g_D3DD->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
    }

    D3DXMATRIX m = this->GetIdentityMatrix();

    m._11 *= GLOBAL_SCALE * (float)(MAP_GROUP_SIZE) / (float)(WATER_SIZE);
    m._22 *= GLOBAL_SCALE * (float)(MAP_GROUP_SIZE) / (float)(WATER_SIZE);
    m._33 *= GLOBAL_SCALE * (float)(MAP_GROUP_SIZE) / (float)(WATER_SIZE);
    m._43 = WATER_LEVEL;

    int curpass;

    for (curpass = 0; curpass < g_Render->m_WaterPassAlpha; ++curpass) {
        g_Render->m_WaterAlpha(m_Water->m_WaterTex1, m_Water->m_WaterTex2, curpass);

        int cnt = m_VisibleGroupsCount;
        CMatrixMapGroup **md = m_VisibleGroups;

        while ((cnt--) > 0) {
            if (!(*(md))->HasWater()) {
                ++md;
                continue;
            }

            if (curpass == 0)
                ASSERT_DX(g_D3DD->SetTexture(0, (*md)->GetWaterAlpha()->Tex()));

            const D3DXVECTOR2 &p = (*(md))->GetPos0();
            m._41 = p.x;
            m._42 = p.y;
            m_Water->Draw(m);
            ++md;
        }
    }

    g_Render->m_WaterClearAlpha();

    for (curpass = 0; curpass < g_Render->m_WaterPassSolid; ++curpass) {
        g_Render->m_WaterSolid(m_Water->m_WaterTex1, m_Water->m_WaterTex2, curpass);

        for (const auto& item : m_VisWater)
        {
            m._41 = item.x;
            m._42 = item.y;
            m_Water->Draw(m);
        }
    }

    g_Render->m_WaterClearSolid();

    // g_D3DD->SetRenderState( D3DRS_NORMALIZENORMALS,  FALSE );

    int cnt;
    CMatrixMapGroup **md;
    if (SInshorewave::m_Tex) {
        SInshorewave::DrawBegin();

        cnt = m_VisibleGroupsCount;
        md = m_VisibleGroups;
        while ((cnt--) > 0) {
            if (!(*(md))->HasWater()) {
                ++md;
                continue;
            }

            (*md)->DrawInshorewaves();
            ++md;
        }
        SInshorewave::DrawEnd();
    }
}

void CMatrixMap::DrawShadowsProjFast(void) {
    DTRACE();

    CVOShadowProj::BeforeRenderAll();

    // g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE,		TRUE);
    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

    g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, m_ShadowColor);

    SetColorOpSelect(0, D3DTA_TFACTOR);
    SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetColorOpDisable(1);

    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));

    CMatrixMapStatic::SortEndDrawShadowProj();
    int cnt = m_VisibleGroupsCount;
    CMatrixMapGroup **md = m_VisibleGroups;
    while (cnt > 0) {
        (*md)->DrawShadowProj();
        md++;
        cnt--;
    }

    // g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE,		FALSE);
    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));
}

void CMatrixMap::DrawShadows(void) {
    DTRACE();

    CVOShadowStencil::BeforeRenderAll();

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &GetIdentityMatrix()));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILREF, 0x1));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILMASK, 0xffffffff));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));

    if (g_D3DDCaps.StencilCaps & D3DSTENCILCAPS_TWOSIDED) {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, TRUE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CCW_STENCILFUNC, D3DCMP_ALWAYS));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_KEEP));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_DECR));

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));

        CMatrixMapStatic::SortEndDrawShadowStencil();

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE));
    }
    else {
        CMatrixMapStatic::SortEndDrawShadowStencil();

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_DECR));

        CMatrixMapStatic::SortEndDrawShadowStencil();
    }
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR));

#ifdef RENDER_PROJ_SHADOWS_IN_STENCIL_PASS
    // Shadow proj

    CVOShadowProj::BeforeRenderAll();

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &GetIdentityMatrix()));

    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHAREF, 0x8));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILREF, 0x1));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILMASK, 0xffffffff));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE));

    SetColorOpSelect(0, D3DTA_TEXTURE);
    SetAlphaOpSelect(0, D3DTA_TEXTURE);
    SetColorOpDisable(1);

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0));

    CMatrixMapStatic::SortEndDrawShadowProj();
    int cnt = m_VisibleGroupsCount;
    CMatrixMapGroup **md = m_VisibleGroups;
    while (cnt > 0) {
        (*md)->DrawShadowProj();
        md++;
        cnt--;
    }

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF));

    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

    ASSERT_DX(g_D3DD->SetTexture(0, NULL));
#endif

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_DECR));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILENABLE, FALSE));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0xC0C0C0C0 /* 0xffffffff*/));

    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_TFACTOR);
    SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_DIFFUSE, D3DTA_TFACTOR);
    SetColorOpDisable(1);

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILREF, 0x1));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP));

    ASSERT_DX(g_D3DD->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE));
    ASSERT_DX(g_D3DD->SetStreamSource(0, GET_VB(m_ShadowVB), 0, sizeof(SShadowRectVertex)));
    ASSERT_DX(g_D3DD->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_STENCILENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
}

void CMatrixMap::DrawEffects(void) {
    DTRACE();

    CMatrixEffect::DrawBegin();

    // CSortable::SortBegin();

    for (PCMatrixEffect e = m_EffectsFirst; e != NULL; e = e->m_Next) {
        e->Draw();
    }

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &GetIdentityMatrix()));
    CBillboard::SortEndDraw(m_Camera.GetViewInversedMatrix(), m_Camera.GetFrustumCenter());
    CMatrixEffect::DrawEnd();
}

void CMatrixMap::DrawSky(void) {
    DTRACE();

    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

    g_D3DD->SetRenderState(D3DRS_FOGENABLE, FALSE);
    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

    if (g_Config.m_SkyBox != 0 && m_SkyTex[0].tex) {
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
        ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));

        SetColorOpSelect(0, D3DTA_TEXTURE);
        SetColorOpDisable(1);
        SetAlphaOpDisable(0);

        g_D3DD->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

        D3DXMATRIX wo;
        D3DXMatrixPerspectiveFovLH(&wo, CAM_HFOV, float(g_ScreenX) * m_Camera.GetResYInversed(), 0.01f, 3);
        ASSERT_DX(g_D3DD->SetTransform(D3DTS_PROJECTION, &wo));

        ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &GetIdentityMatrix()));

        m_Camera.CalcSkyMatrix(wo);
        ASSERT_DX(g_D3DD->SetTransform(D3DTS_VIEW, &wo));
        // ASSERT_DX(g_D3DD->SetTransform( D3DTS_VIEW, &GetIdentityMatrix() ));

        SVert_V3_C_UV verts[4];
        verts[0].color = 0xFFFFFFFF;
        verts[1].color = 0xFFFFFFFF;
        verts[2].color = 0xFFFFFFFF;
        verts[3].color = 0xFFFFFFFF;

        float cut_dn = (200 + m_Camera.GetFrustumCenter().z) * 0.5f * INVERT(MAX_VIEW_DISTANCE) + 0.5f;
        float cut_up = 0.0f;

        float geo_dn = 2 * (1.0f - cut_dn) - 1;

        CInstDraw::BeginDraw(IDFVF_V3_C_UV);

        int tex = SKY_FORE;
        float v1 = (m_SkyTex[tex].v1 - m_SkyTex[tex].v0) * cut_dn + m_SkyTex[tex].v0;
        verts[0].p = D3DXVECTOR3(-1, 1, 1);
        verts[0].tu = m_SkyTex[tex].u0;
        verts[0].tv = m_SkyTex[tex].v0;
        verts[1].p = D3DXVECTOR3(-1, geo_dn, 1);
        verts[1].tu = m_SkyTex[tex].u0;
        verts[1].tv = v1;
        verts[2].p = D3DXVECTOR3(1, 1, 1);
        verts[2].tu = m_SkyTex[tex].u1;
        verts[2].tv = m_SkyTex[tex].v0;
        verts[3].p = D3DXVECTOR3(1, geo_dn, 1);
        verts[3].tu = m_SkyTex[tex].u1;
        verts[3].tv = v1;

        CInstDraw::AddVerts(verts, m_SkyTex[tex].tex);

        tex = SKY_RITE;
        v1 = (m_SkyTex[tex].v1 - m_SkyTex[tex].v0) * cut_dn + m_SkyTex[tex].v0;
        verts[0].p = D3DXVECTOR3(1, 1, 1);
        verts[0].tu = m_SkyTex[tex].u0;
        verts[0].tv = m_SkyTex[tex].v0;
        verts[1].p = D3DXVECTOR3(1, geo_dn, 1);
        verts[1].tu = m_SkyTex[tex].u0;
        verts[1].tv = v1;
        verts[2].p = D3DXVECTOR3(1, 1, -1);
        verts[2].tu = m_SkyTex[tex].u1;
        verts[2].tv = m_SkyTex[tex].v0;
        verts[3].p = D3DXVECTOR3(1, geo_dn, -1);
        verts[3].tu = m_SkyTex[tex].u1;
        verts[3].tv = v1;

        CInstDraw::AddVerts(verts, m_SkyTex[tex].tex);

        tex = SKY_LEFT;
        v1 = (m_SkyTex[tex].v1 - m_SkyTex[tex].v0) * cut_dn + m_SkyTex[tex].v0;
        verts[0].p = D3DXVECTOR3(-1, 1, -1);
        verts[0].tu = m_SkyTex[tex].u0;
        verts[0].tv = m_SkyTex[tex].v0;
        verts[1].p = D3DXVECTOR3(-1, geo_dn, -1);
        verts[1].tu = m_SkyTex[tex].u0;
        verts[1].tv = v1;
        verts[2].p = D3DXVECTOR3(-1, 1, 1);
        verts[2].tu = m_SkyTex[tex].u1;
        verts[2].tv = m_SkyTex[tex].v0;
        verts[3].p = D3DXVECTOR3(-1, geo_dn, 1);
        verts[3].tu = m_SkyTex[tex].u1;
        verts[3].tv = v1;

        CInstDraw::AddVerts(verts, m_SkyTex[tex].tex);

        tex = SKY_BACK;
        v1 = (m_SkyTex[tex].v1 - m_SkyTex[tex].v0) * cut_dn + m_SkyTex[tex].v0;
        verts[0].p = D3DXVECTOR3(1, 1, -1);
        verts[0].tu = m_SkyTex[tex].u0;
        verts[0].tv = m_SkyTex[tex].v0;
        verts[1].p = D3DXVECTOR3(1, geo_dn, -1);
        verts[1].tu = m_SkyTex[tex].u0;
        verts[1].tv = v1;
        verts[2].p = D3DXVECTOR3(-1, 1, -1);
        verts[2].tu = m_SkyTex[tex].u1;
        verts[2].tv = m_SkyTex[tex].v0;
        verts[3].p = D3DXVECTOR3(-1, geo_dn, -1);
        verts[3].tu = m_SkyTex[tex].u1;
        verts[3].tv = v1;

        CInstDraw::AddVerts(verts, m_SkyTex[tex].tex);

        CInstDraw::ActualDraw();

        g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

        float SH1 = float(g_ScreenY * 0.270416666666667);
        float SH2 = float(g_ScreenY * 0.07);

        g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

        DWORD m_SkyColorUp = m_SkyColor & 0x00FFFFFF;
        SVert_V4_C v[6];
        v[0].p = D3DXVECTOR4(0, m_SkyHeight - SH1, 0.0f, 1.0f);
        v[0].col = m_SkyColorUp;
        v[1].p = D3DXVECTOR4(float(g_ScreenX), m_SkyHeight - SH1, 0.0f, 1.0f);
        v[1].col = m_SkyColorUp;
        v[2].p = D3DXVECTOR4(0, m_SkyHeight - SH2, 0.0f, 1.0f);
        v[2].col = m_SkyColor;
        v[3].p = D3DXVECTOR4(float(g_ScreenX), m_SkyHeight - SH2, 0.0f, 1.0f);
        v[3].col = m_SkyColor;
        v[4].p = D3DXVECTOR4(0, m_SkyHeight, 0.0f, 1.0f);
        v[4].col = m_SkyColor;
        v[5].p = D3DXVECTOR4(float(g_ScreenX), m_SkyHeight, 0.0f, 1.0f);
        v[5].col = m_SkyColor;

        SetAlphaOpSelect(0, D3DTA_DIFFUSE);

        SetColorOpSelect(0, D3DTA_DIFFUSE);
        SetColorOpDisable(1);

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, FALSE));

        CInstDraw::BeginDraw(IDFVF_V4_C);
        CInstDraw::AddVerts(v, NULL);
        CInstDraw::AddVerts(v + 2, NULL);
        CInstDraw::ActualDraw();
    }
    else {
        // do not draw skybox

        float SH1 = float(g_ScreenY * 0.270416666666667);
        float SH2 = float(g_ScreenY * 0.07);

        DWORD m_SkyColorUp = 0x00000000;
        SVert_V4_C v[6];
        v[0].p = D3DXVECTOR4(0, m_SkyHeight - SH1, 0.0f, 1.0f);
        v[0].col = m_SkyColorUp;
        v[1].p = D3DXVECTOR4(float(g_ScreenX), m_SkyHeight - SH1, 0.0f, 1.0f);
        v[1].col = m_SkyColorUp;

        v[2].p = D3DXVECTOR4(0, m_SkyHeight - SH2, 0.0f, 1.0f);
        v[2].col = m_SkyColor;
        v[3].p = D3DXVECTOR4(float(g_ScreenX), m_SkyHeight - SH2, 0.0f, 1.0f);
        v[3].col = m_SkyColor;

        v[4].p = D3DXVECTOR4(0, m_SkyHeight, 0.0f, 1.0f);
        v[4].col = m_SkyColor;
        v[5].p = D3DXVECTOR4(float(g_ScreenX), m_SkyHeight, 0.0f, 1.0f);
        v[5].col = m_SkyColor;

        if (v[0].p.y > 0) {
            v[0].p.y = 0;
            v[1].p.y = 0;
        }

        SetAlphaOpDisable(0);

        SetColorOpSelect(0, D3DTA_DIFFUSE);
        SetColorOpDisable(1);

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, FALSE));

        CInstDraw::BeginDraw(IDFVF_V4_C);
        CInstDraw::AddVerts(v, NULL);
        CInstDraw::AddVerts(v + 2, NULL);
        CInstDraw::ActualDraw();

        // D3DRECT r;
        // r.x1 = 0;
        // r.y1 = Float2Int(g_MatrixMap->m_SkyHeight);
        // r.x2 = g_ScreenX;
        // r.y2 = r.y1 + 20;
        // ASSERT_DX(g_D3DD->Clear( 1, &r, D3DCLEAR_TARGET, g_MatrixMap->m_SkyColor, 1.0f, 0 ));
    }

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, TRUE));
    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CMatrixMap::Draw(void) {
    DTRACE();

    float fBias = -1.0f;

    if (FLAG(g_MatrixMap->m_Flags, MMFLAG_SKY_ON)) {
        DrawSky();
    }

    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP));

    // enable fogging

    if (FLAG(g_MatrixMap->m_Flags, MMFLAG_FOG_ON)) {
        float Start = float(MAX_VIEW_DISTANCE * FOG_NEAR_K);  // For linear mode
        float End = float(MAX_VIEW_DISTANCE * FOG_FAR_K);
        if (g_D3DDCaps.RasterCaps & D3DPRASTERCAPS_WFOG) {}
        else {
            Start /= MAX_VIEW_DISTANCE;
            End /= MAX_VIEW_DISTANCE;
        }
        // Enable fog blending.
        g_D3DD->SetRenderState(D3DRS_FOGENABLE, TRUE);

        // Set the fog color.
        g_D3DD->SetRenderState(D3DRS_FOGCOLOR, m_SkyColor);

        g_D3DD->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
        g_D3DD->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
        g_D3DD->SetRenderState(D3DRS_FOGSTART, *(DWORD *)(&Start));
        g_D3DD->SetRenderState(D3DRS_FOGEND, *(DWORD *)(&End));
    }
    else {
        g_D3DD->SetRenderState(D3DRS_FOGENABLE, FALSE);
    }

    // one per frame
    ASSERT_DX(g_D3DD->SetTransform(D3DTS_VIEW, &m_Camera.GetViewMatrix()));
    ASSERT_DX(g_D3DD->SetTransform(D3DTS_PROJECTION, &m_Camera.GetProjMatrix()));

    // DrawObjects();
    // DrawLandscape();

    RESETFLAG(m_Flags, MMFLAG_OBJECTS_DRAWN);
    if (FLAG(g_Flags, GFLAG_STENCILAVAILABLE) && g_Config.m_IzvratMS) {
        bool domask = CMultiSelection::DrawAllPass1Begin();
        DrawObjects();
        DrawLandscape();
        if (CTerSurface::IsSurfacesPresent())
            DrawLandscapeSurfaces();
        if (domask) {
            CMultiSelection::DrawAllPass2Begin();
            SETFLAG(m_Flags, MMFLAG_OBJECTS_DRAWN);
            DrawObjects();

            // g_D3DD->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
            // CMatrixMapGroup::BeforeDrawAll();
            // DrawLandscape();
            CMultiSelection::DrawAllPassEnd();
        }
    }
    else {
        DrawObjects();
        DrawLandscape();
        if (CTerSurface::IsSurfacesPresent())
            DrawLandscapeSurfaces();
    }

    for (int i = 0; i < 4; i++) {
        ASSERT_DX(g_D3DD->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&(fBias)))));
    }

    CMatrixEffectLandscapeSpot::DrawAll();  // we should draw landscape spots immediately after draw landscape

    DrawWater();

    fBias = 0.0f;
    for (int i = 0; i < 4; i++) {
        ASSERT_DX(g_D3DD->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&(fBias)))));
    }
#ifndef RENDER_PROJ_SHADOWS_IN_STENCIL_PASS
    if (g_Config.m_ShowProjShadows) {
        DrawShadowsProjFast();
    }
#endif
    if (g_Config.m_ShowStencilShadows) {
        DrawShadows();
    }

    DrawEffects();

    for (int od = 0; od < m_AD_Obj_cnt; ++od) {
        if (m_AD_Obj[od]->GetObjectType() == OBJECT_TYPE_FLYER) {
            ((CMatrixFlyer *)m_AD_Obj[od])->DrawPropeller();
        }
    }

    // CDText::T("COL", (int)GetColor(m_TraceStopPos.x, m_TraceStopPos.y));

    /*
        CHelper::Create(1,0)->Line(D3DXVECTOR3(m_TraceStopPos.x, m_TraceStopPos.y, -100),
                                D3DXVECTOR3(m_TraceStopPos.x, m_TraceStopPos.y, 100));

        D3DXVECTOR2 dir;
        if (CalcVectorToLandscape(D3DXVECTOR2(m_TraceStopPos.x, m_TraceStopPos.y), dir))

        {

            CHelper::Create(1,0)->Line(D3DXVECTOR3(m_TraceStopPos.x, m_TraceStopPos.y, 10),
                                    D3DXVECTOR3(m_TraceStopPos.x, m_TraceStopPos.y, 10) + 100 * D3DXVECTOR3(dir.x,
       dir.y, 0));
        }
        */

    // m_Minimap.Draw();

    CMatrixProgressBar::DrawAll();
    CMultiSelection::DrawAll();

    //#ifdef _DEBUG
    //
    //    {
    //        static CTextureManaged *test = NULL;
    //
    //        if (m_KeyDown && m_KeyScan == KEY_R)
    //        {
    //            if (IS_TRACE_STOP_OBJECT(m_TraceStopObj))
    //            {
    //
    //                if (test)
    //                {
    //                    test->FreeReminder();
    //                    g_Cache->Destroy(test);
    //                }
    //
    //                test = m_TraceStopObj->RenderToTexture(TEXSIZE_128);
    //            }
    //        }
    //
    //#define SME  256
    //#define SZ  128
    //
    //        if (test)
    //        {
    //            struct { D3DXVECTOR4 v; float tu, tv; } v[4] =
    //            {
    //                { D3DXVECTOR4(0+SME,  SZ+SME, 0.0f, 1.0f), 0,1},
    //                { D3DXVECTOR4(0.0f+SME, 0.0f+SME, 0.0f, 1.0f), 0,0},
    //                { D3DXVECTOR4(SZ+SME,  SZ+SME, 0.0f, 1.0f), 1,1},
    //                { D3DXVECTOR4(SZ+SME, 0+SME, 0.0f, 1.0f), 1,0}
    //            };
    //
    //            g_D3DD->SetTexture(0,test->Tex());
    //
    //            SetAlphaOpSelect(0, D3DTA_TEXTURE);
    //            SetColorOpSelect(0, D3DTA_TEXTURE);
    //            SetColorOpDisable(1);
    //
    //            ASSERT_DX(g_D3DD->SetFVF(D3DFVF_XYZRHW|D3DFVF_TEX1));
    //            ASSERT_DX(g_D3DD->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &v, sizeof(D3DXVECTOR4)+8 ));
    //
    //        }
    //
    //    }
    //
    //#endif

    // shader
    // if (m_ShadeOn)
    //{
    //    float k = (1.0f-(float(m_ShadeTime)*INVERT(SHADER_TIME)));
    //    float y = k * float(g_ScreenY) * (SHADER_PERC / 100.0f);

    //    D3DXVECTOR4 v[8] =
    //    {
    //        D3DXVECTOR4(0.0f, y, 0.0f, 1.0f),
    //        D3DXVECTOR4(0,  0, 0.0f, 1.0f),
    //        D3DXVECTOR4(float(g_ScreenX), y, 0.0f, 1.0f),
    //        D3DXVECTOR4(float(g_ScreenX),  0, 0.0f, 1.0f),

    //        D3DXVECTOR4(0.0f, float(g_ScreenY), 0.0f, 1.0f),
    //        D3DXVECTOR4(0,  float(g_ScreenY)-y, 0.0f, 1.0f),
    //        D3DXVECTOR4(float(g_ScreenX), float(g_ScreenY), 0.0f, 1.0f),
    //        D3DXVECTOR4(float(g_ScreenX),  float(g_ScreenY)-y, 0.0f, 1.0f)

    //    };
    //

    // ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR,		0x0));

    //    SetAlphaOpDisable(0);
    //    SetColorOpSelect(0, D3DTA_TFACTOR);
    //    SetColorOpDisable(1);
    //
    //    ASSERT_DX(g_D3DD->SetFVF(D3DFVF_XYZRHW|D3DFVF_DIFFUSE));
    //    ASSERT_DX(g_D3DD->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &v, sizeof(D3DXVECTOR4) ));
    //    ASSERT_DX(g_D3DD->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &v[4], sizeof(D3DXVECTOR4) ));

    //    if (m_ShadeTimeCurrent > 3000)
    //    {
    //        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE,		TRUE));
    //        float k = (float(m_ShadeTimeCurrent-3000) * INVERT(2000.0f));
    //        if (k < 0) k = 0;
    //        if (k > 1) k = 1;
    //        BYTE a = BYTE(k*255.0);
    //        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, (a << 24)));

    //        D3DXVECTOR4 v[4] =
    //        {
    //            D3DXVECTOR4(0,  float(g_ScreenY)-y, 0.0f, 1.0f),
    //            D3DXVECTOR4(0.0f, y, 0.0f, 1.0f),
    //            D3DXVECTOR4(float(g_ScreenX),  float(g_ScreenY)-y, 0.0f, 1.0f),
    //            D3DXVECTOR4(float(g_ScreenX), y, 0.0f, 1.0f),
    //        };

    //        SetAlphaOpSelect(0, D3DTA_TFACTOR);

    //        ASSERT_DX(g_D3DD->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &v, sizeof(D3DXVECTOR4) ));
    //        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE,		FALSE));

    //    }

    //    m_Camera.SetTarget(m_ShadePosFrom + (m_ShadePosTo-m_ShadePosFrom) * k);

    //}

    if (IsPaused()) {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0x14000000 /* 0xffffffff*/));

        SetColorOpSelect(0, D3DTA_TFACTOR);
        SetAlphaOpSelect(0, D3DTA_TFACTOR);
        SetColorOpDisable(1);

        ASSERT_DX(g_D3DD->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE));
        ASSERT_DX(g_D3DD->SetStreamSource(0, GET_VB(m_ShadowVB), 0, sizeof(SShadowRectVertex)));
        ASSERT_DX(g_D3DD->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    }

#if (defined _DEBUG) && !(defined _RELDEBUG)
    CHelper::DrawAll();
#endif

    m_DI.Draw();  // debug info

    if (m_DialogModeName) {
        if (wcscmp(m_DialogModeName, TEMPLATE_DIALOG_MENU) == 0) {
            m_DialogModeHints[0]->DrawNow();
        }
    }

    g_IFaceList->Render();
    CMatrixProgressBar::DrawAllClones();
    CMatrixHint::DrawAll();

    //_________________________________________________________________________________________________

    if (FLAG(m_Flags, MMFLAG_TRANSITION)) {
        m_Transition.Render();
    }

    //_________________________________________________________________________________________________

    m_Cursor.Draw();
}

void CMatrixMap::Takt(int step) {
    DTRACE();

    float fstep = float(step);

    m_SkyAngle += m_SkyDeltaAngle * step;

    int i;

    DCP();

    CMatrixMapStatic::SortEndGraphicTakt(step);

    DCP();

    int cnt = m_VisibleGroupsCount;
    CMatrixMapGroup **md = m_VisibleGroups;
    while ((cnt--) > 0) {
        (*(md++))->GraphicTakt(step);
    }

    DCP();

    CSkinManager::Takt(fstep);
    DCP();
    m_Water->Takt(step);
    DCP();
    m_Cursor.Takt(step);
    DCP();
    m_DI.Takt(step);
    DCP();

    for (i = 0; i < m_EffectSpawnersCnt; ++i) {
        m_EffectSpawners[i].Takt(fstep);
    }
    DCP();
    RemoveEffectSpawnerByTime();
    DCP();

    // SETFLAG(m_Flags,MMFLAG_EFF_TAKT);
    for (PCMatrixEffect e = m_EffectsFirst; e != NULL;) {
#ifdef DEAD_PTR_SPY_ENABLE
        CMatrixEffect *deade = (CMatrixEffect *)DeadPtr::get_dead_mem(e);
        if (deade) {
            debugbreak();
        }
#endif
#ifdef DEAD_CLASS_SPY_ENABLE
        CMatrixEffectLandscapeSpot *spot = (CMatrixEffectLandscapeSpot *)e->DCS_GetDeadBody();
        if (spot) {
            debugbreak();
        }

#endif

        m_EffectsNextTakt = e->m_Next;
        DCP();
        e->Takt(fstep);
        DCP();
        e = m_EffectsNextTakt;
    }
    DCP();
    // RESETFLAG(m_Flags,MMFLAG_EFF_TAKT);

    //// some effects must be removed
    // while (m_Effects2RemoveFirst)
    //{
    //    PCMatrixEffect e = m_Effects2RemoveFirst;

    //
    //    // delete from base effect list
    //    LIST_DEL(e, m_EffectsFirst, m_EffectsLast, m_Prev, m_Next);

    //    //remove from death list
    //    LIST_DEL(e, m_Effects2RemoveFirst, m_Effects2RemoveLast, m_PrevDel, m_NextDel);

    //    e->Release();
    //}

    DCP();
    m_Minimap.Takt(fstep);
    DCP();

    CSound::Takt();

    //{
    //    typedef CBillboard *PCBillboard;
    //    static bool inited = false;
    //    static PCBillboard ar[8192];
    //    static int bn = 3024;
    //    if (!inited)
    //    {
    //        D3DXVECTOR3 pos(300,100,30);
    //        float a = 0;
    //        float r = 30;
    //        for (int i=0; i<bn; ++i)
    //        {
    //            float sn, cs;
    //            SinCos(a, &sn, &cs);

    //            ar[i] = HNew(g_CacheHeap) CBillboard(TRACE_PARAM_CALL pos + D3DXVECTOR3(sn*r,cs*r,0), 3, a,0xFF400401,
    //            false, CMatrixEffect::GetBBTex(BBT_SMOKE));

    //            a += GRAD2RAD(1);
    //            r += 0.05f;
    //            pos.z += 0.05f;
    //        }

    //        inited = true;
    //    }
    //    for (int i=0; i<bn; ++i)
    //    {
    //        ar[i]->Sort(m_Camera.GetViewMatrix());
    //    }
    //}

    DCP();
}

bool CMatrixMap::FindObjects(const D3DXVECTOR2 &pos, float radius, float oscale, DWORD mask, CMatrixMapStatic *skip,
                             ENUM_OBJECTS2D callback, DWORD user) {
#ifdef _DEBUG
    static int intercount = 0;

    ++intercount;
    if (intercount != 1) {
        ERROR_S(L"CMatrixMap::FindObjects cannot be called recursively");
    }
#endif

    bool hit = false;

    D3DXVECTOR2 vmin(pos.x - radius, pos.y - radius), vmax(pos.x + radius, pos.y + radius);

    int minx1, miny1, maxx1, maxy1;

    const float groupsize = GLOBAL_SCALE * MAP_GROUP_SIZE;
    const float obr_groupsize = 1.0f / groupsize;

    minx1 = TruncFloat(vmin.x * obr_groupsize);
    miny1 = TruncFloat(vmin.y * obr_groupsize);

    maxx1 = TruncFloat(vmax.x * obr_groupsize);
    maxy1 = TruncFloat(vmax.y * obr_groupsize);
    if (maxx1 < minx1) {
        maxx1 ^= minx1;
        minx1 ^= maxx1;
        maxx1 ^= minx1;
    }
    if (maxy1 < miny1) {
        maxy1 ^= miny1;
        miny1 ^= maxy1;
        maxy1 ^= miny1;
    }
    ++maxx1;
    ++maxy1;

    ++m_IntersectFlagFindObjects;

    if (minx1 < 0) {
        minx1 = 0;
        if (0 > maxx1)
            goto skip;
    }
    if (maxx1 >= m_GroupSize.x) {
        maxx1 = m_GroupSize.x - 1;
        if (maxx1 < minx1)
            goto skip;
    }
    if (miny1 < 0) {
        miny1 = 0;
        if (0 > maxy1)
            goto skip;
    }
    if (maxy1 >= m_GroupSize.y) {
        maxy1 = m_GroupSize.y - 1;
        if (maxy1 < miny1)
            goto skip;
    }

    for (int x = minx1; x <= maxx1; ++x) {
        for (int y = miny1; y <= maxy1; ++y) {
            PCMatrixMapGroup g = GetGroupByIndex(x, y);
            if (g == NULL)
                continue;
            int i = 0;
            CMatrixMapStatic *ms;
            CMatrixMapStatic *ms2 = NULL;
            for (;;) {
                if (ms2 == NULL) {
                    ms = g->FindObjectAny(mask, pos, radius, oscale, i);
                }
                else {
                    ms = ms2;
                    ms2 = NULL;
                }
                if (ms == NULL)
                    break;

                if (ms->m_IntersectFlagFindObjects == m_IntersectFlagFindObjects)
                    continue;
                ms->m_IntersectFlagFindObjects = m_IntersectFlagFindObjects;

                if (ms == skip)
                    continue;
                if (ms->IsFlyer()) {
                    ms2 = ((CMatrixFlyer *)ms)->GetCarryingRobot();
                    if (ms2 != NULL) {
                        auto tmp = *(D3DXVECTOR2 *)&ms2->GetGeoCenter() - pos;
                        float dist = D3DXVec2Length(&tmp) -
                                     ms2->GetRadius() * oscale;
                        if (dist >= radius) {
                            ms2 = NULL;
                        }
                    }
                }

                hit = true;
                if (callback) {
                    if (!callback(pos, ms, user)) {
#ifdef _DEBUG
                        --intercount;
#endif

                        return hit;
                    }
                }
                else {
#ifdef _DEBUG
                    --intercount;
#endif

                    return true;
                }
            }
        }
    }

skip:;
    for (int od = 0; od < m_AD_Obj_cnt; ++od) {
        if (m_AD_Obj[od]->m_IntersectFlagFindObjects != m_IntersectFlagFindObjects) {
            m_AD_Obj[od]->m_IntersectFlagFindObjects = m_IntersectFlagFindObjects;

            CMatrixMapStatic *msa[2];
            int mscnt = 1;
            msa[0] = m_AD_Obj[od];

            for (;;) {
                if (mscnt == 0)
                    break;
                CMatrixMapStatic *ms = msa[0];
                if (mscnt == 2)
                    msa[0] = msa[1];
                mscnt--;

                if (ms->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
                    if (((CMatrixRobot *)ms)->m_CurrState == ROBOT_DIP)
                        continue;
                    if ((mask & TRACE_ROBOT) == 0)
                        continue;
                }
                else if (ms->GetObjectType() == OBJECT_TYPE_FLYER) {
                    msa[mscnt] = ((CMatrixFlyer *)ms)->GetCarryingRobot();
                    if (msa[mscnt] != NULL) {
                        auto tmp = *(D3DXVECTOR2 *)&msa[mscnt]->GetGeoCenter() - pos;
                        float dist = D3DXVec2Length(&tmp) -
                                     msa[mscnt]->GetRadius() * oscale;
                        if (dist < radius) {
                            mscnt++;
                        }
                    }
                    if ((mask & TRACE_FLYER) == 0)
                        continue;
                    auto tmp = *(D3DXVECTOR2 *)&ms->GetGeoCenter() - pos;
                    float dist =
                            D3DXVec2Length(&tmp) - ms->GetRadius() * oscale;
                    if (dist >= radius) {
                        continue;
                    }
                }

                if (ms == skip)
                    continue;

                hit = true;
                if (callback) {
                    if (!callback(pos, ms, user)) {
#ifdef _DEBUG
                        --intercount;
#endif

                        return hit;
                    }
                }
                else {
#ifdef _DEBUG
                    --intercount;
#endif

                    return true;
                }
            }
        }
    }

#ifdef _DEBUG
    --intercount;
#endif
    return hit;
}

bool CMatrixMap::FindObjects(const D3DXVECTOR3 &pos, float radius, float oscale, DWORD mask, CMatrixMapStatic *skip,
                             ENUM_OBJECTS callback, DWORD user) {
#ifdef _DEBUG
    static int intercount = 0;

    ++intercount;
    if (intercount != 1) {
        ERROR_S(L"CMatrixMap::FindObjects cannot be called recursively");
    }
#endif

    bool hit = false;

    D3DXVECTOR2 vmin(pos.x - radius, pos.y - radius), vmax(pos.x + radius, pos.y + radius);

    int minx1, miny1, maxx1, maxy1;

    const float groupsize = GLOBAL_SCALE * MAP_GROUP_SIZE;
    const float obr_groupsize = 1.0f / groupsize;

    minx1 = TruncFloat(vmin.x * obr_groupsize);
    miny1 = TruncFloat(vmin.y * obr_groupsize);

    maxx1 = TruncFloat(vmax.x * obr_groupsize);
    maxy1 = TruncFloat(vmax.y * obr_groupsize);
    if (maxx1 < minx1) {
        maxx1 ^= minx1;
        minx1 ^= maxx1;
        maxx1 ^= minx1;
    }
    if (maxy1 < miny1) {
        maxy1 ^= miny1;
        miny1 ^= maxy1;
        maxy1 ^= miny1;
    }
    ++maxx1;
    ++maxy1;

    ++m_IntersectFlagFindObjects;

    if (minx1 < 0) {
        minx1 = 0;
        if (0 > maxx1)
            goto skip;
    }
    if (maxx1 >= m_GroupSize.x) {
        maxx1 = m_GroupSize.x - 1;
        if (maxx1 < minx1)
            goto skip;
    }
    if (miny1 < 0) {
        miny1 = 0;
        if (0 > maxy1)
            goto skip;
    }
    if (maxy1 >= m_GroupSize.y) {
        maxy1 = m_GroupSize.y - 1;
        if (maxy1 < miny1)
            goto skip;
    }

    for (int x = minx1; x <= maxx1; ++x) {
        for (int y = miny1; y <= maxy1; ++y) {
            PCMatrixMapGroup g = GetGroupByIndex(x, y);
            if (g == NULL)
                continue;
            int i = 0;
            CMatrixMapStatic *ms;
            CMatrixMapStatic *ms2 = NULL;
            for (;;) {
                if (ms2 == NULL) {
                    ms = g->FindObjectAny(mask, pos, radius, oscale, i);
                }
                else {
                    ms = ms2;
                    ms2 = NULL;
                }
                if (ms == NULL)
                    break;

                if (ms->m_IntersectFlagFindObjects == m_IntersectFlagFindObjects)
                    continue;
                ms->m_IntersectFlagFindObjects = m_IntersectFlagFindObjects;

                if (ms == skip)
                    continue;
                if (ms->GetObjectType() == OBJECT_TYPE_FLYER) {
                    ms2 = ((CMatrixFlyer *)ms)->GetCarryingRobot();
                    if (ms2 != NULL) {
                        auto tmp = ms2->GetGeoCenter() - pos;
                        float dist = D3DXVec3Length(&tmp) - ms2->GetRadius() * oscale;
                        if (dist >= radius) {
                            ms2 = NULL;
                        }
                    }
                }

                hit = true;
                if (callback) {
                    if (!callback(pos, ms, user)) {
#ifdef _DEBUG
                        --intercount;
#endif

                        return hit;
                    }
                }
                else {
#ifdef _DEBUG
                    --intercount;
#endif

                    return true;
                }
            }
        }
    }

skip:;
    for (int od = 0; od < m_AD_Obj_cnt; ++od) {
        if (m_AD_Obj[od]->m_IntersectFlagFindObjects != m_IntersectFlagFindObjects) {
            m_AD_Obj[od]->m_IntersectFlagFindObjects = m_IntersectFlagFindObjects;

            CMatrixMapStatic *msa[2];
            int mscnt = 1;
            msa[0] = m_AD_Obj[od];

            for (;;) {
                if (mscnt == 0)
                    break;
                CMatrixMapStatic *ms = msa[0];
                if (mscnt == 2)
                    msa[0] = msa[1];
                mscnt--;

                if (ms->IsRobot()) {
                    if (((CMatrixRobot *)ms)->m_CurrState == ROBOT_DIP)
                        continue;
                    if ((mask & TRACE_ROBOT) == 0)
                        continue;
                }
                else if (ms->IsFlyer()) {
                    msa[mscnt] = ((CMatrixFlyer *)ms)->GetCarryingRobot();
                    if (msa[mscnt] != NULL) {
                        auto tmp = msa[mscnt]->GetGeoCenter() - pos;
                        float dist =
                                D3DXVec3Length(&tmp) - msa[mscnt]->GetRadius() * oscale;
                        if (dist < radius) {
                            mscnt++;
                        }
                    }
                    if ((mask & TRACE_FLYER) == 0)
                        continue;

                    auto tmp = ms->GetGeoCenter() - pos;
                    float dist = D3DXVec3Length(&tmp) - ms->GetRadius() * oscale;
                    if (dist >= radius) {
                        continue;
                    }
                }

                if (ms == skip)
                    continue;

                hit = true;
                if (callback) {
                    if (!callback(pos, ms, user)) {
#ifdef _DEBUG
                        --intercount;
#endif

                        return hit;
                    }
                }
                else {
#ifdef _DEBUG
                    --intercount;
#endif

                    return true;
                }
            }
        }
    }

#ifdef _DEBUG
    --intercount;
#endif
    return hit;
}

#ifdef _DEBUG
void CMatrixMap::SubEffect(const SDebugCallInfo &from, PCMatrixEffect e)
#else
void CMatrixMap::SubEffect(PCMatrixEffect e)
#endif
{
    DTRACE();
    if (e->IsDIP())
        return;  // already under destruction

#ifdef _DEBUG
    if (e->GetType() == EFFECT_LANDSCAPE_SPOT) {
        ((CMatrixEffectLandscapeSpot *)e)->BeforeRelease(from._file, from._line);
    }
#endif

    if (e->m_Next == NULL && e->m_Prev == NULL && m_EffectsFirst != e) {
        // effect not in list
        e->Release();
    }
    else {
        //        if (FLAG(m_Flags, MMFLAG_EFF_TAKT))
        //        {
        //            if (!(e->m_PrevDel==NULL && e->m_NextDel==NULL && e!=m_Effects2RemoveFirst)) return; // already
        //            dead
        //            // effect takt in progress. just put effect to dead list
        //            LIST_ADD(e, m_Effects2RemoveFirst, m_Effects2RemoveLast, m_PrevDel, m_NextDel);
        //            e->Unconnect();
        //
        ////#ifdef _DEBUG
        ////            DM("aaaaaaa","del: " + CStr(int(e->GetType())) + " from " + from._file + ":" + from._line);
        ////#endif
        //
        //        } else
        {
            if (m_EffectsNextTakt == e) {
                m_EffectsNextTakt = e->m_Next;
            }

            LIST_DEL(e, m_EffectsFirst, m_EffectsLast, m_Prev, m_Next);
            e->Release();
        }
        --m_EffectsCnt;
    }

#ifdef _DEBUG
    CDText::T("E", m_EffectsCnt);
#endif
}

// bool CMatrixMap::IsEffect(PCMatrixEffect e)
//{
//    DTRACE();
//    //for (int i=0; i<m_EffectsCnt; ++i)
//    //{
//    //    if (m_Effects[i] == e)
//    //    {
//    //        return true;
//    //    }
//    //}
//    //return false;
//    return m_Effects.Get(PTR2KEY(e),NULL);
//}

void CMatrixMap::RemoveEffectSpawnerByObject(CMatrixMapStatic *ms) {
    int i = 0;
    while (i < m_EffectSpawnersCnt) {
        if (m_EffectSpawners[i].GetUnder() == ms) {
            m_EffectSpawners[i].~CEffectSpawner();
            --m_EffectSpawnersCnt;
            if (i == m_EffectSpawnersCnt) {
                return;
            }
            m_EffectSpawners[i] = m_EffectSpawners[m_EffectSpawnersCnt];
            continue;
        }
        ++i;
    }
}

void CMatrixMap::RemoveEffectSpawnerByTime(void) {
    int i = 0;
    while (i < m_EffectSpawnersCnt) {
        if (m_EffectSpawners[i].OutOfTime())
            break;
        ++i;
    }
    if (i >= m_EffectSpawnersCnt)
        return;

    m_EffectSpawners[i].~CEffectSpawner();
    --m_EffectSpawnersCnt;
    if (i == m_EffectSpawnersCnt) {
        return;
    }
    m_EffectSpawners[i] = m_EffectSpawners[m_EffectSpawnersCnt];
}

void CMatrixMap::AddEffectSpawner(float x, float y, float z, int ttl, const std::wstring &in_par) {
    SpawnEffectSmoke smoke;
    SpawnEffectFire fire;
    SpawnEffectSound sound;
    SpawnEffectLightening lightening;

    SpawnEffectProps *props = &smoke;

    ParamParser par{in_par};
    int parcnt = par.GetCountPar(L",");
    if (parcnt < 3)
        return;
    parcnt -= 3;

    int minper = par.GetStrPar(1, L",").GetInt();
    int maxper = par.GetStrPar(2, L",").GetInt();

    int idx = 3;

    m_EffectSpawners = (CEffectSpawner *)HAllocEx(m_EffectSpawners, sizeof(CEffectSpawner) * (m_EffectSpawnersCnt + 1),
                                                  g_MatrixHeap);

    EEffectSpawnerType type = (EEffectSpawnerType)par.GetStrPar(0, L",").GetInt();

    bool light_need_action = false;

    switch (type) {
        case EST_SMOKE: {
            smoke.m_Type = EFFECT_SMOKE;

            smoke.m_ttl = (float)par.GetStrPar(idx++, L",").GetDouble();
            smoke.m_puffttl = (float)par.GetStrPar(idx++, L",").GetDouble();
            smoke.m_spawntime = (float)par.GetStrPar(idx++, L",").GetDouble();
            smoke.m_intense = par.GetStrPar(idx++, L",").GetBool();
            smoke.m_speed = (float)par.GetStrPar(idx++, L",").GetDouble();
            smoke.m_color = par.GetStrPar(idx++, L",").GetHexUnsigned();

            break;
        }
        case EST_FIRE: {
            props = &fire;

            fire.m_Type = EFFECT_FIRE;

            fire.m_ttl = (float)par.GetStrPar(idx++, L",").GetDouble();
            fire.m_puffttl = (float)par.GetStrPar(idx++, L",").GetDouble();
            fire.m_spawntime = (float)par.GetStrPar(idx++, L",").GetDouble();
            fire.m_intense = par.GetStrPar(idx++, L",").GetBool();
            fire.m_speed = (float)par.GetStrPar(idx++, L",").GetDouble();
            fire.m_dispfactor = (float)par.GetStrPar(idx++, L",").GetDouble();

            break;
        }
        case EST_SOUND: {
            props = &sound;

            sound.m_Type = EFFECT_UNDEFINED;

            sound.m_vol0 = (float)par.GetStrPar(idx++, L",").GetDouble();
            sound.m_vol1 = (float)par.GetStrPar(idx++, L",").GetDouble();
            sound.m_pan0 = (float)par.GetStrPar(idx++, L",").GetDouble();
            sound.m_pan1 = (float)par.GetStrPar(idx++, L",").GetDouble();
            sound.m_attn = (float)par.GetStrPar(idx++, L",").GetDouble();
            // sound.m_looped = par.GetStrPar(idx++,L",").GetBool();

            std::wstring nam(par.GetStrPar(idx++, L","));
            if (nam.length() > (sizeof(sound.m_name) / sizeof(sound.m_name[0]))) {
                ERROR_S(L"Effect spawner: sound name too long!");
            }

            memcpy(sound.m_name, nam.c_str(), (nam.length() + 1) * sizeof(wchar));

            break;
        }
        case EST_LIGHTENING: {
            props = &lightening;
            lightening.m_Type = EFFECT_LIGHTENING;

            lightening.m_Tag = HNew(g_MatrixHeap) std::wstring();

            *lightening.m_Tag = par.GetStrPar(idx++, L",");

            lightening.m_ttl = (float)par.GetStrPar(idx++, L",").GetDouble();
            lightening.m_Color = par.GetStrPar(idx++, L",").GetHexUnsigned();

            // set negative width. it means that m_Tag used instead of m_Pair;
            lightening.m_Width = -(float)par.GetStrPar(idx++, L",").GetDouble();
            lightening.m_Dispers = (float)par.GetStrPar(idx++, L",").GetDouble();

            // seek tag
            int bla = m_EffectSpawnersCnt - 1;
            while (bla >= 0) {
                if (m_EffectSpawners[bla].Props()->m_Type == EFFECT_LIGHTENING) {
                    SpawnEffectLightening *l = (SpawnEffectLightening *)m_EffectSpawners[bla].Props();
                    if (l->m_Width < 0) {
                        if (*l->m_Tag == *lightening.m_Tag) {
                            NEG_FLOAT(l->m_Width);
                            NEG_FLOAT(lightening.m_Width);

                            using std::wstring;
                            HDelete(wstring, l->m_Tag, g_MatrixHeap);
                            HDelete(wstring, lightening.m_Tag, g_MatrixHeap);
                            lightening.m_Pair = l;

                            lightening.m_Dispers = -1;  // me - non host

                            light_need_action = true;
                        }
                    }
                }
                --bla;
            }

            break;
        }
        default:
            ERROR_E;
    };

    props->m_Pos.x = x;
    props->m_Pos.y = y;
    props->m_Pos.z = z;

    new(&m_EffectSpawners[m_EffectSpawnersCnt]) CEffectSpawner(minper, maxper, ttl, props);

    if (light_need_action) {
        SpawnEffectLightening *l = (SpawnEffectLightening *)m_EffectSpawners[m_EffectSpawnersCnt].Props();
        l->m_Pair->m_Pair = l;
    }

    ++m_EffectSpawnersCnt;
}

void CMatrixMap::LeaveDialogMode(void) {
    if (m_DialogModeName == NULL)
        return;

    if (0 == wcscmp(m_DialogModeName, TEMPLATE_DIALOG_MENU)) {
        m_DialogModeHints[0]->SoundOut();
    }

    RESETFLAG(m_Flags, MMFLAG_DIALOG_MODE);
    Pause(false);

    for (auto item : m_DialogModeHints)
    {
        item->Release();
    }

    m_DialogModeHints.clear();
    g_IFaceList->HideHintButtons();
    m_DialogModeName = NULL;
}

static void OkHandler(void) {
    g_MatrixMap->LeaveDialogMode();
}
static void OkJustExitHandler(void) {
    g_MatrixMap->LeaveDialogMode();
    SETFLAG(g_Flags, GFLAG_EXITLOOP);
}
static void OkExitWinHandler(void) {
    g_MatrixMap->LeaveDialogMode();
    g_ExitState = 3;

    SETFLAG(g_MatrixMap->m_Flags, MMFLAG_STAT_DIALOG);
    // SETFLAG(g_Flags, GFLAG_EXITLOOP);
}
static void OkExitLooseHandler(void) {
    g_MatrixMap->LeaveDialogMode();
    g_ExitState = 2;
    SETFLAG(g_MatrixMap->m_Flags, MMFLAG_STAT_DIALOG);
    // SETFLAG(g_Flags, GFLAG_EXITLOOP);
}
static void OkExitHandler(void) {
    g_MatrixMap->LeaveDialogMode();
    g_ExitState = 1;
    SETFLAG(g_MatrixMap->m_Flags, MMFLAG_STAT_DIALOG);
    // SETFLAG(g_Flags, GFLAG_EXITLOOP);
}
static void OkSurrenderHandler(void) {
    g_MatrixMap->LeaveDialogMode();
    g_ExitState = 4;
    SETFLAG(g_MatrixMap->m_Flags, MMFLAG_STAT_DIALOG);

    for (int i = 0; i < g_MatrixMap->m_SideCnt; ++i) {
        CMatrixSideUnit *su = g_MatrixMap->m_Side + i;
        if (su->GetStatus() == SS_ACTIVE && su != g_MatrixMap->GetPlayerSide()) {
            su->SetStatValue(STAT_TIME, -su->GetStatValue(STAT_TIME));
        }
    }
    // SETFLAG(g_Flags, GFLAG_EXITLOOP);
}
static void OkResetHandler(void) {
    g_MatrixMap->LeaveDialogMode();
    g_MatrixMap->Restart();
    // g_ExitState = 4;
    // SETFLAG(g_Flags, GFLAG_EXITLOOP);
}
void ConfirmCancelHandler(void) {
    CMatrixHint *h = (CMatrixHint *)g_MatrixMap->m_DialogModeHints[1];
    g_MatrixMap->m_DialogModeHints.erase(g_MatrixMap->m_DialogModeHints.end() - 1);
    h->Release();

    g_IFaceList->EnableMainMenuButton(HINT_CANCEL_MENU);
    g_IFaceList->EnableMainMenuButton(HINT_CONTINUE);
    g_IFaceList->EnableMainMenuButton(HINT_SURRENDER);
    g_IFaceList->EnableMainMenuButton(HINT_RESET);
    g_IFaceList->EnableMainMenuButton(HINT_EXIT);

    g_IFaceList->HideHintButton(HINT_OK);
    g_IFaceList->HideHintButton(HINT_CANCEL);
}

static void CreateConfirmation(const wchar *hint, DialogButtonHandler handler) {
    g_IFaceList->DisableMainMenuButton(HINT_CANCEL_MENU);
    g_IFaceList->DisableMainMenuButton(HINT_CONTINUE);
    g_IFaceList->DisableMainMenuButton(HINT_SURRENDER);
    g_IFaceList->DisableMainMenuButton(HINT_RESET);
    g_IFaceList->DisableMainMenuButton(HINT_EXIT);

    CMatrixHint *h = CMatrixHint::Build(std::wstring(hint), hint);
    int ww = (g_ScreenX - h->m_Width) / 2;
    int hh = (g_ScreenY - h->m_Height) / 2 - Float2Int(float(g_ScreenY) * 0.09f);
    h->Show(ww, hh);

    if (h->GetCopyPosCnt() > 0) {
        int x = h->m_PosX + h->GetCopyPos(0).x;
        int y = h->m_PosY + h->GetCopyPos(0).y;
        g_IFaceList->CreateHintButton(x, y, HINT_OK, handler);
    }
    if (h->GetCopyPosCnt() > 1) {
        int x = h->m_PosX + h->GetCopyPos(1).x;
        int y = h->m_PosY + h->GetCopyPos(1).y;
        g_IFaceList->CreateHintButton(x, y, HINT_CANCEL, ConfirmCancelHandler);
    }

    g_MatrixMap->m_DialogModeHints.push_back(h);
}

void ExitRequestHandler(void) {
    CreateConfirmation(TEMPLATE_DIALOG_EXIT, OkExitHandler);
}

void ResetRequestHandler(void) {
    CreateConfirmation(TEMPLATE_DIALOG_RESET, OkResetHandler);
}

void HelpRequestHandler(void) {
    g_MatrixMap->LeaveDialogMode();
    std::wstring fn;
    if (CFile::FileExist(fn, L"Manual.exe")) {
        PROCESS_INFORMATION pi0;
        STARTUPINFOA si0;

        ZeroMemory(&si0, sizeof(si0));
        si0.cb = sizeof(si0);
        ZeroMemory(&pi0, sizeof(pi0));

        CreateProcess("Manual.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si0, &pi0);

        ShowWindow(g_Wnd, SW_MINIMIZE);

        CloseHandle(pi0.hProcess);
        CloseHandle(pi0.hThread);
    }
}

void SurrenderRequestHandler(void) {
    CreateConfirmation(TEMPLATE_DIALOG_SURRENDER, OkSurrenderHandler);
}

void CMatrixMap::EnterDialogMode(const wchar *hint_i) {
    if (m_PauseHint) {
        m_PauseHint->Release();
        m_PauseHint = NULL;
    }
    LeaveDialogMode();
    Pause(true);
    SETFLAG(m_Flags, MMFLAG_DIALOG_MODE);
    m_DialogModeName = hint_i;

    if (0 != wcscmp(hint_i, TEMPLATE_DIALOG_BEGIN)) {
        g_MatrixMap->GetPlayerSide()->PLDropAllActions();
    }

    CBlockPar *bp = g_MatrixData->BlockGet(PAR_TEMPLATES);

    int cnt = bp->ParCount();

    int ww = 20;
    int hh = 62;

    const wchar *hint = hint_i;
    if (0 == wcscmp(hint_i, TEMPLATE_DIALOG_STATISTICS_D)) {
        hint = TEMPLATE_DIALOG_STATISTICS;
    }

    for (int i = 0; i < cnt; ++i) {
        if (bp->ParGetName(i) == hint) {
            std::wstring templ(bp->ParGet(i));
            std::wstring templ2;
            if (templ[0] == '|')
                continue;

            int ii = i + 1;
            for (; ii < cnt; ++ii) {
                if (bp->ParGetName(ii) == hint) {
                    templ2 = bp->ParGet(ii);
                    if (templ2[0] == '|')
                        templ += templ2;
                }
            }

            CMatrixHint *h = CMatrixHint::Build(templ, g_MatrixData->BlockGet(PAR_REPLACE), hint);

            if (0 == wcscmp(hint, TEMPLATE_DIALOG_MENU)) {
                h->m_PosX = (g_ScreenX - h->m_Width) / 2;
                h->m_PosY = (g_ScreenY - h->m_Height) / 2 - Float2Int(float(g_ScreenY) * 0.09f);
                h->SetVisible(false);
                h->SoundIn();
            }
            else if (0 == wcscmp(hint, TEMPLATE_DIALOG_STATISTICS)) {
                h->m_PosX = (g_ScreenX - h->m_Width) / 2;
                h->m_PosY = (g_ScreenY - h->m_Height) / 2 - Float2Int(float(g_ScreenY) * 0.09f);
                h->SetVisible(true);
            }
            else {
                h->Show(ww, hh);
            }

            if (h->GetCopyPosCnt() > 0) {
                int x = h->m_PosX + h->GetCopyPos(0).x;
                int y = h->m_PosY + h->GetCopyPos(0).y;

                if (0 == wcscmp(hint, TEMPLATE_DIALOG_MENU)) {
                    g_IFaceList->CreateHintButton(x, y, HINT_CONTINUE, OkHandler);
                }
                else if (0 == wcscmp(hint, TEMPLATE_DIALOG_WIN)) {
                    g_IFaceList->CreateHintButton(x, y, HINT_OK, OkExitWinHandler);
                }
                else if (0 == wcscmp(hint, TEMPLATE_DIALOG_LOOSE)) {
                    g_IFaceList->CreateHintButton(x, y, HINT_OK, OkExitLooseHandler);
                }
                else if (0 == wcscmp(hint, TEMPLATE_DIALOG_STATISTICS)) {
                    if (0 == wcscmp(hint_i, TEMPLATE_DIALOG_STATISTICS_D)) {
                        g_IFaceList->CreateHintButton(x, y, HINT_OK, OkHandler);
                    }
                    else {
                        g_IFaceList->CreateHintButton(x, y, HINT_OK, OkJustExitHandler);
                    }
                }
                else {
                    g_IFaceList->CreateHintButton(x, y, HINT_OK, OkHandler);
                }
            }
            if (h->GetCopyPosCnt() > 1) {
                int x = h->m_PosX + h->GetCopyPos(1).x;
                int y = h->m_PosY + h->GetCopyPos(1).y;
                g_IFaceList->CreateHintButton(x, y, HINT_RESET, ResetRequestHandler);
            }
            if (h->GetCopyPosCnt() > 2) {
                int x = h->m_PosX + h->GetCopyPos(2).x;
                int y = h->m_PosY + h->GetCopyPos(2).y;
                g_IFaceList->CreateHintButton(x, y, HINT_HELP, HelpRequestHandler);
            }
            if (h->GetCopyPosCnt() > 3) {
                int x = h->m_PosX + h->GetCopyPos(3).x;
                int y = h->m_PosY + h->GetCopyPos(3).y;
                g_IFaceList->CreateHintButton(x, y, HINT_SURRENDER, SurrenderRequestHandler);
            }
            if (h->GetCopyPosCnt() > 4) {
                int x = h->m_PosX + h->GetCopyPos(4).x;
                int y = h->m_PosY + h->GetCopyPos(4).y;
                g_IFaceList->CreateHintButton(x, y, HINT_EXIT, ExitRequestHandler);
            }
            if (h->GetCopyPosCnt() > 5) {
                int x = h->m_PosX + h->GetCopyPos(5).x;
                int y = h->m_PosY + h->GetCopyPos(5).y;
                g_IFaceList->CreateHintButton(x, y, HINT_CANCEL_MENU, OkHandler);
            }

            ww += h->m_Width + 20;
            m_DialogModeHints.push_back(h);
        }
    }
}

void CMatrixMap::RestoreMusicVolume(void) {
    if (!g_RangersInterface)
        return;
    SETFLAG(m_Flags, MMFLAG_MUSIC_VOL_CHANGING);
    m_TargetMusicVolume = m_StoreCurrentMusicVolume;
}

void CMatrixMap::SetMusicVolume(float vol) {
    if (!g_RangersInterface)
        return;

    if (FLAG(m_Flags, MMFLAG_MUSIC_VOL_CHANGING)) {
        m_StoreCurrentMusicVolume = m_TargetMusicVolume;
    }
    else {
        SETFLAG(m_Flags, MMFLAG_MUSIC_VOL_CHANGING);
        m_StoreCurrentMusicVolume = g_RangersInterface->m_MusicVolumeGet();
    }
    m_TargetMusicVolume = vol;
}

void CMatrixMap::ShowPortrets(void) {
    int n = 0;
    SETFLAG(g_MatrixMap->m_Flags, MMFLAG_SHOWPORTRETS);

    for (auto item : m_AllObjects)
    {
        if (item->GetObjectType() == OBJECT_TYPE_MAPOBJECT && ((CMatrixMapObject*)item)->m_BehFlag == BEHF_PORTRET)
        {
            ((CMatrixMapObject*)item)->m_PrevStateRobotsInRadius = ++n;
            ((CMatrixMapObject*)item)->RChange(MR_Graph);
        }
    }
}

// void    CMatrixMap::BeginDieSequence(void)
//{
//    m_ShadeOn = true;
//    m_ShadeTime = SHADER_TIME;
//    m_ShadeTimeCurrent = 0;
//
//    m_ShadePosFrom = m_Camera.GetTarget();
//
//    m_ShadePosTo  = m_ShadePosFrom ; //GetPlayerSide()->GetFlyer()->GetPos();
//    m_ShadeInterfaceColor = m_Minimap.GetColor();
//}

bool CMatrixMap::IsTraceNonPlayerObj() {
    if (IS_TRACE_STOP_OBJECT(g_MatrixMap->m_TraceStopObj) &&
        (g_MatrixMap->m_TraceStopObj->IsRobot() || g_MatrixMap->m_TraceStopObj->IsBuilding() ||
         g_MatrixMap->m_TraceStopObj->IsCannon() || g_MatrixMap->m_TraceStopObj->GetObjectType() == OBJECT_TYPE_FLYER ||
         g_MatrixMap->m_TraceStopObj->IsSpecial()) &&
        (g_MatrixMap->m_TraceStopObj->GetSide() != PLAYER_SIDE))
        return true;

    return false;
}
