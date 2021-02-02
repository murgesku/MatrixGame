// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_CAMERA_INCLUDE
#define MATRIX_CAMERA_INCLUDE

#include <math.h>
#include "math3d.hpp"
#include "MatrixConfig.hpp"

enum EFrustumPlane
{
	FPLANE_LEFT,
	FPLANE_RIGHT,
	FPLANE_BOTTOM,
	FPLANE_TOP
};

#define CAM_ACTION_MOVE_LEFT    SETBIT(0)
#define CAM_ACTION_MOVE_RIGHT   SETBIT(1)
#define CAM_ACTION_MOVE_UP      SETBIT(2)
#define CAM_ACTION_MOVE_DOWN    SETBIT(3)
#define CAM_ACTION_ROT_LEFT     SETBIT(4)
#define CAM_ACTION_ROT_RIGHT    SETBIT(5)
#define CAM_ACTION_ROT_UP       SETBIT(6)
#define CAM_ACTION_ROT_DOWN     SETBIT(7)

#define CAM_LINK_POINT_CHANGED  SETBIT(8)   // camera changes its link point
//#define CAM_PREVARCADED         SETBIT(9)
//#define CAM_RESTORE             SETBIT(10)
#define CAM_XY_LERP_OFF         SETBIT(11) // xy pos restore
//#define CAM_LANDRELATIVE        SETBIT(12)

#define CAM_SELECTED_TARGET      SETBIT(13)
#define CAM_NEED2END_DIALOG_MODE SETBIT(14)

//#define CAM_LINK_DIST           0.2f
//#define CAM_UNLINK_DIST         0.02f

//#define CAM_MOVE_TIME           1
//#define CAM_ROT_SPEED           0.001f
//#define CAM_MOVE_SPEED          0.35f

//#define CAM_BOT_POINT_DIST      40
//#define CAM_BOT_WEIGHT          1
//#define CAM_BOT_MAX_SPEED       20
//#define CAM_BOT_MAX_FORCE       8
//#define CAM_BOT_HEIGHT          55

#define CAM_HFOV                GRAD2RAD(g_CamFieldOfView) //D3DX_PI/3/*6*/

//#define MAX_VIEW_DISTANCE       4000
//#define MAX_VIEW_DISTANCE_SQ    (MAX_VIEW_DISTANCE * MAX_VIEW_DISTANCE)
//extern float MAX_VIEW_DISTANCE;

void SetMaxCameraDistance(float perc);

//#define FOG_NEAR_K              0.5 //Точка удаления от камеры, в которой начинается отрисовка разреженного тумана
//#define FOG_FAR_K               0.7 //Точка удаления от камеры, в которой начинается отрисовка сплошного тумана


//#define MAX_TIME_ON_TRAJ        8000
#define MAX_WAR_PAIRS           16
#define RECALC_DEST_ANGZ_PERIOD 150
#define WAR_PAIR_TTL            1000
#define OBZOR_TIME              12500
#define PI                      3.141592653589793

//const static double pi = 3.141592653589793;

struct SObjectCore;

enum ETrajectoryStatus
{
    TRJS_JUST_STAYING,
    TRJS_MOVING_TO,
    TRJS_MOVING_TO_BIGBOOM,
    TRJS_FLY_AROUND,
};

struct SAutoFlyData
{
    struct          SWarPair
    {
        SObjectCore     *target;
        SObjectCore     *attacker;
        float           ttl;
    };

    ETrajectoryStatus m_Status;

    //CTrajectory    *m_Traj;

    D3DXVECTOR3     m_Cur;

    float           m_CurDist;
    float           m_CurAngZ;
    float           m_CurAngX;

    //D3DXVECTOR3     m_NewActual;
    D3DXVECTOR3     m_New;
    float           m_NewDist;
    float           m_NewAngZ;
    float           m_NewAngX;

    float           m_RotSpeedZ;
    float           m_RotSpeedX;
    
    float           m_Speed;
    //float           m_OnTrajectorySpeedNeed;
    //float           m_OnTrajectorySpeedCurrent;

    SWarPair        m_WarPairCurrent;
    SWarPair        m_WarPairs[MAX_WAR_PAIRS];
    int             m_WarPairsCnt;

    int             m_LastStatTime;
    int             m_LastObzorTime;


    //float           m_CalcDestTime;
    float           m_CalcDestAngZTime;
    //float           m_OnTrajTime;
    float           m_ObzorTime;
    
