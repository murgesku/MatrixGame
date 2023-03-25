// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include <cmath>

#include "d3d9.h"
#include "d3dx9tex.h"

#include "CHeap.hpp"
#include "CMain.hpp"
#include "CException.hpp"

using namespace Base; // TODO: remove

#define pi_f    3.14159265358979f
#define pi_     3.14159265358979
#define ToRad   (pi_f / 180.0f)
#define ToGrad  (180.0f / pi_f)
#define ToRad_  (pi_ / 180.0)
#define ToGrad_ (180.0 / pi_)

#define GRAD2RAD(a) ((a)*ToRad)

#define M_PI        3.14159265358979323846
#define M_PI_MUL(x) float((x)*M_PI)

#define POW2(x) ((x) * (x))

#define LERPFLOAT(k, c1, c2)  (((k) * (float(c2) - float(c1))) + float(c1))
#define LERPVECTOR(k, v1, v2) (((k) * ((v2) - (v1))) + (v1))

#define RND(from, to) ((double)rand() * (1.0 / (RAND_MAX)) * (fabs(double((to) - (from)))) + (from))
#define FRND(x)       ((float)(RND(0, (x))))
#define FSRND(x)      (FRND(2.0f * (x)) - float(x))
#define IRND(n)       Double2Int(RND(0, double(n) - 0.55))

#define KSCALE(k, k1, k2) ((k) < (k1) ? 0.0f : (k) > (k2) ? 1.0f : ((k) - (k1)) / ((k2) - (k1)))

#define INVERT(c) (float(1.0 / (c)))

#define FP_ONE_BITS 0x3F800000

// NVidia stuff

// r = 1/p
#define FP_INV(r, p)                             \
    {                                            \
        int _i = 2 * FP_ONE_BITS - *(int *)&(p); \
        r = *(float *)&_i;                       \
        r = r * (2.0f - (p)*r);                  \
    }

inline unsigned char FP_NORM_TO_BYTE2(float p) {
    float fpTmp = p + 1.0f;
    return ((*(unsigned *)&fpTmp) >> 15) & 0xFF;
}

inline unsigned char FP_NORM_TO_BYTE3(float p) {
    float ftmp = p + 12582912.0f;
    return (BYTE)((*(unsigned long *)&ftmp) & 0xFF);
}

inline int DetermineGreaterPowerOfTwo(int val) {
    int num = 1;
    while (val > num) {
        num <<= 1;
    }

    return num;
}

struct SPlane {
    // TODO: fix it properly
    // union {
    //     struct {
    //         D3DXPLANE dxplane;
    //     };
    //     struct {
    //         D3DXVECTOR3 norm;
    //         float dist;
    //     };
    // };

    D3DXPLANE dxplane;
    D3DXVECTOR3 norm;
    float dist;
    byte signbits;

    SPlane() {}

    static inline void BuildFromPointNormal(SPlane &out, const D3DXVECTOR3 &pt, const D3DXVECTOR3 &norm) {
        D3DXPlaneFromPointNormal(&out.dxplane, &pt, &norm);
        out.UpdateSignBits();
    }

    static inline void BuildFromPoints(SPlane &out, const D3DXVECTOR3 &p0, const D3DXVECTOR3 &p1,
                                              const D3DXVECTOR3 &p2) {
        D3DXPlaneFromPoints(&out.dxplane, &p0, &p1, &p2);
        out.UpdateSignBits();
    }

