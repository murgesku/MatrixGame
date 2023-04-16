// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_SKIN_MANAGER
#define MATRIX_SKIN_MANAGER

#include "VectorObject.hpp"

enum GSParam {
    GSP_SIDE,
    GSP_ORDINAL,
    GSP_SIDE_NOALPHA,

    GSP_COUNT
};

struct SMatrixSkin : public SSkin {
    CTextureManaged *m_Tex;  // if NULL, default texturing
    CTextureManaged *m_TexGloss;
    CTextureManaged *m_TexMask;
    CTextureManaged *m_TexBack;
    float m_tu, m_tv, m_dtu, m_dtv;

    bool operator==(const SMatrixSkin &sk) const {
        return m_Tex == sk.m_Tex && m_TexGloss == sk.m_TexGloss && m_TexMask == sk.m_TexMask &&
               m_TexBack == sk.m_TexBack && m_dtu == sk.m_dtu && m_dtv == sk.m_dtv;  // &&
        // m_SetupClear == sk.m_SetupClear &&
        // m_SetupStages == sk.m_SetupStages &&
        // m_SetupTex == sk.m_SetupTex &&
        // m_SetupTexShadow == sk.m_SetupTexShadow;
    }

    void Prepare(GSParam gsp);

    void Prepare6Side(void);
    void Prepare6SideNA(void);
    void Prepare6Ordinal(void);

    void Prepare4Side(void);
    void Prepare4SideNA(void);
    void Prepare4Ordinal(void);

    void Prepare3Side(void);
    void Prepare3SideNA(void);
    void Prepare3Ordinal(void);

    void Prepare2Side(void);
    void Prepare2SideNA(void);
    void Prepare2Ordinal(void);
};

typedef SMatrixSkin *PSMatrixSkin;
typedef PSMatrixSkin *PPSMatrixSkin;

class CSkinManager : public CMain {
    static PPSMatrixSkin m_Skins[GSP_COUNT];
    static int m_SkinsCount[GSP_COUNT];

public:
    static void StaticInit(void) {
        m_Skins[GSP_SIDE] = NULL;
        m_SkinsCount[GSP_SIDE] = 0;
        m_Skins[GSP_SIDE_NOALPHA] = NULL;
        m_SkinsCount[GSP_SIDE_NOALPHA] = 0;
        m_Skins[GSP_ORDINAL] = NULL;
        m_SkinsCount[GSP_ORDINAL] = 0;
    }

    static const SSkin *GetSkin(const wchar *tex, DWORD param);

    static void Clear(void);
    static void Takt(float cms);
};

#endif