    float           m_BeforeObzorAngX;

    //void            BuildTrajectory(ETrajectoryStatus s);
    void            Takt(float ms);

    void            FindAutoFlyTarget(void);

    void            KillTrajectory(void);
    void            Stat(void);
    void            Release(void);
    void            AddWarPair(CMatrixMapStatic *tgt, CMatrixMapStatic *attacker);

};

class CMatrixCamera : public CMain
{

    D3DVIEWPORT9    m_ViewPort;
	D3DXMATRIX      m_MatProj;
	D3DXMATRIX      m_MatView;
    D3DXMATRIX      m_MatViewInversed;

    D3DXMATRIX      m_DN_iview;
    D3DXVECTOR3     m_DN_fc;

    D3DXVECTOR3     m_FrustumCenter;
    D3DXVECTOR3     m_FrustumDirLT;
    D3DXVECTOR3     m_FrustumDirLB;
    D3DXVECTOR3     m_FrustumDirRT;
    D3DXVECTOR3     m_FrustumDirRB;

    SPlane          m_FrustPlaneL;
    SPlane          m_FrustPlaneR;
    SPlane          m_FrustPlaneT;
    SPlane          m_FrustPlaneB;

    float           _mp_11_inversed;
    float           _mp_22_inversed;

    float           _res_x_inversed;
    float           _res_y_inversed;

    DWORD           m_Flags;

    float           m_AngleX;
    float           m_AngleZ;
    float           m_AngleParam[CAMERA_PARAM_CNT];
    int             m_ModeIndex;
    
    // current cam values
    D3DXVECTOR3     m_LinkPoint;   // real link point (do not modify, it is calculated automaticaly)

    float           m_Dist;
	float			m_DistParam[CAMERA_PARAM_CNT];

    D3DXVECTOR2     m_XY_Strategy;
    //Содержит угол в радианах, запоминая (суммируя) каждый сделанный камерой по часовой (плюсует), либо против часовой (вычитает) стрелке оборот
    //Запоминает горизонтальный угол поворота камеры в стратегическом режиме (стартовый угол 0 находится на юге карты)
    float m_Ang_Strategy;


    SAutoFlyData    *m_AFD;

   

 //   D3DXVECTOR2 m_XY_Strategy;  // used in permode moving

 //   D3DXVECTOR3 m_Target;
 //   D3DXVECTOR3 m_TargetDisp;

 //   float       m_NextMoveTime;

 //   float m_LandRelativeZ;


 //   float m_DistArcadedStart;
 //   float m_AngleArcadedStart;


    __forceinline float LerpDist(int index = -1)
    {
        if(index < 0)
        {
            index = m_ModeIndex;
        }
        SCamParam * cp = g_Config.m_CamParams+index;
        return LERPFLOAT(m_DistParam[m_ModeIndex], cp->m_CamDistMin, cp->m_CamDistMax);
    }

    __forceinline float LerpAng(int index = -1)
    {
        if(index < 0)
        {
            index = m_ModeIndex;
        }
        SCamParam* cp = g_Config.m_CamParams + index;
        return LERPFLOAT(m_AngleParam[m_ModeIndex], cp->m_CamRotAngleMin, cp->m_CamRotAngleMax);
    }

    void CalcLinkPoint(D3DXVECTOR3 &lp, float &angz);

    friend struct SAutoFlyData;

public:
    //Для вычисления смещения камеры от центра точки центровки с учётом её текущего горизонтального угла
    //dist - дальность необходимого смещения
    //par_type - смещение по какой координате вернуть: 0 - X, !0 - Y
    float CamAngleToCoordOffset(float dist, int par_type)
    {
        //Получаем радианы с учётом только одного полного оборота камеры по часовой стрелке
        //Долго отлаживал это говно, только чтобы убедиться, что вариант с переводом в градусы оптимальнее и удобнее (Klaxons)
        /*
        double ang = m_Ang_Strategy;
        if(abs(ang) > 6.28319)
        {
            ang /= 6.28319;
            ang = std::modf(ang, &ang);
            ang *= 6.28319;
        }
        if(ang < 0) ang += 6.28319;
        */

        //Для перевода радиан в градусы
        double ang = m_Ang_Strategy / 0.0174533;
        ang = int(ang) % 360;
        if(ang < 0) ang += 360.0;
        //Обратите внимание, переводим угол из дабологики (где начало окружности находится на юге и продвигается по часовой стрелке) в нормальную систему
        if(ang <= 270) ang -= abs(270);
        else ang = 360 - ang + 270;

        //Вычисляем смещение по X (смещения вычисляем от нуля)
        if(par_type) return sin(ang * PI / 180) * dist;
        //Вычисляем смещение по Y
        else return cos(ang * PI / 180) * dist;
    }

