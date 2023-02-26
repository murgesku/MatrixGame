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

#include <string>
#include <cwchar>

//////////////////////////
// TODO: remove
#include <windows.h>
#include <stdlib.h>

using wchar = wchar_t;
using dword = unsigned long;
//////////////////////////

namespace Base {

// TODO: replace with plain std func calls
inline int WStrLen(const wchar *str) {
    return std::wcslen(str);
}

inline bool WStrCmp(const wchar_t *s1, const wchar_t *s2) {
    return !std::wcscmp(s1, s2);
}

using CWStr = std::wstring;

}  // namespace Base
