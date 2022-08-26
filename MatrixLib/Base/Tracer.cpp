// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "Base.pch"

#include "Tracer.hpp"
#include <windows.h>
#include <stdio.h>
#include <exception>

#ifdef DMM
#include "CStr.hpp"
void writedump(const char *txt) {
    static CStr buf;
    buf += txt;
    buf += "\n";
    if (buf.Len() > 1000000) {
        FILE *f = fopen("bug", "a+");
        fwrite(txt, strlen(txt), 1, f);
        fwrite(buf.Get(), buf.Len(), 1, f);
        fclose(f);
        buf.SetZeroLenNoTream();
    }
}
#endif

#if defined _DEBUG || defined _TRACE

#ifdef ENABLE_HISTORY
BYTE CDebugTracer::m_history[HISTORY_SIZE * sizeof(SDebugCallInfo)];
int CDebugTracer::m_hist_start;
int CDebugTracer::m_hist_end;
#endif

#ifdef _DEBUG
CDText *CDText::first_dtext;

void CDText::Set(const char *text) {
    delete[] v;
    v = new char[strlen(text) + 1];
    strcpy(v, text);
}
CDText::CDText(const char *key, const char *text) {
    m_prev = 0;
    m_next = 0;

    k = new char[strlen(key) + 1];
    strcpy(k, key);

    v = new char[strlen(text) + 1];
    strcpy(v, text);
}
CDText::~CDText() {
    delete[] k;
    delete[] v;
}
void CDText::T(const char *key, const char *text) {
    if (!first_dtext) {
        first_dtext = new CDText(key, text);
    }
    else {
        CDText *tmp = first_dtext;
        for (;;) {
            if (!strcmp(tmp->k, key)) {
                tmp->Set(text);
                break;
            }
            if (!tmp->m_next) {
                tmp->m_next = new CDText(key, text);
                tmp->m_next->m_prev = tmp;
                break;
            }
            tmp = tmp->m_next;
        }
    }
}

void CDText::D(const char *key) {
    CDText *tmp = first_dtext;
    while (tmp) {
        if (!strcmp(tmp->k, key)) {
            if (tmp->m_prev)
                tmp->m_prev->m_next = tmp->m_next;
            else
                first_dtext = tmp->m_next;
            if (tmp->m_next)
                tmp->m_next->m_prev = tmp->m_prev;
            delete tmp;

            break;
        }
        tmp = tmp->m_next;
    }
}

void CDText::Get(char *out) {
    *out = 0;

    CDText *tmp = first_dtext;
    while (tmp) {
        strcat(out, tmp->k);
        strcat(out, ":");
        strcat(out, tmp->v);
        tmp = tmp->m_next;

        if (tmp) {
            strcat(out, " / ");
        }
    }
}

void DbgShowDword(const char *n, DWORD sz) {
    char buf[32];
    sprintf(buf, "%i", sz);
    CDText::T(n, buf);
}

#endif

#include "../DebugMsg/DebugMsg.h"

int g_CheckPointNumber;

#define X_ELEM_COUNT(a) (sizeof(a) / sizeof(a[0]))

//-------------------------------------------------------//

struct S_EXCEPT_CODE_T {
    unsigned long ecode;
    const char *ecode_str;
};

//===========================================================================//
//                          L O C A L    C O N S T A N T S

static const unsigned long S_LOGIC_ERROR = 0x004E4F47;

//===========================================================================//
//                         L O C A L    V A R I A B L E S

