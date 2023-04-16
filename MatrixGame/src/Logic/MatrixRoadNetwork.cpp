// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixRoadNetwork.hpp"
#include "CException.hpp"

#include <math.h>
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
CMatrixRoad::CMatrixRoad() : CMain() {
    DTRACE();

    m_Parent = NULL;
    m_Prev = NULL;
    m_Next = NULL;

    m_Start = NULL;
    m_End = NULL;

    m_PathCnt = 0;
    m_Path = NULL;

    m_ZoneCnt = 0;
    m_ZoneCntMax = 0;
    m_Zone = NULL;

    m_Dist = 0;

    m_Move = 0;
}

CMatrixRoad::~CMatrixRoad() {
    DTRACE();

    DeleteZone();
    DeletePath();
    if (m_Start != NULL)
        m_Start->DeleteRoad(this);
    if (m_End != NULL)
        m_End->DeleteRoad(this);
    m_Start = NULL;
    m_End = NULL;
    m_Parent = NULL;
}

void CMatrixRoad::DeleteCrotch(const CMatrixCrotch *crotch) {
    DTRACE();

    if (m_Start == crotch)
        m_Start = NULL;
    if (m_End == crotch)
        m_End = NULL;
}

void CMatrixRoad::DeletePath() {
    DTRACE();

    ASSERT(m_Parent);

    if (m_Path != NULL) {
        HFree(m_Path, m_Parent->m_Heap);
        m_Path = NULL;
    }
    m_PathCnt = 0;
}

void CMatrixRoad::LoadPath(const int cnt, const CPoint *path) {
    DTRACE();

    DeletePath();
    if (cnt <= 0)
        return;

    ASSERT(m_Parent);

    m_Path = (CPoint *)HAlloc(cnt * sizeof(CPoint), m_Parent->m_Heap);
    m_PathCnt = cnt;
    CopyMemory(m_Path, path, cnt * sizeof(CPoint));
}

void CMatrixRoad::AddPath(int x, int y) {
    DTRACE();

    m_PathCnt++;
    m_Path = (CPoint *)HAllocEx(m_Path, m_PathCnt * sizeof(CPoint), m_Parent->m_Heap);
    m_Path[m_PathCnt - 1].x = x;
    m_Path[m_PathCnt - 1].y = y;
}

void CMatrixRoad::AddPathFromRoad(CMatrixRoad *road) {
    DTRACE();

    if (road->m_PathCnt <= 0)
        return;

    if (m_PathCnt <= 0) {
        LoadPath(road->m_PathCnt, road->m_Path);
    }
    else {
        m_Path = (CPoint *)HAllocEx(m_Path, (m_PathCnt + road->m_PathCnt - 1) * sizeof(CPoint), m_Parent->m_Heap);

        //        if(m_Path[m_PathCnt-1].Dist2(road->m_Path[0])<=m_Path[0].Dist2(road->m_Path[road->m_PathCnt-1])) { //
        //        After

        if ((m_Path[0] == road->m_Path[0]) || (m_Path[m_PathCnt - 1] == road->m_Path[road->m_PathCnt - 1])) {
            // Invert path
            for (int i = 0; i < (m_PathCnt >> 1); i++) {
                CPoint tp = m_Path[i];
                m_Path[i] = m_Path[m_PathCnt - 1 - i];
                m_Path[m_PathCnt - 1 - i] = tp;
            }
        }

        if (m_Path[m_PathCnt - 1] == road->m_Path[0]) {
            CopyMemory(m_Path + m_PathCnt, road->m_Path + 1, (road->m_PathCnt - 1) * sizeof(CPoint));
        }
        else if (m_Path[0] == road->m_Path[road->m_PathCnt - 1]) {  // Before
            MoveMemory(m_Path + road->m_PathCnt, m_Path + 1, (m_PathCnt - 1) * sizeof(CPoint));
            CopyMemory(m_Path, road->m_Path, road->m_PathCnt * sizeof(CPoint));
        }
        else {
            ERROR_E;
        }
        m_PathCnt += road->m_PathCnt - 1;
    }
}

void CMatrixRoad::CorrectStartEndByPath() {
    DTRACE();

    ASSERT(m_Start && m_End && m_PathCnt);

    if (m_Start->m_Center.Dist2(m_Path[0]) > m_End->m_Center.Dist2(m_Path[0])) {
        CMatrixCrotch *t = m_Start;
        m_Start = m_End;
        m_End = t;
    }
}

void CMatrixRoad::DeleteZone() {
    DTRACE();

    if (m_Zone != NULL) {
        HFree(m_Zone, m_Parent->m_Heap);
        m_Zone = NULL;
    }
    m_ZoneCnt = 0;
    m_ZoneCntMax = 0;
}

void CMatrixRoad::LoadZone(const int cnt, const int *path) {
    DTRACE();

    DeleteZone();
    if (cnt <= 0)
        return;

    ASSERT(m_Parent);

    m_Zone = (int *)HAlloc(cnt * sizeof(int), m_Parent->m_Heap);
    m_ZoneCnt = m_ZoneCntMax = cnt;
    CopyMemory(m_Zone, path, cnt * sizeof(int));
}

void CMatrixRoad::AddZone(int zone) {
    DTRACE();

    for (int i = 0; i < m_ZoneCnt; i++) {
        if (m_Zone[i] == zone) {
            m_ZoneCnt = i + 1;
            return;
        }
    }
    if (m_ZoneCnt >= m_ZoneCntMax) {
        m_ZoneCntMax += 8;
        m_Zone = (int *)HAllocClearEx(m_Zone, sizeof(int) * m_ZoneCntMax, m_Parent->m_Heap);
    }
    m_Zone[m_ZoneCnt] = zone;
    m_ZoneCnt++;
}

void CMatrixRoad::MarkZoneRoad() {
    DTRACE();

    for (int i = 0; i < m_ZoneCnt; i++) {
        m_Parent->m_Zone[m_Zone[i]].m_Road = true;
    }
}

int CMatrixRoad::GetIndexZone(int zone) {
    DTRACE();

    for (int i = 0; i < m_ZoneCnt; i++) {
        if (m_Zone[i] == zone)
            return i;
    }
    return -1;
}

void CMatrixRoad::CorrectStartEndByZone() {
    DTRACE();

    ASSERT(m_Start);
    ASSERT(m_End);
    ASSERT(m_ZoneCnt >= 1);

    if (m_Start->m_Zone == m_Zone[0]) {}
    else if (m_End->m_Zone == m_Zone[0]) {
        CMatrixCrotch *t = m_Start;
        m_Start = m_End;
        m_End = t;
    }
    else {
        ERROR_E;
    }
}

void CMatrixRoad::SplitZone(int index, CMatrixRoad *road) {
    DTRACE();

    ASSERT(index >= 0 && index < m_ZoneCnt);

    road->DeleteZone();
    road->m_ZoneCnt = m_ZoneCnt - index;
    road->m_ZoneCntMax = road->m_ZoneCnt;
    road->m_Zone = (int *)HAllocClearEx(road->m_Zone, sizeof(int) * road->m_ZoneCntMax, m_Parent->m_Heap);
    CopyMemory(road->m_Zone, m_Zone + index, sizeof(int) * road->m_ZoneCnt);

    m_ZoneCnt = index + 1;
}

void CMatrixRoad::AddZoneFromRoad(CMatrixRoad *road) {
    DTRACE();

    if (road->m_ZoneCnt <= 0)
        return;

    if (m_ZoneCnt <= 0) {
        LoadZone(road->m_ZoneCnt, road->m_Zone);
    }
    else {
        m_Zone = (int *)HAllocEx(m_Zone, (m_ZoneCnt + road->m_ZoneCnt - 1) * sizeof(int), m_Parent->m_Heap);

        if ((m_Zone[0] == road->m_Zone[0]) || (m_Zone[m_ZoneCnt - 1] == road->m_Zone[road->m_ZoneCnt - 1])) {
            // Invert zone
            for (int i = 0; i < (m_ZoneCnt >> 1); i++) {
                int tp = m_Zone[i];
                m_Zone[i] = m_Zone[m_ZoneCnt - 1 - i];
                m_Zone[m_ZoneCnt - 1 - i] = tp;
            }
        }

        if (m_Zone[m_ZoneCnt - 1] == road->m_Zone[0]) {
            CopyMemory(m_Zone + m_ZoneCnt, road->m_Zone + 1, (road->m_ZoneCnt - 1) * sizeof(int));
        }
        else if (m_Zone[0] == road->m_Zone[road->m_ZoneCnt - 1]) {  // Before
            MoveMemory(m_Zone + road->m_ZoneCnt, m_Zone + 1, (m_ZoneCnt - 1) * sizeof(int));
            CopyMemory(m_Zone, road->m_Zone, road->m_ZoneCnt * sizeof(int));
        }
        else {
            ERROR_E;
        }
        m_ZoneCnt += road->m_ZoneCnt - 1;
    }
}

void CMatrixRoad::DeleteZoneByIndex(int index) {
    DTRACE();

    ASSERT(index >= 0 && index < m_ZoneCnt);

    MoveMemory(m_Zone + index, m_Zone + index + 1, (m_ZoneCnt - (index + 1)) * sizeof(int));
    m_ZoneCnt--;
}

void CMatrixRoad::DeleteCircleZone() {
    int i = 1;
    while (i < m_ZoneCnt - 2) {
        int u = m_ZoneCnt - 2;
        while (i < u) {
            if (m_Zone[i] == m_Zone[u]) {
                MoveMemory(m_Zone + i, m_Zone + u, (m_ZoneCnt - u) * sizeof(int));
                m_ZoneCnt -= u - i;
                break;
            }
            else
                u--;
        }
        i++;
    }
}

bool CMatrixRoad::CompareZone(CMatrixRoad *road) {
    if (m_ZoneCnt != road->m_ZoneCnt)
        return false;

    int i;

    for (i = 0; i < m_ZoneCnt; i++) {
        if (m_Zone[i] != road->m_Zone[i])
            break;
    }
    if (i < m_ZoneCnt) {
        for (i = 0; i < m_ZoneCnt; i++) {
            if (m_Zone[i] != road->m_Zone[m_ZoneCnt - i - 1])
                return false;
        }
    }

    return true;
}

void CMatrixRoad::CalcDist() {
    DTRACE();

    m_Dist = 0;
    for (int i = 1; i < m_ZoneCnt; i++) {
        m_Dist +=
                (int)sqrt(double(m_Parent->m_Zone[m_Zone[i - 1]].m_Center.Dist2(m_Parent->m_Zone[m_Zone[i]].m_Center)));
    }
}

////////////////////////////////////////////////////////////////////////////////
CMatrixCrotch::CMatrixCrotch() : CMain() {
    DTRACE();

    m_Parent = NULL;
    m_Prev = NULL;
    m_Next = NULL;

    m_Center = CPoint(0, 0);
    m_Zone = -1;

    m_RoadCnt = 0;
    ZeroMemory(m_Road, 8 * sizeof(CMatrixRoad *));

    m_Critical = false;

    m_Island = 0;

    m_Region = -1;

    m_Select = false;
}

CMatrixCrotch::~CMatrixCrotch() {
    DTRACE();

    for (int i = m_RoadCnt - 1; i >= 0; i--) {
        if (m_Road[i] != NULL)
            m_Road[i]->DeleteCrotch(this);
        m_Road[i] = NULL;
    }
    m_RoadCnt = 0;
    m_Parent = NULL;
}

void CMatrixCrotch::DeleteRoad(const CMatrixRoad *road) {
    DTRACE();

    for (int i = m_RoadCnt - 1; i >= 0; i--) {
        if (m_Road[i] == road) {
            MoveMemory(m_Road + i, m_Road + i + 1, (m_RoadCnt - (i + 1)) * sizeof(CMatrixRoad *));
            m_RoadCnt--;
        }
    }
}

void CMatrixCrotch::AddRoad(CMatrixRoad *road) {
    DTRACE();

    ASSERT(m_RoadCnt < 16);
    m_Road[m_RoadCnt] = road;
    m_RoadCnt++;
}

int CMatrixCrotch::CalcCntRoadEqualMoveTypeAndNoDeadlock() {
    DTRACE();

    int r = 0;
    byte mt = m_Parent->m_Zone[m_Zone].m_Move;
    for (int i = 0; i < m_RoadCnt; i++) {
        if (m_Road[i]->GetOtherCrotch(this)->m_RoadCnt <= 1)
            continue;
        if (m_Road[i]->m_Move == mt)
            r++;
    }
    return r;
}

