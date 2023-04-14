// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "CIFaceMenu.h"
#include "CInterface.h"
#include "CIFaceStatic.h"
#include "CIFaceImage.h"
#include "CIFaceElement.h"
#include "../MatrixGame.h"
#include "../MatrixSide.hpp"

CInterface *CIFaceMenu::m_MenuGraphics;
CIFaceMenu *g_PopupMenu;

CIFaceMenu::CIFaceMenu() {
    m_Width = 0;
    m_Height = 0;
    m_ElementsNum = 0;

    m_Ramka = NULL;
    m_Selector = NULL;
    m_Cursor = NULL;
    m_CursikImage = NULL;

    m_RamTex = NULL;
    m_CurMenuPos = 0;

    m_InterfaceParent = MENU_PARENT_UNDEF;

    m_Caller = NULL;
    m_RobotConfig = NULL;
}

CIFaceMenu::~CIFaceMenu() {
    if (m_RamTex) {
        CCache::Destroy(m_RamTex);
        m_RamTex = NULL;
    }

    if (m_RobotConfig) {
        HDelete(SRobotConfig, m_RobotConfig, g_MatrixHeap);
    }
}

bool CIFaceMenu::LoadMenuGraphics(CBlockPar &bp) {
    DTRACE();
    m_MenuGraphics->Load(bp, IF_POPUP_MENU);

    return true;
}

