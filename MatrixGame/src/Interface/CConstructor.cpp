// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "CConstructor.h"
#include "CIFaceButton.h"
#include "../MatrixObjectBuilding.hpp"
#include "../MatrixRenderPipeline.hpp"
#include "CInterface.h"
#include "CCounter.h"
#include "CIFaceMenu.h"
#include "CHistory.h"
#include "../Effects/MatrixEffectWeapon.hpp"

CConstructor::CConstructor() {
    m_ViewPosX = 0;
    m_ViewPosY = 0;
    m_ViewWidthX = 0;
    m_ViewHeightY = 0;
    m_RobotPosX = 0;
    m_RobotPosY = 0;
    m_Side = 0;
    m_nUnitCnt = 0;
    m_ShadowType = SHADOW_OFF;
    m_ShadowSize = 0;
    m_nPos = 0;

    m_Robot = NULL;
    m_Build = NULL;
    m_Base = NULL;

    m_Robot = HNew(g_MatrixHeap) CMatrixRobotAI;
    m_Robot->m_PosX = 0;
    m_Robot->m_PosY = 0;
    m_Robot->m_Side = 0;

    m_Robot->m_ShadowType = SHADOW_OFF;
    m_Robot->m_ShadowSize = 128;

    m_Robot->m_Forward = D3DXVECTOR3(1, 0, 0);
    m_Robot->m_CurrState = ROBOT_EMBRYO;

    m_NewBorn = HNew(g_MatrixHeap) SNewBorn;
}

CConstructor::~CConstructor() {
    if (m_NewBorn)
        HDelete(SNewBorn, m_NewBorn, g_MatrixHeap);
    if (m_Robot) {
        ASSERT(g_MatrixHeap);
        HDelete(CMatrixRobotAI, m_Robot, g_MatrixHeap);
        m_Robot = NULL;
    }
}
SNewBorn *CConstructor::ProduceRobot(void *) {
    DTRACE();
    if (!m_Base || m_Base->m_State != BASE_CLOSED)
        return NULL;

    if (m_nUnitCnt) {
        // m_Base->m_BusyFlag.SetBusy(m_Build); // !SETBUSY
        m_Build = g_MatrixMap->StaticAdd<CMatrixRobotAI>();

        if (m_Head.m_nKind != 0) {
            m_Build->UnitInsert(0, MRT_HEAD, m_Head.m_nKind);
        }
        if (m_Armor.m_Unit.m_nKind != 0) {
            for (int nC = 0; nC < MAX_WEAPON_CNT; nC++) {
                if (m_Weapon[nC].m_Unit.m_nKind != 0) {
                    m_Build->WeaponInsert(0, MRT_WEAPON, m_Weapon[nC].m_Unit.m_nKind, m_Armor.m_Unit.m_nKind,
                                          m_Weapon[nC].m_Pos);
                }
            }

            m_Build->UnitInsert(0, MRT_ARMOR, m_Armor.m_Unit.m_nKind);
        }
        if (m_Chassis.m_nKind != 0) {
            m_Build->UnitInsert(0, MRT_CHASSIS, m_Chassis.m_nKind);
        }

        m_Build->m_ShadowType = g_Config.m_RobotShadow;
        m_Build->m_ShadowSize = 128;

        m_Build->m_Side = m_Side;
        m_Build->RobotWeaponInit();
        m_Build->m_PosX = m_Base->m_Pos.x;
        m_Build->m_PosY = m_Base->m_Pos.y;

        m_Build->CalcRobotMass();
        if (m_Base->m_Angle == 0)
            m_Build->m_Forward = D3DXVECTOR3(0, 1, 0);
        else if (m_Base->m_Angle == 1)
            m_Build->m_Forward = D3DXVECTOR3(-1, 0, 0);
        else if (m_Base->m_Angle == 2)
            m_Build->m_Forward = D3DXVECTOR3(0, -1, 0);
        else if (m_Base->m_Angle == 3)
            m_Build->m_Forward = D3DXVECTOR3(1, 0, 0);

        m_Build->m_HullForward = m_Build->m_Forward;

        // robot sozdan

        m_Build->RobotSpawn(m_Base);
        m_Build->JoinToGroup();

        m_NewBorn->m_Robot = m_Build;

        CMatrixSideUnit *si = g_MatrixMap->GetPlayerSide();

        if (si->GetTeam(0)->m_RobotCnt < si->GetTeam(1)->m_RobotCnt &&
            si->GetTeam(0)->m_RobotCnt < si->GetTeam(2)->m_RobotCnt)
            m_NewBorn->m_Team = 0;
        else if (si->GetTeam(1)->m_RobotCnt < si->GetTeam(0)->m_RobotCnt &&
                 si->GetTeam(1)->m_RobotCnt < si->GetTeam(2)->m_RobotCnt)
            m_NewBorn->m_Team = 1;
        else if (si->GetTeam(2)->m_RobotCnt < si->GetTeam(0)->m_RobotCnt &&
                 si->GetTeam(2)->m_RobotCnt < si->GetTeam(1)->m_RobotCnt)
            m_NewBorn->m_Team = 2;
        else
            m_NewBorn->m_Team = g_MatrixMap->Rnd(0, 2);

        m_NewBorn->m_Team = 0;
        // Team
        m_Build->SetTeam(m_NewBorn->m_Team);
        ResetConstruction();

        m_Build->CreateTextures();
        return m_NewBorn;
    }
    return NULL;
}

void CConstructor::StackRobot(void *pObject, int team) {
    DTRACE();

    bool crazy_bot = false;
    if (team < 0) {
        team = 0;
        crazy_bot = true;
    }

    //	if(!m_Base || m_Base->m_State != BASE_CLOSED)
    //		return;
    if (!m_Base || m_Base->m_BS.GetItemsCnt() >= 6)
        return;
    if (m_nUnitCnt) {
        // m_Base->m_BusyFlag.SetBusy(m_Build);
        m_Build = HNew(g_MatrixHeap) CMatrixRobotAI;

        if (crazy_bot) {
            m_Build->MarkCrazy();
        }

        if (m_Head.m_nKind != 0) {
            m_Build->UnitInsert(0, MRT_HEAD, m_Head.m_nKind);
        }
        if (m_Armor.m_Unit.m_nKind != 0) {
            for (int nC = 0; nC < MAX_WEAPON_CNT; nC++) {
                if (m_Weapon[nC].m_Unit.m_nKind != 0) {
                    m_Build->WeaponInsert(0, MRT_WEAPON, m_Weapon[nC].m_Unit.m_nKind, m_Armor.m_Unit.m_nKind,
                                          m_Weapon[nC].m_Pos);
                }
            }

            m_Build->UnitInsert(0, MRT_ARMOR, m_Armor.m_Unit.m_nKind);
        }
        if (m_Chassis.m_nKind != 0) {
            m_Build->UnitInsert(0, MRT_CHASSIS, m_Chassis.m_nKind);
        }

        m_Build->m_ShadowType = g_Config.m_RobotShadow;
        m_Build->m_ShadowSize = 128;

        m_Build->m_Side = m_Side;
        m_Build->RobotWeaponInit();
        m_Build->m_PosX = m_Base->m_Pos.x;
        m_Build->m_PosY = m_Base->m_Pos.y;

        m_Build->CalcRobotMass();
        if (m_Base->m_Angle == 0)
            m_Build->m_Forward = D3DXVECTOR3(0, 1, 0);
        else if (m_Base->m_Angle == 1)
            m_Build->m_Forward = D3DXVECTOR3(-1, 0, 0);
        else if (m_Base->m_Angle == 2)
            m_Build->m_Forward = D3DXVECTOR3(0, -1, 0);
        else if (m_Base->m_Angle == 3)
            m_Build->m_Forward = D3DXVECTOR3(1, 0, 0);

        m_Build->m_HullForward = m_Build->m_Forward;

        if (m_Base->m_Side == PLAYER_SIDE) {
            CMatrixSideUnit *si = g_MatrixMap->GetPlayerSide();
            int cfg_num = si->m_ConstructPanel->m_CurrentConfig;
        }

        m_Build->SetTeam(team);

        // robot sozdan

        //#ifdef _DEBUG
        //        RESETFLAG(g_Flags, SETBIT(22));
        //#endif

        m_Build->RNeed(MR_Graph);

        // if (!FLAG(g_Flags, SETBIT(22)))
        //{
        //    for(;;) ;

        //}

        if (m_Base->m_Side == PLAYER_SIDE)
            m_Build->CreateTextures();
        m_Build->SetBase(m_Base);
        GetConstructionName((CMatrixRobotAI *)m_Build);
        m_Base->m_BS.AddItem(m_Build);
    }
}

void __stdcall CConstructor::RemoteBuild(void *pObj) {
    DTRACE();
    if (m_Base->m_Side != PLAYER_SIDE) {
        return;
    }
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();

    int cfg_num = player_side->m_ConstructPanel->m_CurrentConfig;
    g_ConfigHistory->AddConfig(&player_side->m_ConstructPanel->m_Configs[cfg_num]);

    for (int i = 0; i < g_IFaceList->m_RCountControl->GetCounter(); i++) {
        StackRobot(pObj);
    }

    int res[MAX_RESOURCES];
    GetConstructionPrice(res);
    player_side->AddResourceAmount(TITAN, -res[TITAN] * g_IFaceList->m_RCountControl->GetCounter());
    player_side->AddResourceAmount(ELECTRONICS, -res[ELECTRONICS] * g_IFaceList->m_RCountControl->GetCounter());
    player_side->AddResourceAmount(ENERGY, -res[ENERGY] * g_IFaceList->m_RCountControl->GetCounter());
    player_side->AddResourceAmount(PLASMA, -res[PLASMA] * g_IFaceList->m_RCountControl->GetCounter());

    if (player_side && player_side->m_ConstructPanel) {
        player_side->m_ConstructPanel->ResetGroupNClose();
    }
    g_IFaceList->m_RCountControl->Reset();
    g_IFaceList->m_RCountControl->CheckUp();
}
void CConstructor::BeforeRender(void) {
    // static float za = 0;
    //   SinCos(za, &m_Robot->m_Forward.x, &m_Robot->m_Forward.y );
    m_Robot->m_HullForward = m_Robot->m_Forward;
    m_Robot->m_Forward.z = 0;
    // za += 0.001f;
    //   if (za > M_PI_MUL(2)) za -= M_PI_MUL(2);

    m_Robot->RChange(MR_Matrix | MR_Graph);
    m_Robot->BeforeDraw();
}

