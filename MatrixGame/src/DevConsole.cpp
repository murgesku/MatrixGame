// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"

#include "MatrixMap.hpp"
#include "DevConsole.hpp"
#include "CWStr.hpp"
#include "MatrixSoundManager.hpp"

static void hHelp(const Base::CWStr &cmd, const Base::CWStr &params) {
    g_MatrixMap->m_Console.ShowHelp();
}

static void hShadows(const Base::CWStr &cmd, const Base::CWStr &params) {
    if (params.GetLen() == 2) {
        g_Config.m_ShowStencilShadows = params[0] == '1';
        g_Config.m_ShowProjShadows = params[1] == '1';
    }
    g_MatrixMap->m_DI.T(L"Stencil shadows", g_Config.m_ShowStencilShadows ? L"ON" : L"OFF");
    g_MatrixMap->m_DI.T(L"Proj shadows", g_Config.m_ShowProjShadows ? L"ON" : L"OFF");
}

static void hCannon(const Base::CWStr &cmd, const Base::CWStr &params) {
    if (params.GetLen() == 1) {
        g_Config.m_CannonsLogic = params[0] == '1';
    }
    g_MatrixMap->m_DI.T(L"Cannon's logic", g_Config.m_CannonsLogic ? L"ON" : L"OFF");
}

static void hLog(const Base::CWStr &cmd, const Base::CWStr &params) {
    if (params == L"s") {
        CSound::SaveSoundLog();
    }
    else if (params == L"e") {
        CBuf b(g_CacheHeap);
        b.StrNZ("Effects:\n");

        for (int i = 0; i < EFFECT_TYPE_COUNT; ++i) {
            int cnt = 0;
            for (PCMatrixEffect e = g_MatrixMap->m_EffectsFirst; e != NULL; e = e->m_Next) {
                if (e->GetType() == (EEffectType)i) {
                    ++cnt;
                }
            }
            if (cnt == 0)
                continue;
            CStr ss(g_CacheHeap);
            ss = i;
            ss += " - ";
            ss += cnt;
            ss += "\n";

            b.StrNZ(ss);
        }

        b.SaveInFile(L"log.txt");
    }
}

static void hBuildCFG(const Base::CWStr &cmd, const Base::CWStr &params) {
    CBlockPar bpi(1, g_CacheHeap);
    bpi.LoadFromTextFile(IF_PATH);

    CStorage stor(g_CacheHeap);

    CBlockPar data(1, g_CacheHeap);
    data.CopyFrom(*g_MatrixData);
    data.BlockDelete(PAR_REPLACE);

    stor.StoreBlockPar(L"if", bpi);
    stor.StoreBlockPar(L"da", data);

    stor.Save(FILE_CONFIGURATION, true);
}

static void hTestSpdTrace(const Base::CWStr &cmd, const Base::CWStr &params) {
    srand(1);
    D3DXVECTOR3 pos1, pos2;

    DWORD time1 = timeGetTime();

    for (int i = 0; i < 100000; ++i) {
        pos1.x = FRND(g_MatrixMap->m_Size.x * GLOBAL_SCALE);
        pos1.y = FRND(g_MatrixMap->m_Size.x * GLOBAL_SCALE);
        pos1.z = FRND(20) + 10;
        pos2.x = FRND(g_MatrixMap->m_Size.x * GLOBAL_SCALE);
        pos2.y = FRND(g_MatrixMap->m_Size.x * GLOBAL_SCALE);
        pos2.z = FRND(20) + 10;

        pos1.z += g_MatrixMap->GetZ(pos1.x, pos1.y);
        pos2.z += g_MatrixMap->GetZ(pos2.x, pos2.y);

        g_MatrixMap->Trace(NULL, pos1, pos2, TRACE_LANDSCAPE, NULL);
    }
    DWORD time2 = timeGetTime();

    g_MatrixMap->m_DI.T(L"Trace time (ms)", CWStr(time2 - time1, g_CacheHeap), 5000);
}

