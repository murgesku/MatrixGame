// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once

#include "d3d9.h"
#include "d3dx9tex.h"

#include "Cache.hpp"
#include "D3DControl.hpp"
#include "BigVB.hpp"
#include "BigIB.hpp"
#include "ShadowProj.hpp"
#include "CBillboard.hpp"
#include "CReminder.hpp"

extern IDirect3DDevice9 *g_D3DD;

#define FREE_TEXTURE_TIME_PERIOD (7000)
#define FREE_VO_TIME_PERIOD      (30000)

class CVOShadowStencil;

#define GLOSS_TEXTURE_SUFFIX L"_gloss"

#define SHADOW_ALTITUDE (0.7f)

// resources
#define VOUF_MATRIX        SETBIT(0)
#define VOUF_SHADOWSTENCIL SETBIT(1)
#define VOUF_RES           (VOUF_MATRIX | VOUF_SHADOWSTENCIL)

#define VOUF_MATRIX_USE SETBIT(2)
#define VOUF_STENCIL_ON SETBIT(3)

enum EObjectLoad { OLF_NO_TEX, OLF_MULTIMATERIAL_ONLY, OLF_AUTO, OLF_SIMPLE };

struct SVOSurface;
struct SSkin;

typedef const SSkin *(*SKIN_GET)(const wchar *tex, DWORD param);  // skin setup

typedef void (*SKIN_SETUP_TEX)(const SSkin *vo, DWORD user_param, int pass);
typedef bool (*SKIN_SETUP_STAGES)(const SSkin *vo, DWORD user_param, int pass);
typedef void (*SKIN_SETUP_CLEAR)(const SSkin *vo, DWORD user_param);
typedef void (*SKIN_SETUP_SHADOW)(const SSkin *vo);
typedef void (*SKIN_PRELOAD)(const SSkin *vo);

#define DEFAULT_ANIM_FRAME_PERIOD 100

// file formats

struct SVOHeader {
    DWORD m_Id;   // 0x00006f76
    DWORD m_Ver;  // Версия
    DWORD m_Flags;  // 1-16 битный индекс иначе 32 битный, 2-откуда брать текстуры
    DWORD dummy;
    int m_MaterialCnt;    // Список материалов(текстур) SMaterial
    DWORD m_MaterialSme;  // Положение от начала заголовка
    int m_GroupCnt;  // Инвормация по группам (Смещение верши и треугольников)
    DWORD m_GroupSme;
    int m_VerCnt;  // Список всех вершин SVertexNorTex
    DWORD m_VerSme;
    int m_TriCnt;  // Список всех треугольников. Кол-во индексов (3 индкса для каждого трегольника по 2 или 4 байта
                   // взависимости от флага)
    DWORD m_TriSme;
    int m_FrameCnt;  // Список кадров SVOFrame
    DWORD m_FrameSme;
    int m_AnimCnt;  // Список анимаций SVOAnimHeader
    DWORD m_AnimSme;
    int m_MatrixCnt;  // Список матриц SVOExpMatrixHeader
    DWORD m_MatrixSme;
    DWORD m_EdgeCnt;  // Список всех граней
    DWORD m_EdgeSme;
};
struct SVOVertexNorTex {
    D3DXVECTOR3 v;
    D3DXVECTOR3 n;
    float tu, tv;
};

struct SVOMaterial {
    DWORD dummy;
    float dr, dg, db, da;
    float ar, ag, ab, aa;
    float sr, sg, sb, sa;
    float er, eg, eb, ea;
    float power;
    wchar tex_diffuse[32];
};

struct SVOGroup {
    DWORD m_Material;
    DWORD m_Flags;  // 0-list
    DWORD m_VerCnt;
    DWORD m_VerStart;
    DWORD m_TriCnt;  // Кол-во индексов
    DWORD m_TriStart;
    DWORD dummy[2];
};

struct SVOFrame {
    DWORD m_GroupIndexCnt;
    DWORD m_GroupIndexSme;  // Каждый индекс 4-байтный указатель
    float m_CenterX, m_CenterY, m_CenterZ;
    float m_RadiusCenter;
    float m_MinX, m_MinY, m_MinZ;
    float m_MaxX, m_MaxY, m_MaxZ;
    float m_RadiusBox;
    DWORD m_EdgeCnt;
    DWORD m_EdgeStart;
    DWORD dummy;
};

