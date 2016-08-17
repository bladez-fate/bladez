#include "Defs.h"

USING_NS_CC;

// Materials
const cc::PhysicsMaterial gPlanetMaterial(0.0, 0.2, 500.0);
const cc::PhysicsMaterial gUnitMaterial(0.0, 0.2, 0.002);
const cc::PhysicsMaterial gRoughUnitMaterial(0.0, 0.2, 0.2);
const cc::PhysicsMaterial gProjectileMaterial(0.0, 0.2, 10.0);
const cc::PhysicsMaterial gBuildingMaterial(0.0, 0.2, 10.0);
const cc::PhysicsMaterial gPlatformMaterial(0.0, 0.2, 150.0);

// Cameras
const cc::CameraFlag gWorldCameraFlag = CameraFlag::USER1;

// Colors
const cc::Color4F gSupplyColor(0.5f, 0.6f, 1.0f, 1.0f);
const cc::Color4F gOreColor(0.9f, 0.7f, 0.3f, 1.0f);
const cc::Color4F gOilColor(0.0f, 0.0f, 0.0f, 1.0f);
const cc::Color4F gResColor[RES_COUNT] = { gOreColor, gOilColor };
const cc::Color4F gNeutralPlayerColor(0.7f, 0.7f, 0.7f, 1.0f);
const cc::Color4F gPlayerColor[3] = {
    Color4F(0.9f, 0.9f, 0.1f, 1.0f),
    Color4F(0.1f, 0.9f, 0.9f, 1.0f),
    Color4F(0.9f, 0.1f, 0.9f, 1.0f)
};
const cc::Color4F gSelectionColor(0.0f, 0.9f, 0.0f, 1.0f);
const float gHpRedLevel    = 0.2f;
const float gHpYellowLevel = 0.5f;
const cc::Color4F gHpRedColor   (1.0f, 0.0f, 0.0f, 1.0f);
const cc::Color4F gHpYellowColor(1.0f, 1.0f, 0.0f, 1.0f);
const cc::Color4F gHpGreenColor (0.0f, 1.0f, 0.0f, 1.0f);
const cc::Color4F gHpBgColor    (0.7f, 0.7f, 0.7f, 1.0f);
const cc::Color4F gProdColor      (0.0f, 1.0f, 1.0f, 1.0f);
const cc::Color4F gProdBgColor    (0.7f, 0.7f, 0.7f, 1.0f);
const cc::Color4F gIndicatorBorderColor(0.0f, 0.0f, 0.0f, 1.0f);

// Keyboard
cc::EventKeyboard::KeyCode gHKGroup[10] = {
    EventKeyboard::KeyCode::KEY_0,
    EventKeyboard::KeyCode::KEY_1,
    EventKeyboard::KeyCode::KEY_2,
    EventKeyboard::KeyCode::KEY_3,
    EventKeyboard::KeyCode::KEY_4,
    EventKeyboard::KeyCode::KEY_5,
    EventKeyboard::KeyCode::KEY_6,
    EventKeyboard::KeyCode::KEY_7,
    EventKeyboard::KeyCode::KEY_8,
    EventKeyboard::KeyCode::KEY_9
};
cc::EventKeyboard::KeyCode gHKSelectArmy = cc::EventKeyboard::KeyCode::KEY_F2;
cc::EventKeyboard::KeyCode gHKMoveLeft = cc::EventKeyboard::KeyCode::KEY_A;
cc::EventKeyboard::KeyCode gHKMoveRight = cc::EventKeyboard::KeyCode::KEY_D;
cc::EventKeyboard::KeyCode gHKGoBack = cc::EventKeyboard::KeyCode::KEY_W;
cc::EventKeyboard::KeyCode gHKGoFront = cc::EventKeyboard::KeyCode::KEY_S;
cc::EventKeyboard::KeyCode gHKAngleInc = cc::EventKeyboard::KeyCode::KEY_Q;
cc::EventKeyboard::KeyCode gHKAngleDec = cc::EventKeyboard::KeyCode::KEY_E;
cc::EventKeyboard::KeyCode gHKPowerInc = cc::EventKeyboard::KeyCode::KEY_R;
cc::EventKeyboard::KeyCode gHKPowerDec = cc::EventKeyboard::KeyCode::KEY_F;
cc::EventKeyboard::KeyCode gHKShoot = cc::EventKeyboard::KeyCode::KEY_SPACE;
cc::EventKeyboard::KeyCode gHKHold = cc::EventKeyboard::KeyCode::KEY_H;
