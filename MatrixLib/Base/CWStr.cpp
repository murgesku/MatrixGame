// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "Base.pch"

#include <cwchar>

#include "CWStr.hpp"
#include "CException.hpp"
#include "CStr.hpp"

namespace Base {

CWStr::CWStr(const char* s): CMain()
{
    CStr tmp(s);
    NewDataLen(nullptr, tmp.Len());
    Set(tmp.Get());
}

#include <stdio.h>

void CWStr::Set(const char* str)
{
    CStr cstr{str};

    static int call_num = 0;
    ++call_num;
    if (cstr.Len() <= 0) {
        ModifyLen(m_Data->m_Heap, 0);
        return;
    }

    ModifyLen(m_Data->m_Heap, cstr.Len());

    if (!MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, cstr.Get(), m_Data->m_Len, m_Data->Data(), m_Data->m_Len)) {
        FILE *file = fopen("error.log", "w+b");

        CStr txt(">>>>>>> MultiByteToWideChar <<<<<<<\n");
        fwrite(txt.Get(), txt.Len(), 1, file);
        DWORD err = GetLastError();

        txt = "unknown error: ";
        if (err == ERROR_INSUFFICIENT_BUFFER)
            txt = "ERROR_INSUFFICIENT_BUFFER";
        else if (err == ERROR_INVALID_FLAGS)
            txt = "ERROR_INVALID_FLAGS";
        else if (err == ERROR_INVALID_PARAMETER)
            txt = "ERROR_INVALID_PARAMETER";
        else if (err == ERROR_NO_UNICODE_TRANSLATION)
            txt = "ERROR_NO_UNICODE_TRANSLATION";
        else {
            txt.Add(int(err));
        }

        txt.Add("\nCall number: ");
        txt.Add(call_num);
        txt.Add("\n");

        fwrite(txt.Get(), txt.Len(), 1, file);
        if (cstr.Get() == NULL) {
            fwrite("<Input is NULL>", strlen("<Input is NULL>"), 1, file);
        }
        else {
            fwrite(cstr.Get(), cstr.Len(), 1, file);
        }

        fclose(file);

        ERROR_E;
    }
    m_Data->Data()[m_Data->m_Len] = 0;
}

void CWStr::Set(int zn) {
    if (m_Data->m_Refs > 1) {
        m_Data->RefDec();
        NewDataLen(m_Data->m_Heap, 32);
    }
    else {
        Tream(32);
    }

    wchar *tstr = m_Data->Data();
    int tsme = m_Data->m_Len;

    int fm = 0;
    if (zn < 0) {
        fm = 1;
        zn = -zn;
    }

    while (zn > 0) {
        tsme--;
        tstr[tsme] = wchar(zn - int(zn / 10) * 10) + wchar('0');
        zn = zn / 10;
    }
    if (fm) {
        tsme--;
        tstr[tsme] = L'-';
    }
    if (tsme >= m_Data->m_Len) {
        tstr[0] = L'0';
        tstr[1] = 0;
        m_Data->m_Len = 1;
        return;
    }
    int cnt = m_Data->m_Len - tsme;
    for (int i = 0; i < cnt; i++)
        tstr[i] = tstr[tsme + i];
    m_Data->m_Len = cnt;
    tstr[cnt] = 0;
}

void CWStr::Set(dword zn) {
    if (m_Data->m_Refs > 1) {
        m_Data->RefDec();
        NewDataLen(m_Data->m_Heap, 32);
    }
    else {
        Tream(32);
    }

    wchar *tstr = m_Data->Data();
    int tsme = m_Data->m_Len;

    while (zn > 0) {
        tsme--;
        tstr[tsme] = wchar(zn - dword(zn / 10) * 10) + wchar('0');
        zn = zn / 10;
    }
    if (tsme >= m_Data->m_Len) {
        tstr[0] = L'0';
        tstr[1] = 0;
        m_Data->m_Len = 1;
        return;
    }
    int cnt = m_Data->m_Len - tsme;
    for (int i = 0; i < cnt; i++)
        tstr[i] = tstr[tsme + i];
    m_Data->m_Len = cnt;
    tstr[cnt] = 0;
}

