// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include <string>

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

    explicit CStr(const CWStr &s);
    explicit CStr(const wchar *s);
    explicit CStr(const char *s) : CMain() { Init(s, int(strlen(s))); }
    explicit CStr(const char *s, int len) : CMain() { Init(s, len); }
    explicit CStr(char sim) : CMain(), m_MaxLen(16), m_Len(1) {
        m_Str = (char *)HAlloc(uint(m_MaxLen + 1), nullptr);
        m_Str[0] = sim;
        m_Str[1] = 0;
    };
    explicit CStr(char sim, int count) : CMain(), m_MaxLen(count + 16), m_Len(count) {
        m_Str = (char *)HAlloc(uint(m_MaxLen + 1), nullptr);
        memset(m_Str, int(sim), uint(count));
        m_Str[count] = 0;
    };
    explicit CStr(int zn);
    explicit CStr(double zn, int zpz = 8);

    ~CStr() {
        if (m_Str != NULL) {
            HFree(m_Str, nullptr);
            m_Str = NULL;
        }
        m_Len = m_MaxLen = 0;
    }

    // Clear - Очищает строку
    void Clear(void);

    void SetLen(int len);  // Установить длину строки (Выделение памяти если нужно)

    void Set(const CStr &cstr);
    void Set(const CWStr &cstr);
    void Set(const wchar *wstr);
    void Set(const char *str);
    void Set(const char *str, int lstr);
    void Set(char sim);
    void Set(char sim, int count);
    void Set(int zn);
    void Set(double zn, int zpz = 8);
    void SetHex(void *zn);
    void SetHex(BYTE zn);

    void Add(const CStr &cstr);
    void Add(const char *str);
    void Add(const char *str, int lstr);
    void Add(char sim);
    void Add(char sim, int count);
    void Add(int zn) { Add(CStr(zn)); }
    void Add(void *zn) {
        CStr s;
        s.SetHex(zn);
        Add(s);
    }
    void Add(BYTE zn) {
        CStr s;
        s.SetHex(zn);
        Add(s);
    }

    const char *Get(void) const { return m_Str; }

    int Len(void) const { return m_Len; }

    bool IsEmpty(void) const { return m_Len < 1; }

    void Split(CStr &beg, CStr &rem, const char *ogsim) const {
        const char *c = Get();
        int ogl = (int)strlen(ogsim);

        for (; *c; ++c) {
            const char *cc = ogsim;
            for (; *cc; ++cc) {
                if (*c == *cc) {
                    beg.Set(Get(), int(c - Get()));
                    rem.Set(c + 1);
                    return;
                }
            }
        }

        beg = *this;
        rem.Clear();
    }

    void LowerCase(int sme = 0, int len = -1);
    void UpperCase(int sme = 0, int len = -1);

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
    CStr &operator+=(const char *str) {
        Add(str);
        return *this;
    }
    CStr &operator+=(char sim) {
        Add(sim);
        return *this;
    }
    CStr &operator+=(int zn) {
        Add(zn);
        return *this;
    }
    CStr &operator+=(void *zn) {
        Add(zn);
        return *this;
    }

    friend CStr operator+(const CStr &s1, const CStr &s2) {
        CStr str(s1);
        str += s2;
        return str;
    }
    friend CStr operator+(const CStr &s1, const char *s2) {
        CStr str(s1);
        str += s2;
        return str;
    }

    bool operator == (const CStr& that) { return strcmp(m_Str, that.m_Str) == 0; }

    template<typename... Args>
    static std::string format(const char* format, Args... args);

    static std::string from_wstring(const std::wstring& wstr);
    static std::wstring to_wstring(const std::string& str);
};

}  // namespace Base
