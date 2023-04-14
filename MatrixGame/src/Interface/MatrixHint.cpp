// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixHint.hpp"
#include "../MatrixGameDll.hpp"
#include "../MatrixInstantDraw.hpp"
#include "../MatrixSampleStateManager.hpp"

#include "StringConstants.hpp"
#include "CFile.hpp"

#include <vector>

CMatrixHint *CMatrixHint::m_First;
CMatrixHint *CMatrixHint::m_Last;

SHintBitmap *CMatrixHint::m_Bitmaps;
int CMatrixHint::m_BitmapsCnt;

CMatrixHint *CMatrixHint::Build(int border, const std::wstring &soundin, const std::wstring &soundout, SHintElement *elems,
                                CRect *otstup) {
    CBitmap bmps;
    CBlockPar *bph = NULL;
    if (border >= 0) {
        bph = g_MatrixData->BlockGet(PAR_SOURCE_HINTS)->BlockGetNE(utils::format(L"%d", border));

        if (bph) {
            std::wstring src;
            if (!CFile::FileExist(src, bph->ParGet(PAR_SOURCE_HINTS_SOURCE).c_str(), L"png")) {
                // return NULL;
            }
            else {
                bmps.LoadFromPNG(src.c_str());
                bmps.SwapByte(CPoint(0, 0), bmps.Size(), 0, 2);
            }
        }
    }

    int cx = 0;
    int cy = 0;
    int cw = 0;
    int ch = 0;

    std::vector<CPoint> pos;

    bool new_coord_f = false;
    CPoint new_coord;

    SHintElement *els = elems;
    for (;; ++els) {
        int bx = 0;
        int by = 0;
        if (els->bmp && els->hem != HEM_COORD && els->hem != HEM_DOWN && els->hem != HEM_RIGHT) {
            bx = els->bmp->SizeX();
            by = els->bmp->SizeY();
        }

        if (els->hem == HEM_BITMAP) {
            if (new_coord_f) {
                pos.push_back(new_coord);
                new_coord_f = false;
            }
            else {
                pos.push_back(CPoint(cx, cy));
                cx += bx;
                if (cx > cw)
                    cw = cx;
                if ((cy + by) > ch)
                    ch = by + cy;
            }
        }
        else if (els->hem == HEM_LAST_ON_LINE) {
            pos.push_back(CPoint(cx, cy));
            cx += bx;
            if (cx > cw)
                cw = cx;
            if ((cy + by) > ch)
                ch = by + cy;
            cx = 0;
            cy = ch;
        }
        else if (els->hem == HEM_LAST) {
            pos.push_back(CPoint(cx, cy));
            cx += bx;
            if (cx > cw)
                cw = cx;
            if ((cy + by) > ch)
                ch = by + cy;
            break;
        }
        else if (els->hem == HEM_CENTER) {
            pos.push_back(CPoint(-1000, cy));
            if (bx > cw)
                cw = bx;
            if ((cy + by) > ch)
                ch = by + cy;
            cx = 0;
            cy = ch;
        }
        else if (els->hem <= HEM_TAB_LARGEST) {
            pos.push_back(CPoint(els->hem, cy));
            cx = els->hem + bx;
            if (cx > cw)
                cw = cx;
            if ((cy + by) > ch)
                ch = by + cy;
        }
        else if (els->hem <= HEM_CENTER_RIGHT_LARGEST) {
            pos.push_back(CPoint(-1001, cy));

            int tv = els->hem - HEM_TAB_LARGEST;
            if ((tv + bx) > (cw / 2))
                cw = (tv + bx) * 2;
            cx = cw / 2 + tv + bx;
            if ((cy + by) > ch)
                ch = by + cy;
        }
        else if (els->hem <= HEM_CENTER_LEFT_LARGEST) {
            pos.push_back(CPoint(-1002, cy));

            int tv = els->hem - HEM_CENTER_RIGHT_LARGEST;
            if ((tv + bx) > (cw / 2))
                cw = (tv + bx) * 2;
            cx = cw / 2 - tv;
            if ((cy + by) > ch)
                ch = by + cy;
        }
        else if (els->hem == HEM_COPY) {
            pos.push_back(CPoint(0, 0));
            // do nothing
        }
        else if (els->hem == HEM_DOWN) {
            pos.push_back(CPoint(0, 0));
            cy += els->y;
            if (cy > ch)
                ch = cy;
            els->bmp = NULL;
        }
        else if (els->hem == HEM_RIGHT) {
            pos.push_back(CPoint(0, 0));
            cx += els->x;
            if (cx > cw)
                cw = cx;
            els->bmp = NULL;
        }
        else if (els->hem == HEM_COORD) {
            new_coord_f = true;
            new_coord.x = els->x;
            new_coord.y = els->y;
            pos.push_back(CPoint(0, 0));
            els->bmp = NULL;
            // do nothing
        }
        else {
            ERROR_E;
        }
    }

    struct SBordPart {
        CPoint pos;  // in texture
        CPoint size;
        CPoint size_down;
        DWORD present;
        const wchar* name;

    } parts[9];

    memset(parts, 0, sizeof(parts));
    parts[0].name = PAR_SOURCE_HINTS_LT;
    parts[1].name = PAR_SOURCE_HINTS_T;
    parts[2].name = PAR_SOURCE_HINTS_RT;
    parts[3].name = PAR_SOURCE_HINTS_L;
    parts[4].name = PAR_SOURCE_HINTS_C;
    parts[5].name = PAR_SOURCE_HINTS_R;
    parts[6].name = PAR_SOURCE_HINTS_LB;
    parts[7].name = PAR_SOURCE_HINTS_B;
    parts[8].name = PAR_SOURCE_HINTS_RB;

    for (int i = 0; i < 9; ++i) {
        parts[i].present = bph && bph->ParCount(parts[i].name) > 0;
        if (parts[i].present) {
            parts[i].pos.x = bph->ParGet(parts[i].name).GetStrPar(0, L",").GetInt();
            parts[i].pos.y = bph->ParGet(parts[i].name).GetStrPar(1, L",").GetInt();
            parts[i].size.x = bph->ParGet(parts[i].name).GetStrPar(2, L",").GetInt();
            parts[i].size.y = bph->ParGet(parts[i].name).GetStrPar(3, L",").GetInt();
        }
        else {
            parts[i].pos.x = 0;
            parts[i].pos.y = 0;
            parts[i].size.x = 0;
            parts[i].pos.y = 0;
        }

        parts[i].size_down = parts[i].size;
    }

    if (bph) {
        int top_down = bph->ParGetNE(L"TopDown").GetInt();
        int bottom_down = bph->ParGetNE(L"BottomDown").GetInt();
        int left_down = bph->ParGetNE(L"LeftDown").GetInt();
        int right_down = bph->ParGetNE(L"RightDown").GetInt();

        parts[0].size_down.x -= left_down;  // PAR_SOURCE_HINTS_LT;
        parts[0].size_down.y -= top_down;   // PAR_SOURCE_HINTS_LT;

        parts[1].size_down.y -= top_down;  // PAR_SOURCE_HINTS_T;

        parts[2].size_down.x -= right_down;  // PAR_SOURCE_HINTS_RT;
        parts[2].size_down.y -= top_down;    // PAR_SOURCE_HINTS_RT;

        parts[3].size_down.x -= left_down;  // PAR_SOURCE_HINTS_L;

        parts[5].size_down.x -= right_down;  // PAR_SOURCE_HINTS_R;

        parts[6].size_down.x -= left_down;    // PAR_SOURCE_HINTS_LB;
        parts[6].size_down.y -= bottom_down;  // PAR_SOURCE_HINTS_LB;

        parts[7].size_down.y -= bottom_down;  // PAR_SOURCE_HINTS_B;

        parts[8].size_down.x -= right_down;   // PAR_SOURCE_HINTS_RB;
        parts[8].size_down.y -= bottom_down;  // PAR_SOURCE_HINTS_RB;
    }

    int h_delta = 1;

    CRect ots(0, 0, 0, 0);
    if (otstup)
        ots = *otstup;
    else {
        if (parts[0].present) {
            ots.left = parts[0].size_down.x;
            ots.top = parts[0].size_down.y;
        }
        if (parts[2].present) {
            ots.right = parts[2].size_down.x;
            ots.top = parts[2].size_down.y;
        }
        if (parts[6].present) {
            ots.left = parts[6].size_down.x;
            ots.bottom = parts[6].size_down.y;
        }
        if (parts[8].present) {
            ots.right = parts[8].size_down.x;
            ots.bottom = parts[8].size_down.y;
        }

        if (parts[1].present) {
            ots.top = parts[1].size_down.y;
        }
        if (parts[3].present) {
            ots.left = parts[3].size_down.x;
        }
        if (parts[5].present) {
            ots.right = parts[5].size_down.x;
        }
        if (parts[7].present) {
            ots.bottom = parts[7].size_down.y;
        }
    }
    if (parts[4].present) {
        h_delta = parts[4].size.y;
    }

    int idx = 0;
    for (auto& item : pos)
    {
        if (item.x == -1000) {
            item.x = (cw - elems[idx].bmp->SizeX()) / 2;
        }
        else if (item.x == -1001) {
            int tv = elems[idx].hem - HEM_TAB_LARGEST;
            item.x = cw / 2 + tv;
        }
        else if (item.x == -1002) {
            int tv = elems[idx].hem - HEM_CENTER_RIGHT_LARGEST;
            item.x = cw / 2 - tv - elems[idx].bmp->SizeX();
        }

        item.x += ots.left;
        item.y += ots.top;

        ++idx;
    }

    if (ch < (parts[1].size.y + parts[7].size.y - ots.top - ots.bottom)) {
        ch = (parts[1].size.y + parts[7].size.y - ots.top - ots.bottom);
    }

    int clw = cw;
    int clh = ch;

    cw += ots.left + ots.right;
    ch += ots.top + ots.bottom;

    int fw = cw;
    int fh = ch;

    int delta = fh - parts[7].size.y - parts[1].size.y;
    int delta2 = ((delta + (h_delta - 1)) / h_delta) * h_delta;
    int delta3 = delta2 - delta;
    fh += delta3;
    ch += delta3;
    clh += delta3;

    cw = DetermineGreaterPowerOfTwo(cw);
    ch = DetermineGreaterPowerOfTwo(ch);

    CBitmap bmpf(g_CacheHeap);
    bmpf.CreateRGBA(cw, ch);
    bmpf.Fill(CPoint(0, 0), CPoint(cw, ch), 0);

    // filling

    // lt
    if (parts[0].present) {
        bmpf.Copy(CPoint(0, 0), parts[0].size, bmps, parts[0].pos);
    }

    // rt
    if (parts[2].present) {
        bmpf.Copy(CPoint(fw - parts[2].size.x, 0), parts[2].size, bmps, parts[2].pos);
    }

    // lb
    if (parts[6].present) {
        bmpf.Copy(CPoint(0, fh - parts[6].size.y), parts[6].size, bmps, parts[6].pos);
    }

    // rb
    if (parts[8].present) {
        bmpf.Copy(CPoint(fw - parts[8].size.x, fh - parts[8].size.y), parts[8].size, bmps, parts[8].pos);
    }

    // top
    if (parts[1].present) {
        bmpf.Tile(CPoint(parts[0].size.x, 0), CPoint(fw - parts[0].size.x - parts[2].size.x, parts[1].size.y), bmps,
                  parts[1].pos, parts[1].size);
    }

    // bottom
    if (parts[7].present) {
        bmpf.Tile(CPoint(parts[0].size.x, fh - parts[7].size.y),
                  CPoint(fw - parts[0].size.x - parts[2].size.x, parts[7].size.y), bmps, parts[7].pos, parts[7].size);
    }

    delta = fh - parts[7].size.y - parts[1].size.y;

    if (delta) {
        // left
        if (parts[3].present) {
            bmpf.Tile(CPoint(0, parts[0].size.y), CPoint(parts[3].size.x, delta), bmps, parts[3].pos, parts[3].size);
        }
        // rite
        if (parts[5].present) {
            bmpf.Tile(CPoint(fw - parts[5].size.x, parts[2].size.y), CPoint(parts[5].size.x, delta), bmps, parts[5].pos,
                      parts[5].size);
        }
        // center
        if (parts[4].present) {
            bmpf.Tile(CPoint(parts[3].size.x, parts[1].size.y), CPoint(fw - parts[3].size.x - parts[5].size.x, delta),
                      bmps, parts[4].pos, parts[4].size);
        }
    }

    CPoint *copypos = NULL;
    int copyposcnt = 0;

    idx = 0;
    bool copy = false;
    for (const auto& item : pos)
    {
        bool new_copy = elems[idx].hem == HEM_COPY;

        if (elems[idx].bmp) {
            if (copy) {
                bmpf.Copy(item, elems[idx].bmp->Size(), *elems[idx].bmp, CPoint(0, 0));

                ++copyposcnt;
                copypos = (CPoint *)HAllocEx(copypos, sizeof(CPoint) * copyposcnt, g_MatrixHeap);
                copypos[copyposcnt - 1] = item;
            }
            else {
                if (elems[idx].bmp->BytePP() == 3) {
                    bmpf.Copy(item, elems[idx].bmp->Size(), *elems[idx].bmp, CPoint(0, 0));
                }
                else {
                    bmpf.MergeWithAlpha(item, elems[idx].bmp->Size(), *elems[idx].bmp, CPoint(0, 0));
                }
            }
        }

        copy = new_copy;
        ++idx;
    }

    // bmpf.SaveInPNG(L"bla.png");

    CTextureManaged *tex = CACHE_CREATE_TEXTUREMANAGED();
    tex->MipmapOff();

    D3DLOCKED_RECT lr;
    tex->CreateLock(D3DFMT_A8R8G8B8, bmpf.SizeX(), bmpf.SizeY(), 1, lr);
    CBitmap bmdes(g_CacheHeap);
    bmdes.CreateRGBA(bmpf.SizeX(), bmpf.SizeY(), lr.Pitch, lr.pBits);
    bmdes.Copy(CPoint(0, 0), bmpf.Size(), bmpf, CPoint(0, 0));
    tex->UnlockRect();
    // tex->LoadFromBitmap(bmpf, D3DFMT_A8R8G8B8, 1);

    CMatrixHint *hint = HNew(g_MatrixHeap)
            CMatrixHint(tex, clw + ots.left + ots.right, clh + ots.top + ots.bottom, soundin, soundout);

    hint->m_CopyPos = copypos;
    hint->m_CopyPosCnt = copyposcnt;

    return hint;
}

