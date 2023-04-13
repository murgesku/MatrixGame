// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include <new>
#include <algorithm>

#include "MatrixObject.hpp"
#include "MatrixObjectBuilding.hpp"
#include "MatrixObjectRobot.hpp"
#include "MatrixShadowManager.hpp"
#include "MatrixFlyer.hpp"
#include "MatrixRenderPipeline.hpp"
#include "ShadowStencil.hpp"
#include "MatrixRobot.hpp"

#include "Effects/MatrixEffectElevatorField.hpp"
#include "Effects/MatrixEffectSmokeAndFire.hpp"

#include "CFile.hpp"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CMatrixRobot::CMatrixRobot() : CMatrixMapStatic(), m_Animation(ANIMATION_OFF), m_Name{L"ROBOT"} {
    DTRACE();
    m_Core->m_Type = OBJECT_TYPE_ROBOTAI;

    m_ShadowProj = NULL;

    m_PosX = m_PosY = 0.0f;

    m_Side = 0;

    m_UnitCnt = 0;
    ZeroMemory(m_Unit, sizeof(m_Unit));

    m_CurrState = ROBOT_IN_SPAWN;

    m_ShadowType = SHADOW_STENCIL;
    m_ShadowSize = 128;

    m_Velocity = D3DXVECTOR3(0, 0, 0);
    m_Forward = D3DXVECTOR3(0, 0, 0);
    m_HullForward = D3DXVECTOR3(0, 0, 0);

    m_HullRotAngle = 0;

    m_Speed = 0;
    m_RotSpeed = 0;
    m_KeelWaterCount = 0;
    m_FallingSpeed = 0;

    m_defHitPoint = 0;

    m_PB.Modify(1000000, 0, PB_ROBOT_WIDTH, 1);

    memset(&m_ChassisData, 0, sizeof(m_ChassisData));

    m_CalcBoundsLastTime = -101;
    m_ShowHitpointTime = 0;

    m_MiniMapFlashTime = 0;
}

CMatrixRobot::~CMatrixRobot() {
    DTRACE();
    UnitClear();
    if (m_ShadowProj) {
        HDelete(CVOShadowProj, m_ShadowProj, g_MatrixHeap);
        m_ShadowProj = NULL;
    }
}

void CMatrixRobot::UnitInsert(int beforeunit, ERobotUnitType type, ERobotUnitKind kind) {
    DTRACE();
    ASSERT(m_UnitCnt < MR_MAXUNIT);
    ASSERT(beforeunit >= 0 && beforeunit <= m_UnitCnt);

    if (beforeunit < m_UnitCnt)
        MoveMemory(m_Unit + beforeunit + 1, m_Unit + beforeunit, (m_UnitCnt - beforeunit) * sizeof(SMatrixRobotUnit));
    ZeroMemory(m_Unit + beforeunit, sizeof(SMatrixRobotUnit));

    m_UnitCnt++;

    m_Unit[beforeunit].m_Type = type;
    m_Unit[beforeunit].u1.s1.m_Kind = kind;
    D3DXMatrixIdentity(&m_Unit[beforeunit].m_Matrix);

    m_Unit[beforeunit].u1.s1.m_NextAnimTime = (float)g_MatrixMap->GetTime();

    RChange(MR_ShadowStencil | MR_ShadowProjGeom | MR_ShadowProjTex);

    if (type == MRT_CHASSIS) {
        if (kind == RUK_CHASSIS_ANTIGRAVITY) {
            m_ChassisData.u1.s1.m_LStream = CMatrixEffect::CreateFireStream(D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(10, 0, 0));
            m_ChassisData.u1.s1.m_RStream = CMatrixEffect::CreateFireStream(D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(10, 0, 0));
            m_ChassisData.u1.s1.m_StreamLen = 10;
        }
        else if (kind == RUK_CHASSIS_TRACK || kind == RUK_CHASSIS_WHEEL) {
            m_ChassisData.u1.s3.m_LastSolePos = D3DXVECTOR3(0, 0, 0);
        }
    }
}

void CMatrixRobot::WeaponInsert(int beforeunit, ERobotUnitType type, ERobotUnitKind kind, int hullno, int pilon) {
    DTRACE();

    ASSERT(m_UnitCnt < MR_MAXUNIT);
    ASSERT(beforeunit >= 0 && beforeunit <= m_UnitCnt);

    if (beforeunit < m_UnitCnt)
        MoveMemory(m_Unit + beforeunit + 1, m_Unit + beforeunit, (m_UnitCnt - beforeunit) * sizeof(SMatrixRobotUnit));
    ZeroMemory(m_Unit + beforeunit, sizeof(SMatrixRobotUnit));

    m_UnitCnt++;

    m_Unit[beforeunit].m_Type = type;
    m_Unit[beforeunit].u1.s1.m_Kind = kind;

    int fis_pilon = pilon;
    int pilon_ost = pilon;
    int weapon_num = g_MatrixMap->m_RobotWeaponMatrix[hullno - 1].cnt;

    if (1 /*kind != 5 && kind != 7*/) {
        if (!(g_MatrixMap->m_RobotWeaponMatrix[hullno - 1].list[pilon - 1].access_invert & (1 << (kind - 1)))) {
            int t;
            for (t = 0; t < weapon_num && pilon_ost > 0; t++) {
                if ((g_MatrixMap->m_RobotWeaponMatrix[hullno - 1].list[t].access_invert & (1 << (kind - 1)))) {
                    pilon_ost--;
                }
            }

            fis_pilon = t;
        }
    }

    m_Unit[beforeunit].u1.s1.m_LinkMatrix = g_MatrixMap->m_RobotWeaponMatrix[hullno - 1].list[fis_pilon - 1].id;
    m_Unit[beforeunit].u1.s1.m_Invert =
            (g_MatrixMap->m_RobotWeaponMatrix[hullno - 1].list[fis_pilon - 1].access_invert & SETBIT(31)) != 0;

    D3DXMatrixIdentity(&m_Unit[beforeunit].m_Matrix);

    m_Unit[beforeunit].u1.s1.m_NextAnimTime = (float)g_MatrixMap->GetTime();

    RChange(MR_ShadowStencil | MR_ShadowProjTex);
}

void CMatrixRobot::UnitDelete(int nounit) {
    DTRACE();
    ASSERT(nounit >= 0 && nounit < m_UnitCnt);

    if (m_CurrState != ROBOT_DIP && m_Unit[nounit].u1.s1.m_Kind == RUK_CHASSIS_ANTIGRAVITY &&
        m_Unit[nounit].m_Type == MRT_CHASSIS) {
        m_ChassisData.u1.s1.m_LStream->Release();
        m_ChassisData.u1.s1.m_RStream->Release();
    }

    if (m_Unit[nounit].m_Graph) {
        UnloadObject(m_Unit[nounit].m_Graph, g_MatrixHeap);
        m_Unit[nounit].m_Graph = NULL;
    }
    if (m_CurrState == ROBOT_DIP) {
        if (m_Unit[nounit].Smoke().effect) {
            ((CMatrixEffectSmoke *)m_Unit[nounit].Smoke().effect)->SetTTL(1000);
            m_Unit[nounit].Smoke().Unconnect();
        }
    }
    else {
        if (m_Unit[nounit].u1.s1.m_WeaponRepairData) {
            m_Unit[nounit].u1.s1.m_WeaponRepairData->Release();
            m_Unit[nounit].u1.s1.m_WeaponRepairData = NULL;
        }
        if (m_Unit[nounit].u1.s1.m_ShadowStencil) {
            HDelete(CVOShadowStencil, m_Unit[nounit].u1.s1.m_ShadowStencil, g_MatrixHeap);
            m_Unit[nounit].u1.s1.m_ShadowStencil = NULL;
        }
    }

    if (nounit < (m_UnitCnt - 1))
        memcpy(m_Unit + nounit, m_Unit + nounit + 1, (m_UnitCnt - 1 - nounit) * sizeof(SMatrixRobotUnit));
    m_UnitCnt--;
    memset(m_Unit + m_UnitCnt, 0, sizeof(SMatrixRobotUnit));
    if (m_CurrState == ROBOT_DIP) {
        while (nounit < m_UnitCnt) {
            m_Unit[nounit].Smoke().Rebase();
            ++nounit;
        }
    }

    RChange(MR_ShadowStencil | MR_ShadowProjGeom | MR_ShadowProjTex);
}

void CMatrixRobot::UnitClear(void) {
    DTRACE();

    while (m_UnitCnt)
        UnitDelete(m_UnitCnt - 1);

    RChange(MR_ShadowStencil | MR_ShadowProjGeom | MR_ShadowProjTex);
}

