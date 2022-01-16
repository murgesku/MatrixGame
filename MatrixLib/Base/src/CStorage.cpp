// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"

#include "CStorage.hpp"

#undef ZEXPORT
#define ZEXPORT _cdecl

#include "../../../ThirdParty/ZLib/include/zlib.h"

namespace Base {

static int ZL03_UnCompress(CBuf &out, BYTE *in, int inlen) {
    out.Clear();

    if (inlen < 8)
        return 0;
    if (*(in + 0) != 'Z' || *(in + 1) != 'L' || *(in + 2) != '0' || *(in + 3) != '3')
        return 0;

    int cnt = *(int *)(in + 4);

    int optr = 0;
    int iptr = 8;

    for (; cnt > 0; --cnt) {
        out.Pointer(optr);
        out.Expand(65000);

        DWORD len = out.Len() - optr;
        int szb = *(DWORD *)(in + iptr);
        if (uncompress(out.Buff<BYTE>() + optr, &len, in + iptr + 4, szb) != Z_OK) {
            out.Clear();
            return 0;
        }

        iptr += szb + 4;
        optr += len;
        out.SetLenNoShrink(optr);
    }

    out.SetLenNoShrink(optr);
    return optr;
}

static void ZL03_Compression(CBuf &out, BYTE *in, int inlen) {
    out.Clear();
    out.Byte('Z');
    out.Byte('L');
    out.Byte('0');
    out.Byte('3');
    out.Dword(0);  // will be updated. blocks count

    int cnt = 0;

    int ptro = out.Pointer();
    int ptri = 0;

    int szi = inlen;

    while (szi >= 65000) {
        out.Pointer(ptro);
        out.Expand(65536 * 2);

        DWORD len = out.Len() - ptro + 4;
        int res = compress2(out.Buff<BYTE>() + ptro + 4, &len, in + ptri, 65000, Z_BEST_COMPRESSION);
        if (res != Z_OK) {
            _asm int 3
        }
        ptri += 65000;

        *(DWORD *)(out.Buff<BYTE>() + ptro) = len;
        ptro += len + 4;
        out.SetLenNoShrink(ptro);

        szi -= 65000;
        ++cnt;
    }

    if (szi > 0) {
        out.Pointer(ptro);
        out.Expand(szi * 2);

        DWORD len = szi;
        compress2(out.Buff<BYTE>() + ptro + 4, &len, in + ptri, szi, Z_BEST_COMPRESSION);

        *(DWORD *)(out.Buff<BYTE>() + ptro) = len;
        ptro += len + 4;

        ++cnt;
    }

    out.SetLenNoShrink(ptro);
    *(DWORD *)(out.Buff<BYTE>() + 4) = cnt;
}

CStorageRecordItem::~CStorageRecordItem() {}

void CStorageRecordItem::InitBuf(CHeap *heap) {
    ReleaseBuf(heap);
    m_Buf = HNew(heap) CDataBuf(heap, m_Type);
}
void CStorageRecordItem::ReleaseBuf(CHeap *heap) {
    if (m_Buf) {
        HDelete(CDataBuf, m_Buf, heap);
        m_Buf = NULL;
    }
}

DWORD CStorageRecordItem::CalcUniqID(DWORD xi) {
    DWORD x = CalcCRC32_Buf(xi, m_Name.Get(), m_Name.GetLen() * sizeof(wchar));
    x = CalcCRC32_Buf(x, m_Buf->Get(), m_Buf->Len());
    x = CalcCRC32_Buf(x, &m_Type, sizeof(m_Type));
    return x;
}

void CStorageRecordItem::Save(CBuf &buf, bool compression) {
    ASSERT(m_Buf);
    m_Buf->Compact();
    buf.WStr(m_Name);

    if (compression) {
        buf.Dword(m_Type | ST_COMPRESSED);
        CBuf cb(buf.m_Heap);
        ZL03_Compression(cb, (BYTE *)m_Buf->Get(), m_Buf->Len());

        buf.Dword(cb.Len());
        buf.BufAdd(cb.Get(), cb.Len());
    }
    else {
        buf.Dword(m_Type);
        buf.Dword(m_Buf->Len());
        buf.BufAdd(m_Buf->Get(), m_Buf->Len());
    }
}
bool CStorageRecordItem::Load(CBuf &buf) {
    ASSERT(m_Buf);

    m_Name = buf.WStr();
    m_Type = (EStorageType)buf.Dword();
    DWORD sz = buf.Dword();

    if (m_Type & ST_COMPRESSED) {
        m_Type = (EStorageType)(m_Type & ST_COMPRESSED);
        if (0 == ZL03_UnCompress(*m_Buf, buf.Buff<BYTE>() + buf.Pointer(), sz))
            return false;
    }
    else {
        m_Buf->Clear();
        m_Buf->Expand(sz);
        buf.BufGet(m_Buf->Get(), sz);
    }

    return true;
}

CStorageRecord::~CStorageRecord() {
    if (m_Items) {
        for (int i = 0; i < m_ItemsCount; ++i) {
            m_Items[i].ReleaseBuf(m_Heap);
            m_Items[i].~CStorageRecordItem();
        }
        HFree(m_Items, m_Heap);
    }
}

void CStorageRecord::AddItem(const CStorageRecordItem &item) {
    ++m_ItemsCount;
    m_Items = (CStorageRecordItem *)HAllocEx(m_Items, sizeof(CStorageRecordItem) * m_ItemsCount, m_Heap);
    m_Items[m_ItemsCount - 1].CStorageRecordItem::CStorageRecordItem(item);
    // m_Items[m_ItemsCount-1].InitBuf(m_Heap);
}

CStorageRecord::CStorageRecord(const CStorageRecord &rec) : m_Heap(rec.m_Heap), m_Name(rec.m_Name, rec.m_Heap) {
    m_ItemsCount = rec.m_ItemsCount;
    m_Items = (CStorageRecordItem *)HAlloc(sizeof(CStorageRecordItem) * m_ItemsCount, m_Heap);
    for (int i = 0; i < m_ItemsCount; ++i) {
        m_Items[i].CStorageRecordItem::CStorageRecordItem(rec.m_Items[i]);
        m_Items[i].InitBuf(m_Heap);
    }
}

CDataBuf *CStorageRecord::GetBuf(const wchar *column, EStorageType st) {
    for (int i = 0; i < m_ItemsCount; ++i) {
        if (m_Items[i].GetName() == column)
            return m_Items[i].GetBuf(st);
    }
    return NULL;
}

void CStorageRecord::Save(CBuf &buf, bool compression) {
    buf.WStr(m_Name);
    buf.Dword(m_ItemsCount);
    for (int i = 0; i < m_ItemsCount; ++i) {
        m_Items[i].Save(buf, compression);
    }
}

DWORD CStorageRecord::CalcUniqID(DWORD xi) {
    DWORD x = CalcCRC32_Buf(xi, m_Name.Get(), m_Name.GetLen() * sizeof(wchar));
    for (int i = 0; i < m_ItemsCount; ++i) {
        x = m_Items[i].CalcUniqID(x);
    }
    return x;
}

bool CStorageRecord::Load(CBuf &buf) {
    if (m_Items) {
        for (int i = 0; i < m_ItemsCount; ++i) {
            m_Items[i].ReleaseBuf(m_Heap);
            m_Items[i].~CStorageRecordItem();
        }
        HFree(m_Items, m_Heap);
        m_Items = NULL;
    }

    m_Name = buf.WStr();
    m_ItemsCount = buf.Dword();
    if (m_ItemsCount == 0)
        return true;

    m_Items = (CStorageRecordItem *)HAlloc(sizeof(CStorageRecordItem) * m_ItemsCount, m_Heap);
    for (int i = 0; i < m_ItemsCount; ++i) {
        m_Items[i].CStorageRecordItem::CStorageRecordItem(m_Heap);
        if (!m_Items[i].Load(buf)) {
            for (int j = 0; j <= i; ++j) {
                m_Items[j].ReleaseBuf(m_Heap);
                m_Items[j].~CStorageRecordItem();
            }
            HFree(m_Items, m_Heap);
            m_Items = NULL;
            m_ItemsCount = 0;
            return false;
        }
    }
    return true;
}

CStorage::CStorage(CHeap *heap) : m_Heap(heap), m_Records(NULL), m_RecordsCnt(0) {}

CStorage::~CStorage() {
    Clear();
}

void CStorage::Clear(void) {
    if (m_Records) {
        for (int i = 0; i < m_RecordsCnt; ++i) {
            m_Records[i].~CStorageRecord();
        }
        HFree(m_Records, m_Heap);

        m_Records = NULL;
        m_RecordsCnt = 0;
    }
}

void CStorage::AddRecord(const CStorageRecord &sr) {
    ++m_RecordsCnt;
    m_Records = (CStorageRecord *)HAllocEx(m_Records, sizeof(CStorageRecord) * m_RecordsCnt, m_Heap);
    m_Records[m_RecordsCnt - 1].CStorageRecord::CStorageRecord(sr);
}

void CStorage::DelRecord(const wchar *table) {
    for (int i = 0; i < m_RecordsCnt; ++i) {
        if (m_Records[i].GetName() == table) {
            m_Records[i].~CStorageRecord();
            --m_RecordsCnt;
            if (m_RecordsCnt != i)
                memcpy(m_Records + i, m_Records + m_RecordsCnt, sizeof(CStorageRecord));

            break;
        }
    }
}

bool CStorage::IsTablePresent(const wchar *table) {
    for (int i = 0; i < m_RecordsCnt; ++i) {
        if (m_Records[i].GetName() == table) {
            return true;
        }
    }
    return false;
}

CDataBuf *CStorage::GetBuf(const wchar *table, const wchar *column, EStorageType st) {
    for (int i = 0; i < m_RecordsCnt; ++i) {
        if (m_Records[i].GetName() == table) {
            return m_Records[i].GetBuf(column, st);
        }
    }
    return NULL;
}

DWORD CStorage::CalcUniqID(void) {
    DWORD x = 0xFFFFFFFF;
    for (int i = 0; i < m_RecordsCnt; ++i) {
        x = m_Records[i].CalcUniqID(x);
    }
    return x;
}

void CStorage::Save(const wchar *fname, bool compression) {
    CBuf buf(m_Heap);
    Save(buf, compression);
    buf.SaveInFile(fname);

    // CBuf    buf1(m_Heap);
    // CBuf    buf2(m_Heap);

    // ZL03_Compression(buf1, buf);
    // buf1.SaveInFile(CWStr(fname)+L"1");

    // ZL03_UnCompress(buf2, buf1);
    // buf2.SaveInFile(CWStr(fname)+L"2");
}

bool CStorage::Load(const wchar *fname) {
    CBuf buf(m_Heap);
    buf.LoadFromFile(fname);
    return Load(buf);
}

void CStorage::Save(CBuf &buf, bool compression) {
    buf.Clear();
    buf.Dword(0x47525453);
    buf.Dword(compression ? 1 : 0);  // version
    buf.Dword(m_RecordsCnt);         // records count

    for (int i = 0; i < m_RecordsCnt; ++i) {
        m_Records[i].Save(buf, false);
    }

    if (compression) {
        CBuf buf2(buf.m_Heap);
        ZL03_Compression(buf2, buf.Buff<BYTE>() + 8, buf.Len() - 8);
        buf.SetLenNoShrink(8);
        buf.Expand(buf2.Len());
        buf.SetLenNoShrink(8 + buf2.Len());
        memcpy(buf.Buff<BYTE>() + 8, buf2.Get(), buf2.Len());
    }
}

bool CStorage::Load(CBuf &buf_in) {
    buf_in.Pointer(0);
    DWORD tag = buf_in.Dword();
    if (tag != 0x47525453)
        return false;
    DWORD ver = buf_in.Dword();

    if (ver > 1)
        return false;

    CBuf buf2(buf_in.m_Heap);
    CBuf *buf = &buf_in;

    if (ver == 1) {
        // compression!
        ZL03_UnCompress(buf2, buf_in.Buff<BYTE>() + 8, buf_in.Len() - 8);
        buf = &buf2;
        buf2.Pointer(0);
    }

    if (m_Records) {
        for (int i = 0; i < m_RecordsCnt; ++i) {
            m_Records[i].~CStorageRecord();
        }
        HFree(m_Records, m_Heap);
        m_Records = NULL;
    }

    m_RecordsCnt = buf->Dword();
    if (m_RecordsCnt == 0)
        return true;

    m_Records = (CStorageRecord *)HAlloc(sizeof(CStorageRecord) * m_RecordsCnt, m_Heap);
    for (int i = 0; i < m_RecordsCnt; ++i) {
        m_Records[i].CStorageRecord::CStorageRecord(m_Heap);
        if (!m_Records[i].Load(*buf)) {
            for (int j = 0; j <= i; ++j) {
                m_Records[j].~CStorageRecord();
            }
            HFree(m_Records, m_Heap);
            m_Records = NULL;
            m_RecordsCnt = 0;
            return false;
        }
    }
    return true;
}

void CStorage::StoreBlockPar(const wchar *root, const CBlockPar &bp) {
    DTRACE();

    CWStr root_name(root, m_Heap);

    CStorageRecord sr(root_name, m_Heap);
    sr.AddItem(CStorageRecordItem(CWStr(L"0", m_Heap), ST_WCHAR));
    sr.AddItem(CStorageRecordItem(CWStr(L"1", m_Heap), ST_WCHAR));
    sr.AddItem(CStorageRecordItem(CWStr(L"2", m_Heap), ST_WCHAR));
    sr.AddItem(CStorageRecordItem(CWStr(L"3", m_Heap), ST_WCHAR));
    AddRecord(sr);

    CDataBuf *propkey = GetBuf(root, L"0", ST_WCHAR);
    CDataBuf *propval = GetBuf(root, L"1", ST_WCHAR);

    int cnt = bp.ParCount();
    for (int i = 0; i < cnt; ++i) {
        propkey->AddWStr(bp.ParGetName(i));
        propval->AddWStr(bp.ParGet(i));
    }

    int uniq = 0;

    propkey = GetBuf(root, L"2", ST_WCHAR);
    propval = GetBuf(root, L"3", ST_WCHAR);
    cnt = bp.BlockCount();

    CWStr uniq_s(m_Heap);
    for (int i = 0; i < cnt; ++i) {
        propkey->AddWStr(bp.BlockGetName(i));

        do {
            uniq_s.Set(uniq);
            uniq++;
        }
        while (IsTablePresent(uniq_s.Get()));

        propval->AddWStr(uniq_s);
        StoreBlockPar(uniq_s.Get(), *bp.BlockGet(i));
    }
}

void CStorage::RestoreBlockPar(const wchar *root, CBlockPar &bp) {
    CDataBuf *propkey = GetBuf(root, L"0", ST_WCHAR);
    CDataBuf *propval = GetBuf(root, L"1", ST_WCHAR);

    for (DWORD i = 0; i < propkey->GetArraysCount(); ++i) {
        bp.ParAdd(propkey->GetAsWStr(i), propval->GetAsWStr(i));
    }

    propkey = GetBuf(root, L"2", ST_WCHAR);
    propval = GetBuf(root, L"3", ST_WCHAR);

    for (DWORD i = 0; i < propkey->GetArraysCount(); ++i) {
        CBlockPar *bp1 = bp.BlockAdd(propkey->GetAsWStr(i));
        RestoreBlockPar(propval->GetAsWStr(i).Get(), *bp1);
    }
}

}  // namespace Base