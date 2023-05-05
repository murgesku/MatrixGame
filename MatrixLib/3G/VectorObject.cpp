// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>
#include <algorithm>

#include "VectorObject.hpp"
#include "ShadowStencil.hpp"
#include "CStorage.hpp"

#include "../../MatrixGame/src/MatrixSampleStateManager.hpp" // TODO: wtf?

// Сделать: Оптимизировать если нет прозрачной текстуры (Так как не нужно устанавливать текстуру)
// Сделать чтобы индексы не всегда устанавливались

//#define SHOW_PICK_OBJECTS

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
extern Base::CHeap *g_MatrixHeap;

CBigVB<SVOVertex> *CVectorObject::m_VB;
CBigIB *CVectorObject::m_IB;

static bool FreeObjectResources(uintptr_t user) {
    DTRACE();
    auto o = reinterpret_cast<CVectorObject*>(user);
    o->DX_Free();
    return false;
}

#pragma warning(disable : 4355)
CVectorObject::CVectorObject(void)
  : CCacheData(), m_RemindCore(FreeObjectResources, reinterpret_cast<uintptr_t>(this)) {
    DTRACE();
    m_Type = cc_VO;

    memset(&m_Geometry, 0, sizeof(SVOGeometry));

    if (m_VB == NULL) {
        m_VB = CBigVB<SVOVertex>::NewBigVB(g_CacheHeap);
        m_IB = CBigIB::NewBigIB(g_CacheHeap);
    }

    m_GeometrySimple = NULL;
}
#pragma warning(default : 4355)

CVectorObject::~CVectorObject() {
    DTRACE();

    if (m_Geometry.m_Vertices.verts != NULL) {
        Unload();
    }
}

void CVectorObject::Unload() {
    DTRACE();

    if (m_GeometrySimple) {
        if (m_GeometrySimple->m_Tris) {
            HFree(m_GeometrySimple->m_Tris, g_CacheHeap);
        }
        if (m_GeometrySimple->m_Verts) {
            HFree(m_GeometrySimple->m_Verts, g_CacheHeap);
        }
        HFree(m_GeometrySimple, g_CacheHeap);
        m_GeometrySimple = NULL;
    }

    if (m_Geometry.m_Vertices.verts != NULL) {
        if (m_Geometry.m_VerticesIdx != NULL) {
            HFree(m_Geometry.m_VerticesIdx, g_CacheHeap);
        }
        if (m_Geometry.m_FramesIdx != NULL) {
            HFree(m_Geometry.m_FramesIdx, g_CacheHeap);
        }
        if (m_Geometry.m_UnionsIdx != NULL) {
            HFree(m_Geometry.m_UnionsIdx, g_CacheHeap);
        }
        // if(m_Geometry.m_EdgesIdx != NULL) { HFree(m_Geometry.m_EdgesIdx, g_CacheHeap); }

        if (m_Geometry.m_Vertices.verts != NULL) {
            if ((DWORD)m_Geometry.m_Vertices.verts != 0xFFFFFFFF) {
                if (m_VB->DelSource(&m_Geometry.m_Vertices)) {
                    CBigVB<SVOVertex>::DestroyBigVB(m_VB);
                    m_VB = NULL;
                }
                HFree(m_Geometry.m_Vertices.verts, g_CacheHeap);
            }
            m_Geometry.m_Vertices.verts = NULL;
        }
        if (m_Geometry.m_Triangles != NULL) {
            HFree(m_Geometry.m_Triangles, g_CacheHeap);
        }
        if (m_Geometry.m_Idxs.inds != NULL) {
            if (m_IB->DelSource(&m_Geometry.m_Idxs)) {
                CBigIB::DestroyBigIB(m_IB);
                m_IB = NULL;
            }
            HFree(m_Geometry.m_Idxs.inds, g_CacheHeap);
            m_Geometry.m_Idxs.inds = NULL;
        }
        if (m_Geometry.m_Unions != NULL) {
            HFree(m_Geometry.m_Unions, g_CacheHeap);
        }
        if (m_Geometry.m_Edges != NULL) {
            HFree(m_Geometry.m_Edges, g_CacheHeap);
        }
        if (m_Geometry.m_Frames != NULL) {
            HFree(m_Geometry.m_Frames, g_CacheHeap);
        }
        if (m_Geometry.m_FramesRuntime != NULL) {
            HFree(m_Geometry.m_FramesRuntime, g_CacheHeap);
        }

        for (int i = 0; i < m_Geometry.m_SurfacesCnt; ++i) {
            m_Geometry.m_Surfaces[i].texname.std::wstring::~wstring();
        }
        if (m_Geometry.m_Surfaces != NULL) {
            HFree(m_Geometry.m_Surfaces, g_CacheHeap);
        }
        if (m_Geometry.m_Animations != NULL) {
            HFree(m_Geometry.m_Animations, g_CacheHeap);
        }
        if (m_Geometry.m_Matrixs != NULL) {
            HFree(m_Geometry.m_Matrixs, g_CacheHeap);
        }
        // if(m_Geometry.m_Normals != NULL) { HFree(m_Geometry.m_Normals, g_CacheHeap); }
        if (m_Geometry.m_AllMatrixs != NULL) {
            HFree(m_Geometry.m_AllMatrixs, g_CacheHeap);
        }

        memset(&m_Geometry, 0, sizeof(SVOGeometry));
    }
}

