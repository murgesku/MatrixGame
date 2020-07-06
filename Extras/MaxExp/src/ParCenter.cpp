#include "stdafx.h"
#include "main.hpp"
#include "resource.hpp"
#include "iparamm2.h"
#include "modstack.h"

#define PARCENTER_CLASS_ID Class_ID(0x5506750d, 0x2995601a);

#define PBLOCK_REF	0

class ParCenterMod : public Modifier {	
    public:
        IParamBlock2 *  m_PBlock;
        Base::CWStr     m_Group;
    public:
        ParCenterMod();

		// From Animatable
        void DeleteThis()               { delete this; }
		void GetClassName(TSTR& s)      { s = "EG.Exp.Center"; }
		virtual Class_ID ClassID()      { return PARCENTER_CLASS_ID; }		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR * GetObjectName()         { return "EG.Exp.Center"; }
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

class ParCenterModClassDesc:public ClassDesc2 {
	public:
	    int 			IsPublic()                      { return 1; }
	    void *			Create(BOOL loading = FALSE)    { return new ParCenterMod; }
	    const TCHAR *	ClassName()                     { return "EG.Exp.Center"; }
	    SClass_ID		SuperClassID()                  { return OSM_CLASS_ID; }
	    Class_ID		ClassID()                       { return PARCENTER_CLASS_ID; }
	    const TCHAR* 	Category()                      { return _T("EG"); }

        const TCHAR*	InternalName()                  { return _T("EG.Exp.Center"); }
    	HINSTANCE		HInstance()                     { return hInstance; }
};

static ParCenterModClassDesc g_ParCenterModClassDesc;
extern ClassDesc* GetParCenterModClassDesc()               { return &g_ParCenterModClassDesc; }

enum { egpb_Export,egpb_Group,egpb_MaxSizeX,egpb_MaxSizeY,egpb_MaxSizeZ,egpb_ScaleX,egpb_ScaleY,egpb_ScaleZ };

class ParCenterDlgProc : public ParamMap2UserDlgProc {
    public:
        ParCenterMod *ob;

