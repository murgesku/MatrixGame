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
    int tlen = length();
    if (tlen < 1)
        return 0;
    const wchar *tstr = c_str();

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
    int tlen = length();
    if (tlen < 1)
        return 0;
    const wchar *tstr = c_str();

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
    int tlen = length();
    if (tlen < 1)
        return 0;
    const wchar *tstr = c_str();

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
    int tlen = length();
    if (tlen < 1)
        return 0;
    const wchar *tstr = c_str();

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
    int tlen = length();
    if (tlen < 1)
        return 0;
    const wchar *tstr = c_str();

    for (int i = 0; i < tlen; i++)
        if (tstr[i] < L'0' || tstr[i] > L'9' && tstr[i] != L'-')
            return 0;
    return 1;
}

int CWStr::GetCountPar(const wchar *ogsim) const {
    int tlen = length();
    if (tlen < 1)
        return 0;

    int c = 1;
    int lenogsim = WStrLen(ogsim);
    if (lenogsim < 1)
        return 0;

    const wchar *tstr = c_str();

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
    int tlen = length();
    // if(tlen<1 || lenogsim<1 || np<0) ERROR_OK("Data in CWStr::GetSmePar()");
    //  if((tlen<1 || lenogsim<1 || np<0))
    //        ASSERT(1);
    ASSERT(!(tlen < 1 || lenogsim < 1 || np < 0));
    int smepar = 0;
    int tekpar = 0;

    const wchar *tstr = c_str();

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
    int tlen = length();
    int lenogsim = WStrLen(ogsim);
    if (tlen < 1 || lenogsim < 1 || smepar > tlen)
        ERROR_E;

    const wchar *tstr = c_str();

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

CWStr CWStr::GetStrPar(int nps, int npe, const wchar *ogsim) const {
    int sme1 = GetSmePar(nps, ogsim);
    int sme2 = GetSmePar(npe, ogsim);
    sme2 += GetLenPar(sme2, ogsim);
    return std::wstring(c_str() + sme1, sme2 - sme1);
}

bool CWStr::GetTrueFalsePar(int np, const wchar *ogsim) const {
    CWStr tstr = GetStrPar(np, ogsim);

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

}  // namespace Base
