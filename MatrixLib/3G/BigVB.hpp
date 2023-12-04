// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef BIGVB_INCLUDE
#define BIGVB_INCLUDE

#include "D3DControl.hpp"

#define BVB_DIRTY_PART  SETBIT(0)
#define BVB_NONEED_PART SETBIT(1)

#define BVB_MAX_SIZE 8388608

template <class V>
class CBigVB;
template <class V>
struct SBigVBOne;

template <class V>
struct SBigVBSource {
private:
    // do not modify these!!!
    int one;   // index of SBigVBOne in SBigVB
    int part;  // index of part in SBigVBOne
    int base;  // index of vert in SBigVBOne
public:
    friend class CBigVB<V>;
    friend struct SBigVBOne<V>;

    SBigVBSource(void) : one(-1) {}

    int Select(CBigVB<V> *bvb);  // SetStreamSource...

    void Invalidate(CBigVB<V> *bvb);
    bool IsDirty(CBigVB<V> *bvb);
    bool IsMarkedNoNeed(CBigVB<V> *bvb);
    bool Prepare(CBigVB<V> *bvb);           // avoid Dirty and NoNeed states
    bool Prepare(CBigVB<V> *bvb, int stl);  // avoid Dirty and NoNeed states. returns true if it is unsuccessful
    void MarkNoNeed(CBigVB<V> *bvb);

    int size;  // in bytes
    V *verts;
};

template <class V>
struct SBigVBPart {
    SBigVBSource<V> *source;
    DWORD flags;
};

template <class V>
struct SBigVBOne {
    SBigVBPart<V> *parts;
    int count;
    int noneedcount;
    int vbsize;  // in bytes
    D3D_VB vb;

    void PrepareVB(void);
    void SetOneIndex(int one) {
        for (int i = 0; i < count; ++i) {
            parts[i].source->one = one;
        }
    }
};

template <class V>
class CBigVB : public Base::CMain {
    CHeap *m_Heap;
    SBigVBOne<V> *m_VB_array;
    int m_VB_count;
    // int          m_ref;

    int m_LastOne;

#ifdef _DEBUG
    bool m_Preparewas;
#endif

    CBigVB(CHeap *heap) : CMain(), m_VB_array(NULL), m_VB_count(0), m_LastOne(-1), m_Heap(heap){};
    ~CBigVB(){};

public:
    friend struct SBigVBSource<V>;

    static CBigVB<V> *NewBigVB(CHeap *heap) { return HNew(heap) CBigVB<V>(heap); }
    static void DestroyBigVB(CBigVB<V> *bvb) { HDelete(CBigVB<V>, bvb, bvb->m_Heap); }
    void BeforeDraw(void) {
#ifdef _DEBUG
        m_Preparewas = false;
#endif
        m_LastOne = -1;
    }
    void AddSource(SBigVBSource<V> *src);
    bool DelSource(SBigVBSource<V> *src);  // return true if BigVB has destroyed

    void ReleaseBuffers(void);

    void PrepareAll(void);
};

template <class V>
inline void SBigVBOne<V>::PrepareVB(void) {
    if (IS_VB(vb))
        return;
    // CREATE_VBM(vbsize, V::FVF, vb);

    for (;;) {
        CREATE_VB(vbsize, V::FVF, vb);
        if (!(IS_VB(vb))) {
#ifdef _DEBUG
            SETFLAG(g_Flags, GFLAG_EXTRAFREERES);  // extra free resources
            RESETFLAG(g_Flags, GFLAG_RENDERINPROGRESS);
#endif
            continue;
        }

        break;
    }
}

template <class V>
inline int SBigVBSource<V>::Select(CBigVB<V> *bvb) {
    if (Prepare(bvb))
        return -1;

    if (bvb->m_LastOne != one) {
        ASSERT_DX(g_D3DD->SetStreamSource(0, GET_VB(bvb->m_VB_array[one].vb), 0, sizeof(V)));
        bvb->m_LastOne = one;
    }
    return base;
}

template <class V>
inline bool SBigVBSource<V>::IsDirty(CBigVB<V> *bvb) {
    return FLAG(bvb->m_VB_array[one].parts[part].flags, BVB_DIRTY_PART);
}

template <class V>
inline bool SBigVBSource<V>::IsMarkedNoNeed(CBigVB<V> *bvb) {
    return FLAG(bvb->m_VB_array[one].parts[part].flags, BVB_NONEED_PART);
}

