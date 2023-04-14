// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "MatrixWater.hpp"
#include "MatrixMap.hpp"
#include <math.h>

CTextureManaged *SInshorewave::m_Tex;
D3D_VB SInshorewave::m_VB;
int SInshorewave::m_VB_ref;

void SInshorewave::Create(int index, const D3DXVECTOR2 &pos, const D3DXVECTOR2 &dir, SInshorewave &iw) {
    DTRACE();
    D3DXVECTOR3 stop;
    float len = 10 + FRND(5);

    /*
    if (TRACE_STOP_NONE == g_MatrixMap->Trace(&stop, D3DXVECTOR3(pos.x, pos.y, WATER_LEVEL),
                                                    D3DXVECTOR3(pos.x, pos.y, WATER_LEVEL) + 100 * D3DXVECTOR3(dir.x,
    dir.y, 0), TRACE_LANDSCAPE))
    */
    { stop = D3DXVECTOR3(pos.x, pos.y, WATER_LEVEL) + D3DXVECTOR3(dir.x, dir.y, 0) * len; }

    float a = FSRND(0.2f);
    float sn = TableSin(a);
    float cs = TableCos(a);

    iw.pos = pos;
    iw.dir = D3DXVECTOR2(cs * dir.x - sn * dir.y, sn * dir.x + cs * dir.y);
    iw.len = len;
    iw.t = 0;
    iw.speed = FRND(0.3f) + 0.7f;
    iw.scale = 10;
    iw.m_Index = index;

    if (m_VB_ref == 0) {
        m_Tex = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, g_MatrixData->BlockGet(PAR_SOURCE_WATER)
                                                                           ->BlockGet(g_MatrixMap->m_WaterName)
                                                                           ->ParGet(PAR_SOURCE_WATER_INSHORE).c_str());
        g_Cache->Up(m_Tex);
    }
    ++m_VB_ref;
}

bool SInshorewave::PrepareVB(void) {
    DTRACE();
    if (IS_VB(m_VB))
        return true;

    SInshoreVertex *v;
    CREATE_VB(4 * sizeof(SInshoreVertex), INSHORE_FVF, m_VB);

    if (!IS_VB(m_VB))
        return false;

    LOCK_VB(m_VB, &v);

    v[0].p.x = -1;
    v[0].p.y = -1;
    v[0].p.z = 0;
    v[0].tu = 0;
    v[0].tv = 0;

    v[1].p.x = 1;
    v[1].p.y = -1;
    v[1].p.z = 0;
    v[1].tu = 1;
    v[1].tv = 0;

    v[3].p.x = 1;
    v[3].p.y = 1;
    v[3].p.z = 0;
    v[3].tu = 1;
    v[3].tv = 1;

    v[2].p.x = -1;
    v[2].p.y = 1;
    v[2].p.z = 0;
    v[2].tu = 0;
    v[2].tv = 1;

    UNLOCK_VB(m_VB);

    return true;
}

void SInshorewave::Release(void) {
    DTRACE();
    --m_VB_ref;
    if (m_VB_ref <= 0) {
        m_VB_ref = 0;
        if (IS_VB(m_VB)) {
            DESTROY_VB(m_VB);
        }
        m_Tex->Unload();
        m_Tex = NULL;
    }
}

void SInshorewave::DrawBegin(void) {
    DTRACE();

    if (!IS_VB(m_VB))
        return;

    ASSERT_DX(g_D3DD->SetFVF(INSHORE_FVF));

    // if (m_Intense) g_D3DD->SetRenderState(D3DRS_DESTBLEND,  D3DBLEND_ONE  );

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));

    SetColorOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);
    SetAlphaOpAnyOrder(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_TFACTOR);

    SetColorOpDisable(1);

    ASSERT_DX(g_D3DD->SetTexture(0, SInshorewave::m_Tex->Tex()));
    g_D3DD->SetStreamSource(0, GET_VB(SInshorewave::m_VB), 0, sizeof(SInshoreVertex));

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));

    // g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
}

void SInshorewave::DrawEnd(void) {
    DTRACE();

    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    ASSERT_DX(g_D3DD->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));

    // g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
}

