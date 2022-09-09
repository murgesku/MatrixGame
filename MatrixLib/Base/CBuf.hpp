// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef CBUF_HEADER
#define CBUF_HEADER
//#pragma once

#include "CMain.hpp"
#include "CStr.hpp"
#include "CWStr.hpp"
#include "Mem.hpp"
#include "Tracer.hpp"

namespace Base {

class BASE_API CBuf : public CMain {
public:
    CHeap *m_Heap;
    int m_Len;      // Кол-во данных
    int m_Max;      // Размер буфера
    int m_Add;      // На сколько увеличивается буфер
    int m_Pointer;  // Указатель
    BYTE *m_Buf;    // Буфер
public:
    CBuf(CHeap *heap = NULL, int add = 32);
    ~CBuf();

    void Clear(void);
    void ClearFull(void);  // А также освобождает память

    void SetGranula(int sz) { m_Add = sz; }

    void *Get(void) const { return m_Buf; }
    template <class D>
    D *Buff(void) {
        return (D *)m_Buf;
    }
    template <class D>
    D *BuffEnd(void) {
        return (D *)(m_Buf + m_Len);
    }
    int Len(void) const { return m_Len; }
    void Len(int zn);
    void SetLenNoShrink(int len) {
        ASSERT(len <= m_Max);
        m_Len = len;
    }

    inline void TestGet(int len) {
        if ((m_Pointer + len) > m_Len)
            ERROR_E;
    }
    void TestAdd(int len) {
        if ((m_Pointer + len) > m_Max) {
            m_Max = m_Pointer + len + m_Add;
            m_Buf = (BYTE *)HAllocEx(m_Buf, m_Max, m_Heap);
        }
        m_Len += len;
    }  // Can change m_Len
    void Expand(int sz) {
        m_Len += sz;
        if (m_Len > m_Max) {
            m_Max = m_Len + m_Add;
            m_Buf = (BYTE *)HAllocEx(m_Buf, m_Max, m_Heap);
        }
    }

    int Pointer(void) const { return m_Pointer; }
    void Pointer(int zn) {
        if ((zn < 0) || (zn > m_Len))
            ERROR_E;
        m_Pointer = zn;
    }

    bool Bool(void) {
        TestGet(sizeof(bool));
        m_Pointer += sizeof(bool);
        return *(bool *)(m_Buf + m_Pointer - sizeof(bool));
    }
    byte Byte(void) {
        TestGet(sizeof(byte));
        m_Pointer += sizeof(byte);
        return *(byte *)(m_Buf + m_Pointer - sizeof(byte));
    }
    char Char(void) {
        TestGet(sizeof(char));
        m_Pointer += sizeof(char);
        return *(char *)(m_Buf + m_Pointer - sizeof(char));
    }
    word Word(void) {
        TestGet(sizeof(word));
        m_Pointer += sizeof(word);
        return *(word *)(m_Buf + m_Pointer - sizeof(word));
    }
    short Short(void) {
        TestGet(sizeof(short));
        m_Pointer += sizeof(short);
        return *(short *)(m_Buf + m_Pointer - sizeof(short));
    }
    dword Dword(void) {
        TestGet(sizeof(dword));
        m_Pointer += sizeof(dword);
        return *(dword *)(m_Buf + m_Pointer - sizeof(dword));
    }
    long Long(void) {
        TestGet(sizeof(long));
        m_Pointer += sizeof(long);
        return *(long *)(m_Buf + m_Pointer - sizeof(long));
    }
    int Int(void) {
        TestGet(sizeof(int));
        m_Pointer += sizeof(int);
        return *(int *)(m_Buf + m_Pointer - sizeof(int));
    }
    int64 Int64(void) {
        TestGet(sizeof(int64));
        m_Pointer += sizeof(int64);
        return *(int64 *)(m_Buf + m_Pointer - sizeof(int64));
    }
    float Float(void) {
        TestGet(sizeof(float));
        m_Pointer += sizeof(float);
        return *(float *)(m_Buf + m_Pointer - sizeof(float));
    }
    double Double(void) {
        TestGet(sizeof(double));
        m_Pointer += sizeof(double);
        return *(double *)(m_Buf + m_Pointer - sizeof(double));
    }

