// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include <CHeap.hpp>

#include <cstdint>

namespace Base {

#define HASH_TABLE_SIZE 1024

typedef bool (*ENUM_DWORD)(uint32_t key, uint32_t val, uint32_t user);

#define PTR2KEY(p) (((uint32_t)p) >> 3)

class CDWORDMap
{
    typedef struct SPair {
        union {
            uint32_t key;
            uint32_t allocated;
        };
        union {
            uint32_t value;
            uint32_t contained;
        };
    } * PSPair;

    PSPair m_Table[HASH_TABLE_SIZE];

    int m_Cnt;

public:
    CDWORDMap(void* heap) // TODO: remove param
    {
        memset(&m_Table, 0, sizeof(m_Table));
        m_Cnt = 0;
    }
    ~CDWORDMap();

    int GetCount(void) const { return m_Cnt; };
    void Set(uint32_t key, uint32_t v);
    bool Get(uint32_t key, uint32_t *v);
    bool Del(uint32_t key);
    void Enum(ENUM_DWORD, uint32_t user);
    void Clear(void);
};

inline CDWORDMap::~CDWORDMap() {
    Clear();
}

inline void CDWORDMap::Clear(void) {
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        if (m_Table[i])
            HFree(m_Table[i], nullptr);
        m_Table[i] = NULL;
    }
}

inline void CDWORDMap::Set(uint32_t key, uint32_t v) {
    uint32_t index = key & (HASH_TABLE_SIZE - 1);
    PSPair vv = m_Table[index];
    if (vv == NULL) {
        // not present
        vv = m_Table[index] = (PSPair)HAlloc(16 * sizeof(SPair), nullptr);

        vv->allocated = 15;
        vv->contained = 1;

        ++vv;

        vv->key = key;
        vv->value = v;

#ifdef _DEBUG
        ++m_Cnt;
#endif
    }
    else {
        PSPair vvseek = vv + vv->contained;
        while (vvseek > vv) {
            if (vvseek->key == key) {
                vvseek->value = v;
                return;
            }
            --vvseek;
        }

        ++m_Cnt;

        if (vv->contained < vv->allocated) {
            ++vv->contained;
            (vv + vv->contained)->key = key;
            (vv + vv->contained)->value = v;
        }
        else {
            int newsize = vv->allocated + 16 + 1;
            vv = m_Table[index] = (PSPair)HAllocEx(vv, newsize * sizeof(SPair), nullptr);

            vv->allocated = newsize - 1;
            ++vv->contained;
            (vv + vv->contained)->key = key;
            (vv + vv->contained)->value = v;
        }
    }
}

inline bool CDWORDMap::Get(uint32_t key, uint32_t *v) {
    uint32_t index = key & (HASH_TABLE_SIZE - 1);
    PSPair vv = m_Table[index];
    if (vv == NULL) {
        // not present

        return false;
    }
    else {
        PSPair vvseek = vv + vv->contained;
        while (vvseek > vv) {
            if (vvseek->key == key) {
                if (v)
                    *v = vvseek->value;
                return true;
            }
            --vvseek;
        }
        return false;
    }
}

inline bool CDWORDMap::Del(uint32_t key) {
    uint32_t index = key & (HASH_TABLE_SIZE - 1);
    PSPair vv = m_Table[index];
    if (vv == NULL) {
        // not present

        return false;
    }
    else {
        PSPair vvseek = vv + vv->contained;
        while (vvseek > vv) {
            if (vvseek->key == key) {
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

}  // namespace Base