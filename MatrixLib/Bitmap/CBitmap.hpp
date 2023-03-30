// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include <algorithm>

#include <windows.h>
#include <stdlib.h>

#include "BaseDef.hpp"
#include "CBuf.hpp"
#include "CHeap.hpp"

#define BMF_USER        0
#define BMF_FLAT        1
#define BMF_PALATE      2
#define BMF_FLAT_PALATE 3  // Alpha in data

enum EAreaType {
    AREA_TRANSPARENT,
    AREA_SOLID,
    AREA_SEMITRANSPARENT,

    EAreaType_FORCE_DWORD = 0x7FFFFFFF
};

class CBitmap
{
private:
    Base::CHeap *m_Heap;

    Base::CPoint m_Pos;   // Смещение изображения
    Base::CPoint m_Size;  // Размер изображения

    int m_Format;       // BMF_*
    int m_BytePP;       // Байт на пиксель
    int m_BitPP;        // Бит на пиксель
    dword m_MColor[4];  // Маска для каждой компоненты цвета

    void *m_Data;    // Изображение
    int m_Pitch;     // Байт на одну строку
    bool m_DataExt;  // True - внешние данные

    void *m_AddData[4];    // Дополнительные данные к изображению
    bool m_AddDataExt[4];  // True - Дополнительные данные во внешнем буфере
    int m_AddDataVal[4];   //

    dword m_UserData;

    HBITMAP m_WindowBitmap;  // Windows-кий bitmap
    HDC m_WindowDC;          // Context windows-кого bitmap-а
public:
    CBitmap(void* heap = nullptr); // TODO: remove param
    ~CBitmap();

    void Clear(void);

    int SizeX(void) const { return m_Size.x; }
    int SizeY(void) const { return m_Size.y; }
    const Base::CPoint &Size(void) const { return m_Size; }
    int BytePP(void) const { return m_BytePP; }
    int BitPP(void) { return m_BitPP; }
    void *Data(void) const { return m_Data; }
    const BYTE *ByteData(void) const { return (const BYTE *)m_Data; }
    int Pitch(void) const { return m_Pitch; }

    void BitmapDuplicate(CBitmap &des);

    bool Equals(const CBitmap &bm) const;

    bool ConvertFrom(CBitmap &bm);
    void Convert32To16(void);

    void AllocData(void);

