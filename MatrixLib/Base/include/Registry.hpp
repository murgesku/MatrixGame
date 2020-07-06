// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "basedef.hpp"

namespace Base {

BASE_API void Reg_GetString(HKEY pkey, const wchar * path, const wchar * name, const CWStr & str);
BASE_API CWStr Reg_GetString(HKEY pkey, const wchar * path, const wchar * name, const wchar * defaultstr);
BASE_API void Reg_SetString(HKEY pkey, const wchar * path, const wchar * name, const wchar * str);

}
