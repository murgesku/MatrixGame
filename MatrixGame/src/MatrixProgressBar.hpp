// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef PROGRESS_BAR_INCLUDE
#define PROGRESS_BAR_INCLUDE

#include "Texture.hpp"
#include "MatrixProgressBar.hpp"

#define PB_BUILDING_WIDTH 200
#define PB_ROBOT_WIDTH    70
#define PB_CANNON_WIDTH   100
#define PB_FLYER_WIDTH    100
#define PB_SPECIAL_WIDTH  50
//#define PB_BUILDING_HEIGHT  8

#define PB_COLOR_0 0x00FF0000
#define PB_COLOR_1 0x00FFFF00
#define PB_COLOR_2 0x0000FF00

#define PB_Z 0

struct SPBPos {
    DWORD present;
    float m_X;
    float m_Y;
    float m_Width;
};

enum EPBCoord {
    PBC_ORIGINAL = 0,
    PBC_CLONE1,
    PBC_CLONE2,

};

class CMatrixProgressBar : public CMain {
    static CMatrixProgressBar *m_First;
    static CMatrixProgressBar *m_Last;
    static CMatrixProgressBar *m_FirstClones;
    static CMatrixProgressBar *m_LastClones;
    static CTextureManaged *m_Tex;

    CMatrixProgressBar *m_Prev;
    CMatrixProgressBar *m_Next;

    SPBPos *m_Coord;
    int m_CoordCount;

    float m_Pos;

#ifdef _DEBUG
    int counter;
#endif

    void DrawClones(bool pbd);
    void Draw(bool pbd);
    void Draw(SPBPos *pos, bool pbd);

public:
    CMatrixProgressBar(void);
    ~CMatrixProgressBar();

    void Modify(float x, float y, float width, /* float height, */ float pos) {
        Modify(x, y);
        m_Coord->m_Width = (float)floor(width); /* m_Height = floor(height); */
        m_Pos = pos;
    };
    void Modify(float x, float y, float pos) {
        Modify(x, y);
        m_Pos = pos;
    };
    void Modify(float x, float y) {
        m_Coord->m_X = (float)floor(x) + 0.5f;
        m_Coord->m_Y = (float)floor(y) + 0.5f;
    };
    void Modify(float pos) { m_Pos = pos; };

    void CreateClone(EPBCoord pbc, float x, float y, float width);
    void KillClone(EPBCoord pbc);
    bool ClonePresent(EPBCoord pbc) const { return (m_CoordCount > pbc) && (m_Coord[pbc].present != 0); };

    static void BeforeDrawAll(void) { m_Tex->Preload(); }

    static void DrawAll(void);
    static void DrawAllClones(void);

    static void StaticInit(void) {
        m_First = NULL;
        m_Last = NULL;
        m_FirstClones = NULL;
        m_LastClones = NULL;
        m_Tex = NULL;
    }
};

#endif