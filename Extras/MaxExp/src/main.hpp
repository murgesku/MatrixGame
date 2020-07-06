#ifndef __MAIN__H
#define __MAIN__H

// Edge=<0,1,2>     0-off 1-all 2-optimize

struct SVOHeader {
	DWORD m_Id;				// 0x00006f76
	DWORD m_Ver;			// Версия
	DWORD m_Flags;			// {1-16 битный индекс иначе 32 битный} {2-default texture}
	DWORD r1;
	DWORD m_MaterialCnt;	// Список материалов(текстур) SMaterial
	DWORD m_MaterialSme;	// Положение от начала заголовка
	DWORD m_GroupCnt;		// Инвормация по группам (Смещение верши и треугольников)
	DWORD m_GroupSme;
	DWORD m_VerCnt;			// Список всех вершин SVertexNorTex
	DWORD m_VerSme;
	DWORD m_TriCnt;			// Список всех треугольников. Кол-во индексов (3 индкса для каждого трегольника по 2 или 4 байта взависимости от флага)
	DWORD m_TriSme;
	DWORD m_FrameCnt;		// Список кадров SVOFrame
	DWORD m_FrameSme;
	DWORD m_AnimCnt;		// Список анимаций SVOAnimHeader
	DWORD m_AnimSme;
	DWORD m_MatrixCnt;		// Список матриц SVOExpMatrixHeader
	DWORD m_MatrixSme;
	DWORD m_EdgeCnt;		// Список всех граней
	DWORD m_EdgeSme;
};

struct SVOGroup {
	DWORD m_Material;
	DWORD m_Flags;			// 0-list
	DWORD m_VerCnt;
	DWORD m_VerStart;
	DWORD m_TriCnt;			// Кол-во индексов
	DWORD m_TriStart;
	DWORD r1;
	DWORD r2;
};

struct SVOFrame {
	DWORD m_GroupIndexCnt;
	DWORD m_GroupIndexSme;	// Каждый индекс 4-байтный указатель
	float m_CenterX,m_CenterY,m_CenterZ;
	float m_RadiusCenter;
	float m_MinX,m_MinY,m_MinZ;
	float m_MaxX,m_MaxY,m_MaxZ;
	float m_RadiusBox;
	DWORD m_EdgeCnt;
	DWORD m_EdgeStart;
	DWORD r1;
};

struct SVOAnimHeader {
	DWORD m_Id;
	wchar m_Name[32];
	DWORD m_UnitCnt;
	DWORD m_UnitSme;
	DWORD r1;
};

struct SVOAnimUnit {
	DWORD m_FrameIndex;
	DWORD m_Time; // in milisecond
};

struct SVOExpMatrixHeader {
	DWORD m_Id;
	wchar m_Name[32];
	DWORD m_MatrixSme; // Спиок SVOMatrix   (кол-во по количеству m_FrameCnt)
	DWORD r1;
	DWORD r2;
};

struct SVOMatrix {
    union {
        struct {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;

        };
        float m[4][4];
    };
};

struct SVOEdge {
	DWORD m_SideTri1; // 0xf0000000 - triangle side	0x0ff00000 - group in frame		0x000fffff - triangle
	DWORD m_SideTri2; // 0xf0000000 - triangle side	0x0ff00000 - group in frame		0x000fffff - triangle
};

extern ClassDesc* GetEGExpDesc();
extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;
extern ClassDesc * GetParObjModClassDesc();
extern ClassDesc * GetParGroupModClassDesc();
extern ClassDesc * GetParCenterModClassDesc();
extern ClassDesc * GetParMatrixModClassDesc();

#define VERSION			200			// Version number * 100
//#define FLOAT_OUTPUT	_T("%4.4f")	// Float precision for output
#define CFGFILENAME		_T("EGExp.CFG")	// Configuration file

class MtlKeeper {
public:
	BOOL	AddMtl(Mtl* mtl);
	int		GetMtlID(Mtl* mtl);
	int		Count();
	Mtl*	GetMtl(int id);

