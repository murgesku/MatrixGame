// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "BaseDef.hpp"

#include <string>
#include <windows.h>

namespace Base {

void Reg_GetString(HKEY pkey, const wchar_t *path, const wchar_t *name, const std::wstring &str);
std::wstring Reg_GetString(HKEY pkey, const wchar_t *path, const wchar_t *name, const wchar_t *defaultstr);
void Reg_SetString(HKEY pkey, const wchar_t *path, const wchar_t *name, const wchar_t *str);

}  // namespace Base
