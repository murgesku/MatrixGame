// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef BIGIB_INCLUDE
#define BIGIB_INCLUDE

#include "D3DControl.hpp"

#define BIB_DIRTY_PART      SETBIT(0)
#define BIB_NONEED_PART     SETBIT(1)

#define BIB_MAX_SIZE        1048576

class CBigIB;
struct SBigIBOne;

struct SBigIBSource
{
private:
     // do not modify these!!!
    int     one;    // index of SBigIBOne in SBigIB
    int     part;   // index of part in SBigIBOne
    int     base;   // index of vert in SBigIBOne
public:
    friend class CBigIB;
    friend struct SBigIBOne;

    SBigIBSource(void):one(-1) {}

    int     Select(CBigIB *bib);     // SetIndices...
    bool    IsDirty(CBigIB *bib);
    bool    IsMarkedNoNeed(CBigIB *bib);
    bool    Prepare(CBigIB *bib);    // avoid Dirty and NoNeed states
    void    MarkNoNeed(CBigIB *bib);

    int     size; // in bytes
    WORD   *inds;
};


struct SBigIBPart
{
    SBigIBSource *source;
    DWORD           flags;

};

struct SBigIBOne
{
    SBigIBPart *parts;
    int         count;
    int         noneedcount;
    int         ibsize; // in bytes
    D3D_IB      ib;

    void        PrepareIB(void);
    void        SetOneIndex(int one) 
    {
        for (int i=0; i<count;++i)
        {
            parts[i].source->one = one;
        }
    }
};

class CBigIB : public Base::CMain
{
    CHeap       *m_Heap; 
    SBigIBOne   *m_IB_array;
    int          m_IB_count;
    //int          m_sz;          // 2 or 4

    int          m_LastOne;

    CBigIB(CHeap *heap):CMain(),m_IB_array(NULL), m_IB_count(0), m_LastOne(-1), m_Heap(heap) {};
    ~CBigIB() {};
public:

    friend struct SBigIBSource;

    static CBigIB *NewBigIB(CHeap *heap)
    {
        return HNew(heap) CBigIB(heap);
    }
    static void       DestroyBigIB(CBigIB *bib)
    {
        HDelete(CBigIB,bib, bib->m_Heap);
    }
    void BeforeDraw(void) {m_LastOne = -1;}
    void AddSource(SBigIBSource *src);
    bool DelSource(SBigIBSource *src); // return true if BigIB has destroyed

    void ReleaseBuffers(void);
};


__inline void  SBigIBOne::PrepareIB(void)
{
    if (IS_IB(ib)) return;
    //CREATE_IBM16(ibsize, ib);
    CREATE_IB16(ibsize, ib);
}

__inline int  SBigIBSource::Select(CBigIB *bib)
{
    //ASSERT(!IsDirty(bib) && !IsMarkedNoNeed(bib));
    if (Prepare(bib)) return -1;

    if (bib->m_LastOne != one)
    {
        ASSERT_DX(g_D3DD->SetIndices(GET_IB(bib->m_IB_array[one].ib)));
        bib->m_LastOne = one;
    }
    return base;
}


__inline bool  SBigIBSource::IsDirty(CBigIB *bib)
{
    return FLAG(bib->m_IB_array[one].parts[part].flags, BIB_DIRTY_PART);
}

__inline bool  SBigIBSource::IsMarkedNoNeed(CBigIB *bib)
{
    return FLAG(bib->m_IB_array[one].parts[part].flags, BIB_NONEED_PART);
}

__inline bool  SBigIBSource::Prepare(CBigIB *bib)
{
    if (IsDirty(bib))
    {
        // dirty part should be marked as "no need" !!!

        SBigIBOne    *bibo = bib->m_IB_array + one;

        bibo->PrepareIB();
        if (bibo->ib == NULL) return true;
        WORD * idx;
        LOCKP_IB(bibo->ib, base * sizeof(WORD), size, &idx);
        memcpy(idx,inds,size);
        UNLOCK_IB(bibo->ib);

        RESETFLAG(bibo->parts[part].flags, BIB_DIRTY_PART);
        RESETFLAG(bibo->parts[part].flags, BIB_NONEED_PART);
        --bibo->noneedcount;
        ASSERT(bibo->noneedcount >= 0);

    } else
    if (IsMarkedNoNeed(bib))
    {
        SBigIBOne    *bibo = bib->m_IB_array + one;

        RESETFLAG(bibo->parts[part].flags, BIB_NONEED_PART);
        --bibo->noneedcount;
        ASSERT(bibo->noneedcount >= 0);
    }
    return false;
}


__inline void  SBigIBSource::MarkNoNeed(CBigIB *bib)
{

    if (!IsMarkedNoNeed(bib))
    {
        SBigIBOne    *bibo = bib->m_IB_array + one;

        ++bibo->noneedcount;

        if (bibo->noneedcount == bibo->count)
        {
            // all parts in CBigIBOne are marked as "no need"
            // let's destroy this VB

            // mark all parts as dirty
            for (int i=0; i<bibo->count; ++i)
            {
                SETFLAG(bibo->parts[i].flags, BIB_DIRTY_PART);
                SETFLAG(bibo->parts[i].flags, BIB_NONEED_PART);
            }
            DESTROY_IB(bibo->ib);

        } else
        {
            // there are some need parts. just mark current as "no need"
            SETFLAG(bibo->parts[part].flags, BIB_NONEED_PART);
        }
    }


}

