// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "CMain.hpp"
#include "CHeap.hpp"
#include "CException.hpp"
#include "CWStr.hpp"
#include "CBuf.hpp"
#include "Tracer.hpp"

namespace Base {

class CBlockPar;
class BPCompiler;

class ParamParser : public std::wstring
{
public:
    ParamParser(const std::wstring& str)
    : std::wstring{str}
    {
    }

    ParamParser() = default;
    ~ParamParser() = default;

    int GetCountPar(const wchar *ogsim) const;

    int GetInt(void) const;
    DWORD GetDword(void) const;
    double GetDouble(void) const;
    int GetHex(void) const;
    DWORD GetHexUnsigned(void) const;

    bool IsOnlyInt(void) const;


    // Функции для работы с параметрами
    // Примеры :
    //      Str="count=5,7"    GetCountPar("=,")      return 3
    //      Str="count=5,7"    GetStrPar(str,1,"=")   str="5,7"
    //      Str="count=5,7"    GetIntPar(2,"=,")      return 7

private:
    int GetSmePar(int np, const wchar *ogsim) const;
    int GetLenPar(int smepar, const wchar *ogsim) const;

public:
    ParamParser GetStrPar(int np, const wchar *ogsim) const {
        int sme = GetSmePar(np, ogsim);
        return std::wstring(c_str() + sme, GetLenPar(sme, ogsim));
    }

    ParamParser GetStrPar(int nps, int npe, const wchar *ogsim) const;
    int GetIntPar(int np, const wchar *ogsim) const { return GetStrPar(np, ogsim).GetInt(); }
    double GetDoublePar(int np, const wchar *ogsim) const { return GetStrPar(np, ogsim).GetDouble(); }
    bool GetTrueFalsePar(int np, const wchar *ogsim) const;
};

class BASE_API CBlockParUnit : public CMain {
    friend CBlockPar;
    friend BPCompiler;

private:
    CHeap *m_Heap;

    CBlockParUnit *m_Prev;
    CBlockParUnit *m_Next;
    CBlockPar *m_Parent;

    int m_Type;  // 0-empty 1-par 2-block
    std::wstring m_Name;
    std::wstring m_Com;
    union {
        std::wstring *m_Par;
        CBlockPar *m_Block;
    };

    int m_FastFirst;  // Смещение до первого элемента с одинаковым именем
    int m_FastCnt;  // Количество с одинаковым именем. Инициализировано только для первого
public:
    CBlockParUnit(CHeap *heap = NULL);
    ~CBlockParUnit();

    void Clear(void);
    void ChangeType(int nt);

    void CopyFrom(CBlockParUnit &bp);
};

class BASE_API CBlockPar : public CMain {
    friend CBlockParUnit;
    friend BPCompiler;

private:
    CHeap *m_Heap;

    CBlockParUnit *m_First;
    CBlockParUnit *m_Last;

    int m_Cnt;
    int m_CntPar;
    int m_CntBlock;

    CBlockParUnit **m_Array;
    int m_ArrayCnt;

    std::wstring *m_FromFile;

    DWORD m_Sort;  // bool
public:
    CBlockPar(bool sort = true, CHeap *heap = NULL);
    ~CBlockPar();

    void Clear(void);

    void CopyFrom(CBlockPar &bp);

private:
    CBlockParUnit *UnitAdd(void);
    void UnitDel(CBlockParUnit *el);
    CBlockParUnit *UnitGet(const wchar *path, int path_len = -1);

    int ArrayFind(const wchar *name, int namelen) const;  // -1-не найден   >=0-Первый юнит с этим названием
    int ArrayFind(const std::wstring &name) const { return ArrayFind(name.c_str(), name.length()); }
    int ArrayFindInsertIndex(CBlockParUnit *ael);  // А также инициализирует ael->m_Fast*
    void ArrayAdd(CBlockParUnit *el);
    void ArrayDel(CBlockParUnit *el);

public:
    //////////////////////////////////////////////////////////////
    CBlockParUnit *ParAdd(const wchar *name, int namelen, const wchar *zn, int znlen);
    CBlockParUnit *ParAdd(const std::wstring &name, const std::wstring &zn) {
        return ParAdd(name.c_str(), name.length(), zn.c_str(), zn.length());
    }
    CBlockParUnit *ParAdd(const std::wstring &name, const wchar *zn) {
        return ParAdd(name.c_str(), name.length(), zn, std::wcslen(zn));
    }
    CBlockParUnit *ParAdd(const wchar *name, const std::wstring &zn) {
        return ParAdd(name, std::wcslen(name), zn.c_str(), zn.length());
    }
    CBlockParUnit *ParAdd(const wchar *name, const wchar *zn) { return ParAdd(name, std::wcslen(name), zn, std::wcslen(zn)); }