void CMatrixRobot::BoundGet(D3DXVECTOR3 &bmin, D3DXVECTOR3 &bmax) {
    DTRACE();
    bmin = D3DXVECTOR3(1e30f, 1e30f, 1e30f);
    bmax = D3DXVECTOR3(-1e30f, -1e30f, -1e30f);

    D3DXVECTOR3 tmin;
    D3DXVECTOR3 tmax;
    D3DXVECTOR3 v[8];

    for (int i = 0; i < m_UnitCnt; i++) {
        m_Unit[i].m_Graph->GetBound(tmin, tmax);

        v[0] = D3DXVECTOR3(tmin.x, tmin.y, tmin.z);
        v[1] = D3DXVECTOR3(tmax.x, tmin.y, tmin.z);
        v[2] = D3DXVECTOR3(tmax.x, tmax.y, tmin.z);
        v[3] = D3DXVECTOR3(tmin.x, tmax.y, tmin.z);
        v[4] = D3DXVECTOR3(tmin.x, tmin.y, tmax.z);
        v[5] = D3DXVECTOR3(tmax.x, tmin.y, tmax.z);
        v[6] = D3DXVECTOR3(tmax.x, tmax.y, tmax.z);
        v[7] = D3DXVECTOR3(tmin.x, tmax.y, tmax.z);

        for (int u = 0; u < 8; u++) {
            D3DXVec3TransformCoord(&(v[u]), &(v[u]), &(m_Unit[i].m_Matrix));
            bmin.x = std::min(bmin.x, v[u].x);
            bmin.y = std::min(bmin.y, v[u].y);
            bmin.z = std::min(bmin.z, v[u].z);
            bmax.x = std::max(bmax.x, v[u].x);
            bmax.y = std::max(bmax.y, v[u].y);
            bmax.z = std::max(bmax.z, v[u].z);
        }
    }
}

void CMatrixRobot::WeaponSelectMatrix(void) {
    DTRACE();

    int i;
    for (i = 0; i < m_UnitCnt; i++) {
        if (m_Unit[i].m_Type == MRT_ARMOR)
            break;
    }
    if (i >= m_UnitCnt)
        return;

    int hullno = m_Unit[i].u1.s1.m_Kind - 1;
    bool maccess[MR_MAXUNIT];
    for (i = 0; i < MR_MAXUNIT; i++)
        maccess[i] = true;
    for (i = 0; i < m_UnitCnt; i++) {
        if (m_Unit[i].m_Type != MRT_WEAPON)
            continue;
        int t;
        for (t = 0; t < g_MatrixMap->m_RobotWeaponMatrix[hullno].cnt; t++) {
            if (!maccess[t])
                continue;
            if (!(g_MatrixMap->m_RobotWeaponMatrix[hullno].list[t].access_invert & (1 << (m_Unit[i].u1.s1.m_Kind - 1))))
                continue;
            maccess[t] = false;
            break;
        }
        if (t >= g_MatrixMap->m_RobotWeaponMatrix[hullno].cnt)
            ERROR_E;
        m_Unit[i].u1.s1.m_LinkMatrix = g_MatrixMap->m_RobotWeaponMatrix[hullno].list[t].id;
        m_Unit[i].u1.s1.m_Invert = (g_MatrixMap->m_RobotWeaponMatrix[hullno].list[t].access_invert & SETBIT(31)) != 0;
    }
}

float CMatrixRobot::GetChassisHeight(void) const {
    const D3DXMATRIX *tm = m_Unit[0].m_Graph->GetMatrixById(1);
    return tm->_43;
}

float CMatrixRobot::Z_From_Pos(void) {
    float roboz = g_MatrixMap->GetZ(m_PosX, m_PosY);
    if (roboz < WATER_LEVEL) {
        SETFLAG(m_ObjectState, ROBOT_FLAG_ONWATER);
        if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_HOVERCRAFT || m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_ANTIGRAVITY)
            roboz = WATER_LEVEL;

        if (roboz < WATER_LEVEL - 100) {
            // утонула? :(
            MustDie();
        }
    }
    else {
        RESETFLAG(m_ObjectState, ROBOT_FLAG_ONWATER);
    }
    return roboz;
}

