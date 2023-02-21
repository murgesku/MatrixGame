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

class BASE_API CWStr : public CMain {
    // as a first step - just replace custom copy-on-write with standard wide string
    std::wstring m_data;

    CWStr(const wchar *s1, int len1, const wchar *s2, int len2, CHeap *heap = nullptr)
    {
        m_data.clear();
        m_data += std::wstring(s1, len1);
        m_data += std::wstring(s2, len2);
    }

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
    , m_data(s.m_data)
    {
    }

    explicit CWStr(const wchar *s, CHeap *heap = NULL)
    : CMain{}
    , m_data{s}
    {
    }

    CWStr(const wchar *s, int len, CHeap *heap = NULL)
    : CMain{}
    , m_data{s, static_cast<size_t>(len)}
    {
    }

    explicit CWStr(wchar sim, CHeap *heap = NULL)
    : CMain{}
    , m_data(size_t{1}, sim)
    {
    }

    CWStr(wchar sim, int count, CHeap *heap = NULL)
    : CMain{}
    , m_data(static_cast<size_t>(count), sim)
    {
    }

    explicit CWStr(int zn, CHeap *heap = NULL)
    : CMain{}
    {
        Set(zn);
    }

    explicit CWStr(DWORD zn, CHeap *heap = NULL)
    : CMain{}
    {
        Set(zn);
    }

    explicit CWStr(double zn, int zpz = 8, CHeap *heap = NULL)
    : CMain{}
    {
        Set(zn, zpz);
    }

    ~CWStr() = default;

    CHeap* GetHeap() const
    {
        return nullptr;
    }

    // Clear - Очищает строку
    void Clear()
    {
        m_data.clear();
    }

    void SetLen(int len)
    {
        m_data.resize(len);
    };

    void Set(const std::wstring& str)
    {
        m_data = str;
    }

    void Set(const CWStr &s)
    {
        m_data = s.m_data;
    }

    void Set(const wchar *s) {
        m_data = s;
    }
    void Set(const wchar *s, int len)
    {
        m_data = std::wstring{s, static_cast<size_t>(len)};
    }

    void Set(wchar sim)
    {
        m_data = std::wstring{1, sim};
    }

    void Set(wchar sim, int count)
    {
        m_data = std::wstring(static_cast<size_t>(count), sim);
    }

    void Set(int zn);
    void Set(dword zn);
    void Set(double zn, int zpz = 8);
    void SetHex(void *zn);
    void SetHex(BYTE zn);

    CWStr& Add(const std::wstring& str);
    CWStr &Add(const CWStr &cstr);
    CWStr &Add(const wchar *str);
    CWStr &Add(const wchar *str, int lstr);
    CWStr &Add(wchar sim);
    CWStr &Add(wchar sim, int count);
    CWStr &Add(int zn) {
        Add(CWStr(zn));
        return *this;
    }
    CWStr &Add(double zn, int zpz = 8) {
        Add(CWStr(zn, zpz));
        return *this;
    }
    CWStr &AddHex(void *zn) {
        CWStr s(GetHeap());
        s.SetHex(zn);
        Add(s);
        return *this;
    }
    CWStr &AddHex(BYTE zn) {
        CWStr s(GetHeap());
        s.SetHex(zn);
        Add(s);
        return *this;
    }

    const wchar* c_str() const
    {
        return m_data.c_str();
    }

    const std::wstring& to_wstring() const
    {
        return m_data;
    }

    const wchar *Get(void) const
    {
        return m_data.c_str();
    }

    const wchar *GetEx(void) const
    {
        if (m_data.empty())
            return nullptr;
        else
            return m_data.c_str();
    }
    wchar *GetBuf(void)
    {
        return const_cast<wchar*>(m_data.data());
    }
    wchar *GetBufEx(void)
    {
        if (m_data.empty())
            return nullptr;
        else
            return const_cast<wchar*>(m_data.data());
    }