    bool ParSetNE(const wchar *name, int namelen, const wchar *zn, int znlen);
    void ParSet(const std::wstring &name, const std::wstring &zn) {
        if (!ParSetNE(name.c_str(), name.length(), zn.c_str(), zn.length()))
            ERROR_E;
    }
    void ParSet(const std::wstring &name, const wchar *zn) {
        if (!ParSetNE(name.c_str(), name.length(), zn, std::wcslen(zn)))
            ERROR_E;
    }
    void ParSet(const wchar *name, const std::wstring &zn) {
        if (!ParSetNE(name, std::wcslen(name), zn.c_str(), zn.length()))
            ERROR_E;
    }
    void ParSet(const wchar *name, const wchar *zn) {
        if (!ParSetNE(name, std::wcslen(name), zn, std::wcslen(zn)))
            ERROR_E;
    }

    void ParSetAdd(const std::wstring &name, const std::wstring &zn) {
        if (!ParSetNE(name.c_str(), name.length(), zn.c_str(), zn.length()))
            ParAdd(name, zn);
    }
    void ParSetAdd(const std::wstring &name, const wchar *zn) {
        if (!ParSetNE(name.c_str(), name.length(), zn, std::wcslen(zn)))
            ParAdd(name, zn);
    }
    void ParSetAdd(const wchar* name, const std::wstring &zn) {
        if (!ParSetNE(name, std::wcslen(name), zn.c_str(), zn.length()))
            ParAdd(name, zn);
    }
    void ParSetAdd(const wchar *name, const wchar *zn) {
        if (!ParSetNE(name, std::wcslen(name), zn, std::wcslen(zn)))
            ParAdd(name, zn);
    }
    void ParSetAdd(const wchar *name, int namelen, const wchar *zn, int znlen) {
        if (!ParSetNE(name, namelen, zn, znlen))
            ParAdd(name, zn);
    }

    void ParSetAdd(const std::wstring& name, double value)
    {
        ParSetAdd(name, utils::format(L"%.8f", value));
    }

    bool ParDeleteNE(const wchar *name, int namelen);
    void ParDelete(const std::wstring &name) {
        if (!ParDeleteNE(name.c_str(), name.length()))
            ERROR_E;
    }
    void ParDelete(const wchar *name) {
        if (!ParDeleteNE(name, std::wcslen(name)))
            ERROR_E;
    }
    void ParDelete(int no);

    const std::wstring* ParGetNE_(const wchar *name, int namelen, int index) const;

    ParamParser ParGet(const std::wstring& name, int index = 0) const
    {
        const std::wstring *str = ParGetNE_(name.c_str(), name.length(), index);
        if (str == NULL)
            ERROR_S2(L"Not found: ", name.c_str());
        return *str;
    }

    ParamParser ParGetNE(const std::wstring& name, int index = 0) const {
        const std::wstring *str = ParGetNE_(name.c_str(), name.length(), index);
        if (str != NULL)
            return *str;
        else
            return std::wstring();
    }

    void Par(const std::wstring &name, const std::wstring &zn) { ParSetAdd(name, zn); }
    void Par(const std::wstring &name, const wchar *zn) { ParSetAdd(name, zn); }
    void Par(const wchar *name, const std::wstring &zn) { ParSetAdd(name, std::wcslen(name), zn.c_str(), zn.length()); }
    void Par(const wchar *name, const wchar *zn) { ParSetAdd(name, std::wcslen(name), zn, std::wcslen(zn)); }
    ParamParser Par(const std::wstring& name) { return ParGet(name); }
    ParamParser ParNE(const std::wstring& name) { return ParGetNE(name); }

    int ParCount(void) const { return m_CntPar; }
    int ParCount(const wchar *name, int namelen) const;
    int ParCount(const std::wstring &name) const { return ParCount(name.c_str(), name.length()); }
    int ParCount(const wchar *name) const { return ParCount(name, std::wcslen(name)); }

