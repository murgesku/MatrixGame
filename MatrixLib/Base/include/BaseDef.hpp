// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include <windows.h>

#ifdef BASE_DLL
	#ifdef BASE_EXPORTS
		#define BASE_API __declspec(dllexport)
	#else
		#define BASE_API __declspec(dllimport)
	#endif
#else
	#define BASE_API
#endif

namespace Base {

//lint -e1401

class CPoint : public tagPOINT
{
public:
        CPoint() {}
        //CPoint(int zn) {x = zn; y = zn;}
        CPoint(int ax, int ay) {x = ax; y =ay;}
        CPoint(const CPoint &p) {x=p.x; y=p.y;}

		bool    operator == (const CPoint & zn) const { return (x==zn.x) && (y==zn.y); }
        CPoint & operator += (const CPoint &zn) { x+=zn.x; y+=zn.y; return *this; }
		CPoint & operator -= (const CPoint &zn) { x-=zn.x; y-=zn.y; return *this; }

        int Dist2(const CPoint & p) const       { return (p.x-x)*(p.x-x)+(p.y-y)*(p.y-y); }
};

class CRect:public tagRECT
{
	public:
		CRect() {}
		//CRect(int zn) { left=top=right=bottom=zn; }
		CRect(int _left,int _top,int _right,int _bottom) { left=_left; top=_top; right=_right; bottom=_bottom; }

        bool IsEmpty(void) const {return (right<=left) || (bottom<=top); }
        bool IsInRect(const CPoint &pos) const
        {
            return (left < pos.x && top < pos.y && right > pos.x && bottom > pos.y);
        }

        void Normalize(void)
        {
            if (left > right)
            {
                left ^= right;
                right ^= left;
                left ^= right;
            }
            if (top > bottom)
            {
                top ^= bottom;
                bottom ^= top;
                top ^= bottom;
            }
        }
};

//lint +e1401
}

typedef wchar_t wchar;
//#ifndef byte
//typedef unsigned char byte;
//#endif
typedef unsigned short word;
typedef unsigned long dword;
typedef __int64 int64;
typedef unsigned int  uint;

#define LIST_ADD(el,first,last,prev,next){if(last!=NULL) {last->next=el;} el->prev=last; el->next=NULL;	last=el; if(first==NULL) {first=el;}}
#define LIST_ADD_FIRST(el,first,last,prev,next) {if(first!=NULL) {first->prev=el;} el->next=first; el->prev=NULL; first=el; if(last==NULL) {last=el;}}
#define LIST_INSERT(perel,el,first,last,prev,next) {if(perel==NULL) { LIST_ADD(el,first,last,prev,next); }  else { el->prev=perel->prev;	el->next=perel;	if(perel->prev!=NULL) {perel->prev->next=el;} perel->prev=el; if(perel==first) { first=el; }}}

#define LIST_DEL(el,first,last,prev,next) \
    {if(el->prev!=NULL) el->prev->next=el->next;\
	if(el->next!=NULL) el->next->prev=el->prev;\
	if(last==el) last=el->prev;\
    if(first==el) first=el->next;}

#define LIST_DEL_CLEAR(el,first,last,prev,next) \
    {if(el->prev!=NULL) el->prev->next=el->next;\
	if(el->next!=NULL) el->next->prev=el->prev;\
	if(last==el) last=el->prev;\
	if(first==el) first=el->next;\
	el->prev=NULL;\
    el->next=NULL;}


#define SETBIT(x) (((DWORD)1)<<x)
#define SETFLAG(f,mask) f|=(mask)
#define RESETFLAG(f,mask) f&=~(mask)
#define INVERTFLAG(f,mask) f^=(mask)
#define FLAG(f,mask) ((f&(mask))!=0)
#define INITFLAG(f,mask,val) if(val) {SETFLAG(f,mask);} else {RESETFLAG(f,mask);}
