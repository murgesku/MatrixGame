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

static char Scan2Char(int scan)
{
    if (scan == KEY_SPACE)  return ' ';
    if (scan == KEY_A)      return 'A';
    if (scan == KEY_B)      return 'B';
    if (scan == KEY_C)      return 'C';
    if (scan == KEY_D)      return 'D';
    if (scan == KEY_E)      return 'E';
    if (scan == KEY_F)      return 'F';
    if (scan == KEY_G)      return 'G';
    if (scan == KEY_H)      return 'H';
    if (scan == KEY_I)      return 'I';
    if (scan == KEY_J)      return 'J';
    if (scan == KEY_K)      return 'K';
    if (scan == KEY_L)      return 'L';
    if (scan == KEY_M)      return 'M';
    if (scan == KEY_N)      return 'N';
    if (scan == KEY_O)      return 'O';
    if (scan == KEY_P)      return 'P';
    if (scan == KEY_Q)      return 'Q';
    if (scan == KEY_R)      return 'R';
    if (scan == KEY_S)      return 'S';
    if (scan == KEY_T)      return 'T';
    if (scan == KEY_U)      return 'U';
    if (scan == KEY_V)      return 'V';
    if (scan == KEY_W)      return 'W';
    if (scan == KEY_X)      return 'X';
    if (scan == KEY_Y)      return 'Y';
    if (scan == KEY_Z)      return 'Z';
    if (scan == KEY_0)      return '0';
    if (scan == KEY_1)      return '1';
    if (scan == KEY_2)      return '2';
    if (scan == KEY_3)      return '3';
    if (scan == KEY_4)      return '4';
    if (scan == KEY_5)      return '5';
    if (scan == KEY_6)      return '6';
    if (scan == KEY_7)      return '7';
    if (scan == KEY_8)      return '8';
    if (scan == KEY_9)      return '9';
    if (scan == KEY_LSLASH) return '\\';
    if (scan == KEY_COMMA)  return '.'; // WTF?
    if (scan == KEY_TILDA)  return '~';
    return 0;
}
