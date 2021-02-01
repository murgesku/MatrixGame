// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef HPP_3G_INCLUDE
#define HPP_3G_INCLUDE

#include "Cache.hpp"
#include "Math3D.hpp"
#include "Form.hpp"
#include "Helper.hpp"


#define GFLAG_APPACTIVE             SETBIT(0)
//#define GFLAG_FORMACCESS            SETBIT(1)
#define GFLAG_APPCLOSE              SETBIT(2)
#define GFLAG_FULLSCREEN            SETBIT(3)
#define GFLAG_STENCILAVAILABLE      SETBIT(4)
#define GFLAG_GAMMA                 SETBIT(5)
#define GFLAG_EXITLOOP      		SETBIT(7)
#define GFLAG_PRESENT_REQUIRED      SETBIT(8)


#ifdef _DEBUG
#define GFLAG_EXTRAFREERES          SETBIT(29)
#define GFLAG_TAKTINPROGRESS 	    SETBIT(30)
#define GFLAG_RENDERINPROGRESS 	    SETBIT(31)
#endif

extern HINSTANCE g_HInst;
extern IDirect3D9* g_D3D;
extern IDirect3DDevice9* g_D3DD;
extern D3DCAPS9 g_D3DDCaps;
extern HWND g_Wnd;
extern int g_ScreenX, g_ScreenY;
extern float g_CamFieldOfView;
extern int g_DrawFPS;
extern int g_MaxFPS;
extern float g_MaxViewDistance;
extern int g_MaxObjectsPerScreen;
extern int g_MaxEffectsCount;
extern int g_ShadowsDrawDistance;
extern float g_ThinFogDrawDistance;
extern float g_DenseFogDrawDistance;
extern int g_AvailableTexMem;
//extern CReminder* g_Reminder;
extern D3DPRESENT_PARAMETERS g_D3Dpp; 


//global config flags
extern DWORD g_Flags;


#ifdef _DEBUG
    __inline int D3DDRef(void)
    {
        int ref = g_D3DD->AddRef();
        ref = g_D3DD->Release();
        return ref;
    }
#endif



#ifdef DO_SMART_COLOROPS

struct STextureStageOp
{
    D3DTEXTUREOP    op;
    DWORD           p1;
    DWORD           p2;
};

extern STextureStageOp     g_ColorOp[];
extern STextureStageOp     g_AlphaOp[];

#ifdef _DEBUG
//#define CHECKTSS(n,t,v) {DWORD vv; g_D3DD->GetTextureStageState(n,t,&vv); if (v!=vv) {_asm int 3}}
#define CHECKTSS(n,t,v) {}
#else
#define CHECKTSS(n,t,v) {}
#endif

__forceinline void SetColorOp(int stage, D3DTEXTUREOP op, DWORD p1, DWORD p2)
{
    if (g_ColorOp[stage].op != op) { g_D3DD->SetTextureStageState(stage, D3DTSS_COLOROP, op); g_ColorOp[stage].op = op;}  else CHECKTSS(stage, D3DTSS_COLOROP, op);
    if (g_ColorOp[stage].p1 != p1) { g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG1, p1); g_ColorOp[stage].p1 = p1;} else CHECKTSS(stage, D3DTSS_COLORARG1, p1);
    if (g_ColorOp[stage].p2 != p2) { g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG2, p2); g_ColorOp[stage].p2 = p2;} else CHECKTSS(stage, D3DTSS_COLORARG2, p2);
}
__forceinline void SetColorOpAnyOrder(int stage, D3DTEXTUREOP op, DWORD p1, DWORD p2)
{
    if (g_ColorOp[stage].op != op) { g_D3DD->SetTextureStageState(stage, D3DTSS_COLOROP, op); g_ColorOp[stage].op = op;}

    if (g_ColorOp[stage].p1 == p1)
    { 
        if (g_ColorOp[stage].p2 != p2) { g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG2, p2); g_ColorOp[stage].p2 = p2;}    
    } else if (g_ColorOp[stage].p1 == p2)
    { 
        if (g_ColorOp[stage].p2 != p1) { g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG2, p1); g_ColorOp[stage].p2 = p1;}    
    } else
    {
        // p2 should be changed
        if (g_ColorOp[stage].p2 == p1)
        { 
            g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG1, p2); g_ColorOp[stage].p1 = p2;
        } else if (g_ColorOp[stage].p2 == p2)
        { 
            g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG1, p1); g_ColorOp[stage].p1 = p1;
        } else
        {
            g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG1, p1); g_ColorOp[stage].p1 = p1;
            g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG2, p2); g_ColorOp[stage].p2 = p2;
        }
    }
}
__forceinline void SetColorOpSelect(int stage, DWORD p)
{
    if (g_ColorOp[stage].op != D3DTOP_SELECTARG1) { g_D3DD->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_SELECTARG1 ); g_ColorOp[stage].op = D3DTOP_SELECTARG1 ;} else CHECKTSS(stage, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    if (g_ColorOp[stage].p1 != p) { g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG1, p); g_ColorOp[stage].p1 = p;}                                                  else CHECKTSS(stage, D3DTSS_COLORARG1, p);
}                                                                                                                                                                    
__forceinline void SetColorOpDisable(int stage)                                                                                                                              
{
    if (g_ColorOp[stage].op != D3DTOP_DISABLE) { g_D3DD->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_DISABLE ); g_ColorOp[stage].op = D3DTOP_DISABLE ;} CHECKTSS(stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
}

