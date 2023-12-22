// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef HPP_3G_INCLUDE
#define HPP_3G_INCLUDE

#include "BaseDef.hpp"
#include "CBlockPar.hpp"
#include "Cache.hpp"
#include "Math3D.hpp"
#include "Form.hpp"
#include "Helper.hpp"

#include "d3d9.h"
#include "d3dx9tex.h"

#define GFLAG_APPACTIVE SETBIT(0)
//#define GFLAG_FORMACCESS            SETBIT(1)
#define GFLAG_APPCLOSE         SETBIT(2)
#define GFLAG_FULLSCREEN       SETBIT(3)
#define GFLAG_STENCILAVAILABLE SETBIT(4)
#define GFLAG_GAMMA            SETBIT(5)
#define GFLAG_EXITLOOP         SETBIT(7)
#define GFLAG_PRESENT_REQUIRED SETBIT(8)
#define GFLAG_KEEPALIVE        SETBIT(9)
#define GFLAG_4SPEED           SETBIT(10)

#ifdef _DEBUG
#define GFLAG_EXTRAFREERES     SETBIT(29)
#define GFLAG_TAKTINPROGRESS   SETBIT(30)
#define GFLAG_RENDERINPROGRESS SETBIT(31)
#endif

extern HINSTANCE g_HInst;
extern IDirect3D9 *g_D3D;
extern IDirect3DDevice9 *g_D3DD;
extern D3DCAPS9 g_D3DDCaps;
extern HWND g_Wnd;
extern int g_ScreenX, g_ScreenY;
extern int g_DrawFPS;
extern int g_MaxFPS;
extern int g_AvailableTexMem;
extern D3DPRESENT_PARAMETERS g_D3Dpp;

// global config flags
extern DWORD g_Flags;

#ifdef DO_SMART_COLOROPS

struct STextureStageOp {
    D3DTEXTUREOP op;
    DWORD p1;
    DWORD p2;
};

extern STextureStageOp g_ColorOp[];
extern STextureStageOp g_AlphaOp[];

#ifdef _DEBUG
#define CHECKTSS(n, t, v) \
    {}
#else
#define CHECKTSS(n, t, v) \
    {}
#endif

