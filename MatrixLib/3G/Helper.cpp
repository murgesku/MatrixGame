// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "Helper.hpp"

#include "CHeap.hpp"
#include "Tracer.hpp"
#include "3g.hpp"

#if (defined _DEBUG) && !(defined _RELDEBUG)

const float pi = 3.14159265358979f;

CHelper *CHelper::m_First;
CHelper *CHelper::m_Last;

void CHelper::StaticInit(void) {
    m_First = NULL;
    m_Last = NULL;
}

CHelper::CHelper(int del_after_cnt_draw, int group) : CMain() {
    DTRACE();

    m_Prev = NULL;
    m_Next = NULL;
    m_Parent = NULL;

    D3DXMatrixIdentity(&m_Matrix);

    m_DeleteAfterCntDraw = del_after_cnt_draw;
    m_Group = group;

    m_Type = D3DPT_POINTLIST;
    m_CntPrimitive = 0;
    m_CntVertex = 0;
    m_CntIndex = 0;
    m_Vertex = NULL;
    m_Index = NULL;

    LIST_ADD(this, m_First, m_Last, m_Prev, m_Next);
}

CHelper::~CHelper() {
    DTRACE();

    Clear();

    LIST_DEL(this, m_First, m_Last, m_Prev, m_Next);
}

void CHelper::Clear() {
    DTRACE();

    m_Type = D3DPT_POINTLIST;
    m_CntPrimitive = 0;
    m_CntVertex = 0;
    if (m_Vertex) {
        HFree(m_Vertex, g_CacheHeap);
        m_Vertex = NULL;
    }
    if (m_Index) {
        HFree(m_Index, g_CacheHeap);
        m_Index = NULL;
    }
}

void CHelper::Line(const D3DXVECTOR3 &start, const D3DXVECTOR3 &end, DWORD color1, DWORD color2) {
    DTRACE();

    Clear();

    m_Type = D3DPT_LINELIST;

    m_CntPrimitive = 1;
    m_CntVertex = 2;
    m_Vertex = (SHelperVertex *)HAllocClear(m_CntVertex * sizeof(SHelperVertex), g_CacheHeap);

    m_Vertex[0].v = start;
    m_Vertex[0].defcol = color1;

    m_Vertex[1].v = end;
    m_Vertex[1].defcol = color2;
}

