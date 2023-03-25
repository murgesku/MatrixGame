// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "Math3D.hpp"
#include "3g.hpp"

#include <algorithm>

float SinCosTable[SIN_TABLE_SIZE];

static class InitMath3D {
public:
    InitMath3D(void) {
        // init sincostable
        for (int i = 0; i < SIN_TABLE_SIZE; i++) {
            SinCosTable[i] = (float)sin(i * 2.0 * M_PI / SIN_TABLE_SIZE);
        }
    }

} InitMath3DObject;

bool IntersectTriangle(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &v0, const D3DXVECTOR3 &v1,
                       const D3DXVECTOR3 &v2, float &ot, float &ou, float &ov) {
    D3DXVECTOR3 edge1, edge2;
    D3DXVECTOR3 pvec;
    D3DXVECTOR3 tvec;
    D3DXVECTOR3 qvec;
    float det;

    edge1.x = v1.x - v0.x;
    edge1.y = v1.y - v0.y;
    edge1.z = v1.z - v0.z;
    edge2.x = v2.x - v0.x;
    edge2.y = v2.y - v0.y;
    edge2.z = v2.z - v0.z;

    D3DXVec3Cross(&pvec, &dir, &edge2);

    det = D3DXVec3Dot(&edge1, &pvec);
    if (det < 0.0001f)
        return false;

    tvec.x = orig.x - v0.x;
    tvec.y = orig.y - v0.y;
    tvec.z = orig.z - v0.z;

    float uu = D3DXVec3Dot(&tvec, &pvec);
    if ((uu < 0.0f) || (uu > det))
        return false;

    D3DXVec3Cross(&qvec, &tvec, &edge1);

    float vv = D3DXVec3Dot(&dir, &qvec);
    if ((vv < 0.0f) || ((uu + vv) > det))
        return false;

    float tt = D3DXVec3Dot(&edge2, &qvec);
    double fInvDet = 1.0 / det;

    ot = float(tt * fInvDet);
    ou = float(uu * fInvDet);
    ov = float(vv * fInvDet);

    return true;
}

float DistLinePoint(const D3DXVECTOR3 &l1, const D3DXVECTOR3 &l2, const D3DXVECTOR3 &p) {
    D3DXVECTOR3 v = l2 - l1;
    D3DXVECTOR3 w = p - l1;

    float c1 = D3DXVec3Dot(&w, &v);
    float c2 = D3DXVec3Dot(&v, &v);

    float b = c1 / c2;

    D3DXVECTOR3 Pb = (l1 + b * v) - p;

    return float(sqrt(D3DXVec3Dot(&Pb, &Pb)));
}

void CalcPick(CPoint mp, D3DXMATRIX &matProj, D3DXMATRIX &matView, D3DXVECTOR3 *vpos, D3DXVECTOR3 *vdir) {
    D3DXVECTOR3 v;
    D3DXMATRIX m;

    v.x = (((2.0f * mp.x) / float(g_ScreenX)) - 1) / matProj._11;
    v.y = -(((2.0f * mp.y) / float(g_ScreenY)) - 1) / matProj._22;
    v.z = 1.0f;

    D3DXMatrixInverse(&m, NULL, &matView);

    vdir->x = v.x * m._11 + v.y * m._21 + v.z * m._31;
    vdir->y = v.x * m._12 + v.y * m._22 + v.z * m._32;
    vdir->z = v.x * m._13 + v.y * m._23 + v.z * m._33;
    D3DXVec3Normalize(vdir, vdir);

    vpos->x = m._41;
    vpos->y = m._42;
    vpos->z = m._43;
}

bool IntersectLine(const D3DXVECTOR2 &s1, const D3DXVECTOR2 &e1, const D3DXVECTOR2 &s2, const D3DXVECTOR2 &e2,
                   D3DXVECTOR2 &out) {
    float l1x = e1.x - s1.x;
    float l1y = e1.y - s1.y;
    float l2x = e2.x - s2.x;
    float l2y = e2.y - s2.y;
    float kofdel = l1y * l2x - l2y * l1x;
    if (fabs(kofdel) < 0.0001f)
        return false;

    out.x = ((s2.y - s1.y) * l1x * l2x + l1y * l2x * s1.x - l2y * l1x * s2.x) / kofdel;

    if (l1x != 0)
        out.y = ((out.x - s1.x) * l1y) / l1x + s1.y;
    else
        out.y = ((out.x - s2.x) * l2y) / l2x + s2.y;

    return true;
}

