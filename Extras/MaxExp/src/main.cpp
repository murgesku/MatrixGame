#include "stdafx.h"
#include "main.hpp"
#include "resource.hpp"

HINSTANCE hInstance;
int controlsInit = FALSE;

static BOOL showPrompts;
static BOOL exportSelected;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
	hInstance = hinstDLL;

	// Initialize the custom controls. This should be done only once.
	if (!controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitCommonControls();
	}
	
	return (TRUE);
}


__declspec( dllexport ) const TCHAR* LibDescription() 
{
	return _T("EG file exporter");
}

__declspec( dllexport ) int LibNumberClasses() 
{
	return 4;
}


__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
	switch(i) {
//		case 0: return GetEGExpDesc();
        case 0: return GetParObjModClassDesc();
        case 1: return GetParGroupModClassDesc();
        case 2: return GetParCenterModClassDesc();
        case 3: return GetParMatrixModClassDesc();
		default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion() 
{
	return VERSION_3DSMAX;
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

class EGExpClassDesc:public ClassDesc {
public:
	int				IsPublic() { return 1; }
	void*			Create(BOOL loading = FALSE) { return new EGExp; } 
	const TCHAR*	ClassName() { return _T("EGExp"); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; } 
	Class_ID		ClassID() { return Class_ID(0x6f5c1b57, 0x678221e2); }
	const TCHAR*	Category() { return _T("EG"); }
};

static EGExpClassDesc EGExpDesc;

ClassDesc* GetEGExpDesc()
{
	return &EGExpDesc;
}

TCHAR * GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance) return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;

	return NULL;
}

EGExp::EGExp():m_Cfg(),m_CfgFromScene(),m_FileName(),m_Log()
{
	// These are the default values that will be active when 
	// the exporter is ran the first time.
	// After the first session these options are sticky.
	bIncludeMesh = TRUE;
	bIncludeMtl =  TRUE;
	bIncludeMeshAnim =  FALSE;
	bIncludeCamLightAnim = FALSE;
	bIncludeIKJoints = FALSE;
	bIncludeNormals  =  FALSE;
	bIncludeTextureCoords = FALSE;
	bIncludeVertexColors = FALSE;
	bIncludeObjGeom = TRUE;
	bIncludeObjShape = TRUE;
	bIncludeObjCamera = TRUE;
	bIncludeObjLight = TRUE;
	bIncludeObjHelper = TRUE;
	bAlwaysSample = FALSE;
	nKeyFrameStep = 5;
	nMeshFrameStep = 5;
	nPrecision = 4;
	nStaticFrame = 0;

	m_GroupFirst=NULL;
	m_GroupLast=NULL;
}

EGExp::~EGExp()
{
	GroupClear();
}

void EGExp::GroupClear()
{
	while(m_GroupFirst!=NULL) GroupDelete(m_GroupLast);
}

void EGExp::GroupDelete(CGroup * el)
{
	LIST_DEL(el,m_GroupFirst,m_GroupLast,m_Prev,m_Next);
	HDelete(CGroup,el,NULL);
}

CGroup * EGExp::GroupFind(Base::CPoint & p1)
{
	CGroup * el=m_GroupLast;
	while(el!=NULL) {
		if(el->m_IdCnt==1 && el->m_Id[0]==p1) {
			if(el!=m_GroupLast) {
				LIST_DEL(el,m_GroupFirst,m_GroupLast,m_Prev,m_Next);
				LIST_ADD(el,m_GroupFirst,m_GroupLast,m_Prev,m_Next);
			}
			return el;
		}
		el=el->m_Prev;
	}
	return NULL;
}

CGroup * EGExp::GroupFind(Base::CPoint & p1, Base::CPoint & p2)
{
	CGroup * el=m_GroupLast;
	while(el!=NULL) {
		if(el->m_IdCnt==2 && el->m_Id[0]==p1 && el->m_Id[1]==p2) {
			if(el!=m_GroupLast) {
				LIST_DEL(el,m_GroupFirst,m_GroupLast,m_Prev,m_Next);
				LIST_ADD(el,m_GroupFirst,m_GroupLast,m_Prev,m_Next);
			}
			return el;
		}
		el=el->m_Prev;
	}
	return NULL;
}

