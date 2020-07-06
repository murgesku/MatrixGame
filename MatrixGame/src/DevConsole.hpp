// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef DEV_CONSOLE_INCLUDE
#define DEV_CONSOLE_INCLUDE

//class CWStr;

typedef void (*CMD_HANDLER)(const Base::CWStr &cmd, const Base::CWStr &params);

struct SCmdItem
{
    const wchar *cmd;
    CMD_HANDLER  handler;
};


#define DCON_ACTIVE   SETBIT(0)
#define DCON_CURSOR   SETBIT(1)
#define DCON_SHIFT    SETBIT(2)


#define DEV_CONSOLE_CURSOR_FLASH_PERIOD 300

class CDevConsole : public CMain
{
    DWORD m_Flags;

    static SCmdItem    m_Commands[];

    CWStr              m_Text;
    int                m_CurPos;

    int                m_NextTime;
    int                m_Time;

public:
    CDevConsole(void);
    ~CDevConsole();

    bool IsActive(void) const {return FLAG(m_Flags,DCON_ACTIVE);}
    void SetActive(bool active);

    void ShowHelp(void);

    void Takt(int ms);
    void Keyboard(int scan, bool down);

};


#endif