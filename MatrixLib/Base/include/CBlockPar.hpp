// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "CMain.hpp"
#include "CHeap.hpp"
#include "CException.hpp"
#include "CStr.hpp"
#include "CWStr.hpp"
#include "CBuf.hpp"
#include "Tracer.hpp"

#define SFT(x) SFT_fun(CStr() + x)

namespace Base {

void SFT_fun(CStr out);

class CBlockPar;
class BPCompiler;

class BASE_API CBlockParUnit : public CMain
{
	friend CBlockPar;
	friend BPCompiler;
	private:
		CHeap* m_Heap;

        CBlockParUnit* m_Prev;
        CBlockParUnit* m_Next;
        CBlockPar* m_Parent;

		int m_Type;		// 0-empty 1-par 2-block
		CWStr m_Name;
		CWStr m_Com;
		union {
			CWStr* m_Par;
			CBlockPar* m_Block;
		};

		int m_FastFirst; // Смещение до первого элемента с одинаковым именем
		int m_FastCnt;	 // Количество с одинаковым именем. Инициализировано только для первого
	public:
		CBlockParUnit(CHeap* heap = NULL);
		~CBlockParUnit();

		void Clear(void);
        void ChangeType(int nt);

		void CopyFrom(CBlockParUnit& bp);
};

class BASE_API CBlockPar : public CMain
{
	friend CBlockParUnit;
	friend BPCompiler;
	private:
		CHeap* m_Heap;

        CBlockParUnit* m_First;
        CBlockParUnit* m_Last;

		int m_Cnt;
        int m_CntPar;
        int m_CntBlock;

        CBlockParUnit** m_Array;
        int m_ArrayCnt;

        CWStr* m_FromFile;

        DWORD m_Sort;   // bool
	public:

		CBlockPar(bool sort = true, CHeap* heap = NULL);
		~CBlockPar();

		void Clear(void);

		void CopyFrom(CBlockPar& bp);

	private:
		CBlockParUnit* UnitAdd(void);
        void UnitDel(CBlockParUnit* el);
		CBlockParUnit* UnitGet(const wchar* path, int path_len = -1);

		int ArrayFind(const wchar* name, int namelen) const; // -1-не найден   >=0-Первый юнит с этим названием
		int ArrayFind(const CWStr& name) const			{ return ArrayFind(name.Get(),name.GetLen()); }
		int ArrayFindInsertIndex(CBlockParUnit* ael); // А также инициализирует ael->m_Fast*
        void ArrayAdd(CBlockParUnit* el);
        void ArrayDel(CBlockParUnit* el);
	public:

		//////////////////////////////////////////////////////////////
        CBlockParUnit* ParAdd(const wchar* name, int namelen, const wchar* zn, int znlen);
		CBlockParUnit* ParAdd(const CWStr& name, const CWStr& zn)	{return ParAdd(name.Get(), name.GetLen(), zn.Get(), zn.GetLen());}
		CBlockParUnit* ParAdd(const CWStr& name, const wchar* zn)	{return ParAdd(name.Get(), name.GetLen(), zn, WStrLen(zn));}
		CBlockParUnit* ParAdd(const wchar* name, const CWStr& zn)	{return ParAdd(name, WStrLen(name), zn.Get(), zn.GetLen());}
		CBlockParUnit* ParAdd(const wchar* name, const wchar* zn)	{return ParAdd(name, WStrLen(name), zn, WStrLen(zn));}

		void ParReplace(int index, const wchar* name, int namelen, const wchar* zn, int znlen);
		void ParReplace(int index, const CWStr& name, const CWStr& zn)	{return ParReplace(index, name.Get(), name.GetLen(), zn.Get(), zn.GetLen());}
		void ParReplace(int index, const CWStr& name, const wchar* zn)	{return ParReplace(index, name.Get(), name.GetLen(), zn, WStrLen(zn));}
		void ParReplace(int index, const wchar* name, const CWStr& zn)	{return ParReplace(index, name, WStrLen(name), zn.Get(), zn.GetLen());}
		void ParReplace(int index, const wchar* name, const wchar* zn)	{return ParReplace(index, name, WStrLen(name), zn, WStrLen(zn));}

