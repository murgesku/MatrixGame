// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "CDWORDMap.hpp"

namespace Base {


void CDWORDMap::Enum(ENUM_DWORD f, DWORD user)
{
    for (int i = 0; i<HASH_TABLE_SIZE; ++i)
    {
        if (m_Table[i])
        {
            for (DWORD j = 0; j<m_Table[i]->contained; ++j)
            {
                if (!f((m_Table[i] + 1 + j)->key, (m_Table[i] + 1 + j)->value, user)) return;
            }
        }
    }
}

}