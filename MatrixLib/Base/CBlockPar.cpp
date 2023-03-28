// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <string>
#include <algorithm>

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

uint32_t ParamParser::GetDword() const {
    int tlen = length();
    if (tlen < 1)
        return 0;
    const wchar *tstr = c_str();

    uint32_t zn = 0;
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

uint32_t ParamParser::GetHexUnsigned(void) const {
    int tlen = length();
    if (tlen < 1)
        return 0;
    const wchar *tstr = c_str();

    uint32_t zn = 0;
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

CBlockParUnit::CBlockParUnit(Type type)
: CMain{}
, m_Parent{nullptr}
, m_Type{type}
{
    DTRACE();

    if (isPar())
        m_Par = new std::wstring{};
    else if (isBlock())
        m_Block = new CBlockPar{};
}

CBlockParUnit::~CBlockParUnit() {
    DTRACE();

    if (isPar())
    {
        if (m_Par != nullptr)
        {
            delete m_Par;
        }
    }
    else if (isBlock())
    {
        if (m_Block != nullptr)
        {
            delete m_Block;
        }
    }
}

CBlockParUnit& CBlockParUnit::operator = (const CBlockParUnit& that)
{
    // TODO: we don't check if unit has the same type here only because there
    // is only one usage of this function. but to be updated...
    DTRACE();
    m_Name = that.m_Name;
    m_Com = that.m_Com;
    if (isPar())
        *m_Par = *that.m_Par;
    else if (isBlock())
        m_Block->CopyFrom(*that.m_Block);

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CBlockPar::CBlockPar() : CMain()
{
    DTRACE();

    m_CntPar = 0;
    m_CntBlock = 0;
}

CBlockPar::~CBlockPar() {
    DTRACE();
    Clear();
}

void CBlockPar::Clear() {
    DTRACE();

    m_CntPar = 0;
    m_CntBlock = 0;

    m_Units.clear();
    m_FromFile.clear();
}

void CBlockPar::CopyFrom(CBlockPar &bp) {
    DTRACE();
    Clear();
    for(auto& el : m_Units)
    {
        CBlockParUnit& el2 = UnitAdd(el.m_Type);
        el2 = el;

        if (el.isPar())
            m_CntPar++;
        else if (el.isBlock())
            m_CntBlock++;
    }
}

CBlockParUnit& CBlockPar::UnitAdd(CBlockParUnit::Type type) {
    DTRACE();

    m_Units.emplace_back(type);

    m_Units.back().m_Parent = this;

    return m_Units.back();
}

void CBlockPar::UnitDel(CBlockParUnit& el) {
    DTRACE();

    if (el.isPar())
        m_CntPar--;
    else if (el.isBlock())
        m_CntBlock--;

    size_t removed = m_Units.remove_if([&el](auto& unit){ return &el == &unit; });

    if (removed != 1)
    {
        ERROR_S(L"Not a single unit removed by UnitDel call");
    }
}

CBlockParUnit& CBlockPar::UnitGet(const std::wstring &path)
{
    DTRACE();
    wchar ch;
    int sme, smeend, no;
    int name_sme, name_len, name_next;

    name_next = 0;

    CBlockPar *us = this;
    CBlockParUnit *ne = nullptr;

    for (;;) {
        // nameExtractNext
        if (name_next >= path.length())
            break;
        name_sme = name_next;
        sme = name_sme;
        while (sme < path.length()) {
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

        int u = 0;
        for(auto& el : us->m_Units)
        {
            if (el.m_Name == std::wstring{path.c_str() + name_sme, static_cast<size_t>(name_len)})
            {
                if (u == no)
                {
                    ne = &el;
                    break;
                }
                u++;
            }
        }

        if (ne == nullptr)
            ERROR_S2(L"Path not found: ", path.c_str());
        if (name_next >= path.length())
            break;
        if (!ne->isBlock())
            ERROR_S2(L"Path not found: ", path.c_str());
        us = ne->m_Block;
    }
    if (ne == nullptr)
        ERROR_S2(L"Path not found: ", path.c_str());

    return *ne;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CBlockParUnit *CBlockPar::ParAdd(const std::wstring& name, const std::wstring& zn) {
    DTRACE();
    CBlockParUnit& el = UnitAdd(CBlockParUnit::Type::Par);
    el.m_Name = name;
    *(el.m_Par) = zn;

    m_CntPar++;

    return &el;
}

void CBlockPar::ParSetAdd(const std::wstring& name, const std::wstring& zn)
{
    DTRACE();

    auto pred = [&name](const auto& u) { return u.isPar() && (u.m_Name == name); };
    auto res = std::ranges::find_if(m_Units, pred);

    if (res == m_Units.end())
    {
        ParAdd(name, zn);
        return;
    }

    *(res->m_Par) = zn;
}

void CBlockPar::ParDelete(int no)
{
    DTRACE();

    auto pred = [&no](const auto& u) { return u.isPar() && (no-- == 0); };
    auto res = std::ranges::find_if(m_Units, pred);

    if (res == m_Units.end())
    {
        ERROR_E;
    }

    UnitDel(*res);
}

ParamParser CBlockPar::ParGet(const std::wstring& name, int index) const
{
    DTRACE();

    auto pred = [&index, &name](const auto& u) { return u.isPar() && (u.m_Name == name) && (index-- == 0); };
    auto res = std::ranges::find_if(m_Units, pred);

    if (res == m_Units.end())
    {
        ERROR_S2(L"Not found: ", name.c_str());
    }

    return *(res->m_Par);
}

ParamParser CBlockPar::ParGetNE(const std::wstring& name, int index) const
{
    DTRACE();

    auto pred = [&index, &name](const auto& u) { return u.isPar() && (u.m_Name == name) && (index-- == 0); };
    auto res = std::ranges::find_if(m_Units, pred);

    if (res == m_Units.end())
    {
        return std::wstring();
    }

    return *(res->m_Par);
}

int CBlockPar::ParCount(const std::wstring& name) const
{
    DTRACE();

    auto pred = [&name](const auto& u) { return u.isPar() && (u.m_Name == name); };
    return std::ranges::count_if(m_Units, pred);
}

ParamParser CBlockPar::ParGet(int no) const
{
    DTRACE();

    auto pred = [&no](const auto& u) { return u.isPar() && (no-- == 0); };
    auto res = std::ranges::find_if(m_Units, pred);

    if (res == m_Units.end())
    {
        ERROR_E;
    }

    return *(res->m_Par);
}

ParamParser CBlockPar::ParGetName(int no) const
{
    DTRACE();

    auto pred = [&no](const auto& u) { return u.isPar() && (no-- == 0); };
    auto res = std::ranges::find_if(m_Units, pred);

    if (res == m_Units.end())
    {
        ERROR_E;
    }

    return res->m_Name;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CBlockPar *CBlockPar::BlockAdd(const std::wstring& name)
{
    DTRACE();
    CBlockParUnit& el = UnitAdd(CBlockParUnit::Type::Block);
    el.m_Name = name;
    m_CntBlock++;

    return el.m_Block;
}

CBlockPar *CBlockPar::BlockGetNE(const std::wstring& name)
{
    DTRACE();

    auto pred = [&name](const auto& u) { return u.isBlock() && (u.m_Name == name); };
    auto res = std::ranges::find_if(m_Units, pred);

    if (res == m_Units.end())
    {
        return nullptr;
    }

    return res->m_Block;
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

    auto pred = [&name](const auto& u) { return u.isBlock() && (u.m_Name == name); };
    auto res = std::ranges::find_if(m_Units, pred);

    if (res == m_Units.end())
    {
        ERROR_E;
    }

    UnitDel(*res);

}

void CBlockPar::BlockDelete(int no)
{
    DTRACE();

    auto pred = [&no](const auto& u) { return u.isBlock() && (no-- == 0); };
    auto res = std::ranges::find_if(m_Units, pred);

    if (res == m_Units.end())
    {
        ERROR_E;
    }

    UnitDel(*res);
}

int CBlockPar::BlockCount(const std::wstring& name) const
{
    DTRACE();

    auto pred = [&name](const auto& u) { return u.isBlock() && (u.m_Name == name); };
    return std::ranges::count_if(m_Units, pred);
}

CBlockPar* CBlockPar::BlockGet(int no)
{
    DTRACE();

    auto pred = [&no](const auto& u) { return u.isBlock() && (no-- == 0); };
    auto res = std::ranges::find_if(m_Units, pred);

    if (res == m_Units.end())
    {
        ERROR_E;
    }

    return res->m_Block;
}

const CBlockPar* CBlockPar::BlockGet(int no) const
{
    DTRACE();

    auto pred = [&no](const auto& u) { return u.isBlock() && (no-- == 0); };
    auto res = std::ranges::find_if(m_Units, pred);

    if (res == m_Units.end())
    {
        ERROR_E;
    }

    return res->m_Block;
}

ParamParser CBlockPar::BlockGetName(int no) const {
    DTRACE();

    auto pred = [&no](const auto& u) { return u.isBlock() && (no-- == 0); };
    auto res = std::ranges::find_if(m_Units, pred);

    if (res == m_Units.end())
    {
        ERROR_E;
    }

    return res->m_Name;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
const std::wstring& CBlockPar::ParPathGet(const std::wstring &path)
{
    DTRACE();
    CBlockParUnit& el = UnitGet(path);
    if (!el.isPar())
    {
        ERROR_E;
    }
    return *el.m_Par;
}

void CBlockPar::ParPathAdd(const std::wstring &path, const std::wstring &zn)
{
    DTRACE();
    CBlockPar *cd;

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
    CBlockParUnit& el = cd->UnitAdd(CBlockParUnit::Type::Par);
    el.m_Name = name;
    *(el.m_Par) = zn;
    m_CntPar++;
}

void CBlockPar::ParPathSet(const std::wstring &path, const std::wstring &zn)
{
    DTRACE();
    CBlockParUnit& te = UnitGet(path);
    if (!te.isPar())
        ERROR_E;
    *(te.m_Par) = zn;
}

void CBlockPar::ParPathSetAdd(const std::wstring &path, const std::wstring &zn)
{
    DTRACE();
    try {
        CBlockParUnit& te = UnitGet(path);
        if (!te.isPar())
            ERROR_E;
        *(te.m_Par) = zn;
    }
    catch (const CException&) {
        ParPathAdd(path, zn);
    }
}

void CBlockPar::ParPathDelete(const std::wstring &path)
{
    DTRACE();
    CBlockParUnit& te = UnitGet(path);
    if (!te.isPar())
        ERROR_E;
    te.m_Parent->UnitDel(te);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CBlockPar *CBlockPar::BlockPathGet(const std::wstring &path)
{
    DTRACE();
    CBlockParUnit& el = UnitGet(path);
    if (!el.isBlock())
        ERROR_E;
    return el.m_Block;
}

CBlockPar *CBlockPar::BlockPathAdd(const std::wstring &path)
{
    DTRACE();
    CBlockPar *cd;

    ParamParser name(path);
    int countep = name.GetCountPar(L"./\\");

    if (countep > 1) {
        cd = BlockPathGetAdd(name.GetStrPar(0, countep - 2, L"./\\"));
        name = name.GetStrPar(countep - 1, L"./\\");
    }
    else {
        cd = this;
    }
    CBlockParUnit& el = cd->UnitAdd(CBlockParUnit::Type::Block);
    el.m_Name = name;
    m_CntBlock++;
    return el.m_Block;
}

CBlockPar *CBlockPar::BlockPathGetAdd(const std::wstring &path)
{
    DTRACE();
    try {
        return BlockPathGet(path);
    }
    catch (const CException&) {
        return BlockPathAdd(path);
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
        m_Line = 0;
        CalcSkipSpace();
        //			if(!CalcEndLine()) return m_Line;
        CalcEndLine();
        FindCom();

        while (m_Line < m_TextLen) {
            if (m_Line >= m_ComSpace) {
                CBlockParUnit& unit = m_BP->UnitAdd(CBlockParUnit::Type::Empty);
                if (m_ComSpace < m_LineEnd)
                    unit.m_Com = std::wstring{m_Text + m_ComSpace, static_cast<size_t>(m_LineEnd - m_ComSpace)};
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

                CBlockParUnit& unit = m_BP->UnitAdd(CBlockParUnit::Type::Block);
                m_BP->m_CntBlock++;

                if (WordFindBlockName()) {
                    unit.m_Name = std::wstring{m_Text + m_WordBegin, static_cast<size_t>(m_WordEnd - m_WordBegin)};
                    m_WordBegin = m_WordEnd;
                }

                m_FileBegin = m_FileEnd = -1;

                while (WordFindOption()) {
                    if (m_Text[m_WordBegin] == '=') {
                        m_FileBegin = m_WordBegin + 1;
                        m_FileEnd = m_WordEnd;
                    }

                    m_WordBegin = m_WordEnd;
                }

                if (IsEmptyBlock()) {
                    if (m_ComSpace < m_LineEnd && (m_Block + 1) >= m_ComSpace) {
                        unit.m_Com = std::wstring{m_Text + m_ComSpace, static_cast<size_t>(m_LineEnd - m_ComSpace)};
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
                        unit.m_Com = std::wstring{m_Text + m_ComSpace, static_cast<size_t>(m_LineEnd - m_ComSpace)};

                        if (!CalcNextLine())
                            break;

                        m_Line = unit.m_Block->LoadFromText(std::wstring{m_Text + m_Line, static_cast<size_t>(m_TextLen - m_Line)}) + m_Line;
                    }
                    else {
                        m_Line = m_Block + 1;
                        CalcSkipSpace();
                        if (m_Line < m_TextLen && (m_Text[m_Line] == 0x0d))
                            m_Line++;
                        if (m_Line < m_TextLen && (m_Text[m_Line] == 0x0a))
                            m_Line++;

                        m_Line = unit.m_Block->LoadFromText(std::wstring{m_Text + m_Line, static_cast<size_t>(m_TextLen - m_Line)}) + m_Line;
                    }
                    CalcSkipSpace();
                    CalcEndLine();
                    FindCom();
                    if (m_Line < m_LineEnd)
                        continue;
                }
                if (m_FileBegin >= 0) {
                    unit.m_Block->LoadFromTextFile(std::wstring{m_Text + m_FileBegin, static_cast<size_t>(m_FileEnd - m_FileBegin)});
                    unit.m_Block->m_FromFile = std::wstring{m_Text + m_FileBegin, static_cast<size_t>(m_FileEnd - m_FileBegin)};
                }
            }
            else if (FindPar()) {
                CBlockParUnit& unit = m_BP->UnitAdd(CBlockParUnit::Type::Par);
                m_BP->m_CntPar++;

                unit.m_Name = std::wstring{m_Text + m_Line, static_cast<size_t>(ParNameSize())};
                *(unit.m_Par) = std::wstring{m_Text + m_Par + 1, static_cast<size_t>(m_ComSpace - (m_Par + 1))};

                if (m_ComSpace < m_LineEnd)
                    unit.m_Com = std::wstring{m_Text + m_ComSpace, static_cast<size_t>(m_LineEnd - m_ComSpace)};
            }
            else {
                CBlockParUnit& unit = m_BP->UnitAdd(CBlockParUnit::Type::Par);
                m_BP->m_CntPar++;

                *(unit.m_Par) = std::wstring{m_Text + m_Line, static_cast<size_t>(m_ComSpace - m_Line)};

                if (m_ComSpace < m_LineEnd)
                    unit.m_Com = std::wstring{m_Text + m_ComSpace, static_cast<size_t>(m_LineEnd - m_ComSpace)};
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
            std::string astr;
            astr.resize(fs);
            fi.Read(&astr[0], fs);
            LoadFromText(utils::to_wstring(astr.c_str()));
        }
        else {
            std::wstring wstr;
            wstr.resize(fs / 2);
            fi.Read(&wstr[0], fs);
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
            buf.Add<uint8_t>(0x20); \
        else                \
            buf.Add<uint16_t>(0x20); \
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
            buf.Add<uint16_t>(0x0a0d);      \
        else                       \
            buf.Add<uint32_t>(0x000a000d); \
    }

    bool addspace;

    for(auto& unit : m_Units)
    {
        SaveLevel;

        if (unit.isPar()) {
            if (!unit.m_Name.empty()) {
                SaveStr(unit.m_Name.c_str());
                SaveStrConst("=");
            }
            SaveStr(unit.m_Par->c_str());
            if (!unit.m_Com.empty())
                SaveStr(unit.m_Com.c_str());
        }
        else if (unit.isBlock())
        {
            addspace = false;
            if (!unit.m_Name.empty()) {
                SaveStr(unit.m_Name.c_str());
                addspace = true;
            }

            if (!unit.m_Block->m_FromFile.empty())
            {
                SaveStrConst("=");
                SaveStr(unit.m_Block->m_FromFile.c_str());
                SaveStrConst(" {}");

                unit.m_Block->SaveInTextFile(unit.m_Block->m_FromFile, ansi);
            }
            else {
                if (addspace)
                    SaveSpace;
                SaveStrConst("{");

                if (unit.m_Block->m_Units.size() == 0) {
                    SaveStrConst("}");
                    if (!unit.m_Com.empty())
                        SaveStr(unit.m_Com.c_str());
                }
                else {
                    if (!unit.m_Com.empty())
                        SaveStr(unit.m_Com.c_str());
                    SaveNewLine;

                    unit.m_Block->SaveInText(buf, ansi, level + 1);

                    SaveLevel;
                    SaveStrConst("}");
                }
            }
        }
        else {
            if (!unit.m_Com.empty())
                SaveStr(unit.m_Com.c_str());
        }

        SaveNewLine;
    }
}

void CBlockPar::SaveInTextFile(const std::wstring &filename, bool ansi)
{
    DTRACE();

    CBuf buf;
    if (!ansi)
        buf.Add<uint16_t>(0x0feff);
    SaveInText(buf, ansi);

    CFile fi(filename);
    fi.Create();
    fi.Write(buf.Get(), buf.Len());
    fi.Close();
}

}  // namespace Base
