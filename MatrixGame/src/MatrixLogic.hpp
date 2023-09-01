// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixMap.hpp"

extern CMatrixRobotAI *g_TestRobot;
extern bool g_TestLocal;

#define LOGIC_TAKT_PERIOD 10

#define MAX_UNIT_ON_PATH 64

struct SMatrixPathUnit {
    float sx, sy;  // Начало (мировые координаты но кратные GLOBAL_SCALE_MOVE)
    float ex, ey;  // Конец (мировые координаты но кратные GLOBAL_SCALE_MOVE)
    float vx, vy;  // Направление (нормализированно)
    float length;  // Длина
};

struct SMatrixPath;

struct SMatrixPathObj {
    SMatrixPathObj *m_Prev;
    SMatrixPathObj *m_Next;

    SMatrixPath *m_Path;

    float cx, cy;  // Где объект сейчас находится
    float tx, ty;  // Где объект будит находится
    float radius;  // Радиус объекта идущего по пути
    int nsh;
    int size;

    bool calc_pos;
};

struct SMatrixPath {
    SMatrixPath *m_Prev;
    SMatrixPath *m_Next;
    int m_Cnt;
    SMatrixPathUnit m_Unit[MAX_UNIT_ON_PATH];

    SMatrixPathObj *m_Owner;

    float m_StartX, m_StartY;  // Boundbox для пути
    float m_EndX, m_EndY;
};

class CMatrixMapLogic : public CMatrixMap {
public:
    //		SMatrixMapZone * m_Zone[5];
    //		int m_ZoneCnt[5];

    SMatrixPath *m_PathFirst;
    SMatrixPath *m_PathLast;
    SMatrixPathObj *m_ObjFirst;
    SMatrixPathObj *m_ObjLast;

    int m_MPFCnt, m_MPF2Cnt;
    CPoint *m_MPF, *m_MPF2;

    int *m_ZoneIndex;
    int *m_ZoneIndexAccess;
    dword *m_ZoneDataZero;

    CPoint *m_MapPoint;

    // int m_Takt;				// Game takt
    int m_TaktNext;

    int m_GatherInfoLast;

public:
    CMatrixMapLogic(void);
    ~CMatrixMapLogic();

    void Clear(void);

    int Rnd(void);
    double RndFloat(void);  // 0-1
    int Rnd(int zmin, int zmax);
    double RndFloat(double zmin, double zmax);

    void GatherInfo(int type);

    void PrepareBuf(void);

    //		void ZoneClear(void);
    //		void ZoneCalc(int nsh,byte mm);
    //		void ZoneCalc(void);
    int ZoneFindNear(int nsh, int mx, int my);
    void PlaceGet(int nsh, float wx, float wy, int *mx, int *my);
    bool IsAbsenceWall(int nsh, int size, int mx, int my);
    BYTE GetCellMoveType(int nsh, int mx, int my)  // ff-free 0-box 1-sphere
    {
        SMatrixMapMove *smm = MoveGetTest(mx, my);
        if (!smm)
            return 0xff;
        return smm->GetType(nsh);
    }

    //        bool PlaceFindNear(int nsh,int size,int & mx,int & my);
    bool PlaceFindNear(int nsh, int size, int &mx, int &my, int other_cnt, int *other_size, CPoint *other_des);
    //        bool PlaceFindNear(int nsh,int size,int & mx,int & my,const D3DXVECTOR2 & vdir,int other_cnt,int *
    //        other_size,CPoint * other_des);
    bool PlaceFindNear(int nsh, int size, int &mx, int &my, CMatrixMapStatic *skip);
    bool PlaceFindNearReturn(int nsh, int size, int &mx, int &my, CMatrixRobotAI *robot);
    //        bool PlaceFindNear(int nsh,int size,int & mx,int & my,const D3DXVECTOR2 & vdir,CMatrixMapStatic * skip);
    bool PlaceIsEmpty(int nsh, int size, int mx, int my, CMatrixMapStatic *skip);

    //		int ZoneMoveFindNear(int nsh,int mx,int my,CPoint * path);
    //		int ZoneMoveFind(int nsh,int size,int mx,int my,int zonesou1,int zonesou2,int zonesou3,int deszone,int dx,int
    //dy,CPoint * path); 		int ZoneMoveIn(int nsh,int size,int sx,int sy,int dx,int dy,CPoint * path); 		int
    //ZoneFindPath(int nsh,int zstart,int zend,int * path);