void CConstructor::Render(void) {
    DTRACE();
    D3DXMATRIX matWorld, matView, matProj, RotMat;
    D3DVIEWPORT9 ViewPort;

    ZeroMemory(&ViewPort, sizeof(D3DVIEWPORT9));

    ViewPort.X = (DWORD)m_ViewPosX;
    ViewPort.Y = (DWORD)m_ViewPosY;
    ViewPort.Width = m_ViewWidthX;
    ViewPort.Height = m_ViewHeightY;

    ViewPort.MinZ = 0.0f;
    ViewPort.MaxZ = 1.0f;
    // View robot

    ASSERT_DX(g_D3DD->SetViewport(&ViewPort));

    D3DLIGHT9 light;
    ZeroMemory(&light, sizeof(D3DLIGHT9));
    light.Type = D3DLIGHT_DIRECTIONAL;
    light.Diffuse.r = 1.0f;
    light.Diffuse.g = 1.0f;
    light.Diffuse.b = 1.0f;
    light.Ambient.r = 0.0f;
    light.Ambient.g = 0.0f;
    light.Ambient.b = 0.0f;
    light.Specular.r = 1.0f;
    light.Specular.g = 1.0f;
    light.Specular.b = 1.0f;

    // static float za = 0;
    // if (g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_N) {za-=0.1f;g_MatrixMap->m_KeyDown = false;}
    // if (g_MatrixMap->m_KeyDown && g_MatrixMap->m_KeyScan == KEY_M) {za+=0.1f;g_MatrixMap->m_KeyDown = false;}
    // float s,c;
    // SinCos(za, &s, &c);

    // light.Direction	= D3DXVECTOR3(s, c, 0);
    light.Direction = D3DXVECTOR3(-0.82242596f, 0.56887215f, 0);
    // D3DXVec3Normalize((D3DXVECTOR3 *)&light.Direction, (D3DXVECTOR3 *)&light.Direction);
    ASSERT_DX(g_D3DD->SetLight(0, &light));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, 0xFF808080));

    if (FLAG(g_Flags, GFLAG_STENCILAVAILABLE)) {
        ASSERT_DX(g_D3DD->Clear(0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_XRGB(255, 0, 0), 1.0f, 0));
    }
    else {
        ASSERT_DX(g_D3DD->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 0, 0), 1.0f, 0));
    }

    float h = m_Robot->GetChassisHeight();

    D3DXMatrixIdentity(&matWorld);
    auto tmp1 = D3DXVECTOR3(80, -30, h + 5);
    auto tmp2 = D3DXVECTOR3(0.0f, 0.0f, h);
    auto tmp3 = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
    D3DXMatrixLookAtLH(&matView, &tmp1, &tmp2, &tmp3);
    D3DXMatrixPerspectiveFovLH(
            &matProj, D3DX_PI / 4,
            float(m_ViewWidthX) / float(m_ViewHeightY) /*float(g_ScreenX)/float(g_ScreenX)*/ /*1.0f*/, 1.0f, 300.0f);
    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &matWorld));
    ASSERT_DX(g_D3DD->SetTransform(D3DTS_VIEW, &matView));
    ASSERT_DX(g_D3DD->SetTransform(D3DTS_PROJECTION, &matProj));

    // Render

    D3DXMATRIX imatView;
    D3DXMatrixInverse(&imatView, NULL, &matView);

    m_Robot->SetInterfaceDraw(true);
    g_MatrixMap->m_Camera.SetDrawNowParams(imatView, D3DXVECTOR3(80, -30, h + 5));
    CVectorObject::DrawBegin();
    if (m_nUnitCnt) {
        m_Robot->Draw();
    }
    CVectorObject::DrawEnd();

    // Return old
    g_MatrixMap->m_Camera.RestoreCameraParams();

    ZeroMemory(&light, sizeof(D3DLIGHT9));
    light.Type = D3DLIGHT_DIRECTIONAL;
    light.Diffuse.r = GetColorR(g_MatrixMap->m_LightMainColorObj);
    light.Diffuse.g = GetColorG(g_MatrixMap->m_LightMainColorObj);
    light.Diffuse.b = GetColorB(g_MatrixMap->m_LightMainColorObj);
    light.Ambient.r = 0.0f;
    light.Ambient.g = 0.0f;
    light.Ambient.b = 0.0f;
    light.Specular.r = GetColorR(g_MatrixMap->m_LightMainColorObj);
    light.Specular.g = GetColorG(g_MatrixMap->m_LightMainColorObj);
    light.Specular.b = GetColorB(g_MatrixMap->m_LightMainColorObj);
    light.Direction = g_MatrixMap->m_LightMain;
    ASSERT_DX(g_D3DD->SetLight(0, &light));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_AMBIENT, g_MatrixMap->m_AmbientColorObj));
}

void __stdcall CConstructor::RemoteOperateUnit(void *pObj) {
    DTRACE();
    CIFaceButton *pButton = (CIFaceButton *)pObj;
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();

    if (!pButton || !player_side)
        return;

    CIFaceElement *super_el = player_side->m_ConstructPanel->m_FocusedElement;

    ERobotUnitType type = ERobotUnitType(Float2Int(pButton->m_Param1));
    ERobotUnitKind kind = ERobotUnitKind(Float2Int(pButton->m_Param2));
    int pilon = 0;

    int cfg_num = player_side->m_ConstructPanel->m_CurrentConfig;

    if (pButton->m_strName == IF_BASE_PILON1) {
        pilon = 0;
    }
    else if (pButton->m_strName == IF_BASE_PILON2) {
        pilon = 1;
    }
    else if (pButton->m_strName == IF_BASE_PILON3) {
        pilon = 2;
    }
    else if (pButton->m_strName == IF_BASE_PILON4) {
        pilon = 3;
    }
    else if (pButton->m_strName == IF_BASE_PILON5) {
        pilon = 4;
    }

    if (type == MRT_HEAD) {
        if (player_side->m_ConstructPanel->m_Configs[cfg_num].m_Head.m_nKind == 4) {
            kind = ERobotUnitKind(0);
        }
        else {
            int a = (int)player_side->m_ConstructPanel->m_Configs[cfg_num].m_Head.m_nKind;
            a++;
            kind = ERobotUnitKind(a);
        }
    }
    else if (type == MRT_ARMOR) {
        if (player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind == 6) {
            kind = ERobotUnitKind(1);
        }
        else {
            int a = (int)player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind;
            a++;
            kind = ERobotUnitKind(a);
        }
    }
    else if (type == MRT_CHASSIS) {
        if (player_side->m_ConstructPanel->m_Configs[cfg_num].m_Chassis.m_nKind == 5) {
            kind = ERobotUnitKind(1);
        }
        else {
            int a = (int)player_side->m_ConstructPanel->m_Configs[cfg_num].m_Chassis.m_nKind;
            a++;
            kind = ERobotUnitKind(a);
        }
    }
    else if (type == MRT_WEAPON && pilon != 4) {
        if (player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[pilon].m_nKind == 4 ||
            player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[pilon].m_nKind == 6) {
            int a = (int)player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[pilon].m_nKind;
            a += 2;
            kind = ERobotUnitKind(a);
            // kind = ERobotUnitKind(player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[pilon].m_nKind);
        }
        else if (player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[pilon].m_nKind == 10) {
            kind = ERobotUnitKind(0);
        }
        else {
            int a = (int)player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[pilon].m_nKind;
            a++;
            kind = ERobotUnitKind(a);
        }
    }
    else if (type == MRT_WEAPON && pilon == 4) {
        if (player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[4].m_nKind == 0) {
            kind = ERobotUnitKind(5);
        }
        else if (player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[4].m_nKind == 5) {
            kind = ERobotUnitKind(7);
        }
        else if (player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[4].m_nKind == 7) {
            kind = ERobotUnitKind(0);
        }
    }

    SuperDjeans(type, kind, pilon);
}