////////////////////////////////////////////////////////////////////////////////
CMatrixRoadRoute::CMatrixRoadRoute(CMatrixRoadNetwork *parent) : CMain() {
    ASSERT(parent);
    m_Parent = parent;
    m_Prev = NULL;
    m_Next = NULL;

    m_Start = NULL;
    m_End = NULL;

    m_ListCnt = 0;
    m_ListCntMax = 0;
    m_Header = NULL;
    m_Units = NULL;
}

CMatrixRoadRoute::~CMatrixRoadRoute() {
    Clear();
}

void CMatrixRoadRoute::Clear() {
    m_Start = NULL;
    m_End = NULL;
    m_ListCnt = 0;
    m_ListCntMax = 0;
    if (m_Header != NULL) {
        HFree(m_Header, m_Parent->m_Heap);
        m_Header = NULL;
    }
    if (m_Units != NULL) {
        HFree(m_Units, m_Parent->m_Heap);
        m_Units = NULL;
    }
}

void CMatrixRoadRoute::ClearFast() {
    m_Start = NULL;
    m_End = NULL;
    m_ListCnt = 0;
}

void CMatrixRoadRoute::NeedEmpty(int cnt, bool strong) {
    if (m_ListCnt + cnt <= m_ListCntMax)
        return;
    m_ListCntMax += m_ListCnt + cnt;
    if (strong) {
        if (m_ListCntMax <= 1)
            ;
        else if (m_ListCntMax <= 4)
            m_ListCntMax = 4;
        else
            m_ListCntMax += 8;
    }

    m_Header = (SMatrixRoadRouteHeader *)HAllocClearEx(m_Header, m_ListCntMax * sizeof(SMatrixRoadRouteHeader),
                                                       m_Parent->m_Heap);
    m_Units = (SMatrixRoadRouteUnit *)HAllocClearEx(
            m_Units, m_ListCntMax * m_Parent->m_CrotchCnt * sizeof(SMatrixRoadRouteUnit), m_Parent->m_Heap);
}

int CMatrixRoadRoute::AddList() {
    NeedEmpty();
    m_Header[m_ListCnt].m_Cnt = 0;
    m_ListCnt++;
    return m_ListCnt - 1;
}

void CMatrixRoadRoute::CopyUnit(int listfrom, int listto) {
    ASSERT(listfrom >= 0 && listfrom < m_ListCnt);
    ASSERT(listto >= 0 && listto < m_ListCnt);

    m_Header[listto] = m_Header[listfrom];
    CopyMemory(m_Units + listto * m_Parent->m_CrotchCnt, m_Units + listfrom * m_Parent->m_CrotchCnt,
               m_Header[listto].m_Cnt * sizeof(SMatrixRoadRouteUnit));
}

int CMatrixRoadRoute::AddUnit(int listno, CMatrixRoad *road, CMatrixCrotch *crotch) {
    ASSERT(listno >= 0 && listno < m_ListCnt);
    // if(!(m_Header[listno].m_Cnt<m_Parent->m_CrotchCnt)) {
    // ASSERT(1);
    //}
    ASSERT(m_Header[listno].m_Cnt < m_Parent->m_CrotchCnt);

    m_Units[listno * m_Parent->m_CrotchCnt + m_Header[listno].m_Cnt].m_Road = road;
    m_Units[listno * m_Parent->m_CrotchCnt + m_Header[listno].m_Cnt].m_Crotch = crotch;
    m_Header[listno].m_Cnt++;
    return m_Header[listno].m_Cnt - 1;
}

////////////////////////////////////////////////////////////////////////////////
CMatrixRoadNetwork::CMatrixRoadNetwork(CHeap *he) : CMain() {
    DTRACE();

    m_Heap = he;

    m_RoadCnt = 0;
    m_RoadFirst = NULL;
    m_RoadLast = NULL;

    m_CrotchCnt = 0;
    m_CrotchFirst = NULL;
    m_CrotchLast = NULL;

    m_Zone = NULL;
    m_ZoneCnt = 0;
    m_ZoneCntMax = 0;

    m_RouteFirst = NULL;
    m_RouteLast = NULL;

    m_PlaceCnt = 0;
    m_PlaceEmpty = -1;
    m_Place = NULL;

    m_RegionCnt = 0;
    m_Region = NULL;

    m_RoadFindIndex = NULL;
    m_CrotchFindIndex = NULL;
    m_RegionFindIndex = NULL;

    m_PLList = NULL;
    m_PLSizeX = m_PLSizeY = 0;
    m_PLShift = 6;  // 64
    m_PLMask = (1 << m_PLShift) - 1;
}

CMatrixRoadNetwork::~CMatrixRoadNetwork() {
    DTRACE();

    Clear();
}

void CMatrixRoadNetwork::Clear() {
    DTRACE();

    if (m_RoadFindIndex) {
        HFree(m_RoadFindIndex, m_Heap);
        m_RoadFindIndex = NULL;
    }
    if (m_CrotchFindIndex) {
        HFree(m_CrotchFindIndex, m_Heap);
        m_CrotchFindIndex = NULL;
    }
    if (m_RegionFindIndex) {
        HFree(m_RegionFindIndex, m_Heap);
        m_RegionFindIndex = NULL;
    }

    ClearPL();

    ClearRegion();

    ClearPlace();

    ClearRoute();

    ClearZone();

    while (m_RoadFirst)
        DeleteRoad(m_RoadLast);
    while (m_CrotchFirst)
        DeleteCrotch(m_CrotchLast);
}

CMatrixRoad *CMatrixRoadNetwork::AddRoad() {
    DTRACE();

    CMatrixRoad *un = HNew(m_Heap) CMatrixRoad;
    un->m_Parent = this;
    LIST_ADD(un, m_RoadFirst, m_RoadLast, m_Prev, m_Next);
    m_RoadCnt++;
    return un;
}

void CMatrixRoadNetwork::DeleteRoad(CMatrixRoad *un) {
    DTRACE();

    LIST_DEL(un, m_RoadFirst, m_RoadLast, m_Prev, m_Next);
    HDelete(CMatrixRoad, un, m_Heap);
    m_RoadCnt--;
}

bool CMatrixRoadNetwork::IsRoadExist(CMatrixRoad *un) {
    DTRACE();

    CMatrixRoad *road = m_RoadFirst;
    while (road != NULL) {
        if (un == road)
            return true;
        road = road->m_Next;
    }
    return false;
}

CMatrixRoad *CMatrixRoadNetwork::FindRoadByZone(int zone) {
    DTRACE();

    CMatrixRoad *road = m_RoadFirst;
    while (road != NULL) {
        for (int i = 0; i < road->m_ZoneCnt; i++) {
            if (road->m_Zone[i] == zone)
                return road;
        }
        road = road->m_Next;
    }
    return NULL;
}

CMatrixCrotch *CMatrixRoadNetwork::AddCrotch() {
    DTRACE();

    CMatrixCrotch *un = HNew(m_Heap) CMatrixCrotch;
    un->m_Parent = this;
    LIST_ADD(un, m_CrotchFirst, m_CrotchLast, m_Prev, m_Next);
    m_CrotchCnt++;
    return un;
}

void CMatrixRoadNetwork::DeleteCrotch(CMatrixCrotch *un) {
    DTRACE();

    LIST_DEL(un, m_CrotchFirst, m_CrotchLast, m_Prev, m_Next);
    HDelete(CMatrixCrotch, un, m_Heap);
    m_CrotchCnt--;
}

bool CMatrixRoadNetwork::IsCrotchExist(CMatrixCrotch *un) {
    DTRACE();

    CMatrixCrotch *crotch = m_CrotchFirst;
    while (crotch != NULL) {
        if (un == crotch)
            return true;
        crotch = crotch->m_Next;
    }
    return false;
}

CMatrixCrotch *CMatrixRoadNetwork::FindCrotchByZone(int zone) {
    DTRACE();

    CMatrixCrotch *crotch = m_CrotchFirst;
    while (crotch != NULL) {
        if (crotch->m_Zone == zone)
            return crotch;
        crotch = crotch->m_Next;
    }
    return NULL;
}

void CMatrixRoadNetwork::UnionRoad(DWORD flags) {
    DTRACE();

    while (UnionRoadStep(flags))
        ;
}

bool CMatrixRoadNetwork::UnionRoadStep(DWORD flags) {
    DTRACE();

    bool rv = false;
    CMatrixCrotch *crotch2, *crotch = m_CrotchFirst;
    while (crotch) {
        crotch2 = crotch;
        crotch = crotch->m_Next;

        if (crotch2->m_RoadCnt == 2 && (!crotch2->m_Critical)) {
            CMatrixRoad *r1 = crotch2->m_Road[0];
            CMatrixRoad *r2 = crotch2->m_Road[1];

            CMatrixCrotch *c1, *c2;
            if (r1->m_Start != crotch2)
                c1 = r1->m_Start;
            else
                c1 = r1->m_End;
            if (r2->m_Start != crotch2)
                c2 = r2->m_Start;
            else
                c2 = r2->m_End;

            if (c1 == c2) {
                continue;
            }
            if (c1->m_Zone == 105 && c2->m_Zone == 105) {
                ASSERT(1);
            }
            if (c1->m_Zone == c2->m_Zone) {
                continue;
            }

            if (flags & 2)
                r1->CorrectStartEndByZone();
            if (flags & 2)
                r2->CorrectStartEndByZone();

            crotch2->DeleteRoad(r1);
            crotch2->DeleteRoad(r2);

            r1->DeleteCrotch(crotch2);
            r2->DeleteCrotch(crotch2);

            c1->DeleteRoad(r1);
            c1->DeleteRoad(r2);
            c2->DeleteRoad(r1);
            c2->DeleteRoad(r2);

            if (flags & 1)
                r1->AddPathFromRoad(r2);
            if (flags & 2)
                r1->AddZoneFromRoad(r2);

            DeleteRoad(r2);
            DeleteCrotch(crotch2);

            r1->m_Start = c1;
            r1->m_End = c2;
            c1->AddRoad(r1);
            c2->AddRoad(r1);

            if (flags & 1)
                r1->CorrectStartEndByPath();
            if (flags & 2)
                r1->CorrectStartEndByZone();

            rv = true;
        }
    }
    return rv;
}

int CMatrixRoadNetwork::SelectCrotchByRoadCnt(int amin, bool skipcritical) {
    DTRACE();

    int cnt = 0;

    CMatrixCrotch *crotch = m_CrotchFirst;
    while (crotch) {
        if (!skipcritical)
            crotch->m_Select = crotch->m_RoadCnt <= amin;
        else
            crotch->m_Select = !crotch->m_Critical && crotch->m_RoadCnt <= amin;

        if (crotch->m_Select)
            cnt++;

        crotch = crotch->m_Next;
    }
    return cnt;
}

void CMatrixRoadNetwork::SelectCrotchSingleAndRadius(int radius) {
    DTRACE();

    CMatrixCrotch *crotch = m_CrotchFirst;
    while (crotch) {
        crotch->m_Select = false;
        if (crotch->m_RoadCnt == 1) {
            CPoint tp = crotch->m_Road[0]->GetOtherCrotch(crotch)->m_Center;
            tp.x -= crotch->m_Center.x;
            tp.y -= crotch->m_Center.y;
            if ((tp.x * tp.x + tp.y * tp.y) < radius * radius)
                crotch->m_Select = true;
        }
        crotch = crotch->m_Next;
    }
}

void CMatrixRoadNetwork::DeleteSelectedCrotchAndRoad() {
    DTRACE();

    CMatrixCrotch *crotch2, *crotch = m_CrotchFirst;
    while (crotch) {
        crotch2 = crotch;
        crotch = crotch->m_Next;

        if (crotch2->m_Select) {
            for (int i = crotch2->m_RoadCnt - 1; i >= 0; i--) {
                DeleteRoad(crotch2->m_Road[i]);
            }
            DeleteCrotch(crotch2);
        }
    }
}