    void CreatePalate(int lenx, int leny, int palcnt);
    void CreateGrayscale(int lenx, int leny);
    void Create16(int lenx, int leny) {
        Clear();
        m_Size.x = lenx;
        m_Size.y = leny;
        m_Pitch = lenx * 2;
        m_Format = BMF_FLAT;
        m_BytePP = 2;
        m_BitPP = 16;
        m_MColor[0] = 0x0000f800;
        m_MColor[1] = 0x000007e0;
        m_MColor[2] = 0x0000001f;
        AllocData();
    }
    void Create15(int lenx, int leny) {
        Clear();
        m_Size.x = lenx;
        m_Size.y = leny;
        m_Pitch = lenx * 2;
        m_Format = BMF_FLAT;
        m_BytePP = 2;
        m_BitPP = 15;
        m_MColor[0] = 0x00007c00;
        m_MColor[1] = 0x000003e0;
        m_MColor[2] = 0x0000001f;
        AllocData();
    }
    void CreateRGB(int lenx, int leny) {
        Clear();
        m_Size.x = lenx;
        m_Size.y = leny;
        m_Pitch = lenx * 3;
        m_Format = BMF_FLAT;
        m_BytePP = 3;
        m_BitPP = 24;
        m_MColor[0] = 0x000000ff;
        m_MColor[1] = 0x0000ff00;
        m_MColor[2] = 0x00ff0000;
        AllocData();
    }
    void CreateRGB(int lenx, int leny, int pitch) {
        Clear();
        m_Size.x = lenx;
        m_Size.y = leny;
        m_Pitch = std::max(lenx * 3, pitch);
        m_Format = BMF_FLAT;
        m_BytePP = 3;
        m_BitPP = 24;
        m_MColor[0] = 0x000000ff;
        m_MColor[1] = 0x0000ff00;
        m_MColor[2] = 0x00ff0000;
        AllocData();
    }
    void CreateRGBA(int lenx, int leny) {
        Clear();
        m_Size.x = lenx;
        m_Size.y = leny;
        m_Pitch = lenx * 4;
        m_Format = BMF_FLAT;
        m_BytePP = 4;
        m_BitPP = 32;
        m_MColor[0] = 0x000000ff;
        m_MColor[1] = 0x0000ff00;
        m_MColor[2] = 0x00ff0000;
        m_MColor[3] = 0xff000000;
        AllocData();
    }
    void CreateRGBA(int lenx, int leny, int pitch) {
        Clear();
        m_Size.x = lenx;
        m_Size.y = leny;
        m_Pitch = std::max(lenx * 4, pitch);
        m_Format = BMF_FLAT;
        m_BytePP = 4;
        m_BitPP = 32;
        m_MColor[0] = 0x000000ff;
        m_MColor[1] = 0x0000ff00;
        m_MColor[2] = 0x00ff0000;
        m_MColor[3] = 0xff000000;
        AllocData();
    }
    void CreateRGBA(int lenx, int leny, int pitch, void *data) {
        Clear();
        m_Size.x = lenx;
        m_Size.y = leny;
        m_Pitch = pitch;
        m_Format = BMF_FLAT;
        m_BytePP = 4;
        m_BitPP = 32;
        m_MColor[0] = 0x000000ff;
        m_MColor[1] = 0x0000ff00;
        m_MColor[2] = 0x00ff0000;
        m_MColor[3] = 0xff000000;
        m_Data = data;
        m_DataExt = true;
    }
    void SetRGBA(int lenx, int leny) {
        m_Size.x = lenx;
        m_Size.y = leny;
        m_Pitch = lenx * 4;
        m_Format = BMF_FLAT;
        m_BytePP = 4;
        m_BitPP = 32;
        m_MColor[0] = 0x000000ff;
        m_MColor[1] = 0x0000ff00;
        m_MColor[2] = 0x00ff0000;
        m_MColor[3] = 0xff000000;
    }

    void Recrate(int lenx, int leny, int pitch);

    void Copy(const Base::CPoint &pdes, const Base::CPoint &size, const CBitmap &bmsou, const Base::CPoint &spsou);

    void Tile(const Base::CPoint &pdes, const Base::CPoint &desize, const CBitmap &bmsou, const Base::CPoint &spsou,
              const Base::CPoint &szsou);

    void Fill(const Base::CPoint &pdes, const Base::CPoint &size, dword color);

    void Rotate90(const Base::CPoint &pdes, const Base::CPoint &sizesou, const CBitmap &bmsou,
                  const Base::CPoint &spsou);
    void Rotate180(const Base::CPoint &pdes, const Base::CPoint &sizesou, const CBitmap &bmsou,
                   const Base::CPoint &spsou);
    void Rotate270(const Base::CPoint &pdes, const Base::CPoint &sizesou, const CBitmap &bmsou,
                   const Base::CPoint &spsou);

    void MakeLarger(int factor);
    void MakeLarger(int factor, CBitmap &des_bitmap) const;
    void Make2xSmaller(void);
    void Make2xSmaller(const Base::CPoint &left_up_corner, const Base::CPoint &size, CBitmap &des_bitmap) const;

    EAreaType GetAreaType(const Base::CPoint &p, const Base::CPoint &size);

    void FlipX(const Base::CPoint &pdes, const Base::CPoint &size, const CBitmap &bmsou, const Base::CPoint &spsou);
    void FlipY(const Base::CPoint &pdes, const Base::CPoint &size, const CBitmap &bmsou, const Base::CPoint &spsou);
    void FlipY(const Base::CPoint &pdes, const Base::CPoint &size);
    void FlipY(void);