    inline void UpdateSignBits(void) {
        signbits = ((norm.x < 0) ? 1 : 0) | ((norm.y < 0) ? 2 : 0) | ((norm.z < 0) ? 4 : 0);
    }
    inline bool IsOnSide(const D3DXVECTOR3 &p) const { return D3DXVec3Dot(&p, &norm) >= (-dist); }
    inline float CalcPointDist(const D3DXVECTOR3 &p) const { return D3DXVec3Dot(&p, &norm); }
    inline float Distance(const D3DXVECTOR3 &p) const { return D3DXVec3Dot(&p, &norm) + dist; }
    inline byte BoxSide(const D3DXVECTOR3 &mins, const D3DXVECTOR3 &maxs) const {
        float dist1, dist2;
        switch (signbits) {
            case 0:
                dist1 = CalcPointDist(maxs);
                dist2 = CalcPointDist(mins);
                break;
            case 1:
                dist1 = CalcPointDist(D3DXVECTOR3(mins.x, maxs.y, maxs.z));
                dist2 = CalcPointDist(D3DXVECTOR3(maxs.x, mins.y, mins.z));
                break;
            case 2:
                dist1 = CalcPointDist(D3DXVECTOR3(maxs.x, mins.y, maxs.z));
                dist2 = CalcPointDist(D3DXVECTOR3(mins.x, maxs.y, mins.z));
                break;
            case 3:
                dist1 = CalcPointDist(D3DXVECTOR3(mins.x, mins.y, maxs.z));
                dist2 = CalcPointDist(D3DXVECTOR3(maxs.x, maxs.y, mins.z));
                break;
            case 4:
                dist1 = CalcPointDist(D3DXVECTOR3(maxs.x, maxs.y, mins.z));
                dist2 = CalcPointDist(D3DXVECTOR3(mins.x, mins.y, maxs.z));
                break;
            case 5:
                dist1 = CalcPointDist(D3DXVECTOR3(mins.x, maxs.y, mins.z));
                dist2 = CalcPointDist(D3DXVECTOR3(maxs.x, mins.y, maxs.z));
                break;
            case 6:
                dist1 = CalcPointDist(D3DXVECTOR3(maxs.x, mins.y, mins.z));
                dist2 = CalcPointDist(D3DXVECTOR3(mins.x, maxs.y, maxs.z));
                break;
            case 7:
                dist1 = CalcPointDist(mins);
                dist2 = CalcPointDist(maxs);
                break;
            default:
                ERROR_E;
        }
        byte sides = 0;
        if (dist1 >= -dist)
            sides = 1;
        if (dist2 < -dist)
            sides |= 2;
        return sides;
    }

    inline bool FindIntersect(const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, float &outt) const {
        float cs = D3DXVec3Dot(&dir, &norm);
        if (fabs(cs) < 0.00001f)
            return false;
        outt = Distance(pos) / -cs;
        return true;
    }
};

// PURPOSE		: Проверяет с какой стороны линии находится точка
// RETURN VALUE	: True если справа или на линии, False в остальных случаях
inline bool PointLineCatch(const D3DXVECTOR2 &s, const D3DXVECTOR2 &e, const D3DXVECTOR2 &p) {
    D3DXVECTOR2 d1, d2;

    d1.x = e.x - s.x;
    d1.y = e.y - s.y;
    d2.x = p.x - e.x;
    d2.y = p.y - e.y;

    float res = d1.x * d2.y - d1.y * d2.x;

    return res > 0;
}

// PURPISE		:Проверят попала ли точка внутрь 2D AABB
// RETURN VALUE	: True если попала
inline bool PointToAABB(const D3DXVECTOR2 &AABB_pos, const D3DXVECTOR2 &p, int nAABB_height, int nAABB_width) {
    return ((p.x >= AABB_pos.x && p.x <= AABB_pos.x + nAABB_width) &&
            ((p.y >= AABB_pos.y && p.y <= AABB_pos.y + nAABB_height)));
}

D3DXVECTOR3 Vec3Projection(/*where*/ const D3DXVECTOR3 &a, /*what*/ const D3DXVECTOR3 &b);

inline float DistOtrezokPoint(const D3DXVECTOR3 &p0, const D3DXVECTOR3 &p1, const D3DXVECTOR3 &p) {
    D3DXVECTOR3 v(p1 - p0);
    D3DXVECTOR3 w(p - p0);
    float c1 = D3DXVec3Dot(&w, &v);
    if (c1 <= 0)
    {
        auto tmp = p0 - p;
        return D3DXVec3Length(&tmp);
    }
    float c2 = D3DXVec3Dot(&v, &v);
    if (c2 <= c1)
    {
        auto tmp = p1 - p;
        return D3DXVec3Length(&tmp);
    }
    float b = c1 / c2;

    auto tmp = p - (p0 + b * v);
    return D3DXVec3Length(&tmp);
}

bool IntersectTriangle(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &v0, const D3DXVECTOR3 &v1,
                       const D3DXVECTOR3 &v2, float &t, float &u, float &v);
