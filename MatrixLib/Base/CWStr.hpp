// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

// it's a total shit that we have to do this...
#ifdef MSVC7
    #undef min
    #undef max
#endif

#include <algorithm>

#include "CException.hpp"
#include "CMain.hpp"
#include "CHeap.hpp"

#include <string>
#include <cwchar>

namespace Base {

// TODO: replace with plain std func calls
inline int WStrLen(const wchar *str) {
    return std::wcslen(str);
}

inline bool WStrCmp(const wchar_t *s1, const wchar_t *s2) {
    return !std::wcscmp(s1, s2);
}

class BASE_API CWStr : public std::wstring, public CMain
{
public:
    explicit CWStr(CHeap *heap = nullptr)
    : CMain{}
    {
    }

    CWStr(const std::wstring& str) : CWStr(str.c_str())
    {
    }

    CWStr(const CWStr &s, CHeap *heap = NULL)
    : CMain{}
    , std::wstring(s)
    {
    }

    explicit CWStr(const wchar *s, CHeap *heap = NULL)
    : CMain{}
    , std::wstring{s}
    {
    }

    CWStr(const wchar *s, int len, CHeap *heap = NULL)
    : CMain{}
    , std::wstring{s, static_cast<size_t>(len)}
    {
    }

    explicit CWStr(wchar sim, CHeap *heap = NULL)
    : CMain{}
    , std::wstring(size_t{1}, sim)
    {
    }

    CWStr(wchar sim, int count, CHeap *heap = NULL)
    : CMain{}
    , std::wstring(static_cast<size_t>(count), sim)
    {
    }

    explicit CWStr(int zn, CHeap *heap = NULL)
    : CMain{}
    , std::wstring{utils::format(L"%d", zn)}
    {
    }

    explicit CWStr(DWORD zn, CHeap *heap = NULL)
    : CMain{}
    , std::wstring{utils::format(L"%u", zn)}
    {
    }

    explicit CWStr(double zn, int zpz = 8, CHeap *heap = NULL)
    : CMain{}
    , std::wstring{utils::format(L"%.*f", zpz, zn)}
    {
    }

    ~CWStr() = default;

    CHeap* GetHeap() const
    {
        return nullptr;
    }

    // Clear - Очищает строку
    void Clear()
    {
        this->clear();
    }

    const wchar *Get(void) const
    {
        return this->c_str();
    }

    wchar *GetBuf(void)
    {
        return const_cast<wchar*>(this->data());
    }

    int GetLen(void) const
    {
        return this->length();
    }

    int GetInt(void) const;
    DWORD GetDword(void) const;
    double GetDouble(void) const;
    int GetHex(void) const;
    DWORD GetHexUnsigned(void) const;

    bool IsOnlyInt(void) const;
    bool IsEmpty(void) const
    {
        return this->empty();
    }

    CWStr &Trim(void);      // Удаляет в начале и в конце символы 0x20,0x9,0x0d,0x0a

    CWStr &Replace(const CWStr &substr, const CWStr &strreplace);  // Заменить часть строки ну другую

    void LowerCase(int sme = 0, int len = -1);
    void UpperCase(int sme = 0, int len = -1);

    // Функции для работы с параметрами
    // Примеры :
    //      Str="count=5,7"    GetCountPar("=,")      return 3
    //      Str="count=5,7"    GetStrPar(str,1,"=")   str="5,7"
    //      Str="count=5,7"    GetIntPar(2,"=,")      return 7
    int GetCountPar(const wchar *ogsim) const;
    int GetSmePar(int np, const wchar *ogsim) const;
    int GetLenPar(int smepar, const wchar *ogsim) const;
    void GetStrPar(CWStr &str, int np, const wchar *ogsim) const;
    CWStr GetStrPar(int np, const wchar *ogsim) const {
        int sme = GetSmePar(np, ogsim);
        return CWStr(Get() + sme, GetLenPar(sme, ogsim), GetHeap());
    }

    void GetStrPar(CWStr &str, int nps, int npe, const wchar *ogsim) const;
    CWStr GetStrPar(int nps, int npe, const wchar *ogsim) const;
    int GetIntPar(int np, const wchar *ogsim) const { return GetStrPar(np, ogsim).GetInt(); }
    double GetDoublePar(int np, const wchar *ogsim) const { return GetStrPar(np, ogsim).GetDouble(); }
    bool GetTrueFalsePar(int np, const wchar *ogsim) const;

    CWStr &operator=(const wchar *s) {
        this->assign(s);
        return *this;
    }

};

}  // namespace Base
