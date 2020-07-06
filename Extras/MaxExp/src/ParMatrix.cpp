#include "stdafx.h"
#include "main.hpp"
#include "resource.hpp"
#include "iparamm2.h"
#include "modstack.h"

#define PARMATRIX_CLASS_ID Class_ID(0x31764fc2, 0x761b1e85);

#define PBLOCK_REF	0

class ParMatrixMod : public Modifier {	
    public:
        IParamBlock2 *  m_PBlock;
        Base::CWStr     m_Group;
    public:
        ParMatrixMod();

		// From Animatable
        void DeleteThis()               { delete this; }
		void GetClassName(TSTR& s)      { s = "EG.Exp.Matrix"; }
		virtual Class_ID ClassID()      { return PARMATRIX_CLASS_ID; }		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR * GetObjectName()         { return "EG.Exp.Matrix"; }
		void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);		

        // From modifier
		ChannelMask ChannelsUsed()      { return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE|PART_TEXMAP|PART_VERTCOLOR; }
		ChannelMask ChannelsChanged()   { return PART_GEOM|PART_TOPO|PART_SELECT|PART_TEXMAP|PART_VERTCOLOR; }
		Class_ID InputType()            { return defObjectClassID; }
        void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node) {}
        Interval LocalValidity(TimeValue t) { return GetValidity(t); }
        Interval GetValidity(TimeValue t)   { return FOREVER; }

        // From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 

		int NumRefs()                                   { return 1; }
		RefTargetHandle GetReference(int i)             { if (i == 0) return m_PBlock;  else return NULL; }
        void SetReference(int i, RefTargetHandle rtarg) { if(i==PBLOCK_REF) m_PBlock = (IParamBlock2*)rtarg; }

		int NumSubs()                                   { return 0; }
		Animatable* SubAnim(int i)                      { return NULL; }
		TSTR SubAnimName(int i)                         { return _T(""); }

        int	NumParamBlocks()                            { return 1; }
		IParamBlock2* GetParamBlock(int i)              { if (i == 0) return m_PBlock;  else return NULL; }
		IParamBlock2* GetParamBlockByID(BlockID id)     { if (m_PBlock->ID() == id) return m_PBlock; else return  NULL; }

        RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message) {return REF_SUCCEED;}

        IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
};

class ParMatrixModClassDesc:public ClassDesc2 {
	public:
	    int 			IsPublic()                      { return 1; }
	    void *			Create(BOOL loading = FALSE)    { return new ParMatrixMod; }
	    const TCHAR *	ClassName()                     { return "EG.Exp.Matrix"; }
	    SClass_ID		SuperClassID()                  { return OSM_CLASS_ID; }
	    Class_ID		ClassID()                       { return PARMATRIX_CLASS_ID; }
	    const TCHAR* 	Category()                      { return _T("EG"); }

        const TCHAR*	InternalName()                  { return _T("EG.Exp.Matrix"); }
    	HINSTANCE		HInstance()                     { return hInstance; }
};

static ParMatrixModClassDesc g_ParMatrixModClassDesc;
extern ClassDesc* GetParMatrixModClassDesc()               { return &g_ParMatrixModClassDesc; }

enum { egpb_Export,egpb_Group,egpb_Name,egpb_Id };

class ParMatrixDlgProc : public ParamMap2UserDlgProc {
    public:
        ParMatrixMod *ob;

        ParMatrixDlgProc(ParMatrixMod *o) { ob = o; }
        BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
            switch(msg) {
                case WM_INITDIALOG: {
                    
                    HWND win = GetDlgItem(hWnd,IDC_PAROBJ_GROUP);
                    SendMessage(win,CB_RESETCONTENT,0,0);
                    SendMessage(win,CB_ADDSTRING,0,(LPARAM) (LPCTSTR) (""));
                    int cnt=ExportGroupCount();
                    int no;
                    for(int i=0;i<cnt;i++) {
                        Base::CWStr tstr;
                        if(ExportGroupGetName(tstr,i,no)) {
                            SendMessage(win,CB_ADDSTRING,0,(LPARAM) (LPCTSTR) (Base::CStr(tstr).Get()));
                        }
                    }
        			SendMessage(win,CB_SETCURSEL,(WPARAM)-1,0);
                    if(!ob->m_Group.IsEmpty()) SetWindowText(win,Base::CStr(ob->m_Group).Get());

                    return TRUE;
                }
                case WM_COMMAND: {
                    if(HIWORD(wParam)==CBN_SETFOCUS && LOWORD(wParam)==IDC_PAROBJ_GROUP) {
                        DisableAccelerators();
                    } else if(HIWORD(wParam)==CBN_KILLFOCUS && LOWORD(wParam)==IDC_PAROBJ_GROUP) {
                        EnableAccelerators();

                    } else if(HIWORD(wParam)==CBN_EDITCHANGE && LOWORD(wParam)==IDC_PAROBJ_GROUP) {
                        
                        HWND win = GetDlgItem(hWnd,IDC_PAROBJ_GROUP);
                        int cnt=SendMessage(win,WM_GETTEXTLENGTH,0,0);
                        Base::CStr tstr;
                        if(cnt>0) {
                            tstr.SetLen(cnt);
                            SendMessage(win,WM_GETTEXT,cnt+1,(LPARAM)tstr.GetBuf());
                        }
                        ob->m_Group=Base::CWStr(tstr);

                    } else if(HIWORD(wParam)==CBN_SELCHANGE && LOWORD(wParam)==IDC_PAROBJ_GROUP) {

                        HWND win = GetDlgItem(hWnd,IDC_PAROBJ_GROUP);
                        int cur=SendMessage(win,CB_GETCURSEL,0,0);
                        int cnt=SendMessage(win,CB_GETLBTEXTLEN,cur,0);
                        Base::CStr tstr;
                        if(cnt>0) {
                            tstr.SetLen(cnt);
                            SendMessage(win,CB_GETLBTEXT,cur,(LPARAM)tstr.GetBuf());
                        }
                        ob->m_Group=Base::CWStr(tstr);
                        

                    }
                    return TRUE;
                }
            }
            return FALSE;
        }
        void DeleteThis() {delete this;}
};