		bool ParSetNE(const wchar* name, int namelen, const wchar* zn, int znlen);
		void ParSet(const CWStr& name, const CWStr& zn)										{ if(!ParSetNE(name.Get(),name.GetLen(),zn.Get(),zn.GetLen())) ERROR_E; }
		void ParSet(const CWStr& name, const wchar* zn)										{ if(!ParSetNE(name.Get(),name.GetLen(),zn,WStrLen(zn))) ERROR_E; }
		void ParSet(const wchar* name, const CWStr& zn)										{ if(!ParSetNE(name,WStrLen(name),zn.Get(),zn.GetLen())) ERROR_E; }
		void ParSet(const wchar* name, const wchar* zn)										{ if(!ParSetNE(name,WStrLen(name),zn,WStrLen(zn))) ERROR_E; }

		void ParSetAdd(const CWStr& name, const CWStr& zn)						    { if(!ParSetNE(name.Get(),name.GetLen(),zn.Get(),zn.GetLen())) ParAdd(name,zn); }
		void ParSetAdd(const CWStr& name, const wchar* zn)						    { if(!ParSetNE(name.Get(),name.GetLen(),zn,WStrLen(zn))) ParAdd(name,zn); }
		void ParSetAdd(const wchar* name, const wchar* zn)						    { if(!ParSetNE(name,WStrLen(name),zn,WStrLen(zn))) ParAdd(name,zn); }
		void ParSetAdd(const wchar* name, int namelen, const wchar* zn, int znlen)		{ if(!ParSetNE(name,namelen,zn,znlen)) ParAdd(name,zn); }

        bool ParDeleteNE(const wchar* name, int namelen);
		void ParDelete(const CWStr& name)															{ if(!ParDeleteNE(name.Get(),name.GetLen())) ERROR_E; }
		void ParDelete(const wchar* name)															{ if(!ParDeleteNE(name,WStrLen(name))) ERROR_E; }
        void ParDelete(int no);

		const CWStr* ParGetNE_(const wchar* name, int namelen, int index) const;
		const CWStr& ParGet(const CWStr& name, int index = 0) const									{ const CWStr* str=ParGetNE_(name.Get(),name.GetLen(), index); if(str==NULL) ERROR_S2(L"Not found: ",name.Get()); return *str; }
		const CWStr& ParGet(const wchar* name, int index = 0) const									{ const CWStr* str=ParGetNE_(name,WStrLen(name),index); if(str==NULL) ERROR_S2(L"Not found: ",name); return *str; }
		CWStr ParGetNE(const CWStr& name, int index = 0) const
		{
			const CWStr* str = ParGetNE_(name.Get(), name.GetLen(), index);
			if(str != NULL) return *str;
			else return CWStr();
		}
		CWStr ParGetNE(const wchar* name, int index = 0) const
		{
			const CWStr* str = ParGetNE_(name, WStrLen(name), index);
			if(str != NULL) return *str;
			else return CWStr();
		}

		void Par(const CWStr& name, const CWStr& zn)														{ ParSetAdd((const CWStr &)name,(const CWStr &)zn); }
		void Par(const CWStr& name, const wchar* zn)														{ ParSetAdd((const CWStr &)name,zn); }
		void Par(const wchar* name, const CWStr& zn)														{ ParSetAdd(name,WStrLen(name),zn,zn.GetLen()); }
		void Par(const wchar* name, const wchar* zn)														{ ParSetAdd(name,WStrLen(name),zn, WStrLen(zn)); }
		const CWStr& Par(const CWStr& name)																{ return ParGet(name); }
		const CWStr& Par(const wchar* name)																{ return ParGet(name); }
		CWStr ParNE(const CWStr& name)																{ return ParGetNE(name); }
		CWStr ParNE(const wchar* name)																{ return ParGetNE(name); }

		int ParCount(void) const																	{ return m_CntPar; }
        int ParCount(const wchar* name, int namelen) const;
		int ParCount(const CWStr& name) const									{ return ParCount(name.Get(),name.GetLen()); }
		int ParCount(const wchar* name) const									{ return ParCount(name,WStrLen(name)); }

