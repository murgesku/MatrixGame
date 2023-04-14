// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef COMMON_INCLUDE
#define COMMON_INCLUDE

#include <cstdint>
#include <cstring> // for memcmp
#include <cmath>

#define MAP_GROUP_SIZE (10)

#define WATER_LEVEL         (-2.0f)
#define WATER_ALPHA_SIZE    64
#define WATER_SIZE          16
#define WATER_TEXTURE_SCALE (1.0 / 16.0)  //(10.0/16.0)
#define WATER_TIME_PERIOD   60

#define INSHORE_PRE_COUNT 50

#define TEX_BOTTOM_SIZE   (64)
#define TEXUNION_DIM_MAX  (32)
#define TEXUNION_SIZE_MAX (TEXUNION_DIM_MAX * TEXUNION_DIM_MAX)

#define MAX_DISTANCE_CANNON_BUILDING 500

#define DEF_SKY_COLOR 0x1070FF

#define PAR_TOP_TEX_GLOSS L"gloss"

#define STR_BREAK_TYPE_NONE    L"None"
#define STR_BREAK_TYPE_EXPLODE L"Explode"
#define STR_BREAK_TYPE_TERRON  L"Terron"

//// table for SCompileTextureOnCell
//#define DATA_TEXTURES           L"textures"
//#define DATA_TEXTURES_DATA      L"Data"

#define DATA_SURFACES_M      L"surfacesM"
#define DATA_SURFACES_DATA_M L"Data"

#define DATA_SURFACES      L"surfaces"
#define DATA_SURFACES_DATA L"Data"

#define DATA_TEXUINIONS      L"texunions"
#define DATA_TEXUINIONS_DATA L"Data"

#define DATA_BRIDGES      L"bridges"
#define DATA_BRIDGES_DATA L"Data"

#define DATA_ROADS      L"roads"
#define DATA_ROADS_DATA L"Data"

#define DATA_EFFECTS      L"effects"
#define DATA_EFFECTS_X    L"X"
#define DATA_EFFECTS_Y    L"Y"
#define DATA_EFFECTS_Z    L"Z"
#define DATA_EFFECTS_TYPE L"Type"

#define DATA_OBJECTS         L"objects"
#define DATA_OBJECTS_X       L"X"
#define DATA_OBJECTS_Y       L"Y"
#define DATA_OBJECTS_ANGLE_Z L"Angle"
#define DATA_OBJECTS_ANGLE_X L"AngleX"
#define DATA_OBJECTS_ANGLE_Y L"AngleY"
#define DATA_OBJECTS_HEIGHT  L"Height"
#define DATA_OBJECTS_Z       L"Z"
#define DATA_OBJECTS_SCALE   L"Scale"
#define DATA_OBJECTS_TYPE    L"Type"
#define DATA_OBJECTS_SHADOW  L"Shadow"  // shadow geometry

#define DATA_CANNONS            L"cannons"
#define DATA_CANNONS_X          L"X"
#define DATA_CANNONS_Y          L"Y"
#define DATA_CANNONS_ANGLE      L"Angle"
#define DATA_CANNONS_KIND       L"Kind"
#define DATA_CANNONS_SIDE       L"Side"
#define DATA_CANNONS_SHADOW     L"Shadow"
#define DATA_CANNONS_SHADOWSIZE L"ShadowSize"
#define DATA_CANNONS_PROP       L"Prop"
#define DATA_CANNONS_ADDH       L"AddH"

#define DATA_ROBOTS            L"robots"
#define DATA_ROBOTS_X          L"X"
#define DATA_ROBOTS_Y          L"Y"
#define DATA_ROBOTS_SIDE       L"Side"
#define DATA_ROBOTS_SHADOW     L"Shadow"
#define DATA_ROBOTS_SHADOWSIZE L"ShadowSize"
#define DATA_ROBOTS_UNITS      L"Units"
#define DATA_ROBOTS_GROUP      L"Group"

#define DATA_BUILDINGS            L"buildings"
#define DATA_BUILDINGS_X          L"X"
#define DATA_BUILDINGS_Y          L"Y"
#define DATA_BUILDINGS_ANGLE      L"Angle"
#define DATA_BUILDINGS_KIND       L"Kind"
#define DATA_BUILDINGS_SIDE       L"Side"
#define DATA_BUILDINGS_SHADOW     L"Shadow"
#define DATA_BUILDINGS_SHADOWSIZE L"ShadowSize"

#define DATA_GROUPS_INSHORES    L"inshores"
#define DATA_GROUPS_INSHORES_X  L"X"
#define DATA_GROUPS_INSHORES_Y  L"Y"
#define DATA_GROUPS_INSHORES_NX L"NX"
#define DATA_GROUPS_INSHORES_NY L"NY"

#define DATA_GROUPS_VIS        L"grpvis"
#define DATA_GROUPS_VIS_LEVELS L"Levels"
#define DATA_GROUPS_VIS_GROUPS L"Groups"
#define DATA_GROUPS_VIS_ZFROM  L"ZFrom"

#define DATA_GROUPS      L"groups"
#define DATA_GROUPS_DATA L"Data"

#define DATA_MOVE      L"move"
#define DATA_MOVE_DATA L"Data"

#define DATA_POINTS      L"points"
#define DATA_POINTS_DATA L"Data"

// bottom textures, used in map
#define DATA_BOTTOM      L"bottom"
#define DATA_BOTTOM_DATA L"Data"

