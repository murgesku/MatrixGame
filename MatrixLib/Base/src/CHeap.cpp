// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "CHeap.hpp"
#include "CException.hpp"

#ifdef DEAD_PTR_SPY_ENABLE
#define DEAD_PTR_SPY_SEEK   10
namespace DeadPtr
{
    int get_id(void)
    {
        static int      last_id;
        return last_id++;
    }
    BYTE     semetery_heap[SEMETERY_HEAP_SIZE];
    SDeadMem semetery[SEMETERY_SIZE];

    int semetery_cnt = 0;
    int semetery_heap_size = 0;

    int find_by_id(int id, BYTE **ptr)
    {
        SDeadMemBody *b = (SDeadMemBody *)&semetery_heap;

        while (id != b->id)
        {
            b = (SDeadMemBody *)(((BYTE *)b) + b->size);
        }

        *ptr = (BYTE *)b;
        return b->size;
    }
    void remove_by_id(int id)
    {
        BYTE *ptr;
        int sz = find_by_id(id, &ptr);

        memcpy(ptr, ptr+sz, semetery_heap_size - sz);
        semetery_heap_size -= sz;
    }
    void remove_by_ptr(void *ptr)
    {
        for (int i=0; i<semetery_cnt; ++i)
        {
            if (semetery[i].ptr_was == ptr)
            {
                int id = semetery[i].id;
                remove_by_id(id);

                memcpy(semetery+i, semetery+i+1, (semetery_cnt-i-1)*sizeof(SDeadMem));
                --semetery_cnt;
                return;
            }
        }
    }

    void remove_first(void)
    {
        remove_by_id(semetery[0].id);
        memcpy(semetery, semetery+1, (SEMETERY_SIZE-1)*sizeof(SDeadMem));
        --semetery_cnt;
    }


    void * get_dead_mem(void *mem)
    {
        for (int i=0; i<semetery_cnt; ++i)
        {
            if (semetery[i].ptr_was == mem)
            {
                BYTE *ptr;
                int sz = find_by_id(semetery[i].id, &ptr);
                return ptr + sizeof(SDeadMemBody);
            }
        }

        return NULL;
    }

    void free_mem(void *mem0)
    {
        int *mem = (int *)mem0;

        int idx=0;
        for (;idx<DEAD_PTR_SPY_SEEK;++idx)
        {
            if (my_signature(mem + idx)) goto ok;
        }
            
        return;

ok:;
        
        {
            int sz = sizeof(SDeadMemBody) + my_signature_size(mem + idx);
            if (sz > SEMETERY_HEAP_SIZE) return;

            if (semetery_cnt >= SEMETERY_SIZE)
            {
                remove_first();
            }

            while ((semetery_heap_size+sz) > SEMETERY_HEAP_SIZE)
            {
                remove_first();
            }

            semetery[semetery_cnt].id = get_id();
            semetery[semetery_cnt].ptr_was = mem;
            semetery[semetery_cnt].size = sz;

            SDeadMemBody body;
            body.id = semetery[semetery_cnt].id;
            body.size = sz;

            memcpy(semetery_heap + semetery_heap_size, &body, sizeof(SDeadMemBody));
            memcpy(semetery_heap + semetery_heap_size + sizeof(SDeadMemBody), mem, sz - sizeof(SDeadMemBody));
            semetery_heap_size += sz;
            ++semetery_cnt;
        }
    }

}
#endif


