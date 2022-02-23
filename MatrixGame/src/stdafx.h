// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef STDAFX_MAIN_HEADER
#define STDAFX_MAIN_HEADER

// Windows Header Files:
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#include "../../MatrixLib/3G/3g.pch"

#include "../../MatrixLib/3G/3g.hpp"
#include "../../MatrixLib/3G/VectorObject.hpp"
#include "../../MatrixLib/3G/CBillboard.hpp"
#include "../../MatrixLib/3G/BigVB.hpp"
#include "../../MatrixLib/3G/BigIB.hpp"
#include "../../MatrixLib/DebugMsg/DebugMsg.h"

#include "MatrixMultiSelection.hpp"

#ifdef MATRIXGAME_EXPORTS
#define MATRIXGAMEDLL_API __declspec(dllexport)
#else
#define MATRIXGAMEDLL_API __declspec(dllimport)
#endif

#include "MatrixGame.h"

using namespace Base;

#endif