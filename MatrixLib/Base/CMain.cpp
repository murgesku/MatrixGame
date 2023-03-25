// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "CMain.hpp"

#include "CFile.hpp"

#include "CReminder.hpp"
#include "Tracer.hpp"

namespace Base {

#ifdef DEAD_CLASS_SPY_ENABLE

#define SEMETERY_SIZE      100000
#define SEMETERY_HEAP_SIZE 1000000

struct DCS_SDeadMem {
    const CMain *ptr_was;
    int size;
    int id;
};

struct DCS_SDeadMemBody {
    int id;
    int size;
};

static int DCS_get_id(void) {
    static int last_id;
    return last_id++;
}
BYTE DCS_semetery_heap[SEMETERY_HEAP_SIZE];
DCS_SDeadMem DCS_semetery[SEMETERY_SIZE];

int DCS_semetery_cnt = 0;
int DCS_semetery_heap_size = 0;

static int DCS_find_by_id(int id, BYTE **ptr) {
    DCS_SDeadMemBody *b = (DCS_SDeadMemBody *)&DCS_semetery_heap;

    while (id != b->id) {
        b = (DCS_SDeadMemBody *)(((BYTE *)b) + b->size);
    }

    *ptr = (BYTE *)b;
    return b->size;
}
static void DCS_remove_by_id(int id) {
    BYTE *ptr;
    int sz = DCS_find_by_id(id, &ptr);

    memcpy(ptr, ptr + sz, DCS_semetery_heap_size - sz);
    DCS_semetery_heap_size -= sz;
}

static void DCS_remove_first(void) {
    if (DCS_semetery_cnt == 0)
        return;
    DCS_remove_by_id(DCS_semetery[0].id);
    memcpy(DCS_semetery, DCS_semetery + 1, (SEMETERY_SIZE - 1) * sizeof(DCS_SDeadMem));
    --DCS_semetery_cnt;
}

void CMain::DCS_ClassDead(int szc) const {
    int sz = sizeof(DCS_SDeadMemBody) + szc;
    if (sz > SEMETERY_HEAP_SIZE)
        return;

    if (DCS_semetery_cnt >= SEMETERY_SIZE) {
        DCS_remove_first();
    }

    while ((DCS_semetery_heap_size + sz) > SEMETERY_HEAP_SIZE) {
        DCS_remove_first();
    }

    DCS_semetery[DCS_semetery_cnt].id = DCS_get_id();
    DCS_semetery[DCS_semetery_cnt].ptr_was = this;
    DCS_semetery[DCS_semetery_cnt].size = sz;

    DCS_SDeadMemBody body;
    body.id = DCS_semetery[DCS_semetery_cnt].id;
    body.size = sz;

    memcpy(DCS_semetery_heap + DCS_semetery_heap_size, &body, sizeof(DCS_SDeadMemBody));
    memcpy(DCS_semetery_heap + DCS_semetery_heap_size + sizeof(DCS_SDeadMemBody), this, sz - sizeof(DCS_SDeadMemBody));
    DCS_semetery_heap_size += sz;
    ++DCS_semetery_cnt;
}
CMain *CMain::DCS_GetDeadBody(void) const {
    for (int i = 0; i < DCS_semetery_cnt; ++i) {
        if (DCS_semetery[i].ptr_was == this) {
            BYTE *ptr;
            int sz = DCS_find_by_id(DCS_semetery[i].id, &ptr);
            return (CMain *)(ptr + sizeof(DCS_SDeadMemBody));
        }
    }

    return NULL;
}
void CMain::DCS_Reincarnation(void) const {
    for (int i = 0; i < DCS_semetery_cnt; ++i) {
        if (DCS_semetery[i].ptr_was == this) {
            int id = DCS_semetery[i].id;
            DCS_remove_by_id(id);

            memcpy(DCS_semetery + i, DCS_semetery + i + 1, (DCS_semetery_cnt - i - 1) * sizeof(DCS_SDeadMem));
            --DCS_semetery_cnt;
            return;
        }
    }
}
#endif

void CMain::BaseInit(void) {
    CFile::StaticInit();

#ifndef MAXEXP_EXPORTS
    SRemindCore::StaticInit();
#endif

#ifdef MEM_SPY_ENABLE
    CHeap::StaticInit();
#endif
#ifdef _DEBUG
    CDText::StaticInit();
#endif
#if defined _DEBUG || defined _TRACE
    CDebugTracer::StaticInit();
#endif
}

void CMain::BaseDeInit(void) {
#ifdef MEM_SPY_ENABLE
    CHeap::StaticDeInit();
#endif
}

}  // namespace Base
