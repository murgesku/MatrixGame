// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef PACK_INCLUDE
#define PACK_INCLUDE

#include "CException.hpp"
#include "CMain.hpp"
#include "CHeap.hpp"
#include "CBuf.hpp"

#include <string>
#include <vector>

namespace Base {

//#define RAISE_ALL_EXCEPTIONS
//#define HANDLE_OUT_OF_PACK_FILES
//#define SUPPORT_IN_MEMORY_STRUCTURES

enum EFileType {
    FILEEC_TEXT = 0,
    FILEEC_BINARY = 1,
    FILEEC_COMPRESSED = 2,
    FILEEC_FOLDER = 3,

    FILEEC_FORCE_DWORD = 0x7fffffff
};

#define MAX_VIRTUAL_HANDLE_COUNT_BITS 4
#define MAX_FILENAME_LENGTH           63  // Максимальная длина имени файла

#define MAX_VIRTUAL_HANDLE_COUNT (1 << MAX_VIRTUAL_HANDLE_COUNT_BITS)  // do not modify this

#pragma pack(push, 1)
struct SFileRec {
    DWORD m_Size;                      // Размер файла (+4) - это размер блока
    DWORD m_RealSize;                  // Настоящий размер файла (не сжатого)
    char m_Name[MAX_FILENAME_LENGTH];  // Имя файла
    char m_RealName[MAX_FILENAME_LENGTH];  // Имя файла в файловой системе с учетом регистров
    EFileType m_Type;                      // Тип файла, на который указывает запись
    EFileType m_NType;                     // Тип который должен быть присвоен файлу
    DWORD m_Free;                          // Свободна данная структура или занята
    DWORD m_Date;                          // Дата и время файла
    DWORD m_Offset;  // Смещение данных относительно начала файла
    DWORD m_Extra;   // Данные во время работы объекта
};
#pragma pack(pop)
typedef SFileRec *PSFileRec;

struct SFolderRec {
    DWORD m_Size;     // Размер всей директории
    DWORD m_Recnum;   // Количество записей в ней
    DWORD m_RecSize;  // Размер одной записи файла
};
typedef SFolderRec *PSFolderRec;

struct SFileHandleRec {
    DWORD m_Handle;  // Логический номер файла
    DWORD m_StartOffset;  // Смещение начала файла относительно начала пакетного файла
    DWORD m_Offset;     // Настоящее смещение
    DWORD m_Size;       // Размер файла
    BYTE *m_SouBuf;     // Буфер для чтения сжатых данных
    BYTE *m_DesBuf;     // Буфер для расжатых данных
    int m_Blocknumber;  // Номер блока в сжатом файле
    bool m_Free;        // Запись свободна
    bool m_Compressed;  // Является ли файл сжатым
    WORD dummy00;       // align
};

typedef bool (*FILENAME_CALLBACK_FUNC)(bool Dir, bool Compr, const std::string& name);
class CHsFolder;

struct SSearchRec {
    std::string Name;
    std::string Path;
    int Ind;
    DWORD T;
    CHsFolder *Folder;

    SSearchRec(CHeap*) {}
};

class CHsFolder : public CMain {
    CHeap *m_Heap;

    std::string m_Name;      // Имя папки без учета регистров
    std::string m_RealName;  // Имя папки с учетом регистров
    SFolderRec m_FolderRec;
    CHsFolder *m_Parent;
    SFileRec *m_Files;  // Память, связанная с файлами
    // bool        m_ToUpdate;         // Необходимо обновить информацию в пакетном файле
    // bool        m_ToSave;           // Необходимо записать на новое место в пакетный файл

    SFileRec *GetFileRec(int ind) const  // Возвращает запись по номеру
    {
        if (DWORD(ind) < m_FolderRec.m_Recnum) {
            return (SFileRec *)(((BYTE *)m_Files) + m_FolderRec.m_RecSize * ind);
        }
        return NULL;
    }

