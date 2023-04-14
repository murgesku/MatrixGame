// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixMap.hpp"

#include <algorithm>
#include <vector>

struct STempPoints {
    DWORD invisible;     // 0 - visible // 1 - invisible
    DWORD invisibleNow;  // 0 - visible // 1 - invisible
    D3DXVECTOR3 p;
};
struct STempCalcs {
    SPlane plane1;
    SPlane plane2;
    D3DXVECTOR3 p[4];
};

struct SPotEdge {
    D3DXVECTOR3 p1, p2;
    SPlane plane1;
    SPlane plane2;
};

struct SVisGroup {
    int x, y;  // group coord
    DWORD id;

    LPDWORD invisible[(MAP_GROUP_SIZE + 1) * (MAP_GROUP_SIZE + 1)];
    int invisibleCount;

    D3DXVECTOR3 sourcePoints[(MAP_GROUP_SIZE + 1) * 4];
    int sourcePointsCount;

    float minz;

    D3DXVECTOR2 myPosition;

    void BuildPoints(STempPoints *points) {
        minz = 0;
        id = x + y * 65536;
        invisibleCount = 0;
        CMatrixMapGroup *group = g_MatrixMap->GetGroupByIndex(x, y);
        int xp = (x * MAP_GROUP_SIZE);
        int yp = (y * MAP_GROUP_SIZE);

        myPosition = D3DXVECTOR2(float(xp * GLOBAL_SCALE), float(yp * GLOBAL_SCALE));

        if (group != NULL) {
            minz = 100000;
            // build dest pts

            int width = std::min(MAP_GROUP_SIZE, (g_MatrixMap->m_Size.x - xp));
            int heigth = std::min(MAP_GROUP_SIZE, (g_MatrixMap->m_Size.y - yp));

            ASSERT(width != 0 && heigth != 0);

            for (int yy = 0; yy <= heigth; ++yy) {
                for (int xx = 0; xx <= width; ++xx) {
                    int ccx = xx + xp;
                    int ccy = yy + yp;
                    STempPoints *zzp = points + ccx + ccy * (g_MatrixMap->m_Size.x + 1);
                    invisible[invisibleCount++] = &zzp->invisible;

                    SMatrixMapPoint *mapPoint = g_MatrixMap->PointGet(xx + xp, yy + yp);
                    float zz = mapPoint->z;
                    if (zz < 0)
                        zz = 0;
                    if (minz > zz)
                        minz = zz;
                }
            }
        }

        sourcePointsCount = 0;

        SMatrixMapPoint *mapPoint;
        for (int yy = 0; yy <= MAP_GROUP_SIZE; ++yy) {
            // left
            sourcePoints[sourcePointsCount].x = xp * GLOBAL_SCALE;
            sourcePoints[sourcePointsCount].y = (yy + yp) * GLOBAL_SCALE;

            mapPoint = g_MatrixMap->PointGetTest(xp, yy + yp);
            if (mapPoint) {
                sourcePoints[sourcePointsCount].z = mapPoint->z >= 0 ? mapPoint->z : 0;
            }
            else
                sourcePoints[sourcePointsCount].z = 0;
            ++sourcePointsCount;

            // right
            sourcePoints[sourcePointsCount].x = (xp + MAP_GROUP_SIZE) * GLOBAL_SCALE;
            sourcePoints[sourcePointsCount].y = (yy + yp) * GLOBAL_SCALE;

            mapPoint = g_MatrixMap->PointGetTest((xp + MAP_GROUP_SIZE), yy + yp);
            if (mapPoint) {
                sourcePoints[sourcePointsCount].z = mapPoint->z >= 0 ? mapPoint->z : 0;
            }
            else
                sourcePoints[sourcePointsCount].z = 0;
            ++sourcePointsCount;
        }
        for (int xx = 1; xx < MAP_GROUP_SIZE; ++xx) {
            // up
            sourcePoints[sourcePointsCount].x = (xx + xp) * GLOBAL_SCALE;
            sourcePoints[sourcePointsCount].y = yp * GLOBAL_SCALE;
            mapPoint = g_MatrixMap->PointGetTest((xx + xp), yp);
            if (mapPoint) {
                sourcePoints[sourcePointsCount].z = mapPoint->z >= 0 ? mapPoint->z : 0;
            }
            else
                sourcePoints[sourcePointsCount].z = 0;
            ++sourcePointsCount;

            // down
            sourcePoints[sourcePointsCount].x = (xx + xp) * GLOBAL_SCALE;
            sourcePoints[sourcePointsCount].y = (yp + MAP_GROUP_SIZE) * GLOBAL_SCALE;
            mapPoint = g_MatrixMap->PointGetTest((xx + xp), (yp + MAP_GROUP_SIZE));
            if (mapPoint) {
                sourcePoints[sourcePointsCount].z = mapPoint->z >= 0 ? mapPoint->z : 0;
            }
            else
                sourcePoints[sourcePointsCount].z = 0;
            ++sourcePointsCount;
        }
    }

