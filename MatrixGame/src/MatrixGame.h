// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixGameDll.hpp"

#include "CHeap.hpp"
#include "CBlockPar.hpp"

class CFormMatrixGame;
class CMatrixMapLogic;
class CIFaceList;
class CRenderPipeline;
class CLoadProgress;
class CHistory;
struct SMenuItemText;

extern Base::CHeap *g_MatrixHeap;
extern Base::CBlockPar *g_MatrixData;
extern CMatrixMapLogic *g_MatrixMap;
extern CIFaceList *g_IFaceList;
extern CRenderPipeline *g_Render;
extern CLoadProgress *g_LoadProgress;
extern SMenuItemText *g_PopupHead;
extern SMenuItemText *g_PopupWeaponNormal;
extern SMenuItemText *g_PopupWeaponExtern;
extern SMenuItemText *g_PopupHull;
extern SMenuItemText *g_PopupChassis;
extern CHistory *g_ConfigHistory;

class CGame {
public:
    CGame() = default;
    ~CGame() = default;

    void Init(HINSTANCE hInstance, HWND wnd, wchar *map = NULL, uint32_t seed = 0, SRobotsSettings *set = NULL,
              wchar *lang = NULL, wchar *txt_start = NULL, wchar *txt_win = NULL, wchar *txt_loss = NULL,
              wchar *planet = NULL);
    void Deinit();
    void SafeFree();
    void RunGameLoop(CFormMatrixGame *formGame);
    void SaveResult(SRobotGameState *state);

private:
    void ApplyVideoParams(SRobotsSettings &settings);
};
