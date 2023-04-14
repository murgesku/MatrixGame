// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "CMain.hpp"
#include "CHeap.hpp"
#include "CBuf.hpp"
#include "Tracer.hpp"
#include "BaseDef.hpp"

class CMatrixCrotch;
class CMatrixRoadNetwork;

class CMatrixRoad : public Base::CMain {
public:
    CMatrixRoadNetwork *m_Parent;
    CMatrixRoad *m_Prev;
    CMatrixRoad *m_Next;

    CMatrixCrotch *m_Start;
    CMatrixCrotch *m_End;

    int m_PathCnt;
    Base::CPoint *m_Path;

    int m_ZoneCnt;
    int m_ZoneCntMax;
    int *m_Zone;

    int m_Dist;

    byte m_Move;

    int m_Data;  // Данные для расчета
public:
    CMatrixRoad(void);
    ~CMatrixRoad();

    void DeleteCrotch(const CMatrixCrotch *crotch);

    void DeletePath(void);
    void LoadPath(const int cnt, const Base::CPoint *path);
    void AddPath(int x, int y);
    void AddPathFromRoad(CMatrixRoad *road);
    void CorrectStartEndByPath(void);

    void DeleteZone(void);
    void LoadZone(const int cnt, const int *path);
    void AddZone(int zone);
    int GetIndexZone(int zone);
    void CorrectStartEndByZone(void);
    void SplitZone(int index, CMatrixRoad *road);
    void AddZoneFromRoad(CMatrixRoad *road);
    void DeleteZoneByIndex(int index);

    void DeleteCircleZone(void);
    bool CompareZone(CMatrixRoad *road);

    void MarkZoneRoad(void);

    void CalcDist(void);

    CMatrixCrotch *GetOtherCrotch(const CMatrixCrotch *crotch) {
        if (m_Start != crotch)
            return m_Start;
        else
            return m_End;
    }
};

class CMatrixCrotch : public Base::CMain {
public:
    CMatrixRoadNetwork *m_Parent;
    CMatrixCrotch *m_Prev;
    CMatrixCrotch *m_Next;

    Base::CPoint m_Center;
    int m_Zone;

    int m_RoadCnt;
    CMatrixRoad *m_Road[16];

    int m_Island;  // Номер острова
    int m_Region;  // Номер региона. -1 - не в регионе
    int m_Data;    // Данные для расчета

    bool m_Select;
    bool m_Critical;  // Важная зона (рядом база, завод)

public:
    CMatrixCrotch(void);
    ~CMatrixCrotch();

    void DeleteRoad(const CMatrixRoad *road);
    void AddRoad(CMatrixRoad *road);

    int CalcCntRoadEqualMoveTypeAndNoDeadlock(void);
};

struct SMatrixMapZone {
    Base::CPoint m_Center;  // Центр
    //	bool m_Critical;                // Важная зона (рядом база, завод)
    //    int m_Group;                  // Сгрупперованные зоны между узкими проходами
    //    int m_Island;                 // Остров
    int m_Size;          // Cnt in unit
    int m_Perim;         // Периметр
    Base::CRect m_Rect;  // Bound zone
    byte m_Move;  // (1-нельзя пройти) 1-Shasi1(Пневматика) 2-Shasi2(Колеса) 4-Shasi3(Гусеницы) 8-Shasi4(Подушка)
                  // 16-Shasi5(Крылья)
    bool m_Road;  // По этой зоне проходит дорога
    bool m_Bottleneck;  // Узкое место для всех видов шаси
    bool m_Access;
    // byte dummy; // only for align

    int m_NearZoneCnt;              // Кол-во ближних зон
    int m_NearZone[64];             // Список ближних зон
    int m_NearZoneConnectSize[64];  // Длина связи межу ближними зонами
    byte m_NearZoneMove[64];

    byte m_PlaceCnt;
    int m_Place[4];

    int m_Region;

