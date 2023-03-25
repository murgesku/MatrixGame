// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include <cstdint>

inline void memcopy_back_dword(void *tgt, const void *src,
                                      uint32_t size)  // same as blk_copy, but copying by sizeof(uint) bytes
{
    uint32_t *uitgt = ((uint32_t *)tgt) + size - 1;
    const uint32_t *uisrc = ((const uint32_t *)src) + size - 1;
    while (size) {
        *uitgt-- = *uisrc--;
        size--;
    }
}
