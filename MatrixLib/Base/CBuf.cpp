// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "CBuf.hpp"
#include "CException.hpp"
#include "Mem.hpp"
#include "CFile.hpp"

namespace Base {

CBuf::CBuf(int add)
: m_Add(add)
{
    m_Len = 0;
    m_Max = 0;

    m_Pointer = 0;
}

CBuf::~CBuf() {
    ClearFull();
}

void CBuf::Clear() {
    m_Len = 0;
    m_Pointer = 0;
}

void CBuf::ClearFull(void) {
    m_Buf.clear();
    m_Len = 0;
    m_Max = 0;
    m_Pointer = 0;
}

void CBuf::Len(int zn) {
    if (zn <= 0) {
        Clear();
        return;
    }
    m_Len = m_Max = zn;
    m_Buf.resize(m_Max);
    if (m_Pointer < 0)
        m_Pointer = 0;
    else if (m_Pointer > m_Len)
        m_Pointer = m_Len;
}

int CBuf::StrLen(void) {
    int len = 0;
    for (int i = m_Pointer; i < m_Len; i++, len++) {
        if (*(char *)(m_Buf.data() + i) == 0)
            return len;
    }
    return 0;
}

int CBuf::WStrLen(void) {
    int len = 0;
    for (int i = m_Pointer; i + 1 < m_Len; i += 2, len++) {
        if (*(wchar *)(m_Buf.data() + i) == 0)
            return len;
    }
    return 0;
}

int CBuf::StrTextLen(void) {
    int len = 0;
    for (int i = m_Pointer; i < m_Len; i++, len++) {
        char ch = *(char *)(m_Buf.data() + i);
        if (ch == 0 || ch == 0x0d || ch == 0x0a)
            return len;
    }
    return len;
}

int CBuf::WStrTextLen(void) {
    int len = 0;
    for (int i = m_Pointer; i + 1 < m_Len; i += 2, len++) {
        wchar ch = *(wchar *)(m_Buf.data() + i);
        if (ch == 0 || ch == 0x0d || ch == 0x0a)
            return len;
    }
    return len;
}

void CBuf::LoadFromFile(const std::wstring &filename)
{
    ClearFull();
    CFile file(filename.c_str(), filename.length());
    file.OpenRead();
    Len(file.Size());
    file.Read(m_Buf.data(), m_Len);
    file.Close();
}

void CBuf::SaveInFile(const std::wstring &filename) const
{
    if (m_Len < 0)
        return;

    CFile file(filename.c_str(), filename.length());
    file.Create();
    file.Write(const_cast<uint8_t*>(m_Buf.data()), m_Len);
    file.Close();
}

}  // namespace Base