static void hMusic(const Base::CWStr &cmd, const Base::CWStr &params) {
    if (params == L"1")
        g_MatrixMap->RestoreMusicVolume();
    else if (params == L"0")
        g_MatrixMap->SetMusicVolume(0);
}

static void hCalcVis(const Base::CWStr &cmd, const Base::CWStr &params) {
    g_MatrixMap->CalcVis();
}

static void hCompress(const Base::CWStr &cmd, const Base::CWStr &params) {
    CWStr name(g_CacheHeap);
    if (CFile::FileExist(name, params)) {
        CBuf fil(g_CacheHeap);
        CStorage out(g_CacheHeap);
        fil.LoadFromFile(params);

        CStorageRecord sr(CWStr(L"0", g_CacheHeap), g_CacheHeap);
        sr.AddItem(CStorageRecordItem(CWStr(L"0", g_CacheHeap), ST_BYTE));
        out.AddRecord(sr);

        CDataBuf *b = out.GetBuf(L"0", L"0", ST_BYTE);
        b->AddArray();
        b->AddToArray<BYTE>(0, (BYTE *)fil.Get(), fil.Len());

        CacheReplaceFileExt(name, params, L".strg");

        out.Save(name, true);
    }
}

SCmdItem CDevConsole::m_Commands[] = {
        {L"HELP", hHelp},   {L"SHADOWS", hShadows},       {L"CANNON", hCannon},
        {L"LOG", hLog},     {L"TRACESPD", hTestSpdTrace}, {L"BUILDCFG", hBuildCFG},
        {L"MUSIC", hMusic}, {L"COMPRESS", hCompress},     {L"CALCVIS", hCalcVis},

        {NULL, NULL}  // last
};

static wchar Scan2Char(int scan) {
    if (scan == KEY_SPACE)
        return ' ';
    if (scan == KEY_A)
        return 'A';
    if (scan == KEY_B)
        return 'B';
    if (scan == KEY_C)
        return 'C';
    if (scan == KEY_D)
        return 'D';
    if (scan == KEY_E)
        return 'E';
    if (scan == KEY_F)
        return 'F';
    if (scan == KEY_G)
        return 'G';
    if (scan == KEY_H)
        return 'H';
    if (scan == KEY_I)
        return 'I';
    if (scan == KEY_J)
        return 'J';
    if (scan == KEY_K)
        return 'K';
    if (scan == KEY_L)
        return 'L';
    if (scan == KEY_M)
        return 'M';
    if (scan == KEY_N)
        return 'N';
    if (scan == KEY_O)
        return 'O';
    if (scan == KEY_P)
        return 'P';
    if (scan == KEY_Q)
        return 'Q';
    if (scan == KEY_R)
        return 'R';
    if (scan == KEY_S)
        return 'S';
    if (scan == KEY_T)
        return 'T';
    if (scan == KEY_U)
        return 'U';
    if (scan == KEY_V)
        return 'V';
    if (scan == KEY_W)
        return 'W';
    if (scan == KEY_X)
        return 'X';
    if (scan == KEY_Y)
        return 'Y';
    if (scan == KEY_Z)
        return 'Z';
    if (scan == KEY_0)
        return '0';
    if (scan == KEY_1)
        return '1';
    if (scan == KEY_2)
        return '2';
    if (scan == KEY_3)
        return '3';
    if (scan == KEY_4)
        return '4';
    if (scan == KEY_5)
        return '5';
    if (scan == KEY_6)
        return '6';
    if (scan == KEY_7)
        return '7';
    if (scan == KEY_8)
        return '8';
    if (scan == KEY_9)
        return '9';
    if (scan == KEY_LSLASH)
        return '\\';
    if (scan == KEY_COMMA)
        return '.';
    return 0;
}

CDevConsole::CDevConsole(void) : m_Flags(0), m_Text(g_MatrixHeap), m_CurPos(0) {
    m_Time = 0;
    m_NextTime = 0;
}

CDevConsole::~CDevConsole() {}

