// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "BaseDef.hpp"
#include "CBuf.hpp"

#include <windows.h>
#include <stdlib.h>

#include <algorithm>
#include <string>

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

public:
    CBitmap();
    ~CBitmap();

    int Format() const { return m_Format; }
    int SizeX(void) const { return m_Size.x; }
    int SizeY(void) const { return m_Size.y; }
    const Base::CPoint &Size(void) const { return m_Size; }
    int BytePP(void) const { return m_BytePP; }
    int BitPP(void) { return m_BitPP; }
    void *Data(void) const { return m_Data; }
    const BYTE *ByteData(void) const { return (const BYTE *)m_Data; }
    int Pitch(void) const { return m_Pitch; }
    dword ColorMask(int index) const { return m_MColor[index]; }

    void CreateRGB(int lenx, int leny);
    void CreateRGBA(int lenx, int leny, int pitch = 0, void* data = nullptr);

    void Copy(const Base::CPoint &pdes, const Base::CPoint &size, const CBitmap &bmsou, const Base::CPoint &spsou);

    void Tile(const Base::CPoint &pdes, const Base::CPoint &desize, const CBitmap &bmsou, const Base::CPoint &spsou,
              const Base::CPoint &szsou);

    void Fill(const Base::CPoint &pdes, const Base::CPoint &size, dword color);

    void Make2xSmaller(void);
    void Make2xSmaller(const Base::CPoint &left_up_corner, const Base::CPoint &size, CBitmap &des_bitmap) const;

    void MergeByMask(const Base::CPoint &pdes, const Base::CPoint &size, const CBitmap &bm1, const Base::CPoint &sp1,
                     const CBitmap &bm2, const Base::CPoint &sp2, const CBitmap &mask, const Base::CPoint &spm);
    void SwapByte(const Base::CPoint &pos, const Base::CPoint &size, int n1, int n2);

    void MergeWithAlpha(const Base::CPoint &pdes, const Base::CPoint &size, const CBitmap &bmsou,
                        const Base::CPoint &spsou);

    void SaveInBMP(const std::wstring_view filename) const;
    void SaveInDDSUncompressed(Base::CBuf &buf) const;
    bool SaveInPNG(const std::wstring_view filename);
    bool LoadFromPNG(const std::wstring_view filename);
    bool LoadFromPNG(void *buf, int buflen);

protected:
    void FlipY(void);
    void Create16(int lenx, int leny);

private:
    void Clear(void);
    void AllocData(void);
    void CreatePalate(int lenx, int leny, int palcnt);
    void CreateGrayscale(int lenx, int leny);

    void SaveInBMP(Base::CBuf &buf) const;
    bool SaveInPNG(Base::CBuf &buf);
    int SaveInPNG(void *buf, int buflen);
};

class WinBitmap : public CBitmap
{
public:
    WinBitmap() = default;

    ~WinBitmap()
    {
        Clear();
    }
    void Clear(void);
    void Init(void);
    void Save(bool save16as32 = false);
    HBITMAP GetHandle(void) { return m_handle; }
    HDC GetDC(void) { return m_dc; }
    void SetHandle(HBITMAP bm) { m_handle = bm; }
    void SetDC(HDC hdc) { m_dc = hdc; }

private:
    HBITMAP m_handle{nullptr};  // Windows-кий bitmap
    HDC m_dc{nullptr};          // Context windows-кого bitmap-а
};