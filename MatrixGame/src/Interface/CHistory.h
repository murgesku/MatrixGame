// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "CConstructor.h"

class CHistory : public CMain
{

    void DelConfig(SRobotConfig* config);
    void FindConfig(SRobotConfig* config);

public:

    SRobotConfig* m_CurrentConfig;
    void LoadCurrentConfigToConstructor();
    SRobotConfig* m_FirstConfig;
    SRobotConfig* m_LastConfig;

    void AddConfig(SRobotConfig* config);
    void __stdcall PrevConfig(void* object);
    void __stdcall NextConfig(void* object);

    bool IsNext()
    {
        if(m_CurrentConfig && m_CurrentConfig->m_NextConfig) return true;
        else return false;
    }
    bool IsPrev()
    {
        if(m_CurrentConfig && m_CurrentConfig->m_PrevConfig) return true;
        else return false;
    }

    CHistory();
    ~CHistory();
};