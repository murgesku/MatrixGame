// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "CException.hpp"
#include "CHeap.hpp"
#include "Mem.hpp"
#include "Tracer.hpp"

#include <vector>

namespace Base {

class BASE_API CBuf
{
private:
    int m_Pointer;  // Указатель
    std::vector<uint8_t> m_Buf;
public:
    CBuf();
    ~CBuf();

    void Clear(void);
    void ClearFull(void);  // А также освобождает память

    void *Get(void) const { return const_cast<uint8_t*>(m_Buf.data()); }
    template <class D>
    D *Buff(void) {
        return (D*)m_Buf.data();
    }
    template <class D>
    D *BuffEnd(void) {
        return (D *)(m_Buf.data() + m_Buf.size());
    }
    int Len(void) const { return m_Buf.size(); }
    void Len(int zn);
    void SetLenNoShrink(int len)
    {
        ASSERT(len <= m_Buf.size());
        m_Buf.resize(len);
    }

    void Expand(int sz)
    {
        m_Buf.resize(m_Buf.size() + sz);
    }

    int Pointer(void) const { return m_Pointer; }
    void Pointer(int zn) {
        if ((zn < 0) || (zn > m_Buf.size()))
            ERROR_E;
        m_Pointer = zn;
    }

    ///////////////////////////////////////////////////////////////

    template <class D>
    D Get(void)
    {
        DTRACE();
        TestGet(sizeof(D));
        m_Pointer += sizeof(D);
        return *(D *)(m_Buf.data() + m_Pointer - sizeof(D));
    }

    void Get(void *buf, int len)
    {
        if (len <= 0)
            return;
        TestGet(len);
        memcpy(buf, m_Buf.data() + m_Pointer, len);
        m_Pointer += len;
    }

    ///////////////////////////////////////////////////////////////

    template <class D>
    void Add(D zn) {
        TestAdd(sizeof(D));
        *(D*)(m_Buf.data() + m_Pointer) = zn;
        m_Pointer += sizeof(D);
    }

    void Add(const void *buf, int len) {
        if (len <= 0)
            return;
        TestAdd(len);
        memcpy(m_Buf.data() + m_Pointer, buf, len);
        m_Pointer += len;
    }

    ///////////////////////////////////////////////////////////////

    void ByteLoop(uint8_t zn, int cnt) {
        if (cnt <= 0)
            return;
        TestAdd(cnt);
        memset(m_Buf.data() + m_Pointer, zn, cnt);
        m_Pointer += cnt;
    }
    void WordLoop(uint16_t zn, int cnt) {
        if (cnt <= 0)
            return;
        TestAdd(cnt * 2);
        for (int i = 0; i < cnt; i++, m_Pointer += 2)
            *(uint16_t *)(m_Buf.data() + m_Pointer) = zn;
    }

    std::string Str(void) {
        int len = StrLen();
        char *abuf = (char *)(m_Buf.data() + m_Pointer);
        m_Pointer += len + 1;
        if (m_Pointer > m_Buf.size())
            m_Pointer = m_Buf.size();
        if (len > 0)
            return std::string(abuf, len);
        else
            return std::string();
    }
    void Str(const std::string& str) {
        if (!str.empty())
            Add(str.c_str(), str.length() + 1);
        else
            Add<uint8_t>(0);
    }
    void StrNZ(const std::string& str)
    {
        if (!str.empty())
            Add(str.c_str(), str.length());
    }

    std::wstring WStr(void) {
        int len = WStrLen();
        wchar *abuf = (wchar *)(m_Buf.data() + m_Pointer);
        m_Pointer += ((len + 1) << 1);
        if (m_Pointer > m_Buf.size())
            m_Pointer = m_Buf.size();
        if (len > 0)
            return std::wstring(abuf, len);
        else
            return {};
    }
    void WStr(const std::wstring &str) {
        if (!str.empty())
            Add(str.c_str(), (str.length() + 1) * 2);
        else
            Add<uint16_t>(0);
    }
    void WStrNZ(const std::wstring &str) {
        if (!str.empty())
            Add(str.c_str(), str.length() * 2);
    }

    std::string StrText(void) {
        char ch;
        int len = StrTextLen();
        char *abuf = (char *)(m_Buf.data() + m_Pointer);
        m_Pointer += len;
        if (m_Pointer < m_Buf.size()) {
            ch = *(char *)(m_Buf.data() + m_Pointer);
            if (ch == 0 || ch == 0x0d || ch == 0x0a)
                m_Pointer++;
            if (m_Pointer < m_Buf.size()) {
                ch = *(char *)(m_Buf.data() + m_Pointer);
                if (ch == 0 || ch == 0x0d || ch == 0x0a)
                    m_Pointer++;
            }
        }
        if (len > 0)
            return std::string(abuf, len);
        else
            return std::string();
    }
    void StrText(const std::string& str)
    {
        StrNZ(str);
        Add<uint16_t>(0x0a0d);
    }

    std::wstring WStrText(void) {
        wchar ch;
        int len = WStrTextLen();
        wchar *abuf = (wchar *)(m_Buf.data() + m_Pointer);
        m_Pointer += len << 1;
        if (m_Pointer + 1 < m_Buf.size()) {
            ch = *(wchar *)(m_Buf.data() + m_Pointer);
            if (ch == 0 || ch == 0x0d || ch == 0x0a)
                m_Pointer += 2;
            if (m_Pointer + 1 < m_Buf.size()) {
                ch = *(wchar *)(m_Buf.data() + m_Pointer);
                if (ch == 0 || ch == 0x0d || ch == 0x0a)
                    m_Pointer += 2;
            }
        }
        if (len > 0)
            return std::wstring(abuf, len);
        else
            return {};
    }
    void WStrText(const std::wstring& str)
    {
        WStrNZ(str);
        Add<DWORD>(0x000a000d);
    }

    void LoadFromFile(const std::wstring &filename);
    void SaveInFile(const std::wstring &filename) const;

private:
    int StrLen(void);
    int WStrLen(void);
    int StrTextLen(void);
    int WStrTextLen(void);

    inline void TestGet(int len)
    {
        if ((m_Pointer + len) > m_Buf.size())
        {
            ERROR_E;
        }
    }
    void TestAdd(int len)
    {
        if (m_Pointer + len > m_Buf.size())
        {
            m_Buf.resize(m_Buf.size() + len);
        }
    }
};

}  // namespace Base
