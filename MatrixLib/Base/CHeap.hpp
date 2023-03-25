// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "Tracer.hpp"
#include "BaseDef.hpp"

#include <cstring> // for std::memset

#if (defined _DEBUG) || (defined FORCE_ENABLE_MEM_SPY)
#define MEM_SPY_ENABLE
//#define DEAD_PTR_SPY_ENABLE
#endif

#ifdef DEAD_PTR_SPY_ENABLE

#define DPTR_SIGNATURE0 (76543210)
#define DPTR_SIGNATURE1 (-12344212)
#define DPTR_SIGNATURE2 (-2132344212)

#define DPTR_MEM_SIGNATURE_DECLARE() int __dead_ptr_signature[4];
#define DPTR_MEM_SIGNATURE_INIT(size)              \
    {                                              \
        __dead_ptr_signature[0] = DPTR_SIGNATURE0; \
        __dead_ptr_signature[1] = DPTR_SIGNATURE1; \
        __dead_ptr_signature[2] = DPTR_SIGNATURE2; \
        __dead_ptr_signature[3] = size;            \
    }

#define SEMETERY_SIZE      100000
#define SEMETERY_HEAP_SIZE 1000000

namespace DeadPtr {
struct SDeadMem {
    void *ptr_was;
    int size;
    int id;
};

struct SDeadMemBody {
    int id;
    int size;
};

void free_mem(void *mem);
void *get_dead_mem(void *mem);

void remove_by_ptr(void *ptr);

inline bool my_signature(void *s) {
    int *signature = (int *)s;
    return signature[0] == DPTR_SIGNATURE0 && signature[1] == DPTR_SIGNATURE1 && signature[2] == DPTR_SIGNATURE2;
}
inline int my_signature_size(void *s) {
    int *signature = (int *)s;
    return signature[3];
}

}  // namespace DeadPtr

#else
#define DPTR_MEM_SIGNATURE_DECLARE()
#define DPTR_MEM_SIGNATURE_INIT(size)

#endif

namespace Base {
#ifdef MEM_SPY_ENABLE

#define MEM_CHECK
#define MEM_CHECK_BOUND_SIZE 256
#define MEM_CHECK_FILLER     0xAC

struct SMemHeader {
    static SMemHeader *first_mem_block;
    static SMemHeader *last_mem_block;
    static uint32_t fullsize;
    static uint32_t maxblocksize;

    SMemHeader *prev;
    SMemHeader *next;
    const char *file;
    int line;
    uint32_t blocksize;

    int cnt;

    static uint32_t CalcSize(uint32_t size) {
        return size + sizeof(SMemHeader)
#ifdef MEM_CHECK
               + MEM_CHECK_BOUND_SIZE + MEM_CHECK_BOUND_SIZE
#endif
                ;
    }
    static SMemHeader *CalcBegin(void *ptr) {
        return (SMemHeader *)(((uint8_t *)ptr) - sizeof(SMemHeader)
#ifdef MEM_CHECK
                              - MEM_CHECK_BOUND_SIZE
#endif
        );
    }

    void Release(void);
    void *Init(uint32_t sz, const char *f, int l) {
        LIST_ADD(this, first_mem_block, last_mem_block, prev, next);

        blocksize = sz;
        fullsize += sz;
        file = f;
        line = l;

        if (sz > maxblocksize) {
            maxblocksize = sz;
        }

        void *ptr;
#ifdef MEM_CHECK
        std::memset(this + 1, MEM_CHECK_FILLER, MEM_CHECK_BOUND_SIZE);
        std::memset((((uint8_t *)this) + sz) - MEM_CHECK_BOUND_SIZE, MEM_CHECK_FILLER, MEM_CHECK_BOUND_SIZE);
        ptr = ((uint8_t *)(this + 1)) + MEM_CHECK_BOUND_SIZE;
#else
        ptr = this + 1;
#endif
#ifdef DEAD_PTR_SPY_ENABLE
        DeadPtr::remove_by_ptr(ptr);
#endif
        return ptr;
    }
};

#endif

class CHeap {
public:
#ifdef MEM_SPY_ENABLE
    static void StaticInit(void);
    static void StaticDeInit(void);
#endif

    static void AllocationError(int zn);

#ifdef MEM_SPY_ENABLE
    static void *Alloc(uint32_t size, const char *file, int line);
    static void *AllocClear(uint32_t size, const char *file, int line);
    static void *ReAlloc(void *buf, uint32_t size, const char *file, int line);
    static void *ReAllocClear(void *buf, uint32_t size, const char *file, int line);
    static void *AllocEx(void *buf, uint32_t size, const char *file, int line);
    static void *AllocClearEx(void *buf, uint32_t size, const char *file, int line);
    static void Free(void *buf, const char *file, int line);
#endif

