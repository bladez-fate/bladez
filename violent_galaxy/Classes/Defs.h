#pragma once

#include "cocos2d.h"

#define UNUSED(var) (void)(var)

namespace cc = cocos2d;

using ui8 = unsigned char;
using ui16 = unsigned short;
using ui32 = unsigned int;
using ui64 = unsigned long;

using i8 = char;
using i16 = short;
using i32 = int;
using i64 = long;

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

// Resources
static constexpr size_t RES_COUNT = 2;

// Constacts
constexpr int gMatterBitmask = 0x01;

// Materials
extern const cc::PhysicsMaterial gPlanetMaterial;
extern const cc::PhysicsMaterial gUnitMaterial;
extern const cc::PhysicsMaterial gProjectileMaterial;
extern const cc::PhysicsMaterial gBuildingMaterial;
extern const cc::PhysicsMaterial gPlatformMaterial;

// Cameras
extern const cc::CameraFlag gWorldCameraFlag;

// Colors
extern const cc::Color4F gOreColor;
extern const cc::Color4F gOilColor;
extern const cc::Color4F gResColor[RES_COUNT];

// Keyboard hotkeys
static constexpr size_t GRP_COUNT = 10;
extern cc::EventKeyboard::KeyCode gHKGroup[GRP_COUNT];

// TODO[fate]: move to some sort of util
// Returns x = a + 2*pi*n, where n is integer and x is in [0; 2*pi)
inline float mainAngle(float a)
{
    if (a < 0 || a >= 2 * M_PI) {
        a -= (2 * M_PI) * floorf(a / (2 * M_PI));
        CCASSERT(a >= 0, "angle is still negative");
        CCASSERT(a < 2 * M_PI , "angle is still above 2*pi");
    }
    return a;
}

