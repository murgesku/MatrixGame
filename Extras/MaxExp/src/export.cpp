//************************************************************************** 
//* Export.cpp	- Ascii File Exporter
//* 
//* By Christer Janson
//* Kinetix Development
//*
//* January 20, 1997 CCJ Initial coding
//*
//* This module contains the main export functions.
//*
//* Copyright (c) 1997, All Rights Reserved. 
//***************************************************************************

#include "stdafx.h"
#include "main.hpp"

void ConvertMatrix(SVOMatrix & out,Matrix3 & in)
{
	Point3 pt;
	pt=in.GetRow(0);
	out._11=pt.x; out._12=pt.y; out._13=pt.z; out._14=0.0f;
	pt=in.GetRow(1);
	out._21=pt.x; out._22=pt.y; out._23=pt.z; out._24=0.0f;
	pt=in.GetRow(2);
	out._31=pt.x; out._32=pt.y; out._33=pt.z; out._34=0.0f;
	pt=in.GetRow(3);
	out._41=pt.x; out._42=pt.y; out._43=pt.z; out._44=1.0f;
}

void ConvertMatrix(Matrix3 & out,SVOMatrix & in)
{
	Point3 pt;
	
	pt.x=in._11; pt.y=in._12; pt.z=in._13; out.SetRow(0,pt);
	pt.x=in._21; pt.y=in._22; pt.z=in._23; out.SetRow(1,pt);
	pt.x=in._31; pt.y=in._32; pt.z=in._33; out.SetRow(2,pt);
	pt.x=in._41; pt.y=in._42; pt.z=in._43; out.SetRow(3,pt);
}

void MulMatrix(SVOMatrix & des,SVOMatrix & a,SVOMatrix & b)
{
	for(int y=0;y<4;y++) for(int x=0;x<4;x++) des.m[x][y]=0.0f;

	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++) {
			for (int k=0; k<4; k++) {
				des.m[i][j] += a.m[k][j] * b.m[i][k];
			}
		}
	}
}

void IdentityMatrix(SVOMatrix & des)
{
    for (int i=0; i<4; i++) {
        for (int j=0; j<4; j++)	{
            des.m[i][j] = 0.0f;
		}
		des.m[i][i] = 1.0f;
	}
}

void ScaleMatrix(SVOMatrix & des,Point3 & v)
{
	IdentityMatrix(des);
	des.m[0][0] = v.x;
	des.m[1][1] = v.y;
	des.m[2][2] = v.z;
}

void TransformVector(Point3 & des, Point3 & v, SVOMatrix & m)
{
	float	hvec[4];

	for (int i=0; i<4; i++) {
		hvec[i] = 0.0f;
		for (int j=0; j<4; j++) {
			if (j==3) {
				hvec[i] += m.m[j][i];
			} else {
				hvec[i] += v[j] * m.m[j][i];
			}
		}
	}
	des.x=hvec[0]/hvec[3];
	des.y=hvec[1]/hvec[3];
	des.z=hvec[2]/hvec[3];
}

void ViewMatrix(Point3 & from,Point3 & at,Point3 & world_up,SVOMatrix & m)
{
	IdentityMatrix(m);

	Point3 view_dir=(at-from);
	view_dir=view_dir.Normalize();

	Point3 right=world_up^view_dir;
	Point3 up=view_dir^right;

	right=right.Normalize();
	up=up.Normalize();

	m._11=right.x;
    m._21=right.y;
    m._31=right.z;
    m._12=up.x;
    m._22=up.y;
    m._32=up.z;
    m._13=view_dir.x;
    m._23=view_dir.y;
    m._33=view_dir.z;

	m._41=-DotProd(right, from);
    m._42=-DotProd(up, from);
    m._43=-DotProd(view_dir, from);
}

/*function ab_ViewMatrix(var from:TDxyz; var at:TDxyz; var world_up:TDxyz):TMatrix4;
var
    view_dir:TDxyz;
    right:TDxyz;
    up:TDxyz;
begin
    Result:=ab_IdentityMatrix;

    view_dir:=Dxyz(at.x-from.x,at.y-from.y,at.z-from.z);
    view_dir:=ab_Normalize(view_dir);

	right:=ab_CrossProduct(world_up, view_dir);
	up:=ab_CrossProduct(view_dir, right);

	right:=ab_Normalize(right);
	up:=ab_Normalize(up);

    Result[0,0]:=right.x;
    Result[1,0]:=right.y;
    Result[2,0]:=right.z;
    Result[0,1]:=up.x;
    Result[1,1]:=up.y;
    Result[2,1]:=up.z;
    Result[0,2]:=view_dir.x;
    Result[1,2]:=view_dir.y;
    Result[2,2]:=view_dir.z;

    Result[3,0]:=-ab_DotProduct(right, from);
    Result[3,1]:=-ab_DotProduct(up, from);
    Result[3,2]:=-ab_DotProduct(view_dir, from);
end;*/

void StrToFrameList(Base::CWStr & str,Base::CBuf & buf)
{
	buf.Clear();
	int cnt=str.GetCountPar(L"[]");
	Base::CWStr st,sf;
	for(int i=0;i<cnt;i++) {
		st=str.GetStrPar(i,L"[]").Trim();
		if(st.IsEmpty()) continue;

		int cntp=st.GetCountPar(L",");
		if(cntp<2) continue;
		int time=st.GetIntPar(0,L",");
		for(int u=1;u<cntp;u++) {
			sf=st.GetStrPar(u,L",").Trim();
			if(sf.IsEmpty()) continue;
			int cp=sf.GetCountPar(L"-:");
			if(cp==0) continue;
			else if(cp==1) {
				buf.Dword(sf.GetIntPar(0,L"-:"));
				buf.Dword(time);
			} else if(cp==2) {
				int ifrom=sf.GetIntPar(0,L"-:");
				int ito=sf.GetIntPar(1,L"-:");
				int k=ifrom;
				while(true) {
					buf.Dword(k);
					buf.Dword(time);

					if(ifrom<ito) k++; else k--;
					if(k==ito) break;
				}
			} else {
				int ifrom=sf.GetIntPar(0,L"-:");
				int ito=sf.GetIntPar(1,L"-:");
				int istep=sf.GetIntPar(2,L"-:");
				if(istep<1) continue;

				int k=ifrom;
				while(true) {
					buf.Dword(k);
					buf.Dword(time);

					if(ifrom<ito) {
						k+=istep;
						if(k>ito) break;
					} else {
						k-=istep;
						if(k<ito) break;
					}
				}
			}
		}
	}
	buf.Pointer(0);
}

void EGExp::ExportSimpleGroup()
{
    int cnt=m_Set->BlockCount(L"Group");
    for(int i=0;i<cnt;i++) {
		Log(Base::CWStr(L"=== Group ==="));
    	GroupClear();
        ExportSimple(m_Set->BlockPathGet(Base::CWStr(L"Group:")+Base::CWStr(i)));
    }
}

void EGExp::ExportSimple(Base::CBlockPar * group)
{
	SVOHeader he;
	SVOGroup vogr;
	SVOFrame voframe;
	SVOFrame * voframep;
	SVOAnimHeader voanimh;
	SVOAnimHeader * voanimhp;

/*	struct SFrameInfo {
		int m_PointBuf;
		int m_PointSme;
		int m_PointSize;
	} * frameInfo=NULL;*/

	Base::CWStr tstr;
	Base::CWStr tstr2;
	Base::CBuf bufout;
	Base::CBuf bframes;
    Base::CWStr filename;

	m_MaterialNextNo=0;

	int * framelist=NULL;
	int framecnt=0;

	int * vergrouplist=NULL;
	int vergroupcnt=0;
	int * trigrouplist=NULL;
	int trigroupcnt=0;
	int * matrixlist=NULL;
	int matrixcnt=0;

	GroupClear();

    m_EdgeExport=0;
	if(group->ParCount(L"Edge")>0) m_EdgeExport=group->ParGet(L"Edge").GetInt();

    m_EdgeExportFactor=cos(0.1 * pi/180.0);
    if(group->ParCount(L"EdgeFactor")>0) {
        m_EdgeExportFactor=group->ParGet(L"EdgeFactor").GetDouble();
        m_EdgeExportFactor=cos(m_EdgeExportFactor * pi/180.0);
    }

    m_TextureDefault=false;
	if(group->ParCount(L"TextureDefault") && group->Par(L"TextureDefault").GetInt()) m_TextureDefault=true;

    filename=m_FileName;
    if(group->ParCount(L"FileName")>0) {
        tstr=group->Par(L"FileName");
        tstr.Trim();
        int cnt=tstr.GetCountPar(L"\\/");
        if(cnt<=0) {
        } else if(cnt<=1) {
            filename=GetFilePath(m_FileName)+tstr;
        } else {
            filename=tstr;
        }
    }
    m_GroupName.Clear();
    if(group->ParCount(L"Name")>0) {
        m_GroupName=group->Par(L"Name");
        m_GroupName.Trim();
    }

	try {
		int animcnt=group->ParCount(L"Anim");
		if(animcnt>0) {
			Interval range = ip->GetAnimRange();
			int framestart=range.Start() / GetTicksPerFrame();
			int frameend=range.End() / GetTicksPerFrame();
			int framemaxcnt=frameend-framestart+1;

			framelist=(int *)HAlloc(framemaxcnt*sizeof(4),NULL);

			for(int i=0;i<animcnt;i++) {
				tstr=group->ParPathGet(Base::CWStr(L"Anim:")+Base::CWStr(i));
				int cnt=tstr.GetCountPar(L",");
				if(cnt<3) throw L"Anim format error";

				StrToFrameList(tstr.GetStrPar(2,cnt-1,L","),bframes);

				for(int u=0;u<bframes.Len()/8;u++) {
					int curframe=bframes.Int(); bframes.Int();
					if(curframe<framestart || curframe>frameend) throw L"Anim error";
					for(int t=0;t<framecnt;t++) if(framelist[t]==curframe) break;
					if(t>=framecnt) {
						framelist[framecnt]=curframe;
						framecnt++;
					}
				}
			}
		} else {
			framelist=(int *)HAlloc(1*sizeof(4),NULL);
			framelist[0]=ip->GetTime();
			framecnt=1;
		}

//		frameInfo=(SFrameInfo *)HAllocClear(framecnt*sizeof(SFrameInfo),NULL);

		for(int i=0;i<framecnt;i++) {
			int numChildren = ip->GetRootNode()->NumberOfChildren();

			m_MatCenter.IdentityMatrix();
			m_Scale=Point3(1.0,1.0,1.0);
			m_MaxSizeX=0; m_MaxSizeY=0; m_MaxSizeZ=0;

			for (int idx=0; idx<numChildren; idx++) {
				if (ip->GetCancel()) throw L"Terminate";
				ExportSimple_NodeEnumFirst(ip->GetRootNode()->GetChildNode(idx),framelist[i]*GetTicksPerFrame(),i);
			}

//			Matrix3 tm;
//			tm.Scale(Point3(-1,1,1),TRUE);
//			m_MatCenter=m_MatCenter*tm;
			m_MatCenter.Scale(m_Scale,FALSE);

			for (int idx=0; idx<numChildren; idx++) {
				if (ip->GetCancel()) throw L"Terminate";
				ExportSimple_NodeEnum(ip->GetRootNode()->GetChildNode(idx),framelist[i]*GetTicksPerFrame(),i);
			}
		}

		GroupSortByNo();

        if(!m_GroupName.IsEmpty()) Log(Base::CWStr(L"Group=")+m_GroupName);
		Log(Base::CWStr(L"OutFile=")+filename);

		int cntvergroup=0;
		int cnttrigroup=0;

		CGroup * gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Ver) cntvergroup++;
			else if(gr->m_Id[0].x==gt_Frame && gr->m_Id[1].x==gt_TriList) cnttrigroup++;
			gr=gr->m_Next;
		}

		// header
		ZeroMemory(&he,sizeof(SVOHeader));
		he.m_Id=0x00006f76;
		he.m_Ver=0;

		if(m_TextureDefault) he.m_Flags|=2;

		bufout.BufAdd(&he,sizeof(SVOHeader));

		// material
		he.m_MaterialSme=bufout.Pointer();
		gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Material) {
				gr->m_User=he.m_MaterialCnt;

				he.m_MaterialCnt+=gr->m_Buf.Len()/sizeof(SMaterial);
				bufout.BufAdd(gr->m_Buf.Get(),gr->m_Buf.Len());

				if(((SMaterial *)(gr->m_Buf.Get()))->tex_diffuse[0]!=0) Log(Base::CWStr(L"Texture=")+((SMaterial *)(gr->m_Buf.Get()))->tex_diffuse);
			}
			gr=gr->m_Next;
		}

		// vertex scale
//		if(m_MaxSizeX>0 || m_MaxSizeY>0 || m_MaxSizeZ>0) {
			double minx=1e30,miny=1e30,minz=1e30,maxx=-1e30,maxy=-1e30,maxz=-1e30;

			gr=m_GroupFirst;
			while(gr!=NULL) {
				if(gr->m_Id[0].x==gt_Ver) {
					int cnt=gr->m_Buf.Len()/sizeof(SVertexNorTex);
					SVertexNorTex * cv=(SVertexNorTex *)gr->m_Buf.Get();
					while(cnt>0) {
						minx=min(cv->x,minx);
						miny=min(cv->y,miny);
						minz=min(cv->z,minz);
						maxx=max(cv->x,maxx);
						maxy=max(cv->y,maxy);
						maxz=max(cv->z,maxz);
						cnt--;
						cv++;
					}
				}
				gr=gr->m_Next;
			}

			double koffx=1e30; if(m_MaxSizeX>0) koffx=m_MaxSizeX/(maxx-minx);
			double koffy=1e30; if(m_MaxSizeY>0) koffy=m_MaxSizeY/(maxy-miny);
			double koffz=1e30; if(m_MaxSizeZ>0) koffz=m_MaxSizeZ/(maxz-minz);

			double kofscale=min(min(koffx,koffy),koffz);
			if(kofscale>1e20) kofscale=1.0;
//			maxx=m_MaxSizeX/max(maxx-minx,max(maxy-miny,maxz-minz));

			if(kofscale!=1.0) {
				gr=m_GroupFirst;
				while(gr!=NULL) {
					if(gr->m_Id[0].x==gt_Ver) {
						int cnt=gr->m_Buf.Len()/sizeof(SVertexNorTex);
						SVertexNorTex * cv=(SVertexNorTex *)gr->m_Buf.Get();
						while(cnt>0) {
							cv->x=float(cv->x*kofscale);
							cv->y=float(cv->y*kofscale);
							cv->z=float(cv->z*kofscale);
							cnt--;
							cv++;
						}
					}
					gr=gr->m_Next;
				}
			}

			gr=m_GroupFirst;
			while(gr!=NULL) {
				if(gr->m_Id[0].x==gt_ExpMatrix) {
					SVOMatrix * em=(SVOMatrix *)((byte *)gr->m_Buf.Get()+sizeof(SVOExpMatrixHeader));

					Matrix3 mm;
					ConvertMatrix(mm,*em);
					mm.Scale(Point3(kofscale,kofscale,kofscale),true);
					mm.Orthogonalize();
					mm.NoScale();
					ConvertMatrix(*em,mm);

/*					Point3 v0=Point3(0,0,0);
					Point3 vx=Point3(1,0,0);
					Point3 vy=Point3(0,1,0);
					Point3 vz=Point3(0,0,1);

					TransformVector(v0,v0,*em);
					TransformVector(vx,vx,*em);
					TransformVector(vy,vy,*em);
					TransformVector(vz,vz,*em);

					vx-=v0; vy-=v0; vz-=v0; v0*=float(kofscale);
					vx=vx.Normalize(); vy=vy.Normalize(); vz=vz.Normalize();

//					ViewMatrix(v0,v0+vz,vy,*em);
//					em->_11=vx.x; em->_21=vx.y; em->_31=vx.z; em->_41=v0.x;
//					em->_12=vy.x; em->_22=vy.y; em->_32=vy.z; em->_42=v0.y;
//					em->_13=vz.x; em->_23=vz.y; em->_33=vz.z; em->_43=v0.z;
//					em->_14=0.0f; em->_24=0.0f; em->_34=0.0f; em->_44=1.0f;

					Point3 ttp0=Point3(0,0,0);
					Point3 ttp1=Point3(0,0,1);
					Point3 ttp2=Point3(0,1,0);
					Point3 ttp3=Point3(1,0,0);
					TransformVector(ttp0,ttp0,*em);
					TransformVector(ttp1,ttp1,*em);
					TransformVector(ttp2,ttp2,*em);
					TransformVector(ttp3,ttp3,*em);
					Point3 ttp1_=ttp1-ttp0;
					Point3 ttp2_=ttp2-ttp0;
					Point3 ttp3_=ttp3-ttp0;
					
					ttp3_=ttp3-ttp0;*/
				}
				gr=gr->m_Next;
			}

