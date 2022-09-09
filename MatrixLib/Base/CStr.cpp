// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <cctype>
#include <locale>
#include <codecvt>
#include <cstdio>


#include <inttypes.h>

#include "Base.pch"

#include "CStr.hpp"

namespace Base {

void CStr::Clear() {
    m_Len = 0;
    if (m_Str != NULL) {
        if (m_MaxLen > 64) {
            m_MaxLen = 16;
            m_Str = (char *)HAllocEx(m_Str, m_MaxLen + 1, nullptr);
        }
        *m_Str = 0;
    }
}

void CStr::Set(const CStr &cstr) {
    Tream(cstr.m_Len);
    m_Len = cstr.m_Len;
    memcpy(m_Str, cstr.m_Str, m_Len);
    m_Str[m_Len] = 0;
}

void CStr::Add(const CStr &cstr) {
    if (cstr.m_Len < 1)
        return;
    Tream(m_Len + cstr.m_Len);

    memcpy(m_Str + m_Len, cstr.m_Str, cstr.m_Len);

    m_Len += cstr.m_Len;
    m_Str[m_Len] = 0;
}

std::string CStr::from_wstring(const std::wstring& wstr)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

std::wstring CStr::to_wstring(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

std::string CStr::to_upper(const std::string& str)
{
    std::string res{str};
    for (auto& sym : res)
    {
        sym = std::toupper(sym);
    }
    return res;
}

}  // namespace Base