CGroup * EGExp::GroupFind(Base::CPoint & p1, Base::CPoint & p2, Base::CPoint & p3)
{
	CGroup * el=m_GroupLast;
	while(el!=NULL) {
		if(el->m_IdCnt==3 && el->m_Id[0]==p1 && el->m_Id[1]==p2 && el->m_Id[2]==p3) {
			if(el!=m_GroupLast) {
				LIST_DEL(el,m_GroupFirst,m_GroupLast,m_Prev,m_Next);
				LIST_ADD(el,m_GroupFirst,m_GroupLast,m_Prev,m_Next);
			}
			return el;
		}
		el=el->m_Prev;
	}
	return NULL;
}

CGroup * EGExp::GroupAdd(Base::CPoint & p1)
{
	CGroup * el=HNew(NULL) CGroup;

	LIST_ADD(el,m_GroupFirst,m_GroupLast,m_Prev,m_Next);

	el->m_IdCnt=1;
	el->m_Id[0]=p1;

	return el;
}

CGroup * EGExp::GroupAdd(Base::CPoint & p1, Base::CPoint & p2)
{
	CGroup * el=HNew(NULL) CGroup;

	LIST_ADD(el,m_GroupFirst,m_GroupLast,m_Prev,m_Next);

	el->m_IdCnt=2;
	el->m_Id[0]=p1;
	el->m_Id[1]=p2;

	return el;
}

CGroup * EGExp::GroupAdd(Base::CPoint & p1, Base::CPoint & p2, Base::CPoint & p3)
{
	CGroup * el=HNew(NULL) CGroup;

	LIST_ADD(el,m_GroupFirst,m_GroupLast,m_Prev,m_Next);

	el->m_IdCnt=3;
	el->m_Id[0]=p1;
	el->m_Id[1]=p2;
	el->m_Id[2]=p3;

	return el;
}

void EGExp::GroupSortByNo()
{
	CGroup * un_first=m_GroupFirst;
	CGroup * un_last=m_GroupLast;
	m_GroupFirst=NULL;
	m_GroupLast=NULL;

	CGroup * gr,* gr2;

	while(un_first) {
		gr=un_first;
		int minn=gr->m_No;
		while(gr!=NULL) {
			if(gr->m_No<minn) minn=gr->m_No;
			gr=gr->m_Next;
		}

		gr=un_first;
		while(gr!=NULL) {
			gr2=gr;
			gr=gr->m_Next;
			if(gr2->m_No==minn) {
				LIST_DEL(gr2,un_first,un_last,m_Prev,m_Next);
				LIST_ADD(gr2,m_GroupFirst,m_GroupLast,m_Prev,m_Next);
			}
		}
	}
}

void EGExp::Log(Base::CWStr str)
{
	m_Log.WStrNZ(str.Get());
	m_Log.Dword(0x000a000d);
}