    static void *Alloc(size_t size);
    static void *AllocClear(size_t size);
    static void *ReAlloc(void *buf, size_t size);
    static void *ReAllocClear(void *buf, size_t size);
    static void *AllocEx(void *buf, size_t size);
    static void *AllocClearEx(void *buf, size_t size);
    static void Free(void *buf);
};

#ifdef MEM_SPY_ENABLE

inline void *CHeap::Alloc(uint32_t size, const char *file, int line) {
    DTRACE();
    uint32_t sz = SMemHeader::CalcSize(size);
    SMemHeader *h = (SMemHeader *)Alloc(sz);
    return h->Init(sz, file, line);
}

inline void *CHeap::AllocClear(uint32_t size, const char *file, int line) {
    DTRACE();
    uint32_t sz = SMemHeader::CalcSize(size);
    SMemHeader *h = (SMemHeader *)AllocClear(sz);
    return h->Init(sz, file, line);
}

inline void *CHeap::ReAlloc(void *buf, uint32_t size, const char *file, int line) {
    DTRACE();
    SMemHeader *h = NULL;
    if (buf != NULL) {
        h = SMemHeader::CalcBegin(buf);
        h->Release();
    }
    uint32_t sz = SMemHeader::CalcSize(size);
    h = (SMemHeader *)ReAlloc(h, sz);
    return h->Init(sz, file, line);
}

inline void *CHeap::ReAllocClear(void *buf, uint32_t size, const char *file, int line) {
    DTRACE();
    SMemHeader *h = NULL;
    if (buf != NULL) {
        h = SMemHeader::CalcBegin(buf);
        h->Release();
    }
    uint32_t sz = SMemHeader::CalcSize(size);
    h = (SMemHeader *)ReAllocClear(h, sz);
    return h->Init(sz, file, line);
}

inline void *CHeap::AllocEx(void *buf, uint32_t size, const char *file, int line) {
    DTRACE();
    SMemHeader *h = NULL;
    if (buf != NULL) {
        h = SMemHeader::CalcBegin(buf);
        h->Release();
    }
    uint32_t sz = SMemHeader::CalcSize(size);
    h = (SMemHeader *)AllocEx(h, sz);
    return h->Init(sz, file, line);
}

inline void *CHeap::AllocClearEx(void *buf, uint32_t size, const char *file, int line) {
    DTRACE();
    SMemHeader *h = NULL;
    if (buf != NULL) {
        h = SMemHeader::CalcBegin(buf);
        h->Release();
    }
    uint32_t sz = SMemHeader::CalcSize(size);
    h = (SMemHeader *)AllocClearEx(h, sz);
    return h->Init(sz, file, line);
}

#endif // MEM_SPY_ENABLE

inline void *CHeap::Alloc(size_t size) {
    void *buf = malloc(size);
    if (!buf) AllocationError(size);
    return buf;
}

inline void *CHeap::AllocClear(size_t size) {
    void *buf = calloc(size, 1);
    if (!buf) AllocationError(size);
    return buf;
}

inline void *CHeap::ReAlloc(void *buf, size_t size) {
    buf = realloc(buf, size);
    if (!buf) AllocationError(size);
    return buf;
}

inline void *CHeap::ReAllocClear(void *buf, size_t size) {
    // there is no standard way to zero-fill only newly added memory
    // after realloc. but i guess this memory is used right after
    // allocation, so there is no reason to clean it up.
    buf = realloc(buf, size);
    if (!buf) AllocationError(size);
    return buf;
}

inline void *CHeap::AllocEx(void *buf, size_t size) {
    if (size > 0)
    {
        return (buf ? ReAlloc(buf, size) : Alloc(size));
    }

    Free(buf);
    return nullptr;
}

inline void *CHeap::AllocClearEx(void *buf, size_t size) {
    if (size > 0)
    {
        return (buf ? ReAllocClear(buf, size) : AllocClear(size));
    }

    Free(buf);
    return nullptr;
}

inline void CHeap::Free(void *buf) {
    if (buf) free(buf);
}

}  // namespace Base

// lint -e1532
#ifdef MEM_SPY_ENABLE
inline void *operator new(size_t size, const char *file, int line, Base::CHeap*)
{
    return Base::CHeap::Alloc(size, file, line);
}
inline void operator delete(void *buf, const char *file, int line, Base::CHeap*)
{
    Base::CHeap::Free(buf, file, line);
}
#else
inline void *operator new(size_t size, Base::CHeap*)
{
    return Base::CHeap::Alloc(size);
}
inline void operator delete(void *buf, Base::CHeap*)
{
    Base::CHeap::Free(buf);
}
#endif
// lint +e1532

namespace Base {

#ifdef MEM_SPY_ENABLE
#define HNew(heap) new (__FILE__, __LINE__, (Base::CHeap*)(heap))
#define HDelete(aclass, buf, heap)                                        \
    {                                                                     \
        (buf)->~aclass();                                                 \
        operator delete((buf), __FILE__, __LINE__, (Base::CHeap*)(heap)); \
    }

#define HAlloc(size, heap)             CHeap::Alloc((size), __FILE__, __LINE__)
#define HAllocClear(size, heap)        CHeap::AllocClear((size), __FILE__, __LINE__)
#define HReAlloc(buf, size, heap)      CHeap::ReAlloc((buf), (size), __FILE__, __LINE__)
#define HReAllocClear(buf, size, heap) CHeap::ReAllocClear((buf), (size), __FILE__, __LINE__)
#define HAllocEx(buf, size, heap)      CHeap::AllocEx((buf), (size), __FILE__, __LINE__)
#define HAllocClearEx(buf, size, heap) CHeap::AllocClearEx((buf), (size), __FILE__, __LINE__)
#define HFree(buf, heap)               CHeap::Free((buf), __FILE__, __LINE__)
#else

#define HNew(heap) new ((CHeap*)(heap))
#define HDelete(aclass, buf, heap)                \
    {                                             \
        (buf)->~aclass();                         \
        operator delete((buf), ((CHeap*)(heap))); \
    }

#define HAlloc(size, heap)             CHeap::Alloc((size))
#define HAllocClear(size, heap)        CHeap::AllocClear((size))
#define HReAlloc(buf, size, heap)      CHeap::ReAlloc((buf), (size))
#define HReAllocClear(buf, size, heap) CHeap::ReAllocClear((buf), (size))
#define HAllocEx(buf, size, heap)      CHeap::AllocEx((buf), (size))
#define HAllocClearEx(buf, size, heap) CHeap::AllocClearEx((buf), (size))
#define HFree(buf, heap)               CHeap::Free((buf))

#endif

}  // namespace Base
