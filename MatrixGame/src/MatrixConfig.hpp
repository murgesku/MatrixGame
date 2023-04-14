// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef MATRIX_CONFIG_INCLUDE
#define MATRIX_CONFIG_INCLUDE

#include "Effects/MatrixEffect.hpp"
#include "Effects/MatrixEffectWeapon.hpp"
#include "MatrixMapStatic.hpp"
#include "MatrixGameDll.hpp"

#define BUILDING_TYPE_CNT 6
#define CANNON_TYPE_CNT   4
#define FLYER_TYPE_CNT    1

#define DAMAGE_PER_TIME 10000
#define SECRET_VALUE    1000000.0f

enum ERes {
    TITAN,
    ELECTRONICS,
    ENERGY,
    PLASMA,

    MAX_RESOURCES,

    ERes_FORCE_DWORD = 0x7FFFFFFF
};

enum ERobotUnitKind : unsigned int {
    RUK_UNKNOWN = 0,

    // chaisis
    RUK_CHASSIS_PNEUMATIC = 1,
    RUK_CHASSIS_WHEEL = 2,
    RUK_CHASSIS_TRACK = 3,
    RUK_CHASSIS_HOVERCRAFT = 4,
    RUK_CHASSIS_ANTIGRAVITY = 5,

    ROBOT_CHASSIS_CNT = 5,

    // weapon
    RUK_WEAPON_MACHINEGUN = 1,
    RUK_WEAPON_CANNON = 2,
    RUK_WEAPON_MISSILE = 3,
    RUK_WEAPON_FLAMETHROWER = 4,
    RUK_WEAPON_MORTAR = 5,
    RUK_WEAPON_LASER = 6,
    RUK_WEAPON_BOMB = 7,
    RUK_WEAPON_PLASMA = 8,
    RUK_WEAPON_ELECTRIC = 9,
    RUK_WEAPON_REPAIR = 10,

    ROBOT_WEAPON_CNT = 10,

    // armor
    RUK_ARMOR_PASSIVE = 1,
    RUK_ARMOR_ACTIVE = 2,
    RUK_ARMOR_FIREPROOF = 3,
    RUK_ARMOR_PLASMIC = 4,
    RUK_ARMOR_NUCLEAR = 5,
    RUK_ARMOR_6 = 6,

    ROBOT_ARMOR_CNT = 6,

    // head
    RUK_HEAD_BLOCKER = 1,
    RUK_HEAD_DYNAMO = 2,
    RUK_HEAD_LOCKATOR = 3,
    RUK_HEAD_FIREWALL = 4,

    ROBOT_HEAD_CNT = 4,
};

struct SStringPair {
    std::wstring key;
    std::wstring val;
};

enum EKeyAction {
    KA_SCROLL_LEFT,
    KA_SCROLL_RIGHT,
    KA_SCROLL_UP,
    KA_SCROLL_DOWN,

    KA_SCROLL_LEFT_ALT,
    KA_SCROLL_RIGHT_ALT,
    KA_SCROLL_UP_ALT,
    KA_SCROLL_DOWN_ALT,

    KA_ROTATE_LEFT,
    KA_ROTATE_RIGHT,
    KA_ROTATE_UP,
    KA_ROTATE_DOWN,

    KA_UNIT_FORWARD,
    KA_UNIT_BACKWARD,
    KA_UNIT_LEFT,
    KA_UNIT_RIGHT,

    KA_UNIT_FORWARD_ALT,
    KA_UNIT_BACKWARD_ALT,
    KA_UNIT_LEFT_ALT,
    KA_UNIT_RIGHT_ALT,

    KA_FIRE,
    KA_AUTO,

    KA_SHIFT,
    KA_CTRL,

    KA_ROTATE_LEFT_ALT,   // zak
    KA_ROTATE_RIGHT_ALT,  // zak

    KA_MINIMAP_ZOOMIN,   // sub
    KA_MINIMAP_ZOOMOUT,  // sub

    KA_CAM_SETDEFAULT,  // zak