void CWStr::Set(double zn, int zpz) {
    CWStr tstr;
    int dec, sign, le;
    int count = 0;
    char *st = _fcvt(zn, zpz, &dec, &sign);
    if (sign == 0)
        ModifyLenNoCopy(m_Data->m_Heap, 0);
    else
        Set((wchar)'-');
    le = strlen(st);
    if (dec < 0) {
        Add(L'0');
        if (le > 0) {
            Add(L'.');
            Add(L'0', -dec);
            for (int i = le - 1; i >= 0; i--)
                if (st[i] == '0')
                    count++;
                else
                    break;
            if (le > count) {
                for (int yu = 0; yu < le - count; yu++)
                    Add(wchar(st[yu]));
            }
        }
    }
    else {
        if (dec > 0) {
            for (int yu = 0; yu < dec; yu++)
                Add(wchar(st[yu]));
        }
        else {
            Add(L'0');
        }

        for (int i = le - 1; i >= dec; i--)
            if (st[i] == '0')
                count++;
            else
                break;
        if (dec < le - count) {
            Add(L'.');
            for (int yu = 0; yu < le - count - dec; yu++)
                Add(wchar(st[dec + yu]));
        }
    }
}

static const wchar *Str_CH = L"0123456789ABCDEF";

void CWStr::SetHex(void *zn) {
    ModifyLenNoCopy(m_Data->m_Heap, 8);

    wchar *tstr = m_Data->Data();
    tstr[8] = 0;

    dword dw = (dword)zn;
    for (int i = 0; i < 8; i++) {
        tstr[7 - i] = Str_CH[(dw >> (i * 4)) & 0x0f];
    }
}

void CWStr::SetHex(BYTE zn) {
    ModifyLenNoCopy(m_Data->m_Heap, 2);

    wchar *tstr = m_Data->Data();
    tstr[2] = 0;

    DWORD dw = (DWORD)zn;
    for (int i = 0; i < 2; i++) {
        tstr[1 - i] = Str_CH[(dw >> (i * 4)) & 0x0f];
    }
}

CWStr &CWStr::Add(const CWStr &cstr) {
    int oldlen = GetLen();
    int addlen = cstr.GetLen();
    ModifyLen(m_Data->m_Heap, oldlen + addlen);

    memcpy(m_Data->Data() + oldlen, cstr.Get(), addlen * 2 + 2);

    return *this;
}

CWStr &CWStr::Add(const wchar *str) {
    int addlen = WStrLen(str);
    if (addlen < 1)
        return *this;
    int oldlen = GetLen();
    ModifyLen(GetHeap(), oldlen + addlen);
    memcpy(m_Data->Data() + oldlen, str, addlen * sizeof(wchar) + sizeof(wchar));

    return *this;
}

CWStr &CWStr::Add(const wchar *str, int lstr) {
    if (lstr < 1)
        return *this;

    int oldlen = GetLen();

    ModifyLen(GetHeap(), oldlen + lstr);

    memcpy(m_Data->Data() + oldlen, str, lstr * sizeof(wchar));
    m_Data->Data()[m_Data->m_Len] = 0;

    return *this;
}

CWStr &CWStr::Add(wchar sim) {
    ModifyLen(GetHeap(), GetLen() + 1);

    m_Data->Data()[m_Data->m_Len - 1] = sim;
    m_Data->Data()[m_Data->m_Len] = 0;

    return *this;
}

CWStr &CWStr::Add(wchar sim, int count) {
    if (count < 1)
        return *this;

    int oldlen = GetLen();
    ModifyLen(GetHeap(), oldlen + count);

    for (int i = 0; i < count; i++)
        m_Data->Data()[oldlen + i] = sim;
    m_Data->Data()[m_Data->m_Len] = 0;

    return *this;
}

int CWStr::GetInt() const {
    int tlen = GetLen();
    if (tlen < 1)
        return 0;
    const wchar *tstr = Get();

    int zn = 0;
    wchar ch;
    for (int i = 0; i < tlen; i++) {
        ch = tstr[i] - '0';
        if (ch < 10)
            zn = zn * 10 + ch;
    }
    for (int i = 0; i < tlen; i++)
        if (tstr[i] == '-') {
            zn = -zn;
            break;
        }

    return zn;
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
        ModifyLenNoCopy(GetHeap(), 0);
        return *this;
    }
    int u;
    for (u = tlen - 1; u >= 0; u--) {
        if (tstr[u] != ' ' && tstr[u] != 0x9 && tstr[u] != 0x0d && tstr[u] != 0x0a)
            break;
    }
    tlen = u - i + 1;
    if (tlen < 1) {
        ModifyLenNoCopy(GetHeap(), 0);
        return *this;
    }

    ModifyLen(GetHeap(), tlen);
    if (i == 0) {
        return *this;
    }

    memcpy(GetBuf(), tstr + i, tlen * sizeof(wchar));
    m_Data->Data()[tlen] = 0;
    return *this;
}