void CDevConsole::ShowHelp(void) {
    int i = 0;
    while (m_Commands[i].cmd != NULL) {
        CWStr desc(g_MatrixHeap);

        if (i == 0)
            desc = L"Shows help";
        else if (i == 1)
            desc = L"Switch shadows : [0|1][0|1]";
        else if (i == 2)
            desc = L"Switch cannons logic : [0|1]";

        g_MatrixMap->m_DI.T(m_Commands[i].cmd, desc, 5000);

        ++i;
    }
}

void CDevConsole::SetActive(bool active) {
    INITFLAG(m_Flags, DCON_ACTIVE, active);
    if (active) {
        m_NextTime = g_MatrixMap->GetTime();
    }
}

void CDevConsole::Takt(int ms) {
    m_Time += ms;
    while (m_NextTime < m_Time) {
        m_NextTime += DEV_CONSOLE_CURSOR_FLASH_PERIOD;
        INVERTFLAG(m_Flags, DCON_CURSOR);
    }

    CWStr out(g_MatrixHeap);
    out.Set(m_Text.Get(), m_CurPos);

    if (FLAG(m_Flags, DCON_CURSOR)) {
        // out.Add(L"&");
        out.Add(L"|");
    }
    else {
    }
    if (m_CurPos < m_Text.GetLen()) {
        out.Add(m_Text.Get() + m_CurPos);
    }
    else {
        out.Add(L" ");
    }

    g_MatrixMap->m_DI.T(L"Console", out.Get(), 1000);
}
void CDevConsole::Keyboard(int scan, bool down) {
    if (down) {
        SETFLAG(m_Flags, DCON_CURSOR);
        if (scan == KEY_BACKSPACE) {
            if (m_CurPos > 0) {
                --m_CurPos;
                m_Text.Del(m_CurPos, 1);
            }
        }
        else if (scan == KEY_DELETE) {
            if (m_CurPos < m_Text.GetLen()) {
                m_Text.Del(m_CurPos, 1);
            }
        }
        else if (scan == KEY_LEFT) {
            if (m_CurPos > 0) {
                --m_CurPos;
            }
        }
        else if (scan == KEY_RIGHT) {
            if (m_CurPos < m_Text.GetLen()) {
                ++m_CurPos;
            }
        }
        else if (scan == KEY_HOME) {
            m_CurPos = 0;
        }
        else if (scan == KEY_END) {
            m_CurPos = m_Text.GetLen();
        }
        else if (scan == KEY_ENTER) {
            CWStr cmd(g_MatrixHeap);
            CWStr params(g_MatrixHeap);
            int i = 0;
            while (i < m_Text.GetLen()) {
                if (m_Text[i] == ' ')
                    break;
                ++i;
            }
            cmd.Set(m_Text.Get(), i);
            if (i < m_Text.GetLen())
                params.Set(m_Text.Get() + i + 1);
            cmd.UpperCase();

            i = 0;
            while (m_Commands[i].cmd != NULL) {
                if (m_Commands[i].cmd == cmd) {
                    m_Commands[i].handler(cmd, params);
                    m_Text.SetLen(0);
                    m_CurPos = 0;
                    break;
                }
                ++i;
            }
        }
        else if (scan == KEY_ESC) {
            if (m_Text.GetLen() == 0)
                SetActive(false);
            else {
                m_Text.SetLen(0);
                m_CurPos = 0;
            }
        }
        else if (scan == KEY_LSHIFT || scan == KEY_RSHIFT) {
            SETFLAG(m_Flags, DCON_SHIFT);
        }
        else {
            wchar c = Scan2Char(scan);
            if (c != 0) {
                if (!FLAG(m_Flags, DCON_SHIFT)) {
                    if (c >= 'A' && c <= 'Z')
                        c |= 32;
                }
                m_Text.Insert(m_CurPos, CWStr(c, g_MatrixHeap));
                ++m_CurPos;
            }
        }
    }
    else {
        if (scan == KEY_LSHIFT || scan == KEY_RSHIFT) {
            RESETFLAG(m_Flags, DCON_SHIFT);
        }
    }
}
