
#include "RangersText.hpp"

#include "Text/Font.hpp"

#include <utils.hpp>
#include <stupid_logger.hpp>

#include <d3dx9tex.h>

extern IDirect3DDevice9* g_D3DD;

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

bool icase_starts_with(std::wstring_view text, std::wstring_view prefix)
{
    if (text.length() < prefix.length())
    {
        return false;
    }

    for (size_t i = 0; i < prefix.length(); ++i)
    {
        if (std::towlower(text[i]) != std::towlower(prefix[i]))
        {
            return false;
        }
    }

    return true;
}

D3DCOLOR GetColorFromTag(std::wstring_view text, D3DCOLOR defaultColor)
{
    if (icase_starts_with(text, L"<color="))
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

int GetTextWidth(LPD3DXFONT pFont, std::wstring_view text)
{
    const DWORD tformat = DT_CALCRECT | DT_NOCLIP | DT_SINGLELINE;
    RECT rr;
    pFont->DrawTextW(NULL, text.data(), text.length(), &rr, tformat, 0);
    return rr.right;
}

struct Token
{
    std::wstring_view text;
    uint32_t color{0};
    RECT rect{0,0,0,0};
};

std::vector<Token> parse_tokens(std::wstring_view str, LPD3DXFONT pFont)
{
    std::vector<Token> result;

    size_t pos = 0;
    while((pos = str.find_first_of(L" \r"), pos) != std::string::npos)
    {
        if (str[pos] == L' ') // space - just split words
        {
            result.emplace_back(str.substr(0, pos));
            result.emplace_back(L" ");
            str.remove_prefix(pos + 1);
        }
        else if(str[pos] == L'\r') // new line
        {
            result.emplace_back(str.substr(0, pos));
            result.emplace_back(L"\r\n");
            str.remove_prefix(pos + 2);
        }
    }

    result.emplace_back(str);

    ////////////////////////////////////////////////////////////////////
    // special case for "<Color=247,195,0>+</color><Color=247,195,0>3</Color>"
    pos = str.find(L"><");
    if (pos != std::wstring::npos)
    {
        auto sub = str.substr(pos + 1);
        str.remove_suffix(str.length() - pos - 1);
        result.clear();
        result.emplace_back(str);
        result.emplace_back(sub);
    }
    ////////////////////////////////////////////////////////////////////

    bool in_color_tag = false;
    uint32_t color = 0;
    for (auto& token : result)
    {
        if (token.text == L" " || token.text == L"\r\n")
        {
            continue;
        }

        std::wstring_view text = token.text;

        if (!in_color_tag)
        {
            color = 0;
        }

        if (icase_starts_with(text, L"<color"))
        {
            in_color_tag = true;
            color = GetColorFromTag(text, 0);
            text.remove_prefix(text.find(L">") + 1);
        }

        auto pos = text.find(L"</");
        if (pos != std::wstring::npos)
        {
            in_color_tag = false;
            text.remove_suffix(text.length() - pos);
        }

        token.color = color;
        token.text = text;

        const DWORD tformat = DT_CALCRECT | DT_SINGLELINE | DT_NOCLIP;
        pFont->DrawTextW(NULL, token.text.data(), token.text.length(), &token.rect, tformat, 0);
    }

    return result;
}

size_t calc_lines(const std::vector<Token>& text, LPD3DXFONT pFont, const RECT &rect)
{
    const size_t line_width = rect.right - rect.left;

    size_t lines = 1;
    size_t cur_width = 0;
    for (auto& token : text)
    {
        if (token.text == L"\r\n")
        {
            cur_width = 0;
            lines++;
        }
        else if (token.text == L" ")
        {
            if (cur_width != 0) // if not a new line
            {
                cur_width += GetFontSpaceSize(pFont);
            }
        }
        else
        {
            if (cur_width + token.rect.right < line_width)
            {
                cur_width += token.rect.right;
            }
            else
            {
                cur_width = 0;
                lines++;
            }
        }
    }

    return lines;
}

void DrawRangersText(const std::vector<Token>& text, LPD3DXFONT pFont, const RECT &rect, DWORD format, D3DCOLOR defaultColor)
{
    const int lineHeight = GetFontHeight(pFont);

    size_t x = rect.left;
    size_t y = rect.top;
    for (const auto& token : text)
    {
        if (token.text == L"\r\n")
        {
            x = rect.left;
            y += lineHeight;
        }
        else if (token.text == L" ")
        {
            x += GetFontSpaceSize(pFont);
        }
        else
        {
            RECT resRect = rect;
            resRect.left = x;
            resRect.top = y;

            if (x + token.rect.right < rect.right)
            {
                x += token.rect.right;
            }
            else
            {
                x = rect.left + token.rect.right - token.rect.left; // length of the word
                y += lineHeight;

                resRect.left = rect.left;
                resRect.top = y;
            }

            uint32_t color = token.color ? token.color : defaultColor;
            pFont->DrawTextW(NULL, token.text.data(), token.text.size(), &resRect, format, color);
        }
    }

    return;
}

} // namespace