    template <class D>
    D Any(void) {
        DTRACE();
        TestGet(sizeof(D));
        m_Pointer += sizeof(D);
        return *(D *)(m_Buf + m_Pointer - sizeof(D));
    }
    template <class D>
    const D &AnyStruct(void) {
        DTRACE();
        TestGet(sizeof(D));
        m_Pointer += sizeof(D);
        return *(D *)(m_Buf + m_Pointer - sizeof(D));
    }

    void Bool(bool zn) {
        TestAdd(sizeof(bool));
        *(bool *)(m_Buf + m_Pointer) = zn;
        m_Pointer += sizeof(bool);
    }
    void Byte(byte zn) {
        TestAdd(sizeof(byte));
        *(byte *)(m_Buf + m_Pointer) = zn;
        m_Pointer += sizeof(byte);
    }
    void Char(char zn) {
        TestAdd(sizeof(char));
        *(char *)(m_Buf + m_Pointer) = zn;
        m_Pointer += sizeof(char);
    }
    void Word(word zn) {
        TestAdd(sizeof(word));
        *(word *)(m_Buf + m_Pointer) = zn;
        m_Pointer += sizeof(word);
    }
    void Short(short zn) {
        TestAdd(sizeof(short));
        *(short *)(m_Buf + m_Pointer) = zn;
        m_Pointer += sizeof(short);
    }
    void Dword(dword zn) {
        TestAdd(sizeof(dword));
        *(dword *)(m_Buf + m_Pointer) = zn;
        m_Pointer += sizeof(dword);
    }
    void Long(long zn) {
        TestAdd(sizeof(long));
        *(long *)(m_Buf + m_Pointer) = zn;
        m_Pointer += sizeof(long);
    }
    void Int(int zn) {
        TestAdd(sizeof(int));
        *(int *)(m_Buf + m_Pointer) = zn;
        m_Pointer += sizeof(int);
    }
    void Int64(int64 zn) {
        TestAdd(sizeof(int64));
        *(int64 *)(m_Buf + m_Pointer) = zn;
        m_Pointer += sizeof(int64);
    }
    void Float(float zn) {
        TestAdd(sizeof(float));
        *(float *)(m_Buf + m_Pointer) = zn;
        m_Pointer += sizeof(float);
    }
    void Double(double zn) {
        TestAdd(sizeof(double));
        *(double *)(m_Buf + m_Pointer) = zn;
        m_Pointer += sizeof(double);
    }

    template <class D>
    void Any(D zn) {
        TestAdd(sizeof(D));
        *(D *)(m_Buf + m_Pointer) = zn;
        m_Pointer += sizeof(D);
    }
    template <class D>
    void AnyStruct(const D &zn) {
        TestAdd(sizeof(D));
        *(D *)(m_Buf + m_Pointer) = zn;
        m_Pointer += sizeof(D);
    }

    void BufAdd(const void *buf, int len) {
        if (len <= 0)
            return;
        TestAdd(len);
        CopyMemory(m_Buf + m_Pointer, buf, len);
        m_Pointer += len;
    }
    void BufGet(void *buf, int len) {
        if (len <= 0)
            return;
        TestGet(len);
        memcpy(buf, m_Buf + m_Pointer, len);
        m_Pointer += len;
    }

    void ByteLoop(byte zn, int cnt) {
        if (cnt <= 0)
            return;
        TestAdd(cnt);
        memset(m_Buf + m_Pointer, zn, cnt);
        m_Pointer += cnt;
    }
    void WordLoop(word zn, int cnt) {
        if (cnt <= 0)
            return;
        TestAdd(cnt * 2);
        for (int i = 0; i < cnt; i++, m_Pointer += 2)
            *(word *)(m_Buf + m_Pointer) = zn;
    }

    int StrLen(void);
    CStr Str(void) {
        int len = StrLen();
        char *abuf = (char *)(m_Buf + m_Pointer);
        m_Pointer += len + 1;
        if (m_Pointer > m_Len)
            m_Pointer = m_Len;
        if (len > 0)
            return CStr(abuf, len);
        else
            return CStr();
    }
    void Str(const CStr &str) {
        int len = str.length();
        if (len > 0)
            BufAdd(str.c_str(), len + 1);
        else
            Byte(0);
    }
    void Str(const char *str, int len) {
        if (len > 0)
            BufAdd(str, len);
        Byte(0);
    }
    void Str(const char *str) {
        int len = (int)strlen(str);
        if (len > 0)
            BufAdd(str, len + 1);
        else
            Byte(0);
    }
    void StrNZ(const CStr &str) {
        int len = str.length();
        if (len > 0)
            BufAdd(str.c_str(), len);
    }
    void StrNZ(const char *str, int len) {
        if (len > 0)
            BufAdd(str, len);
    }
    void StrNZ(const char *str) {
        int len = (int)strlen(str);
        if (len > 0)
            BufAdd(str, len);
    }