static const S_EXCEPT_CODE_T s_ecodes[] = {{EXCEPTION_ACCESS_VIOLATION, "EXCEPTION_ACCESS_VIOLATION"},
                                           {EXCEPTION_ARRAY_BOUNDS_EXCEEDED, "EXCEPTION_ARRAY_BOUNDS_EXCEEDED"},
                                           {EXCEPTION_BREAKPOINT, "EXCEPTION_BREAKPOINT"},
                                           {EXCEPTION_DATATYPE_MISALIGNMENT, "EXCEPTION_DATATYPE_MISALIGNMENT"},
                                           {EXCEPTION_FLT_DENORMAL_OPERAND, "EXCEPTION_FLT_DENORMAL_OPERAND"},
                                           {EXCEPTION_FLT_DIVIDE_BY_ZERO, "EXCEPTION_FLT_DIVIDE_BY_ZERO"},
                                           {EXCEPTION_FLT_INEXACT_RESULT, "EXCEPTION_FLT_INEXACT_RESULT"},
                                           {EXCEPTION_FLT_INVALID_OPERATION, "EXCEPTION_FLT_INVALID_OPERATION"},
                                           {EXCEPTION_FLT_OVERFLOW, "EXCEPTION_FLT_OVERFLOW"},
                                           {EXCEPTION_FLT_STACK_CHECK, "EXCEPTION_FLT_STACK_CHECK"},
                                           {EXCEPTION_FLT_UNDERFLOW, "EXCEPTION_FLT_UNDERFLOW"},
                                           {EXCEPTION_ILLEGAL_INSTRUCTION, "EXCEPTION_ILLEGAL_INSTRUCTION"},
                                           {EXCEPTION_IN_PAGE_ERROR, "EXCEPTION_IN_PAGE_ERROR"},
                                           {EXCEPTION_INT_DIVIDE_BY_ZERO, "EXCEPTION_INT_DIVIDE_BY_ZERO"},
                                           {EXCEPTION_INT_OVERFLOW, "EXCEPTION_INT_OVERFLOW"},
                                           {EXCEPTION_INVALID_DISPOSITION, "EXCEPTION_INVALID_DISPOSITION"},
                                           {EXCEPTION_NONCONTINUABLE_EXCEPTION, "EXCEPTION_NONCONTINUABLE_EXCEPTION"},
                                           {EXCEPTION_PRIV_INSTRUCTION, "EXCEPTION_PRIV_INSTRUCTION"},
                                           {EXCEPTION_SINGLE_STEP, "EXCEPTION_SINGLE_STEP"},
                                           {EXCEPTION_STACK_OVERFLOW, "EXCEPTION_STACK_OVERFLOW"},
                                           {S_LOGIC_ERROR, "LOGIC ERROR"}};

//===========================================================================//
//                      L O C A L    F U N C T I O N S

static void cpp_except_terminate(void) throw() {
    char buff[65536];
    strcpy(buff, generate_trace_text());
    MessageBoxA(0, buff, "Unhandled Exception", MB_ICONEXCLAMATION);
    ExitProcess(-1);
}

//---------------------------------------------------------------------------//

static LONG WINAPI sys_except_handler(

        EXCEPTION_POINTERS *info) throw() {
    const char *ecode_str;
    int i;
    int k;
    unsigned long ecode;

    ecode = info->ExceptionRecord->ExceptionCode;

    if (ecode == 0xE06D7363)  // MS C++ exception
    {
        // DM(L"EXEC HANDLER", L"CPP");
        cpp_except_terminate();
    }

    // DM(L"EXEC HANDLER", L"OTHER");

    for (i = 0; i < X_ELEM_COUNT(s_ecodes); ++i) {
        if (ecode == s_ecodes[i].ecode) {
            break;
        }
    }

    if (i < X_ELEM_COUNT(s_ecodes)) {
        ecode_str = s_ecodes[i].ecode_str;
    }
    else {
        ecode_str = "UNKNOWN EXCEPTION";
    }

    char buff[65536];

    sprintf(buff,
            "Program executed an illegal operation and was closed :("
            "\nexception code: 0x%X(%s)"
            "\nexception address: %p\n",
            ecode, ecode_str, info->ExceptionRecord->ExceptionAddress);

    k = info->ExceptionRecord->NumberParameters;

    for (i = 0; i < k; ++i) {
        sprintf(buff + strlen(buff), "exception parameter %u) 0x%x", i, info->ExceptionRecord->ExceptionInformation[i]);
    }

    sprintf(buff + strlen(buff), "++FATAL ERROR++\n");

    strcpy(buff, generate_trace_text());

    MessageBoxA(0, buff, "Unhandled Exception", MB_ICONEXCLAMATION);

    return EXCEPTION_EXECUTE_HANDLER;
}

//---------------------------------------------------------------------------//

#ifdef ENABLE_HISTORY

void CDebugTracer::SaveHistory(void) throw() {
    FILE *file;

    file = fopen("debug_history.txt", "a");
    const char *shapka = "begin history ===============================================\n";
    fwrite(shapka, strlen(shapka), 1, file);

    int i = m_hist_start;

    if (i == m_hist_end) {
        const char *e = "empty\n";
        fwrite(e, strlen(e), 1, file);
    }
    else {
        char buf[1024];
        SDebugCallInfo *di = (SDebugCallInfo *)&m_history;
        while (i != m_hist_end) {
            sprintf(buf, "%s:%i\n", di[i]._file, di[i]._line);
            fwrite(buf, strlen(buf), 1, file);
            ++i;
            if (i >= HISTORY_SIZE) {
                i = 0;
            }
        }
        const char *e = "end\n";
        fwrite(e, strlen(e), 1, file);
    }

    fclose(file);
}

void CDebugTracer::AddHistory() throw() {
    SDebugCallInfo *di = (SDebugCallInfo *)&m_history;

#ifdef MSVC7
    di[m_hist_end]._file = m_src_func;
#else
    di[m_hist_end]._file = m_src_file;
#endif
    di[m_hist_end]._line = m_src_line;

    ++m_hist_end;
    if (m_hist_end >= HISTORY_SIZE) {
        m_hist_end = 0;
    }

    if (m_hist_end == m_hist_start) {
        ++m_hist_start;
        if (m_hist_start >= HISTORY_SIZE) {
            m_hist_start = 0;
        }
    }
}