CWStr &CWStr::TrimFull() {
    Trim();
    int tlen = GetLen();
    if (tlen < 4)
        return *this;

    ModifyLen(GetHeap(), tlen);
    wchar *tstr = GetBuf();

    for (int i = 2; i < tlen - 1; i++) {
        if ((tstr[i] == ' ' || tstr[i] == 0x9) && (tstr[i - 1] == ' ' || tstr[i - 1] == 0x9)) {
            for (int u = i; u < tlen; u++)
                tstr[u] = tstr[u + 1];
            tlen--;
            i--;
        }
    }
    m_Data->m_Len = tlen;
    return *this;
}

void CWStr::TabToSpace() {
    int tlen = GetLen();
    if (tlen < 1)
        return;
    ModifyLen(GetHeap(), tlen);
    wchar *tstr = GetBuf();

    for (int i = 0; i < tlen; i++)
        if (tstr[i] == 0x9)
            tstr[i] = ' ';
}

CWStr &CWStr::Del(int sme, int len) {
    int ost_sme = sme + len;
    int ost_len = GetLen() - ost_sme;
    if (ost_len > 0) {
        ModifyLen(GetHeap(), GetLen());
        memcpy(GetBuf() + sme, GetBuf() + ost_sme, sizeof(wchar) + sizeof(wchar) * ost_len);
        m_Data->m_Len -= len;
        m_Data->Data()[m_Data->m_Len] = 0;
    }
    else {
        ModifyLen(GetHeap(), GetLen());
        m_Data->m_Len -= len;
        m_Data->Data()[m_Data->m_Len] = 0;
        if (m_Data->m_Len < 1)
            ModifyLenNoCopy(GetHeap(), 0);
    }

    return *this;
}

CWStr &CWStr::Insert(int sme, const wchar *str, int len) {
    if (len < 1)
        return *this;
    int oldlen = GetLen();
    if (oldlen < 1) {
        Set(str, len);
        return *this;
    }
    ModifyLen(GetHeap(), oldlen + len);
    wchar *tstr = GetBuf();
    if (sme > oldlen)
        sme = oldlen;

    for (int i = oldlen; i >= sme; i--)
        tstr[i + len] = tstr[i];

    for (int i = 0; i < len; i++)
        tstr[sme + i] = str[i];

    return *this;
}

CWStr &CWStr::Replace(const CWStr &substr, const CWStr &strreplace) {
    int tlen = GetLen();
    if (tlen < 1 || tlen < substr.GetLen())
        return *this;

    int sme = 0;
    for (;;) {
        //		int ff;
        if ((sme = Find(substr, sme)) == -1)
            break;
        //		sme+=ff;
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

    ModifyLen(GetHeap(), GetLen());

    if (GetVersion() < 0x80000000)
        CharLowerBuffW(GetBuf() + sme, len);
    else {
        CStr tstr(*this, GetHeap());
        tstr.LowerCase(sme, len);
        Set(tstr.Get());
    }
}

void CWStr::UpperCase(int sme, int len) {
    if (len < 0)
        len = GetLen() - sme;
    if ((sme < 0) || (len <= 0) || ((sme + len) > GetLen()))
        return;

    ModifyLen(GetHeap(), GetLen());

    if (GetVersion() < 0x80000000)
        CharUpperBuffW(GetBuf() + sme, len);
    else {
        CStr tstr(*this, GetHeap());
        tstr.UpperCase(sme, len);
        Set(tstr.Get());
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
    //	if((tlen<1 || lenogsim<1 || np<0))
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
    str.Set(Get() + sme, len);
}

void CWStr::GetStrPar(CWStr &str, int nps, int npe, const wchar *ogsim) const {
    int sme1 = GetSmePar(nps, ogsim);
    int sme2 = GetSmePar(npe, ogsim);
    sme2 += GetLenPar(sme2, ogsim);
    str.Set(Get() + sme1, sme2 - sme1);
}

CWStr CWStr::GetStrPar(int nps, int npe, const wchar *ogsim) const {
    int sme1 = GetSmePar(nps, ogsim);
    int sme2 = GetSmePar(npe, ogsim);
    sme2 += GetLenPar(sme2, ogsim);
    return CWStr(Get() + sme1, sme2 - sme1, GetHeap());
}

bool CWStr::GetTrueFalsePar(int np, const wchar *ogsim) const {
    CWStr tstr(*this, GetHeap());
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

bool CWStr::Equal(const wchar *zn, int len) const {
    if (len < 0)
        len = WStrLen(zn);
    if (GetLen() != len)
        return false;
    if (len <= 0)
        return true;
    return !std::wmemcmp(Get(), zn, len);
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
