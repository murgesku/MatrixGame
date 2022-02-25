// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#ifdef FILEPNG_DLL
#ifdef FILEPNG_EXPORTS
#define FILEPNG_API __declspec(dllexport)
#else
#define FILEPNG_API __declspec(dllimport)
#endif
#else
#define FILEPNG_API
#endif

// format: 1-gray 2-rgb 3-rgba 4-palate
FILEPNG_API DWORD __cdecl FilePNG_ReadStart_Buf(void *soubuf, DWORD soubuflen, DWORD *lenx, DWORD *leny,
                                               DWORD *countcolor, DWORD *format);
FILEPNG_API DWORD __cdecl FilePNG_Read(DWORD id, void *buf, DWORD lenline, DWORD *arraycolor);

// Возвращает полный размер файла. Если больше bufoutlen то нужно вызвать повторно. При ошибке 0
FILEPNG_API int __cdecl FilePNG_Write(void *bufout, int bufoutlen, void *buf, DWORD ll, DWORD lx, DWORD ly, DWORD bytepp,
                                     int rgb_to_bgr);