//		}

		// vertex
		vergrouplist=(int *)HAlloc(cntvergroup*2*4,NULL);
		he.m_VerSme=bufout.Pointer();
		gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Ver) {
				for(i=0;i<vergroupcnt;i++) {
					if(vergrouplist[i*2+0]==gr->m_Buf.Len() && memcmp((BYTE *)bufout.Get()+vergrouplist[i*2+1],gr->m_Buf.Get(),gr->m_Buf.Len())==0) break;
				}
				if(i<vergroupcnt) {
					gr->m_User=vergrouplist[i*2+0]; // size
					gr->m_User2=vergrouplist[i*2+1]; // sme
				} else {
					gr->m_User=gr->m_Buf.Len(); // size
					gr->m_User2=bufout.Pointer(); // sme
					bufout.BufAdd(gr->m_Buf.Get(),gr->m_Buf.Len());
					vergrouplist[vergroupcnt*2+0]=gr->m_User;
					vergrouplist[vergroupcnt*2+1]=gr->m_User2;
					vergroupcnt++;
					he.m_VerCnt+=gr->m_Buf.Len()/sizeof(SVertexNorTex);
				}
			}
			gr=gr->m_Next;
		}
		Log(Base::CWStr().Format(L"Vertex=<i> size=<i>",he.m_VerCnt,he.m_VerCnt*sizeof(SVertexNorTex)));


		// triangle
		trigrouplist=(int *)HAlloc(cnttrigroup*2*4,NULL);
		he.m_TriSme=bufout.Pointer();
		gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Frame && gr->m_Id[1].x==gt_TriList) {
/*				CGroup * grpoint=GroupFind(Base::CPoint(gt_Ver,gr->m_Id[0].y));
				int smeindex=(grpoint->m_User2-he.m_VerSme)/sizeof(SVertexNorTex);
				int cnt=gr->m_Buf.Len()/4;
				DWORD * ibuf=(DWORD *)gr->m_Buf.Get();
				if(smeindex>0) while(cnt>0) { *ibuf+=smeindex; ibuf++; cnt--; }
*/
				for(i=0;i<trigroupcnt;i++) {
					if(trigrouplist[i*2+0]==gr->m_Buf.Len() && memcmp((BYTE *)bufout.Get()+trigrouplist[i*2+1],gr->m_Buf.Get(),gr->m_Buf.Len())==0) break;
				}
				if(i<trigroupcnt) {
					gr->m_User=trigrouplist[i*2+0]; // size
					gr->m_User2=trigrouplist[i*2+1]; // sme
				} else {
					gr->m_User=gr->m_Buf.Len(); // size
					gr->m_User2=bufout.Pointer(); // sme
					bufout.BufAdd(gr->m_Buf.Get(),gr->m_Buf.Len());
					trigrouplist[trigroupcnt*2+0]=gr->m_User;
					trigrouplist[trigroupcnt*2+1]=gr->m_User2;
					trigroupcnt++;
					he.m_TriCnt+=gr->m_Buf.Len()/4;
				}
			}
			gr=gr->m_Next;
		}

		// triangle index to 16 bit
		if(he.m_TriCnt<65536) {
			he.m_Flags|=1;
			DWORD * sou=(DWORD *)(((BYTE *)bufout.Get())+he.m_TriSme);
			WORD * des=(WORD *)sou;
			int cnt=he.m_TriCnt;
			while(cnt>0) {
				*des=WORD(*sou);
                sou++;
				des++;
				cnt--;
			}
			bufout.Len(he.m_TriSme+he.m_TriCnt*2);
			bufout.Pointer(bufout.Len());
		}
		Log(Base::CWStr().Format(L"Triangles=<i> size=<i>",he.m_TriCnt/3,he.m_TriCnt*((he.m_Flags&1)?2:4)));

		// group
		he.m_GroupSme=bufout.Pointer();
		gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Frame && gr->m_Id[1].x==gt_TriList) {
				ZeroMemory(&vogr,sizeof(SVOGroup));

				gr->m_User3=he.m_GroupCnt;

				CGroup * grpoint=GroupFind(Base::CPoint(gt_Ver,gr->m_Id[0].y));

				vogr.m_Material=gr->m_Id[1].y;
				vogr.m_VerCnt=grpoint->m_User/sizeof(SVertexNorTex);
				vogr.m_VerStart=(grpoint->m_User2-he.m_VerSme)/sizeof(SVertexNorTex);
				vogr.m_TriCnt=gr->m_User/4;
				vogr.m_TriStart=(gr->m_User2-he.m_TriSme)/4;

				bufout.BufAdd(&vogr,sizeof(SVOGroup));

				he.m_GroupCnt++;
			}
			gr=gr->m_Next;
		}
		Log(Base::CWStr().Format(L"Groups=<i>",he.m_GroupCnt));

		// edge
		if(m_EdgeExport) {
			he.m_EdgeSme=bufout.Pointer();

			gr=m_GroupFirst;
			while(gr!=NULL) {
				if(gr->m_Id[0].x==gt_Edge) {
					gr->m_User=(bufout.Pointer()-he.m_EdgeSme)/sizeof(SVOEdge);
					gr->m_User2=gr->m_Buf.Len()/sizeof(SVOEdge);

					bufout.BufAdd(gr->m_Buf.Get(),gr->m_Buf.Len());
					he.m_EdgeCnt+=gr->m_Buf.Len()/sizeof(SVOEdge);
				}
				gr=gr->m_Next;
			}
			Log(Base::CWStr().Format(L"Edges=<i>",he.m_EdgeCnt));
		}

		// frame
		ZeroMemory(&voframe,sizeof(SVOFrame));
		he.m_FrameSme=bufout.Pointer();
		he.m_FrameCnt=framecnt;
		voframe.m_MinX=1e30f; voframe.m_MinY=1e30f; voframe.m_MinZ=1e30f;
		voframe.m_MaxX=-1e30f; voframe.m_MaxY=-1e30f; voframe.m_MaxZ=-1e30f;
		for(i=0;i<framecnt;i++) {
			bufout.BufAdd(&voframe,sizeof(SVOFrame));
		}
		for(i=0;i<framecnt;i++) {
			int sme=bufout.Pointer();
			int cnt=0;
			gr=m_GroupFirst;
			while(gr!=NULL) {
				if(gr->m_Id[0].x==gt_Frame && gr->m_Id[0].y==i && gr->m_Id[1].x==gt_TriList) {
					bufout.Dword(gr->m_User3);
					gr->m_User4=cnt;
					cnt++;
				}
				gr=gr->m_Next;
			}
			voframep=(SVOFrame *)(((BYTE *)bufout.Get())+he.m_FrameSme+i*sizeof(SVOFrame));
			voframep->m_GroupIndexCnt=cnt;
			voframep->m_GroupIndexSme=sme;

			gr=GroupFind(Base::CPoint(gt_Ver,i));

			cnt=gr->m_Buf.Len()/sizeof(SVertexNorTex);
			SVertexNorTex * cv=(SVertexNorTex *)gr->m_Buf.Get();
			while(cnt>0) {
				voframep->m_MinX=min(cv->x,voframep->m_MinX);
				voframep->m_MinY=min(cv->y,voframep->m_MinY);
				voframep->m_MinZ=min(cv->z,voframep->m_MinZ);
				voframep->m_MaxX=max(cv->x,voframep->m_MaxX);
				voframep->m_MaxY=max(cv->y,voframep->m_MaxY);
				voframep->m_MaxZ=max(cv->z,voframep->m_MaxZ);
				voframep->m_CenterX+=cv->x;
				voframep->m_CenterY+=cv->y;
				voframep->m_CenterZ+=cv->z;
				cnt--;
				cv++;
			}
			cnt=gr->m_Buf.Len()/sizeof(SVertexNorTex);
			voframep->m_CenterX/=cnt;
			voframep->m_CenterY/=cnt;
			voframep->m_CenterZ/=cnt;
			cv=(SVertexNorTex *)gr->m_Buf.Get();
			float mr=0,mr2=0;
			while(cnt>0) {
				mr=max(mr,(voframep->m_CenterX-cv->x)*(voframep->m_CenterX-cv->x)+(voframep->m_CenterY-cv->y)*(voframep->m_CenterY-cv->y)+(voframep->m_CenterZ-cv->z)*(voframep->m_CenterZ-cv->z));
				mr2=max(mr2,((voframep->m_MaxX+voframep->m_MinX)/2-cv->x)*((voframep->m_MaxX+voframep->m_MinX)/2-cv->x)+((voframep->m_MaxY+voframep->m_MinY)/2-cv->y)*((voframep->m_MaxY+voframep->m_MinY)/2-cv->y)+((voframep->m_MaxZ+voframep->m_MinZ)/2-cv->z)*((voframep->m_MaxZ+voframep->m_MinZ)/2-cv->z));
				cnt--;
				cv++;
			}
			voframep->m_RadiusCenter=float(sqrt(mr));
			voframep->m_RadiusBox=float(sqrt(mr2));

			if(m_EdgeExport) {
				gr=GroupFind(Base::CPoint(gt_Edge,i));
				if(gr) {
					voframep->m_EdgeStart=gr->m_User;
					voframep->m_EdgeCnt=gr->m_User2;

					int lastmat1=-1;
					int lastmat2=-1;
					int lastgroupinframe1=-1;
					int lastgroupinframe2=-1;

					SVOEdge * edg=(SVOEdge *)(DWORD(bufout.Get())+he.m_EdgeSme+voframep->m_EdgeStart*sizeof(SVOEdge));
					for(int ei=0;ei<int(voframep->m_EdgeCnt);ei++,edg++) {
						int cmat=((edg->m_SideTri1)>>20)&0x0ff;
						if(lastmat1!=cmat) {
							lastmat1=cmat;
							lastgroupinframe1=GroupFind(Base::CPoint(gt_Frame,i),Base::CPoint(gt_TriList,cmat))->m_User4;
						}
						edg->m_SideTri1=(edg->m_SideTri1&(~(0x0ff<<20))) | ((lastgroupinframe1)<<20);

						cmat=((edg->m_SideTri2)>>20)&0x0ff;
						if(lastmat2!=cmat) {
							lastmat2=cmat;
							lastgroupinframe2=GroupFind(Base::CPoint(gt_Frame,i),Base::CPoint(gt_TriList,cmat))->m_User4;
						}
						edg->m_SideTri2=(edg->m_SideTri2&(~(0x0ff<<20))) | ((lastgroupinframe2)<<20);
					}
				}
			}
		}
		Log(Base::CWStr().Format(L"Frames=<i>",he.m_FrameCnt));

		// anim
		ZeroMemory(&voanimh,sizeof(SVOAnimHeader));
		he.m_AnimSme=bufout.Pointer();
		he.m_AnimCnt=animcnt;
		for(i=0;i<animcnt;i++) {
			bufout.BufAdd(&voanimh,sizeof(SVOAnimHeader));
		}
		for(i=0;i<animcnt;i++) {
			ZeroMemory(&voanimh,sizeof(SVOAnimHeader));

			tstr=group->ParPathGet(Base::CWStr(L"Anim:")+Base::CWStr(i));
			int cnt=tstr.GetCountPar(L",");

			voanimh.m_Id=tstr.GetStrPar(0,L",").GetInt();

			tstr2=RenameByCfg(tstr.GetStrPar(1,L","));
			if(tstr2.GetLen()>31) tstr2.SetLen(31);
			if(tstr2.GetLen()>0) CopyMemory(voanimh.m_Name,tstr2.Get(),tstr2.GetLen()*2);

			StrToFrameList(tstr.GetStrPar(2,cnt-1,L","),bframes);

			voanimh.m_UnitCnt=bframes.Len()/8;
			voanimh.m_UnitSme=bufout.Pointer();

//			bufout.BufAdd(bframes.Get(),bframes.Len());

			tstr=Base::CWStr().Format(L"Anim=<i>,<s>",voanimh.m_Id,voanimh.m_Name);
			bframes.Pointer(0);
			for(int u=0;u<bframes.Len()/8;u++) {
				int cf=bframes.Int();
				int ct=bframes.Int();

				for(int t=0;t<framecnt;t++) if(framelist[t]==cf) { cf=t; break; }
				if(t>=framecnt) throw L"Error in anim";

				tstr+=L",";
				tstr+=Base::CWStr(cf);

				bufout.Int(cf);
				bufout.Int(ct);
			}
			Log(tstr);

			tstr=Base::CWStr().Format(L"Anim=<i>,<s>",voanimh.m_Id,voanimh.m_Name);
			bframes.Pointer(0);
			for(u=0;u<bframes.Len()/8;u++) {
				bframes.Int();
				tstr+=L",";
				tstr+=Base::CWStr(bframes.Int());
			}
			Log(tstr);

			voanimhp=(SVOAnimHeader *)(((BYTE *)bufout.Get())+he.m_AnimSme+i*sizeof(SVOAnimHeader));
			CopyMemory(voanimhp,&voanimh,sizeof(SVOAnimHeader));
		}

		// ExportMatrix
		matrixlist=(int *)HAlloc(GroupCount()*4,NULL);
		he.m_MatrixSme=bufout.Pointer();
		he.m_MatrixCnt=0;
		gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_ExpMatrix && gr->m_Id[0].y==0 && gr->m_Id[1].x==gt_ExpMatrix) {
				he.m_MatrixCnt++;

				SVOExpMatrixHeader * emh=(SVOExpMatrixHeader *)gr->m_Buf.Get();
				Log(Base::CWStr().Format(L"Matrix=<s>",emh->m_Name));

				bufout.BufAdd(gr->m_Buf.Get(),sizeof(SVOExpMatrixHeader));
				matrixlist[matrixcnt]=gr->m_Id[1].y;
				matrixcnt++;
			}
			gr=gr->m_Next;
		}
		for(i=0;i<matrixcnt;i++) {
			for(int u=0;u<framecnt;u++) {
				gr=GroupFind(Base::CPoint(gt_ExpMatrix,u),Base::CPoint(gt_ExpMatrix,matrixlist[i]));

				if(u==0) {
					SVOExpMatrixHeader * emh=(SVOExpMatrixHeader *)((BYTE *)bufout.Get()+he.m_MatrixSme+i*sizeof(SVOExpMatrixHeader));
					emh->m_MatrixSme=bufout.Pointer();
				}

				bufout.BufAdd((BYTE *)gr->m_Buf.Get()+sizeof(SVOExpMatrixHeader),sizeof(SVOMatrix));
			}
		}

		// header again
		CopyMemory(bufout.Get(),&he,sizeof(SVOHeader));

		Log(Base::CWStr().Format(L"File Size=<i>",bufout.Len()));

		// Save
//		Base::CFile fileout(m_FileName);
		Base::CFile fileout(filename);
		fileout.Create();
		fileout.Write(bufout.Get(),bufout.Len());
		fileout.Close();


/*		int cntgroup=0;
		int cntver=0;
		int cnttri=0;

		gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Ver) cntver+=gr->m_Buf.Len() / sizeof(SVertexNorTex);
			else if(gr->m_Id[0].x==gt_Frame && gr->m_Id[1].x==gt_TriList) cnttri+=gr->m_Buf.Len() / (4*3);
			else if(gr->m_Id[0].x==gt_Material) cntgroup++;
			gr=gr->m_Next;
		}
		bufout.Dword(cntgroup);
		bufout.Dword(cntver);
		bufout.Dword(cnttri);
		bufout.Dword(0); // rez

		Log(Base::CWStr(L"GroupCnt=")+cntgroup);
		Log(Base::CWStr(L"VertexCnt=")+cntver);
		Log(Base::CWStr(L"TriangleCnt=")+cnttri);

		int writesmegro=bufout.Pointer();	bufout.Dword(0); // sme list group
		int writesmever=bufout.Pointer();	bufout.Dword(0); // sme list vertex
		int writesmetri=bufout.Pointer();	bufout.Dword(0); // sme list tri
		bufout.Dword(0); // rez

		DWORD * bufgroupsme=(DWORD *)Base::Alloc(4*cntgroup);

		*(DWORD *)(DWORD(bufout.Get())+writesmegro)=bufout.Pointer();
		gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Material) {
				bufgroupsme[gr->m_No]=bufout.Pointer();
				bufout.Dword(0); // begin triangle
				bufout.Dword(0); // cnt triangle
				bufout.BufAdd(gr->m_Buf.Get(),gr->m_Buf.Len());

				if(((SMaterial *)(gr->m_Buf.Get()))->tex_diffuse[0]!=0) Log(Base::CWStr(L"Texture=")+((SMaterial *)(gr->m_Buf.Get()))->tex_diffuse);
			}
			gr=gr->m_Next;
		}

		int pointsme=0;

		*(DWORD *)(DWORD(bufout.Get())+writesmever)=bufout.Pointer();
		gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Ver) {
				for(int i=0;i<framecnt;i++) {
					if(frameInfo[i].m_PointBuf!=0 && frameInfo[i].m_PointSize==gr->m_Buf.Len() && memcmp((BYTE *)bufout.Get()+frameInfo[i].m_PointBuf,gr->m_Buf.Get(),gr->m_Buf.Len())==0) break;
				}
				if(i<framecnt) {
					frameInfo[gr->m_Id[0].y].m_PointBuf=frameInfo[i].m_PointBuf;
					frameInfo[gr->m_Id[0].y].m_PointSme=frameInfo[i].m_PointSme;
					frameInfo[gr->m_Id[0].y].m_PointSize=frameInfo[i].m_PointSize;
				} else {
					frameInfo[gr->m_Id[0].y].m_PointBuf=bufout.Pointer();
					frameInfo[gr->m_Id[0].y].m_PointSme=pointsme;
					frameInfo[gr->m_Id[0].y].m_PointSize=gr->m_Buf.Len();
					pointsme+=gr->m_Buf.Len()/sizeof(SVertexNorTex);
				}
				bufout.BufAdd(gr->m_Buf.Get(),gr->m_Buf.Len());
			}
			gr=gr->m_Next;
		}

		*(DWORD *)(DWORD(bufout.Get())+writesmetri)=bufout.Pointer();
		gr=m_GroupFirst;
		int sme=0;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Frame && gr->m_Id[1].x==gt_TriList) {
				*(DWORD *)(DWORD(bufout.Get())+bufgroupsme[gr->m_Id[1].y])=sme;
				*(DWORD *)(DWORD(bufout.Get())+bufgroupsme[gr->m_Id[1].y]+4)=gr->m_Buf.Len() / (3*4);

				sme=sme+gr->m_Buf.Len() / (3*4);
				bufout.BufAdd(gr->m_Buf.Get(),gr->m_Buf.Len());
			}
			gr=gr->m_Next;
		}

		Base::Free(bufgroupsme);*/

	} catch(wchar * estr) {
        Log(Base::CWStr(estr));
	}

	if(matrixlist!=NULL) { HFree(matrixlist,NULL); matrixlist=NULL; }
	if(vergrouplist!=NULL) { HFree(vergrouplist,NULL); vergrouplist=NULL; }
	if(trigrouplist!=NULL) { HFree(trigrouplist,NULL); trigrouplist=NULL; }

//	if(frameInfo!=NULL) { HFree(frameInfo,NULL); frameInfo=NULL; }
	if(framelist!=NULL) { HFree(framelist,NULL); framelist=NULL; }
}

BOOL EGExp::ExportSimple_NodeEnumFirst(INode *node,TimeValue timev,int frame)
{
	if (ip->GetCancel()) return FALSE;

	ObjectState os = node->EvalWorldState(timev); 
	if(os.obj) {
		switch(os.obj->SuperClassID()) {
			case HELPER_CLASS_ID: {
				TSTR tstr;

                if(!m_GroupName.IsEmpty() && node->GetUserPropString("Group",tstr)) {
                    Base::CWStr atstr(Base::CStr(tstr.data()));
                    atstr.Trim();
                    if(atstr!=m_GroupName) break;
                }

                if(node->GetUserPropString("Type",tstr)) {
					if(tstr==L"Center") {
						m_MatCenter=node->GetObjTMAfterWSM(timev);
						m_MatCenter.Invert();

						if(node->GetUserPropString("MaxSize",tstr)) {
							Base::CWStr ts(Base::CStr(tstr.data()));
//							if(sc>0) m_MaxSize=sc;

							int cp=ts.GetCountPar(L",");
							if(cp>=1 && !ts.GetStrPar(0,L",").Trim().IsEmpty()) m_MaxSizeX=ts.GetDoublePar(0,L",");
							if(cp>=2 && !ts.GetStrPar(1,L",").Trim().IsEmpty()) m_MaxSizeY=ts.GetDoublePar(1,L",");
							if(cp>=3 && !ts.GetStrPar(2,L",").Trim().IsEmpty()) m_MaxSizeZ=ts.GetDoublePar(2,L",");
						}
						if(node->GetUserPropString("Scale",tstr)) {
							Base::CWStr ts(Base::CStr(tstr.data()));

							int cp=ts.GetCountPar(L",");
							if(cp>=1 && !ts.GetStrPar(0,L",").Trim().IsEmpty()) m_Scale.x=(float)ts.GetDoublePar(0,L",");
							if(cp>=2 && !ts.GetStrPar(1,L",").Trim().IsEmpty()) m_Scale.y=(float)ts.GetDoublePar(1,L",");
							if(cp>=3 && !ts.GetStrPar(2,L",").Trim().IsEmpty()) m_Scale.z=(float)ts.GetDoublePar(2,L",");
						}
					}
				}
				break;
			}
		}
	}

	for (int c = 0; c < node->NumberOfChildren(); c++) {
		if (!ExportSimple_NodeEnumFirst(node->GetChildNode(c),timev,frame)) return FALSE;
	}

	return TRUE;
}

