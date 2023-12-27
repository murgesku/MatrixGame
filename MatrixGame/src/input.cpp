#include <input.hpp>

#include <MatrixConfig.hpp>

#include <bitset>

extern CMatrixConfig g_Config;

namespace Input {

static std::bitset<256> down_keys;

void onKeyDown(uint8_t vk)
{
    down_keys.set(vk);
}

void onKeyUp(uint8_t vk)
{
    down_keys.reset(vk);
}

bool isVKeyPressed(uint8_t vk)
{
    return down_keys.test(vk);
}

bool isKeyPressed(EKeyAction ka)
{
    return isVKeyPressed(g_Config.m_KeyActions[ka]);
}

} // namespace Input