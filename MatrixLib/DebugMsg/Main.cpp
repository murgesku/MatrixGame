// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "DebugMsg.pch"

DWORD tlsId = TLS_OUT_OF_INDEXES;

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
CThreadData::CThreadData() {
    FastBuf_File = NULL;
    FastBuf_Buf = NULL;
    FastBuf_Event = NULL;
}

CThreadData::~CThreadData() {}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
CRITICAL_SECTION Global_CS;
HANDLE Global_DMSP = NULL;
DWORD Global_DMSPid = 0;
HANDLE Global_File = NULL;
void *Global_Buf = NULL;
HANDLE Global_Mutex = NULL;
HANDLE Global_EventS = NULL;
HANDLE Global_EventR = NULL;
__int64 Global_LastInitTime = 0;
__int64 Global_TimerFreq = 0;

bool GlobalInit() {
    if (Global_File != NULL) {
        if (WaitForSingleObject(Global_DMSP, 0) == WAIT_TIMEOUT)
            return true;
        GlobalClear();
    }

    Global_File = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, "dab_DMG_f");
    if (Global_File == NULL)
        return false;

    Global_Buf = MapViewOfFile(Global_File, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (Global_Buf == NULL) {
        CloseHandle(Global_File);
        Global_File = NULL;
        return false;
    }

    Global_Mutex = OpenMutex(MUTEX_ALL_ACCESS, false, "dab_DMG_m");
    if (Global_Mutex == NULL) {
        UnmapViewOfFile(Global_Buf);
        Global_Buf = NULL;
        CloseHandle(Global_File);
        Global_File = 0;
        return false;
    }

    Global_EventS = OpenEvent(EVENT_ALL_ACCESS, false, "dab_DMG_es");
    if (Global_EventS == NULL) {
        CloseHandle(Global_Mutex);
        Global_Mutex = NULL;
        UnmapViewOfFile(Global_Buf);
        Global_Buf = NULL;
        CloseHandle(Global_File);
        Global_File = NULL;
        return false;
    }

    Global_EventR = OpenEvent(EVENT_ALL_ACCESS, false, "dab_DMG_er");
    if (Global_EventR == NULL) {
        CloseHandle(Global_EventS);
        Global_EventS = NULL;
        CloseHandle(Global_Mutex);
        Global_Mutex = NULL;
        UnmapViewOfFile(Global_Buf);
        Global_Buf = NULL;
        CloseHandle(Global_File);
        Global_File = NULL;
        return false;
    }

    Global_DMSPid = *(DWORD *)Global_Buf;
    if ((Global_DMSP = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Global_DMSPid)) == 0) {
        GlobalClear();
        return false;
    }

    return true;
}

void GlobalClear() {
    EnterCriticalSection(&Global_CS);
    if (Global_EventR != NULL) {
        CloseHandle(Global_EventR);
        Global_EventR = NULL;
    }
    if (Global_EventS != NULL) {
        CloseHandle(Global_EventS);
        Global_EventS = NULL;
    }
    if (Global_Mutex != NULL) {
        CloseHandle(Global_Mutex);
        Global_Mutex = NULL;
    }
    if (Global_Buf != NULL) {
        UnmapViewOfFile(Global_Buf);
        Global_Buf = NULL;
    }
    if (Global_File != NULL) {
        CloseHandle(Global_File);
        Global_File = NULL;
    }
    if (Global_DMSP != NULL) {
        CloseHandle(Global_DMSP);
        Global_DMSP = NULL;
    }
    Global_DMSPid = 0;
    LeaveCriticalSection(&Global_CS);
}

