// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "WindowDef.hpp"
#include "base.hpp"

namespace Window {

typedef enum { wa_None, wa_Left, wa_Right, wa_Top, wa_Bottom, wa_Client } WindowAlign;

class WINDOW_API CWindowCreateParams : public Base::CMain {
	public:
		Base::CHeap * m_Heap;
		WNDCLASSEXW m_ClassWindow;
		int m_Style;
		int m_StyleEx;
		Base::CPoint m_Pos;
		Base::CPoint m_Size;
	public:
		CWindowCreateParams(Base::CHeap * heap):Base::CMain()									{ m_Heap=heap; ZeroMemory(&m_ClassWindow,sizeof(WNDCLASSEXW)); m_ClassWindow.cbSize = sizeof(WNDCLASSEXW); m_Style=0; m_StyleEx=0; m_Pos.x=0; m_Pos.y=0; m_Size.x=0; m_Size.y=0; }
		~CWindowCreateParams()																	{}
};

class WINDOW_API CWindow : public Base::CMain {
	friend LRESULT CALLBACK Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	protected:
		Base::CHeap * m_Heap;

		HWND m_Wnd;
		HWND m_WndParent;

		CWindowCreateParams m_CreateParams;

		CWindow * m_Parent;
		CWindow * m_First;
		CWindow * m_Last;
		CWindow * m_Next;
		CWindow * m_Prev;

		Base::CWStr m_Text;
		Base::CPoint m_Pos;
		Base::CPoint m_Size;

		WindowAlign m_Align;
		Base::CPoint m_SizeMin;
		Base::CPoint m_SizeMax;

	public:
		CWindow(Base::CHeap * heap=NULL);
		~CWindow();

		HWND Wnd(void)									{ return m_Wnd; }

		virtual wchar * ClassName(void)					{ return L"CWindow"; }
		virtual wchar * StdClassName(void)				{ return NULL; }
		virtual void CreateParams(void);

		virtual CALLBACK WndProc(UINT message, WPARAM wParam, LPARAM lParam);

		void WndParent(HWND wnd);
		void WndCreate(void);
		void WndDestroy(void);

		void Add(CWindow * w);							// Сменить родительское окно
		void Delete(CWindow * w);						// Удалить из родительского окна
		CWindow * Parent(void)							{ return m_Parent; }

		virtual void CalcNewSize(Base::CPoint & newsize);
		virtual void CalcClientRect(Base::CRect & rect)		{}
		void AlignChild(Base::CRect re);
		void AlignChild(void);
		void Align(WindowAlign wa);
		WindowAlign Align(void)								{ return m_Align; }

		void ShowWindow(void);
		void Pos(Base::CPoint & newpos);
		void Size(Base::CPoint & newsize);

		void SizeMin(Base::CPoint & newsize);
		void SizeMax(Base::CPoint & newsize);
		void CalcMinMaxSize(Base::CPoint & minsize,Base::CPoint & maxsize);
};

class CPanel : public CWindow {
	public:
	public:
		CPanel(void);
		~CPanel();

		virtual wchar * ClassName(void)					{ return L"CPanel"; }

		virtual void CreateParams(void);
};

WINDOW_API void Window_Init(HINSTANCE hInstance);

}