BOOL EGExp::ExportSimple_NodeEnum(INode* node,TimeValue timev,int frame)
{
	if (ip->GetCancel()) return FALSE;

	ObjectState os = node->EvalWorldState(timev); 
	if(os.obj) {
		switch(os.obj->SuperClassID()) {
			case GEOMOBJECT_CLASS_ID:
				ExportSimple_GeomObject(node,timev,frame); 
				break;
			case HELPER_CLASS_ID:
				ExportSimple_Helper(node,timev,frame); 
				break;
		}
	}

	for (int c = 0; c < node->NumberOfChildren(); c++) {
		if (!ExportSimple_NodeEnum(node->GetChildNode(c),timev,frame)) return FALSE;
	}

	return TRUE;
}

void EGExp::ExportSimple_GeomObject(INode * node,TimeValue timev,int frame)
{
	int i,u,vi;
	SVertexNorTex vdes,vdess[3];
	Point3 v,v0,v1,v2;//,vtp[3];

	BOOL bval;
	if(node->GetUserPropBool("Export",bval)) {
		if(!bval) return;
	}

    TSTR tstr;
    if(!m_GroupName.IsEmpty() && node->GetUserPropString("Group",tstr)) {
        Base::CWStr atstr(Base::CStr(tstr.data()));
        atstr.Trim();
        if(atstr!=m_GroupName) return;
    }

	Matrix3 tm = node->GetObjTMAfterWSM(timev)*m_MatCenter;

	int vx[3];
	if (TMNegParity(tm)) { vx[0] = 2; vx[1] = 1; vx[2] = 0; }
	else { vx[0] = 0; vx[1] = 1; vx[2] = 2; }

	BOOL needDel;
	TriObject* tri = GetTriObjectFromNode(node, timev, needDel);
	if (!tri) return;

	Mtl * nodeMtl = node->GetMtl();

	Mesh* mesh = &tri->GetMesh();
	mesh->buildRenderNormals();//buildNormals();

	CGroup * gv=GroupFindAdd(Base::CPoint(gt_Ver,frame));
	CGroup * gf=NULL;
	if(m_EdgeExport) gf=GroupFindAdd(Base::CPoint(gt_Edge,frame));

	Matrix3 normalObjToWorld(1);
//	for (int it=0; it<3; it++) {
//		Point4 p = Inverse(tm).GetColumn(it);
//		normalObjToWorld.SetRow(it,Point3(p[0],p[1],p[2]));
//	}
	for (int it=0; it<4; it++) {
		Point3 p = tm.GetRow(it);
		normalObjToWorld.SetRow(it,p);
	}

	DWORD * edgeindexlist=NULL;
	if(m_EdgeExport) edgeindexlist=(DWORD *)HAllocClear(mesh->getNumFaces()*4,NULL);

	for(i=0; i<mesh->getNumFaces(); i++) {
//		Point3	fn = mesh->getFaceNormal(i);
//		Point3	nfn = VectorTransform(normalObjToWorld/*tm*/, fn);
//		nfn=nfn.Normalize();
//		mesh->setFaceNormal(i, nfn);

		v0 = tm * mesh->verts[mesh->faces[i].v[vx[0]]];
		v1 = tm * mesh->verts[mesh->faces[i].v[vx[1]]];
		v2 = tm * mesh->verts[mesh->faces[i].v[vx[2]]];

		Point3 nfn=((v1-v0)^(v2-v0)).Normalize();
		mesh->setFaceNormal(i, nfn);
	}

	mesh->buildNormals();

	if(mesh->tVerts==NULL) {
//		Log(Base::CWStr(L"Mapping coords not found: ")+Base::CWStr(Base::CStr(node->NodeName())));
		TSTR str;
		node->EvalWorldState(timev).obj->GetClassName(str);
		Log(Base::CWStr().Format(L"Mapping coords not found: <s>  Class name: <s>",
				Base::CWStr(Base::CStr(node->NodeName())).Get(),
				Base::CWStr(Base::CStr(str.data())).Get() 
			));
	}

	for(i=0; i<mesh->getNumVerts(); i++) {
		v=mesh->verts[i];
		int cnt=0;
		for(u=i+1; u<mesh->getNumVerts(); u++) {
			if(i!=u) {
				if((mesh->verts[u]-v).Length()<=0.01) {
					mesh->vertSel.Set(u);
					mesh->vertSel.Set(i);
					cnt++;
				}
			}
		}
		if(cnt>=1) {
			mesh->vertSel.Set(i);
		} else {
			mesh->vertSel.Clear(i);
		}
	}

	for(i=0; i<mesh->getNumFaces(); i++) {
		int twosided=0;
		int mno=GroupMaterialFindAdd(nodeMtl,mesh->faces[i].getMatID(),twosided);
		CGroup * gtl=GroupFindAdd(Base::CPoint(gt_Frame,frame),Base::CPoint(gt_TriList,mno));

		if(m_EdgeExport) edgeindexlist[i]=(mno<<20)|(gtl->m_Buf.Pointer()/(4*3));

		for(u=0; u<3; u++) {
			vi=mesh->faces[i].v[vx[u]];

			v = tm * mesh->verts[vi];
			vdes.x=v.x; vdes.y=v.y; vdes.z=v.z;
//vtp[u]=v;

			v = Normalize(GetVertexNormal(mesh, i, mesh->getRVertPtr(vi)));
//			v = Normalize(mesh->getFaceNormal(i));
			vdes.nx=v.x; vdes.ny=v.y; vdes.nz=v.z;

			if(mesh->tVerts!=NULL) {
				v = mesh->tVerts[mesh->tvFace[i].t[vx[u]]];
				vdes.tu=v.x; vdes.tv=1.0f-v.y;
			} else {
				vdes.tu=0; vdes.tv=0;
			}

			vdess[u]=vdes;

			gtl->m_Buf.Dword(AddVertex(gv->m_Buf,vdes));

		}
		if(twosided) {
			vdess[0].nx=-vdess[0].nx; vdess[0].ny=-vdess[0].ny; vdess[0].nz=-vdess[0].nz;
			vdess[1].nx=-vdess[1].nx; vdess[1].ny=-vdess[1].ny; vdess[1].nz=-vdess[1].nz;
			vdess[2].nx=-vdess[2].nx; vdess[2].ny=-vdess[2].ny; vdess[2].nz=-vdess[2].nz;

			gtl->m_Buf.Dword(AddVertex(gv->m_Buf,vdess[0]));
			gtl->m_Buf.Dword(AddVertex(gv->m_Buf,vdess[2]));
			gtl->m_Buf.Dword(AddVertex(gv->m_Buf,vdess[1]));
		}
/*		if(twosided) {
			int sme=gtl->m_Buf.Len()-4*3;
			gtl->m_Buf.Dword(*(DWORD *)((BYTE *)gtl->m_Buf.Get()+sme));
			gtl->m_Buf.Dword(*(DWORD *)((BYTE *)gtl->m_Buf.Get()+sme+8));
			gtl->m_Buf.Dword(*(DWORD *)((BYTE *)gtl->m_Buf.Get()+sme+4));
		}*/
/*		vtp[1]=vtp[1]-vtp[0];
		vtp[2]=vtp[2]-vtp[0];
		vtp[0]=CrossProd(vtp[1],vtp[2]);
//		vtp[0].x=vtp[1].y*vtp[2].z-vtp[1].z*vtp[2].y;
//		vtp[0].y=vtp[1].z*vtp[2].x-vtp[1].x*vtp[2].z;
//		vtp[0].z=vtp[1].x*vtp[2].y-vtp[1].y*vtp[2].x;
		vtp[0]=Normalize(vtp[0]);
		vtp[1]=Normalize(tm * mesh->getFaceNormal(i));
		vtp[2]=Normalize(mesh->getFaceNormal(i));
		double d1=sqrt((vtp[0].x-vtp[1].x)*(vtp[0].x-vtp[1].x)+(vtp[0].y-vtp[1].y)*(vtp[0].y-vtp[1].y)+(vtp[0].z-vtp[1].z)*(vtp[0].z-vtp[1].z));
		double d2=sqrt((vtp[0].x-vtp[2].x)*(vtp[0].x-vtp[2].x)+(vtp[0].y-vtp[2].y)*(vtp[0].y-vtp[2].y)+(vtp[0].z-vtp[2].z)*(vtp[0].z-vtp[2].z));
		double d3=sqrt((vtp[0].x-vtp[2].x)*(vtp[0].x-vtp[2].x)+(vtp[0].y-vtp[2].y)*(vtp[0].y-vtp[2].y)+(vtp[0].z-vtp[2].z)*(vtp[0].z-vtp[2].z));*/
	}

	if(m_EdgeExport) {
		for(i=0; i<mesh->getNumFaces(); i++) {
			for(u=0; u<3; u++) {
				int vi2,vi1=mesh->faces[i].v[vx[u]];
				if(u<2) vi2=mesh->faces[i].v[vx[u+1]];
				else vi2=mesh->faces[i].v[vx[0]];

				int cnttri=0;

				for(int p=0; p<mesh->getNumFaces(); p++) {
					for(int k=0; k<3; k++) {
						int vu2,vu1=mesh->faces[p].v[vx[k]];
						if(k<2) vu2=mesh->faces[p].v[vx[k+1]];
						else vu2=mesh->faces[p].v[vx[0]];

						if(((vi1==vu1) && (vi2==vu2)) || ((vi1==vu2) && (vi2==vu1))) {
							cnttri++;

							if(cnttri==2)  {
//								if((m_EdgeExport==1) || ((mesh->getFaceNormal(i).Normalize()-mesh->getFaceNormal(p).Normalize()).LengthSquared()>1e-20/*0.000000001*/)) {
                                double dp=DotProd(mesh->getFaceNormal(i).Normalize(),mesh->getFaceNormal(p).Normalize());
								if((m_EdgeExport==1) || (dp<m_EdgeExportFactor/*0.000000001*/)) {
									gf->m_Buf.Dword((u<<28)|edgeindexlist[i]);
									gf->m_Buf.Dword((k<<28)|edgeindexlist[p]);
								}
							}
						}
					}
				}

				if(cnttri!=2) {
					mesh->faceSel.Set(i);
					TSTR str;
					node->EvalWorldState(timev).obj->GetClassName(str);

					Log(Base::CWStr().Format(L"У грани не 2 триугольника. Face:<i> Cnt: <i>  Name: <s>  Class name: <s>",
						i+1,
						cnttri,
						Base::CWStr(Base::CStr(node->NodeName())).Get(),
						Base::CWStr(Base::CStr(str.data())).Get() 
					));
				}

			}
		}
	}

	if(edgeindexlist) { HFree(edgeindexlist,NULL); edgeindexlist=NULL; }

	if (needDel) delete tri;
}

void EGExp::ExportSimple_Helper(INode *node,TimeValue timev,int frame)
{
	TSTR tstr;
	
	SVOExpMatrixHeader emh;

	if(node->GetUserPropString("Type",tstr)) {
		if(tstr==L"ExportMatrix") {

//            if(m_GroupName.IsEmpty())

            if(!m_GroupName.IsEmpty() && node->GetUserPropString("Group",tstr)) {
                Base::CWStr atstr(Base::CStr(tstr.data()));
                atstr.Trim();
                if(atstr!=m_GroupName) return;
            }

			Matrix3 tm = node->GetObjTMAfterWSM(timev)*m_MatCenter;

			CGroup * gv=GroupFindAdd(Base::CPoint(gt_ExpMatrix,frame),Base::CPoint(gt_ExpMatrix,int(node)));

			ZeroMemory(&emh,sizeof(SVOExpMatrixHeader));
			int etype;
			if(node->GetUserPropInt("Id",etype)) emh.m_Id=etype;
			Base::CWStr ts;
			if(node->GetUserPropString("Name",tstr)) {
				ts.Set(Base::CStr(tstr.data()));
				ts=RenameByCfg(ts);
				if(ts.GetLen()>31) ts.SetLen(31);
				CopyMemory(emh.m_Name,ts.Get(),ts.GetLen()*2);
			}

			gv->m_Buf.Clear();
			gv->m_Buf.BufAdd(&emh,sizeof(SVOExpMatrixHeader));

			SVOMatrix em;
			tm.Orthogonalize();
			tm.NoScale();
			ConvertMatrix(em,tm);
			gv->m_Buf.BufAdd(&em,sizeof(SVOMatrix));

//			Point3 v0=Point3(0,0,0)*tm;
//			Point3 v1=Point3(1,0,0)*tm;
//			Point3 v2=Point3(0,1,0)*tm;
//			Point3 v3=Point3(0,0,1)*tm;
//			Point3 v4=Point3(1,1,1)*tm;
		}
	}
}

bool EGExp::LoadCfgFromScene(INode * node)
{
	if(node==NULL) {
		int numChildren = ip->GetRootNode()->NumberOfChildren();
		for (int idx=0; idx<numChildren; idx++) {
			if(LoadCfgFromScene(ip->GetRootNode()->GetChildNode(idx))) return true;
		}
		return false;
	}

	ObjectState os = node->EvalWorldState(0); 
	if(os.obj) {
		switch(os.obj->SuperClassID()) {
			case HELPER_CLASS_ID: {
				TSTR tstr;
				if(node->GetUserPropString("Type",tstr)) {
					if(tstr==L"ExportCfg") {
						Base::CWStr ss;
						node->GetUserPropBuffer(tstr);
						ss.Set(Base::CStr(tstr.data()));

						m_CfgFromScene.Clear();
						m_CfgFromScene.LoadFromText(ss.Get());
						return true;
					}
				}
				break;
			}
		}
	}

	for (int c = 0; c < node->NumberOfChildren(); c++) {
		if(LoadCfgFromScene(node->GetChildNode(c))) return true;
	}
	return false;
}

Base::CWStr EGExp::RenameByCfg(Base::CWStr & str)
{
	Base::CBlockPar * bp=m_Set->BlockGetNE(L"Rename");
	if(bp==NULL) return str;
	if(bp->ParCount(str)<=0) return str;
	return bp->Par(str);
}

/****************************************************************************

  GeomObject output
  
****************************************************************************/

void EGExp::ExportGeomObject(INode* node, int indentLevel)
{
	ObjectState os = node->EvalWorldState(GetStaticFrame());
	if (!os.obj)
		return;
	
	// Targets are actually geomobjects, but we will export them
	// from the camera and light objects, so we skip them here.
	if (os.obj->ClassID() == Class_ID(TARGET_CLASS_ID, 0))
		return;
	
	
	TSTR indent = GetIndent(indentLevel);
	
	ExportNodeHeader(node, ID_GEOMETRY, indentLevel);
	
	ExportNodeTM(node, indentLevel);
	
	if (GetIncludeMesh()) {
		ExportMesh(node, GetStaticFrame(), indentLevel);
	}

	// Node properties (only for geomobjects)
	fprintf(pStream, "%s\t%s %d\n", indent.data(), ID_PROP_MOTIONBLUR, node->MotBlur());
	fprintf(pStream, "%s\t%s %d\n", indent.data(), ID_PROP_CASTSHADOW, node->CastShadows());
	fprintf(pStream, "%s\t%s %d\n", indent.data(), ID_PROP_RECVSHADOW, node->RcvShadows());

	// Export the visibility track
	Control* visCont = node->GetVisController();
	if (visCont) {
		fprintf(pStream, "%s\t%s {\n", indent.data(), ID_VISIBILITY_TRACK);
		fprintf(pStream, "\t}\n");
	}

	if (GetIncludeMtl()) {
		ExportMaterial(node, indentLevel);
	}

	if (GetIncludeMeshAnim()) {
		ExportAnimMesh(node, indentLevel);
	}
	
	if (GetIncludeIKJoints()) {
		ExportIKJoints(node, indentLevel);
	}
	
	fprintf(pStream,"%s}\n", indent.data());
}

/****************************************************************************

  Shape output
  
****************************************************************************/

void EGExp::ExportShapeObject(INode* node, int indentLevel)
{
	ExportNodeHeader(node, ID_SHAPE, indentLevel);
	ExportNodeTM(node, indentLevel);
	TimeValue t = GetStaticFrame();
	Matrix3 tm = node->GetObjTMAfterWSM(t);

	TSTR indent = GetIndent(indentLevel);
	
	ObjectState os = node->EvalWorldState(t);
	if (!os.obj || os.obj->SuperClassID()!=SHAPE_CLASS_ID) {
		fprintf(pStream,"%s}\n", indent.data());
		return;
	}
	
	ShapeObject* shape = (ShapeObject*)os.obj;
	PolyShape pShape;
	int numLines;

	// We will output ahspes as a collection of polylines.
	// Each polyline contains collection of line segments.
	shape->MakePolyShape(t, pShape);
	numLines = pShape.numLines;
	
	fprintf(pStream,"%s\t%s %d\n", indent.data(), ID_SHAPE_LINECOUNT, numLines);
	
	for(int poly = 0; poly < numLines; poly++) {
		fprintf(pStream,"%s\t%s %d {\n", indent.data(), ID_SHAPE_LINE, poly);
		DumpPoly(&pShape.lines[poly], tm, indentLevel);
		fprintf(pStream, "%s\t}\n", indent.data());
	}
	

	fprintf(pStream,"%s}\n", indent.data());
}

void EGExp::DumpPoly(PolyLine* line, Matrix3 tm, int indentLevel)
{
	int numVerts = line->numPts;
	
	TSTR indent = GetIndent(indentLevel);
	
	if(line->IsClosed()) {
		fprintf(pStream,"%s\t\t%s\n", indent.data(), ID_SHAPE_CLOSED);
	}
	
	fprintf(pStream,"%s\t\t%s %d\n", indent.data(), ID_SHAPE_VERTEXCOUNT, numVerts);
	
	// We differ between true and interpolated knots
	for (int i=0; i<numVerts; i++) {
		PolyPt* pt = &line->pts[i];
		if (pt->flags & POLYPT_KNOT) {
			fprintf(pStream,"%s\t\t%s\t%d\t%s\n", indent.data(), ID_SHAPE_VERTEX_KNOT, i,
				Format(tm * pt->p));
		}
		else {
			fprintf(pStream,"%s\t\t%s\t%d\t%s\n", indent.data(), ID_SHAPE_VERTEX_INTERP,
				i, Format(tm * pt->p));
		}
		
	}
}

/****************************************************************************

  Light output
  
****************************************************************************/