void CMatrixRobot::RNeed(dword need) {
    DTRACE();

    if (m_CurrState == ROBOT_DIP || IsMustDie())
        return;

    if (need & m_RChange & (MR_Graph)) {
        m_RChange &= ~MR_Graph;

        //#ifdef _DEBUG
        //        SETFLAG(g_Flags, SETBIT(22));
        //#endif

        // float hp = (float)g_Config.m_RobotHitPoint;
        // InitMaxHitpoint(hp);

        std::wstring name, name_e;
        std::wstring path;

        for (int i = 0; i < m_UnitCnt; i++) {
            if (!m_Unit[i].m_Graph) {
                switch (m_Unit[i].m_Type) {
                    case MRT_CHASSIS:
                        path = OBJECT_PATH_ROBOT_CHASSIS;
                        break;
                    case MRT_WEAPON:
                        path = OBJECT_PATH_ROBOT_WEAPON;
                        break;
                    case MRT_ARMOR:
                        path = OBJECT_PATH_ROBOT_ARMOR;
                        break;
                    case MRT_HEAD:
                        path = OBJECT_PATH_ROBOT_HEAD;
                        break;
                    default:
                        ERROR_S(L"Unknown robot unit type");
                }
                path += utils::format(L"%d", static_cast<int>(m_Unit[i].u1.s1.m_Kind));

                name = path.c_str();

                if (m_Side != PLAYER_SIDE) {
                    name_e = path + L"_e";
                    if (CFile::FileExist(name_e, name_e.c_str(), L"dds~png")) {
                        name = name_e;
                    }
                    if (CFile::FileExist(name_e, (name + GLOSS_TEXTURE_SUFFIX).c_str(), L"dds~png")) {
                        name += L"*" + name_e;
                    }
                    else if (CFile::FileExist(name_e, (path + GLOSS_TEXTURE_SUFFIX).c_str(), L"dds~png")) {
                        name += L"*" + name_e;
                    }
                }

                m_Unit[i].m_Graph = LoadObject((path + L".vo").c_str(), g_MatrixHeap, true, name.c_str());

                m_Unit[i].m_Graph->SetAnimByName(ANIMATION_NAME_IDLE);
            }
        }
    }
    DCP();
    if (need & m_RChange & (MR_Matrix)) {
        m_RChange &= ~MR_Matrix;
        float roboz;
        D3DXVECTOR3 side, up;
        if (m_CurrState == ROBOT_IN_SPAWN || m_CurrState == ROBOT_BASE_CAPTURE) {
            DCP();
#if defined _TRACE || defined _DEBUG
            if (TruncFloat(m_PosX * INVERT(GLOBAL_SCALE)) >= (g_MatrixMap->m_Size.x) ||
                TruncFloat(m_PosY * INVERT(GLOBAL_SCALE)) >= g_MatrixMap->m_Size.y || m_PosX < 0 || m_PosY < 0) {
#ifdef _DEBUG
                debugbreak();
#endif
                ERROR_S(L"Crash!");
            }
#endif
            DCP();
            SMatrixMapUnit *mu = g_MatrixMap->UnitGet(TruncFloat(m_PosX * INVERT(GLOBAL_SCALE)),
                                                      TruncFloat(m_PosY * INVERT(GLOBAL_SCALE)));
            DCP();
            if (mu->m_Base) {
                roboz = mu->m_Base->GetFloorZ();
            }
            else {
                MustDie();
                return;
            }

            DCP();
            SwitchAnimation(ANIMATION_STAY);
            DCP();
        }
        else if (m_CurrState == ROBOT_CARRYING) {
            DCP();
            D3DXVec3Normalize(&up, &m_CargoFlyer->GetCarryData()->m_RobotUp);
            D3DXVec3Cross(&side, &m_Forward, &up);
            D3DXVec3Normalize(&side, &side);
            D3DXVec3Cross(&m_Forward, &up, &side);

            m_Core->m_Matrix._11 = side.x;
            m_Core->m_Matrix._12 = side.y;
            m_Core->m_Matrix._13 = side.z;
            m_Core->m_Matrix._14 = 0;
            m_Core->m_Matrix._21 = m_Forward.x;
            m_Core->m_Matrix._22 = m_Forward.y;
            m_Core->m_Matrix._23 = m_Forward.z;
            m_Core->m_Matrix._24 = 0;
            m_Core->m_Matrix._31 = up.x;
            m_Core->m_Matrix._32 = up.y;
            m_Core->m_Matrix._33 = up.z;
            m_Core->m_Matrix._34 = 0;
            // m_Core->m_Matrix._41 = m_PosX;                     m_Core->m_Matrix._42 = m_PosY; m_Core->m_Matrix._43 =
            // roboz;
            m_Core->m_Matrix._44 = 1;

            m_HullForward = m_Forward;

            SwitchAnimation(ANIMATION_OFF);

            goto skip_matrix;
        }
        else if (m_CurrState == ROBOT_BASE_MOVEOUT) {
            DCP();
            SMatrixMapUnit *u = g_MatrixMap->UnitGet(TruncFloat(m_PosX * INVERT(GLOBAL_SCALE)),
                                                     TruncFloat(m_PosY * INVERT(GLOBAL_SCALE)));
            if (u->IsLand() || u->IsWater()) {
                roboz = g_MatrixMap->GetZ(m_PosX, m_PosY);
            }
            else {
                roboz = u->m_Base->GetFloorZ();
            }

            SwitchAnimation(ANIMATION_MOVE);
        }
        else if (m_CurrState == ROBOT_FALLING) {
            DCP();
            SwitchAnimation(ANIMATION_OFF);

            goto skip_matrix;
        }
        else if (m_CurrState == ROBOT_EMBRYO) {
            DCP();
            roboz = 0;
            SwitchAnimation(ANIMATION_STAY);
        }
        else {
            DCP();
            roboz = Z_From_Pos();
        }

        {
            D3DXVECTOR3 tmp_forward;

            // D3DXVec3Normalize(&up, (D3DXVECTOR3*)&m_Unit[0].m_Matrix._31);
            D3DXVec3Normalize(&up, (D3DXVECTOR3 *)&m_Core->m_Matrix._31);
            // g_MatrixMap->GetNormal(&up, m_PosX,m_PosY);
            D3DXVec3Cross(&side, &m_Forward, &up);
            D3DXVec3Normalize(&side, &side);
            D3DXVec3Cross(&tmp_forward, &up, &side);

            m_Core->m_Matrix._11 = side.x;
            m_Core->m_Matrix._12 = side.y;
            m_Core->m_Matrix._13 = side.z;
            m_Core->m_Matrix._14 = 0;
            m_Core->m_Matrix._21 = tmp_forward.x;
            m_Core->m_Matrix._22 = tmp_forward.y;
            m_Core->m_Matrix._23 = tmp_forward.z;
            m_Core->m_Matrix._24 = 0;
            m_Core->m_Matrix._31 = up.x;
            m_Core->m_Matrix._32 = up.y;
            m_Core->m_Matrix._33 = up.z;
            m_Core->m_Matrix._34 = 0;
            m_Core->m_Matrix._41 = m_PosX;
            m_Core->m_Matrix._42 = m_PosY;
            m_Core->m_Matrix._43 = roboz;
            m_Core->m_Matrix._44 = 1;
        }
    skip_matrix:

        DCP();

        // if (m_Unit[0].m_Kind == RUK_CHASSIS_HOVERCRAFT)
        //{
        //    double a = ((((DWORD)this) >> 3) & 1024)/1024*M_PI + double(g_MatrixMap->GetTime() & 1023) / 1024 * 2 *
        //    M_PI; double r = 1;

        //    m_Matrix._43 += float(r * sin(a));
        //}

        D3DXMatrixInverse(&m_Core->m_IMatrix, NULL, &m_Core->m_Matrix);
        DCP();

        // int cntweaponset=0;
        int narmor = -1;

        D3DXMATRIX m;
        const D3DXMATRIX *tm;
        D3DXVECTOR3 p;
        // calc main matrix
        {
            DCP();
            ASSERT(m_Unit[0].m_Type == MRT_CHASSIS);
            DCP();

            ASSERT(m_Unit[0].m_Graph);
            DCP();
            tm = m_Unit[0].m_Graph->GetMatrixById(1);
            DCP();
            p = *(D3DXVECTOR3 *)&tm->_41;

            DCP();
            m_Unit[0].m_Matrix = m_Core->m_Matrix;
            m_Unit[0].u1.s1.m_IMatrix = m_Core->m_IMatrix;
        }

        bool www = false;

        for (int i = 1; i < m_UnitCnt; i++) {
            DCP();
            if (m_Unit[i].m_Type == MRT_WEAPON) {
                ASSERT(narmor >= 0);

                if (!www && m_Unit[i].u1.s1.m_Kind == RUK_WEAPON_REPAIR) {
                    www = true;
                }

                tm = m_Unit[narmor].m_Graph->GetMatrixById(m_Unit[i].u1.s1.m_LinkMatrix);
                m_Unit[i].m_Matrix = (*tm) * m_Unit[narmor].m_Matrix;
                if (m_Unit[i].u1.s1.m_Invert) {
                    D3DXMATRIX *temp = &m_Unit[i].m_Matrix;
                    temp->_11 = -temp->_11;
                    temp->_12 = -temp->_12;
                    temp->_13 = -temp->_13;
                }
                D3DXMatrixInverse(&m_Unit[i].u1.s1.m_IMatrix, NULL, &m_Unit[i].m_Matrix);
            }
            else if (m_Unit[i].m_Type == MRT_ARMOR) {
                narmor = i;
                // D3DXMatrixRotationZ(&m,m_HullRotAngle);
                D3DXMatrixIdentity(&m);
                D3DXVECTOR3 th;
                D3DXVec3TransformNormal(&th, &m_HullForward, &m_Core->m_IMatrix);

                m._21 = th.x;
                m._22 = th.y;
                m._11 = th.y;
                m._12 = -th.x;

                if (g_MatrixMap->GetPlayerSide()->GetArcadedObject() == this &&
                    m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_PNEUMATIC) {
                    p = g_MatrixMap->GetPlayerSide()->CorrectArcadedRobotArmorP(p, this);
                }

                goto calc;
            }
            else if (m_Unit[i].m_Type == MRT_HEAD) {
                D3DXMatrixIdentity(&m);
                D3DXVECTOR3 th;
                D3DXVec3TransformNormal(&th, &m_HullForward, &m_Core->m_IMatrix);

                m._21 = th.x;
                m._22 = th.y;
                m._11 = th.y;
                m._12 = -th.x;

                goto calc;
            }
            else {
                D3DXMatrixRotationZ(&m, m_Unit[i].u1.s1.m_Angle);
            calc:
                *(D3DXVECTOR3 *)&m._41 = p;
                m_Unit[i].m_Matrix = m * m_Core->m_Matrix;
                D3DXMatrixInverse(&m_Unit[i].u1.s1.m_IMatrix, NULL, &m_Unit[i].m_Matrix);

                ASSERT(m_Unit[i].m_Graph);
                tm = m_Unit[i].m_Graph->GetMatrixById(1);

                p.x += tm->_41 * m._11 + tm->_42 * m._21;
                p.y += tm->_41 * m._12 + tm->_42 * m._22;
                p.z += tm->_43;
            }
        }

        if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_ANTIGRAVITY) {
            // D3DXMATRIX mx;
            // BuildRotateMatrix(mx, *(D3DXVECTOR3 *)&m_Unit[narmor].m_Matrix._41, *(D3DXVECTOR3 *)&m_Matrix._11,
            // -0.4f); m_Unit[0].m_Matrix *= mx; D3DXMatrixInverse(&m_Unit[0].m_IMatrix, NULL, &m_Unit[0].m_Matrix);

            D3DXVECTOR3 spos;
            const D3DXMATRIX *m = m_Unit[0].m_Graph->GetMatrixById(10);
            D3DXVECTOR3 sdir = -*(D3DXVECTOR3 *)&m->_21;
            D3DXVec3TransformCoord(&spos, (D3DXVECTOR3 *)&m->_41, &m_Unit[0].m_Matrix);
            D3DXVec3TransformNormal(&sdir, &sdir, &m_Unit[0].m_Matrix);
            m_ChassisData.u1.s1.m_LStream->SetPos(spos, spos + sdir * m_ChassisData.u1.s1.m_StreamLen);

            m = m_Unit[0].m_Graph->GetMatrixById(11);
            sdir = -*(D3DXVECTOR3 *)&m->_21;
            D3DXVec3TransformCoord(&spos, (D3DXVECTOR3 *)&m->_41, &m_Unit[0].m_Matrix);
            D3DXVec3TransformNormal(&sdir, &sdir, &m_Unit[0].m_Matrix);
            m_ChassisData.u1.s1.m_RStream->SetPos(spos, spos + sdir * m_ChassisData.u1.s1.m_StreamLen);
        }

        if (www) {
            if (IsInterfaceDraw()) {
                // if (0)
                for (int i = 0; i < m_UnitCnt; i++) {
                    if (m_Unit[i].m_Type == MRT_WEAPON && m_Unit[i].u1.s1.m_Kind == RUK_WEAPON_REPAIR) {
                        if (m_Unit[i].u1.s1.m_WeaponRepairData == NULL) {
                            m_Unit[i].u1.s1.m_WeaponRepairData = SWeaponRepairData::Allocate();
                        }

                        m_Unit[i].u1.s1.m_WeaponRepairData->Update(m_Unit + i);
                    }
                }

                need &= ~(MR_ShadowStencil);
                m_RChange &= ~(MR_ShadowStencil);
            }
            else {
                int cnt = this->AsRobot()->m_WeaponsCnt;
                for (int i = 0; i < cnt; ++i) {
                    SBotWeapon *w = this->AsRobot()->m_Weapons + i;
                    w->UpdateRepair();
                }
            }
        }
    }
    /*
    if(need & m_RChange & (MR_Sort)) {
        m_RChange&=~MR_Sort;

        D3DXVECTOR3 pz=D3DXVECTOR3(0,0,0);
        D3DXVec3TransformCoord(&pz,&pz,&(m_Matrix*g_MatrixMap->GetSortMatrix()));
        m_Z=pz.z;
    }
    if(need & m_RChange & (MR_GraphSort)) {
        m_RChange&=~MR_GraphSort;

        for(int i=0;i<m_UnitCnt;i++) {
            ASSERT(m_Unit[i].m_Graph);
            m_Unit[i].m_Graph->SortIndexForAlpha(m_Matrix*g_MatrixMap->GetViewMatrix());
        }
    }
    */
    if (need & m_RChange & MR_ShadowStencil) {
        m_RChange &= ~MR_ShadowStencil;

        if (m_ShadowType != SHADOW_STENCIL) {
            for (int i = 0; i < m_UnitCnt; i++) {
                if (m_Unit[i].u1.s1.m_ShadowStencil) {
                    HDelete(CVOShadowStencil, m_Unit[i].u1.s1.m_ShadowStencil, g_MatrixHeap);
                    m_Unit[i].u1.s1.m_ShadowStencil = NULL;
                }
            }
        }
        else {
            for (int i = 0; i < m_UnitCnt; i++) {
                ASSERT(m_Unit[i].m_Graph);
                ASSERT(m_Unit[i].m_Graph->VO());

                // if(m_Unit[i].m_Graph->VO()->EdgeExist()) {

                if (!m_Unit[i].u1.s1.m_ShadowStencil)
                    m_Unit[i].u1.s1.m_ShadowStencil = HNew(g_MatrixHeap) CVOShadowStencil();

                //					if(!(m_Unit[i].m_Graph->VO()->EdgeExist())) m_Unit[i].m_Graph->VO()->EdgeBuild();

                // STENCIL
                // m_Unit[i].u1.s1.m_ShadowStencil->Build(*(m_Unit[i].m_Graph->VO()),m_Unit[i].m_Graph->FrameVO(),
                //    g_MatrixMap->m_LightMain,m_Unit[i].m_Matrix,IsNearBase()?g_MatrixMap->m_ShadowPlaneCutBase:g_MatrixMap->m_ShadowPlaneCut);

                D3DXVECTOR3 light;
                D3DXVec3TransformNormal(&light, &g_MatrixMap->m_LightMain, &m_Unit[i].u1.s1.m_IMatrix);

                // D3DXPLANE cutpl;
                // D3DXPlaneTransform(&cutpl,
                // IsNearBase()?(&g_MatrixMap->m_ShadowPlaneCutBase):(&g_MatrixMap->m_ShadowPlaneCut),
                // &m_Unit[i].m_IMatrix);

                float len = (GetRadius() * 2) + m_Unit[i].m_Matrix._43 -
                            (IsNearBase() ? g_MatrixMap->m_GroundZBase : g_MatrixMap->m_GroundZ);

                m_Unit[i].u1.s1.m_ShadowStencil->Build(*(m_Unit[i].m_Graph->VO()), m_Unit[i].m_Graph->GetVOFrame(), light,
                                                 len, m_Unit[i].u1.s1.m_Invert != 0);

                if (!m_Unit[i].u1.s1.m_ShadowStencil->IsReady()) {
                    HDelete(CVOShadowStencil, m_Unit[i].u1.s1.m_ShadowStencil, g_MatrixHeap);
                    m_Unit[i].u1.s1.m_ShadowStencil = NULL;
                }
            }
        }
    }
    if (need & m_RChange & MR_ShadowProjGeom) {
        m_RChange &= ~MR_ShadowProjGeom;

        if (m_ShadowType != SHADOW_PROJ_DYNAMIC) {
            if (m_ShadowProj) {
                HDelete(CVOShadowProj, m_ShadowProj, g_MatrixHeap);
                m_ShadowProj = NULL;
            }
        }
        else {
            if (!m_ShadowProj)
                m_ShadowProj = HNew(g_MatrixHeap) CVOShadowProj(g_MatrixHeap);

            BYTE *buf = (BYTE *)_alloca(sizeof(CVectorObjectAnim *) * m_UnitCnt + sizeof(int) * m_UnitCnt +
                                        sizeof(D3DXMATRIX) * m_UnitCnt);
            CVectorObjectAnim **obj = (CVectorObjectAnim **)buf;
            int *noframe = (int *)(buf + sizeof(CVectorObjectAnim *) * m_UnitCnt);
            D3DXMATRIX *wm = (D3DXMATRIX *)(buf + sizeof(CVectorObjectAnim *) * m_UnitCnt + sizeof(int) * m_UnitCnt);

            for (int i = 0; i < m_UnitCnt; i++) {
                ASSERT(m_Unit[i].m_Graph);
                ASSERT(m_Unit[i].m_Graph->VO());

                obj[i] = m_Unit[i].m_Graph;
                noframe[i] = m_Unit[i].m_Graph->GetVOFrame();
                wm[i] = m_Unit[i].m_Matrix * m_Core->m_IMatrix;
            }

            ShadowProjBuildGeomList(*m_ShadowProj, m_UnitCnt, obj, noframe, wm, m_Core->m_Matrix, m_Core->m_IMatrix,
                                    g_MatrixMap->m_LightMain, int(100 / GLOBAL_SCALE), false);

            if (!(m_ShadowProj->IsProjected())) {
                HDelete(CVOShadowProj, m_ShadowProj, g_MatrixHeap);
                m_ShadowProj = NULL;
            }
        }
    }
    if (need & m_RChange & MR_ShadowProjTex) {
        m_RChange &= ~MR_ShadowProjTex;

        if (m_ShadowProj != NULL) {
            BYTE *buf = (BYTE *)_alloca(sizeof(CVectorObjectAnim *) * m_UnitCnt + sizeof(int) * m_UnitCnt +
                                        sizeof(D3DXMATRIX) * m_UnitCnt);
            CVectorObjectAnim **obj = (CVectorObjectAnim **)buf;
            int *noframe = (int *)(buf + sizeof(CVectorObjectAnim *) * m_UnitCnt);
            D3DXMATRIX *wm = (D3DXMATRIX *)(buf + sizeof(CVectorObjectAnim *) * m_UnitCnt + sizeof(int) * m_UnitCnt);

            for (int i = 0; i < m_UnitCnt; i++) {
                ASSERT(m_Unit[i].m_Graph);
                ASSERT(m_Unit[i].m_Graph->VO());

                obj[i] = m_Unit[i].m_Graph;
                noframe[i] = m_Unit[i].m_Graph->GetVOFrame();
                wm[i] = m_Unit[i].m_Matrix * m_Core->m_IMatrix;
            }

            CTexture *tex = NULL;

            ShadowProjBuildTextureList(*m_ShadowProj, m_UnitCnt, obj, tex, noframe, wm, m_Core->m_Matrix,
                                       m_Core->m_IMatrix, g_MatrixMap->m_LightMain, m_ShadowSize);
        }
    }
}

