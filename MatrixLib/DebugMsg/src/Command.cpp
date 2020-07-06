// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"

#include "stdio.h"

CRITICAL_SECTION Command_CS;
HANDLE Command_DMSP=NULL;
HANDLE Command_File=NULL;
char * Command_Buf=NULL;
HANDLE Command_Mutex=NULL;
HANDLE Command_EventS=NULL;
HANDLE Command_EventR=NULL;

/*int SFT(char * fmt, ...)
{
	FILE * fa;
	if((fa=fopen("c:\\########.log","a"))==NULL) return -1;

	va_list va;
	va_start(va, fmt);
	vfprintf(fa,fmt,va);
	va_end(va);

	fclose(fa);
	return 0;
}*/

bool CommandInit()
{
    if(Command_File!=NULL) {
		if(WaitForSingleObject(Command_DMSP,0)==WAIT_TIMEOUT) return true;
		CommandClear();
	}

    Command_File=OpenFileMapping(FILE_MAP_ALL_ACCESS,false,"dab_DC_f");
    if(Command_File==NULL) return false;

    Command_Buf=(char *)MapViewOfFile(Command_File,FILE_MAP_ALL_ACCESS,0,0,0);
    if(Command_Buf==NULL) {
    	CloseHandle(Command_File); Command_File=NULL;
    	return false;
    }

	Command_Mutex=OpenMutex(MUTEX_ALL_ACCESS,false,"dab_DC_m");
    if(Command_Mutex==NULL) {
	    UnmapViewOfFile(Command_Buf); Command_Buf=NULL;
    	CloseHandle(Command_File); Command_File=0;
    	return false;
    }

    Command_EventS=OpenEvent(EVENT_ALL_ACCESS,false,"dab_DC_es");
    if(Command_EventS==NULL) {
    	CloseHandle(Command_Mutex); Command_Mutex=NULL;
	    UnmapViewOfFile(Command_Buf); Command_Buf=NULL;
    	CloseHandle(Command_File); Command_File=NULL;
    	return false;
    }

    Command_EventR=OpenEvent(EVENT_ALL_ACCESS,false,"dab_DC_er");
    if(Command_EventR==NULL) {
		CloseHandle(Command_EventS); Command_EventS=NULL;
    	CloseHandle(Command_Mutex); Command_Mutex=NULL;
	    UnmapViewOfFile(Command_Buf); Command_Buf=NULL;
    	CloseHandle(Command_File); Command_File=NULL;
    	return false;
    }

	Command_DMSP=GlobalMsgProcessHandle();
	if(Command_DMSP==0) {
		CommandClear();
		return false;
	}

	return true;
}

void CommandClear()
{
	EnterCriticalSection(&Command_CS);
	if(Command_EventR!=NULL) { SetEvent(Command_EventR); CloseHandle(Command_EventR); Command_EventR=NULL; }
	if(Command_EventS!=NULL) { SetEvent(Command_EventS); CloseHandle(Command_EventS); Command_EventS=NULL; }
	if(Command_Mutex!=NULL) { CloseHandle(Command_Mutex); Command_Mutex=NULL; }
    if(Command_Buf!=NULL) { UnmapViewOfFile(Command_Buf); Command_Buf=NULL; }
	if(Command_File!=NULL) { CloseHandle(Command_File); Command_File=NULL; }
	if(Command_DMSP!=NULL) { CloseHandle(Command_DMSP); Command_DMSP=NULL; }
	LeaveCriticalSection(&Command_CS);
}

class CDCommand {
	public:
		char * m_Buf;
		int m_BufSize;
		int m_MaxAnswerSize;
		wchar_t * m_NameW;
		char * m_NameA;
		int m_NameI;
		int m_No;
		int m_ParCnt;