void EGExp::ExportLightObject(INode* node, int indentLevel)
{
	TimeValue t = GetStaticFrame();
	TSTR indent = GetIndent(indentLevel);

	ExportNodeHeader(node, ID_LIGHT, indentLevel);
	
	ObjectState os = node->EvalWorldState(t);
	if (!os.obj) {
		fprintf(pStream, "%s}\n", indent.data());
		return;
	}
	
	GenLight* light = (GenLight*)os.obj;
	struct LightState ls;
	Interval valid = FOREVER;
	Interval animRange = ip->GetAnimRange();

	light->EvalLightState(t, valid, &ls);

	// This is part os the lightState, but it doesn't
	// make sense to output as an animated setting so
	// we dump it outside of ExportLightSettings()

	fprintf(pStream, "%s\t%s ", indent.data(), ID_LIGHT_TYPE);
	switch(ls.type) {
	case OMNI_LIGHT:  fprintf(pStream, "%s\n", ID_LIGHT_TYPE_OMNI); break;
	case TSPOT_LIGHT: fprintf(pStream, "%s\n", ID_LIGHT_TYPE_TARG);  break;
	case DIR_LIGHT:   fprintf(pStream, "%s\n", ID_LIGHT_TYPE_DIR); break;
	case FSPOT_LIGHT: fprintf(pStream, "%s\n", ID_LIGHT_TYPE_FREE); break;
	}

	ExportNodeTM(node, indentLevel);
	// If we have a target object, export Node TM for the target too.
	INode* target = node->GetTarget();
	if (target) {
		ExportNodeTM(target, indentLevel);
	}

	int shadowMethod = light->GetShadowMethod();
	fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_LIGHT_SHADOWS,
			shadowMethod == LIGHTSHADOW_NONE ? ID_LIGHT_SHAD_OFF :
			shadowMethod == LIGHTSHADOW_MAPPED ? ID_LIGHT_SHAD_MAP :
			ID_LIGHT_SHAD_RAY);

	
	fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_LIGHT_USELIGHT, Format(light->GetUseLight()));
	
	fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_LIGHT_SPOTSHAPE, 
		light->GetSpotShape() == RECT_LIGHT ? ID_LIGHT_SHAPE_RECT : ID_LIGHT_SHAPE_CIRC);

	fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_LIGHT_USEGLOBAL, Format(light->GetUseGlobal()));
	fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_LIGHT_ABSMAPBIAS, Format(light->GetAbsMapBias()));
	fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_LIGHT_OVERSHOOT, Format(light->GetOvershoot()));

	ExclList* el = light->GetExclList();  // DS 8/31/00 . switched to NodeIDTab from NameTab
	if (el->Count()) {
		fprintf(pStream,"%s\t%s {\n", indent.data(), ID_LIGHT_EXCLUSIONLIST);
		fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_LIGHT_NUMEXCLUDED, Format(el->Count()));
		fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_LIGHT_EXCLINCLUDE, Format(el->TestFlag(NT_INCLUDE)));
		fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_LIGHT_EXCL_AFFECT_ILLUM, Format(el->TestFlag(NT_AFFECT_ILLUM)));
		fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_LIGHT_EXCL_AFFECT_SHAD, Format(el->TestFlag(NT_AFFECT_SHADOWCAST)));
		for (int nameid = 0; nameid < el->Count(); nameid++) {
			INode *n = (*el)[nameid];	// DS 8/31/00
			if (n)
				fprintf(pStream,"%s\t\t%s \"%s\"\n", indent.data(), ID_LIGHT_EXCLUDED, n->GetName());
			}
		fprintf(pStream,"%s\t}\n", indent.data());
	}

	// Export light settings for frame 0
	ExportLightSettings(&ls, light, t, indentLevel);

	// Export animated light settings
	if (!valid.InInterval(animRange) && GetIncludeCamLightAnim()) {
		fprintf(pStream,"%s\t%s {\n", indent.data(), ID_LIGHT_ANIMATION);

		TimeValue t = animRange.Start();
		
		while (1) {
			valid = FOREVER; // Extend the validity interval so the camera can shrink it.
			light->EvalLightState(t, valid, &ls);

			t = valid.Start() < animRange.Start() ? animRange.Start() : valid.Start();
			
			// Export the light settings at this frame
			ExportLightSettings(&ls, light, t, indentLevel+1);
			
			if (valid.End() >= animRange.End()) {
				break;
			}
			else {
				t = (valid.End()/GetTicksPerFrame()+GetMeshFrameStep()) * GetTicksPerFrame();
			}
		}

		fprintf(pStream,"%s\t}\n", indent.data());
	}

	// Export animation keys for the light node
	
	fprintf(pStream,"%s}\n", indent.data());
}

void EGExp::ExportLightSettings(LightState* ls, GenLight* light, TimeValue t, int indentLevel)
{
	TSTR indent = GetIndent(indentLevel);

	fprintf(pStream,"%s\t%s {\n", indent.data(), ID_LIGHT_SETTINGS);

	// Frame #
	fprintf(pStream, "%s\t\t%s %d\n",indent.data(), ID_TIMEVALUE, t);

	fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_LIGHT_COLOR, Format(ls->color));
	fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_LIGHT_INTENS, Format(ls->intens));
	fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_LIGHT_ASPECT, Format(ls->aspect));
	
	if (ls->type != OMNI_LIGHT) {
		fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_LIGHT_HOTSPOT, Format(ls->hotsize));
		fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_LIGHT_FALLOFF, Format(ls->fallsize));
	}
	if (ls->type != DIR_LIGHT && ls->useAtten) {
		fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_LIGHT_ATTNSTART, Format(ls->attenStart));
		fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_LIGHT_ATTNEND,	Format(ls->attenEnd));
	}

	fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_LIGHT_TDIST, Format(light->GetTDist(t, FOREVER)));
	fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_LIGHT_MAPBIAS, Format(light->GetMapBias(t, FOREVER)));
	fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_LIGHT_MAPRANGE, Format(light->GetMapRange(t, FOREVER)));
	fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_LIGHT_MAPSIZE, Format(light->GetMapSize(t, FOREVER)));
	fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_LIGHT_RAYBIAS, Format(light->GetRayBias(t, FOREVER)));

	fprintf(pStream,"%s\t}\n", indent.data());
}


/****************************************************************************

  Camera output
  
****************************************************************************/

void EGExp::ExportCameraObject(INode* node, int indentLevel)
{
	TSTR indent = GetIndent(indentLevel);

	ExportNodeHeader(node, ID_CAMERA, indentLevel);

	INode* target = node->GetTarget();
	if (target) {
		fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_CAMERA_TYPE, ID_CAMERATYPE_TARGET);
	}
	else {
		fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_CAMERA_TYPE, ID_CAMERATYPE_FREE);
	}


	ExportNodeTM(node, indentLevel);
	// If we have a target object, export animation keys for the target too.
	if (target) {
		ExportNodeTM(target, indentLevel);
	}
	
	CameraState cs;
	TimeValue t = GetStaticFrame();
	Interval valid = FOREVER;
	// Get animation range
	Interval animRange = ip->GetAnimRange();
	
	ObjectState os = node->EvalWorldState(t);
	CameraObject *cam = (CameraObject *)os.obj;
	
	cam->EvalCameraState(t,valid,&cs);
	
	ExportCameraSettings(&cs, cam, t, indentLevel);

	// Export animated camera settings
	if (!valid.InInterval(animRange) && GetIncludeCamLightAnim()) {

		fprintf(pStream,"%s\t%s {\n", indent.data(), ID_CAMERA_ANIMATION);

		TimeValue t = animRange.Start();
		
		while (1) {
			valid = FOREVER; // Extend the validity interval so the camera can shrink it.
			cam->EvalCameraState(t,valid,&cs);

			t = valid.Start() < animRange.Start() ? animRange.Start() : valid.Start();
			
			// Export the camera settings at this frame
			ExportCameraSettings(&cs, cam, t, indentLevel+1);
			
			if (valid.End() >= animRange.End()) {
				break;
			}
			else {
				t = (valid.End()/GetTicksPerFrame()+GetMeshFrameStep()) * GetTicksPerFrame();
			}
		}

		fprintf(pStream,"%s\t}\n", indent.data());
	}
	
	// Export animation keys for the light node

	fprintf(pStream,"%s}\n", indent.data());
}

void EGExp::ExportCameraSettings(CameraState* cs, CameraObject* cam, TimeValue t, int indentLevel)
{
	TSTR indent = GetIndent(indentLevel);

	fprintf(pStream,"%s\t%s {\n", indent.data(), ID_CAMERA_SETTINGS);

	// Frame #
	fprintf(pStream, "%s\t\t%s %d\n", indent.data(), ID_TIMEVALUE, t);

	if (cs->manualClip) {
		fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_CAMERA_HITHER, Format(cs->hither));
		fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_CAMERA_YON, Format(cs->yon));
	}

	fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_CAMERA_NEAR, Format(cs->nearRange));
	fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_CAMERA_FAR, Format(cs->farRange));
	fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_CAMERA_FOV, Format(cs->fov));
	fprintf(pStream,"%s\t\t%s %s\n", indent.data(), ID_CAMERA_TDIST, Format(cam->GetTDist(t)));

	fprintf(pStream,"%s\t}\n",indent.data());
}


/****************************************************************************

  Helper object output
  
****************************************************************************/

void EGExp::ExportHelperObject(INode* node, int indentLevel)
{
	TSTR indent = GetIndent(indentLevel);
	ExportNodeHeader(node, ID_HELPER, indentLevel);

	// We don't really know what kind of helper this is, but by exporting
	// the Classname of the helper object, the importer has a chance to
	// identify it.
	Object* helperObj = node->EvalWorldState(0).obj;
	if (helperObj) {
		TSTR className;
		helperObj->GetClassName(className);
		fprintf(pStream,"%s\t%s \"%s\"\n", indent.data(), ID_HELPER_CLASS, className);
	}

	ExportNodeTM(node, indentLevel);

	if (helperObj) {
		TimeValue	t = GetStaticFrame();
		Matrix3		oTM = node->GetObjectTM(t);
		Box3		bbox;

		helperObj->GetDeformBBox(t, bbox, &oTM);

		fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_BOUNDINGBOX_MIN, Format(bbox.pmin));
		fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_BOUNDINGBOX_MAX, Format(bbox.pmax));
	}


	fprintf(pStream,"%s}\n", indent.data());
}


/****************************************************************************

  Node Header
  
****************************************************************************/

// The Node Header consists of node type (geometry, helper, camera etc.)
// node name and parent node
void EGExp::ExportNodeHeader(INode* node, TCHAR* type, int indentLevel)
{
	TSTR indent = GetIndent(indentLevel);
	
	// Output node header and object type 
	fprintf(pStream,"%s%s {\n", indent.data(), type);
	
	// Node name
	fprintf(pStream,"%s\t%s \"%s\"\n", indent.data(), ID_NODE_NAME, FixupName(node->GetName()));
	
	//  If the node is linked, export parent node name
	INode* parent = node->GetParentNode();
	if (parent && !parent->IsRootNode()) {
		fprintf(pStream,"%s\t%s \"%s\"\n", indent.data(), ID_NODE_PARENT, FixupName(parent->GetName()));
	}
}


/****************************************************************************

  Node Transformation
  
****************************************************************************/

void EGExp::ExportNodeTM(INode* node, int indentLevel)
{
	Matrix3 pivot = node->GetNodeTM(GetStaticFrame());
	TSTR indent = GetIndent(indentLevel);
	
	fprintf(pStream,"%s\t%s {\n", indent.data(), ID_NODE_TM);
	
	// Node name
	// We export the node name together with the nodeTM, because some objects
	// (like a camera or a spotlight) has an additional node (the target).
	// In that case the nodeTM and the targetTM is exported after eachother
	// and the nodeName is how you can tell them apart.
	fprintf(pStream,"%s\t\t%s \"%s\"\n", indent.data(), ID_NODE_NAME, FixupName(node->GetName()));

	// Export TM inheritance flags
	DWORD iFlags = node->GetTMController()->GetInheritanceFlags();
	fprintf(pStream,"%s\t\t%s %d %d %d\n", indent.data(), ID_INHERIT_POS,
		INHERIT_POS_X & iFlags ? 1 : 0,
		INHERIT_POS_Y & iFlags ? 1 : 0,
		INHERIT_POS_Z & iFlags ? 1 : 0);

	fprintf(pStream,"%s\t\t%s %d %d %d\n", indent.data(), ID_INHERIT_ROT,
		INHERIT_ROT_X & iFlags ? 1 : 0,
		INHERIT_ROT_Y & iFlags ? 1 : 0,
		INHERIT_ROT_Z & iFlags ? 1 : 0);

	fprintf(pStream,"%s\t\t%s %d %d %d\n", indent.data(), ID_INHERIT_SCL,
		INHERIT_SCL_X & iFlags ? 1 : 0,
		INHERIT_SCL_Y & iFlags ? 1 : 0,
		INHERIT_SCL_Z & iFlags ? 1 : 0);

	// Dump the full matrix
	DumpMatrix3(&pivot, indentLevel+2);
	
	fprintf(pStream,"%s\t}\n", indent.data());
}

/****************************************************************************

  Animation output
  
****************************************************************************/

// If the object is animated, then we will output the entire mesh definition
// for every specified frame. This can result in, dare I say, rather large files.
//
// Many target systems (including MAX itself!) cannot always read back this
// information. If the objects maintains the same number of verices it can be
// imported as a morph target, but if the number of vertices are animated it
// could not be read back in withou special tricks.
// Since the target system for this exporter is unknown, it is up to the
// user (or developer) to make sure that the data conforms with the target system.

void EGExp::ExportAnimMesh(INode* node, int indentLevel)
{
	ObjectState os = node->EvalWorldState(GetStaticFrame());
	if (!os.obj)
		return;
	
	TSTR indent = GetIndent(indentLevel);
	
	// Get animation range
	Interval animRange = ip->GetAnimRange();
	// Get validity of the object
	Interval objRange = os.obj->ObjectValidity(GetStaticFrame());
	
	// If the animation range is not fully included in the validity
	// interval of the object, then we're animated.
	if (!objRange.InInterval(animRange)) {
		
		fprintf(pStream,"%s\t%s {\n", indent.data(), ID_MESH_ANIMATION);
		
		TimeValue t = animRange.Start();
		
		while (1) {
			// This may seem strange, but the object in the pipeline
			// might not be valid anymore.
			os = node->EvalWorldState(t);
			objRange = os.obj->ObjectValidity(t);
			t = objRange.Start() < animRange.Start() ? animRange.Start() : objRange.Start();
			
			// Export the mesh definition at this frame
			ExportMesh(node, t, indentLevel+1);
			
			if (objRange.End() >= animRange.End()) {
				break;
			}
			else {
				t = (objRange.End()/GetTicksPerFrame()+GetMeshFrameStep()) * GetTicksPerFrame();
			}
		}
		fprintf(pStream,"%s\t}\n", indent.data());
	}
}


/****************************************************************************

  Mesh output
  
****************************************************************************/

void EGExp::ExportMesh(INode* node, TimeValue t, int indentLevel)
{
	int i;
	Mtl* nodeMtl = node->GetMtl();
	Matrix3 tm = node->GetObjTMAfterWSM(t);
	BOOL negScale = TMNegParity(tm);
	int vx1, vx2, vx3;
	TSTR indent;
	
	indent = GetIndent(indentLevel+1);
	
	ObjectState os = node->EvalWorldState(t);
	if (!os.obj || os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) {
		return; // Safety net. This shouldn't happen.
	}
	
	// Order of the vertices. Get 'em counter clockwise if the objects is
	// negatively scaled.
	if (negScale) {
		vx1 = 2;
		vx2 = 1;
		vx3 = 0;
	}
	else {
		vx1 = 0;
		vx2 = 1;
		vx3 = 2;
	}
	
	BOOL needDel;
	TriObject* tri = GetTriObjectFromNode(node, t, needDel);
	if (!tri) {
		return;
	}
	
	Mesh* mesh = &tri->GetMesh();
	
	mesh->buildNormals();
	
	fprintf(pStream, "%s%s {\n",indent.data(),  ID_MESH);
	fprintf(pStream, "%s\t%s %d\n",indent.data(), ID_TIMEVALUE, t);
	fprintf(pStream, "%s\t%s %d\n",indent.data(), ID_MESH_NUMVERTEX, mesh->getNumVerts());
    fprintf(pStream, "%s\t%s %d\n",indent.data(), ID_MESH_NUMFACES, mesh->getNumFaces());
	
	// Export the vertices
	fprintf(pStream,"%s\t%s {\n",indent.data(), ID_MESH_VERTEX_LIST);
	for (i=0; i<mesh->getNumVerts(); i++) {
		Point3 v = tm * mesh->verts[i];
		fprintf(pStream, "%s\t\t%s %4d\t%s\n",indent.data(), ID_MESH_VERTEX, i, Format(v));
	}
	fprintf(pStream,"%s\t}\n",indent.data()); // End vertex list
	
	// To determine visibility of a face, get the vertices in clockwise order.
	// If the objects has a negative scaling, we must compensate for that by
	// taking the vertices counter clockwise
	fprintf(pStream, "%s\t%s {\n",indent.data(), ID_MESH_FACE_LIST);
	for (i=0; i<mesh->getNumFaces(); i++) {
		fprintf(pStream,"%s\t\t%s %4d:    A: %4d B: %4d C: %4d AB: %4d BC: %4d CA: %4d",
			indent.data(),
			ID_MESH_FACE, i,
			mesh->faces[i].v[vx1],
			mesh->faces[i].v[vx2],
			mesh->faces[i].v[vx3],
			mesh->faces[i].getEdgeVis(vx1) ? 1 : 0,
			mesh->faces[i].getEdgeVis(vx2) ? 1 : 0,
			mesh->faces[i].getEdgeVis(vx3) ? 1 : 0);
		fprintf(pStream,"\t %s ", ID_MESH_SMOOTHING);
		for (int j=0; j<32; j++) {
			if (mesh->faces[i].smGroup & (1<<j)) {
				if (mesh->faces[i].smGroup>>(j+1)) {
					fprintf(pStream,"%d,",j+1); // Add extra comma
				} else {
					fprintf(pStream,"%d ",j+1);
				}
			}
		}
		
		// This is the material ID for the face.
		// Note: If you use this you should make sure that the material ID
		// is not larger than the number of sub materials in the material.
		// The standard approach is to use a modulus function to bring down
		// the material ID.
		fprintf(pStream,"\t%s %d", ID_MESH_MTLID, mesh->faces[i].getMatID());
		
		fprintf(pStream,"\n");
	}
	fprintf(pStream,"%s\t}\n", indent.data()); // End face list
	
	// Export face map texcoords if we have them...
	if (GetIncludeTextureCoords() && !CheckForAndExportFaceMap(nodeMtl, mesh, indentLevel+1)) {
		// If not, export standard tverts
		int numTVx = mesh->getNumTVerts();

		fprintf(pStream, "%s\t%s %d\n",indent.data(), ID_MESH_NUMTVERTEX, numTVx);

		if (numTVx) {
			fprintf(pStream,"%s\t%s {\n",indent.data(), ID_MESH_TVERTLIST);
			for (i=0; i<numTVx; i++) {
				UVVert tv = mesh->tVerts[i];
				fprintf(pStream, "%s\t\t%s %d\t%s\n",indent.data(), ID_MESH_TVERT, i, Format(tv));
			}
			fprintf(pStream,"%s\t}\n",indent.data());
			
			fprintf(pStream, "%s\t%s %d\n",indent.data(), ID_MESH_NUMTVFACES, mesh->getNumFaces());

			fprintf(pStream, "%s\t%s {\n",indent.data(), ID_MESH_TFACELIST);
			for (i=0; i<mesh->getNumFaces(); i++) {
				fprintf(pStream,"%s\t\t%s %d\t%d\t%d\t%d\n",
					indent.data(),
					ID_MESH_TFACE, i,
					mesh->tvFace[i].t[vx1],
					mesh->tvFace[i].t[vx2],
					mesh->tvFace[i].t[vx3]);
			}
			fprintf(pStream, "%s\t}\n",indent.data());
		}

		// CCJ 3/9/99
		// New for R3 - Additional mapping channels
		for (int mp = 2; mp < MAX_MESHMAPS-1; mp++) {
			if (mesh->mapSupport(mp)) {

				fprintf(pStream, "%s\t%s %d {\n",indent.data(), ID_MESH_MAPPINGCHANNEL, mp);


				int numTVx = mesh->getNumMapVerts(mp);
				fprintf(pStream, "%s\t\t%s %d\n",indent.data(), ID_MESH_NUMTVERTEX, numTVx);

				if (numTVx) {
					fprintf(pStream,"%s\t\t%s {\n",indent.data(), ID_MESH_TVERTLIST);
					for (i=0; i<numTVx; i++) {
						UVVert tv = mesh->mapVerts(mp)[i];
						fprintf(pStream, "%s\t\t\t%s %d\t%s\n",indent.data(), ID_MESH_TVERT, i, Format(tv));
					}
					fprintf(pStream,"%s\t\t}\n",indent.data());
					
					fprintf(pStream, "%s\t\t%s %d\n",indent.data(), ID_MESH_NUMTVFACES, mesh->getNumFaces());

					fprintf(pStream, "%s\t\t%s {\n",indent.data(), ID_MESH_TFACELIST);
					for (i=0; i<mesh->getNumFaces(); i++) {
						fprintf(pStream,"%s\t\t\t%s %d\t%d\t%d\t%d\n",
							indent.data(),
							ID_MESH_TFACE, i,
							mesh->mapFaces(mp)[i].t[vx1],
							mesh->mapFaces(mp)[i].t[vx2],
							mesh->mapFaces(mp)[i].t[vx3]);
					}
					fprintf(pStream, "%s\t\t}\n",indent.data());
				}
				fprintf(pStream, "%s\t}\n",indent.data());
			}
		}
	}

	// Export color per vertex info
	if (GetIncludeVertexColors()) {
		int numCVx = mesh->numCVerts;

		fprintf(pStream, "%s\t%s %d\n",indent.data(), ID_MESH_NUMCVERTEX, numCVx);
		if (numCVx) {
			fprintf(pStream,"%s\t%s {\n",indent.data(), ID_MESH_CVERTLIST);
			for (i=0; i<numCVx; i++) {
				Point3 vc = mesh->vertCol[i];
				fprintf(pStream, "%s\t\t%s %d\t%s\n",indent.data(), ID_MESH_VERTCOL, i, Format(vc));
			}
			fprintf(pStream,"%s\t}\n",indent.data());
			
			fprintf(pStream, "%s\t%s %d\n",indent.data(), ID_MESH_NUMCVFACES, mesh->getNumFaces());

			fprintf(pStream, "%s\t%s {\n",indent.data(), ID_MESH_CFACELIST);
			for (i=0; i<mesh->getNumFaces(); i++) {
				fprintf(pStream,"%s\t\t%s %d\t%d\t%d\t%d\n",
					indent.data(),
					ID_MESH_CFACE, i,
					mesh->vcFace[i].t[vx1],
					mesh->vcFace[i].t[vx2],
					mesh->vcFace[i].t[vx3]);
			}
			fprintf(pStream, "%s\t}\n",indent.data());
		}
	}
	
	if (GetIncludeNormals()) {
		// Export mesh (face + vertex) normals
		fprintf(pStream, "%s\t%s {\n",indent.data(), ID_MESH_NORMALS);
		
		Point3 fn;  // Face normal
		Point3 vn;  // Vertex normal
		int  vert;
		Face* f;
		
		// Face and vertex normals.
		// In MAX a vertex can have more than one normal (but doesn't always have it).
		// This is depending on the face you are accessing the vertex through.
		// To get all information we need to export all three vertex normals
		// for every face.
		for (i=0; i<mesh->getNumFaces(); i++) {
			f = &mesh->faces[i];
			fn = mesh->getFaceNormal(i);
			fprintf(pStream,"%s\t\t%s %d\t%s\n", indent.data(), ID_MESH_FACENORMAL, i, Format(fn));
			
			vert = f->getVert(vx1);
			vn = GetVertexNormal(mesh, i, mesh->getRVertPtr(vert));
			fprintf(pStream,"%s\t\t\t%s %d\t%s\n",indent.data(), ID_MESH_VERTEXNORMAL, vert, Format(vn));
			
			vert = f->getVert(vx2);
			vn = GetVertexNormal(mesh, i, mesh->getRVertPtr(vert));
			fprintf(pStream,"%s\t\t\t%s %d\t%s\n",indent.data(), ID_MESH_VERTEXNORMAL, vert, Format(vn));
			
			vert = f->getVert(vx3);
			vn = GetVertexNormal(mesh, i, mesh->getRVertPtr(vert));
			fprintf(pStream,"%s\t\t\t%s %d\t%s\n",indent.data(), ID_MESH_VERTEXNORMAL, vert, Format(vn));
		}
		
		fprintf(pStream, "%s\t}\n",indent.data());
	}
	
	fprintf(pStream, "%s}\n",indent.data());
	
	if (needDel) {
		delete tri;
	}
}