    void BuildShadowFor(const D3DXVECTOR3 &pointsFrom, int pointsCount, STempPoints *points, const std::vector<SPotEdge>& pe)
    {
        for (int i = 0; i < pointsCount; ++i)
            points[i].invisibleNow = 0;

        SPlane vPlane;

        for (auto& edge : pe)
        {
            bool os1 = edge.plane1.IsOnSide(pointsFrom);
            bool os2 = edge.plane2.IsOnSide(pointsFrom);
            if (!(os1 ^ os2))
                continue;

#ifdef _DEBUG
            CHelper::Create(10000, 888)->Cone(edge.p1, edge.p2, 2, 2, 0xFF00FF00, 0xFF00FF00, 3);
#endif

            D3DXVECTOR2 p20(edge.p1.x, edge.p1.y);
            D3DXVECTOR2 p21(edge.p2.x, edge.p2.y);
            bool ppcam = PointLineCatch(p20, p21, D3DXVECTOR2(pointsFrom.x, pointsFrom.y));

            if (ppcam) {
                SPlane::BuildFromPoints(vPlane, pointsFrom, edge.p1, edge.p2);
            }
            else {
                SPlane::BuildFromPoints(vPlane, pointsFrom, edge.p2, edge.p1);
            }

            STempPoints *tempPoints = points;

            for (int i = 0; i < pointsCount; ++i, ++tempPoints) {
                if (!tempPoints->invisible)
                    continue;  // не нужно проверять видимые части
                if (tempPoints->invisibleNow)
                    continue;  // уже пометили как невидимую

                bool vv = PointLineCatch(p20, p21, D3DXVECTOR2(tempPoints->p.x, tempPoints->p.y));
                if (vv == ppcam)
                    continue;

                if (!(((!ppcam) ^ PointLineCatch(D3DXVECTOR2(pointsFrom.x, pointsFrom.y), p20,
                                                 D3DXVECTOR2(tempPoints->p.x, tempPoints->p.y))) &&
                      ((!ppcam) ^ PointLineCatch(p21, D3DXVECTOR2(pointsFrom.x, pointsFrom.y),
                                                 D3DXVECTOR2(tempPoints->p.x, tempPoints->p.y)))))
                    continue;

                if (!vPlane.IsOnSide(tempPoints->p)) {
                    tempPoints->invisibleNow = 1;
                }
            }
        }

        STempPoints *tempPoints = points;

        // реально не видно, бля
        for (int i = 0; i < pointsCount; ++i, ++tempPoints) {
            tempPoints->invisible = tempPoints->invisible & tempPoints->invisibleNow;
        }
    }