void CVectorObject::LoadSpecial(EObjectLoad olf, SKIN_GET sg, DWORD gsp) {
    DTRACE();
    Unload();

    CStorage stor(g_CacheHeap);
#ifdef _DEBUG
    bool ok =
#endif
            stor.Load(m_Name.c_str());
#ifdef _DEBUG
    if (!ok) {
        ERROR_S((L"Unable to load object: " + m_Name).c_str());
    }
#endif

    // CDataBuf *propkey = stor.GetBuf(L"props", L"key", ST_WCHAR);
    // CDataBuf *propval = stor.GetBuf(L"props", L"data", ST_WCHAR);
    {
        CDataBuf *propname = stor.GetBuf(L"properties", L"name", ST_WCHAR);
        CDataBuf *propval = stor.GetBuf(L"properties", L"value", ST_WCHAR);

        if (propname) {
            for (DWORD i = 0; i < propname->GetArraysCount(); ++i) {
                m_Props.ParAdd(propname->GetAsWStr(i), propval->GetAsWStr(i));
            }
        }
    }

    {
        // load simple
        CDataBuf *c = stor.GetBuf(L"simple", L"data", ST_BYTE);
        if (c) {
            BYTE *da = c->GetFirst<BYTE>(0);

            m_GeometrySimple = (SVOGeometrySimple *)HAlloc(sizeof(SVOGeometrySimple), g_CacheHeap);

            memcpy(&m_GeometrySimple->m_AsIs, da, sizeof(SVOGeometrySimple::SVOGS_asis));
            da += sizeof(SVOGeometrySimple::SVOGS_asis);

            int szt = sizeof(SVOTriangle) * m_GeometrySimple->m_AsIs.m_TriCnt;
            m_GeometrySimple->m_Tris = (SVOTriangle *)HAlloc(szt, g_CacheHeap);

            int szv = sizeof(D3DXVECTOR3) * m_GeometrySimple->m_AsIs.m_VertsCnt;
            m_GeometrySimple->m_Verts = (D3DXVECTOR3 *)HAlloc(szv, g_CacheHeap);

            memcpy(m_GeometrySimple->m_Tris, da, szt);
            da += szt;
            memcpy(m_GeometrySimple->m_Verts, da, szv);
            da += szv;
        }
    }

    // matrixs
    {
        CDataBuf *c0 = stor.GetBuf(L"matrices", L"name", ST_WCHAR);
        if (c0) {
            m_Geometry.m_MatrixsCnt = c0->GetArraysCount();

            m_Geometry.m_Matrixs = (SVOMatrix *)HAlloc(sizeof(SVOMatrix) * m_Geometry.m_MatrixsCnt, g_CacheHeap);

            CDataBuf *c1 = stor.GetBuf(L"matrices", L"id", ST_DWORD);
            CDataBuf *c2 = stor.GetBuf(L"matrices", L"disp", ST_DWORD);
            CDataBuf *c3 = stor.GetBuf(L"matrices", L"data", ST_BYTE);

            DWORD *id = c1->GetFirst<DWORD>(0);
            DWORD *disp = c2->GetFirst<DWORD>(0);

            std::wstring mn;
            for (int i = 0; i < m_Geometry.m_MatrixsCnt; ++i) {
                mn = c0->GetAsWStr(i);
                memcpy(m_Geometry.m_Matrixs[i].m_Name, mn.c_str(), (mn.length() + 1) * sizeof(wchar));

                m_Geometry.m_Matrixs[i].m_Id = id[i];
                m_Geometry.m_Matrixs[i].m_MatrixStart = disp[i];
            }
            m_Geometry.m_AllMatrixs = (D3DXMATRIX *)HAlloc(c3->GetArrayLength(0), g_CacheHeap);
            memcpy(m_Geometry.m_AllMatrixs, c3->GetFirst<BYTE>(0), c3->GetArrayLength(0));
        }
    }
    {
        // verts
        CDataBuf *c = stor.GetBuf(L"verts", L"data", ST_BYTE);
        if (c) {
            m_Geometry.m_Vertices.size = c->GetArrayLength(0);
            m_Geometry.m_Vertices.verts = (SVOVertex *)HAlloc(m_Geometry.m_Vertices.size, g_CacheHeap);
            memcpy(m_Geometry.m_Vertices.verts, c->GetFirst<BYTE>(0), m_Geometry.m_Vertices.size);
            m_VB->AddSource(&m_Geometry.m_Vertices);
        }
    }

    // animations and idxs for frames
    {
        CDataBuf *c0 = stor.GetBuf(L"anims", L"name", ST_WCHAR);
        if (c0) {
            m_Geometry.m_AnimationsCnt = c0->GetArraysCount();

            m_Geometry.m_Animations =
                    (SVOAnimation *)HAlloc(sizeof(SVOAnimation) * m_Geometry.m_AnimationsCnt, g_CacheHeap);

            CDataBuf *c1 = stor.GetBuf(L"anims", L"id", ST_DWORD);
            CDataBuf *c2 = stor.GetBuf(L"anims", L"disp", ST_DWORD);
            CDataBuf *c3 = stor.GetBuf(L"anims", L"cnt", ST_DWORD);
            CDataBuf *c4 = stor.GetBuf(L"anims", L"frames", ST_BYTE);

            DWORD *id = c1->GetFirst<DWORD>(0);
            DWORD *disp = c2->GetFirst<DWORD>(0);
            DWORD *cnt = c3->GetFirst<DWORD>(0);

            std::wstring mn;
            for (int i = 0; i < m_Geometry.m_AnimationsCnt; ++i) {
                mn = c0->GetAsWStr(i);
                memcpy(m_Geometry.m_Animations[i].m_Name, mn.c_str(), (mn.length() + 1) * sizeof(wchar));

                m_Geometry.m_Animations[i].m_Id = id[i];
                m_Geometry.m_Animations[i].m_FramesStart = disp[i];
                m_Geometry.m_Animations[i].m_FramesCnt = cnt[i];
            }
            m_Geometry.m_FramesIdx = (SVOFrameIndex *)HAlloc(c4->GetArrayLength(0), g_CacheHeap);
            memcpy(m_Geometry.m_FramesIdx, c4->GetFirst<BYTE>(0), c4->GetArrayLength(0));
        }
    }
    {
        // frames

        CDataBuf *c0 = stor.GetBuf(L"frames", L"data", ST_BYTE);
        CDataBuf *c1 = stor.GetBuf(L"frames", L"verts", ST_BYTE);
        CDataBuf *c2 = stor.GetBuf(L"frames", L"edges", ST_BYTE);
        CDataBuf *c3 = stor.GetBuf(L"frames", L"unions", ST_BYTE);

        int sz;

        // frames data
        sz = c0->GetArrayLength(0);
        m_Geometry.m_FramesCnt = sz / sizeof(SVOKadr);
        m_Geometry.m_Frames = (SVOKadr *)HAlloc(sz, g_CacheHeap);
        memcpy(m_Geometry.m_Frames, c0->GetFirst<BYTE>(0), sz);

        // verts idxs for frames
        sz = c1->GetArrayLength(0);
        // m_Geometry.m_VerticesIdx = sz / sizeof(int);
        m_Geometry.m_VerticesIdx = (int *)HAlloc(sz, g_CacheHeap);
        memcpy(m_Geometry.m_VerticesIdx, c1->GetFirst<BYTE>(0), sz);

        // edges
        sz = c2->GetArrayLength(0);
        if (sz > 0) {
            m_Geometry.m_EdgesCnt = sz / sizeof(SVOKadrEdge);
            m_Geometry.m_Edges = (SVOKadrEdge *)HAlloc(sz, g_CacheHeap);
            memcpy(m_Geometry.m_Edges, c2->GetFirst<BYTE>(0), sz);

            // int ic;
            // if (propkey && (ic=propkey->FindAsWStr(L"MaxEdgeToVertexIndex")) > 0)
            //{
            //    m_Geometry.m_MaxEdgeToVertexIndex = propval->GetAsWStr(ic).GetInt();
            //} else
            {
                m_Geometry.m_FramesRuntime =
                        (SVOFrameRuntime *)HAlloc(sizeof(SVOFrameRuntime) * m_Geometry.m_FramesCnt, g_CacheHeap);
                for (int i = 0; i < m_Geometry.m_FramesCnt; ++i) {
                    SVOKadr *k = m_Geometry.m_Frames + i;
                    SVOFrameRuntime *fr = m_Geometry.m_FramesRuntime + i;
                    fr->m_EdgeVertexIndexMin = 10000000;
                    fr->m_EdgeVertexIndexCount = 0;

                    for (int e = 0; e < k->m_EdgeCnt; ++e) {
                        SVOKadrEdge *ee = m_Geometry.m_Edges + e + k->m_EdgeStart;
                        if (ee->v00 > fr->m_EdgeVertexIndexCount)
                            fr->m_EdgeVertexIndexCount = ee->v00;
                        if (ee->v01 > fr->m_EdgeVertexIndexCount)
                            fr->m_EdgeVertexIndexCount = ee->v01;

                        if (ee->v00 < fr->m_EdgeVertexIndexMin)
                            fr->m_EdgeVertexIndexMin = ee->v00;
                        if (ee->v01 < fr->m_EdgeVertexIndexMin)
                            fr->m_EdgeVertexIndexMin = ee->v01;
                    }

                    fr->m_EdgeVertexIndexCount =
                            1 + (fr->m_EdgeVertexIndexCount - fr->m_EdgeVertexIndexMin) / sizeof(SVOVertex);
                    fr->m_EdgeVertexIndexMin /= sizeof(SVOVertex);
                }
            }
        }

        // union idxs for frames
        sz = c3->GetArrayLength(0);
        // m_Geometry.m_UnionsCnt = sz / sizeof(int);
        m_Geometry.m_UnionsIdx = (DWORD *)HAlloc(sz, g_CacheHeap);
        memcpy(m_Geometry.m_UnionsIdx, c3->GetFirst<BYTE>(0), sz);
    }

    {
        // unions
        CDataBuf *c = stor.GetBuf(L"unions", L"data", ST_BYTE);
        if (c) {
            int sz = c->GetArrayLength(0);

            m_Geometry.m_UnionsCnt = sz / sizeof(SVOUnion);
            m_Geometry.m_Unions = (SVOUnion *)HAlloc(sz, g_CacheHeap);
            memcpy(m_Geometry.m_Unions, c->GetFirst<BYTE>(0), sz);
        }
    }
    {
        // idxs
        CDataBuf *c = stor.GetBuf(L"idxs", L"data", ST_BYTE);
        if (c) {
            int sz = c->GetArrayLength(0);

            // m_Geometry.m_IdxCnt = sz / sizeof(WORD);
            m_Geometry.m_TrianglesCnt = sz / (3 * sizeof(WORD));
            m_Geometry.m_Triangles =
                    (SVOTriangle *)HAlloc(sizeof(SVOTriangle) * m_Geometry.m_TrianglesCnt, g_CacheHeap);

            m_Geometry.m_Idxs.size = sz;
            m_Geometry.m_Idxs.inds = (WORD *)HAlloc(sz, g_CacheHeap);
            memcpy(m_Geometry.m_Idxs.inds, c->GetFirst<BYTE>(0), sz);

            m_IB->AddSource(&m_Geometry.m_Idxs);

            // TODO : restore triangles

            WORD *idx = m_Geometry.m_Idxs.inds;
            for (int i = 0; i < m_Geometry.m_TrianglesCnt; ++i, idx += 3) {
                m_Geometry.m_Triangles[i].i0 = *(idx + 0);
                m_Geometry.m_Triangles[i].i1 = *(idx + 1);
                m_Geometry.m_Triangles[i].i2 = *(idx + 2);
            }
        }
    }
    {
        // surfs
        CDataBuf *c = stor.GetBuf(L"surfs", L"texs", ST_WCHAR);
        if (c) {
            m_Geometry.m_SurfacesCnt = c->GetArraysCount();
            m_Geometry.m_Surfaces = (SVOSurface *)HAlloc(sizeof(SVOSurface) * m_Geometry.m_SurfacesCnt, g_CacheHeap);
            for (int i = 0; i < m_Geometry.m_SurfacesCnt; ++i) {
                new(&m_Geometry.m_Surfaces[i].texname) std::wstring(c->GetAsWStr(i));
                m_Geometry.m_Surfaces[i].skin = NULL;
            }
        }
    }

    //  if (Header(data)->m_Flags&2)
    //  {
    //      for(int i=0;i<m_Geometry.m_SurfacesCnt;++i)
    //      {
    //    SVOSurface * s = m_Geometry.m_Surfaces + i;

    //          std::wstring outstr(g_CacheHeap);
    //    CacheReplaceFileExt(outstr,m_Name);

    //          // load textures
    //          if (i!=0) { outstr += L"_"; outstr += i; }
    //
    //          s->texname = outstr;
    //   }
    //  } else
    //  {
    //      for(int i=0;i<m_Geometry.m_SurfacesCnt;++i)
    //      {
    //    SVOSurface * s = m_Geometry.m_Surfaces + i;

    //          std::wstring outstr((materials + i)->tex_diffuse,g_CacheHeap);
    //          CacheReplaceFileExt(outstr, outstr.c_str());

    //          ASSERT(outstr.length() > 0);
    //          //if (outstr.length() == 0) _asm int 3

    //          std::wstring temp(g_CacheHeap);
    //          if(!CFile::FileExist(temp,outstr.c_str(),CacheExtsTex))
    //          {
    //              CacheReplaceFileNameAndExt(outstr, m_Name, (materials + i)->tex_diffuse);
    //              CacheReplaceFileExt(outstr, outstr.c_str());
    //          }

    //          s->texname = outstr;
    //      }
    //  }

    if (olf == OLF_NO_TEX)
        return;
    if (olf == OLF_MULTIMATERIAL_ONLY && m_Geometry.m_SurfacesCnt <= 1)
        return;

    for (int i = 0; i < m_Geometry.m_SurfacesCnt; ++i) {
        SVOSurface *s = m_Geometry.m_Surfaces + i;
        s->skin = sg(s->texname.c_str(), gsp);
    }
}

int CVectorObject::GetAnimById(DWORD id) {
    DTRACE();
    for (int i = 0; i < m_Geometry.m_AnimationsCnt; i++) {
        if ((m_Geometry.m_Animations + i)->m_Id == id)
            return i;
    }
    return -1;
}

int CVectorObject::GetAnimByName(const std::wstring& name) {
    DTRACE();
    for (int i = 0; i < m_Geometry.m_AnimationsCnt; i++) {
        if (name == (m_Geometry.m_Animations + i)->m_Name)
            return i;
    }
    return -1;
}

const D3DXMATRIX *CVectorObject::GetMatrixById(int frame, DWORD id) const {
    DTRACE();
    for (int i = 0; i < m_Geometry.m_MatrixsCnt; i++) {
        if ((m_Geometry.m_Matrixs + i)->m_Id == id) {
            return m_Geometry.m_AllMatrixs + (m_Geometry.m_Matrixs + i)->m_MatrixStart + frame;
        }
    }
    return NULL;
}

const wchar *CVectorObject::GetMatrixNameById(DWORD id) const {
    DTRACE();
    for (int i = 0; i < m_Geometry.m_MatrixsCnt; i++) {
        if ((m_Geometry.m_Matrixs + i)->m_Id == id) {
            return (m_Geometry.m_Matrixs + i)->m_Name;
        }
    }
    return NULL;
}

const D3DXMATRIX *CVectorObject::GetMatrixByName(int frame, const std::wstring& name) const {
    DTRACE();
    for (int i = 0; i < m_Geometry.m_MatrixsCnt; i++) {
        if (name == (m_Geometry.m_Matrixs + i)->m_Name) {
            return m_Geometry.m_AllMatrixs + (m_Geometry.m_Matrixs + i)->m_MatrixStart + frame;
        }
    }
    return NULL;
}

void CVectorObject::EnumFrameVerts(int noframe, ENUM_VERTS_HANDLER evh, DWORD data) const {
    SVOKadr *fr = m_Geometry.m_Frames + noframe;
    int *verts = m_Geometry.m_VerticesIdx + fr->m_VerStart;
    int *vertse = verts + fr->m_VerCnt;

    SVOVertex *cv;
    while (verts < vertse) {
        int vi = *verts++;
        cv = m_Geometry.m_Vertices.verts + vi;
        if (evh(*cv, data))
            break;
    }
}

