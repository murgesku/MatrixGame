// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "CMain.hpp"
#include "Tracer.hpp"
//#include "DebugMsg.h"

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
    static uint fullsize;
    static uint maxblocksize;

    SMemHeader *prev;
    SMemHeader *next;
    const char *file;
    int line;
    uint blocksize;

    int cnt;

    static uint CalcSize(uint size) {
        return size + sizeof(SMemHeader)
#ifdef MEM_CHECK
               + MEM_CHECK_BOUND_SIZE + MEM_CHECK_BOUND_SIZE
#endif
                ;
    }
    static SMemHeader *CalcBegin(void *ptr) {
        return (SMemHeader *)(((BYTE *)ptr) - sizeof(SMemHeader)
#ifdef MEM_CHECK
                              - MEM_CHECK_BOUND_SIZE
#endif
        );
    }

    void Release(void);
    void *Init(uint sz, const char *f, int l) {
        LIST_ADD(this, first_mem_block, last_mem_block, prev, next);

        blocksize = sz;
        fullsize += sz;
        file = f;
        line = l;

        if (sz > maxblocksize) {
            maxblocksize = sz;
            // DbgShowDword("maxblock",sz);
        }
        // if (sz == 672)
        //{
        //    static int c = 0;
        //    cnt = c++;
        //    if (cnt == -1) _asm int 3
        //}

        void *ptr;
#ifdef MEM_CHECK
        memset(this + 1, MEM_CHECK_FILLER, MEM_CHECK_BOUND_SIZE);
        memset((((BYTE *)this) + sz) - MEM_CHECK_BOUND_SIZE, MEM_CHECK_FILLER, MEM_CHECK_BOUND_SIZE);
        ptr = ((BYTE *)(this + 1)) + MEM_CHECK_BOUND_SIZE;
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

__inline int CheckValidPtr(const void *ptr) {
    SYSTEM_INFO si;
    MEMORY_BASIC_INFORMATION mbi;

    GetSystemInfo(&si);

    if (((uintptr_t)ptr) < ((uintptr_t)si.lpMinimumApplicationAddress))
        return -1;
    if (((uintptr_t)ptr) > ((uintptr_t)si.lpMaximumApplicationAddress))
        return -1;

    VirtualQuery(ptr, &mbi, sizeof(mbi));
    if (mbi.State == MEM_FREE)
        return -1;

    return 0;
}

class BASE_API CHeap : public CMain {
private:
    HANDLE m_Heap;
    dword m_Flags;

public:
#ifdef MEM_SPY_ENABLE
    static void StaticInit(void);
    static void StaticDeInit(void);
#endif

    CHeap(void);
    ~CHeap();

    void Clear(void);

    void AllocationError(int zn);

#ifdef MEM_SPY_ENABLE
    void *Alloc(uint size, const char *file, int line);
    void *AllocClear(uint size, const char *file, int line);
    void *ReAlloc(void *buf, uint size, const char *file, int line);
    void *ReAllocClear(void *buf, uint size, const char *file, int line);
    void *AllocEx(void *buf, uint size, const char *file, int line);
    void *AllocClearEx(void *buf, uint size, const char *file, int line);
    void Free(void *buf, const char *file, int line);
#endif

    void *Alloc(size_t size);
    void *AllocClear(size_t size);
    void *ReAlloc(void *buf, size_t size);
    void *ReAllocClear(void *buf, size_t size);
    void *AllocEx(void *buf, size_t size);
    void *AllocClearEx(void *buf, size_t size);
    void Free(void *buf);
};

#ifdef MEM_SPY_ENABLE
inline void *CHeap::Alloc(uint size, const char *file, int line) {
    DTRACE();
    uint sz = SMemHeader::CalcSize(size);
    SMemHeader *h = (SMemHeader *)Alloc(sz);
    return h->Init(sz, file, line);
}
inline void *CHeap::AllocClear(uint size, const char *file, int line) {
    DTRACE();
    uint sz = SMemHeader::CalcSize(size);
    SMemHeader *h = (SMemHeader *)AllocClear(sz);
    return h->Init(sz, file, line);
}
inline void *CHeap::ReAlloc(void *buf, uint size, const char *file, int line) {
    DTRACE();
    SMemHeader *h = NULL;
    if (buf != NULL) {
        h = SMemHeader::CalcBegin(buf);
        h->Release();
    }
    uint sz = SMemHeader::CalcSize(size);
    h = (SMemHeader *)ReAlloc(h, sz);
    return h->Init(sz, file, line);
}
inline void *CHeap::ReAllocClear(void *buf, uint size, const char *file, int line) {
    DTRACE();
    SMemHeader *h = NULL;
    if (buf != NULL) {
        h = SMemHeader::CalcBegin(buf);
        h->Release();
    }
    uint sz = SMemHeader::CalcSize(size);
    h = (SMemHeader *)ReAllocClear(h, sz);
    return h->Init(sz, file, line);
}

inline void *CHeap::AllocEx(void *buf, uint size, const char *file, int line) {
    DTRACE();
    SMemHeader *h = NULL;
    if (buf != NULL) {
        h = SMemHeader::CalcBegin(buf);
        h->Release();
    }
    uint sz = SMemHeader::CalcSize(size);
    h = (SMemHeader *)AllocEx(h, sz);
    return h->Init(sz, file, line);
}

inline void *CHeap::AllocClearEx(void *buf, uint size, const char *file, int line) {
    DTRACE();
    SMemHeader *h = NULL;
    if (buf != NULL) {
        h = SMemHeader::CalcBegin(buf);
        h->Release();
    }
    uint sz = SMemHeader::CalcSize(size);
    h = (SMemHeader *)AllocClearEx(h, sz);
    return h->Init(sz, file, line);
}

#endif

inline void *CHeap::Alloc(size_t size) {
    void *buf;
    if (this == NULL)
        buf = HeapAlloc(GetProcessHeap(), 0, size);
    else
        buf = HeapAlloc(m_Heap, m_Flags, size);
    if (buf == NULL)
        AllocationError(size);
    return buf;
}

inline void *CHeap::AllocClear(size_t size) {
    void *buf;
    if (this == NULL)
        buf = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
    else
        buf = HeapAlloc(m_Heap, m_Flags | HEAP_ZERO_MEMORY, size);
    if (buf == NULL)
        AllocationError(size);
    return buf;
}

inline void *CHeap::ReAlloc(void *buf, size_t size) {
    if ((buf = HeapReAlloc(GetProcessHeap(), 0, buf, size)) == NULL)
        ERROR_E;

    if (this == NULL)
        buf = HeapReAlloc(GetProcessHeap(), 0, buf, size);
    else
        buf = HeapReAlloc(m_Heap, m_Flags, buf, size);
    if (buf == NULL)
        AllocationError(size);
    return buf;
}

inline void *CHeap::ReAllocClear(void *buf, size_t size) {
    if ((buf = HeapReAlloc(GetProcessHeap(), 0, buf, size)) == NULL)
        ERROR_E;

    if (this == NULL)
        buf = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, buf, size);
    else
        buf = HeapReAlloc(m_Heap, m_Flags | HEAP_ZERO_MEMORY, buf, size);
    if (buf == NULL)
        AllocationError(size);

    return buf;
}