struct SVOAnimHeader {
    DWORD m_Id;
    wchar m_Name[32];
    DWORD m_UnitCnt;
    DWORD m_UnitSme;
    DWORD r1;
};

struct SVOMatrixHeader {
    DWORD m_Id;
    wchar m_Name[32];
    DWORD m_MatrixSme;  // Спиок SVOMatrix   (кол-во по количеству m_FrameCnt)
    DWORD r1;
    DWORD r2;
};

struct SVOFrameEdge {
    DWORD m_SideTri1;  // 0xf0000000 - triangle side		0x0ff00000 - group   0x000fffff - triangle
    DWORD m_SideTri2;  // 0xf0000000 - triangle side		0x0ff00000 - group   0x000fffff - triangle
};

struct SVOEdge {
    int m_Tri1;
    int m_Tri2;
    byte m_Edge1;
    byte m_Enge2;
};

struct SVOEdgeGroup {
    int m_EdgeSme;
    int m_EdgeCnt;
};

////////////////
// memory VO

#define VO_FVF (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
struct SVOVertex {
    D3DXVECTOR3 v;
    D3DXVECTOR3 n;
    float tu, tv;

    static const DWORD FVF = VO_FVF;
};

struct SVOTriangle {
    union {
        struct {
            int i0, i1, i2;
        };
        int i[3];
    };
    // D3DXVECTOR3 norm;
};

struct SVOIndices16 {
    word i0, i1, i2;
};

struct SVOIndices32 {
    dword i0, i1, i2;
};

struct SVOSurface {
    const SSkin *skin;
    std::wstring texname;

    //   float dr,dg,db,da;
    // float ar,ag,ab,aa;
    // float sr,sg,sb,sa;
    // float er,eg,eb,ea;
    // float power;
};

struct SVOUnion {
    int m_Surface;
    int m_Base;  // used for draw indexed primitives
    union {
        int m_VerMinIndex;  // used for draw indexed primitives // always zero for optimized meshes (or negative, see
                            // m_IBase)
        int m_IBase;        // negative value!
    };
    int m_VerCnt;  // used for draw indexed primitives
    int m_TriCnt;
    int m_TriStart;
};

struct SVOKadr {
    D3DXVECTOR3 m_Min;
    D3DXVECTOR3 m_Max;
    D3DXVECTOR3 m_GeoCenter;
    float m_Radius;

    int m_UnionStart;
    int m_UnionCnt;
    int m_EdgeStart;
    int m_EdgeCnt;

    int m_VerCnt;    // used in pick
    int m_VerStart;  // used in pick
};

struct SVOFrameRuntime {
    int m_EdgeVertexIndexMin;
    int m_EdgeVertexIndexCount;
};

struct SVOAnimation {
    DWORD m_Id;
    wchar m_Name[32];
    DWORD m_FramesCnt;
    DWORD m_FramesStart;
};

struct SVOMatrix {
    DWORD m_Id;
    wchar m_Name[32];
    DWORD m_MatrixStart;  // Спиок D3DXMATRIX  (кол-во по количеству кадров)
};

struct SVOFrameIndex {
    int frame;
    int time;
};

struct SVONormal {
    union {
        struct {
            BYTE x, y, z, s;
        };
        DWORD all;
    };
};

struct SVOKadrEdge {
    int v00, v01;      // verts of triangles with common edge (disp in bytes)
    SVONormal n0, n1;  // normals
};

struct SVOGeometrySimple {
    struct SVOGS_asis {
        int m_TriCnt;
        int m_VertsCnt;
        D3DXVECTOR3 m_Mins;
        D3DXVECTOR3 m_Maxs;
        D3DXVECTOR3 m_Center;
        float m_Radius;
    } m_AsIs;

    SVOTriangle *m_Tris;
    D3DXVECTOR3 *m_Verts;
};

struct SVOGeometry {
    int *m_VerticesIdx;  // idxs for frames

    SBigVBSource<SVOVertex> m_Vertices;
    // SVOVertex      *m_Vertices;
    // int             m_VerticesCnt;

    SVOTriangle *m_Triangles;
    int m_TrianglesCnt;
    // union
    //{
    //    SVOIndices16 *m_Idx16;
    //    SVOIndices32 *m_Idx32;
    //};
    // int             m_IdxCnt;
    SBigIBSource m_Idxs;
    // int             m_IdxStride;    // 2 or 4

    DWORD *m_UnionsIdx;  // idxs for frames
    SVOUnion *m_Unions;
    int m_UnionsCnt;