float DistLinePoint(const D3DXVECTOR3 &l1, const D3DXVECTOR3 &l2, const D3DXVECTOR3 &p);
void CalcPick(CPoint mp, D3DXMATRIX &matProj, D3DXMATRIX &matView, D3DXVECTOR3 *vpos, D3DXVECTOR3 *vdir);
bool IntersectLine(const D3DXVECTOR2 &s1, const D3DXVECTOR2 &e1, const D3DXVECTOR2 &s2, const D3DXVECTOR2 &e2,
                   D3DXVECTOR2 &out);  // точка пересечения=out
bool IntersectLine(const D3DXVECTOR2 &s1, const D3DXVECTOR2 &e1, const D3DXVECTOR2 &s2, const D3DXVECTOR2 &e2, float *t,
                   float *u);  // точка пересечения=s1+(e1-s1)*t или s2+(e2-s2)*u.  Если t>=0 and t<=1.0 то точка
                               // пересечения между s1 и e1

bool IsIntersectRect(float sx1, float sy1, float ex1, float ey1, float sx2, float sy2, float ex2, float ey2, float *sx,
                     float *sy);  // True - если два прямоугольника пересекаются sx,sy - направление и величина заступа

bool IsIntersectSphere(const D3DXVECTOR3 &center, float r, const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, float &t);
bool IsIntersectSphere(const D3DXVECTOR2 &center, float r, const D3DXVECTOR2 &s, const D3DXVECTOR2 &e, float &t1);

inline double AngleNorm(double a)  // Нормировать угол. Возращает от -pi до +pi
{
    while (a > pi_f)
        a -= 2.0 * M_PI;
    while (a <= -pi_f)
        a += 2.0 * M_PI;
    return a;
}

inline double AngleDist(double from, double to)  // Дистанция между углами. Возращает от -pi до +pi
{
    while (from < 0.0)
        from += 2.0 * M_PI;
    while (to < 0.0)
        to += 2.0 * M_PI;

    double r = to - from;

    if (from < M_PI) {
        if (r > M_PI)
            r -= 2.0 * M_PI;
    }
    else {
        if (r < -M_PI)
            r += 2.0 * M_PI;
    }

    return r;
}

// convert pos and direction to matrix
void VecToMatrixX(D3DXMATRIX &mat, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir);
void VecToMatrixY(D3DXMATRIX &mat, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir);

void BuildRotateMatrix(D3DXMATRIX &out, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir, float angle);

// CROSS PRODUCT
#define CROSS_X(v1, v2)       ((v1).y * (v2).z - (v1).z * (v2).y)
#define CROSS_Y(v1, v2)       ((v1).z * (v2).x - (v1).x * (v2).z)
#define CROSS_Z(v1, v2)       ((v1).x * (v2).y - (v1).y * (v2).x)
#define CROSS_PRODUCT(v1, v2) D3DXVECTOR3(CROSS_X(v1, v2), CROSS_Y(v1, v2), CROSS_Z(v1, v2))

#define IS_ZERO_VECTOR(v) \
    ((((*((DWORD *)&(v)) | *(((DWORD *)&(v)) + 1) | *(((DWORD *)&(v)) + 2))) & 0x7FFFFFFF) == 0x00000000)

#define NEG_FLOAT(x)       (*((DWORD *)&(x))) ^= 0x80000000;
#define MAKE_ABS_FLOAT(x)  (*((DWORD *)&(x))) &= 0x7FFFFFFF;
#define MAKE_ABS_DOUBLE(x) (*(((DWORD *)&(x)) + 1)) &= 0x7FFFFFFF;
#define COPY_SIGN_FLOAT(to, from) \
    { (*((DWORD *)&(to))) = ((*((DWORD *)&(to))) & 0x7FFFFFFF) | ((*((DWORD *)&(from))) & 0x80000000); }
#define SET_SIGN_FLOAT(to, sign) \
    { *((DWORD *)&(to)) = ((*((DWORD *)&(to))) & 0x7FFFFFFF) | ((DWORD(sign)) << 31); }

inline bool IsVec3Equal(const D3DXVECTOR3 &v0, const D3DXVECTOR3 &v1, float dopusk = 0.0001f) {
    if (fabs(v0.x - v1.x) > dopusk)
        return false;
    if (fabs(v0.y - v1.y) > dopusk)
        return false;
    if (fabs(v0.z - v1.z) > dopusk)
        return false;
    return true;
}

void inline SinCos(float radians, float *sine, float *cosine) {
    *sine = std::sin(radians);
    *cosine = std::cos(radians);
}

