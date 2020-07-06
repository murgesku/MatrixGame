// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "basedef.hpp"

namespace Base {

class CWStr;

#define ERROR_E throw new Base::CException(__FILE__,__LINE__)
#define ERROR_S(s) throw new Base::CExceptionStr(__FILE__,__LINE__,s)
#define ERROR_S2(s,s2) throw new Base::CExceptionStr(__FILE__,__LINE__,s,s2)
#define ERROR_S3(s,s2,s3) throw new Base::CExceptionStr(__FILE__,__LINE__,s,s2,s3)
#define ERROR_S4(s,s2,s3,s4) throw new Base::CExceptionStr(__FILE__,__LINE__,s,s2,s3,s4)

#ifdef ASSERT_OFF
	#define ASSERT(zn)
#else
#ifdef _TRACE
#define ASSERT(zn) {if((zn)==0) ERROR_E;}
#else
#define ASSERT(zn) {if((zn)==0) _asm int 3}
#endif
#endif

//lint -e1712
class BASE_API CException
{
        const char * m_File;
		int m_Line;

        char call_trace[65536];
	public:
        CException(const char * file,int line):m_File(file), m_Line(line) { CreateCallTrace(); }

        void CreateCallTrace(void);

		virtual CWStr Info(void);
};

class BASE_API CExceptionStr : public CException
{
		wchar m_Str[128];
	public:
		CExceptionStr(const char * file,int line,const wchar * str,const wchar * str2=NULL,const wchar * str3=NULL,const wchar * str4=NULL);

		virtual CWStr Info(void);
};
//lint +e1712
}