    CMatrixCamera(void);
    ~CMatrixCamera(void);

    __forceinline float   GetResXInversed(void) const {return _res_x_inversed;}
    __forceinline float   GetResYInversed(void) const {return _res_y_inversed;}

    __forceinline void    SetAngleParam(float p)      { m_AngleParam[m_ModeIndex] = p;}

    __forceinline float   GetAngleX(void) const       {return m_AngleX;}
    __forceinline float   GetAngleZ(void) const       {return m_AngleZ;}
    __forceinline void    SetAngleZ(float angle)      { m_AngleZ = angle; }
    //__forceinline void    RotateZ(float da)          { m_AngleZ += da; }
    //__forceinline void    RotateX(float angle)        { m_AngleX += angle; }

    void RotateByMouse(int dx, int dy)
    {
        if(m_ModeIndex == CAMERA_STRATEGY)
        {
            m_Ang_Strategy += g_Config.m_CamParams[m_ModeIndex].m_CamRotSpeedZ * dx * 10;

        }
        m_AngleParam[m_ModeIndex] -= g_Config.m_CamParams[m_ModeIndex].m_CamRotSpeedX * dy * 5;
        if(m_AngleParam[m_ModeIndex] > 1.0f)  m_AngleParam[m_ModeIndex] = 1.0f;
        if(m_AngleParam[m_ModeIndex] < 0.0f)  m_AngleParam[m_ModeIndex] = 0.0f;
    }

    void InitStrategyAngle(float ang)
    {
        m_Ang_Strategy = (float)AngleNorm(g_Config.m_CamBaseAngleZ + ang);
        m_AngleZ = m_Ang_Strategy;
    }

    void CalcSkyMatrix(D3DXMATRIX &m);

    __forceinline void SetXYStrategy(const D3DXVECTOR2 &pos)
    {
        SETFLAG(m_Flags, CAM_XY_LERP_OFF);
        m_XY_Strategy = pos;
    }

    __forceinline const D3DXVECTOR2 &    GetXYStrategy(void) const {return m_XY_Strategy;}
	//void    SetTarget(const D3DXVECTOR3 & pos)         { RESETFLAG(m_Flags, CAM_RESTOREXY); m_Target = pos;}
	const   D3DXVECTOR3 &GetLinkPoint(void) const                 { return m_LinkPoint; }
 //   float GetZRel(void) const             { return m_LandRelativeZ; }
 //   void  SetZRel(float z)                { m_LandRelativeZ = z; }

    __forceinline const D3DXVECTOR3 &GetDir(void) const {return *(D3DXVECTOR3 *)&m_MatViewInversed._31;}
    __forceinline const D3DXVECTOR3 &GetRight(void) const {return *(D3DXVECTOR3 *)&m_MatViewInversed._11;}
    __forceinline const D3DXVECTOR3 &GetUp(void) const {return *(D3DXVECTOR3 *)&m_MatViewInversed._21;}


    void    RotLeft(void) {SETFLAG(m_Flags, CAM_ACTION_ROT_LEFT);}
    void    RotRight(void) {SETFLAG(m_Flags, CAM_ACTION_ROT_RIGHT);}
    void    RotUp(void) {SETFLAG(m_Flags, CAM_ACTION_ROT_UP);}
    void    RotDown(void) {SETFLAG(m_Flags, CAM_ACTION_ROT_DOWN);}

	void	ZoomOutStep(void)
	{
		m_DistParam[m_ModeIndex] += g_Config.m_CamParams[m_ModeIndex].m_CamMouseWheelStep * 4.5f;//1.5f;
		if(m_DistParam[m_ModeIndex] > 4.0f)  m_DistParam[m_ModeIndex] = 4.0f;
		if(m_DistParam[m_ModeIndex] < 0.25f)  m_DistParam[m_ModeIndex] = 0.25f;
		//if (m_DistParam[m_ModeIndex] < 0.0f)  m_DistParam[m_ModeIndex] = 0.0f;
	}

