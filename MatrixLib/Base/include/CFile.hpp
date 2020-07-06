// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "../src/stdafx.h"
#include "CMain.hpp"
#include "CWStr.hpp"

#include <windows.h>

namespace Base {

class CPackCollection;


typedef void (*ENUM_FILES)(const CWStr &name, DWORD user);


class BASE_API CFile : public CMain 
{

    static CPackCollection *m_Packs;
    static int              m_PacksRef;

    DWORD   m_PackHandle;
	HANDLE  m_Handle;	    // Handle файла
	CWStr   m_FileName;	    // Имя файла
	int     m_Open;			// Кол-во вызовов Open


public:
	CFile(CHeap * heap=NULL);
	CFile(const CWStr & filename,CHeap * heap=NULL);
	CFile(const wchar * filename,CHeap * heap=NULL);
	CFile(const wchar * filename,int len,CHeap * heap=NULL);
	~CFile();

    static void StaticInit(void)
    {
        m_Packs = NULL;
        m_PacksRef = 0;
    }

    static void AddPackFile(const wchar *name, CHeap *heap);
    static void OpenPackFiles(void);
    static void ReleasePackFiles(void);

    static void FindFiles(const CWStr & folderfrom, const wchar *files, ENUM_FILES ef, DWORD user);

	void Clear(void);

	void Init(const wchar * filename,int len);
	void Init(const wchar * filename)						{ Init(filename,WStrLen(filename)); }
	void Init(const CWStr & filename)						{ Init(filename.Get(),filename.GetLen()); }

    bool IsOpen(void)                                       { return m_Open>0; }

	void Open(DWORD shareMode=0); // FILE_SHARE_DELETE FILE_SHARE_READ FILE_SHARE_WRITE
	void OpenRead(DWORD shareMode=FILE_SHARE_READ); // FILE_SHARE_DELETE FILE_SHARE_READ FILE_SHARE_WRITE
	bool OpenReadNE(DWORD shareMode=FILE_SHARE_READ);
	void Create(DWORD shareMode=0);
	bool CreateNE(DWORD shareMode=0);

	void Close(void);

	DWORD Size(void) const;
	__int64 SizeFull(void) const;

	__int64 PointerFull(void) const;
	void PointerFull(__int64 zn,int from=FILE_BEGIN) const; // FILE_BEGIN FILE_CURRENT FILE_END

	DWORD Pointer(void) const;
	void Pointer(DWORD zn,int from=FILE_BEGIN) const; // FILE_BEGIN FILE_CURRENT FILE_END

	void Read(void * buf,DWORD kolbyte);
	void Write(void * buf,DWORD kolbyte);


    static bool FileExist(CWStr & outname,const wchar * mname,const wchar * exts = NULL,bool withpar = false);
};

// У пути всегда наконце символ "\"
// Пустой путь без символа "\" (пустая строка)
BASE_API void CorrectFilePath(CWStr & filepath); // Если нужно добовляет в конец символ "\"
BASE_API CWStr GetFilePath(const CWStr & filepath);

}