void CMatrixRoadNetwork::SplitRoad() {
    DTRACE();

    CMatrixRoad *road = m_RoadFirst;
    while (road) {
        for (int i = 0; /*!crotch &&*/ i < road->m_ZoneCnt /*-1*/; i++) {
            CMatrixCrotch *crotch = NULL;

            if (road->m_Zone[i] == 129) {
                ASSERT(1);
            }

            CMatrixRoad *road2 = road->m_Next;
            while (!crotch && road2) {
                int iz = road2->GetIndexZone(road->m_Zone[i]);
                if (iz >= 0) {
                    crotch = InsertCrotch(road->m_Zone[i]);
                }
                road2 = road2->m_Next;
            }
        }
        road = road->m_Next;
    }

    // InsertCrotch
    /*    CMatrixRoad * road=m_RoadFirst;
        while(road) {
            for(int i=1;i<road->m_ZoneCnt-1;i++) {
                CMatrixCrotch * crotch=NULL;

                CMatrixRoad * road2=road->m_Next;
                while(road2) {
                    int iz=road2->GetIndexZone(road->m_Zone[i]);
                    if(iz==0 || iz==road2->m_ZoneCnt-1) {
                    } else if(iz>=0) {
                        if(!crotch) {
                            crotch=FindCrotchByZone(road->m_Zone[i]);
                            if(crotch==NULL) crotch=AddCrotch();
                        }

                        CMatrixRoad * road3=AddRoad();
                        road2->SplitZone(iz,road3);

                        road3->m_End=road2->m_End;
                        road2->m_End=crotch;
                        road3->m_Start=crotch;

                        crotch->AddRoad(road2);
                        crotch->AddRoad(road3);

                        road2->CorrectStartEndByZone();
                        road3->CorrectStartEndByZone();
                    }
                    road2=road2->m_Next;
                }

                if(crotch) {
                    CMatrixRoad * road3=AddRoad();
                    road->SplitZone(i,road3);

                    road3->m_End=road->m_End;
                    road->m_End=crotch;
                    road3->m_Start=crotch;

                    crotch->AddRoad(road);
                    crotch->AddRoad(road3);

                    road->CorrectStartEndByZone();
                    road3->CorrectStartEndByZone();

                    break;
                }
            }
            road=road->m_Next;
        }*/
}

void CMatrixRoadNetwork::ClearZone() {
    DTRACE();

    if (m_Zone != NULL) {
        HFree(m_Zone, m_Heap);
        m_Zone = NULL;
    }
    m_ZoneCnt = 0;
}

void CMatrixRoadNetwork::SetZoneCntMax(int cnt) {
    DTRACE();

    ASSERT(cnt >= 0);
    if (cnt == m_ZoneCntMax)
        return;

    m_ZoneCntMax = cnt;
    m_Zone = (SMatrixMapZone *)HAllocClearEx(m_Zone, m_ZoneCntMax * sizeof(SMatrixMapZone), m_Heap);

    if (m_ZoneCnt > m_ZoneCntMax)
        m_ZoneCnt = m_ZoneCntMax;
}

int CMatrixRoadNetwork::FindNearZoneByCenter(CPoint &p, int *dist2) {
    DTRACE();

    int rv = -1;
    int mz = 0;
    if (m_ZoneCnt > 0) {
        rv = 0;
        mz = p.Dist2(m_Zone[0].m_Center);
        for (int i = 1; i < m_ZoneCnt; i++) {
            int cz = p.Dist2(m_Zone[i].m_Center);
            if (cz < mz) {
                rv = i;
                mz = cz;
            }
        }
    }
    if (dist2)
        *dist2 = mz;
    return rv;
}

void CMatrixRoadNetwork::CalcZoneColor() {
    DTRACE();

    int i, t, u;

    // Назначить цвета для зон.
    const int colorscnt = 5;
    //  DWORD
    //  colors[colorscnt]={0x8000ff00,0x8000ff40,0x8000ff80,0x8000ffC0,0x8000c000,0x8000c040,0x8000c080,0x8000c0C0,0x80008000,0x80008040,0x80008080,0x800080C0};
    DWORD colors[colorscnt + 1] = {0x8000ff00, 0x80ffff00, 0x8000ff80, 0x8080ff00, 0x8000ffff, 0x800000ff};
    if (m_ZoneCnt > 0) {
        int *zonelist = (int *)HAlloc(sizeof(int) * m_ZoneCnt, m_Heap);
        int zonelistcur = 0;
        int zonelistcnt = 0;

        for (i = 0; i < m_ZoneCnt; i++) {
            m_Zone[i].m_Color = DWORD(-1);
        }

        for (i = 0; i < m_ZoneCnt; i++) {
            if (m_Zone[i].m_Bottleneck) {
                m_Zone[i].m_Color = colorscnt;
            }
            else {
                for (t = 0; t < colorscnt; t++) {
                    for (u = 0; u < m_Zone[i].m_NearZoneCnt; u++) {
                        int cz = m_Zone[i].m_NearZone[u];
                        if (m_Zone[cz].m_Color == t)
                            break;
                    }
                    if (u >= m_Zone[i].m_NearZoneCnt)
                        break;
                }
                if (t >= colorscnt)
                    t = 0;
                m_Zone[i].m_Color = t;
            }
        }

        for (i = 0; i < m_ZoneCnt; i++) {
            if (m_Zone[i].m_Color < 0 || m_Zone[i].m_Color > colorscnt)
                ERROR_E;
            m_Zone[i].m_Color = colors[m_Zone[i].m_Color];
        }

        HFree(zonelist, m_Heap);
    }
}

void CMatrixRoadNetwork::MarkZoneRoad() {
    DTRACE();

    for (int i = 0; i < m_ZoneCnt; i++)
        m_Zone[i].m_Road = false;
    CMatrixRoad *road = m_RoadFirst;
    while (road) {
        road->MarkZoneRoad();
        road = road->m_Next;
    }
}

CMatrixCrotch *CMatrixRoadNetwork::InsertCrotch(int zone) {
    DTRACE();

    if (!m_Zone[zone].m_Road)
        ERROR_E;
    CMatrixCrotch *crotch = FindCrotchByZone(zone);
    //    if(crotch) return crotch;

    if (!crotch) {
        crotch = AddCrotch();
        crotch->m_Center = m_Zone[zone].m_Center;
        crotch->m_Zone = zone;
    }

    CMatrixRoad *road = m_RoadFirst;
    while (road) {
        int ind = road->GetIndexZone(zone);
        if (ind > 0 && ind < (road->m_ZoneCnt - 1)) {
            CMatrixCrotch *c1 = road->m_Start;
            CMatrixCrotch *c2 = road->m_End;
            road->DeleteCrotch(c1);
            road->DeleteCrotch(c2);
            c1->DeleteRoad(road);
            c2->DeleteRoad(road);

            CMatrixRoad *road2 = AddRoad();
            road->SplitZone(ind, road2);

            road->m_Start = c1;
            road->m_End = crotch;
            road2->m_Start = crotch;
            road2->m_End = c2;

            c1->AddRoad(road);
            c2->AddRoad(road2);
            crotch->AddRoad(road);
            crotch->AddRoad(road2);

            road->CorrectStartEndByZone();
            road2->CorrectStartEndByZone();
        }

        road = road->m_Next;
    }

    return crotch;
}

bool CMatrixRoadNetwork::CreateCrotchAndRoadFromZone(int zone) {
    DTRACE();

    int i, u;
    CMatrixCrotch *crotch;

    if (m_Zone[zone].m_Road) {
        return InsertCrotch(zone) != NULL;
    }

    for (i = 0; i < m_ZoneCnt; i++) {
        m_Zone[i].m_FPLevel = -1;
        if (m_Zone[i].m_Bottleneck)
            m_Zone[i].m_FPWt = 4;
        else if (m_Zone[i].m_Move == 0) {
            crotch = FindCrotchByZone(i);
            if (crotch && crotch->m_RoadCnt >= 3)
                m_Zone[i].m_FPWt = 1;
            else if (crotch && crotch->m_RoadCnt >= 1)
                m_Zone[i].m_FPWt = 2;
            else
                m_Zone[i].m_FPWt = 3;
        }
        else
            m_Zone[i].m_FPWt = 10;
        m_Zone[i].m_FPWtp = 0;
    }

    int *zb = (int *)HAlloc(m_ZoneCnt * sizeof(int) * 2, m_Heap);
    int *zb1 = zb;
    int *zb2 = zb1 + m_ZoneCnt;
    int zb1cnt = 0;
    int zb2cnt = 0;

    int level = 0;

    zb1[0] = zone;
    zb1cnt++;
    m_Zone[zone].m_FPLevel = level;
    m_Zone[zone].m_FPWtp = 0;
    level++;

    int zonefound = -1;
    int zonefound_wtp_min = 0x7fffffff;

    while (zb1cnt > 0) {
        for (i = 0; i < zb1cnt; i++) {
            for (u = 0; u < m_Zone[zb1[i]].m_NearZoneCnt; u++) {
                int nz = m_Zone[zb1[i]].m_NearZone[u];
                int wtp = m_Zone[zb1[i]].m_FPWtp + m_Zone[nz].m_FPWt;

                if (m_Zone[nz].m_FPLevel < 0 || wtp < m_Zone[nz].m_FPWtp) {
                    m_Zone[nz].m_FPLevel = level;
                    m_Zone[nz].m_FPWtp = wtp;
                    zb2[zb2cnt] = nz;
                    zb2cnt++;

                    if (m_Zone[nz].m_Road && (zonefound < 0 || wtp < zonefound_wtp_min)) {
                        zonefound = nz;
                        zonefound_wtp_min = wtp;
                    }
                }
            }
        }

        level++;
        zb1cnt = zb2cnt;
        zb2cnt = 0;
        int *zbt = zb1;
        zb1 = zb2;
        zb2 = zbt;
    }

    HFree(zb, m_Heap);

    if (zonefound < 0)
        return false;

    crotch = AddCrotch();
    crotch->m_Center = m_Zone[zone].m_Center;
    crotch->m_Zone = zone;

    CMatrixCrotch *crotch2 = InsertCrotch(zonefound);

    CMatrixRoad *road = AddRoad();
    road->m_Start = crotch;
    road->m_End = crotch2;
    crotch->AddRoad(road);
    crotch2->AddRoad(road);

    road->AddZone(zonefound);
    int cnt = 1;
    int curzone = zonefound;
    int curwt = m_Zone[curzone].m_FPWtp;
    while (true) {
        int nz_ = -1;
        int wtp_ = curwt;
        for (i = 0; i < m_Zone[curzone].m_NearZoneCnt; i++) {
            int nz = m_Zone[curzone].m_NearZone[i];
            int wtp = m_Zone[nz].m_FPWtp;
            if (nz == zone) {
                nz_ = nz;
                wtp_ = wtp;
                break;
            }
            else if (wtp <= wtp_) {
                nz_ = nz;
                wtp_ = wtp;
            }
        }
        if (nz_ < 0) {
            ERROR_E;
        }

        curzone = nz_;
        curwt = wtp_;
        ASSERT(cnt + 1 <= m_ZoneCnt);
        road->AddZone(curzone);
        if (curzone == zone)
            break;
    }

    road->CorrectStartEndByZone();
    road->MarkZoneRoad();

    return true;
}

bool CMatrixRoadNetwork::CreateCrotchAndRoadByPath(int *path, int cnt) {
    ASSERT(cnt >= 3);
    ASSERT(m_Zone[path[0]].m_Road);
    ASSERT(m_Zone[path[cnt - 1]].m_Road);

    int ibegin = 1;

    while (ibegin < cnt - 1) {
        while (ibegin < cnt && m_Zone[path[ibegin]].m_Road)
            ibegin++;
        ibegin--;

        int iend = ibegin + 1;
        while (iend < cnt && !m_Zone[path[iend]].m_Road)
            iend++;
        if (iend >= cnt)
            iend--;

        if (iend - ibegin + 1 >= 2) {
            CMatrixCrotch *c1 = InsertCrotch(path[ibegin]);
            CMatrixCrotch *c2 = InsertCrotch(path[iend]);

            CMatrixRoad *road = AddRoad();
            road->LoadZone(iend - ibegin + 1, path + ibegin);
            road->m_Start = c1;
            road->m_End = c2;
            c1->AddRoad(road);
            c2->AddRoad(road);
            road->MarkZoneRoad();
            road->CorrectStartEndByZone();
        }

        ibegin = iend;
    }

    return true;
}

