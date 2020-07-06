// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "BaseDef.hpp"
#include "CException.hpp"
#include "CMain.hpp"
#include "CHeap.hpp"
#include "CStr.hpp"
#include "CWStr.hpp"
#include "CFile.hpp"
#include "CBuf.hpp"
#include "CBlockPar.hpp"
#include "Registry.hpp"
#include "CReminder.hpp"
#include "CDWORDMap.hpp"
#include "CRC32.hpp"
#include "CStorage.hpp"
#include "Pack.hpp"

#include "Tracer.hpp"

#define IS_UNICODE() (GetVersion()<0x80000000)
