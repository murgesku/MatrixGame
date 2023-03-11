// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <string>

#include "Base.pch"

#include "CBlockPar.hpp"
#include "CFile.hpp"
#include "CException.hpp"

#include <utils.hpp>

namespace Base {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool ParamParser::GetBool() const
{
    std::wstring str = *this;
    utils::to_lower(str);

    if(str == L"true" || str == L"yes" || str == L"on")
    {
        return true;
    }

    if(str == L"false" || str == L"no" || str == L"off")
    {
        return false;
    }

    ERROR_E;
    return false;
}

int ParamParser::GetInt() const
{
    if (this->empty())
    {
        return 0;
    }

    int value = 0;
    bool sign = false;

    // the original algorithm skips all the non-digit chars and treats
    // '-' at any position as a number sign.
    // needless to say - it's a bullshit, but other code expects it
    // to work like this, so...
    for (const auto sym : *this)
    {
        wchar ch = sym - '0';
        if (ch < 10)
        {
            value = value * 10 + ch;
        }

        if (sym == '-')
        {
            sign = true;
        }

    }

    return sign ? -value : value;
}

DWORD ParamParser::GetDword() const {
    int tlen = length();
    if (tlen < 1)
        return 0;
    const wchar *tstr = c_str();

    DWORD zn = 0;
    wchar ch;
    for (int i = 0; i < tlen; i++) {
        ch = tstr[i] - '0';
        if (ch < 10)
            zn = zn * 10 + ch;
    }

    return zn;
}

double ParamParser::GetDouble() const {
    int tlen = length();
    if (tlen < 1)
        return 0;
    const wchar *tstr = c_str();

    int i;
    double zn = 0.0;

    wchar ch;
    for (i = 0; i < tlen; i++) {
        ch = tstr[i];
        if (ch >= L'0' && ch <= L'9')
            zn = zn * 10.0 + (double)(ch - L'0');
        else if (ch == L'.')
            break;
    }
    i++;
    double tra = 10.0;
    for (i; i < tlen; i++) {
        ch = tstr[i];
        if (ch >= L'0' && ch <= L'9') {
            zn = zn + ((double)(ch - L'0')) / tra;
            tra *= 10.0;
        }
    }
    for (i = 0; i < tlen; i++)
        if (tstr[i] == '-') {
            zn = -zn;
            break;
        }

    return zn;
}

int ParamParser::GetHex() const {
    int tlen = length();
    if (tlen < 1)
        return 0;
    const wchar *tstr = c_str();

    int zn = 0;
    int i;

    wchar ch;
    for (i = 0; i < tlen; i++) {
        ch = tstr[i];

        ch -= '0';
        if (ch > 9)
            ch = (tstr[i] & (~32)) - ('A' - 10);
        zn = zn * 16 + ch;
    }
    for (i = 0; i < tlen; i++)
        if (tstr[i] == '-') {
            zn = -zn;
            break;
        }

    return zn;
}

DWORD ParamParser::GetHexUnsigned(void) const {
    int tlen = length();
    if (tlen < 1)
        return 0;
    const wchar *tstr = c_str();

    DWORD zn = 0;
    int i;

    wchar ch;
    for (i = 0; i < tlen; i++) {
        ch = tstr[i];

        ch -= '0';
        if (ch > 9)
            ch = (tstr[i] & (~32)) - ('A' - 10);
        zn = (zn << 4) + ch;
    }
    return zn;
}

bool ParamParser::IsOnlyInt() const {
    int tlen = length();
    if (tlen < 1)
        return 0;
    const wchar *tstr = c_str();

    for (int i = 0; i < tlen; i++)
        if (tstr[i] < L'0' || tstr[i] > L'9' && tstr[i] != L'-')
            return 0;
    return 1;
}

int ParamParser::GetSmePar(int np, const wchar *ogsim) const {
    int lenogsim = std::wcslen(ogsim);
    int tlen = length();
    // if(tlen<1 || lenogsim<1 || np<0) ERROR_OK("Data in CWStr::GetSmePar()");
    //  if((tlen<1 || lenogsim<1 || np<0))
    //        ASSERT(1);
    ASSERT(!(tlen < 1 || lenogsim < 1 || np < 0));
    int smepar = 0;
    int tekpar = 0;

    const wchar *tstr = c_str();

    if (np > 0) {
        int i;
        for (i = 0; i < tlen; i++) {
            int u;
            for (u = 0; u < lenogsim; u++)
                if (tstr[i] == ogsim[u])
                    break;
            if (u < lenogsim) {
                tekpar++;
                smepar = i + 1;
                if (tekpar == np)
                    break;
            }
        }
        if (i == tlen) {
            ERROR_E;
        }
    }

    return smepar;
}

int ParamParser::GetLenPar(int smepar, const wchar *ogsim) const {
    int i;
    int tlen = length();
    int lenogsim = std::wcslen(ogsim);
    if (tlen < 1 || lenogsim < 1 || smepar > tlen)
        ERROR_E;

    const wchar *tstr = c_str();

    for (i = smepar; i < tlen; i++) {
        int u;
        for (u = 0; u < lenogsim; u++)
            if (tstr[i] == ogsim[u])
                break;
        if (u < lenogsim)
            break;
    }
    return i - smepar;
}

ParamParser ParamParser::GetStrPar(int nps, int npe, const wchar *ogsim) const {
    int sme1 = GetSmePar(nps, ogsim);
    int sme2 = GetSmePar(npe, ogsim);
    sme2 += GetLenPar(sme2, ogsim);
    return std::wstring(c_str() + sme1, sme2 - sme1);
}

int ParamParser::GetCountPar(const wchar *ogsim) const {
    int tlen = length();
    if (tlen < 1)
        return 0;

    int c = 1;
    int lenogsim = std::wcslen(ogsim);
    if (lenogsim < 1)
        return 0;

    const wchar *tstr = c_str();

    for (int i = 0; i < tlen; i++) {
        int u;
        for (u = 0; u < lenogsim; u++)
            if (tstr[i] == ogsim[u])
                break;
        if (u < lenogsim)
            c++;
    }
    return c;
}

////////////////////////////////////////////////////////////////////////////////
CBlockParUnit::CBlockParUnit(CHeap *heap) : CMain(), m_Name{}, m_Com{} {
    DTRACE();
    m_Heap = heap;

    m_Prev = NULL;
    m_Next = NULL;
    m_Parent = NULL;

    m_Type = 0;

    m_FastFirst = 0;
    m_FastCnt = 0;
}

CBlockParUnit::~CBlockParUnit() {
    DTRACE();
    Clear();
}

void CBlockParUnit::Clear() {
    DTRACE();
    if (m_Type == 1) {
        if (m_Par != NULL) {
            using std::wstring;
            HDelete(wstring, m_Par, m_Heap);
            m_Par = NULL;
        }
    }
    else if (m_Type == 2) {
        if (m_Block != NULL) {
            HDelete(CBlockPar, m_Block, m_Heap);
            m_Block = NULL;
        }
    }
    m_Type = 0;
    m_Name.clear();
    m_Com.clear();
}

void CBlockParUnit::ChangeType(int nt) {
    DTRACE();
    if (m_Type == 1) {
        if (m_Par != NULL) {
            using std::wstring;
            HDelete(wstring, m_Par, m_Heap);
            m_Par = NULL;
        }
    }
    else if (m_Type == 2) {
        if (m_Block != NULL) {
            HDelete(CBlockPar, m_Block, m_Heap);
            m_Block = NULL;
        }
    }
    m_Type = nt;
    if (nt == 1)
        m_Par = HNew(m_Heap) std::wstring();
    else if (nt == 2)
        m_Block = HNew(m_Heap) CBlockPar(1, m_Heap);
}

void CBlockParUnit::CopyFrom(CBlockParUnit &bp) {
    DTRACE();
    Clear();
    ChangeType(bp.m_Type);
    m_Name = bp.m_Name;
    m_Com = bp.m_Com;
    if (m_Type == 1)
        *m_Par = *bp.m_Par;
    else if (m_Type == 2)
        m_Block->CopyFrom(*bp.m_Block);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CBlockPar::CBlockPar(bool sort, CHeap *heap) : CMain() {
    DTRACE();
    m_Heap = heap;

    m_First = NULL;
    m_Last = NULL;

    m_Cnt = 0;
    m_CntPar = 0;
    m_CntBlock = 0;

    m_Sort = sort;
}

CBlockPar::~CBlockPar() {
    DTRACE();
    Clear();
}

void CBlockPar::Clear() {
    DTRACE();
    CBlockParUnit *tt, *t = m_First;
    while (t != NULL) {
        tt = t;
        t = t->m_Next;
        HDelete(CBlockParUnit, tt, m_Heap);
    }
    m_First = NULL;
    m_Last = NULL;

    m_Cnt = 0;
    m_CntPar = 0;
    m_CntBlock = 0;

    m_Array.clear();
    m_FromFile.clear();
}

void CBlockPar::CopyFrom(CBlockPar &bp) {
    DTRACE();
    Clear();
    m_Sort = bp.m_Sort;
    CBlockParUnit *el2, *el = bp.m_First;
    while (el != NULL) {
        el2 = UnitAdd();
        el2->CopyFrom(*el);
        if (m_Sort)
            ArrayAdd(el2);
        if (el->m_Type == 1)
            m_CntPar++;
        else if (el->m_Type == 2)
            m_CntBlock++;
        el = el->m_Next;
    }
}

CBlockParUnit *CBlockPar::UnitAdd() {
    DTRACE();
    CBlockParUnit *el = HNew(m_Heap) CBlockParUnit(m_Heap);
    el->m_Parent = this;

    LIST_ADD(el, m_First, m_Last, m_Prev, m_Next);

    m_Cnt++;
    return el;
}

void CBlockPar::UnitDel(CBlockParUnit *el) {
    DTRACE();
    LIST_DEL(el, m_First, m_Last, m_Prev, m_Next);

    m_Cnt--;
    if (el->m_Type == 1)
        m_CntPar--;
    else if (el->m_Type == 2)
        m_CntBlock--;

    HDelete(CBlockParUnit, el, m_Heap);
}

CBlockParUnit *CBlockPar::UnitGet(const wchar *path, int path_len) {
    DTRACE();
    wchar ch;
    int i, u, sme, smeend, no;
    int name_sme, name_len, name_next;

    if (path_len < 0)
        path_len = std::wcslen(path);
    name_next = 0;

    CBlockPar *us = this;
    CBlockParUnit *ne = NULL;

    for (;;) {
        // nameExtractNext
        if (name_next >= path_len)
            break;
        name_sme = name_next;
        sme = name_sme;
        while (sme < path_len) {
            ch = path[sme];
            if ((ch == '.') || (ch == '/') || (ch == '\\'))
                break;
            sme++;
        }
        name_len = sme - name_sme;
        name_next = sme + 1;
        // end

        // noExtract;
        no = 0;
        sme = name_sme;
        smeend = name_sme + name_len;
        while (sme < smeend) {
            if (path[sme] == ':') {
                name_len = sme - name_sme;
                sme++;
                while (sme < smeend) {
                    ch = path[sme];
                    if ((ch >= '0') && (ch <= '9'))
                        no = no * 10 + (int(ch) - int('0'));
                    sme++;
                }
                break;
            }
            sme++;
        }
        // end

        if (us->m_Sort) {
            ne = NULL;
            i = us->ArrayFind(std::wstring{(wchar*)path + name_sme, static_cast<size_t>(name_len)});
            if (i >= 0) {
                ne = us->m_Array[i];
                if (no == 0)
                    ;
                else if (no < ne->m_FastCnt)
                    ne = us->m_Array[i + no];
                else
                    ne = NULL;
            }
        }
        else {
            ne = us->m_First;
            u = 0;
            while ((u <= no) && (ne != NULL)) {
                while (ne != NULL) {
                    if (ne->m_Name == std::wstring{path + name_sme, static_cast<size_t>(name_len)}) {
                        if (u < no)
                            ne = ne->m_Next;
                        break;
                    }
                    ne = ne->m_Next;
                }
                u++;
            }
        }

        if (ne == NULL)
            ERROR_S2(L"Path not found: ", path);
        if (name_next >= path_len)
            break;
        if (ne->m_Type != 2)
            ERROR_S2(L"Path not found: ", path);
        us = ne->m_Block;
    }
    if (ne == NULL)
        ERROR_S2(L"Path not found: ", path);
    return ne;
}

int CBlockPar::ArrayFind(const std::wstring& name) const
{
    DTRACE();
    if (m_Array.empty())
        return -1;
    int istart = 0;
    int iend = m_Array.size() - 1;
    for (;;) {
        int icur = istart + ((iend - istart) / 2);
        CBlockParUnit *el = m_Array[icur];
        int cz = name.compare(el->m_Name);
        if (cz == 0)
            return icur - el->m_FastFirst;
        else if (cz < 0)
            iend = icur - 1;
        else
            istart = icur + 1;
        if (iend < istart)
            return -1;
    }
}

int CBlockPar::ArrayFindInsertIndex(CBlockParUnit *ael) const {
    DTRACE();
    int rv;
    if (m_Array.empty())
    {
        ael->m_FastFirst = 0;
        ael->m_FastCnt = 1;
        return 0;
    }
    int istart = 0;
    int iend = m_Array.size() - 1;
    for (;;) {
        int icur = istart + ((iend - istart) / 2);
        CBlockParUnit *el = m_Array[icur];
        int cz = ael->m_Name.compare(el->m_Name);
        if (cz == 0) {
            if (el->m_FastFirst != 0) {
                rv = icur - el->m_FastFirst;
                el = m_Array[rv];
            }
            else
                rv = icur;
            ael->m_FastFirst = el->m_FastCnt;
            rv = rv + el->m_FastCnt;
            el->m_FastCnt++;
            return rv;
        }
        else if (cz < 0)
            iend = icur - 1;
        else
            istart = icur + 1;
        if (iend < istart) {
            ael->m_FastFirst = 0;
            ael->m_FastCnt = 1;

            if (cz < 0)
                return icur;
            else
                return icur + 1;
        }
    }
}

void CBlockPar::ArrayAdd(CBlockParUnit *el) {
    DTRACE();

    int no = ArrayFindInsertIndex(el);
    if (no >= m_Array.size())
    {
        m_Array.push_back(el);
    }
    else
    {
        m_Array.insert(m_Array.begin() + no, el);
    }
}

void CBlockPar::ArrayDel(CBlockParUnit *el) {
    DTRACE();
    for (int no = 0; no < m_Array.size(); no++)
    {
        if (m_Array[no] == el)
        {
            CBlockParUnit *el2 = m_Array[no - el->m_FastFirst];
            for (int i = no + 1; i < no - el->m_FastFirst + el2->m_FastCnt; m_Array[i]->m_FastFirst--)
                ;
            el2->m_FastCnt--;
            if ((el->m_FastFirst == 0) && (el2->m_FastCnt > 0)) {
                m_Array[no + 1]->m_FastCnt = el->m_FastCnt;
            }

            m_Array.erase(m_Array.begin() + no);
            return;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CBlockParUnit *CBlockPar::ParAdd(const std::wstring& name, const std::wstring& zn) {
    DTRACE();
    CBlockParUnit *el = UnitAdd();
    el->ChangeType(1);
    el->m_Name = name;
    *(el->m_Par) = zn;
    if (m_Sort)
        ArrayAdd(el);
    m_CntPar++;

    return el;
}

bool CBlockPar::ParSetNE(const wchar *name, int namelen, const wchar *zn, int znlen) {
    DTRACE();
    if (m_Sort) {
        int i = ArrayFind(std::wstring{name, static_cast<size_t>(namelen)});
        if (i >= 0) {
            for (int li = i + m_Array[i]->m_FastCnt; i < li; i++) {
                if (m_Array[i]->m_Type == 1) {
                    *(m_Array[i]->m_Par) = std::wstring{zn, static_cast<size_t>(znlen)};
                    return true;
                }
            }
        }
    }
    else {
        CBlockParUnit *el = m_First;
        while (el != NULL) {
            if ((el->m_Type == 1) && (el->m_Name == std::wstring{name, static_cast<size_t>(namelen)})) {
                *(el->m_Par) = std::wstring{zn, static_cast<size_t>(znlen)};
                return true;
            }
            el = el->m_Next;
        }
    }
    return false;
}

bool CBlockPar::ParDeleteNE(const wchar *name, int namelen)  // нужно оптимизировать
{
    DTRACE();
    CBlockParUnit *el = m_First;
    while (el != NULL) {
        if ((el->m_Type == 1) && (el->m_Name == std::wstring{name, static_cast<size_t>(namelen)})) {
            if (m_Sort)
                ArrayDel(el);
            UnitDel(el);
            return true;
        }
        el = el->m_Next;
    }
    return false;
}

void CBlockPar::ParDelete(int no)  // нужно оптимизировать
{
    DTRACE();
    CBlockParUnit *el = m_First;
    while (el != NULL) {
        if (el->m_Type == 1) {
            if (no == 0) {
                if (m_Sort)
                    ArrayDel(el);
                UnitDel(el);
                return;
            }
            no--;
        }
        el = el->m_Next;
    }
    ERROR_E;
}

const std::wstring* CBlockPar::ParGetNE_(const wchar *name, int namelen, int index) const {
    DTRACE();
    if (m_Sort) {
        int i = ArrayFind(std::wstring{name, static_cast<size_t>(namelen)});
        if (i >= 0) {
            for (int li = i + m_Array[i]->m_FastCnt; i < li; i++) {
                if (m_Array[i]->m_Type == 1 && index <= 0)
                    return m_Array[i]->m_Par;
                --index;
            }
        }
    }
    else {
        CBlockParUnit *el = m_First;
        while (el != NULL) {
            if ((el->m_Type == 1) && (el->m_Name == std::wstring{name, static_cast<size_t>(namelen)}) && index <= 0)
                return el->m_Par;
            --index;
            el = el->m_Next;
        }
    }
    return NULL;
}

int CBlockPar::ParCount(const wchar *name, int namelen) const {
    DTRACE();
    int rv = 0;

    if (m_Sort) {
        int i = ArrayFind(std::wstring{name, static_cast<size_t>(namelen)});
        if (i >= 0) {
            int li = i + m_Array[i]->m_FastCnt;
            while (i < li) {
                if (m_Array[i]->m_Type == 1)
                    rv++;
                i++;
            }
        }
    }
    else {
        CBlockParUnit *el = m_First;
        while (el != NULL) {
            if ((el->m_Type == 1) && (el->m_Name == std::wstring{name, static_cast<size_t>(namelen)}))
                rv++;
            el = el->m_Next;
        }
    }

    return rv;
}

ParamParser CBlockPar::ParGet(int no) const {
    DTRACE();
    if (m_Sort && (m_Cnt == m_CntPar)) {
        return *m_Array[no]->m_Par;
    }
    else {
        CBlockParUnit *el = m_First;
        while (el != NULL) {
            if (el->m_Type == 1) {
                if (no == 0)
                    return *el->m_Par;
                no--;
            }
            el = el->m_Next;
        }
        ERROR_E;
    }
}

void CBlockPar::ParSet(int no, const wchar *zn, int znlen) {
    DTRACE();
    if (m_Sort && (m_Cnt == m_CntPar)) {
        *(m_Array[no]->m_Par) = std::wstring{zn, static_cast<size_t>(znlen)};
        return;
    }
    else {
        CBlockParUnit *el = m_First;
        while (el != NULL) {
            if (el->m_Type == 1) {
                if (no == 0)
                {
                    *(el->m_Par) = std::wstring{zn, static_cast<size_t>(znlen)};
                    return;
                }
                no--;
            }
            el = el->m_Next;
        }
        ERROR_E;
    }
}

ParamParser CBlockPar::ParGetName(int no) const {
    DTRACE();
    if (m_Sort && (m_Cnt == m_CntPar)) {
        return m_Array[no]->m_Name;
    }
    else {
        CBlockParUnit *el = m_First;
        while (el != NULL) {
            if (el->m_Type == 1) {
                if (no == 0)
                    return el->m_Name;
                no--;
            }
            el = el->m_Next;
        }
        ERROR_E;
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CBlockPar *CBlockPar::BlockAdd(const std::wstring& name)
{
    DTRACE();
    CBlockParUnit *el = UnitAdd();
    el->ChangeType(2);
    el->m_Name = name;
    if (m_Sort)
        ArrayAdd(el);
    m_CntBlock++;

    return el->m_Block;
}

CBlockPar *CBlockPar::BlockGetNE(const std::wstring& name)
{
    DTRACE();
    if (m_Sort) {
        int i = ArrayFind(name);
        if (i >= 0) {
            for (int li = i + m_Array[i]->m_FastCnt; i < li; i++) {
                if (m_Array[i]->m_Type == 2)
                    return m_Array[i]->m_Block;
            }
        }
    }
    else {
        CBlockParUnit *el = m_First;
        while (el != NULL) {
            if ((el->m_Type == 2) && (el->m_Name == name))
                return el->m_Block;
            el = el->m_Next;
        }
    }
    return NULL;
}

CBlockPar* CBlockPar::BlockGet(const std::wstring &name)
{
    CBlockPar *bp = BlockGetNE(name);
    if (!bp)
        ERROR_S2(L"Block not found: ", name.c_str());
    return bp;
}

CBlockPar* CBlockPar::BlockGetAdd(const std::wstring &name)
{
    CBlockPar *bp = BlockGetNE(name);
    if (bp)
        return bp;
    else
        return BlockAdd(name);
}

void CBlockPar::BlockDelete(const std::wstring &name)
{
    DTRACE();
    CBlockParUnit *el = m_First;
    while (el != NULL)
    {
        if ((el->m_Type == 2) && (el->m_Name == name))
        {
            if (m_Sort)
            {
                ArrayDel(el);
            }
            UnitDel(el);
            return;
        }
        el = el->m_Next;
    }

    ERROR_E;
}

void CBlockPar::BlockDelete(int no)
{
    DTRACE();
    CBlockParUnit *el = m_First;
    while (el != NULL) {
        if (el->m_Type == 2) {
            if (no == 0) {
                if (m_Sort)
                    ArrayDel(el);
                UnitDel(el);
                return;
            }
            no--;
        }
        el = el->m_Next;
    }
    ERROR_E;
}

int CBlockPar::BlockCount(const std::wstring& name) const
{
    DTRACE();
    int rv = 0;

    if (m_Sort) {
        int i = ArrayFind(name);
        if (i >= 0) {
            int li = i + m_Array[i]->m_FastCnt;
            while (i < li) {
                if (m_Array[i]->m_Type == 2)
                    rv++;
                i++;
            }
        }
    }
    else {
        CBlockParUnit *el = m_First;
        while (el != NULL) {
            if ((el->m_Type == 2) && (el->m_Name == name))
                rv++;
            el = el->m_Next;
        }
    }

    return rv;
}

CBlockPar *CBlockPar::BlockGet(int no) {
    DTRACE();
    if (m_Sort && (m_Cnt == m_CntBlock)) {
        return m_Array[no]->m_Block;
    }
    else {
        CBlockParUnit *el = m_First;
        while (el != NULL) {
            if (el->m_Type == 2) {
                if (no == 0)
                    return el->m_Block;
                no--;
            }
            el = el->m_Next;
        }
        ERROR_E;
    }
}

const CBlockPar *CBlockPar::BlockGet(int no) const {
    DTRACE();
    if (m_Sort && (m_Cnt == m_CntBlock)) {
        return m_Array[no]->m_Block;
    }
    else {
        CBlockParUnit *el = m_First;
        while (el != NULL) {
            if (el->m_Type == 2) {
                if (no == 0)
                    return el->m_Block;
                no--;
            }
            el = el->m_Next;
        }
        ERROR_E;
    }
}

ParamParser CBlockPar::BlockGetName(int no) const {
    DTRACE();
    if (m_Sort && (m_Cnt == m_CntBlock)) {
        return m_Array[no]->m_Name;
    }
    else {
        CBlockParUnit *el = m_First;
        while (el != NULL) {
            if (el->m_Type == 2) {
                if (no == 0)
                    return el->m_Name;
                no--;
            }
            el = el->m_Next;
        }
        ERROR_E;
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
const std::wstring &CBlockPar::ParPathGet(const std::wstring &path)
{
    DTRACE();
    CBlockParUnit *el = UnitGet(path.c_str(), path.length());
    if (el->m_Type != 1)
        ERROR_E;
    return *el->m_Par;
}

void CBlockPar::ParPathAdd(const std::wstring &path, const std::wstring &zn)
{
    DTRACE();
    CBlockPar *cd;
    CBlockParUnit *el;

    ParamParser name{path};
    int countep = name.GetCountPar(L"./\\");

    if (countep > 1) {
        cd = BlockPathGetAdd(name.GetStrPar(0, countep - 2, L"./\\"));
        name = name.GetStrPar(countep - 1, L"./\\");
    }
    else {
        //        name:=name;
        cd = this;
    }
    el = cd->UnitAdd();
    el->ChangeType(1);
    el->m_Name = name;
    *(el->m_Par) = zn;
    if (m_Sort)
        ArrayAdd(el);
    m_CntPar++;
}

void CBlockPar::ParPathSet(const std::wstring &path, const std::wstring &zn)
{
    DTRACE();
    CBlockParUnit *te = UnitGet(path.c_str(), path.length());
    if (te->m_Type != 1)
        ERROR_E;
    *(te->m_Par) = zn;
}

void CBlockPar::ParPathSetAdd(const std::wstring &path, const std::wstring &zn)
{
    DTRACE();
    try {
        CBlockParUnit *te = UnitGet(path.c_str(), path.length());
        if (te->m_Type != 1)
            ERROR_E;
        *(te->m_Par) = zn;
    }
    catch (const CException& ex) {
        ParPathAdd(path, zn);
    }
}

void CBlockPar::ParPathDelete(const std::wstring &path)
{
    DTRACE();
    CBlockParUnit *te = UnitGet(path.c_str(), path.length());
    if (te->m_Type != 1)
        ERROR_E;
    if (te->m_Parent->m_Sort)
        te->m_Parent->ArrayDel(te);
    te->m_Parent->UnitDel(te);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CBlockPar *CBlockPar::BlockPathGet(const std::wstring &path)
{
    DTRACE();
    CBlockParUnit *el = UnitGet(path.c_str(), path.length());
    if (el->m_Type != 2)
        ERROR_E;
    return el->m_Block;
}

CBlockPar *CBlockPar::BlockPathAdd(const std::wstring &path)
{
    DTRACE();
    CBlockPar *cd;
    CBlockParUnit *el;

    ParamParser name(path);
    int countep = name.GetCountPar(L"./\\");

    if (countep > 1) {
        cd = BlockPathGetAdd(name.GetStrPar(0, countep - 2, L"./\\"));
        name = name.GetStrPar(countep - 1, L"./\\");
    }
    else {
        //        name:=name;
        cd = this;
    }
    el = cd->UnitAdd();
    el->ChangeType(2);
    el->m_Name = name;
    if (m_Sort)
        ArrayAdd(el);
    m_CntBlock++;
    return el->m_Block;
}

CBlockPar *CBlockPar::BlockPathGetAdd(const std::wstring &path)
{
    DTRACE();
    try {
        return BlockPathGet(path);
    }
    catch (const CException& ex) {
        return BlockPathAdd(path);
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int CBlockPar::AllGetType(int no) {
    DTRACE();
    if (m_Sort && (m_Cnt == m_Array.size())) {
        return m_Array[no]->m_Type;
    }
    else {
        CBlockParUnit *el = m_First;
        while (el != NULL) {
            if (no == 0)
                return el->m_Type;
            no--;
            el = el->m_Next;
        }
        ERROR_E;
    }
}

CBlockPar *CBlockPar::AllGetBlock(int no) {
    DTRACE();
    CBlockParUnit *el;
    if (m_Sort && (m_Cnt == m_Array.size())) {
        el = m_Array[no];
        if (el->m_Type != 2)
            ERROR_E;
        return el->m_Block;
    }
    else {
        CBlockParUnit *el = m_First;
        while (el != NULL) {
            if (no == 0) {
                if (el->m_Type != 2)
                    ERROR_E;
                return el->m_Block;
            }
            no--;
            el = el->m_Next;
        }
        ERROR_E;
    }
}

const std::wstring &CBlockPar::AllGetPar(int no) {
    DTRACE();
    CBlockParUnit *el;
    if (m_Sort && (m_Cnt == m_Array.size())) {
        el = m_Array[no];
        if (el->m_Type != 1)
            ERROR_E;
        return *el->m_Par;
    }
    else {
        CBlockParUnit *el = m_First;
        while (el != NULL) {
            if (no == 0) {
                if (el->m_Type != 1)
                    ERROR_E;
                return *el->m_Par;
            }
            no--;
            el = el->m_Next;
        }
        ERROR_E;
    }
}

const std::wstring &CBlockPar::AllGetName(int no) {
    DTRACE();
    CBlockParUnit *el;
    if (m_Sort && (m_Cnt == m_Array.size())) {
        el = m_Array[no];
        if (el->m_Type != 1 && el->m_Type != 2)
            ERROR_E;
        return el->m_Name;
    }
    else {
        CBlockParUnit *el = m_First;
        while (el != NULL) {
            if (no == 0) {
                if (el->m_Type != 1 && el->m_Type != 2)
                    ERROR_E;
                return el->m_Name;
            }
            no--;
            el = el->m_Next;
        }
        ERROR_E;
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class BPCompiler {
public:
    CBlockPar *m_BP;
    const wchar *m_Text;
    int m_TextLen;

    int m_Line;
    int m_LineEnd;
    int m_Com, m_ComSpace;
    int m_Block;
    int m_Par;

    int m_WordBegin;
    int m_WordEnd;
    int m_WordSearchEnd;

    int m_FileBegin, m_FileEnd;

public:
    BPCompiler(CBlockPar *bp, const wchar *text, int textlen) {
        m_BP = bp;
        m_Text = text;
        m_TextLen = textlen;
    }
    ~BPCompiler() {}

    bool IsSpace(wchar ch) { return ch == 0x20 || ch == 0x09; }
    bool IsEqual(const wchar *str1, int str1len, const wchar *str2, const wchar *str3, int str23len) {
        DTRACE();
        if (str1len != str23len)
            return false;
        if (str1len <= 0)
            return true;
        for (int i = 0; i < str1len; i++)
            if ((str1[i] != str2[i]) && (str1[i] != str3[i]))
                return false;
        return true;
    }
    bool CalcEndLine(void) {
        DTRACE();
        m_LineEnd = m_Line;
        while ((m_LineEnd < m_TextLen) && (m_Text[m_LineEnd] != 0xa) && (m_Text[m_LineEnd] != 0xd))
            m_LineEnd++;
        return m_LineEnd > m_Line;
    }
    bool CalcNextLine(void) {
        DTRACE();
        m_Line = m_LineEnd;
        while ((m_Line < m_TextLen) && (m_Text[m_Line] != 0xa))
            m_Line++;
        if (m_Line < m_TextLen) {
            m_Line++;
            return true;
        }
        else
            return false;
    }
    void CalcSkipSpace() {
        DTRACE();
        while ((m_Line < m_TextLen) && IsSpace(m_Text[m_Line]))
            m_Line++;
    }
    void FindCom() {
        DTRACE();
        m_Com = m_Line;
        while ((m_Com + 1 < m_LineEnd) && ((m_Text[m_Com] != '/') || (m_Text[m_Com + 1] != '/')))
            m_Com++;
        if (m_Com + 1 >= m_LineEnd) {
            m_Com = m_LineEnd;
            m_ComSpace = m_Com;
            return;
        }
        m_ComSpace = m_Com - 1;
        while ((m_ComSpace >= m_Line) && (m_Text[m_ComSpace] == 0x20 || m_Text[m_ComSpace] == 0x09))
            m_ComSpace--;
        m_ComSpace++;
    }
    bool FindBlock() {
        DTRACE();
        m_Block = m_Line;
        while ((m_Block < m_ComSpace) && (m_Text[m_Block] != '{'))
            m_Block++;
        if (m_Block >= m_ComSpace) {
            m_Block = -1;
            return false;
        }
        return true;
    }
    bool FindPar() {
        DTRACE();
        m_Par = m_Line;
        while ((m_Par < m_ComSpace) && (m_Text[m_Par] != '='))
            m_Par++;
        if (m_Par >= m_ComSpace) {
            m_Par = -1;
            return false;
        }
        return true;
    }
    int ParNameSize() {
        DTRACE();
        int sme = m_Par - 1;
        while ((sme >= m_Line) && IsSpace(m_Text[sme]))
            sme--;
        sme++;
        return sme - m_Line;
    }
    bool WordFindBlockName() {
        DTRACE();
        while ((m_WordBegin < m_WordSearchEnd) && IsSpace(m_Text[m_WordBegin]))
            m_WordBegin++;
        m_WordEnd = m_WordBegin;
        if (m_WordEnd >= m_WordSearchEnd)
            return false;
        if ((m_Text[m_WordEnd] == '-') || (m_Text[m_WordEnd] == '='))
            return false;
        while ((m_WordEnd < m_WordSearchEnd) && !IsSpace(m_Text[m_WordEnd]) && (m_Text[m_WordEnd] != '='))
            m_WordEnd++;
        return m_WordBegin < m_WordEnd;
    }
    bool WordFindOption() {
        DTRACE();
        while ((m_WordBegin < m_WordSearchEnd) && IsSpace(m_Text[m_WordBegin]))
            m_WordBegin++;
        m_WordEnd = m_WordBegin;
        while ((m_WordEnd < m_WordSearchEnd) && !IsSpace(m_Text[m_WordEnd]))
            m_WordEnd++;
        return m_WordBegin < m_WordEnd;
    }
    bool IsEmptyBlock() {
        DTRACE();
        int sme = m_Block + 1;
        while ((sme < m_ComSpace) && IsSpace(m_Text[sme]))
            sme++;
        if (sme >= m_ComSpace)
            return false;
        if (m_Text[sme] != '}')
            return false;
        m_Block = sme;
        return true;
    }

    int Run(void) {
        DTRACE();
        CBlockParUnit *unit;
        m_Line = 0;
        CalcSkipSpace();
        //			if(!CalcEndLine()) return m_Line;
        CalcEndLine();
        FindCom();

        while (m_Line < m_TextLen) {
            if (m_Line >= m_ComSpace) {
                unit = m_BP->UnitAdd();
                if (m_ComSpace < m_LineEnd)
                    unit->m_Com = std::wstring{m_Text + m_ComSpace, static_cast<size_t>(m_LineEnd - m_ComSpace)};
                if (m_BP->m_Sort)
                    m_BP->ArrayAdd(unit);
            }
            else if (m_Text[m_Line] == '}') {
                m_Line++;
                //					CalcSkipSpace();
                //					if(m_Line<m_TextLen && (m_Text[m_Line]==0x0d)) m_Line++;
                //					if(m_Line<m_TextLen && (m_Text[m_Line]==0x0a)) m_Line++;
                return m_Line;
            }
            else if (FindBlock()) {
                m_WordBegin = m_Line;
                m_WordSearchEnd = m_Block;

                unit = m_BP->UnitAdd();
                unit->ChangeType(2);
                m_BP->m_CntBlock++;

                if (WordFindBlockName()) {
                    unit->m_Name = std::wstring{m_Text + m_WordBegin, static_cast<size_t>(m_WordEnd - m_WordBegin)};
                    m_WordBegin = m_WordEnd;
                }

                if (m_BP->m_Sort)
                    m_BP->ArrayAdd(unit);

                m_FileBegin = m_FileEnd = -1;

                while (WordFindOption()) {
                    if (m_Text[m_WordBegin] == '=') {
                        m_FileBegin = m_WordBegin + 1;
                        m_FileEnd = m_WordEnd;
                    }
                    else if (IsEqual(m_Text + m_WordBegin, m_WordEnd - m_WordBegin, L"-sort", L"-SORT", 5)) {
                        unit->m_Block->m_Sort = false;
                    }
                    else if (IsEqual(m_Text + m_WordBegin, m_WordEnd - m_WordBegin, L"+sort", L"+SORT", 5)) {
                        unit->m_Block->m_Sort = true;
                    }

                    m_WordBegin = m_WordEnd;
                }

                if (IsEmptyBlock()) {
                    if (m_ComSpace < m_LineEnd && (m_Block + 1) >= m_ComSpace) {
                        unit->m_Com = std::wstring{m_Text + m_ComSpace, static_cast<size_t>(m_LineEnd - m_ComSpace)};
                    }
                    else {
                        m_Line = m_Block + 1;
                        CalcSkipSpace();
                        if (m_Line < m_LineEnd)
                            continue;
                    }
                }
                else {
                    if (m_ComSpace < m_LineEnd && (m_Block + 1) >= m_ComSpace) {
                        unit->m_Com = std::wstring{m_Text + m_ComSpace, static_cast<size_t>(m_LineEnd - m_ComSpace)};

                        if (!CalcNextLine())
                            break;

                        m_Line = unit->m_Block->LoadFromText(std::wstring{m_Text + m_Line, static_cast<size_t>(m_TextLen - m_Line)}) + m_Line;
                    }
                    else {
                        m_Line = m_Block + 1;
                        CalcSkipSpace();
                        if (m_Line < m_TextLen && (m_Text[m_Line] == 0x0d))
                            m_Line++;
                        if (m_Line < m_TextLen && (m_Text[m_Line] == 0x0a))
                            m_Line++;

                        m_Line = unit->m_Block->LoadFromText(std::wstring{m_Text + m_Line, static_cast<size_t>(m_TextLen - m_Line)}) + m_Line;
                    }
                    CalcSkipSpace();
                    CalcEndLine();
                    FindCom();
                    if (m_Line < m_LineEnd)
                        continue;
                }
                if (m_FileBegin >= 0) {
                    unit->m_Block->LoadFromTextFile(std::wstring{m_Text + m_FileBegin, static_cast<size_t>(m_FileEnd - m_FileBegin)});
                    unit->m_Block->m_FromFile = std::wstring{m_Text + m_FileBegin, static_cast<size_t>(m_FileEnd - m_FileBegin)};
                }
            }
            else if (FindPar()) {
                unit = m_BP->UnitAdd();
                unit->ChangeType(1);
                m_BP->m_CntPar++;

                unit->m_Name = std::wstring{m_Text + m_Line, static_cast<size_t>(ParNameSize())};
                *(unit->m_Par) = std::wstring{m_Text + m_Par + 1, static_cast<size_t>(m_ComSpace - (m_Par + 1))};

                if (m_ComSpace < m_LineEnd)
                    unit->m_Com = std::wstring{m_Text + m_ComSpace, static_cast<size_t>(m_LineEnd - m_ComSpace)};

                if (m_BP->m_Sort)
                    m_BP->ArrayAdd(unit);
            }
            else {
                unit = m_BP->UnitAdd();
                unit->ChangeType(1);
                m_BP->m_CntPar++;

                *(unit->m_Par) = std::wstring{m_Text + m_Line, static_cast<size_t>(m_ComSpace - m_Line)};

                if (m_ComSpace < m_LineEnd)
                    unit->m_Com = std::wstring{m_Text + m_ComSpace, static_cast<size_t>(m_LineEnd - m_ComSpace)};

                if (m_BP->m_Sort)
                    m_BP->ArrayAdd(unit);
            }

            if (!CalcNextLine())
                break;
            CalcSkipSpace();
            // if(!CalcEndLine()) break;
            CalcEndLine();
            FindCom();
        }

        return m_Line;
    }
};

int CBlockPar::LoadFromText(const std::wstring &text) {
    DTRACE();
    Clear();

    BPCompiler co(this, text.c_str(), text.length());
    return co.Run();
}

void CBlockPar::LoadFromTextFile(const std::wstring &filename) {
    DTRACE();
    CFile fi(filename);
    fi.OpenRead();

    word zn;
    bool fansi = true;
    int fs = fi.Size();
    if ((fs >= 2) && !(fs & 1)) {
        fi.Read(&zn, 2);
        fansi = zn != 0x0feff;
        if (fansi)
            fi.Pointer(0);
    }
    fs -= fi.Pointer();
    if (fs > 0) {
        if (fansi) {
            std::string astr(fs, ' '); // TODO: maybe not spaces?

            fi.Read(&astr[0], fs);
            std::wstring wstr = utils::to_wstring(astr.c_str());

            LoadFromText(wstr);
        }
        else {
            std::wstring wstr;
            wstr.resize(fs / 2);
            fi.Read(&wstr[0], (fs / 2) * 2); // TODO: wtf?
            LoadFromText(wstr);
        }
    }

    fi.Close();
}

void CBlockPar::SaveInText(CBuf &buf, bool ansi, int level) {
    DTRACE();

#define SaveLevel                      \
    {                                  \
        if (ansi)                      \
            buf.ByteLoop(0x09, level); \
        else                           \
            buf.WordLoop(0x09, level); \
    }
#define SaveStr(str)              \
    {                             \
        if (ansi)                 \
            buf.StrNZ(utils::from_wstring(str).c_str()); \
        else                      \
            buf.WStrNZ(str);      \
    }
#define SaveSpace           \
    {                       \
        if (ansi)           \
            buf.Byte(0x20); \
        else                \
            buf.Word(0x20); \
    }
#define SaveStrConst(cstr)       \
    {                            \
        if (ansi)                \
            buf.StrNZ(cstr);     \
        else                     \
            buf.WStrNZ(L##cstr); \
    }
#define SaveNewLine                \
    {                              \
        if (ansi)                  \
            buf.Word(0x0a0d);      \
        else                       \
            buf.Dword(0x000a000d); \
    }

    bool addspace;

    CBlockParUnit *unit = m_First;
    while (unit != 0) {
        SaveLevel;

        if (unit->m_Type == 1) {
            if (!unit->m_Name.empty()) {
                SaveStr(unit->m_Name.c_str());
                SaveStrConst("=");
            }
            SaveStr(unit->m_Par->c_str());
            if (!unit->m_Com.empty())
                SaveStr(unit->m_Com.c_str());
        }
        else if (unit->m_Type == 2) {
            addspace = false;
            if (!unit->m_Name.empty()) {
                SaveStr(unit->m_Name.c_str());
                addspace = true;
            }

            if (!unit->m_Block->m_FromFile.empty())
            {
                SaveStrConst("=");
                SaveStr(unit->m_Block->m_FromFile.c_str());
                SaveStrConst(" {}");

                unit->m_Block->SaveInTextFile(unit->m_Block->m_FromFile, ansi);
            }
            else {
                if (!unit->m_Block->m_Sort) {
                    if (addspace)
                        SaveSpace;
                    SaveStrConst("-sort");
                    addspace = true;
                }

                if (addspace)
                    SaveSpace;
                SaveStrConst("{");

                if (unit->m_Block->AllCount() == 0) {
                    SaveStrConst("}");
                    if (!unit->m_Com.empty())
                        SaveStr(unit->m_Com.c_str());
                }
                else {
                    if (!unit->m_Com.empty())
                        SaveStr(unit->m_Com.c_str());
                    SaveNewLine;

                    unit->m_Block->SaveInText(buf, ansi, level + 1);

                    SaveLevel;
                    SaveStrConst("}");
                }
            }
        }
        else {
            if (!unit->m_Com.empty())
                SaveStr(unit->m_Com.c_str());
        }

        SaveNewLine;

        unit = unit->m_Next;
    }
}

void CBlockPar::SaveInTextFile(const std::wstring &filename, bool ansi)
{
    DTRACE();

    CBuf buf(m_Heap);
    if (!ansi)
        buf.Word(0x0feff);
    SaveInText(buf, ansi);

    CFile fi(filename, m_Heap);
    fi.Create();
    fi.Write(buf.Get(), buf.Len());
    fi.Close();
}

}  // namespace Base