Point3 EGExp::GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv)
{
	Face* f = &mesh->faces[faceNo];
	DWORD smGroup = f->smGroup;
	int numNormals;
	Point3 vertexNormal;
	
	// Is normal specified
	// SPCIFIED is not currently used, but may be used in future versions.
	if (rv->rFlags & SPECIFIED_NORMAL) {
		vertexNormal = rv->rn.getNormal();
	}
	// If normal is not specified it's only available if the face belongs
	// to a smoothing group
	else if ((numNormals = rv->rFlags & NORCT_MASK) && smGroup) {
		// If there is only one vertex is found in the rn member.
		if (numNormals == 1) {
			vertexNormal = rv->rn.getNormal();
		}
		else {
			// If two or more vertices are there you need to step through them
			// and find the vertex with the same smoothing group as the current face.
			// You will find multiple normals in the ern member.
			for (int i = 0; i < numNormals; i++) {
				if (rv->ern[i].getSmGroup() & smGroup) {
					vertexNormal = rv->ern[i].getNormal();
				}
			}
		}
	}
	else {
		// Get the normal from the Face if no smoothing groups are there
		vertexNormal = mesh->getFaceNormal(faceNo);
	}
	
	return vertexNormal;
}

/****************************************************************************

  Inverse Kinematics (IK) Joint information
  
****************************************************************************/

void EGExp::ExportIKJoints(INode* node, int indentLevel)
{
	Control* cont;
	TSTR indent = GetIndent(indentLevel);

	if (node->TestAFlag(A_INODE_IK_TERMINATOR)) 
		fprintf(pStream,"%s\t%s\n", indent.data(), ID_IKTERMINATOR);

	if(node->TestAFlag(A_INODE_IK_POS_PINNED))
		fprintf(pStream,"%s\t%s\n", indent.data(), ID_IKPOS_PINNED);

	if(node->TestAFlag(A_INODE_IK_ROT_PINNED))
		fprintf(pStream,"%s\t%s\n", indent.data(), ID_IKROT_PINNED);

	// Position joint
	cont = node->GetTMController()->GetPositionController();
	if (cont) {
		JointParams* joint = (JointParams*)cont->GetProperty(PROPID_JOINTPARAMS);
		if (joint && !joint->IsDefault()) {
			// Has IK Joints!!!
			fprintf(pStream,"%s\t%s {\n", indent.data(), ID_IKJOINT);
			DumpJointParams(joint, indentLevel+1);
			fprintf(pStream,"%s\t}\n", indent.data());
		}
	}

	// Rotational joint
	cont = node->GetTMController()->GetRotationController();
	if (cont) {
		JointParams* joint = (JointParams*)cont->GetProperty(PROPID_JOINTPARAMS);
		if (joint && !joint->IsDefault()) {
			// Has IK Joints!!!
			fprintf(pStream,"%s\t%s {\n", indent.data(), ID_IKJOINT);
			DumpJointParams(joint, indentLevel+1);
			fprintf(pStream,"%s\t}\n", indent.data());
		}
	}
}

void EGExp::DumpJointParams(JointParams* joint, int indentLevel)
{
	TSTR indent = GetIndent(indentLevel);
	float scale = joint->scale;

	fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_IKTYPE,   joint->Type() == JNT_POS ? ID_IKTYPEPOS : ID_IKTYPEROT);
	fprintf(pStream,"%s\t%s %d\n", indent.data(), ID_IKDOF,    joint->dofs);

	fprintf(pStream,"%s\t%s %d\n", indent.data(), ID_IKXACTIVE,  joint->flags & JNT_XACTIVE  ? 1 : 0);
	fprintf(pStream,"%s\t%s %d\n", indent.data(), ID_IKYACTIVE,  joint->flags & JNT_YACTIVE  ? 1 : 0);
	fprintf(pStream,"%s\t%s %d\n", indent.data(), ID_IKZACTIVE,  joint->flags & JNT_ZACTIVE  ? 1 : 0);

	fprintf(pStream,"%s\t%s %d\n", indent.data(), ID_IKXLIMITED, joint->flags & JNT_XLIMITED ? 1 : 0);
	fprintf(pStream,"%s\t%s %d\n", indent.data(), ID_IKYLIMITED, joint->flags & JNT_YLIMITED ? 1 : 0);
	fprintf(pStream,"%s\t%s %d\n", indent.data(), ID_IKZLIMITED, joint->flags & JNT_ZLIMITED ? 1 : 0);

	fprintf(pStream,"%s\t%s %d\n", indent.data(), ID_IKXEASE,    joint->flags & JNT_XEASE    ? 1 : 0);
	fprintf(pStream,"%s\t%s %d\n", indent.data(), ID_IKYEASE,    joint->flags & JNT_YEASE    ? 1 : 0);
	fprintf(pStream,"%s\t%s %d\n", indent.data(), ID_IKZEASE,    joint->flags & JNT_ZEASE    ? 1 : 0);

	fprintf(pStream,"%s\t%s %d\n", indent.data(), ID_IKLIMITEXACT, joint->flags & JNT_LIMITEXACT ? 1 : 0);

	for (int i=0; i<joint->dofs; i++) {
		fprintf(pStream,"%s\t%s %d %s %s %s\n", indent.data(), ID_IKJOINTINFO, i, Format(joint->min[i]), Format(joint->max[i]), Format(joint->damping[i]));
	}

}

/****************************************************************************

  Material and Texture Export
  
****************************************************************************/

void EGExp::ExportMaterialList()
{
	if (!GetIncludeMtl()) {
		return;
	}

	fprintf(pStream, "%s {\n", ID_MATERIAL_LIST);

	int numMtls = mtlList.Count();
	fprintf(pStream, "\t%s %d\n", ID_MATERIAL_COUNT, numMtls);

	for (int i=0; i<numMtls; i++) {
		DumpMaterial(mtlList.GetMtl(i), i, -1, 0);
	}

	fprintf(pStream, "}\n");
}

void EGExp::ExportMaterial(INode* node, int indentLevel)
{
	Mtl* mtl = node->GetMtl();
	
	TSTR indent = GetIndent(indentLevel);
	
	// If the node does not have a material, export the wireframe color
	if (mtl) {
		int mtlID = mtlList.GetMtlID(mtl);
		if (mtlID >= 0) {
			fprintf(pStream,"%s\t%s %d\n", indent.data(), ID_MATERIAL_REF, mtlID);
		}
	}
	else {
		DWORD c = node->GetWireColor();
		fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_WIRECOLOR,
			Format(Color(GetRValue(c)/255.0f, GetGValue(c)/255.0f, GetBValue(c)/255.0f)));
	}
}

void EGExp::DumpMaterial(Mtl* mtl, int mtlID, int subNo, int indentLevel)
{
	int i;
	TimeValue t = GetStaticFrame();
	
	if (!mtl) return;
	
	TSTR indent = GetIndent(indentLevel+1);
	
	TSTR className;
	mtl->GetClassName(className);
	
	
	if (subNo == -1) {
		// Top level material
		fprintf(pStream,"%s%s %d {\n",indent.data(), ID_MATERIAL, mtlID);
	}
	else {
		fprintf(pStream,"%s%s %d {\n",indent.data(), ID_SUBMATERIAL, subNo);
	}
	fprintf(pStream,"%s\t%s \"%s\"\n",indent.data(), ID_MATNAME, FixupName(mtl->GetName()));
	fprintf(pStream,"%s\t%s \"%s\"\n",indent.data(), ID_MATCLASS, FixupName(className));
	
	// We know the Standard material, so we can get some extra info
	if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
		StdMat* std = (StdMat*)mtl;

		fprintf(pStream,"%s\t%s %s\n",indent.data(), ID_AMBIENT, Format(std->GetAmbient(t)));
		fprintf(pStream,"%s\t%s %s\n",indent.data(), ID_DIFFUSE, Format(std->GetDiffuse(t)));
		fprintf(pStream,"%s\t%s %s\n",indent.data(), ID_SPECULAR, Format(std->GetSpecular(t)));
		fprintf(pStream,"%s\t%s %s\n",indent.data(), ID_SHINE, Format(std->GetShininess(t)));
		fprintf(pStream,"%s\t%s %s\n",indent.data(), ID_SHINE_STRENGTH, Format(std->GetShinStr(t)));
		fprintf(pStream,"%s\t%s %s\n",indent.data(), ID_TRANSPARENCY, Format(std->GetXParency(t)));
		fprintf(pStream,"%s\t%s %s\n",indent.data(), ID_WIRESIZE, Format(std->WireSize(t)));

		fprintf(pStream,"%s\t%s ", indent.data(), ID_SHADING);
		switch(std->GetShading()) {
		case SHADE_CONST: fprintf(pStream,"%s\n", ID_MAT_SHADE_CONST); break;
		case SHADE_PHONG: fprintf(pStream,"%s\n", ID_MAT_SHADE_PHONG); break;
		case SHADE_METAL: fprintf(pStream,"%s\n", ID_MAT_SHADE_METAL); break;
		case SHADE_BLINN: fprintf(pStream,"%s\n", ID_MAT_SHADE_BLINN); break;
		default: fprintf(pStream,"%s\n", ID_MAT_SHADE_OTHER); break;
		}
		
		fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_XP_FALLOFF, Format(std->GetOpacFalloff(t)));
		fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_SELFILLUM, Format(std->GetSelfIllum(t)));
		
		if (std->GetTwoSided()) {
			fprintf(pStream,"%s\t%s\n", indent.data(), ID_TWOSIDED);
		}
		
		if (std->GetWire()) {
			fprintf(pStream,"%s\t%s\n", indent.data(), ID_WIRE);
		}
		
		if (std->GetWireUnits()) {
			fprintf(pStream,"%s\t%s\n", indent.data(), ID_WIREUNITS);
		}
		
		fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_FALLOFF, std->GetFalloffOut() ? ID_FALLOFF_OUT : ID_FALLOFF_IN);
		
		if (std->GetFaceMap()) {
			fprintf(pStream,"%s\t%s\n", indent.data(), ID_FACEMAP);
		}
		
		if (std->GetSoften()) {
			fprintf(pStream,"%s\t%s\n", indent.data(), ID_SOFTEN);
		}
		
		fprintf(pStream,"%s\t%s ", indent.data(), ID_XP_TYPE);
		switch (std->GetTransparencyType()) {
		case TRANSP_FILTER: fprintf(pStream,"%s\n", ID_MAP_XPTYPE_FLT); break;
		case TRANSP_SUBTRACTIVE: fprintf(pStream,"%s\n", ID_MAP_XPTYPE_SUB); break;
		case TRANSP_ADDITIVE: fprintf(pStream,"%s\n", ID_MAP_XPTYPE_ADD); break;
		default: fprintf(pStream,"%s\n", ID_MAP_XPTYPE_OTH); break;
		}
	}
	else {
		// Note about material colors:
		// This is only the color used by the interactive renderer in MAX.
		// To get the color used by the scanline renderer, we need to evaluate
		// the material using the mtl->Shade() method.
		// Since the materials are procedural there is no real "diffuse" color for a MAX material
		// but we can at least take the interactive color.
		
		fprintf(pStream,"%s\t%s %s\n",indent.data(), ID_AMBIENT, Format(mtl->GetAmbient()));
		fprintf(pStream,"%s\t%s %s\n",indent.data(), ID_DIFFUSE, Format(mtl->GetDiffuse()));
		fprintf(pStream,"%s\t%s %s\n",indent.data(), ID_SPECULAR, Format(mtl->GetSpecular()));
		fprintf(pStream,"%s\t%s %s\n",indent.data(), ID_SHINE, Format(mtl->GetShininess()));
		fprintf(pStream,"%s\t%s %s\n",indent.data(), ID_SHINE_STRENGTH, Format(mtl->GetShinStr()));
		fprintf(pStream,"%s\t%s %s\n",indent.data(), ID_TRANSPARENCY, Format(mtl->GetXParency()));
		fprintf(pStream,"%s\t%s %s\n",indent.data(), ID_WIRESIZE, Format(mtl->WireSize()));
	}

	for (i=0; i<mtl->NumSubTexmaps(); i++) {
		Texmap* subTex = mtl->GetSubTexmap(i);
		float amt = 1.0f;
		if (subTex) {
			// If it is a standard material we can see if the map is enabled.
			if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
				if (!((StdMat*)mtl)->MapEnabled(i))
					continue;
				amt = ((StdMat*)mtl)->GetTexmapAmt(i, 0);
				
			}
			DumpTexture(subTex, mtl->ClassID(), i, amt, indentLevel+1);
		}
	}
	
	if (mtl->NumSubMtls() > 0)  {
		fprintf(pStream,"%s\t%s %d\n",indent.data(), ID_NUMSUBMTLS, mtl->NumSubMtls());
		
		for (i=0; i<mtl->NumSubMtls(); i++) {
			Mtl* subMtl = mtl->GetSubMtl(i);
			if (subMtl) {
				DumpMaterial(subMtl, 0, i, indentLevel+1);
			}
		}
	}
	fprintf(pStream,"%s}\n", indent.data());
}


// For a standard material, this will give us the meaning of a map
// givien its submap id.
TCHAR* EGExp::GetMapID(Class_ID cid, int subNo)
{
	static TCHAR buf[50];
	
	if (cid == Class_ID(0,0)) {
		strcpy(buf, ID_ENVMAP);
	}
	else if (cid == Class_ID(DMTL_CLASS_ID, 0)) {
		switch (subNo) {
		case ID_AM: strcpy(buf, ID_MAP_AMBIENT); break;
		case ID_DI: strcpy(buf, ID_MAP_DIFFUSE); break;
		case ID_SP: strcpy(buf, ID_MAP_SPECULAR); break;
		case ID_SH: strcpy(buf, ID_MAP_SHINE); break;
		case ID_SS: strcpy(buf, ID_MAP_SHINESTRENGTH); break;
		case ID_SI: strcpy(buf, ID_MAP_SELFILLUM); break;
		case ID_OP: strcpy(buf, ID_MAP_OPACITY); break;
		case ID_FI: strcpy(buf, ID_MAP_FILTERCOLOR); break;
		case ID_BU: strcpy(buf, ID_MAP_BUMP); break;
		case ID_RL: strcpy(buf, ID_MAP_REFLECT); break;
		case ID_RR: strcpy(buf, ID_MAP_REFRACT); break;
		}
	}
	else {
		strcpy(buf, ID_MAP_GENERIC);
	}
	
	return buf;
}