void CMatrixRoadNetwork::MergeEqualCrotch() {
    DTRACE();

    CMatrixCrotch *crotch = m_CrotchFirst;
    while (crotch) {
        CMatrixCrotch *crotch2 = crotch->m_Next;
        while (crotch2) {
            CMatrixCrotch *crotch3 = crotch2;
            crotch2 = crotch2->m_Next;

            if (crotch->m_Zone == crotch3->m_Zone) {
                CMatrixRoad *road = m_RoadFirst;
                while (road) {
                    CMatrixRoad *road2 = road;
                    road = road->m_Next;

                    if (road2->m_Start == crotch3 || road2->m_End == crotch3) {
                        if (road2->m_Start == crotch || road2->m_End == crotch) {
                            DeleteRoad(road2);
                        }
                        else {
                            if (road2->m_Start == crotch3) {
                                road2->m_Start->DeleteRoad(road2);
                                road2->DeleteCrotch(crotch3);
                                crotch3->DeleteRoad(road2);
                                road2->m_Start = crotch;
                                crotch->AddRoad(road2);
                            }
                            else {
                                road2->m_End->DeleteRoad(road2);
                                road2->DeleteCrotch(crotch3);
                                crotch3->DeleteRoad(road2);
                                road2->m_End = crotch;
                                crotch->AddRoad(road2);
                            }
                            road2->CorrectStartEndByZone();
                        }
                    }
                }
                if (crotch3->m_Critical) {
                    crotch->m_Critical = true;
                }
                DeleteCrotch(crotch3);
            }
        }
        crotch = crotch->m_Next;
    }
}

void CMatrixRoadNetwork::CalcIsland() {
    DTRACE();

    int i, clcnt, clcur;
    CMatrixCrotch *crotch2, *crotch3;

    CMatrixCrotch *crotch = m_CrotchFirst;
    while (crotch) {
        crotch->m_Island = -1;
        crotch = crotch->m_Next;
    }

    int level = 0;

    CMatrixCrotch **cl = (CMatrixCrotch **)HAlloc(m_CrotchCnt * sizeof(CMatrixCrotch *), m_Heap);

    crotch = m_CrotchFirst;
    while (crotch) {
        if (crotch->m_Island < 0) {
            crotch->m_Island = level;
            cl[0] = crotch;
            clcur = 0;
            clcnt = 1;

            while (clcur < clcnt) {
                crotch2 = cl[clcur];
                for (i = 0; i < crotch2->m_RoadCnt; i++) {
                    crotch3 = crotch2->m_Road[i]->GetOtherCrotch(crotch2);
                    if (crotch3->m_Island < 0) {
                        crotch3->m_Island = level;
                        cl[clcnt] = crotch3;
                        clcnt++;
                    }
                }
                clcur++;
            }

            level++;
        }
        crotch = crotch->m_Next;
    }

    HFree(cl, m_Heap);
}

void CMatrixRoadNetwork::SelectUnneededIsland() {
    int cnt = 0;
    CMatrixCrotch *crotch = m_CrotchFirst;
    while (crotch) {
        cnt = std::max(cnt, crotch->m_Island + 1);
        crotch->m_Select = false;
        crotch = crotch->m_Next;
    }

    for (int i = 0; i < cnt; i++) {
        crotch = m_CrotchFirst;
        while (crotch) {
            if (crotch->m_Island == i && crotch->m_Critical)
                break;
            crotch = crotch->m_Next;
        }
        if (!crotch) {
            crotch = m_CrotchFirst;
            while (crotch) {
                if (crotch->m_Island == i)
                    crotch->m_Select = true;
                crotch = crotch->m_Next;
            }
        }
    }
}

void CMatrixRoadNetwork::Unselect() {
    CMatrixCrotch *crotch = m_CrotchFirst;
    while (crotch) {
        crotch->m_Select = false;
        crotch = crotch->m_Next;
    }
}

int CMatrixRoadNetwork::CalcPathZoneByRoad(int zstart, int zend, int *path) {
    DTRACE();

    int i, u;

    if (zstart == zend)
        return 0;

    ASSERT(m_Zone[zstart].m_Road);
    ASSERT(m_Zone[zend].m_Road);

    for (i = 0; i < m_ZoneCnt; i++) {
        m_Zone[i].m_FPLevel = -1;
        m_Zone[i].m_FPWt = 1;
        m_Zone[i].m_FPWtp = 0;
    }

    int *fp = (int *)HAlloc(m_ZoneCnt * sizeof(int *) * 2, m_Heap);
    int *fp1 = fp;
    int *fp2 = fp + m_ZoneCnt;
    int fpcnt1 = 0;
    int fpcnt2 = 0;
    int level = 0;

    fp1[fpcnt1] = zend;
    fpcnt1++;
    m_Zone[zend].m_FPLevel = level;
    m_Zone[zend].m_FPWtp = 0;
    level++;

    bool fok = false;

    // HelperDestroyByGroup(2);

    while (fpcnt1 > 0) {
        for (i = 0; i < fpcnt1; i++) {
            for (u = 0; u < m_Zone[fp1[i]].m_NearZoneCnt; u++) {
                int nz = m_Zone[fp1[i]].m_NearZone[u];
                if (!m_Zone[nz].m_Road)
                    continue;

                int wtp = m_Zone[fp1[i]].m_FPWtp + m_Zone[nz].m_FPWt;

                if (m_Zone[nz].m_FPLevel < 0 || wtp < m_Zone[nz].m_FPWtp) {
                    m_Zone[nz].m_FPLevel = level;
                    m_Zone[nz].m_FPWtp = wtp;
                    fp2[fpcnt2] = nz;
                    fpcnt2++;

                    // CHelper::Create(0,2)->Cone(D3DXVECTOR3(10.0f*m_Zone[nz].m_Center.x,10.0f*m_Zone[nz].m_Center.y,0),D3DXVECTOR3(10.0f*m_Zone[nz].m_Center.x,10.0f*m_Zone[nz].m_Center.y,10.0f+wtp*2.0f),0.5f,0.5f,0xffffffff,0xffff0000,6);

                    if (zstart == nz)
                        fok = true;
                }
            }
        }

        level++;
        fpcnt1 = fpcnt2;
        fpcnt2 = 0;
        int *fpt = fp1;
        fp1 = fp2;
        fp2 = fpt;
    }

    HFree(fp, m_Heap);

    if (!fok)
        return 0;

    path[0] = zstart;
    int cnt = 1;
    int curzone = zstart;
    int curwt = m_Zone[curzone].m_FPWtp;
    while (true) {
        int nz_ = -1;
        int wtp_ = curwt;
        for (int i = 0; i < m_Zone[curzone].m_NearZoneCnt; i++) {
            int nz = m_Zone[curzone].m_NearZone[i];
            if (!m_Zone[nz].m_Road)
                continue;
            int wtp = m_Zone[nz].m_FPWtp;
            if (nz == zend) {
                nz_ = nz;
                wtp_ = wtp;
                break;
            }
            else if (wtp <= wtp_) {
                nz_ = nz;
                wtp_ = wtp;
            }
        }
        if (nz_ < 0) {
            ERROR_E;
        }

        curzone = nz_;
        curwt = wtp_;
        ASSERT(cnt + 1 <= m_ZoneCnt);
        path[cnt] = curzone;
        cnt++;
        // CHelper::Create(0,2)->Cone(D3DXVECTOR3(10.0f*m_Zone[path[cnt-2]].m_Center.x,10.0f*m_Zone[path[cnt-2]].m_Center.y,30),D3DXVECTOR3(10.0f*m_Zone[path[cnt-1]].m_Center.x,10.0f*m_Zone[path[cnt-1]].m_Center.y,30.0f),0.5f,0.5f,0xffffffff,0xffff0000,6);
        if (curzone == zend)
            break;
    }

    return cnt;
}

int CMatrixRoadNetwork::CalcDistByRoad(int zstart, int zend, int *path) {
    DTRACE();

    ASSERT(m_ZoneCnt);

    bool delpath = false;
    if (!path) {
        path = (int *)HAlloc(m_ZoneCnt * sizeof(int), m_Heap);
        delpath = true;
    }

    int rv = 0;
    int cnt = CalcPathZoneByRoad(zstart, zend, path);
    if (cnt <= 0)
        rv = -1;
    else {
        for (int i = 1; i < cnt; i++)
            rv += (int)sqrt((double)m_Zone[path[i - 1]].m_Center.Dist2(m_Zone[path[i]].m_Center));
    }

    if (delpath)
        HFree(path, m_Heap);

    return rv;
}

void CMatrixRoadNetwork::CalcDistRoad() {
    CMatrixRoad *road = m_RoadFirst;
    while (road) {
        road->CalcDist();
        road = road->m_Next;
    }
}

struct SZT_CAR {
    int level;
    int level2;
    CMatrixRoad *road;
    int start;
};

struct SAddPath_CAR {
    int len;
    int *path;
    int dist;
};

int MRN_CAR_BuildPath(CMatrixRoadNetwork *rnw, SZT_CAR *zt, CMatrixRoad *road, int *path, int zstart) {
    DTRACE();

    path[0] = zstart;
    int cnt = 1;

    int curzone = zstart;
    int curlevel = zt[curzone].level2;
    if (zt[curzone].road != road)
        curlevel++;

    while (curlevel > 0) {
        int ccnt = rnw->m_Zone[curzone].m_NearZoneCnt;
        int i;
        for (i = 0; i < ccnt && curlevel > 0; i++) {
            int nz = rnw->m_Zone[curzone].m_NearZone[i];
            if (zt[nz].road == road) {
                if ((zt[nz].level >= 0 && zt[nz].level < curlevel) ||
                    (zt[nz].level2 >= 0 && zt[nz].level2 < curlevel)) {
                    path[cnt] = nz;
                    cnt++;
                    curzone = nz;
                    if (zt[nz].level >= 0)
                        curlevel = zt[nz].level;
                    else
                        curlevel = zt[nz].level2;
                    break;
                }
            }
        }
        if (i >= ccnt)
            break;
    }

    if (curlevel != 0) {
        ERROR_E;
    }

    path[cnt] = zt[curzone].start;
    cnt++;

    return cnt;
}

void CMatrixRoadNetwork::DeleteCircleZone() {
    CMatrixRoad *road = m_RoadFirst;
    while (road) {
        road->DeleteCircleZone();
        road = road->m_Next;
    }
}

void CMatrixRoadNetwork::DeleteEqualRoad() {
    CMatrixRoad *road = m_RoadFirst;
    while (road) {
        CMatrixRoad *road2 = road->m_Next;
        while (road2) {
            CMatrixRoad *road3 = road2;
            road2 = road2->m_Next;

            if (road->CompareZone(road3)) {
                DeleteRoad(road3);
            }
        }
        road = road->m_Next;
    }
}

