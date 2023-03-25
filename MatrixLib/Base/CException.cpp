// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "CException.hpp"
#include <stdio.h>

#include "Tracer.hpp"

#include <utils.hpp>

namespace Base {

CException::CException(const char *file, int line)
: m_File{file}
, m_Line{line}
, call_trace{generate_trace_text()}
{
}

std::wstring CException::Info() const
{
    return
        utils::format(
            L"%lsFile=%ls\nLine=%d\n",
            utils::to_wstring(call_trace).c_str(),
            utils::to_wstring(m_File).c_str(),
            m_Line);
}

CExceptionStr::CExceptionStr(
    const char *file,
    int line,
    const wchar_t *str,
    const wchar_t *str2)
: CException(file, line)
{
    m_str = str;
    m_str += str2;
}

std::wstring CExceptionStr::Info() const
{
    return CException::Info() + L"Text: " + m_str.c_str();
}

}  // namespace Base
