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
    CStr() : CMain(), m_MaxLen(0), m_Len(0), m_Str(NULL) {}
    CStr(const CStr &s) : CMain() { Init(s.m_Str, s.length()); }

    explicit CStr(const char *s) : CMain() { Init(s, int(strlen(s))); }
    explicit CStr(const char *s, int len) : CMain() { Init(s, len); }

    ~CStr() {
        if (m_Str != NULL) {
            HFree(m_Str, nullptr);
            m_Str = NULL;
        }
        m_Len = m_MaxLen = 0;
    }

    const char* c_str() const { return m_Str; }

    int length() const { return m_Len; }

    bool empty() const { return m_Len < 1; }

    CStr &operator=(const CStr &s) {
        if (this != &s) {
            Tream(s.m_Len);
            m_Len = s.m_Len;
            memcpy(m_Str, s.m_Str, m_Len);
            m_Str[m_Len] = 0;
        }
        return *this;
    }

    bool operator == (const CStr& that) { return strcmp(m_Str, that.m_Str) == 0; }
};

}  // namespace Base