void CMatrixRoadNetwork::CreateAdditionalRoad() {
    DTRACE();

    int i, u, t;

    ASSERT(m_ZoneCnt);

    SZT_CAR *zt = (SZT_CAR *)HAllocClear(m_ZoneCnt * sizeof(SZT_CAR), m_Heap);

    int ap_maxcnt = 16;
    int ap_cnt = 0;
    SAddPath_CAR *ap = (SAddPath_CAR *)HAllocClear(ap_maxcnt * sizeof(SAddPath_CAR), m_Heap);

    for (i = 0; i < m_ZoneCnt; i++) {
        zt[i].level = -1;
        zt[i].level2 = -1;
    }

    // const int colorscnt=7;
    // DWORD colors[colorscnt]={0xff00ff00,0xffffff00,0xff00ff80,0xff80ff00,0xff00ffff,0xff0000ff,0xff0080ff};

    // HelperDestroyByGroup(2);
    // Находим границы
    for (i = 0; i < m_ZoneCnt; i++) {
        if (m_Zone[i].m_Road)
            continue;
        for (u = 0; u < m_Zone[i].m_NearZoneCnt; u++) {
            if (m_Zone[m_Zone[i].m_NearZone[u]].m_Road) {
                zt[i].level = 0;
                zt[i].start = m_Zone[i].m_NearZone[u];
                zt[i].road = FindRoadByZone(zt[i].start);
                ASSERT(zt[i].road);

                // CHelper::Create(0,2)->Cone(D3DXVECTOR3(10.0f*m_Zone[i].m_Center.x,10.0f*m_Zone[i].m_Center.y,0),D3DXVECTOR3(10.0f*m_Zone[i].m_Center.x,10.0f*m_Zone[i].m_Center.y,10.0f),1.5f,1.5f,0xffffffff,colors[(zt[i].road->m_Start->m_Zone)
                // % colorscnt],6);

                break;
            }
        }
    }

    // Растем
    bool findok = true;
    int level = 1;
    while (findok) {
        findok = false;

        for (int i = 0; i < m_ZoneCnt; i++)
            zt[i].level2 = -1;

        for (int i = 0; i < m_ZoneCnt; i++) {
            // if(i==135) {
            //    ASSERT(1);
            //}
            if (m_Zone[i].m_Road)
                continue;
            if (zt[i].level >= 0)
                continue;
            for (u = 0; u < m_Zone[i].m_NearZoneCnt; u++) {
                if (zt[m_Zone[i].m_NearZone[u]].level >= 0) {
                    zt[i].level2 = level;
                    zt[i].road = zt[m_Zone[i].m_NearZone[u]].road;
                    findok = true;
                    break;
                }
            }
            if (u < m_Zone[i].m_NearZoneCnt) {
                u++;
                for (; u < m_Zone[i].m_NearZoneCnt; u++) {
                    if ((zt[m_Zone[i].m_NearZone[u]].level >= 0 || zt[m_Zone[i].m_NearZone[u]].level2 >= 0) &&
                        zt[m_Zone[i].m_NearZone[u]].road != zt[i].road) {
                        // CHelper::Create(0,2)->Cone(D3DXVECTOR3(10.0f*m_Zone[i].m_Center.x,10.0f*m_Zone[i].m_Center.y,0),D3DXVECTOR3(10.0f*m_Zone[i].m_Center.x,10.0f*m_Zone[i].m_Center.y,10.0f),2.0f,2.0f,0xffff0000,0xffff0000,6);

                        if (ap_cnt >= ap_maxcnt) {
                            ap_maxcnt = ap_cnt + 16;
                            ap = (SAddPath_CAR *)HAllocClearEx(ap, ap_maxcnt * sizeof(SAddPath_CAR), m_Heap);
                        }

                        ap[ap_cnt].path =
                                (int *)HAllocClearEx(ap[ap_cnt].path, (zt[i].level2 * 2 + 4) * sizeof(int), m_Heap);

                        // Поиск первого пути
                        int len = MRN_CAR_BuildPath(this, zt, zt[i].road, ap[ap_cnt].path, i);
                        for (t = 0; t < (len >> 1); t++) {
                            int swap = ap[ap_cnt].path[t];
                            ap[ap_cnt].path[t] = ap[ap_cnt].path[len - t - 1];
                            ap[ap_cnt].path[len - t - 1] = swap;
                        }

                        // Поиск второго пути
                        ap[ap_cnt].len = len - 1 +
                                         MRN_CAR_BuildPath(this, zt, zt[m_Zone[i].m_NearZone[u]].road,
                                                           ap[ap_cnt].path + len - 1, i);

                        ap_cnt++;

                        //                        break;
                    }
                }
            }
        }

        for (int i = 0; i < m_ZoneCnt; i++) {
            if (zt[i].level2 >= 0) {
                zt[i].level = zt[i].level2;
                // CHelper::Create(0,2)->Cone(D3DXVECTOR3(10.0f*m_Zone[i].m_Center.x,10.0f*m_Zone[i].m_Center.y,0),D3DXVECTOR3(10.0f*m_Zone[i].m_Center.x,10.0f*m_Zone[i].m_Center.y,10.0f+5.0f*zt[i].level),1.5f,1.5f,0xffffffff,colors[(zt[i].road->m_Start->m_Zone)
                // % colorscnt],6);
            }
        }

        level++;
    }

    // Расчитываем дальность
    for (i = 0; i < ap_cnt; i++) {
        ap[i].dist = 0;
        for (u = 1; u < ap[i].len; u++) {
            ap[i].dist += (int)sqrt(double(m_Zone[ap[i].path[u - 1]].m_Center.Dist2(m_Zone[ap[i].path[u]].m_Center)));
        }
    }

    // Сортируем по дальности
    for (i = 0; i < ap_cnt - 2; i++) {
        for (u = i + 1; u < ap_cnt; u++) {
            if (ap[i].dist > ap[u].dist) {
                SAddPath_CAR t = ap[i];
                ap[i] = ap[u];
                ap[u] = t;
            }
        }
    }

    // Удаляем лишние если начало и конец одинаковые
    int distdel = 16;
    int distdel2 = distdel * distdel;
    for (i = 0; i < ap_cnt; i++) {
        if (ap[i].len < 2)
            continue;

        int zs1 = ap[i].path[0];
        int ze1 = ap[i].path[ap[i].len - 1];

        for (u = i + 1; u < ap_cnt; u++) {
            if (ap[u].len < 2)
                continue;

            int zs2 = ap[u].path[0];
            int ze2 = ap[u].path[ap[u].len - 1];

            if (m_Zone[zs1].m_Center.Dist2(m_Zone[zs2].m_Center) < distdel2 &&
                m_Zone[ze1].m_Center.Dist2(m_Zone[ze2].m_Center) < distdel2) {
                ap[u].len = 0;
            }
            else if (m_Zone[zs1].m_Center.Dist2(m_Zone[ze2].m_Center) < distdel2 &&
                     m_Zone[ze1].m_Center.Dist2(m_Zone[zs2].m_Center) < distdel2) {
                ap[u].len = 0;
            }
        }
    }

    // Удаляем если дистанция между начало и концом по основной дороге маленькая
    int *zpath = (int *)HAlloc(m_ZoneCnt * sizeof(int), m_Heap);
    for (i = 0; i < ap_cnt; i++) {
        if (ap[i].len < 2)
            continue;
        int dist = CalcDistByRoad(ap[i].path[0], ap[i].path[ap[i].len - 1], zpath);
        if (dist >= 0 && dist <= 32) {
            ap[i].len = 0;
        }
    }
    HFree(zpath, m_Heap);

    // Создаем дороги
    for (i = 0; i < ap_cnt; i++) {
        if (ap[i].len < 2)
            continue;
        CreateCrotchAndRoadByPath(ap[i].path, ap[i].len);
    }

    //    for(i=0;i<ap_cnt;i++) {
    //        for(u=1;u<ap[i].len;u++) {
    // CHelper::Create(0,2)->Cone(D3DXVECTOR3(10.0f*m_Zone[ap[i].path[u-1]].m_Center.x,10.0f*m_Zone[ap[i].path[u-1]].m_Center.y,10.0f+2.0f*i),D3DXVECTOR3(10.0f*m_Zone[ap[i].path[u]].m_Center.x,10.0f*m_Zone[ap[i].path[u]].m_Center.y,10.0f+2.0f*i),0.5f,0.5f,0xffffffff,0xffffffff,6);
    //        }
    //    }

    for (i = 0; i < ap_maxcnt; i++) {
        if (ap[i].path) {
            HFree(ap[i].path, m_Heap);
            ap[i].path = NULL;
        }
    }
    HFree(ap, m_Heap);
    HFree(zt, m_Heap);
}

void CMatrixRoadNetwork::FinalizeBuild() {
    DTRACE();

    m_ZoneCntMax = m_ZoneCnt;
    m_Zone = (SMatrixMapZone *)HAllocEx(m_Zone, m_ZoneCntMax * sizeof(SMatrixMapZone), m_Heap);

    CMatrixRoad *road = m_RoadFirst;
    while (road) {
        road->DeletePath();
        road->m_ZoneCntMax = road->m_ZoneCnt;
        road->m_Zone = (int *)HAllocEx(road->m_Zone, road->m_ZoneCntMax * sizeof(int), m_Heap);
        road = road->m_Next;
    }
}

void CMatrixRoadNetwork::FinalizePlace() {
    int cnt = 0;
    for (int i = 0; i < m_PlaceCnt; i++) {
        if (!m_Place[i].m_Empty)
            cnt++;
    }

    if (cnt <= 0) {
        ClearPlace();
    }
    else {
        SMatrixPlace *pt = (SMatrixPlace *)HAllocClear(cnt * sizeof(SMatrixPlace), m_Heap);

        int u = 0;
        for (int i = 0; i < m_PlaceCnt; i++) {
            if (!m_Place[i].m_Empty) {
                pt[u] = m_Place[i];
                pt[u].m_EmptyNext = -1;
                u++;
            }
        }
        HFree(m_Place, m_Heap);
        m_Place = pt;
        m_PlaceCnt = u;
        m_PlaceEmpty = -1;
    }
}

bool CMatrixRoadNetwork::IsValidate(bool exception) {
    DTRACE();

    for (int i = 0; i < m_ZoneCnt; i++)
        m_Zone[i].m_FPLevel = 0;

    // Розвилки должны иметь правельные не нулевые указатели
    CMatrixCrotch *crotch = m_CrotchFirst;
    while (crotch) {
        for (int i = 0; i < crotch->m_RoadCnt; i++) {
            if (!IsRoadExist(crotch->m_Road[i])) {
                if (exception)
                    ERROR_E;
                else
                    return false;
            }
        }
        if (crotch->m_Zone < 0) {
            if (exception)
                ERROR_E;
            else
                return false;
        }
        m_Zone[crotch->m_Zone].m_FPLevel++;

        crotch = crotch->m_Next;
    }

    // На одной зоне не должнобыть более одной развилки
    for (int i = 0; i < m_ZoneCnt; i++) {
        if (m_Zone[i].m_FPLevel > 1) {
            if (exception)
                ERROR_E;
            else
                return false;
        }
        m_Zone[i].m_FPLevel = 0;
    }

    // Дороги должны иметь правельные не нулевые указатели
    CMatrixRoad *road = m_RoadFirst;
    while (road) {
        if (!IsCrotchExist(road->m_Start)) {
            if (exception)
                ERROR_E;
            else
                return false;
        }
        if (!IsCrotchExist(road->m_End)) {
            if (exception)
                ERROR_E;
            else
                return false;
        }
        if (road->m_ZoneCnt < 2) {
            if (exception)
                ERROR_E;
            else
                return false;
        }
        for (int i = 0; i < road->m_ZoneCnt; i++) {
            m_Zone[road->m_Zone[i]].m_FPLevel++;
        }
        road = road->m_Next;
    }

    // На одной зоне вне разилок не должнобыть более одной дороги
    for (int i = 0; i < m_ZoneCnt; i++) {
        if (!FindCrotchByZone(i) && m_Zone[i].m_FPLevel > 1) {
            if (exception)
                ERROR_E;
            else
                return false;
        }
        m_Zone[i].m_FPLevel = 0;
    }

    return true;
}

void CMatrixRoadNetwork::Test() {
    CMatrixRoad *road = m_RoadFirst;
    while (road) {
        if (road->m_Start == 0) {
            ERROR_E;
        }
        if (road->m_End == 0) {
            ERROR_E;
        }
        /*        if(!IsCrotchExist(road->m_Start)) {
                    ERROR_E;
                }
                if(!IsCrotchExist(road->m_End)) {
                    ERROR_E;
                }*/
        road = road->m_Next;
    }
}

void CMatrixRoadNetwork::ClearRoute() {
    while (m_RouteFirst)
        DeleteRoute(m_RouteLast);
}

void CMatrixRoadNetwork::DeleteRoute(CMatrixRoadRoute *route) {
    LIST_DEL(route, m_RouteFirst, m_RouteLast, m_Prev, m_Next);
    HDelete(CMatrixRoadRoute, route, m_Heap);
}

CMatrixRoadRoute *CMatrixRoadNetwork::AddRoute() {
    CMatrixRoadRoute *route = HNew(m_Heap) CMatrixRoadRoute(this);
    LIST_DEL(route, m_RouteFirst, m_RouteLast, m_Prev, m_Next);
    return route;
}

void CalcRoute_r(CMatrixCrotch *cstart, CMatrixCrotch *cend, CMatrixRoadRoute *route, CMatrixRoad *road,
                 CMatrixCrotch *crotch, int maxdist) {
    route->AddUnit(route->m_ListCnt - 1, road, crotch);
    if (road)
        route->m_Header[route->m_ListCnt - 1].m_Dist += road->m_Dist;
    if (crotch == cend) {
        if (route->m_Header[route->m_ListCnt - 1].m_Cnt > 1) {
            route->AddList();
            route->CopyUnit(route->m_ListCnt - 2, route->m_ListCnt - 1);
        }
        if (road)
            route->m_Header[route->m_ListCnt - 1].m_Dist -= road->m_Dist;
        route->m_Header[route->m_ListCnt - 1].m_Cnt--;
        return;
    }
    crotch->m_Select = true;

    for (int i = 0; i < crotch->m_RoadCnt; i++) {
        CMatrixCrotch *cin = crotch->m_Road[i]->GetOtherCrotch(crotch);
        if (cin->m_Select)
            continue;
        if (route->m_Header[route->m_ListCnt - 1].m_Dist + crotch->m_Road[i]->m_Dist > maxdist)
            continue;
        if (cin == cend || cin->m_RoadCnt > 1) {
            CalcRoute_r(cstart, cend, route, crotch->m_Road[i], cin, maxdist);
        }
    }

    crotch->m_Select = false;
    if (road)
        route->m_Header[route->m_ListCnt - 1].m_Dist -= road->m_Dist;
    route->m_Header[route->m_ListCnt - 1].m_Cnt--;
}

