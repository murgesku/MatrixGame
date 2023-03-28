// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef D3DCONTROL_HPP
#define D3DCONTROL_HPP

#include "3g.hpp"

#include "CMain.hpp"

//////////D3DResources
#ifdef _DEBUG

enum D3DResType { D3DRESTYPE_IB, D3DRESTYPE_VB, D3DRESTYPE_TEX };

class D3DResource : public Base::CMain {
    static int m_IB_cnt;
    static int m_VB_cnt;
    static int m_TEX_cnt;
    static DWORD m_UID_current;

    D3DResType m_Type;

    static D3DResource *m_First;
    static D3DResource *m_Last;
    D3DResource *m_Prev;
    D3DResource *m_Next;

    DWORD m_UID;

    union {
        IDirect3DIndexBuffer9 *m_IB;
        IDirect3DVertexBuffer9 *m_VB;
        IDirect3DTexture9 *m_TEX;
    };

    const char *m_File;
    int m_Line;

    D3DResource() : CMain() {
        m_UID = m_UID_current++;
        DrawInfo();
        LIST_ADD(this, m_First, m_Last, m_Prev, m_Next);
    }
    ~D3DResource() { LIST_DEL(this, m_First, m_Last, m_Prev, m_Next); }

    void DrawInfo(void) {
        CDText::T("IB", m_IB_cnt);
        CDText::T("VB", m_VB_cnt);
    }

public:
    static void StaticInit(void);
    static void Dump(D3DResType t);

    DWORD GetUID(void) const { return m_UID; }

    IDirect3DIndexBuffer9 *GetIB(void) const {
        ASSERT(m_Type == D3DRESTYPE_IB);
        return m_IB;
    }
    IDirect3DVertexBuffer9 *GetVB(void) const {
        ASSERT(m_Type == D3DRESTYPE_VB);
        return m_VB;
    }
    IDirect3DTexture9 *GetTEX(void) const {
        ASSERT(m_Type == D3DRESTYPE_TEX);
        return m_TEX;
    }

    static D3DResource *BuildIB16(int cnti, const char *file, int line);
    static D3DResource *BuildIBD16(int cnti, const char *file, int line);
    static D3DResource *BuildIBM16(int cnti, const char *file, int line);
    static D3DResource *BuildIB32(int cnti, const char *file, int line);
    static D3DResource *BuildVB(int sz, DWORD fvf, const char *file, int line);
    static D3DResource *BuildVBDynamic(int sz, DWORD fvf, const char *file, int line);
    static D3DResource *BuildVBM(int sz, DWORD fvf, const char *file, int line);

    void Release(void);
};

__inline void D3DResource::Release(void) {
    DTRACE();
    if (m_Type == D3DRESTYPE_IB) {
        --m_IB_cnt;
        m_IB->Release();
    }
    else if (m_Type == D3DRESTYPE_VB) {
        --m_VB_cnt;
        m_IB->Release();
    }
    else if (m_Type == D3DRESTYPE_TEX) {
        --m_TEX_cnt;
        m_TEX->Release();
    }

    HDelete(D3DResource, this, g_CacheHeap);
}

__inline D3DResource *D3DResource::BuildIB16(int cnti, const char *file, int line) {
    DTRACE();
    ++m_IB_cnt;
    D3DResource *r = HNew(g_CacheHeap) D3DResource();
    r->m_Type = D3DRESTYPE_IB;
    r->m_File = file;
    r->m_Line = line;
    if (D3D_OK != (g_D3DD->CreateIndexBuffer(cnti * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT,
                                             &r->m_IB, NULL))) {
        HDelete(D3DResource, r, g_CacheHeap);
        return NULL;
    }
    return r;
}