void EGExp::DumpTexture(Texmap* tex, Class_ID cid, int subNo, float amt, int indentLevel)
{
	if (!tex) return;
	
	TSTR indent = GetIndent(indentLevel+1);
	
	TSTR className;
	tex->GetClassName(className);
	
	fprintf(pStream,"%s%s {\n", indent.data(), GetMapID(cid, subNo));
	
	fprintf(pStream,"%s\t%s \"%s\"\n", indent.data(), ID_TEXNAME, FixupName(tex->GetName()));
	fprintf(pStream,"%s\t%s \"%s\"\n", indent.data(), ID_TEXCLASS, FixupName(className));
	
	// If we include the subtexture ID, a parser could be smart enough to get
	// the class name of the parent texture/material and make it mean something.
	fprintf(pStream,"%s\t%s %d\n", indent.data(), ID_TEXSUBNO, subNo);
	
	fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_TEXAMOUNT, Format(amt));
	
	// Is this a bitmap texture?
	// We know some extra bits 'n pieces about the bitmap texture
	if (tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)) {
		TSTR mapName = ((BitmapTex *)tex)->GetMapName();
		fprintf(pStream,"%s\t%s \"%s\"\n", indent.data(), ID_BITMAP, FixupName(mapName));
		
		StdUVGen* uvGen = ((BitmapTex *)tex)->GetUVGen();
		if (uvGen) {
			DumpUVGen(uvGen, indentLevel+1);
		}
		
		TextureOutput* texout = ((BitmapTex*)tex)->GetTexout();
		if (texout->GetInvert()) {
			fprintf(pStream,"%s\t%s\n", indent.data(), ID_TEX_INVERT);
		}
		
		fprintf(pStream,"%s\t%s ", indent.data(), ID_BMP_FILTER);
		switch(((BitmapTex*)tex)->GetFilterType()) {
		case FILTER_PYR:  fprintf(pStream,"%s\n", ID_BMP_FILT_PYR); break;
		case FILTER_SAT: fprintf(pStream,"%s\n", ID_BMP_FILT_SAT); break;
		default: fprintf(pStream,"%s\n", ID_BMP_FILT_NONE); break;
		}
	}
	
	for (int i=0; i<tex->NumSubTexmaps(); i++) {
		DumpTexture(tex->GetSubTexmap(i), tex->ClassID(), i, 1.0f, indentLevel+1);
	}
	
	fprintf(pStream, "%s}\n", indent.data());
}

void EGExp::DumpUVGen(StdUVGen* uvGen, int indentLevel)
{
	int mapType = uvGen->GetCoordMapping(0);
	TimeValue t = GetStaticFrame();
	TSTR indent = GetIndent(indentLevel+1);
	
	fprintf(pStream,"%s%s ", indent.data(), ID_MAPTYPE);
	
	switch (mapType) {
	case UVMAP_EXPLICIT: fprintf(pStream,"%s\n", ID_MAPTYPE_EXP); break;
	case UVMAP_SPHERE_ENV: fprintf(pStream,"%s\n", ID_MAPTYPE_SPH); break;
	case UVMAP_CYL_ENV:  fprintf(pStream,"%s\n", ID_MAPTYPE_CYL); break;
	case UVMAP_SHRINK_ENV: fprintf(pStream,"%s\n", ID_MAPTYPE_SHR); break;
	case UVMAP_SCREEN_ENV: fprintf(pStream,"%s\n", ID_MAPTYPE_SCR); break;
	}
	
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_U_OFFSET, Format(uvGen->GetUOffs(t)));
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_V_OFFSET, Format(uvGen->GetVOffs(t)));
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_U_TILING, Format(uvGen->GetUScl(t)));
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_V_TILING, Format(uvGen->GetVScl(t)));
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_ANGLE, Format(uvGen->GetAng(t)));
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_BLUR, Format(uvGen->GetBlur(t)));
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_BLUR_OFFSET, Format(uvGen->GetBlurOffs(t)));
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_NOISE_AMT, Format(uvGen->GetNoiseAmt(t)));
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_NOISE_SIZE, Format(uvGen->GetNoiseSize(t)));
	fprintf(pStream,"%s%s %d\n", indent.data(), ID_NOISE_LEVEL, uvGen->GetNoiseLev(t));
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_NOISE_PHASE, Format(uvGen->GetNoisePhs(t)));
}

/****************************************************************************

  Face Mapped Material functions
  
****************************************************************************/

BOOL EGExp::CheckForAndExportFaceMap(Mtl* mtl, Mesh* mesh, int indentLevel)
{
	if (!mtl || !mesh) {
		return FALSE;
	}
	
	ULONG matreq = mtl->Requirements(-1);
	
	// Are we using face mapping?
	if (!(matreq & MTLREQ_FACEMAP)) {
		return FALSE;
	}
	
	TSTR indent = GetIndent(indentLevel+1);
	
	// OK, we have a FaceMap situation here...
	
	fprintf(pStream, "%s%s {\n", indent.data(), ID_MESH_FACEMAPLIST);
	for (int i=0; i<mesh->getNumFaces(); i++) {
		Point3 tv[3];
		Face* f = &mesh->faces[i];
		make_face_uv(f, tv);
		fprintf(pStream, "%s\t%s %d {\n", indent.data(), ID_MESH_FACEMAP, i);
		fprintf(pStream, "%s\t\t%s\t%d\t%d\t%d\n", indent.data(), ID_MESH_FACEVERT, (int)tv[0].x, (int)tv[0].y, (int)tv[0].z);
		fprintf(pStream, "%s\t\t%s\t%d\t%d\t%d\n", indent.data(), ID_MESH_FACEVERT, (int)tv[1].x, (int)tv[1].y, (int)tv[1].z);
		fprintf(pStream, "%s\t\t%s\t%d\t%d\t%d\n", indent.data(), ID_MESH_FACEVERT, (int)tv[2].x, (int)tv[2].y, (int)tv[2].z);
		fprintf(pStream, "%s\t}\n", indent.data());
	}
	fprintf(pStream, "%s}\n", indent.data());
	
	return TRUE;
}


/****************************************************************************

  Misc Utility functions
  
****************************************************************************/

// Return an indentation string
TSTR EGExp::GetIndent(int indentLevel)
{
	TSTR indentString = "";
	for (int i=0; i<indentLevel; i++) {
		indentString += "\t";
	}
	
	return indentString;
}

// Determine is the node has negative scaling.
// This is used for mirrored objects for example. They have a negative scale factor
// so when calculating the normal we should take the vertices counter clockwise.
// If we don't compensate for this the objects will be 'inverted'.
BOOL EGExp::TMNegParity(Matrix3 &m)
{
	return (DotProd(CrossProd(m.GetRow(0),m.GetRow(1)),m.GetRow(2))<0.0)?1:0;
}

// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject* EGExp::GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt)
{
	deleteIt = FALSE;
	Object *obj = node->EvalWorldState(t).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
		TriObject *tri = (TriObject *) obj->ConvertToType(t, 
			Class_ID(TRIOBJ_CLASS_ID, 0));
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		if (obj != tri) deleteIt = TRUE;
		return tri;
	}
	else {
		return NULL;
	}
}

// Print out a transformation matrix in different ways.
// Apart from exporting the full matrix we also decompose
// the matrix and export the components.
void EGExp::DumpMatrix3(Matrix3* m, int indentLevel)
{
	Point3 row;
	TSTR indent = GetIndent(indentLevel);
	
	// Dump the whole Matrix
	row = m->GetRow(0);
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_TM_ROW0, Format(row));
	row = m->GetRow(1);
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_TM_ROW1, Format(row));
	row = m->GetRow(2);
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_TM_ROW2, Format(row));
	row = m->GetRow(3);
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_TM_ROW3, Format(row));
	
	// Decompose the matrix and dump the contents
	AffineParts ap;
	float rotAngle;
	Point3 rotAxis;
	float scaleAxAngle;
	Point3 scaleAxis;
	
	decomp_affine(*m, &ap);

	// Quaternions are dumped as angle axis.
	AngAxisFromQ(ap.q, &rotAngle, rotAxis);
	AngAxisFromQ(ap.u, &scaleAxAngle, scaleAxis);

	fprintf(pStream,"%s%s %s\n", indent.data(), ID_TM_POS, Format(ap.t));
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_TM_ROTAXIS, Format(rotAxis));
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_TM_ROTANGLE, Format(rotAngle));
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_TM_SCALE, Format(ap.k));
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_TM_SCALEAXIS, Format(scaleAxis));
	fprintf(pStream,"%s%s %s\n", indent.data(), ID_TM_SCALEAXISANG, Format(scaleAxAngle));
}

// From the SDK
// How to calculate UV's for face mapped materials.
static Point3 basic_tva[3] = { 
	Point3(0.0,0.0,0.0),Point3(1.0,0.0,0.0),Point3(1.0,1.0,0.0)
};
static Point3 basic_tvb[3] = { 
	Point3(1.0,1.0,0.0),Point3(0.0,1.0,0.0),Point3(0.0,0.0,0.0)
};
static int nextpt[3] = {1,2,0};
static int prevpt[3] = {2,0,1};

void EGExp::make_face_uv(Face *f, Point3 *tv)
{
	int na,nhid,i;
	Point3 *basetv;
	/* make the invisible edge be 2->0 */
	nhid = 2;
	if (!(f->flags&EDGE_A))  nhid=0;
	else if (!(f->flags&EDGE_B)) nhid = 1;
	else if (!(f->flags&EDGE_C)) nhid = 2;
	na = 2-nhid;
	basetv = (f->v[prevpt[nhid]]<f->v[nhid]) ? basic_tva : basic_tvb; 
	for (i=0; i<3; i++) {  
		tv[i] = basetv[na];
		na = nextpt[na];
	}
}


/****************************************************************************

  String manipulation functions
  
****************************************************************************/

#define CTL_CHARS  31
#define SINGLE_QUOTE 39

// Replace some characters we don't care for.
TCHAR* EGExp::FixupName(TCHAR* name)
{
	static char buffer[256];
	TCHAR* cPtr;
	
    _tcscpy(buffer, name);
    cPtr = buffer;
	
    while(*cPtr) {
		if (*cPtr == '"')
			*cPtr = SINGLE_QUOTE;
        else if (*cPtr <= CTL_CHARS)
			*cPtr = _T('_');
        cPtr++;
    }
	
	return buffer;
}

// International settings in Windows could cause a number to be written
// with a "," instead of a ".".
// To compensate for this we need to convert all , to . in order to make the
// format consistent.
void EGExp::CommaScan(TCHAR* buf)
{
    for(; *buf; buf++) if (*buf == ',') *buf = '.';
}

TSTR EGExp::Format(int value)
{
	TCHAR buf[50];
	
	sprintf(buf, _T("%d"), value);
	return buf;
}


TSTR EGExp::Format(float value)
{
	TCHAR buf[40];
	
	sprintf(buf, szFmtStr, value);
	CommaScan(buf);
	return TSTR(buf);
}

TSTR EGExp::Format(Point3 value)
{
	TCHAR buf[120];
	TCHAR fmt[120];
	
	sprintf(fmt, "%s\t%s\t%s", szFmtStr, szFmtStr, szFmtStr);
	sprintf(buf, fmt, value.x, value.y, value.z);

	CommaScan(buf);
	return buf;
}

TSTR EGExp::Format(Color value)
{
	TCHAR buf[120];
	TCHAR fmt[120];
	
	sprintf(fmt, "%s\t%s\t%s", szFmtStr, szFmtStr, szFmtStr);
	sprintf(buf, fmt, value.r, value.g, value.b);

	CommaScan(buf);
	return buf;
}

TSTR EGExp::Format(AngAxis value)
{
	TCHAR buf[160];
	TCHAR fmt[160];
	
	sprintf(fmt, "%s\t%s\t%s\t%s", szFmtStr, szFmtStr, szFmtStr, szFmtStr);
	sprintf(buf, fmt, value.axis.x, value.axis.y, value.axis.z, value.angle);

	CommaScan(buf);
	return buf;
}


TSTR EGExp::Format(Quat value)
{
	// A Quat is converted to an AngAxis before output.
	
	Point3 axis;
	float angle;
	AngAxisFromQ(value, &angle, axis);
	
	return Format(AngAxis(axis, angle));
}

TSTR EGExp::Format(ScaleValue value)
{
	TCHAR buf[280];
	
	sprintf(buf, "%s %s", Format(value.s), Format(value.q));
	CommaScan(buf);
	return buf;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
EGExp2::EGExp2():m_FileName(),m_GroupName(),m_Log(),m_Rename(),m_Anim()
{
    ip=NULL;

    m_MaterialNextNo=0;
	m_GroupFirst=NULL;
	m_GroupLast=NULL;

    m_TextureDefault=true;

    m_MatCenter.IdentityMatrix();
    m_Scale=Point3(1.0,1.0,1.0);

	m_MaxSizeX=0; m_MaxSizeY=0; m_MaxSizeZ=0;
    m_EdgeExport=0;
    m_EdgeExportFactor=cos(0.1 * pi/180.0);

    ip=GetCOREInterface();
}

EGExp2::~EGExp2()
{
    GroupClear();
}

void EGExp2::GroupClear()
{
	while(m_GroupFirst!=NULL) GroupDelete(m_GroupLast);
}

void EGExp2::GroupDelete(CGroup * el)
{
	LIST_DEL(el,m_GroupFirst,m_GroupLast,m_Prev,m_Next);
	HDelete(CGroup,el,NULL);
}

CGroup * EGExp2::GroupFind(Base::CPoint & p1)
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

CGroup * EGExp2::GroupFind(Base::CPoint & p1, Base::CPoint & p2)
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

CGroup * EGExp2::GroupFind(Base::CPoint & p1, Base::CPoint & p2, Base::CPoint & p3)
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

CGroup * EGExp2::GroupAdd(Base::CPoint & p1)
{
	CGroup * el=HNew(NULL) CGroup;

	LIST_ADD(el,m_GroupFirst,m_GroupLast,m_Prev,m_Next);

	el->m_IdCnt=1;
	el->m_Id[0]=p1;

	return el;
}

CGroup * EGExp2::GroupAdd(Base::CPoint & p1, Base::CPoint & p2)
{
	CGroup * el=HNew(NULL) CGroup;

	LIST_ADD(el,m_GroupFirst,m_GroupLast,m_Prev,m_Next);

	el->m_IdCnt=2;
	el->m_Id[0]=p1;
	el->m_Id[1]=p2;

	return el;
}

CGroup * EGExp2::GroupAdd(Base::CPoint & p1, Base::CPoint & p2, Base::CPoint & p3)
{
	CGroup * el=HNew(NULL) CGroup;

	LIST_ADD(el,m_GroupFirst,m_GroupLast,m_Prev,m_Next);

	el->m_IdCnt=3;
	el->m_Id[0]=p1;
	el->m_Id[1]=p2;
	el->m_Id[2]=p3;

	return el;
}

void EGExp2::GroupSortByNo()
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

void EGExp2::Log(Base::CWStr str)
{
	m_Log.WStrNZ(str.Get());
	m_Log.Dword(0x000a000d);
}

int EGExp2::GroupMaterialFindAdd(Mtl * ml, int sm, int & twosided)
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

int EGExp2::AddVertex(Base::CBuf & buf,SVertexNorTex & v)
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

Base::CWStr EGExp2::RenameByCfg(Base::CWStr & str)
{
	if(m_Rename.ParCount(str)<=0) return str;
	return m_Rename.Par(str);
}

void EGExp2::ExportSimpleGroup()
{
    int no,export;
    Base::CWStr tstr;
    int cnt=ExportGroupCount();
    for(int i=0;i<cnt;i++) {
        m_GroupName.Clear();
        m_FileName.Clear();
        m_Anim.Clear();
        tstr.Clear();

        if(ExportGroupGetProp(i,no,NULL,m_GroupName,export,m_FileName,m_TextureDefault,m_EdgeExport,m_EdgeExportFactor,tstr) && export) {
            m_Anim.Clear();
            m_Anim.LoadFromText(tstr.Get());

            tstr.Clear();
            ExportGroupGetRename(tstr);
            m_Rename.Clear();
            m_Rename.LoadFromText(tstr.Get());

            m_EdgeExportFactor=cos(m_EdgeExportFactor * pi/180.0);

    		Log(Base::CWStr(L"=== Group ==="));

            ExportSimple();
        }
    }
}

void EGExp2::ExportSimple()
{
	SVOHeader he;
	SVOGroup vogr;
	SVOFrame voframe;
	SVOFrame * voframep;
	SVOAnimHeader voanimh;
	SVOAnimHeader * voanimhp;

/*	struct SFrameInfo {
		int m_PointBuf;
		int m_PointSme;
		int m_PointSize;
	} * frameInfo=NULL;*/

	Base::CWStr tstr;
	Base::CWStr tstr2;
	Base::CBuf bufout;
	Base::CBuf bframes;
    Base::CWStr filename;

	m_MaterialNextNo=0;

	int * framelist=NULL;
	int framecnt=0;

	int * vergrouplist=NULL;
	int vergroupcnt=0;
	int * trigrouplist=NULL;
	int trigroupcnt=0;
	int * matrixlist=NULL;
	int matrixcnt=0;

	GroupClear();

    filename=m_FileName;

	try {
        int animcnt=m_Anim.ParCount();//group->ParCount(L"Anim");
		if(animcnt>0) {
			Interval range = ip->GetAnimRange();
			int framestart=range.Start() / GetTicksPerFrame();
			int frameend=range.End() / GetTicksPerFrame();
			int framemaxcnt=frameend-framestart+1;

			framelist=(int *)HAlloc(framemaxcnt*sizeof(4),NULL);

			for(int i=0;i<animcnt;i++) {
//				tstr=group->ParPathGet(Base::CWStr(L"Anim:")+Base::CWStr(i));
                tstr=m_Anim.ParGetName(i)+L","+m_Anim.ParGet(i);

				int cnt=tstr.GetCountPar(L",");
				if(cnt<3) throw L"Anim format error";

				StrToFrameList(tstr.GetStrPar(2,cnt-1,L","),bframes);

				for(int u=0;u<bframes.Len()/8;u++) {
					int curframe=bframes.Int(); bframes.Int();
					if(curframe<framestart || curframe>frameend) throw L"Anim error";
					for(int t=0;t<framecnt;t++) if(framelist[t]==curframe) break;
					if(t>=framecnt) {
						framelist[framecnt]=curframe;
						framecnt++;
					}
				}
			}
		} else {
			framelist=(int *)HAlloc(1*sizeof(4),NULL);
			framelist[0]=ip->GetTime();
			framecnt=1;
		}

//		frameInfo=(SFrameInfo *)HAllocClear(framecnt*sizeof(SFrameInfo),NULL);

		for(int i=0;i<framecnt;i++) {
			int numChildren = ip->GetRootNode()->NumberOfChildren();

            if(!ExportCenterGetProp(m_GroupName,framelist[i]*GetTicksPerFrame(),NULL,
                                    m_MaxSizeX,m_MaxSizeY,m_MaxSizeZ,
                                    m_Scale.x,m_Scale.y,m_Scale.z,m_MatCenter)) 
            {
			    m_MatCenter.IdentityMatrix();
			    m_Scale=Point3(1.0,1.0,1.0);
			    m_MaxSizeX=0; m_MaxSizeY=0; m_MaxSizeZ=0;
            }
/*			for (int idx=0; idx<numChildren; idx++) {
				if (ip->GetCancel()) throw L"Terminate";
				ExportSimple_NodeEnumFirst(ip->GetRootNode()->GetChildNode(idx),framelist[i]*GetTicksPerFrame(),i);
			}*/

//			Matrix3 tm;
//			tm.Scale(Point3(-1,1,1),TRUE);
//			m_MatCenter=m_MatCenter*tm;
			m_MatCenter.Scale(m_Scale,FALSE);

			for (int idx=0; idx<numChildren; idx++) {
				if (ip->GetCancel()) throw L"Terminate";
				ExportSimple_NodeEnum(ip->GetRootNode()->GetChildNode(idx),framelist[i]*GetTicksPerFrame(),i);
			}
		}

		GroupSortByNo();

        if(!m_GroupName.IsEmpty()) Log(Base::CWStr(L"Group=")+m_GroupName);
		Log(Base::CWStr(L"OutFile=")+filename);

		int cntvergroup=0;
		int cnttrigroup=0;

		CGroup * gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Ver) cntvergroup++;
			else if(gr->m_Id[0].x==gt_Frame && gr->m_Id[1].x==gt_TriList) cnttrigroup++;
			gr=gr->m_Next;
		}

		// header
		ZeroMemory(&he,sizeof(SVOHeader));
		he.m_Id=0x00006f76;
		he.m_Ver=0;

		if(m_TextureDefault) he.m_Flags|=2;

		bufout.BufAdd(&he,sizeof(SVOHeader));

		// material
		he.m_MaterialSme=bufout.Pointer();
		gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Material) {
				gr->m_User=he.m_MaterialCnt;

				he.m_MaterialCnt+=gr->m_Buf.Len()/sizeof(SMaterial);
				bufout.BufAdd(gr->m_Buf.Get(),gr->m_Buf.Len());

				if(((SMaterial *)(gr->m_Buf.Get()))->tex_diffuse[0]!=0) Log(Base::CWStr(L"Texture=")+((SMaterial *)(gr->m_Buf.Get()))->tex_diffuse);
			}
			gr=gr->m_Next;
		}

		// vertex scale
