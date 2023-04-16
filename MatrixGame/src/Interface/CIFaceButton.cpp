// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "CIFaceButton.h"
#include "CConstructor.h"
#include "CIFaceMenu.h"
#include "CInterface.h"
#include "MatrixHint.hpp"

int gIndex[] = {0, 1, 1, 2, 2, 0, 3, 4, 4, 5, 5, 3};

CIFaceButton::CIFaceButton() {
    m_Type = IFACE_PUSH_BUTTON;
    m_CurState = IFACE_NORMAL;
    // ZeroMemory(m_StateImages, sizeof(m_StateImages));
}

CIFaceButton::~CIFaceButton() {}

bool CIFaceButton::OnMouseLBDown() {
    if (GetState() == IFACE_DISABLED) {
        return true;
    }

    if (m_Type == IFACE_PUSH_BUTTON) {
        if (GetState() == IFACE_FOCUSED) {
            SetState(IFACE_PRESSED);
            Action(ON_PRESS);

            if (m_strName == IF_BASE_CONST_BUILD) {
                CSound::Play(S_BUILD_CLICK, SL_INTERFACE);
            }
            else if (m_strName == IF_BASE_CONST_CANCEL) {
                CSound::Play(S_CANCEL_CLICK, SL_INTERFACE);
            }
            else {
                CSound::Play(S_BCLICK, SL_INTERFACE);
            }
            return TRUE;
        }
    }
    else if (m_Type == IFACE_CHECK_BUTTON) {
        if (GetState() == IFACE_PRESSED) {
            SetState(IFACE_FOCUSED);
            m_DefState = IFACE_NORMAL;
            Action(ON_UN_PRESS);
            CSound::Play(S_BCLICK, SL_INTERFACE);
        }
        else if (GetState() == IFACE_FOCUSED) {
            SetState(IFACE_PRESSED);
            m_DefState = IFACE_PRESSED_UNFOCUSED;
            Action(ON_PRESS);
            CSound::Play(S_BCLICK, SL_INTERFACE);
            return TRUE;
        }
    }
    else if (m_Type == IFACE_CHECK_BUTTON_SPECIAL) {
        if (GetState() == IFACE_FOCUSED) {
            SetState(IFACE_PRESSED);
            m_DefState = IFACE_PRESSED_UNFOCUSED;
            Action(ON_PRESS);
            CSound::Play(S_BCLICK, SL_INTERFACE);
            return TRUE;
        }
    }
    else if (m_Type == IFACE_CHECK_PUSH_BUTTON) {
        if (GetState() == IFACE_FOCUSED) {
            SetState(IFACE_PRESSED);
            m_DefState = IFACE_NORMAL;
            Action(ON_PRESS);
            if (m_strName.find(L"conf")) // TODO: return value not checked, possibly a bug
            {
                CSound::Play(S_PRESET_CLICK, SL_INTERFACE);
            }
            else {
                CSound::Play(S_BCLICK, SL_INTERFACE);
            }

            return TRUE;
        }
        else if (GetState() == IFACE_PRESSED_UNFOCUSED) {
            //			SetState(IFACE_PRESSED);
            //			m_DefState = IFACE_PRESSED_UNFOCUSED;
            //            Action(ON_PRESS);
            return false;
        }
    }

    return FALSE;
}

void CIFaceButton::OnMouseLBUp() {
    if (g_IFaceList && g_IFaceList->m_CurrentHint && g_IFaceList->m_CurrentHintControlName == m_strName) {
        g_IFaceList->m_CurrentHint->Release();
        g_IFaceList->m_CurrentHint = NULL;
        g_IFaceList->m_CurrentHintControlName = L"";
    }

    if (m_Type == IFACE_PUSH_BUTTON) {
        if (GetState() == IFACE_PRESSED) {
            SetState(IFACE_FOCUSED);
            Action(ON_UN_PRESS);
        }
    }
    else if (m_Type == IFACE_CHECK_PUSH_BUTTON) {
        if (GetState() == IFACE_PRESSED) {
            if (m_DefState == IFACE_NORMAL) {
                SetState(IFACE_PRESSED_UNFOCUSED);
                m_DefState = IFACE_PRESSED_UNFOCUSED;
                Action(ON_UN_PRESS);
            }
            else if (m_DefState == IFACE_PRESSED_UNFOCUSED) {
                // SetState(IFACE_FOCUSED);
                // m_DefState = IFACE_NORMAL;
                // Action(ON_UN_PRESS);
            }
        }
    }
}