__inline D3DResource *D3DResource::BuildIBD16(int ibsize, const char *file, int line) {
    DTRACE();
    ++m_IB_cnt;
    D3DResource *r = HNew(g_CacheHeap) D3DResource();
    r->m_Type = D3DRESTYPE_IB;
    r->m_File = file;
    r->m_Line = line;
    if (D3D_OK != (g_D3DD->CreateIndexBuffer(ibsize, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFMT_INDEX16,
                                             D3DPOOL_DEFAULT, &r->m_IB, NULL))) {
        HDelete(D3DResource, r, g_CacheHeap);
        return NULL;
    }
    return r;
}

__inline D3DResource *D3DResource::BuildIBM16(int ibsize, const char *file, int line) {
    DTRACE();
    ++m_IB_cnt;
    D3DResource *r = HNew(g_CacheHeap) D3DResource();
    r->m_Type = D3DRESTYPE_IB;
    r->m_File = file;
    r->m_Line = line;
    if (D3D_OK !=
        (g_D3DD->CreateIndexBuffer(ibsize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &r->m_IB, NULL))) {
        HDelete(D3DResource, r, g_CacheHeap);
        return NULL;
    }

    return r;
}

__inline D3DResource *D3DResource::BuildIB32(int cnti, const char *file, int line) {
    DTRACE();
    ++m_IB_cnt;
    D3DResource *r = HNew(g_CacheHeap) D3DResource();
    r->m_Type = D3DRESTYPE_IB;
    r->m_File = file;
    r->m_Line = line;
    if (D3D_OK != (g_D3DD->CreateIndexBuffer(cnti * sizeof(DWORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT,
                                             &r->m_IB, NULL))) {
        HDelete(D3DResource, r, g_CacheHeap);
        return NULL;
    }
    return r;
}

__inline D3DResource *D3DResource::BuildVB(int sz, DWORD fvf, const char *file, int line) {
    DTRACE();
    ++m_VB_cnt;
    D3DResource *r = HNew(g_CacheHeap) D3DResource();
    r->m_Type = D3DRESTYPE_VB;
    r->m_File = file;
    r->m_Line = line;
    if (D3D_OK != (g_D3DD->CreateVertexBuffer(sz, D3DUSAGE_WRITEONLY, fvf, D3DPOOL_DEFAULT, &r->m_VB, NULL))) {
        HDelete(D3DResource, r, g_CacheHeap);
        return NULL;
    }
    return r;
}

__inline D3DResource *D3DResource::BuildVBDynamic(int sz, DWORD fvf, const char *file, int line) {
    DTRACE();
    ++m_VB_cnt;
    D3DResource *r = HNew(g_CacheHeap) D3DResource();
    r->m_Type = D3DRESTYPE_VB;
    r->m_File = file;
    r->m_Line = line;
    if (D3D_OK !=
        (g_D3DD->CreateVertexBuffer(sz, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, fvf, D3DPOOL_DEFAULT, &r->m_VB, NULL))) {
        HDelete(D3DResource, r, g_CacheHeap);
        return NULL;
    }
    return r;
}

__inline D3DResource *D3DResource::BuildVBM(int sz, DWORD fvf, const char *file, int line) {
    DTRACE();
    ++m_VB_cnt;
    D3DResource *r = HNew(g_CacheHeap) D3DResource();
    r->m_Type = D3DRESTYPE_VB;
    r->m_File = file;
    r->m_Line = line;
    ASSERT_DX(g_D3DD->CreateVertexBuffer(sz, D3DUSAGE_WRITEONLY, fvf, D3DPOOL_MANAGED, &r->m_VB, NULL));
    return r;
}

typedef D3DResource *D3D_IB;
typedef D3DResource *D3D_VB;

#define CREATE_IB16(indexcnt, ib) \
    { (ib) = D3DResource::BuildIB16(indexcnt, __FILE__, __LINE__); }
#define CREATE_IBD16(ibsize, ib) \
    { (ib) = D3DResource::BuildIBD16(ibsize, __FILE__, __LINE__); }
#define CREATE_IBM16(ibsize, ib) \
    { (ib) = D3DResource::BuildIBM16(ibsize, __FILE__, __LINE__); }
#define CREATE_IB32(indexcnt, ib) \
    { (ib) = D3DResource::BuildIB32(indexcnt, __FILE__, __LINE__); }
#define CREATE_VB(sz, fvf, vb) \
    { (vb) = D3DResource::BuildVB(sz, fvf, __FILE__, __LINE__); }
#define CREATE_VB_DYNAMIC(sz, fvf, vb) \
    { (vb) = D3DResource::BuildVBDynamic(sz, fvf, __FILE__, __LINE__); }
#define CREATE_VBM(sz, fvf, vb) \
    { (vb) = D3DResource::BuildVBM(sz, fvf, __FILE__, __LINE__); }

#define DESTROY_IB(ib)   \
    {                    \
        (ib)->Release(); \
        (ib) = NULL;     \
    }
#define GET_IB(ib) (ib)->GetIB()
#define IS_IB(ib)  ((ib) != NULL)
#define LOCK_IB(ib, out)                                      \
    { /* ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS)); */   \
        ASSERT_DX(GET_IB(ib)->Lock(0, 0, (void **)(out), 0)); \
    }
#define LOCKD_IB(ib, out)                                                   \
    { /* ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS)); */                 \
        ASSERT_DX(GET_IB(ib)->Lock(0, 0, (void **)(out), D3DLOCK_DISCARD)); \
    }
#define LOCKPD_IB(ib, x, y, out)                                                \
    { /* ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS)); */                     \
        ASSERT_DX(GET_IB(ib)->Lock((x), (y), (void **)(out), D3DLOCK_DISCARD)); \
    }
#define LOCKP_IB(ib, x, y, out)                                   \
    { /*ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS)); */        \
        ASSERT_DX(GET_IB(ib)->Lock((x), (y), (void **)(out), 0)); \
    }
#define UNLOCK_IB(ib)                                      \
    { /*ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS)); */ \
        ASSERT_DX(GET_IB(ib)->Unlock());                   \
    }

#define DESTROY_VB(vb)   \
    {                    \
        (vb)->Release(); \
        (vb) = NULL;     \
    }
#define GET_VB(vb) (vb)->GetVB()
#define IS_VB(vb)  ((vb) != NULL)
#define LOCK_VB(vb, out)                                      \
    { /*ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS)); */    \
        ASSERT_DX(GET_VB(vb)->Lock(0, 0, (void **)(out), 0)); \
    }
#define LOCK_VB_DYNAMIC(vb, out)                                            \
    { /*ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS)); */                  \
        ASSERT_DX(GET_VB(vb)->Lock(0, 0, (void **)(out), D3DLOCK_DISCARD)); \
    }
#define LOCKP_VB_DYNAMIC(vb, x, y, out)                                         \
    { /*ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS)); */                      \
        ASSERT_DX(GET_VB(vb)->Lock((x), (y), (void **)(out), D3DLOCK_DISCARD)); \
    }
#define LOCKP_VB_NO(vb, x, y, out)                                                  \
    { /*ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS)); */                          \
        ASSERT_DX(GET_VB(vb)->Lock((x), (y), (void **)(out), D3DLOCK_NOOVERWRITE)); \
    }
#define LOCKP_VB(vb, x, y, out)                                   \
    { /*ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS)); */        \
        ASSERT_DX(GET_VB(vb)->Lock((x), (y), (void **)(out), 0)); \
    }
#define UNLOCK_VB(vb)                                       \
    { /* ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS)); */ \
        ASSERT_DX(GET_VB(vb)->Unlock());                    \
    }

#else

typedef IDirect3DIndexBuffer9 *D3D_IB;
typedef IDirect3DVertexBuffer9 *D3D_VB;

#define CREATE_IB16(indexcnt, ib)                                                                             \
    {                                                                                                         \
        if (D3D_OK != (g_D3DD->CreateIndexBuffer(indexcnt * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, \
                                                 D3DPOOL_DEFAULT, &(ib), NULL))) {                            \
            (ib) = NULL;                                                                                      \
        }                                                                                                     \
    }
#define CREATE_IBM16(ibsize, ib) \
    { ASSERT_DX(g_D3DD->CreateIndexBuffer(ibsize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &(ib), NULL)); }
#define CREATE_IBD16(ibsize, ib)                                                                           \
    {                                                                                                      \
        ASSERT_DX(g_D3DD->CreateIndexBuffer(ibsize, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, \
                                            D3DPOOL_DEFAULT, &(ib), NULL));                                \
    }
#define CREATE_IB32(indexcnt, ib)                                                                         \
    {                                                                                                     \
        ASSERT_DX(g_D3DD->CreateIndexBuffer(indexcnt * sizeof(DWORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, \
                                            D3DPOOL_DEFAULT, &(ib), NULL));                               \
    }
#define DESTROY_IB(ib)   \
    {                    \
        (ib)->Release(); \
        (ib) = NULL;     \
    }
#define GET_IB(ib) (ib)
#define IS_IB(ib)  ((ib) != NULL)
#define LOCK_IB(ib, out) \
    { ASSERT_DX(ib->Lock(0, 0, (void **)(out), 0)); }
#define LOCKD_IB(ib, out) \
    { ASSERT_DX(ib->Lock(0, 0, (void **)(out), D3DLOCK_DISCARD)); }
#define LOCKPD_IB(ib, x, y, out) \
    { ASSERT_DX(ib->Lock((x), (y), (void **)(out), D3DLOCK_DISCARD)); }
#define LOCKP_IB(ib, x, y, out) \
    { ASSERT_DX(ib->Lock((x), (y), (void **)(out), 0)); }
#define UNLOCK_IB(ib) \
    { ASSERT_DX(ib->Unlock()); }

#define CREATE_VB(sz, fvf, vb)                                                                                   \
    {                                                                                                            \
        if (D3D_OK != (g_D3DD->CreateVertexBuffer(sz, D3DUSAGE_WRITEONLY, fvf, D3DPOOL_DEFAULT, &(vb), NULL))) { \
            (vb) = NULL;                                                                                         \
        }                                                                                                        \
    }
#define CREATE_VB_DYNAMIC(sz, fvf, vb)                                                                             \
    {                                                                                                              \
        if (D3D_OK != (g_D3DD->CreateVertexBuffer(sz, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, fvf, D3DPOOL_DEFAULT, \
                                                  &(vb), NULL))) {                                                 \
            (vb) = NULL;                                                                                           \
        }                                                                                                          \
    }
#define CREATE_VBM(sz, fvf, vb) \
    { ASSERT_DX(g_D3DD->CreateVertexBuffer(sz, D3DUSAGE_WRITEONLY, fvf, D3DPOOL_MANAGED, &(vb), NULL)); }
#define DESTROY_VB(vb)   \
    {                    \
        (vb)->Release(); \
        (vb) = NULL;     \
    }
#define GET_VB(vb) (vb)
#define IS_VB(vb)  ((vb) != NULL)
#define LOCK_VB_DYNAMIC(vb, out) \
    { ASSERT_DX(vb->Lock(0, 0, (void **)(out), D3DLOCK_DISCARD)); }
#define LOCKP_VB_DYNAMIC(vb, x, y, out) \
    { ASSERT_DX(vb->Lock((x), (y), (void **)(out), D3DLOCK_DISCARD)); }
#define LOCKP_VB_NO(vb, x, y, out) \
    { ASSERT_DX(vb->Lock((x), (y), (void **)(out), D3DLOCK_NOOVERWRITE)); }
#define LOCK_VB(vb, out) \
    { ASSERT_DX(vb->Lock(0, 0, (void **)(out), 0)); }
#define LOCKP_VB(vb, x, y, out) \
    { ASSERT_DX(vb->Lock((x), (y), (void **)(out), 0)); }
#define UNLOCK_VB(vb) \
    { ASSERT_DX(vb->Unlock()); }

#endif

#endif