void CConstructor::SuperDjeans(ERobotUnitType type, ERobotUnitKind kind, int pilon, bool ld_from_history) {
    if (g_IFaceList && g_IFaceList->m_RCountControl)
        g_IFaceList->m_RCountControl->Reset();

    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();
    int cfg_num = player_side->m_ConstructPanel->m_CurrentConfig;
    SRobotConfig *old_cfg = NULL;

    if (type == MRT_HEAD) {
        player_side->m_ConstructPanel->m_Configs[cfg_num].m_Head.m_nKind = kind;
        CInterface::CopyElements(g_IFaceList->m_Head[int(kind)], g_IFaceList->m_HeadPilon);
    }
    else if (type == MRT_ARMOR) {
        if (!ld_from_history) {
            old_cfg = HNew(g_MatrixHeap) SRobotConfig;
            memcpy(old_cfg, &player_side->m_ConstructPanel->m_Configs[cfg_num], sizeof(SRobotConfig));

            CInterface::CopyElements(g_IFaceList->m_Head[0], g_IFaceList->m_HeadPilon);
            SuperDjeans(MRT_HEAD, ERobotUnitKind(0), 0);
        }

        player_side->m_ConstructPanel->ResetWeapon();
        g_IFaceList->WeaponPilonsInit();

        player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind = kind;
        CInterface::CopyElements(g_IFaceList->m_Armor[int(kind) - 1], g_IFaceList->m_ArmorPilon);
    }
    else if (type == MRT_CHASSIS) {
        player_side->m_ConstructPanel->m_Configs[cfg_num].m_Chassis.m_nKind = kind;
        CInterface::CopyElements(g_IFaceList->m_Chassis[int(kind) - 1], g_IFaceList->m_ChassisPilon);
    }
    else if (type == MRT_WEAPON && pilon != 4) {
        player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[pilon].m_nKind = kind;

        int fis_pilon = 0;
        int pilon_ost = pilon + 1;
        int weapon_num =
                g_MatrixMap
                        ->m_RobotWeaponMatrix[player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind -
                                              1]
                        .cnt;
        int t;
        for (t = 0; t < weapon_num && pilon_ost > 0; t++) {
            if (!(g_MatrixMap
                          ->m_RobotWeaponMatrix
                                  [player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind - 1]
                          .list[t]
                          .access_invert &
                  (1 << (6))) &&
                !(g_MatrixMap
                          ->m_RobotWeaponMatrix
                                  [player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind - 1]
                          .list[t]
                          .access_invert &
                  (1 << (4)))) {
                pilon_ost--;
            }
        }

        fis_pilon = t;

        m_Weapon[fis_pilon - 1].m_Unit.m_nType = MRT_WEAPON;
        m_Weapon[fis_pilon - 1].m_Unit.m_nKind = kind;
        m_Weapon[fis_pilon - 1].m_Pos = fis_pilon;

        CInterface::CopyElements(g_IFaceList->m_Weapon[int(kind)], g_IFaceList->m_WeaponPilon[pilon]);
        player_side->m_ConstructPanel->UnFocusElement(g_IFaceList->m_WeaponPilon[pilon]);
        player_side->m_ConstructPanel->FocusElement(g_IFaceList->m_WeaponPilon[pilon]);

        InsertUnits();
        GetConstructionName(g_MatrixMap->GetPlayerSide()->m_Constructor->GetRenderBot());
        g_IFaceList->CreateSummPrice();
        g_IFaceList->m_RCountControl->CheckUp();
        return;
    }
    else if (type == MRT_WEAPON && pilon == 4) {
        int pilon = 0;
        int common_weapon =
                g_MatrixMap
                        ->m_RobotWeaponMatrix[player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind -
                                              1]
                        .cnt;
        for (int t = 0; t < common_weapon; t++) {
            if ((g_MatrixMap
                         ->m_RobotWeaponMatrix[player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind -
                                               1]
                         .list[t]
                         .access_invert &
                 (1 << (6))) ||
                (g_MatrixMap
                         ->m_RobotWeaponMatrix[player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind -
                                               1]
                         .list[t]
                         .access_invert &
                 (1 << (4)))) {
                pilon = t;
                break;
            }
        }

        player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[4].m_nKind = kind;

        m_Weapon[pilon].m_Unit.m_nType = MRT_WEAPON;
        m_Weapon[pilon].m_Unit.m_nKind = kind;
        m_Weapon[pilon].m_Pos = pilon + 1;

        CInterface::CopyElements(g_IFaceList->m_Weapon[int(kind)], g_IFaceList->m_WeaponPilon[4]);

        player_side->m_ConstructPanel->UnFocusElement(g_IFaceList->m_WeaponPilon[4]);
        player_side->m_ConstructPanel->FocusElement(g_IFaceList->m_WeaponPilon[4]);

        InsertUnits();
        GetConstructionName(g_MatrixMap->GetPlayerSide()->m_Constructor->GetRenderBot());
        g_IFaceList->CreateSummPrice();
        g_IFaceList->m_RCountControl->CheckUp();
        return;
    }

    OperateUnit(type, kind);
    GetConstructionName(g_MatrixMap->GetPlayerSide()->m_Constructor->GetRenderBot());

    if (type == MRT_HEAD) {
        player_side->m_ConstructPanel->UnFocusElement(g_IFaceList->m_HeadPilon);
        player_side->m_ConstructPanel->FocusElement(g_IFaceList->m_HeadPilon);
    }
    else if (type == MRT_ARMOR) {
        //
        if (old_cfg) {
            //восстанавливаем обычные ружбайки
            int pilon = 0;
            for (int i = 0; i < MAX_WEAPON_CNT && pilon < m_Armor.m_MaxCommonWeaponCnt; i++) {
                if (old_cfg->m_Weapon[i].m_nKind && !(old_cfg->m_Weapon[i].m_nKind == RUK_WEAPON_BOMB ||
                                                      old_cfg->m_Weapon[i].m_nKind == RUK_WEAPON_MORTAR)) {
                    SuperDjeans(MRT_WEAPON, old_cfg->m_Weapon[i].m_nKind, pilon);
                    pilon++;
                }
                else {
                    if (i < m_Armor.m_MaxCommonWeaponCnt + m_Armor.m_MaxExtraWeaponCnt)
                        SuperDjeans(MRT_WEAPON, ERobotUnitKind(0), i);
                }
            }
            //восстанавливаем супер оружие
            if (m_Armor.m_MaxExtraWeaponCnt) {
                for (int i = 0; i < MAX_WEAPON_CNT; i++) {
                    if ((old_cfg->m_Weapon[i].m_nKind == RUK_WEAPON_BOMB ||
                         old_cfg->m_Weapon[i].m_nKind == RUK_WEAPON_MORTAR)) {
                        SuperDjeans(MRT_WEAPON, old_cfg->m_Weapon[i].m_nKind, 4);
                        break;
                    }
                }
            }
            SuperDjeans(MRT_HEAD, old_cfg->m_Head.m_nKind, 0);
            HDelete(SRobotConfig, old_cfg, g_MatrixHeap);
            old_cfg = NULL;
        }
        player_side->m_ConstructPanel->UnFocusElement(g_IFaceList->m_ArmorPilon);
        player_side->m_ConstructPanel->FocusElement(g_IFaceList->m_ArmorPilon);
    }
    else if (type == MRT_CHASSIS) {
        player_side->m_ConstructPanel->UnFocusElement(g_IFaceList->m_ChassisPilon);
        player_side->m_ConstructPanel->FocusElement(g_IFaceList->m_ChassisPilon);
    }

    g_IFaceList->CreateSummPrice();
    g_IFaceList->m_RCountControl->CheckUp();
}

void CConstructor::Djeans007(ERobotUnitType type, ERobotUnitKind kind, int pilon) {
    g_IFaceList->m_RCountControl->Reset();
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();
    int cfg_num = player_side->m_ConstructPanel->m_CurrentConfig;

    if (type == MRT_WEAPON && pilon != 4) {
        int fis_pilon = 0;
        int pilon_ost = pilon + 1;
        int weapon_num =
                g_MatrixMap
                        ->m_RobotWeaponMatrix[player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind -
                                              1]
                        .cnt;
        int t;
        for (t = 0; t < weapon_num && pilon_ost > 0; t++) {
            if (!(g_MatrixMap
                          ->m_RobotWeaponMatrix
                                  [player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind - 1]
                          .list[t]
                          .access_invert &
                  (1 << (6))) &&
                !(g_MatrixMap
                          ->m_RobotWeaponMatrix
                                  [player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind - 1]
                          .list[t]
                          .access_invert &
                  (1 << (4)))) {
                pilon_ost--;
            }
        }

        fis_pilon = t;

        m_Weapon[fis_pilon - 1].m_Unit.m_nType = MRT_WEAPON;
        m_Weapon[fis_pilon - 1].m_Unit.m_nKind = kind;
        m_Weapon[fis_pilon - 1].m_Pos = fis_pilon;

        player_side->m_ConstructPanel->SetLabelsAndPrice(MRT_WEAPON, kind);

        InsertUnits();
        GetConstructionName(g_MatrixMap->GetPlayerSide()->m_Constructor->GetRenderBot());
        player_side->m_ConstructPanel->m_FocusedElement = g_IFaceList->m_Weapon[kind];
        g_IFaceList->CreateSummPrice();
        g_IFaceList->m_RCountControl->CheckUp();
        return;
    }
    else if (type == MRT_WEAPON && pilon == 4) {
        int pilon = 0;
        int common_weapon =
                g_MatrixMap
                        ->m_RobotWeaponMatrix[player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind -
                                              1]
                        .cnt;
        for (int t = 0; t < common_weapon; t++) {
            if ((g_MatrixMap
                         ->m_RobotWeaponMatrix[player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind -
                                               1]
                         .list[t]
                         .access_invert &
                 (1 << (6))) ||
                (g_MatrixMap
                         ->m_RobotWeaponMatrix[player_side->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind -
                                               1]
                         .list[t]
                         .access_invert &
                 (1 << (4)))) {
                pilon = t;
                break;
            }
        }

        m_Weapon[pilon].m_Unit.m_nType = MRT_WEAPON;
        m_Weapon[pilon].m_Unit.m_nKind = kind;
        m_Weapon[pilon].m_Pos = pilon + 1;

        player_side->m_ConstructPanel->SetLabelsAndPrice(MRT_WEAPON, kind);

        InsertUnits();
        GetConstructionName(g_MatrixMap->GetPlayerSide()->m_Constructor->GetRenderBot());
        player_side->m_ConstructPanel->m_FocusedElement = g_IFaceList->m_Weapon[kind];

        g_IFaceList->CreateSummPrice();

        g_IFaceList->m_RCountControl->CheckUp();
        return;
    }

    OperateUnit(type, kind);
    g_IFaceList->CreateSummPrice();
    GetConstructionName(g_MatrixMap->GetPlayerSide()->m_Constructor->GetRenderBot());

    if (type == MRT_HEAD) {
        player_side->m_ConstructPanel->m_FocusedElement = g_IFaceList->m_Head[int(kind)];
        player_side->m_ConstructPanel->SetLabelsAndPrice(MRT_HEAD, kind);
    }
    else if (type == MRT_ARMOR) {
        // OperateUnit(MRT_HEAD, ERobotUnitKind(0));
        int try_common = g_MatrixMap->m_RobotWeaponMatrix[kind - 1].common;
        int try_extra = g_MatrixMap->m_RobotWeaponMatrix[kind - 1].extra;
        SUnit tmp_weapon[MAX_WEAPON_CNT];
        memcpy(tmp_weapon, player_side->m_ConstructPanel->m_Configs[cfg_num].m_Weapon, sizeof(tmp_weapon));

        for (int i = 0; i < MAX_WEAPON_CNT && try_common; i++) {
            if (tmp_weapon[i].m_nKind && tmp_weapon[i].m_nKind != RUK_WEAPON_BOMB &&
                tmp_weapon[i].m_nKind != RUK_WEAPON_MORTAR) {
                Djeans007(MRT_WEAPON, tmp_weapon[i].m_nKind, i);
                try_common--;
            }
        }
        for (int i = 0; i < MAX_WEAPON_CNT && try_extra; i++) {
            if (tmp_weapon[i].m_nKind == RUK_WEAPON_BOMB || tmp_weapon[i].m_nKind == RUK_WEAPON_MORTAR) {
                Djeans007(MRT_WEAPON, tmp_weapon[i].m_nKind, 4);
                try_extra--;
            }
        }
        player_side->m_ConstructPanel->SetLabelsAndPrice(MRT_ARMOR, kind);
        player_side->m_ConstructPanel->m_FocusedElement = g_IFaceList->m_Armor[int(kind) - 1];
    }
    else if (type == MRT_CHASSIS) {
        player_side->m_ConstructPanel->SetLabelsAndPrice(MRT_CHASSIS, kind);
        player_side->m_ConstructPanel->m_FocusedElement = g_IFaceList->m_Chassis[int(kind) - 1];
    }
    g_IFaceList->m_RCountControl->CheckUp();
}