template <class V>
inline bool SBigVBSource<V>::Prepare(CBigVB<V> *bvb, int stl) {
    DTRACE();
#ifdef _DEBUG
    bvb->m_Preparewas = true;
#endif

    if (IsDirty(bvb)) {
        // dirty part should be marked as "no need" !!!

        SBigVBOne<V> *bvbo = bvb->m_VB_array + one;

        bvbo->PrepareVB();
        if (bvbo->vb == NULL)
            return true;
        V *v;
        LOCKP_VB(bvbo->vb, base * sizeof(V), stl, &v);
        memcpy(v, verts, stl);
        UNLOCK_VB(bvbo->vb);

        RESETFLAG(bvbo->parts[part].flags, BVB_DIRTY_PART);
        RESETFLAG(bvbo->parts[part].flags, BVB_NONEED_PART);
        --bvbo->noneedcount;
        ASSERT(bvbo->noneedcount >= 0);
    }
    else if (IsMarkedNoNeed(bvb)) {
        SBigVBOne<V> *bvbo = bvb->m_VB_array + one;

        RESETFLAG(bvbo->parts[part].flags, BVB_NONEED_PART);
        --bvbo->noneedcount;
        ASSERT(bvbo->noneedcount >= 0);
    }
    return false;
}

template <class V>
inline bool SBigVBSource<V>::Prepare(CBigVB<V> *bvb) {
    DTRACE();
#ifdef _DEBUG
    bvb->m_Preparewas = true;
#endif

    if (IsDirty(bvb)) {
        // dirty part should be marked as "no need" !!!

        SBigVBOne<V> *bvbo = bvb->m_VB_array + one;

        bvbo->PrepareVB();
        if (bvbo->vb == NULL)
            return true;
        V *v;
        LOCKP_VB(bvbo->vb, base * sizeof(V), size, &v);
        memcpy(v, verts, size);
        UNLOCK_VB(bvbo->vb);

        RESETFLAG(bvbo->parts[part].flags, BVB_DIRTY_PART);
        RESETFLAG(bvbo->parts[part].flags, BVB_NONEED_PART);
        --bvbo->noneedcount;
        ASSERT(bvbo->noneedcount >= 0);
    }
    else if (IsMarkedNoNeed(bvb)) {
        SBigVBOne<V> *bvbo = bvb->m_VB_array + one;

        RESETFLAG(bvbo->parts[part].flags, BVB_NONEED_PART);
        --bvbo->noneedcount;
        ASSERT(bvbo->noneedcount >= 0);
    }
    return false;
}

template <class V>
inline void SBigVBSource<V>::Invalidate(CBigVB<V> *bvb) {
    MarkNoNeed(bvb);
    if (!IsDirty(bvb)) {
        SBigVBOne<V> *bvbo = bvb->m_VB_array + one;
        SETFLAG(bvbo->parts[part].flags, BVB_DIRTY_PART);
    }
}

template <class V>
inline void SBigVBSource<V>::MarkNoNeed(CBigVB<V> *bvb) {
    if (!IsMarkedNoNeed(bvb)) {
        SBigVBOne<V> *bvbo = bvb->m_VB_array + one;

        ++bvbo->noneedcount;

        if (bvbo->noneedcount == bvbo->count) {
            // all parts in CBigVBOne are marked as "no need"
            // let's destroy this VB

            // mark all parts as dirty
            for (int i = 0; i < bvbo->count; ++i) {
                SETFLAG(bvbo->parts[i].flags, BVB_DIRTY_PART);
                SETFLAG(bvbo->parts[i].flags, BVB_NONEED_PART);
            }
            DESTROY_VB(bvbo->vb);
        }
        else {
            // there are some need parts. just mark current as "no need"
            SETFLAG(bvbo->parts[part].flags, BVB_NONEED_PART);
        }
    }
}

template <class V>
inline void CBigVB<V>::ReleaseBuffers(void) {
    for (int i = 0; i < m_VB_count; ++i) {
        SBigVBOne<V> *bvbo = m_VB_array + i;

        for (int j = 0; j < bvbo->count; ++j) {
            SETFLAG(bvbo->parts[j].flags, BVB_DIRTY_PART);
            SETFLAG(bvbo->parts[j].flags, BVB_NONEED_PART);
        }
        bvbo->noneedcount = bvbo->count;

        if (bvbo->vb)
            DESTROY_VB(bvbo->vb);
    }
}