void CMatrixHint::PreloadBitmaps(void) {
    CBlockPar *bph = g_MatrixData->BlockGet(PAR_SOURCE_HINTS)->BlockGet(PAR_SOURCE_HINTS_BITMAPS);
    m_BitmapsCnt = bph->ParCount();
    m_Bitmaps = (SHintBitmap *)HAlloc(sizeof(SHintBitmap) * m_BitmapsCnt, g_MatrixHeap);
    std::wstring src;
    for (int i = 0; i < m_BitmapsCnt; ++i) {
        if (!CFile::FileExist(src, bph->ParGet(i).c_str(), L"png")) {
            ERROR_S2(L"Hint bitmap not found:", m_Bitmaps[i].name->c_str());
        }
        m_Bitmaps[i].name = new std::wstring(bph->ParGetName(i));
        m_Bitmaps[i].bmp = HNew(g_MatrixHeap) CBitmap(g_MatrixHeap);
        m_Bitmaps[i].bmp->LoadFromPNG(src.c_str());
        m_Bitmaps[i].bmp->SwapByte(CPoint(0, 0), m_Bitmaps[i].bmp->Size(), 0,
                                   2);  // store format should be [A]RGB, not [A]BGR
    }
}

static EHintElementModificator Convert(std::wstring &bmph, const std::wstring &temp, int index) {
    bmph = ParamParser{temp}.GetStrPar(index, L":");
    if (bmph == L"C")
        return HEM_CENTER;
    else if (bmph == L"L")
        return HEM_LAST_ON_LINE;
    else if (bmph == L"CR") {
        return (EHintElementModificator)(HEM_TAB_LARGEST + ParamParser{temp}.GetStrPar(index + 1, L":").GetInt());
    }
    else if (bmph == L"CL") {
        return (EHintElementModificator)(HEM_CENTER_RIGHT_LARGEST + ParamParser{temp}.GetStrPar(index + 1, L":").GetInt());
    }
    else if (bmph == L"T") {
        return (EHintElementModificator)(ParamParser{temp}.GetStrPar(index + 1, L":").GetInt());
    }
    else if (bmph == L"COPY") {
        return HEM_COPY;
    }
    else {
        return HEM_BITMAP;
    }
}

