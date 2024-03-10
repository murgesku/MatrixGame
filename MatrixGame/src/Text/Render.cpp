#include "Text/Render.hpp"
#include "Text/Font.hpp"
#include "Text/Parser.hpp"

#include <utils.hpp>
#include <stupid_logger.hpp>

#include <d3dx9tex.h>

extern IDirect3DDevice9* g_D3DD;

namespace {

template<typename T>
class AutoRelease
{
public:
    AutoRelease(T* obj)
    : m_obj{obj}
    {}

    ~AutoRelease()
    {
        if (m_obj)
        {
            m_obj->Release();
        }
    }

    operator T* ()
    {
        return m_obj;
    }

    operator T** ()
    {
        return &m_obj;
    }

    T* operator -> ()
    {
        return m_obj;
    }

private:
    T* m_obj;
};

void DrawComplexText(
    const std::vector<Text::Token>& text,
    const Text::Font& font,
    const RECT &rect,
    const DWORD format,
    const D3DCOLOR defaultColor)
{
    const size_t lineHeight = font.GetHeight();
    const size_t spaceWidth = font.GetSpaceWidth();

    struct Point
    {
        size_t x{0};
        size_t y{0};
    };

    const auto drawLine = [&](const std::wstring& str, uint32_t color, Point pos) {
        RECT posRect = rect;
        posRect.left = pos.x;
        posRect.top = pos.y;
        uint32_t textColor = color ? color : defaultColor;
        font->DrawTextW(NULL, str.data(), str.size(), &posRect, format, textColor);
    };

    size_t x = rect.left;
    size_t y = rect.top;

    std::wstring curLine{};
    size_t curLineWidth{0};
    uint32_t curLineColor{0};

    for (const auto& token : text)
    {
        if (token.text == L"\r\n")
        {
            if (!curLine.empty())
            {
                drawLine(curLine, curLineColor, {x,y});

                curLine.clear();
                curLineWidth = 0;
                curLineColor = 0;
            }

            x = rect.left;
            y += lineHeight;
            continue;
        }
        else if (token.text == L" ")
        {
            curLine += L" ";
            curLineWidth += spaceWidth;
        }
        else
        {
            if (curLine.empty())
            {
                curLine = token.text;
                curLineColor = token.color;
                curLineWidth = token.rect.right - token.rect.left;
                continue;
            }

            // if current text + new token does not fit into the line - draw current text

            if (x + curLineWidth + token.rect.right > rect.right)
            {
                drawLine(curLine, curLineColor, {x,y});

                curLine = token.text;
                curLineColor = token.color;
                curLineWidth = token.rect.right - token.rect.left;

                x = rect.left;
                y += lineHeight;
                continue;
            }

            // if current text color is not the new token color - draw current text
            if (curLineColor != token.color)
            {
                drawLine(curLine, curLineColor, {x,y});

                x += curLineWidth;

                curLine = token.text;
                curLineColor = token.color;
                curLineWidth = token.rect.right - token.rect.left;
                continue;
            }

            curLine += token.text;
            curLineWidth += token.rect.right - token.rect.left;
        }
    }

    if (!curLine.empty())
    {
        drawLine(curLine, curLineColor, {x,y});
    }

    return;
}

} // namespace

namespace Text {

void Render(
    std::wstring_view text,
    std::wstring_view font_name,
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

    Font& font = Text::GetFont(g_D3DD, font_name);

    if (!font)
    {
        // can't render text
        return;
    }

    RECT clipRect{clipr.left, clipr.top, clipr.right, clipr.bottom};
    clipRect.left += smex + 2;
    clipRect.top += smy;
    clipRect.right -= 2;

    auto tokens = parse_tokens(text, font);

    if (sizex == 0)
    {
        sizex = tokens[0].rect.right + 4;
        clipRect.right = sizex;
    }

    if (sizey == 0)
    {
        sizey = calc_lines(tokens, font, clipRect) * font.GetHeight();
    }

    if (clipRect.bottom == 0)
    {
        clipRect.bottom = sizey;
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

    // Prepare texture
    AutoRelease<IDirect3DTexture9> texture{nullptr};
    auto res = g_D3DD->CreateTexture(sizex, sizey, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, texture, NULL);
    if (FAILED(res))
    {
        return;
    }

    AutoRelease<IDirect3DSurface9> surface{nullptr};

    texture->GetSurfaceLevel(0, surface);

    D3DLOCKED_RECT rect{};
    texture->LockRect(0, &rect, NULL, 0);

    AutoRelease<ID3DXRenderToSurface> pSurfaceRender{nullptr};

    D3DXCreateRenderToSurface(g_D3DD, sizex, sizey, D3DFMT_A8R8G8B8, 0, D3DFMT_UNKNOWN, pSurfaceRender);

    pSurfaceRender->BeginScene(surface, NULL);

    res = g_D3DD->Clear(0, 0, D3DCLEAR_TARGET, 0, 0, 0);

    if (FAILED(res))
    {
        return;
    }

    if (icase_find(text, COLOR_TAG_START) == std::wstring::npos)
    {
        // simple text optimization
        font->DrawTextW(NULL, text.data(), text.size(), &clipRect, DT_NOCLIP | DT_WORDBREAK | format, color);
    }
    else
    {
        DrawComplexText(tokens, font, clipRect, format, color);
    }

    texture->UnlockRect(0);
    pSurfaceRender->EndScene(0);

    CBitmap tmp;
    tmp.CreateRGBA(sizex, sizey, rect.Pitch, (uint8_t*)rect.pBits);

    // tmp now does not own the image bytes, so we need to duplicate it
    dst.CreateRGBA(sizex, sizey);
    dst.Copy(CPoint(0, 0), tmp.Size(), tmp, CPoint(0, 0));
}

} // namespace Text