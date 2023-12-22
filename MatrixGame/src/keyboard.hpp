#pragma once

#include <cstdint>

enum EKeyAction : int; // predeclaration

namespace Keyboard {

void on_key_down(uint8_t vk);
void on_key_up(uint8_t vk);
bool is_down(uint8_t vk);

bool isVKeyPressed(uint32_t key);

bool isKeyPressed(EKeyAction ka);

} // namespace Keyboard