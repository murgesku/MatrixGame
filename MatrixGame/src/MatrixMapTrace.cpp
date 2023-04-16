// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <algorithm>

#include "MatrixMap.hpp"
#include "MatrixObject.hpp"
#include "MatrixObjectRobot.hpp"
#include "MatrixFlyer.hpp"
#include "MatrixObjectCannon.hpp"

typedef struct {
    D3DXVECTOR3 dir;
    D3DXVECTOR3 out;
    float len;
    float dx, dy, dz, ddx, ddy, dzdx, dzdy;
    float minz, maxz;
    int gdx, gdy;
    bool usex;
    double k;
    float last_t;
    CMatrixMapStatic *obj;
    CMatrixMapStatic *skip;
    DWORD mask;
} internal_trace_data;

CMatrixMap::EScanResult CMatrixMap::ScanLandscapeGroup(void *d, int gx, int gy, const D3DXVECTOR3 &start,
                                                       const D3DXVECTOR3 &end) {
    DTRACE();
    internal_trace_data *data = (internal_trace_data *)d;

    if (gx < 0 || gx >= m_GroupSize.x || gy < 0 || gy >= m_GroupSize.y) {
        return SR_NONE;
    }

    bool object_hit = data->obj != NULL;

    CMatrixMapGroup *mg = m_Group[gx + gy * m_GroupSize.x];
    if (mg == NULL)
        return SR_NONE;

    // mg->DrawBBox();
    /*
    CHelper::Create(1,0)->Triangle(D3DXVECTOR3(mg->GetPos0().x, mg->GetPos0().y, 0),
                                D3DXVECTOR3(mg->GetPos1().x, mg->GetPos1().y, 0),
                                D3DXVECTOR3(mg->GetPos0().x, mg->GetPos1().y, 0), 0x8000FF00);
    CHelper::Create(1,0)->Triangle(D3DXVECTOR3(mg->GetPos0().x, mg->GetPos0().y, 0),
                                D3DXVECTOR3(mg->GetPos1().x, mg->GetPos1().y, 0),
                                D3DXVECTOR3(mg->GetPos1().x, mg->GetPos0().y, 0), 0x8000FF00);
                                */

    float t0 = 0, t1 = 0;
    if (data->dx != 0) {
        float ft0 = ((gx * (GLOBAL_SCALE * MAP_GROUP_SIZE)) - start.x) * data->ddx;
        float ft1 = (((gx + 1) * (GLOBAL_SCALE * MAP_GROUP_SIZE)) - start.x) * data->ddx;
        if (ft0 > ft1) {
            if (ft1 > 0)
                t0 = ft1;
            if (ft0 > 0)
                t1 = ft0;
        }
        else {
            if (ft0 > 0)
                t0 = ft0;
            if (ft1 > 0)
                t1 = ft1;
        }
    }
    if (data->dy != 0) {
        float ft0 = ((gy * (GLOBAL_SCALE * MAP_GROUP_SIZE)) - start.y) * data->ddy;
        float ft1 = (((gy + 1) * (GLOBAL_SCALE * MAP_GROUP_SIZE)) - start.y) * data->ddy;
        if (ft0 > ft1) {
            if (ft1 > t0)
                t0 = ft1;
            if (ft0 < t1)
                t1 = ft0;
        }
        else {
            if (ft0 > t0)
                t0 = ft0;
            if (ft1 < t1)
                t1 = ft1;
        }
    }

    if (t0 > data->last_t)
        return SR_BREAK;

    double minz = std::min(start.z + data->dir.z * t0, start.z + data->dir.z * t1);
    if (minz < data->minz)
        minz = data->minz;

    if (mg->GetMaxZObjRobots() >= minz) {
        // trace objects
        if ((data->mask & (TRACE_OBJECT | TRACE_ROBOT | TRACE_BUILDING | TRACE_CANNON)) != 0) {
            int n = mg->ObjectsCnt();

            CMatrixMapStatic *o2 = NULL;

            for (int i = 0; i < n; ++i) {
                CMatrixMapStatic *o;
                if (o2 == NULL) {
                    o = mg->GetObject(i);
                }
                else {
                    o = o2;
                    o2 = NULL;
                    --i;
                }

                DWORD trace_sphere = TRACE_OBJECTSPHERE;
                if (data->mask & TRACE_SKIP_INVISIBLE) {
                    if (o->IsTraceInvisible())
                        trace_sphere = 0;
                }
                if (o->m_IntersectFlagTracer == m_IntersectFlagTracer)
                    continue;
                if (o == data->skip)
                    continue;
                if (!o->FitToMask(data->mask))
                    continue;

                float t;
                bool hit;
                if ((data->mask & trace_sphere) != 0) {
                    hit = IsIntersectSphere(o->GetGeoCenter(), o->GetRadius(), start, data->dir, t);
                    if (hit && (t < 0))
                        hit = false;
                }
                else {
                    hit = o->Pick(start, data->dir, &t);
                }

                if (hit && (t < data->last_t) && t < t1 && t > t0) {
                    o->m_IntersectFlagTracer = g_MatrixMap->m_IntersectFlagTracer;
                    data->last_t = t;
                    data->obj = o;
                    object_hit = true;
                }
            }
        }
        if (object_hit) {
            data->out = start + data->dir * data->last_t;
        }
    }

    if (!(data->mask & TRACE_LANDSCAPE))
        return object_hit ? SR_FOUND : SR_NONE;
    if (mg->GetMaxZLand() < minz)
        return object_hit ? SR_FOUND : SR_NONE;

    if (start.y < 0 && data->gdy < 0) {
        return object_hit ? SR_FOUND : SR_BREAK;
    }
    if (start.y > (m_Size.y * GLOBAL_SCALE) && data->gdy > 0) {
        return object_hit ? SR_FOUND : SR_BREAK;
    }
    if (start.x < 0 && data->gdx < 0) {
        return object_hit ? SR_FOUND : SR_BREAK;
    }
    if (start.x > (m_Size.x * GLOBAL_SCALE) && data->gdx > 0) {
        return object_hit ? SR_FOUND : SR_BREAK;
    }

    // so, scan can be intersect with current group...

#ifdef EXPEREMENTAL_TRACER_IS_SLOWER_THEN_ORIGINAL
    {  /////////////////////////////////////////////// experemental
        float t = data->last_t;
        bool hit = mg->Pick(start, data->dir, t);

        if (hit && (t >= 0)) {
            if (t < data->last_t) {
                data->last_t = t;
                data->out = start + (data->dir * t);
                data->obj = NULL;
                return SR_FOUND;
            }
            else {
                return SR_BREAK;
            }
        }
        return object_hit ? SR_FOUND : SR_NONE;
    }  /////////////////////////////////////////////// experemental
#endif

    int px0 = gx * MAP_GROUP_SIZE;
    int py0 = gy * MAP_GROUP_SIZE;
    int px1 = (gx + 1) * MAP_GROUP_SIZE;
    int py1 = (gy + 1) * MAP_GROUP_SIZE;
    if (px1 > m_Size.x)
        px1 = m_Size.x;
    if (py1 > m_Size.y)
        py1 = m_Size.y;

    if (data->usex) {
        // along x
        int x, y, ex;
        float bx;
        if (data->gdx > 0) {
            x = (gx * MAP_GROUP_SIZE);
            ex = std::min(((gx + 1) * MAP_GROUP_SIZE), px1);
            bx = (x * GLOBAL_SCALE);
            if (bx < start.x) {
                x = TruncFloat(start.x * INVERT(GLOBAL_SCALE));
                bx = start.x;
            }
            if (ex * GLOBAL_SCALE > end.x)
                ex = TruncFloat(end.x * INVERT(GLOBAL_SCALE)) + 1;
        }
        else {
            x = ((gx + 1) * MAP_GROUP_SIZE) - 1;
            ex = std::min((gx * MAP_GROUP_SIZE), px1);
            bx = ((x + 1) * GLOBAL_SCALE);
            if (bx > start.x) {
                x = TruncFloat(start.x * INVERT(GLOBAL_SCALE));
                bx = start.x;
            }
            if (ex * GLOBAL_SCALE < end.x)
                ex = TruncFloat(end.x * INVERT(GLOBAL_SCALE));
            --ex;
        }
        y = int((data->k * (bx - start.x) + start.y) * INVERT(GLOBAL_SCALE));

        float mz = ((x * GLOBAL_SCALE) - start.x) * data->dzdx + start.z;
        float dmz;
        {
            float mz1 = ((x * GLOBAL_SCALE) + GLOBAL_SCALE - start.x) * data->dzdx + start.z;
            dmz = mz1 - mz;
            COPY_SIGN_FLOAT(dmz, data->dz);
            if (mz1 < mz)
                mz = mz1;
        }

        while (x != ex) {
            if ((x >= px0) && (x < px1) && (y >= py0) && (y < py1)) {
                SMatrixMapPoint *mpo = PointGet(x, y);

                if ((mz < mpo->z) || (mz < (mpo + 1)->z) || (mz < (mpo + m_Size.x + 1)->z) ||
                    (mz < (mpo + 1 + g_MatrixMap->m_Size.x + 1)->z)) {
                    D3DXVECTOR3 p0(GLOBAL_SCALE * x, GLOBAL_SCALE * (y + 1), (mpo + m_Size.x + 1)->z);
                    D3DXVECTOR3 p1(GLOBAL_SCALE * x, GLOBAL_SCALE * y, mpo->z);
                    D3DXVECTOR3 p2(GLOBAL_SCALE * (x + 1), GLOBAL_SCALE * (y + 1),
                                   (mpo + 1 + g_MatrixMap->m_Size.x + 1)->z);

                    // CHelper::Create(1,0)->Triangle(p0,p1,p2, 0xFFFF0000);

                    float t = 0, dummy;

                    bool hit = IntersectTriangle(start, data->dir, p0, p1, p2, t, dummy, dummy);
                    if (!hit) {
                        D3DXVECTOR3 p3(GLOBAL_SCALE * (x + 1), GLOBAL_SCALE * y, (mpo + 1)->z);
                        hit = IntersectTriangle(start, data->dir, p1, p3, p2, t, dummy, dummy);
                        // CHelper::Create(1,0)->Triangle(p1,p3,p2, 0xFFFF0000);
                    }
                    if (hit && (t >= 0)) {
                        if (t < data->last_t) {
                            data->last_t = t;
                            data->out = start + (data->dir * t);
                            data->obj = NULL;
                            return SR_FOUND;
                        }
                        else {
                            return SR_BREAK;
                        }
                    }
                }
            }

            double y1 = data->k * ((double(x) + 0.5) * (GLOBAL_SCALE)-start.x) + start.y;
            double y2 = data->k * ((double(x + data->gdx) + 0.5) * (GLOBAL_SCALE)-start.x) + start.y;
            double dy1 = y1 - ((double(y + data->gdy) + 0.5) * (GLOBAL_SCALE));
            double dy2 = y2 - ((double(y) + 0.5) * (GLOBAL_SCALE));
            MAKE_ABS_DOUBLE(dy1);
            MAKE_ABS_DOUBLE(dy2);
            if (dy1 < dy2) {
                y += data->gdy;
            }
            else {
                x += data->gdx;
                mz += dmz;
            }
        }
    }
    else {
        // along y
        int x, y, ey;
        float by;
        if (data->gdy > 0) {
            y = (gy * MAP_GROUP_SIZE);
            ey = std::min(((gy + 1) * MAP_GROUP_SIZE), py1);
            by = (y * GLOBAL_SCALE);
            if (by < start.y) {
                y = TruncFloat(start.y * INVERT(GLOBAL_SCALE));
                by = start.y;
            }
            if (ey * GLOBAL_SCALE > end.y)
                ey = TruncFloat(end.y * INVERT(GLOBAL_SCALE)) + 1;
        }
        else {
            y = ((gy + 1) * MAP_GROUP_SIZE) - 1;
            ey = std::min((gy * MAP_GROUP_SIZE), py1);
            by = ((y + 1) * GLOBAL_SCALE);
            if (by > start.y) {
                y = TruncFloat(start.y * INVERT(GLOBAL_SCALE));
                by = start.y;
            }
            if (ey * GLOBAL_SCALE < end.y)
                ey = TruncFloat(end.y * INVERT(GLOBAL_SCALE));
            --ey;
        }
        x = int((data->k * (by - start.y) + start.x) * INVERT(GLOBAL_SCALE));

        float mz = ((y * GLOBAL_SCALE) - start.y) * data->dzdy + start.z;
        float dmz;
        {
            float mz1 = ((y * GLOBAL_SCALE) + GLOBAL_SCALE - start.y) * data->dzdy + start.z;
            dmz = mz1 - mz;
            COPY_SIGN_FLOAT(dmz, data->dz);
            if (mz1 < mz)
                mz = mz1;
        }

        while (y != ey) {
            if ((x >= px0) && (x < px1) && (y >= py0) && (y < py1)) {
                SMatrixMapPoint *mpo = PointGet(x, y);
                if ((mz < mpo->z) || (mz < (mpo + 1)->z) || (mz < (mpo + m_Size.x + 1)->z) ||
                    (mz < (mpo + 1 + g_MatrixMap->m_Size.x + 1)->z)) {
                    D3DXVECTOR3 p0(GLOBAL_SCALE * x, GLOBAL_SCALE * (y + 1), (mpo + m_Size.x + 1)->z);
                    D3DXVECTOR3 p1(GLOBAL_SCALE * x, GLOBAL_SCALE * y, mpo->z);
                    D3DXVECTOR3 p2(GLOBAL_SCALE * (x + 1), GLOBAL_SCALE * (y + 1),
                                   (mpo + 1 + g_MatrixMap->m_Size.x + 1)->z);

                    float t = 0, dummy;

                    bool hit = IntersectTriangle(start, data->dir, p0, p1, p2, t, dummy, dummy);
                    if (!hit) {
                        D3DXVECTOR3 p3(GLOBAL_SCALE * (x + 1), GLOBAL_SCALE * y, (mpo + 1)->z);
                        hit = IntersectTriangle(start, data->dir, p1, p3, p2, t, dummy, dummy);
                    }
                    if (hit && (t >= 0)) {
                        if (t < data->last_t) {
                            data->last_t = t;
                            data->out = start + (data->dir * t);
                            data->obj = NULL;
                            return SR_FOUND;
                        }
                        else {
                            return SR_BREAK;
                        }
                    }
                }
            }

            double x1 = data->k * ((double(y) + 0.5) * (GLOBAL_SCALE)-start.y) + start.x;
            double x2 = data->k * ((double(y + data->gdy) + 0.5) * (GLOBAL_SCALE)-start.y) + start.x;
            double dx1 = x1 - ((double(x + data->gdx) + 0.5) * (GLOBAL_SCALE));
            double dx2 = x2 - ((double(x) + 0.5) * (GLOBAL_SCALE));

            MAKE_ABS_DOUBLE(dx1);
            MAKE_ABS_DOUBLE(dx2);

            if (dx1 < dx2) {
                x += data->gdx;
            }
            else {
                y += data->gdy;
                mz += dmz;
            }
        }
    }

    return object_hit ? SR_FOUND : SR_NONE;
}