static void Replace(std::wstring &text, const wchar *baserepl, CBlockPar *repl) {
    std::wstring text2;
    size_t ii = 0;
    for (;;) {
        size_t i1 = text.find(L"[", ii, 1);
        if (i1 != std::wstring::npos && i1 >= ii)
        {
            size_t i2 = text.find(L"]", i1 + 1, 1);
            if (i2 == std::wstring::npos)
                ERROR_S(L"] not found");
            text2 = std::wstring{text.c_str() + i1 + 1, static_cast<size_t>(i2 - i1 - 1)};
            if (text2.empty()) {
                utils::replace(text, L"[]", baserepl);
            }
            else {
                // int cnt = repl->ParCount(text2);
                // repl->Par
                // std::wstring text3;
                // for (int k=0;k<cnt;++k)
                utils::replace(text, L"[" + text2 + L"]", repl->ParGetNE(text2));
            }
            ii = i1;
        }
        else
            break;
    }
}

//#define BGR2RGB(c) (((c & 255) << 16) | (c & 0x0000FF00) | ((c >> 16) & 255) | c&0xFF000000)

CMatrixHint *CMatrixHint::Build(const std::wstring &templatename, const wchar *baserepl) {
    DTRACE();

    CBlockPar *repl = g_MatrixData->BlockGet(PAR_REPLACE);
    CBlockPar *bp = g_MatrixData->BlockGet(PAR_TEMPLATES);
    std::wstring str;

    DCP();

    int cnt = bp->ParCount();
    int ii = -1;
    DCP();

    for (int i = 0; i < cnt; ++i) {
        DCP();
        if (bp->ParGetName(i) == templatename) {
            DCP();
            if (ii < 0)
                ii = i;
            str = bp->ParGet(i);
            if (str[0] == '|')
                continue;
            DCP();

            std::wstring templ;

            for (; ii < cnt; ++ii) {
                DCP();

                if (bp->ParGetName(ii) == templatename) {
                    DCP();

                    templ = bp->ParGet(ii);
                    if (templ[0] == '|')
                        str += templ;
                }
            }
            break;
        }
    }

    return Build(str, repl, baserepl);
}

