// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "BaseDef.hpp"
#include "CException.hpp"
#include "CMain.hpp"
#include "CHeap.hpp"
#include "CFile.hpp"
#include "CBuf.hpp"
#include "CBlockPar.hpp"
#include "Registry.hpp"

#ifndef MAXEXP_EXPORTS
#include "CReminder.hpp"
#include "CDWORDMap.hpp"
#include "CRC32.hpp"
#include "CStorage.hpp"
#include "Pack.hpp"
#endif

#include "Tracer.hpp"

// it's a total shit that we have to do this...
#ifdef MSVC7
    #undef min
    #undef max
#endif

//////////////////////////
using wchar = wchar_t;
using dword = unsigned long;
//////////////////////////

#define IS_UNICODE() true
