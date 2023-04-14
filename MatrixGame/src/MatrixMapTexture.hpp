// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_MAP_TEXTURE
#define MATRIX_MAP_TEXTURE

#include "Texture.hpp"

class CBottomTextureUnion : public CMain {
    CTextureManaged *m_Texture;

    static CBottomTextureUnion *m_Textures;
    static int m_TexturesCount;

public:
    // DWORD   m_ID;

    CBottomTextureUnion(void) : m_Texture(NULL) {}
    ~CBottomTextureUnion();

    static void StaticInit(void) {
        m_Textures = NULL;
        m_TexturesCount = 0;
    }

    void MakeFromBitmap(const CBitmap &bm);

    void Unload(void) {
        DTRACE();
        m_Texture->Unload();
        // m_ID = REMINDER_EMPTY;
    }

    void Prepare(void) { m_Texture->Preload(); }
    CTextureManaged *GetTexture(void) { return m_Texture; };

    static void Init(int n);
    static void Clear(void);
    static CBottomTextureUnion &Get(int i) { return m_Textures[i]; }
};

#endif