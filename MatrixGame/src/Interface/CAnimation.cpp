// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>

#include "CAnimation.h"
#include "CIFaceStatic.h"
#include "CIFaceElement.h"
#include "Interface.h"

CAnimation::CAnimation(int frames, int period) {
    DTRACE();
    m_FramesBuffer = (CIFaceStatic *)HAlloc(sizeof(CIFaceStatic) * frames, g_MatrixHeap);
    for (int i = 0; i < frames; i++) {
        new(&m_FramesBuffer[i]) CIFaceStatic();
    }
    m_Frames = frames;
    m_Period = period;
    m_FramesLoaded = 0;
    m_CurrentFrame = 0;
    m_TimePass = 0;
}

CAnimation::~CAnimation() {
    DTRACE();
    if (m_FramesBuffer) {
        for (int i = 0; i < m_Frames; i++) {
            m_FramesBuffer[i].CIFaceStatic::~CIFaceStatic();
        }
        HFree(m_FramesBuffer, g_MatrixHeap);
    }
}

void CAnimation::LogicTakt(int ms) {
    DTRACE();
    m_TimePass += ms;
    if (m_TimePass >= m_Period) {
        m_TimePass = 0;

        m_CurrentFrame++;
        if (m_CurrentFrame > m_Frames - 1) {
            m_CurrentFrame = 0;
        }
    }
}

bool CAnimation::LoadNextFrame(SFrame *frame) {
    DTRACE();
    if (m_FramesLoaded == m_Frames) {
        return true;
    }

    CIFaceStatic *st = m_FramesBuffer + m_FramesLoaded;
    // st->m_strName = frame->name;
    st->m_xPos = frame->pos_x;
    st->m_yPos = frame->pos_y;
    st->m_zPos = frame->pos_z;
    st->m_xSize = frame->width;
    st->m_ySize = frame->height;
    st->m_DefState = IFACE_NORMAL;

    st->SetStateImage(IFACE_NORMAL, frame->tex, frame->tex_pos_x, frame->tex_pos_y, frame->tex_width,
                      frame->tex_height);

    st->ElementGeomInit((void *)st);

    D3DXVECTOR3 dp(st->m_xPos + frame->ipos_x, st->m_yPos + frame->ipos_y, st->m_zPos);

    int nC = 0;
    while (st->m_StateImages[nC].Set && nC < MAX_STATES) {
        for (int i = 0; i < 4; i++) {
            st->m_StateImages[nC].m_Geom[i].p.x += dp.x;
            st->m_StateImages[nC].m_Geom[i].p.y += dp.y;
            st->m_StateImages[nC].m_Geom[i].p.z += dp.z;
            st->m_PosElInX = dp.x;
            st->m_PosElInY = dp.y;
        }
        nC++;
    }

    m_FramesLoaded++;
    return false;
}

CIFaceElement *CAnimation::GetCurrentFrame() {
    return (CIFaceElement *)m_FramesBuffer + m_CurrentFrame;
}

void CAnimation::RecalcPos(const float &ix, const float &iy) {
    DTRACE();

    for (int i = 0; i < m_FramesLoaded; i++) {
        CIFaceElement *pElement = (CIFaceElement *)(m_FramesBuffer + i);
        int nC;
        for (nC = 0; nC < MAX_STATES; nC++) {
            if (pElement->m_StateImages[nC].Set) {
                pElement->m_StateImages[nC].m_Geom[0].p = D3DXVECTOR4(0, pElement->m_ySize, 0, 1);
                pElement->m_StateImages[nC].m_Geom[1].p = D3DXVECTOR4(0, 0, 0, 1);
                pElement->m_StateImages[nC].m_Geom[2].p = D3DXVECTOR4(pElement->m_xSize, pElement->m_ySize, 0, 1);
                pElement->m_StateImages[nC].m_Geom[3].p = D3DXVECTOR4(pElement->m_xSize, 0, 0, 1);
            }
        }

        D3DXVECTOR3 dp(pElement->m_xPos + ix, pElement->m_yPos + iy, pElement->m_zPos);

        nC = 0;
        while (pElement->m_StateImages[nC].Set && nC < MAX_STATES) {
            for (int i = 0; i < 4; i++) {
                pElement->m_StateImages[nC].m_Geom[i].p.x += dp.x;
                pElement->m_StateImages[nC].m_Geom[i].p.y += dp.y;
                pElement->m_StateImages[nC].m_Geom[i].p.z += dp.z;
                pElement->m_PosElInX = dp.x;
                pElement->m_PosElInY = dp.y;
            }
            nC++;
        }
    }
}
