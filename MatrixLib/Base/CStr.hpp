// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include <string>
#include <stdexcept>
#include "CMain.hpp"

namespace Base {

class BASE_API CStr : public CMain {
    int m_MaxLen;
    int m_Len;
    char *m_Str;

    void Tream(int len) {
        if ((len == 0) || (len > m_MaxLen)) {
            m_MaxLen = len + 16;
            m_Str = (char *)HAllocEx(m_Str, uint(m_MaxLen + 1), nullptr);
        }
    }

    void Init(const char *src, int len) {
        m_Len = len;
        m_MaxLen = len + 16;
        m_Str = (char *)HAlloc(uint(m_MaxLen + 1), nullptr);
        if (len == 0) {
            *m_Str = 0;
        }
        else {
            memcpy(m_Str, src, uint(len + 1));
        }
    };

public:
    CStr(void) : CMain(), m_MaxLen(0), m_Len(0), m_Str(NULL) {}
    CStr(const CStr &s) : CMain() { Init(s.m_Str, s.Len()); }

    explicit CStr(const char *s) : CMain() { Init(s, int(strlen(s))); }
    explicit CStr(const char *s, int len) : CMain() { Init(s, len); }

    ~CStr() {
        if (m_Str != NULL) {
            HFree(m_Str, nullptr);
            m_Str = NULL;
        }
        m_Len = m_MaxLen = 0;
    }

    // Clear - Очищает строку
    void Clear(void);

    void Set(const CStr &cstr);

    void Add(const CStr &cstr);

    const char *Get(void) const { return m_Str; }

    int Len(void) const { return m_Len; }

    bool IsEmpty(void) const { return m_Len < 1; }

    // void Split(CStr &beg, CStr &rem, const char *ogsim) const {
    //     const char *c = Get();
    //     int ogl = (int)strlen(ogsim);

    //     for (; *c; ++c) {
    //         const char *cc = ogsim;
    //         for (; *cc; ++cc) {
    //             if (*c == *cc) {
    //                 beg.Set(CStr(Get(), int(c - Get())));
    //                 rem.Set(CStr(c + 1));
    //                 return;
    //             }
    //         }
    //     }

    //     beg = *this;
    //     rem.Clear();
    // }

    CStr &operator=(const CStr &s) {
        if (this != &s) {
            Set(s);
        }
        return *this;
    }

    CStr &operator+=(const CStr &str) {
        Add(str);
        return *this;
    }

    friend CStr operator+(const CStr &s1, const CStr &s2) {
        CStr str(s1);
        str += s2;
        return str;
    }

    bool operator == (const CStr& that) { return strcmp(m_Str, that.m_Str) == 0; }

    template<typename... Args>
    static std::string format(const char* format, Args... args)
    {
        char buf[10240];
        if (std::snprintf(buf, sizeof(buf), format, args...) < 0)
        {
            throw std::runtime_error("snprintf() failed");
        }
        return std::string{buf};
    }

    static std::string from_wstring(const std::wstring& wstr);
    static std::wstring to_wstring(const std::string& str);

    static std::string to_upper(const std::string& str);
};

}  // namespace Base
