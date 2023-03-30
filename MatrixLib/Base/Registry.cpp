// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <cwchar>

#include "Registry.hpp"

#include "CException.hpp"
#include "CBuf.hpp"

#include <utils.hpp>

namespace Base {

HKEY Reg_OpenKey(HKEY key, const wchar *path, dword access) {
    int len = std::wcslen(path);
    if (len > 5 && !std::wmemcmp(path, L"HKCR\\", 5)) {
        key = HKEY_CLASSES_ROOT;
        path += 5;
    }
    else if (len > 5 && !std::wmemcmp(path, L"HKCU\\", 5)) {
        key = HKEY_CURRENT_USER;
        path += 5;
    }
    else if (len > 5 && !std::wmemcmp(path, L"HKLM\\", 5)) {
        key = HKEY_LOCAL_MACHINE;
        path += 5;
    }

    HKEY kkey;
    dword rz;

    if (GetVersion() < 0x80000000)
        rz = RegOpenKeyExW(key, path, 0, access, &kkey);
    else
        rz = RegOpenKeyExA(key, utils::from_wstring(path).c_str(), 0, access, &kkey);

    if (rz != ERROR_SUCCESS)
        return 0;
    return kkey;
}

HKEY Reg_CreateKey(HKEY key, const wchar *path, dword access) {
    int len = std::wcslen(path);
    if (len > 5 && !std::wmemcmp(path, L"HKCR\\", 5)) {
        key = HKEY_CLASSES_ROOT;
        path += 5;
    }
    else if (len > 5 && !std::wmemcmp(path, L"HKCU\\", 5)) {
        key = HKEY_CURRENT_USER;
        path += 5;
    }
    else if (len > 5 && !std::wmemcmp(path, L"HKLM\\", 5)) {
        key = HKEY_LOCAL_MACHINE;
        path += 5;
    }

    HKEY kkey;
    dword rz;
    dword dv;

    if (GetVersion() < 0x80000000)
        rz = RegCreateKeyExW(key, path, 0, NULL, 0, access, NULL, &kkey, &dv);
    else
        rz = RegCreateKeyExA(key, utils::from_wstring(path).c_str(), 0, NULL, 0, access, NULL, &kkey, &dv);

    if (rz != ERROR_SUCCESS)
        return 0;
    return kkey;
}

bool Reg_GetData(HKEY key, const wchar *name, dword *ltype, CBuf *buf) {
    dword size;
    dword rz;

    if (GetVersion() < 0x80000000)
        rz = RegQueryValueExW(key, name, 0, ltype, NULL, &size);
    else
        rz = RegQueryValueExA(key, utils::from_wstring(name).c_str(), 0, ltype, NULL, &size);
    if (rz != ERROR_SUCCESS)
        return false;
    if (size == 0)
        return false;

    buf->Len(size);
    if (GetVersion() < 0x80000000)
        rz = RegQueryValueExW(key, name, 0, ltype, (byte *)buf->Get(), &size);
    else
        rz = RegQueryValueExA(key, utils::from_wstring(name).c_str(), 0, ltype, (byte *)buf->Get(), &size);
    if (rz != ERROR_SUCCESS)
        return false;

    return true;
}

BASE_API void Reg_GetString(HKEY pkey, const wchar *path, const wchar *name, std::wstring &str) {
    HKEY kkey;
    dword type;

    if ((kkey = Reg_OpenKey(pkey, path, KEY_READ)) == 0)
        return;

    CBuf buf;
    if (Reg_GetData(kkey, name, &type, &buf)) {
        if (type == REG_SZ || type == REG_MULTI_SZ) {
            if (GetVersion() < 0x80000000)
                str = std::wstring{(wchar*)buf.Get(), static_cast<size_t>(buf.Len() / 2 - 1)};
            else
                str = utils::to_wstring((const char*)buf.Get());
        }
        else if (type == REG_DWORD && buf.Len() == 4) {
            str = utils::format(L"%u", *((dword*)buf.Get()));
        }
    }

    RegCloseKey(kkey);
}

BASE_API std::wstring Reg_GetString(HKEY pkey, const wchar *path, const wchar *name, const wchar *) {
    std::wstring str;

    Reg_GetString(pkey, path, name, str);

    return str;
}

BASE_API void Reg_SetString(HKEY pkey, const wchar *path, const wchar *name, const wchar *str) {
    HKEY kkey;

    if ((kkey = Reg_CreateKey(pkey, path, KEY_WRITE)) == 0)
        return;

    if (GetVersion() < 0x80000000)
        RegSetValueExW(kkey, name, 0, REG_SZ, (byte *)str, std::wcslen(str) * 2 + 2);
    else
        RegSetValueExA(kkey, utils::from_wstring(name).c_str(), 0, REG_SZ, (byte*)utils::from_wstring(str).c_str(), std::wcslen(str) + 1);

    RegCloseKey(kkey);
}

}  // namespace Base