static ParamBlockDesc2 descP ( 0, _T("Parameters"),  0, &g_ParMatrixModClassDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	IDD_PARMATRIX, IDS_PARGROUP_PAR, 0, 0, NULL,

	egpb_Export, 	"Export",	TYPE_INT, 	0,  IDS_PARGROUP_EXPORT,
        p_ui,		TYPE_SINGLECHEKBOX, IDC_PAROBJ_EXPORT,
        end,

    egpb_Name, 	    "Name",		TYPE_STRING, 	0,  IDS_PARMATRIX_NAME,
        p_ui,		TYPE_EDITBOX, IDC_PARMATRIX_NAME,
        end,

    egpb_Id,        "MId",     TYPE_INT,              0, IDS_PARMATRIX_ID,
		p_default,		0,
		p_range,		0,2000000000,
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_PARMATRIX_ID_EDIT, IDC_PARMATRIX_ID_SPINNER, 1.0f,
        end,

    end
);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
ParMatrixMod::ParMatrixMod():m_Group()
{
    m_PBlock=NULL;

	GetParMatrixModClassDesc()->MakeAutoParamBlocks(this);
    m_PBlock->SetValue(egpb_Export,0,1,0);
}

RefTargetHandle ParMatrixMod::Clone(RemapDir& remap)
{
    ParMatrixMod *mod = new ParMatrixMod();
	mod->ReplaceReference(PBLOCK_REF,m_PBlock->Clone(remap));	
    mod->m_Group=m_Group;
    BaseClone(this, mod, remap);
    return mod;
}

void ParMatrixMod::BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev)
{
	g_ParMatrixModClassDesc.BeginEditParams(ip, this, flags, prev);
    g_ParMatrixModClassDesc.SetUserDlgProc(&descP,new ParMatrixDlgProc(this));
}

void ParMatrixMod::EndEditParams(IObjParam *ip,ULONG flags,Animatable *next)
{
    g_ParMatrixModClassDesc.EndEditParams(ip, this, flags, next);
}

#define FILEEXPORT_CHUNK_1 0x1000

IOResult ParMatrixMod::Load(ILoad *iload)
{
	wchar_t * buf;
	IOResult res = Modifier::Load(iload);
    if(res!=IO_OK) return res;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
		    case FILEEXPORT_CHUNK_1: {
				res = iload->ReadWStringChunk (&buf);
                if(res!=IO_OK) break;
				m_Group = buf;
                break;
            }
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}

	return IO_OK;
}

IOResult ParMatrixMod::Save(ISave *isave)
{
	IOResult res = Modifier::Save(isave);
    if(res!=IO_OK) return res;

	isave->BeginChunk(FILEEXPORT_CHUNK_1);
    if(m_Group.IsEmpty()) isave->WriteWString(L"");
	else isave->WriteWString(m_Group.GetBuf());
	isave->EndChunk();

	return IO_OK;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
extern bool ExportMatrixGetProp(INode *node, const Base::CWStr & group,
                                int & id, Base::CWStr & name)
{
    Object * obj = node->GetObjectRef();
    if(obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)) {
        IDerivedObject* dobj = (IDerivedObject*)obj;
        int cnt = dobj->NumModifiers();
        for (int i=0; i<cnt; i++) {
            Modifier * mod = dobj->GetModifier(i);
            if(!mod) continue;
            int idok=mod->ClassID()==PARMATRIX_CLASS_ID;
            if(!idok) continue;

            int iv;
            TCHAR * ach;

            if(!group.IsEmpty() && !((ParMatrixMod *)mod)->m_Group.IsEmpty() && (group!=((ParMatrixMod *)mod)->m_Group)) continue;

            ((ParMatrixMod *)mod)->m_PBlock->GetValue(egpb_Export,0,iv,FOREVER);
            if(!iv) continue;

            ((ParMatrixMod *)mod)->m_PBlock->GetValue(egpb_Id,0,id,FOREVER);

            ((ParMatrixMod *)mod)->m_PBlock->GetValue(egpb_Name,0,ach,FOREVER);
            name=Base::CWStr(Base::CStr(ach));

            return true;
        }
    }

    return false;
}
