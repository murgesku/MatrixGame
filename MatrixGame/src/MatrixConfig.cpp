// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>
#include <algorithm>

#include "StringConstants.hpp"
#include "MatrixConfig.hpp"
#include "MatrixGameDll.hpp"
#include "MatrixGame.h"

struct SKeyCodes {
    const wchar *name;
    int code;
};

static SKeyCodes keycodes[] = {{L"KEY_LMB", VK_LBUTTON},
                               {L"KEY_RMB", VK_RBUTTON},

                               {L"KEY_ESC", VK_ESCAPE},
                               {L"KEY_F1", 0x70},
                               {L"KEY_F2", 0x71},
                               {L"KEY_F3", 0x72},
                               {L"KEY_F4", 0x73},
                               {L"KEY_F5", 0x74},
                               {L"KEY_F6", 0x75},
                               {L"KEY_F7", 0x76},
                               {L"KEY_F8", 0x77},
                               {L"KEY_F9", 0x78},
                               {L"KEY_F10", 0x79},
                               {L"KEY_F11", 0x7A},
                               {L"KEY_F12", 0x7B},

                               {L"KEY_PAUSE", VK_PAUSE},

                               {L"KEY_SLOCK", VK_SCROLL},
                               {L"KEY_CLOCK", VK_CAPITAL},
                               {L"KEY_NLOCK", VK_NUMLOCK},

                               {L"KEY_TILDA", -1},             // ?
                               {L"KEY_RSLASH", -1},            // ?
                               {L"KEY_LSLASH", VK_OEM_5},      // ?
                               {L"KEY_COMMA", VK_OEM_COMMA},   // ?
                               {L"KEY_COLON", VK_OEM_PERIOD},  // ?
                               {L"KEY_SEMICOLON", -1},         // ?
                               {L"KEY_APOSTROF", -1},          // ?
                               {L"KEY_LBRACKET", VK_OEM_4},    // ?
                               {L"KEY_RBRACKET", VK_OEM_6},    // ?
                               {L"KEY_MINUS", VK_OEM_MINUS},   // ?
                               {L"KEY_EQUAL", VK_OEM_PLUS},    // ?

                               {L"KEY_PADSLASH", VK_DIVIDE},
                               {L"KEY_PADSTAR", VK_MULTIPLY},
                               {L"KEY_PADMINUS", VK_SUBTRACT},
                               {L"KEY_PADPLUS", VK_ADD},
                               {L"KEY_PADENTER", -1},  // ?
                               {L"KEY_PADDEL", VK_SEPARATOR},
                               {L"KEY_PAD0", VK_NUMPAD0},
                               {L"KEY_PAD1", VK_NUMPAD1},
                               {L"KEY_PAD2", VK_NUMPAD2},
                               {L"KEY_PAD3", VK_NUMPAD3},
                               {L"KEY_PAD4", VK_NUMPAD4},
                               {L"KEY_PAD5", VK_NUMPAD5},
                               {L"KEY_PAD6", VK_NUMPAD6},
                               {L"KEY_PAD7", VK_NUMPAD7},
                               {L"KEY_PAD8", VK_NUMPAD8},
                               {L"KEY_PAD9", VK_NUMPAD9},

                               {L"KEY_1", 0x31},
                               {L"KEY_2", 0x32},
                               {L"KEY_3", 0x33},
                               {L"KEY_4", 0x34},
                               {L"KEY_5", 0x35},
                               {L"KEY_6", 0x36},
                               {L"KEY_7", 0x37},
                               {L"KEY_8", 0x38},
                               {L"KEY_9", 0x39},
                               {L"KEY_0", 0x30},

                               {L"KEY_LEFT", VK_LEFT},
                               {L"KEY_RIGHT", VK_RIGHT},
                               {L"KEY_UP", VK_UP},
                               {L"KEY_DOWN", VK_DOWN},

                               {L"KEY_BACKSPACE", VK_BACK},
                               {L"KEY_TAB", VK_TAB},
                               {L"KEY_ENTER", VK_RETURN},
                               {L"KEY_SPACE", VK_SPACE},

                               {L"KEY_INSERT", VK_INSERT},
                               {L"KEY_DELETE", VK_DELETE},
                               {L"KEY_HOME", VK_HOME},
                               {L"KEY_END", VK_END},
                               {L"KEY_PGUP", VK_PRIOR},
                               {L"KEY_PGDN", VK_NEXT},

                               {L"KEY_SHIFT", VK_SHIFT},
                               {L"KEY_LSHIFT", VK_LSHIFT},
                               {L"KEY_RSHIFT", VK_RSHIFT},
                               {L"KEY_ALT", VK_MENU},
                               {L"KEY_LALT", VK_LMENU},
                               {L"KEY_RALT", VK_RMENU},
                               {L"KEY_CTRL", VK_CONTROL},
                               {L"KEY_LCTRL", VK_LCONTROL},
                               {L"KEY_RCTRL", VK_RCONTROL},

                               {L"KEY_LWIN", VK_LWIN},
                               {L"KEY_RWIN", VK_RWIN},
                               {L"KEY_MENU", VK_APPS},

                               {L"KEY_Q", 0x51},
                               {L"KEY_W", 0x57},
                               {L"KEY_E", 0x45},
                               {L"KEY_R", 0x52},
                               {L"KEY_T", 0x54},
                               {L"KEY_Y", 0x59},
                               {L"KEY_U", 0x55},
                               {L"KEY_I", 0x49},
                               {L"KEY_O", 0x4F},
                               {L"KEY_P", 0x50},

                               {L"KEY_A", 0x41},
                               {L"KEY_S", 0x53},
                               {L"KEY_D", 0x44},
                               {L"KEY_F", 0x46},
                               {L"KEY_G", 0x47},
                               {L"KEY_H", 0x48},
                               {L"KEY_J", 0x4A},
                               {L"KEY_K", 0x4B},
                               {L"KEY_L", 0x4C},

                               {L"KEY_Z", 0x5A},
                               {L"KEY_X", 0x58},
                               {L"KEY_C", 0x43},
                               {L"KEY_V", 0x56},
                               {L"KEY_B", 0x42},
                               {L"KEY_N", 0x4E},
                               {L"KEY_M", 0x4D},

                               {NULL, -1}};

static int KeyName2KeyCode(const std::wstring &name) {
    int i = 0;
    while (keycodes[i].name != NULL) {
        if (keycodes[i].name == name)
            return keycodes[i].code;
        ++i;
    }

    return -1;
}

void CMatrixConfig::ApplySettings(SRobotsSettings *set) {
    m_IzvratMS = set->m_IzvratMS;
    m_LandTexturesGloss = set->m_LandTexturesGloss;
    m_ObjTexturesGloss = set->m_ObjTexturesGloss;

    if (set->m_RobotShadow == 0)
        m_RobotShadow = SHADOW_OFF;
    else if (set->m_RobotShadow == 1)
        m_RobotShadow = SHADOW_STENCIL;

    m_ShowProjShadows = set->m_ShowProjShadows;
    m_ShowStencilShadows = set->m_ShowStencilShadows;
    m_SkyBox = set->m_SkyBox;
    m_SoftwareCursor = set->m_SoftwareCursor;

    m_GammaR.brightness = set->m_Brightness;
    m_GammaR.contrast = set->m_Contrast;
    m_GammaR.gamma = 1.0f;
    m_GammaG = m_GammaR;
    m_GammaB = m_GammaR;
}