bool IntersectLine(const D3DXVECTOR2 &s1, const D3DXVECTOR2 &e1, const D3DXVECTOR2 &s2, const D3DXVECTOR2 &e2, float *t,
                   float *u) {
    D3DXVECTOR2 b = e1 - s1;
    D3DXVECTOR2 d = e2 - s2;
    D3DXVECTOR2 c = s2 - s1;

    float dtb = -d.y * b.x + d.x * b.y;
    if (fabs(dtb) < 0.0001f)
        return false;

    if (t)
        *t = (-d.y * c.x + d.x * c.y) / dtb;
    if (u)
        *u = (-b.y * c.x + b.x * c.y) / dtb;
    return true;
}

bool IsIntersectRect(float sx1, float sy1, float ex1, float ey1, float sx2, float sy2, float ex2, float ey2, float *sx,
                     float *sy) {
    float dfc_x = (ex1 + sx1) / 2.0f - (ex2 + sx2) / 2.0f;
    float dfc_y = (ey1 + sy1) / 2.0f - (ey2 + sy2) / 2.0f;

    float mr_x = ((ex1 - sx1) / 2.0f + (ex2 - sx2) / 2.0f);
    float mr_y = ((ey1 - sy1) / 2.0f + (ey2 - sy2) / 2.0f);

    *sx = 0.0f;
    *sy = 0.0f;

    if (fabs(dfc_x) < mr_x && fabs(dfc_y) < mr_y) {
        if (fabs(dfc_x) < mr_x) {
            if (dfc_x < 0)
                *sx = dfc_x + mr_x;
            else
                *sx = dfc_x - mr_x;
        }
        if (fabs(dfc_y) < mr_y) {
            if (dfc_y < 0)
                *sy = dfc_y + mr_y;
            else
                *sy = dfc_y - mr_y;
        }
        return true;
    }
    else
        return false;

    /*
    return	(float)fabs((ex1+sx1)/2.0f-(ex2+sx2)/2.0f)<
            ((ex1-sx1)/2.0f+(ex2-sx2)/2.0f) &&
            (float)fabs((ey1+sy1)/2.0f-(ey2+sy2)/2.0f)<
            ((ey1-sy1)/2.0f+(ey2-sy2)/2.0f);
    */
}

bool IsIntersectSphere(const D3DXVECTOR3 &center, float r, const D3DXVECTOR3 &s, const D3DXVECTOR3 &dir, float &t) {
    float dx = (s.x - center.x);
    float dy = (s.y - center.y);
    float dz = (s.z - center.z);

    float b, c;

    b = 2.0f * (dir.x * dx + dir.y * dy + dir.z * dz);
    c = (dx * dx) + (dy * dy) + (dz * dz) - (r * r);

    float d;

    d = (b * b) - (4.0f * c);
    if (d < 0) {
        //нет пересечения
        return false;
    }

    d = float(sqrt(d));

    float t1, t2;

    //Вычисяем t1 и t2 (Это длина луча из начальной точки
    t1 = (d - b) * 0.5f;
    t2 = (-d - b) * 0.5f;

    if (t1 < t2) {
        if (t1 < 0)
            t = t2;
        else
            t = t1;
    }
    else {
        if (t2 < 0)
            t = t1;
        else
            t = t2;
    }

    return true;
}