CMatrixMapStatic *CMatrixMap::Trace(D3DXVECTOR3 *result, const D3DXVECTOR3 &start, const D3DXVECTOR3 &end, DWORD mask,
                                    CMatrixMapStatic *skip) {
    DTRACE();

    ++m_IntersectFlagTracer;

    internal_trace_data data;

    data.skip = skip;
    data.mask = mask;

    data.dx = end.x - start.x;
    data.dy = end.y - start.y;
    data.dz = end.z - start.z;

    auto tmp = D3DXVECTOR3(data.dx, data.dy, data.dz);
    data.len = D3DXVec3Length(&tmp);

    const float oblen = INVERT(((data.len != 0) ? data.len : 1.0f));
    data.dir = D3DXVECTOR3(data.dx * oblen, data.dy * oblen, data.dz * oblen);

    data.last_t = data.len;
    data.minz = std::min(start.z, end.z);
    data.maxz = std::max(start.z, end.z);
    data.obj = NULL;

    if (m_AD_Obj_cnt > 0) {
        for (int od = 0; od < m_AD_Obj_cnt; ++od) {
            float ttt;
            bool hit;
            if (m_AD_Obj[od]->IsFlyer()) {
                if (mask & TRACE_FLYER) {
                    if ((data.mask & TRACE_OBJECTSPHERE) != 0) {
                        hit = IsIntersectSphere(m_AD_Obj[od]->GetGeoCenter(), m_AD_Obj[od]->GetRadius(), start,
                                                data.dir, ttt);
                        if (hit && (ttt < 0))
                            hit = false;
                    }
                    else {
                        hit = m_AD_Obj[od]->Pick(start, data.dir, &ttt);
                    }

                    if (hit && (ttt < data.last_t)) {
                        data.last_t = ttt;
                        data.obj = m_AD_Obj[od];
                        m_AD_Obj[od]->m_IntersectFlagTracer = m_IntersectFlagTracer;
                    }
                }

                CMatrixFlyer *fl = (CMatrixFlyer *)m_AD_Obj[od];
                if (fl->CarryingRobot() && (mask & TRACE_ROBOT)) {
                    CMatrixMapStatic *ms = fl->GetCarryingRobot();

                    if ((data.mask & TRACE_OBJECTSPHERE) != 0) {
                        hit = IsIntersectSphere(ms->GetGeoCenter(), ms->GetRadius(), start, data.dir, ttt);
                        if (hit && (ttt < 0))
                            hit = false;
                    }
                    else {
                        hit = ms->Pick(start, data.dir, &ttt);
                    }

                    if (hit && (ttt < data.last_t)) {
                        data.last_t = ttt;
                        data.obj = ms;
                        ms->m_IntersectFlagTracer = m_IntersectFlagTracer;
                    }
                }
            }
            if (data.obj) {
                data.out = start + data.dir * data.last_t;
            }
        }
    }

    if (((*((DWORD *)(&data.dx)) | *((DWORD *)(&data.dy))) & 0x7FFFFFFF) == 0) {
        if (data.dz == 0)
            return TRACE_STOP_NONE;

        float z = GetZ(start.x, start.y);

        bool down = (data.dir.z < 0);
        bool land = (down && (z >= end.z && z < start.z)) || (!down && (z > start.z && z <= end.z));
        bool object_hit = false;

        if ((mask & (TRACE_OBJECT | TRACE_BUILDING | TRACE_ROBOT | TRACE_CANNON | TRACE_FLYER)))

        {
            int gx = TruncDouble(start.x * (1.0 / (GLOBAL_SCALE * MAP_GROUP_SIZE)));
            int gy = TruncDouble(start.y * (1.0 / (GLOBAL_SCALE * MAP_GROUP_SIZE)));

            if ((gx >= 0) && (gx < m_GroupSize.x) && (gy >= 0) && (gy < m_GroupSize.y)) {
                CMatrixMapGroup *mg = m_Group[gx + gy * m_GroupSize.x];
                if (mg != NULL)
                    if (land || ((data.minz < mg->GetMaxZObjRobots()) && (data.maxz > mg->GetMinZ()))) {
                        CMatrixMapStatic *o2 = NULL;
                        // trace objects
                        int n = mg->ObjectsCnt();
                        for (int i = 0; i < n; ++i) {
                            CMatrixMapStatic *o;
                            if (o2 == NULL) {
                                o = mg->GetObject(i);
                            }
                            else {
                                o = o2;
                                o2 = NULL;
                                --i;
                            }

                            if (o == data.skip)
                                continue;
                            if (!o->FitToMask(mask))
                                continue;

                            if (o->m_IntersectFlagTracer == m_IntersectFlagTracer)
                                continue;
                            o->m_IntersectFlagTracer = m_IntersectFlagTracer;

                            float t;
                            bool hit;
                            if ((mask & TRACE_OBJECTSPHERE) != 0) {
                                hit = IsIntersectSphere(o->GetGeoCenter(), o->GetRadius(), start, data.dir, t);
                                if (hit && (t < 0))
                                    hit = false;
                            }
                            else {
                                hit = o->Pick(start, data.dir, &t);
                            }

                            if (hit && (t < data.last_t)) {
                                data.last_t = t;
                                object_hit = true;
                                data.obj = o;
                            }
                        }
                    }
            }
        }

        if (object_hit) {
            data.out = start + data.dir * data.last_t;

            if ((down && data.out.z >= z) || (!down && data.out.z <= z)) {
                if (result)
                    *result = data.out;
                return data.obj;
            }
        }

        if (land && (mask & (TRACE_LANDSCAPE | TRACE_WATER))) {
            if (result) {
                result->x = start.x;
                result->y = start.y;
                result->z = z;
            }
            if (z < WATER_LEVEL) {
                return TRACE_STOP_WATER;
            }
            else {
                return TRACE_STOP_LANDSCAPE;
            }
        }

        return TRACE_STOP_NONE;
    }

    data.usex = fabs(data.dx) > fabs(data.dy);

    if (data.dx != 0) {
        data.ddx = INVERT(data.dir.x);
        data.dzdx = data.dir.z * data.ddx;
    }
    if (data.dy != 0) {
        data.ddy = INVERT(data.dir.y);
        data.dzdy = data.dir.z * data.ddy;
    }

    //  trace landscape
    if ((mask & (TRACE_LANDSCAPE | TRACE_OBJECT | TRACE_BUILDING | TRACE_ROBOT | TRACE_CANNON | TRACE_FLYER)) != 0) {
        int gx0 = TruncDouble(start.x * (1.0 / (GLOBAL_SCALE * MAP_GROUP_SIZE)));
        int gy0 = TruncDouble(start.y * (1.0 / (GLOBAL_SCALE * MAP_GROUP_SIZE)));
        int gx1 = TruncDouble(end.x * (1.0 / (GLOBAL_SCALE * MAP_GROUP_SIZE)));
        int gy1 = TruncDouble(end.y * (1.0 / (GLOBAL_SCALE * MAP_GROUP_SIZE)));
        int gxc;
        int gyc;
        if (data.dx >= 0) {
            data.gdx = 1;
            gxc = gx1 - gx0;
        }
        else {
            data.gdx = -1;
            gxc = gx0 - gx1;
        }
        if (data.dy >= 0) {
            data.gdy = 1;
            gyc = gy1 - gy0;
        }
        else {
            data.gdy = -1;
            gyc = gy0 - gy1;
        }

        CMatrixMap::EScanResult scanhit = SR_NONE;

        if (data.usex) {
            // scan x
            data.k = data.dy / data.dx;

            int prevgy = gy0;

            scanhit = ScanLandscapeGroup(&data, gx0, gy0, start, end);

            if (gxc == 0) {
                if ((scanhit == SR_NONE) && (gyc != 0)) {
                    scanhit = ScanLandscapeGroup(&data, gx1, gy1, start, end);
                }
            }
            else if ((scanhit == SR_NONE)) {
                int gcx;
                if (data.gdx >= 0) {
                    gcx = 1;
                }
                else {
                    gcx = 0;
                }
                while (gxc-- > 0) {
                    int gy =
                            TruncDouble((data.k * ((gx0 + gcx) * (GLOBAL_SCALE * MAP_GROUP_SIZE) - start.x) + start.y) *
                                        INVERT((GLOBAL_SCALE * MAP_GROUP_SIZE)));

                    if (prevgy != gy) {
                        scanhit = ScanLandscapeGroup(&data, gx0, gy, start, end);
                        prevgy = gy;
                    }
                    else {
                        goto second_scan_x;  // to avoid unnecessary condition checking
                    }
                    if ((scanhit == SR_NONE)) {
                    second_scan_x:
                        scanhit = ScanLandscapeGroup(&data, gx0 + data.gdx, gy, start, end);
                    }
                    if ((scanhit != SR_NONE))
                        goto scan_x_complete;

                    if (data.gdx < 0) {
                        if (gx0 > 0)
                            --gx0;
                        else {
                            scanhit = SR_BREAK;
                            goto scan_x_complete;
                        }
                    }
                    else {
                        if (gx0 < (m_GroupSize.x - 1))
                            ++gx0;
                        else {
                            scanhit = SR_BREAK;
                            goto scan_x_complete;
                        }
                    }
                }
                if (prevgy != gy1) {
                    scanhit = ScanLandscapeGroup(&data, gx1, gy1, start, end);
                }
            scan_x_complete:;
            }
        }
        else {
            // scan y
            data.k = data.dx / data.dy;

            int prevgx = gx0;
            scanhit = ScanLandscapeGroup(&data, gx0, gy0, start, end);
            if (gyc == 0) {
                if ((scanhit == SR_NONE) && (gxc != 0)) {
                    scanhit = ScanLandscapeGroup(&data, gx1, gy1, start, end);
                }
            }
            else if ((scanhit == SR_NONE)) {
                int gcy;
                if (data.gdy >= 0) {
                    gcy = 1;
                }
                else {
                    gcy = 0;
                }

                while (gyc-- > 0) {
                    int gx =
                            TruncDouble((data.k * ((gy0 + gcy) * (GLOBAL_SCALE * MAP_GROUP_SIZE) - start.y) + start.x) *
                                        INVERT((GLOBAL_SCALE * MAP_GROUP_SIZE)));

                    if (prevgx != gx) {
                        scanhit = ScanLandscapeGroup(&data, gx, gy0, start, end);
                        prevgx = gx;
                    }
                    else {
                        goto second_scan_y;  // to avoid unnecessary condition checking
                    }
                    if ((scanhit == SR_NONE)) {
                    second_scan_y:
                        scanhit = ScanLandscapeGroup(&data, gx, gy0 + data.gdy, start, end);
                    }
                    if ((scanhit != SR_NONE))
                        goto scan_y_complete;

                    if (data.gdy < 0) {
                        if (gy0 > 0)
                            --gy0;
                        else {
                            scanhit = SR_BREAK;
                            goto scan_y_complete;
                        }
                    }
                    else {
                        if (gy0 < (m_GroupSize.y - 1))
                            ++gy0;
                        else {
                            scanhit = SR_BREAK;
                            goto scan_y_complete;
                        }
                    }
                }
                if (prevgx != gx1) {
                    scanhit = ScanLandscapeGroup(&data, gx1, gy1, start, end);
                }
            scan_y_complete:;
            }
        }

        if (scanhit == SR_BREAK) {
            if (data.obj != NULL)
                goto obj_found;
            return TRACE_STOP_NONE;
        }
        if (scanhit == SR_FOUND) {
            if (data.obj != NULL) {
            obj_found:
                if (result)
                    *result = data.out;
                return data.obj;
            }
            if (data.out.z > WATER_LEVEL) {
                if (result)
                    *result = data.out;

                return TRACE_STOP_LANDSCAPE;
            }
        }
    }

    // trace obects

    // trace water
    if (mask & TRACE_WATER) {
        if (data.minz < WATER_LEVEL && data.maxz >= WATER_LEVEL) {
            double k = (WATER_LEVEL - start.z) / data.dir.z;
            data.out.x = (float)(start.x + data.dir.x * k);
            data.out.y = (float)(start.y + data.dir.y * k);
            data.out.z = WATER_LEVEL;
            if (result)
                *result = data.out;
            return TRACE_STOP_WATER;
        }
    }

    if (result)
        *result = end;
    return TRACE_STOP_NONE;
}