    KA_AUTOORDER_CAPTURE,  // sub
    KA_AUTOORDER_ATTACK,   // sub
    KA_AUTOORDER_DEFEND,   // sub

    KA_ORDER_MOVE,           // sub
    KA_ORDER_STOP,           // sub
    KA_ORDER_CAPTURE,        // sub
    KA_ORDER_PATROL,         // sub
    KA_ORDER_EXPLODE,        // sub
    KA_ORDER_REPAIR,         // sub
    KA_ORDER_ATTACK,         // sub
    KA_ORDER_ROBOT_SWITCH1,  // sub
    KA_ORDER_ROBOT_SWITCH2,  // sub

    KA_ORDER_CANCEL,  // sub

    KA_UNIT_BOOM,       // sub
    KA_UNIT_ENTER,      // sub (! if not dialog mode)
    KA_UNIT_ENTER_ALT,  // sub (! if not dialog mode)

    KA_BUILD_ROBOT,   // sub
    KA_BUILD_TURRET,  // sub
    KA_BUILD_HELP,    // sub

    KA_TURRET_CANNON,
    KA_TURRET_GUN,
    KA_TURRET_LASER,
    KA_TURRET_ROCKET,

    //..........................
    KA_LAST
};

struct SWeaponDamage {
    int damage;
    int friend_damage;
    int mindamage;
};

enum ETimings {
    RESOURCE_TITAN,
    RESOURCE_ELECTRONICS,
    RESOURCE_ENERGY,
    RESOURCE_PLASMA,
    RESOURCE_BASE,
    //
    UNIT_ROBOT,
    UNIT_FLYER,
    UNIT_TURRET,

    MAINTENANCE_PERIOD,
    //
    //............................
    TIMING_LAST
};

enum EOverHeat {
    WHP_VOLCANO,
    WEAPON_VOLCANO_HEAT,
    WCP_VOLCANO,
    WEAPON_VOLCANO_COOL,

    WHP_PLASMA,
    WEAPON_PLASMA_HEAT,
    WCP_PLASMA,
    WEAPON_PLASMA_COOL,

    WHP_LASER,
    WEAPON_LASER_HEAT,
    WCP_LASER,
    WEAPON_LASER_COOL,

    WHP_HOMING_MISSILE,
    WEAPON_HOMING_MISSILE_HEAT,
    WCP_HOMING_MISSILE,
    WEAPON_HOMING_MISSILE_COOL,

    WHP_FLAMETHROWER,
    WEAPON_FLAMETHROWER_HEAT,
    WCP_FLAMETHROWER,
    WEAPON_FLAMETHROWER_COOL,

    WHP_BOMB,
    WEAPON_BOMB_HEAT,
    WCP_BOMB,
    WEAPON_BOMB_COOL,

    WHP_GUN,
    WEAPON_GUN_HEAT,
    WCP_GUN,
    WEAPON_GUN_COOL,

    WHP_LIGHTENING,
    WEAPON_LIGHTENING_HEAT,
    WCP_LIGHTENING,
    WEAPON_LIGHTENING_COOL,

    //............................
    OVERHEAT_LAST
};

enum EPrice {
    HEAD1_TITAN,
    HEAD1_ELECTRONICS,
    HEAD1_ENERGY,
    HEAD1_PLASM,

    HEAD2_TITAN,
    HEAD2_ELECTRONICS,
    HEAD2_ENERGY,
    HEAD2_PLASM,

    HEAD3_TITAN,
    HEAD3_ELECTRONICS,
    HEAD3_ENERGY,
    HEAD3_PLASM,

    HEAD4_TITAN,
    HEAD4_ELECTRONICS,
    HEAD4_ENERGY,
    HEAD4_PLASM,

    ARMOR1_TITAN,
    ARMOR1_ELECTRONICS,
    ARMOR1_ENERGY,
    ARMOR1_PLASM,

    ARMOR2_TITAN,
    ARMOR2_ELECTRONICS,
    ARMOR2_ENERGY,
    ARMOR2_PLASM,

