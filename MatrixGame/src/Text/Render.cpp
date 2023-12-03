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
        m_obj->Release();
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

void DrawComplexText(const std::vector<Text::Token>& text, Text::Font& font, const RECT &rect, DWORD format, D3DCOLOR defaultColor)
{
    const int lineHeight = font.GetHeight();

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
            x += font.GetSpaceWidth();
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
            font->DrawTextW(NULL, token.text.data(), token.text.size(), &resRect, format, color);
        }
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
    tmp.BitmapDuplicate(dst);
}

} // namespace Text