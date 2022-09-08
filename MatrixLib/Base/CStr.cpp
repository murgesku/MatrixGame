// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <cctype>
#include <locale>
#include <codecvt>
#include <cstdio>
#include <stdexcept>

#include <inttypes.h>

#include "Base.pch"

#include "CStr.hpp"

namespace Base {

CStr::CStr(int zn)
  : CMain()
  , m_MaxLen{16}
  , m_Len{0}
{
    m_Str = (char*)HAlloc(m_MaxLen + 1, nullptr);
    m_Str[0] = 0;
    Set(zn);
}

CStr::CStr(double zn, int zpz)
  : CMain()
  , m_MaxLen{16}
  , m_Len{0}
{
    m_Str = (char*)HAlloc(m_MaxLen + 1, nullptr);
    m_Str[0] = 0;
    Set(zn, zpz);
}

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

void CStr::SetLen(int len) {
    if (len < 1) {
        Clear();
        return;
    }
    Tream(len);
    m_Len = len;
    m_Str[m_Len] = 0;
}

void CStr::Set(const CStr &cstr) {
    Tream(cstr.m_Len);
    m_Len = cstr.m_Len;
    memcpy(m_Str, cstr.m_Str, m_Len);
    m_Str[m_Len] = 0;
}

void CStr::Set(const char *value) {
    m_Len = strlen(value);
    Tream(m_Len);
    memcpy(m_Str, value, m_Len);
    m_Str[m_Len] = 0;
}

void CStr::Set(const char *value, int lstr) {
    std::string str(value, lstr);
    Set(str.c_str());
}

void CStr::Set(char sim) {
    std::string str(1, sim);
    Set(str.c_str());
}

void CStr::Set(char sim, int count) {
    std::string str(count, sim);
    Set(str.c_str());
}

void CStr::Set(int zn) {
    auto str = CStr::format("%d", zn);
    Set(str.c_str());
}

void CStr::Set(double zn, int zpz) {
    auto str = CStr::format("%0.*f", zpz, zn);
    Set(str.c_str());
}

void CStr::SetHex(void *zn) {
    auto str = CStr::format("%0" PRIuPTR, reinterpret_cast<uintptr_t>(zn));
    Set(str.c_str());
}

void CStr::SetHex(BYTE zn) {
    auto str = CStr::format("%02u", static_cast<uint8_t>(zn));
    Set(str.c_str());
}

void CStr::Add(const CStr &cstr) {
    if (cstr.m_Len < 1)
        return;
    Tream(m_Len + cstr.m_Len);

    memcpy(m_Str + m_Len, cstr.m_Str, cstr.m_Len);

    m_Len += cstr.m_Len;
    m_Str[m_Len] = 0;
}

void CStr::Add(const char *str) {
    int lstr = strlen(str);
    if (lstr < 1)
        return;

    Tream(m_Len + lstr);

    memcpy(m_Str + m_Len, str, lstr);

    m_Len += lstr;
    m_Str[m_Len] = 0;
}

void CStr::Add(const char *str, int lstr) {
    if (lstr < 1)
        return;

    Tream(m_Len + lstr);

    memcpy(m_Str + m_Len, str, lstr);

    m_Len += lstr;
    m_Str[m_Len] = 0;
}

void CStr::Add(char sim) {
    Tream(m_Len + 1);

    m_Str[m_Len] = sim;

    m_Len += 1;
    m_Str[m_Len] = 0;
}

void CStr::Add(char sim, int count) {
    Tream(m_Len + count);

    for (int i = 0; i < count; i++)
        m_Str[m_Len + i] = sim;

    m_Len += count;
    m_Str[m_Len] = 0;
}

template<typename... Args>
std::string CStr::format(const char* format, Args... args)
{
    char buf[10240];
    if (std::sprintf(buf, format, args...) < 0)
    {
        throw std::runtime_error("sprintf() failed");
    }
    return std::string{buf};
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