    ParamParser ParGet(int no) const;
    void ParSet(int no, const wchar *zn, int znlen);
    void ParSet(int no, const wchar *zn) { ParSet(no, zn, std::wcslen(zn)); }
    void ParSet(int no, const std::wstring &zn) { ParSet(no, zn.c_str(), zn.length()); }
    ParamParser ParGetName(int no) const;

    //////////////////////////////////////////////////////////////
    CBlockPar *BlockAdd(const wchar *name, int namelen);
    CBlockPar *BlockAdd(const std::wstring &name) { return BlockAdd(name.c_str(), name.length()); }
    CBlockPar *BlockAdd(const wchar *name) { return BlockAdd(name, std::wcslen(name)); }

    CBlockPar *BlockGetNE(const wchar *name, int namelen);
    CBlockPar *BlockGetNE(const std::wstring &name) { return BlockGetNE(name.c_str(), name.length()); }
    CBlockPar *BlockGetNE(const wchar *name) { return BlockGetNE(name, std::wcslen(name)); }

    CBlockPar *BlockGet(const std::wstring &name) {
        CBlockPar *bp = BlockGetNE(name.c_str(), name.length());
        if (!bp)
            ERROR_S2(L"Block not found: ", name.c_str());
        return bp;
    }
    CBlockPar *BlockGet(const wchar *name) {
        CBlockPar *bp = BlockGetNE(name, std::wcslen(name));
        if (!bp)
            ERROR_S2(L"Block not found: ", name);
        return bp;
    }

    CBlockPar *BlockGetAdd(const std::wstring &name) {
        CBlockPar *bp = BlockGetNE(name.c_str(), name.length());
        if (bp)
            return bp;
        else
            return BlockAdd(name.c_str(), name.length());
    }
    CBlockPar *BlockGetAdd(const wchar *name) {
        CBlockPar *bp = BlockGetNE(name, std::wcslen(name));
        if (bp)
            return bp;
        else
            return BlockAdd(name, std::wcslen(name));
    }

    bool BlockDeleteNE(const wchar *name, int namelen);
    void BlockDelete(const std::wstring &name) {
        if (!BlockDeleteNE(name.c_str(), name.length()))
            ERROR_E;
    }
    void BlockDelete(const wchar *name) {
        if (!BlockDeleteNE(name, std::wcslen(name)))
            ERROR_E;
    }
    void BlockDelete(int no);

    int BlockCount(void) const { return m_CntBlock; }
    int BlockCount(const wchar *name, int namelen) const;
    int BlockCount(const std::wstring &name) const { return BlockCount(name.c_str(), name.length()); }
    int BlockCount(const wchar *name) const { return BlockCount(name, std::wcslen(name)); }

    CBlockPar *BlockGet(int no);
    const CBlockPar *BlockGet(int no) const;
    ParamParser BlockGetName(int no) const;

    //////////////////////////////////////////////////////////////
    const std::wstring &ParPathGet(const wchar *path, int pathlen);
    const std::wstring &ParPathGet(const std::wstring &path) { return ParPathGet(path.c_str(), path.length()); }
    const std::wstring &ParPathGet(const wchar *path) { return ParPathGet(path, std::wcslen(path)); }

    void ParPathAdd(const wchar *path, int pathlen, const wchar *zn, int znlen);
    void ParPathAdd(const std::wstring &path, const std::wstring &zn) {
        ParPathAdd(path.c_str(), path.length(), zn.c_str(), zn.length());
    }
    void ParPathAdd(const std::wstring &path, const wchar *zn) { ParPathAdd(path.c_str(), path.length(), zn, std::wcslen(zn)); }
    void ParPathAdd(const wchar *path, const std::wstring &zn) { ParPathAdd(path, std::wcslen(path), zn.c_str(), zn.length()); }
    void ParPathAdd(const wchar *path, const wchar *zn) { ParPathAdd(path, std::wcslen(path), zn, std::wcslen(zn)); }

