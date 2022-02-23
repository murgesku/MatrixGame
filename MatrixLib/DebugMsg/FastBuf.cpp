// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "DebugMsg.pch"

bool FastBufInit(CThreadData *td) {
    if (td->FastBuf_File != NULL)
        return true;

    if (!GlobalMsgNewFastBuf(td->FastBuf_FileName, td->FastBuf_EventName))
        return false;

    td->FastBuf_File = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, td->FastBuf_FileName);
    if (td->FastBuf_File == NULL)
        return false;

    td->FastBuf_Buf = (char *)MapViewOfFile(td->FastBuf_File, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (td->FastBuf_Buf == NULL) {
        CloseHandle(td->FastBuf_File);
        td->FastBuf_File = NULL;
        return false;
    }

    td->FastBuf_Event = OpenEvent(EVENT_ALL_ACCESS, false, td->FastBuf_EventName);
    if (td->FastBuf_Event == NULL) {
        UnmapViewOfFile(td->FastBuf_Buf);
        td->FastBuf_Buf = NULL;
        CloseHandle(td->FastBuf_File);
        td->FastBuf_File = NULL;
        return false;
    }

    return true;
}

void FastBufClear(CThreadData *td) {
    if (td->FastBuf_Event != NULL) {
        SetEvent(td->FastBuf_Event);
        CloseHandle(td->FastBuf_Event);
        td->FastBuf_Event = NULL;
    }
    if (td->FastBuf_Buf != NULL) {
        UnmapViewOfFile(td->FastBuf_Buf);
        td->FastBuf_Buf = NULL;
    }
    if (td->FastBuf_File != NULL) {
        CloseHandle(td->FastBuf_File);
        td->FastBuf_File = NULL;
    }
}

bool FastBuf_Write(CThreadData *td, void *buf1, int size1, void *buf2 = NULL, int size2 = 0, void *buf3 = NULL,
                   int size3 = 0, void *buf4 = NULL, int size4 = 0, void *buf5 = NULL, int size5 = 0) {
    int i = 0;

    while (i <= 1) {
        if (!FastBufInit(td))
            break;

        int maxsize = *(int *)td->FastBuf_Buf;
        int cnt = *(int *)(td->FastBuf_Buf + 4);
        int cursize = *(int *)(td->FastBuf_Buf + 8);

        if ((cursize + size1 + size2 + size3 + size4 + size5) > maxsize) {
            FastBufClear(td);
            Sleep(0);
            continue;
        }

        CopyMemory(td->FastBuf_Buf + cursize + 16, buf1, size1);
        if (buf2 != NULL)
            CopyMemory(td->FastBuf_Buf + cursize + 16 + size1, buf2, size2);
        if (buf3 != NULL)
            CopyMemory(td->FastBuf_Buf + cursize + 16 + size1 + size2, buf3, size3);
        if (buf4 != NULL)
            CopyMemory(td->FastBuf_Buf + cursize + 16 + size1 + size2 + size3, buf4, size4);
        if (buf5 != NULL)
            CopyMemory(td->FastBuf_Buf + cursize + 16 + size1 + size2 + size3 + size4, buf5, size5);
        *(int *)(td->FastBuf_Buf + 8) = cursize + size1 + size2 + size3 + size4 + size5;
        *(int *)(td->FastBuf_Buf + 4) = cnt + 1;

        return true;
    }

    return false;
}

DEBUGMSG_API void DMcc_(char *path, char *text) {
    TEST_ACCESS;

    int lenpath = strlen(path);
    int lentext = strlen(text);

    SZagMsg zag;
    zag.Size = sizeof(SZagMsg) + 4 + lenpath + 4 + lentext;
    QueryPerformanceCounter((LARGE_INTEGER *)(&zag.Time));
    zag.ProcessId = GetCurrentProcessId();
    zag.ThreadId = GetCurrentThreadId();
    zag.Type = 1;

    FastBuf_Write(GetThreadData(), &zag, sizeof(SZagMsg), &lenpath, 4, path, lenpath, &lentext, 4, text, lentext);
}

DEBUGMSG_API void DMww_(wchar_t *path, wchar_t *text) {
    TEST_ACCESS;

    int lenpath = wcslen(path);
    int lentext = wcslen(text);

    SZagMsg zag;
    zag.Size = sizeof(SZagMsg) + 4 + lenpath * 2 + 4 + lentext * 2;
    QueryPerformanceCounter((LARGE_INTEGER *)(&zag.Time));
    zag.ProcessId = GetCurrentProcessId();
    zag.ThreadId = GetCurrentThreadId();
    zag.Type = 2;

    FastBuf_Write(GetThreadData(), &zag, sizeof(SZagMsg), &lenpath, 4, path, lenpath * 2, &lentext, 4, text,
                  lentext * 2);
}
