// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "CWindow.hpp"

namespace Window {

using namespace Base;

HINSTANCE g_Instance=0;
CWindow * g_WindowClassInit=NULL;

WINDOW_API void Window_Init(HINSTANCE hInstance)
{
	g_Instance=hInstance;
}

LRESULT CALLBACK Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CWindow * cwc=(CWindow *)GetClassLong(hWnd,0);
	if(!cwc) {
		cwc=g_WindowClassInit;
		g_WindowClassInit=NULL;
		if(cwc) {
			cwc->m_Wnd=hWnd;
			SetClassLong(hWnd,0,DWORD(cwc));
		}
	}
	if(cwc) return cwc->WndProc(message,wParam,lParam);
	else return DefWindowProc(hWnd, message, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CWindow::CWindow(Base::CHeap * heap) : CMain(),m_CreateParams(heap),m_Text(heap)
{
	m_Wnd=0;
	m_WndParent=0;

	m_Heap=heap;
	m_Pos.x=0;
	m_Pos.y=0;
	m_Size.x=100;
	m_Size.y=50;

	m_Parent=NULL;
	m_First=NULL;
	m_Last=NULL;
	m_Next=NULL;
	m_Prev=NULL;

	m_Align=wa_None;

	m_SizeMin.x=0; m_SizeMin.y=0;
	m_SizeMax.x=0; m_SizeMax.y=0;
}

CWindow::~CWindow()
{
}

void CWindow::CreateParams()
{
	m_CreateParams.m_Pos=m_Pos;
	m_CreateParams.m_Size=m_Size;

	m_CreateParams.m_Style=WS_OVERLAPPEDWINDOW;
	m_CreateParams.m_StyleEx=0;

	m_CreateParams.m_ClassWindow.style=CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	m_CreateParams.m_ClassWindow.lpfnWndProc=Window_WndProc;
    m_CreateParams.m_ClassWindow.hInstance=g_Instance;
    m_CreateParams.m_ClassWindow.hCursor=LoadCursor(0, IDC_ARROW);
	m_CreateParams.m_ClassWindow.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
	m_CreateParams.m_ClassWindow.lpszClassName=ClassName();
	m_CreateParams.m_ClassWindow.cbClsExtra=4;
}

CALLBACK CWindow::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message==WM_SIZE) {
		WINDOWPLACEMENT wp;
		GetWindowPlacement(m_Wnd,&wp);
		m_Size.x=wp.rcNormalPosition.right-wp.rcNormalPosition.left;
		m_Size.y=wp.rcNormalPosition.bottom-wp.rcNormalPosition.top;
		AlignChild();
/*	} else if(message==WM_WINDOWPOSCHANGING) {
		CPoint minsize,maxsize;
		CalcMinMaxSize(minsize,maxsize);

		WINDOWPOS * wp=(WINDOWPOS *)lParam;
		if(wp->cx>maxsize.x) wp->cx=maxsize.x;
		if(wp->cx<minsize.x) wp->cx=minsize.x;
		if(wp->cy>maxsize.y) wp->cy=maxsize.y;
		if(wp->cy<minsize.y) wp->cy=minsize.y;*/
	}
	return DefWindowProc(m_Wnd, message, wParam, lParam);
}

void CWindow::WndParent(HWND wnd)
{
	m_WndParent=wnd;
}

void CWindow::WndCreate()
{
	CreateParams();

	g_WindowClassInit=this;
	if(GetVersion()<0x80000000) {
		WNDCLASSW tempclass;
		if(GetClassInfoW(g_Instance, ClassName(), &tempclass)) UnregisterClassW(ClassName(),g_Instance);

		if(StdClassName()==NULL) {
			if(!RegisterClassExW(&m_CreateParams.m_ClassWindow)) ERROR_E;
		}

		m_Wnd=CreateWindowExW(m_CreateParams.m_StyleEx, (StdClassName()==NULL)?ClassName():StdClassName(), m_Text.Get(), m_CreateParams.m_Style, m_Pos.x, m_Pos.y, m_Size.x, m_Size.y, (m_Parent!=NULL)?(m_Parent->m_Wnd):m_WndParent, 0, g_Instance, NULL);
		if(!m_Wnd) ERROR_S(CWStr().Format(L"Error=<i>",GetLastError()).Get());

	} else {
		WNDCLASSA tempclass;
		CStr tempname(m_Heap);
		if(StdClassName()!=NULL) tempname.Set((wchar *)StdClassName());
		else {
			tempname.Set((wchar *)ClassName());

			if(GetClassInfoA(g_Instance, tempname.Get(), &tempclass)) UnregisterClassA(tempname.Get(),g_Instance);

			m_CreateParams.m_ClassWindow.lpszClassName=(wchar *)tempname.Get();
			if(!RegisterClassExA((WNDCLASSEXA *)&m_CreateParams.m_ClassWindow)) ERROR_E;
			m_CreateParams.m_ClassWindow.lpszClassName=ClassName();
		}

		m_Wnd=CreateWindowExA(m_CreateParams.m_StyleEx, tempname.Get(), CStr(m_Text,m_Heap).Get(), m_CreateParams.m_Style, m_Pos.x, m_Pos.y, m_Size.x, m_Size.y, (m_Parent!=NULL)?(m_Parent->m_Wnd):m_WndParent, 0, g_Instance, NULL);
		if(!m_Wnd) ERROR_E;
	}

	CWindow * w=m_First;
	while(w) {
		w->WndCreate();
		w=w->m_Next;
	}
}

