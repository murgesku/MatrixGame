// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "CReminder.hpp"

namespace Base {

SRemindCore *SRemindCore::first;
SRemindCore *SRemindCore::last;
SRemindCore *SRemindCore::current;
int SRemindCore::gtime;
int SRemindCore::ctime;

void SRemindCore::Takt(int ms) {
    if (first == nullptr)
        return;
    if (current == nullptr)
        current = first;

    gtime += ms;
    while (gtime > ctime) {
        int checks = ms / CHECK_TIME;
        if (checks == 0)
            checks = 1;
        ctime += checks * CHECK_TIME;

        while (checks-- > 0) {
            current = current->next;
            if (current == nullptr)
                current = first;
            if (gtime > current->time) {
                if (!current->handler(current->param)) {
                    current->Down();
                }
                if (current == nullptr)
                    current = first;
                if (current == nullptr)
                    return;
            }
        }
    }
}

//
//
//
//
//
//
//
//
// CReminder::CReminder(CHeap *heap):CMain(),m_Items(heap, sizeof(SReminderItem) * 1000), m_Heap(heap)
//{
//    DTRACE();
//
//    for (int i=0; i<REMINDER_MAX_TIME; ++i)
//    {
//        m_TimeArray[i] = HNew(m_Heap) CBuf(m_Heap,256);
//        m_TimeArray[i]->Expand(1024);
//        m_TimeArray[i]->SetLenNoShrink(0);
//
//    }
//
//    m_Time = 0;
//    m_Pointer = 0;
//    m_NextTime = REMINDER_TAKT;
//    m_FirstFree = nullptr;
//    m_LastFree = nullptr;
//
//    m_Ref = 1;
//}
//
//
// CReminder::~CReminder()
//{
//    DTRACE();
//    for (int i=0; i<REMINDER_MAX_TIME; ++i)
//    {
//        HDelete(CBuf, m_TimeArray[i] ,m_Heap);
//    }
//
//#ifdef _DEBUG
//    memset(m_TimeArray, 0, sizeof(m_TimeArray));
//#endif
//}
//
// void CReminder::Clear(void)
//{
//    DTRACE();
//
//    //Validate();
//
//    m_Items.Clear();
//    for (int i=0; i<REMINDER_MAX_TIME; ++i)
//    {
//        m_TimeArray[i]->Clear();
//    }
//    m_Time = 0;
//    m_Pointer = 0;
//    m_NextTime = REMINDER_TAKT;
//    m_FirstFree = nullptr;
//    m_LastFree = nullptr;
//
//    //Validate();
//}
//
// SReminderItem*  CReminder::NewItem(void)
//{
//    DTRACE();
//    if (m_FirstFree)
//    {
//        SReminderItem *i = m_FirstFree;
//        LIST_DEL(i, m_FirstFree, m_LastFree, prev_free, next_free);
//        return i;
//    }
//
//    BYTE *pp = m_Items.Buff<BYTE>();
//
//    m_Items.Expand(sizeof(SReminderItem));
//
//    BYTE *pp2 = m_Items.Buff<BYTE>();
//
//    if (pp != pp2)
//    {
//        //rebasing
//
//        int delta = pp2 - pp;
//
//        if (m_FirstFree) m_FirstFree = (SReminderItem *)((BYTE *)m_FirstFree + delta);
//        if (m_LastFree) m_LastFree = (SReminderItem *)((BYTE *)m_LastFree + delta);
//
//        SReminderItem * i = m_FirstFree;
//        while (i != nullptr)
//        {
//            if (i->next_free) i->next_free = (SReminderItem *)((BYTE *)i->next_free + delta);
//            if (i->prev_free) i->prev_free = (SReminderItem *)((BYTE *)i->prev_free + delta);
//
//            i = i->next_free;
//        }
//    }
//
//    return (SReminderItem*)(((BYTE *)m_Items.Get()) + m_Items.Len() - sizeof(SReminderItem));
//}
//
// void    CReminder::ReleaseItem(SReminderItem *item)
//{
//    DTRACE();
//    LIST_ADD(item, m_FirstFree, m_LastFree, prev_free, next_free);
//    item->handler = nullptr;
//}
//
///*
// void CReminder::Validate(void)
//{
//#ifdef _DEBUG
//    for (int i = 0; i<REMINDER_MAX_TIME; ++i)
//    {
//        CBuf *b = m_TimeArray[i];
//
//        DWORD *id = (DWORD*)b->Get();
//        DWORD *idend = (DWORD*)((BYTE*)b->Get() + b->Len());
//        while (id < idend)
//        {
//            SReminderItem * item = (SReminderItem *)m_Items.Get() + (*id);
//            if (item->handler == nullptr)
//            {
//                DM("ID" + CStr((int)*id), "nullptr" );
//                return;
//                //ERROR_E;
//            }
//
//            SReminderItem *it = m_FirstFree;
//            while (it)
//            {
//                if (it == item)
//                {
//                    DM("ID" + CStr((int)*id), "EQ" );
//                    return;
//                    ERROR_E;
//                }
//                it = it->next_free;
//            }
//
//            id++;
//        }
//    }
//#endif
//}
//*/
//
// bool  CReminder::RemindOldest(void)
//{
//    DTRACE();
//
//    int p = m_Pointer;
//
//    bool ok = false;
//    do
//    {
//        CBuf *b = m_TimeArray[p];
//
//        DWORD *id = b->Buff<DWORD>();
//        DWORD *idend = b->BuffEnd<DWORD>();
//
//        if (id < idend)
//        {
//            --idend;
//
//            SReminderItem * item = (SReminderItem *)m_Items.Get() + (*idend);
//            item->handler(*id, item->user);
//
//            b->SetLenNoShrink(b->Len() - sizeof(DWORD));
//
//            ReleaseItem(item);
//
//            ok = true;
//        } else
//        {
//            ++p; if (p >= REMINDER_MAX_TIME) p = 0;
//            if (p == m_Pointer)
//            {
//                // :((((((((((((((
//                // no free memory
//                return true;
//
//            }
//        }
//
//
//    } while(!ok);
//    return false;
//
//}
//
// void CReminder::Takt(int ms)
//{
//    DTRACE();
//
//    //Validate();
//
//    m_Time += ms;
//    while (m_Time >= m_NextTime)
//    {
//        m_NextTime += REMINDER_TAKT;
//
//
//        CBuf *b = m_TimeArray[m_Pointer];
//
//        DWORD *id = b->Buff<DWORD>();
//        DWORD *idend = b->BuffEnd<DWORD>();
//        while (id < idend)
//        {
//            SReminderItem * item = (SReminderItem *)m_Items.Get() + (*id);
//            item->handler(*id, item->user);
//
//            ++id;
//            ReleaseItem(item);
//        }
//        b->Clear();
//
//        ++m_Pointer; if (m_Pointer >= REMINDER_MAX_TIME) m_Pointer = 0;
//    }
//
//    //Validate();
//}
//
// DWORD CReminder::Create(DWORD id, int in_time, REMINDER_HANDLER handler, DWORD user)
//{
//    DTRACE();
//
//    SReminderItem *item;
//
//    if (id == REMINDER_EMPTY)
//    {
//        item = NewItem();
//        item->handler = handler;
//        item->user = user;
//        id = item - (SReminderItem *)m_Items.Get();
//
//        //if (id == 124) _asm int 3
//
//    } else
//    {
//        item = (SReminderItem *)m_Items.Get() + id;
//        if ((item->time + REMINDER_TAKT) > m_Time) return id;
//
//        //if (id == 46) _asm int 3
//
//
//        int ptl = m_TimeArray[item->index0]->Len() - sizeof(DWORD);
//
//        //ASSERT(ptl >=0);
//
//        DWORD rid =  *(DWORD*)((BYTE *)m_TimeArray[item->index0]->Get() + ptl);
//
//        SReminderItem *item1 = (SReminderItem *)m_Items.Get() + rid;
//        item1->index1 = item->index1;
//        *(DWORD *)(((BYTE *)m_TimeArray[item->index0]->Get()) + item->index1) = rid;
//        m_TimeArray[item->index0]->SetLenNoShrink(ptl);
//
//    }
//
//
//    // validate
//#ifdef _DEBUG
//    for (int i = 0; i<REMINDER_MAX_TIME; ++i)
//    {
//        CBuf *b = m_TimeArray[i];
//
//        DWORD *ids = b->Buff<DWORD>();
//        DWORD *idend = b->BuffEnd<DWORD>();
//        while (ids < idend)
//        {
//            SReminderItem * item = (SReminderItem *)m_Items.Get() + (*ids);
//            if (item->user == user)
//            {
//                _asm int 3
//            }
//            ++ids;
//        }
//    }
//#endif
//    // validate
//
//
//    item->time = m_Time;
//
//    int idx0 = in_time / (REMINDER_TAKT) + 1;
//    if (idx0 > (REMINDER_MAX_TIME-1))
//    {
//        idx0 = REMINDER_MAX_TIME-1;
//    }
//    idx0 += m_Pointer;
//    if (idx0 >= REMINDER_MAX_TIME) idx0 -= REMINDER_MAX_TIME;
//
//    //DM_("idx0", CStr(idx0).Get());
//
//    //Validate();
//
//    //int bl = m_TimeArray[idx0]->Len();
//    //int bm = m_TimeArray[idx0]->m_Max;
//    //void *bb = m_TimeArray[idx0]->Get();
//
//    //m_TimeArray[idx0]->Pointer(m_TimeArray[idx0]->Len());
//    //m_TimeArray[idx0]->Dword(id);
//
//    m_TimeArray[idx0]->Expand(sizeof(DWORD));
//    *(m_TimeArray[idx0]->BuffEnd<DWORD>() - 1) = id;
//
//
//    //Validate();
//
//
//    item->index0 = idx0;
//    item->index1 = m_TimeArray[idx0]->Len() - sizeof(DWORD);
//
//    //Validate();
//
//    return id;
//
//}
// void  CReminder::Delete(DWORD id)
//{
//    DTRACE();
//
//    if (id == REMINDER_EMPTY) return;
//    //Validate();
//#ifdef _DEBUG
//
//    if (int(id * sizeof(SReminderItem)) > m_Items.Len()) _asm int 3
//
//#endif
//    SReminderItem *item = (SReminderItem *)m_Items.Get() + id;
//#ifdef _DEBUG
//
//    SReminderItem *it = m_FirstFree;
//    while (it)
//    {
//        if (it == item)
//        {
//            _asm int 3
//        }
//        it = it->next_free;
//    }
//
//#endif
//
//    ReleaseItem(item);
//    //DM("ID"+ CStr((int)id), "FREED" );
//
//    int ptl = m_TimeArray[item->index0]->Len() - sizeof(DWORD);
//
//    ASSERT(ptl >= 0);
//
//    DWORD rid =  *(DWORD*)((BYTE *)m_TimeArray[item->index0]->Get() + ptl);
//
//    SReminderItem *item1 = (SReminderItem *)m_Items.Get() + rid;
//
//    ASSERT (item1->index1 == ptl);
//
//    item1->index1 = item->index1;
//    *(DWORD *)(((BYTE *)m_TimeArray[item->index0]->Get()) + item->index1) = rid;
//    m_TimeArray[item->index0]->SetLenNoShrink(ptl);
//
//    //Validate();
//}

}  // namespace Base