bool CVectorObject::PickSimple(const D3DXMATRIX &ma, const D3DXMATRIX &ima, const D3DXVECTOR3 &origin,
                               const D3DXVECTOR3 &dir, float *outt) const {
    float kt, ku, kv, okt = 1e20f;
    bool rv = false;

    D3DXVECTOR3 _orig, _dir;

    D3DXVec3TransformCoord(&_orig, &m_GeometrySimple->m_AsIs.m_Center, &ma);

    if (!IsIntersectSphere(_orig, m_GeometrySimple->m_AsIs.m_Radius * ma._33, origin, dir, kt))
        return false;

    D3DXVec3TransformCoord(&_orig, &origin, &ima);
    D3DXVec3TransformNormal(&_dir, &dir, &ima);

    // if (!IsIntersectSphere(m_GeometrySimple->m_AsIs.m_Center, m_GeometrySimple->m_AsIs.m_Radius * ma._33, _orig,
    // _dir, kt)) return false;

    if (!D3DXBoxBoundProbe(&m_GeometrySimple->m_AsIs.m_Mins, &m_GeometrySimple->m_AsIs.m_Maxs, &_orig, &_dir))
        return false;

    BYTE *vflags = (BYTE *)_alloca(m_GeometrySimple->m_AsIs.m_VertsCnt);

    // TAKT_BEGIN();

    D3DXVECTOR3 *cv;

    D3DXVECTOR3 adir;

    *(((DWORD *)&adir) + 0) = *(((DWORD *)&_dir) + 0) & 0x7FFFFFFF;  // adir.x = abs(pd._dir.x);
    *(((DWORD *)&adir) + 1) = *(((DWORD *)&_dir) + 1) & 0x7FFFFFFF;  // adir.y = abs(pd._dir.y);
    *(((DWORD *)&adir) + 2) = *(((DWORD *)&_dir) + 2) & 0x7FFFFFFF;  // adir.z = abs(pd._dir.z);

    if (adir.x >= adir.y) {
        if (adir.x >= adir.z) {
            // YZ

            float kk = 1.0f / _dir.x;
            float k[2] = {-_dir.y * kk, -_dir.z * kk};
            float coordc[2] = {_orig.y + _orig.x * k[0], _orig.z + _orig.x * k[1]};
            for (int i = 0; i < m_GeometrySimple->m_AsIs.m_VertsCnt; ++i) {
                cv = m_GeometrySimple->m_Verts + i;

                float coord[2] = {cv->y + cv->x * k[0] - coordc[0], cv->z + cv->x * k[1] - coordc[1]};
                DWORD t0 = *((DWORD *)&coord[0]);
                DWORD t1 = *((DWORD *)&coord[1]);
                BYTE r0 = (BYTE((t0 >> 31) + 1) & BYTE(((t0 & 0x7FFFFFF) == 0) ? 0 : -1));
                BYTE r1 = (BYTE(((t1 >> 31) + 1) << 2) & BYTE(((t1 & 0x7FFFFFF) == 0) ? 0 : -1));

                *(vflags + i) = r0 | r1;
            }
        }
        else {
            // XY
            goto calcxy;
        }
    }
    else {
        if (adir.y >= adir.z) {
            // XZ
            float kk = 1.0f / _dir.y;
            float k[2] = {k[0] = -_dir.x * kk, k[1] = -_dir.z * kk};

            float coordc[2] = {_orig.x + _orig.y * k[0], _orig.z + _orig.y * k[1]};
            for (int i = 0; i < m_GeometrySimple->m_AsIs.m_VertsCnt; ++i) {
                cv = m_GeometrySimple->m_Verts + i;

                float coord[2] = {cv->x + cv->y * k[0] - coordc[0], cv->z + cv->y * k[1] - coordc[1]};
                DWORD t0 = *((DWORD *)&coord[0]);
                DWORD t1 = *((DWORD *)&coord[1]);
                BYTE r0 = (BYTE((t0 >> 31) + 1) & BYTE(((t0 & 0x7FFFFFF) == 0) ? 0 : -1));
                BYTE r1 = (BYTE(((t1 >> 31) + 1) << 2) & BYTE(((t1 & 0x7FFFFFF) == 0) ? 0 : -1));

                *(vflags + i) = r0 | r1;
            }
        }
        else {
        calcxy:
            // XY

            float kk = 1.0f / _dir.z;
            float k[2] = {k[0] = -_dir.x * kk, -_dir.y * kk};

            float coordc[2] = {_orig.x + _orig.z * k[0], _orig.y + _orig.z * k[1]};

            for (int i = 0; i < m_GeometrySimple->m_AsIs.m_VertsCnt; ++i) {
                cv = m_GeometrySimple->m_Verts + i;

                float coord[2] = {cv->x + cv->z * k[0] - coordc[0], cv->y + cv->z * k[1] - coordc[1]};
                DWORD t0 = *((DWORD *)&coord[0]);
                DWORD t1 = *((DWORD *)&coord[1]);
                BYTE r0 = (BYTE((t0 >> 31) + 1) & BYTE(((t0 & 0x7FFFFFF) == 0) ? 0 : -1));
                BYTE r1 = (BYTE(((t1 >> 31) + 1) << 2) & BYTE(((t1 & 0x7FFFFFF) == 0) ? 0 : -1));

                *(vflags + i) = r0 | r1;
            }
        }
    }

    SVOTriangle *trb = m_GeometrySimple->m_Tris;
    SVOTriangle *tre = trb + m_GeometrySimple->m_AsIs.m_TriCnt;

    D3DXVECTOR3 *vs = m_GeometrySimple->m_Verts;

    for (; trb < tre; ++trb) {
        if ((*(vflags + trb->i0) & *(vflags + trb->i1) & *(vflags + trb->i2)) == 0) {
            if (IntersectTriangle(_orig, _dir, *(vs + trb->i0), *(vs + trb->i1), *(vs + trb->i2), kt, ku, kv) &&
                kt < okt && kt >= 0.0f) {
                okt = kt;
                rv = true;
            }
        }
    }
    if (rv && outt) {
        *outt = okt;
    }
    return rv;
}

bool CVectorObject::Pick(int noframe, const D3DXMATRIX &ma, const D3DXMATRIX &ima, const D3DXVECTOR3 &origin,
                         const D3DXVECTOR3 &dir, float *outt) const {
    if (m_GeometrySimple)
        return PickSimple(ma, ima, origin, dir, outt);
    return PickFull(noframe, ma, ima, origin, dir, outt);
}

bool CVectorObject::PickFull(int noframe, const D3DXMATRIX &ma, const D3DXMATRIX &ima, const D3DXVECTOR3 &origin,
                             const D3DXVECTOR3 &dir, float *outt) const {
    DTRACE();

    SVOKadr *fr;

    float kt, ku, kv, okt = 1e20f;
    bool rv = false;

    D3DXVECTOR3 _orig, _dir;

    fr = m_Geometry.m_Frames + noframe;

    D3DXVec3TransformCoord(&_orig, &fr->m_GeoCenter, &ma);

    if (!IsIntersectSphere(_orig, fr->m_Radius * ma._33, origin, dir, kt))
        return false;

    D3DXVec3TransformCoord(&_orig, &origin, &ima);
    D3DXVec3TransformNormal(&_dir, &dir, &ima);

    if (!D3DXBoxBoundProbe(&fr->m_Min, &fr->m_Max, &_orig, &_dir))
        return false;

    int i;

    BYTE *vflags = (BYTE *)_alloca(m_Geometry.m_Vertices.size / sizeof(SVOVertex));
    BYTE *cvflags;

    // TAKT_BEGIN();

    SVOVertex *cv;

    D3DXVECTOR3 adir;

    *(((DWORD *)&adir) + 0) = *(((DWORD *)&_dir) + 0) & 0x7FFFFFFF;  // adir.x = abs(pd._dir.x);
    *(((DWORD *)&adir) + 1) = *(((DWORD *)&_dir) + 1) & 0x7FFFFFFF;  // adir.y = abs(pd._dir.y);
    *(((DWORD *)&adir) + 2) = *(((DWORD *)&_dir) + 2) & 0x7FFFFFFF;  // adir.z = abs(pd._dir.z);

    int *verts = m_Geometry.m_VerticesIdx + fr->m_VerStart;
    int *vertse = verts + fr->m_VerCnt;

    if (adir.x >= adir.y) {
        if (adir.x >= adir.z) {
            // YZ

            float kk = 1.0f / _dir.x;
            float k[2] = {-_dir.y * kk, -_dir.z * kk};
            float coordc[2] = {_orig.y + _orig.x * k[0], _orig.z + _orig.x * k[1]};
            while (verts < vertse) {
                int vi = *verts++;
                cv = m_Geometry.m_Vertices.verts + vi;

                float coord[2] = {cv->v.y + cv->v.x * k[0] - coordc[0], cv->v.z + cv->v.x * k[1] - coordc[1]};
                DWORD t0 = *((DWORD *)&coord[0]);
                DWORD t1 = *((DWORD *)&coord[1]);
                BYTE r0 = (BYTE((t0 >> 31) + 1) & BYTE(((t0 & 0x7FFFFFF) == 0) ? 0 : -1));
                BYTE r1 = (BYTE(((t1 >> 31) + 1) << 2) & BYTE(((t1 & 0x7FFFFFF) == 0) ? 0 : -1));

                *(vflags + vi) = r0 | r1;
            }
        }
        else {
            // XY
            goto calcxy;
        }
    }
    else {
        if (adir.y >= adir.z) {
            // XZ
            float kk = 1.0f / _dir.y;
            float k[2] = {k[0] = -_dir.x * kk, k[1] = -_dir.z * kk};

            float coordc[2] = {_orig.x + _orig.y * k[0], _orig.z + _orig.y * k[1]};
            while (verts < vertse) {
                int vi = *verts++;
                cv = m_Geometry.m_Vertices.verts + vi;

                float coord[2] = {cv->v.x + cv->v.y * k[0] - coordc[0], cv->v.z + cv->v.y * k[1] - coordc[1]};
                DWORD t0 = *((DWORD *)&coord[0]);
                DWORD t1 = *((DWORD *)&coord[1]);
                BYTE r0 = (BYTE((t0 >> 31) + 1) & BYTE(((t0 & 0x7FFFFFF) == 0) ? 0 : -1));
                BYTE r1 = (BYTE(((t1 >> 31) + 1) << 2) & BYTE(((t1 & 0x7FFFFFF) == 0) ? 0 : -1));

                *(vflags + vi) = r0 | r1;
            }
        }
        else {
        calcxy:
            // XY

            float kk = 1.0f / _dir.z;
            float k[2] = {k[0] = -_dir.x * kk, -_dir.y * kk};

            float coordc[2] = {_orig.x + _orig.z * k[0], _orig.y + _orig.z * k[1]};

            while (verts < vertse) {
                int vi = *verts++;
                cv = m_Geometry.m_Vertices.verts + vi;

                float coord[2] = {cv->v.x + cv->v.z * k[0] - coordc[0], cv->v.y + cv->v.z * k[1] - coordc[1]};
                DWORD t0 = *((DWORD *)&coord[0]);
                DWORD t1 = *((DWORD *)&coord[1]);
                BYTE r0 = (BYTE((t0 >> 31) + 1) & BYTE(((t0 & 0x7FFFFFF) == 0) ? 0 : -1));
                BYTE r1 = (BYTE(((t1 >> 31) + 1) << 2) & BYTE(((t1 & 0x7FFFFFF) == 0) ? 0 : -1));

                *(vflags + vi) = r0 | r1;
            }
        }
    }

    for (i = 0; i < fr->m_UnionCnt; ++i) {
        SVOUnion *gr = m_Geometry.m_Unions + m_Geometry.m_UnionsIdx[i + fr->m_UnionStart];

        SVOVertex *vs = m_Geometry.m_Vertices.verts + gr->m_Base;
        cvflags = vflags + gr->m_Base;

        SVOTriangle *tbegin = m_Geometry.m_Triangles + gr->m_TriStart;
        SVOTriangle *tend = tbegin + gr->m_TriCnt;

        while (tbegin < tend) {
            if ((*(cvflags + tbegin->i0) & *(cvflags + tbegin->i1) & *(cvflags + tbegin->i2)) == 0) {
                if (IntersectTriangle(_orig, _dir, (vs + tbegin->i0)->v, (vs + tbegin->i1)->v, (vs + tbegin->i2)->v, kt,
                                      ku, kv) &&
                    kt < okt && kt >= 0.0f) {
                    okt = kt;
                    rv = true;
                }
            }
            ++tbegin;
        }
    }
    if (rv && outt) {
        *outt = okt;
    }
    return rv;
}

