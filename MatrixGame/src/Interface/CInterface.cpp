// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <algorithm>
#include <array> // for std::begin, std::end

#include "CIFaceButton.h"
#include "CIFaceStatic.h"
#include "CInterface.h"
#include "CIFaceImage.h"
#include "../MatrixObjectBuilding.hpp"
#include "../MatrixMinimap.hpp"
#include "../MatrixGameDll.hpp"
#include "../MatrixFlyer.hpp"
#include "../MatrixObjectCannon.hpp"
#include "CAnimation.h"
#include "../MatrixSoundManager.hpp"
#include "CIFaceMenu.h"
#include "CCounter.h"
#include "CHistory.h"
#include "MatrixHint.hpp"
#include "../MatrixFormGame.hpp"

CIFaceList *g_IFaceList = NULL;

SMenuItemText *g_PopupHead;
SMenuItemText *g_PopupWeaponNormal;
SMenuItemText *g_PopupWeaponExtern;
SMenuItemText *g_PopupHull;
SMenuItemText *g_PopupChassis;

// Constructor destructor
CInterface::CInterface()
  : name{}, m_strName{}, item_label1{}, item_label2{}, rcname{}
{
    factory_res_income = -1;
    base_res_income = -1;
    btype = -1;
    prev_titan = -1;
    prev_energy = -1;
    prev_plasma = -1;
    prev_electro = -1;

    blazer_cnt = -1;
    keller_cnt = -1;
    terron_cnt = -1;

    pilon1 = -1;
    pilon2 = -1;
    pilon3 = -1;
    pilon4 = -1;
    pilon5 = -1;

    pilon_ch = -1;
    pilon_he = -1;
    pilon_hu = -1;

    lives = -1;
    max_lives = -1;

    cur_sel = ESelection(-1);

    titan_summ = -1;
    electronics_summ = -1;
    energy_summ = -1;
    plasma_summ = -1;

    titan_unit = -1;
    electronics_unit = -1;
    energy_unit = -1;
    plasma_unit = -1;

    weight = -1;
    speed = -1;
    structure = -1;
    damage = -1;

    wght = -1;
    spd = -1;

    turmax = -1;
    turhave = -1;

    robots = -1;
    max_robots = -1;

    titan_color = 0xFFF6c000;
    electronics_color = 0xFFF6c000;
    energy_color = 0xFFF6c000;
    plasm_color = 0xFFF6c000;

    titan_unit_color = 0xFFF6c000;
    electronics_unit_color = 0xFFF6c000;
    energy_unit_color = 0xFFF6c000;
    plasm_unit_color = 0xFFF6c000;

    //////////////////

    m_InterfaceFlags = 0;
    m_VisibleAlpha = IS_NOT_VISIBLEA;
    m_xPos = 0;
    m_yPos = 0;
    m_zPos = 0;

    m_FirstElement = NULL;
    m_LastElement = NULL;
    m_FirstImage = NULL;
    m_LastImage = NULL;

    //	m_Vertices = NULL;
    m_PrevInterface = NULL;
    m_NextInterface = NULL;

    m_nTotalElements = 0;

    m_nId = 0;
    m_AlwaysOnTop = FALSE;

    ZeroMemory(&m_Slider, sizeof(SSlide));
}

CInterface::~CInterface() {
    DTRACE();
    CIFaceImage *images = m_FirstImage;
    ASSERT(g_MatrixHeap);

    // List dest

    CIFaceElement *ptmpElement = m_FirstElement;
    while (m_FirstElement)
        DelElement(m_FirstElement);

    while (images != NULL) {
        if (images->m_NextImage)
            images = images->m_NextImage;
        else {
            HDelete(CIFaceImage, images, g_MatrixHeap);
            images = NULL;
            m_FirstImage = NULL;
            m_LastImage = NULL;
        }

        if (images)
            HDelete(CIFaceImage, images->m_PrevImage, g_MatrixHeap);
    }
}