CMatrixHint *CMatrixHint::Build(const std::wstring &str, CBlockPar *repl, const wchar *baserepl) {
    std::wstring soundin;
    std::wstring soundout;

    SHintElement elems[256];

    CBuf its;
    CBuf bmps;
    int ssz = 0;

    bool otstup = false;
    CRect otstup_r;

    int border = ParamParser{str}.GetStrPar(0, L"|").GetInt();
    int cnt = ParamParser{str}.GetCountPar(L"|");
    int idx = 1;
    std::wstring bmpn;
    std::wstring font(L"Font.2Normal");
    DWORD color = 0xFFFFFFFF;
    EHintElementModificator modif = HEM_BITMAP;
    int nelem = 0;
    int h = 0;
    int w = 0;
    int alignx = 1;
    int aligny = 1;

    bool skip = false;

    for (; idx < cnt; ++idx) {
        if (nelem >= 255)
            break;
        auto temp = ParamParser{str}.GetStrPar(idx, L"|");

        if (utils::starts_with(temp, L"_ENDIF")) {
            skip = false;
            continue;
        }
        else if (utils::starts_with(temp, L"_IF:")) {
            std::wstring text(temp.c_str() + 4);
            if (repl)
                Replace(text, baserepl, repl);
            skip = text.empty();
            continue;
        }

        if (skip)
            continue;

        if (utils::starts_with(temp, L"_FONT:")) {
            font = (temp.c_str() + 6);
        }
        else if (utils::starts_with(temp, L"_COLOR:")) {
            // DWORD c = temp.GetStrPar(1,L":").GetHexUnsigned();
            // color = BGR2RGB(c);
            color = temp.GetStrPar(1, L":").GetHexUnsigned();
        }
        else if (utils::starts_with(temp, L"_SOUNDIN:")) {
            soundin = (temp.c_str() + 9);
        }
        else if (utils::starts_with(temp, L"_SOUNDOUT:")) {
            soundout = (temp.c_str() + 10);
        }
        else if (utils::starts_with(temp, L"_BORDER:")) {
            otstup = true;
            otstup_r.left = temp.GetStrPar(1, L":").GetInt();
            otstup_r.top = temp.GetStrPar(2, L":").GetInt();
            otstup_r.right = temp.GetStrPar(3, L":").GetInt();
            otstup_r.bottom = temp.GetStrPar(4, L":").GetInt();
        }
        else if (utils::starts_with(temp, L"_POS:")) {
            elems[nelem].x = temp.GetStrPar(1, L":").GetInt();
            elems[nelem].y = temp.GetStrPar(2, L":").GetInt();
            elems[nelem].hem = HEM_COORD;
            ++nelem;
        }
        else if (utils::starts_with(temp, L"_DOWN:")) {
            elems[nelem].y = temp.GetStrPar(1, L":").GetInt();
            elems[nelem].hem = HEM_DOWN;
            ++nelem;
        }
        else if (utils::starts_with(temp, L"_RIGHT:")) {
            elems[nelem].x = temp.GetStrPar(1, L":").GetInt();
            elems[nelem].hem = HEM_RIGHT;
            ++nelem;
        }
        else if (utils::starts_with(temp, L"_ALIGN:")) {
            alignx = temp.GetStrPar(1, L":").GetInt();
            aligny = temp.GetStrPar(2, L":").GetInt();
        }
        else if (utils::starts_with(temp, L"_WIDTH:")) {
            w = temp.GetInt();
        }
        else if (utils::starts_with(temp, L"_HEIGHT:")) {
            h = temp.GetInt();
        }
        else if (utils::starts_with(temp, L"_MOD:")) {
            elems[nelem].bmp = NULL;
            elems[nelem].hem = Convert(bmpn, temp, 1);
            ++nelem;
        }
        else if (utils::starts_with(temp, L"_TEXTP:")) {
            modif = Convert(bmpn, temp, 1);
        }
        else if (utils::starts_with(temp, L"_BITMAP:")) {
            bmpn = temp.GetStrPar(1, L":");

            if (repl)
                Replace(bmpn, baserepl, repl);

            elems[nelem].bmp = NULL;
            if (!bmpn.empty()) {
                for (int i = 0; i < m_BitmapsCnt; ++i) {
                    if (*m_Bitmaps[i].name == bmpn) {
                        elems[nelem].bmp = m_Bitmaps[i].bmp;
                        break;
                    }
                }
            }

            elems[nelem].hem = Convert(bmpn, temp, 2);
            ++nelem;
        }
        else if (utils::starts_with(temp, L"_TEXT:")) {
            if (g_RangersInterface) {
                CRect cr(0, 0, w, h);
                // if (w == 0) w = g_ScreenX;
                // if (h == 0) h = 200;

                std::wstring text(temp.c_str() + 6);

                if (repl)
                    Replace(text, baserepl, repl);

                //(wchar * text, wchar * font, DWORD color, int sizex, int sizey, int alignx, int aligny, int wordwrap,
                //int smex, int smy, CRect * clipr, SMGDRangersInterfaceText * it);

                utils::replace(text, L"<br>", L"\r\n");

                SMGDRangersInterfaceText *it =
                        (SMGDRangersInterfaceText *)HAlloc(sizeof(SMGDRangersInterfaceText), g_CacheHeap);
                g_RangersInterface->m_RangersText((wchar*)text.c_str(), (wchar*)font.c_str(), color, w, h, alignx, aligny,
                                                  (w == 0) ? 0 : 1, 0, 0, &cr, it);

                CBitmap *bmsrc = HNew(g_CacheHeap) CBitmap(g_CacheHeap);
                bmsrc->CreateRGBA(it->m_SizeX, it->m_SizeY, it->m_Pitch, it->m_Buf);

                its.Add<uint32_t>((DWORD)it);
                bmps.Add<uint32_t>((DWORD)bmsrc);
                ssz++;

                elems[nelem].bmp = bmsrc;
                elems[nelem].hem = modif;
                ++nelem;
            }
        }
    }

    elems[nelem].bmp = NULL;
    elems[nelem].hem = HEM_LAST;

    CMatrixHint *hint = Build(border, soundin, soundout, elems, otstup ? (&otstup_r) : NULL);

    for (int i = 0; i < ssz; ++i) {
        g_RangersInterface->m_RangersTextClear((SMGDRangersInterfaceText *)its.Buff<DWORD>()[i]);
        HFree((SMGDRangersInterfaceText *)its.Buff<DWORD>()[i], g_CacheHeap);
        HDelete(CBitmap, (CBitmap *)bmps.Buff<DWORD>()[i], g_CacheHeap);
    }

    return hint;
}