/*
struct SSortIndexForAlpha {
    D3DXVECTOR3 eye;
    D3DXVECTOR3 pos;
    byte * buf;
    D3DXPLANE pl;
};

int __cdecl SortIndexForAlpha_Fun( const VOID* arg1, const VOID* arg2 )
{
    DTRACE();
    SSortIndexForAlpha * ms1 = (SSortIndexForAlpha *)arg1;
    SSortIndexForAlpha * ms2 = (SSortIndexForAlpha *)arg2;

    float zn1=D3DXPlaneDotCoord(&(ms1->pl),&(ms2->pos));
    float zn2=D3DXPlaneDotCoord(&(ms1->pl),&(ms1->eye));

    zn1=zn1*zn2;

    if(zn1<0) return 1;
    else if(zn1>0) return -1;
    else return 0;
}

void CVectorObject::SortIndexForAlpha(D3DXMATRIX & ma, void * bufout)
{
    DTRACE();
    CopyMemory(bufout,Data()+Header()->m_TriSme,Header()->m_TriCnt*((Header()->m_Flags&1)?2:4));

    SVOVertexNorTex * v;
    D3DXVECTOR3 cv;
    D3DXVECTOR3 * v1,* v2,* v3;

    D3DXMATRIX matt;
    D3DXMatrixInverse(&matt,NULL,&ma);
    D3DXVECTOR3 eyep=D3DXVECTOR3(0,0,0);
    D3DXVec3TransformCoord(&eyep,&eyep,&matt);

    SSortIndexForAlpha * bufsort=(SSortIndexForAlpha
*)HAllocClear((Header()->m_TriCnt/3)*sizeof(SSortIndexForAlpha),g_CacheHeap);

    int cnt=GroupCount();
    for(int i=0;i<cnt;i++) {
        SVOGroup * gr=GroupGet(i);
        if(!MaterialWithAlpha(gr->m_Material)) continue;

        SVOVertexNorTex * vs=((SVOVertexNorTex *)(Data()+Header()->m_VerSme))+gr->m_VerStart;
        byte * in=Data()+Header()->m_TriSme+gr->m_TriStart*((Header()->m_Flags&1)?2:4);
        SSortIndexForAlpha * bsort=bufsort+gr->m_TriStart/3;

        int cnt=gr->m_TriCnt/3;
//		g_TempVertexCnt=0;
        while(cnt>0) {
            bsort->eye=eyep;
            bsort->buf=in;

            v=vs+((Header()->m_Flags&1)?(*(word *)in):(*(dword *)in));
            v1=&(v->v);
            in+=(Header()->m_Flags&1)?2:4;

            v=vs+((Header()->m_Flags&1)?(*(word *)in):(*(dword *)in));
            v2=&(v->v);
            in+=(Header()->m_Flags&1)?2:4;

            v=vs+((Header()->m_Flags&1)?(*(word *)in):(*(dword *)in));
            v3=&(v->v);
            in+=(Header()->m_Flags&1)?2:4;

//			D3DXVec3TransformCoord(&v1,&v1,&ma);
//			D3DXVec3TransformCoord(&v2,&v2,&ma);
//			D3DXVec3TransformCoord(&v3,&v3,&ma);

//			cv.x=(v1->x+v2->x+v3->x)/3;
//			cv.y=(v1->y+v2->y+v3->y)/3;
//			cv.z=(v1->z+v2->z+v3->z)/3;

            D3DXPlaneFromPoints(&(bsort->pl),v1,v2,v3);

            bsort->pos.x=(v1->x+v2->x+v3->x)/3;
            bsort->pos.y=(v1->y+v2->y+v3->y)/3;
            bsort->pos.z=(v1->z+v2->z+v3->z)/3;

//			D3DXVec3TransformCoord(&cv,&cv,&matt);


            cnt--;
            bsort++;

        }

        qsort(bufsort+gr->m_TriStart/3,gr->m_TriCnt/3,sizeof(SSortIndexForAlpha), SortIndexForAlpha_Fun );

        if(Header()->m_Flags&1) {
            in=(byte *)bufout+gr->m_TriStart*2;
            bsort=bufsort+gr->m_TriStart/3;
            cnt=gr->m_TriCnt/3;
            while(cnt>0) {
                *(dword *)in=*(dword *)(bsort->buf);
                *(word *)(in+4)=*(word *)(bsort->buf+4);

                in+=3*2;

                cnt--;
                bsort++;
            }
        } else {
            in=(byte *)bufout+gr->m_TriStart*4;
            bsort=bufsort+gr->m_TriStart/3;
            cnt=gr->m_TriCnt/3;
            while(cnt>0) {
                *(dword *)in=*(dword *)(bsort->buf);
                *(dword *)(in+4)=*(dword *)(bsort->buf+4);
                *(dword *)(in+8)=*(dword *)(bsort->buf+8);

                in+=3*2;

                cnt--;
                bsort++;
            }
        }
    }

    HFree(bufsort,g_CacheHeap);
}

void CVectorObject::SortIndexForAlpha(D3DXMATRIX & ma)
{
    DTRACE();
    int size=Header()->m_TriCnt*((Header()->m_Flags&1)?2:4);
    byte * tb=(byte *)HAlloc(size,g_CacheHeap);

    SortIndexForAlpha(ma,tb);
    CopyMemory(Data()+Header()->m_TriSme,tb,size);

    HFree(tb,g_CacheHeap);
}
*/

// void CVectorObject::EdgeClear()
//{
//    DTRACE();
//	if(m_Edge) { HFree(m_Edge,g_CacheHeap); m_Edge=NULL; }
//	if(m_EdgeGroup) { HFree(m_EdgeGroup,g_CacheHeap); m_EdgeGroup=NULL; }
//}
//
// void CVectorObject::EdgeBuild()
//{
//    DTRACE();
//	int i,ifr,cnt,tricnt,i0,i1,u0,u1;
//	SVOFrame * fr;
//	SVOGroup * gr;
//	SVOVertexNorTex * vs;
//	int * pgr;
//	SVOVertexNorTex * v;
//
//	EdgeClear();
//
//	m_Edge=(SVOEdge *)HAlloc(Header()->m_TriCnt/3*3*sizeof(SVOEdge)*Header()->m_FrameCnt,g_CacheHeap);
//	int edgecnt=0;
//	m_EdgeGroup=(SVOEdgeGroup *)HAllocClear(Header()->m_FrameCnt*sizeof(SVOEdgeGroup),g_CacheHeap);
//
//	fr=((SVOFrame * )(Data()+Header()->m_FrameSme));
//	for(ifr=0;ifr<Header()->m_FrameCnt;ifr++,fr++) {
//		tricnt=0;
//
//		m_EdgeGroup[ifr].m_EdgeSme=edgecnt;
//
//		pgr=(int *)(Data()+fr->m_GroupIndexSme);
//		for(i=0;i<int(fr->m_GroupIndexCnt);i++,pgr++) tricnt+=GroupGet(*pgr)->m_TriCnt/3;
//
//		int * ltr=(int *)HAlloc(tricnt*3*sizeof(int),g_CacheHeap);
//		D3DXVECTOR3 * lpo=(D3DXVECTOR3 *)HAlloc(tricnt*3*sizeof(D3DXVECTOR3),g_CacheHeap);
//		int * ltrt=ltr;
//		int lpocnt=0;
//
//		pgr=(int *)(Data()+fr->m_GroupIndexSme);
//		for(i=0;i<int(fr->m_GroupIndexCnt);i++,pgr++) {
//			gr=GroupGet(*pgr);
//
//			vs=((SVOVertexNorTex *)(Data()+Header()->m_VerSme))+gr->m_VerStart;
//			byte * in=Data()+Header()->m_TriSme+gr->m_TriStart*m_isz;
//
//			cnt=gr->m_TriCnt/3;
//			while(cnt>0) {
//				for(int u=0;u<3;u++) {
//					v=vs+((Header()->m_Flags&1)?(*(word *)in):(*(dword *)in));
//					D3DXVECTOR3 * pv=&(v->v);
//					in+=m_isz;
//
//					D3DXVECTOR3 * lpot=lpo;
//					for(int k=0;k<lpocnt;k++,lpot++) {
//						if(((double(lpot->x)-double(pv->x))*(double(lpot->x)-double(pv->x))+(double(lpot->y)-double(pv->y))*(double(lpot->y)-double(pv->y))+(double(lpot->z)-double(pv->z))*(double(lpot->z)-double(pv->z)))<0.0001)
//break;
//					}
//					if(k>=lpocnt) {
//						lpo[k]=*pv;
//						lpocnt++;
//					}
//					*ltrt=k;
//					ltrt++;
//				}
//
//				cnt--;
//			}
//		}
//
//		ltrt=ltr;
//		for(i=0;i<tricnt;i++,ltrt+=3) {
//			for(int u=0;u<3;u++) {
//				if(u==0) { i0=ltrt[0]; i1=ltrt[1]; }
//				else if(u==1) { i0=ltrt[1]; i1=ltrt[2]; }
//				else if(u==2) { i0=ltrt[2]; i1=ltrt[0]; }
//
//				int cntff=0;
//
//				int * ltrt2=ltr+(i+1)*3;
//				for(int p=i+1;p<tricnt;p++,ltrt2+=3) {
//					for(int k=0;k<3;k++) {
//						if(k==0) { u0=ltrt2[0]; u1=ltrt2[1]; }
//						else if(k==1) { u0=ltrt2[1]; u1=ltrt2[2]; }
//						else if(k==2) { u0=ltrt2[2]; u1=ltrt2[0]; }
//
//						if(((i0==u0) && (i1==u1)) || ((i0==u1) && (i1==u0))) {
////if(!(edgecnt<int(Header()->m_TriCnt/3*3*Header()->m_FrameCnt))) {
//	ASSERT(edgecnt<int(Header()->m_TriCnt/3*3*Header()->m_FrameCnt));
////}
//							SVOEdge * et=m_Edge+edgecnt;
//							et->m_Tri1=i;
//							et->m_Tri2=p;
//							et->m_Edge1=(BYTE)u;
//							et->m_Enge2=(BYTE)k;
//							edgecnt++;
//							cntff++;
////							break;
//						}
//					}
//					if(k<3) break;
//				}
//				if(cntff>=2) {
//					ERROR_S(CWStr().Format(L"More 2 triangles on edge. Face: <i> Cnt: <i>  Model:
//<s>",i+1,cntff,m_Name.c_str()).c_str());
//				}
//			}
//		}
//
//		m_EdgeGroup[ifr].m_EdgeCnt=edgecnt-m_EdgeGroup[ifr].m_EdgeSme;
//
//		HFree(ltr,g_CacheHeap);
//		HFree(lpo,g_CacheHeap);
//	}
//	m_Edge=(SVOEdge *)HAllocEx(m_Edge,edgecnt*sizeof(SVOEdge),g_CacheHeap);
//}

void CVectorObject::GetBound(int noframe, const D3DXMATRIX &ma, D3DXVECTOR3 &bmin, D3DXVECTOR3 &bmax) const {
    DTRACE();

    SVOKadr *k = m_Geometry.m_Frames + noframe;

    bmin = D3DXVECTOR3(1e30f, 1e30f, 1e30f);
    bmax = D3DXVECTOR3(-1e30f, -1e30f, -1e30f);
    D3DXVECTOR3 v[8];

    v[0] = D3DXVECTOR3(k->m_Min.x, k->m_Min.y, k->m_Min.z);
    v[1] = D3DXVECTOR3(k->m_Max.x, k->m_Min.y, k->m_Min.z);
    v[2] = D3DXVECTOR3(k->m_Max.x, k->m_Max.y, k->m_Min.z);
    v[3] = D3DXVECTOR3(k->m_Min.x, k->m_Max.y, k->m_Min.z);
    v[4] = D3DXVECTOR3(k->m_Min.x, k->m_Min.y, k->m_Max.z);
    v[5] = D3DXVECTOR3(k->m_Max.x, k->m_Min.y, k->m_Max.z);
    v[6] = D3DXVECTOR3(k->m_Max.x, k->m_Max.y, k->m_Max.z);
    v[7] = D3DXVECTOR3(k->m_Min.x, k->m_Max.y, k->m_Max.z);

    D3DXVec3TransformCoordArray(v, sizeof(D3DXVECTOR3), v, sizeof(D3DXVECTOR3), &ma, 8);

    for (int u = 0; u < 8; u++) {
        if (v[u].x < bmin.x)
            bmin.x = v[u].x;
        if (v[u].y < bmin.y)
            bmin.y = v[u].y;
        if (v[u].z < bmin.z)
            bmin.z = v[u].z;
        if (v[u].x > bmax.x)
            bmax.x = v[u].x;
        if (v[u].y > bmax.y)
            bmax.y = v[u].y;
        if (v[u].z > bmax.z)
            bmax.z = v[u].z;
    }
}

void CVectorObject::CalcShadowProjMatrix(int noframe, SProjData &pd, D3DXVECTOR3 &dir, float addsize) {
    DTRACE();
    D3DXMATRIX ml;
    auto tmp1 = -dir;
    auto tmp2 = D3DXVECTOR3(0, 0, 0);
    auto tmp3 = D3DXVECTOR3(0, 0, 1);
    D3DXMatrixLookAtLH(&ml, &tmp1, &tmp2, &tmp3);

    // SVOFrame * fr=((SVOFrame * )(Data()+Header()->m_FrameSme))+noframe;

    D3DXVECTOR3 vmin, vmax;
    GetBound(noframe, ml, vmin, vmax);

    vmin.x -= addsize;
    vmin.y -= addsize;
    vmin.z -= addsize;
    vmax.x += addsize;
    vmax.y += addsize;
    vmax.z += addsize;

    pd.vpos = D3DXVECTOR3(vmin.x, vmin.y, vmin.z);
    pd.vx = D3DXVECTOR3(vmax.x, vmin.y, vmin.z);
    pd.vy = D3DXVECTOR3(vmin.x, vmax.y, vmin.z);
    pd.vz = D3DXVECTOR3(vmin.x, vmin.y, vmax.z);

    D3DXMatrixInverse(&ml, NULL, &ml);

    D3DXVec3TransformCoord(&pd.vpos, &pd.vpos, &ml);
    D3DXVec3TransformCoord(&pd.vx, &pd.vx, &ml);
    D3DXVec3TransformCoord(&pd.vy, &pd.vy, &ml);
    D3DXVec3TransformCoord(&pd.vz, &pd.vz, &ml);

    pd.vx -= pd.vpos;
    pd.vy -= pd.vpos;
    pd.vz -= pd.vpos;
}

