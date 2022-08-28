// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "Base.pch"

#include "CStr.hpp"
#include "CException.hpp"
#include "CHeap.hpp"
#include "Mem.hpp"
#include "CWStr.hpp"

namespace Base {

#define FI                                        \
    m_MaxLen = 16;                                \
    m_Len = 0;                                    \
    m_Str = NULL;                                 \
    m_Str = (char *)HAlloc(m_MaxLen + 1, m_Heap); \
    m_Str[0] = 0

CStr::CStr(const CWStr &s, CHeap *heap) : CMain() {
    m_Heap = heap;

    FI;
    Set(s);
}

CStr::CStr(const wchar *s, CHeap *heap) : CMain() {
    m_Heap = heap;

    FI;
    Set(s);
}

CStr::CStr(int zn, CHeap *heap) : CMain() {
    m_Heap = heap;

    FI;

    Set(zn);
}

CStr::CStr(double zn, int zpz, CHeap *heap) : CMain() {
    m_Heap = heap;

    FI;

    Set(zn, zpz);
}

/*CStr::CStr(void * zn, CHeap * heap) : CMain()
{
    m_Heap=heap;

    FI;
    Set(zn);
}

CStr::CStr(BYTE zn, CHeap * heap) : CMain()
{
    m_Heap=heap;

    FI;
    Set(zn);
}*/

void CStr::Clear() {
    m_Len = 0;
    if (m_Str != NULL) {
        if (m_MaxLen > 64) {
            m_MaxLen = 16;
            m_Str = (char *)HAllocEx(m_Str, m_MaxLen + 1, m_Heap);
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
    CStr tstr(m_Heap);
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

// int CStr::GetInt() const {
//     int zn = 0;
//     char ch;
//     int i;
//     for (i = 0; i < m_Len; ++i) {
//         ch = m_Str[i] - '0';
//         if (ch <= 10)
//             zn = zn * 10 + ch;
//     }
//     for (i = 0; i < m_Len; i++)
//         if (m_Str[i] == '-') {
//             zn = -zn;
//             break;
//         }
//     return zn;
// }

// double CStr::GetDouble() const {
//     int i;
//     double zn = 0.0;

//     char ch;
//     for (i = 0; i < m_Len; i++) {
//         ch = m_Str[i];
//         if (ch >= '0' && ch <= '9')
//             zn = zn * 10.0 + (double)(ch - '0');
//         else if (ch == '.')
//             break;
//     }
//     i++;
//     double tra = 10.0;
//     for (i; i < m_Len; i++) {
//         ch = m_Str[i];
//         if (ch >= '0' && ch <= '9') {
//             zn = zn + ((double)(ch - '0')) / tra;
//             tra *= 10.0;
//         }
//     }
//     for (i = 0; i < m_Len; i++)
//         if (m_Str[i] == '-') {
//             zn = -zn;
//             break;
//         }

//     return zn;
// }

// int CStr::GetHex() const {
//     int zn = 0;
//     int i;

//     char ch;
//     for (i = 0; i < m_Len; i++) {
//         ch = m_Str[i];

//         ch -= '0';
//         if (ch > 9)
//             ch = (ch & (~32)) - ('A' - '0' + 10);
//         zn = zn * 16 + ch;
//     }
//     for (i = 0; i < m_Len; i++)
//         if (m_Str[i] == '-') {
//             zn = -zn;
//             break;
//         }

//     return zn;
// }

// DWORD CStr::GetHexUnsigned() const {
//     DWORD zn = 0;
//     int i;

//     char ch;
//     for (i = 0; i < m_Len; i++) {
//         ch = m_Str[i];

//         ch -= '0';
//         if (ch > 9)
//             ch = (ch & (~32)) - ('A' - '0' + 10);
//         zn = (zn << 4) + ch;
//     }
//     return zn;
// }

// bool CStr::IsOnlyInt() const {
//     int i;
//     if (m_Len < 1)
//         return 0;
//     for (i = 0; i < m_Len; i++)
//         if (m_Str[i] < '0' || m_Str[i] > '9' && m_Str[i] != '-')
//             return 0;
//     return 1;
// }

// CStr &CStr::Trim() {
//     int i, u;
//     if (m_Len < 1)
//         return *this;

//     for (i = 0; i < m_Len; i++) {
//         if (m_Str[i] != ' ' && m_Str[i] != 0x9 && m_Str[i] != 0x0d && m_Str[i] != 0x0a)
//             break;
//     }
//     if (i == m_Len) {
//         Clear();
//         return *this;
//     }
//     for (u = m_Len - 1; u >= 0; u--) {
//         if (m_Str[u] != ' ' && m_Str[u] != 0x9 && m_Str[u] != 0x0d && m_Str[u] != 0x0a)
//             break;
//     }
//     m_Len = u - i + 1;
//     if (m_Len < 1) {
//         Clear();
//         return *this;
//     }
//     if (i == 0) {
//         m_Str[m_Len] = 0;
//         return *this;
//     }
//     for (u = 0; u < m_Len; u++)
//         m_Str[u] = m_Str[u + i];
//     m_Str[u] = 0;

//     return *this;
// }

// CStr &CStr::TrimFull() {
//     int i, u;
//     Trim();
//     if (m_Len < 4)
//         return *this;
//     for (i = 2; i < m_Len - 1; i++) {
//         if ((m_Str[i] == ' ' || m_Str[i] == 0x9) && (m_Str[i - 1] == ' ' || m_Str[i - 1] == 0x9)) {
//             for (u = i; u < m_Len; u++)
//                 m_Str[u] = m_Str[u + 1];
//             m_Len--;
//             i--;
//         }
//     }
//     return *this;
// }

// void CStr::TabToSpace() {
//     for (int i = 0; i < m_Len; i++)
//         if (m_Str[i] == 0x9)
//             m_Str[i] = ' ';
// }

// void CStr::Del(int sme, int len) {
//     if (sme < 0 || sme + len > m_Len)
//         return;
//     for (int i = sme; i <= (m_Len - len); i++)
//         m_Str[i] = m_Str[i + len];
//     m_Len -= len;
// }

// void CStr::Insert(int sme, const char *tstr, int inlen) {
//     int i;
//     if (inlen < 1)
//         return;
//     if (sme >= m_Len || m_Len < 1) {
//         Add(tstr);
//         return;
//     }
//     int oldlen = m_Len;
//     SetLen(oldlen + inlen);
//     for (i = oldlen - 1; i >= sme; i--) {
//         m_Str[i + inlen] = m_Str[i];
//     }

//     for (i = 0; i < inlen; i++) {
//         m_Str[sme + i] = tstr[i];
//     }
// }

// void CStr::Replace(const CStr &substr, const CStr &strreplace) {
//     if (substr.GetLen() < 1 || GetLen() < 1)
//         return;

//     int sme = 0;
//     for (;;) {
//         int ff;
//         if ((ff = Find(substr, sme)) == -1)
//             break;
//         sme += ff;
//         Del(sme, substr.GetLen());
//         Insert(sme, strreplace, strreplace.Len());
//         sme += strreplace.GetLen();
//     }
// }

// int CStr::Find(const char *substr, int sublen, int sme) const {
//     int i, u;
//     if ((m_Len - sme < sublen) || (sublen < 1))
//         return -1;

//     for (i = 0; i < m_Len - sme; i++) {
//         for (u = 0; u < sublen; u++) {
//             if (m_Str[sme + i + u] != substr[u])
//                 break;
//         }
//         if (u >= sublen)
//             return i;
//     }
//     return -1;
// }

// int CStr::FindR(const char *substr, int sublen) const {
//     int i, u;
//     if ((m_Len < sublen) || (sublen < 1))
//         return -1;

//     for (i = m_Len - sublen; i >= 0; --i) {
//         for (u = 0; u < sublen; u++) {
//             if (m_Str[i + u] != substr[u])
//                 break;
//         }
//         if (u >= sublen)
//             return i;
//     }
//     return -1;
// }

// int CStr::GetSmePar(int np, const char *ogsim) const {
//     int lenogsim = strlen(ogsim);
//     int tlen = GetLen();

//     ASSERT(!(tlen < 1 || lenogsim < 1 || np < 0));
//     int smepar = 0;
//     int tekpar = 0;

//     const char *tstr = Get();

//     if (np > 0) {
//         int i;
//         for (i = 0; i < tlen; i++) {
//             int u;
//             for (u = 0; u < lenogsim; u++)
//                 if (tstr[i] == ogsim[u])
//                     break;
//             if (u < lenogsim) {
//                 tekpar++;
//                 smepar = i + 1;
//                 if (tekpar == np)
//                     break;
//             }
//         }
//         if (i == tlen)
//             ERROR_E;
//     }

//     return smepar;
// }

// int CStr::GetLenPar(int smepar, const char *ogsim) const {
//     int i;
//     int tlen = GetLen();
//     int lenogsim = strlen(ogsim);
//     if (tlen < 1 || lenogsim < 1 || smepar > tlen)
//         ERROR_E;

//     const char *tstr = Get();

//     for (i = smepar; i < tlen; i++) {
//         int u;
//         for (u = 0; u < lenogsim; u++)
//             if (tstr[i] == ogsim[u])
//                 break;
//         if (u < lenogsim)
//             break;
//     }
//     return i - smepar;
// }

void CStr::LowerCase(int sme, int len) {
    if (len < 0)
        len = Len() - sme;
    if ((sme < 0) || (len <= 0) || ((sme + len) > Len()))
        return;

    CharLowerBuffA(GetBuf() + sme, len);
}

void CStr::UpperCase(int sme, int len) {
    if (len < 0)
        len = Len() - sme;
    if ((sme < 0) || (len <= 0) || ((sme + len) > Len()))
        return;

    CharUpperBuffA(GetBuf() + sme, len);
}

// int CStr::Compare(const CStr &zn1, const CStr &zn2) {
//     char *str1 = zn1.m_Str;
//     char *str2 = zn2.m_Str;

//     if (str1 == NULL && str2 == NULL)
//         return 0;
//     else if (str1 == NULL)
//         return 1;
//     else if (str2 == NULL)
//         return -1;

//     int lens1 = zn1.m_Len;
//     int lens2 = zn2.m_Len;
//     int len = lens1;
//     if (len > lens2)
//         len = lens2;

//     for (int i = 0; i < len; i++) {
//         if (str1[i] < str2[i])
//             return 1;
//         else if (str1[i] > str2[i])
//             return -1;
//     }
//     if (lens1 < lens2)
//         return 1;
//     else if (lens1 > lens2)
//         return -1;
//     return 0;
// }

// int CStr::CompareFirst(const CStr &str) {
//     if (str.m_Str == NULL || m_Str == NULL || str.m_Len > m_Len)
//         return 0;

//     for (int i = 0; i < str.m_Len; i++)
//         if (str.m_Str[i] != m_Str[i])
//             return 0;

//     return 1;
// }

// int CStr::CompareSubstring(const CStr &str) {
//     if (str.m_Str == NULL || m_Str == NULL || str.m_Len > m_Len)
//         return 0;
//     int kolsd = m_Len - str.m_Len + 1;

//     int i, u;
//     for (i = 0; i < kolsd; i++) {
//         for (u = 0; u < str.m_Len; u++) {
//             if (m_Str[u + i] != str.m_Str[u])
//                 break;
//         }
//         if (u == str.m_Len)
//             return 1;
//     }
//     return 0;
// }

// int operator==(const CStr &zn1, const CStr &zn2) {
//     int i;
//     if (zn1.m_Len != zn2.m_Len)
//         return 0;
//     for (i = 0; i < zn1.m_Len; i++)
//         if (zn1.m_Str[i] != zn2.m_Str[i])
//             return 0;
//     return 1;
// }

// int operator==(const CStr &zn1, const char *zn2) {
//     int i;
//     if (zn1.m_Len != strlen(zn2))
//         return 0;
//     for (i = 0; i < zn1.m_Len; i++)
//         if (zn1.m_Str[i] != zn2[i])
//             return 0;
//     return 1;
// }

}  // namespace Base