bool IsIntersectSphere(const D3DXVECTOR2 &center, float r, const D3DXVECTOR2 &s, const D3DXVECTOR2 &e, float &t1) {
    float r2 = r * r;
    auto tmp1 = center - s;
    auto tmp2 = center - e;
    if ((D3DXVec2LengthSq(&tmp1) < r2) && (D3DXVec2LengthSq(&tmp2) < r2))
        return false;

    D3DXVECTOR2 dir;
    auto tmp3 = e - s;
    D3DXVec2Normalize(&dir, &tmp3);

    float dx = (s.x - center.x);
    float dy = (s.y - center.y);

    float b, c;

    b = 2.0f * (dir.x * dx + dir.y * dy);
    c = (dx * dx) + (dy * dy) - (r2);

    float d;

    d = (b * b) - (4.0f * c);
    if (d < 0) {
        //нет пересечения
        return false;
    }

    d = float(sqrt(d));

    //Вычисяем t1 и t2 (Это длина луча из начальной точки
    t1 = (-b - d) * 0.5f;
    float t2 = (-b + d) * 0.5f;

    if (t1 < 0 && t2 < 0)
        return false;

    if (t1 < t2) {
        if (t1 < 0)
            t1 = t2;
    }
    else {
        if (t2 > 0)
            t1 = t2;
    }

    auto tmp4 = e - s;
    if ((t1 * t1) > (D3DXVec2LengthSq(&tmp4)))
        return false;

    return true;
}

void VecToMatrixX(D3DXMATRIX &mat, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &vec) {
    DTRACE();
    float len2 = (float)sqrt(vec.x * vec.x + vec.y * vec.y);
    float _1len2;
    float addx = 0.0f;
    if (len2 == 0.0f) {
        _1len2 = 1;
        addx = 1.0f;
    }
    else {
        _1len2 = 1.0f / len2;
    }

    mat._11 = vec.x;  // x1
    mat._12 = vec.y;  // y1
    mat._13 = vec.z;  // z1
    mat._14 = 0;

    mat._21 = -vec.y * _1len2 + addx;  // x2
    mat._22 = vec.x * _1len2;          // y2
    mat._23 = 0;                       // z2

    mat._24 = 0;

    // cross product
    // x3 = y1 * z2 - z1 * y2
    // y3 = z1 * x2 - x1 * z2
    // z3 = x1 * y2 - y1 * x2

    mat._31 = -mat._13 * mat._22;
    mat._32 = mat._13 * mat._21;
    mat._33 = mat._11 * mat._22 - mat._12 * mat._21;

    mat._34 = 0;

    mat._41 = pos.x;
    mat._42 = pos.y;
    mat._43 = pos.z;
    mat._44 = 1;
}

void VecToMatrixY(D3DXMATRIX &mat, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &vec) {
    DTRACE();

    float len2 = (float)sqrt(vec.x * vec.x + vec.y * vec.y);
    float _1len2;
    float addx = 0.0f;
    if (len2 == 0.0f) {
        _1len2 = 1;
        addx = 1.0f;
    }
    else {
        _1len2 = 1.0f / len2;
    }

    mat._21 = vec.x;  // x1
    mat._22 = vec.y;  // y1
    mat._23 = vec.z;  // z1
    mat._24 = 0;

    mat._11 = vec.y * _1len2 + addx;  // x2
    mat._12 = -vec.x * _1len2;        // y2
    mat._13 = 0;                      // z2
    mat._14 = 0;

    // cross product
    // x3 = y1 * z2 - z1 * y2
    // y3 = z1 * x2 - x1 * z2
    // z3 = x1 * y2 - y1 * x2

    mat._31 = mat._23 * mat._12;
    mat._32 = -mat._23 * mat._11;
    mat._33 = -mat._21 * mat._12 + mat._22 * mat._11;
    mat._34 = 0;

    mat._41 = pos.x;
    mat._42 = pos.y;
    mat._43 = pos.z;
    mat._44 = 1;
}

