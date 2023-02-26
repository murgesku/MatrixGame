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
    CWStr m_Name;
    CWStr m_Com;
    union {
        CWStr *m_Par;
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

    CWStr *m_FromFile;

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
    int ArrayFind(const CWStr &name) const { return ArrayFind(name.c_str(), name.length()); }
    int ArrayFindInsertIndex(CBlockParUnit *ael);  // А также инициализирует ael->m_Fast*
    void ArrayAdd(CBlockParUnit *el);
    void ArrayDel(CBlockParUnit *el);

public:
    //////////////////////////////////////////////////////////////
    CBlockParUnit *ParAdd(const wchar *name, int namelen, const wchar *zn, int znlen);
    CBlockParUnit *ParAdd(const CWStr &name, const CWStr &zn) {
        return ParAdd(name.c_str(), name.length(), zn.c_str(), zn.length());
    }
    CBlockParUnit *ParAdd(const CWStr &name, const wchar *zn) {
        return ParAdd(name.c_str(), name.length(), zn, WStrLen(zn));
    }
    CBlockParUnit *ParAdd(const wchar *name, const CWStr &zn) {
        return ParAdd(name, WStrLen(name), zn.c_str(), zn.length());
    }
    CBlockParUnit *ParAdd(const wchar *name, const wchar *zn) { return ParAdd(name, WStrLen(name), zn, WStrLen(zn)); }

    bool ParSetNE(const wchar *name, int namelen, const wchar *zn, int znlen);
    void ParSet(const CWStr &name, const CWStr &zn) {
        if (!ParSetNE(name.c_str(), name.length(), zn.c_str(), zn.length()))
            ERROR_E;
    }
    void ParSet(const CWStr &name, const wchar *zn) {
        if (!ParSetNE(name.c_str(), name.length(), zn, WStrLen(zn)))
            ERROR_E;
    }
    void ParSet(const wchar *name, const CWStr &zn) {
        if (!ParSetNE(name, WStrLen(name), zn.c_str(), zn.length()))
            ERROR_E;
    }
    void ParSet(const wchar *name, const wchar *zn) {
        if (!ParSetNE(name, WStrLen(name), zn, WStrLen(zn)))
            ERROR_E;
    }

    void ParSetAdd(const CWStr &name, const CWStr &zn) {
        if (!ParSetNE(name.c_str(), name.length(), zn.c_str(), zn.length()))
            ParAdd(name, zn);
    }
    void ParSetAdd(const CWStr &name, const wchar *zn) {
        if (!ParSetNE(name.c_str(), name.length(), zn, WStrLen(zn)))
            ParAdd(name, zn);
    }
    void ParSetAdd(const wchar* name, const CWStr &zn) {
        if (!ParSetNE(name, WStrLen(name), zn.c_str(), zn.length()))
            ParAdd(name, zn);
    }
    void ParSetAdd(const wchar *name, const wchar *zn) {
        if (!ParSetNE(name, WStrLen(name), zn, WStrLen(zn)))
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
    void ParDelete(const CWStr &name) {
        if (!ParDeleteNE(name.c_str(), name.length()))
            ERROR_E;
    }
    void ParDelete(const wchar *name) {
        if (!ParDeleteNE(name, WStrLen(name)))
            ERROR_E;
    }
    void ParDelete(int no);

    const CWStr* ParGetNE_(const wchar *name, int namelen, int index) const;

    ParamParser ParGet(const std::wstring& name, int index = 0) const
    {
        const CWStr *str = ParGetNE_(name.c_str(), name.length(), index);
        if (str == NULL)
            ERROR_S2(L"Not found: ", name.c_str());
        return *str;
    }

    ParamParser ParGetNE(const std::wstring& name, int index = 0) const {
        const CWStr *str = ParGetNE_(name.c_str(), name.length(), index);
        if (str != NULL)
            return *str;
        else
            return CWStr();
    }

    void Par(const CWStr &name, const CWStr &zn) { ParSetAdd(name, zn); }
    void Par(const CWStr &name, const wchar *zn) { ParSetAdd(name, zn); }
    void Par(const wchar *name, const CWStr &zn) { ParSetAdd(name, WStrLen(name), zn.c_str(), zn.length()); }
    void Par(const wchar *name, const wchar *zn) { ParSetAdd(name, WStrLen(name), zn, WStrLen(zn)); }
    ParamParser Par(const std::wstring& name) { return ParGet(name); }
    ParamParser ParNE(const std::wstring& name) { return ParGetNE(name); }

    int ParCount(void) const { return m_CntPar; }
    int ParCount(const wchar *name, int namelen) const;
    int ParCount(const CWStr &name) const { return ParCount(name.c_str(), name.length()); }
    int ParCount(const wchar *name) const { return ParCount(name, WStrLen(name)); }

    ParamParser ParGet(int no) const;
    void ParSet(int no, const wchar *zn, int znlen);
    void ParSet(int no, const wchar *zn) { ParSet(no, zn, WStrLen(zn)); }
    void ParSet(int no, const CWStr &zn) { ParSet(no, zn.c_str(), zn.length()); }
    ParamParser ParGetName(int no) const;

    //////////////////////////////////////////////////////////////
    CBlockPar *BlockAdd(const wchar *name, int namelen);
    CBlockPar *BlockAdd(const CWStr &name) { return BlockAdd(name.c_str(), name.length()); }
    CBlockPar *BlockAdd(const wchar *name) { return BlockAdd(name, WStrLen(name)); }

    CBlockPar *BlockGetNE(const wchar *name, int namelen);
    CBlockPar *BlockGetNE(const CWStr &name) { return BlockGetNE(name.c_str(), name.length()); }
    CBlockPar *BlockGetNE(const wchar *name) { return BlockGetNE(name, WStrLen(name)); }

    CBlockPar *BlockGet(const CWStr &name) {
        CBlockPar *bp = BlockGetNE(name.c_str(), name.length());
        if (!bp)
            ERROR_S2(L"Block not found: ", name.c_str());
        return bp;
    }
    CBlockPar *BlockGet(const wchar *name) {
        CBlockPar *bp = BlockGetNE(name, WStrLen(name));
        if (!bp)
            ERROR_S2(L"Block not found: ", name);
        return bp;
    }

    CBlockPar *BlockGetAdd(const CWStr &name) {
        CBlockPar *bp = BlockGetNE(name.c_str(), name.length());
        if (bp)
            return bp;
        else
            return BlockAdd(name.c_str(), name.length());
    }
    CBlockPar *BlockGetAdd(const wchar *name) {
        CBlockPar *bp = BlockGetNE(name, WStrLen(name));
        if (bp)
            return bp;
        else
            return BlockAdd(name, WStrLen(name));
    }

    bool BlockDeleteNE(const wchar *name, int namelen);
    void BlockDelete(const CWStr &name) {
        if (!BlockDeleteNE(name.c_str(), name.length()))
            ERROR_E;
    }
    void BlockDelete(const wchar *name) {
        if (!BlockDeleteNE(name, WStrLen(name)))
            ERROR_E;
    }
    void BlockDelete(int no);

    int BlockCount(void) const { return m_CntBlock; }
    int BlockCount(const wchar *name, int namelen) const;
    int BlockCount(const CWStr &name) const { return BlockCount(name.c_str(), name.length()); }
    int BlockCount(const wchar *name) const { return BlockCount(name, WStrLen(name)); }

    CBlockPar *BlockGet(int no);
    const CBlockPar *BlockGet(int no) const;
    ParamParser BlockGetName(int no) const;

    //////////////////////////////////////////////////////////////
    const CWStr &ParPathGet(const wchar *path, int pathlen);
    const CWStr &ParPathGet(const CWStr &path) { return ParPathGet(path.c_str(), path.length()); }
    const CWStr &ParPathGet(const wchar *path) { return ParPathGet(path, WStrLen(path)); }

    void ParPathAdd(const wchar *path, int pathlen, const wchar *zn, int znlen);
    void ParPathAdd(const CWStr &path, const CWStr &zn) {
        ParPathAdd(path.c_str(), path.length(), zn.c_str(), zn.length());
    }
    void ParPathAdd(const CWStr &path, const wchar *zn) { ParPathAdd(path.c_str(), path.length(), zn, WStrLen(zn)); }
    void ParPathAdd(const wchar *path, const CWStr &zn) { ParPathAdd(path, WStrLen(path), zn.c_str(), zn.length()); }
    void ParPathAdd(const wchar *path, const wchar *zn) { ParPathAdd(path, WStrLen(path), zn, WStrLen(zn)); }

    void ParPathSet(const wchar *path, int pathlen, const wchar *zn, int znlen);
    void ParPathSet(const CWStr &path, const CWStr &zn) {
        ParPathSet(path.c_str(), path.length(), zn.c_str(), zn.length());
    }
    void ParPathSet(const CWStr &path, const wchar *zn) { ParPathSet(path.c_str(), path.length(), zn, WStrLen(zn)); }
    void ParPathSet(const wchar *path, const CWStr &zn) { ParPathSet(path, WStrLen(path), zn.c_str(), zn.length()); }
    void ParPathSet(const wchar *path, const wchar *zn) { ParPathSet(path, WStrLen(path), zn, WStrLen(zn)); }

    void ParPathSetAdd(const wchar *path, int pathlen, const wchar *zn, int znlen);
    void ParPathSetAdd(const CWStr &path, const CWStr &zn) {
        ParPathSetAdd(path.c_str(), path.length(), zn.c_str(), zn.length());
    }
    void ParPathSetAdd(const CWStr &path, const wchar *zn) {
        ParPathSetAdd(path.c_str(), path.length(), zn, WStrLen(zn));
    }
    void ParPathSetAdd(const wchar *path, const CWStr &zn) {
        ParPathSetAdd(path, WStrLen(path), zn.c_str(), zn.length());
    }
    void ParPathSetAdd(const wchar *path, const wchar *zn) { ParPathSetAdd(path, WStrLen(path), zn, WStrLen(zn)); }

    void ParPathDelete(const wchar *path, int pathlen);
    void ParPathDelete(const CWStr &path) { ParPathDelete(path.c_str(), path.length()); }
    void ParPathDelete(const wchar *path) { ParPathDelete(path, WStrLen(path)); }

    int ParPathCount(const wchar *path, int pathlen);

    //////////////////////////////////////////////////////////////
    CBlockPar *BlockPathGet(const wchar *path, int pathlen);
    CBlockPar *BlockPathGet(const CWStr &path) { return BlockPathGet(path.c_str(), path.length()); }
    CBlockPar *BlockPathGet(const wchar *path) { return BlockPathGet(path, WStrLen(path)); }

    CBlockPar *BlockPathAdd(const wchar *path, int pathlen);
    CBlockPar *BlockPathAdd(const CWStr &path) { return BlockPathAdd(path.c_str(), path.length()); }
    CBlockPar *BlockPathAdd(const wchar *path) { return BlockPathAdd(path, WStrLen(path)); }

    CBlockPar *BlockPathGetAdd(const wchar *path, int pathlen);
    CBlockPar *BlockPathGetAdd(const CWStr &path) { return BlockPathGetAdd(path.c_str(), path.length()); }
    CBlockPar *BlockPathGetAdd(const wchar *path) { return BlockPathGetAdd(path, WStrLen(path)); }

    CBlockPar *BlockPath(const CWStr &path) { return BlockPathGet(path.c_str(), path.length()); }
    CBlockPar *BlockPath(const wchar *path) { return BlockPathGet(path, WStrLen(path)); }

    //////////////////////////////////////////////////////////////
    int AllCount(void) { return m_Cnt; }
    int AllGetType(int no);
    CBlockPar *AllGetBlock(int no);
    const CWStr &AllGetPar(int no);
    const CWStr &AllGetName(int no);

    //////////////////////////////////////////////////////////////
    int LoadFromText(const wchar *text, int textlen);
    int LoadFromText(const CWStr &text) { return LoadFromText(text.c_str(), text.length()); }
    int LoadFromText(const wchar *text) { return LoadFromText(text, WStrLen(text)); }

    void LoadFromTextFile(const wchar *filename, int filenamelen);
    void LoadFromTextFile(const CWStr &filename) { LoadFromTextFile(filename.c_str(), filename.length()); }
    void LoadFromTextFile(const wchar *filename) { LoadFromTextFile(filename, WStrLen(filename)); }

    void SaveInText(CBuf &buf, bool ansi = false, int level = 0);

    void SaveInTextFile(const wchar *filename, int filenamelen, bool ansi = false);
    void SaveInTextFile(const CWStr &filename, bool ansi = false) {
        SaveInTextFile(filename.c_str(), filename.length(), ansi);
    }
    void SaveInTextFile(const wchar *filename, bool ansi = false) { SaveInTextFile(filename, WStrLen(filename), ansi); }
};

}  // namespace Base