// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "CException.hpp"

#include <vector>
#include <cstdint>
#include <cstring> // for memset

namespace Base {

class BASE_API CBuf
{
private:
    size_t m_Pointer = 0;  // Указатель
    std::vector<uint8_t> m_Buf;
public:
    CBuf() = default;
    ~CBuf() = default;

    void Clear(void);

    void *Get(void) const { return const_cast<uint8_t*>(m_Buf.data()); }
    template <class D>
    D *Buff(void) {
        return (D*)m_Buf.data();
    }
    template <class D>
    D *BuffEnd(void) {
        return (D *)(m_Buf.data() + m_Buf.size());
    }
    size_t Len(void) const { return m_Buf.size(); }
    void Len(size_t zn);
    void SetLenNoShrink(size_t len)
    {
        ASSERT(len <= m_Buf.size());
        m_Buf.resize(len);
    }

    void Expand(size_t sz)
    {
        m_Buf.resize(m_Buf.size() + sz);
    }

    size_t Pointer(void) const { return m_Pointer; }
    void Pointer(size_t zn) {
        if (zn > m_Buf.size())
            ERROR_E;
        m_Pointer = zn;
    }

    ///////////////////////////////////////////////////////////////

    template <class D>
    D Get(void)
    {
        TestGet(sizeof(D));
        m_Pointer += sizeof(D);
        return *(D *)(m_Buf.data() + m_Pointer - sizeof(D));
    }

    void Get(void *buf, size_t len)
    {
        if (len == 0)
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

    void Add(const void *buf, size_t len) {
        if (len == 0)
            return;
        TestAdd(len);
        memcpy(m_Buf.data() + m_Pointer, buf, len);
        m_Pointer += len;
    }

    ///////////////////////////////////////////////////////////////

    void ByteLoop(uint8_t zn, size_t cnt) {
        if (cnt == 0)
            return;
        TestAdd(cnt);
        memset(m_Buf.data() + m_Pointer, zn, cnt);
        m_Pointer += cnt;
    }
    void WordLoop(uint16_t zn, size_t cnt) {
        if (cnt == 0)
            return;
        TestAdd(cnt * 2);
        for (size_t i = 0; i < cnt; i++, m_Pointer += 2)
            *(uint16_t *)(m_Buf.data() + m_Pointer) = zn;
    }

    std::string Str(void) {
        size_t len = StrLen();
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
        size_t len = WStrLen();
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
        size_t len = StrTextLen();
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
        size_t len = WStrTextLen();
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
        Add<uint32_t>(0x000a000d);
    }

    void LoadFromFile(const std::wstring &filename);
    void SaveInFile(const std::wstring &filename) const;

private:
    size_t StrLen(void);
    size_t WStrLen(void);
    size_t StrTextLen(void);
    size_t WStrTextLen(void);

    inline void TestGet(size_t len)
    {
        if (m_Pointer + len > m_Buf.size())
        {
            ERROR_E;
        }
    }
    void TestAdd(size_t len)
    {
        if (m_Pointer + len > m_Buf.size())
        {
            m_Buf.resize(m_Buf.size() + len);
        }
    }
};

}  // namespace Base