// Main routines
bool CInterface::Load(CBlockPar &bp, const wchar *name) {
    DTRACE();
    bool need2save = false;
    int nElementNum = 0;
    std::wstring tmpStr;
    CBlockPar *pbp1 = NULL, *pbp2 = NULL;

    // Loading interface file
    pbp1 = bp.BlockGet(name);

    // Interface members initialisation
    m_strName = name;
    m_nTotalElements = pbp1->ParGet(L"eTotal").GetInt();
    m_nId = pbp1->ParGet(L"id").GetInt();
    m_xPos = (float)pbp1->ParGet(L"xPos").GetDouble();
    m_yPos = (float)pbp1->ParGet(L"yPos").GetDouble();
    m_zPos = (float)pbp1->ParGet(L"zPos").GetDouble();

    if (m_xPos != 0) {
        int width = 1024 - Float2Int(m_xPos);
        m_xPos = float(g_ScreenX - width);
    }
    if (m_yPos != 0) {
        int height = 768 - Float2Int(m_yPos);
        m_yPos = float(g_ScreenY - height);
    }

    if (m_strName == IF_RADAR) {
        g_IFaceList->m_IFRadarPosX = Float2Int(m_xPos);
        g_IFaceList->m_IFRadarPosY = Float2Int(m_yPos);
    }

    std::wstring labels_text;
    CBlockPar *labels_file = NULL;

    labels_file = pbp1->BlockGetNE(L"LabelsText");
    if (labels_file) {
        labels_file = labels_file->BlockGetNE(name);
    }

    if (pbp1->ParGet(L"OnTop").GetInt()) {
        m_AlwaysOnTop = TRUE;
        m_VisibleAlpha = 255;
    }
    else
        m_AlwaysOnTop = FALSE;

    int Const = 0;
    Const = pbp1->ParGet(L"ConstPresent").GetInt();

    if (Const) {
        g_MatrixMap->GetPlayerSide()->m_Constructor->SetRenderProps(
                (float)pbp1->ParGet(L"ConstX").GetDouble() + m_xPos, (float)pbp1->ParGet(L"ConstY").GetDouble() + m_yPos,
                pbp1->ParGet(L"ConstWidth").GetInt(), pbp1->ParGet(L"ConstHeight").GetInt());
    }

    CIFaceElement *if_elem = NULL;
    for (int nC = 0; nC < m_nTotalElements; nC++) {
        if_elem = NULL;
        pbp2 = pbp1->BlockGet(nC);
        tmpStr = pbp1->BlockGetName(nC);

        if (tmpStr == L"Button") {
            CIFaceButton *pButton = HNew(g_MatrixHeap) CIFaceButton;
            if_elem = (CIFaceElement *)pButton;

            pButton->m_nId = pbp2->ParGet(L"id").GetInt();
            pButton->m_strName = pbp2->ParGet(L"Name");

            auto hint_par = pbp2->ParGetNE(L"Hint");
            if (!hint_par.empty()) {
                pButton->m_Hint.HintTemplate = hint_par.GetStrPar(0, L",");
                pButton->m_Hint.x = hint_par.GetStrPar(1, L",").GetInt();
                pButton->m_Hint.y = hint_par.GetStrPar(2, L",").GetInt();
            }

            pButton->m_Type = (IFaceElementType)pbp2->ParGet(L"type").GetInt();
            pButton->m_nGroup = pbp2->ParGet(L"group").GetInt();
            pButton->m_xPos = (float)pbp2->ParGet(L"xPos").GetDouble();
            pButton->m_yPos = (float)pbp2->ParGet(L"yPos").GetDouble();
            pButton->m_zPos = (float)pbp2->ParGet(L"zPos").GetDouble();
            pButton->m_xSize = (float)pbp2->ParGet(L"xSize").GetDouble();
            pButton->m_ySize = (float)pbp2->ParGet(L"ySize").GetDouble();
            pButton->m_Param1 = (float)pbp2->ParGet(L"Param1").GetDouble();
            pButton->m_Param2 = (float)pbp2->ParGet(L"Param2").GetDouble();
            pButton->m_DefState = (IFaceElementState)pbp2->ParGet(L"dState").GetInt();

            // FSET(pButton->m_Action, m_Constructor, CConstructor::OperateUnit);

            if (Const) {
                if ((pButton->m_Param1 != 0 && pButton->m_Param2 != 0) || pButton->m_strName == IF_BASE_PILON_HULL ||
                    pButton->m_strName == IF_BASE_PILON_CHASSIS || pButton->m_strName == IF_BASE_PILON_HEAD ||
                    pButton->m_strName == IF_BASE_PILON1 || pButton->m_strName == IF_BASE_PILON2 ||
                    pButton->m_strName == IF_BASE_PILON3 || pButton->m_strName == IF_BASE_PILON4 ||
                    pButton->m_strName == IF_BASE_PILON5 || pButton->m_strName == IF_BASE_HEAD_EMPTY ||
                    pButton->m_strName == IF_BASE_WEAPON_EMPTY) {
                    FSET(ON_UN_PRESS, pButton, g_MatrixMap->GetPlayerSide()->m_Constructor,
                         CConstructor::RemoteOperateUnit);
                    FSET(ON_FOCUS, pButton, g_MatrixMap->GetPlayerSide()->m_ConstructPanel,
                         CConstructorPanel::RemoteFocusElement);
                    FSET(ON_UN_FOCUS, pButton, g_MatrixMap->GetPlayerSide()->m_ConstructPanel,
                         CConstructorPanel::RemoteUnFocusElement);
                    pButton->m_nId = POPUP_REACTION_ELEMENT_ID;
                }
                else if (pButton->m_strName == IF_BASE_CONST_BUILD) {
                    FSET(ON_UN_PRESS, pButton, g_MatrixMap->GetPlayerSide()->m_Constructor,
                         CConstructor::RemoteBuild);
                }
                else if (pButton->m_strName == IF_BASE_CONST_CANCEL) {
                    FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
                }
            }

            if (pButton->m_strName == IF_MAP_ZOOM_IN) {
                FSET(ON_UN_PRESS, pButton, &g_MatrixMap->m_Minimap, CMinimap::ButtonZoomIn);
            }
            else if (pButton->m_strName == IF_MAP_ZOOM_OUT) {
                FSET(ON_UN_PRESS, pButton, &g_MatrixMap->m_Minimap, CMinimap::ButtonZoomOut);
            }
            else if (pButton->m_strName == IF_BUILD_RO) {
                FSET(ON_UN_PRESS, pButton, g_MatrixMap->GetPlayerSide(), CMatrixSideUnit::PlayerAction);
            }
            else if (pButton->m_strName == IF_AORDER_FROBOT_ON) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_AORDER_FROBOT_OFF) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_AORDER_PROTECT_ON) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_AORDER_PROTECT_OFF) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_AORDER_CAPTURE_ON) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_AORDER_CAPTURE_OFF) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_ORDER_FIRE) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_ORDER_CAPTURE) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_ORDER_PATROL) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_ORDER_MOVE) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_ORDER_REPAIR) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_ORDER_BOMB) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_ORDER_CANCEL) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_ORDER_STOP) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_ENTER_ROBOT) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_LEAVE_ROBOT) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_MAIN_SELFBOMB) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_BUILD_CA) {
                g_IFaceList->m_BuildCa = pButton;
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_CALL_FROM_HELL) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_BUILD_HE) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_BUILD_REPAIR) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_SHOWROBOTS_BUTT) {
                FSET(ON_UN_PRESS, pButton, &g_MatrixMap->m_Minimap, CMinimap::ShowPlayerBots);
            }
            else if (pButton->m_strName == IF_BUILD_TUR1) {
                g_IFaceList->m_Turrets[0] = pButton;
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_BUILD_TUR2) {
                g_IFaceList->m_Turrets[1] = pButton;
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_BUILD_TUR3) {
                g_IFaceList->m_Turrets[2] = pButton;
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_BUILD_TUR4) {
                g_IFaceList->m_Turrets[3] = pButton;
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_MAIN_MENU_BUTTON) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList, CIFaceList::PlayerAction);
            }
            else if (pButton->m_strName == IF_BASE_CHASSIS3) {
                g_IFaceList->m_Chassis[0] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_CHASSIS2) {
                g_IFaceList->m_Chassis[1] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_CHASSIS1) {
                g_IFaceList->m_Chassis[2] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_CHASSIS4) {
                g_IFaceList->m_Chassis[3] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_CHASSIS5) {
                g_IFaceList->m_Chassis[4] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_HULL1) {
                g_IFaceList->m_Armor[Float2Int(pButton->m_Param2 - 1)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_HULL2) {
                g_IFaceList->m_Armor[Float2Int(pButton->m_Param2 - 1)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_HULL3) {
                g_IFaceList->m_Armor[Float2Int(pButton->m_Param2 - 1)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_HULL4) {
                g_IFaceList->m_Armor[Float2Int(pButton->m_Param2 - 1)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_HULL5) {
                g_IFaceList->m_Armor[Float2Int(pButton->m_Param2 - 1)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_HULL6) {
                g_IFaceList->m_Armor[Float2Int(pButton->m_Param2 - 1)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_HEAD1) {
                g_IFaceList->m_Head[Float2Int(pButton->m_Param2)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_HEAD2) {
                g_IFaceList->m_Head[Float2Int(pButton->m_Param2)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_HEAD3) {
                g_IFaceList->m_Head[Float2Int(pButton->m_Param2)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_HEAD4) {
                g_IFaceList->m_Head[Float2Int(pButton->m_Param2)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_WEAPON1) {
                g_IFaceList->m_Weapon[Float2Int(pButton->m_Param2)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_WEAPON2) {
                g_IFaceList->m_Weapon[Float2Int(pButton->m_Param2)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_WEAPON3) {
                g_IFaceList->m_Weapon[Float2Int(pButton->m_Param2)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_WEAPON4) {
                g_IFaceList->m_Weapon[Float2Int(pButton->m_Param2)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_WEAPON5) {
                g_IFaceList->m_Weapon[Float2Int(pButton->m_Param2)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_WEAPON6) {
                g_IFaceList->m_Weapon[Float2Int(pButton->m_Param2)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_WEAPON7) {
                g_IFaceList->m_Weapon[Float2Int(pButton->m_Param2)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_WEAPON8) {
                g_IFaceList->m_Weapon[Float2Int(pButton->m_Param2)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_WEAPON9) {
                g_IFaceList->m_Weapon[Float2Int(pButton->m_Param2)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_WEAPON10) {
                g_IFaceList->m_Weapon[Float2Int(pButton->m_Param2)] = pButton;
            }
            else if (pButton->m_strName == IF_BASE_UP) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList->m_RCountControl, CIFaceCounter::Up);
                g_IFaceList->m_RCountControl->SetButtonUp(pButton);
            }
            else if (pButton->m_strName == IF_BASE_DOWN) {
                FSET(ON_UN_PRESS, pButton, g_IFaceList->m_RCountControl, CIFaceCounter::Down);
                g_IFaceList->m_RCountControl->SetButtonDown(pButton);
            }
            else if (pButton->m_strName == IF_BASE_HISTORY_RIGHT) {
                FSET(ON_UN_PRESS, pButton, g_ConfigHistory, CHistory::NextConfig);
            }
            else if (pButton->m_strName == IF_BASE_HISTORY_LEFT) {
                FSET(ON_UN_PRESS, pButton, g_ConfigHistory, CHistory::PrevConfig);
            }

            if (pButton->m_strName == IF_BASE_PILON_CHASSIS) {
                g_IFaceList->m_ChassisPilon = pButton;
            }

            if (pButton->m_strName == IF_BASE_PILON_HULL) {
                g_IFaceList->m_ArmorPilon = pButton;
            }

            if (pButton->m_strName == IF_BASE_PILON_HEAD) {
                g_IFaceList->m_HeadPilon = pButton;
            }

            if (pButton->m_strName == IF_BASE_HEAD_EMPTY) {
                g_IFaceList->m_Head[0] = pButton;
            }

            if (pButton->m_strName == IF_BASE_WEAPON_EMPTY) {
                g_IFaceList->m_Weapon[0] = pButton;
            }

            if (pButton->m_strName == IF_BASE_PILON1) {
                g_IFaceList->m_WeaponPilon[0] = pButton;
            }
            if (pButton->m_strName == IF_BASE_PILON2) {
                g_IFaceList->m_WeaponPilon[1] = pButton;
            }
            if (pButton->m_strName == IF_BASE_PILON3) {
                g_IFaceList->m_WeaponPilon[2] = pButton;
            }
            if (pButton->m_strName == IF_BASE_PILON4) {
                g_IFaceList->m_WeaponPilon[3] = pButton;
            }
            if (pButton->m_strName == IF_BASE_PILON5) {
                g_IFaceList->m_WeaponPilon[4] = pButton;
            }

            pButton->SetStateImage(
                    IFACE_NORMAL, (CTextureManaged *)g_Cache->Get(cc_TextureManaged, pbp2->ParGet(L"sNormal").c_str()),
                    (float)pbp2->ParGet(L"sNormalX").GetDouble(), (float)pbp2->ParGet(L"sNormalY").GetDouble(),
                    (float)pbp2->ParGet(L"sNormalWidth").GetDouble(), (float)pbp2->ParGet(L"sNormalHeight").GetDouble());

            pButton->SetStateImage(
                    IFACE_FOCUSED, (CTextureManaged *)g_Cache->Get(cc_TextureManaged, pbp2->ParGet(L"sFocused").c_str()),
                    (float)pbp2->ParGet(L"sFocusedX").GetDouble(), (float)pbp2->ParGet(L"sFocusedY").GetDouble(),
                    (float)pbp2->ParGet(L"sFocusedWidth").GetDouble(), (float)pbp2->ParGet(L"sFocusedHeight").GetDouble());

            pButton->SetStateImage(
                    IFACE_PRESSED, (CTextureManaged *)g_Cache->Get(cc_TextureManaged, pbp2->ParGet(L"sPressed").c_str()),
                    (float)pbp2->ParGet(L"sPressedX").GetDouble(), (float)pbp2->ParGet(L"sPressedY").GetDouble(),
                    (float)pbp2->ParGet(L"sPressedWidth").GetDouble(), (float)pbp2->ParGet(L"sPressedHeight").GetDouble());

            pButton->SetStateImage(
                    IFACE_DISABLED, (CTextureManaged *)g_Cache->Get(cc_TextureManaged, pbp2->ParGet(L"sDisabled").c_str()),
                    (float)pbp2->ParGet(L"sDisabledX").GetDouble(), (float)pbp2->ParGet(L"sDisabledY").GetDouble(),
                    (float)pbp2->ParGet(L"sDisabledWidth").GetDouble(), (float)pbp2->ParGet(L"sDisabledHeight").GetDouble());

            if (pButton->m_Type == IFACE_CHECK_BUTTON || pButton->m_Type == IFACE_CHECK_BUTTON_SPECIAL ||
                pButton->m_Type == IFACE_CHECK_PUSH_BUTTON) {
                pButton->SetStateImage(
                        IFACE_PRESSED_UNFOCUSED,
                        (CTextureManaged *)g_Cache->Get(cc_TextureManaged, pbp2->ParGet(L"sPressedUnFocused").c_str()),
                        (float)pbp2->ParGet(L"sPressedUnFocusedX").GetDouble(),
                        (float)pbp2->ParGet(L"sPressedUnFocusedY").GetDouble(),
                        (float)pbp2->ParGet(L"sPressedUnFocusedWidth").GetDouble(),
                        (float)pbp2->ParGet(L"sPressedUnFocusedHeight").GetDouble());
            }

            // Animation
            CBlockPar *animation = NULL;
            animation = pbp2->BlockGetNE(L"Animation");
            if (animation) {
                auto par = animation->ParGet(L"Frames");
                if (par.length()) {
                    int frames_cnt = par.GetStrPar(0, L",").GetInt();
                    int period = par.GetStrPar(1, L",").GetInt();
                    int width = par.GetStrPar(2, L",").GetInt();
                    int height = par.GetStrPar(3, L",").GetInt();

                    pButton->m_Animation = HNew(g_MatrixHeap) CAnimation(frames_cnt, period);
                    SFrame frame;
                    ZeroMemory(&frame, sizeof(SFrame));
                    // frame.name = pButton->m_strName;
                    frame.height = (float)height;
                    frame.width = (float)width;
                    frame.pos_x = pButton->m_xPos + 1;
                    frame.pos_y = pButton->m_yPos;
                    frame.pos_z = 0 /*pButton->m_zPos*/;
                    frame.tex = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, pbp2->ParGet(L"sNormal").c_str());
                    frame.tex_width = (float)pbp2->ParGet(L"sNormalWidth").GetDouble();
                    frame.tex_height = (float)pbp2->ParGet(L"sNormalHeight").GetDouble();
                    frame.ipos_x = m_xPos;
                    frame.ipos_y = m_yPos;

                    for (int i = 0; i < frames_cnt * 2; i += 2) {
                        int x = par.GetStrPar(3 + 1 + i, L",").GetInt();
                        int y = par.GetStrPar(3 + 1 + i + 1, L",").GetInt();
                        frame.tex_pos_x = (float)x;
                        frame.tex_pos_y = (float)y;
                        // Load Next Frame here
                        pButton->m_Animation->LoadNextFrame(&frame);
                    }
                }
            }

            //

            // Initialising button geometry
            pButton->ElementGeomInit((void *)pButton);
            pButton->SetState(pButton->m_DefState);

            // Add Element(Button) to the list, Generate polygons and add them to pVertices
            AddElement(pButton);

            nElementNum++;
        }
        else if (tmpStr == L"Static") {
            CIFaceStatic *pStatic = HNew(g_MatrixHeap) CIFaceStatic;
            if_elem = (CIFaceElement *)pStatic;

            pStatic->m_strName = pbp2->ParGet(L"Name");

            auto hint_par = pbp2->ParGetNE(L"Hint");
            if (hint_par != L"") {
                pStatic->m_Hint.HintTemplate = hint_par.GetStrPar(0, L",");
                pStatic->m_Hint.x = hint_par.GetStrPar(1, L",").GetInt();
                pStatic->m_Hint.y = hint_par.GetStrPar(2, L",").GetInt();
            }

            pStatic->m_xPos = (float)pbp2->ParGet(L"xPos").GetDouble();
            pStatic->m_yPos = (float)pbp2->ParGet(L"yPos").GetDouble();
            pStatic->m_zPos = (float)pbp2->ParGet(L"zPos").GetDouble();
            pStatic->m_xSize = (float)pbp2->ParGet(L"xSize").GetDouble();
            pStatic->m_ySize = (float)pbp2->ParGet(L"ySize").GetDouble();
            pStatic->m_DefState = (IFaceElementState)pbp2->ParGet(L"dState").GetInt();

            // pStatic->m_Hint. = pbp2->ParNE(L"Hint");

            if (m_strName == IF_TOP && pStatic->m_strName == IF_TOP_PANEL1) {
                g_MatrixMap->m_DI.SetStartPos(CPoint(10, Float2Int(m_yPos + pStatic->m_yPos + pStatic->m_ySize)));
            }
            else if (pStatic->m_strName == IF_MAP_PANEL) {
                g_MatrixMap->m_Minimap.SetOutParams(Float2Int(m_xPos) + 13, Float2Int(m_yPos) + 51, 145, 145,
                                                    D3DXVECTOR2(g_MatrixMap->m_Size.x * GLOBAL_SCALE * 0.5f,
                                                                g_MatrixMap->m_Size.y * GLOBAL_SCALE * 0.5f),
                                                    1.0f, 0xFFFFFFFF);

                // FSET(ON_UN_PRESS,pStatic, &g_MatrixMap->m_Minimap, CMinimap::ButtonClick);
                pStatic->m_iParam = IF_MAP_PANELI;
            }
            else if (pStatic->m_strName == IF_RADAR_PN) {
                pStatic->m_iParam = IF_RADAR_PNI;
            }
            else if (pStatic->m_strName == IF_BASE_ZERO) {
                g_IFaceList->m_RCountControl->SetImage(0, pStatic);
            }
            else if (pStatic->m_strName == IF_BASE_ONE) {
                g_IFaceList->m_RCountControl->SetImage(1, pStatic);
            }
            else if (pStatic->m_strName == IF_BASE_TWO) {
                g_IFaceList->m_RCountControl->SetImage(2, pStatic);
            }
            else if (pStatic->m_strName == IF_BASE_THREE) {
                g_IFaceList->m_RCountControl->SetImage(3, pStatic);
            }
            else if (pStatic->m_strName == IF_BASE_FOUR) {
                g_IFaceList->m_RCountControl->SetImage(4, pStatic);
            }
            else if (pStatic->m_strName == IF_BASE_FIVE) {
                g_IFaceList->m_RCountControl->SetImage(5, pStatic);
            }
            else if (pStatic->m_strName == IF_BASE_SIX) {
                g_IFaceList->m_RCountControl->SetImage(6, pStatic);
            }
            else if (pStatic->m_strName == IF_TITAN_PLANT) {
                FSET(ON_PRESS, pStatic, g_IFaceList, CIFaceList::JumpToBuilding);
            }
            else if (pStatic->m_strName == IF_PLASMA_PLANT) {
                FSET(ON_PRESS, pStatic, g_IFaceList, CIFaceList::JumpToBuilding);
            }
            else if (pStatic->m_strName == IF_ELECTRO_PLANT) {
                FSET(ON_PRESS, pStatic, g_IFaceList, CIFaceList::JumpToBuilding);
            }
            else if (pStatic->m_strName == IF_ENERGY_PLANT) {
                FSET(ON_PRESS, pStatic, g_IFaceList, CIFaceList::JumpToBuilding);
            }
            else if (pStatic->m_strName == IF_BASE_PLANT) {
                FSET(ON_PRESS, pStatic, g_IFaceList, CIFaceList::JumpToBuilding);
            }

            pStatic->SetStateImage(
                    IFACE_NORMAL, (CTextureManaged *)g_Cache->Get(cc_TextureManaged, pbp2->ParGet(L"sNormal").c_str()),
                    (float)pbp2->ParGet(L"sNormalX").GetDouble(), (float)pbp2->ParGet(L"sNormalY").GetDouble(),
                    (float)pbp2->ParGet(L"sNormalWidth").GetDouble(), (float)pbp2->ParGet(L"sNormalHeight").GetDouble());

            pStatic->ElementGeomInit((void *)pStatic);

            // only Labels free statics have ClearRect
            if (pbp2->BlockGetNE(IF_LABELS) == NULL) {
                if (pbp2->ParCount(L"ClearRect") == 0) {
                    // pStatic->GenerateClearRect();

                    // std::wstring   rect(g_CacheHeap);
                    // rect = pStatic->m_ClearRect.left; rect += L",";
                    // rect += pStatic->m_ClearRect.top; rect += L",";
                    // rect += pStatic->m_ClearRect.right; rect += L",";
                    // rect += pStatic->m_ClearRect.bottom;

                    // pbp2->ParAdd(L"ClearRect", rect);

                    // need2save = true;
                }
                else {
                    pStatic->SetClearRect();

                    auto rect(pbp2->ParGet(L"ClearRect"));

                    // element position relative
                    pStatic->m_ClearRect.left = rect.GetStrPar(0, L",").GetInt();
                    pStatic->m_ClearRect.top = rect.GetStrPar(1, L",").GetInt();
                    pStatic->m_ClearRect.right = rect.GetStrPar(2, L",").GetInt();
                    pStatic->m_ClearRect.bottom = rect.GetStrPar(3, L",").GetInt();
                }
            }

            AddElement(pStatic);
            nElementNum++;
        }
        else if (tmpStr == L"Image") {
            CIFaceImage *image = HNew(g_MatrixHeap) CIFaceImage;
            image->m_strName = pbp2->ParGet(L"Name");

            image->m_Image = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, pbp2->ParGet(L"TextureFile").c_str());

            image->m_xTexPos = (float)pbp2->ParGet(L"TexPosX").GetDouble();
            image->m_yTexPos = (float)pbp2->ParGet(L"TexPosY").GetDouble();
            image->m_TexWidth = (float)pbp2->ParGet(L"TextureWidth").GetDouble();
            image->m_TexHeight = (float)pbp2->ParGet(L"TextureHeight").GetDouble();
            image->m_Width = (float)pbp2->ParGet(L"Width").GetDouble();
            image->m_Height = (float)pbp2->ParGet(L"Height").GetDouble();

            LIST_ADD(image, m_FirstImage, m_LastImage, m_PrevImage, m_NextImage);
            nElementNum++;

            if (image->m_strName == IF_GROUP_GLOW) {
                CIFaceStatic *st = CreateStaticFromImage(0, 0, 0, *image);
                st->m_nId = GROUP_SELECTOR_ID;
            }
        }
        if (labels_file && if_elem) {
            CBlockPar *labels = NULL;
            labels = pbp2->BlockGetNE(IF_LABELS);
            if (labels) {
                int labels_cnt = labels->BlockCount();
                for (int bl_cnt = 0; bl_cnt < labels_cnt; bl_cnt++) {
                    if (labels->BlockGetName(bl_cnt).length()) {
                        CBlockPar *label_block = labels->BlockGet(bl_cnt);
                        auto par = label_block->ParGet(IF_LABEL_PARAMS);
                        if (par.length()) {
                            int x = par.GetStrPar(0, L",").GetInt();
                            int y = par.GetStrPar(1, L",").GetInt();

                            int sme_x = par.GetStrPar(2, L",").GetInt();
                            int sme_y = par.GetStrPar(3, L",").GetInt();

                            int align_x = par.GetStrPar(4, L",").GetInt();
                            int align_y = par.GetStrPar(5, L",").GetInt();

                            int perenos = par.GetStrPar(6, L",").GetInt();

                            int clip_sx = par.GetStrPar(7, L",").GetInt();
                            int clip_sy = par.GetStrPar(8, L",").GetInt();
                            int clip_ex = par.GetStrPar(9, L",").GetInt();
                            int clip_ey = par.GetStrPar(10, L",").GetInt();

                            const std::wstring &state = label_block->ParGet(L"State");
                            const std::wstring &font = label_block->ParGet(L"Font");

                            const auto color_par = label_block->ParGet(L"Color");

                            DWORD color = 0;

                            color |= color_par.GetStrPar(0, L",").GetInt() << 24;
                            color |= color_par.GetStrPar(1, L",").GetInt() << 16;
                            color |= color_par.GetStrPar(2, L",").GetInt() << 8;
                            color |= color_par.GetStrPar(3, L",").GetInt();

                            IFaceElementState st;
                            if (state == IF_STATE_NORMAL) {
                                st = IFACE_NORMAL;
                            }
                            else if (state == IF_STATE_FOCUSED) {
                                st = IFACE_FOCUSED;
                            }
                            else if (state == IF_STATE_PRESSED) {
                                st = IFACE_PRESSED;
                            }
                            else if (state == IF_STATE_DISABLED) {
                                st = IFACE_DISABLED;
                            }
                            else if (state == IF_STATE_PRESSED_UNFOCUSED) {
                                st = IFACE_PRESSED_UNFOCUSED;
                            }
                            if (labels->BlockGetName(bl_cnt) == IF_STATE_STATIC_LABEL) {
                                std::wstring t_code = if_elem->m_strName;
                                t_code += L"_" + state;
                                std::wstring text = labels_file->ParGet(t_code);

                                if (t_code == L"iw1text_sNormal") {
                                    g_PopupWeaponNormal[1].text = text;
                                }
                                else if (t_code == L"iw2text_sNormal") {
                                    g_PopupWeaponNormal[2].text = text;
                                }
                                else if (t_code == L"iw3text_sNormal") {
                                    g_PopupWeaponNormal[3].text = text;
                                }
                                else if (t_code == L"iw4text_sNormal") {
                                    g_PopupWeaponNormal[4].text = text;
                                }
                                else if (t_code == L"iw5text_sNormal") {
                                    // iw5text_sNormal=мортира
                                    g_PopupWeaponExtern[1].text = text;
                                }
                                else if (t_code == L"iw6text_sNormal") {
                                    g_PopupWeaponNormal[5].text = text;
                                }
                                else if (t_code == L"iw7text_sNormal") {
                                    // iw7text_sNormal=бомба
                                    g_PopupWeaponExtern[2].text = text;
                                }
                                else if (t_code == L"iw8text_sNormal") {
                                    g_PopupWeaponNormal[6].text = text;
                                }
                                else if (t_code == L"iw9text_sNormal") {
                                    g_PopupWeaponNormal[7].text = text;
                                }
                                else if (t_code == L"iw10text_sNormal") {
                                    g_PopupWeaponNormal[8].text = text;
                                }
                                else if (t_code == L"ihu6text_sNormal") {
                                    g_PopupHull[0].text = text;
                                }
                                else if (t_code == L"ihu1text_sNormal") {
                                    g_PopupHull[1].text = text;
                                }
                                else if (t_code == L"ihu2text_sNormal") {
                                    g_PopupHull[2].text = text;
                                }
                                else if (t_code == L"ihu3text_sNormal") {
                                    g_PopupHull[3].text = text;
                                }
                                else if (t_code == L"ihu4text_sNormal") {
                                    g_PopupHull[4].text = text;
                                }
                                else if (t_code == L"ihu5text_sNormal") {
                                    g_PopupHull[5].text = text;
                                }
                                else if (t_code == L"ihe1text_sNormal") {
                                    g_PopupHead[1].text = text;
                                }
                                else if (t_code == L"ihe2text_sNormal") {
                                    g_PopupHead[2].text = text;
                                }
                                else if (t_code == L"ihe3text_sNormal") {
                                    g_PopupHead[3].text = text;
                                }
                                else if (t_code == L"ihe4text_sNormal") {
                                    g_PopupHead[4].text = text;
                                }
                                else if (t_code == L"ihe5text_sNormal") {
                                    g_PopupHead[5].text = text;
                                }
                                else if (t_code == L"ihe6text_sNormal") {
                                    g_PopupHead[6].text = text;
                                }
                                else if (t_code == L"ihe7text_sNormal") {
                                    g_PopupHead[7].text = text;
                                }
                                else if (t_code == L"ich1text_sNormal") {
                                    g_PopupChassis[0].text = text;
                                }
                                else if (t_code == L"ich2text_sNormal") {
                                    g_PopupChassis[1].text = text;
                                }
                                else if (t_code == L"ich3text_sNormal") {
                                    g_PopupChassis[2].text = text;
                                }
                                else if (t_code == L"ich4text_sNormal") {
                                    g_PopupChassis[3].text = text;
                                }
                                else if (t_code == L"ich5text_sNormal") {
                                    g_PopupChassis[4].text = text;
                                }
                                if (if_elem->m_strName == IF_MAIN_MENU_BUTTON ||
                                    if_elem->m_strName == IF_BASE_CONST_BUILD ||
                                    if_elem->m_strName == IF_BASE_CONST_CANCEL ||
                                    if_elem->m_strName == IF_ENTER_ROBOT || if_elem->m_strName == IF_LEAVE_ROBOT) {
                                    if_elem->m_StateImages[st].SetStateLabelParams(
                                            x - 1, y - 1, Float2Int(if_elem->m_xSize), Float2Int(if_elem->m_ySize),
                                            align_x, align_y, perenos, sme_x, sme_y,
                                            CRect(clip_sx, clip_sy, clip_ex, clip_ey), text, font, 0xFF000000);
                                    if_elem->m_StateImages[st].SetStateText(false);
                                }

                                if_elem->m_StateImages[st].SetStateLabelParams(
                                        x, y, Float2Int(if_elem->m_xSize), Float2Int(if_elem->m_ySize), align_x,
                                        align_y, perenos, sme_x, sme_y, CRect(clip_sx, clip_sy, clip_ex, clip_ey), text,
                                        font, color);
                                if_elem->m_StateImages[st].SetStateText(false);

                                if (if_elem->m_strName == IF_ENTER_ROBOT) {
                                    text = labels_file->ParGet(L"inro_part2");

                                    if_elem->m_StateImages[st].SetStateLabelParams(
                                            x - 1, y + 8, Float2Int(if_elem->m_xSize), Float2Int(if_elem->m_ySize),
                                            align_x, align_y, perenos, sme_x, sme_y,
                                            CRect(clip_sx, clip_sy, clip_ex, clip_ey), text, font, 0xFF000000);
                                    if_elem->m_StateImages[st].SetStateText(false);

                                    if_elem->m_StateImages[st].SetStateLabelParams(
                                            x, y + 9, Float2Int(if_elem->m_xSize), Float2Int(if_elem->m_ySize), align_x,
                                            align_y, perenos, sme_x, sme_y, CRect(clip_sx, clip_sy, clip_ex, clip_ey),
                                            text, font, color);
                                    if_elem->m_StateImages[st].SetStateText(false);
                                }
                            }
                            else if (labels->BlockGetName(bl_cnt) == IF_STATE_DYNAMIC_LABEL) {
                                if_elem->m_StateImages[st].SetStateLabelParams(
                                        x, y, Float2Int(if_elem->m_xSize), Float2Int(if_elem->m_ySize), align_x,
                                        align_y, perenos, sme_x, sme_y, CRect(clip_sx, clip_sy, clip_ex, clip_ey),
                                        std::wstring(L""), font, color);
                            }
                        }
                    }
                }
            }
        }
    }
    if (m_strName == IF_MAIN) {
        g_IFaceList->CreateGroupSelection(this);
        g_IFaceList->CreateOrdersGlow(this);
    }

    SortElementsByZ();

    return need2save;
}

CIFaceElement *CInterface::DelElement(CIFaceElement *pElement) {
    DTRACE();

#if defined _TRACE || defined _DEBUG

    CIFaceElement *first = m_FirstElement;
    for (; first; first = first->m_NextElement) {
        if (first == pElement)
            break;
    }
    if (first == NULL) {
        debugbreak();
    }

#endif

    DCP();

    CIFaceElement *next = pElement->m_NextElement;
    DCP();

    LIST_DEL(pElement, m_FirstElement, m_LastElement, m_PrevElement, m_NextElement);
    DCP();

    HDelete(CIFaceElement, pElement, g_MatrixHeap);
    DCP();

    return next;
}

bool CInterface::AddElement(CIFaceElement *pElement) {
    DTRACE();

    D3DXVECTOR3 dp(pElement->m_xPos + m_xPos, m_yPos + pElement->m_yPos, pElement->m_zPos);

    int nC = 0;
    while (pElement->m_StateImages[nC].Set && nC < MAX_STATES) {
        for (int i = 0; i < 4; i++) {
            pElement->m_StateImages[nC].m_Geom[i].p.x += dp.x;
            pElement->m_StateImages[nC].m_Geom[i].p.y += dp.y;
            pElement->m_StateImages[nC].m_Geom[i].p.z += dp.z;

            pElement->m_PosElInX = dp.x;
            pElement->m_PosElInY = dp.y;
        }
        nC++;
    }

    LIST_ADD(pElement, m_FirstElement, m_LastElement, m_PrevElement, m_NextElement);
    pElement->SetVisibility(false);
    return TRUE;
}

void CInterface::BeforeRender(void) {
    DTRACE();
#ifdef _DEBUG
    try {
#endif
        DCP();
        if (m_VisibleAlpha == IS_VISIBLEA || m_AlwaysOnTop) {
            DCP();
            CIFaceElement *pObjectsList = m_FirstElement;

            DCP();
            while (pObjectsList != NULL) {
                // if(pObjectsList->m_strName == IF_POPUP_RAMKA)
                //    ASSERT(1);
                DCP();
                if (pObjectsList->GetVisibility()) {
                    DCP();
                    pObjectsList->BeforeRender();
                    DCP();
                }
                DCP();
                pObjectsList = pObjectsList->m_NextElement;
                DCP();
            }
            if (g_MatrixMap->GetPlayerSide()->m_CurrSel == BASE_SELECTED &&
                g_MatrixMap->GetPlayerSide()->m_ConstructPanel->IsActive()) {
                DCP();
                g_MatrixMap->GetPlayerSide()->m_Constructor->BeforeRender();
            }
            DCP();
        }
#ifdef _DEBUG
    }
    catch (...) {
        debugbreak();
    }
#endif
}

void CInterface::Render() {
    DTRACE();
    if (m_VisibleAlpha == IS_VISIBLEA || m_AlwaysOnTop) {
        CIFaceElement *pObjectsList = m_FirstElement;

        while (pObjectsList != NULL) {
            if (pObjectsList->GetVisibility()) {
                pObjectsList->Render(pObjectsList->m_VisibleAlpha);
                if (pObjectsList->m_strName == IF_BASE_CONSTRUCTION_RIGHT) {
                    if (g_MatrixMap->GetPlayerSide()->m_CurrSel == BASE_SELECTED &&
                        g_MatrixMap->GetPlayerSide()->m_ConstructPanel->IsActive()) {
                        g_MatrixMap->GetPlayerSide()->m_Constructor->Render();
                    }
                }
            }
            pObjectsList = pObjectsList->m_NextElement;
        }
    }
}

bool CInterface::OnMouseMove(CPoint mouse) {
    DTRACE();

    if (g_MatrixMap->IsPaused()) {
        if (m_strName != IF_MINI_MAP && m_strName != IF_BASE && m_strName != IF_HINTS && m_strName != IF_POPUP_MENU) {
            return false;
        }
    }
    bool bCatch = false;
    bool bCatchStatic = false;
    bool MiniMapFocused = false;
    bool static_have_hint = false;
    g_IFaceList->m_FocusedElement = NULL;
    std::wstring static_name(L"");

    if (m_VisibleAlpha) {
        CIFaceElement *pObjectsList = m_FirstElement;
        while (pObjectsList != NULL) {
            if (pObjectsList->GetVisibility() || pObjectsList->m_nId == POPUP_SELECTOR_CATCHERS_ID) {
                if ((pObjectsList->m_Type == IFACE_STATIC || pObjectsList->m_Type == IFACE_DYNAMIC_STATIC)) {
                    if ((pObjectsList->m_strName != IF_POPUP_SELECTOR && pObjectsList->m_nId != POPUP_SELECTOR_CATCHERS_ID) && (pObjectsList
                                                                                                                                        ->OnMouseMove(
                                                                                                                                                mouse) /* || (pObjectsList->m_strName == IF_POPUP_RAMKA && pObjectsList->ElementCatch(mouse))*/)) {
                        g_IFaceList->m_FocusedElement = pObjectsList;
                        bCatchStatic = true;
                        static_name = pObjectsList->m_strName;
                        if (pObjectsList->m_Hint.HintTemplate != L"") {
                            static_have_hint = true;
                        }
                    }
                    else {
                        if (pObjectsList->m_nId == POPUP_SELECTOR_CATCHERS_ID && pObjectsList->ElementCatch(mouse)) {
                            if (FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE) && g_PopupMenu &&
                                g_PopupMenu->Selector()) {
                                g_IFaceList->m_FocusedElement = pObjectsList;
                                bCatchStatic = true;
                                g_PopupMenu->SetSelectorPos(pObjectsList->m_xPos, pObjectsList->m_yPos,
                                                            pObjectsList->m_iParam);
                            }
                        }
                    }
                }
                else {
                    if (!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE) && pObjectsList->OnMouseMove(mouse)) {
                        g_IFaceList->m_FocusedElement = pObjectsList;
                        bCatch = true;
                    }
                    else {
                        pObjectsList->Reset();
                    }
                }
            }
            pObjectsList = pObjectsList->m_NextElement;
        }
    }

    if (!bCatch && FLAG(g_IFaceList->m_IfListFlags, MINIMAP_BUTTON_DOWN) &&
        FLAG(g_IFaceList->m_IfListFlags, MINIMAP_ENABLE_DRAG)) {
        if (!IS_PREORDERING_NOSELECT && ((GetAsyncKeyState(g_Config.m_KeyActions[KA_FIRE]) & 0x8000) == 0x8000)) {
            g_MatrixMap->m_Minimap.ButtonClick(NULL);
            RESETFLAG(g_IFaceList->m_IfListFlags, MINIMAP_ENABLE_DRAG);
        }
        else if (IS_PREORDERING_NOSELECT && ((GetAsyncKeyState(g_Config.m_KeyActions[KA_AUTO]) & 0x8000) == 0x8000)) {
            g_MatrixMap->m_Minimap.ButtonClick(NULL);
            RESETFLAG(g_IFaceList->m_IfListFlags, MINIMAP_ENABLE_DRAG);
        }
    }
    // if(!bCatch && bCatchStatic){
    //    if(g_IFaceList->m_CurrentHint && g_IFaceList->m_CurrentHintControlName != static_name){
    //        g_IFaceList->m_CurrentHint->Release();
    //        g_IFaceList->m_CurrentHint = NULL;
    //        g_IFaceList->m_CurrentHintControlName = L"";
    //    }
    //}
    return (bCatch || bCatchStatic);
}

void CInterface::Reset() {
    CIFaceElement *objects = m_FirstElement;
    while (objects) {
        objects->Reset();
        objects = objects->m_NextElement;
    }
}

void CInterface::SetAlpha(BYTE alpha) {
    CIFaceElement *objects = m_FirstElement;
    while (objects) {
        objects->m_VisibleAlpha = alpha;
        objects = objects->m_NextElement;
    }
}

bool CInterface::OnMouseLBDown() {
    DTRACE();

    if (g_MatrixMap->IsPaused()) {
        if (m_strName != IF_MINI_MAP && m_strName != IF_BASE && m_strName != IF_HINTS && m_strName != IF_POPUP_MENU) {
            return false;
        }
    }

    if (FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE) &&
        !(g_PopupMenu && g_PopupMenu->GetRamka() &&
          g_PopupMenu->GetRamka()->ElementCatch(g_MatrixMap->m_Cursor.GetPos()))) {
        if (g_PopupMenu)
            g_PopupMenu->ResetMenu(true);
        return false;
    }

    if (FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE) &&
        /* (g_IFaceList->m_FocusedElement && g_IFaceList->m_FocusedElement->m_nId == POPUP_SELECTOR_CATCHERS_ID) && */
        g_PopupMenu) {
        g_PopupMenu->OnMenuItemPress();
        return true;
    }

    if (m_VisibleAlpha) {
        CIFaceElement *pObjectsList = m_LastElement;
        while (pObjectsList != NULL) {
            if (pObjectsList->GetVisibility()) {
                if (pObjectsList->ElementCatch(g_MatrixMap->m_Cursor.GetPos())) {
                    if (pObjectsList->m_nId == PERSONAL_ICON_ID) {
                        pObjectsList->OnMouseLBDown();
                    }
                    else if (IS_GROUP_ICON(pObjectsList->m_nId)) {
                        CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();

                        if ((GetAsyncKeyState(g_Config.m_KeyActions[KA_SHIFT]) & 0x8000) == 0x8000) {
                            CMatrixMapStatic *o = ps->GetCurGroup()->GetObjectByN(pObjectsList->m_nId - GROUP_ICONS_ID);
                            ps->RemoveObjectFromSelectedGroup(o);
                        }
                        else {
                            if (g_MatrixMap->GetPlayerSide()->GetCurSelNum() == pObjectsList->m_nId - GROUP_ICONS_ID) {
                                if (ps->GetCurSelObject() &&
                                    ps->GetCurSelObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
                                    ps->CreateGroupFromCurrent(ps->GetCurSelObject());
                                    ps->Select(ROBOT, NULL);
                                }
                                else if (ps->GetCurSelObject() &&
                                         ps->GetCurSelObject()->GetObjectType() == OBJECT_TYPE_FLYER) {
                                    ps->CreateGroupFromCurrent(ps->GetCurSelObject());
                                    ps->Select(FLYER, NULL);
                                }
                            }
                            else {
                                g_MatrixMap->GetPlayerSide()->SetCurSelNum(pObjectsList->m_nId - GROUP_ICONS_ID);
                            }
                        }
                        return TRUE;
                    }
                    else if (IS_STACK_ICON(pObjectsList->m_nId)) {
                        CIFaceElement *ne = pObjectsList->m_NextElement;
                        CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
                        ((CMatrixBuilding *)ps->m_ActiveObject)
                                ->m_BS.DeleteItem((pObjectsList->m_nId - STACK_ICON) + 1);
                        pObjectsList = ne;
                        return TRUE;
                    }
                    else if (pObjectsList->m_strName == IF_MAP_PANEL) {
                        if (IS_PREORDERING_NOSELECT) {
                            RESETFLAG(g_IFaceList->m_IfListFlags, MINIMAP_BUTTON_DOWN);
                            g_MatrixMap->GetPlayerSide()->OnLButtonDown(CPoint(0, 0));
                        }
                        else {
                            SETFLAG(g_IFaceList->m_IfListFlags, MINIMAP_BUTTON_DOWN);
                        }
                    }
                    else if (pObjectsList->ElementAlpha(g_MatrixMap->m_Cursor.GetPos()) &&
                             pObjectsList->OnMouseLBDown()) {
                        if (pObjectsList->m_Type == IFACE_CHECK_BUTTON ||
                            pObjectsList->m_Type == IFACE_CHECK_BUTTON_SPECIAL) {
                            pObjectsList->CheckGroupReset(m_FirstElement, pObjectsList);
                        }
                        return TRUE;
                    }
                }
            }
            pObjectsList = pObjectsList->m_PrevElement;
        }
    }
    return FALSE;
}

bool CInterface::OnMouseRBDown() {
    DTRACE();

    if (g_MatrixMap->IsPaused()) {
        if (m_strName != IF_MINI_MAP && m_strName != IF_BASE && m_strName != IF_HINTS && m_strName != IF_POPUP_MENU) {
            return false;
        }
    }

    if (m_VisibleAlpha) {
        CIFaceElement *pObjectsList = m_LastElement;
        while (pObjectsList != NULL) {
            if (pObjectsList->GetVisibility()) {
                if (pObjectsList->ElementCatch(g_MatrixMap->m_Cursor.GetPos())) {
                    if (pObjectsList->m_strName == IF_MAP_PANEL) {
                        g_MatrixMap->GetPlayerSide()->OnRButtonDown(CPoint(0, 0));
                        if (IS_PREORDERING_NOSELECT) {
                            SETFLAG(g_IFaceList->m_IfListFlags, MINIMAP_BUTTON_DOWN);
                        }
                    }
                    else {
                        // other elements
                        pObjectsList->OnMouseRBDown();
                    }
                }
            }
            pObjectsList = pObjectsList->m_PrevElement;
        }
    }

    return false;
}

void CInterface::OnMouseLBUp() {
    DTRACE();

    if (g_MatrixMap->IsPaused()) {
        if (m_strName != IF_MINI_MAP && m_strName != IF_BASE && m_strName != IF_HINTS && m_strName != IF_POPUP_MENU) {
            return;
        }
    }

    if (FLAG(g_IFaceList->m_IfListFlags, MINIMAP_BUTTON_DOWN)) {
        RESETFLAG(g_IFaceList->m_IfListFlags, MINIMAP_BUTTON_DOWN);
    }
    if (m_VisibleAlpha) {
        CIFaceElement *pObjectsList = m_LastElement;
        while (pObjectsList != NULL) {
            if (pObjectsList->GetVisibility() && pObjectsList->ElementCatch(g_MatrixMap->m_Cursor.GetPos()) &&
                pObjectsList->ElementAlpha(g_MatrixMap->m_Cursor.GetPos())) {
                pObjectsList->OnMouseLBUp();
                if (pObjectsList->m_Type == IFACE_CHECK_PUSH_BUTTON) {
                    pObjectsList->CheckGroupReset(m_FirstElement, pObjectsList);
                }

                return;
            }
            pObjectsList = pObjectsList->m_PrevElement;
        }
    }
}

void CInterface::Init(void) {
    DTRACE();

    int nC = 0;
    CIFaceElement *pElement = m_FirstElement;
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();

    if (m_AlwaysOnTop) {
        m_VisibleAlpha = IS_VISIBLEA;

        if (m_strName == IF_MINI_MAP) {
            while (pElement) {
                if (player_side->IsArcadeMode()) {
                    pElement->SetVisibility(false);
                }
                else {
                    pElement->SetVisibility(true);
                }
                if (pElement->m_strName == IF_MAP_ZOOM_IN) {
                    if (g_MatrixMap->m_Minimap.GetCurrentScale() == MINIMAP_MAX_SCALE) {
                        pElement->SetState(IFACE_DISABLED);
                    }
                    else if (g_MatrixMap->m_Minimap.GetCurrentScale() < MINIMAP_MAX_SCALE &&
                             pElement->GetState() == IFACE_DISABLED) {
                        pElement->SetState(IFACE_NORMAL);
                    }
                }
                else if (pElement->m_strName == IF_MAP_ZOOM_OUT) {
                    if (g_MatrixMap->m_Minimap.GetCurrentScale() == MINIMAP_MIN_SCALE) {
                        pElement->SetState(IFACE_DISABLED);
                    }
                    else if (g_MatrixMap->m_Minimap.GetCurrentScale() > MINIMAP_MIN_SCALE &&
                             pElement->GetState() == IFACE_DISABLED) {
                        pElement->SetState(IFACE_NORMAL);
                    }
                }
                else if (pElement->m_strName == IF_SHOWROBOTS_BUTT) {
                    if ((!player_side->GetSideRobots() || !player_side->GetMaxSideRobots())) {
                        pElement->SetState(IFACE_DISABLED);
                    }
                    else if (pElement->GetState() == IFACE_DISABLED &&
                             !(!player_side->GetSideRobots() || !player_side->GetMaxSideRobots())) {
                        pElement->SetState(IFACE_NORMAL);
                    }
                }

                pElement = pElement->m_NextElement;
            }
        }
        else if (m_strName == IF_MAIN) {
            CPoint pl[MAX_PLACES];

            int objects_cnt = 0;
            int robots = 0;

            bool rsel = false;
            bool gsel = false;
            bool ordering = FLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);
            bool singlem = FLAG(g_IFaceList->m_IfListFlags, SINGLE_MODE) || player_side->IsRobotMode();
            bool bld_tu = ordering && FLAG(g_IFaceList->m_IfListFlags, PREORDER_BUILD_TURRET);
            bool bld_re = ordering && FLAG(g_IFaceList->m_IfListFlags, PREORDER_BUILD_REPAIR);

            CMatrixGroup *work_group = NULL;
            CMatrixRobotAI *sel_bot = NULL;

            if (player_side->GetCurGroup() && player_side->GetCurGroup()->GetObjectsCnt()) {
                objects_cnt = player_side->GetCurGroup()->GetObjectsCnt();
                robots = player_side->GetCurGroup()->GetRobotsCnt();
                gsel = true;
                rsel = ((objects_cnt == 1) && (player_side->GetCurGroup()->m_FirstObject->GetObject()->IsLiveRobot()));
                sel_bot = (CMatrixRobotAI *)player_side->GetCurGroup()->m_FirstObject->GetObject();
                work_group = player_side->GetCurGroup();
            }

            bool bombers = false;
            bool repairers = false;
            bool heliors = false;

            bool stop = false;
            bool move = false;
            bool fire = false;
            bool capt = false;
            bool bomb = false;
            bool patrol = false;
            bool repair = false;
            bool getup = false;
            bool drop = false;

            bool bomber_sel = false;
            bool repairer_sel = false;
            bool heli_sel = false;
            bool robot_sel = false;
            bool new_name = false;
            bool new_lives = false;
            bool new_speed = false;
            bool new_weight = false;
            bool new_turmax = false;
            bool new_turhave = false;
            bool cant_build_tu = false;

            if (!player_side->IsEnoughResources(g_Config.m_CannonsProps[0].m_Resources) &&
                !player_side->IsEnoughResources(g_Config.m_CannonsProps[1].m_Resources) &&
                !player_side->IsEnoughResources(g_Config.m_CannonsProps[2].m_Resources) &&
                !player_side->IsEnoughResources(g_Config.m_CannonsProps[3].m_Resources)) {
                cant_build_tu = true;
            }

            CMatrixRobotAI *cur_r = NULL;
            CMatrixFlyer *cur_f = NULL;

            if (player_side->IsRobotMode()) {
                cur_r = (CMatrixRobotAI *)player_side->GetArcadedObject();
                robot_sel = true;
                if (!sel_bot) {
                    sel_bot = player_side->GetArcadedObject()->AsRobot();
                }
            }

            if (!sel_bot && singlem) {
                singlem = false;
                RESETFLAG(g_IFaceList->m_IfListFlags, SINGLE_MODE);
            }

            if (work_group) {
                CMatrixGroupObject *go = work_group->m_FirstObject;

                if (gsel) {
                    for (int i = 0; i < player_side->GetCurSelNum() && go; i++) {
                        go = go->m_NextObject;
                    }
                    if (go) {
                        CMatrixRobotAI *r = (CMatrixRobotAI *)go->GetObject();
                        if (go->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
                            robot_sel = true;
                            cur_r = (CMatrixRobotAI *)go->GetObject();
                            if (r->FindWeapon(WEAPON_BIGBOOM))
                                bomber_sel = true;
                            if (r->FindWeapon(WEAPON_REPAIR))
                                repairer_sel = true;
                        }
                    }
                }
            }

            if (work_group) {
                g_MatrixMap->GetSideById(PLAYER_SIDE)->ShowOrderState();
                if (gsel) {
                    int bombers_cnt = 0;
                    int repairers_cnt = 0;

                    CMatrixGroupObject *objs = work_group->m_FirstObject;
                    while (objs) {
                        CMatrixRobotAI *robot = objs->GetObject()->AsRobot();
                        if (objs->GetObject()->IsRobot()) {
                            if (robot->FindWeapon(WEAPON_BIGBOOM)) {
                                bombers_cnt++;
                            }
                            if (robot->FindWeapon(WEAPON_REPAIR)) {
                                repairers_cnt++;
                            }
                            if (robot->GetGroupLogic() >= 0 &&
                                g_MatrixMap->GetSideById(robot->GetSide())
                                                ->m_PlayerGroup[robot->GetGroupLogic()]
                                                .Order() == mpo_MoveTo) {  // if(robot->FindOrderLikeThat(ROT_MOVE_TO)){
                                move = true;
                            }
                            if (robot->GetGroupLogic() >= 0 &&
                                g_MatrixMap->GetSideById(robot->GetSide())
                                                ->m_PlayerGroup[robot->GetGroupLogic()]
                                                .Order() ==
                                        mpo_Capture) {  // if(robot->FindOrderLikeThat(ROT_CAPTURE_FACTORY)){
                                capt = true;
                            }
                            if (robot->GetGroupLogic() >= 0 &&
                                g_MatrixMap->GetSideById(robot->GetSide())
                                                ->m_PlayerGroup[robot->GetGroupLogic()]
                                                .Order() == mpo_Attack) {  // if(robot->FindOrderLikeThat(ROBOT_FIRE)){
                                fire = true;
                            }
                            if (robot->GetGroupLogic() >= 0 &&
                                g_MatrixMap->GetSideById(robot->GetSide())
                                                ->m_PlayerGroup[robot->GetGroupLogic()]
                                                .Order() == mpo_Stop) {  // if(robot->GetOrdersInPool() == 0){
                                stop = true;
                            }
                            if (robot->GetGroupLogic() >= 0 &&
                                g_MatrixMap->GetSideById(robot->GetSide())
                                                ->m_PlayerGroup[robot->GetGroupLogic()]
                                                .Order() == mpo_Patrol) {  // if(robot->GetOrdersInPool() == 0){
                                patrol = true;
                            }
                            if (robot->GetGroupLogic() >= 0 &&
                                g_MatrixMap->GetSideById(robot->GetSide())
                                                ->m_PlayerGroup[robot->GetGroupLogic()]
                                                .Order() == mpo_Bomb) {  // if(robot->FindOrderLikeThat(ROT_MOVE_TO)){
                                bomb = true;
                            }
                            if (robot->GetGroupLogic() >= 0 &&
                                g_MatrixMap->GetSideById(robot->GetSide())
                                                ->m_PlayerGroup[robot->GetGroupLogic()]
                                                .Order() == mpo_Repair) {  // if(robot->FindOrderLikeThat(ROT_MOVE_TO)){
                                repair = true;
                            }
                        }
                        objs = objs->m_NextObject;
                    }
                    if (bombers_cnt == objects_cnt) {
                        bombers = true;
                    }
                    if (repairers_cnt == objects_cnt) {
                        repairers = true;
                    }
                }
            }

            while (pElement) {
                // Always visible
                // elements////////////////////////////////////////////////////////////////////////////////////
                if (pElement->m_strName == IF_NAME_LABEL) {
                    if (gsel || player_side->IsArcadeMode()) {
                        if ((rsel || robot_sel) && cur_r) {
                            if (name != cur_r->m_Name) {
                                name = cur_r->m_Name;
                                new_name = true;
                            }
                        }
                    }
                    else if (player_side->m_CurrSel == BUILDING_SELECTED || player_side->m_CurrSel == BASE_SELECTED) {
                        CBlockPar *bp_tmp = g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Buildings");
                        if (player_side->m_ActiveObject && player_side->m_ActiveObject->AsBuilding()->IsBase() &&
                            name != bp_tmp->ParGet(L"Base_Name")) {
                            new_name = true;
                            name = bp_tmp->ParGet(L"Base_Name");
                        }
                        else if (player_side->m_ActiveObject &&
                                 player_side->m_ActiveObject->AsBuilding()->m_Kind == BUILDING_TITAN &&
                                 name != bp_tmp->ParGet(L"Titan_Name")) {
                            new_name = true;
                            name = bp_tmp->ParGet(L"Titan_Name");
                        }
                        else if (player_side->m_ActiveObject &&
                                 player_side->m_ActiveObject->AsBuilding()->m_Kind == BUILDING_ELECTRONIC &&
                                 name != bp_tmp->ParGet(L"Electronics_Name")) {
                            new_name = true;
                            name = bp_tmp->ParGet(L"Electronics_Name");
                        }
                        else if (player_side->m_ActiveObject &&
                                 player_side->m_ActiveObject->AsBuilding()->m_Kind == BUILDING_ENERGY &&
                                 name != bp_tmp->ParGet(L"Energy_Name")) {
                            new_name = true;
                            name = bp_tmp->ParGet(L"Energy_Name");
                        }
                        else if (player_side->m_ActiveObject &&
                                 player_side->m_ActiveObject->AsBuilding()->m_Kind == BUILDING_PLASMA &&
                                 name != bp_tmp->ParGet(L"Plasma_Name")) {
                            new_name = true;
                            name = bp_tmp->ParGet(L"Plasma_Name");
                        }
                    }
                }
                else if (pElement->m_strName == IF_LIVES_LABEL) {
                    if (gsel || player_side->IsArcadeMode()) {
                        if ((rsel || robot_sel) && cur_r) {
                            if (lives != cur_r->GetHitPoint()) {
                                lives = cur_r->GetHitPoint();
                                max_lives = cur_r->GetMaxHitPoint();
                                new_lives = true;
                            }
                        }
                    }
                    else if (player_side->m_CurrSel == BUILDING_SELECTED || player_side->m_CurrSel == BASE_SELECTED) {
                        if (lives != player_side->m_ActiveObject->AsBuilding()->GetHitPoint()) {
                            lives = player_side->m_ActiveObject->AsBuilding()->GetHitPoint();
                            max_lives = player_side->m_ActiveObject->AsBuilding()->GetMaxHitPoint();
                            new_lives = true;
                        }
                    }
                }
                else if (pElement->m_strName == IF_TURRETS_MAX) {
                    if (player_side->m_CurrSel == BUILDING_SELECTED || player_side->m_CurrSel == BASE_SELECTED) {
                        if (player_side->m_ActiveObject->AsBuilding()->m_TurretsMax != turmax) {
                            turmax = player_side->m_ActiveObject->AsBuilding()->m_TurretsMax;
                            new_turmax = true;
                        }
                    }
                }
                else if (pElement->m_strName == IF_TURRETS_HAVE) {
                    if (player_side->m_CurrSel == BUILDING_SELECTED || player_side->m_CurrSel == BASE_SELECTED) {
                        CMatrixBuilding *bld = (CMatrixBuilding *)player_side->m_ActiveObject;
                        if (bld->GetPlacesForTurrets(pl) != turhave) {
                            turhave = bld->GetPlacesForTurrets(pl);
                            new_turhave = true;
                        }
                    }
                }

                // Invisible by default
                // elements///////////////////////////////////////////////////////////////////////////////
                if (pElement->m_Type == IFACE_DYNAMIC_STATIC && IS_GROUP_ICON(pElement->m_nId)) {
                    pElement->SetVisibility(true);
                }
                else if (pElement->m_nId == PERSONAL_ICON_ID) {
                    pElement->SetVisibility(true);
                }
                else if (pElement->m_nId == DYNAMIC_TURRET) {
                    pElement->SetVisibility(true);
                }
                else {
                    pElement->SetVisibility(false);
                }

                if (pElement->m_strName == IF_MAIN_PANEL1) {
                    pElement->SetVisibility(true);
                }
                else if (pElement->m_strName == IF_MAIN_PANEL2) {
                    pElement->SetVisibility(true);
                }

                if (pElement->m_strName == IF_NAME_LABEL && player_side->m_CurrSel != NOTHING_SELECTED) {
                    if (new_name) {
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = name;
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                    pElement->SetVisibility(true);
                }
                else if ((player_side->m_CurrSel == BUILDING_SELECTED || player_side->m_CurrSel == BASE_SELECTED)) {
                    CMatrixBuilding *bld = player_side->m_ActiveObject->AsBuilding();
                    CBlockPar *bp_tmp = g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Buildings");
                    int income = player_side->GetIncomePerTime(int(bld->m_Kind), 60000);
                    if (pElement->m_strName == IF_BUILDING_OPIS) {
                        if (!bld->m_BS.GetItemsCnt()) {
                            pElement->SetVisibility(true);
                        }
                        if (btype != int(bld->m_Kind)) {
                            btype = int(bld->m_Kind);
                            if (bld->m_Kind == BUILDING_BASE) {
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = bp_tmp->ParGet(L"Base_Descr");
                            }
                            else if (bld->m_Kind == BUILDING_TITAN) {
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = bp_tmp->ParGet(L"Titan_Descr");
                            }
                            else if (bld->m_Kind == BUILDING_ELECTRONIC) {
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = bp_tmp->ParGet(L"Electronics_Descr");
                            }
                            else if (bld->m_Kind == BUILDING_ENERGY) {
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = bp_tmp->ParGet(L"Energy_Descr");
                            }
                            else if (bld->m_Kind == BUILDING_PLASMA) {
                                pElement->m_StateImages[IFACE_NORMAL].m_Caption = bp_tmp->ParGet(L"Plasma_Descr");
                            }
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                        }
                    }
                    else if (pElement->m_strName == IF_BASE_RES_INC && bld->m_Kind == BUILDING_BASE) {
                        if (!bld->m_BS.GetItemsCnt()) {
                            pElement->SetVisibility(true);
                        }

                        if (income != base_res_income) {
                            base_res_income = income;
                            std::wstring suck(bp_tmp->ParGet(L"ResPer"));
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption =
                                utils::replace(
                                    suck,
                                    L"<resources>",
                                    utils::format(L"<Color=247,195,0>%d</Color>", base_res_income));
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                        }
                    }
                    else if (pElement->m_strName == IF_FACTORY_RES_INC && bld->m_Kind != BUILDING_BASE) {
                        if (!bld->m_BS.GetItemsCnt()) {
                            pElement->SetVisibility(true);
                        }
                        if (income != factory_res_income) {
                            factory_res_income = income;
                            std::wstring suck(bp_tmp->ParGet(L"ResPer"));
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption =
                                utils::replace(
                                    suck,
                                    L"<resources>",
                                    utils::format(L"<Color=247,195,0>%d</Color>", factory_res_income));
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                        }
                    }
                }
                if (pElement->m_strName == IF_LIVES_LABEL && player_side->m_CurrSel != NOTHING_SELECTED) {
                    if (new_lives) {
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption =
                            utils::format(L"%d/%d", Float2Int(lives), Float2Int(max_lives));
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                    pElement->SetVisibility(true);
                }

                if (singlem) {
                    if (pElement->m_strName == IF_MANUAL_BG) {
                        if (!sel_bot->IsDisableManual()) {
                            pElement->SetVisibility(true);
                        }
                    }
                    else if (pElement->m_strName == IF_ENTER_ROBOT) {
                        if (!player_side->IsArcadeMode() && !sel_bot->IsDisableManual()) {
                            pElement->SetVisibility(true);
                        }
                    }
                    else if (pElement->m_nId == DYNAMIC_WEAPON_ON_ID) {
                        pElement->SetVisibility(true);
                    }
                    else if (pElement->m_strName == IF_LEAVE_ROBOT) {
                        if (player_side->IsArcadeMode()) {
                            pElement->SetVisibility(true);
                        }
                    }
                    else if (pElement->m_strName == IF_OVER_HEAT && sel_bot) {
                        pElement->SetVisibility(true);
                        SMatrixRobotUnit *unit = &sel_bot->m_Unit[Float2Int(pElement->m_Param1)];

                        for (int i = 0; i < MAX_WEAPON_CNT; i++) {
                            if (sel_bot->GetWeapon(i).m_Unit == unit) {
                                pElement->m_VisibleAlpha = byte(sel_bot->GetWeapon(i).m_Heat * 0.25f);
                            }
                        }
                    }
                    else if (pElement->m_strName == IF_MAIN_WEAPONSLOTS) {
                        pElement->SetVisibility(true);
                    }
                }

                if (player_side->IsArcadeMode()) {
                    if (bombers && pElement->m_strName == IF_MAIN_SELFBOMB) {
                        pElement->SetVisibility(true);
                    }
                }
                if (work_group) {
                    if (pElement->m_Type == IFACE_DYNAMIC_STATIC && IS_SELECTION(pElement->m_nId)) {
                        int n = pElement->m_nId - GROUP_SELECTION_ID;
                        if (n < work_group->GetObjectsCnt()) {
                            CMatrixGroupObject *so = work_group->m_FirstObject;
                            int i;
                            for (i = 0; i < n && so; i++) {
                                so = so->m_NextObject;
                            }

                            if (so) {
                                if (so->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
                                    if (i == player_side->GetCurSelNum()) {
                                        ((CMatrixRobotAI *)so->GetObject())
                                                ->CreateProgressBarClone(m_xPos + 68, m_yPos + 179, 68, PBC_CLONE2);
                                    }
                                    if (!singlem) {
                                        ((CMatrixRobotAI *)so->GetObject())
                                                ->CreateProgressBarClone(pElement->m_xPos + m_xPos,
                                                                         pElement->m_yPos + m_yPos + 36, 46,
                                                                         PBC_CLONE1);
                                    }
                                    else {
                                        ((CMatrixRobotAI *)so->GetObject())->DeleteProgressBarClone(PBC_CLONE1);
                                    }
                                }
                            }

                            if (!singlem) {
                                pElement->SetVisibility(true);
                            }
                        }
                        if (singlem) {
                            pElement->SetVisibility(false);
                        }
                    }
                }
                else if (player_side->m_CurrSel == BUILDING_SELECTED || player_side->m_CurrSel == BASE_SELECTED) {
                    CMatrixBuilding *bld = (CMatrixBuilding *)player_side->m_ActiveObject;
                    bld->CreateProgressBarClone(m_xPos + 68, m_yPos + 179, 68, PBC_CLONE2);

                    if (bld->m_Kind == BUILDING_TITAN && pElement->m_strName == IF_TITAN_PLANT) {
                        pElement->SetVisibility(true);
                    }
                    else if (bld->m_Kind == BUILDING_PLASMA && pElement->m_strName == IF_PLASMA_PLANT) {
                        pElement->SetVisibility(true);
                    }
                    else if (bld->m_Kind == BUILDING_ELECTRONIC && pElement->m_strName == IF_ELECTRO_PLANT) {
                        pElement->SetVisibility(true);
                    }
                    else if (bld->m_Kind == BUILDING_ENERGY && pElement->m_strName == IF_ENERGY_PLANT) {
                        pElement->SetVisibility(true);
                    }
                    else if (bld->m_Kind == BUILDING_REPAIR &&
                             (pElement->m_strName == IF_REPAIR_PLANT ||
                              (pElement->m_strName == IF_BUILD_REPAIR && !bld_tu && !bld_re))) {
                        pElement->SetVisibility(true);
                    }
                    else if (bld->m_Kind == BUILDING_BASE && pElement->m_strName == IF_BASE_PLANT) {
                        pElement->SetVisibility(true);
                    }
                    else if (bld->m_BS.GetItemsCnt() &&
                             (pElement->m_strName == IF_STACK_ICON || pElement->m_strName == IF_STACK_OTHER ||
                              (IS_STACK_ICON(pElement->m_nId) && pElement->m_iParam == int(bld)))) {
                        pElement->SetVisibility(true);
                    }
                    else if (pElement->m_strName == IF_BASE_LINE) {
                        pElement->SetVisibility(true);
                    }
                    else if (pElement->m_strName == IF_BUILD_RO && !bld_tu) {
                        if (bld->m_Kind == BUILDING_BASE) {
                            pElement->SetVisibility(true);
                        }
                    }
                    else if (pElement->m_strName == IF_BUILD_CA && !bld_tu) {
                        pElement->SetVisibility(true);
                        if (!bld->GetPlacesForTurrets(pl) || cant_build_tu || bld->m_BS.IsMaxItems()) {
                            pElement->SetState(IFACE_DISABLED);
                        }
                        else if (bld->GetPlacesForTurrets(pl) && !cant_build_tu && !bld->m_BS.IsMaxItems() &&
                                 pElement->GetState() == IFACE_DISABLED) {
                            pElement->SetState(IFACE_NORMAL);
                        }
                    }
                    else if (pElement->m_strName == IF_CALL_FROM_HELL && !bld_tu) {
                        pElement->SetVisibility(true);
                        if (g_MatrixMap->MaintenanceDisabled() || g_MatrixMap->BeforeMaintenanceTime() > 0) {
                            pElement->SetState(IFACE_DISABLED);
                        }
                        else if (pElement->GetState() == IFACE_DISABLED) {
                            pElement->SetState(IFACE_NORMAL);
                        }
                    }
                    else if (bld->m_TurretsMax == 1 && pElement->m_strName == IF_PODL1) {
                        pElement->SetVisibility(true);
                    }
                    else if (bld->m_TurretsMax == 2 && pElement->m_strName == IF_PODL2) {
                        pElement->SetVisibility(true);
                    }
                    else if (bld->m_TurretsMax == 3 && pElement->m_strName == IF_PODL3) {
                        pElement->SetVisibility(true);
                    }
                    else if (bld->m_TurretsMax == 4 && pElement->m_strName == IF_PODL4) {
                        pElement->SetVisibility(true);
                    }
                    else if (bld->IsBase() && bld->m_BS.GetItemsCnt() == 0 && pElement->m_strName == IF_MB_RES) {
                        pElement->SetVisibility(true);
                    }
                    else if (bld->m_Kind == BUILDING_TITAN && bld->m_BS.GetItemsCnt() == 0 &&
                             pElement->m_strName == IF_TF_RES) {
                        pElement->SetVisibility(true);
                    }
                    else if (bld->m_Kind == BUILDING_ELECTRONIC && bld->m_BS.GetItemsCnt() == 0 &&
                             pElement->m_strName == IF_ELF_RES) {
                        pElement->SetVisibility(true);
                    }
                    else if (bld->m_Kind == BUILDING_ENERGY && bld->m_BS.GetItemsCnt() == 0 &&
                             pElement->m_strName == IF_ENF_RES) {
                        pElement->SetVisibility(true);
                    }
                    else if (bld->m_Kind == BUILDING_PLASMA && bld->m_BS.GetItemsCnt() == 0 &&
                             pElement->m_strName == IF_PF_RES) {
                        pElement->SetVisibility(true);
                    }
                    else if (pElement->m_strName == IF_ZAGLUSHKA1) {
                        pElement->SetVisibility(true);
                    }
                }

                if (!ordering && !bld_tu) {
                    if (gsel) {
                        if (pElement->m_strName == IF_ORDER_STOP) {
                            pElement->SetVisibility(true);
                        }
                        else if (pElement->m_strName == IF_ORDER_MOVE) {
                            pElement->SetVisibility(true);
                        }
                        else if (pElement->m_strName == IF_ORDER_PATROL) {
                            pElement->SetVisibility(true);
                        }
                        else if (pElement->m_strName == IF_ORDER_FIRE) {
                            pElement->SetVisibility(true);
                        }
                        else if (FLAG(g_IFaceList->m_IfListFlags, AUTO_FROBOT_ON) &&
                                 pElement->m_strName == IF_AORDER_FROBOT_ON) {
                            pElement->SetVisibility(true);
                        }
                        else if (!FLAG(g_IFaceList->m_IfListFlags, AUTO_FROBOT_ON) &&
                                 pElement->m_strName == IF_AORDER_FROBOT_OFF) {
                            pElement->SetVisibility(true);
                        }
                        else if (FLAG(g_IFaceList->m_IfListFlags, AUTO_PROTECT_ON) &&
                                 pElement->m_strName == IF_AORDER_PROTECT_ON) {
                            pElement->SetVisibility(true);
                        }
                        else if (!FLAG(g_IFaceList->m_IfListFlags, AUTO_PROTECT_ON) &&
                                 pElement->m_strName == IF_AORDER_PROTECT_OFF) {
                            pElement->SetVisibility(true);
                        }
                        else if (player_side->GetCurGroup()->GetRobotsCnt() &&
                                 FLAG(g_IFaceList->m_IfListFlags, AUTO_CAPTURE_ON) &&
                                 pElement->m_strName == IF_AORDER_CAPTURE_ON) {
                            pElement->SetVisibility(true);
                        }
                        else if (player_side->GetCurGroup()->GetRobotsCnt() &&
                                 !FLAG(g_IFaceList->m_IfListFlags, AUTO_CAPTURE_ON) &&
                                 pElement->m_strName == IF_AORDER_CAPTURE_OFF) {
                            pElement->SetVisibility(true);
                        }
                        else if (pElement->m_nId == GROUP_SELECTOR_ID && !singlem) {
                            pElement->SetVisibility(true);
                        }

                        if (IS_ORDER_GLOW(pElement->m_nId)) {
                            if (pElement->m_nId - ORDERS_GLOW_ID == 0 && stop)
                                pElement->SetVisibility(true);
                            if (pElement->m_nId - ORDERS_GLOW_ID == 1 && move && !capt && !getup && !drop && !bomb &&
                                !repair)
                                pElement->SetVisibility(true);
                            if (pElement->m_nId - ORDERS_GLOW_ID == 2 && patrol)
                                pElement->SetVisibility(true);
                            if (pElement->m_nId - ORDERS_GLOW_ID == 3 && fire)
                                pElement->SetVisibility(true);
                            if (pElement->m_nId - ORDERS_GLOW_ID == 4 && (capt || getup || drop))
                                pElement->SetVisibility(true);
                            if (pElement->m_nId - ORDERS_GLOW_ID == 5 && (bomber_sel || repairer_sel) &&
                                (bomb || repair))
                                pElement->SetVisibility(true);
                        }

                        if (pElement->m_strName == IF_MAIN_PROG) {
                            pElement->SetVisibility(true);
                        }

                        if (pElement->m_strName == IF_ORDER_CAPTURE) {
                            pElement->SetVisibility(true);
                        }

                        if (/*bombers*/ bomber_sel && pElement->m_strName == IF_ORDER_BOMB) {
                            pElement->SetVisibility(true);
                        }
                        if (/*repairers*/ repairer_sel && pElement->m_strName == IF_ORDER_REPAIR) {
                            pElement->SetVisibility(true);
                        }
                    }
                    else {
                    }
                }
                else {
                    if (pElement->m_strName == IF_ORDER_CANCEL) {
                        pElement->SetVisibility(true);
                    }
                    else if (pElement->m_strName == IF_ZAGLUSHKA1) {
                        pElement->SetVisibility(true);
                    }

                    if (gsel) {
                        if (pElement->m_nId == GROUP_SELECTOR_ID && !singlem) {
                            pElement->SetVisibility(true);
                        }
                    }

                    if (player_side->m_CurrSel == BUILDING_SELECTED || player_side->m_CurrSel == BASE_SELECTED) {
                        CMatrixBuilding *bld = (CMatrixBuilding *)player_side->m_ActiveObject;
                        if (bld_tu && !(player_side->m_CurrentAction == BUILDING_TURRET)) {
                            if (pElement->m_strName == IF_BUILD_TUR1) {
                                pElement->SetVisibility(true);
                                if (!bld->GetPlacesForTurrets(pl) ||
                                    !player_side->IsEnoughResources(g_Config.m_CannonsProps[0].m_Resources)) {
                                    pElement->SetState(IFACE_DISABLED);
                                }
                                else if (bld->GetPlacesForTurrets(pl) &&
                                         player_side->IsEnoughResources(g_Config.m_CannonsProps[0].m_Resources) &&
                                         pElement->GetState() == IFACE_DISABLED) {
                                    pElement->SetState(IFACE_NORMAL);
                                }
                            }
                            else if (pElement->m_strName == IF_BUILD_TUR2) {
                                pElement->SetVisibility(true);
                                if (!bld->GetPlacesForTurrets(pl) ||
                                    !player_side->IsEnoughResources(g_Config.m_CannonsProps[1].m_Resources)) {
                                    pElement->SetState(IFACE_DISABLED);
                                }
                                else if (bld->GetPlacesForTurrets(pl) &&
                                         player_side->IsEnoughResources(g_Config.m_CannonsProps[1].m_Resources) &&
                                         pElement->GetState() == IFACE_DISABLED) {
                                    pElement->SetState(IFACE_NORMAL);
                                }
                            }
                            else if (pElement->m_strName == IF_BUILD_TUR3) {
                                pElement->SetVisibility(true);
                                if (!bld->GetPlacesForTurrets(pl) ||
                                    !player_side->IsEnoughResources(g_Config.m_CannonsProps[2].m_Resources)) {
                                    pElement->SetState(IFACE_DISABLED);
                                }
                                else if (bld->GetPlacesForTurrets(pl) &&
                                         player_side->IsEnoughResources(g_Config.m_CannonsProps[2].m_Resources) &&
                                         pElement->GetState() == IFACE_DISABLED) {
                                    pElement->SetState(IFACE_NORMAL);
                                }
                            }
                            else if (pElement->m_strName == IF_BUILD_TUR4) {
                                pElement->SetVisibility(true);
                                if (!bld->GetPlacesForTurrets(pl) ||
                                    !player_side->IsEnoughResources(g_Config.m_CannonsProps[3].m_Resources)) {
                                    pElement->SetState(IFACE_DISABLED);
                                }
                                else if (bld->GetPlacesForTurrets(pl) &&
                                         player_side->IsEnoughResources(g_Config.m_CannonsProps[3].m_Resources) &&
                                         pElement->GetState() == IFACE_DISABLED) {
                                    pElement->SetState(IFACE_NORMAL);
                                }
                            }
                        }
                    }
                }
                pElement = pElement->m_NextElement;
            }
            ///////////////////EOF
            ///MAIN////////////////////////////////////////////////////////////////////////////////////////////
        }
        else if (m_strName == IF_RADAR) {
            while (pElement) {
                if (player_side->IsArcadeMode()) {
                    pElement->SetVisibility(true);
                }
                else {
                    pElement->SetVisibility(false);
                }
                pElement = pElement->m_NextElement;
            }
        }
        else if (m_strName == IF_TOP) {
            while (pElement) {
                pElement->SetVisibility(true);
                if (pElement->m_strName == IF_TITAN_LABEL) {
                    if (prev_titan != player_side->GetResourcesAmount(TITAN)) {
                        prev_titan = player_side->GetResourcesAmount(TITAN);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", prev_titan);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                }
                else if (pElement->m_strName == IF_ELECTRO_LABEL) {
                    if (prev_electro != player_side->GetResourcesAmount(ELECTRONICS)) {
                        prev_electro = player_side->GetResourcesAmount(ELECTRONICS);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", prev_electro);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                }
                else if (pElement->m_strName == IF_ENERGY_LABEL) {
                    if (prev_energy != player_side->GetResourcesAmount(ENERGY)) {
                        prev_energy = player_side->GetResourcesAmount(ENERGY);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", prev_energy);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                }
                else if (pElement->m_strName == IF_PLASMA_LABEL) {
                    if (prev_plasma != player_side->GetResourcesAmount(PLASMA)) {
                        prev_plasma = player_side->GetResourcesAmount(PLASMA);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", prev_plasma);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                }
                else if (pElement->m_strName == IF_RVALUE_LABEL) {
                    if (/*количество роботов изменилось*/ robots != player_side->GetSideRobots() ||
                        /*максимальное количество изменилось*/ max_robots != player_side->GetMaxSideRobots()) {
                        robots = player_side->GetSideRobots();
                        max_robots = player_side->GetMaxSideRobots();
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d/%d", robots, max_robots);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                }
                pElement = pElement->m_NextElement;
            }
        }
        return;
    }

    m_VisibleAlpha = IS_NOT_VISIBLEA;

    if ((m_strName == IF_BASE) &&
        (player_side->m_CurrSel == BUILDING_SELECTED || player_side->m_CurrSel == BASE_SELECTED) &&
        player_side->m_ActiveObject && player_side->m_ActiveObject->GetObjectType() == OBJECT_TYPE_BUILDING) {
        CMatrixBuilding *building = (CMatrixBuilding *)player_side->m_ActiveObject;
        int total_res[MAX_RESOURCES];
        player_side->m_Constructor->GetConstructionPrice(total_res);
        for (int i = 0; i < MAX_RESOURCES; i++) {
            if (g_IFaceList->m_RCountControl->GetCounter()) {
                total_res[i] *= g_IFaceList->m_RCountControl->GetCounter();
            }
        }

        bool build_flag = (building->m_TurretsHave < building->m_TurretsMax);
        m_VisibleAlpha = IS_VISIBLEA;
        int cfg_num = player_side->m_ConstructPanel->m_CurrentConfig;

        while (pElement) {
            pElement->SetVisibility(false);
            if (building->IsBase()) {
                bool bld = building->m_BS.GetItemsCnt() < MAX_STACK_UNITS;
                int common_weapon =
                        g_MatrixMap
                                ->m_RobotWeaponMatrix
                                        [player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind - 1]
                                .common;
                int extended =
                        g_MatrixMap
                                ->m_RobotWeaponMatrix
                                        [player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind - 1]
                                .extra;

                if (player_side->m_ConstructPanel->IsActive() &&
                    (pElement->m_strName == L"counthz" || pElement->m_strName == IF_BASE_CONSTRUCTION_LEFT ||
                     pElement->m_strName == IF_BASE_CONSTRUCTION_RIGHT ||
                     pElement->m_strName == IF_BASE_CONSTRUCTION_FOOT)) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_RCNAME) {
                    if (rcname != player_side->m_Constructor->GetRenderBot()->m_Name) {
                        rcname = player_side->m_Constructor->GetRenderBot()->m_Name;
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = rcname;
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_WARNING1 || pElement->m_strName == IF_BASE_WARNING_LABEL)) {
                    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
                    if (ps->GetRobotsCnt() + ps->GetRobotsInStack() >= ps->GetMaxSideRobots())
                        pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() && IS_DYNAMIC_WARNING(pElement->m_nId)) {
                    int res[MAX_RESOURCES];
                    player_side->m_Constructor->GetConstructionPrice(res);
                    for (int i = 0; i < MAX_RESOURCES; i++) {
                        if (g_IFaceList->m_RCountControl->GetCounter())
                            res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                    }
                    dword dw = pElement->m_nId - DYNAMIC_WARNING;
                    if (dw < 4) {
                        ERes r = ERes(dw);
                        if (player_side->GetResourcesAmount(r) < res[r]) {
                            pElement->SetVisibility(true);
                        }
                    }
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_CONST_CANCEL) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_CONST_BUILD) {
                    int res[4];
                    player_side->m_Constructor->GetConstructionPrice(res);

                    bool men = false;
                    for (int i = 0; !men && i < MAX_RESOURCES; ++i) {
                        men |= player_side->GetResourcesAmount(ERes(i)) < res[i];
                    }

                    if (!bld || men ||
                        player_side->GetRobotsCnt() + player_side->GetRobotsInStack() >=
                                player_side->GetMaxSideRobots()) {
                        pElement->m_DefState = IFACE_DISABLED;
                        pElement->m_CurState = IFACE_DISABLED;
                        g_IFaceList->m_RCountControl->Disable();
                    }
                    else if (pElement->m_CurState == IFACE_DISABLED) {
                        pElement->m_DefState = IFACE_NORMAL;
                        pElement->m_CurState = IFACE_NORMAL;
                        g_IFaceList->m_RCountControl->Enable();
                    }
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_ITEM_LABEL1) {
                    if (item_label1 != player_side->m_ConstructPanel->m_FocusedLabel) {
                        item_label1 = player_side->m_ConstructPanel->m_FocusedLabel;
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = item_label1;
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_ITEM_LABEL2) {
                    if (item_label2 != player_side->m_ConstructPanel->m_FocusedDescription) {
                        item_label2 = player_side->m_ConstructPanel->m_FocusedDescription;
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = item_label2;
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_PILON_HEAD) {
                    pElement->SetVisibility(true);
                    if (!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE)) {
                        int res[MAX_RESOURCES];
                        player_side->m_Constructor->GetConstructionPrice(res);
                        for (int i = 0; i < MAX_RESOURCES; i++) {
                            if (g_IFaceList->m_RCountControl->GetCounter())
                                res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                        }
                        bool its_critical = false;
                        if (pElement->m_Param2) {
                            int kind = Float2Int(pElement->m_Param2);
                            if (player_side->GetResourcesAmount(TITAN) < res[TITAN] &&
                                g_Config.m_Price[HEAD1_TITAN + (kind - 1) * 4])
                                its_critical = true;
                            if (player_side->GetResourcesAmount(ELECTRONICS) < res[ELECTRONICS] &&
                                g_Config.m_Price[HEAD1_TITAN + (kind - 1) * 4 + 1])
                                its_critical = true;
                            if (player_side->GetResourcesAmount(ENERGY) < res[ENERGY] &&
                                g_Config.m_Price[HEAD1_TITAN + (kind - 1) * 4 + 2])
                                its_critical = true;
                            if (player_side->GetResourcesAmount(PLASMA) < res[PLASMA] &&
                                g_Config.m_Price[HEAD1_TITAN + (kind - 1) * 4 + 3])
                                its_critical = true;
                        }
                        if (its_critical) {
                            g_IFaceList->CreateElementRamka(pElement, CRITICAL_RAMKA);
                        }
                        else {
                            g_IFaceList->CreateElementRamka(pElement, NORMAL_RAMKA);
                        }
                    }
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_PILON_HULL) {
                    pElement->SetVisibility(true);
                    if (!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE)) {
                        int res[MAX_RESOURCES];
                        player_side->m_Constructor->GetConstructionPrice(res);
                        for (int i = 0; i < MAX_RESOURCES; i++) {
                            if (g_IFaceList->m_RCountControl->GetCounter())
                                res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                        }
                        bool its_critical = false;
                        if (pElement->m_Param2) {
                            int kind = Float2Int(pElement->m_Param2);
                            if (player_side->GetResourcesAmount(TITAN) < res[TITAN] &&
                                g_Config.m_Price[ARMOR1_TITAN + (kind - 1) * 4])
                                its_critical = true;
                            if (player_side->GetResourcesAmount(ELECTRONICS) < res[ELECTRONICS] &&
                                g_Config.m_Price[ARMOR1_TITAN + (kind - 1) * 4 + 1])
                                its_critical = true;
                            if (player_side->GetResourcesAmount(ENERGY) < res[ENERGY] &&
                                g_Config.m_Price[ARMOR1_TITAN + (kind - 1) * 4 + 2])
                                its_critical = true;
                            if (player_side->GetResourcesAmount(PLASMA) < res[PLASMA] &&
                                g_Config.m_Price[ARMOR1_TITAN + (kind - 1) * 4 + 3])
                                its_critical = true;
                        }
                        if (its_critical) {
                            g_IFaceList->CreateElementRamka(pElement, CRITICAL_RAMKA);
                        }
                        else {
                            g_IFaceList->CreateElementRamka(pElement, NORMAL_RAMKA);
                        }
                    }
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_PILON_CHASSIS) {
                    pElement->SetVisibility(true);
                    if (!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE)) {
                        int res[MAX_RESOURCES];
                        player_side->m_Constructor->GetConstructionPrice(res);
                        for (int i = 0; i < MAX_RESOURCES; i++) {
                            if (g_IFaceList->m_RCountControl->GetCounter())
                                res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                        }
                        bool its_critical = false;
                        if (pElement->m_Param2) {
                            int kind = Float2Int(pElement->m_Param2);
                            if (player_side->GetResourcesAmount(TITAN) < res[TITAN] &&
                                g_Config.m_Price[CHASSIS1_TITAN + (kind - 1) * 4])
                                its_critical = true;
                            if (player_side->GetResourcesAmount(ELECTRONICS) < res[ELECTRONICS] &&
                                g_Config.m_Price[CHASSIS1_TITAN + (kind - 1) * 4 + 1])
                                its_critical = true;
                            if (player_side->GetResourcesAmount(ENERGY) < res[ENERGY] &&
                                g_Config.m_Price[CHASSIS1_TITAN + (kind - 1) * 4 + 2])
                                its_critical = true;
                            if (player_side->GetResourcesAmount(PLASMA) < res[PLASMA] &&
                                g_Config.m_Price[CHASSIS1_TITAN + (kind - 1) * 4 + 3])
                                its_critical = true;
                        }
                        if (its_critical) {
                            g_IFaceList->CreateElementRamka(pElement, CRITICAL_RAMKA);
                        }
                        else {
                            g_IFaceList->CreateElementRamka(pElement, NORMAL_RAMKA);
                        }
                    }
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_HISTORY_RIGHT) {
                    if (!g_ConfigHistory->IsNext()) {
                        pElement->SetState(IFACE_DISABLED);
                    }
                    else if (g_ConfigHistory->IsNext() && pElement->GetState() == IFACE_DISABLED) {
                        pElement->SetState(IFACE_NORMAL);
                    }
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_HISTORY_LEFT) {
                    if (!g_ConfigHistory->IsPrev()) {
                        pElement->SetState(IFACE_DISABLED);
                    }
                    else if (g_ConfigHistory->IsPrev() && pElement->GetState() == IFACE_DISABLED) {
                        pElement->SetState(IFACE_NORMAL);
                    }
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_UP) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_DOWN) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_PILON1) {
                    if (common_weapon > 0) {
                        pElement->SetVisibility(true);
                        if (!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE)) {
                            int res[MAX_RESOURCES];
                            player_side->m_Constructor->GetConstructionPrice(res);
                            for (int i = 0; i < MAX_RESOURCES; i++) {
                                if (g_IFaceList->m_RCountControl->GetCounter())
                                    res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                            }
                            bool its_critical = false;
                            if (pElement->m_Param2) {
                                int kind = Float2Int(pElement->m_Param2);
                                if (player_side->GetResourcesAmount(TITAN) < res[TITAN] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4])
                                    its_critical = true;
                                if (player_side->GetResourcesAmount(ELECTRONICS) < res[ELECTRONICS] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4 + 1])
                                    its_critical = true;
                                if (player_side->GetResourcesAmount(ENERGY) < res[ENERGY] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4 + 2])
                                    its_critical = true;
                                if (player_side->GetResourcesAmount(PLASMA) < res[PLASMA] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4 + 3])
                                    its_critical = true;
                            }
                            if (its_critical) {
                                g_IFaceList->CreateElementRamka(pElement, CRITICAL_RAMKA);
                            }
                            else {
                                g_IFaceList->CreateElementRamka(pElement, NORMAL_RAMKA);
                            }
                        }
                    }
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_PILON2) {
                    if (common_weapon > 1) {
                        pElement->SetVisibility(true);
                        if (!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE)) {
                            int res[MAX_RESOURCES];
                            player_side->m_Constructor->GetConstructionPrice(res);
                            for (int i = 0; i < MAX_RESOURCES; i++) {
                                if (g_IFaceList->m_RCountControl->GetCounter())
                                    res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                            }
                            bool its_critical = false;
                            if (pElement->m_Param2) {
                                int kind = Float2Int(pElement->m_Param2);
                                if (player_side->GetResourcesAmount(TITAN) < res[TITAN] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4])
                                    its_critical = true;
                                if (player_side->GetResourcesAmount(ELECTRONICS) < res[ELECTRONICS] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4 + 1])
                                    its_critical = true;
                                if (player_side->GetResourcesAmount(ENERGY) < res[ENERGY] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4 + 2])
                                    its_critical = true;
                                if (player_side->GetResourcesAmount(PLASMA) < res[PLASMA] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4 + 3])
                                    its_critical = true;
                            }
                            if (its_critical) {
                                g_IFaceList->CreateElementRamka(pElement, CRITICAL_RAMKA);
                            }
                            else {
                                g_IFaceList->CreateElementRamka(pElement, NORMAL_RAMKA);
                            }
                        }
                    }
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_PILON3) {
                    if (common_weapon > 2) {
                        pElement->SetVisibility(true);
                        if (!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE)) {
                            int res[MAX_RESOURCES];
                            player_side->m_Constructor->GetConstructionPrice(res);
                            for (int i = 0; i < MAX_RESOURCES; i++) {
                                if (g_IFaceList->m_RCountControl->GetCounter())
                                    res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                            }
                            bool its_critical = false;
                            if (pElement->m_Param2) {
                                int kind = Float2Int(pElement->m_Param2);
                                if (player_side->GetResourcesAmount(TITAN) < res[TITAN] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4])
                                    its_critical = true;
                                if (player_side->GetResourcesAmount(ELECTRONICS) < res[ELECTRONICS] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4 + 1])
                                    its_critical = true;
                                if (player_side->GetResourcesAmount(ENERGY) < res[ENERGY] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4 + 2])
                                    its_critical = true;
                                if (player_side->GetResourcesAmount(PLASMA) < res[PLASMA] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4 + 3])
                                    its_critical = true;
                            }
                            if (its_critical) {
                                g_IFaceList->CreateElementRamka(pElement, CRITICAL_RAMKA);
                            }
                            else {
                                g_IFaceList->CreateElementRamka(pElement, NORMAL_RAMKA);
                            }
                        }
                    }
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_PILON4) {
                    if (common_weapon > 3) {
                        pElement->SetVisibility(true);
                        if (!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE)) {
                            int res[MAX_RESOURCES];
                            player_side->m_Constructor->GetConstructionPrice(res);
                            for (int i = 0; i < MAX_RESOURCES; i++) {
                                if (g_IFaceList->m_RCountControl->GetCounter())
                                    res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                            }
                            bool its_critical = false;
                            if (pElement->m_Param2) {
                                int kind = Float2Int(pElement->m_Param2);
                                if (player_side->GetResourcesAmount(TITAN) < res[TITAN] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4])
                                    its_critical = true;
                                if (player_side->GetResourcesAmount(ELECTRONICS) < res[ELECTRONICS] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4 + 1])
                                    its_critical = true;
                                if (player_side->GetResourcesAmount(ENERGY) < res[ENERGY] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4 + 2])
                                    its_critical = true;
                                if (player_side->GetResourcesAmount(PLASMA) < res[PLASMA] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4 + 3])
                                    its_critical = true;
                            }
                            if (its_critical) {
                                g_IFaceList->CreateElementRamka(pElement, CRITICAL_RAMKA);
                            }
                            else {
                                g_IFaceList->CreateElementRamka(pElement, NORMAL_RAMKA);
                            }
                        }
                    }
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_PILON5) {
                    if (extended) {
                        pElement->SetVisibility(true);
                        if (!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE)) {
                            int res[MAX_RESOURCES];
                            player_side->m_Constructor->GetConstructionPrice(res);
                            for (int i = 0; i < MAX_RESOURCES; i++) {
                                if (g_IFaceList->m_RCountControl->GetCounter())
                                    res[i] *= g_IFaceList->m_RCountControl->GetCounter();
                            }
                            bool its_critical = false;
                            if (pElement->m_Param2) {
                                int kind = Float2Int(pElement->m_Param2);
                                if (player_side->GetResourcesAmount(TITAN) < res[TITAN] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4])
                                    its_critical = true;
                                if (player_side->GetResourcesAmount(ELECTRONICS) < res[ELECTRONICS] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4 + 1])
                                    its_critical = true;
                                if (player_side->GetResourcesAmount(ENERGY) < res[ENERGY] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4 + 2])
                                    its_critical = true;
                                if (player_side->GetResourcesAmount(PLASMA) < res[PLASMA] &&
                                    g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4 + 3])
                                    its_critical = true;
                            }
                            if (its_critical) {
                                g_IFaceList->CreateElementRamka(pElement, CRITICAL_RAMKA);
                            }
                            else {
                                g_IFaceList->CreateElementRamka(pElement, NORMAL_RAMKA);
                            }
                        }
                    }
                } /*else if(player_side->m_ConstructPanel->IsActive() && (pElement->m_strName == IF_BASE_CONFIG1 ||
                 pElement->m_strName == IF_BASE_CONFIG2 || pElement->m_strName == IF_BASE_CONFIG3 || pElement->m_strName
                 == IF_BASE_CONFIG4 || pElement->m_strName == IF_BASE_CONFIG5)){ pElement->SetVisibility(true);
                 }*/
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_Type == IFACE_DYNAMIC_STATIC &&
                         (pElement->m_strName == IF_BASE_TITAN_IMAGE ||
                          pElement->m_strName == IF_BASE_ELECTRONICS_IMAGE ||
                          pElement->m_strName == IF_BASE_ENERGY_IMAGE || pElement->m_strName == IF_BASE_PLASMA_IMAGE)) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_Type == IFACE_DYNAMIC_STATIC &&
                         (pElement->m_strName == IF_BASE_TITAN_SUMM ||
                          pElement->m_strName == IF_BASE_ELECTRONICS_SUMM ||
                          pElement->m_strName == IF_BASE_ENERGY_SUMM || pElement->m_strName == IF_BASE_PLASMA_SUMM)) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_SUMM_PANEL) {
                    pElement->SetVisibility(true);

                    bool tit_color_upd = false;
                    if ((player_side->GetResourcesAmount(TITAN) < titan_summ) && titan_color != 0xFFFF0000) {
                        titan_color = 0xFFFF0000;
                        tit_color_upd = true;
                    }
                    else if ((player_side->GetResourcesAmount(TITAN) >= titan_summ) && titan_color != 0xFFF6c000) {
                        titan_color = 0xFFF6c000;
                        tit_color_upd = true;
                    }
                    if (tit_color_upd && titan_summ) {
                        pElement->m_StateImages[IFACE_NORMAL].m_Color = titan_color;
                        pElement->m_StateImages[IFACE_NORMAL].m_SmeX =
                                (player_side->m_ConstructPanel->m_Configs[cfg_num].m_titX + 25) -
                                Float2Int(pElement->m_xPos);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", titan_summ);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                    }

                    bool elec_color_upd = false;
                    if ((player_side->GetResourcesAmount(ELECTRONICS) < electronics_summ) &&
                        electronics_color != 0xFFFF0000) {
                        electronics_color = 0xFFFF0000;
                        elec_color_upd = true;
                    }
                    else if ((player_side->GetResourcesAmount(ELECTRONICS) >= electronics_summ) &&
                             electronics_color != 0xFFF6c000) {
                        electronics_color = 0xFFF6c000;
                        elec_color_upd = true;
                    }
                    if (elec_color_upd && electronics_summ) {
                        pElement->m_StateImages[IFACE_NORMAL].m_Color = electronics_color;
                        pElement->m_StateImages[IFACE_NORMAL].m_SmeX =
                                (player_side->m_ConstructPanel->m_Configs[cfg_num].m_elecX + 25) -
                                Float2Int(pElement->m_xPos);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", electronics_summ);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                    }

                    bool ener_color_upd = false;
                    if ((player_side->GetResourcesAmount(ENERGY) < energy_summ) && energy_color != 0xFFFF0000) {
                        energy_color = 0xFFFF0000;
                        ener_color_upd = true;
                    }
                    else if ((player_side->GetResourcesAmount(ENERGY) >= energy_summ) && energy_color != 0xFFF6c000) {
                        energy_color = 0xFFF6c000;
                        ener_color_upd = true;
                    }
                    if (ener_color_upd && energy_summ) {
                        pElement->m_StateImages[IFACE_NORMAL].m_Color = energy_color;
                        pElement->m_StateImages[IFACE_NORMAL].m_SmeX =
                                (player_side->m_ConstructPanel->m_Configs[cfg_num].m_enerX + 19) -
                                Float2Int(pElement->m_xPos);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", energy_summ);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                    }

                    bool plas_color_upd = false;
                    if ((player_side->GetResourcesAmount(PLASMA) < plasma_summ) && plasm_color != 0xFFFF0000) {
                        plasm_color = 0xFFFF0000;
                        plas_color_upd = true;
                    }
                    else if ((player_side->GetResourcesAmount(PLASMA) > plasma_summ) && plasm_color != 0xFFF6c000) {
                        plasm_color = 0xFFF6c000;
                        plas_color_upd = true;
                    }
                    if (plas_color_upd && plasma_summ) {
                        pElement->m_StateImages[IFACE_NORMAL].m_Color = plasm_color;
                        pElement->m_StateImages[IFACE_NORMAL].m_SmeX =
                                (player_side->m_ConstructPanel->m_Configs[cfg_num].m_plasX + 24) -
                                Float2Int(pElement->m_xPos);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", plasma_summ);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                    }

                    if (titan_summ != total_res[TITAN] || electronics_summ != total_res[ELECTRONICS] ||
                        energy_summ != total_res[ENERGY] || plasma_summ != total_res[PLASMA]) {
                        titan_summ = total_res[TITAN];
                        electronics_summ = total_res[ELECTRONICS];
                        energy_summ = total_res[ENERGY];
                        plasma_summ = total_res[PLASMA];
                        if (titan_summ != 0) {
                            if (player_side->GetResourcesAmount(TITAN) < titan_summ) {
                                titan_color = 0xFFFF0000;
                            }
                            pElement->m_StateImages[IFACE_NORMAL].m_Color = titan_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX =
                                    (player_side->m_ConstructPanel->m_Configs[cfg_num].m_titX + 25) -
                                    Float2Int(pElement->m_xPos);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", titan_summ);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                        }
                        if (electronics_summ != 0) {
                            if (player_side->GetResourcesAmount(ELECTRONICS) < electronics_summ) {
                                electronics_color = 0xFFFF0000;
                            }

                            pElement->m_StateImages[IFACE_NORMAL].m_Color = electronics_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX =
                                    (player_side->m_ConstructPanel->m_Configs[cfg_num].m_elecX + 25) -
                                    Float2Int(pElement->m_xPos);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", electronics_summ);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                        }
                        if (energy_summ != 0) {
                            if (player_side->GetResourcesAmount(ENERGY) < energy_summ) {
                                energy_color = 0xFFFF0000;
                            }
                            pElement->m_StateImages[IFACE_NORMAL].m_Color = energy_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX =
                                    (player_side->m_ConstructPanel->m_Configs[cfg_num].m_enerX + 19) -
                                    Float2Int(pElement->m_xPos);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", energy_summ);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                        }
                        if (plasma_summ != 0) {
                            if (player_side->GetResourcesAmount(PLASMA) < plasma_summ) {
                                plasm_color = 0xFFFF0000;
                            }
                            pElement->m_StateImages[IFACE_NORMAL].m_Color = plasm_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX =
                                    (player_side->m_ConstructPanel->m_Configs[cfg_num].m_plasX + 24) -
                                    Float2Int(pElement->m_xPos);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", plasma_summ);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                        }
                    }
                }
                else if (player_side->m_ConstructPanel->IsActive() && player_side->m_ConstructPanel->m_FocusedElement &&
                         pElement->m_strName == IF_BASE_UNIT_PANEL) {
                    int start = 0;
                    int *unit_res = NULL;
                    int kind = Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param2);
                    if (kind)
                        pElement->SetVisibility(true);

                    if (ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                        MRT_WEAPON) {
                        start = WEAPON1_TITAN;
                    }
                    else if (ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                             MRT_ARMOR) {
                        start = ARMOR1_TITAN;
                    }
                    else if (ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                             MRT_HEAD) {
                        start = HEAD1_TITAN;
                    }
                    else if (ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                             MRT_CHASSIS) {
                        start = CHASSIS1_TITAN;
                    }

                    unit_res = &g_Config.m_Price[start + (kind - 1) * 4];

                    bool tit_color_upd = false;
                    if ((player_side->GetResourcesAmount(TITAN) < total_res[TITAN]) && titan_unit_color != 0xFFFF0000) {
                        titan_unit_color = 0xFFFF0000;
                        tit_color_upd = true;
                    }
                    else if ((player_side->GetResourcesAmount(TITAN) >= total_res[TITAN]) &&
                             titan_unit_color != 0xFFF6c000) {
                        titan_unit_color = 0xFFF6c000;
                        tit_color_upd = true;
                    }
                    if (tit_color_upd) {
                        pElement->m_StateImages[IFACE_NORMAL].m_Color = titan_unit_color;
                        pElement->m_StateImages[IFACE_NORMAL].m_SmeX =
                                (player_side->m_ConstructPanel->m_ftitX + 25) - Float2Int(pElement->m_xPos);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", titan_unit);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                    }

                    bool elec_color_upd = false;
                    if ((player_side->GetResourcesAmount(ELECTRONICS) < total_res[ELECTRONICS]) &&
                        electronics_unit_color != 0xFFFF0000) {
                        electronics_unit_color = 0xFFFF0000;
                        elec_color_upd = true;
                    }
                    else if ((player_side->GetResourcesAmount(ELECTRONICS) >= total_res[ELECTRONICS]) &&
                             electronics_unit_color != 0xFFF6c000) {
                        electronics_unit_color = 0xFFF6c000;
                        elec_color_upd = true;
                    }
                    if (elec_color_upd) {
                        pElement->m_StateImages[IFACE_NORMAL].m_Color = electronics_unit_color;
                        pElement->m_StateImages[IFACE_NORMAL].m_SmeX =
                                (player_side->m_ConstructPanel->m_felecX + 25) - Float2Int(pElement->m_xPos);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", electronics_unit);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                    }

                    bool ener_color_upd = false;
                    if ((player_side->GetResourcesAmount(ENERGY) < total_res[ENERGY]) &&
                        energy_unit_color != 0xFFFF0000) {
                        energy_unit_color = 0xFFFF0000;
                        ener_color_upd = true;
                    }
                    else if ((player_side->GetResourcesAmount(ENERGY) >= total_res[ENERGY]) &&
                             energy_unit_color != 0xFFF6c000) {
                        energy_unit_color = 0xFFF6c000;
                        ener_color_upd = true;
                    }
                    if (ener_color_upd) {
                        pElement->m_StateImages[IFACE_NORMAL].m_Color = energy_unit_color;
                        pElement->m_StateImages[IFACE_NORMAL].m_SmeX =
                                (player_side->m_ConstructPanel->m_fenerX + 19) - Float2Int(pElement->m_xPos);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", energy_unit);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                    }

                    bool plas_color_upd = false;
                    if ((player_side->GetResourcesAmount(PLASMA) < total_res[PLASMA]) &&
                        plasm_unit_color != 0xFFFF0000) {
                        plasm_unit_color = 0xFFFF0000;
                        plas_color_upd = true;
                    }
                    else if ((player_side->GetResourcesAmount(PLASMA) > total_res[PLASMA]) &&
                             plasm_unit_color != 0xFFF6c000) {
                        plasm_unit_color = 0xFFF6c000;
                        plas_color_upd = true;
                    }
                    if (plas_color_upd) {
                        pElement->m_StateImages[IFACE_NORMAL].m_Color = plasm_unit_color;
                        pElement->m_StateImages[IFACE_NORMAL].m_SmeX =
                                (player_side->m_ConstructPanel->m_fplasX + 24) - Float2Int(pElement->m_xPos);
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", plasma_unit);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                    }

                    if (titan_unit != unit_res[TITAN] || electronics_unit != unit_res[ELECTRONICS] ||
                        energy_unit != unit_res[ENERGY] || plasma_unit != unit_res[PLASMA]) {
                        titan_unit = unit_res[TITAN];
                        electronics_unit = unit_res[ELECTRONICS];
                        energy_unit = unit_res[ENERGY];
                        plasma_unit = unit_res[PLASMA];

                        if (titan_unit != 0) {
                            pElement->m_StateImages[IFACE_NORMAL].m_Color = titan_unit_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX =
                                    (player_side->m_ConstructPanel->m_ftitX + 25) - Float2Int(pElement->m_xPos);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", titan_unit);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                        }
                        else {
                            pElement->m_StateImages[IFACE_NORMAL].m_Color = titan_unit_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX = 0;
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = std::wstring(L"");
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                        }

                        if (electronics_unit != 0) {
                            pElement->m_StateImages[IFACE_NORMAL].m_Color = electronics_unit_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX =
                                    (player_side->m_ConstructPanel->m_felecX + 25) - Float2Int(pElement->m_xPos);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", electronics_unit);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                        }

                        if (energy_unit != 0) {
                            pElement->m_StateImages[IFACE_NORMAL].m_Color = energy_unit_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX =
                                    (player_side->m_ConstructPanel->m_fenerX + 19) - Float2Int(pElement->m_xPos);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", energy_unit);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                        }

                        if (plasma_unit != 0) {
                            pElement->m_StateImages[IFACE_NORMAL].m_Color = plasm_unit_color;
                            pElement->m_StateImages[IFACE_NORMAL].m_SmeX =
                                    (player_side->m_ConstructPanel->m_fplasX + 24) - Float2Int(pElement->m_xPos);
                            pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", plasma_unit);
                            pElement->m_StateImages[IFACE_NORMAL].SetStateText(false);
                        }
                    }
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_STRUCTURE) {
                    pElement->SetVisibility(true);
                    if (structure != player_side->m_Constructor->GetConstructionStructure()) {
                        structure = player_side->m_Constructor->GetConstructionStructure();
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", structure);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_DAMAGE) {
                    pElement->SetVisibility(true);

                    int wep = 0;
                    for (int cnt = 0; cnt < MAX_WEAPON_CNT; cnt++) {
                        if (player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[cnt].m_nKind != 0) {
                            wep++;
                            break;
                        }
                    }
                    if (wep == 0) {
                        damage = 0;
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", damage);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                    else if (damage != GetConstructionDamage(player_side->m_Constructor->GetRenderBot())) {
                        damage = GetConstructionDamage(player_side->m_Constructor->GetRenderBot());
                        pElement->m_StateImages[IFACE_NORMAL].m_Caption = utils::format(L"%d", damage);
                        pElement->m_StateImages[IFACE_NORMAL].SetStateText(true);
                    }
                }
                else if (player_side->m_ConstructPanel->IsActive() && pElement->m_strName == IF_BASE_ITEM_PRICE &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 != 0) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_HEAD1_ST || pElement->m_strName == IF_BASE_IHE_TEXT ||
                          pElement->m_strName == IF_BASE_IHE1_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_HEAD &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 1) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_HEAD2_ST || pElement->m_strName == IF_BASE_IHE_TEXT ||
                          pElement->m_strName == IF_BASE_IHE2_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_HEAD &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 2) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_HEAD3_ST || pElement->m_strName == IF_BASE_IHE_TEXT ||
                          pElement->m_strName == IF_BASE_IHE3_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_HEAD &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 3) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_HEAD4_ST || pElement->m_strName == IF_BASE_IHE_TEXT ||
                          pElement->m_strName == IF_BASE_IHE4_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_HEAD &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 4) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_HULL1_ST || pElement->m_strName == IF_BASE_IHU_TEXT ||
                          pElement->m_strName == IF_BASE_IHU1_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_ARMOR &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 1) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_HULL2_ST || pElement->m_strName == IF_BASE_IHU_TEXT ||
                          pElement->m_strName == IF_BASE_IHU2_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_ARMOR &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 2) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_HULL3_ST || pElement->m_strName == IF_BASE_IHU_TEXT ||
                          pElement->m_strName == IF_BASE_IHU3_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_ARMOR &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 3) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_HULL4_ST || pElement->m_strName == IF_BASE_IHU_TEXT ||
                          pElement->m_strName == IF_BASE_IHU4_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_ARMOR &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 4) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_HULL5_ST || pElement->m_strName == IF_BASE_IHU_TEXT ||
                          pElement->m_strName == IF_BASE_IHU5_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_ARMOR &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 5) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_HULL6_ST || pElement->m_strName == IF_BASE_IHU_TEXT ||
                          pElement->m_strName == IF_BASE_IHU6_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_ARMOR &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 6) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_CHAS1_ST || pElement->m_strName == IF_BASE_ICH_TEXT ||
                          pElement->m_strName == IF_BASE_ICH1_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_CHASSIS &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 1) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_CHAS2_ST || pElement->m_strName == IF_BASE_ICH_TEXT ||
                          pElement->m_strName == IF_BASE_ICH2_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_CHASSIS &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 2) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_CHAS3_ST || pElement->m_strName == IF_BASE_ICH_TEXT ||
                          pElement->m_strName == IF_BASE_ICH3_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_CHASSIS &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 3) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_CHAS4_ST || pElement->m_strName == IF_BASE_ICH_TEXT ||
                          pElement->m_strName == IF_BASE_ICH4_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_CHASSIS &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 4) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_CHAS5_ST || pElement->m_strName == IF_BASE_ICH_TEXT ||
                          pElement->m_strName == IF_BASE_ICH5_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_CHASSIS &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 5) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_WEAPON1_ST || pElement->m_strName == IF_BASE_IW_TEXT ||
                          pElement->m_strName == IF_BASE_IW1_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_WEAPON &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 1) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_WEAPON2_ST || pElement->m_strName == IF_BASE_IW_TEXT ||
                          pElement->m_strName == IF_BASE_IW2_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_WEAPON &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 2) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_WEAPON3_ST || pElement->m_strName == IF_BASE_IW_TEXT ||
                          pElement->m_strName == IF_BASE_IW3_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_WEAPON &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 3) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_WEAPON4_ST || pElement->m_strName == IF_BASE_IW_TEXT ||
                          pElement->m_strName == IF_BASE_IW4_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_WEAPON &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 4) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_WEAPON5_ST || pElement->m_strName == IF_BASE_IW_TEXT ||
                          pElement->m_strName == IF_BASE_IW5_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_WEAPON &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 5) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_WEAPON6_ST || pElement->m_strName == IF_BASE_IW_TEXT ||
                          pElement->m_strName == IF_BASE_IW6_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_WEAPON &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 6) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_WEAPON7_ST || pElement->m_strName == IF_BASE_IW_TEXT ||
                          pElement->m_strName == IF_BASE_IW7_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_WEAPON &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 7) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_WEAPON8_ST || pElement->m_strName == IF_BASE_IW_TEXT ||
                          pElement->m_strName == IF_BASE_IW8_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_WEAPON &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 8) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_WEAPON9_ST || pElement->m_strName == IF_BASE_IW_TEXT ||
                          pElement->m_strName == IF_BASE_IW9_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_WEAPON &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 9) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         (pElement->m_strName == IF_BASE_WEAPON10_ST || pElement->m_strName == IF_BASE_IW_TEXT ||
                          pElement->m_strName == IF_BASE_IW10_TEXT) &&
                         player_side->m_ConstructPanel->m_FocusedElement &&
                         ERobotUnitType(Float2Int(player_side->m_ConstructPanel->m_FocusedElement->m_Param1)) ==
                                 MRT_WEAPON &&
                         player_side->m_ConstructPanel->m_FocusedElement->m_Param2 == 10) {
                    pElement->SetVisibility(true);
                }
                else if (player_side->m_ConstructPanel->IsActive() &&
                         pElement == g_IFaceList->m_RCountControl->GetImage()) {
                    pElement->SetVisibility(true);
                }
            }
            pElement = pElement->m_NextElement;
        }
    }
    else if (m_strName == IF_HINTS) {
        m_VisibleAlpha = IS_VISIBLEA;
        CIFaceElement *e = m_FirstElement;
        while (e) {
            // e->SetVisibility(true);
            e = e->m_NextElement;
        }
    }
    else if (m_strName == IF_POPUP_MENU && FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE)) {
        m_VisibleAlpha = IS_VISIBLEA;
        CIFaceElement *pElement = m_FirstElement;
        while (pElement) {
            if (pElement->m_strName == IF_POPUP_RAMKA) {
                pElement->SetVisibility(true);
            }
            else if((pElement->m_strName == IF_POPUP_SELECTOR/* && g_IFaceList->m_FocusedElement && g_IFaceList->m_FocusedElement->m_nId == POPUP_SELECTOR_CATCHERS_ID*/)){
                pElement->SetVisibility(true);
            }
            else if (pElement->m_nId == POPUP_SELECTOR_CATCHERS_ID) {
                pElement->SetVisibility(true);
            }
            else if (pElement->m_strName == IF_POPUP_CURSOR) {
                pElement->SetVisibility(true);
            }
            pElement = pElement->m_NextElement;
        }
    }
}