    int GetLen(void) const
    {
        return m_data.length();
    }

    void RawSetLen(int len)
    {
        m_data.resize(len);
    }

    int GetInt(void) const;
    DWORD GetDword(void) const;
    double GetDouble(void) const;
    int GetHex(void) const;
    DWORD GetHexUnsigned(void) const;

    bool IsOnlyInt(void) const;
    bool IsEmpty(void) const
    {
        return m_data.empty();
    }

    CWStr &Trim(void);      // Удаляет в начале и в конце символы 0x20,0x9,0x0d,0x0a
    CWStr &TrimFull(void);  // Trim() и в середине строки удоляет повторяющиеся 0x20,0x9
    void TabToSpace(void);  // Конвертит 0x9 в 0x20

    CWStr &Del(int sme, int len);              // Удалить символы
    CWStr &Insert(int sme, const CWStr &istr)  // Вставить символы
    {
        return Insert(sme, istr.Get(), istr.GetLen());
    }
    CWStr &Insert(int sme, const wchar *str, int len);  // Вставить символы
    CWStr &Insert(int sme, const wchar *str)            // Вставить символы
    {
        return Insert(sme, str, WStrLen(str));
    }
    CWStr &Replace(const CWStr &substr, const CWStr &strreplace);  // Заменить часть строки ну другую

    int Find(const wchar *substr, int slen,
             int sme = 0) const;  // Поиск подстроки. return = смещение от начала  -1 = Подстрока не найдена
    int Find(const CWStr &substr, int sme = 0) const { return Find(substr.Get(), substr.GetLen(), sme); }
    int Find(const wchar *substr, int sme = 0) const { return Find(substr, WStrLen(substr), sme); }
    int FindR(wchar ch, int sme = -1)
            const;  // Поиск символа в обратном порядке. return = смещение от начала  -1 = Подстрока не найдена

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

    static int Compare(const wchar *zn1, int zn1len, const wchar *zn2, int zn2len);  // "A","B"=-1  "A","A"=0  "B","A"=1
    static int Compare(const CWStr &zn1, const CWStr &zn2) {
        return Compare(zn1.Get(), zn1.GetLen(), zn2.Get(), zn2.GetLen());
    }

    bool Equal(const wchar *zn, int len) const;
    bool Equal(const wchar *zn) const { return Equal(zn, WStrLen(zn)); }

    bool CompareFirst(const CWStr &str) const;
    bool CompareFirst(const wchar *str) const { return CompareFirst(CWStr(str)); }
    int CompareSubstring(const CWStr &str) const;
    int CompareSubstring(const wchar *str) const { return CompareSubstring(CWStr(str)); }

    CWStr &operator=(const CWStr &s) {
        Set(s);
        return *this;
    }
    CWStr &operator=(const wchar *s) {
        Set(s);
        return *this;
    }
    CWStr &operator=(wchar zn) {
        Set(zn);
        return *this;
    }
    CWStr &operator=(int zn) {
        Set(zn);
        return *this;
    }
    CWStr &operator=(double zn) {
        Set(zn);
        return *this;
    }

    wchar &operator[](int n)
    {
        return m_data[n];
    }

    const wchar &operator[](int n) const
    {
        return m_data[n];
    }

    friend bool operator==(const CWStr &zn1, const CWStr &zn2)
    {
        return zn1.m_data == zn2.m_data;
    }
    friend bool operator==(const CWStr &zn1, const wchar *zn2)
    {
        return zn1.m_data == zn2;
    }
    friend bool operator==(const wchar *zn1, const CWStr &zn2) { return zn2 == zn1; }

    friend bool operator!=(const CWStr &zn1, const CWStr &zn2) { return !(zn1 == zn2); }
    friend bool operator!=(const CWStr &zn1, const wchar *zn2) { return !(zn1 == zn2); }
    friend bool operator!=(const wchar *zn1, const CWStr &zn2) { return !(zn1 == zn2); }

