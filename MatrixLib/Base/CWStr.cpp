// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "Base.pch"

#include <cwchar>
#include <cwctype>

#include "CWStr.hpp"
#include "CException.hpp"

#include <utils.hpp>

namespace Base {

int CWStr::GetInt() const
{
    if (this->empty())
    {
        return 0;
    }

    int value = 0;
    bool sign = false;

    // the original algorithm skips all the non-digit chars and treats
    // '-' at any position as a number sign.
    // needless to say - it's a bullshit, but as other code expectes it
    // to work like this, so...
    for (const auto sym : *this)
    {
        wchar ch = sym - '0';
        if (ch < 10)
        {
            value = value * 10 + ch;
        }

        if (sym == '-')
        {
            sign = true;
        }

    }

    return sign ? -value : value;
}

DWORD CWStr::GetDword() const {
    int tlen = GetLen();
    if (tlen < 1)
        return 0;
    const wchar *tstr = Get();

    DWORD zn = 0;
    wchar ch;
    for (int i = 0; i < tlen; i++) {
        ch = tstr[i] - '0';
        if (ch < 10)
            zn = zn * 10 + ch;
    }

    return zn;
}

double CWStr::GetDouble() const {
    int tlen = GetLen();
    if (tlen < 1)
        return 0;
    const wchar *tstr = Get();

    int i;
    double zn = 0.0;

    wchar ch;
    for (i = 0; i < tlen; i++) {
        ch = tstr[i];
        if (ch >= L'0' && ch <= L'9')
            zn = zn * 10.0 + (double)(ch - L'0');
        else if (ch == L'.')
            break;
    }
    i++;
    double tra = 10.0;
    for (i; i < tlen; i++) {
        ch = tstr[i];
        if (ch >= L'0' && ch <= L'9') {
            zn = zn + ((double)(ch - L'0')) / tra;
            tra *= 10.0;
        }
    }
    for (i = 0; i < tlen; i++)
        if (tstr[i] == '-') {
            zn = -zn;
            break;
        }

    return zn;
}

int CWStr::GetHex() const {
    int tlen = GetLen();
    if (tlen < 1)
        return 0;
    const wchar *tstr = Get();

    int zn = 0;
    int i;

    wchar ch;
    for (i = 0; i < tlen; i++) {
        ch = tstr[i];

        ch -= '0';
        if (ch > 9)
            ch = (tstr[i] & (~32)) - ('A' - 10);
        zn = zn * 16 + ch;
    }
    for (i = 0; i < tlen; i++)
        if (tstr[i] == '-') {
            zn = -zn;
            break;
        }

    return zn;
}

DWORD CWStr::GetHexUnsigned(void) const {
    int tlen = GetLen();
    if (tlen < 1)
        return 0;
    const wchar *tstr = Get();

    DWORD zn = 0;
    int i;

    wchar ch;
    for (i = 0; i < tlen; i++) {
        ch = tstr[i];

        ch -= '0';
        if (ch > 9)
            ch = (tstr[i] & (~32)) - ('A' - 10);
        zn = (zn << 4) + ch;
    }
    return zn;
}

bool CWStr::IsOnlyInt() const {
    int tlen = GetLen();
    if (tlen < 1)
        return 0;
    const wchar *tstr = Get();

    for (int i = 0; i < tlen; i++)
        if (tstr[i] < L'0' || tstr[i] > L'9' && tstr[i] != L'-')
            return 0;
    return 1;
}

CWStr &CWStr::Trim() {
    int tlen = GetLen();
    if (tlen < 1)
        return *this;
    const wchar *tstr = Get();

    int i;
    for (i = 0; i < tlen; i++) {
        if (tstr[i] != ' ' && tstr[i] != 0x9 && tstr[i] != 0x0d && tstr[i] != 0x0a)
            break;
    }
    if (i == tlen) {
        this->clear();
        return *this;
    }
    int u;
    for (u = tlen - 1; u >= 0; u--) {
        if (tstr[u] != ' ' && tstr[u] != 0x9 && tstr[u] != 0x0d && tstr[u] != 0x0a)
            break;
    }
    tlen = u - i + 1;
    if (tlen < 1)
    {
        this->clear();
        return *this;
    }

    this->resize(tlen);
    if (i == 0) {
        return *this;
    }

    *this = this->substr(i, tlen);
    return *this;
}

CWStr &CWStr::Del(int sme, int len)
{
    this->erase(sme, static_cast<size_t>(len));
    return *this;
}

CWStr &CWStr::Insert(int sme, const wchar *str, int len)
{
    this->insert(sme, str, static_cast<size_t>(len));
    return *this;
}

CWStr &CWStr::Replace(const CWStr &substr, const CWStr &strreplace) {
    int tlen = GetLen();
    if (tlen < 1 || tlen < substr.GetLen())
        return *this;

    int sme = 0;
    for (;;) {
        //      int ff;
        if ((sme = Find(substr, sme)) == -1)
            break;
        //      sme+=ff;
        Del(sme, substr.GetLen());
        Insert(sme, strreplace);
        sme += strreplace.GetLen();
    }
    return *this;
}

int CWStr::Find(const wchar *substr, int slen, int sme) const {
    int tlen = GetLen();
    if (tlen < 1)
        return -1;
    if (slen < 1 || (tlen - sme) < slen)
        return -1;
    const wchar *tstr = Get();

    for (int i = sme; i <= (tlen - slen); ++i) {
        int u;
        for (u = 0; u < slen; ++u) {
            if (tstr[i + u] != substr[u])
                break;
        }
        if (u >= slen)
            return i;
    }
    return -1;
}

int CWStr::FindR(wchar ch, int sme) const {
    if (sme < 0)
        sme = GetLen() - 1;
    if ((sme < 0) || (sme >= GetLen()))
        return -1;

    const wchar *curch = Get() + sme;
    while (sme >= 0 && *curch != ch) {
        sme--;
        curch--;
    }
    return sme;
}

void CWStr::LowerCase(int sme, int len) {
    if (len < 0)
        len = GetLen() - sme;
    if ((sme < 0) || (len <= 0) || ((sme + len) > GetLen()))
        return;

    if (GetVersion() < 0x80000000)
        CharLowerBuffW(GetBuf() + sme, len);
    else {
        for (size_t i = 0; i < len; ++i)
        {
            GetBuf()[sme + i] = std::towlower(Get()[sme + i]);
        }
    }
}

void CWStr::UpperCase(int sme, int len) {
    if (len < 0)
        len = GetLen() - sme;
    if ((sme < 0) || (len <= 0) || ((sme + len) > GetLen()))
        return;

    if (GetVersion() < 0x80000000)
        CharUpperBuffW(GetBuf() + sme, len);
    else {
        for (int i = 0; i < len; ++i)
        {
            GetBuf()[sme + i] = std::towupper(Get()[sme + i]);
        }
    }
}

int CWStr::GetCountPar(const wchar *ogsim) const {
    int tlen = GetLen();
    if (tlen < 1)
        return 0;

    int c = 1;
    int lenogsim = WStrLen(ogsim);
    if (lenogsim < 1)
        return 0;

    const wchar *tstr = Get();

    for (int i = 0; i < tlen; i++) {
        int u;
        for (u = 0; u < lenogsim; u++)
            if (tstr[i] == ogsim[u])
                break;
        if (u < lenogsim)
            c++;
    }
    return c;
}

int CWStr::GetSmePar(int np, const wchar *ogsim) const {
    int lenogsim = WStrLen(ogsim);
    int tlen = GetLen();
    // if(tlen<1 || lenogsim<1 || np<0) ERROR_OK("Data in CWStr::GetSmePar()");
    //  if((tlen<1 || lenogsim<1 || np<0))
    //        ASSERT(1);
    ASSERT(!(tlen < 1 || lenogsim < 1 || np < 0));
    int smepar = 0;
    int tekpar = 0;

    const wchar *tstr = Get();

    if (np > 0) {
        int i;
        for (i = 0; i < tlen; i++) {
            int u;
            for (u = 0; u < lenogsim; u++)
                if (tstr[i] == ogsim[u])
                    break;
            if (u < lenogsim) {
                tekpar++;
                smepar = i + 1;
                if (tekpar == np)
                    break;
            }
        }
        if (i == tlen) {
            ERROR_E;
        }
    }

    return smepar;
}

int CWStr::GetLenPar(int smepar, const wchar *ogsim) const {
    int i;
    int tlen = GetLen();
    int lenogsim = WStrLen(ogsim);
    if (tlen < 1 || lenogsim < 1 || smepar > tlen)
        ERROR_E;

    const wchar *tstr = Get();

    for (i = smepar; i < tlen; i++) {
        int u;
        for (u = 0; u < lenogsim; u++)
            if (tstr[i] == ogsim[u])
                break;
        if (u < lenogsim)
            break;
    }
    return i - smepar;
}

void CWStr::GetStrPar(CWStr &str, int np, const wchar *ogsim) const {
    int sme = GetSmePar(np, ogsim);
    int len = GetLenPar(sme, ogsim);
    str = std::wstring{Get() + sme, static_cast<size_t>(len)};
}

void CWStr::GetStrPar(CWStr &str, int nps, int npe, const wchar *ogsim) const {
    int sme1 = GetSmePar(nps, ogsim);
    int sme2 = GetSmePar(npe, ogsim);
    sme2 += GetLenPar(sme2, ogsim);
    str = std::wstring{Get() + sme1, static_cast<size_t>(sme2 - sme1)};
}

CWStr CWStr::GetStrPar(int nps, int npe, const wchar *ogsim) const {
    int sme1 = GetSmePar(nps, ogsim);
    int sme2 = GetSmePar(npe, ogsim);
    sme2 += GetLenPar(sme2, ogsim);
    return CWStr(Get() + sme1, sme2 - sme1);
}

bool CWStr::GetTrueFalsePar(int np, const wchar *ogsim) const {
    CWStr tstr(*this);
    GetStrPar(tstr, np, ogsim);

    if (tstr == L"true" || tstr == L"True" || tstr == L"TRUE")
        return 1;
    if (tstr == L"false" || tstr == L"False" || tstr == L"FALSE")
        return 0;

    if (tstr == L"yes" || tstr == L"Yes" || tstr == L"YES")
        return 1;
    if (tstr == L"no" || tstr == L"No" || tstr == L"NO")
        return 0;

    if (tstr == L"on" || tstr == L"On" || tstr == L"ON")
        return 1;
    if (tstr == L"off" || tstr == L"Off" || tstr == L"OFF")
        return 0;

    ERROR_E;
    return 0;
}

int CWStr::Compare(const wchar *zn1, int zn1len, const wchar *zn2, int zn2len) {
    if (zn1len == 0 && zn2len == 0)
        return 0;
    else if (zn1len == 0)
        return -1;
    else if (zn2len == 0)
        return 1;

    int len = zn1len;
    if (len > zn2len)
        len = zn2len;

    for (int i = 0; i < len; i++) {
        if (zn1[i] < zn2[i])
            return -1;
        else if (zn1[i] > zn2[i])
            return 1;
    }
    if (zn1len < zn2len)
        return -1;
    else if (zn1len > zn2len)
        return 1;
    return 0;
}

bool CWStr::CompareFirst(const CWStr &str) const {
    int lens1 = GetLen();
    if (lens1 < 1)
        return false;
    int lens2 = str.GetLen();
    if (lens2 < 1)
        return false;
    if (lens2 > lens1)
        return 0;

    const wchar *str1 = Get();
    const wchar *str2 = str.Get();

    for (int i = 0; i < lens2; i++)
        if (str1[i] != str2[i])
            return 0;

    return 1;
}

int CWStr::CompareSubstring(const CWStr &str) const {
    int lens1 = GetLen();
    if (lens1 < 1)
        return false;
    int lens2 = str.GetLen();
    if (lens2 < 1)
        return false;
    if (lens2 > lens1)
        return 0;
    int kolsd = lens1 - lens2 + 1;

    const wchar *str1 = Get();
    const wchar *str2 = str.Get();

    for (int i = 0; i < kolsd; i++) {
        int u;
        for (u = 0; u < lens2; u++) {
            if (str1[u + i] != str2[u])
                break;
        }
        if (u == lens2)
            return 1;
    }
    return 0;
}

}  // namespace Base
