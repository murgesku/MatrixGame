// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "Texture.hpp"

class CInterface;
class CIFaceStatic;
class CIFaceElement;
class CIFaceImage;
struct SRobotConfig;
enum ERobotUnitType : unsigned int;
enum ERobotUnitKind : unsigned int;

#define UNIT_HEIGHT        19
#define TOPLEFT_HEIGHT     18
#define TOPLEFT_WIDTH      13
#define TOPRIGHT_HEIGHT    18
#define TOPRIGHT_WIDTH     18
#define BOTTOMLEFT_HEIGHT  22
#define BOTTOMLEFT_WIDTH   14
#define BOTTOMRIGHT_HEIGHT 21
#define BOTTOMRIGHT_WIDTH  13
#define TOPLINE_HEIGHT     18
#define TOPLINE_WIDTH      1
#define BOTTOMLINE_HEIGHT  22
#define BOTTOMLINE_WIDTH   1
#define RIGHTLINE_HEIGHT   1
#define RIGHTLINE_WIDTH    18
#define LEFTLINE_HEIGHT    1
#define LEFTLINE_WIDTH     13
#define CURSIK_WIDTH       7
#define LEFT_SPACE         7

#define DEFAULT_LABELS_COLOR  0xFFF6c000
#define NERES_LABELS_COLOR    0xFFFF4319
#define SELECTED_LABELS_COLOR 0xFFFFFFFF

#define _ENGLISH_BUILD

#ifdef _ENGLISH_BUILD
#define WEAPON_MENU_WIDTH  95
#define HULL_MENU_WIDTH    60
#define HEAD_MENU_WIDTH    45
#define CHASSIS_MENU_WIDTH 60
#else
#define WEAPON_MENU_WIDTH  70
#define HULL_MENU_WIDTH    90
#define HEAD_MENU_WIDTH    100
#define CHASSIS_MENU_WIDTH 90
#endif

enum EMenuParent {
    MENU_PARENT_UNDEF = 0,
    MENU_PARENT_PILON1 = 1,
    MENU_PARENT_PILON2 = 2,
    MENU_PARENT_PILON3 = 3,
    MENU_PARENT_PILON4 = 4,
    MENU_PARENT_PILON5 = 5,
    MENU_PARENT_HEAD = 6,
    MENU_PARENT_HULL = 7,
    MENU_PARENT_CHASSIS = 8,
};
struct SMenuItemText {
    std::wstring text;
    DWORD color;
    SMenuItemText(CHeap *heap) : text{} { color = DEFAULT_LABELS_COLOR; }
};

inline int GetIndexFromTK(ERobotUnitType type, ERobotUnitKind kind);

class CIFaceMenu : public CMain {
    float m_Width;
    float m_Height;
    int m_ElementsNum;

    bool m_Visible;

    CIFaceStatic *m_Ramka;
    CIFaceStatic *m_Selector;
    CIFaceStatic *m_Cursor;
    CIFaceImage *m_CursikImage;

    CTextureManaged *m_RamTex;

    int m_CurMenuPos;
    EMenuParent m_InterfaceParent;

    CIFaceElement *m_Caller;

    SRobotConfig *m_RobotConfig;

public:
    static CInterface *m_MenuGraphics;
    static bool LoadMenuGraphics(CBlockPar &bp);
    void CreateMenu(EMenuParent parent, int elements, int width, int x, int y, CIFaceElement *caller,
                    SMenuItemText *labels = NULL, DWORD color = 0);

    void SetSelectorPos(const float &x, const float &y, int pos);
    bool Selector() { return m_Selector != NULL; }
    CIFaceStatic *GetRamka() { return m_Ramka; }

    EMenuParent GetMenuParent() { return m_InterfaceParent; }

    void OnMenuItemPress();
    void CalcSelectedItem(bool set);

    void ResetMenu(bool canceled);

    CIFaceMenu();
    ~CIFaceMenu();
};

extern CIFaceMenu *g_PopupMenu;