	Tab<Mtl*> mtlTab;
};

struct SVertexNorTex {
	float x,y,z;
	float nx,ny,nz;
	float tu,tv;
};

struct SMaterial {
	DWORD m_Id;					// Pointer on material
	float dr,dg,db,da;
	float ar,ag,ab,aa;
	float sr,sg,sb,sa;
	float er,eg,eb,ea;
	float power;
	wchar tex_diffuse[32];
};

#define gt_Ver 1
#define gt_Material 2
#define gt_TriList 3
#define gt_Frame 4
#define gt_ExpMatrix 5
#define gt_Edge 6

class CGroup:public Base::CMain {
	public:
		CGroup * m_Prev;
		CGroup * m_Next;

		DWORD m_User;
		DWORD m_User2;
		DWORD m_User3;
		DWORD m_User4;

		int m_No;
		int m_IdCnt;
		Base::CPoint m_Id[4];
		Base::CBuf m_Buf;
	public:
		CGroup():m_Id(),m_Buf() { m_Prev=NULL; m_Next=NULL; m_IdCnt=0; m_No=0; }
		~CGroup() {}
};

class EGExp2 {
    public:
		Base::CBlockPar m_Rename;
		Base::CBlockPar m_Anim;

		Interface * ip;

		int m_MaterialNextNo;
		CGroup * m_GroupFirst;
		CGroup * m_GroupLast;

		Base::CWStr m_FileName;
        Base::CWStr m_GroupName;
        bool m_TextureDefault;

		Base::CBuf m_Log;

		Matrix3 m_MatCenter;
		Point3 m_Scale;
		double m_MaxSizeX,m_MaxSizeY,m_MaxSizeZ;
		int m_EdgeExport; // 0-off 1-all 2-optimize
        double m_EdgeExportFactor; // in gradus

    public:
        EGExp2();
        ~EGExp2();

		void GroupClear(void);
		void GroupDelete(CGroup * el);
		int GroupCount(void)														{ int cnt=0; CGroup * g=m_GroupFirst; while(g) { cnt++; g=g->m_Next; } return cnt; }
		CGroup * GroupFind(Base::CPoint & p1);
		CGroup * GroupFind(Base::CPoint & p1, Base::CPoint & p2);
		CGroup * GroupFind(Base::CPoint & p1, Base::CPoint & p2, Base::CPoint & p3);
		CGroup * GroupAdd(Base::CPoint & p1);
		CGroup * GroupAdd(Base::CPoint & p1, Base::CPoint & p2);
		CGroup * GroupAdd(Base::CPoint & p1, Base::CPoint & p2, Base::CPoint & p3);
		CGroup * GroupFindAdd(Base::CPoint & p1) { CGroup * g=GroupFind(p1); if(g) return g; return GroupAdd(p1); }
		CGroup * GroupFindAdd(Base::CPoint & p1, Base::CPoint & p2) { CGroup * g=GroupFind(p1,p2); if(g) return g; return GroupAdd(p1,p2); }
		CGroup * GroupFindAdd(Base::CPoint & p1, Base::CPoint & p2, Base::CPoint & p3) { CGroup * g=GroupFind(p1,p2,p3); if(g) return g; return GroupAdd(p1,p2,p3); }

		void GroupSortByNo(void);

		void Log(Base::CWStr str);

		int GroupMaterialFindAdd(Mtl * ml, int sm, int & twosided);
		int AddVertex(Base::CBuf & buf,SVertexNorTex & v);

        void ExportSimpleGroup(void);
        void ExportSimple(void);
		BOOL ExportSimple_NodeEnum(INode *node,TimeValue timev,int frame);
		void ExportSimple_GeomObject(INode *node,TimeValue timev,int frame);
		void ExportSimple_Helper(INode *node,TimeValue timev,int frame);

		Base::CWStr RenameByCfg(Base::CWStr & str);

        BOOL	TMNegParity(Matrix3 &m);
        TriObject* GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);
        Point3 EGExp2::GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv);
};