void CConstructor::OperateUnit(ERobotUnitType type, ERobotUnitKind kind) {
    DTRACE();

    if (type == MRT_CHASSIS) {
        m_Chassis.m_nType = MRT_CHASSIS;
        m_Chassis.m_nKind = kind;
    }
    else if (type == MRT_ARMOR && m_Chassis.m_nKind != 0) {
        ZeroMemory(m_Weapon, MAX_WEAPON_CNT * sizeof(SWeaponUnit));
        m_Armor.m_Unit.m_nType = MRT_ARMOR;
        m_Armor.m_Unit.m_nKind = kind;

        m_Armor.m_MaxCommonWeaponCnt = g_MatrixMap->m_RobotWeaponMatrix[kind - 1].common;
        m_Armor.m_MaxExtraWeaponCnt = g_MatrixMap->m_RobotWeaponMatrix[kind - 1].extra;
    }
    else if (type == MRT_HEAD && m_Armor.m_Unit.m_nKind > 0) {
        m_Head.m_nType = MRT_HEAD;
        m_Head.m_nKind = kind;
    }
    else if (type == MRT_WEAPON && m_Armor.m_Unit.m_nKind > 0) {
        int pos = CheckWeaponLegality(m_Weapon, kind, m_Armor.m_Unit.m_nKind);
        if (pos >= 0) {
            m_Weapon[pos].m_Unit.m_nKind = kind;
            m_Weapon[pos].m_Unit.m_nType = MRT_WEAPON;
            m_Weapon[pos].m_Pos = pos + 1;
        }
    }

    int we_are = 0;
    CMatrixSideUnit *player_side = g_MatrixMap->GetPlayerSide();
    int cfg_num = player_side->m_ConstructPanel->m_CurrentConfig;

    if (player_side && g_MatrixMap->GetPlayerSide()->m_Constructor == this) {
        we_are = 1;
    }

    m_nUnitCnt = 0;
    if (m_Chassis.m_nKind != 0) {
        m_nUnitCnt++;
    }
    if (m_Armor.m_Unit.m_nKind != 0) {
        m_nUnitCnt++;
    }

    if (m_Head.m_nKind != 0) {
        m_nUnitCnt++;
    }

    for (int nC = 0; nC < MAX_WEAPON_CNT; nC++) {
        if (m_Weapon[nC].m_Unit.m_nKind != 0) {
            m_nUnitCnt++;
        }
    }

    if (m_nUnitCnt > 0)
        InsertUnits();
}

int CConstructor::CheckWeaponLegality(SWeaponUnit *weapons, int weaponKind, int armorKind) {
    DTRACE();

    if (weaponKind != 5 && weaponKind != 7) {
        int fis_pilon = 0;
        int weapon_num = g_MatrixMap->m_RobotWeaponMatrix[armorKind - 1].cnt;
        int pilon_ost = weapon_num;
        for (int t = 0; t < weapon_num; t++) {
            if (!(g_MatrixMap->m_RobotWeaponMatrix[armorKind - 1].list[t].access_invert & (1 << (6))) &&
                !(g_MatrixMap->m_RobotWeaponMatrix[armorKind - 1].list[t].access_invert & (1 << (4))) &&
                weapons[t].m_Unit.m_nKind == 0) {
                return t;
            }
        }
    }
    else {
        int pilon = 0;
        int common_weapon = g_MatrixMap->m_RobotWeaponMatrix[armorKind - 1].cnt;
        for (int t = 0; t < common_weapon; t++) {
            if ((g_MatrixMap->m_RobotWeaponMatrix[armorKind - 1].list[t].access_invert & (1 << (6))) ||
                (g_MatrixMap->m_RobotWeaponMatrix[armorKind - 1].list[t].access_invert & (1 << (4)))) {
                return t;
            }
        }
    }

    return -1;
}

void CConstructor::InsertUnits() {
    DTRACE();
    m_Robot->UnitClear();
    if (m_Head.m_nKind != 0) {
        m_Robot->UnitInsert(0, MRT_HEAD, m_Head.m_nKind);
    }
    if (m_Armor.m_Unit.m_nKind != 0) {
        for (int nC = 0; nC < MAX_WEAPON_CNT; nC++) {
            if (m_Weapon[nC].m_Unit.m_nKind != 0) {
                m_Robot->WeaponInsert(0, MRT_WEAPON, m_Weapon[nC].m_Unit.m_nKind, m_Armor.m_Unit.m_nKind,
                                      m_Weapon[nC].m_Pos);
            }
        }

        m_Robot->UnitInsert(0, MRT_ARMOR, m_Armor.m_Unit.m_nKind);
    }
    if (m_Chassis.m_nKind != 0) {
        m_Robot->UnitInsert(0, MRT_CHASSIS, m_Chassis.m_nKind);
        D3DXMatrixIdentity(&m_Robot->m_Unit[0].m_Matrix);
    }
    m_Robot->RChange(MR_Matrix | MR_Graph);
}

void CConstructor::ResetConstruction() {
    DTRACE();
    ZeroMemory(&m_Armor, sizeof(SArmorUnit));
    ZeroMemory(&m_Head, sizeof(SUnit));
    ZeroMemory(m_Weapon, MAX_WEAPON_CNT * sizeof(SUnit));
    ZeroMemory(&m_Chassis, sizeof(SUnit));

    m_nUnitCnt = 0;
}

void CConstructor::BuildSpecialBot(const SSpecialBot &bot) {
    DTRACE();
    // Chassis
    OperateUnit(MRT_CHASSIS, bot.m_Chassis.m_nKind);
    // ARMOR
    OperateUnit(MRT_ARMOR, bot.m_Armor.m_Unit.m_nKind);
    // WEAPON
    for (int i = 0; i < MAX_WEAPON_CNT; i++) {
        OperateUnit(MRT_WEAPON, bot.m_Weapon[i].m_Unit.m_nKind);
    }
    // HEAD
    OperateUnit(MRT_HEAD, bot.m_Head.m_nKind);
    StackRobot(NULL, bot.m_Team);
}

void CConstructor::OperateCurrentConstruction() {
    DTRACE();
    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();

    ResetConstruction();

    int cfg_num = ps->m_ConstructPanel->m_CurrentConfig;

    OperateUnit(MRT_CHASSIS, ps->m_ConstructPanel->m_Configs[cfg_num].m_Chassis.m_nKind);
    OperateUnit(MRT_ARMOR, ps->m_ConstructPanel->m_Configs[cfg_num].m_Hull.m_Unit.m_nKind);

    for (int i = 0; i < MAX_WEAPON_CNT; i++) {
        if (ps->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[i].m_nKind) {
            OperateUnit(MRT_WEAPON, ps->m_ConstructPanel->m_Configs[cfg_num].m_Weapon[i].m_nKind);
        }
    }
    OperateUnit(MRT_HEAD, ps->m_ConstructPanel->m_Configs[cfg_num].m_Head.m_nKind);
}

void CConstructor::GetConstructionPrice(int *res) {
    CMatrixRobotAI *robot = m_Robot;
    ZeroMemory(res, sizeof(int) * MAX_RESOURCES);

    for (int i = 0; i < robot->m_UnitCnt; i++) {
        int price[MAX_RESOURCES];
        ZeroMemory(price, sizeof(price));

        if (robot->m_Unit[i].m_Type == MRT_CHASSIS) {
            memcpy(price, &g_Config.m_Price[CHASSIS1_TITAN + (robot->m_Unit[i].u1.s1.m_Kind - 1) * 4], sizeof(int) * 4);
        }
        else if (robot->m_Unit[i].m_Type == MRT_ARMOR) {
            memcpy(price, &g_Config.m_Price[ARMOR1_TITAN + (robot->m_Unit[i].u1.s1.m_Kind - 1) * 4], sizeof(int) * 4);
        }
        else if (robot->m_Unit[i].m_Type == MRT_WEAPON) {
            memcpy(price, &g_Config.m_Price[WEAPON1_TITAN + (robot->m_Unit[i].u1.s1.m_Kind - 1) * 4], sizeof(int) * 4);
        }
        else if (robot->m_Unit[i].m_Type == MRT_HEAD) {
            memcpy(price, &g_Config.m_Price[HEAD1_TITAN + (robot->m_Unit[i].u1.s1.m_Kind - 1) * 4], sizeof(int) * 4);
        }

        for (int j = 0; j < MAX_RESOURCES; j++) {
            res[j] += price[j];
        }
    }
}