void CMatrixRobot::DoAnimation(int cms) {
    // non chassis animation
    for (int i = 1; i < m_UnitCnt; i++) {
        if (m_Unit[i].m_Graph) {
            if (m_Unit[i].m_Graph->Takt(cms)) {
                if (m_ShadowType == SHADOW_STENCIL)
                    RChange(MR_ShadowStencil);
                else if (m_ShadowType == SHADOW_PROJ_DYNAMIC)
                    RChange(MR_ShadowProjTex);
            }

            if (m_Unit[i].m_Graph->IsAnimEnd()) {
                m_Unit[i].m_Graph->SetAnimByName(ANIMATION_NAME_IDLE);
            }
        }
    }

    if (m_Animation == ANIMATION_STAY || m_Animation == ANIMATION_BEGINMOVE || m_Animation == ANIMATION_ENDMOVE ||
        m_Animation == ANIMATION_BEGINMOVE_BACK || m_Animation == ANIMATION_ENDMOVE_BACK) {
        if ((m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_HOVERCRAFT) || (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_PNEUMATIC) ||
            (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_ANTIGRAVITY)) {
            for (int i = 0; i < 1; i++) {
                // if (m_Unit[i].m_Type != MRT_CHASSIS) continue;
                if (m_Unit[i].m_Graph) {
                    if (m_Unit[i].m_Graph->Takt(cms)) {
                        if (m_ShadowType == SHADOW_STENCIL)
                            RChange(MR_ShadowStencil);
                        else if (m_ShadowType == SHADOW_PROJ_DYNAMIC)
                            RChange(MR_ShadowProjTex);
                    }
                }
            }

            goto endanim;
        }
    }
    if (m_Animation == ANIMATION_ROTATE) {
        for (int i = 0; i < 1; i++) {
            // if (m_Unit[i].m_Type != MRT_CHASSIS) continue;
            if (m_Unit[i].m_Graph) {
                float k = 1;
                if (m_Unit[i].u1.s1.m_Kind == RUK_CHASSIS_TRACK) {
                    k = float(ANIMSPEED_CHAISIS_TRACK) / m_RotSpeed;
                }
                else if (m_Unit[i].u1.s1.m_Kind == RUK_CHASSIS_WHEEL) {
                    k = float(ANIMSPEED_CHAISIS_WHEEL) / m_RotSpeed;
                }
                else if (m_Unit[i].u1.s1.m_Kind == RUK_CHASSIS_PNEUMATIC) {
                    k = float(ANIMSPEED_CHAISIS_PNEUMATIC) / m_RotSpeed;
                }

                if (k > 3)
                    k = 3;
                bool changed = false;
                while (g_MatrixMap->GetTime() > Float2Int(m_Unit[i].u1.s1.m_NextAnimTime)) {
                    changed = true;
                    float add = k * (float)m_Unit[i].m_Graph->NextFrame();
                    m_Unit[i].u1.s1.m_NextAnimTime += std::max(add, 0.1f);
                }
                if (changed) {
                    if (m_ShadowType == SHADOW_STENCIL)
                        RChange(MR_ShadowStencil);
                    else if (m_ShadowType == SHADOW_PROJ_DYNAMIC)
                        RChange(MR_ShadowProjTex);
                }
            }
        }
        SwitchAnimation(ANIMATION_STAY);
        goto endanim;
    }
    if (m_Animation == ANIMATION_MOVE || m_Animation == ANIMATION_BEGINMOVE || m_Animation == ANIMATION_ENDMOVE ||
        m_Animation == ANIMATION_MOVE_BACK || m_Animation == ANIMATION_BEGINMOVE_BACK ||
        m_Animation == ANIMATION_ENDMOVE_BACK) {
        // DCNT("c");
        for (int i = 0; i < 1; i++) {
            // if (m_Unit[i].m_Type != MRT_CHASSIS) continue;
            if (m_Unit[i].m_Graph) {
                float k = 1;
                if (m_Unit[i].u1.s1.m_Kind == RUK_CHASSIS_TRACK) {
                    k = float(ANIMSPEED_CHAISIS_TRACK) / m_Speed;
                }
                else if (m_Unit[i].u1.s1.m_Kind == RUK_CHASSIS_WHEEL) {
                    k = float(ANIMSPEED_CHAISIS_WHEEL) / m_Speed;
                }
                else if (m_Unit[i].u1.s1.m_Kind == RUK_CHASSIS_PNEUMATIC) {
                    k = float(ANIMSPEED_CHAISIS_PNEUMATIC) / m_Speed;
                }

                if (k > 3)
                    k = 3;
                bool changed = false;
                while (g_MatrixMap->GetTime() > Float2Int(m_Unit[i].u1.s1.m_NextAnimTime)) {
                    changed = true;
                    float add = k * (float)m_Unit[i].m_Graph->NextFrame();
                    m_Unit[i].u1.s1.m_NextAnimTime += std::max(add, 0.1f);
                }
                if (changed) {
                    if (m_ShadowType == SHADOW_STENCIL)
                        RChange(MR_ShadowStencil);
                    else if (m_ShadowType == SHADOW_PROJ_DYNAMIC)
                        RChange(MR_ShadowProjTex);
                }
            }
        }
    }
endanim:;
}