void CInterface::SortElementsByZ() {
    CIFaceElement *elements = NULL, *el_cur = NULL, *el_plus_one = NULL, *el_prev = NULL, *el_next = NULL;

    int sorting;
    do {
        elements = m_FirstElement;
        sorting = 0;
        while (elements && elements->m_NextElement) {
            if (elements->m_zPos < elements->m_NextElement->m_zPos) {
                sorting = 1;

                el_cur = elements;
                el_plus_one = elements->m_NextElement;

                if (el_plus_one->m_NextElement == NULL) {
                    m_LastElement = el_cur;
                    el_cur->m_NextElement = NULL;
                }
                else {
                    el_cur->m_NextElement = el_plus_one->m_NextElement;
                    el_plus_one->m_NextElement->m_PrevElement = el_cur;
                }

                if (elements->m_PrevElement == NULL) {
                    m_FirstElement = el_plus_one;
                    el_plus_one->m_PrevElement = NULL;
                }
                else {
                    el_plus_one->m_PrevElement = elements->m_PrevElement;
                    elements->m_PrevElement->m_NextElement = el_plus_one;
                }

                el_plus_one->m_NextElement = el_cur;
                el_cur->m_PrevElement = el_plus_one;
            }

            elements = elements->m_NextElement;
        }
    }
    while (sorting);
}