        ParCenterDlgProc(ParCenterMod *o) { ob = o; }
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

static ParamBlockDesc2 descP ( 0, _T("Parameters"),  0, &g_ParCenterModClassDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	IDD_PARCENTER, IDS_PARGROUP_PAR, 0, 0, NULL,

	egpb_Export, 	"Export",	TYPE_INT, 	0,  IDS_PARGROUP_EXPORT,
        p_ui,		TYPE_SINGLECHEKBOX, IDC_PAROBJ_EXPORT,
        end,

    egpb_MaxSizeX, "MaxSizeX",TYPE_FLOAT,0, IDS_PARCENTER_MAXSIZEX,
		p_default,		0.0f,
		p_range,		0.0f,1000000000.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PARCENTER_MAXSIZE_X_EDIT, IDC_PARCENTER_MAXSIZE_X_SPINNER, 0.01f,
        end,

    egpb_MaxSizeY, "MaxSizeY",TYPE_FLOAT,0, IDS_PARCENTER_MAXSIZEY,
		p_default,		0.0f,
		p_range,		0.0f,1000000000.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PARCENTER_MAXSIZE_Y_EDIT, IDC_PARCENTER_MAXSIZE_Y_SPINNER, 0.01f,
        end,

    egpb_MaxSizeZ, "MaxSizeZ",TYPE_FLOAT,0, IDS_PARCENTER_MAXSIZEZ,
		p_default,		0.0f,
		p_range,		0.0f,1000000000.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PARCENTER_MAXSIZE_Z_EDIT, IDC_PARCENTER_MAXSIZE_Z_SPINNER, 0.01f,
        end,

    egpb_ScaleX, "ScaleX",TYPE_FLOAT,0, IDS_PARCENTER_SCALEX,
		p_default,		1.0f,
		p_range,		-1000000000.0f,1000000000.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PARCENTER_SCALE_X_EDIT, IDC_PARCENTER_SCALE_X_SPINNER, 0.01f,
        end,

    egpb_ScaleY, "ScaleY",TYPE_FLOAT,0, IDS_PARCENTER_SCALEY,
		p_default,		1.0f,
		p_range,		-1000000000.0f,1000000000.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PARCENTER_SCALE_Y_EDIT, IDC_PARCENTER_SCALE_Y_SPINNER, 0.01f,
        end,

    egpb_ScaleZ, "ScaleZ",TYPE_FLOAT,0, IDS_PARCENTER_SCALEZ,
		p_default,		1.0f,
		p_range,		-1000000000.0f,1000000000.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PARCENTER_SCALE_Z_EDIT, IDC_PARCENTER_SCALE_Z_SPINNER, 0.01f,
        end,

    end
);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
ParCenterMod::ParCenterMod():m_Group()
{
    m_PBlock=NULL;

	GetParCenterModClassDesc()->MakeAutoParamBlocks(this);
    m_PBlock->SetValue(egpb_Export,0,1,0);
}

RefTargetHandle ParCenterMod::Clone(RemapDir& remap)
{
    ParCenterMod *mod = new ParCenterMod();
	mod->ReplaceReference(PBLOCK_REF,m_PBlock->Clone(remap));	
    mod->m_Group=m_Group;
    BaseClone(this, mod, remap);
    return mod;
}

void ParCenterMod::BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev)
{
	g_ParCenterModClassDesc.BeginEditParams(ip, this, flags, prev);
    g_ParCenterModClassDesc.SetUserDlgProc(&descP,new ParCenterDlgProc(this));
}

void ParCenterMod::EndEditParams(IObjParam *ip,ULONG flags,Animatable *next)
{
    g_ParCenterModClassDesc.EndEditParams(ip, this, flags, next);
}

#define FILEEXPORT_CHUNK_1 0x1000

IOResult ParCenterMod::Load(ILoad *iload)
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

IOResult ParCenterMod::Save(ISave *isave)
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
extern bool ExportCenterGetProp(const Base::CWStr & group,TimeValue timev,INode *node,
                                double & maxsizex,double & maxsizey,double & maxsizez,float & scalex,float & scaley,float & scalez,Matrix3 & center)
{
    if(node==NULL) node=GetCOREInterface()->GetRootNode();

    Object * obj = node->GetObjectRef();
    if(obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)) {
        IDerivedObject* dobj = (IDerivedObject*)obj;
        int cnt = dobj->NumModifiers();
        for (int i=0; i<cnt; i++) {
            Modifier * mod = dobj->GetModifier(i);
            if(!mod) continue;
            int idok=mod->ClassID()==g_ParCenterModClassDesc.ClassID();
            if(!idok) continue;

            int iv;
            float fv;

            if(!group.IsEmpty() && !((ParCenterMod *)mod)->m_Group.IsEmpty() && (group!=((ParCenterMod *)mod)->m_Group)) continue;

            ((ParCenterMod *)mod)->m_PBlock->GetValue(egpb_Export,0,iv,FOREVER);
            if(!iv) continue;

            ((ParCenterMod *)mod)->m_PBlock->GetValue(egpb_MaxSizeX,0,fv,FOREVER); maxsizex=fv;
            ((ParCenterMod *)mod)->m_PBlock->GetValue(egpb_MaxSizeY,0,fv,FOREVER); maxsizey=fv;
            ((ParCenterMod *)mod)->m_PBlock->GetValue(egpb_MaxSizeZ,0,fv,FOREVER); maxsizez=fv;

            ((ParCenterMod *)mod)->m_PBlock->GetValue(egpb_ScaleX,0,fv,FOREVER); scalex=fv;
            ((ParCenterMod *)mod)->m_PBlock->GetValue(egpb_ScaleY,0,fv,FOREVER); scaley=fv;
            ((ParCenterMod *)mod)->m_PBlock->GetValue(egpb_ScaleZ,0,fv,FOREVER); scalez=fv;

            center=node->GetObjTMAfterWSM(timev);
//            center.Orthogonalize();
//            center.NoScale();
            center.Invert();

            return true;
        }
    }

    for (int c = 0; c < node->NumberOfChildren(); c++) {
        if(ExportCenterGetProp(group,timev,node->GetChildNode(c),maxsizex,maxsizey,maxsizez,scalex,scaley,scalez,center)) return true;
	}
    return false;
}
