// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "Base.pch"

#include "CStr.hpp"
// #include "Mem.hpp"
#include "CWStr.hpp"

#include <cctype>

namespace Base {

#define FI                                        \
    m_MaxLen = 16;                                \
    m_Len = 0;                                    \
    m_Str = NULL;                                 \
    m_Str = (char *)HAlloc(m_MaxLen + 1, nullptr); \
    m_Str[0] = 0

CStr::CStr(const CWStr &s) : CMain() {
    FI;
    Set(s);
}

CStr::CStr(const wchar *s) : CMain() {
    FI;
    Set(s);
}

CStr::CStr(int zn) : CMain() {
    FI;
    Set(zn);
}

CStr::CStr(double zn, int zpz) : CMain() {
    FI;
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

void CStr::Set(const CWStr &cstr) {
    Tream(cstr.GetLen());
    m_Len = cstr.GetLen();
    if (m_Len > 0) {
        if (!WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DEFAULTCHAR, cstr.Get(), cstr.GetLen(), m_Str, m_MaxLen,
                                 " ", NULL))
            ERROR_E;
    }
    m_Str[m_Len] = 0;
}

void CStr::Set(const wchar *wstr) {
    int len = WStrLen(wstr);
    Tream(len);
    m_Len = len;
    if (len > 0) {
        if (!WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DEFAULTCHAR, wstr, len, m_Str, m_MaxLen, " ", NULL))
            ERROR_E;
    }
    m_Str[m_Len] = 0;
}

void CStr::Set(const char *str) {
    m_Len = strlen(str);
    Tream(m_Len);
    memcpy(m_Str, str, m_Len);
    m_Str[m_Len] = 0;
}

void CStr::Set(const char *str, int lstr) {
    Tream(lstr);
    m_Len = lstr;
    memcpy(m_Str, str, m_Len);
    m_Str[m_Len] = 0;
}

void CStr::Set(char sim) {
    Tream(1);
    m_Len = 1;
    m_Str[0] = sim;
    m_Str[1] = 0;
}

void CStr::Set(char sim, int count) {
    Tream(count);
    m_Len = count;
    for (int i = 0; i < count; i++)
        m_Str[i] = sim;
    m_Str[m_Len] = 0;
}

void CStr::Set(int zn) {
    Tream(32u);
    m_Len = 32;

    int fm = 0;
    if (zn < 0) {
        fm = 1;
        zn = -zn;
    }

    while (zn > 0) {
        m_Str[m_Len] = char(zn - int(zn / 10) * 10) + char('0');
        m_Len++;
        zn = zn / 10;
    }
    if (fm) {
        m_Str[m_Len] = '-';
        m_Len++;
    }
    if (m_Len == 32 || (m_Len == 33 && fm)) {
        m_Str[m_Len] = char('0');
        m_Len++;
    }
    for (int i = 0; i < int(m_Len - 32); i++)
        m_Str[i] = m_Str[m_Len - 1 - i];
    m_Len -= 32;
    m_Str[m_Len] = 0;
}

void CStr::Set(double zn, int zpz) {
    CStr tstr;
    int dec, sign, le;
    int count = 0;
    char *st = _fcvt(zn, zpz, &dec, &sign);
    if (sign == 0)
        Clear();
    else
        Set((char)'-');
    le = strlen(st);
    if (dec < 0) {
        Add('0');
        if (le > 0) {
            Add('.');
            Add('0', -dec);
            for (int i = le - 1; i >= 0; i--)
                if (st[i] == '0')
                    count++;
                else
                    break;
            if (le > count) {
                for (int yu = 0; yu < le - count; yu++)
                    Add(char(st[yu]));
            }
        }
    }
    else {
        if (dec > 0) {
            for (int yu = 0; yu < dec; yu++)
                Add(char(st[yu]));
        }
        else {
            Add('0');
        }

        for (int i = le - 1; i >= dec; i--)
            if (st[i] == '0')
                count++;
            else
                break;
        if (dec < le - count) {
            Add('.');
            for (int yu = 0; yu < le - count - dec; yu++)
                Add(char(st[dec + yu]));
        }
    }
}

char *Str_CH = "0123456789ABCDEF";
void CStr::SetHex(void *zn) {
    DWORD dw = DWORD(zn);
    m_Len = 8;
    Tream(m_Len);
    m_Str[m_Len] = 0;
    for (int i = 0; i < 8; i++) {
        m_Str[7 - i] = Str_CH[(dw >> (i * 4)) & 0x0f];
    }
}

void CStr::SetHex(BYTE zn) {
    DWORD dw = (DWORD)zn;
    m_Len = 2;
    Tream(m_Len);
    m_Str[m_Len] = 0;
    for (int i = 0; i < 2; i++) {
        m_Str[1 - i] = Str_CH[(dw >> (i * 4)) & 0x0f];
    }
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

void CStr::LowerCase(int sme, int len) {
    if (len < 0)
        len = Len() - sme;
    if ((sme < 0) || (len <= 0) || ((sme + len) > Len()))
        return;

    for (int i = 0; i < len; ++i)
    {
        m_Str[sme + i] = static_cast<char>(std::tolower(m_Str[sme + i]));
    }
}

void CStr::UpperCase(int sme, int len) {
    if (len < 0)
        len = Len() - sme;
    if ((sme < 0) || (len <= 0) || ((sme + len) > Len()))
        return;

    for (int i = 0; i < len; ++i)
    {
        m_Str[sme + i] = static_cast<char>(std::toupper(m_Str[sme + i]));
    }
}

}  // namespace Base
