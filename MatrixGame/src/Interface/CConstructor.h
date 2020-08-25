// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#pragma once
#include "../MatrixObject.hpp"
#include "../MatrixRobot.hpp"
#include "../Logic/MatrixAIGroup.h"

class CIFaceElement;

struct SPrice
{
    int m_Resources[MAX_RESOURCES];                                     //titan, electronics, energy, plasma

    void ResetPrice()                                                   { ZeroMemory(m_Resources, sizeof(m_Resources)); }
    void GiveRndPrice()                                                 { for(int cnt = 0; cnt < MAX_RESOURCES;cnt++){ m_Resources[cnt] = g_MatrixMap->Rnd(0, 50); }}
    
    void SetPrice(ERobotUnitType type, ERobotUnitKind kind);

    SPrice()                                                            { ResetPrice(); }
};

struct SUnit {
    ERobotUnitType  m_nType;
    ERobotUnitKind  m_nKind;
    SPrice m_Price;
    
    SUnit():m_nType(MRT_EMPTY), m_nKind(RUK_UNKNOWN) {}
};

struct SArmorUnit {
	int				m_MaxCommonWeaponCnt;
	int				m_MaxExtraWeaponCnt;

    SUnit			m_Unit;

    SArmorUnit():m_MaxCommonWeaponCnt(0), m_MaxExtraWeaponCnt(0) {}
};

struct SWeaponUnit {
    int m_Pos;

    SUnit m_Unit;
    SWeaponUnit()                                                        { m_Pos = 0; m_Unit.m_nKind = RUK_UNKNOWN; m_Unit.m_nType = MRT_WEAPON;}
};

struct SNewBorn {
    CMatrixRobotAI*     m_Robot;
    int                 m_Team;
    SNewBorn()
    {
        m_Robot = NULL;
        m_Team = -1;
    }
    //~SNewBorn();
};

struct SSpecialBot {
    SUnit               m_Head;
	SWeaponUnit         m_Weapon[MAX_WEAPON_CNT];
    SArmorUnit          m_Armor;
	SUnit               m_Chassis;
    int                 m_Team;

    int                 m_Resources[MAX_RESOURCES];                                     //titan, electronics, energy, plasma

    int                 m_Pripor;
    float               m_Hitpoints;
    float               m_Strength;
    bool                m_HaveBomb;
    bool                m_HaveRepair;

    static SSpecialBot * m_AIRobotTypeList;
    static int m_AIRobotTypeCnt;

    static void LoadAIRobotType(CBlockPar & bp);
    static void ClearAIRobotType(void);

    void CalcStrength(void);                                 // Расчитываем силу робота
    float DifWeapon(SSpecialBot & other);                    //  0..1 на сколько отличается оружие у роботов

    bool BuildFromPar(const CWStr & parname, int parval, bool with_hp=false);

    CMatrixRobotAI*     GetRobot(const D3DXVECTOR3 &pos, int side);
};
void GetConstructionName(CMatrixRobotAI* robot);
int GetConstructionDamage(CMatrixRobotAI* robot);
class CConstructor : public CMain
{
	float               m_RobotPosX, m_RobotPosY;
	int                 m_Side, m_ShadowType, m_ShadowSize, m_nUnitCnt,  m_ViewWidthX, m_ViewHeightY;
	float               m_ViewPosX, m_ViewPosY;
	int                 m_nPos;
	CMatrixRobotAI*     m_Robot, *m_Build;
	
    SUnit               m_Unit[MR_MAXUNIT];
	
    SUnit               m_Head;
	SWeaponUnit         m_Weapon[MAX_WEAPON_CNT];
    SArmorUnit          m_Armor;
	SUnit               m_Chassis;
	

	CMatrixBuilding*    m_Base;
    SNewBorn*           m_NewBorn;
	CWStr           m_ConstructionName;
	void InsertUnits();
	void ResetConstruction();

public:
    CMatrixRobotAI*     GetRenderBot()                                          { return m_Robot; }
    int CheckWeaponLegality(SWeaponUnit* weapons, int weaponKind, int armorKind);
    void GetConstructionPrice(int* res);
    int GetConstructionStructure();

    void SetSide(int side)                                                      { m_Side = side; if(m_Robot) m_Robot->m_Side = side; }
    int GetSide()                                                               { return m_Side; }

    void SetBase(CMatrixBuilding *pBase)                                        { m_Base = pBase; /*m_Side = side; */}

    bool CheckMaxUnits()                                                        { return (m_nUnitCnt <= MR_MAXUNIT); }

    void SetRenderProps(float x, float y, int width, int height)                { m_ViewPosX = x; m_ViewPosY = y; m_ViewWidthX = width;	m_ViewHeightY = height; }

