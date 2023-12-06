// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "BaseDef.hpp"
#include "CBuf.hpp"

#include <windows.h>
#include <string>

class CBitmap
{
private:
    Base::CPoint m_Pos;   // Смещение изображения
    Base::CPoint m_Size;  // Размер изображения

    int m_Format;       // BMF_*
    int m_BytePP;       // Байт на пиксель
    int m_BitPP;        // Бит на пиксель
    uint32_t m_MColor[4];  // Маска для каждой компоненты цвета

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
    int SizeX() const { return m_Size.x; }
    int SizeY() const { return m_Size.y; }
    const Base::CPoint &Size() const { return m_Size; }
    int BytePP() const { return m_BytePP; }
    int BitPP() { return m_BitPP; }
    void *Data() const { return m_Data; }
    const uint8_t *ByteData() const { return (const uint8_t *)m_Data; }
    int Pitch() const { return m_Pitch; }
    uint32_t ColorMask(int index) const { return m_MColor[index]; }

    void CreateRGB(int lenx, int leny);
    void CreateRGBA(int lenx, int leny, int pitch = 0, void* data = nullptr);

    void Copy(const Base::CPoint &pdes, const Base::CPoint &size, const CBitmap &bmsou, const Base::CPoint &spsou);

    void Tile(const Base::CPoint &pdes, const Base::CPoint &desize, const CBitmap &bmsou, const Base::CPoint &spsou,
              const Base::CPoint &szsou);

    void Fill(const Base::CPoint &pdes, const Base::CPoint &size, uint32_t color);

    void Make2xSmaller();
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
    void FlipY();
    void Create16(int lenx, int leny);

private:
    void Clear();
    void AllocData();
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
    void Clear();
    void Init();
    void Save(bool save16as32 = false);
    HBITMAP GetHandle() { return m_handle; }
    HDC GetDC() { return m_dc; }
    void SetHandle(HBITMAP bm) { m_handle = bm; }
    void SetDC(HDC hdc) { m_dc = hdc; }

private:
    HBITMAP m_handle{nullptr};  // Windows-кий bitmap
    HDC m_dc{nullptr};          // Context windows-кого bitmap-а
};