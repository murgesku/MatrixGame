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

    struct
    {
        std::wstring text{};
        size_t width{0};
        uint32_t color{0};

        void operator = (const Text::Token& token)
        {
            text = token.text;
            color = token.color;
            width = token.width;
        }

        void clear()
        {
            text.clear();
            width = 0;
            color = 0;
        }
    } current;

    const auto drawLine = [&](const std::wstring& str, uint32_t color, Point pos) {
        RECT posRect = rect;
        posRect.left = pos.x;
        posRect.top = pos.y;
        uint32_t textColor = color ? color : defaultColor;
        font->DrawTextW(NULL, str.data(), str.size(), &posRect, format, textColor);
    };

    size_t x = rect.left;
    size_t y = rect.top;

    for (const auto& token : text)
    {
        if (token.text == L"\r\n")
        {
            if (!current.text.empty())
            {
                drawLine(current.text, current.color, {x,y});
                current.clear();
            }

            x = rect.left;
            y += lineHeight;
            continue;
        }
        else if (token.text == L" ")
        {
            current.text += L" ";
            current.width += spaceWidth;
            continue;
        }

        if (current.text.empty())
        {
            current = token;
            continue;
        }

        // if current text + new token does not fit into the line - draw current text
        if (x + current.width + token.width > rect.right)
        {
            drawLine(current.text, current.color, {x,y});

            current = token;

            x = rect.left;
            y += lineHeight;
            continue;
        }

        // if current text color is not the new token color - draw current text
        if (current.color != token.color)
        {
            drawLine(current.text, current.color, {x,y});

            x += current.width;

            current = token;
            continue;
        }

        current.text += token.text;
        current.width += token.width;
    }

    if (!current.text.empty())
    {
        drawLine(current.text, current.color, {x,y});
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
        sizex = tokens[0].width + 4;
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