CMatrixRoadRoute *CMatrixRoadNetwork::CalcRoute(CMatrixCrotch *cstart, CMatrixCrotch *cend) {
    Unselect();

    int maxdist = CalcDistByRoad(cstart->m_Zone, cend->m_Zone) * 2;

    CMatrixRoadRoute *route = AddRoute();
    route->AddList();
    CalcRoute_r(cstart, cend, route, NULL, cstart, maxdist);
    route->m_ListCnt--;

    return route;
}

CMatrixRoadRoute *CMatrixRoadNetwork::FindRoute(CMatrixCrotch *cstart, CMatrixCrotch *cend) {
    CMatrixRoadRoute *route = m_RouteFirst;
    while (route) {
        if (route->m_Start == cstart && route->m_End == cend) {
            return route;
        }
        route = route->m_Next;
    }
    return NULL;
}

CMatrixRoadRoute *CMatrixRoadNetwork::GetRoute(CMatrixCrotch *cstart, CMatrixCrotch *cend) {
    CMatrixRoadRoute *route = FindRoute(cstart, cend);
    if (!route)
        route = CalcRoute(cstart, cend);
    return route;
}

#if (defined _DEBUG) && !(defined _RELDEBUG)
#include "../MatrixSide.hpp"
#include "../MatrixMap.hpp"
#include "../MatrixLogic.hpp"
#endif

void CMatrixRoadNetwork::FindPathFromCrotchToRegion(byte mm, CMatrixCrotch *cstart, int region, CMatrixRoadRoute *rr,
                                                    bool test) {
    // cstart в rr не добовляется
    int i;
    if (cstart->m_Region == region)
        return;

    if (!m_RoadFindIndex)
        m_RoadFindIndex = (CMatrixRoad **)HAlloc(m_RoadCnt * sizeof(CMatrixRoad *), m_Heap);
    if (!m_CrotchFindIndex)
        m_CrotchFindIndex = (CMatrixCrotch **)HAlloc(m_CrotchCnt * sizeof(CMatrixCrotch *), m_Heap);

    CMatrixCrotch *crotchcur = NULL;

    for (int type = 0; type <= 2; type++) {
        CMatrixRoad *road2;
        CMatrixCrotch *crotch2;
        CMatrixCrotch *crotch = m_CrotchFirst;
        while (crotch) {
            crotch->m_Data = 0;
            crotch = crotch->m_Next;
        }

        int next, cnt = 0, sme = 0, level = 1;
        m_CrotchFindIndex[cnt] = cstart;
        cstart->m_Data = level;
        cnt++;
        level++;

        next = cnt;

        while (sme < cnt) {
            crotch = m_CrotchFindIndex[sme];
            for (i = 0; i < crotch->m_RoadCnt; i++) {
                crotch2 = crotch->m_Road[i]->GetOtherCrotch(crotch);
                if (crotch2->m_Data)
                    continue;
                if (type == 0 && crotch2->m_Region != cstart->m_Region && crotch2->m_Region != region &&
                    crotch2->m_Region >= 0)
                    continue;  // В другие регионы не заглядываем
                if ((type == 0 || type == 1) && crotch->m_Road[i]->m_Move & mm)
                    continue;  // Если хоть один робот не проходит то пропускаем.

#if (defined _DEBUG) && !(defined _RELDEBUG) && !(defined _DISABLE_AI_HELPERS)
                if (test && !g_TestLocal) {
                    D3DXVECTOR3 v2;
                    v2.x = crotch2->m_Center.x * GLOBAL_SCALE_MOVE;
                    v2.y = crotch2->m_Center.y * GLOBAL_SCALE_MOVE;
                    v2.z = g_MatrixMap->GetZ(v2.x, v2.y);
                    CHelper::Create(0, 101)->Cone(v2, D3DXVECTOR3(v2.x, v2.y, v2.z + 10.0f + level * 2.0f), 5.0f, 5.0f,
                                                  0xff00ff00, 0xff00ff00, 3);
                }
#endif

                ASSERT(cnt < m_CrotchCnt);

                crotch2->m_Data = level;
                m_CrotchFindIndex[cnt] = crotch2;
                cnt++;

                if (crotch2->m_Region == region) {
                    crotchcur = crotch2;
                    break;
                }
            }
            if (crotchcur)
                break;

            sme++;
            if (sme >= next) {
                next = cnt;
                level++;
            }
        }

        if (!crotchcur)
            continue;

#if (defined _DEBUG) && !(defined _RELDEBUG) && !(defined _DISABLE_AI_HELPERS)
        if (test && !g_TestLocal) {
            D3DXVECTOR3 v2;
            v2.x = crotchcur->m_Center.x * GLOBAL_SCALE_MOVE;
            v2.y = crotchcur->m_Center.y * GLOBAL_SCALE_MOVE;
            v2.z = g_MatrixMap->GetZ(v2.x, v2.y);
            CHelper::Create(0, 101)->Cone(v2, D3DXVECTOR3(v2.x, v2.y, v2.z + 8.0f + level * 2.0f), 6.0f, 6.0f,
                                          0xffffff00, 0xffffff00, 3);
        }
#endif

        cnt = 0;
        m_CrotchFindIndex[cnt] = crotchcur;
        cnt++;

        while (crotchcur != cstart) {
            crotch2 = NULL;
            for (i = 0; i < crotchcur->m_RoadCnt; i++) {
                crotch = crotchcur->m_Road[i]->GetOtherCrotch(crotchcur);
                if (crotch->m_Data && crotch->m_Data < level) {
                    crotch2 = crotch;
                    road2 = crotchcur->m_Road[i];
                    break;
                }
            }
            if (!crotch2)
                ERROR_E;

            ASSERT(cnt < m_CrotchCnt);
            ASSERT(cnt < m_RoadCnt);

            m_RoadFindIndex[cnt - 1] = road2;
            m_CrotchFindIndex[cnt] = crotch2;
            cnt++;

            crotchcur = crotch2;
            level = crotchcur->m_Data;
        }

        for (i = cnt - 2; i >= 0; i--) {
            rr->AddUnit(rr->m_ListCnt - 1, m_RoadFindIndex[i], m_CrotchFindIndex[i]);
            rr->m_Header[rr->m_ListCnt - 1].m_Dist += m_RoadFindIndex[i]->m_Dist;
        }
        break;
    }
    if (!crotchcur) {
        rr->Clear();
        // ERROR_E;
    }
}

void CMatrixRoadNetwork::FindPathFromRegionPath(byte mm, int rcnt, int *rlist, CMatrixRoadRoute *rr, bool test) {
    if (rcnt <= 1)
        return;

    int ln = rr->AddList();
    rr->AddUnit(ln, NULL, m_Region[rlist[0]].m_Crotch[0]);

#if (defined _DEBUG) && !(defined _RELDEBUG) && !(defined _DISABLE_AI_HELPERS)
    if (test && !g_TestLocal) {
        CHelper::DestroyByGroup(103);
        CHelper::DestroyByGroup(102);
    }
#endif

    for (int i = 1; i < rcnt; i++) {
#if (defined _DEBUG) && !(defined _RELDEBUG) && !(defined _DISABLE_AI_HELPERS)
        if (test && !g_TestLocal) {
            D3DXVECTOR3 v2;
            v2.x = rr->m_Units[ln * m_CrotchCnt + rr->m_Header[ln].m_Cnt - 1].m_Crotch->m_Center.x * GLOBAL_SCALE_MOVE;
            v2.y = rr->m_Units[ln * m_CrotchCnt + rr->m_Header[ln].m_Cnt - 1].m_Crotch->m_Center.y * GLOBAL_SCALE_MOVE;
            v2.z = g_MatrixMap->GetZ(v2.x, v2.y) + 150.0f;
            CHelper::Create(0, 101)->Sphere(v2, 5.0f, 5, 0xff00ff00);
        }
#endif

        FindPathFromCrotchToRegion(mm, rr->m_Units[ln * m_CrotchCnt + rr->m_Header[ln].m_Cnt - 1].m_Crotch, rlist[i],
                                   rr, test);
        if (rr->m_Header == NULL)
            return;
    }
}

void CMatrixRoadNetwork::ClearPlace() {
    m_PlaceCnt = 0;
    m_PlaceEmpty = -1;
    if (m_Place != NULL) {
        HFree(m_Place, m_Heap);
        m_Place = NULL;
    }
}

void CMatrixRoadNetwork::SetPlaceCnt(int cnt) {
    if (m_PlaceCnt == cnt)
        return;

    m_Place = (SMatrixPlace *)HAllocClearEx(m_Place, cnt * sizeof(SMatrixPlace), m_Heap);
    if (cnt < m_PlaceCnt) {
        if (m_PlaceEmpty >= cnt)
            m_PlaceEmpty = -1;
        else if (m_PlaceEmpty >= 0) {
            int i = m_PlaceEmpty;
            while (m_Place[i].m_EmptyNext >= 0) {
                if (m_Place[i].m_EmptyNext >= cnt)
                    m_Place[i].m_EmptyNext = -1;
                i = m_Place[i].m_EmptyNext;
            }
        }
    }
    else {
        for (int i = m_PlaceCnt; i < cnt; i++) {
            m_Place[i].m_Empty = true;
            m_Place[i].m_EmptyNext = m_PlaceEmpty;
            m_PlaceEmpty = i;
        }
    }
    m_PlaceCnt = cnt;  // m_PlaceCnt;
}

int CMatrixRoadNetwork::AllocPlace() {
    if (m_PlaceEmpty < 0)
        SetPlaceCnt(m_PlaceCnt + 256);
    int no = m_PlaceEmpty;
    m_PlaceEmpty = m_Place[no].m_EmptyNext;

    ZeroMemory(m_Place + no, sizeof(SMatrixPlace));
    m_Place[no].m_Empty = false;
    m_Place[no].m_EmptyNext = -1;
    return no;
}

void CMatrixRoadNetwork::FreePlace(int no) {
    ASSERT(no >= 0 && no < m_PlaceCnt);
    ASSERT(!m_Place[no].m_Empty);

    m_Place[no].m_Empty = true;
    m_Place[no].m_EmptyNext = m_PlaceEmpty;
    m_PlaceEmpty = no;
}

void CMatrixRoadNetwork::ClearRegion() {
    if (m_Region != NULL) {
        HFree(m_Region, m_Heap);
        m_Region = NULL;
    }
    m_RegionCnt = 0;
}

int CMatrixRoadNetwork::AddRegion() {
    m_RegionCnt++;
    m_Region = (SMatrixRegion *)HAllocClearEx(m_Region, m_RegionCnt * sizeof(SMatrixRegion), m_Heap);
    return m_RegionCnt - 1;
}

void CMatrixRoadNetwork::DeleteRegion(int no) {
    ASSERT(no >= 0 && no < m_RegionCnt);

    if (m_RegionCnt - 1 > 0 && no != m_RegionCnt - 1) {
        MoveMemory(m_Region + no, m_Region + no + 1, (m_RegionCnt - (no + 1)) * sizeof(SMatrixRegion));
    }
    m_RegionCnt--;
    if (m_RegionCnt <= 0)
        ClearRegion();
}

