#pragma once

#include <MatrixConfig.hpp>

#include <windows.h>

extern CMatrixConfig g_Config;

namespace Keyboard {

inline bool isKeyPressed(EKeyAction ka)
{
    return (GetAsyncKeyState(g_Config.m_KeyActions[ka]) & 0x8000) == 0x8000;
}

inline bool isVKeyPressed(uint32_t key)
{
    return (GetAsyncKeyState(key) & 0x8000) == 0x8000;
}

} // namespace Keyboard