// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "BaseDef.hpp"

namespace Base {

dword CalcCRC32(const void *buf, int len);
dword CalcCRC32_Begin(const void *buf, int len);
dword CalcCRC32_Buf(dword crc, const void *buf, int len);
dword CalcCRC32_End(dword crc);

}  // namespace Base