void CMatrixConfig::SetDefaults(void) {
    DTRACE();

    g_MaxFPS = 1000;

    // m_TexTopDownScalefactor = 0;
    // m_TexTopMinSize = 32;
    // m_TexBotDownScalefactor = 0;
    // m_TexBotMinSize = 32;
    m_SoftwareCursor = false;

    m_ObjTexturesGloss = true;
    // m_ObjTextures16 = true;

    m_LandTexturesGloss = true;
    m_DIFlags = 0;
    m_VertexLight = true;

    m_Cursors = NULL;

    m_ShowStencilShadows = true;
    m_ShowProjShadows = true;
    m_CannonsLogic = true;

    m_IzvratMS = false;

    // m_PlayerRobotsCnt = 0;
    // m_CompRobotsCnt = 0;

    m_RobotShadow = SHADOW_STENCIL;

    m_GammaR.brightness = 0.5f;
    m_GammaR.contrast = 0.5f;
    m_GammaR.gamma = 1.0f;

    m_GammaG.brightness = 0.5f;
    m_GammaG.contrast = 0.5f;
    m_GammaG.gamma = 1.0f;

    m_GammaB.brightness = 0.5f;
    m_GammaB.contrast = 0.5f;
    m_GammaB.gamma = 1.0f;

    // camera properties

    m_CamParams[CAMERA_STRATEGY].m_CamMouseWheelStep = 0.05f;
    m_CamParams[CAMERA_STRATEGY].m_CamRotSpeedX = 0.0005f;
    m_CamParams[CAMERA_STRATEGY].m_CamRotSpeedZ = 0.001f;
    m_CamParams[CAMERA_STRATEGY].m_CamRotAngleMin = GRAD2RAD(60);
    m_CamParams[CAMERA_STRATEGY].m_CamRotAngleMax = GRAD2RAD(20);
    m_CamParams[CAMERA_STRATEGY].m_CamDistMin = 70;
    m_CamParams[CAMERA_STRATEGY].m_CamDistMax = 250;
    m_CamParams[CAMERA_STRATEGY].m_CamAngleParam = 0.4f;
    m_CamParams[CAMERA_STRATEGY].m_CamHeight = 140.0f;

    m_CamParams[CAMERA_INROBOT].m_CamMouseWheelStep = 0.05f;
    m_CamParams[CAMERA_INROBOT].m_CamRotSpeedX = 0.0005f;
    m_CamParams[CAMERA_INROBOT].m_CamRotSpeedZ = 0.001f;
    m_CamParams[CAMERA_INROBOT].m_CamRotAngleMin = GRAD2RAD(60);
    m_CamParams[CAMERA_INROBOT].m_CamRotAngleMax = GRAD2RAD(20);
    m_CamParams[CAMERA_INROBOT].m_CamDistMin = 70;
    m_CamParams[CAMERA_INROBOT].m_CamDistMax = 250;
    m_CamParams[CAMERA_INROBOT].m_CamAngleParam = 0.0f;
    m_CamParams[CAMERA_INROBOT].m_CamHeight = 40.0f;

    m_CamBaseAngleZ = 0;  // GRAD2RAD(38.0f);
    m_CamMoveSpeed = 1.05f;
    m_CamInRobotForward0 = 10.0f;
    m_CamInRobotForward1 = 30.0f;

    m_KeyActions[KA_ROTATE_LEFT] = VK_HOME;
    m_KeyActions[KA_ROTATE_RIGHT] = VK_END;
    m_KeyActions[KA_ROTATE_UP] = VK_PRIOR;
    m_KeyActions[KA_ROTATE_DOWN] = VK_NEXT;

    m_KeyActions[KA_SCROLL_UP_ALT] = VK_UP;        // 0x57;
    m_KeyActions[KA_SCROLL_DOWN_ALT] = VK_DOWN;    // 0x53;
    m_KeyActions[KA_SCROLL_LEFT_ALT] = VK_LEFT;    // 0x41;
    m_KeyActions[KA_SCROLL_RIGHT_ALT] = VK_RIGHT;  // 0x44;

    m_KeyActions[KA_SCROLL_UP] = VK_UP;
    m_KeyActions[KA_SCROLL_DOWN] = VK_DOWN;
    m_KeyActions[KA_SCROLL_LEFT] = VK_LEFT;
    m_KeyActions[KA_SCROLL_RIGHT] = VK_RIGHT;

    m_KeyActions[KA_UNIT_FORWARD] = 0x57;
    m_KeyActions[KA_UNIT_BACKWARD] = 0x53;
    m_KeyActions[KA_UNIT_LEFT] = 0x41;
    m_KeyActions[KA_UNIT_RIGHT] = 0x44;

    m_KeyActions[KA_UNIT_FORWARD_ALT] = VK_UP;
    m_KeyActions[KA_UNIT_BACKWARD_ALT] = VK_DOWN;
    m_KeyActions[KA_UNIT_LEFT_ALT] = VK_LEFT;
    m_KeyActions[KA_UNIT_RIGHT_ALT] = VK_RIGHT;

    m_KeyActions[KA_FIRE] = VK_LBUTTON;
    m_KeyActions[KA_AUTO] = VK_RBUTTON;

    m_KeyActions[KA_SHIFT] = VK_SHIFT;
    m_KeyActions[KA_CTRL] = VK_CONTROL;

    m_KeyActions[KA_ROTATE_LEFT_ALT] = VK_OEM_4;   // zak
    m_KeyActions[KA_ROTATE_RIGHT_ALT] = VK_OEM_6;  // zak

    m_KeyActions[KA_MINIMAP_ZOOMIN] = VK_OEM_PLUS;    // sub
    m_KeyActions[KA_MINIMAP_ZOOMOUT] = VK_OEM_MINUS;  // sub

    m_KeyActions[KA_CAM_SETDEFAULT] = VK_OEM_5;  // zak

    m_KeyActions[KA_AUTOORDER_CAPTURE] = 0x43;  // sub
    m_KeyActions[KA_AUTOORDER_ATTACK] = 0x55;   // sub
    m_KeyActions[KA_AUTOORDER_DEFEND] = 0x44;   // sub

    m_KeyActions[KA_ORDER_MOVE] = 0x4D;     // sub
    m_KeyActions[KA_ORDER_STOP] = 0x53;     // sub
    m_KeyActions[KA_ORDER_CAPTURE] = 0x4B;  // sub
    m_KeyActions[KA_ORDER_PATROL] = 0x50;   // sub
    m_KeyActions[KA_ORDER_EXPLODE] = 0x45;  // sub
    m_KeyActions[KA_ORDER_REPAIR] = 0x52;   // sub
    m_KeyActions[KA_ORDER_ATTACK] = 0x41;   // sub
    m_KeyActions[KA_ORDER_CANCEL] = 0x58;
    m_KeyActions[KA_ORDER_ROBOT_SWITCH1] = VK_OEM_COMMA;   // sub
    m_KeyActions[KA_ORDER_ROBOT_SWITCH2] = VK_OEM_PERIOD;  // sub

    m_KeyActions[KA_UNIT_BOOM] = 0x45;           // sub
    m_KeyActions[KA_UNIT_ENTER] = VK_RETURN;     // sub (! if not dialog mode)
    m_KeyActions[KA_UNIT_ENTER_ALT] = VK_SPACE;  // sub (! if not dialog mode)

    m_KeyActions[KA_BUILD_ROBOT] = 0x42;   // sub
    m_KeyActions[KA_BUILD_TURRET] = 0x54;  // sub
    m_KeyActions[KA_BUILD_HELP] = 0x48;    // sub

    m_KeyActions[KA_TURRET_CANNON] = 0x43;
    m_KeyActions[KA_TURRET_GUN] = 0x47;
    m_KeyActions[KA_TURRET_LASER] = 0x4C;
    m_KeyActions[KA_TURRET_ROCKET] = 0x52;

    m_CaptureTimeErase = 750;
    m_CaptureTimePaint = 500;
    m_CaptureTimeRolback = 1500;

    m_SkyBox = 1;
    m_DrawAllObjectsToMinimap = 2;
}

void CMatrixConfig::Clear(void) {
    DTRACE();
    if (m_Cursors) {
        for (int i = 0; i < m_CursorsCnt; ++i) {
            m_Cursors[i].key.std::wstring::~wstring();
            m_Cursors[i].val.std::wstring::~wstring();
        }

        HFree(m_Cursors, g_CacheHeap);
    }
}

