// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "Base.pch"

#include <cwchar>

#include "CWStr.hpp"
#include "CException.hpp"
#include "CStr.hpp"
#include "CBuf.hpp"

namespace Base {

HKEY Reg_OpenKey(HKEY key, const wchar *path, dword access) {
    int len = WStrLen(path);
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
        rz = RegOpenKeyExA(key, CWStr(path).toCStr().c_str(), 0, access, &kkey);

    if (rz != ERROR_SUCCESS)
        return 0;
    return kkey;
}

HKEY Reg_CreateKey(HKEY key, const wchar *path, dword access) {
    int len = WStrLen(path);
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
        rz = RegCreateKeyExA(key, CWStr(path).toCStr().c_str(), 0, NULL, 0, access, NULL, &kkey, &dv);

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
        rz = RegQueryValueExA(key, CWStr(name).toCStr().c_str(), 0, ltype, NULL, &size);
    if (rz != ERROR_SUCCESS)
        return false;
    if (size == 0)
        return false;

    buf->Len(size);
    if (GetVersion() < 0x80000000)
        rz = RegQueryValueExW(key, name, 0, ltype, (byte *)buf->Get(), &size);
    else
        rz = RegQueryValueExA(key, CWStr(name).toCStr().c_str(), 0, ltype, (byte *)buf->Get(), &size);
    if (rz != ERROR_SUCCESS)
        return false;

    return true;
}

BASE_API void Reg_GetString(HKEY pkey, const wchar *path, const wchar *name, CWStr &str) {
    HKEY kkey;
    dword type;

    if ((kkey = Reg_OpenKey(pkey, path, KEY_READ)) == 0)
        return;

    CBuf buf;
    if (Reg_GetData(kkey, name, &type, &buf)) {
        if (type == REG_SZ || type == REG_MULTI_SZ) {
            if (GetVersion() < 0x80000000)
                str.Set((wchar *)buf.Get(), buf.Len() / 2 - 1);
            else
                str.Set((const char*)buf.Get());
        }
        else if (type == REG_DWORD && buf.Len() == 4) {
            str.Set(*((dword *)buf.Get()));
        }
    }

    RegCloseKey(kkey);
}

BASE_API CWStr Reg_GetString(HKEY pkey, const wchar *path, const wchar *name, const wchar *) {
    CWStr str;

    Reg_GetString(pkey, path, name, str);

    return str;
}

BASE_API void Reg_SetString(HKEY pkey, const wchar *path, const wchar *name, const wchar *str) {
    HKEY kkey;

    if ((kkey = Reg_CreateKey(pkey, path, KEY_WRITE)) == 0)
        return;

    if (GetVersion() < 0x80000000)
        RegSetValueExW(kkey, name, 0, REG_SZ, (byte *)str, WStrLen(str) * 2 + 2);
    else
        RegSetValueExA(kkey, CWStr(name).toCStr().c_str(), 0, REG_SZ, (byte*)CWStr(str).toCStr().c_str(), WStrLen(str) + 1);

    RegCloseKey(kkey);
}

}  // namespace Base
