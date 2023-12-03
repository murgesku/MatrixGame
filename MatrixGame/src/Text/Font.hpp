#pragma once

#include <stupid_logger.hpp>

#include <D3dx9core.h>

#include <string>
#include <map>

namespace Text {

class Font
{
public:
    // no copy, no move
    Font(const Font&) = delete;
    Font(Font&&) = delete;
    Font& operator = (const Font&) = delete;
    Font& operator = (Font&&) = delete;

    Font(LPD3DXFONT font)
    : m_font{font}
    {
    }

    ~Font()
    {
        if (m_font)
        {
            m_font->Release();
        }
    }

    LPD3DXFONT operator -> () const
    {
        return m_font;
    }

    explicit operator bool () const
    {
        return !(m_font == nullptr);
    }

    size_t GetHeight() const
    {
        D3DXFONT_DESCW desc;
        m_font->GetDescW(&desc);
        return desc.Height;
    }

    size_t GetSpaceWidth() const
    {
        SIZE size{};
        GetTextExtentPoint32W(m_font->GetDC(), L" ", 1, &size);
        return size.cx;
    }

    size_t CalcTextWidth(std::wstring_view text)
    {
        const DWORD format = DT_CALCRECT | DT_NOCLIP | DT_SINGLELINE;
        RECT rect;
        m_font->DrawTextW(NULL, text.data(), text.length(), &rect, format, 0);
        return rect.right;
    }

private:
    LPD3DXFONT m_font{nullptr};
};

Font& GetFont(IDirect3DDevice9* device, std::wstring_view font_name)
{
    static std::map<std::wstring, Font> m_fonts;
    const std::wstring font_name_str{font_name};

    LPD3DXFONT font{nullptr};
    if (!m_fonts.contains(font_name_str))
    {
        if(font_name == L"Font.1Normal")
        {
            D3DXCreateFontW(device, 13, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
                            DEFAULT_PITCH, L"Verdana", &font);
        }
        else if(font_name == L"Font.2Mini")
        {
            D3DXCreateFontW(device, 12, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
                            DEFAULT_PITCH, L"Verdana", &font);
        }
        else if(font_name == L"Font.2Small")
        {
            D3DXCreateFontW(device, 13, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
                            DEFAULT_PITCH, L"Verdana", &font);
        }
        else if(font_name == L"Font.2Normal")
        {
            D3DXCreateFontW(device, 14, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
                            DEFAULT_PITCH, L"Verdana", &font);
        }
        else if(font_name == L"Font.2Ranger")
        {
            D3DXCreateFontW(device, 10, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, DEFAULT_QUALITY,
                            DEFAULT_PITCH, L"Rangers", &font);
        }
        else
        {
            throw std::runtime_error("Unknown font: " + utils::from_wstring(font_name));
        }

        m_fonts.emplace(std::wstring{font_name}, font);
    }

    lgr.info("Get font: {}")(utils::from_wstring(font_name));
    return m_fonts.at(font_name_str);
}

} // namespace Text
