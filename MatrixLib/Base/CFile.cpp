// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <cstdint>

#include "CFile.hpp"
#include "CHeap.hpp"
#include "CException.hpp"
#include "Pack.hpp"

#include <utils.hpp>

namespace Base {

#ifndef MAXEXP_EXPORTS
CPackCollection *CFile::m_Packs;
int CFile::m_PacksRef;

void CFile::AddPackFile(const wchar *name) {
    if (m_Packs == NULL) {
        m_Packs = HNew(nullptr) CPackCollection(nullptr);
    }

    m_Packs->AddPacketFile(name);
}
void CFile::OpenPackFiles(void) {
    if (m_Packs) {
        if (!m_Packs->OpenPacketFiles()) {
            ReleasePackFiles();
        }
    }
}

void CFile::ReleasePackFiles(void) {
    if (m_Packs) {
        ASSERT(m_PacksRef == 0);

        HDelete(CPackCollection, m_Packs, m_Packs->m_Heap);
        m_Packs = NULL;
    }
}
#endif

CFile::CFile() : CMain(), m_FileName{} {
#ifndef MAXEXP_EXPORTS
    m_PackHandle = 0xFFFFFFFF;
#endif
    m_Handle = INVALID_HANDLE_VALUE;
    m_Open = 0;
}

CFile::CFile(const std::wstring &filename) : CMain(), m_FileName{} {
#ifndef MAXEXP_EXPORTS
    m_PackHandle = 0xFFFFFFFF;
#endif
    m_Handle = INVALID_HANDLE_VALUE;
    m_Open = 0;
    Init(filename);
}

CFile::CFile(const wchar *filename) : CMain(), m_FileName{} {
#ifndef MAXEXP_EXPORTS
    m_PackHandle = 0xFFFFFFFF;
#endif
    m_Handle = INVALID_HANDLE_VALUE;
    m_Open = 0;
    Init(filename);
}

CFile::CFile(const wchar *filename, int len) : CMain(), m_FileName{} {
#ifndef MAXEXP_EXPORTS
    m_PackHandle = 0xFFFFFFFF;
#endif
    m_Handle = INVALID_HANDLE_VALUE;
    m_Open = 0;
    Init(filename, len);
}

CFile::~CFile() {
    Clear();
}

void CFile::Clear(void) {
    m_Open = 0;
    Close();
    m_FileName.clear();
}

void CFile::Init(const wchar *filename, int len) {
    Clear();
    m_FileName = std::wstring{filename, static_cast<size_t>(len)};
}

void CFile::Open(DWORD shareMode) {
    if (m_Open == 0) {
        if (IS_UNICODE()) {
            m_Handle = CreateFileW(m_FileName.c_str(), GENERIC_READ | GENERIC_WRITE, shareMode, NULL,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        }
        else {
            m_Handle = CreateFileA(utils::from_wstring(m_FileName).c_str(), GENERIC_READ | GENERIC_WRITE,
                                   shareMode, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        }
        if (m_Handle == INVALID_HANDLE_VALUE) {
            // only real files can be opened for read/WRITE mode

            ERROR_S2(L"Error open file: ", m_FileName.c_str());
        }
    }
    m_Open++;
}

void CFile::OpenRead(DWORD shareMode) {
    if (m_Open == 0) {
        if (IS_UNICODE()) {
            m_Handle = CreateFileW(m_FileName.c_str(), GENERIC_READ, shareMode, NULL, OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL, NULL);
        }
        else {
            m_Handle = CreateFileA(utils::from_wstring(m_FileName).c_str(), GENERIC_READ, shareMode, NULL,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        }

#ifndef MAXEXP_EXPORTS
        if (m_Handle == INVALID_HANDLE_VALUE) {
            // so, real file not found. may be it is in packet?

            if (m_Packs) {
                m_PackHandle = m_Packs->Open(utils::from_wstring(m_FileName.c_str()));
            }

            if (m_PackHandle == 0xFFFFFFFF) {
                ERROR_S2(L"Error open file: ", m_FileName.c_str());
            }
            else {
                ++m_PacksRef;
            }
        }
#endif
    }
    m_Open++;
}

bool CFile::OpenReadNE(DWORD shareMode) {
    if (m_Open == 0) {
        if (IS_UNICODE()) {
            m_Handle = CreateFileW(m_FileName.c_str(), GENERIC_READ, shareMode, NULL, OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL, NULL);
        }
        else {
            m_Handle = CreateFileA(utils::from_wstring(m_FileName.c_str()).c_str(), GENERIC_READ, shareMode, NULL,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        }
        if (m_Handle == INVALID_HANDLE_VALUE) {
            return false;
        }
    }
    m_Open++;
    return true;
}

void CFile::Create(DWORD shareMode) {
    if (m_Open == 0) {
        if (IS_UNICODE()) {
            m_Handle = CreateFileW(m_FileName.c_str(), GENERIC_READ | GENERIC_WRITE, shareMode, NULL, CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL, NULL);
        }
        else {
            m_Handle = CreateFileA(utils::from_wstring(m_FileName.c_str()).c_str(), GENERIC_READ | GENERIC_WRITE,
                                   shareMode, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }
        if (m_Handle == INVALID_HANDLE_VALUE) {
            ERROR_S2(L"Error creating file: ", m_FileName.c_str());
        }
    }
    m_Open++;
}

bool CFile::CreateNE(DWORD shareMode) {
    if (m_Open == 0) {
        if (IS_UNICODE()) {
            m_Handle = CreateFileW(m_FileName.c_str(), GENERIC_READ | GENERIC_WRITE, shareMode, NULL, CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL, NULL);
        }
        else {
            m_Handle = CreateFileA(utils::from_wstring(m_FileName.c_str()).c_str(), GENERIC_READ | GENERIC_WRITE,
                                   shareMode, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }
        if (m_Handle == INVALID_HANDLE_VALUE) {
            return false;
        }
    }
    m_Open++;
    return true;
}

void CFile::Close(void) {
    --m_Open;
    if (m_Open <= 0) {
#ifndef MAXEXP_EXPORTS
        if (m_PackHandle != 0xFFFFFFFF) {
            m_Packs->Close(m_PackHandle);
            m_PackHandle = 0xFFFFFFFF;
            --m_PacksRef;
        }
#endif
        if (m_Handle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_Handle);
            m_Handle = INVALID_HANDLE_VALUE;
        }
        m_Open = 0;
    }
}

DWORD CFile::Size(void) const {
    DWORD lo = 0xFFFFFFFF;

#ifndef MAXEXP_EXPORTS
    if (m_PackHandle != 0xFFFFFFFF) {
        lo = m_Packs->GetSize(m_PackHandle);
    }
#endif

    if (m_Handle != INVALID_HANDLE_VALUE) {
        lo = GetFileSize(m_Handle, NULL);
    }

    if (lo == 0xFFFFFFFF) {
        ERROR_S(L"Error getting file size");
    }
    return lo;
}

int64_t CFile::SizeFull(void) const {
    DWORD lo = 0xFFFFFFFF, hi;

#ifndef MAXEXP_EXPORTS
    if (m_PackHandle != 0xFFFFFFFF) {
        lo = m_Packs->GetSize(m_PackHandle);
        hi = 0;
    }
#endif

    if (m_Handle != INVALID_HANDLE_VALUE) {
        lo = GetFileSize(m_Handle, &hi);
    }

    if (lo == 0xFFFFFFFF) {
        ERROR_S(L"Error getting file size");
    }
    return int64_t(DWORD(lo)) | (int64_t(hi) << 32);
}

int64_t CFile::PointerFull(void) const {
    DWORD lo = 0xFFFFFFFF;
    LONG hi;

#ifndef MAXEXP_EXPORTS
    if (m_PackHandle != 0xFFFFFFFF) {
        lo = m_Packs->GetPos(m_PackHandle);
        hi = 0;
    }
#endif

    if (m_Handle != INVALID_HANDLE_VALUE) {
        lo = SetFilePointer(m_Handle, 0, &hi, FILE_CURRENT);
    }

    if (lo == 0xFFFFFFFF) {
        ERROR_S(L"Error getting file pointer");
    }

    return int64_t(DWORD(lo)) | (int64_t(hi) << 32);
}

void CFile::PointerFull(int64_t zn, int from) const {
    LONG lo = LONG(zn & 0xffffffff), hi = DWORD(zn >> 32);

#ifndef MAXEXP_EXPORTS
    if (m_PackHandle != 0xFFFFFFFF) {
        lo = m_Packs->SetPos(m_PackHandle, lo, from);
    }
#endif

    if (m_Handle != INVALID_HANDLE_VALUE) {
        lo = SetFilePointer(m_Handle, lo, &hi, from);
    }

    if (lo == 0xFFFFFFFF) {
        ERROR_S(L"Error setting file pointer");
    }
}

DWORD CFile::Pointer(void) const {
    DWORD lo = 0xFFFFFFFF;

#ifndef MAXEXP_EXPORTS
    if (m_PackHandle != 0xFFFFFFFF) {
        lo = m_Packs->GetPos(m_PackHandle);
    }
#endif

    if (m_Handle != INVALID_HANDLE_VALUE) {
        lo = SetFilePointer(m_Handle, 0, NULL, FILE_CURRENT);
    }
    if (lo == 0xFFFFFFFF) {
        ERROR_S(L"Error getting file pointer");
    }
    return lo;
}

void CFile::Pointer(DWORD lo, int from) const {
#ifndef MAXEXP_EXPORTS
    if (m_PackHandle != 0xFFFFFFFF) {
        lo = m_Packs->SetPos(m_PackHandle, lo, from);
    }
#endif

    if (m_Handle != INVALID_HANDLE_VALUE) {
        lo = SetFilePointer(m_Handle, lo, NULL, from);
    }

    if (lo == 0xFFFFFFFF) {
        ERROR_S(L"Error setting file pointer");
    }
}

void CFile::Read(void *buf, DWORD kolbyte) {
    bool ok = false;

#ifndef MAXEXP_EXPORTS
    if (m_PackHandle != 0xFFFFFFFF) {
        ok = m_Packs->Read(m_PackHandle, buf, kolbyte);
    }
#endif

    if (m_Handle != INVALID_HANDLE_VALUE) {
        DWORD temp;
        ok = ReadFile(m_Handle, buf, kolbyte, &temp, NULL) != FALSE;
        ok &= (temp == (DWORD)kolbyte);
    }

    if (!ok) {
        ERROR_S(utils::format(L"Error read file: %ls cnt=%d", m_FileName.c_str(), kolbyte));
    }
}

void CFile::Write(void *buf, DWORD kolbyte) {
    if (kolbyte <= 0)
        return;

    ASSERT(m_Handle != INVALID_HANDLE_VALUE);

    DWORD temp;

    if (WriteFile(m_Handle, buf, kolbyte, &temp, NULL) == FALSE || temp != kolbyte) {
        ERROR_S(utils::format(L"Error write file: %ls cnt=%d", m_FileName.c_str(), kolbyte));
    }
}

static bool FileExistA(std::wstring &outname, const wchar *mname, const wchar *exts, bool withpar) {
    DTRACE();

    size_t len = std::wcslen(mname);
    const wchar *str = mname;

    size_t lenfile = 0;
    while (lenfile < len && str[lenfile] != '?')
        lenfile++;

    std::wstring filename(str, lenfile);

    WIN32_FIND_DATAA fd;
    HANDLE fh = FindFirstFileA(utils::from_wstring(filename.c_str()).c_str(), &fd);
    if (fh != INVALID_HANDLE_VALUE) {
        FindClose(fh);
        if (withpar)
            outname = mname;
        else
            outname = filename;
        return true;
    }

    fh = FindFirstFileA(utils::from_wstring(std::wstring(str, lenfile) + L".*").c_str(), &fd);
    if (fh == INVALID_HANDLE_VALUE)
        return false;
    if (exts != NULL) {
        std::wstring curname;
        while (true) {
            curname = utils::to_wstring(fd.cFileName);
            int sme = curname.rfind(L'.') + 1; // TODO: pay attention
            if (sme > 0 && sme < curname.length()) {
                utils::to_lower(curname, sme);
                const wchar *str = curname.c_str() + sme;
                int len = curname.length() - sme;

                const wchar *exts2 = exts;
                int cntok = 0;
                int lenext = 0;
                while (true) {
                    if (*exts2 == 0 || *exts2 == L'~') {
                        if (cntok == len && lenext == len)
                            break;
                        cntok = 0;
                        lenext = -1;
                        if (*exts2 == 0)
                            break;
                    }
                    else if (*exts2 == str[cntok])
                        cntok++;
                    exts2++;
                    lenext++;
                }
                if (cntok == len)
                    break;
            }

            if (!FindNextFileA(fh, &fd)) {
                FindClose(fh);
                return false;
            }
        }
    }
    FindClose(fh);

    int lenpath = lenfile;
    while (lenpath > 0 && str[lenpath - 1] != '\\' && str[lenpath - 1] != '/')
        lenpath--;

    if (lenpath > 0) {
        outname = std::wstring{str, static_cast<size_t>(lenpath)};
        outname += utils::to_wstring(fd.cFileName);
    }
    else
        outname = utils::to_wstring(fd.cFileName);

    if (withpar && lenfile < len)
        outname += std::wstring{str + lenfile, static_cast<size_t>(len - lenfile)};

    return true;
}

static bool FileExistW(std::wstring &outname, const wchar *mname, const wchar *exts, bool withpar) {
    DTRACE();

    int len = std::wcslen(mname);
    const wchar *str = mname;

    int lenfile = 0;
    while (lenfile < len && str[lenfile] != '?')
        lenfile++;

    std::wstring filename{str, static_cast<size_t>(lenfile)};

    WIN32_FIND_DATAW fd;
    HANDLE fh = FindFirstFileW(filename.c_str(), &fd);
    if (fh != INVALID_HANDLE_VALUE) {
        FindClose(fh);
        if (withpar)
            outname = mname;
        else
            outname = filename;
        return true;
    }

    fh = FindFirstFileW((std::wstring{str, static_cast<size_t>(lenfile)} + L".*").c_str(), &fd);
    if (fh == INVALID_HANDLE_VALUE)
        return false;
    if (exts != NULL) {
        std::wstring curname;
        while (true) {
            curname = fd.cFileName;
            int sme = curname.rfind(L'.') + 1; // TODO: pay attention
            if (sme > 0 && sme < curname.length()) {
                utils::to_lower(curname, sme);
                const wchar *str = curname.c_str() + sme;
                int len = curname.length() - sme;

                const wchar *exts2 = exts;
                int cntok = 0;
                int lenext = 0;
                while (true) {
                    if (*exts2 == 0 || *exts2 == L'~') {
                        if (cntok == len && lenext == len)
                            break;
                        cntok = 0;
                        lenext = -1;
                        if (*exts2 == 0)
                            break;
                    }
                    else if (*exts2 == str[cntok])
                        cntok++;
                    exts2++;
                    lenext++;
                }
                if (cntok == len)
                    break;
            }

            if (!FindNextFileW(fh, &fd)) {
                FindClose(fh);
                return false;
            }
        }
    }
    FindClose(fh);

    int lenpath = lenfile;
    while (lenpath > 0 && str[lenpath - 1] != '\\' && str[lenpath - 1] != '/')
        lenpath--;

    if (lenpath > 0) {
        outname = std::wstring{str, static_cast<size_t>(lenpath)};
        outname += fd.cFileName;
    }
    else
        outname = fd.cFileName;

    if (withpar && lenfile < len)
        outname += std::wstring{str + lenfile, static_cast<size_t>(len - lenfile)};

    return true;
}

bool CFile::FileExist(std::wstring &outname, const wchar *mname, const wchar *exts, bool withpar) {
    DTRACE();

    if (mname[0] == '.' && (mname[1] == '\\' || mname[1] == '/'))
        mname += 2;

    if (IS_UNICODE()) {
        if (FileExistW(outname, mname, exts, withpar))
            return true;
    }
    else {
        if (FileExistA(outname, mname, exts, withpar))
            return true;
    }

#ifndef MAXEXP_EXPORTS
    // real file not found... may be it is in pack file

    if (!m_Packs)
        return false;

    int len = std::wcslen(mname);
    const wchar *str = mname;

    int lenfile = 0;
    while (lenfile < len && str[lenfile] != '?')
        lenfile++;

    std::wstring filename{str, static_cast<size_t>(lenfile)};

    if (m_Packs->FileExists(utils::from_wstring(filename.c_str()).c_str())) {
        if (withpar)
            outname = mname;
        else
            outname = filename;
        return true;
    }

    if (!exts)
        return false;

    int sm0 = 0;
    int sm1 = 0;

    int l = std::wcslen(exts);

    std::string fn;
    for (; sm1 <= l; ++sm1) {
        if (exts[sm1] == '~' || exts[sm1] == 0) {
            if (sm0 != sm1) {
                fn = utils::format(
                        "%s.%s",
                        utils::from_wstring(filename.c_str()).c_str(),
                        utils::from_wstring(exts + sm0).c_str());

                fn.resize(fn.length() - (l - sm1));

                if (m_Packs->FileExists(fn)) {
                    outname = utils::to_wstring(fn);
                    if (withpar)
                        outname += (mname + lenfile);
                    return true;
                }
            }

            sm0 = sm1 + 1;
        }
    }
#endif

    return false;
}

void CFile::FindFiles(const std::wstring &folderfrom, const wchar *files, ENUM_FILES ef, DWORD user) {
    if (IS_UNICODE()) {
        std::wstring fn(folderfrom);
        if (fn.length() > 0 && !(*(fn.c_str() + fn.length() - 1) == '\\' || (*(fn.c_str() + fn.length() - 1) == '/'))) {
            fn += L"\\";
        }
        std::wstring fnf(fn);

        WIN32_FIND_DATAW fd;

        // seek files
        fnf += files;
        HANDLE h = FindFirstFileW(fnf.c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    std::wstring found(fd.cFileName);
                    ef(fn + found, user);
                }
            }
            while (FindNextFileW(h, &fd));
            FindClose(h);
        }

        fnf = fn + L"*.*";
        h = FindFirstFileW(fnf.c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    std::wstring found(fd.cFileName);
                    if (found != L"." && found != L"..") {
                        FindFiles(fn + found, files, ef, user);
                    }
                }
            }
            while (FindNextFileW(h, &fd));
            FindClose(h);
        }
    }
    else {
        std::string fn{utils::from_wstring(folderfrom.c_str())};
        if (fn.length() > 0 && !(*(fn.c_str() + fn.length() - 1) == '\\' || (*(fn.c_str() + fn.length() - 1) == '/'))) {
            fn +=  "\\";
        }
        std::string fnf{fn};

        WIN32_FIND_DATAA fd;

        // seek files
        fnf += utils::from_wstring(files);
        HANDLE h = FindFirstFileA(fnf.c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    std::wstring found(utils::to_wstring(fn + fd.cFileName));
                    ef(found, user);
                }
            }
            while (FindNextFileA(h, &fd));
            FindClose(h);
        }

        fnf = fn + "*.*";
        h = FindFirstFileA(fnf.c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    std::wstring found(utils::to_wstring(fd.cFileName));
                    if (found != L"." && found != L"..") {
                        FindFiles(utils::to_wstring(fn) + found, files, ef, user);
                    }
                }
            }
            while (FindNextFileA(h, &fd));
            FindClose(h);
        }
    }
}

}  // namespace Base
