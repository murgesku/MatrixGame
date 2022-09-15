#pragma once

#include <string>
#include <cstdio>
#include <stdexcept>
#include <locale>
#include <codecvt>

namespace utils {

template<typename... Args>
inline std::string format(const char* format, Args... args)
{
    // TODO: replace with std::format/fmt::format
    char buf[10240];
    if (std::snprintf(buf, sizeof(buf), format, args...) < 0)
    {
        throw std::runtime_error("snprintf() failed");
    }
    return std::string{buf};
}

template<typename... Args>
inline std::wstring format(const wchar_t* format, Args... args)
{
    // TODO: replace with std::format/fmt::format
    wchar_t buf[10240];
    if (std::swprintf(buf, sizeof(buf), format, args...) < 0)
    {
        throw std::runtime_error("swprintf() failed");
    }
    return std::wstring{buf};
}

inline std::string from_wstring(const std::wstring& wstr)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

inline std::wstring to_wstring(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

} // namespace utils