        const CWStr& ParGet(int no) const;
		void ParSet(int no, const wchar* zn, int znlen);
		void ParSet(int no, const wchar* zn)															{ ParSet(no,zn,WStrLen(zn)); }
		void ParSet(int no, const CWStr& zn)															{ ParSet(no,zn.Get(),zn.GetLen()); }
        const CWStr& ParGetName(int no) const;
		const CWStr& ParGetNameNE(int no) const;

		//////////////////////////////////////////////////////////////
		CBlockPar* BlockAdd(const wchar* name, int namelen);
		CBlockPar* BlockAdd(const CWStr& name) { return BlockAdd(name.Get(), name.GetLen()); }
		CBlockPar* BlockAdd(const wchar* name) { return BlockAdd(name, WStrLen(name)); }

		CBlockPar* BlockGetNE(const wchar* name, int namelen);
		CBlockPar* BlockGetNE(const CWStr& name) { return BlockGetNE(name.Get(), name.GetLen()); }
		CBlockPar* BlockGetNE(const wchar* name) { return BlockGetNE(name, WStrLen(name)); }

		CBlockPar* BlockGet(const CWStr& name) { CBlockPar* bp = BlockGetNE(name.Get(), name.GetLen()); if (!bp) ERROR_S2(L"Block not found: ", name.Get()); return bp; }
		CBlockPar* BlockGet(const wchar* name) { CBlockPar* bp = BlockGetNE(name, WStrLen(name)); if (!bp) ERROR_S2(L"Block not found: ", name); return bp; }

		CBlockPar* BlockGetAdd(const CWStr& name) { CBlockPar* bp = BlockGetNE(name.Get(), name.GetLen()); if (bp) return bp; else return BlockAdd(name.Get(), name.GetLen()); }
		CBlockPar* BlockGetAdd(const wchar* name) { CBlockPar* bp = BlockGetNE(name, WStrLen(name)); if (bp) return bp; else return BlockAdd(name, WStrLen(name)); }

		bool BlockDeleteNE(const wchar* name, int namelen);
		void BlockDelete(const CWStr& name)															{ if(!BlockDeleteNE(name.Get(),name.GetLen())) ERROR_E; }
		void BlockDelete(const wchar* name)															{ if(!BlockDeleteNE(name,WStrLen(name))) ERROR_E; }
        void BlockDelete(int no);

		int BlockCount(void) const																	{ return m_CntBlock; }
        int BlockCount(const wchar* name, int namelen) const;
		int BlockCount(const CWStr& name)	const													{ return BlockCount(name.Get(),name.GetLen()); }
		int BlockCount(const wchar* name)	const													{ return BlockCount(name,WStrLen(name)); }

		CBlockPar* BlockGet(int no);
        const CBlockPar* BlockGet(int no) const;
        const CWStr& BlockGetName(int no) const;

		void BlockMerge(CBlockPar& bp);

		//////////////////////////////////////////////////////////////
		const CWStr & ParPathGet(const wchar* path, int pathlen);
		const CWStr & ParPathGet(const CWStr& path)														{ return ParPathGet(path.Get(),path.GetLen()); }
		const CWStr & ParPathGet(const wchar* path)														{ return ParPathGet(path,WStrLen(path)); }

		void ParPathAdd(const wchar* path, int pathlen, const wchar* zn, int znlen);
		void ParPathAdd(const CWStr& path, const CWStr& zn)												{ ParPathAdd(path.Get(),path.GetLen(),zn.Get(),zn.GetLen()); }
		void ParPathAdd(const CWStr& path, const wchar* zn)												{ ParPathAdd(path.Get(),path.GetLen(),zn,WStrLen(zn)); }
		void ParPathAdd(const wchar* path, const CWStr& zn)												{ ParPathAdd(path,WStrLen(path),zn.Get(),zn.GetLen()); }
		void ParPathAdd(const wchar* path, const wchar* zn)												{ ParPathAdd(path,WStrLen(path),zn,WStrLen(zn)); }