inline void *CHeap::AllocEx(void *buf, size_t size) {
    if (this == NULL) {
        if (size <= 0 && buf != NULL) {
            HeapFree(GetProcessHeap(), 0, buf);
            buf = NULL;
        }
        else if (size <= 0) {
            buf = NULL;
        }
        else if (size > 0 && buf != NULL) {
            if ((buf = HeapReAlloc(GetProcessHeap(), 0, buf, size)) == NULL)
                AllocationError(size);
        }
        else {
            if ((buf = HeapAlloc(GetProcessHeap(), 0, size)) == NULL)
                AllocationError(size);
        }
    }
    else {
        if (size <= 0 && buf != NULL) {
            HeapFree(m_Heap, m_Flags, buf);
            buf = NULL;
        }
        else if (size <= 0) {
            buf = NULL;
        }
        else if (size > 0 && buf != NULL) {
            if ((buf = HeapReAlloc(m_Heap, m_Flags, buf, size)) == NULL)
                AllocationError(size);
        }
        else {
            if ((buf = HeapAlloc(m_Heap, m_Flags, size)) == NULL)
                AllocationError(size);
        }
    }
    return buf;
}

inline void *CHeap::AllocClearEx(void *buf, size_t size) {
    if (this == NULL) {
        if (size <= 0 && buf != NULL) {
            HeapFree(GetProcessHeap(), 0, buf);
            buf = NULL;
        }
        else if (size <= 0) {
            buf = NULL;
        }
        else if (size > 0 && buf != NULL) {
            if ((buf = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, buf, size)) == NULL)
                AllocationError(size);
        }
        else {
            if ((buf = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size)) == NULL)
                AllocationError(size);
        }
    }
    else {
        if (size <= 0 && buf != NULL) {
            HeapFree(m_Heap, m_Flags, buf);
            buf = NULL;
        }
        else if (size <= 0) {
            buf = NULL;
        }
        else if (size > 0 && buf != NULL) {
            if ((buf = HeapReAlloc(m_Heap, m_Flags | HEAP_ZERO_MEMORY, buf, size)) == NULL)
                AllocationError(size);
        }
        else {
            if ((buf = HeapAlloc(m_Heap, m_Flags | HEAP_ZERO_MEMORY, size)) == NULL)
                AllocationError(size);
        }
    }
    return buf;
}

