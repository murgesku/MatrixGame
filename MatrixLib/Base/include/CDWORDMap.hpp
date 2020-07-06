// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

namespace Base {


#define HASH_TABLE_SIZE 1024

typedef bool (*ENUM_DWORD)(DWORD key, DWORD val, DWORD user);

#define PTR2KEY(p) (((DWORD)p)>>3)

class CDWORDMap : public CMain
{
    typedef struct SPair
    {
        union
        {
            DWORD   key;
            DWORD   allocated;
        };
        union
        {
            DWORD   value;
            DWORD   contained;
        };
    } *PSPair;
    

    CHeap       *m_Heap;
    PSPair       m_Table[HASH_TABLE_SIZE];

    int          m_Cnt;
public:

    CDWORDMap(CHeap *heap = NULL):m_Heap(heap) 
    {
        memset(&m_Table, 0, sizeof(m_Table)); 
        m_Cnt = 0;
    }
    ~CDWORDMap();

    int  GetCount(void) const {return m_Cnt;};
    void Set(DWORD key, DWORD v);
    bool Get(DWORD key, DWORD *v);
    bool Del(DWORD key);
    void Enum(ENUM_DWORD, DWORD user);
    void Clear(void);


};

__forceinline CDWORDMap::~CDWORDMap()
{
    Clear();
}

__forceinline void CDWORDMap::Clear(void)
{
    for (int i=0; i<HASH_TABLE_SIZE; ++i)
    {
        if (m_Table[i]) HFree(m_Table[i], m_Heap);
        m_Table[i] = NULL;
    }
}

__forceinline void CDWORDMap::Set(DWORD key, DWORD v)
{
    DWORD index = key & (HASH_TABLE_SIZE-1);
    PSPair vv = m_Table[index];
    if (vv == NULL)
    {
        //not present
        vv = m_Table[index] = (PSPair)HAlloc(16 * sizeof(SPair), m_Heap);

        vv->allocated = 15;
        vv->contained = 1;

        ++vv;

        vv->key = key;
        vv->value = v;

#ifdef _DEBUG
        ++m_Cnt;
#endif

    } else
    {
        PSPair vvseek = vv + vv->contained;
        while (vvseek > vv)
        {
            if (vvseek->key == key)
            {
                vvseek->value = v;
                return;
            }
            --vvseek;
        }

        ++m_Cnt;

        if ( vv->contained < vv->allocated)
        {
            ++vv->contained;
            (vv + vv->contained)->key = key;
            (vv + vv->contained)->value = v;

        } else
        {
            int newsize = vv->allocated + 16 + 1;
            vv = m_Table[index] = (PSPair)HAllocEx(vv, newsize * sizeof(SPair), m_Heap);

            vv->allocated = newsize - 1;
            ++vv->contained;
            (vv + vv->contained)->key = key;
            (vv + vv->contained)->value = v;
        }

    }
 
}

__forceinline bool CDWORDMap::Get(DWORD key, DWORD *v)
{
    DWORD index = key & (HASH_TABLE_SIZE-1);
    PSPair vv = m_Table[index];
    if (vv == NULL)
    {
        //not present

        return false;

    } else
    {
        PSPair vvseek = vv + vv->contained;
        while (vvseek > vv)
        {
            if (vvseek->key == key)
            {
                if (v) *v = vvseek->value;
                return true;
            }
            --vvseek;
        }
        return false;
    }
}

__forceinline bool CDWORDMap::Del(DWORD key)
{
    DWORD index = key & (HASH_TABLE_SIZE-1);
    PSPair vv = m_Table[index];
    if (vv == NULL)
    {
        //not present

        return false;

    } else
    {
        PSPair vvseek = vv + vv->contained;
        while (vvseek > vv)
        {
            if (vvseek->key == key)
            {
                *vvseek = *(vv + vv->contained);
                --vv->contained;
                --m_Cnt;
                return true;
            }
            --vvseek;
        }
        return false;
    }
}

}