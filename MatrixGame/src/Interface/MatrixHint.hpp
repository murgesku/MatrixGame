// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#define HINT_Z 0.0000f

#include "MatrixGame.h"
#include "Texture.hpp"
#include "CBitmap.hpp"

struct SHintBitmap {
    CBitmap bmp;
    std::wstring name;
};

enum EHintElementModificator {
    HEM_TAB_5 = 5,
    HEM_TAB_10 = 10,
    HEM_TAB_15 = 15,
    HEM_TAB_20 = 20,
    HEM_TAB_25 = 25,
    HEM_TAB_30 = 30,
    HEM_TAB_35 = 35,
    HEM_TAB_40 = 40,
    HEM_TAB_LARGEST = 1000,  // last value

    HEM_CENTER_RIGHT_5 = 1005,
    HEM_CENTER_RIGHT_30 = 1030,
    HEM_CENTER_RIGHT_LARGEST = 2000,  // last value

    HEM_CENTER_LEFT_5 = 2005,
    HEM_CENTER_LEFT_30 = 2030,
    HEM_CENTER_LEFT_LARGEST = 3000,  // last value

    HEM_COPY,  // next bitmap will be copied instead of merged
    HEM_BITMAP,
    HEM_CENTER,
    HEM_LAST_ON_LINE,
    HEM_COORD,  // next bitmap with specified coords
    HEM_DOWN,   // just down
    HEM_RIGHT,  // just right
    HEM_LAST,

    EHintElementModificator_FORCE_DWORD = 0x7FFFFFFF
};

struct SHintElement {
    union {
        CBitmap *bmp;
        struct {
            signed short x, y;
        };
    };
    EHintElementModificator hem;
};

class CMatrixHint : public CMain
{
    std::wstring m_SoundIn;
    std::wstring m_SoundOut;

    DWORD m_Flags;
    CTextureManaged *m_Texture;

    std::vector<CPoint> m_CopyPos;

    CMatrixHint(CTextureManaged *tex, int w, int h, const std::wstring &si, const std::wstring &so);
    ~CMatrixHint();

public:
    int m_PosX;
    int m_PosY;
    int m_Width;
    int m_Height;

    static void StaticInit(void) {
    }

    static void PreloadBitmaps(void);

    static CMatrixHint *Build(int border, const std::wstring &soundin, const std::wstring &soundout, SHintElement *elems,
                              CRect *otstup = nullptr);
    static CMatrixHint *Build(const std::wstring &templatename, const wchar *baserepl = nullptr);
    static CMatrixHint *Build(const std::wstring &str, CBlockPar *repl, const wchar *baserepl = nullptr);
    void Release(void) {
        SetVisible(false);
        HDelete(CMatrixHint, this, g_MatrixHeap);
    }

    void SoundIn(void) const;
    void SoundOut(void) const;

    bool IsVisible(void) const { return m_Flags != 0; }
    void SetVisible(bool flag) {
        if ((m_Flags != 0) != flag) {
            if (flag) {
                SoundIn();
            }
            else {
                SoundOut();
            }
        }
        m_Flags = flag;
    }

    void Show(int x, int y) {
        m_PosX = x;
        m_PosY = y;
        SetVisible(true);
    }

    int GetCopyPosCnt(void) const { return m_CopyPos.size(); }
    const CPoint &GetCopyPos(int i) const { return m_CopyPos[i]; }

    void DrawNow(void);
    static void DrawAll(void);
    static void ClearAll(void);
};