// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef TRACER_HPP
#define TRACER_HPP

#ifdef DMM
void writedump(const char *txt);
#endif

// lint -e522, -e10, -e533, -e1712
#pragma warning(disable : 4508)
inline unsigned __int64 GetCPUTakt(void) {
    return __rdtsc();  // 0x0F, 0x31
    //        _emit      0x0f
    //        _emit      0x31
}
#pragma warning(default : 4508)
// lint +e522, +e10, +e533

#if defined _DEBUG || defined _TRACE

#define ENABLE_HISTORY
#define HISTORY_PATH_ONLY

#define SLOG(n, s)                  \
    {                               \
        FILE *f = fopen(n, "a");    \
        fwrite(s, strlen(s), 1, f); \
        fclose(f);                  \
    }

#ifdef _DEBUG

#define DCNT(s)                    \
    {                              \
        static int ccc = 1;        \
        CDText::T(s, CStr(ccc++)); \
    }
#define DDVECT(s, v) CDText::T(s, "(" + CStr((v).x) + "|" + CStr((v).y) + "|" + CStr((v).z) + ")");

class CDText {
    static CDText *first_dtext;

    char *k;
    char *v;

    CDText *m_next;
    CDText *m_prev;

    void Set(const char *text);
    CDText(const char *key, const char *text);
    ~CDText();

public:
    static void StaticInit(void) { first_dtext = NULL; }

    static void T(const char *key, const char *text);
    static void D(const char *key);

    static void Get(char *out);
};
#endif
// lint +e1712

extern int g_CheckPointNumber;

#define DCPO_START() \
    { g_CheckPointNumber = 0; }
#define DCPO(x)                                \
    {                                          \
        if ((x) == (g_CheckPointNumber + 1)) { \
            g_CheckPointNumber = x;            \
        }                                      \
        else if ((x) != g_CheckPointNumber) {  \
            debugbreak();                      \
        };                                     \
    }

void DbgShowDword(const char *n, DWORD sz);

#if _MSC_VER >= 1300
#define MSVC7
#endif

//===========================================================================//
//                              M A C R O S

#define JOUNMACRO2(x, y) x##y
#define JOUNMACRO1(x, y) JOUNMACRO2(x, y)

#ifdef MSVC7
#define UNIQID(x) JOUNMACRO1(x, __COUNTER__)
#define DTRACE()                                                             /*lint -e1025 -e1703*/ \
    CDebugTracer call_trace_profiler(__FILE__, __LINE__, __FUNCSIG__, false) /*lint +e1025 +e1703*/
#define DCP()                                                                /*lint -e1025 -e1703*/ \
    CDebugTracer UNIQID(check_point)(__FILE__, __LINE__, __FUNCSIG__, true)  /*lint +e1025 +e1703*/
#else
#define UNIQID(x) JOUNMACRO1(x, __LINE__)
#define DTRACE()  CDebugTracer call_trace_profiler(__FILE__, __LINE__, false)
#define DCP()     CDebugTracer UNIQID(check_point)(__FILE__, __LINE__, true)
#endif

#define TRACE_PARAM_DEF  const char *_file, int _line,
#define TRACE_PARAM_CALL __FILE__, __LINE__,

#define HISTORY_SIZE 1024

struct SDebugCallInfo {
    const char *_file;
    int _line;
    SDebugCallInfo(const char *file, int line) : _file(file), _line(line) {}
    SDebugCallInfo(const SDebugCallInfo &ci) { *this = ci; }
};
#define DEBUG_CALL_INFO SDebugCallInfo(__FILE__, __LINE__)

// lint -e773
#define TAKT_BEGIN() unsigned __int64 t___takt = GetCPUTakt()
// lint +e773
#define TAKT_END(ss)                               \
    unsigned __int64 t___takt2 = GetCPUTakt();     \
    static int t___i = 0;                          \
    static int t___smooth[64];                     \
    t___smooth[t___i] = int(t___takt2 - t___takt); \
    t___i = (t___i + 1) & 63;                      \
    int t___s = 0;                                 \
    for (int t___ii = 0; t___ii < 64; ++t___ii)    \
        t___s += t___smooth[t___ii];               \
    CDText::T(ss, CStr(t___s / 64))

//===========================================================================//
//                              T Y P E S
// lint -e1712
class CDebugTracer {
    //-------------------------------------------------------//
    //                          D A T A

    static CDebugTracer *m_call_trace;

    CDebugTracer *m_next;
    CDebugTracer *m_prev;
#ifdef _TIMING
    LARGE_INTEGER m_intime;
#endif

#ifdef MSVC7
    const char *m_src_func;
#endif
    const char *m_src_file;
    int m_src_line;
    bool m_checkpoint;

#ifdef ENABLE_HISTORY
    static BYTE m_history[HISTORY_SIZE * sizeof(SDebugCallInfo)];
    static int m_hist_start;
    static int m_hist_end;
#endif

    //-------------------------------------------------------//
    //                      M E T H O D S

    inline void init(void) throw() {
        this->m_next = 0;
        this->m_prev = this;
    }

    //-------------------------------------------------------//

#ifdef ENABLE_HISTORY
    void AddHistory(void) throw();
#ifdef HISTORY_PATH_ONLY
    void DelLastHistory(void) throw();
#endif
#endif

    void add(CDebugTracer *head) throw();

    CDebugTracer *sub(CDebugTracer *item) throw();

public:
#ifdef ENABLE_HISTORY
    static void SaveHistory(void) throw();
#endif

    static void StaticInit(void);

    friend char *generate_trace_text(void);

#ifdef MSVC7
    CDebugTracer(const char *src_file, int src_line, const char *src_func, bool cp) throw();
#else
    CDebugTracer(const char *src_file, int src_line, bool cp) throw();
#endif
    //-------------------------------------------------------//

    ~CDebugTracer() throw();

    //-------------------------------------------------------//

    static const CDebugTracer *get_call(const CDebugTracer *item) throw();
};
// lint +e1712

#else  //  #ifdef DEBUG  //

#ifdef DMM
#define DTRACE() writedump(__FUNCSIG__)
#else
#define DTRACE()
#endif

#define DCP()
#define DCPO_START()
#define DCPO(x)

#define SLOG(n, s)

#define TRACE_PARAM_DEF
#define TRACE_PARAM_CALL
#define DEBUG_CALL_INFO

struct CDText {
    static void T(const char *, const char *){};
};

char *generate_trace_text(void);

#endif  //  #ifdef _DEBUG  //

#endif  //  #ifndef PROF_HPP  //