__forceinline void SetAlphaOp(int stage, D3DTEXTUREOP op, DWORD p1, DWORD p2)
{
    if (g_AlphaOp[stage].op != op) { g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAOP, op); g_AlphaOp[stage].op = op;}   else CHECKTSS(stage, D3DTSS_ALPHAOP, op);
    if (g_AlphaOp[stage].p1 != p1) { g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG1, p1); g_AlphaOp[stage].p1 = p1;}  else CHECKTSS(stage, D3DTSS_ALPHAARG1, p1);
    if (g_AlphaOp[stage].p2 != p2) { g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG2, p2); g_AlphaOp[stage].p2 = p2;}  else CHECKTSS(stage, D3DTSS_ALPHAARG2, p2);
}
__forceinline void SetAlphaOpAnyOrder(int stage, D3DTEXTUREOP op, DWORD p1, DWORD p2)
{
    if (g_AlphaOp[stage].op != op) { g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAOP, op); g_AlphaOp[stage].op = op;}

    if (g_AlphaOp[stage].p1 == p1)
    { 
        if (g_AlphaOp[stage].p2 != p2) { g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG2, p2); g_AlphaOp[stage].p2 = p2;}    
    } else if (g_AlphaOp[stage].p1 == p2)
    { 
        if (g_AlphaOp[stage].p2 != p1) { g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG2, p1); g_AlphaOp[stage].p2 = p1;}    
    } else
    {
        // p2 should be changed
        if (g_AlphaOp[stage].p2 == p1)
        { 
            g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG1, p2); g_AlphaOp[stage].p1 = p2;
        } else if (g_AlphaOp[stage].p2 == p2)
        { 
            g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG1, p1); g_AlphaOp[stage].p1 = p1;
        } else
        {
            g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG1, p1); g_AlphaOp[stage].p1 = p1;
            g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG2, p2); g_AlphaOp[stage].p2 = p2;
        }
    }

}
__forceinline void SetAlphaOpSelect(int stage, DWORD p)
{
    if (g_AlphaOp[stage].op != D3DTOP_SELECTARG1) { g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ); g_AlphaOp[stage].op = D3DTOP_SELECTARG1 ;}
    if (g_AlphaOp[stage].p1 != p) { g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG1, p); g_AlphaOp[stage].p1 = p;}
}
__forceinline void SetAlphaOpDisable(int stage)
{
    if (g_AlphaOp[stage].op != D3DTOP_DISABLE) { g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE ); g_AlphaOp[stage].op = D3DTOP_DISABLE ;}
}
#else
__forceinline void SetColorOp(int stage, D3DTEXTUREOP op, DWORD p1, DWORD p2) { g_D3DD->SetTextureStageState(stage, D3DTSS_COLOROP, op); g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG1, p1);  g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG2, p2); }
__forceinline void SetAlphaOp(int stage, D3DTEXTUREOP op, DWORD p1, DWORD p2) { g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAOP, op); g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG1, p1);  g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG2, p2); }
__forceinline void SetColorOpAnyOrder(int stage, D3DTEXTUREOP op, DWORD p1, DWORD p2) { SetColorOp(stage, op, p1, p2); }
__forceinline void SetAlphaOpAnyOrder(int stage, D3DTEXTUREOP op, DWORD p1, DWORD p2) { SetAlphaOp(stage, op, p1, p2); }
__forceinline void SetColorOpSelect(int stage, DWORD p){ g_D3DD->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_SELECTARG1 ); g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG1, p); g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG2, p);}
__forceinline void SetColorOpDisable(int stage) { g_D3DD->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_DISABLE );}
__forceinline void SetAlphaOpSelect(int stage, DWORD p){ g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ); g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG1, p); g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG2, p);}
__forceinline void SetAlphaOpDisable(int stage) { g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE );}
#endif 