__inline void  CBigIB::ReleaseBuffers(void)
{
    
    for (int i=0;i<m_IB_count;++i)
    {

        SBigIBOne    *bibo = m_IB_array + i;

        for (int j=0; j<bibo->count; ++j)
        {
            SETFLAG(bibo->parts[j].flags, BIB_DIRTY_PART);
            SETFLAG(bibo->parts[j].flags, BIB_NONEED_PART);
        }
        bibo->noneedcount = bibo->count;

        if (bibo->ib) DESTROY_IB(bibo->ib);
    }

}


__inline void CBigIB::AddSource(SBigIBSource *src)
{
    int index;

    if (m_IB_count == 0)
    {
        ASSERT(src->size <= BIB_MAX_SIZE);

        m_IB_count = 1;
        m_IB_array = (SBigIBOne *)HAlloc(sizeof(SBigIBOne), m_Heap);
        memset(m_IB_array, 0, sizeof(SBigIBOne));
        m_IB_array[0].parts = (SBigIBPart *)HAlloc(sizeof(SBigIBPart), m_Heap);
        m_IB_array[0].parts[0].source = src;
        m_IB_array[0].count = 1;
        m_IB_array[0].ibsize = 0;

        index = 0;
    } else
    {
        // find vb one
        int i;
        for (i=0; i<m_IB_count; ++i)
        {
            if (m_IB_array[i].ibsize <= (BIB_MAX_SIZE-src->size))
            {
                // found vb
                if (m_IB_array[i].ib)
                {
                    // :( IB already allocated. so make it dirty...


                    for (int j = 0; j<m_IB_array[i].count; ++j)
                    {
                        m_IB_array[i].parts[j].flags = BIB_DIRTY_PART | BIB_NONEED_PART;
                    }

                    DESTROY_IB(m_IB_array[i].ib);
                }

                m_IB_array[i].parts = (SBigIBPart *)HAllocEx(m_IB_array[i].parts, sizeof(SBigIBPart) * (m_IB_array[i].count + 1), m_Heap);
                m_IB_array[i].parts[m_IB_array[i].count].source = src;
                ++m_IB_array[i].count;
                index = i;
                break;
            }
        }
        if (i >= m_IB_count)
        {
            // vbone not found. create new one...

            m_IB_array = (SBigIBOne *)HAllocEx(m_IB_array, sizeof(SBigIBOne) * (m_IB_count+1), m_Heap);
            memset(m_IB_array+m_IB_count, 0, sizeof(SBigIBOne));

            m_IB_array[m_IB_count].parts = (SBigIBPart *)HAlloc(sizeof(SBigIBPart), m_Heap);
            m_IB_array[m_IB_count].parts[0].source = src;
            m_IB_array[m_IB_count].count = 1;
            m_IB_array[m_IB_count].ibsize = 0;

            index = m_IB_count;
            ++m_IB_count;
        }
    }

    // index has index of desired VBOne
    // update ones

    m_IB_array[index].noneedcount = m_IB_array[index].count;

    int part = m_IB_array[index].count-1;

    src->base = m_IB_array[index].ibsize / sizeof(WORD);
    src->one = index;
    src->part = part;
    m_IB_array[index].parts[part].flags = BIB_DIRTY_PART | BIB_NONEED_PART;

    m_IB_array[index].ibsize += src->size;

}

__inline bool CBigIB::DelSource(SBigIBSource *src) // return true if BigIB has destroyed
{
    ASSERT(src->one < m_IB_count);

    int i;
    int nn = 0;

    if (!FLAG(m_IB_array[src->one].parts[src->part].flags, BIB_NONEED_PART))
    {
        ++m_IB_array[src->one].noneedcount;
    }

    // calc "needed" parts that will be dirted
    for (i=src->part + 1; i<m_IB_array[src->one].count; ++i)
    {
        if (!FLAG(m_IB_array[src->one].parts[i].flags, BIB_NONEED_PART))
        {
            ++nn;
        }
    }
    if (nn > 0)
    {
        ASSERT((nn + m_IB_array[src->one].noneedcount) <= m_IB_array[src->one].count);

        if ((nn + m_IB_array[src->one].noneedcount) == m_IB_array[src->one].count)
        {
            // so, whole vbone should be dirty...
            
            ASSERT(m_IB_array[src->one].ib != NULL); 
            DESTROY_VB(m_IB_array[src->one].ib);

            // mark all as dirty
            for (i=0; i<src->part; ++i)
            {
                m_IB_array[src->one].parts[i].flags = BIB_NONEED_PART | BIB_DIRTY_PART;
            }
        }

    }

    int dbase = src->size / sizeof(WORD);

    for (int i=src->part + 1; i<m_IB_array[src->one].count; ++i)
    {
        m_IB_array[src->one].parts[i-1].flags = BIB_NONEED_PART | BIB_DIRTY_PART;
        m_IB_array[src->one].parts[i-1].source = m_IB_array[src->one].parts[i].source;
        m_IB_array[src->one].parts[i-1].source->base -= dbase;
        --m_IB_array[src->one].parts[i-1].source->part;
    }

    m_IB_array[src->one].noneedcount += nn - 1;
    --m_IB_array[src->one].count;

    if (m_IB_array[src->one].count == 0)
    {
        // lets destroy this vbone
        HFree(m_IB_array[src->one].parts, m_Heap);
        if (m_IB_array[src->one].ib) {DESTROY_IB(m_IB_array[src->one].ib);}

        if (src->one < (m_IB_count - 1))
        {
            m_IB_array[src->one] = m_IB_array[m_IB_count - 1];
            m_IB_array[src->one].SetOneIndex(src->one);
        }
        --m_IB_count;
        if (m_IB_count == 0)
        {
            HFree(m_IB_array, m_Heap);
            m_IB_array = NULL;

            src->one = -1;
            return true;
        }
    }

    src->one = -1;
    return false;
}


#endif