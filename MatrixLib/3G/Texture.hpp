// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef TEXTURE_INCLUDE_HPP
#define TEXTURE_INCLUDE_HPP

#include "3g.hpp"
#include "Cache.hpp"
#include "CBitmap.hpp"
#include "CBlockPar.hpp"

#include "d3d9.h"
#include "d3dx9tex.h"

//#define TF_NORMAL	SETBIT(0)		// У текстуры есть пиксели без alpha и прозрачности
//#define TF_TRANS	SETBIT(1)		// У текстуры есть прозрачные пиксели
//#define TF_ALPHA	SETBIT(2)		// У текстуры есть alpha пиксели
#define TF_NOMIPMAP SETBIT(3)  // Если этот флаг стоит, то мипмэпы не будут генериться

#define TF_ALPHATEST  SETBIT(4)  // При наложении текстуры включить Alpha Test
#define TF_ALPHABLEND SETBIT(5)  // При наложении текстуры включить Alpha Blend

#define TF_LOST       SETBIT(6)  // texture lost. m_Tex is SM texture
#define TF_COMPRESSED SETBIT(7)

#define OOM_TEXTRUE_HIT_COUNTER 50

#define USE_DX_MANAGED_TEXTURES

enum ETexSize {
    TEXSIZE_16 = 4,
    TEXSIZE_32,
    TEXSIZE_64,
    TEXSIZE_128,
    TEXSIZE_256,
    TEXSIZE_512,
    TEXSIZE_1024,
    TEXSIZE_2048
};

inline int ConvertTexSize(ETexSize ts) {
    return (1 << ts);
}

class CBaseTexture : public CCacheData {
protected:
    CBaseTexture(void) : CCacheData() {
        LIST_ADD(this, m_TexturesFirst, m_TexturesLast, m_TexturesPrev, m_TexturesNext);
    }

    static CBaseTexture *m_TexturesFirst;
    static CBaseTexture *m_TexturesLast;
    CBaseTexture *m_TexturesPrev;
    CBaseTexture *m_TexturesNext;

    LPDIRECT3DTEXTURE9 m_Tex;
    DWORD m_Flags;  // TF_*
    CPoint m_Size;

public:
    static void StaticInit(void) {
        m_TexturesFirst = NULL;
        m_TexturesLast = NULL;
    }

    virtual ~CBaseTexture() { LIST_DEL(this, m_TexturesFirst, m_TexturesLast, m_TexturesPrev, m_TexturesNext); };

    LPDIRECT3DTEXTURE9 DX(void) { return m_Tex; }
    LPDIRECT3DTEXTURE9 LoadTextureFromFile(bool to16bit, D3DPOOL pool = D3DPOOL_DEFAULT);
    void ParseFlags(const ParamParser& name);

    bool IsTextureManaged(void) const { return m_Type == cc_TextureManaged; };
    void MipmapOff(void) { SETFLAG(m_Flags, TF_NOMIPMAP); }

    DWORD Flags(void) {
        if (!m_Tex)
            Load();
        return m_Flags;
    }
    const CPoint &GetSize(void) const { return m_Size; }
    int GetSizeX(void) const { return m_Size.x; }
    int GetSizeY(void) const { return m_Size.y; }

    void Set(LPDIRECT3DTEXTURE9 tex, int flags = 0) {
        Unload();
        m_Tex = tex;
        m_Flags = flags;
    }

    virtual bool IsLoaded(void) { return m_Tex != NULL; }
    virtual void Unload(void) = 0;
    virtual void Load(void) = 0;

    static void OnLostDevice(void);
    static void OnResetDevice(void);
};

class CTexture : public CBaseTexture {
    union {
        int m_OOM_counter;
        DWORD m_Usage;
    };

public:
    CTexture(void) : CBaseTexture() {
        DTRACE();

        m_Type = cc_Texture;

        m_Tex = NULL;
        m_Flags = 0;
        m_OOM_counter = 0;
    }

    virtual ~CTexture() {
        DTRACE();
        Unload();
    }

    void Preload(void) {
        if (FLAG(m_Flags, TF_LOST))
            return;
        if (m_OOM_counter > 0) {
            --m_OOM_counter;
            return;
        }
        if (!m_Tex) {
            Load();
        }
    };
    LPDIRECT3DTEXTURE9 Tex(void) {
        if (m_OOM_counter > 0 || FLAG(m_Flags, TF_LOST))
            return NULL;
        ASSERT(m_Type == cc_Texture);
        if (!m_Tex) {
#ifdef _DEBUG
            // ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS));
            if (FLAG(g_Flags, GFLAG_RENDERINPROGRESS))
                debugbreak();
#endif
            Load();
        }
        return m_Tex;
    }

    void MakeSolidTexture(int sx, int sy, DWORD color);
    void LoadFromBitmap(CBitmap &bm);

    void OnLostDevice(void);
    void OnResetDevice(void);

    virtual void Unload(void);
    virtual void Load(void);
};

#ifndef USE_DX_MANAGED_TEXTURES
void UnloadTextureManaged(DWORD user);
#endif

class CTextureManaged : public CBaseTexture {
#ifndef USE_DX_MANAGED_TEXTURES
    D3DFORMAT m_Format;
    LPDIRECT3DTEXTURE9 m_TexFrom;  // system memory texture
    int m_Levelcnt;
    SRemindCore m_RemindCore;
    int m_OOM_counter;
#endif