void CVectorObject::CalcShadowProjMatrix(int cnt, CVectorObjectAnim **obj, const int *noframe, const D3DXMATRIX *wm,
                                         SProjData &pd, D3DXVECTOR3 &dir, float addsize) {
    DTRACE();
    D3DXMATRIX ml, ml2;
    auto tmp1 = -dir;
    auto tmp2 = D3DXVECTOR3(0, 0, 0);
    auto tmp3 = D3DXVECTOR3(0, 0, 1);
    D3DXMatrixLookAtLH(&ml, &tmp1, &tmp2, &tmp3);

    D3DXVECTOR3 vmin, vmax, _vmin, _vmax;

    vmin.x = 1e30f;
    vmin.y = 1e30f;
    vmin.z = 1e30f;
    vmax.x = -1e30f;
    vmax.y = -1e30f;
    vmax.z = -1e30f;

    for (int i = 0; i < cnt; i++) {
        D3DXMatrixMultiply(&ml2, wm + i, &ml);
        obj[i]->VO()->GetBound(noframe[i], ml2, _vmin, _vmax);
        vmin.x = std::min(vmin.x, _vmin.x - addsize);
        vmin.y = std::min(vmin.y, _vmin.y - addsize);
        vmin.z = std::min(vmin.z, _vmin.z - addsize);
        vmax.x = std::max(vmax.x, _vmax.x + addsize);
        vmax.y = std::max(vmax.y, _vmax.y + addsize);
        vmax.z = std::max(vmax.z, _vmax.z + addsize);
    }

    pd.vpos = D3DXVECTOR3(vmin.x, vmin.y, vmin.z);
    pd.vx = D3DXVECTOR3(vmax.x, vmin.y, vmin.z);
    pd.vy = D3DXVECTOR3(vmin.x, vmax.y, vmin.z);
    pd.vz = D3DXVECTOR3(vmin.x, vmin.y, vmax.z);

    D3DXMatrixInverse(&ml, NULL, &ml);

    D3DXVec3TransformCoord(&pd.vpos, &pd.vpos, &ml);
    D3DXVec3TransformCoord(&pd.vx, &pd.vx, &ml);
    D3DXVec3TransformCoord(&pd.vy, &pd.vy, &ml);
    D3DXVec3TransformCoord(&pd.vz, &pd.vz, &ml);

    pd.vx -= pd.vpos;
    pd.vy -= pd.vpos;
    pd.vz -= pd.vpos;
}

CTextureManaged *CVectorObject::CalcShadowTexture(int cnt, CVectorObjectAnim **obj, const int *noframe,
                                                  const D3DXMATRIX *wm, const SProjData &pd, int texsize,
                                                  CVOShadowCliper *cliper, CBaseTexture *tex_to_update) {
    DTRACE();

    D3DXMATRIX mWorld;
    D3DXMATRIX mView;
    D3DXMATRIX mProj(-2, 0, 0, 0, 0, 2, 0, 0, 0, 0, float(1.0 / (10000 - 1)), 0, 0, 0, -float(1.0 / (10000 - 1)), 1);
    D3DXVECTOR3 vpos(pd.vpos + ((pd.vx + pd.vy) * 0.5f));

    auto tmp1 = vpos - pd.vz;
    auto tmp2 = -pd.vy;
    D3DXMatrixLookAtLH(&mView, &tmp1, &vpos, &tmp2);

    D3DXMatrixIdentity(&mWorld);
    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &mWorld));

    float _sx = (1.0f / D3DXVec3Length(&pd.vx));
    float _sy = (1.0f / D3DXVec3Length(&pd.vy));
    mView._11 *= _sx;
    mView._21 *= _sx;
    mView._31 *= _sx;
    mView._41 *= _sx;
    mView._12 *= _sy;
    mView._22 *= _sy;
    mView._32 *= _sy;
    mView._42 *= _sy;
    ASSERT_DX(g_D3DD->SetTransform(D3DTS_VIEW, &mView));

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_PROJECTION, &mProj));

    return CalcShadowTextureWOMat(cnt, obj, noframe, wm, texsize, cliper, tex_to_update);
}

CTextureManaged *CVectorObject::CalcShadowTextureWOMat(int cnt, CVectorObjectAnim **obj, const int *noframe,
                                                       const D3DXMATRIX *wm, int texsize, CVOShadowCliper *cliper,
                                                       CBaseTexture *tex_to_update) {
    DTRACE();

    // store old viewport
    D3DVIEWPORT9 oldViewport;
    ASSERT_DX(g_D3DD->GetViewport(&oldViewport));

    // IDirect3DSurface9 * oldZBuffer = NULL;
    // FAILED_DX(g_D3DD->GetDepthStencilSurface(&oldZBuffer));

    // set new viewport
    D3DVIEWPORT9 newViewport;
    newViewport.X = 0;
    newViewport.Y = 0;
    newViewport.Width = texsize;
    newViewport.Height = texsize;
    newViewport.MinZ = 0.0f;
    newViewport.MaxZ = 1.0f;
    ASSERT_DX(g_D3DD->SetViewport(&newViewport));

    // store old render target
    IDirect3DSurface9 *oldTarget;
    ASSERT_DX(g_D3DD->GetRenderTarget(0, &oldTarget));

    // create and set render target
    LPDIRECT3DTEXTURE9 newTexture;
    IDirect3DSurface9 *newTarget;
    // IDirect3DSurface9 * tss;
    const int pxsz_src = 4;
    int pxsz_dst;

    if (tex_to_update != NULL) {
        if (!tex_to_update->IsLoaded()) {
            if (D3D_OK != D3DXCreateTexture(g_D3DD, texsize, texsize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8,
                                            D3DPOOL_DEFAULT, &newTexture)) {
                ASSERT_DX(g_D3DD->SetViewport(&oldViewport));
                return NULL;
            }
            ASSERT_DX(newTexture->GetSurfaceLevel(0, &newTarget));
            tex_to_update->Set(newTexture, 0);
        }
        else {
            ASSERT_DX(tex_to_update->DX()->GetSurfaceLevel(0, &newTarget));
        }
    }
    else {
        if (D3D_OK != g_D3DD->CreateRenderTarget(texsize, texsize, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, TRUE,
                                                 &newTarget, 0)) {
            ASSERT_DX(g_D3DD->SetViewport(&oldViewport));
            return NULL;
        }
    }

    ASSERT_DX(g_D3DD->SetRenderTarget(0, newTarget));

    IDirect3DSurface9 *newZ, *oldZ;
    bool CustomZ = (texsize > g_ScreenY) || (texsize > g_ScreenX);
    if (CustomZ) {
        D3DSURFACE_DESC d;
        g_D3DD->GetDepthStencilSurface(&oldZ);
        oldZ->GetDesc(&d);
        g_D3DD->CreateDepthStencilSurface(texsize, texsize, d.Format, d.MultiSampleType, d.MultiSampleQuality, TRUE,
                                          &newZ, NULL);
        g_D3DD->SetDepthStencilSurface(newZ);
        newZ->Release();
    }

    if (FLAG(g_Flags, GFLAG_STENCILAVAILABLE)) {
        ASSERT_DX(g_D3DD->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0x00000000L, 1.0f, 0L));
    }
    else {
        ASSERT_DX(g_D3DD->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000L, 1.0f, 0L));
    }

    // before draw: prepare geometry
    CVectorObject::DrawBegin();

    for (int k = 0; k < cnt; ++k) {
        obj[k]->BeforeDraw();
    }

    if (cliper) {
        cliper->BeforeRender();
    }

    ASSERT_DX(g_D3DD->BeginScene());

    if (cliper) {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE));
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));

        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_COLORWRITEENABLE, 0));
        SetColorOpDisable(0);
        cliper->Render();
    }
    else {
        ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));
    }

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF));

    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));

    SetColorOpSelect(0, D3DTA_TFACTOR);
    SetAlphaOpSelect(0, D3DTA_TEXTURE);
    SetColorOpDisable(1);

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHAREF, 0xC0));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));

    // float bias = 3.0f;
    // for(int k=0;k<4;k++)
    //{
    //    ASSERT_DX(g_D3DD->SetSamplerState(k,D3DSAMP_MIPMAPLODBIAS,		*((LPDWORD) (&bias))));
    //}

    CVectorObject::DrawBegin();

    for (int ob = 0; ob < cnt; ++ob) {
        CVectorObjectAnim *o = obj[ob];
        if (o->GetVOFrame() < 0)
            continue;
        if (wm) {
            g_D3DD->SetTransform(D3DTS_WORLD, wm + ob);
        }
        else {
            D3DXMATRIX m;
            D3DXMatrixIdentity(&m);
            g_D3DD->SetTransform(D3DTS_WORLD, &m);
        }

        int vbase = o->VO()->m_Geometry.m_Vertices.Select(CVectorObject::m_VB);
        if (vbase >= 0) {
            int ibase = o->VO()->m_Geometry.m_Idxs.Select(CVectorObject::m_IB);
            if (ibase >= 0) {
                SVOKadr *k = o->VO()->m_Geometry.m_Frames + noframe[ob];
                for (int i = 0; i < k->m_UnionCnt; ++i) {
                    SVOUnion *gr = o->VO()->m_Geometry.m_Unions + o->VO()->m_Geometry.m_UnionsIdx[i + k->m_UnionStart];

                    const SSkin *skin = o->VO()->m_Geometry.m_Surfaces[gr->m_Surface].skin;
                    if (skin == NULL)
                        skin = o->GetSkin();

                    if (skin == NULL) {
                        g_D3DD->SetTexture(0, NULL);
                    }
                    else {
                        skin->m_SetupTexShadow(skin);
                    }

                    if (gr->m_IBase < 0) {
                        g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, gr->m_Base + vbase, 0, gr->m_VerCnt,
                                                     -gr->m_IBase + ibase, gr->m_TriCnt);
                    }
                    else {
                        g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, gr->m_Base + vbase, gr->m_VerMinIndex,
                                                     gr->m_VerCnt, gr->m_TriStart * 3 + ibase, gr->m_TriCnt);
                    }
                }
            }
        }
    }

    ASSERT_DX(g_D3DD->EndScene());
    ASSERT_DX(g_D3DD->SetRenderTarget(0, oldTarget));

    CTextureManaged *tm = NULL;
    if (tex_to_update == NULL) {
        D3DLOCKED_RECT loro;

        tm = CACHE_CREATE_TEXTUREMANAGED();

        if (D3D_OK == tm->CreateLock(D3DFMT_A8, texsize, texsize, 1, loro)) {
            pxsz_dst = 1;
        }
        else if (D3D_OK == tm->CreateLock(D3DFMT_A8L8, texsize, texsize, 1, loro)) {
            pxsz_dst = 2;
        }
        else if (D3D_OK == tm->CreateLock(D3DFMT_A8R3G3B2, texsize, texsize, 1, loro)) {
            pxsz_dst = 2;
        }
        else {
            if (D3D_OK != tm->CreateLock(D3DFMT_A8R8G8B8, texsize, texsize, 1, loro)) {
                g_Cache->Destroy(tm);
                tm = NULL;
                goto endofshadow;
            }
            pxsz_dst = 4;
        }

        D3DLOCKED_RECT lor;
        newTarget->LockRect(&lor, NULL, 0);

        byte *odata = ((byte *)loro.pBits) + pxsz_dst - 1;
        byte *idata = ((byte *)lor.pBits) + pxsz_src - 1;

        int addi = lor.Pitch - texsize * pxsz_src;
        int addo = loro.Pitch - texsize * pxsz_dst;

        for (int i = 0; i < texsize; i++)
        {
            for (int j = 0; j < texsize; j++)
            {
                *odata = *idata;
                idata += pxsz_src;
                odata += pxsz_dst;
            }

            idata += addi;
            odata += addo;
        }

        // CBitmap bm;
        // bm.CreateRGBA(texsize,texsize,lor.Pitch, lor.pBits);
        // bm.SaveInPNG(L"bla.png");

        tm->UnlockRect();

        newTarget->UnlockRect();
    }
    else {
        // LPDIRECT3DTEXTURE9 out;
        // IDirect3DSurface9 * outtgt;
        // ASSERT_DX(g_D3DD->CreateTexture(texsize, texsize, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &out, NULL));

        // ASSERT_DX(newTexture->GetSurfaceLevel(0, &newTarget));
        // ASSERT_DX(out->GetSurfaceLevel(0, &outtgt));

        // ASSERT_DX(g_D3DD->GetRenderTargetData(newTarget, outtgt));

        // D3DLOCKED_RECT lr;
        // outtgt->LockRect(&lr, NULL, 0);

        // CBitmap bm(g_CacheHeap);
        // bm.CreateRGBA(texsize,texsize,lr.Pitch, lr.pBits);
        // bm.SwapByte(CPoint(0,0),CPoint(texsize,texsize),0,2);

        //            //*(des)=*(sou+2);
        //            //*(des+2)=*(sou+0);

        // bm.SaveInPNG(L"Blabla.png");

        // outtgt->UnlockRect();
    }

    newTarget->Release();

    if (CustomZ) {
        g_D3DD->SetDepthStencilSurface(oldZ);
        oldZ->Release();
    }

