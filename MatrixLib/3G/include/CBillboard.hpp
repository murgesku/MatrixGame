// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef CBILLBOARD_HEADER
#define CBILLBOARD_HEADER

#include "D3DControl.hpp"

#define SORTABLE_FLAG_INTENSE SETBIT(0)

#define MAX_BBOARDS      8192
#define MAX_BB_TEXGROUPS 64

#define BILLBOARD_FVF (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)
struct SBillboardVertex {
    D3DXVECTOR3 p;  // Vertex position
    DWORD color;
    float tu, tv;  // Vertex texture coordinates
};

struct SBillboardTexture {
    float tu0, tv0;
    float tu1, tv1;
};

class CBillboard : public Base::CMain {
    typedef CBillboard *PCBillboard;

    static PCBillboard bboards[MAX_BBOARDS];
    static int bboards_left;
    static int bboards_rite;
    static CBillboard *m_FirstIntense;
    static D3D_VB m_VB;
    static D3D_IB m_IB;

    static CTextureManaged *m_SortableTex;
    // static CTextureManaged * m_IntenseTex;

    union {
        int m_Z;
        CBillboard *m_Next;  // list of billboars
    };
    CBillboard *m_NextTex;

    DWORD m_Flags;

public:
#ifdef _DEBUG
    DWORD release_called;
    const char *m_file;
    int m_line;
#endif
    D3DXVECTOR3 m_Pos;  // sortable center
private:
    float m_Scale;  // sortable scale
    DWORD m_Color;  // diffuse color

    union {
        const SBillboardTexture *m_Tex;
        CTextureManaged *m_TexIntense;
    };

    D3DXMATRIX m_Rot;

    bool IsIntense(void) const { return (m_Flags & SORTABLE_FLAG_INTENSE) != 0; }
    void SetIntense(bool f) { m_Flags = (m_Flags & (~SORTABLE_FLAG_INTENSE)) | (f ? SORTABLE_FLAG_INTENSE : 0); }

    void UpdateVBSlot(SBillboardVertex *vb, const D3DXMATRIX &iview);

public:
    CBillboard(void) {
#ifdef _DEBUG
        release_called = true;
#endif
    };

    static void StaticInit(void) {
        m_VB = NULL;
        m_IB = NULL;
        m_FirstIntense = NULL;
        bboards_left = MAX_BBOARDS >> 1;
        bboards_rite = (MAX_BBOARDS >> 1);

        m_SortableTex = NULL;
        // m_IntenseTex = NULL;
    }

    static void Init(void);    // prepare VB
    static void Deinit(void);  // prepare VB

    CBillboard(TRACE_PARAM_DEF const D3DXVECTOR3 &pos, float scale, float angle, DWORD color,
               const SBillboardTexture *tex);  // sorted
    CBillboard(TRACE_PARAM_DEF const D3DXVECTOR3 &pos, float scale, float angle, DWORD color,
               CTextureManaged *tex);  // intense
    ~CBillboard() {
        DTRACE();
        // do nothing!
#ifdef _DEBUG
        if (m_FirstIntense != NULL)
        // if (m_FirstIntense != NULL || m_Root != NULL)
        // if (m_FirstIntense != NULL || m_First != NULL)
        {
            __asm int 3
        }
#endif
#ifdef _DEBUG
        if (!release_called) {
            __asm int 3
        }
#endif
    }

    void Release(void) {
        DTRACE();

#ifdef _DEBUG
        if (m_FirstIntense != NULL) {
            __asm int 3
        }
        release_called = true;
#endif
    }

    static void SortEndDraw(const D3DXMATRIX &iview,
                            const D3DXVECTOR3 &campos);  // its actualy draw all sortable from sorted list

    void Sort(const D3DXMATRIX &sort);
    void SortIntense(void);

    void DrawNow(const D3DXMATRIX &iview);

    static void BeforeDraw(void) { m_SortableTex->Preload(); }

    void SetAngle(float angle, float dx, float dy) {
        D3DXMatrixRotationZ(&m_Rot, angle);
        // m_Rot._11 *= m_Scale; m_Rot._12 *= m_Scale;
        // m_Rot._21 *= m_Scale; m_Rot._22 *= m_Scale;
        m_Rot._41 = dx;
        m_Rot._42 = dy;
    }

    void SetPos(const D3DXVECTOR3 &pos) { m_Pos = pos; }
    const D3DXVECTOR3 &GetPos(void) { return m_Pos; }
    void DisplaceTo(const D3DXVECTOR2 &d) {
        m_Rot._41 = d.x;
        m_Rot._42 = d.y;
    }
    void SetScale(float scale) { m_Scale = scale; }
    float GetScale(void) const { return m_Scale; }
    void SetAlpha(BYTE a) { m_Color = (m_Color & 0x00FFFFFF) | (a << 24); }
    void SetColor(DWORD c) { m_Color = c; }

    // static CTextureManaged * GetIntenseTex(void) {return m_IntenseTex;}
    // static void SetTextures(CTextureManaged *st, CTextureManaged *it) {m_SortableTex = st; m_IntenseTex = it;};
    static void SetSortTexture(CTextureManaged *st) { m_SortableTex = st; };
};

class CBillboardLine : public CMain {
    static CBillboardLine *m_First;
    CBillboardLine *m_Next;
    CBillboardLine *m_NextTex;

    D3DXMATRIX m_Rot;

    float m_Width;
    DWORD m_Color;
    CTextureManaged *m_Tex;

    D3DXVECTOR3 m_Pos0, m_Pos1;

#ifdef _DEBUG
public:
    bool release_called;
    const char *m_file;
    int m_line;
#endif

public:
    friend void CBillboard::SortEndDraw(const D3DXMATRIX &iview, const D3DXVECTOR3 &campos);

    static void StaticInit(void) { m_First = NULL; }

    CBillboardLine(void) {
#ifdef _DEBUG
        release_called = true;
#endif
    }
    CBillboardLine(TRACE_PARAM_DEF const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1, float width, DWORD color,
                   CTextureManaged *tex);
    ~CBillboardLine() {
        // do nothing
#ifdef _DEBUG
        if (m_First != NULL) {
            __asm int 3
        }
        if (!release_called) {
            __asm int 3
        }
#endif
    }

    void Release(void) {
        DTRACE();

#ifdef _DEBUG
        release_called = true;
#endif
        ASSERT(m_First == NULL);
    }

    void DrawNow(const D3DXVECTOR3 &campos);
    void AddToDrawQueue(void);
    void UpdateVBSlot(SBillboardVertex *vb, const D3DXVECTOR3 &campos);

    void SetPos(const D3DXVECTOR3 &pos0, const D3DXVECTOR3 &pos1) {
        m_Pos0 = pos0;
        m_Pos1 = pos1;
    }
    void SetWidth(float w) { m_Width = w; };
    void SetAlpha(BYTE a) { m_Color = (m_Color & 0x00FFFFFF) | (a << 24); }
    void SetColor(DWORD c) { m_Color = c; }

    const D3DXVECTOR3 &GetPos0(void) { return m_Pos0; }
    const D3DXVECTOR3 &GetPos1(void) { return m_Pos1; }
};

#endif