int EGExp::GroupMaterialFindAdd(Mtl * ml, int sm, int & twosided)
{
	CGroup * gr;
	if(ml==NULL) {
	} else if(ml->NumSubMtls()>0) {
		Mtl * ml2=ml->GetSubMtl(sm % ml->NumSubMtls());
		if(ml2!=NULL) ml=ml2;
	}

	if(ml && ml->ClassID()==Class_ID(DMTL_CLASS_ID, 0) && ((StdMat *)ml)->GetTwoSided()) twosided=1;
	else twosided=0;

	gr=GroupFind(Base::CPoint(gt_Material,int(ml)));
	if(gr!=NULL) {
		return gr->m_No;
	} else {
		gr=GroupAdd(Base::CPoint(gt_Material,int(ml)));
		gr->m_No=m_MaterialNextNo;
		m_MaterialNextNo++;
	}

	SMaterial outm; ZeroMemory(&outm,sizeof(SMaterial));
	Color cl;

	if(ml==NULL) {
	} else {
		cl=ml->GetAmbient(); outm.ar=cl.r; outm.ag=cl.g; outm.ab=cl.b;
		cl=ml->GetDiffuse(); outm.dr=cl.r; outm.dg=cl.g; outm.db=cl.b;
		cl=ml->GetSpecular(); outm.sr=cl.r; outm.sg=cl.g; outm.sb=cl.b;

//		if(!(m_Set->ParCount(L"TextureDefault") && m_Set->Par(L"TextureDefault").GetInt())) {
        if(!m_TextureDefault) {
			if(ml->ClassID()==Class_ID(DMTL_CLASS_ID, 0) && ID_DI<ml->NumSubTexmaps() && ml->GetSubTexmap(ID_DI)) {
				Texmap * tex = ml->GetSubTexmap(ID_DI);
				if(tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)) {
					TSTR mapName = ((BitmapTex *)tex)->GetMapName();
					Base::CWStr namestr(Base::CStr((char *)mapName));
					if(!namestr.IsEmpty()) {
						namestr=RenameByCfg(namestr.GetStrPar(namestr.GetCountPar(L"\\/")-1,L"\\/"));
//						int cnt=namestr.GetCountPar(L".");
//						if(cnt>1) namestr=namestr.GetStrPar(0,cnt-2,L".");
	
						if(namestr.GetLen()>31) namestr.SetLen(31);
						if(namestr.GetLen()>0) CopyMemory(&(outm.tex_diffuse),namestr.Get(),namestr.GetLen()*2);
					}
				}
			}
		}
	
	}

	gr->m_Buf.BufAdd(&outm,sizeof(SMaterial));

	return gr->m_No;
}

int EGExp::AddVertex(Base::CBuf & buf,SVertexNorTex & v)
{
	int cnt=buf.Len() / sizeof(SVertexNorTex);
	for(int i=0;i<cnt;i++) {
		SVertexNorTex & vc=*(((SVertexNorTex *)buf.Get())+i);
		if(((double(v.x)-double(vc.x))*(double(v.x)-double(vc.x))+(double(v.y)-double(vc.y))*(double(v.y)-double(vc.y))+(double(v.z)-double(vc.z))*(double(v.z)-double(vc.z)))>=0.000001) continue;
		if(((double(v.nx)-double(vc.nx))*(double(v.nx)-double(vc.nx))+(double(v.ny)-double(vc.ny))*(double(v.ny)-double(vc.ny))+(double(v.nz)-double(vc.nz))*(double(v.nz)-double(vc.nz)))>=0.000001) continue;
		if(((double(v.tu)-double(vc.tu))*(double(v.tu)-double(vc.tu))+(double(v.tv)-double(vc.tv))*(double(v.tv)-double(vc.tv)))>=0.000001) continue;
		return i;
	}
	buf.Pointer(buf.Len());
	buf.BufAdd(&v,sizeof(SVertexNorTex));
	return cnt;
}

int EGExp::ExtCount()
{
	return 1;
}

const TCHAR * EGExp::Ext(int n)
{
	switch(n) {
	case 0:
		return _T("vo");
	}
	return _T("");
}

const TCHAR * EGExp::LongDesc()
{
	return _T("3ds max EG Scene Exporter");
}

const TCHAR * EGExp::ShortDesc()
{
	return _T("EG Scene Export");
}

const TCHAR * EGExp::AuthorName() 
{
	return _T("");
}

const TCHAR * EGExp::CopyrightMessage() 
{
	return _T("Copyright (c) 2003 ElementalGames");
}

const TCHAR * EGExp::OtherMessage1() 
{
	return _T("");
}

const TCHAR * EGExp::OtherMessage2() 
{
	return _T("");
}

unsigned int EGExp::Version()
{
	return 100;
}

static INT_PTR CALLBACK AboutBoxDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		CenterWindow(hWnd, GetParent(hWnd)); 
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hWnd, 1);
			break;
		}
		break;
		default:
			return FALSE;
	}
	return TRUE;
}       

void EGExp::ShowAbout(HWND hWnd)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutBoxDlgProc, 0);
}

