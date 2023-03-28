// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "CIFaceElement.h"
#include "Interface.h"
#include "CConstructor.h"

class CIFaceImage;
class CIFaceStatic;
class CIFaceCounter;
class CMatrixHint;

#define HINT_OTSTUP     2
#define RADAR_RADIUS    85
#define IS_VISIBLE      255
#define IS_VISIBLEA     255
#define IS_NOT_VISIBLEA 0

#define CRITICAL_RAMKA 0xFFFF8C00
#define NORMAL_RAMKA   0xFFB6E68B

// Interface flags
#define INTERFACE_SLIDE_LEFT  SETBIT(0)
#define INTERFACE_SLIDE_RIGHT SETBIT(1)
//#define INTERFACE_SLIDE_UP SETBIT(2)
//#define INTERFACE_SLIDE_DOWN SETBIT(3)
#define SLIDE_MAX_SPEED  1
#define SLIDE_STEP_SIZE  0.01f
#define SLIDE_FUNC_PARAM 2.0f

// Interface-list flags
#define FACTORY_PANEL2_ACTIVE SETBIT(0)
#define AUTO_FROBOT_ON        SETBIT(1)
#define AUTO_PROTECT_ON       SETBIT(2)
#define AUTO_CAPTURE_ON       SETBIT(3)
#define FLYER_TRACK_DOWN      SETBIT(4)
#define ORDERING_MODE         SETBIT(5)
#define SINGLE_MODE           SETBIT(6)
#define MINIMAP_BUTTON_DOWN   SETBIT(7)
#define MINIMAP_ENABLE_DRAG   SETBIT(8)
#define POPUP_MENU_ACTIVE     SETBIT(9)
//#define TURRET_BUILD_MODE           SETBIT(7)
//#define FLYER_BUILD_MODE            SETBIT(8)
//
#define PREORDER_FIRE         SETBIT(16)
#define PREORDER_MOVE         SETBIT(17)
#define PREORDER_PATROL       SETBIT(18)
#define PREORDER_CAPTURE      SETBIT(19)
#define PREORDER_TRACKUP      SETBIT(20)
#define PREORDER_TRACKDOWN    SETBIT(21)
#define PREORDER_BOMB         SETBIT(22)
#define PREORDER_REPAIR       SETBIT(23)
#define PREORDER_BUILD_TURRET SETBIT(24)
#define PREORDER_BUILD_FLYER  SETBIT(25)
#define PREORDER_BUILD_REPAIR SETBIT(26)

#define DYNAMIC_TURRET             970
#define DYNAMIC_WARNING            960  // till 964
#define DYNAMIC_WEAPON_OFF_ID      951
#define DYNAMIC_WEAPON_ON_ID       950
#define ORDERS_GLOW_ID             850  // till 855
#define PERSONAL_ICON_ID           755
#define GROUP_SELECTOR_ID          655
#define GROUP_ICONS_ID             550  // till 558
#define GROUP_SELECTION_ID         450  // till 458
#define SUMM_PRICE_ID              355
#define ITEM_PRICE_ID              255
#define STACK_ICON                 100  // till 105
#define POPUP_SELECTOR_CATCHERS_ID 50
#define POPUP_REACTION_ELEMENT_ID  25

#define GROUP_SELECTOR_DEF_X 225
#define GROUP_SELECTOR_DEF_Y 49

#define IS_DYNAMIC_WARNING(x) (((x - DYNAMIC_WARNING) >= 0) && ((x - DYNAMIC_WARNING) < 4))
#define IS_PREORDERING        ((g_IFaceList->m_IfListFlags & 0xffff0000))
#define IS_ORDER_GLOW(x)      (((x - ORDERS_GLOW_ID) >= 0) && ((x - ORDERS_GLOW_ID) < 6))
#define IS_GROUP_ICON(x)      (((x - GROUP_ICONS_ID) >= 0) && ((x - GROUP_ICONS_ID) < 9))
#define IS_SELECTION(x)       (((x - GROUP_SELECTION_ID) >= 0) && ((x - GROUP_SELECTION_ID) < 9))
#define IS_STACK_ICON(x)      (((x - STACK_ICON) >= 0) && ((x - STACK_ICON) < 9))
#define IS_PREORDERING_NOSELECT                                                                            \
    (FLAG(g_IFaceList->m_IfListFlags, PREORDER_FIRE | PREORDER_MOVE | PREORDER_PATROL | PREORDER_CAPTURE | \
                                              PREORDER_TRACKUP | PREORDER_TRACKDOWN | PREORDER_BOMB |      \
                                              PREORDER_REPAIR))

