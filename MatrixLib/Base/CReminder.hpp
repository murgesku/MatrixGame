// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include <cstdint>
#include "BaseDef.hpp"

namespace Base {

typedef bool (*REMIND_HANDLER)(uintptr_t param);  // returns true, if core is dead

#define CHECK_TIME 10

struct SRemindCore {
    static SRemindCore *first;
    static SRemindCore *last;
    static SRemindCore *current;

    static int gtime;
    static int ctime;

    SRemindCore *next;
    SRemindCore *prev;
    int time;
    REMIND_HANDLER handler;
    uintptr_t param;

    SRemindCore(REMIND_HANDLER hand, uintptr_t par) : next(nullptr), prev(nullptr), time(gtime), handler(hand), param(par) {}
    ~SRemindCore(void) { Down(); }

    static void StaticInit(void) {
        first = nullptr;
        last = nullptr;
        current = nullptr;
        gtime = 0;
        ctime = 0;
    }

    void Down(void) {
        if (current == this)
            current = this->next;
        LIST_DEL_CLEAR(this, first, last, prev, next);
    }

    void Use(int nexttime) {
        if (first != nullptr) {
            if ((((uintptr_t)next) | ((uintptr_t)prev)) == 0 && (this != first)) {
                LIST_ADD(this, first, last, prev, next);
            }
        }
        else {
            first = this;
            last = this;
            next = nullptr;
            prev = nullptr;
        }
        time = gtime + nexttime;
    }

    static void Takt(int ms);
};

//
//#define REMINDER_TAKT       1000 // one second
//#define REMINDER_MAX_TIME   128 // in REMINDER_TAKT
//
//#define REMINDER_EMPTY      ((DWORD)(-1))
//
// typedef void (*REMINDER_HANDLER)(DWORD uid, DWORD user);
//
// typedef CBuf* PCBuf;
//
// struct SReminderItem
//{
//    SReminderItem *next_free;
//    SReminderItem *prev_free;
//
//    REMINDER_HANDLER handler;
//    DWORD            user;
//
//    int              index0;
//    int              index1;
//    int              time;
//};
//
// class CReminder : public CMain
//{
//    CHeap  *m_Heap;
//    CBuf    m_Items;
//    PCBuf   m_TimeArray[REMINDER_MAX_TIME];
//
//    int     m_Time;
//    int     m_NextTime;
//    int     m_Pointer;
//
//    int     m_Ref;
//
//    SReminderItem *m_FirstFree;
//    SReminderItem *m_LastFree;
//    SReminderItem*  NewItem(void);
//    void    ReleaseItem(SReminderItem *item);
//
//
//    ~CReminder();
//    CReminder(CHeap *heap);
//
//    //void Validate(void);
// public:
//
//    void Clear(void);
//    void Takt(int ms);
//
//    static CReminder* Build(CHeap *heap)
//    {
//        return HNew (heap) CReminder(heap);
//    }
//
//    void Release(void)
//    {
//        //--m_Ref;
//        //if (m_Ref<=0)
//        HDelete(CReminder, this, m_Heap);
//    }
//
//    //void RefInc(void) {++m_Ref;}
//
//
//    bool  RemindOldest(void);
//
//
//    DWORD Create(DWORD id, int in_time, REMINDER_HANDLER handler, DWORD user);
//    void  Delete(DWORD id);
//
//
//};
//
}  // namespace Base