void CInterface::CopyElements(CIFaceElement *el_src, CIFaceElement *el_dest) {
    for (int state = 0; state < MAX_STATES; state++) {
        el_dest->m_StateImages[state].pImage = el_src->m_StateImages[state].pImage;
        el_dest->m_StateImages[state].xTexPos = el_src->m_StateImages[state].xTexPos;
        el_dest->m_StateImages[state].yTexPos = el_src->m_StateImages[state].yTexPos;
        for (int cnt = 0; cnt < 4; cnt++) {
            el_dest->m_StateImages[state].m_Geom[cnt].tu = el_src->m_StateImages[state].m_Geom[cnt].tu;
            el_dest->m_StateImages[state].m_Geom[cnt].tv = el_src->m_StateImages[state].m_Geom[cnt].tv;
        }
    }
    el_dest->m_Param1 = el_src->m_Param1;
    el_dest->m_Param2 = el_src->m_Param2;

    std::copy(
        std::begin(el_src->m_Actions),
        std::end(el_src->m_Actions),
        std::begin(el_dest->m_Actions)
    );
}

CIFaceImage *CInterface::FindImageByName(std::wstring name) {
    CIFaceImage *images = m_FirstImage;
    while (images) {
        if (images->m_strName == name)
            return images;
        images = images->m_NextImage;
    }
    return NULL;
}