		char * m_ABuf;
		int m_ABufSize;
		int m_ABufSizeMax;
	public:
		CDCommand(void * buf) {
			m_BufSize=*(int *)((char *)(buf)+8);
			m_Buf=(char *)HeapAlloc(GetProcessHeap(),0,m_BufSize);
			CopyMemory(m_Buf,buf,m_BufSize);
			m_NameW=(wchar_t *)((m_Buf)+(*(int *)((m_Buf)+16)));
			m_NameA=(char *)((m_Buf)+(*(int *)((m_Buf)+16+4)));
			m_MaxAnswerSize=*(int *)((char *)(m_Buf))-m_BufSize;
			m_ParCnt=*(int *)((char *)(m_Buf)+4);

			m_ABuf=NULL;
			m_ABufSize=0;
			m_ABufSizeMax=0;

			m_No=2;
		}
		~CDCommand() {
			if(m_Buf!=NULL) { HeapFree(GetProcessHeap(),0,m_Buf); m_Buf=NULL; }
			if(m_ABuf!=NULL) { HeapFree(GetProcessHeap(),0,m_ABuf); m_ABuf=NULL; }
		}
		
		wchar_t * GetParW(void) {
			if(m_No<2) return NULL;
			else if(m_No>=m_ParCnt) return NULL;
			m_No=m_No+2;
			return (wchar_t *)((m_Buf)+(*(int *)((m_Buf)+16+(m_No-2)*4)));
		}
		char * GetParA(void) {
			if(m_No<2) return NULL;
			else if(m_No>=m_ParCnt) return NULL;
			m_No=m_No+2;
			return (char *)((m_Buf)+(*(int *)((m_Buf)+16+(m_No-2+1)*4)));
		}
		int GetInt(void) {
			char * str=GetParA();
			if(str==NULL) return 0;
			
			int zn=0;
			int znak=0;
			while(*str!=0) {
				if((*str>='0') && (*str<='9')) zn=zn*10+(*str-'0');
				if(*str=='-') znak=1;
				str++;
			}

			if(znak) zn=-zn;
			return zn;
		}
		double GetFloat(void) {
			char * str=GetParA();
			if(str==NULL) return 0;

			double zn=0.0;
			int znak=0;
			while(*str!=0) {
				if((*str>='0') && (*str<='9')) zn=zn*10.0+(*str-'0');
				else if(*str=='-') znak=1;
				else if((*str=='.') || (*str==',')) break;
				str++;
			}
			if((*str=='.') || (*str==',')) {
				double tra=10;
				while(*str!=0) {
					if((*str>='0') && (*str<='9')) {
						zn=zn+double(*str-'0')/tra;
						tra=tra*10;
					}
					str++;
				}
			}

			if(znak) zn=-zn;
			return zn;
		}
		void AnswerTest(int size) {
			if(m_ABuf==NULL) {
				m_ABuf=(char *)HeapAlloc(GetProcessHeap(),0,size);
				m_ABufSizeMax=size;
			} else if(m_ABufSizeMax<(m_ABufSize+size)) {
				m_ABufSizeMax=m_ABufSize+size+1024;
				m_ABuf=(char *)HeapReAlloc(GetProcessHeap(),0,m_ABuf,m_ABufSizeMax);
			}
		}
		void Answer(wchar_t * str,int strsize) {
			AnswerTest(strsize*2);
			CopyMemory(m_ABuf+m_ABufSize,str,strsize*2);
			m_ABufSize=m_ABufSize+strsize*2;
		}
		void Answer(char * str,int strsize) {
			int bs=MultiByteToWideChar(CP_ACP,MB_COMPOSITE,
								str,strsize,
								(wchar_t *)(m_ABuf+m_ABufSize),0);

			AnswerTest(bs*2);

			MultiByteToWideChar(CP_ACP,MB_COMPOSITE,
								str,strsize,
								(wchar_t *)(m_ABuf+m_ABufSize),bs);

			m_ABufSize=m_ABufSize+bs*2;
		}
};

