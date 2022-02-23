// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "DebugMsg.h"

#pragma pack(1)
struct SZagMsg {
    int Size;
    __int64 Time;
    DWORD ProcessId;
    DWORD ThreadId;
    char Type;
};
#pragma pack()

class CThreadData {
public:
    HANDLE FastBuf_File;
    char *FastBuf_Buf;
    HANDLE FastBuf_Event;

    char FastBuf_FileName[MAX_PATH];
    char FastBuf_EventName[MAX_PATH];

public:
    CThreadData(void);
    ~CThreadData();
};

#define TEST_ACCESS                                              \
    if (Global_File == NULL) {                                   \
        __int64 zn;                                              \
        QueryPerformanceCounter((LARGE_INTEGER *)(&zn));         \
        bool tv = (zn - Global_LastInitTime) < Global_TimerFreq; \
        if (tv)                                                  \
            return;                                              \
        Global_LastInitTime = zn;                                \
    }

extern DWORD tlsId;
extern CRITICAL_SECTION Global_CS;
extern HANDLE Global_File;
extern __int64 Global_LastInitTime;
extern __int64 Global_TimerFreq;

extern CRITICAL_SECTION SynBuf_CS;
extern CRITICAL_SECTION Command_CS;

inline CThreadData *GetThreadData(void) {
    return (CThreadData *)TlsGetValue(tlsId);
}

bool GlobalInit(void);
void GlobalClear(void);
bool GlobalMsg(void *buf, int size, void *outbuf, int &outbufsize);
DWORD GlobalMsgProcessId(void);
HANDLE GlobalMsgProcessHandle(void);
bool GlobalMsgNewSynBuf(char *namefile, char *namemutex, char *nameevent);
bool GlobalMsgNewFastBuf(char *namefile, char *nameevent);

bool SynBufInit(void);
void SynBufClear(void);

bool FastBufInit(CThreadData *td);
void FastBufClear(CThreadData *td);

void CommandClear(void);
bool CommandInit(void);
