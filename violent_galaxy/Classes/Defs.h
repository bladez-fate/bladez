#pragma once

#include "cocos2d.h"

#define UNUSED(var) (void)(var)

namespace cc = cocos2d;

using ui8 = unsigned char;
using ui16 = unsigned short;
using ui32 = unsigned int;
using ui64 = unsigned long long;

using i8 = char;
using i16 = short;
using i32 = int;
using i64 = long long;

static_assert(sizeof(ui8)  == 1, "built-in type size mismatch");
static_assert(sizeof(ui16) == 2, "built-in type size mismatch");
static_assert(sizeof(ui32) == 4, "built-in type size mismatch");
static_assert(sizeof(ui64) == 8, "built-in type size mismatch");

static_assert(sizeof(i8)  == 1, "built-in type size mismatch");
static_assert(sizeof(i16) == 2, "built-in type size mismatch");
static_assert(sizeof(i32) == 4, "built-in type size mismatch");
static_assert(sizeof(i64) == 8, "built-in type size mismatch");

class GameScene;
class Obj;
class VisualObj;
class Unit;
class AstroObj;
class Player;
struct ContactInfo;

using Id = ui32; // 24 bits max

// Z-COORDINATES (8 bits)
using Zs = int; // Z-coordinate bitmask; also is used as contact bitmask
static constexpr Zs ZsNone = 0x00;
static constexpr Zs Zs0 = 0x01;
static constexpr Zs Zs1 = 0x02;
static constexpr Zs Zs2 = 0x04;
static constexpr Zs Zs3 = 0x08;
static constexpr Zs Zs4 = 0x10;
static constexpr Zs Zs5 = 0x20;
static constexpr Zs Zs6 = 0x40;
static constexpr Zs Zs7 = 0x80;

static constexpr Zs ZsAstroObjDefault = Zs2 | Zs3 | Zs4 | Zs5;
static constexpr Zs ZsUnitDefault = Zs4;
static constexpr Zs ZsProjectileDefault = Zs3 | Zs4;
static constexpr Zs ZsBuildingDefault = Zs3;
static constexpr Zs ZsBackground = Zs3;
static constexpr Zs ZsForeground = Zs4;

// Resources
static constexpr size_t RES_COUNT = 2;

// Contacts
extern const float gMaxUnitSize;
extern const float gMaxSeparationVelocity;
extern const float gMaxSeparationVelocitySq;

// Materials
extern const cc::PhysicsMaterial gPlanetMaterial;
extern const cc::PhysicsMaterial gUnitMaterial;
extern const cc::PhysicsMaterial gRoughUnitMaterial;
extern const cc::PhysicsMaterial gProjectileMaterial;
extern const cc::PhysicsMaterial gBuildingMaterial;
extern const cc::PhysicsMaterial gPlatformMaterial;

// Cameras
extern const cc::CameraFlag gWorldCameraFlag;

// Colors
extern const cc::Color4F gSupplyColor;
extern const cc::Color4F gOreColor;
extern const cc::Color4F gOilColor;
extern const cc::Color4F gResColor[RES_COUNT];
extern const cc::Color4F gNeutralPlayerColor;
extern const cc::Color4F gPlayerColor[3];
extern const cc::Color4F gSelectionColor;
extern const float gHpRedLevel;
extern const float gHpYellowLevel;
extern const cc::Color4F gHpRedColor;
extern const cc::Color4F gHpYellowColor;
extern const cc::Color4F gHpGreenColor;
extern const cc::Color4F gHpBgColor;
extern const cc::Color4F gProdColor;
extern const cc::Color4F gProdBgColor;
extern const cc::Color4F gIndicatorBorderColor;

// GUI
extern const cc::Color4F gPanelBgColor;
extern const cc::Color4F gPanelBorderColor;
//extern float gSelectionPanelWidth;
extern float gSelectionPanelHeight;
extern float gMiniMapPanelWidth;
extern float gMiniMapPanelHeight;
extern float gControlPanelWidth;
extern float gControlPanelHeight;

// Z-Order
extern const int gZOrderMouseSelectionRect;
extern const int gZOrderIndicators;
extern const int gZOrderResIcons;
extern const int gZOrderResLabels;
extern const int gZOrderSelectionPanel;

// Keyboard hotkeys
static constexpr size_t GRP_COUNT = 10;
extern cc::EventKeyboard::KeyCode gHKGroup[GRP_COUNT];
extern cc::EventKeyboard::KeyCode gHKSelectArmy;
extern cc::EventKeyboard::KeyCode gHKMoveLeft;
extern cc::EventKeyboard::KeyCode gHKMoveRight;
extern cc::EventKeyboard::KeyCode gHKGoBack;
extern cc::EventKeyboard::KeyCode gHKGoFront;
extern cc::EventKeyboard::KeyCode gHKAngleInc;
extern cc::EventKeyboard::KeyCode gHKAngleDec;
extern cc::EventKeyboard::KeyCode gHKPowerInc;
extern cc::EventKeyboard::KeyCode gHKPowerDec;
extern cc::EventKeyboard::KeyCode gHKShoot;
extern cc::EventKeyboard::KeyCode gHKHold;

// Orders
extern size_t gMaxOrders;
extern float gOrderDelayTimeout; // Time after which delayed order fails

// TODO[fate]: move to some sort of util
// Returns x = a + 2*pi*n, where n is integer and x is in [0; 2*pi)
inline float angleMain(float a)
{
    if (a < 0 || a >= 2 * M_PI) {
        a -= (2 * M_PI) * floorf(a / (2 * M_PI));
        CCASSERT(a >= 0, "angle is still negative");
    }
    return a;
}

// Returns x = a + 2*pi*n, where n is integer and x is in [-pi; pi)
inline float angleShort(float a)
{
    a = angleMain(a);
    if (a >= M_PI) {
        a -= 2*M_PI;
    }
    return a;
}

// Return angle in radians that should be added to a1 to give a2
inline float angleDistance(float a1, float a2)
{
    return angleShort(a2 - a1);
}

// Converts Zs to local z-order in game scene
inline int ZsOrder(Zs zs)
{
    if (zs & Zs7) {
        return 80;
    } else if (zs & Zs6) {
        return 70;
    } else if (zs & Zs5) {
        return 60;
    } else if (zs & Zs4) {
        return 50;
    } else if (zs & Zs3) {
        return 40;
    } else if (zs & Zs2) {
        return 30;
    } else if (zs & Zs1) {
        return 20;
    } else if (zs & Zs0) {
        return 10;
    } else {
        return -1; // ZsNone
    }
}
