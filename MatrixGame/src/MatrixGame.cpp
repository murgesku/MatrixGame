// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

// MatrixGame.cpp : Defines the entry point for the application.

#include <new>
#include <fstream>

#include "stdafx.h"

#include "MatrixGame.h"
#include "MatrixMap.hpp"
#include "Interface/CInterface.h"
#include "MatrixRenderPipeline.hpp"
#include "ShadowStencil.hpp"
#include "MatrixLoadProgress.hpp"
#include "MatrixSkinManager.hpp"
#include "Interface/CIFaceMenu.h"
#include "Interface/MatrixHint.hpp"
#include "Interface/CHistory.h"
#include "MatrixSampleStateManager.hpp"


////////////////////////////////////////////////////////////////////////////////
CHeap *g_MatrixHeap;
CBlockPar *g_MatrixData;
CMatrixMapLogic *g_MatrixMap;
CRenderPipeline *g_Render;
CLoadProgress *g_LoadProgress;

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int) {
    const wchar *cmd = GetCommandLineW();

    int numarg;
    wchar **args = CommandLineToArgvW(cmd, &numarg);
    wchar *map = NULL;

    if (numarg > 1) {
        map = args[1];
    }

    try {
        CGame game;

        uint32_t seed = (unsigned)time(NULL);
        game.Init(hInstance, NULL, map, seed);

        CFormMatrixGame *formgame = HNew(NULL) CFormMatrixGame();
        FormChange(formgame);

        timeBeginPeriod(1);

        if (map)
        {
            {
                std::ofstream out("calcvis.log", std::ios::app);
                std::string name = utils::from_wstring(g_MatrixMap->MapName());
                out << name << " ...";
            }

            g_MatrixMap->CalcVis();

            {
                std::ofstream out("calcvis.log", std::ios::app);
                out << "done\n";
            }
        }
        else
        {
            L3GRun();
        }

        timeEndPeriod(1);

        game.Deinit();

        FormChange(NULL);
        HDelete(CFormMatrixGame, formgame, NULL);

        g_Cache->Clear();
        L3GDeinit();
        CacheDeinit();

        CMain::BaseDeInit();
    }
    catch (const CException& ex)
    {
        ClipCursor(NULL);
#ifdef ENABLE_HISTORY
        CDebugTracer::SaveHistory();
#endif
        if (g_Cache)
        {
            g_Cache->Clear();
        }
        L3GDeinit();

        MessageBox(NULL, utils::from_wstring(ex.Info().c_str()).c_str(), "Exception:", MB_OK);
    }
    catch (const std::exception& e)
    {
        MessageBox(NULL, e.what(), "Exception:", MB_OK);
    }
    catch (...) {
#ifdef ENABLE_HISTORY
        CDebugTracer::SaveHistory();
#endif
        MessageBox(NULL, "Unknown bug :(", "Exception:", MB_OK);
    }

    ClipCursor(NULL);

    return 1;
}

static void static_init(void) {
    // Base
    CMain::BaseInit();

    // 3G
#ifdef _DEBUG
    D3DResource::StaticInit();
#endif
#if (defined _DEBUG) && !(defined _RELDEBUG)
    CHelper::StaticInit();
#endif
    CCacheData::StaticInit();
    CVectorObject::StaticInit();
    CVOShadowProj::StaticInit();
    CVOShadowStencil::StaticInit();
    CBillboard::StaticInit();
    CBillboardLine::StaticInit();
    CForm::StaticInit();

    // Game
    CMatrixProgressBar::StaticInit();
    CMatrixMapStatic::StaticInit();
    CMatrixMapObject::StaticInit();
    CMatrixFlyer::StaticInit();
    CMatrixMapGroup::StaticInit();
    CInterface::StaticInit();
    CMultiSelection::StaticInit();
    CTerSurface::StaticInit();
    CBottomTextureUnion::StaticInit();
    CSkinManager::StaticInit();
    CMatrixHint::StaticInit();
    CInstDraw::StaticInit();
    SInshorewave::StaticInit();

    g_Flags = 0;  // GFLAG_FORMACCESS;
}