void SInshorewave::Draw(void) {
    DTRACE();

    if (!IS_VB(m_VB))
        return;

    ///////
    float xx = pos.x + (dir.x) * t * len;
    float yy = pos.y + (dir.y) * t * len;

    // CHelper::Create(1,0)->Line( D3DXVECTOR3(xx,yy,-10), D3DXVECTOR3(xx,yy,10) );
    // return;

    D3DXMATRIX m;

    m._11 = dir.x * scale;
    m._12 = dir.y * scale;
    m._13 = 0;
    m._14 = 0;
    m._21 = -dir.y * scale;
    m._22 = dir.x * scale;
    m._23 = 0;
    m._24 = 0;
    m._31 = 0;
    m._32 = 0;
    m._33 = 1;
    m._34 = 0;
    m._41 = xx;
    m._42 = yy;
    m._43 = WATER_LEVEL;
    m._44 = 1;

    BYTE a = BYTE((KSCALE(t, 0, 0.6f) * 255) - (KSCALE(t, 0.6f, 1.0f) * 255));

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m));

    g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, (g_MatrixMap->m_InshorewaveColor & 0x00FFFFFF) | (a << 24));
    g_D3DD->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CMatrixWater::CMatrixWater() : CMain() {
    DTRACE();

    //	m_WaterVB=NULL;
    m_WaterTex1 = NULL;
    m_WaterTex2 = NULL;

    m_VB = NULL;
    m_IB = NULL;

    const int pole = (WATER_SIZE) * (WATER_SIZE);
    for (int i = 0; i < pole; ++i) {
        // h[i] = (float)(rand()%255)/255.0f;
        // r[i] = (float)(rand()%1024)/2500.0f;//3024.0f;
        r[i] = (float)512 / 2500.0f;  // 3024.0f;
        f[i] = rand() % (SIN_TABLE_SIZE - 1);
    }

    m_angle = 0;
    m_next_time = WATER_TIME_PERIOD;
}

CMatrixWater::~CMatrixWater() {
    DTRACE();
    Clear();
}

void CMatrixWater::Clear() {
    DTRACE();
    if (IS_VB(m_VB)) {
        DESTROY_VB(m_VB);
    }
    if (IS_IB(m_IB)) {
        DESTROY_IB(m_IB);
    }
    m_WaterTex1 = NULL;
    m_WaterTex2 = NULL;
}

void CMatrixWater::Init() {
    DTRACE();

    CBlockPar *bp = g_MatrixData->BlockGet(PAR_SOURCE_WATER)->BlockGet(g_MatrixMap->m_WaterName);

    m_WaterTex1 = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, bp->ParGet(PAR_SOURCE_WATER_WATER).c_str());
    m_WaterTex2 = (CTextureManaged *)g_Cache->Get(cc_TextureManaged, bp->ParGet(PAR_SOURCE_WATER_MIRROR).c_str());

    m_WaterTex1->Tex()->SetPriority(0xFFFFFFFF);
    m_WaterTex2->Tex()->SetPriority(0xFFFFFFFF);

    const int pole = (WATER_SIZE + 1) * (WATER_SIZE + 1);
    const int one_strip_idxs = (WATER_SIZE * 2 + 2);
    const int all_idxs = one_strip_idxs * WATER_SIZE + (WATER_SIZE - 1) * 2;
    const int pole_size = (WATER_SIZE) * (WATER_SIZE);

    CREATE_VB_DYNAMIC(pole * sizeof(SMatrixMapWaterVertex), MATRIX_WATER_VERTEX_FORMAT, m_VB);
    CREATE_IB16(all_idxs, m_IB);

    // build indexes

    WORD *ibuf;
    LOCK_IB(m_IB, &ibuf);
    int k = 0;

    for (int i = 0; i < WATER_SIZE; ++i) {
        if (k > 0) {
            ibuf[k + 0] = ibuf[k - 1];
            ibuf[k + 1] = (i + 1) * (WATER_SIZE + 1);

            k += 2;
        }

        ibuf[k++] = (i + 1) * (WATER_SIZE + 1);
        ibuf[k++] = i * (WATER_SIZE + 1);
        for (int j = 0; j < WATER_SIZE; ++j) {
            ibuf[k++] = (WORD)(j + (WATER_SIZE + 2)) + i * (WATER_SIZE + 1);
            ibuf[k++] = (WORD)(j + 1) + i * (WATER_SIZE + 1);
        }
    }
    UNLOCK_IB(m_IB);

    FillVB(0);
}

void CMatrixWater::BeforeDraw(void) {
    if (SInshorewave::m_Tex)
        SInshorewave::m_Tex->Preload();
    SInshorewave::PrepareVB();
    m_WaterTex1->Preload();
    m_WaterTex2->Preload();
}