namespace Base {

#ifdef MEM_SPY_ENABLE

#include <stdio.h>

SMemHeader *SMemHeader::first_mem_block;
SMemHeader *SMemHeader::last_mem_block;
uint SMemHeader::fullsize;
uint SMemHeader::maxblocksize;

void CHeap::StaticInit(void)
{
    SMemHeader::first_mem_block = NULL;
    SMemHeader::last_mem_block = NULL;
    SMemHeader::fullsize = 0;
    SMemHeader::maxblocksize = 0;
}

void        SMemHeader::Release(void)
{
    LIST_DEL(this, first_mem_block, last_mem_block, prev, next);
    fullsize -= blocksize;
#ifdef MEM_CHECK
    BYTE *d1 = (BYTE *)(this + 1);
    BYTE *d2 = ((BYTE *)this) + blocksize - MEM_CHECK_BOUND_SIZE;
    for (int i=0; i<MEM_CHECK_BOUND_SIZE; ++i)
    {
        bool begin = *(d1 + i) != MEM_CHECK_FILLER;
        bool end = *(d2 + i) != MEM_CHECK_FILLER;
        if (begin || end)
        {
#ifdef _DEBUG
            if (begin)
            {
                ERROR_S((L"Memory corruption detected at (begin):\n" + CWStr(CStr(file)) + L" - " + CWStr(line)).Get());
            } else
            {
                ERROR_S((L"Memory corruption detected at (end):\n" + CWStr(CStr(file)) + L" - " + CWStr(line)).Get());
            }
#else
            char buf[1024];
            strcpy(buf, "Memory corruption detected at:\n");
            strcat(buf,file);
            sprintf(buf + strlen(buf), " - %i", line);

            MessageBox(NULL, buf, "Memory corruption!", MB_OK|MB_ICONERROR);

            _asm int 3
#endif
        }
    }

    memset(d2, 0, MEM_CHECK_BOUND_SIZE);
#endif
}
void CHeap::Free(void * buf, const char * file, int line)
{
    if (buf == NULL)
    {
        CWStr   t(L"NULL pointer is passed to Free function at:\n");
        t += CWStr(CStr(file));
        t += L" - ";
        t += line;

        ERROR_S(t.Get());
    }

#ifdef DEAD_PTR_SPY_ENABLE
    DeadPtr::free_mem(buf);
#endif

    SMemHeader *h = SMemHeader::CalcBegin(buf); 
    h->Release();
    Free(h);
}

void CHeap::StaticDeInit(void)
{
    // check!!!!!!!!!
    if (SMemHeader::first_mem_block)
    {
        char    buf[65536];
        strcpy(buf, "There are some memory leaks have detected:\n");
        while (SMemHeader::last_mem_block)
        {
            int len = strlen(buf);
            if (len < 65000)
            {
                sprintf(buf + len, "%i, %u : %s - %i\n", SMemHeader::last_mem_block->cnt, SMemHeader::last_mem_block->blocksize, SMemHeader::last_mem_block->file, SMemHeader::last_mem_block->line);
                char *f = strstr(buf, buf + len);
                if (f && f != (buf + len))
                {
                    buf[len] = 0;
                }
                //sprintf(buf + len, "%u : %s - %i\n", SMemHeader::last_mem_block->blocksize, SMemHeader::last_mem_block->file, SMemHeader::last_mem_block->line);
            } else 
            {
                strcat(buf, "............ and more.......");
                break;
            }
            SMemHeader::last_mem_block = SMemHeader::last_mem_block->prev;
        }
        MessageBoxA(0,buf,"Memory leaks!!!!!!!!!", MB_ICONEXCLAMATION);
    }

}

#endif


CHeap::CHeap() : CMain()
{
	m_Flags=0;
	m_Heap=GetProcessHeap();
	if(m_Heap==0) ERROR_E;
}

CHeap::CHeap(int initsize,int maxsize,dword flags)
{
	m_Flags=flags;
	m_Heap=HeapCreate(flags,initsize,maxsize);
	if(m_Heap==NULL) { m_Flags=0; m_Heap=GetProcessHeap(); ERROR_E; }
}

CHeap::~CHeap()
{
	Clear();
}

void CHeap::Clear()
{
	if(m_Heap!=GetProcessHeap()) {
		HeapDestroy(m_Heap);
		m_Heap=GetProcessHeap();
		if(m_Heap==0) ERROR_E;
	}
	m_Flags=0;
}

void CHeap::Create(int initsize, int maxsize, dword flags)
{
	Clear();

	m_Flags=flags;
	m_Heap=HeapCreate(flags,initsize,maxsize);
	if(m_Heap==NULL) { m_Flags=0; m_Heap=GetProcessHeap(); ERROR_E; }
}

void CHeap::AllocationError(int zn)
{
    __debugbreak();
#ifdef _DEBUG
    __debugbreak();
#else
    
    wchar buf[256];
    wcscpy(buf, L"NULL memory allocated on ");

    wchar temp[64];


	wchar * tstr=temp;
	int tsme=63;

	int fm=0;
	if(zn<0) { fm=1; zn=-zn; }

	while(zn>0) {
		tsme--;
		tstr[tsme]=wchar(zn-int(zn/10)*10)+wchar('0');
		zn=zn/10;
	}
	if(fm) { tsme--; tstr[tsme]=L'-'; }
	int cnt=63-tsme;
	for(int i=0;i<cnt;i++) tstr[i]=tstr[tsme+i];
	tstr[cnt]=0;

    wcscat(buf,temp);
    wcscat(buf,L" bytes request.");

    ERROR_S(buf);
#endif

}


#ifdef MEM_SPY_ENABLE
void HListPrint(wchar * filename)
{
    char sbuf[1024];

    CBuf buf;
    buf.SetGranula(1024*1024);

    SMemHeader * fb=SMemHeader::first_mem_block;
    while(fb) {
        sprintf(sbuf,"%d\t%d\t%d\t%s\t%d\n",DWORD(fb),DWORD(fb)+fb->blocksize-1,fb->blocksize,fb->file,fb->line);

        buf.BufAdd(sbuf,strlen(sbuf));
//        fi.Write(sbuf,strlen(sbuf));

        fb=fb->next;
    }
    CFile fi(filename);
    fi.Create();
    fi.Write(buf.Get(),buf.Len());
}
#endif

}