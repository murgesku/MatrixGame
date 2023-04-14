// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_MULTI_SELECTION
#define MATRIX_MULTI_SELECTION

#include "StringConstants.hpp"

#include "3g.hpp"

#include <vector>

class CMatrixMapStatic;

typedef void (*SELECT_ENUM)(CMatrixMapStatic *ms, DWORD param);

#define MS_DIP_TIME 50

#define MS_FLAG_DIP       SETBIT(0)
#define MS_FLAG_ROBOTS    SETBIT(1)
#define MS_FLAG_BUILDINGS SETBIT(2)

#define MULTISEL_FVF (D3DFVF_XYZRHW | D3DFVF_TEX1)
struct SMultiSelVertex {
    D3DXVECTOR4 p;
    float u, v;
};

class CMultiSelection : public CMain {
    static CMultiSelection *m_First;
    static CMultiSelection *m_Last;
    CMultiSelection *m_Next;
    CMultiSelection *m_Prev;

    Base::CPoint m_LT;
    Base::CPoint m_RB;

    DWORD m_Flags;

    int m_TimeBeforeDip;

    std::vector<CMatrixMapStatic*> m_SelItems;

    static int m_Time;

    // CTextureManaged *m_Tex;

    CMultiSelection(const Base::CPoint &pos);
    ~CMultiSelection() {
        if (CMultiSelection::m_GameSelection == this)
            CMultiSelection::m_GameSelection = NULL;

        LIST_DEL(this, m_First, m_Last, m_Prev, m_Next);
    };

    void Draw(void);

    bool DrawPass1(void);
    void DrawPass2(void);
    void DrawPassEnd(void);

    void RemoveSelItems() {
        RESETFLAG(m_Flags, MS_FLAG_BUILDINGS);
        RESETFLAG(m_Flags, MS_FLAG_ROBOTS);

        m_SelItems.clear();
    }

public:
    static CMultiSelection *m_GameSelection;

    static void StaticInit(void) {
        m_First = NULL;
        m_Last = NULL;
        m_Time = 0;
        m_GameSelection = NULL;
        //        m_FirstItem = NULL;
        //        m_LastItem = NULL;
    }

    static bool DrawAllPass1Begin(void) {
        bool ret = false;
        CMultiSelection *f = m_First;
        while (f) {
            ret |= f->DrawPass1();
            f = f->m_Next;
        }
        return ret;
    }
    static void DrawAllPass2Begin(void) {
        CMultiSelection *f = m_First;
        while (f) {
            f->DrawPass2();
            f = f->m_Next;
        }
    }

    static void DrawAllPassEnd(void) {
        CMultiSelection *f = m_First;
        while (f) {
            f->DrawPassEnd();
            f = f->m_Next;
        }
    }

    static void DrawAll() {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
        g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

        CMultiSelection *f = m_First;
        while (f) {
            // this izvrat required because Draw method can delete object itself
            CMultiSelection *next = f->m_Next;
            f->Draw();
            f = next;
        }

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
        g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    }

    static CMultiSelection *Begin(const Base::CPoint &pos);

    void Update(const Base::CPoint &pos) { m_RB = pos; }
    void Update(const Base::CPoint &pos, DWORD mask, SELECT_ENUM callback, DWORD param);
    void End(bool add_to_selection = true);

    static void AddTime(int ms) { m_Time += ms; }

    bool FindItem(const CMatrixMapStatic *o);
    void Remove(const CMatrixMapStatic *o);
};

#endif