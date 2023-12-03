// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixGamePathUtils.hpp"
#include "windows.h"
#include <utils.hpp>
#include <shlobj.h>

std::wstring PathToOutputFiles(const wchar_t *dest)
{
    std::wstring path{};

    if (path.empty()) {
        ITEMIDLIST *pidl;

        HRESULT hRes = SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl);
        if (hRes == NOERROR) {
            wchar_t lpPath[MAX_PATH];
            SHGetPathFromIDListW(pidl, lpPath);

            path = utils::format(L"%ls\\SpaceRangersHD", lpPath);
            CreateDirectoryW(path.c_str(), NULL);
            path += L"\\";
            path += dest;
        }
        else {
            path = utils::format(L".\\%ls", dest);
        }
    }

    return path;
}