CIFaceStatic *CInterface::CreateStaticFromImage(float x, float y, float z, const CIFaceImage &image, bool fullsize) {
    DTRACE();
    CIFaceStatic *stat = HNew(g_MatrixHeap) CIFaceStatic;

    stat->m_strName = image.m_strName;
    stat->m_xPos = x;
    stat->m_yPos = y;
    stat->m_zPos = z;
    stat->m_xSize = image.m_Width;
    stat->m_ySize = image.m_Height;
    stat->m_DefState = IFACE_NORMAL;

    stat->SetStateImage(IFACE_NORMAL, image.m_Image, image.m_xTexPos, image.m_yTexPos, image.m_TexWidth,
                        image.m_TexHeight);

    stat->ElementGeomInit((void *)stat, fullsize);
    stat->m_Type = IFACE_DYNAMIC_STATIC;

    AddElement(stat);
    return stat;
}

void CInterface::LogicTakt(int ms) {
    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
    if (g_IFaceList->m_FocusedInterface == this) {
        if (FLAG(m_InterfaceFlags, (INTERFACE_SLIDE_LEFT | INTERFACE_SLIDE_RIGHT))) {
            SlideStep();
        }
        // if(!ps->IsArcadeMode() && (GetAsyncKeyState(g_MatrixMap->m_Config.m_KeyActions[KA_UNIT_LEFT]) &
        // 0x8000)==0x8000){
        //    MoveLeft();
        //    ReCalcElementsPos();
        //}
        // if(!ps->IsArcadeMode() && (GetAsyncKeyState(g_MatrixMap->m_Config.m_KeyActions[KA_UNIT_RIGHT]) &
        // 0x8000)==0x8000){
        //    MoveRight();
        //    ReCalcElementsPos();
        //}
        // if(!ps->IsArcadeMode() && (GetAsyncKeyState(g_MatrixMap->m_Config.m_KeyActions[KA_UNIT_FORWARD]) &
        // 0x8000)==0x8000){
        //    MoveUp();
        //    ReCalcElementsPos();
        //}
        // if(!ps->IsArcadeMode() && (GetAsyncKeyState(g_MatrixMap->m_Config.m_KeyActions[KA_UNIT_BACKWARD]) &
        // 0x8000)==0x8000){
        //    MoveDown();
        //    ReCalcElementsPos();
        //}
    }

    CIFaceElement *els = m_FirstElement;

    while (els) {
        els->LogicTakt(ms);
        els = els->m_NextElement;
    }
}

void CInterface::MoveLeft() {
    m_xPos -= 5.0f;
}

void CInterface::MoveRight() {
    m_xPos += 5.0f;
}

void CInterface::MoveUp() {
    m_yPos -= 5.0f;
}

void CInterface::MoveDown() {
    m_yPos += 5.0f;
}

void CInterface::ReCalcElementsPos() {
    DTRACE();

    CIFaceElement *pElement = m_FirstElement;

    if (m_strName == IF_MAIN) {
        g_IFaceList->SetMainPos(m_xPos, m_yPos);
    }
    while (pElement) {
        pElement->RecalcPos(m_xPos, m_yPos);
        pElement = pElement->m_NextElement;
    }
}

void CInterface::BeginSlide(float to_x, float to_y) {
    ZeroMemory(&m_Slider, sizeof(SSlide));
    if (to_x != m_xPos) {
        RESETFLAG(m_InterfaceFlags, (INTERFACE_SLIDE_LEFT | INTERFACE_SLIDE_RIGHT));
        if (to_x < m_xPos) {
            SETFLAG(m_InterfaceFlags, INTERFACE_SLIDE_LEFT);
            m_Slider.startX = m_xPos;
            m_Slider.startY = m_yPos;
            m_Slider.stopX = to_x;
            m_Slider.stopY = to_y;
            m_Slider.startLength = m_xPos - to_x;
        }
        else {
            SETFLAG(m_InterfaceFlags, INTERFACE_SLIDE_RIGHT);
            m_Slider.startX = m_xPos;
            m_Slider.startY = m_yPos;
            m_Slider.stopX = to_x;
            m_Slider.stopY = to_y;
            m_Slider.startLength = to_x - m_xPos;
        }
    }
    else if (to_y != m_yPos) {
    }
}

void CInterface::SlideStep() {
    if (FLAG(m_InterfaceFlags, INTERFACE_SLIDE_LEFT)) {
        if (m_Slider.step > 1.0f) {
            RESETFLAG(m_InterfaceFlags, INTERFACE_SLIDE_LEFT);
            ZeroMemory(&m_Slider, sizeof(SSlide));
            return;
        }
    }
    else if (FLAG(m_InterfaceFlags, INTERFACE_SLIDE_RIGHT)) {
        if (m_Slider.step > SLIDE_FUNC_PARAM) {
            RESETFLAG(m_InterfaceFlags, INTERFACE_SLIDE_RIGHT);
            ZeroMemory(&m_Slider, sizeof(SSlide));
            return;
        }

        float speed = 0;
        float x = m_Slider.step;
        float b = SLIDE_FUNC_PARAM;
        float c = 0;

        speed = (-(x * x)) + (b * x) + c;
        if (speed < ((-(SLIDE_STEP_SIZE * SLIDE_STEP_SIZE)) + (b * SLIDE_STEP_SIZE) + c)) {
            speed = 0;
        }

        m_Slider.step += SLIDE_STEP_SIZE;
        float half_way = (m_Slider.startX + (m_Slider.startLength * 0.5f));

        // if(m_xPos >= half_way){
        //    m_xPos = LERPFLOAT(speed, m_Slider.stopX, half_way);
        //}else{
        //    m_xPos = LERPFLOAT(speed, m_Slider.startX, half_way);
        //}

        m_xPos += LERPFLOAT(speed, 0, SLIDE_MAX_SPEED);
        ReCalcElementsPos();
    }
}