void CGame::Init(HINSTANCE inst, HWND wnd, wchar *map, uint32_t seed, SRobotsSettings *set, wchar *lang,
                 wchar *txt_start, wchar *txt_win, wchar *txt_loss, wchar *planet) {
    srand(seed);
    static_init();

    DTRACE();

    g_MatrixHeap = HNew(NULL) CHeap;

    CFile::AddPackFile(L"DATA\\robots.pkg", NULL);
    CFile::OpenPackFiles();

    CLoadProgress lp;
    g_LoadProgress = &lp;
    DCP();

    CStorage stor_cfg(g_MatrixHeap);
    bool stor_cfg_present = false;
    std::wstring stor_cfg_name;
    std::wstring conf_file{FILE_CONFIGURATION_LOCATION};
    if (lang != NULL) {
        conf_file += lang;
        conf_file += L"\\";
    }
    conf_file += FILE_CONFIGURATION;
    if (CFile::FileExist(stor_cfg_name, conf_file.c_str())) {
        stor_cfg.Load(conf_file.c_str());
        stor_cfg_present = true;
    }

    g_MatrixData = HNew(g_MatrixHeap) CBlockPar(1, g_MatrixHeap);
    if (stor_cfg_present) {
        stor_cfg.RestoreBlockPar(L"da", *g_MatrixData);
        // stor_cfg.RestoreBlockPar(L"if", *g_MatrixData);
        // g_MatrixData->SaveInTextFile(L"bbb.txt");

        if (CFile::FileExist(stor_cfg_name, L"cfg\\robots\\cfg.txt")) {
            CBlockPar *bpc = g_MatrixData->BlockGet(L"Config");
            bpc->LoadFromTextFile(L"cfg\\robots\\cfg.txt");
        }
    }
    else {
        g_MatrixData->LoadFromTextFile(L"cfg\\robots\\data.txt");
    }

    {
        CBlockPar *repl = g_MatrixData->BlockGetAdd(PAR_REPLACE);

        // init menu replaces

        CBlockPar *rr = g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Replaces");
        int cnt = rr->ParCount();
        for (int i = 0; i < cnt; ++i) {
            repl->ParAdd(rr->ParGetName(i), rr->ParGet(i));
        }

        if (txt_start) {
            if (txt_start[0] >= '1' && txt_start[0] <= '6') {
                repl->ParSetAdd(PAR_REPLACE_BEGIN_ICON_RACE, std::wstring{txt_start, 1});
                repl->ParSetAdd(PAR_REPLACE_DIFFICULTY, std::wstring{txt_start + 1, 2});
                repl->ParSetAdd(PAR_REPLACE_BEGIN_TEXT, txt_start + 3);
            }
            else {
                repl->ParSetAdd(PAR_REPLACE_BEGIN_ICON_RACE, L"1");
                repl->ParSetAdd(PAR_REPLACE_BEGIN_TEXT, txt_start);
            }
        }
        else {
            repl->ParSetAdd(PAR_REPLACE_BEGIN_ICON_RACE, L"1");
            repl->ParSetAdd(PAR_REPLACE_BEGIN_TEXT, L"Go! Go! Go!");
        }

        if (txt_win) {
            repl->ParSetAdd(PAR_REPLACE_END_TEXT_WIN, txt_win);
        }
        else {
            repl->ParSetAdd(PAR_REPLACE_END_TEXT_WIN, L"Good job man :)");
        }
        if (txt_loss) {
            repl->ParSetAdd(PAR_REPLACE_END_TEXT_LOOSE, txt_loss);
        }
        else {
            repl->ParSetAdd(PAR_REPLACE_END_TEXT_LOOSE, L"Sux :(");
        }
        if (planet) {
            repl->ParSetAdd(PAR_REPLACE_END_TEXT_PLANET, planet);
        }
        else {
            repl->ParSetAdd(PAR_REPLACE_END_TEXT_PLANET, L"Luna");
        }
    }

    DCP();

    CacheInit();
    DCP();

#ifdef _DEBUG
    g_MatrixData->BlockGet(L"Config")->SaveInTextFile(L"g_ConfigDump.txt");
#endif

    if (set)
        L3GInitAsDLL(inst, *g_MatrixData->BlockGet(L"Config"), L"MatrixGame", L"Matrix Game", wnd, set->FDirect3D,
                     set->FD3DDevice);
    else
        L3GInitAsEXE(inst, *g_MatrixData->BlockGet(L"Config"), L"MatrixGame", L"Matrix Game");

    //=========================================================================
    // this shit is pretending to be a settings provided by a main game
    // when this engine is used as dll. dirty solution, but seems to be
    // necessary to make it working in a standalone (exe) mode.
    SRobotsSettings settings;
    settings.m_ShowStencilShadows = 1;
    settings.m_ShowProjShadows = 1;
    settings.m_IzvratMS = 0;
    settings.m_LandTexturesGloss = 1;
    settings.m_ObjTexturesGloss = 1;
    settings.m_SoftwareCursor = 0;
    settings.m_SkyBox = 2;
    settings.m_RobotShadow = 1;
    settings.m_BPP = 32;
    settings.m_ResolutionX = g_ScreenX;
    settings.m_ResolutionY = g_ScreenY;
    settings.m_RefreshRate = 60;
    settings.m_Brightness = 0.5;
    settings.m_Contrast = 0.5;
    settings.m_FSAASamples = 0;
    settings.m_AFDegree = 0;
    settings.m_MaxDistance = 1;
    settings.m_VSync = 1;
    //=========================================================================

    if (set) {
        g_ScreenX = set->m_ResolutionX;
        g_ScreenY = set->m_ResolutionY;
    }

    g_Render = HNew(g_MatrixHeap) CRenderPipeline;  // prepare pipelines

    ShowWindow(g_Wnd, SW_SHOWNORMAL);
    UpdateWindow(g_Wnd);

    DCP();

    g_Config.SetDefaults();
    g_Config.ReadParams();
    if (set)
    {
        g_Config.ApplySettings(set);
        g_Sampler.ApplySettings(set);
        SetMaxCameraDistance(set->m_MaxDistance);
    }
    else
    {
        g_Config.ApplySettings(&settings);
        g_Sampler.ApplySettings(&settings);
        SetMaxCameraDistance(settings.m_MaxDistance);
    }

    DCP();

    g_MatrixMap = HNew(g_MatrixHeap) CMatrixMapLogic;

    g_MatrixMap->LoadSide(*g_MatrixData->BlockGet(L"Side"));
    // g_MatrixMap->LoadTactics(*g_MatrixData->BlockGet(L"Tactics"));
    g_IFaceList = HNew(g_MatrixHeap) CIFaceList;

    // load new map
    DCP();
    g_MatrixMap->RobotPreload();

    CStorage stor(g_CacheHeap);
    DCP();

    std::wstring mapname;

    if (map) {
        if (wcschr(map, '\\') == NULL) {
            mapname = L"Matrix\\Map\\";
            mapname += map;
        }
        else {
            mapname = map;
        }
    }
    else {
        mapname = g_MatrixData->BlockGet(L"Config")->Par(L"Map");
    }

    stor.Load(mapname.c_str());
    DCP();

    if (0 > g_MatrixMap->PrepareMap(stor, mapname)) {
        ERROR_S(L"Unable to load map. Error happens.");
    }
    DCP();

    std::wstring mn(g_MatrixMap->MapName());
    utils::to_lower(mn);
    if (mn.find(L"demo") != std::wstring::npos)
    {
        SETFLAG(g_MatrixMap->m_Flags, MMFLAG_AUTOMATIC_MODE | MMFLAG_FLYCAM | MMFLAG_FULLAUTO);
    }
    DCP();

    g_MatrixMap->CalcCannonPlace();
    SSpecialBot::LoadAIRobotType(*g_MatrixData->BlockGet(L"AIRobotType"));

    g_LoadProgress->SetCurLP(LP_PREPARININTERFACE);
    g_LoadProgress->InitCurLP(701);

    DCP();
    CBlockPar bpi(1, g_CacheHeap);
    if (stor_cfg_present)
    {
        stor_cfg.RestoreBlockPar(L"if", bpi);
    }
    else
    {
        bpi.LoadFromTextFile(IF_PATH);

        // CStorage stor(g_CacheHeap);
        // stor.StoreBlockPar(L"if", bpi);
        // stor.StoreBlockPar(L"da", *g_MatrixData);
        // stor.Save(FILE_CONFIGURATION, true);
    }

    g_ConfigHistory = HNew(g_MatrixHeap) CHistory;
    CInterface *pInterface = NULL;

    g_LoadProgress->SetCurLPPos(10);

    CMatrixHint::PreloadBitmaps();

    bool iface_save = false;

    g_PopupHead = (SMenuItemText *)HAlloc(sizeof(SMenuItemText) * MENU_HEAD_ITEMS, g_MatrixHeap);
    for (int i = 0; i < MENU_HEAD_ITEMS; i++) {
        new(&g_PopupHead[i]) SMenuItemText(g_MatrixHeap);
    }

    g_PopupHull = (SMenuItemText *)HAlloc(sizeof(SMenuItemText) * MENU_HULL_ITEMS, g_MatrixHeap);
    for (int i = 0; i < MENU_HULL_ITEMS; i++) {
        new(&g_PopupHull[i]) SMenuItemText(g_MatrixHeap);
    }

    g_PopupWeaponNormal = (SMenuItemText *)HAlloc(sizeof(SMenuItemText) * MENU_WEAPONNORM_ITEMS, g_MatrixHeap);
    for (int i = 0; i < MENU_WEAPONNORM_ITEMS; i++) {
        new(&g_PopupWeaponNormal[i]) SMenuItemText(g_MatrixHeap);
    }

    g_PopupWeaponExtern = (SMenuItemText *)HAlloc(sizeof(SMenuItemText) * MENU_WEAPONEXTERN_ITEMS, g_MatrixHeap);
    for (int i = 0; i < MENU_WEAPONEXTERN_ITEMS; i++) {
        new(&g_PopupWeaponExtern[i]) SMenuItemText(g_MatrixHeap);
    }

    g_PopupChassis = (SMenuItemText *)HAlloc(sizeof(SMenuItemText) * MENU_CHASSIS_ITEMS, g_MatrixHeap);
    for (int i = 0; i < MENU_CHASSIS_ITEMS; i++) {
        new(&g_PopupChassis[i]) SMenuItemText(g_MatrixHeap);
    }

    CIFaceMenu::m_MenuGraphics = HNew(g_MatrixHeap) CInterface;

    g_PopupMenu = HNew(g_MatrixHeap) CIFaceMenu;

    pInterface = HNew(g_MatrixHeap) CInterface;
    iface_save |= pInterface->Load(bpi, IF_TOP);
    LIST_ADD(pInterface, g_IFaceList->m_First, g_IFaceList->m_Last, m_PrevInterface, m_NextInterface);

    g_LoadProgress->SetCurLPPos(100);

    pInterface = HNew(g_MatrixHeap) CInterface;
    iface_save |= pInterface->Load(bpi, IF_MINI_MAP);
    LIST_ADD(pInterface, g_IFaceList->m_First, g_IFaceList->m_Last, m_PrevInterface, m_NextInterface);

    g_LoadProgress->SetCurLPPos(200);

    pInterface = HNew(g_MatrixHeap) CInterface;
    iface_save |= pInterface->Load(bpi, IF_RADAR);
    LIST_ADD(pInterface, g_IFaceList->m_First, g_IFaceList->m_Last, m_PrevInterface, m_NextInterface);

    g_LoadProgress->SetCurLPPos(300);

    pInterface = HNew(g_MatrixHeap) CInterface;
    iface_save |= pInterface->Load(bpi, IF_BASE);
    LIST_ADD(pInterface, g_IFaceList->m_First, g_IFaceList->m_Last, m_PrevInterface, m_NextInterface);

    g_IFaceList->m_BaseX = Float2Int(pInterface->m_xPos);
    g_IFaceList->m_BaseY = Float2Int(pInterface->m_yPos);
    g_LoadProgress->SetCurLPPos(400);

    pInterface = HNew(g_MatrixHeap) CInterface;
    iface_save |= pInterface->Load(bpi, IF_MAIN);
    g_IFaceList->SetMainPos(pInterface->m_xPos, pInterface->m_yPos);
    LIST_ADD(pInterface, g_IFaceList->m_First, g_IFaceList->m_Last, m_PrevInterface, m_NextInterface);

    g_LoadProgress->SetCurLPPos(500);

    pInterface = HNew(g_MatrixHeap) CInterface;
    iface_save |= pInterface->Load(bpi, IF_HINTS);
    LIST_ADD(pInterface, g_IFaceList->m_First, g_IFaceList->m_Last, m_PrevInterface, m_NextInterface);
    g_IFaceList->m_Hints = pInterface;

    g_LoadProgress->SetCurLPPos(600);

    iface_save |= CIFaceMenu::LoadMenuGraphics(bpi);
    // LIST_ADD(CIFaceMenu::m_MenuGraphics, g_IFaceList->m_First, g_IFaceList->m_Last, m_PrevInterface,
    // m_NextInterface);

    g_IFaceList->ConstructorButtonsInit();

    g_LoadProgress->SetCurLPPos(700);

    g_PopupWeaponExtern[0].text = g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Base")->ParGet(L"none");
    g_PopupWeaponNormal[0].text = g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Base")->ParGet(L"none");
    g_PopupHead[0].text = g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Base")->ParGet(L"none");

    if (g_RangersInterface) {
        g_MatrixMap->m_Transition.BuildTexture();
        // g_D3DD->Reset(&g_D3Dpp);
        g_RangersInterface->m_Begin();
    }

    if (set)
    {
        ApplyVideoParams(set);
    }
    else
    {
        ApplyVideoParams(&settings);
    }

    /*IDirect3DSurface9 * surf;
    g_D3DD->GetRenderTarget(0,&surf);
    if (!(surf==NULL)) g_D3DD->ColorFill(surf, NULL, 0);
    surf->Release();*/

    g_MatrixMap->m_Transition.RenderToPrimaryScreen();

    CMatrixEffect::InitEffects(*g_MatrixData);
    g_MatrixMap->CreatePoolDefaultResources(true);
    g_MatrixMap->InitObjectsLights();

    g_MatrixMap->GetPlayerSide()->Select(BUILDING, g_MatrixMap->GetPlayerSide()->m_ActiveObject);
    g_MatrixMap->m_Cursor.Select(CURSOR_ARROW);

    if (!FLAG(g_MatrixMap->m_Flags, MMFLAG_FULLAUTO))
        g_MatrixMap->EnterDialogMode(TEMPLATE_DIALOG_BEGIN);

    // this code can be safely removed from release : RELEASE_OFF

    // if (iface_save) bpi.SaveInTextFile(IF_PATH, true);
}