bool CIFaceButton::OnMouseMove(CPoint mouse) {
    if (GetVisibility() && ElementCatch(mouse) && ElementAlpha(mouse)) {
        if (g_IFaceList->m_CurrentHint && g_IFaceList->m_CurrentHintControlName != m_strName) {
            g_IFaceList->m_CurrentHint->Release();
            g_IFaceList->m_CurrentHint = NULL;
            g_IFaceList->m_CurrentHintControlName = L"";
        }

        if (g_IFaceList->m_CurrentHint == NULL && m_Hint.HintTemplate != L"") {
            if (g_IFaceList->CheckShowHintLogic(m_strName)) {
                g_IFaceList->AddHintReplacements(m_strName);
                CMatrixHint *hint = CMatrixHint::Build(m_Hint.HintTemplate, m_strName.c_str());
                int x = Float2Int(m_PosElInX) + m_Hint.x;
                int y = Float2Int(m_PosElInY) + m_Hint.y;
                g_IFaceList->CorrectCoordinates(g_ScreenX, g_ScreenY, x, y, hint->m_Width, hint->m_Height, m_strName);
                hint->Show(x, y);
                g_IFaceList->m_CurrentHint = hint;
                g_IFaceList->m_CurrentHintControlName = m_strName;
            }
        }

        if (m_Type == IFACE_PUSH_BUTTON) {
            if (GetState() == IFACE_NORMAL) {
                SetState(IFACE_FOCUSED);
                Action(ON_FOCUS);
                CSound::Play(S_BENTER, SL_INTERFACE);
            }
        }
        else if (m_Type == IFACE_CHECK_BUTTON || m_Type == IFACE_CHECK_BUTTON_SPECIAL) {
            if (GetState() == IFACE_NORMAL) {
                SetState(IFACE_FOCUSED);
                Action(ON_FOCUS);
                CSound::Play(S_BENTER, SL_INTERFACE);
            }
            else if (GetState() == IFACE_PRESSED_UNFOCUSED) {
                SetState(IFACE_PRESSED);
                Action(ON_FOCUS);
                CSound::Play(S_BENTER, SL_INTERFACE);
            }
        }
        else if (m_Type == IFACE_CHECK_PUSH_BUTTON) {
            if (GetState() == IFACE_NORMAL) {
                SetState(IFACE_FOCUSED);
                Action(ON_FOCUS);
                CSound::Play(S_BENTER, SL_INTERFACE);
            }
        }
        return TRUE;
    }
    else {
        if (g_IFaceList->m_CurrentHint && g_IFaceList->m_CurrentHintControlName == m_strName) {
            g_IFaceList->m_CurrentHint->Release();
            g_IFaceList->m_CurrentHint = NULL;
            g_IFaceList->m_CurrentHintControlName = L"";
        }
        return FALSE;
    }

    return FALSE;
}