    SFileRec *GetFreeRec(void) const;
    bool AddFileEx(const std::string& name, DWORD date, DWORD size, EFileType ftype);  // Добавляет файл в пакетный файл
public:
    CHsFolder(const std::string& name, CHeap *heap);  // Создает пустую папку, не пригодную к работе
    CHsFolder(const std::string& name, CHsFolder *Parent, CHeap *heap);  // Создает пустую папку, не пригодную к работе
    ~CHsFolder() { Clear(); };  // Уничтожает все данные, связанные с объектом

    bool ReadFolder(DWORD Handle, DWORD Offset);  // Читает данные из пакетного файла
    // void    AllocEmptyFolder(void);

    void Clear(void);                         // Очищает память и информацию о файлах
    bool FileExists(const std::string& name) const;  // Существует ли указанный файл
    bool PathExists(const std::string& name) const;  // Существует ли указанный путь
    SFileRec *GetFileRec(const std::string& name) const;    // Возвращает запись по имени файла
    SFileRec *GetFileRecEx(const std::string& name) const;  // Возвращает запись по пути файла
    CHsFolder *GetFolderEx(const std::string& path);
    int FileRecsNumber(void) const  // Возвращает количество записей в директории
    {
        return m_FolderRec.m_Recnum;
    }

    // bool        ReAllocFileRecs(int number);        // Изменяет количество записей в директории (только в сторону
    // увеличения)
    void UpdateFileRec(void);  // Обновляет информацию во внутренней файловой системе

    SFolderRec *GetFolderRec(void) { return &m_FolderRec; }

    //*** Функции, используемые при упаковке в пакетный файл

    // bool    CreateFolder(const std::string& name);
    // bool    AddPath(const std::string& name);
    // bool    AddPath(const std::string& name,EFileType ftype);
    // bool    AddFile(const std::string& name);
    // bool    AddFile(const std::string& name,EFileType ftype);
    // bool    DeleteFile(const std::string& name);
    std::string GetFullPath(const std::string& name);

    DWORD CompressedFileSize(DWORD Handle, const std::string& filename);
    DWORD DecompressedFileSize(DWORD Handle, const std::string& filename);
    int GetSize(void) { return GetLocalSize(); }

    int GetLocalSize(void);
    int GetFolderSize(void) { return sizeof(SFolderRec) + m_FolderRec.m_Recnum * m_FolderRec.m_RecSize; }

    // DWORD   Pack(DWORD Handle,DWORD Offset,int *PInt,const std::string& PStr);
    // DWORD   PackFile(DWORD Handle,const std::string& name,DWORD Offset);
    // DWORD   PackAndCompressFile(DWORD Handle,const std::string& name,DWORD Offset);

    // DWORD   DecodeFile(DWORD Handle,SFileRec *PFile,DWORD Offset);
    // DWORD   EncodeFile(DWORD Handle,SFileRec *PFile,DWORD Offset);

    // DWORD   CompressFile(DWORD SouHandle,DWORD Handle,const std::string& name,DWORD Offset);
    // DWORD   Compress(DWORD SouHandle,DWORD Handle,DWORD Offset,int *PInt,const std::string& PStr);
    // void    CompressFolder(void);

    //    //*** Функции, используемые при распаковке пакетного файла
    // bool    UnpackFile(DWORD SouHandle,const std::string& name);
    // bool    UnpackCompressedFile(DWORD SouHandle,const std::string& name);
    // bool    UnpackFolder(const std::string& name);
    // bool    Unpack(DWORD SouHandle,int *PInt,const std::string& PStr);

    void ListFileNames(FILENAME_CALLBACK_FUNC Func);

    //*** Функции, используемые при установке типа файла *****
    // void    SetFileType(const std::string& name, EFileType NType);
    // void    SetFolderType(const std::string& name, EFileType NType);

