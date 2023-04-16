// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "CIFaceStatic.h"
#include "CIFaceMenu.h"
#include "CInterface.h"
#include "MatrixHint.hpp"

CIFaceStatic::CIFaceStatic() {
    m_Type = IFACE_STATIC;
    m_xPos = m_yPos = m_zPos = m_xSize = m_ySize = 0;
    m_NextElement = m_PrevElement = NULL;
    m_CurState = IFACE_NORMAL;
}

CIFaceStatic::~CIFaceStatic() {}

bool CIFaceStatic::OnMouseMove(CPoint mouse) {
    if (ElementCatch(mouse)) {
        if (g_IFaceList->m_CurrentHint == NULL && m_Hint.HintTemplate != L"") {
            if (g_IFaceList->CheckShowHintLogic(m_strName)) {
                g_IFaceList->AddHintReplacements(m_strName);
                CMatrixHint *hint = CMatrixHint::Build(m_Hint.HintTemplate, m_strName.c_str());
                int x = Float2Int(m_PosElInX) + m_Hint.x;
                int y = Float2Int(m_PosElInY) + m_Hint.y;
                g_IFaceList->CorrectCoordinates(g_ScreenX, g_ScreenY, x, y, hint->m_Width, hint->m_Height, m_strName);
                hint->Show(x, y);
                g_IFaceList->m_CurrentHint = hint;
                g_IFaceList->m_CurrentHintControlName = m_strName;
            }
        }
        if (ElementAlpha(mouse))
            return TRUE;
    }
    else {
        if (g_IFaceList->m_CurrentHint && g_IFaceList->m_CurrentHintControlName == m_strName) {
            g_IFaceList->m_CurrentHint->Release();
            g_IFaceList->m_CurrentHint = NULL;
            g_IFaceList->m_CurrentHintControlName = L"";
        }
        return FALSE;
    }
    return false;
}

bool CIFaceStatic::OnMouseLBDown() {
    Action(ON_PRESS);
    return FALSE;
}

void CIFaceStatic::OnMouseLBUp() {
    Action(ON_UN_PRESS);
}

bool CIFaceStatic::OnMouseRBDown() {
    //    CPoint mouse = g_MatrixMap->m_Cursor.GetPos();
    //    if(ElementCatch(mouse) && ElementAlpha(mouse)){
    //        return true;
    //    }else{
    //        return false;
    //    }
    return false;
}