#ifdef HISTORY_PATH_ONLY
void CDebugTracer::DelLastHistory(void) throw() {
    --m_hist_end;
}
#endif

#endif

void CDebugTracer::StaticInit(void) throw() {
    m_call_trace = 0;

#ifdef ENABLE_HISTORY
    m_hist_start = 0;
    m_hist_end = 0;
#endif

    std::set_terminate((terminate_handler)&cpp_except_terminate);

    SetUnhandledExceptionFilter(sys_except_handler);
}

//===========================================================================//
//                          G L O B A L    V A R I A B L E S

CDebugTracer *CDebugTracer::m_call_trace;

//===========================================================================//
//                          G L O B A L    F U N C T I O N S

#ifdef MSVC7
CDebugTracer ::CDebugTracer(const char *src_file, int src_line, const char *src_func, bool cp) throw()
#else
CDebugTracer ::CDebugTracer(const char *src_file, int src_line, bool cp) throw()
#endif
{

    this->init();

    this->m_src_file = src_file;
    this->m_src_line = src_line;
    this->m_checkpoint = cp;
#ifdef MSVC7
    this->m_src_func = src_func;
#endif

#ifdef ENABLE_HISTORY
    AddHistory();
#endif

    if (this->m_call_trace != 0) {
        m_call_trace->add(this);
    }
    else {
        m_call_trace = this;
    }
#ifdef _TIMING
    QueryPerformanceCounter(&m_intime);
#endif
}

CDebugTracer::~CDebugTracer() throw() {
#ifdef HISTORY_PATH_ONLY
    if (!std::uncaught_exception()) {
        DelLastHistory();
    }
#endif

#ifdef _TIMING
    LARGE_INTEGER m_outtime;
    LARGE_INTEGER m_freq;
    QueryPerformanceCounter(&m_outtime);
#endif

    this->m_call_trace = this->m_call_trace->sub(this->m_call_trace->m_prev);
#ifdef _TIMING

    QueryPerformanceFrequency(&m_freq);

    DWORD time = (DWORD)(m_outtime.QuadPart - m_intime.QuadPart);
#ifdef MSVC7
    // if (!m_checkpoint) DM((char *)m_src_func, ("Time: " + CStr((int)time)).Get());
#endif
#endif
}

//---------------------------------------------------------------------------//

void CDebugTracer ::add(

        CDebugTracer *head) throw() {
    CDebugTracer *p;

    p = this->m_prev;

    this->m_prev = head->m_prev;

    p->m_next = head;

    head->m_prev = p;
}

//---------------------------------------------------------------------------//

CDebugTracer *CDebugTracer ::sub(

        CDebugTracer *item) throw() {
    CDebugTracer *head;
    CDebugTracer *n;
    CDebugTracer *p;

    head = this;
    n = item->m_next;
    p = item->m_prev;

    if (head != item) {
        p->m_next = n;
    }
    else {
        head = n;
    }

    if (n != 0) {
        n->m_prev = p;
    }
    else {
        if (head != 0) {
            head->m_prev = p;
        }
    }

    item->init();

    return head;
}

//---------------------------------------------------------------------------//

const CDebugTracer *CDebugTracer::get_call(const CDebugTracer *item) throw() {
    if (item != 0) {
        if (item != CDebugTracer::m_call_trace) {
            return item->m_prev;
        }

        return 0;
    }
    else {
        return CDebugTracer::m_call_trace->m_prev;
    }
}

#endif  //  #ifdef _DEBUG  //

char *generate_trace_text(void) {
    static char call_trace[65536];
    strcpy(call_trace, "call stack:\n");
#ifdef _DEBUG
    for (const CDebugTracer *item = 0;;) {
        item = CDebugTracer::get_call(item);
        if (item == 0) {
            break;
        }
#ifdef MSVC7
        if (item->m_checkpoint) {
            sprintf(call_trace + strlen(call_trace), "\tcheck point: %s - %i\n", item->m_src_file, item->m_src_line);
        }
        else {
            sprintf(call_trace + strlen(call_trace), "\t%s - %s\n", item->m_src_file, item->m_src_func);
        }
#else
        if (item->m_checkpoint) {
            sprintf(call_trace + strlen(call_trace), "\tcheck point: %s - %i\n", item->m_src_file, item->m_src_line);
        }
        else {
            sprintf(call_trace + strlen(call_trace), "\t%s:%u\n", item->m_src_file, item->m_src_line);
        }

#endif
    }
    strcat(call_trace, "\n");
#else
    strcat(call_trace, "[unavailable in release]\n");
#endif

    return call_trace;
}
