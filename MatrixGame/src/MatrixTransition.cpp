// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "MatrixTransition.hpp"
#include "MatrixMap.hpp"
#include "MatrixSampleStateManager.hpp"


void CTransition::Clear(void)
{
    DTRACE();

    if(m_Tex)
    {
        g_Cache->Destroy(m_Tex);
        m_Tex = NULL;

        HFree(m_Geom, g_MatrixHeap);
        m_Geom = NULL;
        m_GeomCnt = 0;
    }
}

//Готовим "створки" для раздвижения в разные стороны сразу после загрузки ПБ
//(делается скриншот экрана загрузки и режется на 4 части по указанным в конфиге точкам)
void CTransition::BuildTexture(void)
{
    DTRACE();

    //Проверяем, включено ли раскрытие створок в data.txt
    bool loadingScreenAnim = g_MatrixData->BlockGet(L"Config")->ParGet(CFG_LOADING_SCREEN_ANIM).GetInt() == 1;
    if(!loadingScreenAnim) return;

    //Также проверяем, включены ли анимированные переходы между формами в основной игре
    CBlockPar gameCFG = CBlockPar(true, g_MatrixHeap);
    wchar gameCFG_Path[MAX_PATH];
    SHGetSpecialFolderPathW(0, gameCFG_Path, CSIDL_PERSONAL, true);
    wcscat(gameCFG_Path, L"\\SpaceRangersHD\\CFG.TXT");
    CWStr temp_path(g_MatrixHeap);
    if(CFile::FileExist(temp_path, gameCFG_Path))
    {
        gameCFG.LoadFromTextFile(gameCFG_Path);
        CWStr animChangeForm = gameCFG.ParGetNE(L"AnimChangeForm");
        if(animChangeForm != NULL)
        {
            if(animChangeForm != L"True") return;
        }
    }

    HDC hdc = GetDC(g_Wnd);

    RECT r;
    GetClientRect(g_Wnd, &r);

    m_ScreenX = r.right - r.left;
    m_ScreenY = r.bottom - r.top;

    CBlockPar* tra = g_MatrixData->BlockGetNE(L"Transitions");

    if(tra == NULL) return;

    double coord_mod = 1.0;
    int x_offset = 0;
    int native_aspect_res = m_ScreenX;
    //Проверяем, имеются ли в конфиге координаты нужных для разделения точек для данного разрешения
    if(m_ScreenX <= 1024) tra = tra->BlockGetNE(CWStr(m_ScreenX, g_CacheHeap).Add(L".").Add(m_ScreenY).Get());
    //Если нет, то берём их значения для 1024x768 и рассчитываем модификатор от фактического разрешения экрана
    //Плюс сдвиг в пикселях, если соотношение сторон не 4x3
    else
    {
        tra = tra->BlockGetNE(CWStr(1024, g_CacheHeap).Add(L".").Add(768).Get());

        coord_mod = double(m_ScreenY) / m_ScreenX;
        if(coord_mod != 0.75) //0.75 - базовое игровое соотношение сторон 4 на 3
        {
            x_offset = (m_ScreenX - m_ScreenY / 0.75) / 2;
            native_aspect_res = m_ScreenX - (m_ScreenX - m_ScreenY / 0.75);
        }
        coord_mod = m_ScreenY / 768.0;
    }

    //Если было выставлено (не знаю как и зачем) кастомное разрешение не равное ни 1024x768, ни 800x600
    //Либо если поломан конфиг
    if(tra == NULL) return;

    m_GeomCnt = tra->ParCount();
    if(!m_GeomCnt) return;

    CBitmap bm(g_CacheHeap);
    CBitmap bmout(g_CacheHeap);
    bmout.CreateRGB(DetermineGreaterPowerOfTwo(m_ScreenX), DetermineGreaterPowerOfTwo(m_ScreenY));
    bmout.Fill(CPoint(0, 0), bmout.Size(), 0);

    //float diver = 1.0f / float(m_ScreenY / 2);

    m_Geom = (SGeom*)HAllocClear(sizeof(SGeom) * m_GeomCnt, g_MatrixHeap);
    for(int i = 0; i < m_GeomCnt; ++i)
    {
        const CWStr& da = tra->ParGet(i);

        int x_check = da.GetStrPar(0, L"|").GetIntPar(0, L",");
        if(!x_check) m_Geom[i].verts[0].p.x = 0;
        else if(x_check == 1024) m_Geom[i].verts[0].p.x = m_ScreenX;
        else m_Geom[i].verts[0].p.x = (float)da.GetStrPar(0, L"|").GetDoublePar(0, L",") * coord_mod + x_offset;
        m_Geom[i].verts[0].p.y = (float)da.GetStrPar(0, L"|").GetDoublePar(1, L",") * coord_mod;
        m_Geom[i].verts[0].p.z = 0;
        m_Geom[i].verts[0].p.w = 1;
        m_Geom[i].verts[0].tu = m_Geom[i].verts[0].p.x / float(bmout.SizeX());
        m_Geom[i].verts[0].tv = m_Geom[i].verts[0].p.y / float(bmout.SizeY());

        x_check = da.GetStrPar(1, L"|").GetIntPar(0, L",");
        if(!x_check) m_Geom[i].verts[1].p.x = 0;
        else if(x_check == 1024) m_Geom[i].verts[1].p.x = m_ScreenX;
        else m_Geom[i].verts[1].p.x = (float)da.GetStrPar(1, L"|").GetDoublePar(0, L",") * coord_mod + x_offset;
        m_Geom[i].verts[1].p.y = (float)da.GetStrPar(1, L"|").GetDoublePar(1, L",") * coord_mod;
        m_Geom[i].verts[1].p.z = 0;
        m_Geom[i].verts[1].p.w = 1;
        m_Geom[i].verts[1].tu = m_Geom[i].verts[1].p.x / float(bmout.SizeX());
        m_Geom[i].verts[1].tv = m_Geom[i].verts[1].p.y / float(bmout.SizeY());

        x_check = da.GetStrPar(3, L"|").GetIntPar(0, L",");
        if(!x_check) m_Geom[i].verts[2].p.x = 0;
        else if(x_check == 1024) m_Geom[i].verts[2].p.x = m_ScreenX;
        else m_Geom[i].verts[2].p.x = (float)da.GetStrPar(3, L"|").GetDoublePar(0, L",") * coord_mod + x_offset;
        m_Geom[i].verts[2].p.y = (float)da.GetStrPar(3, L"|").GetDoublePar(1, L",") * coord_mod;
        m_Geom[i].verts[2].p.z = 0;
        m_Geom[i].verts[2].p.w = 1;
        m_Geom[i].verts[2].tu = m_Geom[i].verts[2].p.x / float(bmout.SizeX());
        m_Geom[i].verts[2].tv = m_Geom[i].verts[2].p.y / float(bmout.SizeY());

        x_check = da.GetStrPar(2, L"|").GetIntPar(0, L",");
        if(!x_check) m_Geom[i].verts[3].p.x = 0;
        else if(x_check == 1024) m_Geom[i].verts[3].p.x = m_ScreenX;
        else m_Geom[i].verts[3].p.x = (float)da.GetStrPar(2, L"|").GetDoublePar(0, L",") * coord_mod + x_offset;
        m_Geom[i].verts[3].p.y = (float)da.GetStrPar(2, L"|").GetDoublePar(1, L",") * coord_mod;
        m_Geom[i].verts[3].p.z = 0;
        m_Geom[i].verts[3].p.w = 1;
        m_Geom[i].verts[3].tu = m_Geom[i].verts[3].p.x / float(bmout.SizeX());
        m_Geom[i].verts[3].tv = m_Geom[i].verts[3].p.y / float(bmout.SizeY());

        if(tra->ParGetName(i).CompareFirst(L"Up,"))
        {
            m_Geom[i].dir.y = -1;
            //Получаем скорость движения верхней створки
            m_Geom[i].dir *= (float)tra->ParGetName(i).GetDoublePar(1, L",");
        }
        else if(tra->ParGetName(i).CompareFirst(L"Down,"))
        {
            m_Geom[i].dir.y = 1;
            //Получаем скорость движения нижней створки
            m_Geom[i].dir *= (float)tra->ParGetName(i).GetDoublePar(1, L",");
        }
        else if(tra->ParGetName(i).CompareFirst(L"Left,"))
        {
            m_Geom[i].dir.x = -1;
            //Получаем скорость движения левой створки (ускоряем створку, если соотношение экрана не 4x3)
            //1.08f здесь просто рандомный коэффициент, чтобы скорость лучше соотносилась с разрешениями формата 16:9 и 16:10
            m_Geom[i].dir *= (float)tra->ParGetName(i).GetDoublePar(1, L",") * (float(g_ScreenX) * 1.08f / native_aspect_res);
        }
        else if(tra->ParGetName(i).CompareFirst(L"Right,"))
        {
            m_Geom[i].dir.x = 1;
            //Получаем скорость движения правой створки (ускоряем створку, если соотношение экрана не 4x3)
            //1.08f здесь просто рандомный коэффициент, чтобы скорость лучше соотносилась с разрешениями формата 16:9 и 16:10
            m_Geom[i].dir *= (float)tra->ParGetName(i).GetDoublePar(1, L",") * (float(g_ScreenX) * 1.08f / native_aspect_res);
        }
    }

    bm.WBM_Bitmap(CreateCompatibleBitmap(hdc, g_ScreenX, g_ScreenY));
    bm.WBM_BitmapDC(CreateCompatibleDC(hdc));

    if(!SelectObject(bm.WBM_BitmapDC(), bm.WBM_Bitmap()))
    {
        ReleaseDC(g_Wnd, hdc);
        return;
    }

    BitBlt(bm.WBM_BitmapDC(), 0, 0, g_ScreenX, g_ScreenY, hdc, 0, 0, SRCCOPY);

    ReleaseDC(g_Wnd, hdc);

    bm.WBM_Save(true);

    bmout.Copy(CPoint(0, 0), bm.Size(), bm, CPoint(0, 0));

    m_Tex = CACHE_CREATE_TEXTUREMANAGED();
    m_Tex->MipmapOff();
    m_Tex->LoadFromBitmap(bmout, D3DFMT_DXT1, 1);
}