    void CalcVis(SVisGroup *visibilityGroup, int groupCount, int pointsCount, STempPoints *points, STempCalcs *calcs,
                 const std::vector<SPotEdge>& pe)
    {
        float z0 = minz + GLOBAL_SCALE + 1;

        // TEMP:
        z0 = g_MatrixMap->m_Camera.GetFrustumCenter().z;

        // building shadow...
        // set full shadow
        for (int i = 0; i < pointsCount; ++i)
            points[i].invisible = 1;

        for (int i = 0; i < sourcePointsCount; ++i) {
            if (z0 >= sourcePoints[i].z) {
                BuildShadowFor(D3DXVECTOR3(sourcePoints[i].x, sourcePoints[i].y, z0), pointsCount, points, pe);
            }
        }

#ifdef _DEBUG
        for (int i = 0; i < pointsCount; ++i) {
            if (points[i].invisible) {
                CHelper::Create(1000, 888)->Sphere(points[i].p, 2, 3, 0xFF00FF00);
            }
        }
#endif
    }
};

void CMatrixMap::CalcVis(void) {
    DTRACE();

#ifdef _DEBUG
    CHelper::DestroyByGroup(888);
#endif

    std::vector<SPotEdge> pe;

    int pointsCount = (g_MatrixMap->m_Size.y + 1) * (g_MatrixMap->m_Size.x + 1);

    STempPoints *aps = (STempPoints *)HAlloc(pointsCount * sizeof(STempPoints), g_MatrixHeap);
    STempCalcs *bla =
            (STempCalcs *)HAlloc(g_MatrixMap->m_Size.y * g_MatrixMap->m_Size.x * sizeof(STempCalcs), g_MatrixHeap);

    STempCalcs *calcs = bla;
    STempPoints *points = aps;

    for (int j = 0; j <= g_MatrixMap->m_Size.y; ++j) {
        for (int i = 0; i <= g_MatrixMap->m_Size.x; ++i, ++points) {
            points->p.x = i * GLOBAL_SCALE;
            points->p.y = j * GLOBAL_SCALE;
            points->p.z = g_MatrixMap->PointGet(i, j)->z_land;
        }
    }

    for (int j = 0; j < g_MatrixMap->m_Size.y; ++j) {
        for (int i = 0; i < g_MatrixMap->m_Size.x; ++i, ++calcs) {
            SMatrixMapUnit *unit = g_MatrixMap->UnitGet(i, j);
            if (!unit->IsLand())
                continue;
            SMatrixMapPoint *point = g_MatrixMap->PointGet(i, j);

            D3DXVECTOR3 p[4];

            p[0].x = i * GLOBAL_SCALE;
            p[0].y = (j + 1) * GLOBAL_SCALE;
            p[0].z = (point + g_MatrixMap->m_Size.x + 1)->z_land;
            if (p[0].z < 0)
                p[0].z = 0;

            p[1].x = i * GLOBAL_SCALE;
            p[1].y = j * GLOBAL_SCALE;
            p[1].z = point->z_land;
            if (p[1].z < 0)
                p[1].z = 0;

            p[2].x = (i + 1) * GLOBAL_SCALE;
            p[2].y = (j + 1) * GLOBAL_SCALE;
            p[2].z = (point + g_MatrixMap->m_Size.x + 2)->z_land;
            if (p[2].z < 0)
                p[2].z = 0;

            p[3].x = (i + 1) * GLOBAL_SCALE;
            p[3].y = j * GLOBAL_SCALE;
            p[3].z = (point + 1)->z_land;
            if (p[3].z < 0)
                p[3].z = 0;

            memcpy(calcs->p, p, sizeof(p));

            SPlane::BuildFromPoints(calcs->plane1, p[0], p[1], p[2]);  // 012
            SPlane::BuildFromPoints(calcs->plane2, p[1], p[3], p[2]);  // 132
        }
    }

    // calc potential edges
    {
        calcs = bla;

        SPotEdge edge;

        for (int j = 0; j < g_MatrixMap->m_Size.y; ++j) {
            for (int i = 0; i < g_MatrixMap->m_Size.x; ++i, ++calcs) {
                if (!calcs->plane1.IsOnSide(calcs->p[3])) {
                    edge.p1 = calcs->p[1];
                    edge.p2 = calcs->p[2];
                    edge.plane1 = calcs->plane1;
                    edge.plane2 = calcs->plane2;

                    if (edge.plane1.norm != edge.plane2.norm) {
                        pe.push_back(edge);
                    }
                }

                if (j < (g_MatrixMap->m_Size.y - 1)) {
                    if (!calcs->plane1.IsOnSide((calcs + g_MatrixMap->m_Size.x)->p[2])) {
                        edge.p1 = calcs->p[0];
                        edge.p2 = calcs->p[2];

                        edge.plane1 = calcs->plane1;
                        edge.plane2 = (calcs + g_MatrixMap->m_Size.x)->plane2;

                        if (edge.plane1.norm != edge.plane2.norm) {
                            pe.push_back(edge);
                        }
                    }
                }

                if (i < (g_MatrixMap->m_Size.x - 1)) {
                    if (!calcs->plane2.IsOnSide((calcs + 1)->p[2])) {
                        edge.p1 = calcs->p[2];
                        edge.p2 = calcs->p[3];

                        edge.plane1 = calcs->plane2;
                        edge.plane2 = (calcs + 1)->plane1;

                        if (edge.plane1.norm != edge.plane2.norm) {
                            pe.push_back(edge);
                        }
                    }
                }
            }
        }
    }

    int groupCount = m_GroupSize.x * m_GroupSize.y;
    SVisGroup *visibilityGroup = (SVisGroup *)HAllocClear(sizeof(SVisGroup) * groupCount, g_MatrixHeap);

    ClearGroupVis();
    m_GroupVis = (SGroupVisibility *)HAllocClear(sizeof(SGroupVisibility) * groupCount, g_MatrixHeap);

    for (int i = 0; i < m_GroupSize.x; ++i) {
        for (int j = 0; j < m_GroupSize.y; ++j) {
            SVisGroup *cvg = visibilityGroup + i + j * m_GroupSize.x;
            cvg->x = i;
            cvg->y = j;
            cvg->BuildPoints(aps);
        }
    }

    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));

    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    SetColorOpSelect(0, D3DTA_TFACTOR);
    SetAlphaOpDisable(0);
    SetColorOpDisable(1);

    g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFF003000);

    SVert_V4 v[4];
    v[0].p = D3DXVECTOR4(0, float(g_ScreenY), 0, 1);
    v[1].p = D3DXVECTOR4(0, 0, 0, 1);
    v[2].p = D3DXVECTOR4(0, float(g_ScreenY), 0, 1);
    v[3].p = D3DXVECTOR4(0, 0, 0, 1);

    // TEMP:
    int gx = TruncFloat(g_MatrixMap->m_Camera.GetFrustumCenter().x / MAP_GROUP_SIZE / GLOBAL_SCALE);
    int gy = TruncFloat(g_MatrixMap->m_Camera.GetFrustumCenter().y / MAP_GROUP_SIZE / GLOBAL_SCALE);
    visibilityGroup[gx + gy * g_MatrixMap->m_GroupSize.x].CalcVis(visibilityGroup, groupCount, pointsCount, aps, bla, pe);

    HFree(visibilityGroup, g_MatrixHeap);

    CStorage stor(g_MatrixHeap);
    stor.Load(MapName().c_str());

    stor.DelRecord(DATA_GROUPS_VIS);

    CStorageRecord storageRecord(std::wstring{DATA_GROUPS_VIS}, g_MatrixHeap);
    storageRecord.AddItem(CStorageRecordItem(std::wstring{DATA_GROUPS_VIS_LEVELS}, ST_INT32));
    storageRecord.AddItem(CStorageRecordItem(std::wstring{DATA_GROUPS_VIS_GROUPS}, ST_INT32));
    storageRecord.AddItem(CStorageRecordItem(std::wstring{DATA_GROUPS_VIS_ZFROM}, ST_FLOAT));
    stor.AddRecord(storageRecord);

    CDataBuf *levelsDataBuffer = stor.GetBuf(DATA_GROUPS_VIS, DATA_GROUPS_VIS_LEVELS, ST_INT32);
    CDataBuf *groupsDataBuffer = stor.GetBuf(DATA_GROUPS_VIS, DATA_GROUPS_VIS_GROUPS, ST_INT32);
    CDataBuf *zfromDataBuffer = stor.GetBuf(DATA_GROUPS_VIS, DATA_GROUPS_VIS_ZFROM, ST_FLOAT);

    zfromDataBuffer->AddArray();

    for (int i = 0; i < groupCount; ++i) {
        int levelsArray = levelsDataBuffer->AddArray();
        int groupsArray = groupsDataBuffer->AddArray();

        SGroupVisibility *visibilityGroup = m_GroupVis + i;

        zfromDataBuffer->AddToArray<float>(0, visibilityGroup->z_from);

        levelsDataBuffer->AddToArray<int>(levelsArray, visibilityGroup->levels, visibilityGroup->levels_cnt);
        for (int vIndex = 0; vIndex < visibilityGroup->vis_cnt; ++vIndex) {
            for (int gIndex = 0; gIndex < groupCount; ++gIndex) {
                if (m_Group[gIndex] == visibilityGroup->vis[vIndex]) {
                    groupsDataBuffer->AddToArray<int>(groupsArray, gIndex);
                    break;
                }
            }
        }
    }

    HFree(bla, g_MatrixHeap);
    HFree(aps, g_MatrixHeap);
}