    SVOSurface *m_Surfaces;
    int m_SurfacesCnt;

    SVOKadrEdge *m_Edges;
    int m_EdgesCnt;

    SVOFrameIndex *m_FramesIdx;  // idx for animations
    SVOKadr *m_Frames;
    int m_FramesCnt;

    SVOFrameRuntime *m_FramesRuntime;

    SVOAnimation *m_Animations;
    int m_AnimationsCnt;

    SVOMatrix *m_Matrixs;
    int m_MatrixsCnt;

    D3DXMATRIX *m_AllMatrixs;
    // SVONormal      *m_Normals;
};

typedef bool (*ENUM_VERTS_HANDLER)(const SVOVertex &v, DWORD data);

class CVectorObjectAnim;
class CVectorObject : public CCacheData {
    static CBigVB<SVOVertex> *m_VB;
    static CBigIB *m_IB;

    SVOGeometry m_Geometry;
    SVOGeometrySimple *m_GeometrySimple;

    CBlockPar m_Props;

    static SVOHeader *Header(const CBuf &buf) { return (SVOHeader *)buf.Get(); }

    bool PickSimple(const D3DXMATRIX &ma, const D3DXMATRIX &ima, const D3DXVECTOR3 &origin, const D3DXVECTOR3 &dir,
                    float *outt) const;

    SRemindCore m_RemindCore;

public:
    friend class CVOShadowStencil;

    SVOGeometrySimple *GetGS(void) { return m_GeometrySimple; }

    static void StaticInit(void) {
        m_VB = NULL;
        m_IB = NULL;
    }

    CVectorObject(void);
    virtual ~CVectorObject();

    static void DrawBegin(void) {
        m_IB->BeforeDraw();
        m_VB->BeforeDraw();
        ASSERT_DX(g_D3DD->SetFVF(VO_FVF));
    }
    static void DrawEnd(void) {}

    static void MarkAllBuffersNoNeed(void) {
        if (m_VB)
            m_VB->ReleaseBuffers();
        if (m_IB)
            m_IB->ReleaseBuffers();
    }

    // bool    DX_IsEmpty(void) {return !IS_VB(m_IB);}
    void DX_Prepare(void) {
        DTRACE();
        m_Geometry.m_Vertices.Prepare(m_VB);
        m_Geometry.m_Idxs.Prepare(m_IB);
    };
    void DX_Free(void) {
        m_Geometry.m_Vertices.MarkNoNeed(m_VB);
        m_Geometry.m_Idxs.MarkNoNeed(m_IB);
    }

    int GetFramesCnt(void) const { return m_Geometry.m_FramesCnt; }
    const SVOKadr *GetFrame(int no) const { return m_Geometry.m_Frames + no; }

    const wchar *GetAnimName(int i) { return m_Geometry.m_Animations[i].m_Name; }

    int GetAnimCount(void) const { return m_Geometry.m_AnimationsCnt; }
    int GetAnimById(DWORD id);
    int GetAnimByName(const std::wstring& name);
    int GetAnimFramesCount(int no) const { return m_Geometry.m_Animations[no].m_FramesCnt; }
    int GetAnimFrameTime(int anim, int frame) const {
        return abs(m_Geometry.m_FramesIdx[m_Geometry.m_Animations[anim].m_FramesStart + frame].time);
    }
    int GetAnimLooped(int anim) const {
        return m_Geometry.m_FramesIdx[m_Geometry.m_Animations[anim].m_FramesStart].time > 0;
    }
    int GetAnimFrameIndex(int anim, int frame) const {
        return m_Geometry.m_FramesIdx[m_Geometry.m_Animations[anim].m_FramesStart + frame].frame;
    }

    std::wstring GetPropValue(const wchar *name) const { return m_Props.ParGetNE(name); }

    const D3DXVECTOR3 &GetFrameGeoCenter(int frame) const { return m_Geometry.m_Frames[frame].m_GeoCenter; }

    int GetMatrixCount(void) const { return m_Geometry.m_MatrixsCnt; }
    const D3DXMATRIX *GetMatrixById(int frame, DWORD id) const;
    const D3DXMATRIX *GetMatrixByName(int frame, const std::wstring& name) const;
    const D3DXMATRIX *GetMatrix(int frame, int matrix) const {
        return m_Geometry.m_AllMatrixs + (m_Geometry.m_Matrixs + matrix)->m_MatrixStart + frame;
    }
    const wchar *GetMatrixNameById(DWORD id) const;
    const wchar *GetMatrixName(int idx) const { return m_Geometry.m_Matrixs[idx].m_Name; }
    DWORD GetMatrixId(int idx) const { return m_Geometry.m_Matrixs[idx].m_Id; }

