// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "Interface.h"
#include "CIFaceElement.h"

////////////////////////////////////////////////////////
// Image container class

class CIFaceImage : public CMain {
public:
    CTextureManaged *m_Image;
    std::wstring m_strName;
    float m_xTexPos;
    float m_yTexPos;
    float m_TexWidth;
    float m_TexHeight;
    float m_Width;
    float m_Height;
    IFaceElementType m_Type;
    CIFaceImage *m_PrevImage, *m_NextImage;

    CIFaceImage();
    ~CIFaceImage();
};