//		if(m_MaxSizeX>0 || m_MaxSizeY>0 || m_MaxSizeZ>0) {
			double minx=1e30,miny=1e30,minz=1e30,maxx=-1e30,maxy=-1e30,maxz=-1e30;

			gr=m_GroupFirst;
			while(gr!=NULL) {
				if(gr->m_Id[0].x==gt_Ver) {
					int cnt=gr->m_Buf.Len()/sizeof(SVertexNorTex);
					SVertexNorTex * cv=(SVertexNorTex *)gr->m_Buf.Get();
					while(cnt>0) {
						minx=min(cv->x,minx);
						miny=min(cv->y,miny);
						minz=min(cv->z,minz);
						maxx=max(cv->x,maxx);
						maxy=max(cv->y,maxy);
						maxz=max(cv->z,maxz);
						cnt--;
						cv++;
					}
				}
				gr=gr->m_Next;
			}

			double koffx=1e30; if(m_MaxSizeX>0) koffx=m_MaxSizeX/(maxx-minx);
			double koffy=1e30; if(m_MaxSizeY>0) koffy=m_MaxSizeY/(maxy-miny);
			double koffz=1e30; if(m_MaxSizeZ>0) koffz=m_MaxSizeZ/(maxz-minz);

			double kofscale=min(min(koffx,koffy),koffz);
			if(kofscale>1e20) kofscale=1.0;
//			maxx=m_MaxSizeX/max(maxx-minx,max(maxy-miny,maxz-minz));

			if(kofscale!=1.0) {
				gr=m_GroupFirst;
				while(gr!=NULL) {
					if(gr->m_Id[0].x==gt_Ver) {
						int cnt=gr->m_Buf.Len()/sizeof(SVertexNorTex);
						SVertexNorTex * cv=(SVertexNorTex *)gr->m_Buf.Get();
						while(cnt>0) {
							cv->x=float(cv->x*kofscale);
							cv->y=float(cv->y*kofscale);
							cv->z=float(cv->z*kofscale);
							cnt--;
							cv++;
						}
					}
					gr=gr->m_Next;
				}
			}

			gr=m_GroupFirst;
			while(gr!=NULL) {
				if(gr->m_Id[0].x==gt_ExpMatrix) {
					SVOMatrix * em=(SVOMatrix *)((byte *)gr->m_Buf.Get()+sizeof(SVOExpMatrixHeader));

					Matrix3 mm;
					ConvertMatrix(mm,*em);
					mm.Scale(Point3(kofscale,kofscale,kofscale),true);
					mm.Orthogonalize();
					mm.NoScale();
					ConvertMatrix(*em,mm);

/*					Point3 v0=Point3(0,0,0);
					Point3 vx=Point3(1,0,0);
					Point3 vy=Point3(0,1,0);
					Point3 vz=Point3(0,0,1);

					TransformVector(v0,v0,*em);
					TransformVector(vx,vx,*em);
					TransformVector(vy,vy,*em);
					TransformVector(vz,vz,*em);

					vx-=v0; vy-=v0; vz-=v0; v0*=float(kofscale);
					vx=vx.Normalize(); vy=vy.Normalize(); vz=vz.Normalize();

//					ViewMatrix(v0,v0+vz,vy,*em);
//					em->_11=vx.x; em->_21=vx.y; em->_31=vx.z; em->_41=v0.x;
//					em->_12=vy.x; em->_22=vy.y; em->_32=vy.z; em->_42=v0.y;
//					em->_13=vz.x; em->_23=vz.y; em->_33=vz.z; em->_43=v0.z;
//					em->_14=0.0f; em->_24=0.0f; em->_34=0.0f; em->_44=1.0f;

					Point3 ttp0=Point3(0,0,0);
					Point3 ttp1=Point3(0,0,1);
					Point3 ttp2=Point3(0,1,0);
					Point3 ttp3=Point3(1,0,0);
					TransformVector(ttp0,ttp0,*em);
					TransformVector(ttp1,ttp1,*em);
					TransformVector(ttp2,ttp2,*em);
					TransformVector(ttp3,ttp3,*em);
					Point3 ttp1_=ttp1-ttp0;
					Point3 ttp2_=ttp2-ttp0;
					Point3 ttp3_=ttp3-ttp0;
					
					ttp3_=ttp3-ttp0;*/
				}
				gr=gr->m_Next;
			}

//		}

		// vertex
		vergrouplist=(int *)HAlloc(cntvergroup*2*4,NULL);
		he.m_VerSme=bufout.Pointer();
		gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Ver) {
				for(i=0;i<vergroupcnt;i++) {
					if(vergrouplist[i*2+0]==gr->m_Buf.Len() && memcmp((BYTE *)bufout.Get()+vergrouplist[i*2+1],gr->m_Buf.Get(),gr->m_Buf.Len())==0) break;
				}
				if(i<vergroupcnt) {
					gr->m_User=vergrouplist[i*2+0]; // size
					gr->m_User2=vergrouplist[i*2+1]; // sme
				} else {
					gr->m_User=gr->m_Buf.Len(); // size
					gr->m_User2=bufout.Pointer(); // sme
					bufout.BufAdd(gr->m_Buf.Get(),gr->m_Buf.Len());
					vergrouplist[vergroupcnt*2+0]=gr->m_User;
					vergrouplist[vergroupcnt*2+1]=gr->m_User2;
					vergroupcnt++;
					he.m_VerCnt+=gr->m_Buf.Len()/sizeof(SVertexNorTex);
				}
			}
			gr=gr->m_Next;
		}
		Log(Base::CWStr().Format(L"Vertex=<i> size=<i>",he.m_VerCnt,he.m_VerCnt*sizeof(SVertexNorTex)));


		// triangle
		trigrouplist=(int *)HAlloc(cnttrigroup*2*4,NULL);
		he.m_TriSme=bufout.Pointer();
		gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Frame && gr->m_Id[1].x==gt_TriList) {
/*				CGroup * grpoint=GroupFind(Base::CPoint(gt_Ver,gr->m_Id[0].y));
				int smeindex=(grpoint->m_User2-he.m_VerSme)/sizeof(SVertexNorTex);
				int cnt=gr->m_Buf.Len()/4;
				DWORD * ibuf=(DWORD *)gr->m_Buf.Get();
				if(smeindex>0) while(cnt>0) { *ibuf+=smeindex; ibuf++; cnt--; }
*/
				for(i=0;i<trigroupcnt;i++) {
					if(trigrouplist[i*2+0]==gr->m_Buf.Len() && memcmp((BYTE *)bufout.Get()+trigrouplist[i*2+1],gr->m_Buf.Get(),gr->m_Buf.Len())==0) break;
				}
				if(i<trigroupcnt) {
					gr->m_User=trigrouplist[i*2+0]; // size
					gr->m_User2=trigrouplist[i*2+1]; // sme
				} else {
					gr->m_User=gr->m_Buf.Len(); // size
					gr->m_User2=bufout.Pointer(); // sme
					bufout.BufAdd(gr->m_Buf.Get(),gr->m_Buf.Len());
					trigrouplist[trigroupcnt*2+0]=gr->m_User;
					trigrouplist[trigroupcnt*2+1]=gr->m_User2;
					trigroupcnt++;
					he.m_TriCnt+=gr->m_Buf.Len()/4;
				}
			}
			gr=gr->m_Next;
		}

		// triangle index to 16 bit
		if(he.m_TriCnt<65536) {
			he.m_Flags|=1;
			DWORD * sou=(DWORD *)(((BYTE *)bufout.Get())+he.m_TriSme);
			WORD * des=(WORD *)sou;
			int cnt=he.m_TriCnt;
			while(cnt>0) {
				*des=WORD(*sou);
                sou++;
				des++;
				cnt--;
			}
			bufout.Len(he.m_TriSme+he.m_TriCnt*2);
			bufout.Pointer(bufout.Len());
		}
		Log(Base::CWStr().Format(L"Triangles=<i> size=<i>",he.m_TriCnt/3,he.m_TriCnt*((he.m_Flags&1)?2:4)));

		// group
		he.m_GroupSme=bufout.Pointer();
		gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Frame && gr->m_Id[1].x==gt_TriList) {
				ZeroMemory(&vogr,sizeof(SVOGroup));

				gr->m_User3=he.m_GroupCnt;

				CGroup * grpoint=GroupFind(Base::CPoint(gt_Ver,gr->m_Id[0].y));

				vogr.m_Material=gr->m_Id[1].y;
				vogr.m_VerCnt=grpoint->m_User/sizeof(SVertexNorTex);
				vogr.m_VerStart=(grpoint->m_User2-he.m_VerSme)/sizeof(SVertexNorTex);
				vogr.m_TriCnt=gr->m_User/4;
				vogr.m_TriStart=(gr->m_User2-he.m_TriSme)/4;

				bufout.BufAdd(&vogr,sizeof(SVOGroup));

				he.m_GroupCnt++;
			}
			gr=gr->m_Next;
		}
		Log(Base::CWStr().Format(L"Groups=<i>",he.m_GroupCnt));

		// edge
		if(m_EdgeExport) {
			he.m_EdgeSme=bufout.Pointer();

			gr=m_GroupFirst;
			while(gr!=NULL) {
				if(gr->m_Id[0].x==gt_Edge) {
					gr->m_User=(bufout.Pointer()-he.m_EdgeSme)/sizeof(SVOEdge);
					gr->m_User2=gr->m_Buf.Len()/sizeof(SVOEdge);

					bufout.BufAdd(gr->m_Buf.Get(),gr->m_Buf.Len());
					he.m_EdgeCnt+=gr->m_Buf.Len()/sizeof(SVOEdge);
				}
				gr=gr->m_Next;
			}
			Log(Base::CWStr().Format(L"Edges=<i>",he.m_EdgeCnt));
		}

		// frame
		ZeroMemory(&voframe,sizeof(SVOFrame));
		he.m_FrameSme=bufout.Pointer();
		he.m_FrameCnt=framecnt;
		voframe.m_MinX=1e30f; voframe.m_MinY=1e30f; voframe.m_MinZ=1e30f;
		voframe.m_MaxX=-1e30f; voframe.m_MaxY=-1e30f; voframe.m_MaxZ=-1e30f;
		for(i=0;i<framecnt;i++) {
			bufout.BufAdd(&voframe,sizeof(SVOFrame));
		}
		for(i=0;i<framecnt;i++) {
			int sme=bufout.Pointer();
			int cnt=0;
			gr=m_GroupFirst;
			while(gr!=NULL) {
				if(gr->m_Id[0].x==gt_Frame && gr->m_Id[0].y==i && gr->m_Id[1].x==gt_TriList) {
					bufout.Dword(gr->m_User3);
					gr->m_User4=cnt;
					cnt++;
				}
				gr=gr->m_Next;
			}
			voframep=(SVOFrame *)(((BYTE *)bufout.Get())+he.m_FrameSme+i*sizeof(SVOFrame));
			voframep->m_GroupIndexCnt=cnt;
			voframep->m_GroupIndexSme=sme;

			gr=GroupFind(Base::CPoint(gt_Ver,i));

			cnt=gr->m_Buf.Len()/sizeof(SVertexNorTex);
			SVertexNorTex * cv=(SVertexNorTex *)gr->m_Buf.Get();
			while(cnt>0) {
				voframep->m_MinX=min(cv->x,voframep->m_MinX);
				voframep->m_MinY=min(cv->y,voframep->m_MinY);
				voframep->m_MinZ=min(cv->z,voframep->m_MinZ);
				voframep->m_MaxX=max(cv->x,voframep->m_MaxX);
				voframep->m_MaxY=max(cv->y,voframep->m_MaxY);
				voframep->m_MaxZ=max(cv->z,voframep->m_MaxZ);
				voframep->m_CenterX+=cv->x;
				voframep->m_CenterY+=cv->y;
				voframep->m_CenterZ+=cv->z;
				cnt--;
				cv++;
			}
			cnt=gr->m_Buf.Len()/sizeof(SVertexNorTex);
			voframep->m_CenterX/=cnt;
			voframep->m_CenterY/=cnt;
			voframep->m_CenterZ/=cnt;
			cv=(SVertexNorTex *)gr->m_Buf.Get();
			float mr=0,mr2=0;
			while(cnt>0) {
				mr=max(mr,(voframep->m_CenterX-cv->x)*(voframep->m_CenterX-cv->x)+(voframep->m_CenterY-cv->y)*(voframep->m_CenterY-cv->y)+(voframep->m_CenterZ-cv->z)*(voframep->m_CenterZ-cv->z));
				mr2=max(mr2,((voframep->m_MaxX+voframep->m_MinX)/2-cv->x)*((voframep->m_MaxX+voframep->m_MinX)/2-cv->x)+((voframep->m_MaxY+voframep->m_MinY)/2-cv->y)*((voframep->m_MaxY+voframep->m_MinY)/2-cv->y)+((voframep->m_MaxZ+voframep->m_MinZ)/2-cv->z)*((voframep->m_MaxZ+voframep->m_MinZ)/2-cv->z));
				cnt--;
				cv++;
			}
			voframep->m_RadiusCenter=float(sqrt(mr));
			voframep->m_RadiusBox=float(sqrt(mr2));

			if(m_EdgeExport) {
				gr=GroupFind(Base::CPoint(gt_Edge,i));
				if(gr) {
					voframep->m_EdgeStart=gr->m_User;
					voframep->m_EdgeCnt=gr->m_User2;

					int lastmat1=-1;
					int lastmat2=-1;
					int lastgroupinframe1=-1;
					int lastgroupinframe2=-1;

					SVOEdge * edg=(SVOEdge *)(DWORD(bufout.Get())+he.m_EdgeSme+voframep->m_EdgeStart*sizeof(SVOEdge));
					for(int ei=0;ei<int(voframep->m_EdgeCnt);ei++,edg++) {
						int cmat=((edg->m_SideTri1)>>20)&0x0ff;
						if(lastmat1!=cmat) {
							lastmat1=cmat;
							lastgroupinframe1=GroupFind(Base::CPoint(gt_Frame,i),Base::CPoint(gt_TriList,cmat))->m_User4;
						}
						edg->m_SideTri1=(edg->m_SideTri1&(~(0x0ff<<20))) | ((lastgroupinframe1)<<20);

						cmat=((edg->m_SideTri2)>>20)&0x0ff;
						if(lastmat2!=cmat) {
							lastmat2=cmat;
							lastgroupinframe2=GroupFind(Base::CPoint(gt_Frame,i),Base::CPoint(gt_TriList,cmat))->m_User4;
						}
						edg->m_SideTri2=(edg->m_SideTri2&(~(0x0ff<<20))) | ((lastgroupinframe2)<<20);
					}
				}
			}
		}
		Log(Base::CWStr().Format(L"Frames=<i>",he.m_FrameCnt));

		// anim
		ZeroMemory(&voanimh,sizeof(SVOAnimHeader));
		he.m_AnimSme=bufout.Pointer();
		he.m_AnimCnt=animcnt;
		for(i=0;i<animcnt;i++) {
			bufout.BufAdd(&voanimh,sizeof(SVOAnimHeader));
		}
		for(i=0;i<animcnt;i++) {
			ZeroMemory(&voanimh,sizeof(SVOAnimHeader));

            tstr=m_Anim.ParGetName(i)+L","+m_Anim.ParGet(i);
//			tstr=group->ParPathGet(Base::CWStr(L"Anim:")+Base::CWStr(i));
			int cnt=tstr.GetCountPar(L",");

			voanimh.m_Id=tstr.GetStrPar(0,L",").GetInt();

			tstr2=RenameByCfg(tstr.GetStrPar(1,L","));
			if(tstr2.GetLen()>31) tstr2.SetLen(31);
			if(tstr2.GetLen()>0) CopyMemory(voanimh.m_Name,tstr2.Get(),tstr2.GetLen()*2);

			StrToFrameList(tstr.GetStrPar(2,cnt-1,L","),bframes);

			voanimh.m_UnitCnt=bframes.Len()/8;
			voanimh.m_UnitSme=bufout.Pointer();

//			bufout.BufAdd(bframes.Get(),bframes.Len());

			tstr=Base::CWStr().Format(L"Anim=<i>,<s>",voanimh.m_Id,voanimh.m_Name);
			bframes.Pointer(0);
			for(int u=0;u<bframes.Len()/8;u++) {
				int cf=bframes.Int();
				int ct=bframes.Int();

				for(int t=0;t<framecnt;t++) if(framelist[t]==cf) { cf=t; break; }
				if(t>=framecnt) throw L"Error in anim";

				tstr+=L",";
				tstr+=Base::CWStr(cf);

				bufout.Int(cf);
				bufout.Int(ct);
			}
			Log(tstr);

			tstr=Base::CWStr().Format(L"Anim=<i>,<s>",voanimh.m_Id,voanimh.m_Name);
			bframes.Pointer(0);
			for(u=0;u<bframes.Len()/8;u++) {
				bframes.Int();
				tstr+=L",";
				tstr+=Base::CWStr(bframes.Int());
			}
			Log(tstr);

			voanimhp=(SVOAnimHeader *)(((BYTE *)bufout.Get())+he.m_AnimSme+i*sizeof(SVOAnimHeader));
			CopyMemory(voanimhp,&voanimh,sizeof(SVOAnimHeader));
		}

		// ExportMatrix
		matrixlist=(int *)HAlloc(GroupCount()*4,NULL);
		he.m_MatrixSme=bufout.Pointer();
		he.m_MatrixCnt=0;
		gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_ExpMatrix && gr->m_Id[0].y==0 && gr->m_Id[1].x==gt_ExpMatrix) {
				he.m_MatrixCnt++;

				SVOExpMatrixHeader * emh=(SVOExpMatrixHeader *)gr->m_Buf.Get();
				Log(Base::CWStr().Format(L"Matrix=<s>",emh->m_Name));

				bufout.BufAdd(gr->m_Buf.Get(),sizeof(SVOExpMatrixHeader));
				matrixlist[matrixcnt]=gr->m_Id[1].y;
				matrixcnt++;
			}
			gr=gr->m_Next;
		}
		for(i=0;i<matrixcnt;i++) {
			for(int u=0;u<framecnt;u++) {
				gr=GroupFind(Base::CPoint(gt_ExpMatrix,u),Base::CPoint(gt_ExpMatrix,matrixlist[i]));

				if(u==0) {
					SVOExpMatrixHeader * emh=(SVOExpMatrixHeader *)((BYTE *)bufout.Get()+he.m_MatrixSme+i*sizeof(SVOExpMatrixHeader));
					emh->m_MatrixSme=bufout.Pointer();
				}

				bufout.BufAdd((BYTE *)gr->m_Buf.Get()+sizeof(SVOExpMatrixHeader),sizeof(SVOMatrix));
			}
		}

		// header again
		CopyMemory(bufout.Get(),&he,sizeof(SVOHeader));

		Log(Base::CWStr().Format(L"File Size=<i>",bufout.Len()));

		// Save