    int FindNext(SSearchRec &S);
};

#ifdef SUPPORT_IN_MEMORY_STRUCTURES
#define PFFLAG_EMPTY SETBIT(0)  // Структура физически не связана ни с каким файлом
#endif

class CPackFile : public CMain {
public:
    CHeap *m_Heap;
    std::wstring m_FileName;  // Имя пакетного файла

#ifdef SUPPORT_IN_MEMORY_STRUCTURES
    DWORD m_Flags;
#endif

    // Позволяет создавать виртуальные пакеты в памяти - не связаны с файлом
    DWORD m_Handle;           // Файловый номер
    CHsFolder *m_RootFolder;  // Корневой каталог
    SFileHandleRec m_Handles[MAX_VIRTUAL_HANDLE_COUNT];
    DWORD m_RootOffset;  // Смещение корневого каталога относительно начала файла
    // DWORD           m_WorkFileSize;             // Размер рабочего файла (не пакетного)
    // DWORD           m_WorkFileOffset;           // Смещение рабочего файла
    // DWORD           m_WorkFileStartOffset;      // Начальное смещение рабочего файла
    // DWORD           m_ID;                       // Идентификационный номер пакета в группе.

    int GetFreeHandle(void);

    // Устанавливает указатель в файл - возвращает размер сжатого блока
    DWORD SetCompressedBlockPointer(DWORD StartOffset, int nBlock);

public:
    CPackFile(CHeap *heap, const wchar *name);
    ~CPackFile(void);

    const std::wstring GetName(void) const { return m_FileName; }

    // ******* Процедуры работы с пакетным файлом ******** //
    bool OpenPacketFile(void);  // Открывает пакетный файл и считывает данные чтение
    bool ClosePacketFile(void);  // Закрывает пакетный файл и уничтожает посторонние объекты чтение
    // bool    CreatePacketFileEx(void);               // Создает новый пакетный файл чтение/запись
    bool OpenPacketFileEx(void);  // Открывает пакетный файл и считывает данные запись/чтение
    bool ClosePacketFileEx(void)  // Закрывает пакетный файл и уничтожает посторонние объекты запись/чтение
    {
        return ClosePacketFile();
    }
    DWORD GetHandle(void) const { return m_Handle; }
    void Clear(void);
    //***** Процедуры работы с файлами -- позиционирование указателя в файл ложится на объект PackFile
    DWORD Open(const std::string& filename, DWORD modeopen = GENERIC_READ | GENERIC_WRITE);
    bool Close(DWORD Handle);
    bool Read(DWORD Handle, void *buf, int Size);
#ifdef HANDLE_OUT_OF_PACK_FILES
    bool Write(DWORD Handle, const void *buf, int Size);
#endif
    bool SetPos(DWORD Handle, DWORD Pos, int ftype = FILE_BEGIN);
    DWORD GetPos(DWORD Handle);
    DWORD GetSize(DWORD Handle);
    DWORD GetHandle(DWORD Handle);

    //***** Общесистемные процедуры работы с файлами ****

#ifdef HANDLE_OUT_OF_PACK_FILES
    bool PathExists(const std::string& path);
    bool FileExists(const std::string& path);
#else
    bool PathExists(const std::string& path) { return m_RootFolder->PathExists(path); }
    bool FileExists(const std::string& path) { return m_RootFolder->FileExists(path); }
#endif
    DWORD CompressedFileSize(const std::string& filename) {
        if (m_Handle == 0xFFFFFFFF)
            return 0xFFFFFFFF;
        if (m_RootFolder)
            return m_RootFolder->CompressedFileSize(m_Handle, filename);
        return 0xFFFFFFFF;
    }

    DWORD DecompressedFileSize(const std::string& filename) {
        if (m_Handle == 0xFFFFFFFF)
            return 0xFFFFFFFF;
        if (m_RootFolder)
            return m_RootFolder->DecompressedFileSize(m_Handle, filename);
        return 0xFFFFFFFF;
    }