// runtime

#define NPOS 4

struct CMatrixMap::SCalcVisRuntime {
    D3DXVECTOR2 pos[4];
    PCMatrixMapGroup *mapGroup;
    int i, j;
};

void CMatrixMap::CheckCandidate(SCalcVisRuntime &visRuntime, CMatrixMapGroup *mapGroup) {
    int i0 = NPOS - 1;
    int i1 = 0;

    if (mapGroup->IsPointIn(visRuntime.pos[0]))
        goto visible;
    if (mapGroup->IsPointIn(visRuntime.pos[1]))
        goto visible;
    if (mapGroup->IsPointIn(visRuntime.pos[2]))
        goto visible;
#if NPOS == 4
    if (mapGroup->IsPointIn(visRuntime.pos[3]))
        goto visible;
#endif

    while (i1 < NPOS) {
        if (PointLineCatch(visRuntime.pos[i0], visRuntime.pos[i1], mapGroup->GetPos0())) {
            goto checknext;
        }
        if (PointLineCatch(visRuntime.pos[i0], visRuntime.pos[i1], mapGroup->GetPos1())) {
            goto checknext;
        }
        if (PointLineCatch(visRuntime.pos[i0], visRuntime.pos[i1],
                           D3DXVECTOR2(mapGroup->GetPos0().x, mapGroup->GetPos1().y))) {
            goto checknext;
        }
        if (PointLineCatch(visRuntime.pos[i0], visRuntime.pos[i1],
                           D3DXVECTOR2(mapGroup->GetPos1().x, mapGroup->GetPos0().y))) {
            goto checknext;
        }
        goto invisible;
    checknext:
        i0 = i1++;
    }

    // last check : frustum
    if (mapGroup->IsInFrustum()) {
    visible:
        // cmg->SetVisible(true);
        (*visRuntime.mapGroup) = mapGroup;
        ++m_VisibleGroupsCount;
        ++visRuntime.mapGroup;
    }
    else {
    invisible:
        // cmg->SetVisible(false);
        // so, map group is invisible.
        // if it is an edge of whole map, it's water can be visible
        if ((visRuntime.i == (m_GroupSize.x - 1)) || (visRuntime.j == (m_GroupSize.y - 1))) {
            D3DXVECTOR3 mins(mapGroup->GetPos0().x, mapGroup->GetPos0().y, WATER_LEVEL);
            D3DXVECTOR3 maxs(mins.x + float(MAP_GROUP_SIZE * GLOBAL_SCALE),
                             mins.y + float(MAP_GROUP_SIZE * GLOBAL_SCALE), WATER_LEVEL);
            if (m_Camera.IsInFrustum(mins, maxs)) {
                m_VisWater.push_back(mapGroup->GetPos0());
            }
        }
    }
}