void CHelper::BoundBox(const D3DXVECTOR3 &mins, const D3DXVECTOR3 &maxs, DWORD color) {
    DTRACE();

    Clear();

    m_Type = D3DPT_LINELIST;

    m_CntPrimitive = 12;
    m_CntVertex = 24;
    m_Vertex = (SHelperVertex *)HAllocClear(m_CntVertex * sizeof(SHelperVertex), g_CacheHeap);

    int i = 0;

    m_Vertex[i].v = D3DXVECTOR3(mins.x, mins.y, mins.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(maxs.x, mins.y, mins.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(mins.x, mins.y, maxs.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(maxs.x, mins.y, maxs.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(mins.x, maxs.y, maxs.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(maxs.x, maxs.y, maxs.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(mins.x, maxs.y, mins.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(maxs.x, maxs.y, mins.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(mins.x, mins.y, mins.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(mins.x, maxs.y, mins.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(mins.x, mins.y, maxs.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(mins.x, maxs.y, maxs.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(maxs.x, mins.y, maxs.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(maxs.x, maxs.y, maxs.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(maxs.x, mins.y, mins.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(maxs.x, maxs.y, mins.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(mins.x, mins.y, mins.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(mins.x, mins.y, maxs.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(mins.x, maxs.y, mins.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(mins.x, maxs.y, maxs.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(maxs.x, maxs.y, mins.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(maxs.x, maxs.y, maxs.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(maxs.x, mins.y, mins.z);
    m_Vertex[i].defcol = color;
    ++i;

    m_Vertex[i].v = D3DXVECTOR3(maxs.x, mins.y, maxs.z);
    m_Vertex[i].defcol = color;
    ++i;
}

SHelperVertex *CHelper::Lines(int cnt) {
    DTRACE();

    Clear();

    m_Type = D3DPT_LINELIST;

    m_CntPrimitive = cnt;
    m_CntVertex = cnt * 2;
    m_Vertex = (SHelperVertex *)HAllocClear(m_CntVertex * sizeof(SHelperVertex), g_CacheHeap);

    return m_Vertex;
}

SHelperVertex *CHelper::LineStrip(int cnt) {
    DTRACE();

    Clear();

    m_Type = D3DPT_LINESTRIP;

    m_CntPrimitive = cnt;
    m_CntVertex = cnt * 2 - 2;
    m_Vertex = (SHelperVertex *)HAllocClear(m_CntVertex * sizeof(SHelperVertex), g_CacheHeap);

    return m_Vertex;
}

void CHelper::Sphere(const D3DXVECTOR3 &pos, float radius, int cnt_rings, DWORD color) {
    DTRACE();

    SetMatrixPos(pos);
    word x, y, vtx, index;
    float fDAng;
    float fDAngY0;
    float y0, r0, tv, fDAngX0, tu;
    D3DXVECTOR3 v, vy;
    dword wNorthVtx, wSouthVtx;
    word p1, p2, p3;

    m_CntVertex = (cnt_rings * (2 * cnt_rings + 1) + 2);
    m_CntIndex = 6 * (cnt_rings * 2) * ((cnt_rings - 1) + 1);

    m_Vertex = (SHelperVertex *)HAllocClear(m_CntVertex * sizeof(SHelperVertex), g_CacheHeap);
    m_Index = (word *)HAllocClear(m_CntIndex * sizeof(word), g_CacheHeap);

    m_Type = D3DPT_TRIANGLELIST;

    vtx = 0;
    index = 0;
    fDAng = pi / cnt_rings;
    fDAngY0 = fDAng;

    for (y = 0; y < cnt_rings; y++) {
        y0 = (float)cos(fDAngY0);
        r0 = (float)sin(fDAngY0);
        tv = (1.0f - y0) / 2.0f;

        for (x = 0; x < (cnt_rings * 2) + 1; x++) {
            fDAngX0 = x * fDAng;

            v.x = r0 * (float)sin(fDAngX0);
            v.y = y0;
            v.z = r0 * (float)cos(fDAngX0);

            tu = 1.0f - float(x) / (cnt_rings * 2.0f);

            m_Vertex[vtx].v = v * radius;
            m_Vertex[vtx].n = v;
            m_Vertex[vtx].tu = tu;
            m_Vertex[vtx].tv = tv;
            m_Vertex[vtx].defcol = color;

            vtx++;
        }
        fDAngY0 = fDAngY0 + fDAng;
    }

    for (y = 0; y < cnt_rings - 1; y++) {
        for (x = 0; x < cnt_rings * 2; x++) {
            m_Index[index++] = ((y + 0) * (cnt_rings * 2 + 1) + (x + 0));
            m_Index[index++] = ((y + 1) * (cnt_rings * 2 + 1) + (x + 0));
            m_Index[index++] = ((y + 0) * (cnt_rings * 2 + 1) + (x + 1));
            m_Index[index++] = ((y + 0) * (cnt_rings * 2 + 1) + (x + 1));
            m_Index[index++] = ((y + 1) * (cnt_rings * 2 + 1) + (x + 0));
            m_Index[index++] = ((y + 1) * (cnt_rings * 2 + 1) + (x + 1));
        }
    }

    y = cnt_rings - 1;

    vy.x = 0;
    vy.y = 1.0;
    vy.z = 0;
    wNorthVtx = vtx;
    m_Vertex[vtx].v = vy * radius;
    m_Vertex[vtx].n = v;
    m_Vertex[vtx].tu = 0.5f;
    m_Vertex[vtx].tv = 0.0f;
    m_Vertex[vtx].defcol = color;
    vtx++;
    wSouthVtx = vtx;
    m_Vertex[vtx].v = -vy * radius;
    m_Vertex[vtx].n = v;
    m_Vertex[vtx].tu = 0.5f;
    m_Vertex[vtx].tv = 1.0f;
    m_Vertex[vtx].defcol = color;
    vtx++;

    for (x = 0; x < cnt_rings * 2; x++) {
        p1 = (word)wSouthVtx;
        p2 = ((y) * (cnt_rings * 2 + 1) + (x + 1));
        p3 = ((y) * (cnt_rings * 2 + 1) + (x + 0));

        m_Index[index++] = p1;
        m_Index[index++] = p3;
        m_Index[index++] = p2;
    }

    for (x = 0; x < cnt_rings * 2; x++) {
        p1 = (word)wNorthVtx;
        p2 = ((0) * (cnt_rings * 2 + 1) + (x + 1));
        p3 = ((0) * (cnt_rings * 2 + 1) + (x + 0));

        m_Index[index++] = p1;
        m_Index[index++] = p3;
        m_Index[index++] = p2;
    }

    m_CntPrimitive = index / 3;
}

void CHelper::Cone(D3DXVECTOR3 vFrom, D3DXVECTOR3 vTo, float rFrom, float rTo, DWORD colorFrom, DWORD colorTo,
                   int seg_cnt) {
    DTRACE();

    int i, index;

    if (seg_cnt < 3)
        return;

    m_CntVertex = seg_cnt * 2;
    m_CntIndex = seg_cnt * 2 * 3;

    m_Vertex = (SHelperVertex *)HAllocClear(m_CntVertex * sizeof(SHelperVertex), g_CacheHeap);
    m_Index = (word *)HAllocClear(m_CntIndex * sizeof(word), g_CacheHeap);

    m_Type = D3DPT_TRIANGLELIST;

    ConeUpdate(vFrom, vTo, rFrom, rTo, colorFrom, colorTo);

    index = 0;
    for (i = 0; i < seg_cnt; i++) {
        m_Index[index++] = i;
        m_Index[index] = i + 1;
        if (m_Index[index] >= seg_cnt)
            m_Index[index] = 0;
        index++;
        m_Index[index++] = i + seg_cnt;

        m_Index[index] = i + 1;
        if (m_Index[index] >= seg_cnt)
            m_Index[index] = 0;
        index++;
        m_Index[index] = i + 1 + seg_cnt;
        if (m_Index[index] >= (seg_cnt + seg_cnt))
            m_Index[index] = seg_cnt;
        index++;
        m_Index[index] = i + seg_cnt;
        index++;
    }

    m_CntPrimitive = index / 3;
}

void CHelper::ConeUpdate(D3DXVECTOR3 vFrom, D3DXVECTOR3 vTo, float rFrom, float rTo, DWORD colorFrom, DWORD colorTo) {
    DTRACE();

    int i, segcnt;
    float a;
    D3DXVECTOR3 v;
    D3DXMATRIX m, m1, m2;

    if ((m_CntVertex < 6) || ((m_CntVertex & 1) == 1))
        return;

    segcnt = m_CntVertex / 2;

    v.x = vTo.x - vFrom.x;
    v.y = vTo.y - vFrom.y;
    v.z = vTo.z - vFrom.z;
    D3DXVec3Normalize(&v, &v);

    float az;

    if (v.x > 0)
        az = (float)atan(v.y / v.x);
    else if (v.x < 0)
        az = pi + (float)atan(v.y / v.x);
    else if ((v.x = 0) && (v.y >= 0))
        az = pi / 2.0f;
    else
        az = 3.0f * pi / 2.0f;

    float ay = (float)acos(v.z / sqrt(v.x * v.x + v.y * v.y + v.z * v.z));

    D3DXMatrixIdentity(&m1);
    D3DXMatrixIdentity(&m2);

    m1(0, 0) = (float)cos(az);
    m1(1, 0) = (float)-sin(az);
    m1(0, 1) = (float)sin(az);
    m1(1, 1) = (float)cos(az);

    m2(0, 0) = (float)cos(ay);
    m2(2, 0) = (float)sin(ay);
    m2(0, 2) = (float)-sin(ay);
    m2(2, 2) = (float)cos(ay);

    m = m2 * m1;

    a = 0;
    for (i = 0; i < segcnt; i++) {
        v.x = (float)sin(a) * rFrom;
        v.y = (float)-cos(a) * rFrom;
        v.z = 0.0f;
        D3DXVec3TransformNormal(&v, &v, &m);

        m_Vertex[i].v = v + vFrom;
        m_Vertex[i].n = v;
        m_Vertex[i].tu = a / (2.0f * pi);
        m_Vertex[i].tv = 0.0f;
        m_Vertex[i].defcol = colorFrom;

        a = a + 2.0f * pi / segcnt;
    }
    a = 0;
    for (i = 0; i < segcnt; i++) {
        v.x = (float)sin(a) * rTo;
        v.y = (float)-cos(a) * rTo;
        v.z = 0.0f;
        D3DXVec3TransformNormal(&v, &v, &m);

        m_Vertex[segcnt + i].v = v + vTo;
        m_Vertex[segcnt + i].n = v;
        m_Vertex[segcnt + i].tu = a / (2.0f * pi);
        m_Vertex[segcnt + i].tv = 1.0f;
        m_Vertex[segcnt + i].defcol = colorTo;

        a = a + 2.0f * pi / segcnt;
    }
}

void CHelper::Triangle(const D3DXVECTOR3 &p0, const D3DXVECTOR3 &p1, const D3DXVECTOR3 &p2, DWORD color) {
    DTRACE();

    this->m_CntPrimitive = 1;
    this->m_Type = D3DPT_TRIANGLELIST;

    m_CntVertex = 3;
    m_Vertex = (SHelperVertex *)HAllocClear(m_CntVertex * sizeof(SHelperVertex), g_CacheHeap);

    m_Vertex[0].v = p0;
    m_Vertex[0].defcol = color;

    m_Vertex[1].v = p1;
    m_Vertex[1].defcol = color;

    m_Vertex[2].v = p2;
    m_Vertex[2].defcol = color;
}

void CHelper::Draw() {
    DTRACE();

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m_Matrix));

    if (m_CntIndex) {
        ASSERT_DX(g_D3DD->DrawIndexedPrimitiveUP(m_Type, 0, m_CntVertex, m_CntPrimitive, m_Index, D3DFMT_INDEX16,
                                                 m_Vertex, sizeof(SHelperVertex)));
    }
    else {
        ASSERT_DX(g_D3DD->DrawPrimitiveUP(m_Type, m_CntPrimitive, m_Vertex, sizeof(SHelperVertex)));
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CHelper::DrawBegin(void) {
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

    SetColorOpSelect(0, D3DTA_DIFFUSE);
    SetAlphaOpSelect(0, D3DTA_DIFFUSE);
    g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    ASSERT_DX(g_D3DD->SetFVF(HelperVertexFormat));
}

void CHelper::DrawEnd(void) {
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

CHelper *CHelper::Create(int del_after_cnt_draw, int group) {
    DTRACE();
    return HNew(g_CacheHeap) CHelper(del_after_cnt_draw, group);
}

void CHelper::Destroy(CHelper *he) {
    DTRACE();

    HDelete(CHelper, he, g_CacheHeap);
}

void CHelper::ClearAll(void) {
    DTRACE();

    CHelper *he2, *he = m_First;
    while (he) {
        he2 = he;
        he = he->Next();
        HDelete(CHelper, he2, g_CacheHeap);
    }
}

void CHelper::DrawAll(void) {
    DTRACE();

    DrawBegin();

    CHelper *he = m_First;
    while (he) {
        he->Draw();
        he = he->Next();
    }

    DrawEnd();
}

void CHelper::AfterDraw_r(CHelper *del) {
    DTRACE();

    CHelper *he = m_First;
    while (he) {
        if (he->Parent() == del) {
            del->DeleteAfterCntDraw(1);
            CHelper::AfterDraw_r(del);
        }
        he = he->Next();
    }
}

void CHelper::AfterDraw(void) {
    DTRACE();

    CHelper *he = m_First;
    while (he) {
        if (he->DeleteAfterCntDraw() == 1)
            CHelper::AfterDraw_r(he);
        he = he->Next();
    }

    he = m_First;
    CHelper *he2;
    while (he) {
        he2 = he;
        he = he->Next();
        if (he2->DeleteAfterCntDraw() == 1)
            CHelper::Destroy(he2);
    }

    he = m_First;
    while (he) {
        if (he->DeleteAfterCntDraw() > 1)
            he->DeleteAfterCntDraw(he->DeleteAfterCntDraw() - 1);
        he = he->Next();
    }
}

void CHelper::DestroyByGroup(int group) {
    DTRACE();

    CHelper *he = m_First;
    CHelper *he2;
    while (he) {
        he2 = he;
        he = he->Next();
        if (he2->Group() == group)
            CHelper::Destroy(he2);
    }
}

#endif