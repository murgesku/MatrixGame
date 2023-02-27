// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef STORAGE_INCLUDE
#define STORAGE_INCLUDE

#include "Base.hpp"

namespace Base {

enum EStorageType {
    ST_INT32,
    ST_DWORD,
    ST_BYTE,
    ST_FLOAT,
    ST_DOUBLE,
    ST_WCHAR,

    ST_COMPRESSED = SETBIT(31)
};

inline int StorageTypeSize(EStorageType t) {
    return (t == ST_DOUBLE) ? 8 : ((t == ST_BYTE) ? 1 : ((t == ST_WCHAR) ? 2 : (4)));
}

class CDataBuf : public CBuf {
    struct SDataBufHeader {
        DWORD alloc_table_disp;
        DWORD arrays_count;
        int element_type_size;
    };

    struct SDataBufAllocTableEntry {
        DWORD disp;
        DWORD count;
        DWORD allocated_count;
    };

    SDataBufAllocTableEntry *TableEntry(int i) {
        SDataBufHeader *header = Buff<SDataBufHeader>();
        return (SDataBufAllocTableEntry *)(Buff<BYTE>() + header->alloc_table_disp +
                                           i * sizeof(SDataBufAllocTableEntry));
    }

    template <class D>
    D *GetFirstElement(SDataBufAllocTableEntry *te) {
        return (D *)(Buff<BYTE>() + te->disp);
    }
    template <class D>
    D *GetEndElement(SDataBufAllocTableEntry *te) {
        return ((D *)(Buff<BYTE>() + te->disp)) + te->count;
    }

    void ExpandArray(DWORD i, DWORD sz) {
        Expand(sz);
        SDataBufHeader *header = Buff<SDataBufHeader>();
        ++i;

        BYTE *data0 = (header->arrays_count <= i) ? (Buff<BYTE>() + header->alloc_table_disp)
                                                  : GetFirstElement<BYTE>(TableEntry(i));
        BYTE *data1 = BuffEnd<BYTE>() - sz;

        for (; i < header->arrays_count; ++i) {
            TableEntry(i)->disp += sz;
        }
        memmove(data0 + sz, data0, data1 - data0);
        header->alloc_table_disp += sz;
    }

public:
    CDataBuf(CHeap *heap, EStorageType st) : CBuf(heap, 1024) {
        int ets = StorageTypeSize(st);
        Expand(sizeof(SDataBufHeader));
        SDataBufHeader *header = Buff<SDataBufHeader>();
        header->alloc_table_disp = sizeof(SDataBufHeader);
        header->arrays_count = 0;
        header->element_type_size = ets;
    }

    template <class D>
    void AddToArray(DWORD i, D d) {
        SDataBufHeader *header = Buff<SDataBufHeader>();
        ASSERT(sizeof(D) == header->element_type_size);
        ASSERT(i < header->arrays_count);
        SDataBufAllocTableEntry *te = TableEntry(i);
        if (te->count >= te->allocated_count) {
            ExpandArray(i, sizeof(D) * 16);
            te = TableEntry(i);
            te->allocated_count += 16;
        }
        *GetEndElement<D>(te) = d;
        ++te->count;
    }

    template <class D>
    void AddToArray(DWORD i, const D *d, int count) {
        SDataBufHeader *header = Buff<SDataBufHeader>();
        ASSERT(sizeof(D) == header->element_type_size);
        ASSERT(i < header->arrays_count);
        SDataBufAllocTableEntry *te = TableEntry(i);
        if ((te->count + count) > te->allocated_count) {
            ExpandArray(i, sizeof(D) * (count + 16));
            te = TableEntry(i);
            te->allocated_count += (count + 16);
        }
        memcpy(GetEndElement<D>(te), d, sizeof(D) * count);
        te->count += count;
    }

    template <class D>
    void SetArrayLength(DWORD i, int newlen) {
        SDataBufHeader *header = Buff<SDataBufHeader>();
        ASSERT(sizeof(D) == header->element_type_size);
        ASSERT(i < header->arrays_count);
        SDataBufAllocTableEntry *te = TableEntry(i);
        if (newlen > (int)te->allocated_count) {
            ExpandArray(i, sizeof(D) * (newlen - te->allocated_count + 16));
            te = TableEntry(i);
            te->allocated_count += (newlen - te->allocated_count + 16);
        }
        te->count = newlen;
    }

    template <class D>
    void SetArray(DWORD i, const D *d, int count) {
        SetArrayLength<D>(i, count);
        memcpy(GetFirst<D>(i), d, count * sizeof(D));
    }

    int AddArray(void) {
        SDataBufHeader *header = Buff<SDataBufHeader>();
        int sz = header->element_type_size * 16 + sizeof(SDataBufAllocTableEntry);
        Expand(sz);
        header = Buff<SDataBufHeader>();

        BYTE *data0 = Buff<BYTE>() + header->alloc_table_disp;
        BYTE *data1 = BuffEnd<BYTE>() - sz;
        memmove(data0 + header->element_type_size * 16, data0, data1 - data0);

        header->alloc_table_disp += header->element_type_size * 16;

        SDataBufAllocTableEntry *te = TableEntry(header->arrays_count);
        te->allocated_count = 16;
        te->count = 0;
        te->disp = header->alloc_table_disp - header->element_type_size * 16;
        ++header->arrays_count;
        return header->arrays_count - 1;
    }

    DWORD GetArrayLength(int i) { return TableEntry(i)->count; }
    DWORD GetArraysCount(void) { return Buff<SDataBufHeader>()->arrays_count; }

