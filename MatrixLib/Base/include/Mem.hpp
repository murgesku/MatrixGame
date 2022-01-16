// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "basedef.hpp"

__forceinline void memcopy_back_dword(void *tgt, const void *src,
                                      DWORD size)  // same as blk_copy, but copying by sizeof(uint) bytes
{
    DWORD *uitgt = ((DWORD *)tgt) + size - 1;
    const DWORD *uisrc = ((const DWORD *)src) + size - 1;
    while (size) {
        *uitgt-- = *uisrc--;
        size--;
    }
}