class CExceptionD3D : public CException {
	public:
		HRESULT m_Error;
	public:
		CExceptionD3D(char * file,int line,HRESULT err):CException(file,line) { m_Error=err; }

		virtual CWStr Info(void);
};

#define FAILED_DX(x) { HRESULT hr=x; if(hr!=D3D_OK) throw new CExceptionD3D(__FILE__,__LINE__,hr); }

#ifdef ASSERT_OFF
	#define ASSERT_DX(x) x
#else
	#define ASSERT_DX(x) FAILED_DX(x)
#endif

void L3GInitAsEXE(HINSTANCE hinst, CBlockPar & bpcfg, wchar * sysname, wchar * captionname);
void L3GInitAsDLL(HINSTANCE hinst, CBlockPar & bpcfg, wchar * sysname, wchar * captionname, HWND hwnd, long FDirect3D, long FD3DDevice);
void L3GDeinit(void);
int L3GRun(void);

void S3D_Default(void);

#include "Texture.hpp"
#include "DeviceState.hpp"




// keyboard

#pragma warning (disable: 4035)
inline byte __fastcall lp2key(dword lp)
{
    _asm
    {
        mov eax,lp
        shr eax,15
        shr ah,1
        shr ah,1
        rcr al,1
        //and eax,255
    };
}
#pragma warning (default: 4035)

#define KEY_ESC         1
#define KEY_F1          59
#define KEY_F2          60
#define KEY_F3          61
#define KEY_F4          62
#define KEY_F5          63
#define KEY_F6          64
#define KEY_F7          65
#define KEY_F8          66
#define KEY_F9          67
#define KEY_F10         68
#define KEY_F11         87
#define KEY_F12         88

#define KEY_PAUSE       69

#define KEY_SLOCK       70
#define KEY_CLOCK       58
#define KEY_NLOCK       197

#define KEY_TILDA       41
#define KEY_RSLASH      53
#define KEY_LSLASH      43
#define KEY_COMMA       52
#define KEY_COLON       51
#define KEY_SEMICOLON   39
#define KEY_APOSTROF    40
#define KEY_LBRACKET    26
#define KEY_RBRACKET    27
#define KEY_MINUS       12
#define KEY_EQUAL       13

#define KEY_PADSLASH    181
#define KEY_PADSTAR     55
#define KEY_PADMINUS    74
#define KEY_PADPLUS     78
#define KEY_PADENTER    156
#define KEY_PADDEL      83
#define KEY_PAD0        82
#define KEY_PAD1        79
#define KEY_PAD2        80
#define KEY_PAD3        81
#define KEY_PAD4        75
#define KEY_PAD5        76
#define KEY_PAD6        77
#define KEY_PAD7        71
#define KEY_PAD8        72
#define KEY_PAD9        73

#define KEY_1           2
#define KEY_2           3
#define KEY_3           4
#define KEY_4           5
#define KEY_5           6
#define KEY_6           7
#define KEY_7           8
#define KEY_8           9
#define KEY_9           10
#define KEY_0           11

#define KEY_LEFT        203
#define KEY_RIGHT       205
#define KEY_UP          200
#define KEY_DOWN        208

#define KEY_BACKSPACE   14
#define KEY_TAB         15
#define KEY_ENTER       28
#define KEY_SPACE       57

#define KEY_INSERT      210
#define KEY_DELETE      211
#define KEY_HOME        199
#define KEY_END         207
#define KEY_PGUP        201
#define KEY_PGDN        209

#define KEY_LSHIFT      42
#define KEY_RSHIFT      54
#define KEY_LALT        56
#define KEY_RALT        184
#define KEY_LCTRL       29
#define KEY_RCTRL       157

#define KEY_LWIN        219
#define KEY_RWIN        220
#define KEY_MENU        221

#define KEY_Q           16
#define KEY_W           17
#define KEY_E           18
#define KEY_R           19
#define KEY_T           20
#define KEY_Y           21
#define KEY_U           22
#define KEY_I           23
#define KEY_O           24
#define KEY_P           25

#define KEY_A           30
#define KEY_S           31
#define KEY_D           32
#define KEY_F           33
#define KEY_G           34
#define KEY_H           35
#define KEY_J           36
#define KEY_K           37
#define KEY_L           38

#define KEY_Z           44
#define KEY_X           45
#define KEY_C           46
#define KEY_V           47
#define KEY_B           48
#define KEY_N           49
#define KEY_M           50

#endif