    //***** Процедуры работы с пакетным файлом **********
    // bool    AddFile(const std::string& name);
    // bool    AddFile(const std::string& name,EFileType ftype);
    // bool    AddFiles(CBlockPar &block, std::string& log);
    // bool    AddFiles(CBlockPar &block);
    // bool    AddPath(const std::string& name);
    // bool    AddPath(const std::string& name,EFileType ftype);
    // bool    DeleteFile(const std::string& name);
    // bool    CreateFolder(const std::string& name);

    // bool    Compress(int *PInt,std::string& PStr);
    // bool    Compress(const std::string& name,int *PInt,std::string& PStr);
    // bool    Pack(int *PInt,std::string& PStr);
    // bool    Unpack(int *PInt,std::string& PStr);
    // bool    Compress(void);
    // bool    Compress(const std::string& name);
    // bool    Pack(void);
    // bool    Unpack(void);
    // bool    UnpackFile(const std::string& souname);
    // bool    UnpackFile(const std::string& souname,const std::string& desname);

    void ListFileNames(FILENAME_CALLBACK_FUNC Func) {
        if (m_Handle == 0xFFFFFFFF)
            return;
        if (m_RootFolder) {
            m_RootFolder->ListFileNames(Func);
        }
    }

    //*** Функции, используемые при установке типа файла *****
    // void    SetFileType(const std::string& name, EFileType NType)
    //{
    //    if (m_RootFolder == NULL) return;
    //    if (m_Handle == 0xFFFFFFFF) return;
    //    m_RootFolder->SetFileType(name, NType);
    //}
    // void    SetFolderType(const std::string& name, EFileType NType)
    //{
    //    if (m_RootFolder == NULL) return;
    //    if (m_Handle == 0xFFFFFFFF) return;
    //    m_RootFolder->SetFolderType(name, NType);
    //}

    int FindFirst(const std::string& path, DWORD Attr, SSearchRec &S);
    int FindNext(SSearchRec &S) {
        if (m_RootFolder == NULL)
            return 1;
        if (S.Folder == NULL)
            return 1;
        return S.Folder->FindNext(S);
    }
    void FindClose(SSearchRec &S) {
        S.Folder = NULL;
        S.Path = "";
        S.Name = "";
    }
};

class CPackCollection : public CMain {
public:
    CHeap *m_Heap;
    std::vector<CPackFile*> m_PackFiles;

public:
    CPackCollection(CHeap *heap) : m_Heap(heap) {}
    ~CPackCollection() { Clear(); };

    //******** Процедуры работы со списком пакетных файлом ********//
    void Clear(void);
    void AddPacketFile(const wchar *filename);
    void DelPacketFile(const wchar *filename);
    //******** Процедуры для работы с пакетными файлами ***********//
    bool OpenPacketFiles(void);
    bool ClosePacketFiles(void);
    bool OpenPacketFilesEx(void);
    bool ClosePacketFilesEx(void);
    CPackFile *GetPacketFile(int i) { return m_PackFiles[i]; };
    //******** Процедуры для работы файлами ***********//
    bool FileExists(const std::string& name);
    bool PathExists(const std::string& path);
    //******* работа с виртуальными номерами объектов CPackFile
    DWORD Open(const std::string& name, DWORD modeopen = GENERIC_READ);
    bool Close(DWORD Handle);
    bool Read(DWORD Handle, void *Buf, int Size);
    // bool        Write(DWORD Handle, const void *Buf, int Size);
    bool SetPos(DWORD Handle, DWORD Pos, int ftype = FILE_BEGIN);
    DWORD GetPos(DWORD Handle);
    DWORD GetSize(DWORD Handle);
    DWORD GetHandle(DWORD Handle);
    //******* работа с извлекаемыми файлами *********************
    // bool        UnpackFile(const std::string& souname) {UnpackFile(souname,souname);};
    // bool        UnpackFile(const std::string& souname,const std::string& desname);
};

}  // namespace Base
#endif