endofshadow:

    oldTarget->Release();

    ASSERT_DX(g_D3DD->SetViewport(&oldViewport));

    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
    ASSERT_DX(g_Sampler.SetState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHAREF, 0x08));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
    return tm;
}

/*
void CVectorObject::UnloadTextures(void)
{
    DTRACE();
    if (m_Data.Len() > 0)
    {
        int n = MaterialCount();
        for(int i=0;i<n;i++)
        {
            SVOMaterial * ma=MaterialGet(i);
            if (ma->m_Tex != NULL)
            {
                ma->m_Tex->m_Tex->Unload();
                if (ma->m_Tex->m_TexGloss) ma->m_Tex->m_TexGloss->Unload();
            }
        }
    }
}
*/

void CVectorObject::BeforeDraw(void) {
    DTRACE();

    for (int i = 0; i < m_Geometry.m_SurfacesCnt; ++i) {
        if (m_Geometry.m_Surfaces[i].skin) {
            m_Geometry.m_Surfaces[i].skin->m_Preload(m_Geometry.m_Surfaces[i].skin);
        }
    }

    DX_Prepare();
    m_RemindCore.Use(FREE_VO_TIME_PERIOD);
}

void CVectorObject::Draw(int noframe, DWORD user_param, const SSkin *ds) {
    DTRACE();

    if (noframe < 0)
        return;

#ifdef SHOW_PICK_OBJECTS

    if (m_GeometrySimple) {
        D3DXMATRIX m;
        g_D3DD->GetTransform(D3DTS_WORLD, &m);
        for (int i = 0; i < m_GeometrySimple->m_AsIs.m_TriCnt; ++i) {
            D3DXVECTOR3 p0, p1, p2;
            D3DXVec3TransformCoord(&p0, &m_GeometrySimple->m_Verts[m_GeometrySimple->m_Tris[i].i0], &m);
            D3DXVec3TransformCoord(&p1, &m_GeometrySimple->m_Verts[m_GeometrySimple->m_Tris[i].i1], &m);
            D3DXVec3TransformCoord(&p2, &m_GeometrySimple->m_Verts[m_GeometrySimple->m_Tris[i].i2], &m);
            CHelper::Create(1, 0)->Line(p0, p1);
            CHelper::Create(1, 0)->Line(p1, p2);
            CHelper::Create(1, 0)->Line(p2, p0);
        }
    }
#endif

    int vbase = m_Geometry.m_Vertices.Select(m_VB);
    if (vbase < 0)
        return;
    int ibase = m_Geometry.m_Idxs.Select(m_IB);
    if (ibase < 0)
        return;

    int pass = -1;

    bool *nextpass = (bool *)_alloca(m_Geometry.m_SurfacesCnt);

    for (int i = 0; i < m_Geometry.m_SurfacesCnt; ++i)
        nextpass[i] = true;

    bool again;
    do {
        ++pass;

        again = false;

        for (int surf = 0; surf < m_Geometry.m_SurfacesCnt; ++surf) {
            if (!nextpass[surf])
                continue;

            const SSkin *ss = m_Geometry.m_Surfaces[surf].skin;
            if (ss == NULL)
                ss = ds;

            again |= (nextpass[surf] = ss->m_SetupStages(ss, user_param, pass));
            ss->m_SetupTex(ss, user_param, pass);

            SVOKadr *k = m_Geometry.m_Frames + noframe;
            for (int i = 0; i < k->m_UnionCnt; ++i) {
                SVOUnion *gr = m_Geometry.m_Unions + m_Geometry.m_UnionsIdx[i + k->m_UnionStart];
                if (gr->m_Surface != surf)
                    continue;

#ifdef _DEBUG

                HRESULT res;
                if (gr->m_IBase < 0) {
                    res = g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, gr->m_Base + vbase, 0, gr->m_VerCnt,
                                                       -gr->m_IBase + ibase, gr->m_TriCnt);
                }
                else {
                    res = g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, gr->m_Base + vbase, gr->m_VerMinIndex,
                                                       gr->m_VerCnt, gr->m_TriStart * 3 + ibase, gr->m_TriCnt);
                }

                if (D3D_OK != res) {
                    debugbreak();
                }

#else
                // g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,gr->m_Base +
                // vbase,gr->m_VerMinIndex,gr->m_VerCnt,gr->m_TriStart * 3 + ibase, gr->m_TriCnt);
                if (gr->m_IBase < 0) {
                    g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, gr->m_Base + vbase, 0, gr->m_VerCnt,
                                                 -gr->m_IBase + ibase, gr->m_TriCnt);
                }
                else {
                    g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, gr->m_Base + vbase, gr->m_VerMinIndex,
                                                 gr->m_VerCnt, gr->m_TriStart * 3 + ibase, gr->m_TriCnt);
                }

#endif
            }

            ss->m_SetupClear(ss, user_param);
        }
    }
    while (again);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CVectorObjectAnim::CVectorObjectAnim() : CMain(), m_Lights(NULL), m_LightsCnt(0) {
    m_VO = NULL;

    m_Time = 0;
    m_TimeNext = 0;

    m_Frame = 0;
    m_VOFrame = 0;
    m_Anim = 0;
    m_AnimLooped = 1;
}

CVectorObjectAnim::~CVectorObjectAnim() {
    DTRACE();
    Clear();
}

void CVectorObjectAnim::Clear() {
    DTRACE();
    if (m_VO) {
        // m_VO->RefDecUnload();
        m_VO = NULL;
    }

    if (m_Lights) {
        for (int i = 0; i < m_LightsCnt; ++i) {
            m_Lights[i].Release();
        }
        HFree(m_Lights, g_CacheHeap);
        m_Lights = NULL;
        m_LightsCnt = 0;
    }
}

void SLightData::Release(void) {
    HFree(intervals, g_CacheHeap);
    BB().Release();
}

void CVectorObjectAnim::Init(CVectorObject *vo, CTextureManaged *tex_light, const SSkin *skin) {
    DTRACE();
    Clear();
    m_VO = vo;
    m_Skin = skin;

    if (tex_light)
        InitLights(tex_light);
}

void CVectorObjectAnim::InitLights(CTextureManaged *tex_light) {
    DTRACE();
    ASSERT(m_VO);

    m_LightsCnt = 0;
    for (int i = 0; i < m_VO->GetMatrixCount(); ++i) {
        const wchar *mn = m_VO->GetMatrixName(i);
        if (mn[0] == '$') {
            ++m_LightsCnt;
        }
    }
    if (m_LightsCnt > 0) {
        m_Lights = (SLightData *)HAllocClear(sizeof(SLightData) * m_LightsCnt, g_CacheHeap);

        int l = 0;
        for (int m = 0; m < m_VO->GetMatrixCount(); ++m) {
            const wchar *mn = m_VO->GetMatrixName(m);
            if (mn[0] == '$') {
                ParamParser li_info(m_VO->GetPropValue(m_VO->GetMatrixName(m)));
                float ra = (float)li_info.GetStrPar(0, L",").GetDouble();

                int pars = li_info.GetCountPar(L",");
                m_Lights[l].intervals_cnt = pars - 2;
                if (m_Lights[l].intervals_cnt == 0) {
                    m_Lights[l].intervals = (SColorInterval *)HAlloc(sizeof(SColorInterval), g_CacheHeap);
                    m_Lights[l].intervals_cnt = 1;
                    m_Lights[l].intervals[0].time1 = li_info.GetStrPar(1, L",").GetStrPar(0, L":").GetInt();
                    m_Lights[l].intervals[0].time2 = m_Lights[l].intervals[0].time1;
                    m_Lights[l].intervals[0].c1 =
                            0xFF000000 | li_info.GetStrPar(1, L",").GetStrPar(1, L":").GetHexUnsigned();
                    m_Lights[l].intervals[0].c2 = m_Lights[l].intervals[0].c1;

                    m_Lights[l].period = 0;
                }
                else {
                    m_Lights[l].intervals =
                            (SColorInterval *)HAlloc(sizeof(SColorInterval) * m_Lights[l].intervals_cnt, g_CacheHeap);

                    int *times = (int *)_alloca((sizeof(int) + sizeof(DWORD)) * (pars - 1));
                    DWORD *colors = (DWORD *)(times + (pars - 1));

                    for (int pa = 1; pa < pars; ++pa) {
                        times[pa - 1] = li_info.GetStrPar(pa, L",").GetStrPar(0, L":").GetInt();
                        colors[pa - 1] = 0xFF000000 | li_info.GetStrPar(pa, L",").GetStrPar(1, L":").GetHexUnsigned();
                    }

                    pars -= 1;

                    // bubble sort

                    int idx = 1;
                    while (idx < pars) {
                        if (times[idx - 1] > times[idx]) {
                            // exchange
                            int temp1 = times[idx - 1];
                            DWORD temp2 = colors[idx - 1];
                            times[idx - 1] = times[idx];
                            colors[idx - 1] = colors[idx];
                            times[idx] = temp1;
                            colors[idx] = temp2;

                            if (idx == 1)
                                ++idx;
                            else
                                --idx;
                        }
                        else
                            ++idx;
                    }

                    m_Lights[l].period = times[idx - 1];

                    for (int pa = 0; pa < m_Lights[l].intervals_cnt; ++pa) {
                        m_Lights[l].intervals[pa].c1 = colors[pa];
                        m_Lights[l].intervals[pa].c2 = colors[pa + 1];
                        m_Lights[l].intervals[pa].time1 = times[pa];
                        m_Lights[l].intervals[pa].time2 = times[pa + 1];
                    }
                }

                m_Lights[l].matid = m_VO->GetMatrixId(m);
                new(&m_Lights[l].BB()) CBillboard(TRACE_PARAM_CALL D3DXVECTOR3(0, 0, 0), ra, 0,
                                                        m_Lights[l].intervals[0].c1, tex_light);
                ++l;
            }
        }
    }
}