void CGame::ApplyVideoParams(SRobotsSettings *set) {
    DTRACE();

    int bpp;
    D3DDISPLAYMODE d3ddm;

    D3DDEVICE_CREATION_PARAMETERS params;
    g_D3DD->GetCreationParameters(&params);
    // ASSERT_DX(g_D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&d3ddm));
    ASSERT_DX(g_D3D->GetAdapterDisplayMode(params.AdapterOrdinal, &d3ddm));

    if (d3ddm.Format == D3DFMT_X8R8G8B8 || d3ddm.Format == D3DFMT_A8R8G8B8) {
        d3ddm.Format = D3DFMT_A8R8G8B8;
        bpp = 32;
    }
    else {
        bpp = 16;
    }

    bool change_refresh_rate = set->m_RefreshRate != 0 && set->m_RefreshRate != d3ddm.RefreshRate;
    int refresh_rate_required = change_refresh_rate ? set->m_RefreshRate : 0;

    RECT rect;
    GetClientRect(g_Wnd, &rect);
    bool was_in_window_mode = (rect.right != d3ddm.Width || rect.bottom != d3ddm.Height);
    bool now_in_window_mode = was_in_window_mode && (bpp == set->m_BPP) && !change_refresh_rate;

    if (set->m_FSAASamples > 16)
        ERROR_S(L"Invalid multisample type");
    _D3DMULTISAMPLE_TYPE expectedMultiSampleType = (_D3DMULTISAMPLE_TYPE)set->m_FSAASamples;  //(m_FSAASamples >> 24);

    ZeroMemory(&g_D3Dpp, sizeof(g_D3Dpp));

    if (now_in_window_mode) {
        RESETFLAG(g_Flags, GFLAG_FULLSCREEN);
        g_D3Dpp.Windowed = TRUE;

        RECT r1, r2;
        GetWindowRect(g_Wnd, &r1);
        GetClientRect(g_Wnd, &r2);
        SetWindowPos(g_Wnd, NULL, 0, 0, set->m_ResolutionX + (r1.right - r1.left - r2.right),
                     set->m_ResolutionY + (r1.bottom - r1.top - r2.bottom),
                     SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
    }
    else {
        SETFLAG(g_Flags, GFLAG_FULLSCREEN);
        g_D3Dpp.Windowed = FALSE;

        if (was_in_window_mode) {
            SetWindowLong(g_Wnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
            MoveWindow(g_Wnd, 0, 0, set->m_ResolutionX, set->m_ResolutionY, false);
        }
    }

    g_D3Dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_D3Dpp.BackBufferFormat = (set->m_BPP == 16) ? D3DFMT_R5G6B5 : D3DFMT_A8R8G8B8;
    g_D3Dpp.EnableAutoDepthStencil = TRUE;
    g_D3Dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    g_D3Dpp.PresentationInterval = set->m_VSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
    g_D3Dpp.FullScreen_RefreshRateInHz = refresh_rate_required;
    g_D3Dpp.BackBufferWidth = set->m_ResolutionX;
    g_D3Dpp.BackBufferHeight = set->m_ResolutionY;
    g_D3Dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
    g_D3Dpp.hDeviceWindow = g_Wnd;

    if (SUCCEEDED(g_D3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_D3Dpp.BackBufferFormat,
                                                    g_D3Dpp.Windowed, expectedMultiSampleType, NULL))) {
        g_D3Dpp.MultiSampleType = expectedMultiSampleType;
    }

    SETFLAG(g_Flags, GFLAG_STENCILAVAILABLE);

    if (D3D_OK != g_D3DD->Reset(&g_D3Dpp)) {
        if (bpp == 16)
            g_D3Dpp.AutoDepthStencilFormat = D3DFMT_D16;
        else
            g_D3Dpp.AutoDepthStencilFormat = D3DFMT_D24X8;

        if (D3D_OK != g_D3DD->Reset(&g_D3Dpp))
            ERROR_S(L"Sorry, unable to set this params.");
        RESETFLAG(g_Flags, GFLAG_STENCILAVAILABLE);
    }

    D3DVIEWPORT9 ViewPort;
    ZeroMemory(&ViewPort, sizeof(D3DVIEWPORT9));

    ViewPort.X = 0;
    ViewPort.Y = 0;
    ViewPort.Width = set->m_ResolutionX;
    ViewPort.Height = set->m_ResolutionY;

    ViewPort.MinZ = 0.0f;
    ViewPort.MaxZ = 1.0f;

    ASSERT_DX(g_D3DD->SetViewport(&ViewPort));

    S3D_Default();

    g_D3DD->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, (g_D3Dpp.MultiSampleType != D3DMULTISAMPLE_NONE));

    D3DMATERIAL9 mtrl;
    ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
    mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
    mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
    mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
    mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
    mtrl.Specular.r = 0.5f;
    mtrl.Specular.g = 0.5f;
    mtrl.Specular.b = 0.5f;
    mtrl.Specular.a = 0.5f;
    g_D3DD->SetMaterial(&mtrl);
    g_D3DD->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);

    D3DLIGHT9 light;
    ZeroMemory(&light, sizeof(D3DLIGHT9));
    light.Type = D3DLIGHT_DIRECTIONAL;  // D3DLIGHT_POINT;//D3DLIGHT_DIRECTIONAL;
    light.Diffuse.r = GetColorR(g_MatrixMap->m_LightMainColorObj);
    light.Diffuse.g = GetColorG(g_MatrixMap->m_LightMainColorObj);
    light.Diffuse.b = GetColorB(g_MatrixMap->m_LightMainColorObj);
    light.Ambient.r = 0.0f;
    light.Ambient.g = 0.0f;
    light.Ambient.b = 0.0f;
    light.Specular.r = GetColorR(g_MatrixMap->m_LightMainColorObj);
    light.Specular.g = GetColorG(g_MatrixMap->m_LightMainColorObj);
    light.Specular.b = GetColorB(g_MatrixMap->m_LightMainColorObj);
    // light.Range       = 0;
    light.Direction = g_MatrixMap->m_LightMain;
    //	light.Direction=D3DXVECTOR3(250.0f,-50.0f,-250.0f);
    //	D3DXVec3Normalize((D3DXVECTOR3 *)(&(light.Direction)),(D3DXVECTOR3 *)(&(light.Direction)));
    ASSERT_DX(g_D3DD->SetLight(0, &light));
    ASSERT_DX(g_D3DD->LightEnable(0, TRUE));
}