LRESULT APIENTRY EditSubclassProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam) 
{ 
	EGExp *exp = (EGExp*)GetWindowLongPtr(hwnd,GWLP_USERDATA); 

	if (uMsg == WM_GETDLGCODE) return DLGC_WANTALLKEYS|DLGC_WANTARROWS|DLGC_WANTCHARS|DLGC_WANTMESSAGE|DLGC_WANTTAB; 
 
	return CallWindowProc(exp->m_OldProc, hwnd, uMsg, wParam, lParam); 
} 

void SaveCfgFromDialog(EGExp * exp, HWND hWnd)
{
	Base::CWStr str;

	HWND cw=GetDlgItem(hWnd, IDC_CFG);
	GETTEXTLENGTHEX gtl; gtl.flags=GTL_DEFAULT|GTL_USECRLF|GTL_NUMBYTES; gtl.codepage=1200;
	int len=SendMessage(cw,EM_GETTEXTLENGTHEX,DWORD(&gtl),0);
	if(len!=E_INVALIDARG) {
		if((len>0) && !(len & 1)) {
			GETTEXTEX gte; gte.cb=len+2; gte.codepage=1200; gte.flags=GT_USECRLF; gte.lpDefaultChar=NULL; gte.lpUsedDefChar=NULL;
			str.SetLen(len >> 1);
			SendMessage(cw,EM_GETTEXTEX,DWORD(&gte),DWORD(str.Get()));
			exp->m_Cfg.LoadFromText(str);
		} else exp->m_Cfg.Clear();
	}

	cw=GetDlgItem(hWnd, IDC_SELECTCFG);
	len=GetWindowTextLength(cw);
	str.Clear();
	if(len>0) {
		str.SetLen(len);
		GetWindowTextW(cw,str.GetBuf(),len+1);
		str.Trim();
	}
	if(exp->m_Cfg.BlockGetNE(str)==NULL) str.Clear();
	if(!str.IsEmpty()) exp->m_Cfg.ParSetAdd(L"Cur",str);
	else if(exp->m_Cfg.ParCount(L"Cur")>0) exp->m_Cfg.ParDelete(L"Cur");
}