bool CVectorObjectAnim::Takt(int cms) {
    DTRACE();

    m_Time += cms;

    for (int i = 0; i < m_LightsCnt; ++i) {
        if (m_Lights[i].period > 0) {
            m_Lights[i].time += cms;
            while (m_Lights[i].time > m_Lights[i].period) {
                m_Lights[i].time -= m_Lights[i].period;
            }
            for (int ci = 0; ci < m_Lights[i].intervals_cnt; ++ci) {
                SColorInterval *cis = m_Lights[i].intervals + ci;
                if (m_Lights[i].time >= cis->time1 && m_Lights[i].time <= cis->time2) {
                    int delta = cis->time2 - cis->time1;
                    if (delta == 0)
                        m_Lights[i].BB().SetColor(cis->c1);
                    else {
                        float t = float(m_Lights[i].time - cis->time1) / float(delta);
                        DWORD c = LIC(cis->c1, cis->c2, t);
                        m_Lights[i].BB().SetColor(c);
                    }
                    break;
                }
            }
        }
    }

    int old_frame = m_Frame;

    int fcnt = m_VO->GetAnimFramesCount(m_Anim);

    while (m_Time > m_TimeNext) {
        ++m_Frame;
        if (m_AnimLooped) {
            if (m_Frame >= fcnt)
                m_Frame = 0;
        }
        else {
            if (m_Frame >= fcnt) {
                m_TimeNext += 1000;
                m_Frame = fcnt - 1;
                break;
            }
        }
        m_TimeNext += m_VO->GetAnimFrameTime(m_Anim, m_Frame);
    }
    if (old_frame != m_Frame) {
        m_VOFrame = m_VO->GetAnimFrameIndex(m_Anim, m_Frame);
        return true;
    }
    return false;
}

DWORD CVectorObjectAnim::NextFrame(void) {
    DTRACE();
    int fcnt = m_VO->GetAnimFramesCount(m_Anim);
    if (m_AnimLooped) {
        ++m_Frame;
        if (m_Frame >= fcnt)
            m_Frame = 0;
    }
    else {
        if (m_Frame < (fcnt - 1))
            m_Frame++;
        else
            return m_VO->GetAnimFrameTime(m_Anim, m_Frame);
    }
    m_VOFrame = m_VO->GetAnimFrameIndex(m_Anim, m_Frame);
    return m_VO->GetAnimFrameTime(m_Anim, m_Frame);
}

void CVectorObjectAnim::GetBound(D3DXVECTOR3 &bmin, D3DXVECTOR3 &bmax) const {
    DTRACE();
    ASSERT(m_VO);
    const SVOKadr *k = m_VO->GetFrame(m_VOFrame);
    bmin = k->m_Min;
    bmax = k->m_Max;
}

bool CVectorObjectAnim::Pick(const D3DXMATRIX &ma, const D3DXMATRIX &ima, const D3DXVECTOR3 &orig,
                             const D3DXVECTOR3 &dir, float *outt) const {
    DTRACE();
    if (!m_VO)
        return false;

    return m_VO->Pick(m_VOFrame, ma, ima, orig, dir, outt);
}

bool CVectorObjectAnim::PickFull(const D3DXMATRIX &ma, const D3DXMATRIX &ima, const D3DXVECTOR3 &orig,
                                 const D3DXVECTOR3 &dir, float *outt) const {
    DTRACE();
    if (!m_VO)
        return false;

    return m_VO->PickFull(m_VOFrame, ma, ima, orig, dir, outt);
}

/*
void CVectorObjectAnim::IBCreate()
{
    DTRACE();
    IBDestroy();
    if(!m_VO) return;
    m_IB=m_VO->DXIndexCreate();
}

void CVectorObjectAnim::IBDestroy()
{
    DTRACE();
    if(IsIB(m_IB)) { DestroyVB(m_IB); }
}

void CVectorObjectAnim::SortIndexForAlpha(D3DXMATRIX & ma)
{
    DTRACE();
    if(!m_VO) return;

    if(!m_IB) IBCreate();

    byte * tb=(byte *)HAlloc(m_VO->SortIndexForAlphaSize(),g_CacheHeap);
    m_VO->SortIndexForAlpha(ma,tb);
    m_VO->DXIndexInit(m_IB,tb);
    HFree(tb,g_CacheHeap);
}
*/

void CVectorObjectAnim::BeforeDraw(void) {
    if (m_Skin) {
        m_Skin->m_Preload(m_Skin);
    }
    VO()->BeforeDraw();
};

