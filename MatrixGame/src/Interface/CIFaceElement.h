// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once
#include "Interface.h"
#include "../MatrixMap.hpp"
#include "../MatrixObject.hpp"
#include "../MatrixRobot.hpp"

class CAnimation;


//////////////////////////////////////////////////
//Interface Element Class

#define IFEF_VISIBLE    SETBIT(0)
#define IFEF_CLEARRECT  SETBIT(1)

struct SElementHint
{
    CWStr   HintTemplate;
    int     timer;
    int     x;
    int     y;

    SElementHint():HintTemplate(g_MatrixHeap)
    {
        timer = 0;
        x = 0;
        y = 0;
    }
};

class CIFaceElement : public CMain{
	DWORD               m_Flags;
    

public:

    CRect               m_ClearRect;    // rect for clearing Z buffer
    SElementHint        m_Hint;
    CAnimation*         m_Animation;
    byte                m_VisibleAlpha;    
    SStateImages        m_StateImages[MAX_STATES];
	CWStr               m_strName;
	float               m_Param1;
	float               m_Param2;
    int                 m_iParam;
	int                 m_nId;
	int                 m_nGroup;

	IFaceElementType    m_Type;
	float               m_xPos, m_yPos, m_zPos, m_xSize, m_ySize, m_PosElInX, m_PosElInY;
	CIFaceElement*      m_NextElement, *m_PrevElement;
	
    SAction             m_Actions[MAX_ACTIONS];
	
    IFaceElementState   m_CurState;
	IFaceElementState   m_DefState;
	
    void Action(EActions action);                                                                   
	//CSound m_StateSounds;//MAX_STATES
	//bool SetStateSound(IFaceElementState State, CSound Sound);

    IFaceElementState GetState();
	bool SetState(IFaceElementState State);
    bool SetStateImage(IFaceElementState State, CTextureManaged *pImage, float x, float y, float width, float height);
	LPDIRECT3DTEXTURE9 GetStateImage(IFaceElementState State);
	
	bool GetVisibility(void) const                                                  { return FLAG(m_Flags,IFEF_VISIBLE); }
    void SetVisibility(bool visible);                                                

    bool HasClearRect(void) const           { return FLAG(m_Flags,IFEF_CLEARRECT); }
    void SetClearRect(void)                 { SETFLAG(m_Flags,IFEF_CLEARRECT); }

        

	bool ElementCatch(CPoint);
    bool ElementAlpha(CPoint mouse);

    virtual void CheckGroupReset(CIFaceElement*, CIFaceElement*);
	virtual void ElementGeomInit(void *pObj, bool full_size = false);
	virtual bool OnMouseMove(CPoint)                                                { return false; }
	virtual void OnMouseLBUp()                                                      { }
    virtual bool OnMouseLBDown()                                                    { return false; }
    virtual void OnMouseRBUp()                                                      { }
    virtual bool OnMouseRBDown()                                                    { return false; }
    virtual void BeforeRender(void);
	virtual void Render(BYTE m_VisibleAlpha);
	virtual void Reset();
    void LogicTakt(int ms);

    void RecalcPos(const float &x, const float &y, bool ichanged = true);


    //void    GenerateClearRect(void);

	CIFaceElement();
	~CIFaceElement();
};