	void	ZoomInStep(void)
	{
		m_DistParam[m_ModeIndex] -= g_Config.m_CamParams[m_ModeIndex].m_CamMouseWheelStep * 4.5f;//1.5f;
		if(m_DistParam[m_ModeIndex] > 4.0f)  m_DistParam[m_ModeIndex] = 4.0f;
		if(m_DistParam[m_ModeIndex] < 0.25f)  m_DistParam[m_ModeIndex] = 0.25f;
		//if (m_DistParam[m_ModeIndex] < 0.0f)  m_DistParam[m_ModeIndex] = 0.0f;
	}

    void    RotUpStep(void)
    {
        m_AngleParam[m_ModeIndex] += g_Config.m_CamParams[m_ModeIndex].m_CamMouseWheelStep;
        if(m_AngleParam[m_ModeIndex] > 1.0f)  m_AngleParam[m_ModeIndex] = 1.0f;
        if(m_AngleParam[m_ModeIndex] < 0.0f)  m_AngleParam[m_ModeIndex] = 0.0f;
    }
    void    RotDownStep(void)
    {
        m_AngleParam[m_ModeIndex] -= g_Config.m_CamParams[m_ModeIndex].m_CamMouseWheelStep;
        if(m_AngleParam[m_ModeIndex] > 1.0f)  m_AngleParam[m_ModeIndex] = 1.0f;
        if(m_AngleParam[m_ModeIndex] < 0.0f)  m_AngleParam[m_ModeIndex] = 0.0f;
    }

    void    MoveLeft(void) {SETFLAG(m_Flags, CAM_ACTION_MOVE_LEFT);}
    void    MoveRight(void) {SETFLAG(m_Flags, CAM_ACTION_MOVE_RIGHT);}
    void    MoveUp(void) {SETFLAG(m_Flags, CAM_ACTION_MOVE_UP);}
    void    MoveDown(void) {SETFLAG(m_Flags, CAM_ACTION_MOVE_DOWN);}

    void    ResetAngles(void);

    void CalcPickVector(const CPoint &p, D3DXVECTOR3 &vdir) const ;
    //void MoveCamera( const D3DXVECTOR3 & d) { m_Target += d; }

    void Stat(void)
    {
        if(m_AFD) m_AFD->Stat();
    }

    //bool IsLinked(void) const {return FLAG(m_Flags, CAM_LINKED);}

    void    AddWarPair(CMatrixMapStatic *tgt, CMatrixMapStatic *attacker)
    {
        DTRACE();
        if(m_AFD) m_AFD->AddWarPair(tgt, attacker);
    }

    const D3DXMATRIX & GetViewMatrix(void) const                    { return m_MatView; }
    const D3DXMATRIX & GetProjMatrix(void) const                    { return m_MatProj; }
	const D3DXMATRIX & GetViewInversedMatrix(void) const            { return m_MatViewInversed; }
    const D3DXVECTOR3 & GetFrustumCenter(void) const                { return m_FrustumCenter; }
    const D3DXVECTOR3 & GetFrustumLT(void) const                    { return m_FrustumDirLT;  }
    const D3DXVECTOR3 & GetFrustumLB(void) const                    { return m_FrustumDirLB;  }
    const D3DXVECTOR3 & GetFrustumRT(void) const                    { return m_FrustumDirRT;  }
    const D3DXVECTOR3 & GetFrustumRB(void) const                    { return m_FrustumDirRB;  }
    const SPlane      & GetFrustPlaneL(void) const                  { return m_FrustPlaneL;  }
    const SPlane      & GetFrustPlaneR(void) const                  { return m_FrustPlaneR;  }
    const SPlane      & GetFrustPlaneT(void) const                  { return m_FrustPlaneT;  }
    const SPlane      & GetFrustPlaneB(void) const                  { return m_FrustPlaneB;  }

    void SetDrawNowParams(const D3DXMATRIX &iview, const D3DXVECTOR3 &fc) { m_DN_iview = iview;  m_DN_fc = fc;}    // hack function
    const D3DXMATRIX &GetDrawNowIView(void) const {return m_DN_iview;}
    const D3DXVECTOR3 &GetDrawNowFC(void) const {return m_DN_fc;}