    bool Pick(int noframe, const D3DXMATRIX &ma, const D3DXMATRIX &ima, const D3DXVECTOR3 &origin,
              const D3DXVECTOR3 &dir, float *outt) const;
    bool PickFull(int noframe, const D3DXMATRIX &ma, const D3DXMATRIX &ima, const D3DXVECTOR3 &origin,
                  const D3DXVECTOR3 &dir, float *outt) const;

    void EnumFrameVerts(int noframe, ENUM_VERTS_HANDLER evh, DWORD data) const;

    const std::wstring &GetSurfaceFileName(int i) const { return m_Geometry.m_Surfaces[i].texname; }

    // void EdgeClear(void);
    // void EdgeBuild(void);
    // bool EdgeExist(void)			{ return m_Edge!=NULL; }

    void GetBound(int noframe, const D3DXMATRIX &objma, D3DXVECTOR3 &bmin, D3DXVECTOR3 &bmax) const;

    void CalcShadowProjMatrix(int noframe, SProjData &pd, D3DXVECTOR3 &dir, float addsize);
    static void CalcShadowProjMatrix(int cnt, CVectorObjectAnim **obj, const int *noframe, const D3DXMATRIX *wm,
                                     SProjData &pd, D3DXVECTOR3 &dir, float addsize);
    static CTextureManaged *CalcShadowTexture(int cnt, CVectorObjectAnim **obj, const int *noframe,
                                              const D3DXMATRIX *wm, const SProjData &pd, int texsize,
                                              CVOShadowCliper *cliper, CBaseTexture *tex_to_update = NULL);
    static CTextureManaged *CalcShadowTextureWOMat(int cnt, CVectorObjectAnim **obj, const int *noframe,
                                                   const D3DXMATRIX *wm, int texsize, CVOShadowCliper *cliper,
                                                   CBaseTexture *tex_to_update = NULL);

    void BeforeDraw(void);
    void Draw(int noframe, DWORD user_param, const SSkin *ds);

    void LoadSpecial(EObjectLoad flags, SKIN_GET sg, DWORD gsp);
    void PrepareSpecial(EObjectLoad flags, SKIN_GET sg, DWORD gsp) {
        if (!IsLoaded())
            LoadSpecial(flags, sg, gsp);
    };

    bool IsNoSkin(int surf) { return m_Geometry.m_Surfaces[surf].skin == NULL; }

    virtual bool IsLoaded(void) { return (m_Geometry.m_Vertices.verts != NULL); }
    virtual void Unload(void);
    virtual void Load(void) { LoadSpecial(OLF_NO_TEX, NULL, 0); };
};

struct SSkin {
    SKIN_SETUP_TEX m_SetupTex;
    SKIN_SETUP_STAGES m_SetupStages;
    SKIN_SETUP_CLEAR m_SetupClear;
    SKIN_SETUP_SHADOW m_SetupTexShadow;  // used only for shadow projecting
    SKIN_PRELOAD m_Preload;
};

struct SColorInterval {
    DWORD c1, c2;
    int time1, time2;
};

struct SLightData {
    SColorInterval *intervals;
    int intervals_cnt;
    int time;
    int period;
    DWORD matid;

    BYTE bbytes[sizeof(CBillboard)];

    CBillboard &BB(void) { return *(CBillboard *)&bbytes; }

    void Release(void);  // just free internal data. not itself
};

class CVectorObjectAnim : public CMain {
    CVectorObject *m_VO;

    const SSkin *m_Skin;

    int m_Time;
    int m_TimeNext;

    int m_Frame;
    int m_VOFrame;
    int m_Anim;

    int m_AnimLooped;

    SLightData *m_Lights;
    int m_LightsCnt;

    void SetLoopStatus(void) {
        if (m_VO->GetAnimLooped(m_Anim)) {
            m_AnimLooped = 1;
        }
        else {
            m_AnimLooped = 0;
        }
    }

public:
    CVectorObjectAnim(void);
    ~CVectorObjectAnim();

    void Clear(void);

    CVectorObject *VO(void) { return m_VO; }
    const SSkin *GetSkin(void) const { return m_Skin; }
    void SetSkin(const SSkin *s) { m_Skin = s; }

