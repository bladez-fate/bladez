#include "Defs.h"

USING_NS_CC;

// Materials
const cc::PhysicsMaterial gPlanetMaterial(0.0, 0.2, 500.0);
const cc::PhysicsMaterial gUnitMaterial(0.0, 0.2, 0.002);
const cc::PhysicsMaterial gProjectileMaterial(0.0, 0.2, 10.0);
const cc::PhysicsMaterial gBuildingMaterial(0.0, 0.2, 10.0);
const cc::PhysicsMaterial gPlatformMaterial(0.0, 0.2, 150.0);

// Cameras
const CameraFlag gWorldCameraFlag = CameraFlag::USER1;

// Colors
const cc::Color4F gOreColor(0.9f, 0.7f, 0.3f, 1.0f);
const cc::Color4F gOilColor(0.0f, 0.0f, 0.0f, 1.0f);
const cc::Color4F gResColor[RES_COUNT] = { gOreColor, gOilColor };