template <class V>
inline void CBigVB<V>::AddSource(SBigVBSource<V> *src) {
    int index = 0;

    if (m_VB_count == 0) {
        ASSERT(src->size <= BVB_MAX_SIZE);

        m_VB_count = 1;
        m_VB_array = (SBigVBOne<V> *)HAlloc(sizeof(SBigVBOne<V>), m_Heap);
        memset(m_VB_array, 0, sizeof(SBigVBOne<V>));
        m_VB_array[0].parts = (SBigVBPart<V> *)HAlloc(sizeof(SBigVBPart<V>), m_Heap);
        m_VB_array[0].parts[0].source = src;
        m_VB_array[0].count = 1;
        m_VB_array[0].vbsize = 0;

        index = 0;
    }
    else {
        // find vb one
        int i;
        for (i = 0; i < m_VB_count; ++i) {
            if (m_VB_array[i].vbsize <= (BVB_MAX_SIZE - src->size)) {
                // found vb
                if (m_VB_array[i].vb) {
                    // :( VB already allocated. so make it dirty...

                    for (int j = 0; j < m_VB_array[i].count; ++j) {
                        m_VB_array[i].parts[j].flags = BVB_DIRTY_PART | BVB_NONEED_PART;
                    }

                    DESTROY_VB(m_VB_array[i].vb);
                }

                int cc = m_VB_array[i].count;
                m_VB_array[i].count = cc + 1;
                m_VB_array[i].parts =
                        (SBigVBPart<V> *)HAllocEx(m_VB_array[i].parts, sizeof(SBigVBPart<V>) * (cc + 1), m_Heap);
                m_VB_array[i].parts[cc].source = src;
                index = i;
                break;
            }
        }
        if (i >= m_VB_count) {
            // vbone not found. create new one...

            m_VB_array = (SBigVBOne<V> *)HAllocEx(m_VB_array, sizeof(SBigVBOne<V>) * (m_VB_count + 1), m_Heap);
            memset(m_VB_array + m_VB_count, 0, sizeof(SBigVBOne<V>));

            m_VB_array[m_VB_count].parts = (SBigVBPart<V> *)HAlloc(sizeof(SBigVBPart<V>), m_Heap);
            m_VB_array[m_VB_count].parts[0].source = src;
            m_VB_array[m_VB_count].count = 1;
            m_VB_array[m_VB_count].vbsize = 0;

            index = m_VB_count;
            ++m_VB_count;
        }
    }

    // index has index of desired VBOne
    // update ones

    m_VB_array[index].noneedcount = m_VB_array[index].count;

    int part = m_VB_array[index].count - 1;

    src->base = m_VB_array[index].vbsize / sizeof(V);
    src->one = index;
    src->part = part;
    m_VB_array[index].parts[part].flags = BVB_DIRTY_PART | BVB_NONEED_PART;

    m_VB_array[index].vbsize += src->size;
}

template <class V>
__inline bool CBigVB<V>::DelSource(SBigVBSource<V> *src)  // return true if BigVB has destroyed
{
    ASSERT(src->one < m_VB_count);

    int i;
    int nn = 0;

    if (!FLAG(m_VB_array[src->one].parts[src->part].flags, BVB_NONEED_PART)) {
        ++m_VB_array[src->one].noneedcount;
    }

    // calc "needed" parts that will be dirted
    for (i = src->part + 1; i < m_VB_array[src->one].count; ++i) {
        if (!FLAG(m_VB_array[src->one].parts[i].flags, BVB_NONEED_PART)) {
            ++nn;
        }
    }
    if (nn > 0) {
        ASSERT((nn + m_VB_array[src->one].noneedcount) <= m_VB_array[src->one].count);

        if ((nn + m_VB_array[src->one].noneedcount) == m_VB_array[src->one].count) {
            // so, whole vbone should be dirty...

            ASSERT(m_VB_array[src->one].vb != NULL);
            DESTROY_VB(m_VB_array[src->one].vb);

            // mark all as dirty
            for (i = 0; i < src->part; ++i) {
                m_VB_array[src->one].parts[i].flags = BVB_NONEED_PART | BVB_DIRTY_PART;
            }
        }
    }

    int dbase = src->size / sizeof(V);

    for (int i = src->part + 1; i < m_VB_array[src->one].count; ++i) {
        m_VB_array[src->one].parts[i - 1].flags = BVB_NONEED_PART | BVB_DIRTY_PART;
        m_VB_array[src->one].parts[i - 1].source = m_VB_array[src->one].parts[i].source;
        m_VB_array[src->one].parts[i - 1].source->base -= dbase;
        --m_VB_array[src->one].parts[i - 1].source->part;
    }

    m_VB_array[src->one].noneedcount += nn - 1;
    --m_VB_array[src->one].count;

    if (m_VB_array[src->one].count == 0) {
        // lets destroy this vbone
        HFree(m_VB_array[src->one].parts, m_Heap);
        if (m_VB_array[src->one].vb) {
            DESTROY_VB(m_VB_array[src->one].vb);
        }

        if (src->one < (m_VB_count - 1)) {
            m_VB_array[src->one] = m_VB_array[m_VB_count - 1];
            m_VB_array[src->one].SetOneIndex(src->one);
        }
        --m_VB_count;
        if (m_VB_count == 0) {
            HFree(m_VB_array, m_Heap);
            m_VB_array = NULL;

            src->one = -1;
            return true;
        }
    }

    src->one = -1;
    return false;
}

template <class V>
inline void CBigVB<V>::PrepareAll(void) {
    for (int i = 0; i < m_VB_count; ++i) {
        for (int j = 0; j < m_VB_array[i].count; ++j) {
            m_VB_array[i].parts[j].source->Prepare(this);
        }
    }
}

#endif