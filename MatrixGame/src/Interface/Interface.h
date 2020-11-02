// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#define MAX_STATES 5
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

enum IFaceElementState
{
	IFACE_NORMAL = 0,
	IFACE_FOCUSED = 1,
	IFACE_PRESSED = 2,
	IFACE_DISABLED = 3,
	IFACE_PRESSED_UNFOCUSED = 4,


    IFaceElementState_FORCE_DWORD = 0x7FFFFFFF
};

enum EIFaceLabel
{
    IFACE_STATIC_LABEL = 0,
    IFACE_DYNAMIC_LABEL = 1,
    IFACE_STATE_STATIC_LABEL = 2,

    EIFaceLabel_FORCE_DWORD = 0x7FFFFFFF
}; 

enum EActions
{
    ON_PRESS = 0,
    ON_UN_PRESS = 1,
    ON_FOCUS = 2,
    ON_UN_FOCUS = 3,

    EActions_FORCE_DWORD = 0x7FFFFFFF
};

struct SAction {
	void * m_class;
	void * m_function;

    SAction(): m_class(NULL), m_function(NULL) {}
};

struct SStateImages {
	CTextureManaged*        pImage;
	SVert_V4_UV             m_Geom[4];
	float                   xTexPos;
	float                   yTexPos;
	float                   TexWidth;
	float                   TexHeight;


    int                     m_x;
    int                     m_y;
    int                     m_boundX;
    int                     m_boundY;
    int                     m_xAlign;
    int                     m_yAlign;
    int                     m_Perenos;
    int                     m_SmeX;
    int                     m_SmeY;
    CRect                   m_ClipRect;
    CWStr                   m_Caption;
    CWStr                   m_Font;
    DWORD                   m_Color;

	DWORD                   Set; // This is boolean var. Used DWORD becouse align!


    void SetStateText(bool copy);
    void SetStateLabelParams(int x, int y, int bound_x, int bound_y, int xAlign, int yAlign, int perenos, int smeX, int smeY, CRect clipRect, CWStr t, CWStr font, DWORD color);
	
    SStateImages():m_Caption(g_MatrixHeap),m_Font(g_MatrixHeap) {
		xTexPos = yTexPos = TexWidth = TexHeight = 0;
        m_x = m_y = m_boundX = m_boundY = m_xAlign = m_yAlign = m_Perenos = m_SmeX = m_SmeY = 0;
		m_Color = 0;
        pImage = NULL;
		ZeroMemory(m_Geom, sizeof(m_Geom));
        ZeroMemory(&m_ClipRect, sizeof(CRect));
		Set = false;
	}
};

/*
void* g_asm_iface_tmp;

#define FSET(dest, pObj, src) \
	dest.m_class=(void *)pObj;\
	g_asm_iface_tmp=dest.m_function;\
	__asm {mov eax,src}\
	__asm {mov g_asm_iface_tmp,eax}\
	dest.m_function = g_asm_iface_tmp;
*/

#define FSET(act, pBut, cl, fn, pObj, src) \
	cl=(void *)pObj;\
	__asm {mov eax,offset src}\
	__asm {mov [fn], eax}\
	pBut->m_Actions[act].m_class = cl;\
	pBut->m_Actions[act].m_function = fn;


#define FCALL(a,from) \
	__asm push	from \
	__asm mov	eax,(a)->m_class \
	__asm push	eax \
	__asm mov	eax,(a)->m_function \
	__asm call	eax

#define FCALLFROMCLASS(a) \
	__asm push	this\
	__asm mov	eax,dword ptr a\
	__asm add	eax,dword ptr this\
	__asm push	[eax]\
	__asm add	eax,4\
	__asm mov	eax,[eax]\
	__asm call	eax

#define FCALLFROMCLASS2(a) \
	__asm push	this\
	__asm mov	eax,dword ptr a\
	__asm push	[eax]\
	__asm add	eax,4\
	__asm mov	eax,[eax]\
	__asm call	eax


/////////////////////////////////////////////////
extern IDirect3DDevice9 * g_D3DD;
extern CHeap * g_MatrixHeap;
extern CCache * g_Cache;
////////////////////////////////////////////////