    friend bool operator<(const CWStr &zn1, const CWStr &zn2) {
        if (Compare(zn1, zn2) < 0)
            return 1;
        return 0;
    }
    friend bool operator<(const CWStr &zn1, const wchar *zn2) {
        if (Compare(zn1, CWStr(zn2)) < 0)
            return 1;
        return 0;
    }
    friend bool operator<(const wchar *zn1, const CWStr &zn2) {
        if (Compare(CWStr(zn1), zn2) < 0)
            return 1;
        return 0;
    }

    friend bool operator>(const CWStr &zn1, const CWStr &zn2) {
        if (Compare(zn1, zn2) > 0)
            return 1;
        return 0;
    }
    friend bool operator>(const CWStr &zn1, const wchar *zn2) {
        if (Compare(zn1, CWStr(zn2)) > 0)
            return 1;
        return 0;
    }
    friend bool operator>(const wchar *zn1, const CWStr &zn2) {
        if (Compare(CWStr(zn1), zn2) > 0)
            return 1;
        return 0;
    }

    friend bool operator<=(const CWStr &zn1, const CWStr &zn2) {
        if (Compare(zn1, zn2) <= 0)
            return 1;
        return 0;
    }
    friend bool operator<=(const CWStr &zn1, const wchar *zn2) {
        if (Compare(zn1, CWStr(zn2)) <= 0)
            return 1;
        return 0;
    }
    friend bool operator<=(const wchar *zn1, const CWStr &zn2) {
        if (Compare(CWStr(zn1), zn2) <= 0)
            return 1;
        return 0;
    }

    friend bool operator>=(const CWStr &zn1, const CWStr &zn2) {
        if (Compare(zn1, zn2) >= 0)
            return 1;
        return 0;
    }
    friend bool operator>=(const CWStr &zn1, const wchar *zn2) {
        if (Compare(zn1, CWStr(zn2)) >= 0)
            return 1;
        return 0;
    }
    friend bool operator>=(const wchar *zn1, const CWStr &zn2) {
        if (Compare(CWStr(zn1), zn2) >= 0)
            return 1;
        return 0;
    }

    CWStr &operator+=(const CWStr &str) {
        Add(str);
        return *this;
    }
    CWStr &operator+=(const wchar *str) {
        Add(str);
        return *this;
    }
    CWStr &operator+=(const wchar sim) {
        Add(sim);
        return *this;
    }
    CWStr &operator+=(int zn) {
        Add(zn);
        return *this;
    }
    CWStr &operator+=(double zn) {
        Add(zn);
        return *this;
    }
    //      CWStr & operator +=(void * zn)                                  { Add(zn); return *this; }

    friend CWStr operator+(const CWStr &s1, const CWStr &s2) {
        return CWStr(s1.Get(), s1.GetLen(), s2.Get(), s2.GetLen(), s1.GetHeap());
    }
    friend CWStr operator+(const CWStr &s1, const wchar *s2) {
        return CWStr(s1.Get(), s1.GetLen(), s2, WStrLen(s2), s1.GetHeap());
    }
    friend CWStr operator+(const wchar *s1, const CWStr &s2) {
        return CWStr(s1, WStrLen(s1), s2.Get(), s2.GetLen(), s2.GetHeap());
    }
    friend CWStr operator+(const CWStr &s1, wchar sim) {
        CWStr str(s1.Get(), s1.GetLen(), s1.GetHeap());
        str += sim;
        return str;
    }
    friend CWStr operator+(wchar sim, const CWStr &s2) {
        CWStr str(sim, s2.GetHeap());
        str += s2;
        return str;
    }
    friend CWStr operator+(const CWStr &s, int zn) {
        CWStr str(s);
        str += zn;
        return str;
    }
    friend CWStr operator+(int zn, const CWStr &s) {
        CWStr str(zn, s.GetHeap());
        str += s;
        return str;
    }
};

}  // namespace Base