void SMatrixRobotUnit::PrepareForDIP(void) {
    DTRACE();
    if (u1.s1.m_ShadowStencil) {
        HDelete(CVOShadowStencil, u1.s1.m_ShadowStencil, g_MatrixHeap);
        u1.s1.m_ShadowStencil = nullptr;
    }
    if (u1.s1.m_WeaponRepairData) {
        u1.s1.m_WeaponRepairData->Release();
        u1.s1.m_WeaponRepairData = nullptr;
    }
#ifdef _DEBUG
    new(&Smoke()) SEffectHandler(DEBUG_CALL_INFO);
#endif
    Smoke().effect = NULL;
}

void CMatrixRobot::ApplyNaklon(const D3DXVECTOR3 &dir) {
    *(D3DXVECTOR3 *)&m_Core->m_Matrix._31;  // = LERPVECTOR(mul, *(D3DXVECTOR3*)&m_Core->m_Matrix._31, up);
    auto tmp = *(D3DXVECTOR3 *)&m_Core->m_Matrix._31 + dir;
    D3DXVec3Normalize((D3DXVECTOR3 *)&m_Core->m_Matrix._31, &tmp);
}

void CMatrixRobot::Takt(int cms) {
    DTRACE();

    if (m_CurrState != ROBOT_CARRYING && m_CurrState != ROBOT_FALLING && m_CurrState != ROBOT_IN_SPAWN &&
        m_CurrState != ROBOT_BASE_CAPTURE) {
        if (FLAG(m_ObjectState, ROBOT_FLAG_ONWATER)) {
            m_KeelWaterCount += m_Speed * KEELWATER_SPAWN_FACTOR * cms;
        }
        else if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_HOVERCRAFT) {
            m_ChassisData.u1.s2.m_DustCount += (m_Speed + 1) * DUST_SPAWN_FACTOR * cms;
        }

        while (m_KeelWaterCount > 1.0) {
            CMatrixEffect::CreateBillboard(NULL, D3DXVECTOR3(m_PosX, m_PosY, WATER_LEVEL), 5, 40, 0xFFFFFFFF,
                                           0x00FFFFFF, 3000, 0, TEXTURE_PATH_KEELWATER, m_Forward);
            m_KeelWaterCount -= 1.0f;
        }
        if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_HOVERCRAFT) {
            while (m_ChassisData.u1.s2.m_DustCount > 1.0) {
                // CDText::T("spd", m_Speed);
                D3DXVECTOR2 spd(m_Velocity.x, m_Velocity.y);
                spd *= float(float(cms) / LOGIC_TAKT_PERIOD);
                CMatrixEffect::CreateDust(NULL, *(D3DXVECTOR2 *)&GetGeoCenter(), spd, 500);
                m_ChassisData.u1.s2.m_DustCount -= 1.0f;
            }
        }
    }

    if (m_Unit[0].u1.s1.m_Kind != RUK_CHASSIS_PNEUMATIC)
        DoAnimation(cms);  // proceed animation only for NON pneumatic chassis

    // RChange(MR_Matrix);
}

bool CMatrixRobot::PickFull(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, float *outt) const {
    DTRACE();
    if (m_CurrState == ROBOT_DIP)
        return false;
    for (int i = 0; i < m_UnitCnt; i++) {
        if (m_Unit[i].m_Graph) {
            if (m_Unit[i].m_Graph->PickFull(m_Unit[i].m_Matrix, m_Unit[i].u1.s1.m_IMatrix, orig, dir, outt))
                return true;
        }
    }
    return false;
}

bool CMatrixRobot::Pick(const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir, float *outt) const {
    DTRACE();
    if (m_CurrState == ROBOT_DIP)
        return false;
    for (int i = 0; i < m_UnitCnt; i++) {
        if (m_Unit[i].m_Graph) {
            if (m_Unit[i].m_Graph->Pick(m_Unit[i].m_Matrix, m_Unit[i].u1.s1.m_IMatrix, orig, dir, outt))
                return true;
        }
    }
    return false;
}

void CMatrixRobot::FreeDynamicResources(void) {
    DTRACE();

    if (m_ShadowProj && (m_ShadowType == SHADOW_PROJ_DYNAMIC)) {
        HDelete(CVOShadowProj, m_ShadowProj, g_MatrixHeap);
        m_ShadowProj = NULL;
        RChange(MR_ShadowProjGeom);
    }
    else if (m_ShadowType == SHADOW_STENCIL) {
        for (int i = 0; i < m_UnitCnt; ++i) {
            if (m_Unit[i].u1.s1.m_ShadowStencil)
                m_Unit[i].u1.s1.m_ShadowStencil->DX_Free();
        }
    }
}

void CMatrixRobot::BeforeDraw(void) {
    DTRACE();

    if (IsMustDie())
        return;

    // RNeed(MR_Matrix|MR_Graph|MR_GraphSort|MR_ShadowStencil|MR_ShadowProj);
    DWORD sh = (g_Config.m_ShowProjShadows ? (MR_ShadowProjGeom | MR_ShadowProjTex) : 0) |
               (g_Config.m_ShowStencilShadows ? MR_ShadowStencil : 0);
    RNeed(MR_Matrix | MR_Graph | sh);

    if (m_ShowHitpointTime > 0 && m_HitPoint > 0 && m_CurrState != ROBOT_DIP) {
        D3DXVECTOR3 pos(*(D3DXVECTOR3 *)&m_Core->m_Matrix._41);
        pos.z += 20;

        if (TRACE_STOP_NONE ==
            g_MatrixMap->Trace(NULL, g_MatrixMap->m_Camera.GetFrustumCenter(), pos, TRACE_LANDSCAPE, NULL)) {
            D3DXVECTOR2 p = g_MatrixMap->m_Camera.Project(pos, g_MatrixMap->GetIdentityMatrix());
            m_PB.Modify(p.x - (PB_ROBOT_WIDTH * 0.5f), p.y - GetRadius(), m_HitPoint * m_MaxHitPointInversed);
        }
    }
    if (g_Config.m_ShowStencilShadows && !IsInterfaceDraw()) {
        for (int i = 0; i < m_UnitCnt; i++) {
            m_Unit[i].m_Graph->BeforeDraw();
            if (m_CurrState != ROBOT_DIP && m_Unit[i].u1.s1.m_ShadowStencil)
                m_Unit[i].u1.s1.m_ShadowStencil->BeforeRender();
        }
    }
    if (m_CurrState != ROBOT_DIP && m_ShadowProj && g_Config.m_ShowProjShadows)
        m_ShadowProj->BeforeRender();

    if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_ANTIGRAVITY) {
        m_ChassisData.u1.s1.m_LStream->BeforeDraw();
    }
}