// all strings used in map
#define DATA_STRINGS        L"strings"
#define DATA_STRINGS_STRING L"String"

// all bitmaps used in map
#define DATA_BITMAPS        L"bitmaps"
#define DATA_BITMAPS_BITMAP L"Bitmap"

// property table
#define DATA_PROPERTIES       L"properties"
#define DATA_PROPERTIES_NAME  L"Name"
#define DATA_PROPERTIES_VALUE L"Value"

#define DATA_SIZEX             L"SizeInUnitsX"
#define DATA_SIZEY             L"SizeInUnitsY"
#define DATA_BIASTER           L"BiasTer"
#define DATA_BIASWATER         L"BiasWater"
#define DATA_BIASCANNONS       L"BiasCannons"
#define DATA_BIASROBOTS        L"BiasRobots"
#define DATA_BIASBUILDINGS     L"BiasBuildings"
#define DATA_INFLUENCE         L"Influence"
#define DATA_WATERCOLOR        L"WaterColor"
#define DATA_SKYCOLOR          L"SkyColor"
#define DATA_INSHOREWAVECOLOR  L"InshorewaveColor"
#define DATA_AMBIENTCOLOR      L"AmbientColor"
#define DATA_AMBIENTCOLOROBJ   L"AmbientColorObj"
#define DATA_LIGHTMAINCOLOR    L"LightMainColor"
#define DATA_LIGHTMAINCOLOROBJ L"LightMainColorObj"
#define DATA_LIGHTMAINANGLEZ   L"LightMainAngleZ"
#define DATA_LIGHTMAINANGLEX   L"LightMainAngleX"
#define DATA_SHADOWCOLOR       L"ShadowColor"
#define DATA_MACROTEXTURE      L"MacroTexture"
#define DATA_TEXUNIONSCOUNT    L"TextureUnions"
#define DATA_GROUPSCOUNT       L"GroupsCount"
#define DATA_SHADOWTAGCOUNT    L"ShadowTagsCount"
#define DATA_TEXUNIONDIM       L"TexUnionDim"
#define DATA_TOPTEXSTRIPED     L"TopTexStriped"
#define DATA_UNIQID            L"UniqID"
#define DATA_SIDERESINFO       L"SideResInfo"
#define DATA_SIDEAIINFO        L"SideAIInfo"
#define DATA_MAINTENANCETIME   L"MaintenanceTime"
#define DATA_WATERNAME         L"WaterName"
#define DATA_WATERNORMLEN      L"WaterNormLen"
#define DATA_SKYNAME           L"SkyName"
#define DATA_SKYANGLE          L"SkyAngle"
#define DATA_CAMANGLE          L"CamAngle"
#define DATA_CAMPOSX           L"CamPosX"
#define DATA_CAMPOSY           L"CamPosY"
#define DATA_DISABLEINSHORE    L"DisableInshore"

enum EObjectTypeProperty {
    OTP_PATH = 0,       // 0    - path
    OTP_VO,             // 1    - vo
    OTP_TEXTURE,        // 2    - texture
    OTP_TEXTURE_GLOSS,  // 3    - texture gloss
    OTP_TEXTURE_BACK,   // 4    - texture back
    OTP_TEXTURE_MASK,   // 5    - texture mask
    OTP_TEXTURE_SCROLL,
    OTP_SHADOW,
    OTP_BEHAVIOUR,
    OTP_BIAS,
    OTP_INVLOGIC,

    OTP_COUNT
};

#define CELLFLAG_LAND    SETBIT(0)  // land or land under water
#define CELLFLAG_WATER   SETBIT(1)  // water (empty cell, no geometry)
#define CELLFLAG_BRIDGE  SETBIT(2)  // bridge
#define CELLFLAG_INSHORE SETBIT(3)  // inshore here
#define CELLFLAG_FLAT    SETBIT(4)  // cell is flat, so normal is (0,0,1)

#define CELLFLAG_DOWN SETBIT(5)  // points down

// if CELLFLAG_LAND is 0 and CELLFLAG_WATER is 0 then cell is empty (under base cell)

struct SCompilePoint {
    int move;
    float z;
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t flags;  // flags for cell
};

struct SCompileMove {
    int m_Zone;
    uint32_t m_Move;
    uint32_t m_Sphere;
    uint32_t m_Zubchik;
};

struct SCompileMoveCell {
    SCompileMove c[4];

    bool operator==(const SCompileMoveCell &c) const { return memcmp(this, &c, sizeof(SCompileMoveCell)) == 0; }
};

struct SCompileBottomVert {
    uint16_t x, y;  // point. z and color in points list
    uint16_t tx, ty;
    // float tu, tv;
    // float tum, tvm; // calc it from x and y

    bool operator==(const SCompileBottomVert &v) {
        // return x==v.x && y==v.y && tu == v.tu && tv == v.tv; // && tum == v.tum && tvm == v.tvm;
        int dtx = abs(tx - v.tx);
        int dty = abs(ty - v.ty);
        return x == v.x && y == v.y && (dtx <= 2) && (dty <= 2);
        // return x==v.x && y==v.y && tx == v.tx && ty == v.ty;
    }
};

/*
    bottom texture format:
    array of int32

    array[0] - ids name of base texture
    array[1] - ids name of mix-with texture; if -1 then mask bitmap is texture with alpha
    array[2] - index of mask bitmap
    array[3] - same as 1
    array[4] - same as 2
    ...
    etc

*/

#endif