    ARMOR3_TITAN,
    ARMOR3_ELECTRONICS,
    ARMOR3_ENERGY,
    ARMOR3_PLASM,

    ARMOR4_TITAN,
    ARMOR4_ELECTRONICS,
    ARMOR4_ENERGY,
    ARMOR4_PLASM,

    ARMOR5_TITAN,
    ARMOR5_ELECTRONICS,
    ARMOR5_ENERGY,
    ARMOR5_PLASM,

    ARMOR6_TITAN,
    ARMOR6_ELECTRONICS,
    ARMOR6_ENERGY,
    ARMOR6_PLASM,

    WEAPON1_TITAN,
    WEAPON1_ELECTRONICS,
    WEAPON1_ENERGY,
    WEAPON1_PLASM,

    WEAPON2_TITAN,
    WEAPON2_ELECTRONICS,
    WEAPON2_ENERGY,
    WEAPON2_PLASM,

    WEAPON3_TITAN,
    WEAPON3_ELECTRONICS,
    WEAPON3_ENERGY,
    WEAPON3_PLASM,

    WEAPON4_TITAN,
    WEAPON4_ELECTRONICS,
    WEAPON4_ENERGY,
    WEAPON4_PLASM,

    WEAPON5_TITAN,
    WEAPON5_ELECTRONICS,
    WEAPON5_ENERGY,
    WEAPON5_PLASM,

    WEAPON6_TITAN,
    WEAPON6_ELECTRONICS,
    WEAPON6_ENERGY,
    WEAPON6_PLASM,

    WEAPON7_TITAN,
    WEAPON7_ELECTRONICS,
    WEAPON7_ENERGY,
    WEAPON7_PLASM,

    WEAPON8_TITAN,
    WEAPON8_ELECTRONICS,
    WEAPON8_ENERGY,
    WEAPON8_PLASM,

    WEAPON9_TITAN,
    WEAPON9_ELECTRONICS,
    WEAPON9_ENERGY,
    WEAPON9_PLASM,

    WEAPON10_TITAN,
    WEAPON10_ELECTRONICS,
    WEAPON10_ENERGY,
    WEAPON10_PLASM,

    CHASSIS1_TITAN,
    CHASSIS1_ELECTRONICS,
    CHASSIS1_ENERGY,
    CHASSIS1_PLASM,

    CHASSIS2_TITAN,
    CHASSIS2_ELECTRONICS,
    CHASSIS2_ENERGY,
    CHASSIS2_PLASM,

    CHASSIS3_TITAN,
    CHASSIS3_ELECTRONICS,
    CHASSIS3_ENERGY,
    CHASSIS3_PLASM,

    CHASSIS4_TITAN,
    CHASSIS4_ELECTRONICS,
    CHASSIS4_ENERGY,
    CHASSIS4_PLASM,

    CHASSIS5_TITAN,
    CHASSIS5_ELECTRONICS,
    CHASSIS5_ENERGY,
    CHASSIS5_PLASM,

    //............................
    PRICE_LAST
};

enum ELabels {
    W1_CHAR,
    W2_CHAR,
    W3_CHAR,
    W4_CHAR,
    W5_CHAR,
    W6_CHAR,
    W7_CHAR,
    W8_CHAR,
    W9_CHAR,
    W10_CHAR,

    HU1_CHAR,
    HU2_CHAR,
    HU3_CHAR,
    HU4_CHAR,
    HU5_CHAR,
    HU6_CHAR,

    HE1_CHAR,
    HE2_CHAR,
    HE3_CHAR,
    HE4_CHAR,

    CH1_CHAR,
    CH2_CHAR,
    CH3_CHAR,
    CH4_CHAR,
    CH5_CHAR,

    //............................
    LABELS_LAST
};

enum EDescriptions {
    W1_DESCR,
    W2_DESCR,
    W3_DESCR,
    W4_DESCR,
    W5_DESCR,
    W6_DESCR,
    W7_DESCR,
    W8_DESCR,
    W9_DESCR,
    W10_DESCR,