void CMatrixConfig::ReadParams(void) {
    DTRACE();

    Clear();

    CBlockPar *cfg_par = g_MatrixData->BlockGet(L"Config");

    // loading config
    CBlockPar *bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_CURSORS);

    m_CursorsCnt = bp_tmp->ParCount();

    m_Cursors = (SStringPair *)HAlloc(sizeof(SStringPair) * m_CursorsCnt, g_CacheHeap);
    for (int i = 0; i < m_CursorsCnt; ++i) {
        new(&m_Cursors[i].key) std::wstring(bp_tmp->ParGetName(i));
        new(&m_Cursors[i].val) std::wstring(bp_tmp->ParGet(i));
    }

    // top size
    // if (g_MatrixCfg->ParCount(CFG_TOPSIZE) != 0)
    //    m_TexTopMinSize = g_MatrixCfg->ParGet(CFG_TOPSIZE).GetInt();

    // if (g_MatrixCfg->ParCount(CFG_TOPSCALE) != 0)
    //    m_TexTopDownScalefactor = g_MatrixCfg->ParGet(CFG_TOPSCALE).GetInt();

    // if (m_TexTopMinSize < 32) m_TexTopMinSize = 32;

    // bottom size
    // if (g_MatrixCfg->ParCount(CFG_BOTSIZE) != 0)
    //    m_TexBotMinSize = g_MatrixCfg->ParGet(CFG_BOTSIZE).GetInt();

    // if (g_MatrixCfg->ParCount(CFG_BOTSCALE) != 0)
    //    m_TexBotDownScalefactor = g_MatrixCfg->ParGet(CFG_BOTSCALE).GetInt();

    // if (m_TexBotMinSize < 32) m_TexBotMinSize = 32;
    // if (m_TexBotMinSize < 16) m_TexBotMinSize = 16;

    // if (g_MatrixCfg->ParCount(CFG_LANDTEXTURES16) != 0)
    //    m_LandTextures16 = g_MatrixCfg->ParGet(CFG_LANDTEXTURES16).GetInt() == 1;

    if (cfg_par->ParCount(CFG_SOFTWARECURSOR) != 0)
        m_SoftwareCursor = cfg_par->ParGet(CFG_SOFTWARECURSOR).GetInt() == 1;

    if (cfg_par->ParCount(CFG_GLOSSLAND) != 0)
        m_LandTexturesGloss = cfg_par->ParGet(CFG_GLOSSLAND).GetInt() == 1;

    if (cfg_par->ParCount(CFG_GLOSSOBJECT) != 0) {
        m_ObjTexturesGloss = cfg_par->ParGet(CFG_GLOSSOBJECT).GetInt() == 1;
    }
    // if (g_MatrixCfg->ParCount(CFG_OBJECTTEX16) != 0)
    //{
    //    m_ObjTextures16 = g_MatrixCfg->ParGet(CFG_OBJECTTEX16).GetInt() == 1;
    //}

    if (cfg_par->ParCount(CFG_IZVRATMS) != 0) {
        m_IzvratMS = cfg_par->ParGet(CFG_IZVRATMS).GetInt() == 1;
    }

    if (cfg_par->ParCount(CFG_SKYBOX) != 0) {
        m_SkyBox = (byte)(cfg_par->ParGet(CFG_SKYBOX).GetInt() & 0xFF);
    }

    if (cfg_par->ParCount(CFG_MAXFPS) != 0) {
        g_MaxFPS = cfg_par->ParGet(CFG_MAXFPS).GetInt();
    }

    if (cfg_par->ParCount(CFG_OBJECTTOMINIMAP) != 0) {
        m_DrawAllObjectsToMinimap = (byte)(cfg_par->ParGet(CFG_OBJECTTOMINIMAP).GetInt() & 0xFF);
    }

    if (cfg_par->ParCount(CFG_DEBUGINFO) != 0) {
        m_DIFlags = cfg_par->ParGet(CFG_DEBUGINFO).GetHexUnsigned();
    }

    if (cfg_par->ParCount(CFG_VERTEXLIGHT) != 0) {
        m_VertexLight = cfg_par->ParGet(CFG_VERTEXLIGHT).GetInt() == 1;
    }

    if (cfg_par->BlockCount(CFG_GAMMARAMP) != 0) {
        CBlockPar *g = cfg_par->BlockGet(CFG_GAMMARAMP);
        m_GammaR.brightness = (float)g->ParGet(L"R").GetStrPar(0, L",").GetDouble();
        m_GammaR.contrast = (float)g->ParGet(L"R").GetStrPar(1, L",").GetDouble();
        m_GammaR.gamma = (float)g->ParGet(L"R").GetStrPar(2, L",").GetDouble();

        m_GammaG.brightness = (float)g->ParGet(L"G").GetStrPar(0, L",").GetDouble();
        m_GammaG.contrast = (float)g->ParGet(L"G").GetStrPar(1, L",").GetDouble();
        m_GammaG.gamma = (float)g->ParGet(L"G").GetStrPar(2, L",").GetDouble();

        m_GammaB.brightness = (float)g->ParGet(L"B").GetStrPar(0, L",").GetDouble();
        m_GammaB.contrast = (float)g->ParGet(L"B").GetStrPar(1, L",").GetDouble();
        m_GammaB.gamma = (float)g->ParGet(L"B").GetStrPar(2, L",").GetDouble();
    }

    ApplyGammaRamp();

    if (cfg_par->BlockCount(CFG_ASSIGNKEY) != 0) {
        CBlockPar *ak = cfg_par->BlockGet(CFG_ASSIGNKEY);
        int n = ak->ParCount();
        for (int i = 0; i < n; ++i) {
            int akn = ak->ParGetName(i).GetInt();
            if (akn < 0 || akn >= KA_LAST) {
                continue;
            }
            int kk = KeyName2KeyCode(ak->ParGet(i));

            m_KeyActions[akn] = kk;
        }
    }

    if (cfg_par->ParCount(CFG_ROBOTSHADOW) != 0) {
        int sh = cfg_par->ParGet(CFG_ROBOTSHADOW).GetInt();

        if (sh == 0)
            m_RobotShadow = SHADOW_OFF;
        else if (sh == 1)
            m_RobotShadow = SHADOW_STENCIL;
    }

    // load damages

    int n;
    // cannons
    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_WEAPONS)
                     ->BlockGet(PAR_SOURCE_DAMAGES)
                     ->BlockGet(PAR_SOURCE_DAMAGES_CANNON);

    memset(&m_CannonDamages, 0, sizeof(m_CannonDamages));

    n = bp_tmp->ParCount();
    for (int i = 0; i < n; ++i) {
        const auto name = bp_tmp->ParGetName(i);
        int idx = WeapName2Index(name);
        if (idx >= 0) {
            auto par = bp_tmp->ParGet(i);
            int nn = par.GetCountPar(L",");
            m_CannonDamages[idx].damage = par.GetStrPar(0, L",").GetInt();
            if (nn > 1)
                m_CannonDamages[idx].mindamage = par.GetStrPar(1, L",").GetInt();
            if (nn > 2)
                m_CannonDamages[idx].friend_damage = par.GetStrPar(2, L",").GetInt();
            else
                m_CannonDamages[idx].friend_damage = m_CannonDamages[idx].damage;
        }
    }

    // buildings
    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_WEAPONS)
                     ->BlockGet(PAR_SOURCE_DAMAGES)
                     ->BlockGet(PAR_SOURCE_DAMAGES_BUILDING);

    memset(&m_BuildingDamages, 0, sizeof(m_BuildingDamages));

    n = bp_tmp->ParCount();
    for (int i = 0; i < n; ++i) {
        const auto name = bp_tmp->ParGetName(i);
        if (name == PAR_SOURCE_DAMAGES_HITPOINT)
        {
            auto par = bp_tmp->ParGet(i);
            int nn = par.GetCountPar(L",");
            for (int j = 0; j < nn; ++j) {
                m_BuildingHitPoints[j] = par.GetStrPar(j, L",").GetInt();
            }
        }
        else {
            int idx = WeapName2Index(name);
            if (idx >= 0) {
                auto par = bp_tmp->ParGet(i);
                int nn = par.GetCountPar(L",");
                m_BuildingDamages[idx].damage = par.GetStrPar(0, L",").GetInt();
                if (nn > 1)
                    m_BuildingDamages[idx].mindamage = par.GetStrPar(1, L",").GetInt();
                if (nn > 2)
                    m_BuildingDamages[idx].friend_damage = par.GetStrPar(2, L",").GetInt();
                else
                    m_BuildingDamages[idx].friend_damage = m_BuildingDamages[idx].damage;
            }
        }
    }

    // flyer
    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_WEAPONS)
                     ->BlockGet(PAR_SOURCE_DAMAGES)
                     ->BlockGet(PAR_SOURCE_DAMAGES_FLYER);

    memset(&m_FlyerDamages, 0, sizeof(m_FlyerDamages));

    n = bp_tmp->ParCount();
    for (int i = 0; i < n; ++i) {
        const auto name = bp_tmp->ParGetName(i);
        // if (name == PAR_SOURCE_DAMAGES_HITPOINT)
        //{
        //    const CWStr &par = bp_tmp->ParGet(i);
        //    int nn = par.GetCountPar(L",");
        //    for (int j=0;j<nn;++j)
        //    {
        //        m_FlyerHitPoints[j] = par.GetStrPar(j, L",").GetInt();
        //    }
        //} else
        {
            int idx = WeapName2Index(name);
            if (idx >= 0) {
                auto par = bp_tmp->ParGet(i);
                int nn = par.GetCountPar(L",");
                m_FlyerDamages[idx].damage = par.GetStrPar(0, L",").GetInt();
                if (nn > 1)
                    m_FlyerDamages[idx].mindamage = par.GetStrPar(1, L",").GetInt();
                if (nn > 2)
                    m_FlyerDamages[idx].friend_damage = par.GetStrPar(2, L",").GetInt();
                else
                    m_FlyerDamages[idx].friend_damage = m_FlyerDamages[idx].damage;
            }
        }
    }

    // robot
    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_WEAPONS)
                     ->BlockGet(PAR_SOURCE_DAMAGES)
                     ->BlockGet(PAR_SOURCE_DAMAGES_ROBOT);

    memset(&m_RobotDamages, 0, sizeof(m_RobotDamages));

    n = bp_tmp->ParCount();
    for (int i = 0; i < n; ++i) {
        const auto name = bp_tmp->ParGetName(i);
        int idx = WeapName2Index(name);
        if (idx >= 0) {
            auto par = bp_tmp->ParGet(i);
            int nn = par.GetCountPar(L",");
            m_RobotDamages[idx].damage = par.GetStrPar(0, L",").GetInt();
            if (nn > 1)
                m_RobotDamages[idx].mindamage = par.GetStrPar(1, L",").GetInt();
            if (nn > 2)
                m_RobotDamages[idx].friend_damage = par.GetStrPar(2, L",").GetInt();
            else
                m_RobotDamages[idx].friend_damage = m_RobotDamages[idx].damage;
        }
    }

    // object
    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_WEAPONS)
                     ->BlockGet(PAR_SOURCE_DAMAGES)
                     ->BlockGet(PAR_SOURCE_DAMAGES_OBJECT);

    memset(&m_ObjectDamages, 0, sizeof(m_ObjectDamages));

    n = bp_tmp->ParCount();
    for (int i = 0; i < n; ++i) {
        const auto name = bp_tmp->ParGetName(i);
        int idx = WeapName2Index(name);
        if (idx >= 0) {
            auto par = bp_tmp->ParGet(i);
            int nn = par.GetCountPar(L",");
            m_ObjectDamages[idx].damage = par.GetStrPar(0, L",").GetInt();
            if (nn > 1)
                m_ObjectDamages[idx].mindamage = par.GetStrPar(1, L",").GetInt();
        }
    }

    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_WEAPONS)->BlockGet(PAR_SOURCE_RADIUS);

    memset(&m_WeaponRadius, 0, sizeof(m_WeaponRadius));

    n = bp_tmp->ParCount();
    for (int i = 0; i < n; ++i) {
        const auto name = bp_tmp->ParGetName(i);
        int idx = WeapName2Index(name);
        if (idx >= 0) {
            auto par = bp_tmp->ParGet(i);
            m_WeaponRadius[idx] = (float)par.GetDouble();
        }
    }

    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_WEAPONS)->BlockGet(PAR_SOURCE_COOLDOWN);

    memset(&m_WeaponCooldown, 0, sizeof(m_WeaponCooldown));

    n = bp_tmp->ParCount();
    for (int i = 0; i < n; ++i) {
        const auto name = bp_tmp->ParGetName(i);
        int idx = WeapName2Index(name);
        if (idx >= 0) {
            auto par = bp_tmp->ParGet(i);
            m_WeaponCooldown[idx] = par.GetInt();
        }
    }

    // timings

    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_TIMINGS);
    m_MaintenanceTime = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_MAINTENANCE).GetInt();

    // resources
    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_TIMINGS)->BlockGet(PAR_SOURCE_TIMINGS_RESOURCES);
    m_Timings[RESOURCE_TITAN] = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_TITAN).GetInt();
    m_Timings[RESOURCE_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_ENERGY).GetInt();
    m_Timings[RESOURCE_PLASMA] = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_PLASMA).GetInt();
    m_Timings[RESOURCE_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_ELECTRONICS).GetInt();
    m_Timings[RESOURCE_BASE] = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_BASE).GetInt();
    // units
    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_TIMINGS)->BlockGet(PAR_SOURCE_TIMINGS_UNITS);
    m_Timings[UNIT_ROBOT] = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_ROBOT).GetInt();
    m_Timings[UNIT_FLYER] = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_FLYER).GetInt();
    m_Timings[UNIT_TURRET] = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_TURRET).GetInt();

    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_TIMINGS)->BlockGet(PAR_SOURCE_TIMINGS_CAPTURE);
    m_CaptureTimeErase = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_ERASE).GetInt();
    m_CaptureTimePaint = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_PAINT).GetInt();
    m_CaptureTimeRolback = bp_tmp->ParGet(PAR_SOURCE_TIMINGS_ROLLBACK).GetInt();

    // weapons owerheat
    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_WEAPONS)->BlockGet(PAR_SOURCE_OVERHEAT);

    // m_Overheat[WHP_VOLCANO] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WHP_VOLCANO).GetInt();
    m_Overheat[WEAPON_VOLCANO_HEAT] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WEAPON_VOLCANO_HEAT).GetInt();
    m_Overheat[WCP_VOLCANO] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WCP_VOLCANO).GetInt();
    m_Overheat[WEAPON_VOLCANO_COOL] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WEAPON_VOLCANO_COOL).GetInt();

    // m_Overheat[WHP_PLASMA] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WHP_PLASMA).GetInt();
    m_Overheat[WEAPON_PLASMA_HEAT] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WEAPON_PLASMA_HEAT).GetInt();
    m_Overheat[WCP_PLASMA] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WCP_PLASMA).GetInt();
    m_Overheat[WEAPON_PLASMA_COOL] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WEAPON_PLASMA_COOL).GetInt();

    // m_Overheat[WHP_LASER] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WHP_LASER).GetInt();
    m_Overheat[WEAPON_LASER_HEAT] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WEAPON_LASER_HEAT).GetInt();
    m_Overheat[WCP_LASER] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WCP_LASER).GetInt();
    m_Overheat[WEAPON_LASER_COOL] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WEAPON_LASER_COOL).GetInt();

    // m_Overheat[WHP_HOMING_MISSILE] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WHP_HOMING_MISSILE).GetInt();
    m_Overheat[WEAPON_HOMING_MISSILE_HEAT] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WEAPON_HOMING_MISSILE_HEAT).GetInt();
    m_Overheat[WCP_HOMING_MISSILE] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WCP_HOMING_MISSILE).GetInt();
    m_Overheat[WEAPON_HOMING_MISSILE_COOL] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WEAPON_HOMING_MISSILE_COOL).GetInt();

    // m_Overheat[WHP_FLAMETHROWER] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WHP_FLAMETHROWER).GetInt();
    m_Overheat[WEAPON_FLAMETHROWER_HEAT] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WEAPON_FLAMETHROWER_HEAT).GetInt();
    m_Overheat[WCP_FLAMETHROWER] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WCP_FLAMETHROWER).GetInt();
    m_Overheat[WEAPON_FLAMETHROWER_COOL] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WEAPON_FLAMETHROWER_COOL).GetInt();

    // m_Overheat[WHP_BOMB] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WHP_BOMB).GetInt();
    m_Overheat[WEAPON_BOMB_HEAT] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WEAPON_BOMB_HEAT).GetInt();
    m_Overheat[WCP_BOMB] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WCP_BOMB).GetInt();
    m_Overheat[WEAPON_BOMB_COOL] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WEAPON_BOMB_COOL).GetInt();

    // m_Overheat[WHP_GUN] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WHP_GUN).GetInt();
    m_Overheat[WEAPON_GUN_HEAT] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WEAPON_GUN_HEAT).GetInt();
    m_Overheat[WCP_GUN] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WCP_GUN).GetInt();
    m_Overheat[WEAPON_GUN_COOL] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WEAPON_GUN_COOL).GetInt();

    // m_Overheat[WHP_LIGHTENING] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WHP_LIGHTENING).GetInt();
    m_Overheat[WEAPON_LIGHTENING_HEAT] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WEAPON_LIGHTENING_HEAT).GetInt();
    m_Overheat[WCP_LIGHTENING] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WCP_LIGHTENING).GetInt();
    m_Overheat[WEAPON_LIGHTENING_COOL] = bp_tmp->ParGet(PAR_SOURCE_OVERHEAT_WEAPON_LIGHTENING_COOL).GetInt();

    // Price

    // Head
    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_PRICE)->BlockGet(PAR_SOURCE_PRICE_HEAD);

    m_Price[HEAD1_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_HEAD1_TITAN).GetInt();
    m_Price[HEAD1_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_HEAD1_ELECTRONICS).GetInt();
    m_Price[HEAD1_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_HEAD1_ENERGY).GetInt();
    m_Price[HEAD1_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_HEAD1_PLASM).GetInt();

    m_Price[HEAD2_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_HEAD2_TITAN).GetInt();
    m_Price[HEAD2_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_HEAD2_ELECTRONICS).GetInt();
    m_Price[HEAD2_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_HEAD2_ENERGY).GetInt();
    m_Price[HEAD2_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_HEAD2_PLASM).GetInt();

    m_Price[HEAD3_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_HEAD3_TITAN).GetInt();
    m_Price[HEAD3_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_HEAD3_ELECTRONICS).GetInt();
    m_Price[HEAD3_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_HEAD3_ENERGY).GetInt();
    m_Price[HEAD3_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_HEAD3_PLASM).GetInt();

    m_Price[HEAD4_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_HEAD4_TITAN).GetInt();
    m_Price[HEAD4_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_HEAD4_ELECTRONICS).GetInt();
    m_Price[HEAD4_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_HEAD4_ENERGY).GetInt();
    m_Price[HEAD4_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_HEAD4_PLASM).GetInt();

    // Armor
    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_PRICE)->BlockGet(PAR_SOURCE_PRICE_ARMOR);

    m_Price[ARMOR1_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR1_TITAN).GetInt();
    m_Price[ARMOR1_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR1_ELECTRONICS).GetInt();
    m_Price[ARMOR1_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR1_ENERGY).GetInt();
    m_Price[ARMOR1_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR1_PLASM).GetInt();

    m_Price[ARMOR2_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR2_TITAN).GetInt();
    m_Price[ARMOR2_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR2_ELECTRONICS).GetInt();
    m_Price[ARMOR2_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR2_ENERGY).GetInt();
    m_Price[ARMOR2_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR2_PLASM).GetInt();

    m_Price[ARMOR3_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR3_TITAN).GetInt();
    m_Price[ARMOR3_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR3_ELECTRONICS).GetInt();
    m_Price[ARMOR3_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR3_ENERGY).GetInt();
    m_Price[ARMOR3_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR3_PLASM).GetInt();

    m_Price[ARMOR4_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR4_TITAN).GetInt();
    m_Price[ARMOR4_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR4_ELECTRONICS).GetInt();
    m_Price[ARMOR4_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR4_ENERGY).GetInt();
    m_Price[ARMOR4_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR4_PLASM).GetInt();

    m_Price[ARMOR5_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR5_TITAN).GetInt();
    m_Price[ARMOR5_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR5_ELECTRONICS).GetInt();
    m_Price[ARMOR5_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR5_ENERGY).GetInt();
    m_Price[ARMOR5_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR5_PLASM).GetInt();

    m_Price[ARMOR6_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR6_TITAN).GetInt();
    m_Price[ARMOR6_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR6_ELECTRONICS).GetInt();
    m_Price[ARMOR6_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR6_ENERGY).GetInt();
    m_Price[ARMOR6_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_ARMOR6_PLASM).GetInt();

    // Weapon
    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_PRICE)->BlockGet(PAR_SOURCE_PRICE_WEAPON);

    m_Price[WEAPON1_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON1_TITAN).GetInt();
    m_Price[WEAPON1_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON1_ELECTRONICS).GetInt();
    m_Price[WEAPON1_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON1_ENERGY).GetInt();
    m_Price[WEAPON1_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON1_PLASM).GetInt();

    m_Price[WEAPON2_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON2_TITAN).GetInt();
    m_Price[WEAPON2_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON2_ELECTRONICS).GetInt();
    m_Price[WEAPON2_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON2_ENERGY).GetInt();
    m_Price[WEAPON2_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON2_PLASM).GetInt();

    m_Price[WEAPON3_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON3_TITAN).GetInt();
    m_Price[WEAPON3_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON3_ELECTRONICS).GetInt();
    m_Price[WEAPON3_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON3_ENERGY).GetInt();
    m_Price[WEAPON3_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON3_PLASM).GetInt();

    m_Price[WEAPON4_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON4_TITAN).GetInt();
    m_Price[WEAPON4_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON4_ELECTRONICS).GetInt();
    m_Price[WEAPON4_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON4_ENERGY).GetInt();
    m_Price[WEAPON4_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON4_PLASM).GetInt();

    m_Price[WEAPON5_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON5_TITAN).GetInt();
    m_Price[WEAPON5_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON5_ELECTRONICS).GetInt();
    m_Price[WEAPON5_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON5_ENERGY).GetInt();
    m_Price[WEAPON5_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON5_PLASM).GetInt();

    m_Price[WEAPON6_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON6_TITAN).GetInt();
    m_Price[WEAPON6_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON6_ELECTRONICS).GetInt();
    m_Price[WEAPON6_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON6_ENERGY).GetInt();
    m_Price[WEAPON6_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON6_PLASM).GetInt();

    m_Price[WEAPON7_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON7_TITAN).GetInt();
    m_Price[WEAPON7_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON7_ELECTRONICS).GetInt();
    m_Price[WEAPON7_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON7_ENERGY).GetInt();
    m_Price[WEAPON7_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON7_PLASM).GetInt();

    m_Price[WEAPON8_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON8_TITAN).GetInt();
    m_Price[WEAPON8_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON8_ELECTRONICS).GetInt();
    m_Price[WEAPON8_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON8_ENERGY).GetInt();
    m_Price[WEAPON8_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON8_PLASM).GetInt();

    m_Price[WEAPON9_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON9_TITAN).GetInt();
    m_Price[WEAPON9_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON9_ELECTRONICS).GetInt();
    m_Price[WEAPON9_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON9_ENERGY).GetInt();
    m_Price[WEAPON9_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON9_PLASM).GetInt();

    m_Price[WEAPON10_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON10_TITAN).GetInt();
    m_Price[WEAPON10_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON10_ELECTRONICS).GetInt();
    m_Price[WEAPON10_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON10_ENERGY).GetInt();
    m_Price[WEAPON10_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_WEAPON10_PLASM).GetInt();

    // Chassis
    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_PRICE)->BlockGet(PAR_SOURCE_PRICE_CHASSIS);

    m_Price[CHASSIS1_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS1_TITAN).GetInt();
    m_Price[CHASSIS1_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS1_ELECTRONICS).GetInt();
    m_Price[CHASSIS1_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS1_ENERGY).GetInt();
    m_Price[CHASSIS1_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS1_PLASM).GetInt();

    m_Price[CHASSIS2_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS2_TITAN).GetInt();
    m_Price[CHASSIS2_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS2_ELECTRONICS).GetInt();
    m_Price[CHASSIS2_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS2_ENERGY).GetInt();
    m_Price[CHASSIS2_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS2_PLASM).GetInt();

    m_Price[CHASSIS3_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS3_TITAN).GetInt();
    m_Price[CHASSIS3_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS3_ELECTRONICS).GetInt();
    m_Price[CHASSIS3_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS3_ENERGY).GetInt();
    m_Price[CHASSIS3_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS3_PLASM).GetInt();

    m_Price[CHASSIS4_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS4_TITAN).GetInt();
    m_Price[CHASSIS4_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS4_ELECTRONICS).GetInt();
    m_Price[CHASSIS4_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS4_ENERGY).GetInt();
    m_Price[CHASSIS4_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS4_PLASM).GetInt();

    m_Price[CHASSIS5_TITAN] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS5_TITAN).GetInt();
    m_Price[CHASSIS5_ELECTRONICS] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS5_ELECTRONICS).GetInt();
    m_Price[CHASSIS5_ENERGY] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS5_ENERGY).GetInt();
    m_Price[CHASSIS5_PLASM] = bp_tmp->ParGet(PAR_SOURCE_PRICE_CHASSIS5_PLASM).GetInt();

    // Chars
    // Armor
    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_CHARS)->BlockGet(L"Armor");

    m_ItemChars[ARMOR6_STRUCTURE] = (float)bp_tmp->ParGet(PAR_SOURCE_ARMOR6_STRUCTURE).GetDouble();
    m_ItemChars[ARMOR6_ROTATION_SPEED] = (float)bp_tmp->ParGet(PAR_SOURCE_ARMOR6_ROTATION_SPEED).GetDouble();

    m_ItemChars[ARMOR1_STRUCTURE] = (float)bp_tmp->ParGet(PAR_SOURCE_ARMOR1_STRUCTURE).GetDouble();
    m_ItemChars[ARMOR1_ROTATION_SPEED] = (float)bp_tmp->ParGet(PAR_SOURCE_ARMOR1_ROTATION_SPEED).GetDouble();

    m_ItemChars[ARMOR2_STRUCTURE] = (float)bp_tmp->ParGet(PAR_SOURCE_ARMOR2_STRUCTURE).GetDouble();
    m_ItemChars[ARMOR2_ROTATION_SPEED] = (float)bp_tmp->ParGet(PAR_SOURCE_ARMOR2_ROTATION_SPEED).GetDouble();

    m_ItemChars[ARMOR3_STRUCTURE] = (float)bp_tmp->ParGet(PAR_SOURCE_ARMOR3_STRUCTURE).GetDouble();
    m_ItemChars[ARMOR3_ROTATION_SPEED] = (float)bp_tmp->ParGet(PAR_SOURCE_ARMOR3_ROTATION_SPEED).GetDouble();

    m_ItemChars[ARMOR4_STRUCTURE] = (float)bp_tmp->ParGet(PAR_SOURCE_ARMOR4_STRUCTURE).GetDouble();
    m_ItemChars[ARMOR4_ROTATION_SPEED] = (float)bp_tmp->ParGet(PAR_SOURCE_ARMOR4_ROTATION_SPEED).GetDouble();

    m_ItemChars[ARMOR5_STRUCTURE] = (float)bp_tmp->ParGet(PAR_SOURCE_ARMOR5_STRUCTURE).GetDouble();
    m_ItemChars[ARMOR5_ROTATION_SPEED] = (float)bp_tmp->ParGet(PAR_SOURCE_ARMOR5_ROTATION_SPEED).GetDouble();

    // Chassis
    bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_CHARS)->BlockGet(L"Chassis");

    m_ItemChars[CHASSIS1_STRUCTURE] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS1_STRUCTURE).GetDouble();
    m_ItemChars[CHASSIS1_ROTATION_SPEED] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS1_ROTATION_SPEED).GetDouble();
    m_ItemChars[CHASSIS1_MOVE_SPEED] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS1_MOVE_SPEED).GetDouble();
    m_ItemChars[CHASSIS1_MOVE_WATER_CORR] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS1_MOVE_WATER_CORR).GetDouble();
    m_ItemChars[CHASSIS1_MOVE_SLOPE_CORR_DOWN] =
            (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS1_MOVE_SLOPE_CORR_DN).GetDouble();
    m_ItemChars[CHASSIS1_MOVE_SLOPE_CORR_UP] =
            (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS1_MOVE_SLOPE_CORR_UP).GetDouble();

    m_ItemChars[CHASSIS2_STRUCTURE] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS2_STRUCTURE).GetDouble();
    m_ItemChars[CHASSIS2_ROTATION_SPEED] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS2_ROTATION_SPEED).GetDouble();
    m_ItemChars[CHASSIS2_MOVE_SPEED] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS2_MOVE_SPEED).GetDouble();
    m_ItemChars[CHASSIS2_MOVE_WATER_CORR] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS2_MOVE_WATER_CORR).GetDouble();
    m_ItemChars[CHASSIS2_MOVE_SLOPE_CORR_DOWN] =
            (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS2_MOVE_SLOPE_CORR_DN).GetDouble();
    m_ItemChars[CHASSIS2_MOVE_SLOPE_CORR_UP] =
            (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS2_MOVE_SLOPE_CORR_UP).GetDouble();

    m_ItemChars[CHASSIS3_STRUCTURE] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS3_STRUCTURE).GetDouble();
    m_ItemChars[CHASSIS3_ROTATION_SPEED] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS3_ROTATION_SPEED).GetDouble();
    m_ItemChars[CHASSIS3_MOVE_SPEED] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS3_MOVE_SPEED).GetDouble();
    m_ItemChars[CHASSIS3_MOVE_WATER_CORR] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS3_MOVE_WATER_CORR).GetDouble();
    m_ItemChars[CHASSIS3_MOVE_SLOPE_CORR_DOWN] =
            (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS3_MOVE_SLOPE_CORR_DN).GetDouble();
    m_ItemChars[CHASSIS3_MOVE_SLOPE_CORR_UP] =
            (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS3_MOVE_SLOPE_CORR_UP).GetDouble();

    m_ItemChars[CHASSIS4_STRUCTURE] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS4_STRUCTURE).GetDouble();
    m_ItemChars[CHASSIS4_ROTATION_SPEED] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS4_ROTATION_SPEED).GetDouble();
    m_ItemChars[CHASSIS4_MOVE_SPEED] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS4_MOVE_SPEED).GetDouble();
    m_ItemChars[CHASSIS4_MOVE_WATER_CORR] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS4_MOVE_WATER_CORR).GetDouble();
    m_ItemChars[CHASSIS4_MOVE_SLOPE_CORR_DOWN] =
            (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS4_MOVE_SLOPE_CORR_DN).GetDouble();
    m_ItemChars[CHASSIS4_MOVE_SLOPE_CORR_UP] =
            (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS4_MOVE_SLOPE_CORR_UP).GetDouble();

    m_ItemChars[CHASSIS5_STRUCTURE] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS5_STRUCTURE).GetDouble();
    m_ItemChars[CHASSIS5_ROTATION_SPEED] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS5_ROTATION_SPEED).GetDouble();
    m_ItemChars[CHASSIS5_MOVE_SPEED] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS5_MOVE_SPEED).GetDouble();
    m_ItemChars[CHASSIS5_MOVE_WATER_CORR] = (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS5_MOVE_WATER_CORR).GetDouble();
    m_ItemChars[CHASSIS5_MOVE_SLOPE_CORR_DOWN] =
            (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS5_MOVE_SLOPE_CORR_DN).GetDouble();
    m_ItemChars[CHASSIS5_MOVE_SLOPE_CORR_UP] =
            (float)bp_tmp->ParGet(PAR_SOURCE_CHASSIS5_MOVE_SLOPE_CORR_UP).GetDouble();

    // Chassis
    // bp_tmp = g_MatrixData->BlockGet(PAR_SOURCE_CHARS)->BlockGet(L"Bonuses");

    // m_ItemChars[ARMOR] = (float)bp_tmp->ParGet(L"ARMOR").GetDouble();
    // m_ItemChars[WEAPON_SPEED] = (float)bp_tmp->ParGet(L"WEAPON_SPEED").GetDouble();
    // m_ItemChars[WEAPON_HEAT] = (float)bp_tmp->ParGet(L"WEAPON_HEAT").GetDouble();
    // m_ItemChars[LESS_HIT] = (float)bp_tmp->ParGet(L"LESS_HIT").GetDouble();
    // m_ItemChars[CHASSIS_SPEED] = (float)bp_tmp->ParGet(L"CHASSIS_SPEED").GetDouble();
    // m_ItemChars[WEAPON_RADIUS] = (float)bp_tmp->ParGet(L"WEAPON_RADIUS").GetDouble();
    // m_ItemChars[ROBOT_RADAR_RADIUS] = (float)bp_tmp->ParGet(L"RADAR_RADIUS").GetDouble();
    // m_ItemChars[ELECTRO_PROTECTION] = (float)bp_tmp->ParGet(L"ELECTRO_PROTECTION").GetDouble();

    m_RobotRadarR = (float)g_MatrixData->BlockGet(PAR_SOURCE_RADAR)->ParGet(PAR_SOURCE_RADAR_ROBOT_RADIUS).GetDouble();
    m_FlyerRadarR = (float)g_MatrixData->BlockGet(PAR_SOURCE_RADAR)->ParGet(PAR_SOURCE_RADAR_FLYER_RADIUS).GetDouble();

    // AI strange
    bp_tmp = g_MatrixData->BlockPathGet(L"AIStrange.Weapon");
    m_WeaponStrengthAI[RUK_WEAPON_MACHINEGUN] = (float)(bp_tmp->ParGet(L"Machinegun").GetDouble());
    m_WeaponStrengthAI[RUK_WEAPON_CANNON] = (float)(bp_tmp->ParGet(L"Cannon").GetDouble());
    m_WeaponStrengthAI[RUK_WEAPON_MISSILE] = (float)(bp_tmp->ParGet(L"Missile").GetDouble());
    m_WeaponStrengthAI[RUK_WEAPON_FLAMETHROWER] = (float)(bp_tmp->ParGet(L"Flamethrower").GetDouble());
    m_WeaponStrengthAI[RUK_WEAPON_MORTAR] = (float)(bp_tmp->ParGet(L"Mortar").GetDouble());
    m_WeaponStrengthAI[RUK_WEAPON_LASER] = (float)(bp_tmp->ParGet(L"Laser").GetDouble());
    m_WeaponStrengthAI[RUK_WEAPON_BOMB] = (float)(bp_tmp->ParGet(L"Bomb").GetDouble());
    m_WeaponStrengthAI[RUK_WEAPON_PLASMA] = (float)(bp_tmp->ParGet(L"Plasma").GetDouble());
    m_WeaponStrengthAI[RUK_WEAPON_ELECTRIC] = (float)(bp_tmp->ParGet(L"Electric").GetDouble());
    m_WeaponStrengthAI[RUK_WEAPON_REPAIR] = (float)(bp_tmp->ParGet(L"Repair").GetDouble());

    // Items labels
    CBlockPar *bpl = g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR);
    bp_tmp = bpl->BlockGetNE(PAR_SOURCE_ITEMS_LABELS);

    m_Labels = (std::wstring*)HAlloc(sizeof(std::wstring) * LABELS_LAST, g_MatrixHeap);
    for (int i = 0; i < LABELS_LAST; i++) {
        new(&m_Labels[i]) std::wstring();
    }
    m_Labels[W1_CHAR] = bp_tmp->ParGet(PAR_SOURCE_W1_CHAR);
    m_Labels[W2_CHAR] = bp_tmp->ParGet(PAR_SOURCE_W2_CHAR);
    m_Labels[W3_CHAR] = bp_tmp->ParGet(PAR_SOURCE_W3_CHAR);
    m_Labels[W4_CHAR] = bp_tmp->ParGet(PAR_SOURCE_W4_CHAR);
    m_Labels[W5_CHAR] = bp_tmp->ParGet(PAR_SOURCE_W5_CHAR);
    m_Labels[W6_CHAR] = bp_tmp->ParGet(PAR_SOURCE_W6_CHAR);
    m_Labels[W7_CHAR] = bp_tmp->ParGet(PAR_SOURCE_W7_CHAR);
    m_Labels[W8_CHAR] = bp_tmp->ParGet(PAR_SOURCE_W8_CHAR);
    m_Labels[W9_CHAR] = bp_tmp->ParGet(PAR_SOURCE_W9_CHAR);
    m_Labels[W10_CHAR] = bp_tmp->ParGet(PAR_SOURCE_W10_CHAR);

    m_Labels[HE1_CHAR] = bp_tmp->ParGet(PAR_SOURCE_HE1_CHAR);
    m_Labels[HE2_CHAR] = bp_tmp->ParGet(PAR_SOURCE_HE2_CHAR);
    m_Labels[HE3_CHAR] = bp_tmp->ParGet(PAR_SOURCE_HE3_CHAR);
    m_Labels[HE4_CHAR] = bp_tmp->ParGet(PAR_SOURCE_HE4_CHAR);

    m_Labels[HU1_CHAR] = bp_tmp->ParGet(PAR_SOURCE_HU1_CHAR);
    m_Labels[HU2_CHAR] = bp_tmp->ParGet(PAR_SOURCE_HU2_CHAR);
    m_Labels[HU3_CHAR] = bp_tmp->ParGet(PAR_SOURCE_HU3_CHAR);
    m_Labels[HU4_CHAR] = bp_tmp->ParGet(PAR_SOURCE_HU4_CHAR);
    m_Labels[HU5_CHAR] = bp_tmp->ParGet(PAR_SOURCE_HU5_CHAR);
    m_Labels[HU6_CHAR] = bp_tmp->ParGet(PAR_SOURCE_HU6_CHAR);

    m_Labels[CH1_CHAR] = bp_tmp->ParGet(PAR_SOURCE_CH1_CHAR);
    m_Labels[CH2_CHAR] = bp_tmp->ParGet(PAR_SOURCE_CH2_CHAR);
    m_Labels[CH3_CHAR] = bp_tmp->ParGet(PAR_SOURCE_CH3_CHAR);
    m_Labels[CH4_CHAR] = bp_tmp->ParGet(PAR_SOURCE_CH4_CHAR);
    m_Labels[CH5_CHAR] = bp_tmp->ParGet(PAR_SOURCE_CH5_CHAR);

    // Items descriptions
    bp_tmp = bpl->BlockGetNE(PAR_SOURCE_ITEMS_DESCRIPTIONS);

    m_Descriptions = (std::wstring*)HAlloc(sizeof(std::wstring) * DESCRIPTIONS_LAST, g_MatrixHeap);
    for (int i = 0; i < DESCRIPTIONS_LAST; i++) {
        new(&m_Descriptions[i]) std::wstring();
    }
    m_Descriptions[W1_DESCR] = bp_tmp->ParGet(PAR_SOURCE_W1_DESCR);
    m_Descriptions[W2_DESCR] = bp_tmp->ParGet(PAR_SOURCE_W2_DESCR);
    m_Descriptions[W3_DESCR] = bp_tmp->ParGet(PAR_SOURCE_W3_DESCR);
    m_Descriptions[W4_DESCR] = bp_tmp->ParGet(PAR_SOURCE_W4_DESCR);
    m_Descriptions[W5_DESCR] = bp_tmp->ParGet(PAR_SOURCE_W5_DESCR);
    m_Descriptions[W6_DESCR] = bp_tmp->ParGet(PAR_SOURCE_W6_DESCR);
    m_Descriptions[W7_DESCR] = bp_tmp->ParGet(PAR_SOURCE_W7_DESCR);
    m_Descriptions[W8_DESCR] = bp_tmp->ParGet(PAR_SOURCE_W8_DESCR);
    m_Descriptions[W9_DESCR] = bp_tmp->ParGet(PAR_SOURCE_W9_DESCR);
    m_Descriptions[W10_DESCR] = bp_tmp->ParGet(PAR_SOURCE_W10_DESCR);

    m_Descriptions[HE1_DESCR] = bp_tmp->ParGet(PAR_SOURCE_HE1_DESCR);
    m_Descriptions[HE2_DESCR] = bp_tmp->ParGet(PAR_SOURCE_HE2_DESCR);
    m_Descriptions[HE3_DESCR] = bp_tmp->ParGet(PAR_SOURCE_HE3_DESCR);
    m_Descriptions[HE4_DESCR] = bp_tmp->ParGet(PAR_SOURCE_HE4_DESCR);

    m_Descriptions[HU1_DESCR] = bp_tmp->ParGet(PAR_SOURCE_HU1_DESCR);
    m_Descriptions[HU2_DESCR] = bp_tmp->ParGet(PAR_SOURCE_HU2_DESCR);
    m_Descriptions[HU3_DESCR] = bp_tmp->ParGet(PAR_SOURCE_HU3_DESCR);
    m_Descriptions[HU4_DESCR] = bp_tmp->ParGet(PAR_SOURCE_HU4_DESCR);
    m_Descriptions[HU5_DESCR] = bp_tmp->ParGet(PAR_SOURCE_HU5_DESCR);
    m_Descriptions[HU6_DESCR] = bp_tmp->ParGet(PAR_SOURCE_HU6_DESCR);

    m_Descriptions[CH1_DESCR] = bp_tmp->ParGet(PAR_SOURCE_CH1_DESCR);
    m_Descriptions[CH2_DESCR] = bp_tmp->ParGet(PAR_SOURCE_CH2_DESCR);
    m_Descriptions[CH3_DESCR] = bp_tmp->ParGet(PAR_SOURCE_CH3_DESCR);
    m_Descriptions[CH4_DESCR] = bp_tmp->ParGet(PAR_SOURCE_CH4_DESCR);
    m_Descriptions[CH5_DESCR] = bp_tmp->ParGet(PAR_SOURCE_CH5_DESCR);

    // camera properties
    bp_tmp = g_MatrixData->BlockGetNE(PAR_SOURCE_CAMERA);
    if (bp_tmp) {
        int cnt = bp_tmp->ParCount();
        for (int i = 0; i < cnt; ++i) {
            if (bp_tmp->ParGetName(i) == PAR_SOURCE_CAMERA_BASEANGLEZ)
                m_CamBaseAngleZ = GRAD2RAD((float)bp_tmp->ParGet(i).GetDouble());
            else if (bp_tmp->ParGetName(i) == PAR_SOURCE_CAMERA_INROBOTFWD0)
                m_CamInRobotForward0 = (float)bp_tmp->ParGet(i).GetDouble();
            else if (bp_tmp->ParGetName(i) == PAR_SOURCE_CAMERA_INROBOTFWD1)
                m_CamInRobotForward1 = (float)bp_tmp->ParGet(i).GetDouble();
            else if (bp_tmp->ParGetName(i) == PAR_SOURCE_CAMERA_MOVESPEED)
                m_CamMoveSpeed = (float)bp_tmp->ParGet(i).GetDouble();
        }

        cnt = bp_tmp->BlockCount();
        for (int i = 0; i < cnt; ++i) {
            int index = -1;
            if (bp_tmp->BlockGetName(i) == PAR_SOURCE_CAMERA_STRATEGY)
                index = CAMERA_STRATEGY;
            else if (bp_tmp->BlockGetName(i) == PAR_SOURCE_CAMERA_INROBOT)
                index = CAMERA_INROBOT;
            if (index >= 0) {
                CBlockPar *bp_c = bp_tmp->BlockGet(i);

                int cnt2 = bp_c->ParCount();
                for (int j = 0; j < cnt2; ++j) {
                    if (bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_ROTSPEEDX)
                        m_CamParams[index].m_CamRotSpeedX = (float)bp_c->ParGet(j).GetDouble();
                    else if (bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_ROTSPEEDZ)
                        m_CamParams[index].m_CamRotSpeedZ = (float)bp_c->ParGet(j).GetDouble();
                    else if (bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_WHEELSTEP)
                        m_CamParams[index].m_CamMouseWheelStep = (float)bp_c->ParGet(j).GetDouble();
                    else if (bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_ROTANGLEMIN)
                        m_CamParams[index].m_CamRotAngleMin = GRAD2RAD(std::min(94.0f, (float)bp_c->ParGet(j).GetDouble()));
                    else if (bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_ROTANGLEMAX)
                        m_CamParams[index].m_CamRotAngleMax = GRAD2RAD((float)bp_c->ParGet(j).GetDouble());
                    else if (bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_DISTMIN)
                        m_CamParams[index].m_CamDistMin = (float)bp_c->ParGet(j).GetDouble();
                    else if (bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_DISTMAX)
                        m_CamParams[index].m_CamDistMax = (float)bp_c->ParGet(j).GetDouble();
                    else if (bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_ANGLEPARAM)
                        m_CamParams[index].m_CamAngleParam = (float)bp_c->ParGet(j).GetDouble();
                    else if (bp_c->ParGetName(j) == PAR_SOURCE_CAMERA_HEIGHT)
                        m_CamParams[index].m_CamHeight = (float)bp_c->ParGet(j).GetDouble();
                }
            }
        }
    }

    bp_tmp = g_MatrixData->BlockGet(L"Models")->BlockGet(PAR_SOURCE_CANNONS);
    {
        int cnt = bp_tmp->BlockCount();
        ASSERT(cnt == CANNON_TYPE_CNT);
        for (int i = 0; i < cnt; ++i) {
            CBlockPar *bp = bp_tmp->BlockGet(i);

            m_CannonsProps[i].max_top_angle = GRAD2RAD((float)bp->ParGet(PAR_SOURCE_CANNONS_MAX_TOP_ANGLE).GetDouble());
            m_CannonsProps[i].max_bottom_angle =
                    GRAD2RAD((float)bp->ParGet(PAR_SOURCE_CANNONS_MAX_BOTTOM_ANGLE).GetDouble());
            m_CannonsProps[i].seek_radius = (float)bp->ParGet(PAR_SOURCE_CANNONS_RADIUS).GetDouble();
            m_CannonsProps[i].max_da = GRAD2RAD((float)bp->ParGet(PAR_SOURCE_CANNONS_ROTATION).GetDouble());
            m_CannonsProps[i].weapon = WeapName2Weap(bp->ParGet(PAR_SOURCE_CANNONS_WEAPON).c_str());
            // m_CannonsProps[i].cooldown = bp->ParGet(PAR_SOURCE_CANNONS_COOLDOWN).GetInt();
            m_CannonsProps[i].m_Resources[TITAN] = bp->ParGet(L"Titan").GetInt();
            m_CannonsProps[i].m_Resources[ENERGY] = bp->ParGet(L"Energy").GetInt();
            m_CannonsProps[i].m_Resources[PLASMA] = bp->ParGet(L"Plasm").GetInt();
            m_CannonsProps[i].m_Resources[ELECTRONICS] = bp->ParGet(L"Electronics").GetInt();
            m_CannonsProps[i].m_Strength = float(bp->ParGet(L"Strength").GetDouble());
            m_CannonsProps[i].m_Hitpoint = float(bp->ParGet(L"Hitpoint").GetDouble());
        }
    }
}