void CWindow::WndDestroy()
{
	CWindow * w=m_First;
	while(w) {
		w->WndDestroy();
		w=w->m_Next;
	}

	if(m_Wnd) {
		DestroyWindow(m_Wnd);
	}
}

void CWindow::Add(CWindow * w)
{
	if(w->m_Parent) {
		w->m_Parent->Delete(w);
	}

	LIST_ADD(w,m_First,m_Last,m_Prev,m_Next);
	w->m_Parent=this;
}

void CWindow::Delete(CWindow * w)
{
	if(w->m_Parent!=this) ERROR_E;

	LIST_DEL(w,m_First,m_Last,m_Prev,m_Next);
	w->m_Parent=NULL;
}

void CWindow::CalcNewSize(Base::CPoint & newsize)
{
	if((m_SizeMax.x!=0) && (newsize.x>m_SizeMax.x)) newsize.x=m_SizeMax.x;
	if((m_SizeMax.y!=0) && (newsize.y>m_SizeMax.y)) newsize.y=m_SizeMax.y;
	if(newsize.x<m_SizeMin.x) newsize.x=m_SizeMin.x;
	if(newsize.y<m_SizeMin.y) newsize.y=m_SizeMin.y;
}

void CWindow::AlignChild(Base::CRect re)
{
	CPoint p;
	CWindow * w=m_First;
	while(w!=NULL) {
		if(w->m_Align==wa_Client) {
			p=CPoint(re.right-re.left,re.bottom-re.top);
			w->CalcNewSize(p);
			w->Pos(CPoint(re.left,re.top));
			w->Size(p);

			re.left+=p.x; re.top+=p.y;
		} else if(w->m_Align==wa_Left) {
			p=CPoint(w->m_Size.x,re.bottom-re.top);
			w->CalcNewSize(p);
			w->Pos(CPoint(re.left,re.top));
			w->Size(p);

			re.left+=p.x;
		} else if(w->m_Align==wa_Right) {
			p=CPoint(w->m_Size.x,re.bottom-re.top);
			w->CalcNewSize(p);
			w->Pos(CPoint(re.right-p.x,re.top));
			w->Size(p);

			re.right-=p.x;
		} else if(w->m_Align==wa_Top) {
			p=CPoint(re.right-re.left,w->m_Size.y);
			w->CalcNewSize(p);
			w->Pos(CPoint(re.left,re.top));
			w->Size(p);

			re.top+=p.y;
		} else if(w->m_Align==wa_Bottom) {
			p=CPoint(re.right-re.left,w->m_Size.y);
			w->CalcNewSize(p);
			w->Pos(CPoint(re.left,re.bottom-p.y));
			w->Size(p);

			re.bottom-=p.y;
		}
		w=w->m_Next;
	}
}

void CWindow::AlignChild()
{
	if(!m_Wnd) return;

	CRect re;
	GetClientRect(m_Wnd,&re);
	CalcClientRect(re);

	AlignChild(re);
}

void CWindow::Align(WindowAlign wa)
{
	if(m_Align==wa) return;
	m_Align=wa;
	if(m_Parent) m_Parent->AlignChild();
}

void CWindow::ShowWindow()
{
	if(!m_Wnd) return;
	::ShowWindow(m_Wnd,SW_SHOW);
	UpdateWindow(m_Wnd);

	CWindow * w=m_First;
	while(w) {
		w->ShowWindow();
		w=w->m_Next;
	}
}

void CWindow::Pos(Base::CPoint & newpos)
{
	if((m_Pos.x==newpos.x) && (m_Pos.y==newpos.y)) return;

	m_Pos=newpos;

	if(m_Wnd) ::SetWindowPos(m_Wnd,0,newpos.x,newpos.y,0,0,SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER);
}

void CWindow::Size(Base::CPoint & newsize)
{
	if((m_Size.x==newsize.x) && (m_Size.y==newsize.y)) return;

	m_Size=newsize;

	if(m_Wnd) ::SetWindowPos(m_Wnd,0,0,0,newsize.x,newsize.y,SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOMOVE|SWP_NOZORDER);
}

void CWindow::SizeMin(Base::CPoint & newsize)
{
	if(m_SizeMin.x==newsize.x && m_SizeMin.y==newsize.y) return;
	m_SizeMin=newsize;

	CPoint p=m_Size;
	CalcNewSize(p);
	if((m_Size.x==p.x) && (m_Size.y==p.y)) return;

	Size(p);
	if(m_Parent) m_Parent->AlignChild();
}

void CWindow::SizeMax(Base::CPoint & newsize)
{
	if(m_SizeMax.x==newsize.x && m_SizeMax.y==newsize.y) return;
	m_SizeMax=newsize;

	CPoint p=m_Size;
	CalcNewSize(p);
	if((m_Size.x==p.x) && (m_Size.y==p.y)) return;

	Size(p);
	if(m_Parent) m_Parent->AlignChild();
}

void CWindow::CalcMinMaxSize(Base::CPoint & minsize,Base::CPoint & maxsize)
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CPanel::CPanel():CWindow()
{
	m_Size.x=100;
	m_Size.y=100;
}

CPanel::~CPanel()
{
}

void CPanel::CreateParams()
{
	CWindow::CreateParams();

	m_CreateParams.m_Style=WS_CHILD;
	m_CreateParams.m_StyleEx=0;
}

}