int CConstructor::GetConstructionStructure() {
    CMatrixRobotAI *robot = m_Robot;
    int structure = 0;

    for (int i = 0; i < robot->m_UnitCnt; i++) {
        if (robot->m_Unit[i].m_Type == MRT_CHASSIS) {
            structure += Float2Int(g_Config.m_ItemChars[CHASSIS1_STRUCTURE + (robot->m_Unit[i].u1.s1.m_Kind - 1) * 6]);
        }
        else if (robot->m_Unit[i].m_Type == MRT_ARMOR) {
            structure += Float2Int(g_Config.m_ItemChars[ARMOR1_STRUCTURE + (robot->m_Unit[i].u1.s1.m_Kind - 1) * 2]);
        }
    }

    float up = 0;
    for (int i = 0; i < robot->m_UnitCnt; i++) {
        if (robot->m_Unit[i].m_Type == MRT_HEAD) {
            CBlockPar *bp = g_MatrixData->BlockGet(PAR_SOURCE_CHARS)->BlockGet(L"Heads");

            const wchar *hn = L"";
            if (robot->m_Unit[i].u1.s1.m_Kind == RUK_HEAD_BLOCKER) {
                hn = L"S";
            }
            else if (robot->m_Unit[i].u1.s1.m_Kind == RUK_HEAD_DYNAMO) {
                hn = L"D";
            }
            else if (robot->m_Unit[i].u1.s1.m_Kind == RUK_HEAD_LOCKATOR) {
                hn = L"L";
            }
            else if (robot->m_Unit[i].u1.s1.m_Kind == RUK_HEAD_FIREWALL) {
                hn = L"F";
            }

            bp = bp->BlockGetNE(hn);
            if (bp) {
                // up = float(bp->ParGetNE(L"HIT_POINT_ADD").GetDouble() / 100.0f);
                // if (up < -1.0f) up = -1.0f;
                structure += bp->ParGetNE(L"HIT_POINT_ADD").GetInt();
            }
            break;
        }
    }

    // return Float2Int((float(structure) + float(structure)*up) * 0.1f);
    return structure / 10;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void __stdcall CConstructorPanel::RemoteFocusElement(void *object) {
    FocusElement((CIFaceElement *)object);
}
void __stdcall CConstructorPanel::RemoteUnFocusElement(void *object) {
    if (!FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE))
        UnFocusElement((CIFaceElement *)object);
}

void CConstructorPanel::FocusElement(CIFaceElement *element) {
    if (m_FocusedElement != element) {
        m_FocusedElement = element;

        CIFaceElement *el = m_FocusedElement;

        ERobotUnitType type = MRT_EMPTY;
        ERobotUnitKind kind = RUK_UNKNOWN;

        int cfg_num = m_CurrentConfig;

        if (el->m_strName == IF_BASE_PILON1) {
            type = MRT_WEAPON;
            kind = m_Configs[cfg_num].m_Weapon[0].m_nKind;
            SetLabelsAndPrice(type, kind);
        }
        else if (el->m_strName == IF_BASE_PILON2) {
            type = MRT_WEAPON;
            kind = m_Configs[cfg_num].m_Weapon[1].m_nKind;
            SetLabelsAndPrice(type, kind);
        }
        else if (el->m_strName == IF_BASE_PILON3) {
            type = MRT_WEAPON;
            kind = m_Configs[cfg_num].m_Weapon[2].m_nKind;
            SetLabelsAndPrice(type, kind);
        }
        else if (el->m_strName == IF_BASE_PILON4) {
            type = MRT_WEAPON;
            kind = m_Configs[cfg_num].m_Weapon[3].m_nKind;
            SetLabelsAndPrice(type, kind);
        }
        else if (el->m_strName == IF_BASE_PILON5) {
            type = MRT_WEAPON;
            kind = m_Configs[cfg_num].m_Weapon[4].m_nKind;
            SetLabelsAndPrice(type, kind);
        }
        else if (el->m_strName == IF_BASE_PILON_HEAD) {
            type = MRT_HEAD;
            kind = m_Configs[cfg_num].m_Head.m_nKind;
            SetLabelsAndPrice(type, kind);
        }
        else if (el->m_strName == IF_BASE_PILON_HULL) {
            type = MRT_ARMOR;
            kind = m_Configs[cfg_num].m_Hull.m_Unit.m_nKind;
            SetLabelsAndPrice(type, kind);
        }
        else if (el->m_strName == IF_BASE_PILON_CHASSIS) {
            type = MRT_CHASSIS;
            kind = m_Configs[cfg_num].m_Chassis.m_nKind;
            SetLabelsAndPrice(type, kind);
        }
    }
}

void CConstructorPanel::UnFocusElement(CIFaceElement *element) {
    if (m_FocusedElement == element) {
        g_IFaceList->DeleteItemPrice();
        m_FocusedElement = NULL;
        m_FocusedLabel = std::wstring(L"");
        m_FocusedDescription = std::wstring(L"");
    }
}

void CConstructorPanel::ActivateAndSelect() {
    m_Active = 1;

    g_IFaceList->CreateSummPrice();
    g_MatrixMap->Pause(true);
}
// void CConstructorPanel::FocusElement(void* object)
//{
//    if((void*)m_FocusedElement != object){
//        g_IFaceList->DeleteItemPrice();
//        m_FocusedElement = (CIFaceElement*)object;
//        g_IFaceList->CreateItemPrice();
//    }
//}

void CConstructorPanel::ResetGroupNClose() {
    m_Active = 0;
    if (FLAG(g_IFaceList->m_IfListFlags, POPUP_MENU_ACTIVE) && g_PopupMenu) {
        g_PopupMenu->ResetMenu(true);
    }
    g_MatrixMap->Pause(false);
}

bool CConstructorPanel::IsEnoughResourcesForThisPieceOfShit(int pilon, ERobotUnitType type, ERobotUnitKind kind) {
    // if(g_MatrixMap->Rnd(0, 1))
    //    return true;

    int total_res[4];
    int minus_res[4];
    int plus_res[4];
    int item_res[4];

    CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
    ps->m_Constructor->GetConstructionPrice(total_res);
    ZeroMemory(minus_res, sizeof(int) * 4);
    ZeroMemory(plus_res, sizeof(int) * 4);
    ZeroMemory(item_res, sizeof(int) * 4);

    if (type == MRT_HEAD) {
        if (m_Configs[m_CurrentConfig].m_Head.m_nKind) {
            memcpy(minus_res, &g_Config.m_Price[HEAD1_TITAN + (int(m_Configs[m_CurrentConfig].m_Head.m_nKind) - 1) * 4],
                   sizeof(int) * 4);
        }
        memcpy(plus_res, &g_Config.m_Price[HEAD1_TITAN + (int(kind) - 1) * 4], sizeof(int) * 4);
        memcpy(item_res, &g_Config.m_Price[HEAD1_TITAN + (int(kind) - 1) * 4], sizeof(int) * 4);
    }
    else if (type == MRT_ARMOR) {
        int set_common = 0;
        int set_extra = 0;
        int try_common = g_MatrixMap->m_RobotWeaponMatrix[kind - 1].common;
        int try_extra = g_MatrixMap->m_RobotWeaponMatrix[kind - 1].extra;

        //        m_Armor.m_MaxCommonWeaponCnt = g_MatrixMap->m_RobotWeaponMatrix[kind-1].common;
        //        m_Armor.m_MaxExtraWeaponCnt = g_MatrixMap->m_RobotWeaponMatrix[kind-1].extra;

        if (m_Configs[m_CurrentConfig].m_Hull.m_Unit.m_nKind) {
            set_common = g_MatrixMap->m_RobotWeaponMatrix[m_Configs[m_CurrentConfig].m_Hull.m_Unit.m_nKind - 1].common;
            set_extra = g_MatrixMap->m_RobotWeaponMatrix[m_Configs[m_CurrentConfig].m_Hull.m_Unit.m_nKind - 1].extra;
            memcpy(minus_res,
                   &g_Config.m_Price[ARMOR1_TITAN + (int(m_Configs[m_CurrentConfig].m_Hull.m_Unit.m_nKind) - 1) * 4],
                   sizeof(int) * 4);

            for (int i = 0; i < MAX_WEAPON_CNT; i++) {
                if (m_Configs[m_CurrentConfig].m_Weapon[i].m_nKind) {
                    for (int j = 0; j < 4; j++) {
                        minus_res[j] +=
                                g_Config.m_Price[WEAPON1_TITAN +
                                                 ((m_Configs[m_CurrentConfig].m_Weapon[i].m_nKind - 1) * 4) + j];
                    }
                }
            }
        }
        memcpy(plus_res, &g_Config.m_Price[ARMOR1_TITAN + (int(kind) - 1) * 4], sizeof(int) * 4);
        memcpy(item_res, &g_Config.m_Price[ARMOR1_TITAN + (int(kind) - 1) * 4], sizeof(int) * 4);

        for (int i = 0; i < MAX_WEAPON_CNT && try_common; i++) {
            if (m_Configs[m_CurrentConfig].m_Weapon[i].m_nKind &&
                m_Configs[m_CurrentConfig].m_Weapon[i].m_nKind != RUK_WEAPON_BOMB &&
                m_Configs[m_CurrentConfig].m_Weapon[i].m_nKind != RUK_WEAPON_MORTAR) {
                for (int j = 0; j < 4; j++) {
                    plus_res[j] += g_Config.m_Price[WEAPON1_TITAN +
                                                    ((m_Configs[m_CurrentConfig].m_Weapon[i].m_nKind - 1) * 4) + j];
                }
                try_common--;
            }
        }

        for (int i = 0; i < MAX_WEAPON_CNT && try_extra; i++) {
            if (m_Configs[m_CurrentConfig].m_Weapon[i].m_nKind == RUK_WEAPON_BOMB ||
                m_Configs[m_CurrentConfig].m_Weapon[i].m_nKind == RUK_WEAPON_MORTAR) {
                for (int j = 0; j < 4; j++) {
                    plus_res[j] += g_Config.m_Price[WEAPON1_TITAN +
                                                    ((m_Configs[m_CurrentConfig].m_Weapon[i].m_nKind - 1) * 4) + j];
                }
                try_extra--;
            }
        }
    }
    else if (type == MRT_CHASSIS) {
        if (m_Configs[m_CurrentConfig].m_Chassis.m_nKind) {
            memcpy(minus_res,
                   &g_Config.m_Price[CHASSIS1_TITAN + (m_Configs[m_CurrentConfig].m_Chassis.m_nKind - 1) * 4],
                   sizeof(int) * 4);
        }
        memcpy(plus_res, &g_Config.m_Price[CHASSIS1_TITAN + (int(kind) - 1) * 4], sizeof(int) * 4);
        memcpy(item_res, &g_Config.m_Price[CHASSIS1_TITAN + (int(kind) - 1) * 4], sizeof(int) * 4);
    }
    else if (type == MRT_WEAPON) {
        if (m_Configs[m_CurrentConfig].m_Weapon[pilon].m_nKind) {
            memcpy(minus_res,
                   &g_Config.m_Price[WEAPON1_TITAN + ((m_Configs[m_CurrentConfig].m_Weapon[pilon].m_nKind - 1) * 4)],
                   sizeof(int) * 4);
        }
        memcpy(plus_res, &g_Config.m_Price[WEAPON1_TITAN + (int(kind) - 1) * 4], sizeof(int) * 4);
        memcpy(item_res, &g_Config.m_Price[WEAPON1_TITAN + (int(kind) - 1) * 4], sizeof(int) * 4);
    }

    for (int i = 0; i < 4; i++) {
        total_res[i] -= minus_res[i];
        total_res[i] += plus_res[i];
    }

    // for players only

    // if(ps->IsEnoughResources(total_res)){
    //    return true;
    //}

    for (int i = 0; i < MAX_RESOURCES; ++i) {
        if (ps->GetResourcesAmount(ERes(i)) < total_res[i] && item_res[i] != 0)
            return false;
    }
    return true;

    // if(type == MRT_CHASSIS){
    //}

    // return false;
}

