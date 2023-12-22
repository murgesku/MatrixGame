// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "Form.hpp"

#include <cheats.hpp>

#include <deque>
#include <string>
#include <cstdint>

#pragma once

#define MAX_SCANS             16
#define MOUSE_BORDER          4
#define DOUBLESCAN_TIME_DELTA 200

class CFormMatrixGame : public CForm {
private:
    float m_LastWorldX, m_LastWorldY;
    int m_Action;

    std::deque<SKeyScan> m_LastScans{MAX_SCANS};

public:
    CFormMatrixGame(void);
    ~CFormMatrixGame();

    virtual void Enter(void);
    virtual void Leave(void);
    virtual void Draw(void);

    virtual void Takt(int step);

    virtual void MouseMove(int x, int y);
    virtual void MouseKey(ButtonStatus status, int key, int x, int y);

    virtual void Keyboard(bool down, int scan, uint8_t vk);
    virtual void SystemEvent(ESysEvent se);
};