#define SIN_TABLE_SIZE 256
#define FTOIBIAS       12582912.f
extern float SinCosTable[SIN_TABLE_SIZE];

inline float TableCos(float theta) {
    union {
        int i;
        float f;
    } ftmp;

    // ideally, the following should compile down to: theta * constant + constant, changing any of these constants from
    // defines sometimes fubars this.
    ftmp.f = theta * (float)(SIN_TABLE_SIZE / (2.0f * M_PI)) + (FTOIBIAS + (SIN_TABLE_SIZE / 4));
    return SinCosTable[ftmp.i & (SIN_TABLE_SIZE - 1)];
}

inline float TableSin(float theta) {
    union {
        int i;
        float f;
    } ftmp;

    // ideally, the following should compile down to: theta * constant + constant
    ftmp.f = theta * (float)(SIN_TABLE_SIZE / (2.0f * M_PI)) + FTOIBIAS;
    return SinCosTable[ftmp.i & (SIN_TABLE_SIZE - 1)];
}

// TODO: replace this funcs with std calls inplace
inline int TruncFloat(float x) {
    return static_cast<int>(std::trunc(x));
}
inline int TruncDouble(double x) {
    return static_cast<int>(std::trunc(x));
}
inline int Float2Int(float x) {
    return static_cast<int>(std::round(x));
}
inline int Double2Int(double x) {
    return static_cast<int>(std::round(x));
}

inline float GetColorA(DWORD c) {
    return float((c >> 24) & 255) / 255.0f;
}
inline float GetColorR(DWORD c) {
    return float((c >> 16) & 255) / 255.0f;
}
inline float GetColorG(DWORD c) {
    return float((c >> 8) & 255) / 255.0f;
}
inline float GetColorB(DWORD c) {
    return float((c >> 0) & 255) / 255.0f;
}

inline DWORD LIC(DWORD c0, DWORD c1, float t) {
    DWORD c = 0;

    c |= 0xFF & Float2Int((0xFF & c0) + (int(0xFF & c1) - int(0xFF & c0)) * t);

    c0 >>= 8;
    c1 >>= 8;
    c |= (0xFF & Float2Int((0xFF & c0) + (int(0xFF & c1) - int(0xFF & c0)) * t)) << 8;

    c0 >>= 8;
    c1 >>= 8;
    c |= (0xFF & Float2Int((0xFF & c0) + (int(0xFF & c1) - int(0xFF & c0)) * t)) << 16;

    c0 >>= 8;
    c1 >>= 8;
    c |= (0xFF & Float2Int((0xFF & c0) + (int(0xFF & c1) - int(0xFF & c0)) * t)) << 24;
    return c;
}

inline void AddSphereToBox(D3DXVECTOR3 &mins, D3DXVECTOR3 &maxs, const D3DXVECTOR3 &center, float radius) {
    float minx = center.x - radius;
    float miny = center.y - radius;
    float minz = center.z - radius;
    float maxx = center.x + radius;
    float maxy = center.y + radius;
    float maxz = center.z + radius;

    if (minx < mins.x)
        mins.x = minx;
    if (miny < mins.y)
        mins.y = miny;
    if (minz < mins.z)
        mins.z = minz;

    if (maxx > maxs.x)
        maxs.x = maxx;
    if (maxy > maxs.y)
        maxs.y = maxy;
    if (maxz > maxs.z)
        maxs.z = maxz;
}

inline D3DXVECTOR2 RotatePoint(D3DXVECTOR2 &point, float angle) {
    float s, c;
    D3DXVECTOR2 p;
    SinCos(angle, &s, &c);
    p.x = c * point.x - s * point.y;
    p.y = s * point.x + c * point.y;
    return p;
}

inline D3DXVECTOR3 Vec3Truncate(const D3DXVECTOR3 &vec3, float tValue) {
    float sq = D3DXVec3LengthSq(&vec3);
    float sq2 = tValue * tValue;

    if (sq <= sq2) {
        return vec3;
    }
    else {
        return vec3 * (tValue / float(sqrt(sq)));
    }
}

inline void Vec2Truncate(D3DXVECTOR2 &vec2, float tValue) {
    float sq = D3DXVec2LengthSq(&vec2);
    float sq2 = tValue * tValue;

    if (sq > sq2) {
        vec2 *= float(tValue / sqrt(sq));
    }
}