class EGExp : public SceneExport {
	public:
		Base::CBlockPar m_Cfg;
		Base::CBlockPar m_CfgFromScene;
		Base::CBlockPar * m_Set;
		Interface * ip;

		WNDPROC m_OldProc;

		int m_MaterialNextNo;
		CGroup * m_GroupFirst;
		CGroup * m_GroupLast;

		Base::CWStr m_FileName;
        Base::CWStr m_GroupName;
        bool m_TextureDefault;

		Base::CBuf m_Log;

		Matrix3 m_MatCenter;
		Point3 m_Scale;
		double m_MaxSizeX,m_MaxSizeY,m_MaxSizeZ;
		int m_EdgeExport; // 0-off 1-all 2-optimize
        double m_EdgeExportFactor; // in gradus

	public:
		EGExp();
		~EGExp();

		void GroupClear(void);
		void GroupDelete(CGroup * el);
		int GroupCount(void)														{ int cnt=0; CGroup * g=m_GroupFirst; while(g) { cnt++; g=g->m_Next; } return cnt; }
		CGroup * GroupFind(Base::CPoint & p1);
		CGroup * GroupFind(Base::CPoint & p1, Base::CPoint & p2);
		CGroup * GroupFind(Base::CPoint & p1, Base::CPoint & p2, Base::CPoint & p3);
		CGroup * GroupAdd(Base::CPoint & p1);
		CGroup * GroupAdd(Base::CPoint & p1, Base::CPoint & p2);
		CGroup * GroupAdd(Base::CPoint & p1, Base::CPoint & p2, Base::CPoint & p3);
		CGroup * GroupFindAdd(Base::CPoint & p1) { CGroup * g=GroupFind(p1); if(g) return g; return GroupAdd(p1); }
		CGroup * GroupFindAdd(Base::CPoint & p1, Base::CPoint & p2) { CGroup * g=GroupFind(p1,p2); if(g) return g; return GroupAdd(p1,p2); }
		CGroup * GroupFindAdd(Base::CPoint & p1, Base::CPoint & p2, Base::CPoint & p3) { CGroup * g=GroupFind(p1,p2,p3); if(g) return g; return GroupAdd(p1,p2,p3); }

		void GroupSortByNo(void);

		void Log(Base::CWStr str);

		int GroupMaterialFindAdd(Mtl * ml, int sm, int & twosided);
		int AddVertex(Base::CBuf & buf,SVertexNorTex & v);

        void ExportSimpleGroup(void);
        void ExportSimple(Base::CBlockPar * group);
		BOOL ExportSimple_NodeEnumFirst(INode *node,TimeValue timev,int frame);
		BOOL ExportSimple_NodeEnum(INode *node,TimeValue timev,int frame);
		void ExportSimple_GeomObject(INode *node,TimeValue timev,int frame);
		void ExportSimple_Helper(INode *node,TimeValue timev,int frame);

		bool LoadCfgFromScene(INode * node=NULL);

		Base::CWStr RenameByCfg(Base::CWStr & str);

