// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "MatrixInstantDraw.hpp"

#define MINIMAP_MAX_SCALE (3.0f)
#define MINIMAP_MIN_SCALE (1.0f)

#define MINIMAP_SIZE       512
#define MINIMAP_MAX_EVENTS 32
#define MINIMAP_EVENT_TTL  1000.0f
#define MINIMAP_EVENT_R1   15.0f
#define MINIMAP_EVENT_R2   1.0f
#define MINIMAP_OUT_SCALE  0.8f

#define MINIMAP_FLASH_PERIOD 100.0f

#define MINIMAP_OUT_INDICATOR_R 15.0f

#define MINIMAP_FLYER_R         8.0f
#define MINIMAP_ROBOT_R         8.0f
#define MINIMAP_BUILDING_R      8.0f
#define MINIMAP_BUILDING_BASE_R 8.0f
#define MINIMAP_CANNON_R        8.0f

#define MINIMAP_FLYER_C1 0xFFFFFFFF
#define MINIMAP_FLYER_C2 0x00FFFFFF

#define EVENT_SHOWPB_C1 0xFF00FFFF
#define EVENT_SHOWPB_C2 0xFF00FFFF

#define EVENT_SHOW_MOVETO_C1 0xFFFF0000
#define EVENT_SHOW_MOVETO_C2 0xFFFF0000

#define EVENT_CANNON_C1 0xFFFF00FF
#define EVENT_CANNON_C2 0xFFFF00FF

#define MINIMAP_SUPPORT_ROTATION

#define FLASH_PERIOD 1000

struct SMinimapEvent {
    D3DXVECTOR2 pos_in_world;
    float ttl;
    DWORD color1;
    DWORD color2;
};

class CMinimap : public CMain {
    struct SMMTex {
        CTextureManaged *tex;
        float u0, v0;
        float u1, v1;

        void Load(CBlockPar *mm, const wchar *name);
    };

    enum EMMTex {
        MMT_POINT,
        MMT_ARROW,

        MMT_FLYER,
        MMT_TURRET,
        MMT_ROBOT,
        MMT_BASE,
        MMT_FACTORY,

        MMT_FLYER_R,
        MMT_TURRET_R,
        MMT_ROBOT_R,
        MMT_BASE_R,
        MMT_FACTORY_R,

        MMT_CNT
    };

    CTexture *m_Texture;
    IDirect3DTexture9 *m_TextureStore;  // SM texture
    SMMTex m_Tex[MMT_CNT];

    int m_PosX, m_PosY, m_SizeX, m_SizeY;
    DWORD m_Color;
    SVert_V4_UV m_Verts[4];

    D3DVIEWPORT9 m_Viewport;

    float m_Time;

    float m_Scale;
    float m_TgtScale;
    D3DXVECTOR2 m_Center;
    D3DXVECTOR2 m_Delta;

#ifdef MINIMAP_SUPPORT_ROTATION
    D3DXMATRIX m_Rotation;
#endif

    SMinimapEvent m_Events[MINIMAP_MAX_EVENTS];
    int m_EventsCnt;

    DWORD m_Dirty;

    bool IsOut(const D3DXVECTOR2 &pos) const {
        return (pos.x < float(m_PosX)) || (pos.x > float(m_PosX + m_SizeX)) || (pos.y < float(m_PosY)) ||
               (pos.y > float(m_PosY + m_SizeY));
    };

    void Clip(D3DXVECTOR2 &out, const D3DXVECTOR2 &in);

public:
    CMinimap(void) : m_Texture(NULL), m_Dirty(1), m_Time(0), m_TgtScale(1), m_Scale(1), m_TextureStore(NULL) {
        memset(m_Tex, 0, sizeof(m_Tex));
    };
    ~CMinimap(void){};

    void StoreTexture(void);
    void RestoreTexture(void);

    CTexture *GetTexture() const { return m_Texture; };

    float GetScale(void) const { return m_Scale; }

    void SetColor(DWORD color) { m_Color = color; }
    DWORD GetColor(void) const { return m_Color; }

#ifdef MINIMAP_SUPPORT_ROTATION
    void SetAngle(float angle);
#endif

    void SetOutParams(int x, int y, int sx, int sy, const D3DXVECTOR2 &center, float scale, DWORD color) {
        m_PosX = x;
        m_PosY = y;
        m_SizeX = sx;
        m_SizeY = sy;
        m_Color = color;
        m_Center = center;
        if (scale < MINIMAP_MIN_SCALE)
            scale = MINIMAP_MIN_SCALE;
        if (scale > MINIMAP_MAX_SCALE)
            scale = MINIMAP_MAX_SCALE;
        m_TgtScale = scale;
        m_Dirty = 1;
    }
    void SetOutParams(const D3DXVECTOR2 &center) {
        m_Center = center;
        m_Dirty = 1;
    }
    void SetOutParams(const D3DXVECTOR2 &center, float scale) {
        m_Center = center;
        if (scale < MINIMAP_MIN_SCALE)
            scale = MINIMAP_MIN_SCALE;
        if (scale > MINIMAP_MAX_SCALE)
            scale = MINIMAP_MAX_SCALE;
        m_TgtScale = scale;
        m_Dirty = 1;
    }
    void SetOutParams(float scale) {
        if (scale < MINIMAP_MIN_SCALE)
            scale = MINIMAP_MIN_SCALE;
        if (scale > MINIMAP_MAX_SCALE)
            scale = MINIMAP_MAX_SCALE;
        m_TgtScale = scale;
        m_Dirty = 1;
    }

    void BeforeDraw(void);
    void DrawOutIndicator(const D3DXVECTOR2 &in, float r, DWORD c1, DWORD c2, bool doclip, EMMTex tex);
    void Draw(void);
    void DrawRadar(float x, float y, float radius);
    void Takt(float takt);
    void PauseTakt(float takt);

    void World2Map(D3DXVECTOR2 &out, const D3DXVECTOR2 &in);
    void Map2World(D3DXVECTOR2 &out, const D3DXVECTOR2 &in);
    bool CalcMinimap2World(D3DXVECTOR2 &tgt);

    void AddEvent(float x, float y, DWORD color1, DWORD color2);

    void RenderBackground(const std::wstring &name, DWORD uniq);
    void RenderObjectToBackground(CMatrixMapStatic *s);

    void Init(void);
    void Clear(void);

    float GetCurrentScale() { return m_TgtScale; }

    void __stdcall ButtonZoomIn(void *object);
    void __stdcall ButtonZoomOut(void *object);
    void __stdcall ButtonClick(void *object);
    void __stdcall ShowPlayerBots(void *object);
};