void CGame::Deinit(void) {
    DTRACE();

    SSpecialBot::ClearAIRobotType();

    g_Config.Clear();

    if (g_Render) {
        HDelete(CRenderPipeline, g_Render, g_MatrixHeap);
        g_Render = NULL;
    }

    CMatrixHint::ClearAll();

    if (g_MatrixMap) {
        ASSERT(g_MatrixHeap);

        HDelete(CMatrixMapLogic, g_MatrixMap, g_MatrixHeap);
        g_MatrixMap = NULL;
    }

    if (g_MatrixData) {
        HDelete(CBlockPar, g_MatrixData, NULL);
        g_MatrixData = NULL;
    }

    CMatrixRobot::DestroyPneumaticData();

    if (g_IFaceList) {
        ASSERT(g_MatrixHeap);
        HDelete(CIFaceList, g_IFaceList, g_MatrixHeap);
        g_IFaceList = NULL;
    }

    if (g_ConfigHistory) {
        ASSERT(g_MatrixHeap);
        HDelete(CHistory, g_ConfigHistory, g_MatrixHeap);
        g_ConfigHistory = NULL;
    }

    if (CIFaceMenu::m_MenuGraphics) {
        HDelete(CInterface, CIFaceMenu::m_MenuGraphics, g_MatrixHeap);
        CIFaceMenu::m_MenuGraphics = NULL;
    }

    if (g_PopupMenu) {
        HDelete(CIFaceMenu, g_PopupMenu, g_MatrixHeap);
        g_PopupMenu = NULL;
    }

    if (g_PopupHead) {
        for (int i = 0; i < MENU_HEAD_ITEMS; i++) {
            g_PopupHead[i].text.std::wstring::~wstring();
        }

        HFree(g_PopupHead, g_MatrixHeap);
        g_PopupHead = NULL;
    }

    if (g_PopupHull) {
        for (int i = 0; i < MENU_HULL_ITEMS; i++) {
            g_PopupHull[i].text.std::wstring::~wstring();
        }

        HFree(g_PopupHull, g_MatrixHeap);
        g_PopupHull = NULL;
    }

    if (g_PopupWeaponNormal) {
        for (int i = 0; i < MENU_WEAPONNORM_ITEMS; i++) {
            g_PopupWeaponNormal[i].text.std::wstring::~wstring();
        }

        HFree(g_PopupWeaponNormal, g_MatrixHeap);
        g_PopupWeaponNormal = NULL;
    }

    if (g_PopupWeaponExtern) {
        for (int i = 0; i < MENU_WEAPONEXTERN_ITEMS; i++) {
            g_PopupWeaponExtern[i].text.std::wstring::~wstring();
        }

        HFree(g_PopupWeaponExtern, g_MatrixHeap);
        g_PopupWeaponExtern = NULL;
    }

    if (g_PopupChassis) {
        for (int i = 0; i < MENU_CHASSIS_ITEMS; i++) {
            g_PopupChassis[i].text.std::wstring::~wstring();
        }

        HFree(g_PopupChassis, g_MatrixHeap);
        g_PopupChassis = NULL;
    }

    if (g_Config.m_Labels) {
        for (int i = 0; i < LABELS_LAST; i++) {
            g_Config.m_Labels[i].std::wstring::~wstring();
        }
        HFree(g_Config.m_Labels, g_MatrixHeap);
        g_Config.m_Labels = NULL;
    }

    if (g_Config.m_Descriptions) {
        for (int i = 0; i < DESCRIPTIONS_LAST; i++) {
            g_Config.m_Descriptions[i].std::wstring::~wstring();
        }
        HFree(g_Config.m_Descriptions, g_MatrixHeap);
        g_Config.m_Descriptions = NULL;
    }

    CInstDraw::ClearAll();

    CFile::ReleasePackFiles();

    if (g_MatrixHeap) {
        HDelete(CHeap, g_MatrixHeap, NULL);
        g_MatrixHeap = NULL;
    }
}