void BuildRotateMatrix(D3DXMATRIX &out, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, float angle) {
    float sn;
    float cs;
    SinCos(angle, &sn, &cs);

    float cs1 = 1.0f - cs;

    float lm = dir.x * dir.y;
    float ln = dir.x * dir.z;
    float mn = dir.y * dir.z;

    float lsinf = dir.x * sn;
    float msinf = dir.y * sn;
    float nsinf = dir.z * sn;

    out._11 = dir.x * dir.x * (1.0f - cs) + cs;
    out._12 = lm * cs1 + nsinf;
    out._13 = ln * cs1 - msinf;
    out._14 = 0;

    out._21 = lm * cs1 - nsinf;
    out._22 = dir.y * dir.y * (1.0f - cs) + cs;
    out._23 = mn * cs1 + lsinf;
    out._24 = 0;

    out._31 = ln * cs1 + msinf;
    out._32 = mn * cs1 - lsinf;
    out._33 = dir.z * dir.z * (1.0f - cs) + cs;
    out._34 = 0;

    out._41 = -pos.x * out._11 - pos.y * out._21 - pos.z * out._31 + pos.x;
    out._42 = -pos.x * out._12 - pos.y * out._22 - pos.z * out._32 + pos.y;
    out._43 = -pos.x * out._13 - pos.y * out._23 - pos.z * out._33 + pos.z;
    out._44 = 1.0f;
}

D3DXVECTOR3 Vec3Projection(const D3DXVECTOR3 &a, const D3DXVECTOR3 &b) {
    return (a * (D3DXVec3Dot(&a, &b) / D3DXVec3LengthSq(&a)));
}

void CalcBSplineKoefs1(SBSplineKoefs &out, const D3DXVECTOR3 &p0, const D3DXVECTOR3 &p1, const D3DXVECTOR3 &p2,
                       const D3DXVECTOR3 &p3) {
    out.pos0.x = (4 * p1.x + p0.x + p2.x) * float(1.0 / 6.0);
    out.a1 = (p2.x - p0.x) * 0.5f;
    out.a2 = (p0.x + p2.x) * 0.5f - p1.x;
    out.a3 = 0.5f * (p1.x - p2.x) + (p3.x - p0.x) * float(1.0 / 6.0);

    out.pos0.y = (4 * p1.y + p0.y + p2.y) * float(1.0 / 6.0);
    out.b1 = (p2.y - p0.y) * 0.5f;
    out.b2 = (p0.y + p2.y) * 0.5f - p1.y;
    out.b3 = 0.5f * (p1.y - p2.y) + (p3.y - p0.y) * float(1.0 / 6.0);

    out.pos0.z = (4 * p1.z + p0.z + p2.z) * float(1.0 / 6.0);
    out.c1 = (p2.z - p0.z) * 0.5f;
    out.c2 = (p0.z + p2.z) * 0.5f - p1.z;
    out.c3 = 0.5f * (p1.z - p2.z) + (p3.z - p0.z) * float(1.0 / 6.0);
}

void CalcBSplineKoefs2(SBSplineKoefs &out, const D3DXVECTOR3 &p0, const D3DXVECTOR3 &p1, const D3DXVECTOR3 &p2,
                       const D3DXVECTOR3 &p3) {
    // Ap.X := -CtrlPt[N-1].X + 3*CtrlPt[N].X - 3*CtrlPt[N+1].X + CtrlPt[N+2].X;
    // Bp.X := 2*CtrlPt[N-1].X - 5*CtrlPt[N].X + 4*CtrlPt[N+1].X - CtrlPt[N+2].X;
    // Cp.X := -CtrlPt[N-1].X + CtrlPt[N+1].X;
    // Dp.X := 2*CtrlPt[N].X;

    out.pos0 = p1;

    out.a1 = (p2.x - p0.x) * 0.5f;
    out.a2 = p0.x - 2.5f * p1.x + 2.0f * p2.x - 0.5f * p3.x;
    out.a3 = 1.5f * (p1.x - p2.x) + (p3.x - p0.x) * 0.5f;

    out.b1 = (p2.y - p0.y) * 0.5f;
    out.b2 = p0.y - 2.5f * p1.y + 2.0f * p2.y - 0.5f * p3.y;
    out.b3 = 1.5f * (p1.y - p2.y) + (p3.y - p0.y) * 0.5f;

    out.c1 = (p2.z - p0.z) * 0.5f;
    out.c2 = p0.z - 2.5f * p1.z + 2.0f * p2.z - 0.5f * p3.z;
    out.c3 = 1.5f * (p1.z - p2.z) + (p3.z - p0.z) * 0.5f;
}

