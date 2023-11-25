
#include "RangersText.hpp"

#include <utils.hpp>
#include <stupid_logger.hpp>

#include <list>
#include <cstdio>

extern IDirect3DDevice9* g_D3DD;

std::map<std::wstring_view, RangersText::Font> RangersText::m_fonts;

namespace {

std::wstring GetTextWithoutTags(std::wstring_view text)
{
    std::wstring result;
    bool insideTag = false;

    for (const wchar_t c : text)
    {
        if (c == L'<')
        {
            insideTag = true;
        }
        else if (c == L'>')
        {
            insideTag = false;
        }
        else if (!insideTag)
        {
            result += c;
        }
    }
    return result;
}

D3DCOLOR GetColorFromTag(std::wstring text, D3DCOLOR defaultColor)
{
    utils::to_lower(text);
    if (text.starts_with(L"<color="))
    {
        try
        {
            const size_t rEnd = text.find(L',', 7);
            const size_t gEnd = text.find(L',', rEnd + 1);

            std::size_t pos{};

            const int a = 255;
            const int r = std::stoi(&text[7], &pos);
            const int g = std::stoi(&text[rEnd + 1], &pos);
            const int b = std::stoi(&text[gEnd + 1], &pos);

            return a << 24 | r << 16 | g << 8 | b;
        }
        catch (const std::exception& e)
        {
            lgr.error("Failed to parse color from text: {}")(utils::from_wstring(std::wstring{text}));
        }
    }
    return defaultColor;
}

int GetFontSpaceSize(const LPD3DXFONT pFont)
{
    SIZE size = {};
    GetTextExtentPoint32W(pFont->GetDC(), L" ", 1, &size);
    return size.cx;
}

int GetFontHeight(LPD3DXFONT pDXFont)
{
    D3DXFONT_DESCW desc;
    pDXFont->GetDescW(&desc);
    // lgr.info("Font height: {}")(desc.Height);
    return desc.Height;
}

std::vector<std::wstring> split(std::wstring str, const std::wstring& delim)
{
    std::vector<std::wstring> result;

    size_t pos;
    while((pos = str.find(delim)) != std::string::npos)
    {
        result.push_back(str.substr(0, pos));
        str.erase(0, pos + delim.length());
    }

    result.push_back(str);
    return result;
}

struct Word
{
    std::wstring text;
    uint32_t color;
};

using Line = std::vector<Word>;

std::vector<Line> prepareText(std::wstring_view text, LPD3DXFONT pFont, const RECT& rect)
{
    std::wstring textWithoutTags{text}; // GetTextWithoutTags(text);

    // TODO: this function assumes there is always only one word in color tag.
    //       this is not true, so to be fixed

    std::vector<Line> lines;
    const DWORD tformat = DT_CALCRECT | DT_WORDBREAK | DT_NOCLIP | DT_SINGLELINE;

    for (const auto& line : split(textWithoutTags, L"\r\n"))
    {
        const auto words = split(line, L" ");
        std::wstring str;

        Line words_in_line;

        for (const auto& wordRaw : words)
        {
            std::wstring word = GetTextWithoutTags(wordRaw);
            uint32_t color = GetColorFromTag(wordRaw, 0);
            RECT resRect = rect;
            std::wstring test = str.empty() ? word : str + L" " + word;
            pFont->DrawTextW(NULL, test.c_str(), test.length(), &resRect, tformat, 0);

            if (resRect.right <= rect.right)
            {
                str = test;
                words_in_line.emplace_back(word, color);
            }
            else
            {
                if (str.empty())
                {
                    lgr.fatal("{} > {}")(resRect.right, rect.right);
                    throw std::runtime_error("String didn't fit into provided rect");
                }

                lines.push_back(words_in_line);
                words_in_line.clear();

                // TODO: potential bug here, if one words fits the line, but two already not
                str = word;
                words_in_line.emplace_back(word, color);
            }
        }

        if (!str.empty())
        {
            lines.push_back(words_in_line);
            words_in_line.clear();
            str.clear();
        }
    }

    return lines;
}

void DrawRangersText(const std::vector<Line> lines, LPD3DXFONT pFont, RECT &rect, DWORD format, D3DCOLOR defaultColor)
{
    (void)format;
    (void)defaultColor;

    const int lineHeight = GetFontHeight(pFont) + -1;
    const DWORD rformat = DT_NOCLIP | DT_SINGLELINE | format;
    size_t i = 0;
    for (const auto& line : lines)
    {
        size_t y = rect.left;
        for (const auto& word : line)
        {
            RECT resRect = rect;
            resRect.top += lineHeight * i;
            resRect.left = y;

            const DWORD tformat = DT_CALCRECT | format;
            pFont->DrawTextW(NULL, word.text.c_str(), word.text.length(), &resRect, tformat, 0);
            y = resRect.right + GetFontSpaceSize(pFont);

            uint32_t color = word.color ? word.color : defaultColor;
            pFont->DrawTextW(NULL, word.text.c_str(), word.text.size(), &resRect, rformat, color);
        }
        i++;
    }

    return;
}

} // namespace