static INT_PTR CALLBACK ExportDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Interval animRange;
	ISpinnerControl  *spin;
	HWND cw;

	EGExp *exp = (EGExp*)GetWindowLongPtr(hWnd,GWLP_USERDATA); 
	switch (msg) {
		case WM_INITDIALOG: {
			exp = (EGExp*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam); 
			CenterWindow(hWnd, GetParent(hWnd)); 

			cw=GetDlgItem(hWnd, IDC_SELECTCFG);
			if(exp->m_Cfg.ParCount(L"Cur")>0) {
				SetWindowTextW(cw,exp->m_Cfg.ParGet(L"Cur").Get());
				exp->m_Cfg.ParDelete(L"Cur");
			}

			cw=GetDlgItem(hWnd, IDC_CFG);
			SetWindowLongPtr(cw,GWLP_USERDATA,lParam);
			exp->m_OldProc = (WNDPROC) SetWindowLong(cw, GWL_WNDPROC, (LONG) EditSubclassProc); 

			SETTEXTEX ste; ste.codepage=1200; ste.flags=ST_DEFAULT;
			Base::CBuf buf; exp->m_Cfg.SaveInText(buf); buf.Word(0);
			SendMessage(cw,EM_SETTEXTEX,DWORD(&ste),DWORD(buf.Get()));

			CHARFORMAT2 cf; ZeroMemory(&cf,sizeof(cf));
			cf.cbSize=sizeof(cf);
			cf.dwMask=CFM_FACE | CFM_SIZE;
			cf.yHeight=10*20;
			cf.bPitchAndFamily=FIXED_PITCH;
			strcpy(cf.szFaceName,"Courier New");
			SendMessage(cw,EM_SETCHARFORMAT,SCF_ALL,DWORD(&cf));

			cw=GetDlgItem(hWnd, IDC_LOG);
			SendMessage(cw,EM_SETCHARFORMAT,SCF_ALL,DWORD(&cf));

			break;
		}
		case WM_DESTROY: {
			cw=GetDlgItem(hWnd, IDC_CFG);
			SetWindowLong(cw,GWL_WNDPROC,(LONG)exp->m_OldProc); 
		}

		case CC_SPINNER_CHANGE:
			spin = (ISpinnerControl*)lParam; 
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_SELECTCFG: {
					if(HIWORD(wParam)==CBN_DROPDOWN) {
						Base::CBlockPar bp(false);

						cw=GetDlgItem(hWnd, IDC_CFG);
						GETTEXTLENGTHEX gtl; gtl.flags=GTL_DEFAULT|GTL_USECRLF|GTL_NUMBYTES; gtl.codepage=1200;
						int len=SendMessage(cw,EM_GETTEXTLENGTHEX,DWORD(&gtl),0);
						if(len!=E_INVALIDARG) {
							if((len>0) && !(len & 1)) {
								GETTEXTEX gte; gte.cb=len+2; gte.codepage=1200; gte.flags=GT_USECRLF; gte.lpDefaultChar=NULL; gte.lpUsedDefChar=NULL;
								Base::CWStr str; str.SetLen(len >> 1);
								SendMessage(cw,EM_GETTEXTEX,DWORD(&gte),DWORD(str.Get()));
								bp.LoadFromText(str);
							} else exp->m_Cfg.Clear();
						}

						cw=GetDlgItem(hWnd, IDC_SELECTCFG);
						SendMessage(cw,CB_RESETCONTENT,0,0);

						int cnt=bp.BlockCount();
						for(int i=0;i<cnt;i++) {
							if(!bp.BlockGetName(i).IsEmpty()) SendMessageW(cw,CB_ADDSTRING,0,DWORD(bp.BlockGetName(i).Get()));
						}
					}
					break;
				}
				case IDBUILD: {
					Base::CWStr tstr;
					SaveCfgFromDialog(exp,hWnd);
					exp->m_Log.Clear();

					while(true) {
						if(exp->m_Cfg.ParCount(L"Cur")<=0) {
							if(!exp->LoadCfgFromScene()) {
                                exp->Log(Base::CWStr(L"Settings not selected"));
								break;
							} else {
								exp->m_Set=&exp->m_CfgFromScene;
							}
						} else {
							exp->m_Set=exp->m_Cfg.BlockGetNE(exp->m_Cfg.Par(L"Cur"));
							if(exp->m_Set==NULL) {
                                exp->Log(Base::CWStr(L"Settings not found"));
								break;
							}
						}

						tstr=exp->m_Set->ParNE(L"ExportType");
						if(tstr==L"Simple" || tstr==L"") {
							exp->ExportSimpleGroup();
						} else {
                            exp->Log(Base::CWStr(L"Unknown type"));
						}

						break;
					}

					if(exp->m_Log.Len()>0) {
						exp->m_Log.Word(0);
						SETTEXTEX ste; ste.codepage=1200; ste.flags=ST_DEFAULT;
						cw=GetDlgItem(hWnd, IDC_LOG);
						SendMessage(cw,EM_SETTEXTEX,DWORD(&ste),DWORD(exp->m_Log.Get()));
					}
					break;
				}
				case IDOK: {
/*					Base::CWStr str;

					cw=GetDlgItem(hWnd, IDC_CFG);
					GETTEXTLENGTHEX gtl; gtl.flags=GTL_DEFAULT|GTL_USECRLF|GTL_NUMBYTES; gtl.codepage=1200;
					int len=SendMessage(cw,EM_GETTEXTLENGTHEX,DWORD(&gtl),0);
					if(len!=E_INVALIDARG) {
						if((len>0) && !(len & 1)) {
							GETTEXTEX gte; gte.cb=len+2; gte.codepage=1200; gte.flags=GT_USECRLF; gte.lpDefaultChar=NULL; gte.lpUsedDefChar=NULL;
							str.SetLen(len >> 1);
							SendMessage(cw,EM_GETTEXTEX,DWORD(&gte),DWORD(str.Get()));
							exp->m_Cfg.LoadFromText(str);
						} else exp->m_Cfg.Clear();

					}

					cw=GetDlgItem(hWnd, IDC_SELECTCFG);
					len=GetWindowTextLength(cw);
					str.Clear();
					if(len>0) {
						str.SetLen(len);
						GetWindowTextW(cw,str.Get(),len+1);
						str.Trim();
					}
					if(exp->m_Cfg.BlockGetNE(str)==NULL) str.Clear();
					if(!str.IsEmpty()) exp->m_Cfg.ParSetAdd(L"Cur",str);
					else if(exp->m_Cfg.ParCount(L"Cur")>0) exp->m_Cfg.ParDelete(L"Cur");
*/
					SaveCfgFromDialog(exp,hWnd);

					EndDialog(hWnd, 1);
					break;
				}
                case IDC_BUTTON_HELP: {
					exp->m_Log.Clear();

                    exp->Log(Base::CWStr(
                        L"Version=1.1\n\n"
                        L"Group=string\n"
                        L"Type=<ExportCfg,Center,ExportMatrix>\n"
                        L"Export=<0,1>\n\n"
                        L"ExportCfg {\n"
                        L"    ExportType=Simple\n"
                        L"    Group {\n"
                        L"        Name=string\n"
                        L"        FileName=string\n"
                        L"        TextureDefault=<0,1>\n"
                        L"        Edge=[0,1,2]                                  ; 0-export edge off 1-export all edge 2-export optimized edge\n"
                        L"        EdgeFactor=float                              ; in gradus. default=0.1\n"
                        L"        Anim=id,name,[50,0-100:5]\n\n"
                        L"    }\n"
                        L"    Rename {\n"
                        L"        image.png=robot.00\n"
                        L"    }\n"
                        L"}\n\n"
                        L"Center {\n"
                        L"    MaxSize=float[,float[,float]]\n"
                        L"    Scale=float[,float[,float]]\n"
                        L"}\n\n"
                        L"Matrix {\n"
                        L"    Id=int\n"
                        L"    Name=string\n"
                        L"}\n\n"
                    ));

					if(exp->m_Log.Len()>0) {
						exp->m_Log.Word(0);
						SETTEXTEX ste; ste.codepage=1200; ste.flags=ST_DEFAULT;
						cw=GetDlgItem(hWnd, IDC_LOG);
						SendMessage(cw,EM_SETTEXTEX,DWORD(&ste),DWORD(exp->m_Log.Get()));
					}
                    break;
                }
				case IDCANCEL:
					EndDialog(hWnd, 0);
					break;
			}
			break;
		default:
			return FALSE;
	}
	return TRUE;
}       