bool GlobalMsg(void *buf, int size, void *outbuf, int &outbufsize) {
    int i = 0;

    EnterCriticalSection(&Global_CS);

    while (i <= 1) {
        if (!GlobalInit())
            break;

        if (WaitForSingleObject(Global_Mutex, 1 * 1000) != WAIT_OBJECT_0) {
            GlobalClear();
            i++;
            continue;
        }

        *(int *)((char *)Global_Buf + 4) = size;
        CopyMemory((char *)Global_Buf + 4 + 4, buf, size);

        char *gbu = (char *)Global_Buf + 4 + 4 + size;

        SetEvent(Global_EventS);
        if (WaitForSingleObject(Global_EventR, 30 * 1000) != WAIT_OBJECT_0) {
            ReleaseMutex(Global_Mutex);
            GlobalClear();
            i++;
            continue;
        }

        if (outbufsize < *(int *)gbu) {
            ReleaseMutex(Global_Mutex);
            break;
        }

        outbufsize = *(int *)gbu;
        if (outbufsize > 0)
            CopyMemory(outbuf, (char *)gbu + 4, outbufsize);
        else {
            ReleaseMutex(Global_Mutex);
            break;
        }

        ReleaseMutex(Global_Mutex);
        LeaveCriticalSection(&Global_CS);
        return true;
    }

    LeaveCriticalSection(&Global_CS);

    return false;
}

DWORD GlobalMsgProcessId() {
    /*	int tv=4;
        *zn=0;
        if(!GlobalMsg(zn,4,zn,tv)) return false;

        return true;*/

    DWORD zn;

    EnterCriticalSection(&Global_CS);
    if (!GlobalInit()) {
        LeaveCriticalSection(&Global_CS);
        return 0;
    }
    zn = Global_DMSPid;
    LeaveCriticalSection(&Global_CS);
    return zn;
}

HANDLE GlobalMsgProcessHandle() {
    DWORD id;
    if ((id = GlobalMsgProcessId()) == 0)
        return NULL;

    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, id);
}

bool GlobalMsgNewSynBuf(char *namefile, char *namemutex, char *nameevent) {
    int bufsize = MAX_PATH + MAX_PATH + MAX_PATH;
    char buf[MAX_PATH + MAX_PATH + MAX_PATH];
    *(int *)buf = 1;
    *(DWORD *)(buf + 4) = GetCurrentProcessId();

    if (!GlobalMsg(buf, 8, buf, bufsize))
        return false;

    int sme = 0;
    int l;

    l = strlen(buf + sme);
    if (l > 0)
        CopyMemory(namefile, buf + sme, l);
    namefile[l] = 0;
    sme += l + 1;

    l = strlen(buf + sme);
    if (l > 0)
        CopyMemory(namemutex, buf + sme, l);
    namemutex[l] = 0;
    sme += l + 1;

    l = strlen(buf + sme);
    if (l > 0)
        CopyMemory(nameevent, buf + sme, l);
    nameevent[l] = 0;
    sme += l + 1;

    return true;
}

bool GlobalMsgNewFastBuf(char *namefile, char *nameevent) {
    int bufsize = MAX_PATH + MAX_PATH + MAX_PATH;
    char buf[MAX_PATH + MAX_PATH + MAX_PATH];
    *(int *)buf = 2;

    HANDLE opih;
    if ((opih = GlobalMsgProcessHandle()) == 0)
        return false;

    HANDLE newh;
    if (!DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), opih /*GetCurrentProcess()*/, &newh,
                         THREAD_ALL_ACCESS, FALSE, 0)) {
        CloseHandle(opih);
        return false;
    }
    CloseHandle(opih);
    *(DWORD *)(buf + 4) = DWORD(newh);

    if (!GlobalMsg(buf, 8, buf, bufsize)) {
        CloseHandle(newh);
        return false;
    }

    int sme = 0;
    int l;

    l = strlen(buf + sme);
    if (l > 0)
        CopyMemory(namefile, buf + sme, l);
    namefile[l] = 0;
    sme += l + 1;

    l = strlen(buf + sme);
    if (l > 0)
        CopyMemory(nameevent, buf + sme, l);
    nameevent[l] = 0;
    sme += l + 1;

    return true;
}

bool DMActivate() {
    int bufsize = MAX_PATH + MAX_PATH + MAX_PATH;
    char buf[MAX_PATH + MAX_PATH + MAX_PATH];
    *(int *)buf = 3;

    if (!GlobalMsg(buf, 4, buf, bufsize)) {
        return false;
    }

    return (*((int *)buf)) != 0;
}