    template <class D>
    D *GetFirst(int i) {
        return GetFirstElement<D>(TableEntry(i));
    }
    template <class D>
    D *GetEnd(int i) {
        return GetEndElement<D>(TableEntry(i));
    }

    void SetWStr(DWORD idx, const std::wstring &str) { SetArray<wchar>(idx, str.c_str(), str.length()); }

    int AddWStr(const std::wstring &str) {
        int r = AddArray();
        AddToArray<wchar>(r, str.c_str(), str.length());
        return r;
    }

    std::wstring GetAsWStr(int i) {
        DTRACE();
        wchar *str = GetFirst<wchar>(i);
        int l = GetArrayLength(i);

        return std::wstring{str, static_cast<size_t>(l)};
    }

    ParamParser GetAsParamParser(int i)
    {
        DTRACE();
        wchar *str = GetFirst<wchar>(i);
        int l = GetArrayLength(i);

        return std::wstring{str, static_cast<size_t>(l)};
    }

    int FindAsWStr(const wchar *val, int len) {
        SDataBufHeader *header = Buff<SDataBufHeader>();
        for (DWORD i = 0; i < header->arrays_count; ++i) {
            wchar *str = GetFirst<wchar>(i);
            if (GetArrayLength(i) == len) {
                if (0 == memcmp(val, str, len * sizeof(wchar)))
                    return i;
            }
        }
        return -1;
    }
    int FindAsWStr(const std::wstring &val) { return FindAsWStr(val.c_str(), val.length()); }
    int FindAsWStr(const wchar *val) { return FindAsWStr(val, std::wcslen(val)); }

    void Compact(void) {
        SDataBufHeader *header = Buff<SDataBufHeader>();
        DWORD saved = 0;
        for (DWORD i = 0; i < header->arrays_count; ++i) {
            SDataBufAllocTableEntry *te = TableEntry(i);
            if (saved) {
                // there are some bytes saved
                // its need to move data
                BYTE *data0 = GetFirstElement<BYTE>(te);
                BYTE *data1 = data0 + te->count * header->element_type_size;
                memcpy(data0 - saved, data0, data1 - data0);
                te->disp -= saved;
            }
            if (te->count < te->allocated_count) {
                saved += (te->allocated_count - te->count) * header->element_type_size;
                te->allocated_count = te->count;
            }
        }
        if (saved) {
            // copy table
            memcpy(Buff<BYTE>() + (header->alloc_table_disp - saved), Buff<BYTE>() + header->alloc_table_disp,
                   sizeof(SDataBufAllocTableEntry) * header->arrays_count);
            SetLenNoShrink(Len() - saved);
            header->alloc_table_disp -= saved;
        }
    }
};

class CStorageRecordItem : public CMain {
    std::wstring m_Name;
    EStorageType m_Type;

    CDataBuf *m_Buf;

public:
    CStorageRecordItem(const CStorageRecordItem &item)
      : m_Name(item.m_Name), m_Type(item.m_Type), m_Buf(NULL) {}
    CStorageRecordItem(const std::wstring &name, EStorageType type)
      : m_Name(name), m_Type(type), m_Buf(NULL) {}
    CStorageRecordItem(CHeap *heap) : m_Name{}, m_Buf(NULL) { InitBuf(heap); }
    ~CStorageRecordItem();

    void InitBuf(CHeap *heap);
    void ReleaseBuf(CHeap *heap);
    const std::wstring &GetName(void) const { return m_Name; }

    CDataBuf *GetBuf(EStorageType st) { return (st == m_Type) ? m_Buf : NULL; }

    DWORD CalcUniqID(DWORD x);

    void Save(CBuf &buf, bool compression);
    bool Load(CBuf &buf);
};

class CStorageRecord : public CMain {
    CHeap *m_Heap;

    std::wstring m_Name;  // record name (table name)
    CStorageRecordItem *m_Items;
    int m_ItemsCount;

public:
    CStorageRecord(const CStorageRecord &rec);
    CStorageRecord(std::wstring name, CHeap *heap = NULL)
      : m_Heap(heap), m_Name(name), m_Items(NULL), m_ItemsCount(0) {}
    CStorageRecord(CHeap *heap) : m_Heap(heap), m_Name{}, m_Items(NULL), m_ItemsCount(0) {}
    ~CStorageRecord();

    const std::wstring &GetName(void) const { return m_Name; }

    void AddItem(const CStorageRecordItem &item);
    CDataBuf *GetBuf(const wchar *column, EStorageType st);

    DWORD CalcUniqID(DWORD x);

    void Save(CBuf &buf, bool compression);
    bool Load(CBuf &buf);
};

class CStorage : public CMain {
    CHeap *m_Heap;

    CStorageRecord *m_Records;
    int m_RecordsCnt;

public:
    CStorage(CHeap *heap = NULL);
    ~CStorage();

    void Clear(void);

    void AddRecord(const CStorageRecord &sr);
    void DelRecord(const wchar *table);

    CDataBuf *GetBuf(const wchar *table, const wchar *column, EStorageType st);
    bool IsTablePresent(const wchar *table);

    DWORD CalcUniqID(void);

    void Save(const wchar *fname, bool compression = false);
    bool Load(const wchar *fname);

    void Save(CBuf &buf, bool compression = false);
    bool Load(CBuf &buf);

    void StoreBlockPar(const wchar *root, const CBlockPar &bp);
    void RestoreBlockPar(const wchar *root, CBlockPar &bp);
};

}  // namespace Base

#endif