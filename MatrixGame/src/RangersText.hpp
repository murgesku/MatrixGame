#pragma once

#include <CBitmap.hpp>

#include <d3dx9tex.h>

#include <string>
#include <map>

class RangersText
{
public:
    static void CreateText(std::wstring_view text, std::wstring_view font, uint32_t color, int sizex, int sizey, int alignx, int aligny,
                    int wordwrap, int smex, int smy, const Base::CRect& clipr, CBitmap& dst);

private:
    static LPD3DXFONT GetFont(std::wstring_view font_name);

private:
    struct Font
    {
        LPD3DXFONT m_font{nullptr};
        ~Font()
        {
            if (m_font)
            {
                m_font->Release();
            }
        }
    };

    static std::map<std::wstring_view, Font> m_fonts;
};