// Dummy function for progress bar
DWORD WINAPI fn(LPVOID arg)
{
	return(0);
}

// Start the exporter!
// This is the real entrypoint to the exporter. After the user has selected
// the filename (and he's prompted for overwrite etc.) this method is called.
int EGExp::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options) 
{
	showPrompts = suppressPrompts ? FALSE : TRUE;
	exportSelected = (options & SCENE_EXPORT_SELECTED) ? TRUE : FALSE;

	m_FileName.Set(Base::CStr((char *)name));

	ip = i;

	try {
		m_Cfg.LoadFromTextFile(Base::CWStr(Base::CStr((char *)ip->GetDir(APP_PLUGCFG_DIR)))+L"\\EGExp.cfg");
	} catch(...) {
	}

//	if(showPrompts) {
		if (!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EGEXPORT_DLG), ip->GetMAXHWnd(), ExportDlgProc, (LPARAM)this)) {
			return 1;
		}
//	}

	m_Cfg.SaveInTextFile(Base::CWStr(Base::CStr((char *)ip->GetDir(APP_PLUGCFG_DIR)))+L"\\EGExp.cfg",true);

	return 1;
}

BOOL EGExp::SupportsOptions(int ext, DWORD options) {
	assert(ext == 0);	// We only support one extension
	return(options == SCENE_EXPORT_SELECTED) ? TRUE : FALSE;
	}

// This method is the main object exporter.
// It is called once of every node in the scene. The objects are
// exported as they are encoutered.