void CMatrixRobot::Draw(void) {
    DWORD coltex = (DWORD)g_MatrixMap->GetSideColorTexture(m_Side)->Tex();
    // g_D3DD->SetRenderState( D3DRS_NORMALIZENORMALS,  TRUE );

    for (int i = 0; i < 4; i++) {
        ASSERT_DX(g_D3DD->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&g_MatrixMap->m_BiasRobots))));
    }

    if (m_CurrState == ROBOT_DIP) {
        for (int i = 0; i < m_UnitCnt; i++) {
            if (m_Unit[i].u1.s2.m_TTL <= 0)
                continue;
            g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFF808080);
            ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &(m_Unit[i].m_Matrix)));
            if (m_Unit[i].u1.s1.m_Invert) {
                g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
                m_Unit[i].m_Graph->Draw(coltex);
                g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
            }
            else {
                m_Unit[i].m_Graph->Draw(coltex);
            }
        }
    }
    else {
        for (int i = 0; i < m_UnitCnt; i++) {
            ASSERT(m_Unit[i].m_Graph);
            g_D3DD->SetRenderState(D3DRS_TEXTUREFACTOR, m_Core->m_TerainColor);

            ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &(m_Unit[i].m_Matrix)));
            if (m_Unit[i].u1.s1.m_Invert) {
                g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
                m_Unit[i].m_Graph->Draw(coltex);
                g_D3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
            }
            else {
                m_Unit[i].m_Graph->Draw(coltex);
            }
        }
        for (int i = 0; i < m_UnitCnt; i++) {
            if (IsInterfaceDraw()) {
                m_Unit[i].m_Graph->DrawLights(true, m_Unit[i].m_Matrix, &g_MatrixMap->m_Camera.GetDrawNowIView());
            }
            else {
                m_Unit[i].m_Graph->DrawLights(false, m_Unit[i].m_Matrix, NULL);
            }
        }

        //#ifdef _DEBUG
        //
        //        // draw foots
        //
        //        if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_PNEUMATIC && m_Animation == ANIMATION_MOVE)
        //        {
        //
        //            int vof = m_Unit[0].m_Graph->GetVOFrame();
        //            D3DXVECTOR3 p;
        //            p.x = m_Pneumaic[vof].foot.x;
        //            p.y = m_Pneumaic[vof].foot.y;
        //            p.z = 0;
        //
        //            D3DXVec3TransformCoord(&p, &p, &m_Unit[0].m_Matrix);
        //            CHelper::Create(1)->Line(p, p + D3DXVECTOR3(0,0,100));
        //        }
        //
        //#endif
    }
    // g_D3DD->SetRenderState( D3DRS_NORMALIZENORMALS,  FALSE );

    /*
    for (int k=0; k<100; ++k)
    {
        D3DXVECTOR3 d(FSRND(1),FSRND(1),FSRND(1));
        D3DXVec3Normalize(&d, &d);
        CHelper::Create(1,0)->Line(m_GeoCenter, m_GeoCenter + d*m_Radius);
    }
    */

    if (m_CurrState == ROBOT_DIP)
        return;

    if (m_CamDistSq > MAX_EFFECT_DISTANCE_SQ)
        return;

    if (!FLAG(g_MatrixMap->m_Flags, MMFLAG_OBJECTS_DRAWN)) {
        if (IsInterfaceDraw()) {
            for (int i = 0; i < m_UnitCnt; i++) {
                if (m_Unit[i].m_Type == MRT_WEAPON && m_Unit[i].u1.s1.m_Kind == RUK_WEAPON_REPAIR) {
                    if (m_Unit[i].u1.s1.m_WeaponRepairData) {
                        // draw
                        m_Unit[i].u1.s1.m_WeaponRepairData->Draw(true);
                    }
                }
            }
        }
        else {
            int cnt = this->AsRobot()->m_WeaponsCnt;
            for (int i = 0; i < cnt; ++i) {
                SBotWeapon *w = this->AsRobot()->m_Weapons + i;
                w->Draw(this->AsRobot());
            }
        }

        if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_ANTIGRAVITY) {
            if (IsInterfaceDraw()) {
                CVectorObject::DrawEnd();

                m_ChassisData.u1.s1.m_LStream->Draw(true);
                m_ChassisData.u1.s1.m_RStream->Draw(true);

                // CVectorObject::DrawBegin();
            }
            else {
                m_ChassisData.u1.s1.m_LStream->Draw();
                m_ChassisData.u1.s1.m_RStream->Draw();
            }
        }
    }
}

void CMatrixRobot::DrawShadowStencil(void) {
    DTRACE();

    if (m_ShadowType != SHADOW_STENCIL)
        return;

    if (this->AsRobot() == g_MatrixMap->GetPlayerSide()->GetArcadedObject()) {
        if (!g_MatrixMap->m_Camera.IsInFrustum(*(D3DXVECTOR3 *)&m_Unit[1].m_Matrix._41))
            return;
    }

    if (m_CurrState == ROBOT_CARRYING) {
        if (m_PosX < 0 || m_PosY < 0 || m_PosX > (GLOBAL_SCALE * g_MatrixMap->m_Size.x) ||
            m_PosY > (GLOBAL_SCALE * g_MatrixMap->m_Size.y))
            return;
    }

    for (int i = 0; i < m_UnitCnt; i++) {
        if (m_Unit[i].u1.s1.m_ShadowStencil) {
            m_Unit[i].u1.s1.m_ShadowStencil->Render(m_Unit[i].m_Matrix);
        }
    }
}

void CMatrixRobot::DrawShadowProj(void) {
    DTRACE();
    if (!m_ShadowProj)
        return;

    D3DXMATRIX m = g_MatrixMap->GetIdentityMatrix();
    m._41 = m_ShadowProj->GetDX();
    m._42 = m_ShadowProj->GetDY();
    ASSERT_DX(g_D3DD->SetTransform(D3DTS_WORLD, &m));

    m_ShadowProj->Render();
}

bool CMatrixRobot::CalcBounds(D3DXVECTOR3 &minv, D3DXVECTOR3 &maxv) {
    if (m_UnitCnt == 0)
        return true;
    DCP();

    int delta = abs(m_CalcBoundsLastTime - g_MatrixMap->GetTime());
    if (delta < 100) {
        minv = m_CalcBoundMin;
        maxv = m_CalcBoundMax;
        return false;
    }

    DCP();

    RNeed(MR_Matrix | MR_Graph);
    DCP();

    D3DXVECTOR3 bminout, bmaxout;

    minv.x = 1e30f;
    minv.y = 1e30f;
    minv.z = 1e30f;
    maxv.x = -1e30f;
    maxv.y = -1e30f;
    maxv.z = -1e30f;

    for (int u = 0; u < m_UnitCnt; ++u) {
        int cnt = 1;  // m_Unit[u].m_Graph->VO()->GetFramesCnt();
        for (int i = 0; i < cnt; ++i) {
            m_Unit[u].m_Graph->VO()->GetBound(i, m_Unit[u].m_Matrix, bminout, bmaxout);

            minv.x = std::min(minv.x, bminout.x);
            minv.y = std::min(minv.y, bminout.y);
            minv.z = std::min(minv.z, bminout.z);
            maxv.x = std::max(maxv.x, bmaxout.x);
            maxv.y = std::max(maxv.y, bmaxout.y);
            maxv.z = std::max(maxv.z, bmaxout.z);
        }
    }
    DCP();

    m_CalcBoundMin = minv;
    m_CalcBoundMax = maxv;
    m_CalcBoundsLastTime = g_MatrixMap->GetTime();

    DCP();

    return false;

    /*	RNeed(MR_Graph|MR_Matrix);

        SVOFrame *f = m_Unit[0].m_Graph->VO()->FrameGet(0);
        minv.x = f->m_MinX;
        minv.y = f->m_MinY;
        maxv.x = f->m_MaxX;
        maxv.y = f->m_MaxY;

        for (int u = 0; u<m_UnitCnt; ++u)
        {
            int cnt = m_Unit[u].m_Graph->VO()->Header()->m_FrameCnt;
            for (int i = 1; i<cnt; ++i)
            {
                SVOFrame *f = m_Unit[u].m_Graph->VO()->FrameGet(i);

                minv.x = min(minv.x,f->m_MinX);
                minv.y = min(minv.y,f->m_MinY);
                maxv.x = max(maxv.x,f->m_MaxX);
                maxv.y = max(maxv.y,f->m_MaxY);
            }
        }

        return false;*/
}

bool CMatrixRobot::Carry(CMatrixFlyer *cargo, bool quick_connect) {
    DTRACE();

    // if (side_index < 0)
    //{
    //    if (!g_MatrixMap->PlaceIsEmpty(m_Unit[0].m_Kind, 4, int(m_PosX / GLOBAL_SCALE), int(m_PosY / GLOBAL_SCALE)))
    //    {
    //        return false;
    //    }
    //    }

    if (m_CurrState == ROBOT_CARRYING) {
        CMatrixRobot *r = m_CargoFlyer->GetCarryingRobot();
        if (r != this) {
            if (!r->Carry(NULL))
                return false;
        }
        m_CargoFlyer->GetCarryData()->m_Robot = NULL;
        // m_CargoFlyer->SetAlt(FLYER_ALT_EMPTY);
        if (m_CargoFlyer->GetCarryData()->m_RobotElevatorField != NULL) {
#ifdef _DEBUG
            g_MatrixMap->SubEffect(DEBUG_CALL_INFO, m_CargoFlyer->GetCarryData()->m_RobotElevatorField);
#else
            g_MatrixMap->SubEffect(m_CargoFlyer->GetCarryData()->m_RobotElevatorField);
#endif

            m_CargoFlyer->GetCarryData()->m_RobotElevatorField = NULL;
        }
    }
    if (cargo == NULL) {
        // if (m_CurrState == ROBOT_MUST_DIE) return true;
        m_CurrState = ROBOT_FALLING;
        m_Velocity = D3DXVECTOR3(0, 0, 0);
        m_FallingSpeed = 0;
        JoinToGroup();
        return true;
    }
    if (cargo->CarryingRobot()) {
        cargo->GetCarryingRobot()->Carry(NULL);
    }

    m_CargoFlyer = cargo;
    m_CurrState = ROBOT_CARRYING;
    SwitchAnimation(ANIMATION_OFF);

    cargo->GetCarryData()->m_Robot = this;
    cargo->GetCarryData()->m_RobotForward = m_Forward;
    D3DXVec3Normalize(&cargo->GetCarryData()->m_RobotUp, (D3DXVECTOR3 *)&m_Core->m_Matrix._31);
    cargo->GetCarryData()->m_RobotUpBack = D3DXVECTOR3(0, 0, 0);

    *((D3DXVECTOR2 *)&cargo->GetCarryData()->m_RobotForward) =
            RotatePoint(*(D3DXVECTOR2 *)&m_Forward, -cargo->GetAngle());
    cargo->GetCarryData()->m_RobotForward.z = m_Forward.z;

    cargo->GetCarryData()->m_RobotAngle = cargo->GetAngle();
    // cargo->SetAlt(FLYER_ALT_CARRYING);
    cargo->GetCarryData()->m_RobotElevatorField = NULL;
    // cargo->GetCarryData()->m_RobotQuickConnect = quick_connect;
    cargo->GetCarryData()->m_RobotMassFactor = quick_connect ? 1.0f : 0;

    UnjoinGroup();
    ClearSelection();
    return true;
}