DEBUGMSG_API DWORD DCGet(wchar_t * scom)
{
	int i;
	CDCommand * dc=NULL;

	if(scom==NULL) return 0;

	EnterCriticalSection(&Command_CS);
	while(1) {
		if(!CommandInit()) break;
		if(WaitForSingleObject(Command_EventS,/*1000*/0)!=WAIT_OBJECT_0) break;

		if(WaitForSingleObject(Command_Mutex,1000)!=WAIT_OBJECT_0) break;

		if((*(int *)((char *)Command_Buf+12))!=0) { ReleaseMutex(Command_Mutex); break; }

		wchar_t * strcc=(wchar_t *)(((char *)Command_Buf)+(*(int *)(((char *)Command_Buf)+16)));
		i=0;
		while(*scom!=0) {
			while((*scom==32) || (*scom==9)) scom++;
			if(*scom==0) break;

			wchar_t * sc=strcc;
			while(true) {
				if((*sc==0) && ((*scom==0) || (*scom==32) || (*scom==9))) { strcc=NULL; break; }
				if(*sc==*scom) { sc++; scom++; continue; }
				break;
			}
			if(strcc==NULL) break;

			while((*scom!=32) && (*scom!=9) && (*scom!=0)) scom++;
			i++;
		}
		if(strcc!=NULL) { ReleaseMutex(Command_Mutex); break; }

		dc=new CDCommand(Command_Buf);
		dc->m_NameI=i;
		ReleaseMutex(Command_Mutex);

		break;
	}
	LeaveCriticalSection(&Command_CS);

	return DWORD(dc);
}

DEBUGMSG_API void DCFree(DWORD id)
{
	if(id==0) return;
	CDCommand * dc=(CDCommand *)id;
	
	if(dc->m_ABufSize>0) {
		EnterCriticalSection(&Command_CS);
		while(1) {
			if(Command_File=NULL) break;
			if(WaitForSingleObject(Command_Mutex,1000)!=WAIT_OBJECT_0) break;
			if((*(int *)((char *)Command_Buf+12))!=0) { ReleaseMutex(Command_Mutex); break; }
			if((*(int *)((char *)Command_Buf+8))==0) { ReleaseMutex(Command_Mutex); break; }

			if(dc->m_ABufSize>dc->m_MaxAnswerSize) dc->m_ABufSize=dc->m_MaxAnswerSize;
			CopyMemory((char *)Command_Buf+dc->m_BufSize,dc->m_ABuf,dc->m_ABufSize);

			*(int *)((char *)Command_Buf+12)=(dc->m_ABufSize) >> 1;

			SetEvent(Command_EventR);
			ReleaseMutex(Command_Mutex); 
			break;
		}
		LeaveCriticalSection(&Command_CS);
	}

	delete dc;
}

DEBUGMSG_API wchar_t * DCNameW(DWORD id)
{
	if(!id) return NULL;
	return ((CDCommand *)id)->m_NameW;
}

DEBUGMSG_API char * DCNameA(DWORD id)
{
	if(!id) return NULL;
	return ((CDCommand *)id)->m_NameA;
}

DEBUGMSG_API int DCNameI(DWORD id)
{
	if(!id) return -1;
	return ((CDCommand *)id)->m_NameI;
}

DEBUGMSG_API int DCCnt(DWORD id)
{
	if(!id) return 0;
	return (((CDCommand *)id)->m_ParCnt-2)>>1;
}

DEBUGMSG_API wchar_t * DCStrW(DWORD id)
{
	if(!id) return NULL;
	return ((CDCommand *)id)->GetParW();
}

DEBUGMSG_API char * DCStrA(DWORD id)
{
	if(!id) return NULL;
	return ((CDCommand *)id)->GetParA();
}

DEBUGMSG_API int DCInt(DWORD id)
{
	if(!id) return 0;
	return ((CDCommand *)id)->GetInt();
}

DEBUGMSG_API double DCFloat(DWORD id)
{
	if(!id) return 0;
	return ((CDCommand *)id)->GetFloat();
}

DEBUGMSG_API void DCAnswerW(DWORD id,wchar_t * str)
{
	if(!id) return;
	if(str==NULL) return;
	int len=wcslen(str);
	if(len>0) ((CDCommand *)id)->Answer(str,len);
}

DEBUGMSG_API void DCAnswerA(DWORD id,char * str)
{
	if(!id) return;
	if(str==NULL) return;
	int len=strlen(str);
	if(len>0) ((CDCommand *)id)->Answer(str,len);
}