    HU1_DESCR,
    HU2_DESCR,
    HU3_DESCR,
    HU4_DESCR,
    HU5_DESCR,
    HU6_DESCR,

    HE1_DESCR,
    HE2_DESCR,
    HE3_DESCR,
    HE4_DESCR,

    CH1_DESCR,
    CH2_DESCR,
    CH3_DESCR,
    CH4_DESCR,
    CH5_DESCR,

    //............................
    DESCRIPTIONS_LAST
};

enum EChars {
    ARMOR1_STRUCTURE,
    ARMOR1_ROTATION_SPEED,

    ARMOR2_STRUCTURE,
    ARMOR2_ROTATION_SPEED,

    ARMOR3_STRUCTURE,
    ARMOR3_ROTATION_SPEED,

    ARMOR4_STRUCTURE,
    ARMOR4_ROTATION_SPEED,

    ARMOR5_STRUCTURE,
    ARMOR5_ROTATION_SPEED,

    ARMOR6_STRUCTURE,
    ARMOR6_ROTATION_SPEED,

    CHASSIS1_STRUCTURE,
    CHASSIS1_ROTATION_SPEED,
    CHASSIS1_MOVE_SPEED,
    CHASSIS1_MOVE_WATER_CORR,
    CHASSIS1_MOVE_SLOPE_CORR_UP,
    CHASSIS1_MOVE_SLOPE_CORR_DOWN,

    CHASSIS2_STRUCTURE,
    CHASSIS2_ROTATION_SPEED,
    CHASSIS2_MOVE_SPEED,
    CHASSIS2_MOVE_WATER_CORR,
    CHASSIS2_MOVE_SLOPE_CORR_UP,
    CHASSIS2_MOVE_SLOPE_CORR_DOWN,

    CHASSIS3_STRUCTURE,
    CHASSIS3_ROTATION_SPEED,
    CHASSIS3_MOVE_SPEED,
    CHASSIS3_MOVE_WATER_CORR,
    CHASSIS3_MOVE_SLOPE_CORR_UP,
    CHASSIS3_MOVE_SLOPE_CORR_DOWN,

    CHASSIS4_STRUCTURE,
    CHASSIS4_ROTATION_SPEED,
    CHASSIS4_MOVE_SPEED,
    CHASSIS4_MOVE_WATER_CORR,
    CHASSIS4_MOVE_SLOPE_CORR_UP,
    CHASSIS4_MOVE_SLOPE_CORR_DOWN,

    CHASSIS5_STRUCTURE,
    CHASSIS5_ROTATION_SPEED,
    CHASSIS5_MOVE_SPEED,
    CHASSIS5_MOVE_WATER_CORR,
    CHASSIS5_MOVE_SLOPE_CORR_UP,
    CHASSIS5_MOVE_SLOPE_CORR_DOWN,

    // ARMOR,
    // WEAPON_SPEED,
    // WEAPON_HEAT,
    // LESS_HIT,
    // CHASSIS_SPEED,
    // WEAPON_RADIUS,
    // ROBOT_RADAR_RADIUS,
    // ELECTRO_PROTECTION,

    //..........................
    CHARS_LAST
};

struct SCannonProps {
    float max_top_angle;
    float max_bottom_angle;
    EWeapon weapon;
    float max_da;  // if 0 then best rotation
    float seek_radius;

    int m_Resources[MAX_RESOURCES];
    float m_Strength;
    float m_Hitpoint;
};

struct SCamParam {
    float m_CamMouseWheelStep;
    float m_CamRotSpeedX;
    float m_CamRotSpeedZ;
    float m_CamRotAngleMin;
    float m_CamRotAngleMax;
    float m_CamDistMin;
    float m_CamDistMax;
    float m_CamAngleParam;
    float m_CamHeight;
};

struct SGammaVals {
    float brightness, contrast, gamma;
};

enum {
    CAMERA_STRATEGY,
    CAMERA_INROBOT,

    CAMERA_PARAM_CNT
};