    DWORD m_Color;
    DWORD m_ColorGroup;

    //    DWORD m_CriticalCnt;

    int m_FPLevel;
    int m_FPWt;
    int m_FPWtp;
};

struct SMatrixRoadRouteUnit {
    CMatrixRoad *m_Road;
    CMatrixCrotch *m_Crotch;
};

struct SMatrixRoadRouteHeader {
    int m_Cnt;
    int m_Dist;
};

class CMatrixRoadRoute : public Base::CMain {
public:
    CMatrixRoadNetwork *m_Parent;
    CMatrixRoadRoute *m_Prev;
    CMatrixRoadRoute *m_Next;

    CMatrixCrotch *m_Start;
    CMatrixCrotch *m_End;

    int m_ListCnt, m_ListCntMax;
    SMatrixRoadRouteHeader *m_Header;
    SMatrixRoadRouteUnit *m_Units;

public:
    CMatrixRoadRoute(CMatrixRoadNetwork *parent);
    ~CMatrixRoadRoute();

    void Clear(void);
    void ClearFast(void);

    void NeedEmpty(int cnt = 1, bool strong = false);

    int AddList(void);
    void CopyUnit(int listfrom, int listto);
    int AddUnit(int listno, CMatrixRoad *road, CMatrixCrotch *crotch);
};

struct SMatrixPlace {
    Base::CPoint m_Pos;
    byte m_Move;
    byte m_BorderLeft, m_BorderTop, m_BorderRight, m_BorderBottom, m_BorderCnt;
    byte m_EdgeLeft, m_EdgeTop, m_EdgeRight, m_EdgeBottom, m_EdgeCnt;
    bool m_Blockade[3 * 3];  // Непроходимая стена относительно места

    byte m_Cannon;

    int m_Region;

    byte m_NearCnt;
    int m_Near[16];
    byte m_NearMove[16];

    // Нужны только в редакторе карт
    bool m_Empty;
    int m_EmptyNext;
    int m_Rank;

    // Нужны только в игре
    int m_Data;
    byte m_Underfire;
};

struct SMatrixPlaceList {
    int m_Sme;
    int m_Cnt;
};

struct SMatrixRegion {
    CPoint m_Center;
    int m_RadiusPlace;

    int m_CrotchCnt;
    CMatrixCrotch *m_Crotch[16];
    int m_PlaceCnt;
    int m_Place[16];

    int m_PlaceAllCnt;
    int m_PlaceAll[64];

    int m_NearCnt;
    int m_Near[16];
    byte m_NearMove[16];

    int m_FPLevel;
    int m_FPWt;
    int m_FPWtp;

    DWORD m_Color;
};

class CMatrixRoadNetwork : public Base::CMain {
public:
    Base::CHeap *m_Heap;

    int m_RoadCnt;
    CMatrixRoad *m_RoadFirst;
    CMatrixRoad *m_RoadLast;
    CMatrixRoad **m_RoadFindIndex;

    int m_CrotchCnt;
    CMatrixCrotch *m_CrotchFirst;
    CMatrixCrotch *m_CrotchLast;
    CMatrixCrotch **m_CrotchFindIndex;

    SMatrixMapZone *m_Zone;
    int m_ZoneCnt;
    int m_ZoneCntMax;

    CMatrixRoadRoute *m_RouteFirst;
    CMatrixRoadRoute *m_RouteLast;

    int m_PlaceCnt;
    int m_PlaceEmpty;
    SMatrixPlace *m_Place;

    int m_RegionCnt;
    SMatrixRegion *m_Region;
    int *m_RegionFindIndex;

    SMatrixPlaceList *m_PLList;
    int m_PLSizeX, m_PLSizeY;
    int m_PLShift;
    int m_PLMask;

public:
    CMatrixRoadNetwork(Base::CHeap *he);
    ~CMatrixRoadNetwork();

    void Clear(void);

