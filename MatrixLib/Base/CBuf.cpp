// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "CBuf.hpp"
#include "CException.hpp"
#include "Mem.hpp"
#include "CFile.hpp"

namespace Base {

void CBuf::Clear() {
    m_Buf.clear();
    m_Pointer = 0;
}

void CBuf::Len(size_t zn)
{
    if (zn == 0) {
        Clear();
        return;
    }
    m_Buf.resize(zn);
    if (m_Pointer < 0)
        m_Pointer = 0;
    else if (m_Pointer > m_Buf.size())
        m_Pointer = m_Buf.size();
}

size_t CBuf::StrLen(void) {
    size_t len = 0;
    for (size_t i = m_Pointer; i < m_Buf.size(); i++, len++) {
        if (*(char *)(m_Buf.data() + i) == 0)
            return len;
    }
    return 0;
}

size_t CBuf::WStrLen(void) {
    size_t len = 0;
    for (size_t i = m_Pointer; i + 1 < m_Buf.size(); i += 2, len++) {
        if (*(wchar *)(m_Buf.data() + i) == 0)
            return len;
    }
    return 0;
}

size_t CBuf::StrTextLen(void) {
    size_t len = 0;
    for (size_t i = m_Pointer; i < m_Buf.size(); i++, len++) {
        char ch = *(char *)(m_Buf.data() + i);
        if (ch == 0 || ch == 0x0d || ch == 0x0a)
            return len;
    }
    return len;
}

size_t CBuf::WStrTextLen(void) {
    size_t len = 0;
    for (size_t i = m_Pointer; i + 1 < m_Buf.size(); i += 2, len++) {
        wchar ch = *(wchar *)(m_Buf.data() + i);
        if (ch == 0 || ch == 0x0d || ch == 0x0a)
            return len;
    }
    return len;
}

void CBuf::LoadFromFile(const std::wstring &filename)
{
    Clear();
    CFile file(filename.c_str(), filename.length());
    file.OpenRead();
    Len(file.Size());
    file.Read(m_Buf.data(), m_Buf.size());
    file.Close();
}

void CBuf::SaveInFile(const std::wstring &filename) const
{
    if (m_Buf.size() < 0)
        return;

    CFile file(filename.c_str(), filename.length());
    file.Create();
    file.Write(const_cast<uint8_t*>(m_Buf.data()), m_Buf.size());
    file.Close();
}

}  // namespace Base