inline int WeapKind2Index(ERobotUnitKind w) {
    if (w == RUK_WEAPON_PLASMA)
        return 0;
    if (w == RUK_WEAPON_MACHINEGUN)
        return 1;
    if (w == RUK_WEAPON_MISSILE)
        return 2;
    if (w == RUK_WEAPON_MORTAR)
        return 3;
    if (w == RUK_WEAPON_FLAMETHROWER)
        return 4;
    if (w == RUK_WEAPON_BOMB)
        return 5;
    if (w == RUK_WEAPON_ELECTRIC)
        return 6;
    if (w == RUK_WEAPON_LASER)
        return 7;
    if (w == RUK_WEAPON_CANNON)
        return 8;
    if (w == RUK_WEAPON_REPAIR)
        return 12;
    return -1;
}

class CMatrixConfig : public CMain {
public:
    SStringPair *m_Cursors;
    int m_CursorsCnt;

    SWeaponDamage m_CannonDamages[WEAPON_COUNT];

    int m_BuildingHitPoints[BUILDING_TYPE_CNT];
    SWeaponDamage m_BuildingDamages[WEAPON_COUNT];

    SWeaponDamage m_RobotDamages[WEAPON_COUNT];
    SWeaponDamage m_ObjectDamages[WEAPON_COUNT];
    SWeaponDamage m_FlyerDamages[WEAPON_COUNT];

    float m_WeaponStrengthAI[WEAPON_COUNT];  // Сила оружия, по которому AI ориентируется, какой робот сильнее.

    float m_WeaponRadius[WEAPON_COUNT];
    int m_WeaponCooldown[WEAPON_COUNT];

    int m_MaintenanceTime;

    // int   m_PlayerRobotsCnt;
    // int   m_CompRobotsCnt;

    // cannons
    SCannonProps m_CannonsProps[CANNON_TYPE_CNT];

    // camera params

    SCamParam m_CamParams[CAMERA_PARAM_CNT];

    float m_CamBaseAngleZ;
    float m_CamMoveSpeed;
    float m_CamInRobotForward0;
    float m_CamInRobotForward1;

    // float m_CamRotAngleMinInFlyer;
    // float m_CamDistMinInFlyer;

    SGammaVals m_GammaR, m_GammaG, m_GammaB;

    // params
    DWORD m_DIFlags;
    // int   m_TexTopMinSize;
    // int   m_TexTopDownScalefactor;
    // int   m_TexBotMinSize;
    // int   m_TexBotDownScalefactor;

    bool m_ShowStencilShadows;
    bool m_ShowProjShadows;
    bool m_CannonsLogic;
    // bool  m_LandTextures16;

    bool m_LandTexturesGloss;
    bool m_SoftwareCursor;
    bool m_VertexLight;

    bool m_ObjTexturesGloss;
    // bool  m_ObjTextures16;

    bool m_IzvratMS;
    byte m_SkyBox;                   // 0 - none, 1 - dds (low quality), 2 - png (high quality)
    byte m_DrawAllObjectsToMinimap;  // 0 - none, 1 - force yes, 2 - auto

    int m_CaptureTimeErase;
    int m_CaptureTimePaint;
    int m_CaptureTimeRolback;

    int m_KeyActions[KA_LAST];
    int m_Timings[TIMING_LAST];
    int m_Price[PRICE_LAST];
    int m_Overheat[OVERHEAT_LAST];
    float m_ItemChars[CHARS_LAST];

    std::wstring* m_Labels;
    std::wstring* m_Descriptions;

    float m_RobotRadarR;
    float m_FlyerRadarR;

    EShadowType m_RobotShadow;

    void Clear(void);

    void SetDefaults(void);
    void ReadParams(void);
    void ApplySettings(SRobotsSettings *set);

    void ApplyGammaRamp(void);

    CMatrixConfig(void) : CMain() {
        m_Labels = NULL;
        m_Descriptions = NULL;
    };
    ~CMatrixConfig(){};
};

extern CMatrixConfig g_Config;

#endif