    int WStrLen(void);
    CWStr WStr(void) {
        int len = WStrLen();
        wchar *abuf = (wchar *)(m_Buf + m_Pointer);
        m_Pointer += ((len + 1) << 1);
        if (m_Pointer > m_Len)
            m_Pointer = m_Len;
        if (len > 0)
            return CWStr(abuf, len, m_Heap);
        else
            return CWStr(m_Heap);
    }
    void WStr(const CWStr &str) {
        int len = str.GetLen();
        if (len > 0)
            BufAdd(str.Get(), (len + 1) << 1);
        else
            Word(0);
    }
    void WStr(const wchar *str, int len) {
        if (len > 0)
            BufAdd(str, len << 1);
        Word(0);
    }
    void WStr(const wchar *str) {
        int len = Base::WStrLen(str);
        if (len > 0)
            BufAdd(str, (len + 1) << 1);
        else
            Word(0);
    }
    void WStrNZ(const CWStr &str) {
        int len = str.GetLen();
        if (len > 0)
            BufAdd(str.Get(), len << 1);
    }
    void WStrNZ(const wchar *str, int len) {
        if (len > 0)
            BufAdd(str, len << 1);
    }
    void WStrNZ(const wchar *str) {
        int len = Base::WStrLen(str);
        if (len > 0)
            BufAdd(str, len << 1);
    }

    int StrTextLen(void);
    CStr StrText(void) {
        char ch;
        int len = StrTextLen();
        char *abuf = (char *)(m_Buf + m_Pointer);
        m_Pointer += len;
        if (m_Pointer < m_Len) {
            ch = *(char *)(m_Buf + m_Pointer);
            if (ch == 0 || ch == 0x0d || ch == 0x0a)
                m_Pointer++;
            if (m_Pointer < m_Len) {
                ch = *(char *)(m_Buf + m_Pointer);
                if (ch == 0 || ch == 0x0d || ch == 0x0a)
                    m_Pointer++;
            }
        }
        if (len > 0)
            return CStr(abuf, len);
        else
            return CStr();
    }
    void StrText(CStr &str) {
        int len = str.length();
        if (len > 0)
            BufAdd(str.c_str(), len);
        Word(0x0a0d);
    }

    int WStrTextLen(void);
    CWStr WStrText(void) {
        wchar ch;
        int len = WStrTextLen();
        wchar *abuf = (wchar *)(m_Buf + m_Pointer);
        m_Pointer += len << 1;
        if (m_Pointer + 1 < m_Len) {
            ch = *(wchar *)(m_Buf + m_Pointer);
            if (ch == 0 || ch == 0x0d || ch == 0x0a)
                m_Pointer += 2;
            if (m_Pointer + 1 < m_Len) {
                ch = *(wchar *)(m_Buf + m_Pointer);
                if (ch == 0 || ch == 0x0d || ch == 0x0a)
                    m_Pointer += 2;
            }
        }
        if (len > 0)
            return CWStr(abuf, len);
        else
            return CWStr();
    }
    void WStrText(CWStr &str) {
        int len = str.GetLen();
        if (len > 0)
            BufAdd(str.Get(), len << 1);
        Dword(0x000a000d);
    }

    void LoadFromFile(const wchar *filename, int len);
    void LoadFromFile(const wchar *filename) { LoadFromFile(filename, Base::WStrLen(filename)); }
    void LoadFromFile(const CWStr &filename) { LoadFromFile(filename.Get(), filename.GetLen()); }
    void SaveInFile(const wchar *filename, int len) const;
    void SaveInFile(const wchar *filename) const { SaveInFile(filename, Base::WStrLen(filename)); }
    void SaveInFile(const CWStr &filename) const { SaveInFile(filename.Get(), filename.GetLen()); }
};

}  // namespace Base

#endif