void CConstructorPanel::MakeItemReplacements(ERobotUnitType type, ERobotUnitKind kind) {
    if (type == MRT_EMPTY || kind == RUK_UNKNOWN)
        return;

    utils::replace(m_FocusedLabel, L"<br>", L"\r\n");

    std::wstring color(L"<Color=247,195,0>");
    if (type == MRT_WEAPON) {
        int wspeed = g_Config.m_WeaponCooldown[WeapKind2Index(kind)];
        int damage = g_Config.m_RobotDamages[WeapKind2Index(kind)].damage;

        damage = Float2Int(float(damage) * 1000.0f / (float)wspeed);

        int adamage = 0;
        if (kind == RUK_WEAPON_FLAMETHROWER) {
            adamage = g_Config.m_RobotDamages[Weap2Index(WEAPON_ABLAZE)].damage;
            adamage = Float2Int(float(adamage) * 1000.0f / ((float)OBJECT_ROBOT_ABLAZE_PERIOD));
        }
        else if (kind == RUK_WEAPON_ELECTRIC) {
            adamage = g_Config.m_RobotDamages[Weap2Index(WEAPON_SHORTED)].damage;
            adamage = Float2Int(float(adamage) * 1000.0f / ((float)OBJECT_SHORTED_PERIOD));
        }
        else if (kind == RUK_WEAPON_BOMB) {
            damage = g_Config.m_RobotDamages[Weap2Index(WEAPON_BIGBOOM)].damage;
        }
        // else if(kind == RUK_WEAPON_MORTAR){
        //    damage = g_Config.m_RobotDamages[Weap2Index(WEAPON_BOMB)].damage / wspeed;
        //}

        std::wstring colored_damage = color + utils::format(L"%d", damage / 10) + L"</color>";
        std::wstring colored_adamage = color + utils::format(L"%d", adamage / 10) + L"</color>";

        utils::replace(m_FocusedLabel, L"<Damage>", colored_damage);
        utils::replace(m_FocusedLabel, L"<AddDamage>", colored_adamage);
    }
    else if (type == MRT_HEAD) {
        CBlockPar *bp = g_MatrixData->BlockGet(PAR_SOURCE_CHARS)->BlockGet(L"Heads");

        const wchar *hn = L"";
        if (kind == RUK_HEAD_BLOCKER) {
            hn = L"S";
        }
        else if (kind == RUK_HEAD_DYNAMO) {
            hn = L"D";
        }
        else if (kind == RUK_HEAD_LOCKATOR) {
            hn = L"L";
        }
        else if (kind == RUK_HEAD_FIREWALL) {
            hn = L"F";
        }

        bp = bp->BlockGetNE(hn);
        if (bp) {
            int ccc = bp->ParCount();
            std::wstring repl;
            for (int i = 0; i < ccc; ++i) {
                double sign = 1;

                if (bp->ParGetName(i) == L"HIT_POINT_ADD") {
                    repl = L"<Size>";
                    sign = 0.1f;
                }
                else if (bp->ParGetName(i) == L"COOL_DOWN") {
                    repl = L"<WeaponSpeed>";
                    sign = -1;
                }
                else if (bp->ParGetName(i) == L"OVERHEAT") {
                    repl = L"<WeaponHeat>";
                    sign = -1;
                }
                else if (bp->ParGetName(i) == L"CHASSIS_SPEED") {
                    repl = L"<ChassisSpeed>";
                }
                else if (bp->ParGetName(i) == L"FIRE_DISTANCE") {
                    repl = L"<WeaponRadius>";
                }
                else if (bp->ParGetName(i) == L"RADAR_DISTANCE") {
                    repl = L"<RadarRadius>";
                }
                else if (bp->ParGetName(i) == L"LIGHT_PROTECT") {
                    repl = L"<Max>";
                }
                else if (bp->ParGetName(i) == L"AIM_PROTECT") {
                    repl = L"<LessHit>";
                }
                else if (bp->ParGetName(i) == L"BOMB_PROTECT") {
                    repl = L"<BombProtect>";
                }

                int val = Double2Int(sign * bp->ParGet(i).GetDouble());

                utils::replace(m_FocusedLabel, repl, color + utils::format(L"%d", val) + L"</color>");
            }
        }
    }
    else if (type == MRT_ARMOR) {
        int structure = (int)g_Config.m_ItemChars[ARMOR1_STRUCTURE + (kind - 1) * 2];
        int pilons = g_MatrixMap->m_RobotWeaponMatrix[kind - 1].common;
        int ext_pilons = g_MatrixMap->m_RobotWeaponMatrix[kind - 1].extra;

        std::wstring colored_size = color + utils::format(L"%d", structure / 10) + L"</color>";
        std::wstring colored_pilons = color + utils::format(L"%d", pilons) + L"</color>";
        std::wstring colored_epilons = color + utils::format(L"%d", ext_pilons) + L"</color>";

        utils::replace(m_FocusedLabel, L"<Size>", colored_size);
        utils::replace(m_FocusedLabel, L"<Pilons>", colored_pilons);
        utils::replace(m_FocusedLabel, L"<AddPilons>", colored_epilons);
    }
    else if (type == MRT_CHASSIS) {
        int structure = (int)g_Config.m_ItemChars[CHASSIS1_STRUCTURE + (kind - 1) * 6];
        std::wstring colored_size = color + utils::format(L"%d", structure / 10) + L"</color>";
        utils::replace(m_FocusedLabel, L"<Size>", colored_size);
    }
}