    __forceinline D3DXVECTOR2 Project(const D3DXVECTOR3 &pos, const D3DXMATRIX &world)
    {
        D3DXVECTOR3 out;
        D3DXVec3Project(&out, &pos, &m_ViewPort, &m_MatProj, &m_MatView, &world);
        return D3DXVECTOR2(out.x, out.y);
    }
    __forceinline D3DXVECTOR3 ProjectNorm(const D3DXVECTOR3 &pos)
    {
        D3DXVECTOR3 out;
        D3DXMATRIX m(GetViewMatrix() * GetProjMatrix());
        D3DXVec3TransformCoord(&out, &pos, &m);
        out.z = GetViewMatrix()._13 * pos.x + GetViewMatrix()._23 * pos.y + GetViewMatrix()._33 * pos.z + GetViewMatrix()._43;
        return out;
    }

    void RestoreCameraParams(void);
    
    // visibility functions

    bool IsInFrustum(const D3DXVECTOR3 & p) const;                  //point in frustum
    bool IsInFrustum(const D3DXVECTOR3 & p, float radius) const;    // sphere in frustum
    bool IsInFrustum(const D3DXVECTOR3 & mins, const D3DXVECTOR3 & maxs) const; // check that box is in frustum
    float GetFrustPlaneDist(EFrustumPlane plane, const D3DXVECTOR3 &pos, const D3DXVECTOR3 &dir);
    float GetFrustPlaneMinDist(const D3DXVECTOR3 &pos);

    void BeforeDraw(void);
    void Takt(float ms);
};

__forceinline void CMatrixCamera::CalcPickVector(const CPoint &p, D3DXVECTOR3 &vdir) const
{
    D3DXVECTOR3 v;

    v.x = (((float)(p.x << 1) * _res_x_inversed) - 1.0f) * _mp_11_inversed;
    v.y = (((float)(p.y << 1) * _res_y_inversed) - 1.0f) * _mp_22_inversed;
    v.z = 1.0f;

    vdir.x = v.x*m_MatViewInversed._11 - v.y*m_MatViewInversed._21 + m_MatViewInversed._31;
    vdir.y = v.x*m_MatViewInversed._12 - v.y*m_MatViewInversed._22 + m_MatViewInversed._32;
    vdir.z = v.x*m_MatViewInversed._13 - v.y*m_MatViewInversed._23 + m_MatViewInversed._33;
    D3DXVec3Normalize(&vdir,&vdir);
}

__forceinline bool CMatrixCamera::IsInFrustum(const D3DXVECTOR3 & p) const
{
    /*
    D3DXVECTOR3 tp =  p - GetFrustumCenter();
    if ( (tp.x * GetFrustPlaneL().norm.x + tp.y * GetFrustPlaneL().norm.y + tp.z * GetFrustPlaneL().norm.z) < 0.0f) return false;
    if ( (tp.x * GetFrustPlaneR().norm.x + tp.y * GetFrustPlaneR().norm.y + tp.z * GetFrustPlaneR().norm.z) < 0.0f) return false;
    if ( (tp.x * GetFrustPlaneT().norm.x + tp.y * GetFrustPlaneT().norm.y + tp.z * GetFrustPlaneT().norm.z) < 0.0f) return false;
    if ( (tp.x * GetFrustPlaneB().norm.x + tp.y * GetFrustPlaneB().norm.y + tp.z * GetFrustPlaneB().norm.z) < 0.0f) return false;
    */
    if(!GetFrustPlaneL().IsOnSide(p)) return false;
    if(!GetFrustPlaneR().IsOnSide(p)) return false;
    if(!GetFrustPlaneT().IsOnSide(p)) return false;
    if(!GetFrustPlaneB().IsOnSide(p)) return false;
    return true;
}

__forceinline bool CMatrixCamera::IsInFrustum(const D3DXVECTOR3 & p, float r) const
{
    if(!GetFrustPlaneL().IsOnSide(p + GetFrustPlaneL().norm * r)) return false;
    if(!GetFrustPlaneR().IsOnSide(p + GetFrustPlaneR().norm * r)) return false;
    if(!GetFrustPlaneT().IsOnSide(p + GetFrustPlaneT().norm * r)) return false;
    if(!GetFrustPlaneB().IsOnSide(p + GetFrustPlaneB().norm * r)) return false;
    return true;
}


__forceinline bool CMatrixCamera::IsInFrustum(const D3DXVECTOR3 & mins, const D3DXVECTOR3 & maxs) const
{
    if(GetFrustPlaneL().BoxSide(mins, maxs) == 2) return false;
    if(GetFrustPlaneR().BoxSide(mins, maxs) == 2) return false;
    if(GetFrustPlaneB().BoxSide(mins, maxs) == 2) return false;
    if(GetFrustPlaneT().BoxSide(mins, maxs) == 2) return false;
    return true;
}


#endif