    void SetWeightFromTo(int size, int x1, int y1, int x2, int y2);
    int FindLocalPath(int nsh, int size, int mx, int my,  // Начальная точка
                      int *zonepath, int zonepathcnt,  // Список зон через которые нужной найти путь
                      int dx, int dy,                  // Точка назначения
                      CPoint *path,                    // Рассчитанный путь
                      int other_cnt,                   // Кол-во путей от других роботов
                      int *other_size,           // Список размеров в других путях
                      CPoint **other_path_list,  // Список указателей на другие пути
                      int *other_path_cnt,  // Список кол-во элементов в других путях
                      CPoint *other_des,    // Список конечных точек в других путях
                      bool test);

    void SetZoneAccess(int *list, int cnt, bool value);
    int FindPathInZone(int nsh, int zstart, int zend, const CMatrixRoadRoute *route, int routeno, int *path, bool test);
    bool CanMoveFromTo(int nsh, int size, int x1, int y1, int x2, int y2, CPoint *path);
    bool CanOptimize(int nsh, int size, int x1, int y1, int x2, int y2);
    int OptimizeMovePath(int nsh, int size, int cnt, CPoint *path);
    int OptimizeMovePathSimple(int nsh, int size, int cnt, CPoint *path);
    int RandomizeMovePath(int nsh, int size, int cnt, CPoint *path);
    int FindNearPlace(byte mm, const CPoint &mappos);
    int FindPlace(const CPoint &mappos);

    int PlaceList(byte mm, const CPoint &from, const CPoint &to, int radius, bool farpath, int *list, int *listcnt,
                  int *outdist = NULL);  // Return 0-not found 1-can move to 2-barrier
    int PlaceListGrow(byte mm, int *list, int *listcnt, int growcnt);

    SMatrixPathObj *ObjAdd(void);
    void ObjDelete(SMatrixPathObj *obj);
    void ObjClear(void);

    SMatrixPath *PathAlloc(void);
    void PathFree(SMatrixPath *path);
    void PathClear(void);
    void PathAdd(SMatrixPath *path) { LIST_ADD(path, m_PathFirst, m_PathLast, m_Prev, m_Next); }
    void PathInsert(SMatrixPath *ito, SMatrixPath *path) {
        LIST_INSERT(ito, path, m_PathFirst, m_PathLast, m_Prev, m_Next);
    }
    void PathDelete(SMatrixPath *path, bool free = true) {
        LIST_DEL(path, m_PathFirst, m_PathLast, m_Prev, m_Next);
        if (free)
            PathFree(path);
    }
    void PathCalcPosAfter(SMatrixPath *path = NULL);
    bool PathCheckInFindInsert(SMatrixPath *path);
    void PathCheckIn(SMatrixPath *path);
    bool PathCheckInFindPos(SMatrixPath *path, SMatrixPathObj *obj);
    bool PathIntersectAfter(SMatrixPath *path);
    bool PathIntersect(SMatrixPath *path, float cx, float cy, float radius);
    //        bool PathIntersect(SMatrixPath * path1,SMatrixPath * path2);

    inline int GetRegion(const CPoint &tp);
    inline int GetRegion(int x, int y);

    void CalcCannonPlace(void);

    void Takt(int step);

    bool IsLogicVisible(CMatrixMapStatic *ofrom, CMatrixMapStatic *oto, float second_z = 0.0f);

    void DumpLogic(void);
};

inline int CMatrixMapGroup::ObjectsCnt(void) const {
    return m_ObjectsContained;
}
inline CMatrixMapStatic *CMatrixMapGroup::GetObject(int i) const {
    return m_Objects[i];
}

inline int CMatrixMapLogic::GetRegion(const CPoint &tp) {
    while (true) {
        SMatrixMapMove *mm = g_MatrixMap->MoveGetTest(tp.x, tp.y);
        if (mm == NULL)
            break;
        if (mm->m_Zone < 0)
            break;
        if (g_MatrixMap->m_RN.m_Zone[mm->m_Zone].m_Region < 0)
            break;
        return g_MatrixMap->m_RN.m_Zone[mm->m_Zone].m_Region;
        break;
    }
    return g_MatrixMap->m_RN.FindNerestRegion(tp);
}

inline int CMatrixMapLogic::GetRegion(int x, int y) {
    while (true) {
        SMatrixMapMove *mm = g_MatrixMap->MoveGetTest(x, y);
        if (mm == NULL)
            break;
        if (mm->m_Zone < 0)
            break;
        if (g_MatrixMap->m_RN.m_Zone[mm->m_Zone].m_Region < 0)
            break;
        return g_MatrixMap->m_RN.m_Zone[mm->m_Zone].m_Region;
        break;
    }
    return g_MatrixMap->m_RN.FindNerestRegion(CPoint(x, y));
}
