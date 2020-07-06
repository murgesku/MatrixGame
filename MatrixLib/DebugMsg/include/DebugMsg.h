// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifdef DEBUGMSG_EXPORTS
#define DEBUGMSG_API __declspec(dllexport)
#else
#define DEBUGMSG_API __declspec(dllimport)
#endif

/*class DEBUGMSG_API CDebugMsg {
public:
	CDebugMsg(void);
};

extern DEBUGMSG_API int nDebugMsg;

DEBUGMSG_API int fnDebugMsg(void);*/

extern "C" {
DEBUGMSG_API void DMcc( char * path, char * text);
DEBUGMSG_API void DMcc_( char * path, char * text);
DEBUGMSG_API void DMww( wchar_t * path, wchar_t * text);
DEBUGMSG_API void DMww_( wchar_t * path, wchar_t * text);

DEBUGMSG_API bool DMActivate(void);

DEBUGMSG_API unsigned long DCGet(wchar_t * scom);
DEBUGMSG_API void DCFree(unsigned long id);
DEBUGMSG_API wchar_t * DCNameW(unsigned long id);
DEBUGMSG_API char * DCNameA(unsigned long id);
DEBUGMSG_API int DCNameI(unsigned long id);
DEBUGMSG_API int DCCnt(unsigned long id);
DEBUGMSG_API wchar_t * DCStrW(unsigned long id);
DEBUGMSG_API char * DCStrA(unsigned long id);
DEBUGMSG_API int DCInt(unsigned long id);
DEBUGMSG_API double DCFloat(unsigned long id);
DEBUGMSG_API void DCAnswerW(unsigned long id,wchar_t * str);
DEBUGMSG_API void DCAnswerA(unsigned long id,char * str);
}

DEBUGMSG_API void _cdecl DM(char * path,char * text);
DEBUGMSG_API void _cdecl DM_(char * path,char * text);
DEBUGMSG_API void _cdecl DM(  wchar_t * path, wchar_t * text);
DEBUGMSG_API void _cdecl DM_( wchar_t * path, wchar_t * text);

#ifndef DMDEFINES
#define DMDEFINES
// hack!
__inline void DM(const char*s1, const char*s2) { DM((char *)s1, (char *)s2); };
__inline void DM_(const char*s1, const char*s2) {DM_((char *)s1, (char *)s2); }
__inline void DM(const wchar_t*s1, const wchar_t*s2) {DM((wchar_t *)s1, (wchar_t *)s2); }
__inline void DM_(const wchar_t*s1, const wchar_t*s2) {DM_((wchar_t *)s1, (wchar_t *)s2); }
#endif