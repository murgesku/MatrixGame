// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "CHeap.hpp"
#include "CException.hpp"

#include <windows.h>
#include <stdlib.h>

#include <utils.hpp>

#ifdef DEAD_PTR_SPY_ENABLE
#define DEAD_PTR_SPY_SEEK 10
namespace DeadPtr {
int get_id(void) {
    static int last_id;
    return last_id++;
}
uint8_t semetery_heap[SEMETERY_HEAP_SIZE];
SDeadMem semetery[SEMETERY_SIZE];

int semetery_cnt = 0;
int semetery_heap_size = 0;

int find_by_id(int id, uint8_t **ptr) {
    SDeadMemBody *b = (SDeadMemBody *)&semetery_heap;

    while (id != b->id) {
        b = (SDeadMemBody *)(((uint8_t *)b) + b->size);
    }

    *ptr = (uint8_t *)b;
    return b->size;
}
void remove_by_id(int id) {
    uint8_t *ptr;
    int sz = find_by_id(id, &ptr);

    memcpy(ptr, ptr + sz, semetery_heap_size - sz);
    semetery_heap_size -= sz;
}
void remove_by_ptr(void *ptr) {
    for (int i = 0; i < semetery_cnt; ++i) {
        if (semetery[i].ptr_was == ptr) {
            int id = semetery[i].id;
            remove_by_id(id);

            memcpy(semetery + i, semetery + i + 1, (semetery_cnt - i - 1) * sizeof(SDeadMem));
            --semetery_cnt;
            return;
        }
    }
}

void remove_first(void) {
    remove_by_id(semetery[0].id);
    memcpy(semetery, semetery + 1, (SEMETERY_SIZE - 1) * sizeof(SDeadMem));
    --semetery_cnt;
}

void *get_dead_mem(void *mem) {
    for (int i = 0; i < semetery_cnt; ++i) {
        if (semetery[i].ptr_was == mem) {
            uint8_t *ptr;
            int sz = find_by_id(semetery[i].id, &ptr);
            return ptr + sizeof(SDeadMemBody);
        }
    }

    return NULL;
}

void free_mem(void *mem0) {
    int *mem = (int *)mem0;

    int idx = 0;
    for (; idx < DEAD_PTR_SPY_SEEK; ++idx) {
        if (my_signature(mem + idx))
            goto ok;
    }

    return;

ok:;

    {
        int sz = sizeof(SDeadMemBody) + my_signature_size(mem + idx);
        if (sz > SEMETERY_HEAP_SIZE)
            return;

        if (semetery_cnt >= SEMETERY_SIZE) {
            remove_first();
        }

        while ((semetery_heap_size + sz) > SEMETERY_HEAP_SIZE) {
            remove_first();
        }

        semetery[semetery_cnt].id = get_id();
        semetery[semetery_cnt].ptr_was = mem;
        semetery[semetery_cnt].size = sz;

        SDeadMemBody body;
        body.id = semetery[semetery_cnt].id;
        body.size = sz;

        memcpy(semetery_heap + semetery_heap_size, &body, sizeof(SDeadMemBody));
        memcpy(semetery_heap + semetery_heap_size + sizeof(SDeadMemBody), mem, sz - sizeof(SDeadMemBody));
        semetery_heap_size += sz;
        ++semetery_cnt;
    }
}

}  // namespace DeadPtr
#endif

namespace Base {

#ifdef MEM_SPY_ENABLE

#include <stdio.h>

SMemHeader *SMemHeader::first_mem_block;
SMemHeader *SMemHeader::last_mem_block;
uint SMemHeader::fullsize;
uint SMemHeader::maxblocksize;

void CHeap::StaticInit(void) {
    SMemHeader::first_mem_block = NULL;
    SMemHeader::last_mem_block = NULL;
    SMemHeader::fullsize = 0;
    SMemHeader::maxblocksize = 0;
}

void SMemHeader::Release(void) {
    LIST_DEL(this, first_mem_block, last_mem_block, prev, next);
    fullsize -= blocksize;
#ifdef MEM_CHECK
    uint8_t *d1 = (uint8_t *)(this + 1);
    uint8_t *d2 = ((uint8_t *)this) + blocksize - MEM_CHECK_BOUND_SIZE;
    for (int i = 0; i < MEM_CHECK_BOUND_SIZE; ++i) {
        bool begin = *(d1 + i) != MEM_CHECK_FILLER;
        bool end = *(d2 + i) != MEM_CHECK_FILLER;
        if (begin || end) {
#ifdef _DEBUG
            ERROR_S(utils::format(L"Memory corruption detected at (%ls)\n%ls - %d",
                begin ? L"begin" : L"end",
                utils::to_wstring(file).c_str(),
                line));
#else
            auto str = utils::format("Memory corruption detected at:\n%s - %d", file, line);
            MessageBox(NULL, str.c_str(), "Memory corruption!", MB_OK | MB_ICONERROR);
            debugbreak();
#endif
        }
    }

    memset(d2, 0, MEM_CHECK_BOUND_SIZE);
#endif
}

void CHeap::StaticDeInit(void) {
    // check!!!!!!!!!
    if (SMemHeader::first_mem_block) {
        std::string buf{"There are some memory leaks have detected:\n"};
        while (SMemHeader::last_mem_block) {
            if (buf.length() < 65000)
            {
                auto rec =
                    utils::format("%i, %u : %s - %i\n",
                        SMemHeader::last_mem_block->cnt,
                        SMemHeader::last_mem_block->blocksize,
                        SMemHeader::last_mem_block->file,
                        SMemHeader::last_mem_block->line);

                // if the same record is already in the buffer - don't add it again
                if (buf.find(rec) == std::string::npos)
                {
                    buf += rec;
                }

                // sprintf(buf + len, "%u : %s - %i\n", SMemHeader::last_mem_block->blocksize,
                // SMemHeader::last_mem_block->file, SMemHeader::last_mem_block->line);
            }
            else
            {
                buf += "............ and more.......";
                break;
            }
            SMemHeader::last_mem_block = SMemHeader::last_mem_block->prev;
        }
        MessageBoxA(0, buf.c_str(), "Memory leaks!!!!!!!!!", MB_ICONEXCLAMATION);
    }
}

void CHeap::Free(void *buf, const char *file, int line) {
    if (!buf)
    {
        ERROR_S(
            utils::format(
                L"NULL pointer is passed to Free function at: %ls - %d",
                utils::to_wstring(file).c_str(),
                line));
    }

#ifdef DEAD_PTR_SPY_ENABLE
    DeadPtr::free_mem(buf);
#endif

    SMemHeader *h = SMemHeader::CalcBegin(buf);
    h->Release();
    Free(h);
}

#endif

void CHeap::AllocationError(int zn) {
#ifdef _DEBUG
    debugbreak();
#else

    wchar buf[256];
    wcscpy(buf, L"NULL memory allocated on ");

    wchar temp[64];

    wchar *tstr = temp;
    int tsme = 63;

    int fm = 0;
    if (zn < 0) {
        fm = 1;
        zn = -zn;
    }

    while (zn > 0) {
        tsme--;
        tstr[tsme] = wchar(zn - int(zn / 10) * 10) + wchar('0');
        zn = zn / 10;
    }
    if (fm) {
        tsme--;
        tstr[tsme] = L'-';
    }
    int cnt = 63 - tsme;
    for (int i = 0; i < cnt; i++)
        tstr[i] = tstr[tsme + i];
    tstr[cnt] = 0;

    wcscat(buf, temp);
    wcscat(buf, L" bytes request.");

    ERROR_S(buf);
#endif
}

}  // namespace Base