void CVectorObjectAnim::DrawLights(bool now, const D3DXMATRIX &objma, const D3DXMATRIX *iview) {
    for (int i = 0; i < m_LightsCnt; ++i) {
        const D3DXMATRIX *m = VO()->GetMatrixById(m_VOFrame, m_Lights[i].matid);
        D3DXVec3TransformCoord(&m_Lights[i].BB().m_Pos, (D3DXVECTOR3 *)&m->_41, &objma);

        if (now) {
            ASSERT(iview);
            m_Lights[i].BB().DrawNow(*iview);
        }
        else {
            m_Lights[i].BB().SortIntense();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CVectorObjectGroupUnit::CVectorObjectGroupUnit() : CMain(), m_LinkMatrixName{}, m_Name{} {
    DTRACE();

    m_Parent = NULL;
    m_Prev = NULL;
    m_Next = NULL;

    m_Id = 0;

    m_Obj = HNew(g_CacheHeap) CVectorObjectAnim();

    m_Link = NULL;
    m_LinkMatrixId = -1;

    m_Flags = VOUF_MATRIX | VOUF_SHADOWSTENCIL;
    D3DXMatrixIdentity(&m_Matrix);
    D3DXMatrixIdentity(&m_MatrixWorld);
    m_ShadowStencil = NULL;
}

CVectorObjectGroupUnit::~CVectorObjectGroupUnit() {
    DTRACE();

    if (m_ShadowStencil) {
        HDelete(CVOShadowStencil, m_ShadowStencil, g_CacheHeap);
        m_ShadowStencil = NULL;
    }

    if (m_Obj)
        HDelete(CVectorObjectAnim, m_Obj, g_CacheHeap);
}

void CVectorObjectGroupUnit::RNeed(dword need) {
    DTRACE();
    if (need & m_Flags & (VOUF_MATRIX)) {
        RESETFLAG(m_Flags, VOUF_MATRIX);

        if (m_Link) {
            m_Link->RNeed(VOUF_MATRIX);

            const D3DXMATRIX *um;
            if (m_LinkMatrixId >= 0)
                um = m_Link->m_Obj->GetMatrixById(m_LinkMatrixId);
            else
                um = m_Link->m_Obj->GetMatrixByName(m_LinkMatrixName.c_str());

            if (FLAG(m_Flags, VOUF_MATRIX_USE)) {
                m_MatrixWorld = m_Matrix * (*um) * m_Link->m_MatrixWorld;
            }
            else {
                m_MatrixWorld = (*um) * m_Link->m_MatrixWorld;
            }
        }
        else {
            if (FLAG(m_Flags, VOUF_MATRIX_USE)) {
                m_MatrixWorld = m_Matrix * (*m_Parent->m_GroupToWorldMatrix);
            }
            else {
                m_MatrixWorld = *m_Parent->m_GroupToWorldMatrix;
            }
        }
        D3DXMatrixInverse(&m_IMatrixWorld, NULL, &m_MatrixWorld);
    }
    if (need & m_Flags & (VOUF_SHADOWSTENCIL)) {
        RESETFLAG(m_Flags, VOUF_SHADOWSTENCIL);

        if (!FLAG(m_Flags, VOUF_STENCIL_ON)) {
            if (m_ShadowStencil) {
                HDelete(CVOShadowStencil, m_ShadowStencil, g_CacheHeap);
                m_ShadowStencil = NULL;
            }
        }
        else {
            // if(m_Obj->VO()->EdgeExist())
            {
                if (!m_ShadowStencil)
                    m_ShadowStencil = HNew(g_CacheHeap) CVOShadowStencil();

                // m_ShadowStencil->Build(*(m_Obj->VO()),m_Obj->FrameVO(),m_Parent->m_ShadowStencilLight,m_MatrixWorld,m_Parent->m_ShadowStencilCutPlane);
                // STENCIL
                D3DXVECTOR3 light;
                D3DXVec3TransformNormal(&light, &m_Parent->m_ShadowStencilLight, &m_IMatrixWorld);
                // D3DXPLANE cutpl;
                // D3DXPlaneTransform(&cutpl, &m_Parent->m_ShadowStencilCutPlane, &m_IMatrixWorld);
                m_ShadowStencil->Build(*(m_Obj->VO()), m_Obj->GetVOFrame(), light,
                                       (float)fabs(m_MatrixWorld._43 - m_Parent->m_GroundZ), false);
            }
        }
    }
}

void CVectorObjectGroupUnit::ShadowStencilOn(bool zn) {
    INITFLAG(m_Flags, VOUF_STENCIL_ON, zn);

    if (zn) {
        RChange(VOUF_SHADOWSTENCIL);
    }
    else {
        if (m_ShadowStencil) {
            HDelete(CVOShadowStencil, m_ShadowStencil, g_CacheHeap);
            m_ShadowStencil = NULL;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CVectorObjectGroup::CVectorObjectGroup() : CMain(), m_Name{}, m_GroundZ(0) {
    DTRACE();

    m_First = NULL;
    m_Last = NULL;

    m_GroupToWorldMatrix = NULL;

    m_ShadowStencilLight.x = 0.0f;
    m_ShadowStencilLight.y = 0.0f;
    m_ShadowStencilLight.z = -1.0f;
    // m_ShadowStencilCutPlane.a=0.0f;	m_ShadowStencilCutPlane.b=0.0f;	m_ShadowStencilCutPlane.c=1.0f;
    // m_ShadowStencilCutPlane.d=0.0f;
}

CVectorObjectGroup::~CVectorObjectGroup() {
    DTRACE();

    Clear();
}

void CVectorObjectGroup::Clear() {
    DTRACE();

    while (m_First)
        Delete(m_Last);
}

void CVectorObjectGroup::Delete(CVectorObjectGroupUnit *un) {
    DTRACE();

    LIST_DEL(un, m_First, m_Last, m_Prev, m_Next);
    HDelete(CVectorObjectGroupUnit, un, g_CacheHeap);
}

CVectorObjectGroupUnit *CVectorObjectGroup::Add() {
    DTRACE();

    CVectorObjectGroupUnit *un = HNew(g_CacheHeap) CVectorObjectGroupUnit();
    un->m_Parent = this;
    un->RChange(VOUF_SHADOWSTENCIL);
    LIST_ADD(un, m_First, m_Last, m_Prev, m_Next);
    return un;
}

CVectorObjectGroupUnit *CVectorObjectGroup::GetByName(const wchar *name) {
    CVectorObjectGroupUnit *un = m_First;
    while (un != NULL) {
        if (un->m_Name == name)
            return un;
        un = un->m_Next;
    }
    ERROR_E;
}

CVectorObjectGroupUnit *CVectorObjectGroup::GetByNameNE(const wchar *name) {
    CVectorObjectGroupUnit *un = m_First;
    while (un != NULL) {
        if (un->m_Name == name)
            return un;
        un = un->m_Next;
    }
    return NULL;
}

CVectorObjectGroupUnit *CVectorObjectGroup::GetById(int id) {
    CVectorObjectGroupUnit *un = m_First;
    while (un != NULL) {
        if (un->m_Id == id)
            return un;
        un = un->m_Next;
    }
    ERROR_E;
}

CVectorObjectGroupUnit *CVectorObjectGroup::GetByIdNE(int id) {
    CVectorObjectGroupUnit *un = m_First;
    while (un != NULL) {
        if (un->m_Id == id)
            return un;
        un = un->m_Next;
    }
    return NULL;
}

void CVectorObjectGroup::RNeed(dword need) {
    CVectorObjectGroupUnit *un = m_First;
    while (un != NULL) {
        un->RNeed(need);
        un = un->m_Next;
    }
}

void CVectorObjectGroup::RChange(dword zn) {
    CVectorObjectGroupUnit *un = m_First;
    while (un != NULL) {
        un->RChange(zn);
        un = un->m_Next;
    }
}

void CVectorObjectGroup::RChangeByLink(CVectorObjectGroupUnit *link, dword zn) {
    CVectorObjectGroupUnit *un = m_First;
    while (un != NULL) {
        if (un->m_Link == link) {
            un->RChange(zn);
            RChangeByLink(un, zn);
        }
        un = un->m_Next;
    }
}

bool CVectorObjectGroup::Takt(int cms) {
    DTRACE();

    bool rv = false;
    CVectorObjectGroupUnit *u = m_First;
    while (u) {
        if (u->m_Obj) {
            bool change = u->m_Obj->Takt(cms);
            rv |= change;
            if (change) {
                u->RChange(VOUF_SHADOWSTENCIL);
                RChangeByLink(u, VOUF_MATRIX | VOUF_SHADOWSTENCIL);
            }
        }
        u = u->m_Next;
    }

    return rv;
}

CVectorObjectGroupUnit *CVectorObjectGroup::Pick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, float *outt) const {
    DTRACE();

    float otmin, ot;
    CVectorObjectGroupUnit *rv = NULL;

    CVectorObjectGroupUnit *u = m_First;
    while (u) {
        if (u->m_Obj && u->m_Obj->Pick(u->m_MatrixWorld, u->m_IMatrixWorld, orig, dir, &ot)) {
            if (!rv || ot < otmin) {
                rv = u;
                otmin = ot;
            }
        }
        u = u->m_Next;
    }

    if (rv && outt)
        *outt = otmin;
    return rv;
}

void CVectorObjectGroup::BoundGet(D3DXVECTOR3 &bmin, D3DXVECTOR3 &bmax) {
    DTRACE();

    RNeed(VOUF_MATRIX | VOUF_SHADOWSTENCIL);

    bool ok = false;
    D3DXVECTOR3 bminout, bmaxout;

    CVectorObjectGroupUnit *u = m_First;
    while (u) {
        if (u->m_Obj && u->m_Obj->VO()) {
            u->m_Obj->VO()->GetBound(u->m_Obj->GetVOFrame(), u->m_MatrixWorld, bminout, bmaxout);

            if (!ok) {
                ok = true;
                bmin = bminout;
                bmax = bmaxout;
            }
            else {
                bmin.x = std::min(bmin.x, bminout.x);
                bmin.y = std::min(bmin.y, bminout.y);
                bmin.z = std::min(bmin.z, bminout.z);

                bmax.x = std::max(bmax.x, bmaxout.x);
                bmax.y = std::max(bmax.y, bmaxout.y);
                bmax.z = std::max(bmax.z, bmaxout.z);
            }
        }
        u = u->m_Next;
    }
}

D3DXVECTOR3 CVectorObjectGroup::GetPosByName(const wchar *name) const {
    const D3DXMATRIX *m = NULL;
    CVectorObjectGroupUnit *u = m_First;
    while (u) {
        m = u->m_Obj->GetMatrixByName(name);
        if (m) {
            D3DXVECTOR3 pos;
            D3DXVec3TransformCoord(&pos, (D3DXVECTOR3 *)&m->_41, &u->m_MatrixWorld);
            return pos;
            break;
        }
        u = u->m_Next;
    }
    return *(D3DXVECTOR3 *)&m_GroupToWorldMatrix->_41;
}

void CVectorObjectGroup::BoundGetAllFrame(D3DXVECTOR3 &bmin, D3DXVECTOR3 &bmax) {
    DTRACE();

    RNeed(VOUF_MATRIX | VOUF_SHADOWSTENCIL);

    bool ok = false;
    D3DXVECTOR3 bminout, bmaxout;

    CVectorObjectGroupUnit *u = m_First;
    while (u) {
        if (u->m_Obj && u->m_Obj->VO()) {
            int cnt = u->m_Obj->VO()->GetFramesCnt();
            for (int i = 0; i < cnt; i++) {
                u->m_Obj->VO()->GetBound(i, u->m_MatrixWorld, bminout, bmaxout);

                if (!ok) {
                    ok = true;
                    bmin = bminout;
                    bmax = bmaxout;
                }
                else {
                    bmin.x = std::min(bmin.x, bminout.x);
                    bmin.y = std::min(bmin.y, bminout.y);
                    bmin.z = std::min(bmin.z, bminout.z);

                    bmax.x = std::max(bmax.x, bmaxout.x);
                    bmax.y = std::max(bmax.y, bmaxout.y);
                    bmax.z = std::max(bmax.z, bmaxout.z);
                }
            }
        }
        u = u->m_Next;
    }
}

void CVectorObjectGroup::ShadowStencilOn(bool zn) {
    CVectorObjectGroupUnit *u = m_First;
    while (u) {
        u->ShadowStencilOn(zn);
        u = u->m_Next;
    }
}

struct SEVH_data_group {
    const D3DXMATRIX *m;
    DWORD data;
    ENUM_VERTS_HANDLER evh;
};

static bool evhg_tp(const SVOVertex &v, DWORD data) {
    SEVH_data_group *d = (SEVH_data_group *)data;
    SVOVertex vv = v;
    D3DXVec3TransformCoord(&vv.v, &v.v, d->m);
    return d->evh(vv, d->data);
};

void CVectorObjectGroup::EnumFrameVerts_transform_position(ENUM_VERTS_HANDLER evh, DWORD data) const {
    SEVH_data_group d;
    d.data = data;
    d.evh = evh;

    CVectorObjectGroupUnit *u = m_First;
    while (u) {
        d.m = &u->m_MatrixWorld;
        u->m_Obj->EnumFrameVerts(evhg_tp, (DWORD)&d);
        u = u->m_Next;
    }
}

void CVectorObjectGroup::Load(const wchar *filename, CTextureManaged *lt, SKIN_GET sg, DWORD gsp) {
    DTRACE();

    Clear();

    // if(need & m_RChange & (VOR_Graph)) {
    //	m_RChange&=~VOR_Graph;

    //	if(!m_Obj)
    //       {
    //		m_Obj=HNew(g_CacheHeap) CVectorObjectAnim;
    //		m_Obj->Init((CVectorObject *)(g_Cache->Get(cc_VO,m_Model.c_str(), m_Reminder)), &m_Skin);

    //		m_Obj->VO()->AddName(m_Texture.IsEmpty()?m_Parent->m_Name.c_str():m_Texture.c_str());
    //		m_Obj->VO()->Prepare();
    //           m_Obj->SetAnimDefault();
    //	}
    //}

    m_Name = filename;

    std::wstring tstr, tstr2, unit, bs;
    std::wstring texture, texture_gloss, texture_back, texture_mask;

    CBlockPar bp;
    CVectorObjectGroupUnit *gu;

    bp.LoadFromTextFile(filename);

    int cnt = bp.BlockCount();
    for (int i = 0; i < cnt; i++) {
        texture = L"";
        texture_gloss = L"";
        texture_mask = L"";
        texture_back = L"";
        bs = L"";

        CBlockPar &bp2 = *bp.BlockGet(i);

        gu = Add();
        gu->m_Name = bp.BlockGetName(i);

        tstr = bp2.ParGetNE(L"Model");
        if (!tstr.empty()) {
            auto idx = tstr.find(L"?");
            if (idx != std::wstring::npos)
                tstr.resize(idx);
            CacheReplaceFileNameAndExt(unit, filename, tstr.c_str());
        }

        if (bp2.ParCount(L"Id") > 0)
            gu->m_Id = bp2.ParGet(L"Id").GetInt();

        if (bp2.ParCount(L"Texture")) {
            CacheReplaceFileNameAndExt(texture, filename, bp2.ParGet(L"Texture").c_str());
        }
        if (bp2.ParCount(L"TextureGloss")) {
            CacheReplaceFileNameAndExt(texture_gloss, filename, bp2.ParGet(L"TextureGloss").c_str());
        }
        if (bp2.ParCount(L"TextureBack")) {
            CacheReplaceFileNameAndExt(texture_back, filename, bp2.ParGet(L"TextureBack").c_str());
        }
        if (bp2.ParCount(L"TextureMask")) {
            CacheReplaceFileNameAndExt(texture_mask, filename, bp2.ParGet(L"TextureMask").c_str());
        }
        if (bp2.ParCount(L"TextureBackScroll")) {
            bs = bp2.ParGet(L"TextureBackScroll");
        }

        if (bp2.ParCount(L"Link"))
            gu->m_LinkMatrixName = bp2.ParGet(L"Link");

        if (texture_mask.empty())
            texture_back.clear();

        CVectorObject *vo = (CVectorObject *)(g_Cache->Get(cc_VO, unit.c_str()));
        if (texture.empty() && texture_gloss.empty() && texture_mask.empty()) {
            vo->PrepareSpecial(OLF_AUTO, sg, gsp);
            gu->m_Obj->Init(vo, NULL);
        }
        else {
            vo->PrepareSpecial(OLF_MULTIMATERIAL_ONLY, sg, gsp);

            if (vo->IsNoSkin(0)) {
                if (texture.empty()) {
                    // oops....
                    texture = vo->GetSurfaceFileName(0);
                }

                tstr2 = texture + L"*" + texture_gloss + L"*" + texture_back + L"*" + texture_mask + L"*" + bs;
                const SSkin *skin = sg(tstr2.c_str(), gsp);
                gu->m_Obj->Init(vo, lt, skin);
            }
            else {
                gu->m_Obj->Init(vo, lt);
            }
        }
    }

    gu = m_First;
    while (gu) {
        ParamParser LinkMatrixName{gu->m_LinkMatrixName};
        if (!LinkMatrixName.empty()) {
            if (LinkMatrixName.GetCountPar(L",") != 2)
                ERROR_S(utils::format(L"In file: %ls   Unknown format link: %ls", filename, LinkMatrixName.c_str()));

            tstr = LinkMatrixName.GetStrPar(0, L",");
            LinkMatrixName = LinkMatrixName.GetStrPar(1, L",");

            if (ParamParser{tstr}.IsOnlyInt()) {
                gu->m_Link = GetByIdNE(ParamParser{tstr}.GetInt());
            }
            else {
                gu->m_Link = GetByNameNE(tstr.c_str());
            }
            if (gu->m_Link == NULL)
                ERROR_S(utils::format(L"In file: %ls   Not found link: %ls", filename, tstr.c_str()));

            if (LinkMatrixName.IsOnlyInt()) {
                gu->m_LinkMatrixId = LinkMatrixName.GetInt();
                gu->m_LinkMatrixName.clear();
            }
            else {
                gu->m_LinkMatrixId = -2;
            }
        }
        gu = gu->m_Next;
    }

    RChange(VOUF_MATRIX | VOUF_SHADOWSTENCIL);
}

void CVectorObjectGroup::BeforeDraw(bool proceed_shadows) {
    DTRACE();

    CVectorObjectGroupUnit *u = m_First;
    while (u) {
        if (u->m_Obj) {
            u->m_Obj->BeforeDraw();
            if (proceed_shadows && u->m_ShadowStencil)
                u->m_ShadowStencil->BeforeRender();
        }
        if (FLAG(u->m_Flags, VOUF_STENCIL_ON)) {
            u->RNeed(VOUF_MATRIX | VOUF_SHADOWSTENCIL);
        }
        else {
            u->RNeed(VOUF_MATRIX);
        }
        u = u->m_Next;
    }
}

void CVectorObjectGroup::DrawLights(bool now, const D3DXMATRIX *iview) {
    DTRACE();

    CVectorObjectGroupUnit *u = m_First;
    while (u) {
        if (u->m_Obj) {
            u->m_Obj->DrawLights(now, u->m_MatrixWorld, iview);
        }
        u = u->m_Next;
    }
}

void CVectorObjectGroup::Draw(DWORD userparam) {
    DTRACE();

    CVectorObjectGroupUnit *u = m_First;
    while (u) {
        if (u->m_Obj) {
            g_D3DD->SetTransform(D3DTS_WORLD, &(u->m_MatrixWorld));
            u->m_Obj->Draw(userparam);
        }
        u = u->m_Next;
    }
}

void CVectorObjectGroup::ShadowStencilDraw(void) {
    DTRACE();

    CVectorObjectGroupUnit *u = m_First;
    while (u) {
        if (u->m_ShadowStencil) {
            u->m_ShadowStencil->Render(u->m_MatrixWorld);
        }
        u = u->m_Next;
    }
}

void CVectorObjectGroup::ShadowStencil_DX_Free(void) {
    CVectorObjectGroupUnit *u = m_First;
    while (u) {
        if (u->m_ShadowStencil) {
            u->m_ShadowStencil->DX_Free();
        }
        u = u->m_Next;
    }
}
