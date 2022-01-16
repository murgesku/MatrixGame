// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "CException.hpp"
#include "CMain.hpp"
#include "CHeap.hpp"

namespace Base {

class BASE_API CStr : public CMain {
    CHeap *m_Heap;
    int m_MaxLen;
    int m_Len;
    char *m_Str;

    void Tream(int len) {
        if ((len == 0) || (len > m_MaxLen)) {
            m_MaxLen = len + 16;
            m_Str = (char *)HAllocEx(m_Str, uint(m_MaxLen + 1), m_Heap);
        }
    }

    void Init(const char *src, int len, CHeap *heap) {
        m_Heap = heap;
        m_Len = len;
        m_MaxLen = len + 16;
        m_Str = (char *)HAlloc(uint(m_MaxLen + 1), m_Heap);
        if (len == 0) {
            *m_Str = 0;
        }
        else {
            memcpy(m_Str, src, uint(len + 1));
        }
    };
    void Init(const wchar *src, int len, CHeap *heap);

public:
    CStr(void) : CMain(), m_Heap(NULL), m_MaxLen(0), m_Len(0), m_Str(NULL) {}
    explicit CStr(CHeap *heap) : CMain(), m_Heap(heap), m_MaxLen(0), m_Len(0), m_Str(NULL) {}
    CStr(const CStr &s, CHeap *heap = NULL) : CMain() { Init(s.m_Str, s.Len(), heap ? heap : s.m_Heap); }
    explicit CStr(const CWStr &s, CHeap *heap = NULL);
    explicit CStr(const wchar *s, CHeap *heap = NULL);
    explicit CStr(const char *s, CHeap *heap = NULL) : CMain() { Init(s, int(strlen(s)), heap); }
    CStr(const char *s, int len, CHeap *heap = NULL) : CMain() { Init(s, len, heap); }
    explicit CStr(char sim, CHeap *heap = NULL) : CMain(), m_Heap(heap), m_MaxLen(16), m_Len(1) {
        m_Str = (char *)HAlloc(uint(m_MaxLen + 1), m_Heap);
        m_Str[0] = sim;
        m_Str[1] = 0;
    };
    CStr(char sim, int count, CHeap *heap = NULL) : CMain(), m_Heap(heap), m_MaxLen(count + 16), m_Len(count) {
        m_Str = (char *)HAlloc(uint(m_MaxLen + 1), m_Heap);
        memset(m_Str, int(sim), uint(count));
        m_Str[count] = 0;
    };
    explicit CStr(int zn, CHeap *heap = NULL);
    explicit CStr(double zn, int zpz = 8, CHeap *heap = NULL);
    //		CStr(void * zn, CHeap * heap=NULL);
    //		CStr(BYTE zn, CHeap * heap=NULL);
    ~CStr() { ClearFull(); }

    // Clear - Очищает строку
    void Clear(void);

    // ClearFull - Очищает строку и удаляет буфер
    void ClearFull(void) {
        if (m_Str != NULL) {
            HFree(m_Str, m_Heap);
            m_Str = NULL;
        }
        m_Len = m_MaxLen = 0;
    }

    void SetLen(int len);  // Установить длину строки (Выделение памяти если нужно)
    void SetZeroLenNoTream(void) {
        m_Len = 0;
        m_Str[0] = 0;
    };

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
    void Add(double zn, int zpz = 8) { Add(CStr(zn, zpz)); }
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
    const char *GetEx(void) const {
        if (IsEmpty())
            return NULL;
        return m_Str;
    }
    char *GetBuf(void) { return m_Str; }
    char *GetBufEx(void) const {
        if (IsEmpty())
            return NULL;
        return m_Str;
    }
    int GetLen(void) const { return m_Len; }
    int Len(void) const { return m_Len; }

    int GetInt(void) const;
    double GetDouble(void) const;
    int GetHex(void) const;
    DWORD GetHexUnsigned(void) const;

    bool IsOnlyInt(void) const;
    bool IsEmpty(void) const { return m_Len < 1; }

    CStr &Trim(void);      // Удаляет в начале и в конце символы 0x20,0x9,0x0d,0x0a
    CStr &TrimFull(void);  // Trim() и в середине строки удоляет повторяющиеся 0x20,0x9
    void TabToSpace(void);  // Конвертит 0x9 в 0x20

    void Del(int sme, int len);                        // Удалить символы
    void Insert(int sme, const char *tstr, int slen);  // Вставить символы
    void Insert(int sme, const char *tstr) { Insert(sme, tstr, (int)strlen(tstr)); };
    void Insert(int sme, const CStr &tstr) { Insert(sme, tstr.Get(), tstr.Len()); };
    void Replace(const CStr &substr, const CStr &strreplace);  // Заменить часть строки ну другую

    int Find(const char *substr, int sublen,
             int sme = 0) const;  // Поиск подстроки. return = смещение от начала  -1 = Подстрока не найдена
    int Find(const CStr &substr, int sme = 0) const { return Find(substr.Get(), substr.Len(), sme); };
    int FindR(const char *substr, int sublen) const;
    int FindR(const char *substr) const { return FindR(substr, (int)strlen(substr)); };

    int GetSmePar(int np, const char *ogsim) const;
    int GetLenPar(int smepar, const char *ogsim) const;

    CStr GetStrPar(int np, const char *ogsim) const {
        int sme = GetSmePar(np, ogsim);
        return CStr(Get() + sme, GetLenPar(sme, ogsim), GetHeap());
    }

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
        rem.SetLen(0);
    }