void CTrajectory::Init1(const D3DXVECTOR3 *points, int pcnt) {
    DTRACE();

    ASSERT(pcnt >= 4);
    m_SegCnt = pcnt - 1;

    if (m_Segments) {
        if (m_SegCnt > m_SegAlloc) {
        alloc:
            m_Segments = (STrajectorySegment *)HAllocEx(m_Segments, sizeof(STrajectorySegment) * m_SegCnt, m_Heap);
            m_SegAlloc = m_SegCnt;
        }
    }
    else
        goto alloc;

    for (int i = 0; i < m_SegCnt; ++i) {
        int i0 = std::max(i - 1, 0);
        int i1 = i;
        int i2 = i + 1;
        int i3 = std::min(i + 2, pcnt - 1);

        m_Segments[i].Build1(points[i0], points[i1], points[i2], points[i3]);
    }
    m_CurSeg = 0;
    m_CurSegT = 0;
}
void CTrajectory::Init2(const D3DXVECTOR3 *points, int pcnt) {
    DTRACE();

    ASSERT(pcnt >= 4);
    m_SegCnt = pcnt - 1;

    if (m_Segments) {
        if (m_SegCnt > m_SegAlloc) {
        alloc:
            m_Segments = (STrajectorySegment *)HAllocEx(m_Segments, sizeof(STrajectorySegment) * m_SegCnt, m_Heap);
            m_SegAlloc = m_SegCnt;
        }
    }
    else
        goto alloc;

    for (int i = 0; i < m_SegCnt; ++i) {
        int i0 = std::max(i - 1, 0);
        int i1 = i;
        int i2 = i + 1;
        int i3 = std::min(i + 2, pcnt - 1);

        m_Segments[i].Build2(points[i0], points[i1], points[i2], points[i3]);
    }
    m_CurSeg = 0;
    m_CurSegT = 0;
}

void CTrajectory::CutBeforeCurrent(void) {
    if (m_CurSeg == 0)
        return;
    m_SegCnt -= m_CurSeg;
    memcpy(m_Segments, m_Segments + m_CurSeg, sizeof(STrajectorySegment) * m_SegCnt);
    m_CurSeg = 0;
    m_Len = -1;
}

float CTrajectory::CutBefore(float t) {
    if (m_SegCnt == 1)
        return t;
    float tseg = t * float(m_SegCnt);
    int seg = TruncFloat(tseg);
    if (seg <= 0)
        return t;
    if (seg >= m_SegCnt) {
        m_SegCnt = 1;
        memcpy(m_Segments, m_Segments + m_SegCnt - 1, sizeof(STrajectorySegment));
        return 1;
    }
    else {
        m_SegCnt -= seg;
        memcpy(m_Segments, m_Segments + seg, sizeof(STrajectorySegment) * m_SegCnt);
        return tseg - seg;
    }
    m_Len = -1;
}

void CTrajectory::Move(float dist) {
    if (m_CurSegLen < 0) {
        m_CurSegLen = 0;
        const float dt = 0.01f;

        D3DXVECTOR3 p0, p1;
        CalcBSplinePoint(m_Segments[m_CurSeg].koefs, p1, 0);

        float t = dt;
        while (t <= 1.0f) {
            p0 = p1;
            CalcBSplinePoint(m_Segments[m_CurSeg].koefs, p1, t);
            t += dt;
            auto tmp = p1 - p0;
            m_CurSegLen += D3DXVec3Length(&tmp);
        }
    }

    if (m_CurSegLen < 0.0001f) {
        m_CurSegT = 1.0f;
    }
    else {
        m_CurSegT += dist / m_CurSegLen;
    }

    while (m_CurSegT >= 1.0f) {
        if (m_CurSeg >= (m_SegCnt - 1)) {
            m_CurSegT = 1.0f;
            return;
        }
        float ost = (m_CurSegT - 1.0f) * m_CurSegLen;

        ++m_CurSeg;

        m_CurSegLen = 0;
        const float dt = 0.01f;

        D3DXVECTOR3 p0, p1;
        CalcBSplinePoint(m_Segments[m_CurSeg].koefs, p1, 0);

        float t = dt;
        while (t <= 1.0f) {
            p0 = p1;
            CalcBSplinePoint(m_Segments[m_CurSeg].koefs, p1, t);
            t += dt;
            auto tmp = p1 - p0;
            m_CurSegLen += D3DXVec3Length(&tmp);
        }

        if (m_CurSegLen < 0.0001f) {
            m_CurSegT = 0.9f;
            break;
        }
        else {
            m_CurSegT = ost / m_CurSegLen;
        }
    }

#ifdef _DEBUG
    if (_isnan(m_CurSegT))
        debugbreak();
#endif
}