void CGame::RunGameLoop(CFormMatrixGame *formgame) {
    try {
        g_ExitState = 0;
        FormChange(formgame);

        timeBeginPeriod(1);

        SETFLAG(g_Flags, GFLAG_APPACTIVE);
        RESETFLAG(g_Flags, GFLAG_EXITLOOP);

        L3GRun();
    }
    catch (... /*ExceptionHandler(GetExceptionInformation())*/) {
        SETFLAG(g_Flags, GFLAG_EXITLOOP);
        g_ExitState += 100;
    }
}

void CGame::SaveResult(SRobotGameState *state) {
    state->m_Time = g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_TIME);
    state->m_BuildRobot = g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_ROBOT_BUILD);
    state->m_KillRobot = g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_ROBOT_KILL);
    state->m_BuildTurret = g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_TURRET_BUILD);
    state->m_KillTurret = g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_TURRET_KILL);
    state->m_KillBuilding = g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_BUILDING_KILL);
}

void CGame::SafeFree() {
    try {
        timeEndPeriod(1);

        Deinit();
    }
    catch (...) {
    }

    try {
        FormChange(NULL);
    }
    catch (...) {
    }

    try {
        g_Cache->Clear();
    }
    catch (...) {
    }

    try {
        L3GDeinit();
    }
    catch (...) {
    }

    try {
        CacheDeinit();
    }
    catch (...) {
    }

#ifdef MEM_SPY_ENABLE
    try {
        CMain::BaseDeInit();
    }
    catch (...) {
    }
#endif
}