// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#ifdef BITMAP_DLL
	#ifdef BITMAP_EXPORTS
		#define BITMAP_API __declspec(dllexport)
	#else
		#define BITMAP_API __declspec(dllimport)
	#endif
#else
	#define BITMAP_API
#endif