    CMatrixRoad *AddRoad(void);
    void DeleteRoad(CMatrixRoad *un);
    bool IsRoadExist(CMatrixRoad *un);
    CMatrixRoad *FindRoadByZone(int zone);

    CMatrixCrotch *AddCrotch(void);
    void DeleteCrotch(CMatrixCrotch *un);
    bool IsCrotchExist(CMatrixCrotch *un);
    CMatrixCrotch *FindCrotchByZone(int zone);

    void UnionRoad(DWORD flags = 1);
    bool UnionRoadStep(DWORD flags = 1);  // 1-merge path 2-merge zone

    int SelectCrotchByRoadCnt(int amin, bool skipcritical = false);  // Select crotch.m_PathCnt from 0 to amin
    void SelectCrotchSingleAndRadius(int radius);
    void DeleteSelectedCrotchAndRoad(void);

    void SplitRoad(void);

    void ClearZone(void);
    void SetZoneCntMax(int cnt);
    int FindNearZoneByCenter(Base::CPoint &p, int *dist2 = NULL);
    void CalcZoneColor(void);

    void MarkZoneRoad(void);
    CMatrixCrotch *InsertCrotch(int zone);
    bool CreateCrotchAndRoadFromZone(int zone);
    bool CreateCrotchAndRoadByPath(int *path, int cnt);
    void MergeEqualCrotch(void);

    void CalcIsland(void);
    void SelectUnneededIsland(void);
    void Unselect(void);

    int CalcPathZoneByRoad(int zstart, int zend, int *path);
    int CalcDistByRoad(int zstart, int zend, int *path = NULL);
    void CalcDistRoad(void);

    void DeleteCircleZone(void);
    void DeleteEqualRoad(void);

    void CreateAdditionalRoad(void);

    void FinalizeBuild(void);
    void FinalizePlace(void);

    bool IsValidate(bool exception = true);
    void Test(void);

    void ClearRoute(void);
    void DeleteRoute(CMatrixRoadRoute *route);
    CMatrixRoadRoute *AddRoute(void);
    CMatrixRoadRoute *CalcRoute(CMatrixCrotch *cstart, CMatrixCrotch *cend);
    CMatrixRoadRoute *FindRoute(CMatrixCrotch *cstart, CMatrixCrotch *cend);
    CMatrixRoadRoute *GetRoute(CMatrixCrotch *cstart, CMatrixCrotch *cend);

    void FindPathFromCrotchToRegion(byte mm, CMatrixCrotch *cstart, int region, CMatrixRoadRoute *rr, bool test);
    void FindPathFromRegionPath(byte mm, int rcnt, int *rlist, CMatrixRoadRoute *rr, bool test = false);

    void ClearPlace(void);
    SMatrixPlace *GetPlace(int no) { return m_Place + no; }
    void SetPlaceCnt(int cnt);
    int AllocPlace(void);
    void FreePlace(int no);

    void ClearRegion(void);
    int AddRegion(void);
    void DeleteRegion(int no);
    SMatrixRegion *GetRegion(int no) { return m_Region + no; }
    void CalcNearRegion(int no);
    void CalcCenterRegion(int no);
    int CalcRadiusRegion(int no);
    void CalcRadiusPlaceRegion(int no);
    bool IsNerestRegion(int r1, int r2);
    int FindNerestRegion(const CPoint &tp);
    int FindNerestRegionByRadius(const CPoint &tp, int curregion = -1);

    void FindPathInRegionInit(void);
    int FindPathInRegionRun(byte mm, int rstart, int rend, int *path, int maxpath, bool err = true);

    void ClearPL(void);
    void InitPL(int mapsizex, int mapsizey);
    CRect CorrectRectPL(const CRect &mapcoords);
    void ActionDataPL(const CRect &mapcoords, dword mask_and, dword mask_or = 0);
    int FindInPL(const CPoint &mappos);

    void Save(CBuf &b);
    void Load(CBuf &b, int ver);
};
