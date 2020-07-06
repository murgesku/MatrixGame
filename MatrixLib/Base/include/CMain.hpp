// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "BaseDef.hpp"

#ifdef _DEBUG
//#define DEAD_CLASS_SPY_ENABLE
#endif

namespace Base {

class BASE_API CMain
{
public:
    CMain()
    {
    }
    ~CMain()
    {
    }

#ifdef DEAD_CLASS_SPY_ENABLE
    //virtual int DCS_UnderSpy(void) const {return 0;}
    void        DCS_ClassDead(int sz) const;
    CMain *     DCS_GetDeadBody(void) const;
    void        DCS_Reincarnation(void) const;

#define DCS_INCONSTRUCTOR() {DCS_Reincarnation();}
#define DCS_INDESTRUCTOR() {int sz = DCS_UnderSpy(); if (sz > 0) { DCS_ClassDead(sz); }}

#else
#define DCS_INCONSTRUCTOR()
#define DCS_INDESTRUCTOR()
#endif

    static void BaseInit(void);
    static void BaseDeInit(void);
};

}
