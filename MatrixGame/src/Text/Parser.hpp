#pragma once

#include <Text/Font.hpp>
#include <stupid_logger.hpp>

#include <D3dx9core.h>

#include <string>

namespace Text {

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

struct Token
{
    std::wstring_view text;
    uint32_t color{0};
    RECT rect{0,0,0,0};
};

std::vector<Token> parse_tokens(std::wstring_view str, Font& font)
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
    // trick for "<Color=247,195,0>+</color><Color=247,195,0>3</Color>"
    pos = str.find(L"><");
    if (pos != std::wstring::npos)
    {
        result.clear();
        auto noTagsText = GetTextWithoutTags(str);
        auto color = GetColorFromTag(str, 0);
        if (noTagsText == L"+3")
        {
            result.emplace_back(L"+3", color);
        }
        else if (noTagsText == L"+10")
        {
            result.emplace_back(L"+10", color);
        }
        else
        {
            throw std::runtime_error("You shouldn't see this");
        }
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
            color = token.color;
        }

        if (icase_starts_with(text, L"<color"))
        {
            in_color_tag = true;
            color = GetColorFromTag(text, token.color);
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
        font->DrawTextW(NULL, token.text.data(), token.text.length(), &token.rect, tformat, 0);
    }

    return result;
}

size_t calc_lines(const std::vector<Token>& text, Font& font, const RECT &rect)
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
                cur_width += font.GetSpaceWidth();
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

} // namespace Text