void RenderText(
    std::wstring_view text,
    std::wstring_view font,
    uint32_t color,
    int sizex,
    int sizey,
    int alignx,
    int aligny,
    int wordwrap,
    int smex,
    int smy,
    const Base::CRect& clipr,
    CBitmap& dst)
{
    (void)wordwrap;

    LPD3DXFONT pFont = GetFont(g_D3DD, font);

    if (!pFont)
    {
        lgr.error("Failed to load font: {}")(utils::from_wstring(font.data()));
        return;
    }
    RECT clipRect{clipr.left, clipr.top, clipr.right, clipr.bottom};
    clipRect.left += smex + 2;
    clipRect.top += smy;
    clipRect.right -= 2;
    // clipRect.bottom -= smy;

    auto tokens = parse_tokens(text, pFont);

    if (sizey == 0)
    {
        sizey = calc_lines(tokens, pFont, clipRect) * GetFontHeight(pFont);
    }

    if (clipRect.bottom == 0)
    {
        clipRect.bottom = sizey;
    }

    // Prepare texture
    IDirect3DTexture9* texture{nullptr};
    auto res = g_D3DD->CreateTexture(sizex, sizey, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture,
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

    if (FAILED(res))
    {
        return;
    }

    /////////////////////////////////////////////////////////
    // ACHTUNG: dirtiest trick for robot building menu text
    if (sizey == 18)
    {
        clipRect.bottom -= 1;
        aligny = 2;
    }
    /////////////////////////////////////////////////////////

    // Render text
    DWORD format = DT_NOCLIP;

    switch (alignx)
    {
        case 0:
            format = format | DT_LEFT;
            break;
        case 1:
            format = format | DT_CENTER;
            break;
        case 2:
            format = format | DT_RIGHT;
            break;
        case 3:
            // auto (full width) should be here
        default:
            break;
    }

    switch (aligny)
    {
        case 0:
            format = format | DT_TOP;
            break;
        case 1:
            format = format | DT_VCENTER;
            break;
        case 2:
            format = format | DT_BOTTOM;
            break;
        default:
            break;
    }

    if (text == GetTextWithoutTags(text) && GetTextWidth(pFont, text) <= clipRect.right - clipRect.left)
    {
        // simple text optimization
        lgr.debug("Optimized render for: {}")(utils::from_wstring(text));
        pFont->DrawTextW(NULL, text.data(), text.size(), &clipRect, DT_NOCLIP | DT_SINGLELINE | format, color);
    }
    else
    {
        DrawRangersText(tokens, pFont, clipRect, format, color);
    }


    texture->UnlockRect(0);
    pSurfaceRender->EndScene(0);

    CBitmap tmp;
    tmp.CreateRGBA(sizex, sizey, rect.Pitch, (uint8_t*)rect.pBits);
    tmp.BitmapDuplicate(dst);

    pSurfaceRender->Release();
    surface->Release();
    texture->Release();
}