bool CInterface::FindElementByName(const std::wstring &name) {
    CIFaceElement *elements = m_FirstElement;
    while (elements) {
        if (elements->m_strName == name) {
            return true;
        }
        elements = elements->m_NextElement;
    }
    return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
CIFaceList::CIFaceList() : m_CurrentHintControlName{} {
    m_First = NULL;
    m_Last = NULL;
    m_IfListFlags = 0;
    m_FocusedInterface = NULL;
    m_FocusedElement = NULL;

    ZeroMemory(m_Chassis, sizeof(m_Chassis));
    ZeroMemory(m_Armor, sizeof(m_Armor));
    ZeroMemory(m_Head, sizeof(m_Head));
    ZeroMemory(m_Weapon, sizeof(m_Weapon));
    ZeroMemory(m_WeaponPilon, sizeof(m_WeaponPilon));
    ZeroMemory(m_Turrets, sizeof(m_Turrets));

    m_ChassisPilon = NULL;
    m_ArmorPilon = NULL;
    m_HeadPilon = NULL;
    m_BuildCa = NULL;

    CInterface::m_ClearRects = HNew(g_MatrixHeap) CBuf();

    m_RCountControl = HNew(g_MatrixHeap) CIFaceCounter;
    m_CurrentHint = NULL;

    m_DynamicTY = 153;
    m_DynamicTX[0] = 280;
    m_DynamicTX[1] = 262;
    m_DynamicTX[2] = 304;
    m_DynamicTX[3] = 242;
    m_DynamicTX[4] = 279;
    m_DynamicTX[5] = 316;
    m_DynamicTX[6] = 231;
    m_DynamicTX[7] = 265;
    m_DynamicTX[8] = 299;
    m_DynamicTX[9] = 333;

    m_DWeaponX[0] = 243;
    m_DWeaponY[0] = 106;
    m_DWeaponX[1] = 283;
    m_DWeaponY[1] = 106;
    m_DWeaponX[2] = 243;
    m_DWeaponY[2] = 65;
    m_DWeaponX[3] = 283;
    m_DWeaponY[3] = 65;
    m_DWeaponX[4] = 323;
    m_DWeaponY[4] = 65;

    ZeroMemory(m_Pilons, sizeof(m_Pilons));

    // h1 0 - 4
    m_Pilons[0] = 21;
    m_Pilons[4] = 20;
    // h2 5 - 9
    m_Pilons[5] = 20;
    m_Pilons[6] = 22;
    // h3 10 - 14
    m_Pilons[10] = 20;
    m_Pilons[11] = 22;
    m_Pilons[14] = 21;
    // h4 15 - 19
    m_Pilons[15] = 20;
    m_Pilons[16] = 21;
    m_Pilons[17] = 22;
    // h5 20 - 24
    m_Pilons[20] = 21;
    m_Pilons[21] = 20;
    m_Pilons[22] = 23;
    m_Pilons[23] = 22;
    m_Pilons[24] = 24;
    // h6 25 - 29
    m_Pilons[25] = 20;
}

CIFaceList::~CIFaceList() {
    if (m_RCountControl) {
        HDelete(CIFaceCounter, m_RCountControl, g_MatrixHeap);
    }

    while (m_First != NULL) {
        if (m_First->m_NextInterface)
            m_First = m_First->m_NextInterface;
        else {
            HDelete(CInterface, m_First, g_MatrixHeap);
            m_First = NULL;
            m_Last = NULL;
        }

        if (m_First)
            HDelete(CInterface, m_First->m_PrevInterface, g_MatrixHeap);
    }

    if (CInterface::m_ClearRects)
        HDelete(CBuf, CInterface::m_ClearRects, g_MatrixHeap);
    if (g_IFaceList->m_CurrentHint)
        g_IFaceList->m_CurrentHint->Release();
    g_IFaceList->m_CurrentHint = NULL;
}

void CIFaceList::ShowInterface() {
    DTRACE();
    CInterface *pIFace = m_First;
    while (pIFace) {
        pIFace->Init();
        if (m_CurrentHint) {
            CIFaceElement *els = pIFace->m_FirstElement;
            while (els) {
                if (els->m_strName == m_CurrentHintControlName) {
                    if (!els->GetVisibility()) {
                        m_CurrentHint->Release();
                        m_CurrentHint = NULL;
                        m_CurrentHintControlName = L"";
                    }
                    break;
                }
                els = els->m_NextElement;
            }
        }
        pIFace = pIFace->m_NextInterface;
    }
    if (g_PopupMenu) {
        g_PopupMenu->m_MenuGraphics->Init();
    }
}

void CIFaceList::BeforeRender(void) {
    DTRACE();

    CInterface::ClearRects_Clear();

    DCP();

    CInterface *pIFace = m_First;
    while (pIFace) {
        DCP();
        if (pIFace->m_VisibleAlpha) {
            DCP();

            pIFace->BeforeRender();
        }
        DCP();
        pIFace = pIFace->m_NextInterface;
    }
    DCP();
    if (g_PopupMenu) {
        DCP();
        g_PopupMenu->m_MenuGraphics->BeforeRender();
    }
    DCP();
}

void CIFaceList::Render() {
    DTRACE();
    CInterface *pIFace = m_First;
    while (pIFace) {
        if (pIFace->m_VisibleAlpha) {
            pIFace->Render();
        }
        pIFace = pIFace->m_NextInterface;
    }
    if (g_PopupMenu) {
        g_PopupMenu->m_MenuGraphics->Render();
    }
}

bool CIFaceList::OnMouseLBDown() {
    DTRACE();

    bool bCatch = FALSE;
    CInterface *pIFace = m_First;
    while (pIFace) {
        if (pIFace->m_VisibleAlpha) {
            if (pIFace->OnMouseLBDown()) {
                bCatch = TRUE;
            }
        }
        pIFace = pIFace->m_NextInterface;
    }

    if (g_PopupMenu) {
        if (g_PopupMenu->m_MenuGraphics->OnMouseLBDown()) {
            bCatch = true;
        }
    }

    return bCatch;
}

bool CIFaceList::OnMouseRBDown() {
    DTRACE();

    if (FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE) && g_PopupMenu) {
        g_PopupMenu->ResetMenu(true);
        return false;
    }

    bool bCatch = FALSE;
    CInterface *pIFace = m_First;
    while (pIFace) {
        if (pIFace->m_VisibleAlpha) {
            if (pIFace->OnMouseRBDown()) {
                bCatch = TRUE;
            }
        }
        pIFace = pIFace->m_NextInterface;
    }
    return bCatch;
}

bool CIFaceList::OnMouseMove(CPoint mouse) {
    DTRACE();

    bool bCatch = FALSE;
    CInterface *pIFace = m_First;
    while (pIFace) {
        if (pIFace->m_VisibleAlpha && pIFace->OnMouseMove(mouse)) {
            bCatch = TRUE;
            m_FocusedInterface = pIFace;
        }
        pIFace = pIFace->m_NextInterface;
    }
    if (g_PopupMenu) {
        if (g_PopupMenu->m_MenuGraphics->m_VisibleAlpha && g_PopupMenu->m_MenuGraphics->OnMouseMove(mouse)) {
            bCatch = true;
            m_FocusedInterface = g_PopupMenu->m_MenuGraphics;
        }
    }

    return bCatch;
}

void CIFaceList::OnMouseLBUp() {
    DTRACE();
    CInterface *pIFace = m_First;
    while (pIFace) {
        if (pIFace->m_VisibleAlpha) {
            pIFace->OnMouseLBUp();
        }
        pIFace = pIFace->m_NextInterface;
    }
    if (g_PopupMenu) {
        g_PopupMenu->m_MenuGraphics->OnMouseLBUp();
    }
}
void CIFaceList::LogicTakt(int ms) {
    g_IFaceList->m_InFocus = UNKNOWN;
    if (g_IFaceList->OnMouseMove(g_MatrixMap->m_Cursor.GetPos())) {
        g_IFaceList->m_InFocus = INTERFACE;
    }

    CInterface *i = m_First;
    while (i) {
        i->LogicTakt(ms);
        i = i->m_NextInterface;
    }
    if (g_PopupMenu) {
        g_PopupMenu->m_MenuGraphics->LogicTakt(ms);
    }
    ShowInterface();

    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();

    // Cursor logic
    if (/*если мы в аркадном режиме*/ ps->IsArcadeMode() && ps->GetArcadedObject()->IsLiveRobot()) {
        if (/*если курсор на интерфейсе*/ g_IFaceList->m_InFocus == INTERFACE) {
            //устанавливаем курсор ARROW
            g_MatrixMap->m_Cursor.Select(CURSOR_ARROW);
        }
        else {
            if (/*если чтолибо вражеское под курсором*/ g_MatrixMap->IsTraceNonPlayerObj()) {
                if (/*оружие достреливает*/ ((CMatrixRobotAI *)ps->GetArcadedObject())
                            ->CheckFireDist(g_MatrixMap->m_TraceStopPos)) {
                    //устанавливаем курсор CROSS_RED
                    g_MatrixMap->m_Cursor.Select(CURSOR_CROSS_RED);
                }
                else {
                    //устанавливаем курсор CROSS_YELLOW
                    g_MatrixMap->m_Cursor.Select(CURSOR_CROSS_YELLOW);
                }
            }
            else {
                //устанавливаем курсор CROSS_BLUE
                g_MatrixMap->m_Cursor.Select(CURSOR_CROSS_BLUE);
            }
        }
    }
    else {
        CPoint mp = g_MatrixMap->m_Cursor.GetPos();

        if ((mp.x >= 0 && mp.x < g_ScreenX && mp.y >= 0 && mp.y < g_ScreenY) &&
            ((mp.x < MOUSE_BORDER) || (mp.x > (g_ScreenX - MOUSE_BORDER)) || (mp.y < MOUSE_BORDER) ||
             (mp.y > (g_ScreenY - MOUSE_BORDER)))) {
            g_MatrixMap->m_Cursor.Select(CURSOR_STAR);
        }
        else {
            if (/*приказ АТАКОВАТЬ*/ FLAG(g_IFaceList->m_IfListFlags, PREORDER_FIRE | PREORDER_BOMB)) {
                if (/*под прицелом находится вражеский объект*/ g_MatrixMap->IsTraceNonPlayerObj()) {
                    //устанавливаем курсор CROSS_RED
                    g_MatrixMap->m_Cursor.Select(CURSOR_CROSS_RED);
                }
                else {
                    //устанавливаем курсор CROSS_BLUE
                    g_MatrixMap->m_Cursor.Select(CURSOR_CROSS_BLUE);
                }
            }
            else if (/*приказ захватывать*/ FLAG(g_IFaceList->m_IfListFlags, PREORDER_CAPTURE)) {
                if (/*под прицелом находится не игроковское здание*/ IS_TRACE_STOP_OBJECT(
                            g_MatrixMap->m_TraceStopObj) &&
                    g_MatrixMap->m_TraceStopObj->GetObjectType() == OBJECT_TYPE_BUILDING &&
                    g_MatrixMap->m_TraceStopObj->GetSide() != PLAYER_SIDE) {
                    //устанавливаем курсор CROSS_RED
                    g_MatrixMap->m_Cursor.Select(CURSOR_CROSS_RED);
                }
                else {
                    //устанавливаем курсор CROSS_BLUE
                    g_MatrixMap->m_Cursor.Select(CURSOR_CROSS_BLUE);
                }
            }
            else if (/*приказ патруль или идти*/ FLAG(g_IFaceList->m_IfListFlags,
                                                      PREORDER_MOVE | PREORDER_PATROL | PREORDER_REPAIR)) {
                //устанавливаем курсор CROSS_BLUE
                g_MatrixMap->m_Cursor.Select(CURSOR_CROSS_BLUE);
            }
            else {
                //устанавливаем курсор ARROW
                g_MatrixMap->m_Cursor.Select(CURSOR_ARROW);
            }
        }
    }
}

