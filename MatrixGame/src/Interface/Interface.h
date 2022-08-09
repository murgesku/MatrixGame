// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#define MAX_STATES  5
#define MAX_ACTIONS 4

#include "../MatrixInstantDraw.hpp"

enum IFaceElementType {
    IFACE_UNDEF = -1,
    IFACE_STATIC = 0,
    IFACE_ANIMATION = 1,
    IFACE_PUSH_BUTTON = 2,
    IFACE_ANIMATED_BUTTON = 3,
    IFACE_CHECK_BUTTON = 4,
    IFACE_DYNAMIC_STATIC = 5,
    IFACE_IMAGE = 6,
    IFACE_CHECK_BUTTON_SPECIAL = 7,
    IFACE_CHECK_PUSH_BUTTON = 8,

    IFaceElementType_FORCE_DWORD = 0x7FFFFFFF
};

enum IFaceElementState {
    IFACE_NORMAL = 0,
    IFACE_FOCUSED = 1,
    IFACE_PRESSED = 2,
    IFACE_DISABLED = 3,
    IFACE_PRESSED_UNFOCUSED = 4,

    IFaceElementState_FORCE_DWORD = 0x7FFFFFFF
};

enum EIFaceLabel {
    IFACE_STATIC_LABEL = 0,
    IFACE_DYNAMIC_LABEL = 1,
    IFACE_STATE_STATIC_LABEL = 2,

    EIFaceLabel_FORCE_DWORD = 0x7FFFFFFF
};

enum EActions {
    ON_PRESS = 0,
    ON_UN_PRESS = 1,
    ON_FOCUS = 2,
    ON_UN_FOCUS = 3,

    EActions_FORCE_DWORD = 0x7FFFFFFF
};

struct SStateImages {
    CTextureManaged *pImage;
    SVert_V4_UV m_Geom[4];
    float xTexPos;
    float yTexPos;
    float TexWidth;
    float TexHeight;

    int m_x;
    int m_y;
    int m_boundX;
    int m_boundY;
    int m_xAlign;
    int m_yAlign;
    int m_Perenos;
    int m_SmeX;
    int m_SmeY;
    CRect m_ClipRect;
    CWStr m_Caption;
    CWStr m_Font;
    DWORD m_Color;

    DWORD Set;  // This is boolean var. Used DWORD becouse align!

    void SetStateText(bool copy);
    void SetStateLabelParams(int x, int y, int bound_x, int bound_y, int xAlign, int yAlign, int perenos, int smeX,
                             int smeY, CRect clipRect, CWStr t, CWStr font, DWORD color);

    SStateImages() : m_Caption(g_MatrixHeap), m_Font(g_MatrixHeap) {
        xTexPos = yTexPos = TexWidth = TexHeight = 0;
        m_x = m_y = m_boundX = m_boundY = m_xAlign = m_yAlign = m_Perenos = m_SmeX = m_SmeY = 0;
        m_Color = 0;
        pImage = NULL;
        ZeroMemory(m_Geom, sizeof(m_Geom));
        ZeroMemory(&m_ClipRect, sizeof(CRect));
        Set = false;
    }
};

/////////////////////////////////////////////////
extern IDirect3DDevice9 *g_D3DD;
extern CHeap *g_MatrixHeap;
extern CCache *g_Cache;
////////////////////////////////////////////////
