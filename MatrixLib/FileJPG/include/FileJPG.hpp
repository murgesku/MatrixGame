// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#ifdef FILEJPG_DLL
	#ifdef FILEJPG_EXPORTS
		#define FILEJPG_API __declspec(dllexport)
	#else
		#define FILEJPG_API __declspec(dllimport)
	#endif
#else
	#define FILEJPG_API
#endif

FILEJPG_API DWORD FileJPG_ReadStart_Buf(void * soubuf,DWORD soubuflen,DWORD * lenx,DWORD * leny);
FILEJPG_API DWORD FileJPG_Read(DWORD id,void * buf,DWORD lenline);