void CMatrixRoadNetwork::CalcNearRegion(int no) {
    if (m_RegionCnt <= 2)
        return;

    int *rl = (int *)HAlloc(m_RegionCnt * sizeof(int), m_Heap);
    int rlcnt = 0;
    CMatrixCrotch **cl = (CMatrixCrotch **)HAlloc(m_CrotchCnt * sizeof(CMatrixCrotch *), m_Heap);
    int clcnt = 0;
    int clsme = 0;

    int i, u;
    SMatrixRegion *region;
    CMatrixCrotch *crotch2;
    CMatrixCrotch *crotch = m_CrotchFirst;
    while (crotch) {
        crotch->m_Data = 0x10000000;
        crotch = crotch->m_Next;
    }

    for (i = 0; i < m_RegionCnt; i++) {
        region = GetRegion(i);
        for (u = 0; u < region->m_CrotchCnt; u++) {
            region->m_Crotch[u]->m_Data = i;
        }
    }

    region = GetRegion(no);
    region->m_NearCnt = 0;
    for (i = 0; i < region->m_CrotchCnt; i++) {
        region->m_Crotch[i]->m_Data |= 0x20000000;
        cl[clcnt] = region->m_Crotch[i];
        clcnt++;
    }

    while (clsme < clcnt) {
        crotch = cl[clsme];
        clsme++;

        for (i = 0; i < crotch->m_RoadCnt; i++) {
            crotch2 = crotch->m_Road[i]->GetOtherCrotch(crotch);
            if (crotch2->m_Data & 0x20000000)
                continue;

            if (crotch2->m_Data >= 0x10000000) {  // не кому не принадлежит
                cl[clcnt] = crotch2;
                clcnt++;
            }
            else if (crotch2->m_Data != no) {  // Принодлежит другому региону
                for (u = 0; u < rlcnt; u++)
                    if (rl[u] == crotch2->m_Data)
                        break;
                if (u >= rlcnt) {
                    rl[rlcnt] = crotch2->m_Data;
                    rlcnt++;
                }
            }
            crotch2->m_Data |= 0x20000000;
        }
    }

    // Сортируем по дальности
    for (i = 0; i < rlcnt - 1; i++) {
        for (u = i + 1; u < rlcnt; u++) {
            if (region->m_Center.Dist2(GetRegion(rl[i])->m_Center) >
                region->m_Center.Dist2(GetRegion(rl[u])->m_Center)) {
                int temp = rl[i];
                rl[i] = rl[u];
                rl[u] = temp;
            }
        }
    }

    // Сохраняем
    for (i = 0; i < 16 && i < rlcnt; i++) {
        region->m_Near[i] = rl[i];
        region->m_NearCnt++;
    }

    HFree(cl, m_Heap);
    HFree(rl, m_Heap);
}

void CMatrixRoadNetwork::CalcCenterRegion(int no) {
    SMatrixRegion *region = GetRegion(no);

    region->m_Center.x = 0;
    region->m_Center.y = 0;
    for (int u = 0; u < region->m_CrotchCnt; u++) {
        region->m_Center += m_Zone[region->m_Crotch[u]->m_Zone].m_Center;
    }
    region->m_Center.x /= region->m_CrotchCnt;
    region->m_Center.y /= region->m_CrotchCnt;
}

int CMatrixRoadNetwork::CalcRadiusRegion(int no) {
    SMatrixRegion *region = GetRegion(no);

    int r = 0;
    for (int u = 0; u < region->m_CrotchCnt; u++) {
        r = std::max(r, region->m_Center.Dist2(m_Zone[region->m_Crotch[u]->m_Zone].m_Center));
    }
    return int(sqrt(double(r)));
}

void CMatrixRoadNetwork::CalcRadiusPlaceRegion(int no) {
    SMatrixRegion *region = GetRegion(no);

    region->m_RadiusPlace = 0;

    for (int i = 0; i < region->m_PlaceCnt; i++) {
        SMatrixPlace *place = GetPlace(region->m_Place[i]);
        region->m_RadiusPlace =
                std::max(region->m_RadiusPlace, region->m_Center.Dist2(CPoint(place->m_Pos.x + 2, place->m_Pos.y + 2)));
    }
    region->m_RadiusPlace = int(sqrt(double(region->m_RadiusPlace))) + 2;
}

bool CMatrixRoadNetwork::IsNerestRegion(int r1, int r2) {
    if (r1 < 0 || r2 < 0)
        return false;

    SMatrixRegion *region = GetRegion(r1);

    for (int i = 0; i < region->m_NearCnt; i++) {
        if (region->m_Near[i] == r2)
            return true;
    }
    return false;
}

int CMatrixRoadNetwork::FindNerestRegion(const CPoint &tp) {
    if (m_RegionCnt <= 0)
        return -1;

    int mindist = tp.Dist2(m_Region[0].m_Center);
    int r = 0;

    for (int i = 1; i < m_RegionCnt; i++) {
        int t = tp.Dist2(m_Region[i].m_Center);
        if (t < mindist) {
            mindist = t;
            r = i;
        }
    }
    return r;
}

#ifndef POW2
#define POW2(x) ((x) * (x))
#endif

int CMatrixRoadNetwork::FindNerestRegionByRadius(const CPoint &tp, int curregion) {
    int i, r;

    if (curregion >= 0) {
        // Проверяем текущий регион
        if (POW2(m_Region[curregion].m_RadiusPlace) >= m_Region[curregion].m_Center.Dist2(tp))
            return curregion;

        // Проверяем ближайшие регионы
        for (i = 0; i < m_Region[curregion].m_NearCnt; i++) {
            r = m_Region[curregion].m_Near[i];
            if (POW2(m_Region[r].m_RadiusPlace) >= m_Region[r].m_Center.Dist2(tp))
                return r;
        }
    }

    // Находим ближайший
    int mindist = 1000000000;
    r = -1;
    for (i = 0; i < m_RegionCnt; i++) {
        int t = m_Region[i].m_Center.Dist2(tp);
        if (POW2(m_Region[i].m_RadiusPlace) >= t)
            return i;
        if (t < mindist) {
            mindist = t;
            r = i;
        }
    }

    return r;
}

void CMatrixRoadNetwork::FindPathInRegionInit() {
    SMatrixRegion *r = m_Region;
    for (int i = 0; i < m_RegionCnt; i++, r++) {
        r->m_FPWt = 1;
    }
}

int CMatrixRoadNetwork::FindPathInRegionRun(byte mm, int rstart, int rend, int *path, int maxpath, bool err) {
    if (m_RegionCnt <= 1)
        return 0;
    if (rstart == rend)
        return 0;

    if (!m_RegionFindIndex)
        m_RegionFindIndex = (int *)HAlloc(m_RegionCnt * sizeof(int), m_Heap);

    SMatrixRegion *r2;
    SMatrixRegion *r = m_Region;
    for (int i = 0; i < m_RegionCnt; i++, r++) {
        r->m_FPLevel = -1;
        r->m_FPWtp = 0;
    }

    int sme = 0;
    int cnt = 1;
    int level = 1;
    int next = cnt;
    m_RegionFindIndex[0] = rend;
    m_Region[rend].m_FPLevel = 0;
    m_Region[rend].m_FPWtp = 0;

    bool ok = false;

    while (sme < cnt) {
        r = m_Region + m_RegionFindIndex[sme];

        for (int i = 0; i < r->m_NearCnt; i++) {
            if (r->m_NearMove[i] & mm)
                continue;
            int cr = r->m_Near[i];
            int nr = r->m_Near[i];
            r2 = m_Region + nr;
            int wtp = r->m_FPWtp + r2->m_FPWt;

            if (r2->m_FPLevel < 0 || wtp < r2->m_FPWtp) {
                r2->m_FPLevel = level;
                r2->m_FPWtp = wtp;

                m_RegionFindIndex[cnt] = nr;
                cnt++;

                if (nr == rstart)
                    ok = true;
            }
        }

        sme++;
        if (sme >= next) {
            next = cnt;
            level++;
            if (!ok && level > maxpath) {
                if (err)
                    ERROR_E;
                else
                    return 0;
            }
        }
    }

    if (!ok)
        return 0;

    cnt = 1;
    int cr = rstart;
    r = m_Region + cr;
    int curwt = r->m_FPWtp;
    if (path)
        path[0] = rstart;

    while (true) {
        int nr_ = -1;
        int wtp_ = curwt;
        for (int i = 0; i < r->m_NearCnt; i++) {
            if (r->m_NearMove[i] & mm)
                continue;
            int nr = r->m_Near[i];
            int wtp = m_Region[nr].m_FPWtp;
            if (nr == rend) {
                nr_ = nr;
                wtp_ = wtp;
                break;
            }
            else if (wtp <= wtp_) {
                nr_ = nr;
                wtp_ = wtp;
            }
        }
        if (nr_ < 0) {
            ERROR_E;
        }

        cr = nr_;
        r = m_Region + cr;
        curwt = wtp_;
        if (!(cnt + 1 <= maxpath)) {
            if (err)
                ERROR_E;
            else
                return 0;
        }
        if (path)
            path[cnt] = cr;
        cnt++;
        if (cr == rend)
            break;
    }

    return cnt;
}

void CMatrixRoadNetwork::ClearPL() {
    if (m_PLList != NULL) {
        HFree(m_PLList, m_Heap);
        m_PLList = NULL;
    }
    m_PLSizeX = 0;
    m_PLSizeY = 0;
}

void CMatrixRoadNetwork::InitPL(int mapsizex, int mapsizey) {
    ClearPL();

    m_PLSizeX = (mapsizex >> m_PLShift) + ((mapsizex & m_PLMask) ? 1 : 0);
    m_PLSizeY = (mapsizey >> m_PLShift) + ((mapsizey & m_PLMask) ? 1 : 0);

    if (m_PlaceCnt <= 0 || m_PLSizeX <= 0 || m_PLSizeX <= 0)
        return;

    SMatrixPlace *nap = (SMatrixPlace *)HAlloc(m_PlaceCnt * sizeof(SMatrixPlace), m_Heap);
    int *changei = (int *)HAlloc(m_PlaceCnt * sizeof(int), m_Heap);
    m_PLList = (SMatrixPlaceList *)HAllocClear(m_PLSizeX * m_PLSizeY * sizeof(SMatrixPlaceList), m_Heap);

    int napcnt = 0;

    SMatrixPlaceList *pl = m_PLList;
    for (int y = 0; y < m_PLSizeY; y++) {
        for (int x = 0; x < m_PLSizeX; x++, pl++) {
            pl->m_Sme = napcnt;
            SMatrixPlace *place = m_Place;
            for (int i = 0; i < m_PlaceCnt; i++, place++) {
                if ((place->m_Pos.x >> m_PLShift) == x && (place->m_Pos.y >> m_PLShift) == y) {
                    CopyMemory(nap + napcnt, place, sizeof(SMatrixPlace));
                    nap[napcnt].m_Empty = false;
                    nap[napcnt].m_EmptyNext = -1;
                    changei[i] = napcnt;

                    napcnt++;
                    pl->m_Cnt++;
                }
            }
        }
    }
    ASSERT(napcnt == m_PlaceCnt);

    SMatrixRegion *region = m_Region;
    for (int i = 0; i < m_RegionCnt; i++, region++) {
        for (int u = 0; u < region->m_PlaceCnt; u++) {
            region->m_Place[u] = changei[region->m_Place[u]];
        }
        for (int u = 0; u < region->m_PlaceAllCnt; u++) {
            region->m_PlaceAll[u] = changei[region->m_PlaceAll[u]];
        }
    }

    HFree(m_Place, m_Heap);
    m_Place = nap;
    m_PlaceEmpty = -1;

    for (int i = 0; i < m_ZoneCnt; i++) {
        for (int u = 0; u < m_Zone[i].m_PlaceCnt; u++) {
            m_Zone[i].m_Place[u] = changei[m_Zone[i].m_Place[u]];
        }
    }
    for (int i = 0; i < m_PlaceCnt; i++) {
        for (int u = 0; u < m_Place[i].m_NearCnt; u++) {
            m_Place[i].m_Near[u] = changei[m_Place[i].m_Near[u]];
        }
    }

    HFree(changei, m_Heap);
}

CRect CMatrixRoadNetwork::CorrectRectPL(const CRect &mapcoords) {
    return CRect(std::max(0, mapcoords.left >> m_PLShift), std::max(0, mapcoords.top >> m_PLShift),
                 std::min(m_PLSizeX, (mapcoords.right >> m_PLShift) + 1),
                 std::min(m_PLSizeY, (mapcoords.bottom >> m_PLShift) + 1));
}