    void ParPathSet(const wchar *path, int pathlen, const wchar *zn, int znlen);
    void ParPathSet(const std::wstring &path, const std::wstring &zn) {
        ParPathSet(path.c_str(), path.length(), zn.c_str(), zn.length());
    }
    void ParPathSet(const std::wstring &path, const wchar *zn) { ParPathSet(path.c_str(), path.length(), zn, std::wcslen(zn)); }
    void ParPathSet(const wchar *path, const std::wstring &zn) { ParPathSet(path, std::wcslen(path), zn.c_str(), zn.length()); }
    void ParPathSet(const wchar *path, const wchar *zn) { ParPathSet(path, std::wcslen(path), zn, std::wcslen(zn)); }

    void ParPathSetAdd(const wchar *path, int pathlen, const wchar *zn, int znlen);
    void ParPathSetAdd(const std::wstring &path, const std::wstring &zn) {
        ParPathSetAdd(path.c_str(), path.length(), zn.c_str(), zn.length());
    }
    void ParPathSetAdd(const std::wstring &path, const wchar *zn) {
        ParPathSetAdd(path.c_str(), path.length(), zn, std::wcslen(zn));
    }
    void ParPathSetAdd(const wchar *path, const std::wstring &zn) {
        ParPathSetAdd(path, std::wcslen(path), zn.c_str(), zn.length());
    }
    void ParPathSetAdd(const wchar *path, const wchar *zn) { ParPathSetAdd(path, std::wcslen(path), zn, std::wcslen(zn)); }

    void ParPathDelete(const wchar *path, int pathlen);
    void ParPathDelete(const std::wstring &path) { ParPathDelete(path.c_str(), path.length()); }
    void ParPathDelete(const wchar *path) { ParPathDelete(path, std::wcslen(path)); }

    int ParPathCount(const wchar *path, int pathlen);

    //////////////////////////////////////////////////////////////
    CBlockPar *BlockPathGet(const wchar *path, int pathlen);
    CBlockPar *BlockPathGet(const std::wstring &path) { return BlockPathGet(path.c_str(), path.length()); }
    CBlockPar *BlockPathGet(const wchar *path) { return BlockPathGet(path, std::wcslen(path)); }

    CBlockPar *BlockPathAdd(const wchar *path, int pathlen);
    CBlockPar *BlockPathAdd(const std::wstring &path) { return BlockPathAdd(path.c_str(), path.length()); }
    CBlockPar *BlockPathAdd(const wchar *path) { return BlockPathAdd(path, std::wcslen(path)); }

    CBlockPar *BlockPathGetAdd(const wchar *path, int pathlen);
    CBlockPar *BlockPathGetAdd(const std::wstring &path) { return BlockPathGetAdd(path.c_str(), path.length()); }
    CBlockPar *BlockPathGetAdd(const wchar *path) { return BlockPathGetAdd(path, std::wcslen(path)); }

    CBlockPar *BlockPath(const std::wstring &path) { return BlockPathGet(path.c_str(), path.length()); }
    CBlockPar *BlockPath(const wchar *path) { return BlockPathGet(path, std::wcslen(path)); }

    //////////////////////////////////////////////////////////////
    int AllCount(void) { return m_Cnt; }
    int AllGetType(int no);
    CBlockPar *AllGetBlock(int no);
    const std::wstring &AllGetPar(int no);
    const std::wstring &AllGetName(int no);

    //////////////////////////////////////////////////////////////
    int LoadFromText(const wchar *text, int textlen);
    int LoadFromText(const std::wstring &text) { return LoadFromText(text.c_str(), text.length()); }
    int LoadFromText(const wchar *text) { return LoadFromText(text, std::wcslen(text)); }

    void LoadFromTextFile(const wchar *filename, int filenamelen);
    void LoadFromTextFile(const std::wstring &filename) { LoadFromTextFile(filename.c_str(), filename.length()); }
    void LoadFromTextFile(const wchar *filename) { LoadFromTextFile(filename, std::wcslen(filename)); }

    void SaveInText(CBuf &buf, bool ansi = false, int level = 0);

    void SaveInTextFile(const wchar *filename, int filenamelen, bool ansi = false);
    void SaveInTextFile(const std::wstring &filename, bool ansi = false) {
        SaveInTextFile(filename.c_str(), filename.length(), ansi);
    }
    void SaveInTextFile(const wchar *filename, bool ansi = false) { SaveInTextFile(filename, std::wcslen(filename), ansi); }
};

}  // namespace Base