#define IF_MAP_PANELI  65000
#define IF_RADAR_PNI   65001
#define IF_CALLHELL_ID 65535

#define MENU_HEAD_ITEMS         5
#define MENU_WEAPONNORM_ITEMS   9
#define MENU_WEAPONEXTERN_ITEMS 3
#define MENU_HULL_ITEMS         6
#define MENU_CHASSIS_ITEMS      5

#define NEW_LINE 0x0d
#define CARRET   0x0a

typedef void (*DialogButtonHandler)(void);
struct SSlide {
    float startX;
    float startY;
    float stopX;
    float stopY;
    float startLength;
    float step;
};

enum EHintButton {
    HINT_OK,
    HINT_CANCEL,
    HINT_EXIT,
    HINT_CANCEL_MENU,
    HINT_RESET,
    HINT_CONTINUE,
    HINT_SURRENDER,
    HINT_HELP,
    //.....
};

//////////////////////////////////////////////////
// root class
class CInterface : public CMain {
    int factory_res_income;
    int base_res_income;
    int btype;
    int prev_titan;
    int prev_energy;
    int prev_plasma;
    int prev_electro;

    int blazer_cnt;
    int keller_cnt;
    int terron_cnt;

    int pilon1;
    int pilon2;
    int pilon3;
    int pilon4;
    int pilon5;

    int pilon_ch;
    int pilon_he;
    int pilon_hu;

    float lives;
    float max_lives;
    float spd;
    float wght;
    std::wstring name;
    std::wstring item_label1;
    std::wstring item_label2;
    std::wstring rcname;

    ESelection cur_sel;

    int titan_summ;
    int electronics_summ;
    int energy_summ;
    int plasma_summ;

    int titan_unit;
    int electronics_unit;
    int energy_unit;
    int plasma_unit;

    int weight;
    int speed;
    int structure;
    int damage;

    int turmax;
    int turhave;

    int robots;
    int max_robots;

    DWORD titan_color;
    DWORD electronics_color;
    DWORD energy_color;
    DWORD plasm_color;

    DWORD titan_unit_color;
    DWORD electronics_unit_color;
    DWORD energy_unit_color;
    DWORD plasm_unit_color;

public:
    SSlide m_Slider;
    DWORD m_InterfaceFlags;
    int m_nTotalElements;
    BYTE m_VisibleAlpha;
    DWORD m_AlwaysOnTop;  // it is boolean var. used DWORD only align reasons

    int m_nId;
    std::wstring m_strName;
    float m_xPos, m_yPos, m_zPos;
    CIFaceElement *m_FirstElement, *m_LastElement;
    CIFaceImage *m_FirstImage, *m_LastImage;
    CInterface *m_PrevInterface, *m_NextInterface;

    void Init();
    bool Load(CBlockPar &bp, const wchar *name);

    bool AddElement(CIFaceElement *pElement);
    CIFaceElement *DelElement(CIFaceElement *pElement);  // returns next

    void SortElementsByZ();
    void Reset();

    CIFaceImage *FindImageByName(std::wstring name);
    CIFaceStatic *CreateStaticFromImage(float x, float y, float z, const CIFaceImage &image, bool fullsize = false);

    BYTE GetAlpha() { return m_VisibleAlpha; }
    void SetAlpha(BYTE alpha);
    void BeforeRender(void);
    void Render();

    bool FindElementByName(const std::wstring &name);

    static void CopyElements(CIFaceElement *el1, CIFaceElement *el2);

    void BeginSlide(float to_x, float to_y);
    void SlideStep();

    bool OnMouseMove(CPoint);
    void OnMouseLBUp();
    bool OnMouseLBDown();
    bool OnMouseRBDown();

    void MoveUp();
    void MoveDown();
    void MoveLeft();
    void MoveRight();

    void ReCalcElementsPos();

    void LogicTakt(int ms);

    CInterface();
    ~CInterface();

    static void StaticInit(void) { m_ClearRects = NULL; }

    static CBuf *m_ClearRects;
    static void ClearRects_Clear(void) { m_ClearRects->Clear(); };
    static void ClearRects_Add(const D3DRECT &rect) { m_ClearRects->Add<D3DRECT>(rect); }
    static D3DRECT *ClearRects_Get(void) { return (D3DRECT *)m_ClearRects->Get(); }
    static int ClearRects_GetCount(void) { return m_ClearRects->Len() / sizeof(D3DRECT); }
};
////////////////////////////////////////////////////
enum Focus {
    UNKNOWN = 0,
    INTERFACE = 1,