CMatrixMap::EScanResult CMatrixMap::ScanLandscapeGroupForLand(void *d, int gx, int gy, const D3DXVECTOR3 &start,
                                                              const D3DXVECTOR3 &end) {
    DTRACE();
    internal_trace_data *data = (internal_trace_data *)d;

    if (gx < 0 || gx >= m_GroupSize.x || gy < 0 || gy >= m_GroupSize.y) {
        return SR_NONE;
    }

    CMatrixMapGroup *mg = m_Group[gx + gy * m_GroupSize.x];
    if (mg == NULL)
        return SR_NONE;

    float t0 = 0, t1 = 0;
    if (data->dx != 0) {
        float ft0 = ((gx * (GLOBAL_SCALE * MAP_GROUP_SIZE)) - start.x) * data->ddx;
        float ft1 = (((gx + 1) * (GLOBAL_SCALE * MAP_GROUP_SIZE)) - start.x) * data->ddx;
        if (ft0 > ft1) {
            if (ft1 > 0)
                t0 = ft1;
            if (ft0 > 0)
                t1 = ft0;
        }
        else {
            if (ft0 > 0)
                t0 = ft0;
            if (ft1 > 0)
                t1 = ft1;
        }
    }
    if (data->dy != 0) {
        float ft0 = ((gy * (GLOBAL_SCALE * MAP_GROUP_SIZE)) - start.y) * data->ddy;
        float ft1 = (((gy + 1) * (GLOBAL_SCALE * MAP_GROUP_SIZE)) - start.y) * data->ddy;
        if (ft0 > ft1) {
            if (ft1 > t0)
                t0 = ft1;
            if (ft0 < t1)
                t1 = ft0;
        }
        else {
            if (ft0 > t0)
                t0 = ft0;
            if (ft1 < t1)
                t1 = ft1;
        }
    }

    if (t0 > data->last_t)
        return SR_BREAK;

    double minz = std::min(start.z + data->dir.z * t0, start.z + data->dir.z * t1);
    if (minz < data->minz)
        minz = data->minz;

    if (mg->GetMaxZLand() < minz)
        return SR_NONE;

    if (start.y < 0 && data->gdy < 0) {
        return SR_BREAK;
    }
    if (start.y > (m_Size.y * GLOBAL_SCALE) && data->gdy > 0) {
        return SR_BREAK;
    }
    if (start.x < 0 && data->gdx < 0) {
        return SR_BREAK;
    }
    if (start.x > (m_Size.x * GLOBAL_SCALE) && data->gdx > 0) {
        return SR_BREAK;
    }

    // so, scan can be intersect with current group...

    int px0 = gx * MAP_GROUP_SIZE;
    int py0 = gy * MAP_GROUP_SIZE;
    int px1 = (gx + 1) * MAP_GROUP_SIZE;
    int py1 = (gy + 1) * MAP_GROUP_SIZE;
    if (px1 > m_Size.x)
        px1 = m_Size.x;
    if (py1 > m_Size.y)
        py1 = m_Size.y;

    if (data->usex) {
        // along x
        int x, y, ex;
        float bx;
        if (data->gdx > 0) {
            x = (gx * MAP_GROUP_SIZE);
            ex = std::min(((gx + 1) * MAP_GROUP_SIZE), px1);
            bx = (x * GLOBAL_SCALE);
            if (bx < start.x) {
                x = TruncFloat(start.x * INVERT(GLOBAL_SCALE));
                bx = start.x;
            }
            if (ex * GLOBAL_SCALE > end.x)
                ex = TruncFloat(end.x * INVERT(GLOBAL_SCALE)) + 1;
        }
        else {
            x = ((gx + 1) * MAP_GROUP_SIZE) - 1;
            ex = std::min((gx * MAP_GROUP_SIZE), px1);
            bx = ((x + 1) * GLOBAL_SCALE);
            if (bx > start.x) {
                x = TruncFloat(start.x * INVERT(GLOBAL_SCALE));
                bx = start.x;
            }
            if (ex * GLOBAL_SCALE < end.x)
                ex = TruncFloat(end.x * INVERT(GLOBAL_SCALE));
            --ex;
        }
        y = int((data->k * (bx - start.x) + start.y) * INVERT(GLOBAL_SCALE));

        float mz = ((x * GLOBAL_SCALE) - start.x) * data->dzdx + start.z;
        float dmz;
        {
            float mz1 = ((x * GLOBAL_SCALE) + GLOBAL_SCALE - start.x) * data->dzdx + start.z;
            dmz = mz1 - mz;
            COPY_SIGN_FLOAT(dmz, data->dz);
            if (mz1 < mz)
                mz = mz1;
        }

        while (x != ex) {
            if ((x >= px0) && (x < px1) && (y >= py0) && (y < py1)) {
                SMatrixMapPoint *mpo = PointGet(x, y);

                if ((mz < mpo->z_land) || (mz < (mpo + 1)->z_land) || (mz < (mpo + m_Size.x + 1)->z_land) ||
                    (mz < (mpo + 1 + g_MatrixMap->m_Size.x + 1)->z_land)) {
                    D3DXVECTOR3 p0(GLOBAL_SCALE * x, GLOBAL_SCALE * (y + 1), (mpo + m_Size.x + 1)->z_land);
                    D3DXVECTOR3 p1(GLOBAL_SCALE * x, GLOBAL_SCALE * y, mpo->z_land);
                    D3DXVECTOR3 p2(GLOBAL_SCALE * (x + 1), GLOBAL_SCALE * (y + 1),
                                   (mpo + 1 + g_MatrixMap->m_Size.x + 1)->z_land);

                    // CHelper::Create(1,0)->Triangle(p0,p1,p2, 0xFFFF0000);

                    float t = 0, dummy;

                    bool hit = IntersectTriangle(start, data->dir, p0, p1, p2, t, dummy, dummy);
                    if (!hit) {
                        D3DXVECTOR3 p3(GLOBAL_SCALE * (x + 1), GLOBAL_SCALE * y, (mpo + 1)->z_land);
                        hit = IntersectTriangle(start, data->dir, p1, p3, p2, t, dummy, dummy);
                        // CHelper::Create(1,0)->Triangle(p1,p3,p2, 0xFFFF0000);
                    }
                    if (hit && (t >= 0)) {
                        if (t < data->last_t) {
                            data->last_t = t;
                            data->out = start + (data->dir * t);
                            return SR_FOUND;
                        }
                        else {
                            return SR_BREAK;
                        }
                    }
                }
            }

            double y1 = data->k * ((double(x) + 0.5) * (GLOBAL_SCALE)-start.x) + start.y;
            double y2 = data->k * ((double(x + data->gdx) + 0.5) * (GLOBAL_SCALE)-start.x) + start.y;
            double dy1 = y1 - ((double(y + data->gdy) + 0.5) * (GLOBAL_SCALE));
            double dy2 = y2 - ((double(y) + 0.5) * (GLOBAL_SCALE));
            MAKE_ABS_DOUBLE(dy1);
            MAKE_ABS_DOUBLE(dy2);
            if (dy1 < dy2) {
                y += data->gdy;
            }
            else {
                x += data->gdx;
                mz += dmz;
            }
        }
    }
    else {
        // along y
        int x, y, ey;
        float by;
        if (data->gdy > 0) {
            y = (gy * MAP_GROUP_SIZE);
            ey = std::min(((gy + 1) * MAP_GROUP_SIZE), py1);
            by = (y * GLOBAL_SCALE);
            if (by < start.y) {
                y = TruncFloat(start.y * INVERT(GLOBAL_SCALE));
                by = start.y;
            }
            if (ey * GLOBAL_SCALE > end.y)
                ey = TruncFloat(end.y * INVERT(GLOBAL_SCALE)) + 1;
        }
        else {
            y = ((gy + 1) * MAP_GROUP_SIZE) - 1;
            ey = std::min((gy * MAP_GROUP_SIZE), py1);
            by = ((y + 1) * GLOBAL_SCALE);
            if (by > start.y) {
                y = TruncFloat(start.y * INVERT(GLOBAL_SCALE));
                by = start.y;
            }
            if (ey * GLOBAL_SCALE < end.y)
                ey = TruncFloat(end.y * INVERT(GLOBAL_SCALE));
            --ey;
        }
        x = int((data->k * (by - start.y) + start.x) * INVERT(GLOBAL_SCALE));

        float mz = ((y * GLOBAL_SCALE) - start.y) * data->dzdy + start.z;
        float dmz;
        {
            float mz1 = ((y * GLOBAL_SCALE) + GLOBAL_SCALE - start.y) * data->dzdy + start.z;
            dmz = mz1 - mz;
            COPY_SIGN_FLOAT(dmz, data->dz);
            if (mz1 < mz)
                mz = mz1;
        }

        while (y != ey) {
            if ((x >= px0) && (x < px1) && (y >= py0) && (y < py1)) {
                SMatrixMapPoint *mpo = PointGet(x, y);
                if ((mz < mpo->z_land) || (mz < (mpo + 1)->z_land) || (mz < (mpo + m_Size.x + 1)->z_land) ||
                    (mz < (mpo + 1 + g_MatrixMap->m_Size.x + 1)->z_land)) {
                    D3DXVECTOR3 p0(GLOBAL_SCALE * x, GLOBAL_SCALE * (y + 1), (mpo + m_Size.x + 1)->z_land);
                    D3DXVECTOR3 p1(GLOBAL_SCALE * x, GLOBAL_SCALE * y, mpo->z_land);
                    D3DXVECTOR3 p2(GLOBAL_SCALE * (x + 1), GLOBAL_SCALE * (y + 1),
                                   (mpo + 1 + g_MatrixMap->m_Size.x + 1)->z_land);

                    float t = 0, dummy;

                    bool hit = IntersectTriangle(start, data->dir, p0, p1, p2, t, dummy, dummy);
                    if (!hit) {
                        D3DXVECTOR3 p3(GLOBAL_SCALE * (x + 1), GLOBAL_SCALE * y, (mpo + 1)->z_land);
                        hit = IntersectTriangle(start, data->dir, p1, p3, p2, t, dummy, dummy);
                    }
                    if (hit && (t >= 0)) {
                        if (t < data->last_t) {
                            data->last_t = t;
                            data->out = start + (data->dir * t);
                            return SR_FOUND;
                        }
                        else {
                            return SR_BREAK;
                        }
                    }
                }
            }

            double x1 = data->k * ((double(y) + 0.5) * (GLOBAL_SCALE)-start.y) + start.x;
            double x2 = data->k * ((double(y + data->gdy) + 0.5) * (GLOBAL_SCALE)-start.y) + start.x;
            double dx1 = x1 - ((double(x + data->gdx) + 0.5) * (GLOBAL_SCALE));
            double dx2 = x2 - ((double(x) + 0.5) * (GLOBAL_SCALE));

            MAKE_ABS_DOUBLE(dx1);
            MAKE_ABS_DOUBLE(dx2);

            if (dx1 < dx2) {
                x += data->gdx;
            }
            else {
                y += data->gdy;
                mz += dmz;
            }
        }
    }

    return SR_NONE;
}