// visibility
void CMatrixMap::CalcMapGroupVisibility(void) {
    DTRACE();

    SCalcVisRuntime visRuntime;

    D3DXVECTOR3 topForward(m_Camera.GetFrustumLT() + m_Camera.GetFrustumRT());
    float cameraDirection2d = D3DXVec2Length((D3DXVECTOR2 *)&topForward);
    float k_cam = (float)fabs(topForward.z) / cameraDirection2d;
    float k_etalon = m_Camera.GetFrustumCenter().z * INVERT(MAX_VIEW_DISTANCE);
    float k_etalon_fog = m_Camera.GetFrustumCenter().z * INVERT(MAX_VIEW_DISTANCE * FOG_NEAR_K);
    // float dist_naklon = (float)sqrt(dist + m_Camera.GetFrustumCenter().z * m_Camera.GetFrustumCenter().z);

    if (topForward.z > 0 || k_cam < k_etalon_fog) {
        SETFLAG(m_Flags, MMFLAG_FOG_ON);
    }
    else {
        RESETFLAG(m_Flags, MMFLAG_FOG_ON);
    }

    if (k_cam < k_etalon || topForward.z > 0) {
        // float d2 = dist * 2;
        float k;
        // right top <- left top
        if (m_Camera.GetFrustumLT().z >= (-0.001)) {
            k = MAX_VIEW_DISTANCE;
        }
        else {
            k = (m_minz - m_Camera.GetFrustumCenter().z) / m_Camera.GetFrustumLT().z;
            if (k > MAX_VIEW_DISTANCE)
                k = MAX_VIEW_DISTANCE;
        }
        visRuntime.pos[0].x = m_Camera.GetFrustumCenter().x + m_Camera.GetFrustumLT().x * k;
        visRuntime.pos[0].y = m_Camera.GetFrustumCenter().y + m_Camera.GetFrustumLT().y * k;

        if (m_Camera.GetFrustumRT().z >= (-0.001)) {
            k = MAX_VIEW_DISTANCE;
        }
        else {
            k = (m_minz - m_Camera.GetFrustumCenter().z) / m_Camera.GetFrustumRT().z;
            if (k > MAX_VIEW_DISTANCE)
                k = MAX_VIEW_DISTANCE;
        }
        visRuntime.pos[1].x = m_Camera.GetFrustumCenter().x + m_Camera.GetFrustumRT().x * k;
        visRuntime.pos[1].y = m_Camera.GetFrustumCenter().y + m_Camera.GetFrustumRT().y * k;

        D3DXVECTOR2 ex;
        auto tmp = visRuntime.pos[1] - visRuntime.pos[0];
        D3DXVec2Normalize(&ex, &tmp);
        ex *= GLOBAL_SCALE * MAP_GROUP_SIZE;
        visRuntime.pos[0] -= ex;
        visRuntime.pos[1] += ex;

        k = (m_minz - m_Camera.GetFrustumCenter().z) / m_Camera.GetFrustumRB().z;
        visRuntime.pos[2].x = m_Camera.GetFrustumCenter().x + m_Camera.GetFrustumRB().x * k;
        visRuntime.pos[2].y = m_Camera.GetFrustumCenter().y + m_Camera.GetFrustumRB().y * k;

        k = (m_minz - m_Camera.GetFrustumCenter().z) / m_Camera.GetFrustumLB().z;
        visRuntime.pos[3].x = m_Camera.GetFrustumCenter().x + m_Camera.GetFrustumLB().x * k;
        visRuntime.pos[3].y = m_Camera.GetFrustumCenter().y + m_Camera.GetFrustumLB().y * k;

        D3DXVECTOR2 disp(*(D3DXVECTOR2 *)&m_Camera.GetFrustumCenter() - (visRuntime.pos[2] + visRuntime.pos[3]) * 0.5f);
        visRuntime.pos[2] += disp;
        visRuntime.pos[3] += disp;

        // enable fog
        SETFLAG(m_Flags, MMFLAG_SKY_ON);

        {
            D3DXVECTOR3 projection(m_Camera.GetDir());
            projection.z = 0;
            float len = D3DXVec3Length(&projection);
            if (len < 0.0001) {
                m_SkyHeight = -100;
            }
            else {
                projection *= (1.0f / len);
                projection = m_Camera.GetFrustumCenter() + projection * MAX_VIEW_DISTANCE;
                projection.z -= m_Camera.GetFrustumCenter().z * 1.5f;

                D3DXVECTOR2 ptr = m_Camera.Project(projection, GetIdentityMatrix());
                m_SkyHeight = ptr.y;
            }
        }
    }
    else {
        RESETFLAG(m_Flags, MMFLAG_SKY_ON);

        float k;

        // right top <- left top
        k = (m_minz - m_Camera.GetFrustumCenter().z) / m_Camera.GetFrustumLT().z;
        visRuntime.pos[0].x = m_Camera.GetFrustumCenter().x + m_Camera.GetFrustumLT().x * k;
        visRuntime.pos[0].y = m_Camera.GetFrustumCenter().y + m_Camera.GetFrustumLT().y * k;

        k = (m_minz - m_Camera.GetFrustumCenter().z) / m_Camera.GetFrustumRT().z;
        visRuntime.pos[1].x = m_Camera.GetFrustumCenter().x + m_Camera.GetFrustumRT().x * k;
        visRuntime.pos[1].y = m_Camera.GetFrustumCenter().y + m_Camera.GetFrustumRT().y * k;

        k = (m_minz - m_Camera.GetFrustumCenter().z) / m_Camera.GetFrustumRB().z;
        visRuntime.pos[2].x = m_Camera.GetFrustumCenter().x + m_Camera.GetFrustumRB().x * k;
        visRuntime.pos[2].y = m_Camera.GetFrustumCenter().y + m_Camera.GetFrustumRB().y * k;

        k = (m_minz - m_Camera.GetFrustumCenter().z) / m_Camera.GetFrustumLB().z;
        visRuntime.pos[3].x = m_Camera.GetFrustumCenter().x + m_Camera.GetFrustumLB().x * k;
        visRuntime.pos[3].y = m_Camera.GetFrustumCenter().y + m_Camera.GetFrustumLB().y * k;

        if (m_Camera.GetFrustPlaneB().norm.z > 0) {
            D3DXVECTOR2 disp(*(D3DXVECTOR2 *)&m_Camera.GetFrustumCenter() -
                             (visRuntime.pos[2] + visRuntime.pos[3]) * 0.5f);
            visRuntime.pos[2] += disp;
            visRuntime.pos[3] += disp;
        }
    }

    bool no_info_about_visibility = true;

    visRuntime.mapGroup = m_VisibleGroups;
    m_VisibleGroupsCount = 0;

    if (m_GroupVis != NULL) {
        const D3DXVECTOR3 &cameraPoint = m_Camera.GetFrustumCenter();
        int gx = TruncDouble(cameraPoint.x * (1.0 / (GLOBAL_SCALE * MAP_GROUP_SIZE)));
        int gy = TruncDouble(cameraPoint.y * (1.0 / (GLOBAL_SCALE * MAP_GROUP_SIZE)));

        if (gx >= 0 && gx < m_GroupSize.x && gy >= 0 && gy < m_GroupSize.y) {
            SGroupVisibility *visibilityGroup = m_GroupVis + gx + gy * m_GroupSize.x;

            int level = TruncFloat((cameraPoint.z - visibilityGroup->z_from) / GLOBAL_SCALE);

            if (level >= 0 && level < visibilityGroup->levels_cnt) {
                int n = visibilityGroup->levels[level];

                for (int i = 0; i < n; ++i) {
                    PCMatrixMapGroup g = visibilityGroup->vis[i];

                    visRuntime.i = Float2Int(g->GetPos0().x) / int(MAP_GROUP_SIZE * GLOBAL_SCALE);
                    visRuntime.j = Float2Int(g->GetPos0().y) / int(MAP_GROUP_SIZE * GLOBAL_SCALE);

                    CheckCandidate(visRuntime, g);
                }

                no_info_about_visibility = false;
            }
        }
    }

    float minx = visRuntime.pos[0].x;
    float maxx = visRuntime.pos[0].x;
    float miny = visRuntime.pos[0].y;
    float maxy = visRuntime.pos[0].y;
    for (int k = 1; k < NPOS; ++k) {
        if (visRuntime.pos[k].x < minx)
            minx = visRuntime.pos[k].x;
        if (visRuntime.pos[k].x > maxx)
            maxx = visRuntime.pos[k].x;
        if (visRuntime.pos[k].y < miny)
            miny = visRuntime.pos[k].y;
        if (visRuntime.pos[k].y > maxy)
            maxy = visRuntime.pos[k].y;
    }

    int iminx = (int)floor(minx * (1.0 / (MAP_GROUP_SIZE * GLOBAL_SCALE))) - 1;
    int imaxx = (int)(maxx * (1.0 / (MAP_GROUP_SIZE * GLOBAL_SCALE))) + 1;
    int iminy = (int)floor(miny * (1.0 / (MAP_GROUP_SIZE * GLOBAL_SCALE))) - 1;
    int imaxy = (int)(maxy * (1.0 / (MAP_GROUP_SIZE * GLOBAL_SCALE))) + 1;

    m_VisWater.clear();

    // DM("TIME", "TIME_STEP1 " + CStr(iminx) + " " + CStr(iminy)+ " " + CStr(imaxx)+ " " + CStr(imaxy));

    for (visRuntime.j = iminy; visRuntime.j <= imaxy /*m_GroupSize.y*/; ++visRuntime.j) {
        for (visRuntime.i = iminx; visRuntime.i <= imaxx /*m_GroupSize.x*/; ++visRuntime.i) {
            bool is_map = (visRuntime.i >= 0) && (visRuntime.i < m_GroupSize.x) && (visRuntime.j >= 0) &&
                          (visRuntime.j < m_GroupSize.y);
            if (is_map) {
                // calculate visibility of map group
                CMatrixMapGroup *cmg = m_Group[visRuntime.j * m_GroupSize.x + visRuntime.i];

                if (cmg == NULL) {
                    goto water_calc;
                }
                else {
                    if (no_info_about_visibility)
                        CheckCandidate(visRuntime, cmg);
                }
            }
            else {
                // calculate visibility of free water
            water_calc:
                D3DXVECTOR3 p0(float(visRuntime.i * (MAP_GROUP_SIZE * GLOBAL_SCALE)),
                               float(visRuntime.j * (MAP_GROUP_SIZE * GLOBAL_SCALE)), WATER_LEVEL);
                D3DXVECTOR3 p1(p0.x + float(MAP_GROUP_SIZE * GLOBAL_SCALE), p0.y + float(MAP_GROUP_SIZE * GLOBAL_SCALE),
                               WATER_LEVEL);

                int i0 = NPOS - 1;
                int i1 = 0;
                while (i1 < NPOS) {
                    if (PointLineCatch(visRuntime.pos[i0], visRuntime.pos[i1], *(D3DXVECTOR2 *)&p0)) {
                        goto checknext_w;
                    }
                    if (PointLineCatch(visRuntime.pos[i0], visRuntime.pos[i1], *(D3DXVECTOR2 *)&p1)) {
                        goto checknext_w;
                    }
                    if (PointLineCatch(visRuntime.pos[i0], visRuntime.pos[i1], D3DXVECTOR2(p0.x, p1.y))) {
                        goto checknext_w;
                    }
                    if (PointLineCatch(visRuntime.pos[i0], visRuntime.pos[i1], D3DXVECTOR2(p1.x, p0.y))) {
                        goto checknext_w;
                    }
                    goto invisible_w;
                checknext_w:
                    i0 = i1++;
                }

                if (m_Camera.IsInFrustum(p0, p1)) {
                    m_VisWater.push_back(*(D3DXVECTOR2 *)&p0);
                }
            invisible_w:;
            }
        }
    }

    // DM("TIME", "TIME_END");
}