void CMatrixRobot::ClearSelection(void) {
    if (g_MatrixMap->GetPlayerSide()->m_CurrSel == ROBOT_SELECTED &&
        g_MatrixMap->GetPlayerSide()->m_ActiveObject == this)
        g_MatrixMap->GetPlayerSide()->Select(NOTHING, NULL);
}

void CMatrixRobot::SwitchAnimation(EAnimation a) {
    if (a != m_Animation) {
        RChange(MR_Matrix | MR_ShadowStencil | MR_ShadowProjTex);
    }

    if (a == ANIMATION_ROTATE && (m_Speed != 0))
        return;
    if (a != ANIMATION_OFF) {
        if ((m_CurrState == ROBOT_DIP) || (m_CurrState == ROBOT_CARRYING) || IsShorted())
            return;
        if (a != m_Animation) {
            for (int i = 0; i < m_UnitCnt; ++i) {
                m_Unit[i].u1.s1.m_NextAnimTime = float(g_MatrixMap->GetTime());
            }
        }
    }
    if (a == ANIMATION_MOVE) {
        RESETFLAG(m_ObjectState, ROBOT_FLAG_MOVING_BACK);

        if (m_Animation == ANIMATION_STAY || m_Animation == ANIMATION_ENDMOVE || m_Animation == ANIMATION_OFF ||
            m_Animation == ANIMATION_ROTATE || m_Animation == ANIMATION_MOVE_BACK ||
            m_Animation == ANIMATION_ENDMOVE_BACK || m_Animation == ANIMATION_BEGINMOVE_BACK) {
            m_Animation = ANIMATION_BEGINMOVE;
            if (m_Unit[0].m_Graph->SetAnimByName(ANIMATION_NAME_BEGIN_MOVE, 0)) {
                m_Animation = ANIMATION_MOVE;
                m_Unit[0].m_Graph->SetAnimByName(ANIMATION_NAME_MOVE);

                if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_PNEUMATIC)
                    FirstLinkPneumatic();
            }
        }
        else if (m_Animation == ANIMATION_BEGINMOVE) {
            if (m_Unit[0].m_Graph->IsAnimEnd()) {
                m_Animation = ANIMATION_MOVE;
                m_Unit[0].m_Graph->SetAnimByName(ANIMATION_NAME_MOVE);

                if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_PNEUMATIC)
                    FirstLinkPneumatic();
            }
        }
    }
    else if (a == ANIMATION_MOVE_BACK) {
        if (m_Animation == ANIMATION_STAY || m_Animation == ANIMATION_ENDMOVE || m_Animation == ANIMATION_OFF ||
            m_Animation == ANIMATION_ROTATE || m_Animation == ANIMATION_BEGINMOVE ||
            m_Animation == ANIMATION_ENDMOVE_BACK || m_Animation == ANIMATION_MOVE) {
            m_Animation = ANIMATION_BEGINMOVE_BACK;
            if (m_Unit[0].m_Graph->SetAnimByName(ANIMATION_NAME_BEGIN_MOVE_BACK, 0)) {
                m_Animation = ANIMATION_MOVE_BACK;
                m_Unit[0].m_Graph->SetAnimByName(ANIMATION_NAME_MOVE_BACK);
                SETFLAG(m_ObjectState, ROBOT_FLAG_MOVING_BACK);

                if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_PNEUMATIC)
                    FirstLinkPneumatic();
            }
        }
        else if (m_Animation == ANIMATION_BEGINMOVE_BACK) {
            if (m_Unit[0].m_Graph->IsAnimEnd()) {
                m_Animation = ANIMATION_MOVE_BACK;
                m_Unit[0].m_Graph->SetAnimByName(ANIMATION_NAME_MOVE_BACK);
                SETFLAG(m_ObjectState, ROBOT_FLAG_MOVING_BACK);

                if (m_Unit[0].u1.s1.m_Kind == RUK_CHASSIS_PNEUMATIC)
                    FirstLinkPneumatic();
            }
        }
    }
    else if (a == ANIMATION_STAY) {
        RESETFLAG(m_ObjectState, ROBOT_FLAG_MOVING_BACK);
        if (m_Animation == ANIMATION_MOVE || m_Animation == ANIMATION_BEGINMOVE) {
            m_Animation = ANIMATION_ENDMOVE;
            if (m_Unit[0].m_Graph->SetAnimByName(ANIMATION_NAME_END_MOVE, 0)) {
                m_Animation = ANIMATION_STAY;
                m_Unit[0].m_Graph->SetAnimByName(ANIMATION_NAME_STAY);
            }
        }
        else if (m_Animation == ANIMATION_ENDMOVE) {
            if (m_Unit[0].m_Graph->IsAnimEnd()) {
                m_Animation = ANIMATION_STAY;
                m_Unit[0].m_Graph->SetAnimByName(ANIMATION_NAME_STAY);
            }
        }
        else if (m_Animation == ANIMATION_MOVE_BACK || m_Animation == ANIMATION_BEGINMOVE_BACK) {
            m_Animation = ANIMATION_ENDMOVE_BACK;
            if (m_Unit[0].m_Graph->SetAnimByName(ANIMATION_NAME_END_MOVE_BACK, 0)) {
                m_Animation = ANIMATION_STAY;
                m_Unit[0].m_Graph->SetAnimByName(ANIMATION_NAME_STAY);
            }
        }
        else if (m_Animation == ANIMATION_ENDMOVE_BACK) {
            if (m_Unit[0].m_Graph->IsAnimEnd()) {
                m_Animation = ANIMATION_STAY;
                m_Unit[0].m_Graph->SetAnimByName(ANIMATION_NAME_STAY);
            }
        }
        else if (m_Animation == ANIMATION_ROTATE || m_Animation == ANIMATION_OFF) {
            m_Unit[0].m_Graph->SetAnimByName(ANIMATION_NAME_STAY);
            m_Animation = ANIMATION_STAY;
        }
    }
    else if (a == ANIMATION_ROTATE) {
        RESETFLAG(m_ObjectState, ROBOT_FLAG_MOVING_BACK);
        if (m_Animation != ANIMATION_ROTATE) {
            m_Unit[0].m_Graph->SetAnimByName(ANIMATION_NAME_ROTATE);
            m_Animation = ANIMATION_ROTATE;
        }
    }
    else {
        m_Animation = a;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

SPneumaticData *CMatrixRobot::m_Pneumaic;

int g_fcnt;

void CMatrixRobot::LinkPneumatic(void) {
    if (!FLAG(m_ObjectState, ROBOT_FLAG_LINKED))
        return;  // never linked
    if (!m_Unit[0].m_Graph->IsAnim(ANIMATION_NAME_MOVE) && !m_Unit[0].m_Graph->IsAnim(ANIMATION_NAME_MOVE_BACK))
        return;

    int vof = m_Unit[0].m_Graph->GetFrame();
    int vof_add = 0;

    if (FLAG(m_ObjectState, ROBOT_FLAG_MOVING_BACK)) {
        // reverse linking
        vof_add = g_fcnt;
    }

    if (m_ChassisData.u1.s4.m_LinkPrevFrame == vof)
        goto correction;

    // bool newlink = false;
    while (m_ChassisData.u1.s4.m_LinkPrevFrame != vof) {
        if (m_Pneumaic[m_ChassisData.u1.s4.m_LinkPrevFrame + vof_add].newlink) {
            D3DXVec2TransformCoord(&m_ChassisData.u1.s4.m_LinkPos,
                                   &m_Pneumaic[m_ChassisData.u1.s4.m_LinkPrevFrame + vof_add].other_foot, &m_Core->m_Matrix);

            int x = TruncFloat(m_ChassisData.u1.s4.m_LinkPos.x * INVERT(GLOBAL_SCALE));
            int y = TruncFloat(m_ChassisData.u1.s4.m_LinkPos.y * INVERT(GLOBAL_SCALE));
            SMatrixMapUnit *mu = g_MatrixMap->UnitGetTest(x, y);
            if (mu && mu->IsLand() && !mu->IsBridge()) {
                float ang = (float)atan2(-m_Forward.x, m_Forward.y);
                CMatrixEffect::CreateLandscapeSpot(NULL, m_ChassisData.u1.s4.m_LinkPos, ang, 1.0f, SPOT_SOLE_PNEUMATIC);
            }
        }
        ++m_ChassisData.u1.s4.m_LinkPrevFrame;
        if (m_ChassisData.u1.s4.m_LinkPrevFrame >= g_fcnt)
            m_ChassisData.u1.s4.m_LinkPrevFrame = 0;
    }

correction:

    if (m_CurrState != ROBOT_CARRYING && !FLAG(m_ObjectState, ROBOT_FLAG_COLLISION)) {
        D3DXMATRIX m;
        D3DXVECTOR3 tmp_forward, up, side;

        // D3DXVec3Normalize(&up, (D3DXVECTOR3*)&m_Unit[0].m_Matrix._31);
        D3DXVec3Normalize(&up, (D3DXVECTOR3 *)&m_Core->m_Matrix._31);
        // g_MatrixMap->GetNormal(&up, m_PosX,m_PosY);
        D3DXVec3Cross(&side, &m_Forward, &up);
        D3DXVec3Normalize(&side, &side);
        D3DXVec3Cross(&tmp_forward, &up, &side);

        m._11 = side.x;
        m._12 = side.y;
        m._13 = side.z;
        m._14 = 0;
        m._21 = tmp_forward.x;
        m._22 = tmp_forward.y;
        m._23 = tmp_forward.z;
        m._24 = 0;
        m._31 = up.x;
        m._32 = up.y;
        m._33 = up.z;
        m._34 = 0;
        m._41 = m_PosX;
        m._42 = m_PosY;
        m._43 = Z_From_Pos();
        m._44 = 1;

        D3DXVECTOR2 curlink;
        D3DXVec2TransformCoord(&curlink, &m_Pneumaic[vof + vof_add].foot, &m);

        curlink -= m_ChassisData.u1.s4.m_LinkPos;

        m_PosX -= curlink.x;
        m_PosY -= curlink.y;
    }
}

void CMatrixRobot::FirstLinkPneumatic(void) {
    if (!m_Unit[0].m_Graph->IsAnim(ANIMATION_NAME_MOVE) && !m_Unit[0].m_Graph->IsAnim(ANIMATION_NAME_MOVE_BACK))
        return;

    SETFLAG(m_ObjectState, ROBOT_FLAG_LINKED);

    D3DXMATRIX m;
    D3DXVECTOR3 tmp_forward, up, side;

    // D3DXVec3Normalize(&up, (D3DXVECTOR3*)&m_Unit[0].m_Matrix._31);
    D3DXVec3Normalize(&up, (D3DXVECTOR3 *)&m_Core->m_Matrix._31);
    // g_MatrixMap->GetNormal(&up, m_PosX,m_PosY);
    D3DXVec3Cross(&side, &m_Forward, &up);
    D3DXVec3Normalize(&side, &side);
    D3DXVec3Cross(&tmp_forward, &up, &side);

    m._11 = side.x;
    m._12 = side.y;
    m._13 = side.z;
    m._14 = 0;
    m._21 = tmp_forward.x;
    m._22 = tmp_forward.y;
    m._23 = tmp_forward.z;
    m._24 = 0;
    m._31 = up.x;
    m._32 = up.y;
    m._33 = up.z;
    m._34 = 0;
    m._41 = m_PosX;
    m._42 = m_PosY;
    m._43 = Z_From_Pos();
    m._44 = 1;

    int vof = m_Unit[0].m_Graph->GetFrame();
    int vof_add = 0;

    if (FLAG(m_ObjectState, ROBOT_FLAG_MOVING_BACK)) {
        // reverse linking
        vof_add = g_fcnt;
    }

    D3DXVec2TransformCoord(&m_ChassisData.u1.s4.m_LinkPos, &m_Pneumaic[vof + vof_add].foot, &m);
    m_ChassisData.u1.s4.m_LinkPrevFrame = vof;
}

void CMatrixRobot::BuildPneumaticData(CVectorObject *vo) {
    int moveanim = vo->GetAnimByName(ANIMATION_NAME_MOVE);
    g_fcnt = vo->GetAnimFramesCount(moveanim);

    m_Pneumaic = (SPneumaticData *)HAlloc(sizeof(SPneumaticData) * g_fcnt * 2, g_MatrixHeap);

    {
        int leftf = -1;
        int ritef = -1;

        float maxl = -100;
        float maxr = -100;

        for (int i = 0; i < g_fcnt; ++i) {
            int frame = vo->GetAnimFrameIndex(moveanim, i);

            const D3DXMATRIX *ml = vo->GetMatrixById(frame, 2);
            const D3DXMATRIX *mr = vo->GetMatrixById(frame, 3);

            if (ml->_42 > maxl) {
                maxl = ml->_42;
                leftf = i;
            }

            if (mr->_42 > maxr) {
                maxr = mr->_42;
                ritef = i;
            }
        }

        int prev = (leftf < ritef) ? 3 : 2;
        for (int i = 0; i < g_fcnt; ++i) {
            // if (i==fcnt)
            //{
            //    // check first frame of animation (may be it looped with last frame)
            //    fcnt = 0;
            //    i = 0;
            //}

            int frame = vo->GetAnimFrameIndex(moveanim, i);

            const D3DXMATRIX *ml = vo->GetMatrixById(frame, 2);
            const D3DXMATRIX *mr = vo->GetMatrixById(frame, 3);

            int newv = prev;

            if (i == leftf)
                newv = 2;
            else if (i == ritef)
                newv = 3;

            if (newv == 2) {
                m_Pneumaic[i].newlink = prev != newv;
                prev = newv;
                if (m_Pneumaic[i].newlink) {
                    m_Pneumaic[i].other_foot.x = ml->_41;
                    m_Pneumaic[i].other_foot.y = ml->_42;

                    m_Pneumaic[i].foot.x = mr->_41;
                    m_Pneumaic[i].foot.y = mr->_42;
                }
                else {
                    m_Pneumaic[i].foot.x = ml->_41;
                    m_Pneumaic[i].foot.y = ml->_42;
                }
            }
            else {
                m_Pneumaic[i].newlink = prev != 3;
                prev = newv;
                if (m_Pneumaic[i].newlink) {
                    m_Pneumaic[i].other_foot.x = mr->_41;
                    m_Pneumaic[i].other_foot.y = mr->_42;

                    m_Pneumaic[i].foot.x = ml->_41;
                    m_Pneumaic[i].foot.y = ml->_42;
                }
                else {
                    m_Pneumaic[i].foot.x = mr->_41;
                    m_Pneumaic[i].foot.y = mr->_42;
                }
            }
        }
    }

    // back
    {
        int leftf = -1;
        int ritef = -1;

        float maxl = 100;
        float maxr = 100;

        moveanim = vo->GetAnimByName(ANIMATION_NAME_MOVE_BACK);

        for (int i = 0; i < g_fcnt; ++i) {
            int frame = vo->GetAnimFrameIndex(moveanim, i);

            const D3DXMATRIX *ml = vo->GetMatrixById(frame, 2);
            const D3DXMATRIX *mr = vo->GetMatrixById(frame, 3);

            if (ml->_42 < maxl) {
                maxl = ml->_42;
                leftf = i;
            }

            if (mr->_42 < maxr) {
                maxr = mr->_42;
                ritef = i;
            }
        }

        int prev = (leftf < ritef) ? 3 : 2;
        for (int i = 0; i < g_fcnt; ++i) {
            // if (i==fcnt)
            //{
            //    // check first frame of animation (may be it looped with last frame)
            //    fcnt = 0;
            //    i = 0;
            //}

            int frame = vo->GetAnimFrameIndex(moveanim, i);

            const D3DXMATRIX *ml = vo->GetMatrixById(frame, 2);
            const D3DXMATRIX *mr = vo->GetMatrixById(frame, 3);

            int newv = prev;

            if (i == leftf)
                newv = 2;
            else if (i == ritef)
                newv = 3;

            if (newv == 2) {
                m_Pneumaic[i + g_fcnt].newlink = prev != newv;
                prev = newv;
                if (m_Pneumaic[i + g_fcnt].newlink) {
                    m_Pneumaic[i + g_fcnt].other_foot.x = ml->_41;
                    m_Pneumaic[i + g_fcnt].other_foot.y = ml->_42;

                    m_Pneumaic[i + g_fcnt].foot.x = mr->_41;
                    m_Pneumaic[i + g_fcnt].foot.y = mr->_42;
                }
                else {
                    m_Pneumaic[i + g_fcnt].foot.x = ml->_41;
                    m_Pneumaic[i + g_fcnt].foot.y = ml->_42;
                }
            }
            else {
                m_Pneumaic[i + g_fcnt].newlink = prev != 3;
                prev = newv;
                if (m_Pneumaic[i + g_fcnt].newlink) {
                    m_Pneumaic[i + g_fcnt].other_foot.x = mr->_41;
                    m_Pneumaic[i + g_fcnt].other_foot.y = mr->_42;

                    m_Pneumaic[i + g_fcnt].foot.x = ml->_41;
                    m_Pneumaic[i + g_fcnt].foot.y = ml->_42;
                }
                else {
                    m_Pneumaic[i + g_fcnt].foot.x = mr->_41;
                    m_Pneumaic[i + g_fcnt].foot.y = mr->_42;
                }
            }
        }
    };
}
void CMatrixRobot::DestroyPneumaticData(void) {
    if (m_Pneumaic)
        HFree(m_Pneumaic, g_MatrixHeap);
}

bool CMatrixRobot::InRect(const CRect &rect) const {
    if (m_CurrState == ROBOT_DIP)
        return false;

    D3DXVECTOR3 dir;
    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.left, rect.top), dir);
    if (Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, NULL))
        return true;

    D3DXMATRIX s, t;
    SEVH_data d;

    t = g_MatrixMap->m_Camera.GetViewMatrix() * g_MatrixMap->m_Camera.GetProjMatrix();
    D3DXMatrixScaling(&s, float(g_ScreenX / 2), float(-g_ScreenY / 2), 1);
    s._41 = s._11;
    s._42 = float(g_ScreenY / 2);
    t *= s;
    d.rect = &rect;

    for (int i = 0; i < m_UnitCnt; ++i) {
        if (m_Unit[i].m_Graph) {
            d.found = false;
            d.m = m_Unit[i].m_Matrix * t;
            m_Unit[i].m_Graph->EnumFrameVerts(EnumVertsHandler, (DWORD)&d);
            if (d.found)
                return true;
        }
    }

    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.left, rect.bottom), dir);
    if (Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, NULL))
        return true;

    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.right, rect.top), dir);
    if (Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, NULL))
        return true;

    g_MatrixMap->m_Camera.CalcPickVector(CPoint(rect.right, rect.bottom), dir);
    if (Pick(g_MatrixMap->m_Camera.GetFrustumCenter(), dir, NULL))
        return true;

    return false;
}