	// SceneExport methods
	int    ExtCount();     // Number of extensions supported 
	const TCHAR * Ext(int n);     // Extension #n (i.e. "ASC")
	const TCHAR * LongDesc();     // Long ASCII description (i.e. "Ascii Export") 
	const TCHAR * ShortDesc();    // Short ASCII description (i.e. "Ascii")
	const TCHAR * AuthorName();    // ASCII Author name
	const TCHAR * CopyrightMessage();   // ASCII Copyright message 
	const TCHAR * OtherMessage1();   // Other message #1
	const TCHAR * OtherMessage2();   // Other message #2
	unsigned int Version();     // Version number * 100 (i.e. v3.01 = 301) 
	void	ShowAbout(HWND hWnd);  // Show DLL's "About..." box
	int		DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0); // Export	file
	BOOL	SupportsOptions(int ext, DWORD options);

	// Other methods

	// Node enumeration
	BOOL	nodeEnum(INode* node, int indentLevel);
	void	PreProcess(INode* node, int& nodeCount);

	// High level export
	void	ExportMaterialList();
	void	ExportGeomObject(INode* node, int indentLevel); 
	void	ExportLightObject(INode* node, int indentLevel); 
	void	ExportCameraObject(INode* node, int indentLevel); 
	void	ExportShapeObject(INode* node, int indentLevel); 
	void	ExportHelperObject(INode* node, int indentLevel);

	// Mid level export
	void	ExportMesh(INode* node, TimeValue t, int indentLevel); 
	void	ExportMaterial(INode* node, int indentLevel); 
	void	ExportAnimMesh(INode* node, int indentLevel); 
	void	ExportIKJoints(INode* node, int indentLevel);
	void	ExportNodeTM(INode* node, int indentLevel);
	void	ExportNodeHeader(INode* node, TCHAR* type, int indentLevel);
	void	ExportCameraSettings(CameraState* cs, CameraObject* cam, TimeValue t, int indentLevel);
	void	ExportLightSettings(LightState* ls, GenLight* light, TimeValue t, int indentLevel);

	// Low level export
	void	DumpPoly(PolyLine* line, Matrix3 tm, int indentLevel);
	void	DumpMatrix3(Matrix3* m, int indentLevel);
	void	DumpMaterial(Mtl* mtl, int mtlID, int subNo, int indentLevel);
	void	DumpTexture(Texmap* tex, Class_ID cid, int subNo, float amt, int	indentLevel);
	void	DumpJointParams(JointParams* joint, int indentLevel);
	void	DumpUVGen(StdUVGen* uvGen, int indentLevel); 
	
	// Misc methods
	TCHAR*	GetMapID(Class_ID cid, int subNo);
	Point3	GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv);
	BOOL	CheckForAndExportFaceMap(Mtl* mtl, Mesh* mesh, int indentLevel); 
	void	make_face_uv(Face *f, Point3 *tv);
	BOOL	TMNegParity(Matrix3 &m);
	TSTR	GetIndent(int indentLevel);
	TCHAR*	FixupName(TCHAR* name);
	void	CommaScan(TCHAR* buf);
	TriObject*	GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);

	// A collection of overloaded value to string converters.
	TSTR	Format(int value);
	TSTR	Format(float value);
	TSTR	Format(Color value);
	TSTR	Format(Point3 value); 
	TSTR	Format(AngAxis value); 
	TSTR	Format(Quat value);
	TSTR	Format(ScaleValue value);

	// Interface to member variables
	inline BOOL	GetIncludeMesh()			{ return bIncludeMesh; }
	inline BOOL	GetIncludeMtl()				{ return bIncludeMtl; }
	inline BOOL	GetIncludeMeshAnim()		{ return bIncludeMeshAnim; }
	inline BOOL	GetIncludeCamLightAnim()	{ return bIncludeCamLightAnim; }
	inline BOOL	GetIncludeIKJoints()		{ return bIncludeIKJoints; }
	inline BOOL	GetIncludeNormals()			{ return bIncludeNormals; }
	inline BOOL	GetIncludeTextureCoords()	{ return bIncludeTextureCoords; }
	inline BOOL	GetIncludeVertexColors()	{ return bIncludeVertexColors; }
	inline BOOL	GetIncludeObjGeom()			{ return bIncludeObjGeom; }
	inline BOOL	GetIncludeObjShape()		{ return bIncludeObjShape; }
	inline BOOL	GetIncludeObjCamera()		{ return bIncludeObjCamera; }
	inline BOOL	GetIncludeObjLight()		{ return bIncludeObjLight; }
	inline BOOL	GetIncludeObjHelper()		{ return bIncludeObjHelper; }
	inline BOOL	GetAlwaysSample()			{ return bAlwaysSample; }
	inline int	GetMeshFrameStep()			{ return nMeshFrameStep; }
	inline int	GetKeyFrameStep()			{ return nKeyFrameStep; }
	inline int	GetPrecision()				{ return nPrecision; }
	inline TimeValue GetStaticFrame()		{ return nStaticFrame; }
	inline Interface*	GetInterface()		{ return ip; }


	inline void	SetIncludeMesh(BOOL val)			{ bIncludeMesh = val; }
	inline void	SetIncludeMtl(BOOL val)				{ bIncludeMtl = val; }
	inline void	SetIncludeMeshAnim(BOOL val)		{ bIncludeMeshAnim = val; }
	inline void	SetIncludeCamLightAnim(BOOL val)	{ bIncludeCamLightAnim = val; }
	inline void	SetIncludeIKJoints(BOOL val)		{ bIncludeIKJoints = val; }
	inline void	SetIncludeNormals(BOOL val)			{ bIncludeNormals = val; }
	inline void	SetIncludeTextureCoords(BOOL val)	{ bIncludeTextureCoords = val; }
	inline void	SetIncludeVertexColors(BOOL val)	{ bIncludeVertexColors = val; }
	inline void	SetIncludeObjGeom(BOOL val)			{ bIncludeObjGeom = val; }
	inline void	SetIncludeObjShape(BOOL val)		{ bIncludeObjShape = val; }
	inline void	SetIncludeObjCamera(BOOL val)		{ bIncludeObjCamera = val; }
	inline void	SetIncludeObjLight(BOOL val)		{ bIncludeObjLight = val; }
	inline void	SetIncludeObjHelper(BOOL val)		{ bIncludeObjHelper = val; }
	inline void	SetAlwaysSample(BOOL val)			{ bAlwaysSample = val; }
	inline void SetMeshFrameStep(int val)			{ nMeshFrameStep = val; }
	inline void SetKeyFrameStep(int val)			{ nKeyFrameStep = val; }
	inline void SetPrecision(int val)				{ nPrecision = val; }
	inline void SetStaticFrame(TimeValue val)		{ nStaticFrame = val; }