bool CMatrixMap::TraceLand(D3DXVECTOR3 *out, const D3DXVECTOR3 &start, const D3DXVECTOR3 &dir) {
    DTRACE();

    internal_trace_data data;

    data.dx = dir.x;
    data.dy = dir.y;
    data.dz = dir.z;

    data.len = std::max(m_Size.x, m_Size.y) * GLOBAL_SCALE * 1.5f;

    const double oblen = 1.0 / data.len;
    data.dir = dir;

    data.last_t = data.len;
    float zz = (start.z + dir.z * data.len);
    data.minz = std::min(start.z, zz);
    data.maxz = std::max(start.z, zz);
    if (data.minz < 0)
        data.minz = 0;

    D3DXVECTOR3 end = start + dir * data.len;

    if (((*((DWORD *)(&data.dx)) | *((DWORD *)(&data.dy))) & 0x7FFFFFFF) == 0) {
        if (data.dz == 0) {
            if (out)
                *out = start;
            return false;
        }

        float z = GetZLand(start.x, start.y);
        if (z < 0)
            z = 0;

        bool down = (data.dir.z < 0);
        if (out) {
            out->x = start.x;
            out->y = start.y;
            out->z = z;
        }
        return true;
    }

    if (dir.z < 0) {
        // data.out = start + dir * data.len;
        float k = (0 - start.z) / dir.z;
        data.out.x = start.x + dir.x * k;
        data.out.y = start.y + dir.y * k;
        data.out.z = 0;

        end = data.out;
        auto tmp = start - end;
        data.last_t = D3DXVec3Length(&tmp);
    }
    else {
        data.out = start + dir * data.len;
    }

    data.usex = fabs(data.dx) > fabs(data.dy);

    if (data.dx != 0) {
        data.ddx = INVERT(data.dir.x);
        data.dzdx = data.dir.z * data.ddx;
    }
    if (data.dy != 0) {
        data.ddy = INVERT(data.dir.y);
        data.dzdy = data.dir.z * data.ddy;
    }

    //  trace landscape
    {
        int gx0 = TruncDouble(start.x * (1.0 / (GLOBAL_SCALE * MAP_GROUP_SIZE)));
        int gy0 = TruncDouble(start.y * (1.0 / (GLOBAL_SCALE * MAP_GROUP_SIZE)));
        int gx1 = TruncDouble((start.x + dir.x * data.len) * (1.0 / (GLOBAL_SCALE * MAP_GROUP_SIZE)));
        int gy1 = TruncDouble((start.y + dir.y * data.len) * (1.0 / (GLOBAL_SCALE * MAP_GROUP_SIZE)));
        int gxc;
        int gyc;
        if (data.dx >= 0) {
            data.gdx = 1;
            gxc = gx1 - gx0;
        }
        else {
            data.gdx = -1;
            gxc = gx0 - gx1;
        }
        if (data.dy >= 0) {
            data.gdy = 1;
            gyc = gy1 - gy0;
        }
        else {
            data.gdy = -1;
            gyc = gy0 - gy1;
        }

        EScanResult scanhit = SR_NONE;

        if (data.usex) {
            // scan x
            data.k = data.dy / data.dx;

            int prevgy = gy0;

            scanhit = ScanLandscapeGroupForLand(&data, gx0, gy0, start, end);

            if (gxc == 0) {
                if ((scanhit == SR_NONE) && (gyc != 0)) {
                    scanhit = ScanLandscapeGroupForLand(&data, gx1, gy1, start, end);
                }
            }
            else if ((scanhit == SR_NONE)) {
                int gcx;
                if (data.gdx >= 0) {
                    gcx = 1;
                }
                else {
                    gcx = 0;
                }
                while (gxc-- > 0) {
                    int gy =
                            TruncDouble((data.k * ((gx0 + gcx) * (GLOBAL_SCALE * MAP_GROUP_SIZE) - start.x) + start.y) *
                                        INVERT((GLOBAL_SCALE * MAP_GROUP_SIZE)));

                    if (prevgy != gy) {
                        scanhit = ScanLandscapeGroupForLand(&data, gx0, gy, start, end);
                        prevgy = gy;
                    }
                    else {
                        goto second_scan_x;  // to avoid unnecessary condition checking
                    }
                    if ((scanhit == SR_NONE)) {
                    second_scan_x:
                        scanhit = ScanLandscapeGroupForLand(&data, gx0 + data.gdx, gy, start, end);
                    }
                    if ((scanhit != SR_NONE))
                        goto scan_x_complete;

                    if (data.gdx < 0) {
                        if (gx0 > 0)
                            --gx0;
                        else {
                            scanhit = SR_BREAK;
                            goto scan_x_complete;
                        }
                    }
                    else {
                        if (gx0 < (m_GroupSize.x - 1))
                            ++gx0;
                        else {
                            scanhit = SR_BREAK;
                            goto scan_x_complete;
                        }
                    }
                }
                if (prevgy != gy1) {
                    scanhit = ScanLandscapeGroupForLand(&data, gx1, gy1, start, end);
                }
            scan_x_complete:;
            }
        }
        else {
            // scan y
            data.k = data.dx / data.dy;

            int prevgx = gx0;
            scanhit = ScanLandscapeGroupForLand(&data, gx0, gy0, start, end);
            if (gyc == 0) {
                if ((scanhit == SR_NONE) && (gxc != 0)) {
                    scanhit = ScanLandscapeGroupForLand(&data, gx1, gy1, start, end);
                }
            }
            else if ((scanhit == SR_NONE)) {
                int gcy;
                if (data.gdy >= 0) {
                    gcy = 1;
                }
                else {
                    gcy = 0;
                }

                while (gyc-- > 0) {
                    int gx =
                            TruncDouble((data.k * ((gy0 + gcy) * (GLOBAL_SCALE * MAP_GROUP_SIZE) - start.y) + start.x) *
                                        INVERT((GLOBAL_SCALE * MAP_GROUP_SIZE)));

                    if (prevgx != gx) {
                        scanhit = ScanLandscapeGroupForLand(&data, gx, gy0, start, end);
                        prevgx = gx;
                    }
                    else {
                        goto second_scan_y;  // to avoid unnecessary condition checking
                    }
                    if ((scanhit == SR_NONE)) {
                    second_scan_y:
                        scanhit = ScanLandscapeGroupForLand(&data, gx, gy0 + data.gdy, start, end);
                    }
                    if ((scanhit != SR_NONE))
                        goto scan_y_complete;

                    if (data.gdy < 0) {
                        if (gy0 > 0)
                            --gy0;
                        else {
                            scanhit = SR_BREAK;
                            goto scan_y_complete;
                        }
                    }
                    else {
                        if (gy0 < (m_GroupSize.y - 1))
                            ++gy0;
                        else {
                            scanhit = SR_BREAK;
                            goto scan_y_complete;
                        }
                    }
                }
                if (prevgx != gx1) {
                    scanhit = ScanLandscapeGroupForLand(&data, gx1, gy1, start, end);
                }
            scan_y_complete:;
            }
        }

        if (scanhit == SR_BREAK) {
            if (out)
                *out = data.out;
            return false;
        }
    }

    if (out)
        *out = data.out;
    return true;
}

