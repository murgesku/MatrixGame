#pragma once

#include <string>
#include <cstdio>
#include <stdexcept>
#include <locale>
#include <codecvt>

namespace utils {

template<typename... Args>
std::string format(const char* format, Args... args)
{
    // TODO: replace with std::format/fmt::format
    char buf[10240];
    if (std::snprintf(buf, sizeof(buf), format, args...) < 0)
    {
        throw std::runtime_error("sprintf_s() failed");
    }
    return std::string{buf};
}

std::string from_wstring(const std::wstring& wstr)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

std::wstring to_wstring(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

} // namespace utils