#pragma once

#include <D3dx9core.h>

#include <string>
#include <map>

LPD3DXFONT GetFont(IDirect3DDevice9* device, std::wstring_view font_name)
{
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

    LPD3DXFONT font{nullptr};
    if (!m_fonts.contains(font_name))
    {
        if(font_name == L"Font.2Small")
        {
            D3DXCreateFontW(device, 13, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
                            DEFAULT_PITCH, L"Verdana", &font);
        }
        else if(font_name == L"Font.2Ranger")
        {
            D3DXCreateFontW(device, 10, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, DEFAULT_QUALITY,
                            DEFAULT_PITCH, L"Rangers", &font);
        }
        else if(font_name == L"Font.2Normal")
        {
            D3DXCreateFontW(device, 14, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
                            DEFAULT_PITCH, L"Verdana", &font);
        }
        else if(font_name == L"Font.2Mini")
        {
            D3DXCreateFontW(device, 12, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
                            DEFAULT_PITCH, L"Verdana", &font);
        }
        else
        {
            throw std::runtime_error("Failed to load font: " + utils::from_wstring(font_name));
        }

        m_fonts.emplace(font_name, font);
    }

    return m_fonts[font_name].m_font;
}

int GetFontSpaceSize(const LPD3DXFONT pFont)
{
    SIZE size{};
    GetTextExtentPoint32W(pFont->GetDC(), L" ", 1, &size);
    return size.cx;
}

int GetFontHeight(LPD3DXFONT pDXFont)
{
    D3DXFONT_DESCW desc;
    pDXFont->GetDescW(&desc);
    return desc.Height;
}