bool CIFaceButton::OnMouseRBDown() {
    CPoint mouse = g_MatrixMap->m_Cursor.GetPos();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();
    if (ElementCatch(mouse) /*&& ElementAlpha(mouse)*/) {
        if (m_strName == IF_BASE_PILON1) {
            if (g_PopupMenu) {
                for (int i = 1; i < MENU_WEAPONNORM_ITEMS; i++) {
                    int kind = i;
                    if (kind == 5) {
                        kind++;
                    }
                    else if (kind > 5) {
                        kind += 2;
                    }
                    if (!player_side->m_ConstructPanel->IsEnoughResourcesForThisPieceOfShit(0, MRT_WEAPON,
                                                                                            ERobotUnitKind(kind))) {
                        g_PopupWeaponNormal[i].color = NERES_LABELS_COLOR;
                    }
                    else {
                        g_PopupWeaponNormal[i].color = DEFAULT_LABELS_COLOR;
                    }
                }
                g_PopupMenu->CreateMenu(MENU_PARENT_PILON1, MENU_WEAPONNORM_ITEMS, WEAPON_MENU_WIDTH,
                                        g_IFaceList->m_BaseX + 242, g_IFaceList->m_BaseY + 155, this,
                                        g_PopupWeaponNormal);
            }
        }
        else if (m_strName == IF_BASE_PILON2) {
            if (g_PopupMenu) {
                for (int i = 1; i < MENU_WEAPONNORM_ITEMS; i++) {
                    int kind = i;
                    if (kind == 5) {
                        kind++;
                    }
                    else if (kind > 5) {
                        kind += 2;
                    }
                    if (!player_side->m_ConstructPanel->IsEnoughResourcesForThisPieceOfShit(1, MRT_WEAPON,
                                                                                            ERobotUnitKind(kind))) {
                        g_PopupWeaponNormal[i].color = NERES_LABELS_COLOR;
                    }
                    else {
                        g_PopupWeaponNormal[i].color = DEFAULT_LABELS_COLOR;
                    }
                }
                g_PopupMenu->CreateMenu(MENU_PARENT_PILON2, MENU_WEAPONNORM_ITEMS, WEAPON_MENU_WIDTH,
                                        g_IFaceList->m_BaseX + 389, g_IFaceList->m_BaseY + 155, this,
                                        g_PopupWeaponNormal);
            }
        }
        else if (m_strName == IF_BASE_PILON3) {
            if (g_PopupMenu) {
                for (int i = 1; i < MENU_WEAPONNORM_ITEMS; i++) {
                    int kind = i;
                    if (kind == 5) {
                        kind++;
                    }
                    else if (kind > 5) {
                        kind += 2;
                    }
                    if (!player_side->m_ConstructPanel->IsEnoughResourcesForThisPieceOfShit(2, MRT_WEAPON,
                                                                                            ERobotUnitKind(kind))) {
                        g_PopupWeaponNormal[i].color = NERES_LABELS_COLOR;
                    }
                    else {
                        g_PopupWeaponNormal[i].color = DEFAULT_LABELS_COLOR;
                    }
                }
                g_PopupMenu->CreateMenu(MENU_PARENT_PILON3, MENU_WEAPONNORM_ITEMS, WEAPON_MENU_WIDTH,
                                        g_IFaceList->m_BaseX + 242, g_IFaceList->m_BaseY + 135, this,
                                        g_PopupWeaponNormal);
            }
        }
        else if (m_strName == IF_BASE_PILON4) {
            if (g_PopupMenu) {
                for (int i = 1; i < MENU_WEAPONNORM_ITEMS; i++) {
                    int kind = i;
                    if (kind == 5) {
                        kind++;
                    }
                    else if (kind > 5) {
                        kind += 2;
                    }
                    if (!player_side->m_ConstructPanel->IsEnoughResourcesForThisPieceOfShit(3, MRT_WEAPON,
                                                                                            ERobotUnitKind(kind))) {
                        g_PopupWeaponNormal[i].color = NERES_LABELS_COLOR;
                    }
                    else {
                        g_PopupWeaponNormal[i].color = DEFAULT_LABELS_COLOR;
                    }
                }
                g_PopupMenu->CreateMenu(MENU_PARENT_PILON4, MENU_WEAPONNORM_ITEMS, WEAPON_MENU_WIDTH,
                                        g_IFaceList->m_BaseX + 389, g_IFaceList->m_BaseY + 135, this,
                                        g_PopupWeaponNormal);
            }
        }
        else if (m_strName == IF_BASE_PILON5) {
            if (g_PopupMenu) {
                if (!player_side->m_ConstructPanel->IsEnoughResourcesForThisPieceOfShit(
                            4, MRT_WEAPON, ERobotUnitKind(RUK_WEAPON_MORTAR))) {
                    g_PopupWeaponExtern[1].color = NERES_LABELS_COLOR;
                }
                else {
                    g_PopupWeaponExtern[1].color = DEFAULT_LABELS_COLOR;
                }

                if (!player_side->m_ConstructPanel->IsEnoughResourcesForThisPieceOfShit(
                            4, MRT_WEAPON, ERobotUnitKind(RUK_WEAPON_BOMB))) {
                    g_PopupWeaponExtern[2].color = NERES_LABELS_COLOR;
                }
                else {
                    g_PopupWeaponExtern[2].color = DEFAULT_LABELS_COLOR;
                }
                g_PopupMenu->CreateMenu(MENU_PARENT_PILON5, MENU_WEAPONEXTERN_ITEMS, WEAPON_MENU_WIDTH,
                                        g_IFaceList->m_BaseX + 389, g_IFaceList->m_BaseY + 76, this,
                                        g_PopupWeaponExtern);
            }
        }
        else if (m_strName == IF_BASE_PILON_HEAD) {
            if (g_PopupMenu) {
                for (int i = 1; i < MENU_HEAD_ITEMS; i++) {
                    if (!player_side->m_ConstructPanel->IsEnoughResourcesForThisPieceOfShit(1, MRT_HEAD,
                                                                                            ERobotUnitKind(i))) {
                        g_PopupHead[i].color = NERES_LABELS_COLOR;
                    }
                    else {
                        g_PopupHead[i].color = DEFAULT_LABELS_COLOR;
                    }
                }
                g_PopupMenu->CreateMenu(MENU_PARENT_HEAD, MENU_HEAD_ITEMS, HEAD_MENU_WIDTH, g_IFaceList->m_BaseX + 315,
                                        g_IFaceList->m_BaseY + 76, this, g_PopupHead);
            }
        }
        else if (m_strName == IF_BASE_PILON_HULL) {
            if (g_PopupMenu) {
                for (int i = 1; i <= MENU_HULL_ITEMS; i++) {
                    int kind;

                    if (i == 1) {
                        kind = 6;
                    }
                    else if (i > 1) {
                        kind = i - 1;
                    }
                    if (!player_side->m_ConstructPanel->IsEnoughResourcesForThisPieceOfShit(1, MRT_ARMOR,
                                                                                            ERobotUnitKind(kind))) {
                        g_PopupHull[i - 1].color = NERES_LABELS_COLOR;
                    }
                    else {
                        g_PopupHull[i - 1].color = DEFAULT_LABELS_COLOR;
                    }
                }
                g_PopupMenu->CreateMenu(MENU_PARENT_HULL, MENU_HULL_ITEMS, HULL_MENU_WIDTH, g_IFaceList->m_BaseX + 321,
                                        g_IFaceList->m_BaseY + 148, this, g_PopupHull);
            }
        }
        else if (m_strName == IF_BASE_PILON_CHASSIS) {
            if (g_PopupMenu) {
                for (int i = 1; i <= MENU_CHASSIS_ITEMS; i++) {
                    if (!player_side->m_ConstructPanel->IsEnoughResourcesForThisPieceOfShit(1, MRT_CHASSIS,
                                                                                            ERobotUnitKind(i))) {
                        g_PopupChassis[i - 1].color = NERES_LABELS_COLOR;
                    }
                    else {
                        g_PopupChassis[i - 1].color = DEFAULT_LABELS_COLOR;
                    }
                }
                g_PopupMenu->CreateMenu(MENU_PARENT_CHASSIS, MENU_CHASSIS_ITEMS, CHASSIS_MENU_WIDTH,
                                        g_IFaceList->m_BaseX + 321, g_IFaceList->m_BaseY + 231, this, g_PopupChassis);
            }
        }

        return true;
    }
    else {
        if (FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE) && g_PopupMenu) {
            g_PopupMenu->ResetMenu(true);
        }
        return false;
    }
}