    void Init(CVectorObject *vo, CTextureManaged *tex_light, const SSkin *skin = NULL);
    void InitLights(CTextureManaged *tex_light);

    bool IsAnim(const wchar *name) { return wcscmp(VO()->GetAnimName(m_Anim), name) == 0; }

    int GetAnimIndex(void) const { return m_Anim; }
    void SetAnimDefault(void) {
        m_Anim = 0;
        FirstFrame();
        SetLoopStatus();
    }
    void SetAnimDefault(int loop) {
        m_Anim = 0;
        FirstFrame();
        m_AnimLooped = loop;
    }

    void SetAnimByIndex(int idx) {
        ASSERT(m_VO);
        m_Anim = idx;
        FirstFrame();
        SetLoopStatus();
    }
    void SetAnimByIndex(int idx, int loop) {
        ASSERT(m_VO);
        m_Anim = idx;
        FirstFrame();
        m_AnimLooped = loop;
    }

    void SetAnimById(DWORD id) {
        ASSERT(m_VO);
        m_Anim = VO()->GetAnimById(id);
        if (m_Anim < 0)
            m_Anim = 0;
        FirstFrame();
        SetLoopStatus();
    }
    void SetAnimById(DWORD id, int loop) {
        ASSERT(m_VO);
        m_Anim = VO()->GetAnimById(id);
        if (m_Anim < 0)
            m_Anim = 0;
        FirstFrame();
        m_AnimLooped = loop;
    }

    bool SetAnimByName(const wchar *name) {
        ASSERT(m_VO);
        int i = VO()->GetAnimByName(name);
        if (i < 0) {
            return true;
        }
        m_Anim = i;
        FirstFrame();
        SetLoopStatus();
        return false;
    }
    bool SetAnimByName(const wchar *name, int loop) {
        ASSERT(m_VO);
        int i = VO()->GetAnimByName(name);
        if (i < 0) {
            return true;
        }
        m_Anim = i;
        FirstFrame();
        m_AnimLooped = loop;
        return false;
    }

    bool SetAnimByNameNoBegin(const wchar *name) {
        ASSERT(m_VO);
        int i = VO()->GetAnimByName(name);
        if (i < 0) {
            return true;
        }
        SetLoopStatus();
        if (m_Anim != i) {
            m_Anim = i;
            FirstFrame();
        }
        return false;
    }
    bool SetAnimByNameNoBegin(const wchar *name, int loop) {
        ASSERT(m_VO);
        int i = VO()->GetAnimByName(name);
        if (i < 0) {
            return true;
        }
        m_AnimLooped = loop;
        if (m_Anim != i) {
            m_Anim = i;
            FirstFrame();
        }
        return false;
    }

    void SetAnimLooped(int loop) { m_AnimLooped = loop; }

    int GetVOFrame(void) const { return m_VOFrame; }
    int GetFrame(void) const { return m_Frame; }

    // int FrameCnt(void)							{ return m_AnimCnt; }
    // int Frame(void)								{ return m_Frame; }
    // void Frame(int zn)							{ m_Frame=zn; if(m_Frame<0) m_Frame=0; else if(m_Frame>=m_AnimCnt) m_Frame=0;
    // }

    const D3DXMATRIX *GetMatrixById(DWORD id) const {
        ASSERT(m_VO);
        return m_VO->GetMatrixById(m_VOFrame, id);
    }
    const D3DXMATRIX *GetMatrixByName(const wchar *name) const {
        ASSERT(m_VO);
        return m_VO->GetMatrixByName(m_VOFrame, name);
    }
    const D3DXMATRIX *GetMatrix(int no) const {
        ASSERT(m_VO);
        return m_VO->GetMatrix(m_VOFrame, no);
    }

    void CalcShadowProjMatrix(int noframe, SProjData &pd, D3DXVECTOR3 &dir, float addsize) {
        m_VO->CalcShadowProjMatrix(noframe, pd, dir, addsize);
    }

    void EnumFrameVerts(ENUM_VERTS_HANDLER evh, DWORD data) const { m_VO->EnumFrameVerts(m_VOFrame, evh, data); };

    bool Takt(int cms);  // True-if change
    DWORD NextFrame(void);
    void FirstFrame(void) {
        m_Frame = 0;
        m_VOFrame = VO()->GetAnimFrameIndex(m_Anim, 0);
        m_TimeNext = m_Time + VO()->GetAnimFrameTime(m_Anim, 0);
    }
    bool IsAnimEnd(void) const { return (m_AnimLooped == 0) && m_Frame == (m_VO->GetAnimFramesCount(m_Anim) - 1); }