		void ParPathSet(const wchar* path, int pathlen, const wchar* zn, int znlen);
		void ParPathSet(const CWStr& path, const CWStr& zn)												{ ParPathSet(path.Get(),path.GetLen(),zn.Get(),zn.GetLen()); }
		void ParPathSet(const CWStr& path, const wchar* zn)												{ ParPathSet(path.Get(),path.GetLen(),zn,WStrLen(zn)); }
		void ParPathSet(const wchar* path, const CWStr& zn)												{ ParPathSet(path,WStrLen(path),zn.Get(),zn.GetLen()); }
		void ParPathSet(const wchar* path, const wchar* zn)												{ ParPathSet(path,WStrLen(path),zn,WStrLen(zn)); }

		void ParPathSetAdd(const wchar* path, int pathlen, const wchar* zn, int znlen);
		void ParPathSetAdd(const CWStr& path, const CWStr& zn)											{ ParPathSetAdd(path.Get(),path.GetLen(),zn.Get(),zn.GetLen()); }
		void ParPathSetAdd(const CWStr& path, const wchar* zn)											{ ParPathSetAdd(path.Get(),path.GetLen(),zn,WStrLen(zn)); }
		void ParPathSetAdd(const wchar* path, const CWStr& zn)											{ ParPathSetAdd(path,WStrLen(path),zn.Get(),zn.GetLen()); }
		void ParPathSetAdd(const wchar* path, const wchar* zn)											{ ParPathSetAdd(path,WStrLen(path),zn,WStrLen(zn)); }

		void ParPathDelete(const wchar* path, int pathlen);
		void ParPathDelete(const CWStr& path)														{  ParPathDelete(path.Get(),path.GetLen()); }
		void ParPathDelete(const wchar* path)														{  ParPathDelete(path,WStrLen(path)); }

		int ParPathCount(const wchar* path, int pathlen);

		//////////////////////////////////////////////////////////////
		CBlockPar* BlockPathGet(const wchar* path, int pathlen);
		CBlockPar* BlockPathGet(const CWStr& path)													{  return BlockPathGet(path.Get(),path.GetLen()); }
		CBlockPar* BlockPathGet(const wchar* path)													{  return BlockPathGet(path,WStrLen(path)); }

		CBlockPar* BlockPathAdd(const wchar* path, int pathlen);
		CBlockPar* BlockPathAdd(const CWStr& path)													{  return BlockPathAdd(path.Get(),path.GetLen()); }
		CBlockPar* BlockPathAdd(const wchar* path)													{  return BlockPathAdd(path,WStrLen(path)); }

		CBlockPar* BlockPathGetAdd(const wchar* path, int pathlen);
		CBlockPar* BlockPathGetAdd(const CWStr& path)												{ return BlockPathGetAdd(path.Get(),path.GetLen()); }
		CBlockPar* BlockPathGetAdd(const wchar* path)												{ return BlockPathGetAdd(path,WStrLen(path)); }

		CBlockPar* BlockPath(const CWStr& path)														{ return BlockPathGet(path.Get(),path.GetLen()); }
		CBlockPar* BlockPath(const wchar* path)														{ return BlockPathGet(path,WStrLen(path)); }

		//////////////////////////////////////////////////////////////
		int AllCount(void)																		{  return m_Cnt; }
		int AllGetType(int no);
		CBlockPar * AllGetBlock(int no);
		const CWStr & AllGetPar(int no);
		const CWStr & AllGetName(int no);

		//////////////////////////////////////////////////////////////
		int LoadFromText(const wchar * text, int textlen);
		int LoadFromText(const CWStr & text)													{ return LoadFromText(text.Get(), text.GetLen()); }
		int LoadFromText(const wchar * text)													{ return LoadFromText(text, WStrLen(text)); }

		void LoadFromTextFile(const wchar * filename, int filenamelen);
		void LoadFromTextFile(const CWStr & filename)											{ LoadFromTextFile(filename.Get(), filename.GetLen()); }
		void LoadFromTextFile(const wchar * filename)											{ LoadFromTextFile(filename, WStrLen(filename)); }

		void SaveInText(CBuf & buf, bool ansi=false, int level=0);

		void SaveInTextFile(const wchar * filename, int filenamelen, bool ansi=false);
		void SaveInTextFile(const CWStr & filename, bool ansi=false)							{ SaveInTextFile(filename.Get(), filename.GetLen(),ansi); }
		void SaveInTextFile(const wchar * filename, bool ansi=false)							{ SaveInTextFile(filename, WStrLen(filename), ansi); }
};

}