struct SBSplineKoefs {
    D3DXVECTOR3 pos0;
    float a1, a2, a3;
    float b1, b2, b3;
    float c1, c2, c3;
};

void CalcBSplineKoefs1(SBSplineKoefs &out, const D3DXVECTOR3 &p0, const D3DXVECTOR3 &p1, const D3DXVECTOR3 &p2,
                       const D3DXVECTOR3 &p3);
void CalcBSplineKoefs2(SBSplineKoefs &out, const D3DXVECTOR3 &p0, const D3DXVECTOR3 &p1, const D3DXVECTOR3 &p2,
                       const D3DXVECTOR3 &p3);
inline void CalcBSplinePoint(const SBSplineKoefs &k, D3DXVECTOR3 &out, float t) {
    out.x = ((k.a3 * t + k.a2) * t + k.a1) * t + k.pos0.x;
    out.y = ((k.b3 * t + k.b2) * t + k.b1) * t + k.pos0.y;
    out.z = ((k.c3 * t + k.c2) * t + k.c1) * t + k.pos0.z;
}

class CTrajectory : public CMain {
    CHeap *m_Heap;
    // float   m_TPerSeg;

    float m_Len;  // if < 0 then not calculated

    struct STrajectorySegment {
        SBSplineKoefs koefs;
        D3DXVECTOR3 based_on[4];

        void Build1(const D3DXVECTOR3 &p0, const D3DXVECTOR3 &p1, const D3DXVECTOR3 &p2, const D3DXVECTOR3 &p3) {
            based_on[0] = p0;
            based_on[1] = p1;
            based_on[2] = p2;
            based_on[3] = p3;
            CalcBSplineKoefs1(koefs, p0, p1, p2, p3);
        }

        void Build2(const D3DXVECTOR3 &p0, const D3DXVECTOR3 &p1, const D3DXVECTOR3 &p2, const D3DXVECTOR3 &p3) {
            based_on[0] = p0;
            based_on[1] = p1;
            based_on[2] = p2;
            based_on[3] = p3;
            CalcBSplineKoefs2(koefs, p0, p1, p2, p3);
        }

    } * m_Segments;
    int m_SegCnt;
    int m_SegAlloc;

    int m_CurSeg;
    float m_CurSegT;

    float m_CurSegLen;

public:
    void Init1(const D3DXVECTOR3 *points, int pcnt);  // примерно
    void Init2(const D3DXVECTOR3 *points, int pcnt);  // точно

    void Continue1(const D3DXVECTOR3 *points, int pcnt);  // примерно
    void Continue2(const D3DXVECTOR3 *points, int pcnt);  // точно

    CTrajectory(void) : m_CurSeg(0), m_CurSegT(0), m_SegAlloc(0), m_SegCnt(0), m_CurSegLen(-1) {
        m_Segments = NULL;
        m_Len = -1;
    };
    CTrajectory(CHeap *heap) : m_CurSeg(0), m_CurSegT(0), m_SegAlloc(0), m_SegCnt(0), m_CurSegLen(-1) {
        m_Heap = heap;
        m_Segments = NULL;
        m_Len = -1;
    };
    CTrajectory(CHeap *heap, D3DXVECTOR3 *points, int pcnt)
      : m_Heap(heap), m_Len(-1), m_Segments(NULL), m_CurSegLen(-1) {
        Init2(points, pcnt);
    }  // init2 // точно

    ~CTrajectory() {
        if (m_Segments) {
            HFree(m_Segments, m_Heap);
        }
    };

    float CutBefore(float t);
    void CutBeforeCurrent(void);
    void CutAfterCurrent(void) {
        m_SegCnt = m_CurSeg + 1;
        m_Len = -1;
    }

    inline bool NeedContinue(float t) const {
        float tseg = t * float(m_SegCnt);
        int seg = TruncFloat(tseg);
        return seg >= (m_SegCnt - 2);
    }
    inline bool NeedContinue(void) const { return m_CurSeg >= (m_SegCnt - 2); }

    inline void GetCurPos(D3DXVECTOR3 &pos) { CalcBSplinePoint(m_Segments[m_CurSeg].koefs, pos, m_CurSegT); }

    void Move(float dist);

    float CalcLength(void);

    void CalcPoint(D3DXVECTOR3 &out, float t);
};

#define BASE_MATH_DEFINED