// Before recursing into the children of a node, we will export it.
// The benefit of this is that a nodes parent is always before the
// children in the resulting file. This is desired since a child's
// transformation matrix is optionally relative to the parent.

BOOL EGExp::nodeEnum(INode* node, int indentLevel) 
{
	if(exportSelected && node->Selected() == FALSE)
		return TREE_CONTINUE;

	nCurNode++;
	ip->ProgressUpdate((int)((float)nCurNode/nTotalNodeCount*100.0f)); 

	// Stop recursing if the user pressed Cancel 
	if (ip->GetCancel())
		return FALSE;
	
	TSTR indent = GetIndent(indentLevel);
	
	// If this node is a group head, all children are 
	// members of this group. The node will be a dummy node and the node name
	// is the actualy group name.
	if (node->IsGroupHead()) {
		fprintf(pStream,"%s%s \"%s\" {\n", indent.data(), ID_GROUP, FixupName(node->GetName())); 
		indentLevel++;
	}
	
	// Only export if exporting everything or it's selected
	if(!exportSelected || node->Selected()) {

		// The ObjectState is a 'thing' that flows down the pipeline containing
		// all information about the object. By calling EvalWorldState() we tell
		// max to eveluate the object at end of the pipeline.
		ObjectState os = node->EvalWorldState(0); 

		// The obj member of ObjectState is the actual object we will export.
		if (os.obj) {

			// We look at the super class ID to determine the type of the object.
			switch(os.obj->SuperClassID()) {
			case GEOMOBJECT_CLASS_ID: 
				if (GetIncludeObjGeom()) ExportGeomObject(node, indentLevel); 
				break;
			case CAMERA_CLASS_ID:
				if (GetIncludeObjCamera()) ExportCameraObject(node, indentLevel); 
				break;
			case LIGHT_CLASS_ID:
				if (GetIncludeObjLight()) ExportLightObject(node, indentLevel); 
				break;
			case SHAPE_CLASS_ID:
				if (GetIncludeObjShape()) ExportShapeObject(node, indentLevel); 
				break;
			case HELPER_CLASS_ID:
				if (GetIncludeObjHelper()) ExportHelperObject(node, indentLevel); 
				break;
			}
		}
	}	
	
	// For each child of this node, we recurse into ourselves 
	// until no more children are found.
	for (int c = 0; c < node->NumberOfChildren(); c++) {
		if (!nodeEnum(node->GetChildNode(c), indentLevel))
			return FALSE;
	}
	
	// If thie is true here, it is the end of the group we started above.
	if (node->IsGroupHead()) {
		fprintf(pStream,"%s}\n", indent.data());
		indentLevel--;
	}

	return TRUE;
}


void EGExp::PreProcess(INode* node, int& nodeCount)
{
	nodeCount++;
	
	// Add the nodes material to out material list
	// Null entries are ignored when added...
	mtlList.AddMtl(node->GetMtl());

	// For each child of this node, we recurse into ourselves 
	// and increment the counter until no more children are found.
	for (int c = 0; c < node->NumberOfChildren(); c++) {
		PreProcess(node->GetChildNode(c), nodeCount);
	}
}

/****************************************************************************

 Configuration.
 To make all options "sticky" across sessions, the options are read and
 written to a configuration file every time the exporter is executed.

 ****************************************************************************/

BOOL MtlKeeper::AddMtl(Mtl* mtl)
{
	if (!mtl) {
		return FALSE;
	}

	int numMtls = mtlTab.Count();
	for (int i=0; i<numMtls; i++) {
		if (mtlTab[i] == mtl) {
			return FALSE;
		}
	}
	mtlTab.Append(1, &mtl, 25);

	return TRUE;
}

int MtlKeeper::GetMtlID(Mtl* mtl)
{
	int numMtls = mtlTab.Count();
	for (int i=0; i<numMtls; i++) {
		if (mtlTab[i] == mtl) {
			return i;
		}
	}
	return -1;
}

int MtlKeeper::Count()
{
	return mtlTab.Count();
}

Mtl* MtlKeeper::GetMtl(int id)
{
	return mtlTab[id];
}