void CIFaceList::CreateWeaponDynamicStatics() {
    DTRACE();
    DeleteWeaponDynamicStatics();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();

    if (!FLAG(m_IfListFlags, SINGLE_MODE) && !player_side->IsArcadeMode())
        return;
    if ((player_side->GetCurGroup()->m_FirstObject &&
         player_side->GetCurGroup()->m_FirstObject->GetObject()->GetObjectType() != OBJECT_TYPE_ROBOTAI))
        return;

    CMatrixRobotAI *bot = NULL;

    if (player_side->IsArcadeMode()) {
        CMatrixMapStatic *s = player_side->GetArcadedObject();
        if (s && s->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
            bot = (CMatrixRobotAI *)s;
        }
        else {
            return;
        }
    }
    else {
        bot = (CMatrixRobotAI *)player_side->GetCurGroup()->m_FirstObject->GetObject();
    }

    CInterface *interfaces = m_First;

    while (interfaces) {
        if (interfaces->m_strName == IF_MAIN) {
            int hu = 0;
            for (int i = 0; i < MR_MAXUNIT; i++) {
                if (bot->m_Unit[i].m_Type == MRT_ARMOR) {
                    hu = int(bot->m_Unit[i].u1.s1.m_Kind);
                }
            }
            if (!hu)
                return;
            hu = hu - 1;

            CIFaceImage overheat_image = *interfaces->FindImageByName(std::wstring(IF_OVER_HEAT));
            for (int i = 0; i < MR_MAXUNIT; i++) {
                if (bot->m_Unit[i].m_Type == MRT_WEAPON) {
                    int pos;
                    for (pos = 0; pos < 5; pos++) {
                        if (g_IFaceList->m_Pilons[pos + hu * 5] == bot->m_Unit[i].u1.s1.m_LinkMatrix) {
                            break;
                        }
                    }

                    std::wstring name;
                    switch (bot->m_Unit[i].u1.s1.m_Kind) {
                        case RUK_WEAPON_MACHINEGUN:
                            name = std::wstring(IF_WEAPON_MACHINEGUN_ON);
                            break;
                        case RUK_WEAPON_CANNON:
                            name = std::wstring(IF_WEAPON_CANNON_ON);
                            break;
                        case RUK_WEAPON_LASER:
                            name = std::wstring(IF_WEAPON_LASER_ON);
                            break;
                        case RUK_WEAPON_PLASMA:
                            name = std::wstring(IF_WEAPON_PLASMA_ON);
                            break;
                        case RUK_WEAPON_FLAMETHROWER:
                            name = std::wstring(IF_WEAPON_FIRE_ON);
                            break;
                        case RUK_WEAPON_MISSILE:
                            name = std::wstring(IF_WEAPON_ROCKET_ON);
                            break;
                        case RUK_WEAPON_ELECTRIC:
                            name = std::wstring(IF_WEAPON_ELECTRO_ON);
                            break;
                        case RUK_WEAPON_REPAIR:
                            name = std::wstring(IF_WEAPON_REPAIR_ON);
                            break;
                        case RUK_WEAPON_BOMB:
                            name = std::wstring(IF_WEAPON_BOOM_ON);
                            break;
                        case RUK_WEAPON_MORTAR:
                            name = std::wstring(IF_WEAPON_MINOMET_ON);
                            break;
                    }

                    CIFaceStatic *s = interfaces->CreateStaticFromImage(float(g_IFaceList->m_DWeaponX[pos]),
                                                                        float(g_IFaceList->m_DWeaponY[pos]), 0.0000001f,
                                                                        *interfaces->FindImageByName(name));
                    s->m_nId = DYNAMIC_WEAPON_ON_ID;

                    s = interfaces->CreateStaticFromImage(float(g_IFaceList->m_DWeaponX[pos]),
                                                          float(g_IFaceList->m_DWeaponY[pos]), 0.000001f,
                                                          overheat_image);
                    s->m_Param1 = (float)i;
                }
            }

            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::DeleteWeaponDynamicStatics() {
    DTRACE();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();

    if (!(player_side))
        return;

    CInterface *interfaces = m_First;

    while (interfaces) {
        if (interfaces->m_strName == IF_MAIN) {
            CIFaceElement *elements = interfaces->m_FirstElement;
            while (elements) {
                if (elements->m_nId == DYNAMIC_WEAPON_ON_ID || elements->m_strName == IF_OVER_HEAT) {
                    elements = interfaces->DelElement(elements);
                    continue;
                }
                elements = elements->m_NextElement;
            }
            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::CreateItemPrice(int *price) {
    DTRACE();
    DeleteItemPrice();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();
    CInterface *interfaces = m_First;

    int *res = price;
    //    player_side->m_ConstructPanel->m_FocusedPrice.ResetPrice();

    //    memcpy(player_side->m_ConstructPanel->m_FocusedPrice.m_Resources, price.m_Resources,
    //    sizeof(player_side->m_ConstructPanel->m_FocusedPrice.m_Resources));

    while (interfaces) {
        if (interfaces->m_strName == IF_BASE) {
            float x = 22, y = 243, z = 0.00001f;
            CIFaceImage titan_image = *interfaces->FindImageByName(std::wstring(IF_BASE_TITAN_IMAGE));
            CIFaceImage electronics_image = *interfaces->FindImageByName(std::wstring(IF_BASE_ELECTRONICS_IMAGE));
            CIFaceImage energy_image = *interfaces->FindImageByName(std::wstring(IF_BASE_ENERGY_IMAGE));
            CIFaceImage plasma_image = *interfaces->FindImageByName(std::wstring(IF_BASE_PLASMA_IMAGE));

            for (int cnt = 0; cnt < MAX_RESOURCES; cnt++) {
                if (res[cnt] != 0) {
                    CIFaceStatic *s = NULL;
                    if (cnt == TITAN) {
                        s = interfaces->CreateStaticFromImage(x, y, z, titan_image);
                        player_side->m_ConstructPanel->m_ftitX = Float2Int(x);
                    }
                    else if (cnt == ELECTRONICS) {
                        s = interfaces->CreateStaticFromImage(x, y, z, electronics_image);
                        player_side->m_ConstructPanel->m_felecX = Float2Int(x);
                    }
                    else if (cnt == ENERGY) {
                        s = interfaces->CreateStaticFromImage(x, y, z, energy_image);
                        player_side->m_ConstructPanel->m_fenerX = Float2Int(x);
                    }
                    else if (cnt == PLASMA) {
                        s = interfaces->CreateStaticFromImage(x, y, z, plasma_image);
                        player_side->m_ConstructPanel->m_fplasX = Float2Int(x);
                    }
                    if (s) {
                        s->SetVisibility(false);
                        x = x + s->m_xSize + 25;
                        s->m_nId = ITEM_PRICE_ID;
                    }
                }
            }
            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::DeleteItemPrice() {
    DTRACE();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();

    CInterface *interfaces = m_First;
    while (interfaces) {
        if (interfaces->m_strName == IF_BASE) {
            CIFaceElement *elements = interfaces->m_FirstElement;
            while (elements) {
                if (elements->m_Type == IFACE_DYNAMIC_STATIC && elements->m_nId == ITEM_PRICE_ID) {
                    elements = interfaces->DelElement(elements);
                    continue;
                }
                elements = elements->m_NextElement;
            }
            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::CreateSummPrice(int multiplier) {
    DTRACE();
    DeleteSummPrice();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();

    CInterface *interfaces = m_First;

    int cfg_num = player_side->m_ConstructPanel->m_CurrentConfig;
    int res[MAX_RESOURCES];
    ZeroMemory(res, sizeof(res));

    player_side->m_Constructor->GetConstructionPrice(res);

    int fuck = 0;
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (res[i]) {
            if (multiplier) {
                res[i] *= multiplier;
            }
            fuck++;
        }
    }

    while (interfaces) {
        if (interfaces->m_strName == IF_BASE) {
            float x = 200, y = 352, z = 0.00001f;
            if (fuck == 3) {
                x = 235;
            }
            else if (fuck == 2) {
                x = 250;
            }
            CIFaceImage titan_image = *interfaces->FindImageByName(std::wstring(IF_BASE_TITAN_IMAGE));
            CIFaceImage electronics_image = *interfaces->FindImageByName(std::wstring(IF_BASE_ELECTRONICS_IMAGE));
            CIFaceImage energy_image = *interfaces->FindImageByName(std::wstring(IF_BASE_ENERGY_IMAGE));
            CIFaceImage plasma_image = *interfaces->FindImageByName(std::wstring(IF_BASE_PLASMA_IMAGE));
            CIFaceImage warning_image = *interfaces->FindImageByName(std::wstring(IF_BASE_WARNING));

            for (int cnt = 0; cnt < MAX_RESOURCES; cnt++) {
                if (res[cnt] != 0) {
                    CIFaceStatic *s = NULL;
                    int warning_id = DYNAMIC_WARNING;
                    if (cnt == TITAN) {
                        player_side->m_ConstructPanel->m_Configs[cfg_num].m_titX = Float2Int(x) - 2;
                        s = interfaces->CreateStaticFromImage(x, y, z, titan_image);
                        s->m_strName = IF_BASE_TITAN_SUMM;
                    }
                    else if (cnt == ELECTRONICS) {
                        player_side->m_ConstructPanel->m_Configs[cfg_num].m_elecX = Float2Int(x) - 2;
                        s = interfaces->CreateStaticFromImage(x, y, z, electronics_image);
                        s->m_strName = IF_BASE_ELECTRONICS_SUMM;
                        warning_id += 1;
                    }
                    else if (cnt == ENERGY) {
                        player_side->m_ConstructPanel->m_Configs[cfg_num].m_enerX = Float2Int(x) - 2;
                        s = interfaces->CreateStaticFromImage(x, y, z, energy_image);
                        s->m_strName = IF_BASE_ENERGY_SUMM;
                        warning_id += 2;
                    }
                    else if (cnt == PLASMA) {
                        player_side->m_ConstructPanel->m_Configs[cfg_num].m_plasX = Float2Int(x) - 2;
                        s = interfaces->CreateStaticFromImage(x, y, z, plasma_image);
                        s->m_strName = IF_BASE_PLASMA_SUMM;
                        warning_id += 3;
                    }
                    if (s) {
                        s->SetVisibility(false);
                        CIFaceStatic *swarn =
                                interfaces->CreateStaticFromImage(x + s->m_xSize, y + 22, z, warning_image);
                        x = x + s->m_xSize + 31;
                        s->m_nId = SUMM_PRICE_ID;
                        swarn->m_nId = warning_id;
                        swarn->SetVisibility(true);
                    }
                }
            }
            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::DeleteSummPrice() {
    DTRACE();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();

    CInterface *interfaces = m_First;
    while (interfaces) {
        if (interfaces->m_strName == IF_BASE) {
            CIFaceElement *elements = interfaces->m_FirstElement;
            while (elements) {
                /*                if(elements->m_strName == IF_BASE_SUMM_PANEL){
                                    elements->m_StateImages[IFACE_NORMAL].m_Caption = std::wstring(L"");
                                    elements->m_StateImages[IFACE_NORMAL].SetStateText(true);
                                }else */
                if (elements->m_Type == IFACE_DYNAMIC_STATIC &&
                    (elements->m_nId == SUMM_PRICE_ID || IS_DYNAMIC_WARNING(elements->m_nId))) {
                    elements = interfaces->DelElement(elements);
                    continue;
                }
                elements = elements->m_NextElement;
            }
            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::SlideFocusedInterfaceRight() {
    if (m_FocusedInterface) {
        m_FocusedInterface->BeginSlide(m_FocusedInterface->m_xPos + 100, m_FocusedInterface->m_yPos);
    }
}

void CIFaceList::SlideFocusedInterfaceLeft() {
    if (m_FocusedInterface) {
        m_FocusedInterface->BeginSlide(m_FocusedInterface->m_xPos - 100, m_FocusedInterface->m_yPos);
    }
}

void CIFaceList::LiveRobot(void) {
    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
    if (ps->IsArcadeMode()) {
        CMatrixMapStatic *obj = ps->GetArcadedObject();
        ESelType type = NOTHING;

        if (obj && obj->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
            type = ROBOT;
        }
        else if (obj && obj->GetObjectType() == OBJECT_TYPE_FLYER) {
            type = FLYER;
        }

        ps->SetArcadedObject(NULL);

        CMatrixGroup *grp = ps->GetCurSelGroup();

        grp->RemoveAll();
        grp->AddObject(obj, -4);

        ps->SetCurGroup(ps->CreateGroupFromCurrent());
        ps->Select(type, NULL);

        CInterface *ifs = g_IFaceList->m_First;
        while (ifs) {
            if (ifs->m_strName == IF_MAIN) {
                ifs->m_xPos = float(g_ScreenX - (1024 - 447));
                ifs->ReCalcElementsPos();
                break;
            }
            ifs = ifs->m_NextInterface;
        }
    }
}

void CIFaceList::EnterRobot(bool pos) {
    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
    if (ps->GetCurGroup() && ps->GetCurGroup()->m_FirstObject) {
        CMatrixMapStatic *o = ps->GetCurGroup()->m_FirstObject->GetObject();
        if (pos) {
            o = ps->GetCurGroup()->GetObjectByN(ps->GetCurSelNum());
        }

        ps->SetArcadedObject(o);

        CInterface *ifs = g_IFaceList->m_First;
        while (ifs) {
            if (ifs->m_strName == IF_MAIN) {
                ifs->m_xPos = float(g_ScreenX - (1024 - (447 + 196)));
                ifs->ReCalcElementsPos();
            }
            ifs = ifs->m_NextInterface;
        }
    }
}

void __stdcall CIFaceList::PlayerAction(void *object) {
    if (!object)
        return;

    CIFaceElement *element = (CIFaceElement *)object;
    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();

    if (element->m_strName == IF_MAIN_MENU_BUTTON) {
        g_MatrixMap->EnterDialogMode(TEMPLATE_DIALOG_MENU);
    }
    else if (element->m_strName == IF_AORDER_FROBOT_ON) {
        RESETFLAG(m_IfListFlags, AUTO_FROBOT_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);

        ps->PGOrderStop(ps->SelGroupToLogicGroup());
    }
    else if (element->m_strName == IF_AORDER_FROBOT_OFF) {
        RESETFLAG(m_IfListFlags, AUTO_FROBOT_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);
        SETFLAG(m_IfListFlags, AUTO_FROBOT_ON);

        ps->PGOrderAutoAttack(ps->SelGroupToLogicGroup());
    }
    else if (element->m_strName == IF_MAIN_SELFBOMB) {
        if (ps->IsArcadeMode() && ps->GetArcadedObject()->IsLiveRobot()) {
            ps->GetArcadedObject()->AsRobot()->BigBoom();
        }
    }

    if (element->m_strName == IF_AORDER_PROTECT_ON) {
        RESETFLAG(m_IfListFlags, AUTO_FROBOT_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);

        ps->PGOrderStop(ps->SelGroupToLogicGroup());
    }
    else if (element->m_strName == IF_AORDER_PROTECT_OFF) {
        RESETFLAG(m_IfListFlags, AUTO_FROBOT_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);
        SETFLAG(m_IfListFlags, AUTO_PROTECT_ON);

        ps->PGOrderAutoDefence(ps->SelGroupToLogicGroup());
    }

    if (element->m_strName == IF_AORDER_CAPTURE_ON) {
        RESETFLAG(m_IfListFlags, AUTO_FROBOT_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);

        ps->PGOrderStop(ps->SelGroupToLogicGroup());
    }
    else if (element->m_strName == IF_AORDER_CAPTURE_OFF) {
        RESETFLAG(m_IfListFlags, AUTO_FROBOT_ON | AUTO_CAPTURE_ON | AUTO_PROTECT_ON);
        SETFLAG(m_IfListFlags, AUTO_CAPTURE_ON);

        ps->PGOrderAutoCapture(ps->SelGroupToLogicGroup());
    }

    if (element->m_strName == IF_ORDER_FIRE) {
        SETFLAG(m_IfListFlags, PREORDER_FIRE);
        SETFLAG(m_IfListFlags, ORDERING_MODE);
    }
    else if (element->m_strName == IF_ORDER_CAPTURE) {
        SETFLAG(m_IfListFlags, PREORDER_CAPTURE);
        SETFLAG(m_IfListFlags, ORDERING_MODE);
    }
    else if (element->m_strName == IF_ORDER_PATROL) {
        SETFLAG(m_IfListFlags, PREORDER_PATROL);
        SETFLAG(m_IfListFlags, ORDERING_MODE);
    }
    else if (element->m_strName == IF_ORDER_MOVE) {
        SETFLAG(m_IfListFlags, PREORDER_MOVE);
        SETFLAG(m_IfListFlags, ORDERING_MODE);
    }
    else if (element->m_strName == IF_ORDER_REPAIR) {
        SETFLAG(m_IfListFlags, PREORDER_REPAIR);
        SETFLAG(m_IfListFlags, ORDERING_MODE);
    }
    else if (element->m_strName == IF_ORDER_BOMB) {
        SETFLAG(m_IfListFlags, PREORDER_BOMB);
        SETFLAG(m_IfListFlags, ORDERING_MODE);
    }
    else if (element->m_strName == IF_ORDER_CANCEL) {
        if (ps->m_CurrentAction == BUILDING_TURRET) {
            ps->m_CannonForBuild.Delete();
            ps->m_CurrentAction = NOTHING_SPECIAL;
        }
        ResetOrderingMode();
    }
    else if (element->m_strName == IF_ORDER_STOP) {
        ps->PGOrderStop(ps->SelGroupToLogicGroup());
        // if(ps->m_CurGroup->m_Tactics){
        //    ps->m_CurGroup->DeInstallTactics();
        //}else{
        //    ps->SelectedGroupBreakOrders();
        //}
    }

    if (element->m_strName == IF_ENTER_ROBOT) {
        EnterRobot();
    }
    else if (element->m_strName == IF_LEAVE_ROBOT) {
        LiveRobot();
    }
    if (element->m_strName == IF_BASE_CONST_CANCEL) {
        ps->m_ConstructPanel->ResetGroupNClose();
    }

    if (element->m_strName == IF_BUILD_HE) {
        ps->m_ConstructPanel->ResetGroupNClose();
        SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);
        SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_BUILD_FLYER);
    }
    else if (element->m_strName == IF_BUILD_CA) {
        ps->m_ConstructPanel->ResetGroupNClose();
        SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);
        SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_BUILD_TURRET);
        if (ps->m_ActiveObject) {
            ((CMatrixBuilding *)ps->m_ActiveObject)->CreatePlacesShow();
        }
    }
    else if (element->m_strName == IF_BUILD_REPAIR) {
        ps->m_ConstructPanel->ResetGroupNClose();
        SETFLAG(m_IfListFlags, PREORDER_BUILD_REPAIR);
        SETFLAG(m_IfListFlags, ORDERING_MODE);
    }
    else if (element->m_strName == IF_CALL_FROM_HELL &&
             (ps->m_CurrSel == BASE_SELECTED || ps->m_CurrSel == BUILDING_SELECTED)) {
        CMatrixBuilding *bld = (CMatrixBuilding *)ps->m_ActiveObject;
        bld->Maintenance();
    }

    if (ps->m_CurrSel == BASE_SELECTED || ps->m_CurrSel == BUILDING_SELECTED) {
        CMatrixBuilding *base = (CMatrixBuilding *)ps->m_ActiveObject;
        if (element->m_strName == IF_BUILD_TUR1) {
            if (0 /*((CMatrixBuilding*)ps->m_ActiveObject)->HaveMaxTurrets()*/) {
                // sound
                CSound::Play(S_CANTBE, SL_INTERFACE);
                // hint
            }
            else {
                BeginBuildTurret(1);
            }
        }
        else if (element->m_strName == IF_BUILD_TUR2) {
            if (0 /*((CMatrixBuilding*)ps->m_ActiveObject)->HaveMaxTurrets()*/) {
                // sound
                CSound::Play(S_CANTBE, SL_INTERFACE);
                // hint
            }
            else {
                BeginBuildTurret(2);
            }
        }
        else if (element->m_strName == IF_BUILD_TUR3) {
            if (0 /*((CMatrixBuilding*)ps->m_ActiveObject)->HaveMaxTurrets()*/) {
                // sound
                CSound::Play(S_CANTBE, SL_INTERFACE);
                // hint
            }
            else {
                BeginBuildTurret(3);
            }
        }
        else if (element->m_strName == IF_BUILD_TUR4) {
            if (0 /*((CMatrixBuilding*)ps->m_ActiveObject)->HaveMaxTurrets()*/) {
                // sound
                CSound::Play(S_CANTBE, SL_INTERFACE);
                // hint
            }
            else {
                BeginBuildTurret(4);
            }
        }
    }

    int *cfg_num = &ps->m_ConstructPanel->m_CurrentConfig;
}

void CIFaceList::CreateGroupSelection(CInterface *iface) {
    DTRACE();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();
    int sel_objs = 9;

    float x = 225, y = 49, z = 0.000001f;
    CIFaceImage ramka_image = *iface->FindImageByName(std::wstring(IF_GROUP_RAMKA));
    for (int i = 0; i < sel_objs; i++) {
        float pos = (i + 1.0f) / 3.0f;

        if (pos <= 1) {
            x = (float)(225 + 48 * i);
            y = 49;
        }
        else if (pos > 1 && pos <= 2) {
            x = (float)((225 + 48 * i) - 48 * 3);
            y = 49 * 2;
        }
        else if (pos > 2) {
            x = (float)((225 + 48 * i) - 48 * 6);
            y = 49 * 3;
        }

        CIFaceStatic *s = iface->CreateStaticFromImage(x, y, z, ramka_image);
        if (s) {
            s->SetVisibility(false);
            s->m_nId = GROUP_SELECTION_ID + i;
        }
    }
}

void CIFaceList::DeleteGroupSelection() {
    DTRACE();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();

    CMatrixMapStatic *so = CMatrixMapStatic::GetFirstLogic();

    while (so) {
        if (so->IsRobot()) {
            so->AsRobot()->DeleteProgressBarClone(PBC_CLONE1);
        }
        else if (so->IsFlyer()) {
            so->AsFlyer()->DeleteProgressBarClone(PBC_CLONE1);
        }

        so = so->GetNextLogic();
    }

    CInterface *interfaces = m_First;
    while (interfaces) {
        if (interfaces->m_strName == IF_MAIN) {
            CIFaceElement *elements = interfaces->m_FirstElement;
            while (elements) {
                if (elements->m_Type == IFACE_DYNAMIC_STATIC && elements->m_nId == GROUP_SELECTION_ID) {
                    elements = interfaces->DelElement(elements);
                    continue;
                }
                elements = elements->m_NextElement;
            }
            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
}
void CIFaceList::DeleteProgressBars(CMatrixMapStatic *from) {
    DTRACE();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();
    if (!player_side->GetCurGroup()) {
        return;
    }

    CMatrixGroupObject *gos = player_side->GetCurGroup()->m_FirstObject;

    if (!from) {
        CMatrixMapStatic *so = CMatrixMapStatic::GetFirstLogic();

        while (so) {
            if (so->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
                ((CMatrixRobotAI *)so)->DeleteProgressBarClone(PBC_CLONE1);
            }
            else if (so->GetObjectType() == OBJECT_TYPE_FLYER) {
                ((CMatrixFlyer *)so)->DeleteProgressBarClone(PBC_CLONE1);
            }

            so = so->GetNextLogic();
        }
    }
    else {
        while (gos) {
            if (gos->GetObject() == from)
                break;
            gos = gos->m_NextObject;
        }

        while (gos) {
            if (gos->GetObject()->IsRobot()) {
                gos->GetObject()->AsRobot()->DeleteProgressBarClone(PBC_CLONE1);
            }
            else if (gos->GetObject()->GetObjectType() == OBJECT_TYPE_FLYER) {
                ((CMatrixFlyer *)gos->GetObject())->DeleteProgressBarClone(PBC_CLONE1);
            }

            gos = gos->m_NextObject;
        }
    }
}

void CIFaceList::CreateGroupIcons() {
    DTRACE();
    DeleteGroupIcons();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();
    if (!player_side->GetCurGroup()) {
        return;
    }

    int sel_objs = player_side->GetCurGroup()->GetRobotsCnt() + player_side->GetCurGroup()->GetFlyersCnt();
    if (!sel_objs)
        return;

    CInterface *interfaces = m_First;
    CMatrixGroupObject *so = player_side->GetCurGroup()->m_FirstObject;

    CIFaceImage *image = HNew(g_MatrixHeap) CIFaceImage;

    while (interfaces) {
        if (interfaces->m_strName == IF_MAIN) {
            float x = 225, y = 49, z = 0;
            int pos = 0;
            for (int i = 0; i < sel_objs; i++) {
                if (i < 3) {
                    y = 49;
                }
                else if (i < 6) {
                    y = 49 * 2;
                }
                else if (i < 9) {
                    y = 49 * 3;
                }

                x = (float)((225 + 48 * pos));
                pos++;

                if (x > (225 + 48 * 2)) {
                    pos = 1;
                    x = 225;
                }

                CTextureManaged *tex = NULL;
                bool robot = false, flyer = false;
                float xmed = 0, ymed = 0;

                if (so) {
                    if (so->GetObject()->IsLiveRobot()) {
                        tex = so->GetObject()->AsRobot()->GetMedTexture();
                        robot = true;
                    }
                }

                if (tex) {
                    image->m_Image = tex;
                    image->m_Height = 36;
                    image->m_Width = 47;
                    image->m_NextImage = NULL;
                    image->m_PrevImage = NULL;
                    image->m_strName = L"";
                    image->m_Type = IFACE_IMAGE;

                    CIFaceStatic *s = NULL;
                    if (robot) {
                        image->m_TexHeight = 64;
                        image->m_TexWidth = 64;
                        image->m_xTexPos = 0;
                        image->m_yTexPos = 0;
                        s = interfaces->CreateStaticFromImage(x, y, z, *image, true);
                    }

                    if (s) {
                        s->SetVisibility(true);
                        s->m_nId = GROUP_ICONS_ID + i;
                    }
                }

                so = so->m_NextObject;
            }

            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
    if (image) {
        HDelete(CIFaceImage, image, g_MatrixHeap);
    }
}

void CIFaceList::DeleteGroupIcons() {
    DTRACE();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();

    CMatrixMapStatic *so = CMatrixMapStatic::GetFirstLogic();

    CInterface *interfaces = m_First;
    while (interfaces) {
        if (interfaces->m_strName == IF_MAIN) {
            CIFaceElement *elements = interfaces->m_FirstElement;
            while (elements) {
                if (elements->m_Type == IFACE_DYNAMIC_STATIC && IS_GROUP_ICON(elements->m_nId)) {
                    elements = interfaces->DelElement(elements);
                    continue;
                }
                elements = elements->m_NextElement;
            }
            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::CreatePersonal() {
    DTRACE();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();
    if (!player_side->GetCurGroup()) {
        return;
    }

    int selected = player_side->GetCurSelNum();

    CMatrixGroup *group = player_side->GetCurGroup();

    CMatrixGroupObject *go = group->m_FirstObject;
    for (int i = 0; i < selected && go; i++) {
        go = go->m_NextObject;
    }

    if (!go)
        return;
    CInterface *interfaces = g_IFaceList->m_First;
    while (interfaces) {
        if (interfaces->m_strName == IF_MAIN) {
            CTextureManaged *tex = NULL;
            float xbig = 0, ybig = 0;
            bool flyer = false;
            bool robot = false;

            CIFaceImage *image = HNew(g_MatrixHeap) CIFaceImage;

            if (go->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI) {
                tex = ((CMatrixRobotAI *)go->GetObject())->GetBigTexture();
                robot = true;
            } /*else if(go->GetObject()->GetObjectType() == OBJECT_TYPE_FLYER){
                 if(((CMatrixFlyer*)go->GetObject())->m_FlyerKind == FLYER_SPEED){
                     tex = interfaces->FindImageByName(std::wstring(IF_FLYER_BIG1))->m_Image;

                     xbig = interfaces->FindImageByName(std::wstring(IF_FLYER_BIG1))->m_xTexPos;
                     ybig = interfaces->FindImageByName(std::wstring(IF_FLYER_BIG1))->m_yTexPos;
                 }else if(((CMatrixFlyer*)go->GetObject())->m_FlyerKind == FLYER_TRANSPORT){
                     tex = interfaces->FindImageByName(std::wstring(IF_FLYER_BIG2))->m_Image;

                     xbig = interfaces->FindImageByName(std::wstring(IF_FLYER_BIG2))->m_xTexPos;
                     ybig = interfaces->FindImageByName(std::wstring(IF_FLYER_BIG2))->m_yTexPos;
                 }else if(((CMatrixFlyer*)go->GetObject())->m_FlyerKind == FLYER_BOMB){
                     tex = interfaces->FindImageByName(std::wstring(IF_FLYER_BIG3))->m_Image;

                     xbig = interfaces->FindImageByName(std::wstring(IF_FLYER_BIG3))->m_xTexPos;
                     ybig = interfaces->FindImageByName(std::wstring(IF_FLYER_BIG3))->m_yTexPos;
                 }else if(((CMatrixFlyer*)go->GetObject())->m_FlyerKind == FLYER_ATTACK){
                     tex = interfaces->FindImageByName(std::wstring(IF_FLYER_BIG4))->m_Image;

                     xbig = interfaces->FindImageByName(std::wstring(IF_FLYER_BIG4))->m_xTexPos;
                     ybig = interfaces->FindImageByName(std::wstring(IF_FLYER_BIG4))->m_yTexPos;
                 }
                 flyer = true;
             }*/

            if (tex) {
                image->m_Image = tex;
                image->m_Height = 114;
                image->m_Width = 114;
                image->m_NextImage = NULL;
                image->m_PrevImage = NULL;
                image->m_strName = L"";
                image->m_Type = IFACE_IMAGE;

                CIFaceStatic *s = NULL;
                /*if(flyer){
                    image->m_TexHeight = 512;
                    image->m_TexWidth = 512;
                    image->m_xTexPos = xbig;
                    image->m_yTexPos = ybig;
                    s = interfaces->CreateStaticFromImage(81, 61, 0.0000001f, *image, false);
                }else */
                if (robot) {
                    image->m_TexHeight = 256;
                    image->m_TexWidth = 256;
                    image->m_xTexPos = 0;
                    image->m_yTexPos = 0;
                    s = interfaces->CreateStaticFromImage(81, 61, 0.0000001f, *image, true);
                }

                if (s) {
                    FSET(ON_PRESS, s, g_IFaceList, CIFaceList::JumpToRobot);
                    s->SetVisibility(true);
                    s->m_nId = PERSONAL_ICON_ID;
                }
            }

            interfaces->SortElementsByZ();
            if (image) {
                HDelete(CIFaceImage, image, g_MatrixHeap);
            }

            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::DeletePersonal() {
    DTRACE();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();

    // icon

    CInterface *interfaces = m_First;
    while (interfaces) {
        if (interfaces->m_strName == IF_MAIN) {
            CIFaceElement *elements = interfaces->m_FirstElement;
            while (elements) {
                if (elements->m_Type == IFACE_DYNAMIC_STATIC && elements->m_nId == PERSONAL_ICON_ID) {
                    elements = interfaces->DelElement(elements);
                    continue;
                }
                elements = elements->m_NextElement;
            }
            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }

    // progress bar
    CMatrixMapStatic *s = CMatrixMapStatic::GetFirstLogic();

    while (s) {
        if (s->IsRobot()) {
            s->AsRobot()->DeleteProgressBarClone(PBC_CLONE2);
        }
        else if (s->GetObjectType() == OBJECT_TYPE_FLYER) {
            ((CMatrixFlyer *)s)->DeleteProgressBarClone(PBC_CLONE2);
        }
        else if (s->IsBuilding()) {
            s->AsBuilding()->DeleteProgressBarClone(PBC_CLONE2);
        }

        s = s->GetNextLogic();
    }
}

void CIFaceList::CreateOrdersGlow(CInterface *iface) {
    DTRACE();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();
    int orders = 6;

    float x = 419, y = 47, z = 0.0000001f;
    for (int i = 0; i < orders; i++) {
        float pos = (i + 1.0f) / 3.0f;

        if (pos <= 1) {
            x = (float)(419 + 49 * i);
            y = 47;
        }
        else if (pos > 1 && pos <= 2) {
            x = (float)((419 + 49 * i) - 49 * 3);
            y = 47 + 49;
        }

        CIFaceStatic *s = iface->CreateStaticFromImage(x, y, z, *iface->FindImageByName(std::wstring(IF_ORDER_GLOW)));
        if (s) {
            s->SetVisibility(false);
            s->m_nId = ORDERS_GLOW_ID + i;
        }
    }
}

void CIFaceList::ResetOrderingMode() {
    DTRACE();
    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
    if (ps->m_ActiveObject && ps->m_ActiveObject->IsBuilding()) {
        ((CMatrixBuilding *)ps->m_ActiveObject)->DeletePlacesShow();
    }

    RESETFLAG(m_IfListFlags, ORDERING_MODE);
    m_IfListFlags &= 0x0000ffff;
}

void CIFaceList::CreateStackIcon(int num, CMatrixBuilding *base, CMatrixMapStatic *object) {
    DTRACE();
    if (!object)
        return;

    CInterface *ifs = m_First;
    while (ifs) {
        if (ifs->m_strName == IF_MAIN) {
            CIFaceStatic *s = NULL;
            CTextureManaged *tex_med = NULL, *tex_small = NULL;
            bool flyer = false;
            bool robot = false;
            bool turret = false;
            float xmed = 0;
            float ymed = 0;
            float xsmall = 0;
            float ysmall = 0;

            if (object->IsRobot()) {
                tex_med = ((CMatrixRobotAI *)object)->GetMedTexture();
#ifdef USE_SMALL_TEXTURE_IN_ROBOT_ICON
                tex_small = ((CMatrixRobotAI *)object)->GetSmallTexture();
#else
                tex_small = ((CMatrixRobotAI *)object)->GetMedTexture();
#endif
                robot = true;
            } /*else if(object->GetObjectType() == OBJECT_TYPE_FLYER){
                 if(((CMatrixFlyer*)object)->m_FlyerKind == FLYER_SPEED){
                     tex_med = ifs->FindImageByName(std::wstring(IF_FLYER_MED1))->m_Image;
                     tex_small = ifs->FindImageByName(std::wstring(IF_FLYER_SMALL1))->m_Image;

                     xmed = ifs->FindImageByName(std::wstring(IF_FLYER_MED1))->m_xTexPos;
                     ymed = ifs->FindImageByName(std::wstring(IF_FLYER_MED1))->m_yTexPos;
                     xsmall = ifs->FindImageByName(std::wstring(IF_FLYER_SMALL1))->m_xTexPos;
                     ysmall = ifs->FindImageByName(std::wstring(IF_FLYER_SMALL1))->m_yTexPos;
                 }else if(((CMatrixFlyer*)object)->m_FlyerKind == FLYER_TRANSPORT){
                     tex_med = ifs->FindImageByName(std::wstring(IF_FLYER_MED2))->m_Image;
                     tex_small = ifs->FindImageByName(std::wstring(IF_FLYER_SMALL2))->m_Image;

                     xmed = ifs->FindImageByName(std::wstring(IF_FLYER_MED2))->m_xTexPos;
                     ymed = ifs->FindImageByName(std::wstring(IF_FLYER_MED2))->m_yTexPos;
                     xsmall = ifs->FindImageByName(std::wstring(IF_FLYER_SMALL2))->m_xTexPos;
                     ysmall = ifs->FindImageByName(std::wstring(IF_FLYER_SMALL2))->m_yTexPos;
                 }else if(((CMatrixFlyer*)object)->m_FlyerKind == FLYER_BOMB){
                     tex_med = ifs->FindImageByName(std::wstring(IF_FLYER_MED3))->m_Image;
                     tex_small = ifs->FindImageByName(std::wstring(IF_FLYER_SMALL3))->m_Image;

                     xmed = ifs->FindImageByName(std::wstring(IF_FLYER_MED3))->m_xTexPos;
                     ymed = ifs->FindImageByName(std::wstring(IF_FLYER_MED3))->m_yTexPos;
                     xsmall = ifs->FindImageByName(std::wstring(IF_FLYER_SMALL3))->m_xTexPos;
                     ysmall = ifs->FindImageByName(std::wstring(IF_FLYER_SMALL3))->m_yTexPos;
                 }else if(((CMatrixFlyer*)object)->m_FlyerKind == FLYER_ATTACK){
                     tex_med = ifs->FindImageByName(std::wstring(IF_FLYER_MED4))->m_Image;
                     tex_small = ifs->FindImageByName(std::wstring(IF_FLYER_SMALL4))->m_Image;

                     xmed = ifs->FindImageByName(std::wstring(IF_FLYER_MED4))->m_xTexPos;
                     ymed = ifs->FindImageByName(std::wstring(IF_FLYER_MED4))->m_yTexPos;
                     xsmall = ifs->FindImageByName(std::wstring(IF_FLYER_SMALL4))->m_xTexPos;
                     ysmall = ifs->FindImageByName(std::wstring(IF_FLYER_SMALL4))->m_yTexPos;
                 }
                 flyer = true;
             }*/
            else if (object->IsCannon()) {
                if (((CMatrixCannon *)object)->m_Num == 1) {
                    tex_med = ifs->FindImageByName(std::wstring(IF_TURRET_MED1))->m_Image;
                    tex_small = ifs->FindImageByName(std::wstring(IF_TURRET_SMALL1))->m_Image;

                    xmed = ifs->FindImageByName(std::wstring(IF_TURRET_MED1))->m_xTexPos;
                    ymed = ifs->FindImageByName(std::wstring(IF_TURRET_MED1))->m_yTexPos;
                    xsmall = ifs->FindImageByName(std::wstring(IF_TURRET_SMALL1))->m_xTexPos;
                    ysmall = ifs->FindImageByName(std::wstring(IF_TURRET_SMALL1))->m_yTexPos;
                }
                else if (((CMatrixCannon *)object)->m_Num == 2) {
                    tex_med = ifs->FindImageByName(std::wstring(IF_TURRET_MED2))->m_Image;
                    tex_small = ifs->FindImageByName(std::wstring(IF_TURRET_SMALL2))->m_Image;

                    xmed = ifs->FindImageByName(std::wstring(IF_TURRET_MED2))->m_xTexPos;
                    ymed = ifs->FindImageByName(std::wstring(IF_TURRET_MED2))->m_yTexPos;
                    xsmall = ifs->FindImageByName(std::wstring(IF_TURRET_SMALL2))->m_xTexPos;
                    ysmall = ifs->FindImageByName(std::wstring(IF_TURRET_SMALL2))->m_yTexPos;
                }
                else if (((CMatrixCannon *)object)->m_Num == 3) {
                    tex_med = ifs->FindImageByName(std::wstring(IF_TURRET_MED3))->m_Image;
                    tex_small = ifs->FindImageByName(std::wstring(IF_TURRET_SMALL3))->m_Image;

                    xmed = ifs->FindImageByName(std::wstring(IF_TURRET_MED3))->m_xTexPos;
                    ymed = ifs->FindImageByName(std::wstring(IF_TURRET_MED3))->m_yTexPos;
                    xsmall = ifs->FindImageByName(std::wstring(IF_TURRET_SMALL3))->m_xTexPos;
                    ysmall = ifs->FindImageByName(std::wstring(IF_TURRET_SMALL3))->m_yTexPos;
                }
                else if (((CMatrixCannon *)object)->m_Num == 4) {
                    tex_med = ifs->FindImageByName(std::wstring(IF_TURRET_MED4))->m_Image;
                    tex_small = ifs->FindImageByName(std::wstring(IF_TURRET_SMALL4))->m_Image;

                    xmed = ifs->FindImageByName(std::wstring(IF_TURRET_MED4))->m_xTexPos;
                    ymed = ifs->FindImageByName(std::wstring(IF_TURRET_MED4))->m_yTexPos;
                    xsmall = ifs->FindImageByName(std::wstring(IF_TURRET_SMALL4))->m_xTexPos;
                    ysmall = ifs->FindImageByName(std::wstring(IF_TURRET_SMALL4))->m_yTexPos;
                }
                turret = true;
            }

            CIFaceImage *image = NULL;
            if (num == 1) {
                if (tex_med) {
                    image = HNew(g_MatrixHeap) CIFaceImage;
                    image->m_Image = tex_med;
                    image->m_Height = 42;
                    image->m_Width = 42;
                    image->m_NextImage = NULL;
                    image->m_PrevImage = NULL;
                    image->m_strName = L"";
                    image->m_Type = IFACE_IMAGE;

                    if (flyer || turret) {
                        image->m_TexHeight = 512;
                        image->m_TexWidth = 512;
                        image->m_xTexPos = xmed;
                        image->m_yTexPos = ymed;
                        s = ifs->CreateStaticFromImage(232, 55, 0, *image, false);
                    }
                    else if (robot) {
                        image->m_xTexPos = 0;
                        image->m_yTexPos = 0;
                        image->m_TexHeight = 64;
                        image->m_TexWidth = 64;
                        s = ifs->CreateStaticFromImage(232, 55, 0, *image, true);
                    } /*else if(turret){
                     }*/

                    if (s) {
                        s->m_nId = STACK_ICON + (num - 1);
                    }
                }
            }
            else {
                if (tex_small) {
                    image = HNew(g_MatrixHeap) CIFaceImage;
                    image->m_Image = tex_small;
                    image->m_Height = 25;
                    image->m_Width = 25;
                    image->m_NextImage = NULL;
                    image->m_PrevImage = NULL;
                    image->m_strName = L"";
                    image->m_Type = IFACE_IMAGE;

                    if (flyer || turret) {
                        image->m_TexHeight = 512;
                        image->m_TexWidth = 512;
                        image->m_xTexPos = xsmall;
                        image->m_yTexPos = ysmall;
                        s = ifs->CreateStaticFromImage(225 + (((float)num - 2) * 31), 105, 0, *image, false);
                    }
                    else if (robot) {
                        image->m_TexHeight = 32;
                        image->m_TexWidth = 32;
                        image->m_xTexPos = 0;
                        image->m_yTexPos = 0;
                        s = ifs->CreateStaticFromImage(225 + (((float)num - 2) * 31), 105, 0, *image, true);
                    } /*else if(turret){
                     }*/

                    if (s) {
                        s->m_nId = STACK_ICON + (num - 1);
                    }
                }
            }
            if (s) {
                s->m_iParam = int(base);
                s->SetVisibility(false);
            }

            if (image) {
                HDelete(CIFaceImage, image, g_MatrixHeap);
            }

            return;
        }
        ifs = ifs->m_NextInterface;
    }
}

void CIFaceList::DeleteStackIcon(int num, CMatrixBuilding *base) {
    DTRACE();

    if (!base) {
        return;
    }
    CInterface *ifs = m_First;
    while (ifs) {
        if (ifs->m_strName == IF_MAIN) {
            CIFaceElement *els = ifs->m_FirstElement;
            while (els) {
                if (IS_STACK_ICON(els->m_nId) && els->m_iParam == (int)base) {
                    if (num == 1 && els->m_nId == STACK_ICON + 1) {
                        els = ifs->DelElement(els);
                        continue;
                    }
                    else if (num == 1 && els->m_nId == STACK_ICON) {
                        els = ifs->DelElement(els);
                        continue;
                    }
                    else if (els->m_nId - STACK_ICON > (num - 1)) {
                        els->m_nId--;
                        els->RecalcPos(els->m_xPos - 31, els->m_yPos, false);
                    }
                    else if (els->m_nId - STACK_ICON == (num - 1)) {
                        els = ifs->DelElement(els);
                        continue;
                    }
                }
                els = els->m_NextElement;
            }
            if (num == 1 && base->m_BS.GetItemsCnt() > 1 && base->m_BS.GetTopItem()->m_NextStackItem) {
                CreateStackIcon(1, base, base->m_BS.GetTopItem()->m_NextStackItem);
            }
            return;
        }
        ifs = ifs->m_NextInterface;
    }
}

void CIFaceList::ConstructorButtonsInit() {
    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
    if (!ps)
        return;

    ps->m_Constructor->SuperDjeans(MRT_CHASSIS, RUK_CHASSIS_PNEUMATIC, 0);
    ps->m_Constructor->SuperDjeans(MRT_ARMOR, RUK_ARMOR_6, 0);
    ps->m_Constructor->SuperDjeans(MRT_HEAD, ERobotUnitKind(0), 0);
    WeaponPilonsInit();
    ps->m_Constructor->SuperDjeans(MRT_WEAPON, RUK_WEAPON_MACHINEGUN, 0);
    m_RCountControl->ManageButtons();
    GetConstructionName(ps->m_Constructor->GetRenderBot());
}

void CIFaceList::WeaponPilonsInit() {
    for (int i = 0; i < 5; i++) {
        CInterface::CopyElements(m_Weapon[0], m_WeaponPilon[i]);
    }
}

void CIFaceList::CreateElementRamka(CIFaceElement *element, DWORD color) {
    if (!element)
        return;
    for (int i = 0; i < MAX_STATES; i++) {
        if (element->m_StateImages[i].pImage) {
            element->m_StateImages[i].pImage->Ramka(Float2Int(element->m_StateImages[i].xTexPos),
                                                    Float2Int(element->m_StateImages[i].yTexPos),
                                                    Float2Int(element->m_xSize), Float2Int(element->m_ySize), color);
        }
    }
}

void CIFaceList::CreateHintButton(int x, int y, EHintButton type, DialogButtonHandler in_handler) {
    EnableMainMenuButton(type);
    auto handler = [in_handler](void*) { in_handler(); };
    void *fn = NULL, *cl = NULL;
    CIFaceElement *els = m_Hints->m_FirstElement;
    while (els) {
        if (type == HINT_OK && els->m_strName == IF_HINTS_OK) {
            els->m_Actions[ON_UN_PRESS].m_function = handler;
            els->RecalcPos((float)x, (float)y, false);
            els->SetVisibility(true);
            return;
        }
        else if (type == HINT_CANCEL && els->m_strName == IF_HINTS_CANCEL) {
            els->m_Actions[ON_UN_PRESS].m_function = handler;
            els->RecalcPos((float)x, (float)y, false);
            els->SetVisibility(true);
            return;
        }
        else if (type == HINT_CANCEL_MENU && els->m_strName == IF_HINTS_CANCEL_MENU) {
            els->m_Actions[ON_UN_PRESS].m_function = handler;
            els->RecalcPos((float)x, (float)y, false);
            els->SetVisibility(true);
            return;
        }
        else if (type == HINT_CONTINUE && els->m_strName == IF_HINTS_CONTINUE) {
            els->m_Actions[ON_UN_PRESS].m_function = handler;
            els->RecalcPos((float)x, (float)y, false);
            els->SetVisibility(true);
            return;
        }
        else if (type == HINT_SURRENDER && els->m_strName == IF_HINTS_SURRENDER) {
            els->m_Actions[ON_UN_PRESS].m_function = handler;
            els->RecalcPos((float)x, (float)y, false);
            els->SetVisibility(true);
            return;
        }
        else if (type == HINT_EXIT && els->m_strName == IF_HINTS_EXIT) {
            els->m_Actions[ON_UN_PRESS].m_function = handler;
            els->RecalcPos((float)x, (float)y, false);
            els->SetVisibility(true);
            return;
        }
        else if (type == HINT_RESET && els->m_strName == IF_HINTS_RESET) {
            els->m_Actions[ON_UN_PRESS].m_function = handler;
            els->RecalcPos((float)x, (float)y, false);
            els->SetVisibility(true);
            return;
        }
        else if (type == HINT_HELP && els->m_strName == IF_HINTS_HELP) {
            els->m_Actions[ON_UN_PRESS].m_function = handler;
            els->RecalcPos((float)x, (float)y, false);
            els->SetVisibility(true);
            return;
        }
        els = els->m_NextElement;
    }
}

void CIFaceList::HideHintButtons() {
    CIFaceElement *els = m_Hints->m_FirstElement;
    while (els) {
        els->SetVisibility(false);
        els = els->m_NextElement;
    }
}

void CIFaceList::HideHintButton(EHintButton butt) {
    CIFaceElement *els = m_Hints->m_FirstElement;

    std::wstring sname;
    HintButtonId2Name(butt, sname);

    while (els) {
        if (els->m_strName == sname) {
            els->SetVisibility(false);
        }
        els = els->m_NextElement;
    }
}

void CIFaceList::DisableMainMenuButton(EHintButton butt) {
    std::wstring sname;
    HintButtonId2Name(butt, sname);

    CIFaceElement *els = m_Hints->m_FirstElement;
    while (els) {
        if (els->m_strName == sname) {
            els->SetState(IFACE_DISABLED);
        }
        els = els->m_NextElement;
    }
}

void CIFaceList::EnableMainMenuButton(EHintButton butt) {
    std::wstring sname;
    HintButtonId2Name(butt, sname);

    CIFaceElement *els = m_Hints->m_FirstElement;
    while (els) {
        if (els->m_strName == sname) {
            if (els->GetState() == IFACE_DISABLED)
                els->SetState(IFACE_NORMAL);
        }
        els = els->m_NextElement;
    }
}
void CIFaceList::PressHintButton(EHintButton butt) {
    std::wstring sname;
    HintButtonId2Name(butt, sname);

    CIFaceElement *els = m_Hints->m_FirstElement;
    while (els) {
        if (els->m_strName == sname) {
            els->Action(ON_UN_PRESS);
            break;
        }
        els = els->m_NextElement;
    }
}

void CIFaceList::HintButtonId2Name(EHintButton butt, std::wstring &sname) {
    switch (butt) {
        case HINT_OK:
            sname = IF_HINTS_OK;
            break;
        case HINT_CANCEL:
            sname = IF_HINTS_CANCEL;
            break;
        case HINT_CANCEL_MENU:
            sname = IF_HINTS_CANCEL_MENU;
            break;
        case HINT_CONTINUE:
            sname = IF_HINTS_CONTINUE;
            break;
        case HINT_SURRENDER:
            sname = IF_HINTS_SURRENDER;
            break;
        case HINT_EXIT:
            sname = IF_HINTS_EXIT;
            break;
        case HINT_RESET:
            sname = IF_HINTS_RESET;
            break;
        case HINT_HELP:
            sname = IF_HINTS_HELP;
            break;
    }
}

bool CIFaceList::CorrectCoordinates(int screen_width, int screen_height, int &posx, int &posy, int width, int height,
                                    const std::wstring &element_name) {
    if (element_name == L"buro" || element_name == L"buca" || element_name == IF_ORDER_STOP ||
        element_name == IF_ORDER_MOVE || element_name == IF_ORDER_PATROL || element_name == IF_ORDER_FIRE ||
        element_name == IF_ORDER_CAPTURE || element_name == IF_ORDER_CANCEL || element_name == IF_ORDER_REPAIR ||
        element_name == IF_ORDER_BOMB || element_name == IF_LEAVE_ROBOT || element_name == IF_ENTER_ROBOT ||
        element_name == IF_AORDER_CAPTURE_OFF || element_name == IF_AORDER_CAPTURE_ON ||
        element_name == IF_AORDER_PROTECT_OFF || element_name == IF_AORDER_PROTECT_ON ||
        element_name == IF_AORDER_FROBOT_OFF || element_name == IF_AORDER_FROBOT_ON || element_name == IF_BUILD_TUR1 ||
        element_name == IF_BUILD_TUR2 || element_name == IF_BUILD_TUR3 || element_name == IF_BUILD_TUR4 ||
        element_name == IF_MAIN_SELFBOMB || element_name == IF_CALL_FROM_HELL) {
        int needx = 0;
        if (element_name == IF_LEAVE_ROBOT || element_name == IF_MAIN_SELFBOMB) {
            needx = Float2Int(m_MainX) + 354 - width;
        }
        else {
            needx = Float2Int(m_MainX) + 554 - width;
        }

        int needy = Float2Int(m_MainY) + 28 - height;

        if (posx > needx)
            posx -= (posx - needx);
        if (posx < needx)
            posx += (needx - posx);
        if (posy > needy)
            posy -= (posy - needy);
        if (posy < needy)
            posy += (needy - posy);
    }
    else if (element_name == IF_BASE_HISTORY_RIGHT || element_name == IF_BASE_HISTORY_LEFT ||
             element_name == IF_BASE_COUNTHZ) {
        posx -= Float2Int(width / 2.0f);
        posy -= height;
    }

    bool corx = false;
    if (posx + width + HINT_OTSTUP > screen_width) {
        corx = true;
        int val = posx + width + HINT_OTSTUP - screen_width;
        posx -= val;
    }

    if (corx == false) {
        if (posx < 0) {
            posx = HINT_OTSTUP;
        }
    }
    else {
        // eto znachit - chto hint nel'zya pokazyvat' - on vylazit so vseh shelei
        return false;
    }

    bool cory = false;
    if (posy + height + HINT_OTSTUP > screen_height) {
        cory = true;
        int val = posy + height + HINT_OTSTUP - screen_height;
        posy -= val;
    }

    if (cory == false) {
        if (posy < 0) {
            posy = HINT_OTSTUP;
        }
    }
    else {
        // eto znachit - chto hint nel'zya pokazyvat' - on vylazit so vseh shelei
        return false;
    }

    return true;
}

void CIFaceList::AddHintReplacements(const std::wstring &element_name) {
    CBlockPar *repl = g_MatrixData->BlockGet(PAR_REPLACE);
    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();

    if (element_name == L"thz") {
        int base_i, fa_i;
        ps->GetResourceIncome(base_i, fa_i, TITAN);
        repl->ParSetAdd(L"_titan_income", utils::format(L"%d", base_i + fa_i));
    }
    else if (element_name == L"enhz1" || element_name == L"enhz2") {
        int base_i, fa_i;
        ps->GetResourceIncome(base_i, fa_i, ENERGY);
        repl->ParSetAdd(L"_energy_income", utils::format(L"%d", base_i + fa_i));
    }
    else if (element_name == L"elhz") {
        int base_i, fa_i;
        ps->GetResourceIncome(base_i, fa_i, ELECTRONICS);
        repl->ParSetAdd(L"_electronics_income", utils::format(L"%d", base_i + fa_i));
    }
    else if (element_name == L"phz") {
        int base_i, fa_i;
        ps->GetResourceIncome(base_i, fa_i, PLASMA);
        repl->ParSetAdd(L"_plasma_income", utils::format(L"%d", base_i + fa_i));
    }
    else if (element_name == L"rvhz") {
        repl->ParSetAdd(L"_total_robots", utils::format(L"%d", ps->GetRobotsCnt()));
        repl->ParSetAdd(L"_max_robots", utils::format(L"%d", ps->GetMaxSideRobots()));
    }
    else if (element_name == IF_BUILD_TUR1) {
        int damage = Float2Int(1.0f / (g_Config.m_WeaponCooldown[Weap2Index(WEAPON_CANNON0)] / 1000.0f)) *
                     g_Config.m_RobotDamages[Weap2Index(WEAPON_CANNON0)].damage;
        repl->ParSetAdd(L"_turret_name",
                        g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Turrets")->ParGet(L"Tur1_Name"));
        repl->ParSetAdd(L"_turret_range",
                        g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Turrets")->ParGet(L"Tur1_Range"));
        repl->ParSetAdd(L"_turret_structure", g_Config.m_CannonsProps[0].m_Hitpoint / 10);
        repl->ParSetAdd(L"_turret_damage", utils::format(L"%d", damage / 10) + L"+" + utils::format(L"%d", damage / 10));

        for (int i = 0; i < MAX_RESOURCES; i++) {
            if (g_Config.m_CannonsProps[0].m_Resources[i]) {
                repl->ParSetAdd(L"_turret_res" + utils::format(L"%d", i + 1),
                                utils::format(L"%d", g_Config.m_CannonsProps[0].m_Resources[i]));
            }
            else {
                repl->ParSetAdd(L"_turret_res" + utils::format(L"%d", i + 1), L"");
            }
        }
    }
    else if (element_name == IF_BUILD_TUR2) {
        int damage = Float2Int(1.0f / (g_Config.m_WeaponCooldown[Weap2Index(WEAPON_CANNON1)] / 1000.0f)) *
                     g_Config.m_RobotDamages[Weap2Index(WEAPON_CANNON1)].damage;
        repl->ParSetAdd(L"_turret_name",
                        g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Turrets")->ParGet(L"Tur2_Name"));
        repl->ParSetAdd(L"_turret_range",
                        g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Turrets")->ParGet(L"Tur2_Range"));
        repl->ParSetAdd(L"_turret_structure", g_Config.m_CannonsProps[1].m_Hitpoint / 10);
        repl->ParSetAdd(L"_turret_damage", utils::format(L"%d", damage / 10));
        for (int i = 0; i < MAX_RESOURCES; i++) {
            if (g_Config.m_CannonsProps[1].m_Resources[i]) {
                repl->ParSetAdd(L"_turret_res" + utils::format(L"%d", i + 1),
                                utils::format(L"%d", g_Config.m_CannonsProps[1].m_Resources[i]));
            }
            else {
                repl->ParSetAdd(L"_turret_res" + utils::format(L"%d", i + 1), L"");
            }
        }
    }
    else if (element_name == IF_BUILD_TUR3) {
        int damage = Float2Int(1.0f / (g_Config.m_WeaponCooldown[Weap2Index(WEAPON_CANNON2)] / 1000.0f)) *
                     g_Config.m_RobotDamages[Weap2Index(WEAPON_CANNON2)].damage;
        repl->ParSetAdd(L"_turret_name",
                        g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Turrets")->ParGet(L"Tur3_Name"));
        repl->ParSetAdd(L"_turret_range",
                        g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Turrets")->ParGet(L"Tur3_Range"));
        repl->ParSetAdd(L"_turret_structure", g_Config.m_CannonsProps[2].m_Hitpoint / 10);
        repl->ParSetAdd(L"_turret_damage", utils::format(L"%d", damage / 10));
        for (int i = 0; i < MAX_RESOURCES; i++) {
            if (g_Config.m_CannonsProps[2].m_Resources[i]) {
                repl->ParSetAdd(L"_turret_res" + utils::format(L"%d", i + 1),
                                utils::format(L"%d", g_Config.m_CannonsProps[2].m_Resources[i]));
            }
            else {
                repl->ParSetAdd(L"_turret_res" + utils::format(L"%d", i + 1), L"");
            }
        }
    }
    else if (element_name == IF_BUILD_TUR4) {
        int damage = Float2Int(1.0f / (g_Config.m_WeaponCooldown[Weap2Index(WEAPON_CANNON3)] / 1000.0f)) *
                     g_Config.m_RobotDamages[Weap2Index(WEAPON_CANNON3)].damage;
        repl->ParSetAdd(L"_turret_name",
                        g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Turrets")->ParGet(L"Tur4_Name"));
        repl->ParSetAdd(L"_turret_range",
                        g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Turrets")->ParGet(L"Tur4_Range"));
        repl->ParSetAdd(L"_turret_structure", g_Config.m_CannonsProps[3].m_Hitpoint / 10);
        repl->ParSetAdd(L"_turret_damage", utils::format(L"%d", damage / 10) + L"+" + utils::format(L"%d", damage / 10));

        for (int i = 0; i < MAX_RESOURCES; i++) {
            if (g_Config.m_CannonsProps[3].m_Resources[i]) {
                repl->ParSetAdd(L"_turret_res" + utils::format(L"%d", i + 1),
                                utils::format(L"%d", g_Config.m_CannonsProps[3].m_Resources[i]));
            }
            else {
                repl->ParSetAdd(L"_turret_res" + utils::format(L"%d", i + 1), L"");
            }
        }
    }
    else if (element_name == IF_CALL_FROM_HELL) {
        repl->ParSetAdd(L"_ch_cant", L"");
        repl->ParSetAdd(L"_ch_can", L"");
        repl->ParSetAdd(L"_ch_time_min", L"");
        repl->ParSetAdd(L"_ch_time_sec", L"");

        if (g_MatrixMap->MaintenanceDisabled()) {
            repl->ParSetAdd(L"_ch_cant", L"1");
        }
        else {
            if (g_MatrixMap->BeforeMaintenanceTime()) {
                int milliseconds = g_MatrixMap->BeforeMaintenanceTime();
                int minutes = milliseconds / 60000;
                int seconds = milliseconds / 1000 - minutes * 60;
                repl->ParSetAdd(L"_ch_time_min", utils::format(L"%d", (minutes > 0 ? minutes : 0)));
                repl->ParSetAdd(L"_ch_time_sec", utils::format(L"%d", (seconds > 0 ? seconds : 0)));
            }
            else {
                repl->ParSetAdd(L"_ch_can", L"1");
            }
        }
    }
}

bool CIFaceList::CheckShowHintLogic(const std::wstring &element_name) {
    if (element_name == IF_BASE_COUNTHZ) {
        if (FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE)) {
            return false;
        }
    }
    return true;
}

void __stdcall CIFaceList::JumpToBuilding(void *o) {
    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();

    if (ps->m_CurrSel == BUILDING_SELECTED ||
        ps->m_CurrSel == BASE_SELECTED && ps->m_ActiveObject && ps->m_ActiveObject->IsBuilding()) {
        D3DXVECTOR2 tgt(ps->m_ActiveObject->GetGeoCenter().x, ps->m_ActiveObject->GetGeoCenter().y);
        g_MatrixMap->m_Camera.SetXYStrategy(tgt);
    }
}

void __stdcall CIFaceList::JumpToRobot(void *o) {
    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();

    if (!ps->GetCurGroup())
        return;

    D3DXVECTOR3 vec = ps->GetCurGroup()->GetObjectByN(ps->GetCurSelNum())->GetGeoCenter();
    g_MatrixMap->m_Camera.SetXYStrategy(D3DXVECTOR2(vec.x, vec.y));
}

void CIFaceList::CreateDynamicTurrets(CMatrixBuilding *building) {
    DTRACE();

    DeleteDynamicTurrets();
    if (!building)
        return;

    int tur_sheme = building->m_TurretsMax;

    CInterface *interfaces = m_First;

    CIFaceImage *image = NULL /*HNew(g_MatrixHeap) CIFaceImage*/;
    while (interfaces) {
        if (interfaces->m_strName == IF_MAIN) {
            for (int i = 0; i < tur_sheme; i++) {
                image = NULL;
                if (building->m_TurretsPlaces[i].m_CannonType == 1) {
                    image = interfaces->FindImageByName(std::wstring{IF_BT1_ICON});
                }
                else if (building->m_TurretsPlaces[i].m_CannonType == 2) {
                    image = interfaces->FindImageByName(std::wstring{IF_BT2_ICON});
                }
                else if (building->m_TurretsPlaces[i].m_CannonType == 3) {
                    image = interfaces->FindImageByName(std::wstring{IF_BT3_ICON});
                }
                else if (building->m_TurretsPlaces[i].m_CannonType == 4) {
                    image = interfaces->FindImageByName(std::wstring{IF_BT4_ICON});
                }
                if (image) {
                    float x = 0;
                    if (tur_sheme == 1) {
                        x = (float)g_IFaceList->m_DynamicTX[i];
                    }
                    else if (tur_sheme == 2) {
                        x = (float)g_IFaceList->m_DynamicTX[1 + i];
                    }
                    else if (tur_sheme == 3) {
                        x = (float)g_IFaceList->m_DynamicTX[3 + i];
                    }
                    else if (tur_sheme == 4) {
                        x = (float)g_IFaceList->m_DynamicTX[6 + i];
                    }
                    CIFaceStatic *s =
                            interfaces->CreateStaticFromImage(x, (float)g_IFaceList->m_DynamicTY, 0.000001f, *image);

                    s->SetVisibility(true);
                    s->m_nId = DYNAMIC_TURRET;
                }
            }

            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::DeleteDynamicTurrets() {
    DTRACE();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();

    // icon

    CInterface *interfaces = m_First;
    while (interfaces) {
        if (interfaces->m_strName == IF_MAIN) {
            CIFaceElement *elements = interfaces->m_FirstElement;
            while (elements) {
                if (elements->m_Type == IFACE_DYNAMIC_STATIC && elements->m_nId == DYNAMIC_TURRET) {
                    elements = interfaces->DelElement(elements);
                    continue;
                }
                elements = elements->m_NextElement;
            }
            interfaces->SortElementsByZ();
            break;
        }
        interfaces = interfaces->m_NextInterface;
    }
}

void CIFaceList::BeginBuildTurret(int no) {
    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
    if (!ps->IsEnoughResources(g_Config.m_CannonsProps[no - 1].m_Resources))
        return;

    ps->m_CannonForBuild.Delete();
    CMatrixCannon *cannon = HNew(g_MatrixHeap) CMatrixCannon;
    cannon->m_Pos.x = g_MatrixMap->m_TraceStopPos.x;
    cannon->m_Pos.y = g_MatrixMap->m_TraceStopPos.y;
    cannon->SetSide(PLAYER_SIDE);
    cannon->UnitInit(no);
    cannon->m_Angle = 0;

    cannon->m_ShadowType = SHADOW_OFF;
    cannon->m_ShadowSize = 128;

    cannon->RNeed(MR_Matrix | MR_Graph);

    ps->m_CannonForBuild.m_Cannon = cannon;
    ps->m_CannonForBuild.m_ParentBuilding = (CMatrixBuilding *)ps->m_ActiveObject;
    ps->m_CurrentAction = BUILDING_TURRET;
    g_MatrixMap->m_Cursor.SetPos(g_MatrixMap->m_Cursor.GetPosX(), (int)GetMainY() - 40);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SStateImages::SetStateLabelParams(int x, int y, int bound_x, int bound_y, int xAlign, int yAlign, int perenos,
                                       int smeX, int smeY, CRect clipRect, std::wstring t, std::wstring font, DWORD color) {
    m_Caption = t;
    m_x = x;
    m_y = y;
    m_boundX = bound_x;
    m_boundY = bound_y;
    m_xAlign = xAlign;
    m_yAlign = yAlign;
    m_Perenos = perenos;
    m_SmeX = smeX;
    m_SmeY = smeY;
    m_ClipRect = clipRect;
    m_Font = font;
    m_Color = color;
}
void SStateImages::SetStateText(bool copy) {
    CTextureManaged *texture = pImage;
    D3DLOCKED_RECT lr;

    if (g_RangersInterface) {
        SMGDRangersInterfaceText it;
        g_RangersInterface->m_RangersText((wchar *)m_Caption.c_str(), (wchar *)m_Font.c_str(), m_Color, m_boundX, m_boundY,
                                          m_xAlign, m_yAlign, m_Perenos, m_SmeX, m_SmeY, &m_ClipRect, &it);

        texture->LockRect(lr, 0);

        CBitmap bmsrc(g_CacheHeap);
        bmsrc.CreateRGBA(it.m_SizeX, it.m_SizeY, it.m_Pitch, it.m_Buf);
        CBitmap bmdes(g_CacheHeap);
        bmdes.CreateRGBA(Float2Int(TexWidth), Float2Int(TexHeight), lr.Pitch, lr.pBits);
        if (copy) {
            bmdes.Copy(CPoint(Float2Int(xTexPos) + m_x, Float2Int(yTexPos) + m_y), CPoint(m_boundX, m_boundY), bmsrc,
                       CPoint(0, 0));
        }
        else {
            bmdes.MergeWithAlpha(CPoint(Float2Int(xTexPos) + m_x, Float2Int(yTexPos) + m_y), CPoint(m_boundX, m_boundY),
                                 bmsrc, CPoint(0, 0));
        }
        g_RangersInterface->m_RangersTextClear(&it);

        texture->UnlockRect();
        texture->Unload();
    }
}

CBuf *CInterface::m_ClearRects;

#ifdef _DEBUG
void t_pause(void) {
    g_MatrixMap->Pause(true);
}
void t_unpause(void) {
    g_MatrixMap->Pause(false);
}
#endif