//		Base::CFile fileout(m_FileName);
		Base::CFile fileout(filename);
		fileout.Create();
		fileout.Write(bufout.Get(),bufout.Len());
		fileout.Close();


/*		int cntgroup=0;
		int cntver=0;
		int cnttri=0;

		gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Ver) cntver+=gr->m_Buf.Len() / sizeof(SVertexNorTex);
			else if(gr->m_Id[0].x==gt_Frame && gr->m_Id[1].x==gt_TriList) cnttri+=gr->m_Buf.Len() / (4*3);
			else if(gr->m_Id[0].x==gt_Material) cntgroup++;
			gr=gr->m_Next;
		}
		bufout.Dword(cntgroup);
		bufout.Dword(cntver);
		bufout.Dword(cnttri);
		bufout.Dword(0); // rez

		Log(Base::CWStr(L"GroupCnt=")+cntgroup);
		Log(Base::CWStr(L"VertexCnt=")+cntver);
		Log(Base::CWStr(L"TriangleCnt=")+cnttri);

		int writesmegro=bufout.Pointer();	bufout.Dword(0); // sme list group
		int writesmever=bufout.Pointer();	bufout.Dword(0); // sme list vertex
		int writesmetri=bufout.Pointer();	bufout.Dword(0); // sme list tri
		bufout.Dword(0); // rez

		DWORD * bufgroupsme=(DWORD *)Base::Alloc(4*cntgroup);

		*(DWORD *)(DWORD(bufout.Get())+writesmegro)=bufout.Pointer();
		gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Material) {
				bufgroupsme[gr->m_No]=bufout.Pointer();
				bufout.Dword(0); // begin triangle
				bufout.Dword(0); // cnt triangle
				bufout.BufAdd(gr->m_Buf.Get(),gr->m_Buf.Len());

				if(((SMaterial *)(gr->m_Buf.Get()))->tex_diffuse[0]!=0) Log(Base::CWStr(L"Texture=")+((SMaterial *)(gr->m_Buf.Get()))->tex_diffuse);
			}
			gr=gr->m_Next;
		}

		int pointsme=0;

		*(DWORD *)(DWORD(bufout.Get())+writesmever)=bufout.Pointer();
		gr=m_GroupFirst;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Ver) {
				for(int i=0;i<framecnt;i++) {
					if(frameInfo[i].m_PointBuf!=0 && frameInfo[i].m_PointSize==gr->m_Buf.Len() && memcmp((BYTE *)bufout.Get()+frameInfo[i].m_PointBuf,gr->m_Buf.Get(),gr->m_Buf.Len())==0) break;
				}
				if(i<framecnt) {
					frameInfo[gr->m_Id[0].y].m_PointBuf=frameInfo[i].m_PointBuf;
					frameInfo[gr->m_Id[0].y].m_PointSme=frameInfo[i].m_PointSme;
					frameInfo[gr->m_Id[0].y].m_PointSize=frameInfo[i].m_PointSize;
				} else {
					frameInfo[gr->m_Id[0].y].m_PointBuf=bufout.Pointer();
					frameInfo[gr->m_Id[0].y].m_PointSme=pointsme;
					frameInfo[gr->m_Id[0].y].m_PointSize=gr->m_Buf.Len();
					pointsme+=gr->m_Buf.Len()/sizeof(SVertexNorTex);
				}
				bufout.BufAdd(gr->m_Buf.Get(),gr->m_Buf.Len());
			}
			gr=gr->m_Next;
		}

		*(DWORD *)(DWORD(bufout.Get())+writesmetri)=bufout.Pointer();
		gr=m_GroupFirst;
		int sme=0;
		while(gr!=NULL) {
			if(gr->m_Id[0].x==gt_Frame && gr->m_Id[1].x==gt_TriList) {
				*(DWORD *)(DWORD(bufout.Get())+bufgroupsme[gr->m_Id[1].y])=sme;
				*(DWORD *)(DWORD(bufout.Get())+bufgroupsme[gr->m_Id[1].y]+4)=gr->m_Buf.Len() / (3*4);

				sme=sme+gr->m_Buf.Len() / (3*4);
				bufout.BufAdd(gr->m_Buf.Get(),gr->m_Buf.Len());
			}
			gr=gr->m_Next;
		}

		Base::Free(bufgroupsme);*/

	} catch(wchar * estr) {
        Log(Base::CWStr(estr));
	}

	if(matrixlist!=NULL) { HFree(matrixlist,NULL); matrixlist=NULL; }
	if(vergrouplist!=NULL) { HFree(vergrouplist,NULL); vergrouplist=NULL; }
	if(trigrouplist!=NULL) { HFree(trigrouplist,NULL); trigrouplist=NULL; }

//	if(frameInfo!=NULL) { HFree(frameInfo,NULL); frameInfo=NULL; }
	if(framelist!=NULL) { HFree(framelist,NULL); framelist=NULL; }
}

BOOL EGExp2::ExportSimple_NodeEnum(INode* node,TimeValue timev,int frame)
{
	if (ip->GetCancel()) return FALSE;

	ObjectState os = node->EvalWorldState(timev); 
	if(os.obj) {
		switch(os.obj->SuperClassID()) {
			case GEOMOBJECT_CLASS_ID:
				ExportSimple_GeomObject(node,timev,frame); 
				break;
			case HELPER_CLASS_ID:
				ExportSimple_Helper(node,timev,frame); 
				break;
		}
	}

	for (int c = 0; c < node->NumberOfChildren(); c++) {
		if (!ExportSimple_NodeEnum(node->GetChildNode(c),timev,frame)) return FALSE;
	}

	return TRUE;
}

void EGExp2::ExportSimple_GeomObject(INode * node,TimeValue timev,int frame)
{
	int i,u,vi;
	SVertexNorTex vdes,vdess[3];
	Point3 v,v0,v1,v2;//,vtp[3];

    Base::CWStr tstr;
    if(ExportObjGetProp(node,tstr,i)) {
        if(!i) return;
        if(!m_GroupName.IsEmpty() && !tstr.IsEmpty() && (m_GroupName!=tstr)) return;
    }

	Matrix3 tm = node->GetObjTMAfterWSM(timev)*m_MatCenter;

	int vx[3];
	if (TMNegParity(tm)) { vx[0] = 2; vx[1] = 1; vx[2] = 0; }
	else { vx[0] = 0; vx[1] = 1; vx[2] = 2; }

	BOOL needDel;
	TriObject* tri = GetTriObjectFromNode(node, timev, needDel);
	if (!tri) return;

	Mtl * nodeMtl = node->GetMtl();

	Mesh* mesh = &tri->GetMesh();
	mesh->buildRenderNormals();//buildNormals();

	CGroup * gv=GroupFindAdd(Base::CPoint(gt_Ver,frame));
	CGroup * gf=NULL;
	if(m_EdgeExport) gf=GroupFindAdd(Base::CPoint(gt_Edge,frame));

	Matrix3 normalObjToWorld(1);
//	for (int it=0; it<3; it++) {
//		Point4 p = Inverse(tm).GetColumn(it);
//		normalObjToWorld.SetRow(it,Point3(p[0],p[1],p[2]));
//	}
	for (int it=0; it<4; it++) {
		Point3 p = tm.GetRow(it);
		normalObjToWorld.SetRow(it,p);
	}

	DWORD * edgeindexlist=NULL;
	if(m_EdgeExport) edgeindexlist=(DWORD *)HAllocClear(mesh->getNumFaces()*4,NULL);

	for(i=0; i<mesh->getNumFaces(); i++) {
//		Point3	fn = mesh->getFaceNormal(i);
//		Point3	nfn = VectorTransform(normalObjToWorld/*tm*/, fn);
//		nfn=nfn.Normalize();
//		mesh->setFaceNormal(i, nfn);

		v0 = tm * mesh->verts[mesh->faces[i].v[vx[0]]];
		v1 = tm * mesh->verts[mesh->faces[i].v[vx[1]]];
		v2 = tm * mesh->verts[mesh->faces[i].v[vx[2]]];

		Point3 nfn=((v1-v0)^(v2-v0)).Normalize();
		mesh->setFaceNormal(i, nfn);
	}

	mesh->buildNormals();

	if(mesh->tVerts==NULL) {
//		Log(Base::CWStr(L"Mapping coords not found: ")+Base::CWStr(Base::CStr(node->NodeName())));
		TSTR str;
		node->EvalWorldState(timev).obj->GetClassName(str);
		Log(Base::CWStr().Format(L"Mapping coords not found: <s>  Class name: <s>",
				Base::CWStr(Base::CStr(node->NodeName())).Get(),
				Base::CWStr(Base::CStr(str.data())).Get() 
			));
	}

	for(i=0; i<mesh->getNumVerts(); i++) {
		v=mesh->verts[i];
		int cnt=0;
		for(u=i+1; u<mesh->getNumVerts(); u++) {
			if(i!=u) {
				if((mesh->verts[u]-v).Length()<=0.01) {
					mesh->vertSel.Set(u);
					mesh->vertSel.Set(i);
					cnt++;
				}
			}
		}
		if(cnt>=1) {
			mesh->vertSel.Set(i);
		} else {
			mesh->vertSel.Clear(i);
		}
	}

	for(i=0; i<mesh->getNumFaces(); i++) {
		int twosided=0;
		int mno=GroupMaterialFindAdd(nodeMtl,mesh->faces[i].getMatID(),twosided);
		CGroup * gtl=GroupFindAdd(Base::CPoint(gt_Frame,frame),Base::CPoint(gt_TriList,mno));

		if(m_EdgeExport) edgeindexlist[i]=(mno<<20)|(gtl->m_Buf.Pointer()/(4*3));

		for(u=0; u<3; u++) {
			vi=mesh->faces[i].v[vx[u]];

			v = tm * mesh->verts[vi];
			vdes.x=v.x; vdes.y=v.y; vdes.z=v.z;
//vtp[u]=v;

			v = Normalize(GetVertexNormal(mesh, i, mesh->getRVertPtr(vi)));
//			v = Normalize(mesh->getFaceNormal(i));
			vdes.nx=v.x; vdes.ny=v.y; vdes.nz=v.z;

			if(mesh->tVerts!=NULL) {
				v = mesh->tVerts[mesh->tvFace[i].t[vx[u]]];
				vdes.tu=v.x; vdes.tv=1.0f-v.y;
			} else {
				vdes.tu=0; vdes.tv=0;
			}

			vdess[u]=vdes;

			gtl->m_Buf.Dword(AddVertex(gv->m_Buf,vdes));

		}
		if(twosided) {
			vdess[0].nx=-vdess[0].nx; vdess[0].ny=-vdess[0].ny; vdess[0].nz=-vdess[0].nz;
			vdess[1].nx=-vdess[1].nx; vdess[1].ny=-vdess[1].ny; vdess[1].nz=-vdess[1].nz;
			vdess[2].nx=-vdess[2].nx; vdess[2].ny=-vdess[2].ny; vdess[2].nz=-vdess[2].nz;

			gtl->m_Buf.Dword(AddVertex(gv->m_Buf,vdess[0]));
			gtl->m_Buf.Dword(AddVertex(gv->m_Buf,vdess[2]));
			gtl->m_Buf.Dword(AddVertex(gv->m_Buf,vdess[1]));
		}
/*		if(twosided) {
			int sme=gtl->m_Buf.Len()-4*3;
			gtl->m_Buf.Dword(*(DWORD *)((BYTE *)gtl->m_Buf.Get()+sme));
			gtl->m_Buf.Dword(*(DWORD *)((BYTE *)gtl->m_Buf.Get()+sme+8));
			gtl->m_Buf.Dword(*(DWORD *)((BYTE *)gtl->m_Buf.Get()+sme+4));
		}*/
/*		vtp[1]=vtp[1]-vtp[0];
		vtp[2]=vtp[2]-vtp[0];
		vtp[0]=CrossProd(vtp[1],vtp[2]);
//		vtp[0].x=vtp[1].y*vtp[2].z-vtp[1].z*vtp[2].y;
//		vtp[0].y=vtp[1].z*vtp[2].x-vtp[1].x*vtp[2].z;
//		vtp[0].z=vtp[1].x*vtp[2].y-vtp[1].y*vtp[2].x;
		vtp[0]=Normalize(vtp[0]);
		vtp[1]=Normalize(tm * mesh->getFaceNormal(i));
		vtp[2]=Normalize(mesh->getFaceNormal(i));
		double d1=sqrt((vtp[0].x-vtp[1].x)*(vtp[0].x-vtp[1].x)+(vtp[0].y-vtp[1].y)*(vtp[0].y-vtp[1].y)+(vtp[0].z-vtp[1].z)*(vtp[0].z-vtp[1].z));
		double d2=sqrt((vtp[0].x-vtp[2].x)*(vtp[0].x-vtp[2].x)+(vtp[0].y-vtp[2].y)*(vtp[0].y-vtp[2].y)+(vtp[0].z-vtp[2].z)*(vtp[0].z-vtp[2].z));
		double d3=sqrt((vtp[0].x-vtp[2].x)*(vtp[0].x-vtp[2].x)+(vtp[0].y-vtp[2].y)*(vtp[0].y-vtp[2].y)+(vtp[0].z-vtp[2].z)*(vtp[0].z-vtp[2].z));*/
	}

	if(m_EdgeExport) {
		for(i=0; i<mesh->getNumFaces(); i++) {
			for(u=0; u<3; u++) {
				int vi2,vi1=mesh->faces[i].v[vx[u]];
				if(u<2) vi2=mesh->faces[i].v[vx[u+1]];
				else vi2=mesh->faces[i].v[vx[0]];

				int cnttri=0;

				for(int p=0; p<mesh->getNumFaces(); p++) {
					for(int k=0; k<3; k++) {
						int vu2,vu1=mesh->faces[p].v[vx[k]];
						if(k<2) vu2=mesh->faces[p].v[vx[k+1]];
						else vu2=mesh->faces[p].v[vx[0]];

						if(((vi1==vu1) && (vi2==vu2)) || ((vi1==vu2) && (vi2==vu1))) {
							cnttri++;

							if(cnttri==2)  {
//								if((m_EdgeExport==1) || ((mesh->getFaceNormal(i).Normalize()-mesh->getFaceNormal(p).Normalize()).LengthSquared()>1e-20/*0.000000001*/)) {
                                double dp=DotProd(mesh->getFaceNormal(i).Normalize(),mesh->getFaceNormal(p).Normalize());
								if((m_EdgeExport==1) || (dp<m_EdgeExportFactor/*0.000000001*/)) {
									gf->m_Buf.Dword((u<<28)|edgeindexlist[i]);
									gf->m_Buf.Dword((k<<28)|edgeindexlist[p]);
								}
							}
						}
					}
				}

				if(cnttri!=2) {
					mesh->faceSel.Set(i);
					TSTR str;
					node->EvalWorldState(timev).obj->GetClassName(str);

					Log(Base::CWStr().Format(L"У грани не 2 триугольника. Face:<i> Cnt: <i>  Name: <s>  Class name: <s>",
						i+1,
						cnttri,
						Base::CWStr(Base::CStr(node->NodeName())).Get(),
						Base::CWStr(Base::CStr(str.data())).Get() 
					));
				}

			}
		}
	}

	if(edgeindexlist) { HFree(edgeindexlist,NULL); edgeindexlist=NULL; }

	if (needDel) delete tri;
}

void EGExp2::ExportSimple_Helper(INode *node,TimeValue timev,int frame)
{
	TSTR tstr;
	
	SVOExpMatrixHeader emh;

    int id;
    Base::CWStr name;
    if(!ExportMatrixGetProp(node,m_GroupName,id,name)) return;

	Matrix3 tm = node->GetObjTMAfterWSM(timev)*m_MatCenter;

	CGroup * gv=GroupFindAdd(Base::CPoint(gt_ExpMatrix,frame),Base::CPoint(gt_ExpMatrix,int(node)));

	ZeroMemory(&emh,sizeof(SVOExpMatrixHeader));
    emh.m_Id=id;
    name=RenameByCfg(name);
    if(name.GetLen()>31) name.SetLen(31);
    CopyMemory(emh.m_Name,name.Get(),name.GetLen()*2);

	gv->m_Buf.Clear();
	gv->m_Buf.BufAdd(&emh,sizeof(SVOExpMatrixHeader));

	SVOMatrix em;
	tm.Orthogonalize();
	tm.NoScale();
	ConvertMatrix(em,tm);
	gv->m_Buf.BufAdd(&em,sizeof(SVOMatrix));
}

BOOL EGExp2::TMNegParity(Matrix3 &m)
{
    return (DotProd(CrossProd(m.GetRow(0),m.GetRow(1)),m.GetRow(2))<0.0)?1:0;
}

TriObject* EGExp2::GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt)
{
    deleteIt = FALSE;
    Object *obj = node->EvalWorldState(t).obj;
    if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
	    TriObject *tri = (TriObject *) obj->ConvertToType(t, 
		    Class_ID(TRIOBJ_CLASS_ID, 0));
	    // Note that the TriObject should only be deleted
	    // if the pointer to it is not equal to the object
	    // pointer that called ConvertToType()
	    if (obj != tri) deleteIt = TRUE;
	    return tri;
    } else {
	    return NULL;
    }
}

Point3 EGExp2::GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv)
{
    Face* f = &mesh->faces[faceNo];
    DWORD smGroup = f->smGroup;
    int numNormals;
    Point3 vertexNormal;

    // Is normal specified
    // SPCIFIED is not currently used, but may be used in future versions.
    if (rv->rFlags & SPECIFIED_NORMAL) {
	    vertexNormal = rv->rn.getNormal();
    }
    // If normal is not specified it's only available if the face belongs
    // to a smoothing group
    else if ((numNormals = rv->rFlags & NORCT_MASK) && smGroup) {
	    // If there is only one vertex is found in the rn member.
	    if (numNormals == 1) {
		    vertexNormal = rv->rn.getNormal();
	    } else {
		    // If two or more vertices are there you need to step through them
		    // and find the vertex with the same smoothing group as the current face.
		    // You will find multiple normals in the ern member.
		    for (int i = 0; i < numNormals; i++) {
			    if (rv->ern[i].getSmGroup() & smGroup) {
				    vertexNormal = rv->ern[i].getNormal();
			    }
		    }
	    }
    } else {
	    // Get the normal from the Face if no smoothing groups are there
	    vertexNormal = mesh->getFaceNormal(faceNo);
    }

    return vertexNormal;
}