void RangersText::CreateText(std::wstring_view text, std::wstring_view font, uint32_t color, int sizex, int sizey, int alignx,
                             int aligny, int wordwrap, int smex, int smy, const Base::CRect& clipr,
                             CBitmap& dst)
{
    int res = 0;

    LPD3DXFONT pFont = GetFont(font);

    if (!pFont)
    {
        lgr.error("Failed to load font");
        return;
    }
    RECT clipRect{clipr.left, clipr.top, clipr.right, clipr.bottom};
    clipRect.left += smex + 2;
    clipRect.top += smy;
    clipRect.right -= 2;
    // clipRect.bottom -= smy;

    const auto lines = prepareText(text, pFont, clipRect);

    if (sizey == 0)
    {
        const int lineHeight = GetFontHeight(pFont) + -1;
        sizey = lines.size() * lineHeight + 2;
    }

    // Prepare texture
    IDirect3DTexture9* texture{nullptr};
    res = g_D3DD->CreateTexture(sizex, sizey, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture,
                                NULL);
    if (FAILED(res)) {
        return;
    }

    IDirect3DSurface9 *surface;

    texture->GetSurfaceLevel(0, &surface);

    D3DLOCKED_RECT rect{};
    texture->LockRect(0, &rect, NULL, 0);

    memset(rect.pBits, 0, sizey * rect.Pitch);

    LPD3DXRENDERTOSURFACE pSurfaceRender = nullptr;

    D3DXCreateRenderToSurface(g_D3DD, sizex, sizey, D3DFMT_A8R8G8B8, 0, D3DFMT_UNKNOWN, &pSurfaceRender);

    pSurfaceRender->BeginScene(surface, NULL);

    res = g_D3DD->Clear(0, 0, D3DCLEAR_TARGET, 0, 0, 0);

    if (FAILED(res)) {
        return;
    }

    // Render text

    DWORD format = DT_NOCLIP;
    switch (alignx) {
        case 1:
            format = format | DT_CENTER;
            break;
        case 2:
            format = format | DT_RIGHT;
            break;
        default:
            format = format | DT_LEFT;
            break;
    }

    switch (aligny) {
        case 1:
            format = format | DT_VCENTER;
            // format = format | DT_BOTTOM;
            break;
        case 2:
            format = format | DT_BOTTOM;
            break;
        default:
            format = format | DT_TOP;
            break;
    }

    if (wordwrap)
        format = format | DT_WORDBREAK;

    DrawRangersText(lines, pFont, clipRect, format, color);

    texture->UnlockRect(0);
    pSurfaceRender->EndScene(0);

    // Unload resource

    pSurfaceRender->Release();
    surface->Release();

    CBitmap tmp;
    tmp.CreateRGBA(sizex, sizey, rect.Pitch, (uint8_t*)rect.pBits);
    tmp.BitmapDuplicate(dst);

    texture->Release();
}

LPD3DXFONT RangersText::GetFont(std::wstring_view font_name)
{
    LPD3DXFONT font{nullptr};
    if (!m_fonts.contains(font_name))
    {
        if(font_name == L"Font.2Small")
        {
            D3DXCreateFontW(g_D3DD, 13, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
                            DEFAULT_PITCH, L"Verdana", &font);
        }
        else if(font_name == L"Font.2Ranger")
        {
            D3DXCreateFontW(g_D3DD, 10, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, DEFAULT_QUALITY,
                            DEFAULT_PITCH, L"Rangers", &font);
        }
        // else if(font_name == L"Font.2Normal")
        // {
        // }
        else
        {
            D3DXCreateFontW(g_D3DD, 13, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
                            DEFAULT_PITCH, L"Verdana", &font);
        }

        m_fonts.emplace(font_name, font);
    }

    return m_fonts[font_name].m_font;
}