    Focus_FORCE_DWORD = 0x7FFFFFFF
};

class CIFaceList : public CMain {
    float m_MainX;  // IF_MAIN position
    float m_MainY;

public:
    int m_BaseX;
    int m_BaseY;

    int m_DynamicTY;
    int m_DynamicTX[10];

    int m_DWeaponX[MAX_WEAPON_CNT];
    int m_DWeaponY[MAX_WEAPON_CNT];

    int m_Pilons[5 * 6];  // 5 pilons max, 6 armors

    CMatrixHint *m_CurrentHint;
    std::wstring m_CurrentHintControlName;

    DWORD m_IfListFlags;
    Focus m_InFocus;
    CInterface *m_First, *m_Last;
    CInterface *m_FocusedInterface;
    CInterface *m_Hints;
    CIFaceElement *m_FocusedElement;

    CIFaceElement *m_Chassis[ROBOT_CHASSIS_CNT];
    CIFaceElement *m_ChassisPilon;

    CIFaceElement *m_Armor[ROBOT_ARMOR_CNT];
    CIFaceElement *m_ArmorPilon;

    CIFaceElement *m_Head[ROBOT_HEAD_CNT + 1];
    CIFaceElement *m_HeadPilon;

    CIFaceElement *m_Weapon[ROBOT_WEAPON_CNT + 1];
    CIFaceElement *m_WeaponPilon[5 /*define expected*/];

    CIFaceElement *m_Turrets[4 /*define expected*/];
    CIFaceElement *m_BuildCa;

    CIFaceCounter *m_RCountControl;

    int m_IFRadarPosX;
    int m_IFRadarPosY;

    void ResetBuildCaMode() {
        if (m_BuildCa && m_BuildCa->GetState() == IFACE_FOCUSED) {
            m_BuildCa->SetState(IFACE_NORMAL);
        }
        ResetOrderingMode();
    }
    void SetMainPos(const float &x, const float &y) {
        m_MainX = x;
        m_MainY = y;
    }
    float GetMainX() { return m_MainX; }
    float GetMainY() { return m_MainY; }

    void CreateWeaponDynamicStatics();
    void DeleteWeaponDynamicStatics();

    void CreateGroupSelection(CInterface *iface);
    void DeleteGroupSelection();

    void CreateItemPrice(int *res);
    void DeleteItemPrice();

    void CreateSummPrice(int multiplier = 1);
    void DeleteSummPrice();

    void CreateGroupIcons();
    void DeleteGroupIcons();

    void DeleteProgressBars(CMatrixMapStatic *from);

    void CreatePersonal();
    void DeletePersonal();

    void ResetOrderingMode();

    void CreateOrdersGlow(CInterface *iface);

    void CreateStackIcon(int num, CMatrixBuilding *base, CMatrixMapStatic *object);
    void DeleteStackIcon(int num, CMatrixBuilding *base);
    void MoveStackIcons();

    void ShowInterface();
    void BeforeRender();
    void Render();
    bool OnMouseMove(CPoint);
    void OnMouseLBUp();
    bool OnMouseLBDown();
    bool OnMouseRBDown();
    void LogicTakt(int ms);

    void CreateElementRamka(CIFaceElement *element, DWORD color);
    void SlideFocusedInterfaceRight();
    void SlideFocusedInterfaceLeft();

    void ConstructorButtonsInit();
    void WeaponPilonsInit();

    void CreateHintButton(int x, int y, EHintButton type, DialogButtonHandler handler);
    void HideHintButtons();
    void HideHintButton(EHintButton butt);
    void DisableMainMenuButton(EHintButton butt);
    void EnableMainMenuButton(EHintButton butt);
    void HintButtonId2Name(EHintButton butt, std::wstring &name);
    void PressHintButton(EHintButton butt);

    bool CorrectCoordinates(int screen_width, int screen_height, int &posx, int &posy, int width, int height,
                            const std::wstring &name);
    void AddHintReplacements(const std::wstring &element_name);
    bool CheckShowHintLogic(const std::wstring &element_name);
    void LiveRobot(void);
    void EnterRobot(bool pos = false);
    void BeginBuildTurret(int no);

    void CreateDynamicTurrets(CMatrixBuilding *building);
    void DeleteDynamicTurrets();

    void __stdcall PlayerAction(void *object);
    void __stdcall JumpToBuilding(void *element);
    void __stdcall JumpToRobot(void *element);

    CIFaceList();
    ~CIFaceList();
};

#ifdef _DEBUG
void t_pause(void);
void t_unpause(void);
#endif