	void __stdcall RemoteOperateUnit(void* pObj);
	void OperateUnit(ERobotUnitType type, ERobotUnitKind kind);
    void SuperDjeans(ERobotUnitType type, ERobotUnitKind kind, int pilon, bool ld_from_history = false);
    void Djeans007(ERobotUnitType type, ERobotUnitKind kind, int pilon);

	void __stdcall RemoteBuild(void* pObj);
	SNewBorn* ProduceRobot(void* pObject);
    void StackRobot(void* pObject,int team=0);
    void BeforeRender(void);

	void Render();

//STUB: FAKE FUNCTIONS MOTHERFUCKERS
    void BuildRandomBot()
	{
//Chassis
		int rnd = g_MatrixMap->Rnd(1,5);//(int)RND(1, 5);
		//if(rnd == 2) rnd = 4;
        //rnd = 4;
		OperateUnit(MRT_CHASSIS, (ERobotUnitKind)rnd);
//ARMOR
		rnd = g_MatrixMap->Rnd(1,6);
       
        OperateUnit(MRT_ARMOR, (ERobotUnitKind)rnd);
//WEAPON
		rnd = (int)RND(1, 5);
		for(int nC = 0; nC <= rnd; nC++){
			OperateUnit(MRT_WEAPON, (ERobotUnitKind)/*6*/g_MatrixMap->Rnd(1,9));
		}
//HEAD
		OperateUnit(MRT_HEAD, (ERobotUnitKind)g_MatrixMap->Rnd(1,7));
	}
    
    //STUB:
    void BuildSpecialBot(const SSpecialBot &bot);

    void OperateCurrentConstruction(); 
///////////////////////////////

	CConstructor();
	~CConstructor();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PRESETS 1

struct SRobotConfig{
    SUnit           m_Head;
    SUnit           m_Weapon[MAX_WEAPON_CNT];
    SUnit           m_Chassis;
    SArmorUnit      m_Hull;

    int             m_titX;
    int             m_elecX;
    int             m_enerX;
    int             m_plasX;

    int             m_Structure;
    int             m_Damage;

    SRobotConfig*   m_NextConfig;
    SRobotConfig*   m_PrevConfig;

    SRobotConfig()                                                              { m_titX = 0; m_elecX = 0; m_enerX = 0; m_plasX = 0; m_Structure = 0; m_Damage = 0; m_NextConfig = NULL; m_PrevConfig = NULL;}
};


class CConstructorPanel : public CMain {
public:
    CIFaceElement*  m_FocusedElement;
    int             m_ftitX;
    int             m_felecX;
    int             m_fenerX;
    int             m_fplasX;
    int             m_CurrentConfig;
    CWStr           m_FocusedLabel;
    CWStr           m_FocusedDescription;
    byte            m_Active;

    SRobotConfig    m_Configs[PRESETS];

    void ActivateAndSelect();
    void ResetGroupNClose();                                                     

    void ResetConfig()                                                          { ZeroMemory(&m_Configs, sizeof(m_Configs)); }
    bool IsActive()                                                             { return m_Active == 1; }

    void ResetWeapon()                                                          { ZeroMemory(m_Configs[m_CurrentConfig].m_Weapon, sizeof(SUnit) * MAX_WEAPON_CNT); m_Configs[m_CurrentConfig].m_Damage = 0; }
    void __stdcall RemoteFocusElement(void* object);
    void __stdcall RemoteUnFocusElement(void* object);
    void FocusElement(CIFaceElement* element);
    void UnFocusElement(CIFaceElement* element);
    void SetLabelsAndPrice(ERobotUnitType type, ERobotUnitKind kind);

    bool IsEnoughResourcesForThisPieceOfShit(int pilon, ERobotUnitType type, ERobotUnitKind kind);

    void MakeItemReplacements(ERobotUnitType type, ERobotUnitKind kind);

    CConstructorPanel():m_FocusedLabel(g_MatrixHeap),m_FocusedDescription(g_MatrixHeap)                                                       
    { 
        m_CurrentConfig = 0;
        m_Active = 0; 
        ZeroMemory(&m_Configs, sizeof(m_Configs)); 
        m_Configs[m_CurrentConfig].m_Chassis.m_nType = MRT_CHASSIS; 
        m_Configs[m_CurrentConfig].m_Hull.m_Unit.m_nType = MRT_ARMOR; 
        m_Configs[m_CurrentConfig].m_Head.m_nType = MRT_HEAD; 
        for(int cnt = 0; cnt < MAX_WEAPON_CNT;cnt++){ 
            m_Configs[m_CurrentConfig].m_Weapon[cnt].m_nType = MRT_WEAPON;
        } 
        m_FocusedElement = NULL; 
        m_ftitX = 0; 
        m_felecX = 0; 
        m_fenerX = 0; 
        m_fplasX = 0;
    }
    ~CConstructorPanel()                                                        {}
};


