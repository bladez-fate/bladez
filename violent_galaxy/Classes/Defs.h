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

constexpr int gMatterBitmask = 0x1;

extern const cc::PhysicsMaterial gPlanetMaterial;
extern const cc::PhysicsMaterial gUnitMaterial;
extern const cc::PhysicsMaterial gProjectileMaterial;

extern const cc::CameraFlag gWorldCameraFlag;