void CTrajectory::Continue1(const D3DXVECTOR3 *points, int pcnt) {
    m_Len = -1;

    int newsegcnt = (m_SegCnt + pcnt + 1);

    if (newsegcnt > m_SegAlloc) {
        m_Segments = (STrajectorySegment *)HAllocEx(m_Segments, sizeof(STrajectorySegment) * newsegcnt, m_Heap);
        m_SegAlloc = newsegcnt;
    }

    const D3DXVECTOR3 *p = m_Segments[m_SegCnt - 1].based_on;
    m_Segments[m_SegCnt].Build1(p[1], p[2], p[3], points[0]);

    for (int i = 1; i <= pcnt; ++i) {
        int i0 = i - 3;
        int i1 = i - 2;
        int i2 = i - 1;
        int i3 = std::min(i, pcnt - 1);

        m_Segments[i + m_SegCnt].Build1((i0 >= 0) ? points[i0] : p[4 + i0], (i1 >= 0) ? points[i1] : p[4 + i1],
                                        points[i2], points[i3]);
    }

    m_SegCnt = newsegcnt;
}

void CTrajectory::Continue2(const D3DXVECTOR3 *points, int pcnt) {
    m_Len = -1;

    int newsegcnt = (m_SegCnt + pcnt + 1);

    if (newsegcnt > m_SegAlloc) {
        m_Segments = (STrajectorySegment *)HAllocEx(m_Segments, sizeof(STrajectorySegment) * newsegcnt, m_Heap);
        m_SegAlloc = newsegcnt;
    }

    const D3DXVECTOR3 *p = m_Segments[m_SegCnt - 1].based_on;
    m_Segments[m_SegCnt].Build2(p[1], p[2], p[3], points[0]);

    for (int i = 1; i <= pcnt; ++i) {
        int i0 = i - 3;
        int i1 = i - 2;
        int i2 = i - 1;
        int i3 = std::min(i, pcnt - 1);

        m_Segments[i + m_SegCnt].Build2((i0 >= 0) ? points[i0] : p[4 + i0], (i1 >= 0) ? points[i1] : p[4 + i1],
                                        points[i2], points[i3]);
    }

    m_SegCnt = newsegcnt;
}

void CTrajectory::CalcPoint(D3DXVECTOR3 &out, float t) {
    float tseg = t * float(m_SegCnt);
    int seg = TruncFloat(tseg);

    if (seg < 0)
        CalcBSplinePoint(m_Segments[0].koefs, out, 0);
    else if (seg >= m_SegCnt)
        CalcBSplinePoint(m_Segments[m_SegCnt - 1].koefs, out, 1.0f);
    else
        CalcBSplinePoint(m_Segments[seg].koefs, out, tseg - float(seg));
}

float CTrajectory::CalcLength(void) {
    if (m_Len >= 0)
        return m_Len;
    m_Len = 0;
    const float dt = 0.01f;

    D3DXVECTOR3 p0, p1;
    CalcPoint(p1, 0);

    float t = dt;
    while (t <= 1.0f) {
        p0 = p1;
        CalcPoint(p1, t);
        t += dt;
        auto tmp = p1 - p0;
        m_Len += D3DXVec3Length(&tmp);
    }

    return m_Len;
}