void CMatrixWater::Draw(const D3DXMATRIX &m) {
    DTRACE();

    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m));

    // D3DXMATRIX tm;
    // D3DXMatrixTranspose(&tm, &m);
    // g_D3DD->SetVertexShaderConstantF(0, (float *)&tm, 4);

    // for (int i = 0; i<WATER_SIZE; ++i)
    //{
    //    ASSERT_DX(g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,i*(WATER_SIZE+1),0,WATER_SIZE*2+2,0,WATER_SIZE*2));
    //}

    const int pole = (WATER_SIZE + 1) * (WATER_SIZE + 1);
    const int one_strip_idxs = (WATER_SIZE * 2 + 2);
    const int all_idxs = one_strip_idxs * WATER_SIZE + (WATER_SIZE - 1) * 2;

    ASSERT_DX(g_D3DD->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, pole, 0, all_idxs - 2));
}

void CMatrixWater::Takt(int step) {
    bool skip = true;

    int k = 1;

    while (g_MatrixMap->GetTime() > m_next_time) {
        m_next_time += WATER_TIME_PERIOD;
        ++k;
        skip = false;
    }
    if (skip)
        return;

    FillVB(k);
}

void CMatrixWater::FillVB(int k) {
    DTRACE();
    // building vertexes
    const int pole = (WATER_SIZE + 1) * (WATER_SIZE + 1);
    const int one_strip_idxs = (WATER_SIZE * 2 + 2);
    const int all_idxs = one_strip_idxs * WATER_SIZE + (WATER_SIZE - 1) * 2;
    const int pole_size = (WATER_SIZE) * (WATER_SIZE);

    m_angle += k;
    for (int zzz = 0; zzz < pole_size; ++zzz) {
        h[zzz] = r[zzz] * SinCosTable[(m_angle + f[zzz]) & (SIN_TABLE_SIZE - 1)];
    }

    int j;

    SMatrixMapWaterVertex *vbuf;
    LOCK_VB_DYNAMIC(m_VB, &vbuf);
    // memcpy(pbuf,vbuf,pole*sizeof(SMatrixMapWaterVertex));

    float _1watersize = 1.0f / (WATER_SIZE);
    float u, v;
    float au, av;

    av = 0;
    v = 0;

    k = 0;
    int k1 = 0;
    for (j = 0; j < WATER_SIZE; ++j) {
        u = 0;
        au = 0;
        float hh = h[k1];
        for (int i = 0; i < WATER_SIZE; ++i, ++k) {
            vbuf[k].v.x = (float)i;
            vbuf[k].v.y = (float)j;
            vbuf[k].v.z = h[k1++];
            vbuf[k].tu = u;
            vbuf[k].tv = v;
            vbuf[k].au = au;
            vbuf[k].av = av;
            u += WATER_TEXTURE_SCALE;
            au += _1watersize;
        }
        vbuf[k].v.x = (float)WATER_SIZE;
        vbuf[k].v.y = (float)j;
        vbuf[k].v.z = hh;
        vbuf[k].tu = u;
        vbuf[k].tv = v;
        vbuf[k].au = au;
        vbuf[k].av = av;
        v += WATER_TEXTURE_SCALE;
        av += _1watersize;
        ++k;
    }
    u = 0;
    au = 0;
    for (int i = 0; i <= WATER_SIZE; ++i, ++k) {
        vbuf[k].v.x = (float)i;
        vbuf[k].v.y = (float)j;
        vbuf[k].v.z = vbuf[i].v.z;
        vbuf[k].tu = u;
        vbuf[k].tv = v;
        vbuf[k].au = au;
        vbuf[k].av = av;
        u += WATER_TEXTURE_SCALE;
        au += _1watersize;
    }

    // calculate normals
    for (int j = 0; j <= WATER_SIZE; ++j) {
        for (int i = 0; i <= WATER_SIZE; ++i) {
            int c = j * (WATER_SIZE + 1) + i;

            int il = (i) ? (i - 1) : (WATER_SIZE - 1);
            int ir = (i < WATER_SIZE) ? (i + 1) : (1);

            int ju = (j) ? (j - 1) : (WATER_SIZE - 1);
            int jd = (j < WATER_SIZE) ? (j + 1) : (1);

            int cl = j * (WATER_SIZE + 1) + il;
            int cr = j * (WATER_SIZE + 1) + ir;
            int cu = ju * (WATER_SIZE + 1) + i;
            int cd = jd * (WATER_SIZE + 1) + i;

            vbuf[c].n = D3DXVECTOR3(vbuf[cl].v.z - vbuf[cr].v.z, vbuf[cu].v.z - vbuf[cd].v.z, 1);
            D3DXVec3Normalize(&vbuf[c].n, &vbuf[c].n);

            vbuf[c].n *= g_MatrixMap->m_WaterNormalLen;

            // vbuf[c].n *= (GLOBAL_SCALE * (float)(MAP_GROUP_SIZE) / (float)(WATER_SIZE));
        }
    }

    UNLOCK_VB(m_VB);
}