static void bdh(void) {
    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHAREF, 0));

    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));

    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
    ASSERT_DX(g_D3DD->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));

    // SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetColorOpSelect(0, D3DTA_TEXTURE);
    SetAlphaOpSelect(0, D3DTA_TEXTURE);
    SetColorOpDisable(1);

    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

    ASSERT_DX(g_D3DD->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
}

static void adh(void) {
    g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHAREF, 0x08));

    g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    g_D3DD->SetRenderState(D3DRS_ZENABLE, TRUE);
}

void CMatrixHint::DrawNow(void) {
    bdh();

    SVert_V4_UV v[4];
    v[0].tu = 0;
    v[0].tv = 1.0f;
    v[1].tu = 0;
    v[1].tv = 0.0f;
    v[2].tu = 1.0f;
    v[2].tv = 1.0f;
    v[3].tu = 1.0f;
    v[3].tv = 0.0f;

    CInstDraw::BeginDraw(IDFVF_V4_UV);

    int szx = m_Texture->GetSizeX();
    int szy = m_Texture->GetSizeY();

    v[0].p = D3DXVECTOR4(float(m_PosX) - 0.5f, float(m_PosY + szy) - 0.5f, HINT_Z, 1.0f);
    v[1].p = D3DXVECTOR4(float(m_PosX) - 0.5f, float(m_PosY) - 0.5f, HINT_Z, 1.0f);
    v[2].p = D3DXVECTOR4(float(m_PosX + szx) - 0.5f, float(m_PosY + szy) - 0.5f, HINT_Z, 1.0f);
    v[3].p = D3DXVECTOR4(float(m_PosX + szx) - 0.5f, float(m_PosY) - 0.5f, HINT_Z, 1.0f);

    CInstDraw::AddVerts(v, m_Texture);
    CInstDraw::ActualDraw();

    adh();
}