// width poshitan snaruzhi - max. dlina kakogo-to elementa
void CIFaceMenu::CreateMenu(EMenuParent parent, int elements, int width, int x, int y, CIFaceElement *caller,
                            SMenuItemText *labels, DWORD color) {
    DTRACE();

    //
    CInterface *main = NULL, *ifs = g_IFaceList->m_First;

    main = m_MenuGraphics;

    // while(ifs){
    //    if(ifs->m_strName == IF_BASE){
    //        main = ifs;
    //        break;
    //    }
    //    ifs = ifs->m_NextInterface;
    //}
    //
    // if(!main)
    //    return;

    m_InterfaceParent = parent;
    m_Caller = caller;

    int h = 0, w = 0, h_clean = 0;

    h += TOPLINE_HEIGHT + BOTTOMLINE_HEIGHT;
    h_clean = (elements * UNIT_HEIGHT);
    h += h_clean;
    width += CURSIK_WIDTH;
    w += width + RIGHTLINE_WIDTH + LEFTLINE_WIDTH;

    int cursik_hpos = 0;
    int idx = GetIndexFromTK(ERobotUnitType(Float2Int(caller->m_Param1)), ERobotUnitKind(Float2Int(caller->m_Param2)));
    cursik_hpos = (idx * UNIT_HEIGHT) + TOPLINE_HEIGHT;

    // Ramka preparation
    if (m_RamTex) {
        CCache::Destroy(m_RamTex);
        m_RamTex = NULL;
    }

    m_RamTex = CACHE_CREATE_TEXTUREMANAGED();
    // Draw the shit! to the fucking textures, fuck off y'all. KidRock motherfuckers!

    D3DLOCKED_RECT lr_src, lr_dest;

    CBitmap bm_src, bm_dest;

    int tex_height = 512, tex_width = 512;
    const int mod = 8;

    if (D3D_OK != m_RamTex->CreateLock(D3DFMT_A8R8G8B8, tex_height, tex_width, 1, lr_dest))
        return;

    bm_dest.CreateRGBA(tex_width, tex_height, lr_dest.Pitch, lr_dest.pBits);

    CIFaceImage *els = m_MenuGraphics->m_FirstImage;
    while (els) {
        if (els->m_strName == IF_POPUP_TOPLEFT) {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            // bm_src.SaveInPNG(L"topleft.png");
            bm_dest.MergeWithAlpha(CPoint(1, 0), CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src,
                                   sou_tp);
            els->m_Image->UnlockRect();
        }
        else if (els->m_strName == IF_POPUP_TOPRIGHT) {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            // bm_src.SaveInPNG(L"topright.png");
            bm_dest.MergeWithAlpha(CPoint(w - TOPRIGHT_WIDTH + 1, 0),
                                   CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            els->m_Image->UnlockRect();
        }
        else if (els->m_strName == IF_POPUP_BOTTOMLEFT) {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            // bm_src.SaveInPNG(L"bottomleft.png");
            bm_dest.MergeWithAlpha(CPoint(0, h - BOTTOMLEFT_HEIGHT - mod),
                                   CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            els->m_Image->UnlockRect();
        }
        else if (els->m_strName == IF_POPUP_BOTTOMRIGHT) {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            // bm_src.SaveInPNG(L"bottomright.png");
            bm_dest.MergeWithAlpha(CPoint(w - BOTTOMRIGHT_WIDTH - 4, h - BOTTOMRIGHT_HEIGHT - 1 - mod),
                                   CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            els->m_Image->UnlockRect();
        }
        else if (els->m_strName == IF_POPUP_LEFTLINE) {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            for (int i = 0; i < h_clean - mod; i++) {
                bm_dest.MergeWithAlpha(CPoint(1, TOPLEFT_HEIGHT + i),
                                       CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            }
            els->m_Image->UnlockRect();
        }
        else if (els->m_strName == IF_POPUP_RIGHTLINE) {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            for (int i = 0; i < h_clean - mod; i++) {
                bm_dest.MergeWithAlpha(CPoint(w - RIGHTLINE_WIDTH + 1, TOPRIGHT_HEIGHT + i),
                                       CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            }
            els->m_Image->UnlockRect();
        }
        else if (els->m_strName == IF_POPUP_TOPLINE) {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            for (int i = 0; i < width; i++) {
                bm_dest.MergeWithAlpha(CPoint(TOPLEFT_WIDTH + i + 1, 0),
                                       CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            }
            els->m_Image->UnlockRect();
        }
        else if (els->m_strName == IF_POPUP_BOTTOMLINE) {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            for (int i = 0; i < width; i++) {
                bm_dest.MergeWithAlpha(CPoint(BOTTOMLEFT_WIDTH + i, h - BOTTOMLEFT_HEIGHT - mod),
                                       CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            }
            els->m_Image->UnlockRect();
        }
        else if (els->m_strName == IF_POPUP_SEL) {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            for (int i = 0; i < elements; i++) {
                for (int j = 5; j < w - 9; j++) {
                    bm_dest.MergeWithAlpha(CPoint(j, 11 + (UNIT_HEIGHT * i)),
                                           CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
                }
            }
            els->m_Image->UnlockRect();
        }
        else if (els->m_strName == IF_POPUP_SELRIGHT) {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            for (int i = 0; i < elements; i++) {
                bm_dest.MergeWithAlpha(CPoint(w - 12, 11 + (UNIT_HEIGHT * i)),
                                       CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)), bm_src, sou_tp);
            }
            els->m_Image->UnlockRect();
        }
        else if (els->m_strName == IF_POPUP_SELMOUSE) {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            for (int j = 5; j < w - 9; j++) {
                bm_dest.MergeWithAlpha(CPoint(w + j, 0), CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)),
                                       bm_src, sou_tp);
            }
            els->m_Image->UnlockRect();
        }
        else if (els->m_strName == IF_POPUP_SELRIGHTMOUSE) {
            els->m_Image->LockRect(lr_src, 0);
            CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            bm_dest.MergeWithAlpha(CPoint((w * 2) - 12, 0), CPoint(Float2Int(els->m_Width), Float2Int(els->m_Height)),
                                   bm_src, sou_tp);
            els->m_Image->UnlockRect();
        }
        else if (els->m_strName == IF_POPUP_CURSIK) {
            m_CursikImage = els;
            // els->m_Image->LockRect(lr_src, 0);
            // CPoint sou_tp(Float2Int(els->m_xTexPos), Float2Int(els->m_yTexPos));
            // bm_src.CreateRGBA(Float2Int(els->m_TexWidth), Float2Int(els->m_TexHeight), lr_src.Pitch, lr_src.pBits);
            // bm_dest.MergeWithAlpha(CPoint(LEFT_SPACE+1,cursik_hpos), CPoint(Float2Int(els->m_Width),
            // Float2Int(els->m_Height)), bm_src, sou_tp); els->m_Image->UnlockRect();
        }
        els = els->m_NextImage;
    }

    int text_zone_width = w;
    int text_zone_height = 18;

    m_RamTex->UnlockRect();

    if (m_Ramka) {
        if (main && main->FindElementByName(m_Ramka->m_strName)) {
            LIST_DEL(m_Ramka, main->m_FirstElement, main->m_LastElement, m_PrevElement, m_NextElement);
        }
        HDelete(CIFaceStatic, m_Ramka, g_MatrixHeap);
    }

    m_Ramka = HNew(g_MatrixHeap) CIFaceStatic;
    m_Ramka->m_strName = IF_POPUP_RAMKA;
    // if(main->m_xPos > x){
    x -= Float2Int(main->m_xPos);
    //}
    // if(main->m_yPos > y){
    y -= Float2Int(main->m_yPos);
    //}
    m_Ramka->m_xPos = (float)x;
    m_Ramka->m_yPos = (float)y;
    m_Ramka->m_zPos = 0;
    m_Ramka->m_xSize = (float)w;
    m_Ramka->m_ySize = (float)h;
    m_Ramka->m_DefState = IFACE_NORMAL;

    m_Ramka->SetStateImage(IFACE_NORMAL, m_RamTex, 0, 0, (float)tex_height, (float)tex_width);

    m_Ramka->ElementGeomInit((void *)m_Ramka);
    main->AddElement(m_Ramka);
    m_Ramka->SetVisibility(true);

    // Selector preparation

    if (m_Selector) {
        if (main && main->FindElementByName(m_Selector->m_strName)) {
            LIST_DEL(m_Selector, main->m_FirstElement, main->m_LastElement, m_PrevElement, m_NextElement);
        }
        HDelete(CIFaceStatic, m_Selector, g_MatrixHeap);
    }

    m_Selector = HNew(g_MatrixHeap) CIFaceStatic;
    m_Selector->m_strName = IF_POPUP_SELECTOR;
    m_Selector->m_xPos = (float)x;
    m_Selector->m_yPos = (float)y + 11;
    m_Selector->m_zPos = 0;
    m_Selector->m_xSize = (float)w;
    m_Selector->m_ySize = (float)18;
    m_Selector->m_DefState = IFACE_NORMAL;

    m_Selector->SetStateImage(IFACE_NORMAL, m_RamTex, (float)w, 0, (float)tex_height, (float)tex_width);

    m_Selector->ElementGeomInit((void *)m_Selector);
    main->AddElement(m_Selector);
    m_Selector->SetVisibility(false);

    // Catchers
    CIFaceElement *e = main->m_FirstElement;
    while (e) {
        if (e->m_nId == POPUP_SELECTOR_CATCHERS_ID) {
            CIFaceElement *tmp_e = e->m_NextElement;
            LIST_DEL(e, main->m_FirstElement, main->m_LastElement, m_PrevElement, m_NextElement);
            HDelete(CIFaceStatic, (CIFaceStatic *)e, g_MatrixHeap);
            e = tmp_e;
            continue;
        }
        e = e->m_NextElement;
    }

    for (int i = 0; i < elements; i++) {
        CIFaceStatic *catcher = HNew(g_MatrixHeap) CIFaceStatic;
        catcher->m_strName = L"";
        catcher->m_xPos = (float)x;
        catcher->m_yPos = (float)y + 11 + (UNIT_HEIGHT * i);
        catcher->m_zPos = 0;
        catcher->m_xSize = (float)(w - 8);
        catcher->m_ySize = (float)19;
        catcher->m_DefState = IFACE_NORMAL;

        catcher->SetStateImage(IFACE_NORMAL, m_RamTex, (float)w * 2, (float)(11 + (UNIT_HEIGHT * i)), (float)tex_height,
                               (float)tex_width);

        catcher->ElementGeomInit((void *)catcher);
        main->AddElement(catcher);
        catcher->SetVisibility(false);
        catcher->m_nId = POPUP_SELECTOR_CATCHERS_ID;
        catcher->m_iParam = i;

        if (labels) {
            catcher->m_StateImages[IFACE_NORMAL].SetStateLabelParams(
                    LEFT_SPACE + 6, -3, text_zone_width, text_zone_height, 0, 1, 0, 0, 0,
                    CRect(0, 0, text_zone_width, text_zone_height), labels->text, std::wstring(L"Font.2Ranger"),
                    labels->color);
            catcher->m_StateImages[IFACE_NORMAL].SetStateText(true);
            labels++;
        }
    }
    // Cursor preparation

    if (m_Cursor) {
        if (main && main->FindElementByName(m_Cursor->m_strName)) {
            LIST_DEL(m_Cursor, main->m_FirstElement, main->m_LastElement, m_PrevElement, m_NextElement);
        }
        HDelete(CIFaceStatic, m_Cursor, g_MatrixHeap);
    }

    if (m_CursikImage) {
        m_Cursor = main->CreateStaticFromImage(float(x + LEFT_SPACE), float(y + cursik_hpos), 0, (*m_CursikImage));
        m_Cursor->SetVisibility(false);
        m_Cursor->m_strName = IF_POPUP_CURSOR;
    }

    // bm_dest.SaveInPNG(L"test_menu.png");
    SETFLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE);
    m_CurMenuPos = -1;
    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();

    if (m_RobotConfig) {
        HDelete(SRobotConfig, m_RobotConfig, g_MatrixHeap);
    }
    m_RobotConfig = HNew(g_MatrixHeap) SRobotConfig;
    *m_RobotConfig = ps->m_ConstructPanel->m_Configs[ps->m_ConstructPanel->m_CurrentConfig];
}

void CIFaceMenu::SetSelectorPos(const float &x, const float &y, int pos) {
    if (!m_Selector || m_CurMenuPos == pos)
        return;

    m_CurMenuPos = pos;
    m_Selector->RecalcPos(x, y, false);

    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
    if (!ps)
        return;

    CalcSelectedItem(false);
    // if(m_Caller) ps->m_ConstructPanel->FocusElement(m_Caller);
}

void CIFaceMenu::OnMenuItemPress() {
    bool rmode = false;
    if (m_Selector && m_Selector->ElementCatch(g_MatrixMap->m_Cursor.GetPos())) {
        CalcSelectedItem(true);
    }
    else {
        rmode = true;
    }
    ResetMenu(rmode);
}

void CIFaceMenu::ResetMenu(bool canceled) {
    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();

    // restore saved robot configuration
    if (m_RobotConfig) {
        if (canceled) {
            if (m_InterfaceParent == MENU_PARENT_PILON1) {
                ps->m_Constructor->SuperDjeans(MRT_WEAPON, m_RobotConfig->m_Weapon[0].m_nKind, 0);
            }
            else if (m_InterfaceParent == MENU_PARENT_PILON2) {
                ps->m_Constructor->SuperDjeans(MRT_WEAPON, m_RobotConfig->m_Weapon[1].m_nKind, 1);
            }
            else if (m_InterfaceParent == MENU_PARENT_PILON3) {
                ps->m_Constructor->SuperDjeans(MRT_WEAPON, m_RobotConfig->m_Weapon[2].m_nKind, 2);
            }
            else if (m_InterfaceParent == MENU_PARENT_PILON4) {
                ps->m_Constructor->SuperDjeans(MRT_WEAPON, m_RobotConfig->m_Weapon[3].m_nKind, 3);
            }
            else if (m_InterfaceParent == MENU_PARENT_PILON5) {
                ps->m_Constructor->SuperDjeans(MRT_WEAPON, m_RobotConfig->m_Weapon[4].m_nKind, 4);
            }
            else if (m_InterfaceParent == MENU_PARENT_HEAD) {
                ps->m_Constructor->SuperDjeans(MRT_HEAD, m_RobotConfig->m_Head.m_nKind, 0);
            }
            else if (m_InterfaceParent == MENU_PARENT_HULL) {
                ps->m_Constructor->SuperDjeans(MRT_ARMOR, m_RobotConfig->m_Hull.m_Unit.m_nKind, 0);
                for (int i = 0; i < 5 && m_RobotConfig->m_Weapon[i].m_nKind; i++) {
                    ps->m_Constructor->SuperDjeans(MRT_WEAPON, m_RobotConfig->m_Weapon[i].m_nKind, i);
                }
                ps->m_Constructor->SuperDjeans(MRT_HEAD, m_RobotConfig->m_Head.m_nKind, 0);
            }
            else if (m_InterfaceParent == MENU_PARENT_CHASSIS) {
                ps->m_Constructor->SuperDjeans(MRT_CHASSIS, m_RobotConfig->m_Chassis.m_nKind, 0);
            }
            // ps->m_Constructor->SuperDjeans
            // int cfg = ps->m_ConstructPanel->m_CurrentConfig;
            // ps->m_ConstructPanel->m_Configs[cfg] = *m_RobotConfig;
        }
        HDelete(SRobotConfig, m_RobotConfig, g_MatrixHeap);
        m_RobotConfig = NULL;
    }

    m_Caller = NULL;
    m_InterfaceParent = MENU_PARENT_UNDEF;
    m_CurMenuPos = -1;
    if (g_IFaceList)
        RESETFLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE);
}

void CIFaceMenu::CalcSelectedItem(bool set) {
    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
    if (!ps || m_InterfaceParent == MENU_PARENT_UNDEF)
        return;

    ERobotUnitType type;
    ERobotUnitKind kind;
    int pilon = -1;

    // RUK_WEAPON_MACHINEGUN      = 1,  1
    // RUK_WEAPON_CANNON          = 2,  2
    // RUK_WEAPON_MISSILE         = 3,  3
    // RUK_WEAPON_FLAMETHROWER    = 4,  4
    // RUK_WEAPON_MORTAR          = 5,
    // RUK_WEAPON_LASER           = 6,  5
    // RUK_WEAPON_BOMB            = 7,
    // RUK_WEAPON_PLASMA          = 8,  6
    // RUK_WEAPON_ELECTRIC        = 9,  7
    // RUK_WEAPON_REPAIR          = 10, 8

    if (m_InterfaceParent == MENU_PARENT_PILON1) {
        pilon = 0;
        type = MRT_WEAPON;

        if (m_CurMenuPos == 0) {
            kind = ERobotUnitKind(0);
        }
        else if (m_CurMenuPos == 5) {
            kind = ERobotUnitKind(m_CurMenuPos + 1);
        }
        else if (m_CurMenuPos > 5) {
            kind = ERobotUnitKind(m_CurMenuPos + 2);
        }
        else {
            kind = ERobotUnitKind(m_CurMenuPos);
        }
    }
    else if (m_InterfaceParent == MENU_PARENT_PILON2) {
        pilon = 1;
        type = MRT_WEAPON;

        if (m_CurMenuPos == 0) {
            kind = ERobotUnitKind(0);
        }
        else if (m_CurMenuPos == 5) {
            kind = ERobotUnitKind(m_CurMenuPos + 1);
        }
        else if (m_CurMenuPos > 5) {
            kind = ERobotUnitKind(m_CurMenuPos + 2);
        }
        else {
            kind = ERobotUnitKind(m_CurMenuPos);
        }
    }
    else if (m_InterfaceParent == MENU_PARENT_PILON3) {
        pilon = 2;
        type = MRT_WEAPON;

        if (m_CurMenuPos == 0) {
            kind = ERobotUnitKind(0);
        }
        else if (m_CurMenuPos == 5) {
            kind = ERobotUnitKind(m_CurMenuPos + 1);
        }
        else if (m_CurMenuPos > 5) {
            kind = ERobotUnitKind(m_CurMenuPos + 2);
        }
        else {
            kind = ERobotUnitKind(m_CurMenuPos);
        }
    }
    else if (m_InterfaceParent == MENU_PARENT_PILON4) {
        pilon = 3;
        type = MRT_WEAPON;

        if (m_CurMenuPos == 0) {
            kind = ERobotUnitKind(0);
        }
        else if (m_CurMenuPos == 5) {
            kind = ERobotUnitKind(m_CurMenuPos + 1);
        }
        else if (m_CurMenuPos > 5) {
            kind = ERobotUnitKind(m_CurMenuPos + 2);
        }
        else {
            kind = ERobotUnitKind(m_CurMenuPos);
        }
    }
    else if (m_InterfaceParent == MENU_PARENT_PILON5) {
        pilon = 4;
        type = MRT_WEAPON;

        if (m_CurMenuPos == 0) {
            kind = ERobotUnitKind(0);
        }
        else if (m_CurMenuPos == 1) {
            kind = RUK_WEAPON_MORTAR;
        }
        else if (m_CurMenuPos == 2) {
            kind = RUK_WEAPON_BOMB;
        }
    }
    else if (m_InterfaceParent == MENU_PARENT_HEAD) {
        pilon = 0;
        type = MRT_HEAD;

        if (m_CurMenuPos == 0) {
            kind = ERobotUnitKind(0);
        }
        else {
            kind = (ERobotUnitKind)m_CurMenuPos;
        }
    }
    else if (m_InterfaceParent == MENU_PARENT_HULL) {
        pilon = 0;
        type = MRT_ARMOR;
        int pos = m_CurMenuPos;
        if (pos == 0)
            pos = 6;
        kind = (ERobotUnitKind)pos;
    }
    else if (m_InterfaceParent == MENU_PARENT_CHASSIS) {
        pilon = 0;
        type = MRT_CHASSIS;
        kind = (ERobotUnitKind)(m_CurMenuPos + 1);
    }

    if (set)
        ps->m_Constructor->SuperDjeans(type, kind, pilon);
    else
        ps->m_Constructor->Djeans007(type, kind, pilon);
}

inline int GetIndexFromTK(ERobotUnitType type, ERobotUnitKind kind) {
    if (type == MRT_WEAPON && kind == 0)
        return 0;
    if (type == MRT_HEAD && kind == 0)
        return 0;

    if (type == MRT_WEAPON && kind == 1)
        return 1;
    if (type == MRT_WEAPON && kind == 2)
        return 2;
    if (type == MRT_WEAPON && kind == 3)
        return 3;
    if (type == MRT_WEAPON && kind == 4)
        return 4;
    if (type == MRT_WEAPON && kind == 5)
        return 1;
    if (type == MRT_WEAPON && kind == 6)
        return 5;
    if (type == MRT_WEAPON && kind == 7)
        return 2;
    if (type == MRT_WEAPON && kind == 8)
        return 6;
    if (type == MRT_WEAPON && kind == 9)
        return 7;
    if (type == MRT_WEAPON && kind == 10)
        return 8;

    if (type == MRT_HEAD && kind == 1)
        return 1;
    if (type == MRT_HEAD && kind == 2)
        return 2;
    if (type == MRT_HEAD && kind == 3)
        return 3;
    if (type == MRT_HEAD && kind == 4)
        return 4;

    if (type == MRT_CHASSIS && kind == 1)
        return 0;
    if (type == MRT_CHASSIS && kind == 2)
        return 1;
    if (type == MRT_CHASSIS && kind == 3)
        return 2;
    if (type == MRT_CHASSIS && kind == 4)
        return 3;
    if (type == MRT_CHASSIS && kind == 5)
        return 4;

    if (type == MRT_ARMOR && kind == 1)
        return 1;
    if (type == MRT_ARMOR && kind == 2)
        return 2;
    if (type == MRT_ARMOR && kind == 3)
        return 3;
    if (type == MRT_ARMOR && kind == 4)
        return 4;
    if (type == MRT_ARMOR && kind == 5)
        return 5;
    if (type == MRT_ARMOR && kind == 6)
        return 0;

    return 0;
}