inline void SetColorOp(int stage, D3DTEXTUREOP op, DWORD p1, DWORD p2) {
    if (g_ColorOp[stage].op != op) {
        g_D3DD->SetTextureStageState(stage, D3DTSS_COLOROP, op);
        g_ColorOp[stage].op = op;
    }
    else
        CHECKTSS(stage, D3DTSS_COLOROP, op);
    if (g_ColorOp[stage].p1 != p1) {
        g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG1, p1);
        g_ColorOp[stage].p1 = p1;
    }
    else
        CHECKTSS(stage, D3DTSS_COLORARG1, p1);
    if (g_ColorOp[stage].p2 != p2) {
        g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG2, p2);
        g_ColorOp[stage].p2 = p2;
    }
    else
        CHECKTSS(stage, D3DTSS_COLORARG2, p2);
}
inline void SetColorOpAnyOrder(int stage, D3DTEXTUREOP op, DWORD p1, DWORD p2) {
    if (g_ColorOp[stage].op != op) {
        g_D3DD->SetTextureStageState(stage, D3DTSS_COLOROP, op);
        g_ColorOp[stage].op = op;
    }

    if (g_ColorOp[stage].p1 == p1) {
        if (g_ColorOp[stage].p2 != p2) {
            g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG2, p2);
            g_ColorOp[stage].p2 = p2;
        }
    }
    else if (g_ColorOp[stage].p1 == p2) {
        if (g_ColorOp[stage].p2 != p1) {
            g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG2, p1);
            g_ColorOp[stage].p2 = p1;
        }
    }
    else {
        // p2 should be changed
        if (g_ColorOp[stage].p2 == p1) {
            g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG1, p2);
            g_ColorOp[stage].p1 = p2;
        }
        else if (g_ColorOp[stage].p2 == p2) {
            g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG1, p1);
            g_ColorOp[stage].p1 = p1;
        }
        else {
            g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG1, p1);
            g_ColorOp[stage].p1 = p1;
            g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG2, p2);
            g_ColorOp[stage].p2 = p2;
        }
    }
}
inline void SetColorOpSelect(int stage, DWORD p) {
    if (g_ColorOp[stage].op != D3DTOP_SELECTARG1) {
        g_D3DD->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        g_ColorOp[stage].op = D3DTOP_SELECTARG1;
    }
    else
        CHECKTSS(stage, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    if (g_ColorOp[stage].p1 != p) {
        g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG1, p);
        g_ColorOp[stage].p1 = p;
    }
    else
        CHECKTSS(stage, D3DTSS_COLORARG1, p);
}
inline void SetColorOpDisable(int stage) {
    if (g_ColorOp[stage].op != D3DTOP_DISABLE) {
        g_D3DD->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
        g_ColorOp[stage].op = D3DTOP_DISABLE;
    }
    CHECKTSS(stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
}

inline void SetAlphaOp(int stage, D3DTEXTUREOP op, DWORD p1, DWORD p2) {
    if (g_AlphaOp[stage].op != op) {
        g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAOP, op);
        g_AlphaOp[stage].op = op;
    }
    else
        CHECKTSS(stage, D3DTSS_ALPHAOP, op);
    if (g_AlphaOp[stage].p1 != p1) {
        g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG1, p1);
        g_AlphaOp[stage].p1 = p1;
    }
    else
        CHECKTSS(stage, D3DTSS_ALPHAARG1, p1);
    if (g_AlphaOp[stage].p2 != p2) {
        g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG2, p2);
        g_AlphaOp[stage].p2 = p2;
    }
    else
        CHECKTSS(stage, D3DTSS_ALPHAARG2, p2);
}
inline void SetAlphaOpAnyOrder(int stage, D3DTEXTUREOP op, DWORD p1, DWORD p2) {
    if (g_AlphaOp[stage].op != op) {
        g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAOP, op);
        g_AlphaOp[stage].op = op;
    }

    if (g_AlphaOp[stage].p1 == p1) {
        if (g_AlphaOp[stage].p2 != p2) {
            g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG2, p2);
            g_AlphaOp[stage].p2 = p2;
        }
    }
    else if (g_AlphaOp[stage].p1 == p2) {
        if (g_AlphaOp[stage].p2 != p1) {
            g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG2, p1);
            g_AlphaOp[stage].p2 = p1;
        }
    }
    else {
        // p2 should be changed
        if (g_AlphaOp[stage].p2 == p1) {
            g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG1, p2);
            g_AlphaOp[stage].p1 = p2;
        }
        else if (g_AlphaOp[stage].p2 == p2) {
            g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG1, p1);
            g_AlphaOp[stage].p1 = p1;
        }
        else {
            g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG1, p1);
            g_AlphaOp[stage].p1 = p1;
            g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG2, p2);
            g_AlphaOp[stage].p2 = p2;
        }
    }
}
inline void SetAlphaOpSelect(int stage, DWORD p) {
    if (g_AlphaOp[stage].op != D3DTOP_SELECTARG1) {
        g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_AlphaOp[stage].op = D3DTOP_SELECTARG1;
    }
    if (g_AlphaOp[stage].p1 != p) {
        g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG1, p);
        g_AlphaOp[stage].p1 = p;
    }
}
inline void SetAlphaOpDisable(int stage) {
    if (g_AlphaOp[stage].op != D3DTOP_DISABLE) {
        g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        g_AlphaOp[stage].op = D3DTOP_DISABLE;
    }
}
#else
inline void SetColorOp(int stage, D3DTEXTUREOP op, DWORD p1, DWORD p2) {
    g_D3DD->SetTextureStageState(stage, D3DTSS_COLOROP, op);
    g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG1, p1);
    g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG2, p2);
}
inline void SetAlphaOp(int stage, D3DTEXTUREOP op, DWORD p1, DWORD p2) {
    g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAOP, op);
    g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG1, p1);
    g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG2, p2);
}
inline void SetColorOpAnyOrder(int stage, D3DTEXTUREOP op, DWORD p1, DWORD p2) {
    SetColorOp(stage, op, p1, p2);
}
inline void SetAlphaOpAnyOrder(int stage, D3DTEXTUREOP op, DWORD p1, DWORD p2) {
    SetAlphaOp(stage, op, p1, p2);
}
inline void SetColorOpSelect(int stage, DWORD p) {
    g_D3DD->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG1, p);
    g_D3DD->SetTextureStageState(stage, D3DTSS_COLORARG2, p);
}
inline void SetColorOpDisable(int stage) {
    g_D3DD->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
}
inline void SetAlphaOpSelect(int stage, DWORD p) {
    g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG1, p);
    g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAARG2, p);
}
inline void SetAlphaOpDisable(int stage) {
    g_D3DD->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}
#endif

class CExceptionD3D : public CException {
public:
    HRESULT m_Error;

public:
    CExceptionD3D(const char *file, int line, HRESULT err) : CException(file, line) { m_Error = err; }

    std::wstring Info(void) const;
};

#define FAILED_DX(x)                                     \
    {                                                    \
        HRESULT hr = x;                                  \
        if (hr != D3D_OK)                                \
            throw CExceptionD3D(__FILE__, __LINE__, hr); \
    }

#ifdef ASSERT_OFF
#define ASSERT_DX(x) x
#else
#define ASSERT_DX(x) FAILED_DX(x)
#endif

void L3GInitAsEXE(HINSTANCE hinst, CBlockPar& bpcfg, const wchar* sysname, const wchar* captionname);
void L3GInitAsDLL(HINSTANCE hinst, CBlockPar& bpcfg, const wchar* sysname, const wchar* captionname, HWND hwnd, long FDirect3D,
                  long FD3DDevice);
void L3GDeinit(void);
int L3GRun(void);

void S3D_Default(void);

#include "Texture.hpp"
#include "DeviceState.hpp"

#endif