static void GenRamp(WORD *out, SGammaVals &vals) {
    const float brk = 1;
    const float cok = 1;

    float contrast = (vals.contrast - 0.5f) * cok;
    float brightness = (vals.brightness - 0.5f) * brk;

    float x = (contrast + 0.5f) * 0.45f;
    float y = 0.45f - x;
    float mu = (-x) / (0.5f - x);

    y += mu * (0.5f - y);
    mu = (0.5f - y) / 128.0f;

    for (int i = 0; i < 256; i++) {
        float f = (float)pow((float)y, 1.0f / vals.gamma) + brightness;
        y += mu;

        int v = Float2Int(f * 65535.0f);
        if (v < 0)
            v = 0;
        if (v > 65535)
            v = 65535;
        out[i] = (WORD)v;
    }
}

void CMatrixConfig::ApplyGammaRamp(void) {
    D3DGAMMARAMP newramp;
    GenRamp(newramp.red, m_GammaR);
    GenRamp(newramp.green, m_GammaG);
    GenRamp(newramp.blue, m_GammaB);

    g_D3DD->SetGammaRamp(0, D3DSGR_CALIBRATE, &newramp);
    // g_D3DD->SetGammaRamp(1, D3DSGR_CALIBRATE, &newramp);
}

CMatrixConfig g_Config;
