// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "CMain.hpp"
#include "CHeap.hpp"
#include "CBuf.hpp"

using namespace Base; // TODO: remove

class CCache;

enum CacheClass { cc_Unknown = 0, cc_Texture, cc_TextureManaged, cc_VO };

class CCacheData : public Base::CMain {
public:
    CCacheData *m_Prev;
    CCacheData *m_Next;
    CCache *m_Cache;
#ifdef _DEBUG
    CCacheData *d_Prev;
    CCacheData *d_Next;
    const char *d_file;
    int d_line;
#endif

    CacheClass m_Type;
    std::wstring m_Name;

    int m_Ref;  // Кол-во ссылок на эти данные. (Для временных данных)

    static bool m_dip;  // cache del in progress
public:
    static void StaticInit(void);

    CCacheData(void);
    virtual ~CCacheData(){};

    void RefInc(void) { m_Ref++; }
    void RefDec(void) {
        m_Ref--;
        ASSERT(m_Ref >= 0);
    }
    int Ref(void) { return m_Ref; }
    bool RefDecUnload(void) {
        m_Ref--;
        if (m_Ref <= 0) {
            Unload();
            return true;
        }
        return false;
    }

    void Prepare(void);

    void LoadFromFile(CBuf &buf, const wchar *exts = NULL);

    virtual bool IsLoaded(void) = 0;
    virtual void Unload(void) = 0;
    virtual void Load(void) = 0;
};

class CCache : public CMain {
public:
    CCacheData *m_First;
    CCacheData *m_Last;

public:
    CCache(void);
    ~CCache();

#ifdef _DEBUG
    static void Dump(void);
#endif

    void Add(CCacheData *cd);
    void Delete(CCacheData *cd);
    void Up(CCacheData *cd);

    void PreLoad(void);

    CCacheData *Find(CacheClass cc, const wchar *name);
    CCacheData *Get(CacheClass cc, const wchar *name);
#ifdef _DEBUG
    static CCacheData *Create(CacheClass cc, const char *file, int line);
#else
    static CCacheData *Create(CacheClass cc);
#endif
    static void Destroy(CCacheData *cd);

    void Clear(void);
};

#ifdef _DEBUG
#define CACHE_CREATE_VO()             (CVectorObject *)CCache::Create(cc_VO, __FILE__, __LINE__)
#define CACHE_CREATE_TEXTURE()        (CTexture *)CCache::Create(cc_Texture, __FILE__, __LINE__)
#define CACHE_CREATE_TEXTUREMANAGED() (CTextureManaged *)CCache::Create(cc_TextureManaged, __FILE__, __LINE__)
#else
#define CACHE_CREATE_VO()             (CVectorObject *)CCache::Create(cc_VO)
#define CACHE_CREATE_TEXTURE()        (CTexture *)CCache::Create(cc_Texture)
#define CACHE_CREATE_TEXTUREMANAGED() (CTextureManaged *)CCache::Create(cc_TextureManaged)
#endif

extern CCache *g_Cache;
extern CHeap *g_CacheHeap;
extern const wchar *CacheExtsTex;

void CacheInit(void);
void CacheDeinit(void);

// bool CacheFileGet(std::wstring & outname,const wchar * mname,const wchar * exts=NULL,bool withpar=false);
void CacheReplaceFileExt(std::wstring &outname, const wchar *mname, const wchar *ext = NULL);
void CacheReplaceFileNameAndExt(std::wstring &outname, const wchar *mname, const wchar *replname);
