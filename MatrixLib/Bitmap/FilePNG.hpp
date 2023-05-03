// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include <cstdint>

namespace FilePNG
{

// format: 1-gray 2-rgb 3-rgba 4-palate
uint32_t ReadStart_Buf(void *soubuf, uint32_t soubuflen, uint32_t *lenx, uint32_t *leny, uint32_t *countcolor, uint32_t *format);
uint32_t Read(uint32_t id, void *buf, uint32_t lenline, uint32_t *arraycolor);

// Возвращает полный размер файла. Если больше bufoutlen то нужно вызвать повторно. При ошибке 0
int Write(void *bufout, int bufoutlen, void *buf, uint32_t ll, uint32_t lx, uint32_t ly, uint32_t bytepp,
                                     int rgb_to_bgr);

} // namespace FilePNG