    void LoadFromBitmap(int level, const CBitmap &bm, bool convert_to_16bit);  // valid only for 24 and 32 bpp images
public:
#pragma warning(disable : 4355)
    CTextureManaged(void)
      : CBaseTexture()
#ifndef USE_DX_MANAGED_TEXTURES
        ,
        m_TexFrom(NULL), m_RemindCore(UnloadTextureManaged, (DWORD)this)
#endif
    {
        m_Type = cc_TextureManaged;
        m_Tex = NULL;
        m_Flags = 0;
#ifndef USE_DX_MANAGED_TEXTURES
        m_OOM_counter = 0;
#endif
    }
#pragma warning(default : 4355)

#ifndef USE_DX_MANAGED_TEXTURES
    static CTextureManaged *Get(const wchar *name, bool c16);
    LPDIRECT3DTEXTURE9 TexFrom(void) { return m_TexFrom; }
#endif

    virtual ~CTextureManaged() {
        DTRACE();
#ifndef USE_DX_MANAGED_TEXTURES
        if (m_TexFrom) {
            m_TexFrom->Release();
        }
#else
        if (m_Tex) {
            m_Tex->Release();
        }
#endif
        Unload();
    };

    LPDIRECT3DTEXTURE9 Tex(void) {
#ifndef USE_DX_MANAGED_TEXTURES
        if (m_OOM_counter > 0)
            return NULL;
#endif
        if (!m_Tex) {
            Load();
        }
#ifndef USE_DX_MANAGED_TEXTURES
        m_RemindCore.Use(5000);
#endif

        return m_Tex;
    }

    void Preload(void) {
#ifndef USE_DX_MANAGED_TEXTURES

        if (m_OOM_counter > 0) {
            --m_OOM_counter;
            return;
        }
        if (!m_Tex) {
            Load();
        }
        m_RemindCore.Use(5000);
#else
        if (!m_Tex) {
            Load();
        }
        m_Tex->PreLoad();
#endif
    }

    HRESULT CreateLock(D3DFORMAT fmt, int sx, int sy, int levels, D3DLOCKED_RECT &lr) {
#ifdef USE_DX_MANAGED_TEXTURES
        ASSERT(m_Tex == NULL);

        HRESULT res = g_D3DD->CreateTexture(sx, sy, levels, 0, fmt, D3DPOOL_MANAGED, &m_Tex, 0);

        if (res != D3D_OK)
            return res;

        D3DSURFACE_DESC desc;
        m_Tex->GetLevelDesc(0, &desc);
        m_Size.x = desc.Width;
        m_Size.y = desc.Height;

#else
        ASSERT(m_TexFrom == NULL);

        HRESULT res = g_D3DD->CreateTexture(sx, sy, levels, 0, fmt, D3DPOOL_SYSTEMMEM, &m_TexFrom, 0);
        if (res != D3D_OK)
            return res;

        D3DSURFACE_DESC desc;
        m_TexFrom->GetLevelDesc(0, &desc);
        m_Format = desc.Format;
        m_Levelcnt = m_TexFrom->GetLevelCount();
        m_Size.x = desc.Width;
        m_Size.y = desc.Height;

#endif
        LockRect(lr, 0);
        return res;
    }

    void Init(bool to16);

    DWORD GetPixelColor(int x, int y);

    void Ramka(int x, int y, int w, int h, DWORD c);

    void LockRect(D3DLOCKED_RECT &lr, DWORD Flags) {
#ifdef _DEBUG
        if (FLAG(g_Flags, GFLAG_RENDERINPROGRESS))
            debugbreak();
            // ASSERT(!FLAG(g_Flags, GFLAG_RENDERINPROGRESS));
#endif
#ifndef USE_DX_MANAGED_TEXTURES
                    ASSERT_DX(TexFrom()->LockRect(0, &lr, NULL, Flags));
#else
        ASSERT_DX(Tex()->LockRect(0, &lr, NULL, Flags));
#endif
    }
    void UnlockRect(void) {
#ifndef USE_DX_MANAGED_TEXTURES
        ASSERT_DX(TexFrom()->UnlockRect(0));
#else
        ASSERT_DX(Tex()->UnlockRect(0));
#endif
    }

    void LoadFromBitmap(const CBitmap &bm, bool convert_to_16bit, int levelcnt = 0);
    void LoadFromBitmap(const CBitmap &bm, D3DFORMAT fmt, int levels = 0);
    // void LoadFromBitmapAsIs(const CBitmap & bm);

    virtual void Unload(void);
    virtual void Load(void);
};

#ifndef USE_DX_MANAGED_TEXTURES
inline CTextureManaged *CTextureManaged::Get(const wchar *name, bool c16) {
    CTextureManaged *tex = CACHE_CREATE_TEXTUREMANAGED();
    CBitmap bm(g_CacheHeap);
    std::wstring tn(name);
    CFile::FileExist(tn, tn.Get(), CacheExtsTex);
    bm.LoadFromPNG(tn.Get());
    tex->LoadFromBitmap(bm, c16, 1);
    return tex;
}
#endif

#endif