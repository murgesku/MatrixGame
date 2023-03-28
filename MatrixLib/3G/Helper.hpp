// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef HELPER_INCLUDE
#define HELPER_INCLUDE

#include <windows.h>
#include <cstdint>

#include "CMain.hpp"

#include "d3d9.h"
#include "d3dx9tex.h"

// Класс для отображения дебуг информации в 3D

#if (defined _DEBUG) && !(defined _RELDEBUG)

#define HelperVertexFormat (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1)
struct SHelperVertex {
    D3DXVECTOR3 v;
    D3DXVECTOR3 n;
    DWORD defcol;
    float tu, tv;
};

class CHelper : public Base::CMain {
    static CHelper *m_First;
    static CHelper *m_Last;

    CHelper(int del_after_cnt_draw = 0, int group = 0);
    ~CHelper();

    CHelper *m_Prev;
    CHelper *m_Next;
    CHelper *m_Parent;

    D3DXMATRIX m_Matrix;

    int m_DeleteAfterCntDraw;
    int m_Group;

    D3DPRIMITIVETYPE m_Type;
    int m_CntPrimitive;
    int m_CntVertex;
    int m_CntIndex;
    SHelperVertex *m_Vertex;
    uint16_t *m_Index;

    void SetMatrixPos(const D3DXVECTOR3 &pos) {
        m_Matrix._41 = pos.x;
        m_Matrix._42 = pos.y;
        m_Matrix._43 = pos.z;
    }

public:
    static void StaticInit(void);
    static CHelper *Create(int del_after_cnt_draw = 0, int group = 0);
    static void Destroy(CHelper *he);
    static void ClearAll(void);
    static void DrawAll(void);
    static void AfterDraw_r(CHelper *del);
    static void AfterDraw(void);
    static void DestroyByGroup(int group);

    static void DrawBegin(void);
    static void DrawEnd(void);

    CHelper *Prev(void) { return m_Prev; }
    CHelper *Next(void) { return m_Next; }

    int DeleteAfterCntDraw(void) { return m_DeleteAfterCntDraw; }
    void DeleteAfterCntDraw(int zn) { m_DeleteAfterCntDraw = zn; }
    CHelper *Parent(void) { return m_Parent; }
    void Parent(CHelper *parent) { m_Parent = parent; }
    int Group(void) { return m_Group; }
    void Group(int zn) { m_Group = zn; }

    D3DXMATRIX *Matrix(void) { return &m_Matrix; }

    void Clear(void);
    void Line(const D3DXVECTOR3 &start, const D3DXVECTOR3 &end, DWORD color1 = 0xffffffff, DWORD color2 = 0xffffffff);
    void BoundBox(const D3DXVECTOR3 &mins, const D3DXVECTOR3 &maxs, DWORD color = 0xffffffff);
    SHelperVertex *Lines(int cnt);      // cnt-кол-во линий. (точек=cnt*2)
    SHelperVertex *LineStrip(int cnt);  // cnt-кол-во линий. (точек=cnt*2-2)
    void Sphere(const D3DXVECTOR3 &pos, float radius, int cnt_rings, DWORD color);
    void Cone(D3DXVECTOR3 vFrom, D3DXVECTOR3 vTo, float rFrom, float rTo, DWORD colorFrom, DWORD colorTo, int seg_cnt);
    void ConeUpdate(D3DXVECTOR3 vFrom, D3DXVECTOR3 vTo, float rFrom, float rTo, DWORD colorFrom, DWORD colorTo);

    void Triangle(const D3DXVECTOR3 &p0, const D3DXVECTOR3 &p1, const D3DXVECTOR3 &p2, DWORD color);

    void Draw(void);
};

#endif

#endif