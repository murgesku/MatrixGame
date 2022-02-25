// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "Base.pch"

#include <stdlib.h>
#include "CWStr.hpp"
#include "CStr.hpp"
#include "CException.hpp"
#include "Mem.hpp"

namespace Base {

int ff_GetInt(const wchar *tstr, int len) {
    int zn = 0;
    wchar ch;
    for (int i = 0; i < len; i++) {
        ch = tstr[i];
        if (ch >= '0' && ch <= '9')
            zn = zn * 10 + ch - '0';
    }
    for (int i = 0; i < len; i++)
        if (tstr[i] == '-') {
            zn = -zn;
            break;
        }

    return zn;
}

int ff_Int64ToStr(__int64 zn, wchar *str, int base) {
    bool si = false;
    int count = 0;

    if (zn == 0) {
        *str = L'0';
        return 1;
    }
    else if (zn < 0) {
        si = true;
        zn = -zn;
    }

    while (zn != 0) {
        int nom = (int)(zn % base);
        if (nom < 10)
            *str = nom + L'0';
        else
            *str = nom - 10 + L'A';
        zn = zn / base;
        count++;
        str--;
    }

    if (si) {
        *str = '-';
        count++;
    }

    return count;
}

void ff_FormatInt64(CWStr &cs, __int64 zn, wchar *str, int lenstr, int width, wchar fill, int align, int base) {
    int tt;
    int tlen = ff_Int64ToStr(zn, str + lenstr - 1, base);

    if (width != 0 && tlen > width)
        tt = width;
    else
        tt = tlen;

    if (width <= 0) {
        cs.Add(str + lenstr - tlen, tt);
    }
    else if (align == -1) {
        cs.Add(str + lenstr - tlen, tt);
        cs.Add(fill, width - tlen);
    }
    else if (align == 0) {
        int uu = (width - tt) / 2;
        cs.Add(fill, uu);
        cs.Add(str + lenstr - tlen, tt);
        cs.Add(fill, (width - tt - uu));
    }
    else {
        cs.Add(fill, width - tlen);
        cs.Add(str + lenstr - tlen, tt);
    }
}

int ff_DoubleToStr(double zn, wchar *str, int zpz) {
    int outcount = 0;
    int dec, sign, le;
    int count = 0;
    char *st = _fcvt(zn, zpz, &dec, &sign);
    if (sign != 0) {
        *str = L'-';
        str++;
        outcount++;
    }

    le = strlen(st);
    if (dec < 0) {
        *str = L'0';
        str++;
        outcount++;
        if (le > 0) {
            *str = L'.';
            str++;
            outcount++;
            for (int i = 0; i < -dec; i++) {
                *str = L'0';
                str++;
                outcount++;
            }
            for (int i = le - 1; i >= 0; i--)
                if (st[i] == '0')
                    count++;
                else
                    break;
            if (le > count) {
                for (int yu = 0; yu < le - count; yu++) {
                    *str = wchar(st[yu]);
                    str++;
                    outcount++;
                }
            }
        }
    }
    else {
        if (dec > 0) {
            for (int yu = 0; yu < dec; yu++) {
                *str = wchar(st[yu]);
                str++;
                outcount++;
            }
        }
        else {
            *str = L'0';
            str++;
            outcount++;
        }

        for (int i = le - 1; i >= dec; i--)
            if (st[i] == '0')
                count++;
            else
                break;
        if (dec < le - count) {
            *str = L'.';
            str++;
            outcount++;
            for (int yu = 0; yu < le - count - dec; yu++) {
                *str = wchar(st[dec + yu]);
                str++;
                outcount++;
            }
        }
    }
    return outcount;
}

void ff_FormatDouble(CWStr &cs, double zn, wchar *str, int zpz, int width, wchar fill, int align) {
    int tt;
    int tlen = ff_DoubleToStr(zn, str, zpz);

    if (width != 0 && tlen > width)
        tt = width;
    else
        tt = tlen;

    if (width <= 0) {
        cs.Add(str, tt);
    }
    else if (align == -1) {
        cs.Add(str, tt);
        cs.Add(fill, width - tlen);
    }
    else if (align == 0) {
        int uu = (width - tt) / 2;
        cs.Add(fill, uu);
        cs.Add(str, tt);
        cs.Add(fill, (width - tt - uu));
    }
    else {
        cs.Add(fill, width - tlen);
        cs.Add(str, tt);
    }
}

void ff_FormatStr(CWStr &cs, wchar *str, int width, wchar fill, int align) {
    int tt;
    int tlen = WStrLen(str);

    if (width != 0 && tlen > width)
        tt = width;
    else
        tt = tlen;

    if (width <= 0) {
        cs.Add(str, tt);
    }
    else if (align == -1) {
        cs.Add(str, tt);
        cs.Add(fill, width - tlen);
    }
    else if (align == 0) {
        int uu = (width - tt) / 2;
        cs.Add(fill, uu);
        cs.Add(str, tt);
        cs.Add(fill, (width - tt - uu));
    }
    else {
        cs.Add(fill, width - tlen);
        cs.Add(str, tt);
    }
}

// <<			- символ <
// <w=int>		- размер выходной строки для параметра
// <f=int>		- код символа заполнения
// <a=[,-,]>	- align влево, вправо, по центру
// <p=int>		- знаков после запятой
// <b=int>		- основание
// <i1>,<i2>,<i4>,<i8>   <i>=<i4>
// <u1>,<u2>,<u4>		 <u>=<u4>
// <f>,<d>
// <s>

CWStr &CWStr::Format(const wchar *format, ...) {
    wchar tbuf[256];
    bool tag = false;
    int t_width = 0;
    wchar t_fill = L' ';
    int t_aling = -1;  // -1=left 0=center 1=right
    int t_len;
    int t_p = 8;
    int t_base = 10;
    const wchar *end;

    if (format == NULL)
        return *this;
    ModifyLenNoCopy(GetHeap(), 0);

    va_list marker;
    va_start(marker, format);

    for (;;) {
        tag = false;

        if (*format == 0) {
            break;
        }
        else if (*format == L'<' && *(format + 1) == L'<') {
            format++;
            end = format;
        }
        else if (*format == L'<') {
            format++;
            end = format;
            while (*end != 0 && *end != L'>')
                end++;
            tag = *end == L'>';
            end--;
        }
        else {
            end = format + 1;
            while (*end != 0 && *end != L'<')
                end++;
            end--;
        }

        if (tag) {
            t_len = ((DWORD(end) - DWORD(format)) >> 1) + 1;
            if (t_len >= 3 && *format == 'w' && *(format + 1) == '=') {
                t_width = ff_GetInt(format + 2, t_len - 2);
                if (t_width < 0)
                    t_width = 0;
                else if (t_width > 1024 * 32)
                    t_width = 1024 * 32;  // 255;
            }
            else if (t_len >= 3 && *format == 'f' && *(format + 1) == '=') {
                t_fill = ff_GetInt(format + 2, t_len - 2);
            }
            else if (t_len >= 3 && MemCmp(format, L"a=[", 3)) {
                t_aling = -1;
            }
            else if (t_len >= 3 && MemCmp(format, L"a=-", 3)) {
                t_aling = 0;
            }
            else if (t_len >= 3 && MemCmp(format, L"a=]", 3)) {
                t_aling = 1;
            }
            else if (t_len >= 3 && *format == 'p' && *(format + 1) == '=') {
                t_p = ff_GetInt(format + 2, t_len - 2);
            }
            else if (t_len >= 3 && *format == 'b' && *(format + 1) == '=') {
                t_base = ff_GetInt(format + 2, t_len - 2);
                if (t_base < 2)
                    t_base = 2;
                else if (t_base > 32)
                    t_base = 32;
            }
            else if (t_len == 1 && *format == L'i') {
                ff_FormatInt64(*this, va_arg(marker, long), tbuf, 255, t_width, t_fill, t_aling, t_base);
            }
            else if (t_len == 2 && *format == L'i' && *(format + 1) == L'1') {
                ff_FormatInt64(*this, va_arg(marker, char), tbuf, 255, t_width, t_fill, t_aling, t_base);
            }
            else if (t_len == 2 && *format == L'i' && *(format + 1) == L'2') {
                ff_FormatInt64(*this, va_arg(marker, short), tbuf, 255, t_width, t_fill, t_aling, t_base);
            }
            else if (t_len == 2 && *format == L'i' && *(format + 1) == L'4') {
                ff_FormatInt64(*this, va_arg(marker, long), tbuf, 255, t_width, t_fill, t_aling, t_base);
            }
            else if (t_len == 2 && *format == L'i' && *(format + 1) == L'8') {
                ff_FormatInt64(*this, va_arg(marker, __int64), tbuf, 255, t_width, t_fill, t_aling, t_base);
            }
            else if (t_len == 1 && *format == L'u') {
                ff_FormatInt64(*this, va_arg(marker, unsigned long), tbuf, 255, t_width, t_fill, t_aling, t_base);
            }
            else if (t_len == 2 && *format == L'u' && *(format + 1) == L'1') {
                ff_FormatInt64(*this, va_arg(marker, unsigned char), tbuf, 255, t_width, t_fill, t_aling, t_base);
            }
            else if (t_len == 2 && *format == L'u' && *(format + 1) == L'2') {
                ff_FormatInt64(*this, va_arg(marker, unsigned short), tbuf, 255, t_width, t_fill, t_aling, t_base);
            }
            else if (t_len == 2 && *format == L'u' && *(format + 1) == L'4') {
                ff_FormatInt64(*this, va_arg(marker, unsigned long), tbuf, 255, t_width, t_fill, t_aling, t_base);
            }
            else if (t_len == 1 && *format == L'f') {
                ff_FormatDouble(*this, va_arg(marker, double /*float*/), tbuf, t_p, t_width, t_fill, t_aling);
            }
            else if (t_len == 1 && *format == L'd') {
                ff_FormatDouble(*this, va_arg(marker, double), tbuf, t_p, t_width, t_fill, t_aling);
            }
            else if (t_len == 1 && *format == L's') {
                ff_FormatStr(*this, va_arg(marker, wchar *), t_width, t_fill, t_aling);
            }

            end++;
        }
        else {
            if (DWORD(end) < DWORD(format))
                break;
            Add(format, ((DWORD(end) - DWORD(format)) >> 1) + 1);
        }

        format = end + 1;
    }

    va_end(marker);
    return *this;
}

}  // namespace Base