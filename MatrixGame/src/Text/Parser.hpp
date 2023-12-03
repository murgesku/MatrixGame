#pragma once

#include <Text/Font.hpp>

#include <string>
#include <vector>
#include <cstdint>

namespace Text {

constexpr std::wstring_view COLOR_TAG_START{L"<color="};
constexpr std::wstring_view COLOR_TAG_END{L"</color>"};

bool icase_starts_with(std::wstring_view text, std::wstring_view prefix);
size_t icase_find(std::wstring_view text, std::wstring_view prefix);

struct Token
{
    std::wstring_view text;
    uint32_t color{0};
    RECT rect{0,0,0,0};
};

std::vector<Token> parse_tokens(std::wstring_view str, Font& font);
size_t calc_lines(const std::vector<Token>& text, Font& font, const RECT &rect);

} // namespace Text