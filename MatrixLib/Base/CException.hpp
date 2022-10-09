// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include <string>

namespace Base {

class CWStr;

#define ERROR_E         throw Base::CException(__FILE__, __LINE__)
#define ERROR_S(s)      throw Base::CExceptionStr(__FILE__, __LINE__, s)
#define ERROR_S2(s, s2) throw Base::CExceptionStr(__FILE__, __LINE__, s, s2)

#ifdef ASSERT_OFF
#define ASSERT(zn)
#else
#ifdef _TRACE
#define ASSERT(zn)     \
    {                  \
        if ((zn) == 0) \
            ERROR_E;   \
    }
#else
#define ASSERT(zn)        \
    {                     \
        if ((zn) == 0)    \
            debugbreak(); \
    }
#endif
#endif

// lint -e1712
class CException
{
    const char *m_File;
    int m_Line;

    std::string call_trace;

public:
    CException(const char *file, int line);

    virtual ~CException() = default;

    virtual CWStr Info(void) const;
};

class BASE_API CExceptionStr : public CException
{
    std::wstring m_str;

public:
    CExceptionStr(const char *file, int line, const wchar *str, const wchar *str2 = NULL);

    CExceptionStr(const char *file, int line, const std::wstring& str)
        :CExceptionStr(file, line, str.c_str())
    {}

    virtual CWStr Info(void) const;
};
// lint +e1712
}  // namespace Base