    void LowerCase(int sme = 0, int len = -1);
    void UpperCase(int sme = 0, int len = -1);

    static int Compare(const CStr &zn1, const CStr &zn2);  // "A","B"=-1  "A","A"=0  "B","A"=1
    int CompareFirst(const CStr &str);
    int CompareFirst(const char *str) { return CompareFirst(CStr(str)); }
    int CompareSubstring(const CStr &str);
    int CompareSubstring(const char *str) { return CompareSubstring(CStr(str)); }

    CStr &operator=(const CStr &s) {
        if (this != &s) {
            Set(s);
        }
        return *this;
    }
    CStr &operator=(const CWStr &s) {
        Set(s);
        return *this;
    }
    CStr &operator=(const char *s) {
        Set(s);
        return *this;
    }
    CStr &operator=(char zn) {
        Set(zn);
        return *this;
    }
    CStr &operator=(int zn) {
        Set(zn);
        return *this;
    }
    CStr &operator=(double zn) {
        Set(zn);
        return *this;
    }
    //		CStr & operator = (void * zn)									{ Set(zn); return *this; }

    char operator[](int n) const { /*lint -e613*/
        return *(m_Str + n);       /*lint +e613*/
    }
    char &operator[](int n) { /*lint -e613*/
        return *(m_Str + n);  /*lint +e613*/
    }

    friend int operator==(const CStr &zn1, const CStr &zn2);
    friend int operator==(const CStr &zn1, const char *zn2);
    friend int operator==(const char *zn1, const CStr &zn2) { return zn2 == zn1; }

    friend int operator!=(const CStr &zn1, const CStr &zn2) { return !(zn1 == zn2); }
    friend int operator!=(const CStr &zn1, const char *zn2) { return !(zn1 == zn2); }
    friend int operator!=(const char *zn1, const CStr &zn2) { return !(zn1 == zn2); }

    friend int operator<(const CStr &zn1, const CStr &zn2) {
        if (CStr::Compare(zn1, zn2) < 0) {
            return 1;
        }
        return 0;
    }
    friend int operator<(const CStr &zn1, const char *zn2) {
        if (CStr::Compare(zn1, CStr(zn2)) < 0) {
            return 1;
        }
        return 0;
    }
    friend int operator<(const char *zn1, const CStr &zn2) {
        if (CStr::Compare(CStr(zn1), zn2) < 0) {
            return 1;
        }
        return 0;
    }

    friend int operator>(const CStr &zn1, const CStr &zn2) {
        if (CStr::Compare(zn1, zn2) > 0) {
            return 1;
        }
        return 0;
    }
    friend int operator>(const CStr &zn1, const char *zn2) {
        if (CStr::Compare(zn1, CStr(zn2)) > 0) {
            return 1;
        }
        return 0;
    }
    friend int operator>(const char *zn1, const CStr &zn2) {
        if (CStr::Compare(CStr(zn1), zn2) > 0) {
            return 1;
        }
        return 0;
    }

    friend int operator<=(const CStr &zn1, const CStr &zn2) {
        if (CStr::Compare(zn1, zn2) <= 0) {
            return 1;
        }
        return 0;
    }
    friend int operator<=(const CStr &zn1, const char *zn2) {
        if (CStr::Compare(zn1, CStr(zn2)) <= 0) {
            return 1;
        }
        return 0;
    }
    friend int operator<=(const char *zn1, const CStr &zn2) {
        if (CStr::Compare(CStr(zn1), zn2) <= 0) {
            return 1;
        }
        return 0;
    }

    friend int operator>=(const CStr &zn1, const CStr &zn2) {
        if (CStr::Compare(zn1, zn2) >= 0) {
            return 1;
        }
        return 0;
    }
    friend int operator>=(const CStr &zn1, const char *zn2) {
        if (CStr::Compare(zn1, CStr(zn2)) >= 0) {
            return 1;
        }
        return 0;
    }
    friend int operator>=(const char *zn1, const CStr &zn2) {
        if (CStr::Compare(CStr(zn1), zn2) >= 0) {
            return 1;
        }
        return 0;
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
    CStr &operator+=(double zn) {
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
    friend CStr operator+(const char *s1, const CStr &s2) {
        CStr str(s1);
        str += s2;
        return str;
    }
    friend CStr operator+(const CStr &s1, char sim) {
        CStr str(s1);
        str += sim;
        return str;
    }
    friend CStr operator+(char sim, const CStr &s2) {
        CStr str(sim);
        str += s2;
        return str;
    }
    friend CStr operator+(const CStr &s, int zn) {
        CStr str(s);
        str += zn;
        return str;
    }
    friend CStr operator+(int zn, const CStr &s) {
        CStr str(zn);
        str += s;
        return str;
    }
    friend CStr operator+(const CStr &s, double zn) {
        CStr str(s);
        str += zn;
        return str;
    }
    friend CStr operator+(double zn, const CStr &s) {
        CStr str(zn);
        str += s;
        return str;
    }
    //		friend CStr operator + (void * zn,CStr & s)						{ CStr str(zn); str+=s; return str; }
    //		friend CStr operator + (CStr & s,void * zn)						{ s.Add(zn); return s; }

    // operator int (void) const										{ return GetInt(); }
    // operator double (void) const			    					{ return GetDouble(); }

    // lint -e1930
    operator const char *(void) const { return Get(); }
    // lint +e1930

    // lint -e1763
    CHeap *GetHeap(void) const { return m_Heap; }
    // lint +e1763
};

}  // namespace Base
