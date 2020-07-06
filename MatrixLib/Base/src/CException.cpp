// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "CException.hpp"
#include "CWStr.hpp"
#include "CStr.hpp"
#include <stdio.h>

#include "Tracer.hpp"

namespace Base {

void CException::CreateCallTrace(void)
{
    strcpy(call_trace, generate_trace_text());
}

CWStr CException::Info() 
{
    return CWStr().Format(L"<s>File=<s>\nLine=<i>\n",CWStr(CStr(call_trace)).Get(),CWStr(CStr(m_File)).Get(),m_Line);
}

CExceptionStr::CExceptionStr(const char * file,int line,const wchar * str,const wchar * str2,const wchar * str3,const wchar * str4):CException(file,line) 
{ 
	int sme,len;

	ZeroMemory(m_Str,sizeof(m_Str));
	int maxlen=(sizeof(m_Str)>>1)-1;

	sme=0;

	len=WStrLen(str);
	if((sme+len)>=maxlen) len=maxlen-sme;
	if(len>0) CopyMemory(m_Str+sme,str,len*2);
	sme+=len;

	len=WStrLen(str2);
	if((sme+len)>=maxlen) len=maxlen-sme;
	if(len>0) CopyMemory(m_Str+sme,str2,len*2);
	sme+=len;

	len=WStrLen(str3);
	if((sme+len)>=maxlen) len=maxlen-sme;
	if(len>0) CopyMemory(m_Str+sme,str3,len*2);
	sme+=len;

	len=WStrLen(str4);
	if((sme+len)>=maxlen) len=maxlen-sme;
	if(len>0) CopyMemory(m_Str+sme,str4,len*2);
	sme+=len;
}

CWStr CExceptionStr::Info()
{
	return CException::Info()+L"Text: "+m_Str;
}

}
