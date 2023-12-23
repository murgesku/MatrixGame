#pragma once

#include <cstdint>
#include <windows.h>

enum EKeyAction : int; // predeclaration

namespace Keyboard {

void on_key_down(uint8_t vk);
void on_key_up(uint8_t vk);
bool is_down(uint8_t vk);

bool isKeyPressed(EKeyAction ka);

} // namespace Keyboard

#define KEY_ESC 1
#define KEY_F1  59
#define KEY_F2  60
#define KEY_F3  61
#define KEY_F4  62
#define KEY_F5  63
#define KEY_F6  64
#define KEY_F7  65
#define KEY_F8  66
#define KEY_F9  67
#define KEY_F10 68
#define KEY_F11 87
#define KEY_F12 88

#define KEY_PAUSE 69

#define KEY_SLOCK 70
#define KEY_CLOCK 58
#define KEY_NLOCK 197

#define KEY_TILDA     41
#define KEY_RSLASH    53
#define KEY_LSLASH    43
#define KEY_COMMA     52
#define KEY_COLON     51
#define KEY_SEMICOLON 39
#define KEY_APOSTROF  40
#define KEY_LBRACKET  26
#define KEY_RBRACKET  27
#define KEY_MINUS     12
#define KEY_EQUAL     13

#define KEY_PADSLASH 181
#define KEY_PADSTAR  55
#define KEY_PADMINUS 74
#define KEY_PADPLUS  78
#define KEY_PADENTER 156
#define KEY_PADDEL   83
#define KEY_PAD0     82
#define KEY_PAD1     79
#define KEY_PAD2     80
#define KEY_PAD3     81
#define KEY_PAD4     75
#define KEY_PAD5     76
#define KEY_PAD6     77
#define KEY_PAD7     71
#define KEY_PAD8     72
#define KEY_PAD9     73

#define KEY_1 2
#define KEY_2 3
#define KEY_3 4
#define KEY_4 5
#define KEY_5 6
#define KEY_6 7
#define KEY_7 8
#define KEY_8 9
#define KEY_9 10
#define KEY_0 11

#define KEY_LEFT  203
#define KEY_RIGHT 205
#define KEY_UP    200
#define KEY_DOWN  208

#define KEY_BACKSPACE 14
#define KEY_TAB       15
#define KEY_ENTER     28
#define KEY_SPACE     57

#define KEY_INSERT 210
#define KEY_DELETE 211
#define KEY_HOME   199
#define KEY_END    207
#define KEY_PGUP   201
#define KEY_PGDN   209

#define KEY_LSHIFT 42
#define KEY_RSHIFT 54
#define KEY_LALT   56
#define KEY_RALT   184
#define KEY_LCTRL  29
#define KEY_RCTRL  157

#define KEY_LWIN 219
#define KEY_RWIN 220
#define KEY_MENU 221

#define KEY_Q 16
#define KEY_W 17
#define KEY_E 18
#define KEY_R 19
#define KEY_T 20
#define KEY_Y 21
#define KEY_U 22
#define KEY_I 23
#define KEY_O 24
#define KEY_P 25

#define KEY_A 30
#define KEY_S 31
#define KEY_D 32
#define KEY_F 33
#define KEY_G 34
#define KEY_H 35
#define KEY_J 36
#define KEY_K 37
#define KEY_L 38

#define KEY_Z 44
#define KEY_X 45
#define KEY_C 46
#define KEY_V 47
#define KEY_B 48
#define KEY_N 49
#define KEY_M 50

#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A

#define VK_LSLASH   VK_OEM_2
#define VK_TILDA    VK_OEM_3
#define VK_PERIOD   VK_OEM_PERIOD
#define VK_LBRACKET VK_OEM_4
#define VK_RBRACKET VK_OEM_6

static char VKey2Char(uint8_t vk)
{
    if (vk == VK_SPACE)  return ' ';
    if (vk == VK_A)      return 'A';
    if (vk == VK_B)      return 'B';
    if (vk == VK_C)      return 'C';
    if (vk == VK_D)      return 'D';
    if (vk == VK_E)      return 'E';
    if (vk == VK_F)      return 'F';
    if (vk == VK_G)      return 'G';
    if (vk == VK_H)      return 'H';
    if (vk == VK_I)      return 'I';
    if (vk == VK_J)      return 'J';
    if (vk == VK_K)      return 'K';
    if (vk == VK_L)      return 'L';
    if (vk == VK_M)      return 'M';
    if (vk == VK_N)      return 'N';
    if (vk == VK_O)      return 'O';
    if (vk == VK_P)      return 'P';
    if (vk == VK_Q)      return 'Q';
    if (vk == VK_R)      return 'R';
    if (vk == VK_S)      return 'S';
    if (vk == VK_T)      return 'T';
    if (vk == VK_U)      return 'U';
    if (vk == VK_V)      return 'V';
    if (vk == VK_W)      return 'W';
    if (vk == VK_X)      return 'X';
    if (vk == VK_Y)      return 'Y';
    if (vk == VK_Z)      return 'Z';
    if (vk == VK_0)      return '0';
    if (vk == VK_1)      return '1';
    if (vk == VK_2)      return '2';
    if (vk == VK_3)      return '3';
    if (vk == VK_4)      return '4';
    if (vk == VK_5)      return '5';
    if (vk == VK_6)      return '6';
    if (vk == VK_7)      return '7';
    if (vk == VK_8)      return '8';
    if (vk == VK_9)      return '9';
    if (vk == VK_LSLASH) return '\\';
    if (vk == VK_PERIOD) return '.';
    if (vk == VK_TILDA)  return '~';
    return 0;
}