// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    CThreadData *td;

    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            InitializeCriticalSection(&Global_CS);
            InitializeCriticalSection(&SynBuf_CS);
            InitializeCriticalSection(&Command_CS);

            QueryPerformanceFrequency((LARGE_INTEGER *)(&Global_TimerFreq));

            if ((tlsId = TlsAlloc()) == TLS_OUT_OF_INDEXES)
                return FALSE;

            // No break: Initialize for first thread.

        case DLL_THREAD_ATTACH:
            td = new CThreadData;
            TlsSetValue(tlsId, td);

            break;

        case DLL_THREAD_DETACH:
            td = (CThreadData *)TlsGetValue(tlsId);
            if (td != NULL)
                delete td;

            break;

        case DLL_PROCESS_DETACH:
            CommandClear();
            SynBufClear();
            GlobalClear();
            DeleteCriticalSection(&Global_CS);
            DeleteCriticalSection(&SynBuf_CS);
            DeleteCriticalSection(&Command_CS);

            td = (CThreadData *)TlsGetValue(tlsId);
            if (td != NULL)
                delete td;
            TlsFree(tlsId);
            tlsId = TLS_OUT_OF_INDEXES;

            break;
    }
    return TRUE;
}

DEBUGMSG_API void DM(char *path, char *text) {
    DMcc(path, text);
}

DEBUGMSG_API void DM_(char *path, char *text) {
    DMcc_(path, text);
}

DEBUGMSG_API void DM(wchar_t *path, wchar_t *text) {
    DMww(path, text);
}

DEBUGMSG_API void DM_(wchar_t *path, wchar_t *text) {
    DMww_(path, text);
}

/*DEBUGMSG_API int nDebugMsg=0;

DEBUGMSG_API int fnDebugMsg(void)
{
    return 42;
}

CDebugMsg::CDebugMsg()
{
    return;
}*/