inline void CHeap::Free(void *buf) {
    if (buf == NULL)
        return;
    if (this == NULL)
        HeapFree(GetProcessHeap(), 0, buf);
    else
        HeapFree(m_Heap, m_Flags, buf);
}

}  // namespace Base

// lint -e1532
#ifdef MEM_SPY_ENABLE
inline BASE_API void *operator new(size_t size, const char *file, int line, Base::CHeap *heap) {
    return heap->Alloc(size, file, line);
}
inline BASE_API void operator delete(void *buf, const char *file, int line, Base::CHeap *heap) {
    heap->Free(buf, file, line);
}
#else
inline BASE_API void *operator new(size_t size, Base::CHeap *heap) {
    return heap->Alloc(size);
}
inline BASE_API void operator delete(void *buf, Base::CHeap *heap) {
    heap->Free(buf);
}
#endif
// lint +e1532

namespace Base {

#ifdef MEM_SPY_ENABLE
#define HNew(heap) new (__FILE__, __LINE__, (Base::CHeap *)(heap))
#define HDelete(aclass, buf, heap)                                         \
    {                                                                      \
        (buf)->~aclass();                                                  \
        operator delete((buf), __FILE__, __LINE__, (Base::CHeap *)(heap)); \
    }

#define HAlloc(size, heap)             ((Base::CHeap *)(heap))->Alloc((size), __FILE__, __LINE__)
#define HAllocClear(size, heap)        ((Base::CHeap *)(heap))->AllocClear((size), __FILE__, __LINE__)
#define HReAlloc(buf, size, heap)      ((Base::CHeap *)(heap))->ReAlloc((buf), (size), __FILE__, __LINE__)
#define HReAllocClear(buf, size, heap) ((Base::CHeap *)(heap))->ReAllocClear((buf), (size), __FILE__, __LINE__)
#define HAllocEx(buf, size, heap)      ((Base::CHeap *)(heap))->AllocEx((buf), (size), __FILE__, __LINE__)
#define HAllocClearEx(buf, size, heap) ((Base::CHeap *)(heap))->AllocClearEx((buf), (size), __FILE__, __LINE__)
#define HFree(buf, heap)               ((Base::CHeap *)(heap))->Free((buf), __FILE__, __LINE__)
#else

#define HNew(heap) new ((Base::CHeap *)(heap))
#define HDelete(aclass, buf, heap)                       \
    {                                                    \
        (buf)->~aclass();                                \
        operator delete((buf), ((Base::CHeap *)(heap))); \
    }

#define HAlloc(size, heap)             ((Base::CHeap *)(heap))->Alloc((size))
#define HAllocClear(size, heap)        ((Base::CHeap *)(heap))->AllocClear((size))
#define HReAlloc(buf, size, heap)      ((Base::CHeap *)(heap))->ReAlloc((buf), (size))
#define HReAllocClear(buf, size, heap) ((Base::CHeap *)(heap))->ReAllocClear((buf), (size))
#define HAllocEx(buf, size, heap)      ((Base::CHeap *)(heap))->AllocEx((buf), (size))
#define HAllocClearEx(buf, size, heap) ((Base::CHeap *)(heap))->AllocClearEx((buf), (size))
#define HFree(buf, heap)               ((Base::CHeap *)(heap))->Free((buf))

#endif

#ifdef _DEBUG
void HListPrint(wchar *filename);
#endif

}  // namespace Base