    void GetBound(D3DXVECTOR3 &bmin, D3DXVECTOR3 &bmax) const;

    bool Pick(const D3DXMATRIX &ma, const D3DXMATRIX &ima, const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir,
              float *outt) const;
    bool PickFull(const D3DXMATRIX &ma, const D3DXMATRIX &ima, const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir,
                  float *outt) const;

    void BeforeDraw(void);
    void Draw(DWORD user_param) { VO()->Draw(m_VOFrame, user_param, m_Skin); }
    void DrawLights(bool now, const D3DXMATRIX &objma, const D3DXMATRIX *iview);
};

class CVectorObjectGroup;

class CVectorObjectGroupUnit : public CMain {
public:
    DWORD m_Flags;  // Какой ресурс объекта изменился. При созданнии класса устанавливается в 0xffffffff

    CVectorObjectGroup *m_Parent;
    CVectorObjectGroupUnit *m_Prev;
    CVectorObjectGroupUnit *m_Next;

    int m_Id;

    std::wstring m_Name;

    CVectorObjectAnim *m_Obj;

    CVectorObjectGroupUnit *m_Link;
    int m_LinkMatrixId;  // -1-center -2-by name >=0-by id
    std::wstring m_LinkMatrixName;

    D3DXMATRIX m_Matrix;  // Дополнительная матрица позиционирования в локальных координатах.

    D3DXMATRIX m_MatrixWorld;  // Конечная матрица трансформации объекта в мировые координаты.
    D3DXMATRIX m_IMatrixWorld;

    CVOShadowStencil *m_ShadowStencil;

public:
    CVectorObjectGroupUnit(void);
    ~CVectorObjectGroupUnit();

    void RNeed(dword need);  // Запрашиваем нужные ресурсы объекта
    void RChange(dword zn) { m_Flags |= VOUF_RES & zn; }  // Указываем какие ресурсы изменились

    void ShadowStencilOn(bool zn = true);
};

class CVectorObjectGroup : public CMain {
public:
    CVectorObjectGroupUnit *m_First;
    CVectorObjectGroupUnit *m_Last;

    D3DXMATRIX *m_GroupToWorldMatrix;  // Матрица позиционирования группы в мировом пространстве.

    std::wstring m_Name;  // Имя файла

    D3DXVECTOR3 m_ShadowStencilLight;
    // D3DXPLANE m_ShadowStencilCutPlane;
    float m_GroundZ;  // for stencil shadow len
public:
    CVectorObjectGroup(void);
    ~CVectorObjectGroup();

    void Clear(void);

    void Delete(CVectorObjectGroupUnit *un);
    CVectorObjectGroupUnit *Add(void);
    CVectorObjectGroupUnit *GetByName(const wchar *name);
    CVectorObjectGroupUnit *GetByNameNE(const wchar *name);
    CVectorObjectGroupUnit *GetById(int id);
    CVectorObjectGroupUnit *GetByIdNE(int id);

    D3DXVECTOR3 GetPosByName(const wchar *name) const;

    // void ChangeSetupFunction(OBJECT_SETUP_TEX setup_tex, OBJECT_SETUP_STAGES setup_stages);
    void RNeed(dword need);  // Запрашиваем нужные ресурсы объекта
    void RChange(dword zn);  // Указываем какие ресурсы изменились
    void RChangeByLink(CVectorObjectGroupUnit *link, dword zn);

    bool IsAlreadyLoaded(void) { return m_First != NULL; }

    bool Takt(int cms);

    CVectorObjectGroupUnit *Pick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, float *outt) const;

    void EnumFrameVerts_transform_position(ENUM_VERTS_HANDLER evh, DWORD data) const;

    void BoundGet(D3DXVECTOR3 &bmin, D3DXVECTOR3 &bmax);
    void BoundGetAllFrame(D3DXVECTOR3 &bmin, D3DXVECTOR3 &bmax);

    void ShadowStencilOn(bool zn = true);

    void ShadowStencil_DX_Free(void);

    void Load(const wchar *filename, CTextureManaged *lt, SKIN_GET sg, DWORD gsp);

    void BeforeDraw(bool proceed_shadows);
    void Draw(DWORD user_param);
    void DrawLights(bool now = false, const D3DXMATRIX *iview = NULL);
    void ShadowStencilDraw(void);
};