bool CMatrixMap::CatchPoint(const D3DXVECTOR3 &from, const D3DXVECTOR3 &to) {
    DTRACE();

    internal_trace_data data;

    data.dx = to.x - from.x;
    data.dy = to.y - from.y;
    data.dz = to.z - from.z;

    data.len = float(sqrt(data.dx * data.dx + data.dy * data.dy + data.dz * data.dz));

    const double oblen = 1.0 / data.len;
    data.dir.x = float(data.dx * oblen);
    data.dir.y = float(data.dy * oblen);
    data.dir.z = float(data.dz * oblen);

    data.last_t = data.len;
    data.minz = std::min(from.z, to.z);
    data.maxz = std::max(from.z, to.z);

    if (((*((DWORD *)(&data.dx)) | *((DWORD *)(&data.dy))) & 0x7FFFFFFF) == 0) {
        if (data.dz == 0) {
            return true;
        }

        float z = GetZLand(from.x, from.y);

        return (z < data.minz);
    }

    data.usex = fabs(data.dx) > fabs(data.dy);

    if (data.dx != 0) {
        data.ddx = INVERT(data.dir.x);
        data.dzdx = data.dir.z * data.ddx;
    }
    if (data.dy != 0) {
        data.ddy = INVERT(data.dir.y);
        data.dzdy = data.dir.z * data.ddy;
    }

    //  trace landscape
    EScanResult scanhit = SR_NONE;
    {
        int gx0 = TruncDouble(from.x * (1.0 / (GLOBAL_SCALE * MAP_GROUP_SIZE)));
        int gy0 = TruncDouble(from.y * (1.0 / (GLOBAL_SCALE * MAP_GROUP_SIZE)));
        int gx1 = TruncDouble(to.x * (1.0 / (GLOBAL_SCALE * MAP_GROUP_SIZE)));
        int gy1 = TruncDouble(to.y * (1.0 / (GLOBAL_SCALE * MAP_GROUP_SIZE)));
        int gxc;
        int gyc;
        if (data.dx >= 0) {
            data.gdx = 1;
            gxc = gx1 - gx0;
        }
        else {
            data.gdx = -1;
            gxc = gx0 - gx1;
        }
        if (data.dy >= 0) {
            data.gdy = 1;
            gyc = gy1 - gy0;
        }
        else {
            data.gdy = -1;
            gyc = gy0 - gy1;
        }

        if (data.usex) {
            // scan x
            data.k = data.dy / data.dx;

            int prevgy = gy0;

            scanhit = ScanLandscapeGroupForLand(&data, gx0, gy0, from, to);

            if (gxc == 0) {
                if ((scanhit == SR_NONE) && (gyc != 0)) {
                    scanhit = ScanLandscapeGroupForLand(&data, gx1, gy1, from, to);
                }
            }
            else if ((scanhit == SR_NONE)) {
                int gcx;
                if (data.gdx >= 0) {
                    gcx = 1;
                }
                else {
                    gcx = 0;
                }
                while (gxc-- > 0) {
                    int gy = TruncDouble((data.k * ((gx0 + gcx) * (GLOBAL_SCALE * MAP_GROUP_SIZE) - from.x) + from.y) *
                                         INVERT((GLOBAL_SCALE * MAP_GROUP_SIZE)));

                    if (prevgy != gy) {
                        scanhit = ScanLandscapeGroupForLand(&data, gx0, gy, from, to);
                        prevgy = gy;
                    }
                    else {
                        goto second_scan_x;  // to avoid unnecessary condition checking
                    }
                    if ((scanhit == SR_NONE)) {
                    second_scan_x:
                        scanhit = ScanLandscapeGroupForLand(&data, gx0 + data.gdx, gy, from, to);
                    }
                    if ((scanhit != SR_NONE))
                        goto scan_x_complete;

                    if (data.gdx < 0) {
                        if (gx0 > 0)
                            --gx0;
                        else {
                            scanhit = SR_BREAK;
                            goto scan_x_complete;
                        }
                    }
                    else {
                        if (gx0 < (m_GroupSize.x - 1))
                            ++gx0;
                        else {
                            scanhit = SR_BREAK;
                            goto scan_x_complete;
                        }
                    }
                }
                if (prevgy != gy1) {
                    scanhit = ScanLandscapeGroupForLand(&data, gx1, gy1, from, to);
                }
            scan_x_complete:;
            }
        }
        else {
            // scan y
            data.k = data.dx / data.dy;

            int prevgx = gx0;
            scanhit = ScanLandscapeGroupForLand(&data, gx0, gy0, from, to);
            if (gyc == 0) {
                if ((scanhit == SR_NONE) && (gxc != 0)) {
                    scanhit = ScanLandscapeGroupForLand(&data, gx1, gy1, from, to);
                }
            }
            else if ((scanhit == SR_NONE)) {
                int gcy;
                if (data.gdy >= 0) {
                    gcy = 1;
                }
                else {
                    gcy = 0;
                }

                while (gyc-- > 0) {
                    int gx = TruncDouble((data.k * ((gy0 + gcy) * (GLOBAL_SCALE * MAP_GROUP_SIZE) - from.y) + from.x) *
                                         INVERT((GLOBAL_SCALE * MAP_GROUP_SIZE)));

                    if (prevgx != gx) {
                        scanhit = ScanLandscapeGroupForLand(&data, gx, gy0, from, to);
                        prevgx = gx;
                    }
                    else {
                        goto second_scan_y;  // to avoid unnecessary condition checking
                    }
                    if ((scanhit == SR_NONE)) {
                    second_scan_y:
                        scanhit = ScanLandscapeGroupForLand(&data, gx, gy0 + data.gdy, from, to);
                    }
                    if ((scanhit != SR_NONE))
                        goto scan_y_complete;

                    if (data.gdy < 0) {
                        if (gy0 > 0)
                            --gy0;
                        else {
                            scanhit = SR_BREAK;
                            goto scan_y_complete;
                        }
                    }
                    else {
                        if (gy0 < (m_GroupSize.y - 1))
                            ++gy0;
                        else {
                            scanhit = SR_BREAK;
                            goto scan_y_complete;
                        }
                    }
                }
                if (prevgx != gx1) {
                    scanhit = ScanLandscapeGroupForLand(&data, gx1, gy1, from, to);
                }
            scan_y_complete:;
            }
        }
    }

    return scanhit != SR_FOUND;
}