void CTransition::Render(void)
{
    DTRACE();

    if(m_Tex == NULL) return;

    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
    //ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHAREF, 0));

    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));

    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    SetColorOpSelect(0, D3DTA_TEXTURE);
    SetAlphaOpSelect(0, D3DTA_TEXTURE);
    SetColorOpDisable(1);

    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));

    m_Tex->Preload();

    SVert_V4_UV verts[4];

    CInstDraw::BeginDraw(IDFVF_V4_UV);

    for(int i = 0; i < m_GeomCnt; ++i)
    {
        float posx = (float)floor(m_Geom[i].pos.x) - 0.5f;
        float posy = (float)floor(m_Geom[i].pos.y) - 0.5f;

        memcpy(verts, m_Geom[i].verts, sizeof(verts));
        for(int j = 0; j < 4; ++j)
        {
            verts[j].p.x += posx;
            verts[j].p.y += posy;
        }

        CInstDraw::AddVerts(verts, m_Tex);
    }

    CInstDraw::ActualDraw();

    //g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    //ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
    //ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHAREF, 0x08));

    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    g_D3DD->SetRenderState(D3DRS_ZENABLE, TRUE);
}

void CTransition::RenderToPrimaryScreen(void)
{
    if(m_ScreenX != g_ScreenX || m_ScreenY != g_ScreenY)
    {
        float kx = float(double(g_ScreenX) / double(m_ScreenX));
        float ky = float(double(g_ScreenY) / double(m_ScreenY));

        for(int i = 0; i < m_GeomCnt; ++i)
        {
            for(int j = 0; j < 4; ++j)
            {
                m_Geom[i].verts[j].p.x *= kx;
                m_Geom[i].verts[j].p.y *= ky;
            }
        }
    }

    g_D3DD->BeginScene();
    Render();
    g_D3DD->EndScene();
    g_D3DD->Present(NULL, NULL, NULL, NULL);

    SETFLAG(g_MatrixMap->m_Flags, MMFLAG_TRANSITION);
}

void CTransition::Takt(int ms)
{
    float add = float(ms) * 0.5f;
    for(int i = 0; i < m_GeomCnt;)
    {
        m_Geom[i].pos += m_Geom[i].dir * add;

        int j = 0;
        for(j = 0; j < 4; ++j)
        {
            float x = m_Geom[i].verts[j].p.x + m_Geom[i].pos.x;
            float y = m_Geom[i].verts[j].p.y + m_Geom[i].pos.y;
            if(x >= 0 && x < float(g_ScreenX) && y >= 0 && y < float(g_ScreenY)) break;
        }
        // if(j == 4)
        // {
            //delete this

            // m_Geom[i] = m_Geom[--m_GeomCnt];

            // if(m_GeomCnt == 0)
            // {
                // RESETFLAG(g_MatrixMap->m_Flags, MMFLAG_TRANSITION);
                // Clear();
                // return;
            // }
            // continue;
        // }

        ++i;
    }
}