    void MergeByMask(const Base::CPoint &pdes, const Base::CPoint &size, const CBitmap &bm1, const Base::CPoint &sp1,
                     const CBitmap &bm2, const Base::CPoint &sp2, const CBitmap &mask, const Base::CPoint &spm);
    void SwapByte(const Base::CPoint &pos, const Base::CPoint &size, int n1, int n2);

    void MergeWithAlpha(const Base::CPoint &pdes, const Base::CPoint &size, const CBitmap &bmsou,
                        const Base::CPoint &spsou);
    void MergeWithAlphaPM(const Base::CPoint &pdes, const Base::CPoint &size, const CBitmap &bmsou,
                          const Base::CPoint &spsou);  // PM - premultiplied alpha

    void MergeGrayscaleWithAlpha(const Base::CPoint &pdes, const Base::CPoint &size, const CBitmap *data_src,
                                 const Base::CPoint &data_src_point, const CBitmap *alpha_src,
                                 const Base::CPoint &alpha_src_point);
    void MergeGrayscaleWithAlphaPM(const Base::CPoint &pdes, const Base::CPoint &size, const CBitmap *data_src,
                                   const Base::CPoint &data_src_point, const CBitmap *alpha_src,
                                   const Base::CPoint &alpha_src_point);
    void ModulateGrayscaleWithAlpha(const Base::CPoint &pdes, const Base::CPoint &size, const CBitmap *alpha_src,
                                    const Base::CPoint &alpha_src_point);

    dword UserData(void) { return m_UserData; }
    void UserData(dword zn) {
        m_UserData = zn;
        ;
    }

    DWORD ARGB(float x, float y);  // get interpolated ARGB for specified coordinate (0.0 - 1.0)
                                   // 0.0,0.0 - center of left-top pixel
                                   // 1.0,1.0 - center of right-bottom pixel

    DWORD ARGBPixel(int x, int y)  // get ARGB color of specified pixel
    {
        DWORD c;
        c = *(DWORD *)((BYTE *)m_Data + (y * m_Pitch + x * m_BytePP));
        if (m_Format == BMF_PALATE) {
            return 0;  // не работаем с палитровыми изображениями
        }
        DWORD mask = 0xFF;
        if (m_BytePP == 2)
            mask = 0x0000FFFF;
        else if (m_BytePP == 3)
            mask = 0x00FFFFFF;
        else if (m_BytePP == 4)
            mask = 0xFFFFFFFF;
        return c & mask;
    }

    DWORD Hash(void);

    void WBM_Clear(void);
    void WBM_Init(void);
    void WBM_Save(bool save16as32 = false);
    HBITMAP WBM_Bitmap(void) { return m_WindowBitmap; }
    HDC WBM_BitmapDC(void) { return m_WindowDC; }
    void WBM_Bitmap(HBITMAP bm) { m_WindowBitmap = bm; }
    void WBM_BitmapDC(HDC hdc) { m_WindowDC = hdc; }

    void SaveInBMP(Base::CBuf &buf) const;
    void SaveInBMP(const wchar *filename, int filenamelen) const;
    void SaveInBMP(const wchar *filename) const { SaveInBMP(filename, std::wcslen(filename)); }

    void SaveInDDSUncompressed(Base::CBuf &buf) const;
    void SaveInDDSUncompressed(const wchar *filename, int filenamelen) const;
    void SaveInDDSUncompressed(const wchar *filename) const {
        SaveInDDSUncompressed(filename, std::wcslen(filename));
    }

#ifdef USE_PNGLIB
    bool LoadFromPNG(void *buf, int buflen);
    bool LoadFromPNG(const wchar *filename);
    int SaveInPNG(void *buf, int buflen);
    bool SaveInPNG(Base::CBuf &buf);
    bool SaveInPNG(const wchar *filename, int filenamelen);
    bool SaveInPNG(const wchar *filename) { return SaveInPNG(filename, std::wcslen(filename)); }
#endif
};