void CMatrixRoadNetwork::ActionDataPL(const CRect &mapcoords, dword mask_and, dword mask_or) {
    int sx = std::max(0, mapcoords.left >> m_PLShift);
    int sy = std::max(0, mapcoords.top >> m_PLShift);
    int ex = std::min(m_PLSizeX, (mapcoords.right >> m_PLShift) + 1);
    int ey = std::min(m_PLSizeY, (mapcoords.bottom >> m_PLShift) + 1);

    SMatrixPlaceList *pl = m_PLList + sx + sy * m_PLSizeX;
    for (int y = sy; y < ey; y++, pl += m_PLSizeX - (ex - sx)) {
        for (int x = sx; x < ex; x++, pl++) {
            SMatrixPlace *mp = m_Place + pl->m_Sme;
            for (int i = 0; i < pl->m_Cnt; i++, mp++)
                mp->m_Data = (mp->m_Data & mask_and) | mask_or;
        }
    }
}

int CMatrixRoadNetwork::FindInPL(const CPoint &mappos) {
    CRect plr = CorrectRectPL(CRect(mappos.x - 4, mappos.y - 4, mappos.x + 4, mappos.y + 4));
    SMatrixPlaceList *plist = m_PLList + plr.left + plr.top * m_PLSizeX;
    for (int y = plr.top; y < plr.bottom; y++, plist += m_PLSizeX - (plr.right - plr.left)) {
        for (int x = plr.left; x < plr.right; x++, plist++) {
            SMatrixPlace *place = m_Place + plist->m_Sme;
            for (int u = 0; u < plist->m_Cnt; u++, place++) {
                if (mappos.x >= place->m_Pos.x && mappos.y >= place->m_Pos.y && mappos.x < (place->m_Pos.x + 4) &&
                    mappos.y < (place->m_Pos.y + 4)) {
                    return plist->m_Sme + u;
                }
            }
        }
    }
    return -1;
}

void CMatrixRoadNetwork::Save(CBuf &b) {
    int u;

    b.Add<int>(m_ZoneCnt);
    for (int i = 0; i < m_ZoneCnt; i++) {
        b.Add<CPoint>(m_Zone[i].m_Center);
        b.Add<bool>(m_Zone[i].m_Bottleneck);
        b.Add<int>(m_Zone[i].m_Size);
        b.Add<int>(m_Zone[i].m_Perim);
        b.Add<CRect>(m_Zone[i].m_Rect);
        b.Add<bool>(m_Zone[i].m_Road);
        b.Add<uint8_t>(BYTE(m_Zone[i].m_Move));
        b.Add<uint8_t>(m_Zone[i].m_NearZoneCnt);
        for (u = 0; u < m_Zone[i].m_NearZoneCnt; u++) {
            b.Add<int>(m_Zone[i].m_NearZone[u]);
            b.Add<int>(m_Zone[i].m_NearZoneConnectSize[u]);
            b.Add<uint8_t>(m_Zone[i].m_NearZoneMove[u]);
        }
        b.Add<int>(m_Zone[i].m_Region);
        b.Add<uint8_t>(m_Zone[i].m_PlaceCnt);
        for (u = 0; u < m_Zone[i].m_PlaceCnt; u++)
            b.Add<int>(m_Zone[i].m_Place[u]);
    }

    b.Add<int>(m_CrotchCnt);
    CMatrixCrotch *crotch = m_CrotchFirst;
    int i = 0;
    while (crotch) {
        crotch->m_Data = i;

        b.Add<CPoint>(crotch->m_Center);
        b.Add<int>(crotch->m_Zone);
        b.Add<bool>(crotch->m_Critical);
        b.Add<int>(crotch->m_Island);

        i++;
        crotch = crotch->m_Next;
    }

    b.Add<int>(m_RoadCnt);
    CMatrixRoad *road = m_RoadFirst;
    while (road) {
        b.Add<int>(road->m_Start->m_Data);
        b.Add<int>(road->m_End->m_Data);

        b.Add<int>(road->m_ZoneCnt);
        for (u = 0; u < road->m_ZoneCnt; u++) {
            b.Add<int>(road->m_Zone[u]);
        }

        b.Add<int>(road->m_Dist);
        b.Add<uint8_t>(road->m_Move);

        road = road->m_Next;
    }

    b.Add<int>(m_PlaceCnt);
    for (i = 0; i < m_PlaceCnt; i++) {
        SMatrixPlace *place = GetPlace(i);

        b.Add<CPoint>(place->m_Pos);
        b.Add<uint8_t>(BYTE(place->m_Move));
        b.Add<uint8_t>(place->m_BorderLeft);
        b.Add<uint8_t>(place->m_BorderTop);
        b.Add<uint8_t>(place->m_BorderRight);
        b.Add<uint8_t>(place->m_BorderBottom);
        b.Add<uint8_t>(place->m_BorderCnt);
        b.Add<uint8_t>(place->m_EdgeLeft);
        b.Add<uint8_t>(place->m_EdgeTop);
        b.Add<uint8_t>(place->m_EdgeRight);
        b.Add<uint8_t>(place->m_EdgeBottom);
        b.Add<uint8_t>(place->m_EdgeCnt);
        for (u = 0; u < 9; u++)
            b.Add<bool>(place->m_Blockade[u]);
        b.Add<uint8_t>(place->m_Cannon);
        b.Add<uint8_t>(place->m_NearCnt);
        for (u = 0; u < place->m_NearCnt; u++) {
            b.Add<int>(place->m_Near[u]);
            b.Add<uint8_t>(place->m_NearMove[u]);
        }
    }

    b.Add<int>(m_RegionCnt);
    for (i = 0; i < m_RegionCnt; i++) {
        SMatrixRegion *region = GetRegion(i);

        b.Add<CPoint>(region->m_Center);

        b.Add<int>(region->m_RadiusPlace);

        b.Add<uint8_t>(region->m_CrotchCnt);
        for (u = 0; u < region->m_CrotchCnt; u++)
            b.Add<int>(region->m_Crotch[u]->m_Data);

        b.Add<uint8_t>(region->m_PlaceCnt);
        for (u = 0; u < region->m_PlaceCnt; u++)
            b.Add<int>(region->m_Place[u]);

        b.Add<uint8_t>(region->m_PlaceAllCnt);
        for (u = 0; u < region->m_PlaceAllCnt; u++)
            b.Add<int>(region->m_PlaceAll[u]);

        b.Add<uint8_t>(region->m_NearCnt);
        for (u = 0; u < region->m_NearCnt; u++) {
            b.Add<int>(region->m_Near[u]);
            b.Add<uint8_t>(region->m_NearMove[u]);
        }

        b.Add<uint32_t>(region->m_Color);
    }
}

void CMatrixRoadNetwork::Load(CBuf &b, int ver) {
    Clear();

    int u;

    int cnt = b.Get<int>();
    SetZoneCntMax(cnt);
    m_ZoneCnt = cnt;
    for (int i = 0; i < cnt; i++) {
        m_Zone[i].m_Center = b.Get<CPoint>();
        m_Zone[i].m_Bottleneck = b.Get<bool>();
        m_Zone[i].m_Size = b.Get<int>();
        m_Zone[i].m_Perim = b.Get<int>();
        m_Zone[i].m_Rect = b.Get<CRect>();
        m_Zone[i].m_Road = b.Get<bool>();
        m_Zone[i].m_Move = b.Get<uint8_t>();
        m_Zone[i].m_NearZoneCnt = b.Get<uint8_t>();
        for (u = 0; u < m_Zone[i].m_NearZoneCnt; u++) {
            m_Zone[i].m_NearZone[u] = b.Get<int>();
            m_Zone[i].m_NearZoneConnectSize[u] = b.Get<int>();
            if (ver >= 25)
                m_Zone[i].m_NearZoneMove[u] = b.Get<uint8_t>();
        }
        if (ver >= 24) {
            m_Zone[i].m_Region = b.Get<int>();
        }
        if (ver >= 27) {
            m_Zone[i].m_PlaceCnt = b.Get<uint8_t>();
            for (u = 0; u < m_Zone[i].m_PlaceCnt; u++)
                m_Zone[i].m_Place[u] = b.Get<int>();
        }
        m_Zone[i].m_Access = false;
    }

    cnt = b.Get<int>();
    CMatrixCrotch **indx = (CMatrixCrotch **)HAlloc(sizeof(CMatrixCrotch *) * cnt, m_Heap);
    for (int i = 0; i < cnt; i++) {
        CMatrixCrotch *crotch = AddCrotch();
        indx[i] = crotch;

        crotch->m_Center = b.Get<CPoint>();
        crotch->m_Zone = b.Get<int>();
        crotch->m_Critical = b.Get<bool>();
        crotch->m_Island = b.Get<int>();
        crotch->m_Region = m_Zone[crotch->m_Zone].m_Region;
    }

    cnt = b.Get<int>();
    for (int i = 0; i < cnt; i++) {
        CMatrixRoad *road = AddRoad();

        road->m_Start = indx[b.Get<int>()];
        road->m_End = indx[b.Get<int>()];

        int ucnt = b.Get<int>();
        road->LoadZone(ucnt, (int *)((byte *)b.Get() + b.Pointer()));
        b.Pointer(b.Pointer() + ucnt * sizeof(int));

        road->m_Dist = b.Get<int>();
        road->m_Move = b.Get<uint8_t>();

        road->m_Start->AddRoad(road);
        road->m_End->AddRoad(road);
    }

    if (ver >= 21) {
        cnt = b.Get<int>();
        SetPlaceCnt(cnt);
        m_PlaceEmpty = -1;

        for (int i = 0; i < cnt; i++) {
            SMatrixPlace *place = GetPlace(i);
            place->m_Empty = false;
            place->m_EmptyNext = -1;

            place->m_Pos = b.Get<CPoint>();
            place->m_Move = b.Get<uint8_t>();
            place->m_BorderLeft = b.Get<uint8_t>();
            place->m_BorderTop = b.Get<uint8_t>();
            place->m_BorderRight = b.Get<uint8_t>();
            place->m_BorderBottom = b.Get<uint8_t>();
            place->m_BorderCnt = b.Get<uint8_t>();
            place->m_EdgeLeft = b.Get<uint8_t>();
            place->m_EdgeTop = b.Get<uint8_t>();
            place->m_EdgeRight = b.Get<uint8_t>();
            place->m_EdgeBottom = b.Get<uint8_t>();
            place->m_EdgeCnt = b.Get<uint8_t>();
            for (u = 0; u < 9; u++)
                place->m_Blockade[u] = b.Get<bool>();
            if (ver >= 24)
                place->m_Cannon = b.Get<uint8_t>();
            else
                place->m_Cannon = 0;
            if (place->m_Cannon) {
                place->m_Move = 1 + 2 + 4 + 8 + 16;
            }

            if (ver >= 27) {
                place->m_NearCnt = b.Get<uint8_t>();
                for (u = 0; u < place->m_NearCnt; u++) {
                    place->m_Near[u] = b.Get<int>();
                    place->m_NearMove[u] = b.Get<uint8_t>();
                }
            }

            place->m_Region = -1;
        }

        cnt = b.Get<int>();
        m_RegionCnt = cnt;
        m_Region = (SMatrixRegion *)HAllocClearEx(m_Region, m_RegionCnt * sizeof(SMatrixRegion), m_Heap);

        for (int i = 0; i < cnt; i++) {
            SMatrixRegion *region = GetRegion(i);

            region->m_Center = b.Get<CPoint>();

            if (ver >= 24)
                region->m_RadiusPlace = b.Get<int>();

            region->m_CrotchCnt = b.Get<uint8_t>();
            for (u = 0; u < region->m_CrotchCnt; u++) {
                region->m_Crotch[u] = indx[b.Get<int>()];
                //                region->m_Crotch[u]->m_Region=i;
            }

            region->m_PlaceCnt = b.Get<uint8_t>();
            for (u = 0; u < region->m_PlaceCnt; u++) {
                region->m_Place[u] = b.Get<int>();
                GetPlace(region->m_Place[u])->m_Region = i;
            }

            if (ver >= 24) {
                region->m_PlaceAllCnt = b.Get<uint8_t>();
                for (u = 0; u < region->m_PlaceAllCnt; u++)
                    region->m_PlaceAll[u] = b.Get<int>();
            }

            region->m_NearCnt = b.Get<uint8_t>();
            for (u = 0; u < region->m_NearCnt; u++) {
                region->m_Near[u] = b.Get<int>();
                if (ver >= 26)
                    region->m_NearMove[u] = b.Get<uint8_t>();
            }

            if (ver >= 24) {
                region->m_Color = b.Get<DWORD>();
            }
        }
    }

    HFree(indx, m_Heap);
}
