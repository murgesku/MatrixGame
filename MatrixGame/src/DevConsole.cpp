// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixMap.hpp"
#include "DevConsole.hpp"
#include "MatrixSoundManager.hpp"

#include "CFile.hpp"

#include <utils.hpp>

static void hHelp(const std::wstring& cmd, const std::wstring& params) {
    g_MatrixMap->m_Console.ShowHelp();
}

static void hShadows(const std::wstring& cmd, const std::wstring& params) {
    if (params.length() == 2) {
        g_Config.m_ShowStencilShadows = params[0] == '1';
        g_Config.m_ShowProjShadows = params[1] == '1';
    }
    g_MatrixMap->m_DI.T(L"Stencil shadows", g_Config.m_ShowStencilShadows ? L"ON" : L"OFF");
    g_MatrixMap->m_DI.T(L"Proj shadows", g_Config.m_ShowProjShadows ? L"ON" : L"OFF");
}

static void hCannon(const std::wstring& cmd, const std::wstring& params) {
    if (params.length() == 1) {
        g_Config.m_CannonsLogic = params[0] == '1';
    }
    g_MatrixMap->m_DI.T(L"Cannon's logic", g_Config.m_CannonsLogic ? L"ON" : L"OFF");
}

static void hLog(const std::wstring& cmd, const std::wstring& params) {
    if (params == L"s") {
        CSound::SaveSoundLog();
    }
    else if (params == L"e") {
        CBuf b;
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

            b.StrNZ(utils::format("%d - %d\n", i, cnt));
        }

        b.SaveInFile(L"log.txt");
    }
}

static void hBuildCFG(const std::wstring& cmd, const std::wstring& params) {
    CBlockPar bpi;
    bpi.LoadFromTextFile(IF_PATH);

    CStorage stor(g_CacheHeap);

    CBlockPar data;
    data.CopyFrom(*g_MatrixData);
    data.BlockDelete(PAR_REPLACE);

    stor.StoreBlockPar(L"if", bpi);
    stor.StoreBlockPar(L"da", data);

    stor.Save(FILE_CONFIGURATION, true);
}

static void hTestSpdTrace(const std::wstring& cmd, const std::wstring& params) {
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

    g_MatrixMap->m_DI.T(L"Trace time (ms)", utils::format(L"%u", time2 - time1).c_str(), 5000);
}

static void hMusic(const std::wstring& cmd, const std::wstring& params) {
    if (params == L"1")
        g_MatrixMap->RestoreMusicVolume();
    else if (params == L"0")
        g_MatrixMap->SetMusicVolume(0);
}

static void hCalcVis(const std::wstring& cmd, const std::wstring& params) {
    g_MatrixMap->CalcVis();
}

static void hCompress(const std::wstring& cmd, const std::wstring& params) {
    std::wstring name;
    if (CFile::FileExist(name, params.c_str())) {
        CBuf fil;
        CStorage out(g_CacheHeap);
        fil.LoadFromFile(params);

        CStorageRecord sr(L"0", g_CacheHeap);
        sr.AddItem(CStorageRecordItem(L"0", ST_BYTE));
        out.AddRecord(sr);

        CDataBuf *b = out.GetBuf(L"0", L"0", ST_BYTE);
        b->AddArray();
        b->AddToArray<BYTE>(0, (BYTE *)fil.Get(), fil.Len());

        CacheReplaceFileExt(name, params.c_str(), L".strg");

        out.Save(name.c_str(), true);
    }
}

SCmdItem CDevConsole::m_Commands[] = {
        {L"HELP", hHelp},   {L"SHADOWS", hShadows},       {L"CANNON", hCannon},
        {L"LOG", hLog},     {L"TRACESPD", hTestSpdTrace}, {L"BUILDCFG", hBuildCFG},
        {L"MUSIC", hMusic}, {L"COMPRESS", hCompress},     {L"CALCVIS", hCalcVis},

        {NULL, NULL}  // last
};

CDevConsole::CDevConsole(void) : m_Flags(0), m_Text{}, m_CurPos(0) {
    m_Time = 0;
    m_NextTime = 0;
}

CDevConsole::~CDevConsole() {}

void CDevConsole::ShowHelp(void) {
    int i = 0;
    while (m_Commands[i].cmd != NULL) {
        std::wstring desc;

        if (i == 0)
            desc = L"Shows help";
        else if (i == 1)
            desc = L"Switch shadows : [0|1][0|1]";
        else if (i == 2)
            desc = L"Switch cannons logic : [0|1]";

        g_MatrixMap->m_DI.T(m_Commands[i].cmd, desc.c_str(), 5000);

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

    std::wstring out{m_Text.c_str(), static_cast<size_t>(m_CurPos)};

    if (FLAG(m_Flags, DCON_CURSOR)) {
        out += L"|";
    }
    else {
    }
    if (m_CurPos < m_Text.length()) {
        out += std::wstring{m_Text.c_str() + m_CurPos};
    }
    else {
        out += L" ";
    }

    g_MatrixMap->m_DI.T(L"Console", out.c_str(), 1000);
}
void CDevConsole::Keyboard(int scan, bool down) {
    if (down) {
        SETFLAG(m_Flags, DCON_CURSOR);
        if (scan == KEY_BACKSPACE) {
            if (m_CurPos > 0) {
                --m_CurPos;
                m_Text.erase(m_CurPos, 1);
            }
        }
        else if (scan == KEY_DELETE) {
            if (m_CurPos < m_Text.length()) {
                m_Text.erase(m_CurPos, 1);
            }
        }
        else if (scan == KEY_LEFT) {
            if (m_CurPos > 0) {
                --m_CurPos;
            }
        }
        else if (scan == KEY_RIGHT) {
            if (m_CurPos < m_Text.length()) {
                ++m_CurPos;
            }
        }
        else if (scan == KEY_HOME) {
            m_CurPos = 0;
        }
        else if (scan == KEY_END) {
            m_CurPos = m_Text.length();
        }
        else if (scan == KEY_ENTER) {
            std::wstring cmd;
            std::wstring params;
            int i = 0;
            while (i < m_Text.length()) {
                if (m_Text[i] == ' ')
                    break;
                ++i;
            }
            cmd = std::wstring{m_Text.c_str(), static_cast<size_t>(i)};
            if (i < m_Text.length())
                params = (m_Text.c_str() + i + 1);

            for (auto& sym : cmd)
            {
                sym = towupper(sym);
            }

            i = 0;
            while (m_Commands[i].cmd != NULL) {
                if (m_Commands[i].cmd == cmd) {
                    m_Commands[i].handler(cmd, params);
                    m_Text.clear();
                    m_CurPos = 0;
                    break;
                }
                ++i;
            }
        }
        else if (scan == KEY_ESC) {
            if (m_Text.length() == 0)
                SetActive(false);
            else {
                m_Text.clear();
                m_CurPos = 0;
            }
        }
        else if (scan == KEY_LSHIFT || scan == KEY_RSHIFT) {
            SETFLAG(m_Flags, DCON_SHIFT);
        }
        else {
            wchar c = static_cast<wchar>(Scan2Char(scan));
            if (c != 0) {
                if (!FLAG(m_Flags, DCON_SHIFT)) {
                    if (c >= 'A' && c <= 'Z')
                        c |= 32;
                }
                m_Text.insert(m_CurPos, 1, c);
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