void CMatrixHint::DrawAll(void) {
    bdh();

    SVert_V4_UV v[4];
    v[0].tu = 0;
    v[0].tv = 1.0f;
    v[1].tu = 0;
    v[1].tv = 0.0f;
    v[2].tu = 1.0f;
    v[2].tv = 1.0f;
    v[3].tu = 1.0f;
    v[3].tv = 0.0f;

    CInstDraw::BeginDraw(IDFVF_V4_UV);

    CMatrixHint *h = m_First;
    for (; h; h = h->m_Next) {
        if (!h->IsVisible())
            continue;

        int szx = h->m_Texture->GetSizeX();
        int szy = h->m_Texture->GetSizeY();

        v[0].p = D3DXVECTOR4(float(h->m_PosX) - 0.5f, float(h->m_PosY + szy) - 0.5f, HINT_Z, 1.0f);
        v[1].p = D3DXVECTOR4(float(h->m_PosX) - 0.5f, float(h->m_PosY) - 0.5f, HINT_Z, 1.0f);
        v[2].p = D3DXVECTOR4(float(h->m_PosX + szx) - 0.5f, float(h->m_PosY + szy) - 0.5f, HINT_Z, 1.0f);
        v[3].p = D3DXVECTOR4(float(h->m_PosX + szx) - 0.5f, float(h->m_PosY) - 0.5f, HINT_Z, 1.0f);

        CInstDraw::AddVerts(v, h->m_Texture);
    }

    CInstDraw::ActualDraw();

    adh();
}

void CMatrixHint::ClearAll(void) {
    if (m_Bitmaps) {
        for (int i = 0; i < m_BitmapsCnt; ++i) {
            HDelete(CBitmap, m_Bitmaps[i].bmp, g_MatrixHeap);
            delete(m_Bitmaps[i].name);
        }
        HFree(m_Bitmaps, g_MatrixHeap);
        m_Bitmaps = NULL;
        m_BitmapsCnt = 0;
    }
}