private:
	BOOL	bIncludeMesh;
	BOOL	bIncludeMtl;
	BOOL	bIncludeMeshAnim;
	BOOL	bIncludeCamLightAnim;
	BOOL	bIncludeIKJoints;
	BOOL	bIncludeNormals;
	BOOL	bIncludeTextureCoords;
	BOOL	bIncludeObjGeom;
	BOOL	bIncludeObjShape;
	BOOL	bIncludeObjCamera;
	BOOL	bIncludeObjLight;
	BOOL	bIncludeObjHelper;
	BOOL	bAlwaysSample;
	BOOL	bIncludeVertexColors;
	int		nMeshFrameStep;
	int		nKeyFrameStep;
	int		nPrecision;
 	TimeValue	nStaticFrame;

	FILE*		pStream;
	int			nTotalNodeCount;
	int			nCurNode;
	TCHAR		szFmtStr[16];

	MtlKeeper	mtlList;
};

extern void ExportGroupSetRename(const Base::CWStr & value,INode *node=NULL);
extern void ExportGroupGetRename(Base::CWStr & value,INode *node=NULL);
extern int ExportGroupCount(INode *node=NULL);
extern bool ExportGroupGetName(Base::CWStr & tstr,int no,int & nocur,INode *node=NULL);
extern bool ExportGroupGetProp(int no,int & nocur,INode *node,
                               Base::CWStr & name,int & export,Base::CWStr & file,bool & textureDefault,int & edgetype,double & edgefactor,Base::CWStr & anim);
extern bool ExportCenterGetProp(const Base::CWStr & group,TimeValue timev,INode *node,
                                double & maxsizex,double & maxsizey,double & maxsizez,float & scalex,float & scaley,float & scalez,Matrix3 & center);
extern bool ExportObjGetProp(INode *node,
                             Base::CWStr & group, int & export);
extern bool ExportMatrixGetProp(INode *node, const Base::CWStr & group,
                                int & id, Base::CWStr & name);

#endif