void CConstructorPanel::SetLabelsAndPrice(ERobotUnitType type, ERobotUnitKind kind) {
    g_IFaceList->DeleteItemPrice();
    if (kind == RUK_UNKNOWN) {
        m_FocusedLabel = std::wstring(L"");
        m_FocusedDescription = std::wstring(L"");
        return;
    }

    if (type == MRT_WEAPON) {
        if (kind != RUK_UNKNOWN) {
            g_IFaceList->CreateItemPrice(&g_Config.m_Price[WEAPON1_TITAN + (kind - 1) * 4]);
            m_FocusedLabel = g_Config.m_Labels[W1_CHAR + (kind - 1)];
            m_FocusedDescription = g_Config.m_Descriptions[W1_DESCR + (kind - 1)];
        }
    }
    else if (type == MRT_HEAD) {
        if (kind) {
            g_IFaceList->CreateItemPrice(&g_Config.m_Price[HEAD1_TITAN + (kind - 1) * 4]);
            m_FocusedLabel = g_Config.m_Labels[HE1_CHAR + (kind - 1)];
            m_FocusedDescription = g_Config.m_Descriptions[HE1_DESCR + (kind - 1)];
        }
    }
    else if (type == MRT_ARMOR) {
        if (kind) {
            g_IFaceList->CreateItemPrice(&g_Config.m_Price[ARMOR1_TITAN + (kind - 1) * 4]);
            m_FocusedLabel = g_Config.m_Labels[HU1_CHAR + (kind - 1)];
            m_FocusedDescription = g_Config.m_Descriptions[HU1_DESCR + (kind - 1)];
        }
    }
    else if (type == MRT_CHASSIS) {
        if (kind) {
            g_IFaceList->CreateItemPrice(&g_Config.m_Price[CHASSIS1_TITAN + (kind - 1) * 4]);
            m_FocusedLabel = g_Config.m_Labels[CH1_CHAR + (kind - 1)];
            m_FocusedDescription = g_Config.m_Descriptions[CH1_DESCR + (kind - 1)];
        }
    }
    MakeItemReplacements(type, kind);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SPrice::SetPrice(ERobotUnitType type, ERobotUnitKind kind) {
    ZeroMemory(m_Resources, sizeof(m_Resources));
    if (!kind)
        return;
    switch (type) {
        case MRT_HEAD: {
            int i = int(kind) - 1;
            m_Resources[TITAN] = g_Config.m_Price[HEAD1_TITAN + i * 4];
            m_Resources[ELECTRONICS] = g_Config.m_Price[HEAD1_ELECTRONICS + i * 4];
            m_Resources[ENERGY] = g_Config.m_Price[HEAD1_ENERGY + i * 4];
            m_Resources[PLASMA] = g_Config.m_Price[HEAD1_PLASM + i * 4];
        } break;
        case MRT_WEAPON: {
            int i = int(kind) - 1;
            m_Resources[TITAN] = g_Config.m_Price[WEAPON1_TITAN + i * 4];
            m_Resources[ELECTRONICS] = g_Config.m_Price[WEAPON1_ELECTRONICS + i * 4];
            m_Resources[ENERGY] = g_Config.m_Price[WEAPON1_ENERGY + i * 4];
            m_Resources[PLASMA] = g_Config.m_Price[WEAPON1_PLASM + i * 4];

        } break;
        case MRT_ARMOR: {
            int i = int(kind) - 1;
            m_Resources[TITAN] = g_Config.m_Price[ARMOR1_TITAN + i * 4];
            m_Resources[ELECTRONICS] = g_Config.m_Price[ARMOR1_ELECTRONICS + i * 4];
            m_Resources[ENERGY] = g_Config.m_Price[ARMOR1_ENERGY + i * 4];
            m_Resources[PLASMA] = g_Config.m_Price[ARMOR1_PLASM + i * 4];
        } break;
        case MRT_CHASSIS: {
            int i = int(kind) - 1;
            m_Resources[TITAN] = g_Config.m_Price[CHASSIS1_TITAN + i * 4];
            m_Resources[ELECTRONICS] = g_Config.m_Price[CHASSIS1_ELECTRONICS + i * 4];
            m_Resources[ENERGY] = g_Config.m_Price[CHASSIS1_ENERGY + i * 4];
            m_Resources[PLASMA] = g_Config.m_Price[CHASSIS1_PLASM + i * 4];

        } break;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SSpecialBot *SSpecialBot::m_AIRobotTypeList = NULL;
int SSpecialBot::m_AIRobotTypeCnt = 0;

CMatrixRobotAI *SSpecialBot::GetRobot(const D3DXVECTOR3 &pos, int side_id) {
    DTRACE();

    CMatrixRobotAI *robot = HNew(g_MatrixHeap) CMatrixRobotAI;
    CMatrixSideUnit *side = g_MatrixMap->GetSideById(side_id);

    if (!side || !robot) {
        return NULL;
    }

    robot->UnitClear();
    if (m_Head.m_nKind != 0) {
        robot->UnitInsert(0, MRT_HEAD, m_Head.m_nKind);
    }
    SWeaponUnit weapons[MAX_WEAPON_CNT];
    ZeroMemory(weapons, sizeof(weapons));

    if (m_Armor.m_Unit.m_nKind != 0) {
        for (int nC = 0; nC < MAX_WEAPON_CNT; nC++) {
            if (m_Weapon[nC].m_Unit.m_nKind != 0) {
                int pilon = side->m_Constructor->CheckWeaponLegality(weapons, m_Weapon[nC].m_Unit.m_nKind,
                                                                     m_Armor.m_Unit.m_nKind);
                if (pilon != -1) {
                    weapons[pilon].m_Pos = pilon + 1;
                    weapons[pilon].m_Unit.m_nType = MRT_WEAPON;
                    weapons[pilon].m_Unit.m_nKind = m_Weapon[nC].m_Unit.m_nKind;
                }
            }
        }
    }

    if (m_Armor.m_Unit.m_nKind != 0) {
        for (int nC = 0; nC < MAX_WEAPON_CNT; nC++) {
            if (weapons[nC].m_Unit.m_nKind != 0) {
                robot->WeaponInsert(0, MRT_WEAPON, weapons[nC].m_Unit.m_nKind, m_Armor.m_Unit.m_nKind,
                                    weapons[nC].m_Pos);
            }
        }

        robot->UnitInsert(0, MRT_ARMOR, m_Armor.m_Unit.m_nKind);
    }
    if (m_Chassis.m_nKind != 0) {
        robot->UnitInsert(0, MRT_CHASSIS, m_Chassis.m_nKind);
        D3DXMatrixIdentity(&robot->m_Unit[0].m_Matrix);
    }

    robot->m_ShadowType = g_Config.m_RobotShadow;
    robot->m_ShadowSize = 128;

    robot->m_Side = side_id;
    robot->RobotWeaponInit();

    robot->m_PosX = pos.x;
    robot->m_PosY = pos.y;

    robot->CalcRobotMass();

    robot->m_Forward = D3DXVECTOR3(0, 1, 0);
    robot->m_HullForward = robot->m_Forward;

    robot->SetTeam(m_Team);

    // robot sozdan
    GetConstructionName(robot);
    robot->m_CurrState = ROBOT_SUCCESSFULLY_BUILD;

    robot->RNeed(MR_Matrix | MR_Graph);
    // robot->CreateTextures();
    robot->SetBase(NULL);

    robot->ResetMustDie();

    return robot;
}

void SSpecialBot::LoadAIRobotType(CBlockPar &bp)
{
    ClearAIRobotType();

    int k, u;
    int cnt = bp.ParCount();
    m_AIRobotTypeList = (SSpecialBot *)HAllocClear(cnt * sizeof(SSpecialBot), g_MatrixHeap);

    int i;
    for (i = 0; i < cnt; i++) {
        auto str = bp.ParGet(i);
        if (str.GetCountPar(L",") < 2)
            continue;

        std::wstring str2;

        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Pripor = bp.ParGetName(i).GetInt();
        if (m_AIRobotTypeList[m_AIRobotTypeCnt].m_Pripor < 1)
            ERROR_S2(L"LoadAIRobotType Pripor no=", utils::format(L"%d", i).c_str());

        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Chassis.m_nType = MRT_CHASSIS;
        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Chassis.m_nKind = RUK_UNKNOWN;

        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_Unit.m_nType = MRT_ARMOR;
        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_Unit.m_nKind = RUK_UNKNOWN;
        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_MaxCommonWeaponCnt = 0;
        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_MaxExtraWeaponCnt = 0;

        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Head.m_nType = MRT_WEAPON;
        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Head.m_nKind = RUK_UNKNOWN;

        m_AIRobotTypeList[m_AIRobotTypeCnt].m_HaveBomb = false;
        m_AIRobotTypeList[m_AIRobotTypeCnt].m_HaveRepair = false;

        for (u = 0; u < MAX_WEAPON_CNT; u++) {
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_nType = MRT_EMPTY;
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_nKind = RUK_UNKNOWN;
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Pos = u;
        }

        for (k = 0; k < MAX_RESOURCES; k++)
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Resources[k] = 0;

        str2 = utils::trim(str.GetStrPar(0, L","));
        if (str2 == L"Pneumatic" || str2 == L"P")
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Chassis.m_nKind = RUK_CHASSIS_PNEUMATIC;
        else if (str2 == L"Whell" || str2 == L"W")
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Chassis.m_nKind = RUK_CHASSIS_WHEEL;
        else if (str2 == L"Track" || str2 == L"T")
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Chassis.m_nKind = RUK_CHASSIS_TRACK;
        else if (str2 == L"Hovercraft" || str2 == L"H")
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Chassis.m_nKind = RUK_CHASSIS_HOVERCRAFT;
        else if (str2 == L"Antigravity" || str2 == L"A")
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Chassis.m_nKind = RUK_CHASSIS_ANTIGRAVITY;
        else
            ERROR_S2(L"LoadAIRobotType Chassis no=", utils::format(L"%d", i).c_str());

        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Chassis.m_Price.SetPrice(
                m_AIRobotTypeList[m_AIRobotTypeCnt].m_Chassis.m_nType,
                m_AIRobotTypeList[m_AIRobotTypeCnt].m_Chassis.m_nKind);
        for (k = 0; k < MAX_RESOURCES; k++)
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Resources[k] +=
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_Chassis.m_Price.m_Resources[k];

        str2 = utils::trim(str.GetStrPar(1, L","));

        if (str2 == L"1")
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_Unit.m_nKind = RUK_ARMOR_6;
        else if (str2 == L"1S")
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_Unit.m_nKind = RUK_ARMOR_PASSIVE;
        else if (str2 == L"2")
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_Unit.m_nKind = RUK_ARMOR_ACTIVE;
        else if (str2 == L"2S")
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_Unit.m_nKind = RUK_ARMOR_FIREPROOF;
        else if (str2 == L"3")
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_Unit.m_nKind = RUK_ARMOR_PLASMIC;
        else if (str2 == L"4S")
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_Unit.m_nKind = RUK_ARMOR_NUCLEAR;
        else
            ERROR_S2(L"LoadAIRobotType Armor no=", utils::format(L"%d", i).c_str());

        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_MaxCommonWeaponCnt =
                g_MatrixMap->m_RobotWeaponMatrix[m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_Unit.m_nKind - 1].common;
        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_MaxExtraWeaponCnt =
                g_MatrixMap->m_RobotWeaponMatrix[m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_Unit.m_nKind - 1].extra;
        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_Unit.m_Price.SetPrice(
                m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_Unit.m_nType,
                m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_Unit.m_nKind);
        for (k = 0; k < MAX_RESOURCES; k++)
            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Resources[k] +=
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_Unit.m_Price.m_Resources[k];

        if (str.GetCountPar(L",") >= 3) {
            str2 = utils::trim(str.GetStrPar(2, L","));

            if (str2.length() > m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_MaxCommonWeaponCnt +
                                        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_MaxExtraWeaponCnt)
                ERROR_S2(L"LoadAIRobotType WeaponCnt no=", utils::format(L"%d", i).c_str());

            int cntnormal = 0;
            int cntextra = 0;

            for (u = 0; u < str2.length(); u++) {
                wchar ch = str2[u];
                if (ch == L'G') {
                    cntnormal++;
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_nKind = RUK_WEAPON_MACHINEGUN;
                }
                else if (ch == L'C') {
                    cntnormal++;
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_nKind = RUK_WEAPON_CANNON;
                }
                else if (ch == L'M') {
                    cntnormal++;
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_nKind = RUK_WEAPON_MISSILE;
                }
                else if (ch == L'F') {
                    cntnormal++;
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_nKind = RUK_WEAPON_FLAMETHROWER;
                }
                else if (ch == L'O') {
                    cntextra++;
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_nKind = RUK_WEAPON_MORTAR;
                }
                else if (ch == L'L') {
                    cntnormal++;
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_nKind = RUK_WEAPON_LASER;
                }
                else if (ch == L'B') {
                    cntextra++;
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_nKind = RUK_WEAPON_BOMB;
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_HaveBomb = true;
                }
                else if (ch == L'P') {
                    cntnormal++;
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_nKind = RUK_WEAPON_PLASMA;
                }
                else if (ch == L'E') {
                    cntnormal++;
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_nKind = RUK_WEAPON_ELECTRIC;
                }
                else if (ch == L'R') {
                    cntnormal++;
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_nKind = RUK_WEAPON_REPAIR;
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_HaveRepair = true;
                }
                else
                    ERROR_S2(L"LoadAIRobotType WeaponType no=", utils::format(L"%d", i).c_str());

                m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_nType = MRT_WEAPON;

                m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_Price.SetPrice(
                        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_nType,
                        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_nKind);
                for (k = 0; k < MAX_RESOURCES; k++)
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_Resources[k] +=
                            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Weapon[u].m_Unit.m_Price.m_Resources[k];
            }

            if (cntnormal > m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_MaxCommonWeaponCnt)
                ERROR_S2(L"LoadAIRobotType WeaponCnt no=", utils::format(L"%d", i).c_str());
            if (cntextra > m_AIRobotTypeList[m_AIRobotTypeCnt].m_Armor.m_MaxExtraWeaponCnt)
                ERROR_S2(L"LoadAIRobotType WeaponExtraCnt no=", utils::format(L"%d", i).c_str());
        }

        if (str.GetCountPar(L",") >= 4) {
            str2 = utils::trim(str.GetStrPar(3, L","));

            if (str2 == L"Strength" || str2 == L"S")
                m_AIRobotTypeList[m_AIRobotTypeCnt].m_Head.m_nKind = RUK_HEAD_BLOCKER;
            else if (str2 == L"Dynamo" || str2 == L"D")
                m_AIRobotTypeList[m_AIRobotTypeCnt].m_Head.m_nKind = RUK_HEAD_DYNAMO;
            else if (str2 == L"Locator" || str2 == L"L")
                m_AIRobotTypeList[m_AIRobotTypeCnt].m_Head.m_nKind = RUK_HEAD_LOCKATOR;
            else if (str2 == L"Firewall" || str2 == L"F")
                m_AIRobotTypeList[m_AIRobotTypeCnt].m_Head.m_nKind = RUK_HEAD_FIREWALL;
            // else if(str2==L"Rapid" || str2==L"R") m_AIRobotTypeList[m_AIRobotTypeCnt].m_Head.m_nKind=RUK_HEAD_RAPID;
            // else if(str2==L"Design" || str2==L"D")
            // m_AIRobotTypeList[m_AIRobotTypeCnt].m_Head.m_nKind=RUK_HEAD_DESIGN; else if(str2==L"Speaker" ||
            // str2==L"P") m_AIRobotTypeList[m_AIRobotTypeCnt].m_Head.m_nKind=RUK_HEAD_SPEAKER;
            else
                ERROR_S2(L"LoadAIRobotType Head no=", utils::format(L"%d", i).c_str());

            m_AIRobotTypeList[m_AIRobotTypeCnt].m_Head.m_Price.SetPrice(
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_Head.m_nType,
                    m_AIRobotTypeList[m_AIRobotTypeCnt].m_Head.m_nKind);
            for (k = 0; k < MAX_RESOURCES; k++)
                m_AIRobotTypeList[m_AIRobotTypeCnt].m_Resources[k] +=
                        m_AIRobotTypeList[m_AIRobotTypeCnt].m_Head.m_Price.m_Resources[k];
        }

        m_AIRobotTypeList[m_AIRobotTypeCnt].CalcStrength();

        m_AIRobotTypeCnt++;
    }

    m_AIRobotTypeList =
            (SSpecialBot *)HAllocEx(m_AIRobotTypeList, m_AIRobotTypeCnt * sizeof(SSpecialBot), g_MatrixHeap);

    // Сортируем по силе
    for (i = 0; i < m_AIRobotTypeCnt - 1; i++) {
        for (u = i + 1; u < m_AIRobotTypeCnt; u++) {
            if (m_AIRobotTypeList[u].m_Strength > m_AIRobotTypeList[i].m_Strength) {
                SSpecialBot temp = m_AIRobotTypeList[u];
                m_AIRobotTypeList[u] = m_AIRobotTypeList[i];
                m_AIRobotTypeList[i] = temp;
            }
        }
    }
}

void SSpecialBot::ClearAIRobotType() {
    if (m_AIRobotTypeList) {
        HFree(m_AIRobotTypeList, g_MatrixHeap);
        m_AIRobotTypeList = NULL;
    }
    m_AIRobotTypeCnt = 0;
}

void SSpecialBot::CalcStrength()  // Расчитываем силу робота
{
    m_Strength = 0.0f;

    for (int i = 0; i < MAX_WEAPON_CNT; i++) {
        if (m_Weapon[i].m_Unit.m_nType != MRT_WEAPON)
            continue;

        m_Strength += g_Config.m_WeaponStrengthAI[m_Weapon[i].m_Unit.m_nKind];
    }
}

float SSpecialBot::DifWeapon(SSpecialBot &other) {
    int t1 = 0, t2 = 0;
    for (int i = 0; i < MAX_WEAPON_CNT; i++) {
        if (m_Weapon[i].m_Unit.m_nType != MRT_WEAPON)
            continue;
        t1++;
    }
    for (int i = 0; i < MAX_WEAPON_CNT; i++) {
        if (other.m_Weapon[i].m_Unit.m_nType != MRT_WEAPON)
            continue;
        t2++;
    }
    if (t2 > t1)
        return other.DifWeapon(*this);

    float cnt = 0;
    float cntr = 0;
    for (int i = 0; i < MAX_WEAPON_CNT; i++) {
        if (m_Weapon[i].m_Unit.m_nType != MRT_WEAPON)
            continue;
        cnt += 1.0f;

        for (int u = 0; u < MAX_WEAPON_CNT; u++) {
            if (other.m_Weapon[u].m_Unit.m_nType != MRT_WEAPON)
                continue;

            if (m_Weapon[i].m_Unit.m_nKind == other.m_Weapon[u].m_Unit.m_nKind) {
                cntr += 1.0f;
                break;
            }
        }
    }
    return cntr / cnt;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void GetConstructionName(CMatrixRobotAI *robot) {
    // CMatrixRobotAI* robot = m_Robot;

    CBlockPar *bp_tmp = g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"RobotNames");

    robot->m_Name = L"";
    for (int i = 0; i < robot->m_UnitCnt; i++) {
        if (robot->m_Unit[i].m_Type == MRT_ARMOR) {
            robot->m_Name += bp_tmp->ParGet(utils::format(L"Hull%dKey", static_cast<int>(robot->m_Unit[i].u1.s1.m_Kind)));
            break;
        }
    }
    for (int i = 0; i < robot->m_UnitCnt; i++) {
        if (robot->m_Unit[i].m_Type == MRT_CHASSIS) {
            robot->m_Name += bp_tmp->ParGet(utils::format(L"Chas%dKey", static_cast<int>(robot->m_Unit[i].u1.s1.m_Kind)));
            break;
        }
    }
    for (int i = 0; i < robot->m_UnitCnt; i++) {
        if (robot->m_Unit[i].m_Type == MRT_HEAD) {
            robot->m_Name += bp_tmp->ParGet(utils::format(L"Head%dKey", static_cast<int>(robot->m_Unit[i].u1.s1.m_Kind)));
            break;
        }
    }

    int dmg = GetConstructionDamage(robot);
    if (dmg) {
        robot->m_Name += utils::format(L"-%d", dmg);
    }
}

int GetConstructionDamage(CMatrixRobotAI *robot) {
    int damage = 0;

    float cooldown = 0;
    for (int i = 0; i < robot->m_UnitCnt; i++) {
        if (robot->m_Unit[i].m_Type == MRT_HEAD) {
            CBlockPar *bp = g_MatrixData->BlockGet(PAR_SOURCE_CHARS)->BlockGet(L"Heads");

            const wchar *hn = L"";
            if (robot->m_Unit[i].u1.s1.m_Kind == RUK_HEAD_BLOCKER) {
                hn = L"S";
            }
            else if (robot->m_Unit[i].u1.s1.m_Kind == RUK_HEAD_DYNAMO) {
                hn = L"D";
            }
            else if (robot->m_Unit[i].u1.s1.m_Kind == RUK_HEAD_LOCKATOR) {
                hn = L"L";
            }
            else if (robot->m_Unit[i].u1.s1.m_Kind == RUK_HEAD_FIREWALL) {
                hn = L"F";
            }

            bp = bp->BlockGetNE(hn);
            if (bp) {
                cooldown = float(bp->ParGetNE(L"COOL_DOWN").GetDouble() / 100.0f);
                if (cooldown < -1.0f)
                    cooldown = -1.0f;
            }
            break;
        }
    }

    for (int i = 0; i < robot->m_UnitCnt; i++) {
        if (robot->m_Unit[i].m_Type == MRT_WEAPON && robot->m_Unit[i].u1.s1.m_Kind != RUK_WEAPON_BOMB) {
            // m_RobotDamages
            int wspeed = g_Config.m_WeaponCooldown[WeapKind2Index(robot->m_Unit[i].u1.s1.m_Kind)];
            wspeed = Float2Int(float(wspeed) + float(wspeed) * cooldown);
            int wsingle_damage = g_Config.m_RobotDamages[WeapKind2Index(robot->m_Unit[i].u1.s1.m_Kind)].damage;
            wsingle_damage = Float2Int(float(wsingle_damage) * 1000.0f / ((float)wspeed));
            damage += wsingle_damage;
        }
    }

    return damage / 10;
}
