#include "GameScene.h"
#include <base/ccRandom.h>
#include <chipmunk/chipmunk_private.h>
#include <SimpleAudioEngine.h>
#include "Projectiles.h"

USING_NS_CC;

Scene* GameScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::createWithPhysics();
//    scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL, (unsigned short)CameraFlag::USER1);
    scene->getPhysicsWorld()->setGravity(Vec2::ZERO);

    auto ffield = PhysicsForceField::create();
    scene->getPhysicsWorld()->setForceField(ffield);

    // 'layer' is an autorelease object
    auto layer = GameScene::create();
    layer->createWorld(scene, scene->getPhysicsWorld());

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

bool GameScene::init()
{
    if (!Layer::init()) {
        return false;
    }
    
    auto s = Director::getInstance()->getVisibleSize();
    Vec2 o = Director::getInstance()->getVisibleOrigin();

    auto closeItem = MenuItemImage::create(
        "CloseNormal.png",
        "CloseSelected.png",
        CC_CALLBACK_1(GameScene::menuCloseCallback, this)
    );
    closeItem->setPosition(Vec2(o.x + s.width - closeItem->getContentSize().width/2 ,
                                o.y + closeItem->getContentSize().height/2));

//    auto menu = Menu::create(closeItem, NULL);
//    menu->setPosition(Vec2::ZERO);
//    this->addChild(menu, 1);

//    auto label = Label::createWithTTF("Level 1", "fonts/Marker Felt.ttf", 24);
//    label->setPosition(Vec2(origin.x + visibleSize.width/2,
//                            origin.y + visibleSize.height - label->getContentSize().height));
//    addChild(label, 1);

    _objs = ObjStorage::create();

    return true;
}


void GameScene::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

void GameScene::createWorld(Scene* scene, PhysicsWorld* pworld)
{
    _pworld = pworld;
    pworld->setSpeed(1.0);

    initWorldView();
    initGalaxy();
    initCollisions();

    initPlayers();
    initMouse();
    initKeyboard();
}

void GameScene::initKeyboard()
{
    createKeyHoldHandler();
}

void GameScene::createKeyHoldHandler()
{
    auto eventListener = EventListenerKeyboard::create();
    eventListener->onKeyPressed = [=](EventKeyboard::KeyCode keyCode, Event* event) {
        if (_keyHold.find(keyCode) == _keyHold.end()) {
            _keyHold[keyCode] = std::chrono::high_resolution_clock::now();
        }
    };
    eventListener->onKeyReleased = [=](EventKeyboard::KeyCode keyCode, Event* event) {
        _keyHold.erase(keyCode);
    };
    this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, this);
}

bool GameScene::isKeyHeld(EventKeyboard::KeyCode code) {
    return _keyHold.count(code);
}

void GameScene::initMouse()
{
    auto s = Director::getInstance()->getVisibleSize();
    _mouseLastLoc = Vec2(s.width / 2, s.height / 2); // To avoid screen scrolling just after startup
    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseMove = CC_CALLBACK_1(GameScene::onMouseMove, this);
    mouseListener->onMouseUp = CC_CALLBACK_1(GameScene::onMouseUp, this);
    mouseListener->onMouseDown = CC_CALLBACK_1(GameScene::onMouseDown, this);
    mouseListener->onMouseScroll = CC_CALLBACK_1(GameScene::onMouseWheel, this);
    _eventDispatcher->addEventListenerWithFixedPriority(mouseListener, 1);
}

void GameScene::onMouseMove(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    Vec2 screenLoc = e->getLocationInView();
    if (e->getMouseButton() == 2) {
        onViewPan(screenLoc);
    }

    _mouseLastLoc = screenLoc;
//    CCLOG("MOUSEMOVE button# %d scrX# %f scrY# %f", e->getMouseButton(), screenLoc.x, screenLoc.y);
}

void GameScene::onMouseDown(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    if (e->getMouseButton() == 2) {
        onViewPan(e->getLocationInView());
    }
//    CCLOG("MOUSEDOWN button# %d", e->getMouseButton());
}

void GameScene::onMouseUp(Event *event)
{
    EventMouse* e = (EventMouse*)event;
    if (e->getMouseButton() == 0) {
        auto cs = ColonyShip::create(this);
        cs->setPosition(screen2world(e->getLocationInView()));
        cs->onLandCreate = [](GameScene* game) {
            return Tank::create(game);
        };
    } else if (e->getMouseButton() == 1) {
        if (isKeyHeld(EventKeyboard::KeyCode::KEY_CTRL)) {
            viewFollow(screen2world(e->getLocationInView()));
        }
    } else if (e->getMouseButton() == 2) {
        onViewPanStop();
    }
//    CCLOG("MOUSEUP button# %d", e->getMouseButton());
}

void GameScene::onMouseWheel(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    if (e->getScrollY() != 0.0) {
        onViewZoom(e->getScrollY(), screen2world(e->getLocationInView()));
    }
    if (e->getScrollX() != 0.0) {
        onViewRotate(-e->getScrollX(), screen2world(e->getLocationInView()));
    }
    CCLOG("MOUSEWHEEL button# %d scrollX# %f scrollY# %f", e->getMouseButton(), e->getScrollX(), e->getScrollY());
}

void GameScene::initWorldView()
{
    createWorldCamera(Vec2::ZERO);
    this->schedule(schedule_selector(GameScene::onViewTimer), _viewTimerIntervalSec);
}

void GameScene::createWorldCamera(Vec2 eye)
{
    auto s = Director::getInstance()->getVisibleSize();
    _worldCameraSize = Vec2(s.width * _viewZoom, s.height * _viewZoom);
    _worldCamera = Camera::createOrthographic(
        _worldCameraSize.x, _worldCameraSize.y,
        _viewNearPlane, _viewFarPlane
    );
    _worldCamera->setCameraFlag(CameraFlag::USER1);
    _worldCamera->setPositionZ(1.0f);
    viewLookAt(eye, false);
    addChild(_worldCamera);
}

Vec2 GameScene::viewCenter() const
{
    Vec2 eye = _worldCamera->getPosition();
    return eye + (_worldCameraSize / 2.0).rotate(Vec2::forAngle(_viewRotation - M_PI_2));
}

void GameScene::viewEyeAt(Vec2 eye)
{
    _worldCamera->setPosition(eye);
    _worldCamera->lookAt(Vec3(eye.x, eye.y, 0.0f), Vec3(cosf(_viewRotation), sinf(_viewRotation), 0.0f));
}

void GameScene::viewLookAt(Vec2 eye, bool continuos)
{
    Vec2 prevCenter = viewCenter();
    viewEyeAt(eye);
    if (_viewSurfaceId) {
        viewSurface(viewCenter(), prevCenter, continuos, false);
    }
}

void GameScene::viewCenterAt(Vec2 center)
{
    viewEyeAt(center - (_worldCameraSize/2.0).rotate(Vec2::forAngle(_viewRotation - M_PI_2)));
}

void GameScene::viewZoom(float scaleBy, Vec2 center)
{
    Vec2 eye = _worldCamera->getPosition();
    Vec2 offs = eye - center;
    _viewZoom *= scaleBy;
    offs *= scaleBy;
    Vec2 eyeNew = center + offs;
    _worldCamera->removeFromParent();
    createWorldCamera(eyeNew);
}

void GameScene::viewRotate(float rotateBy, Vec2 center)
{
    Vec2 eye = _worldCamera->getPosition();
    Vec2 offs = eye - center;
    _viewRotation += rotateBy;
    offs = offs.rotate(Vec2::forAngle(rotateBy));
    Vec2 eyeNew = center + offs;
    viewLookAt(eyeNew, false);
}

void GameScene::viewSurface(Vec2 center, Vec2 prevCenter, bool continuos, bool zoomIfRequired)
{
    if (!_viewSurfaceId) {
        return;
    }
    if (AstroObj* aobj = _objs->getByIdAs<AstroObj>(_viewSurfaceId)) {
        if (continuos) {
            // For enforcing constant altitude during horizontal scrolling
            Vec2 prevUp = prevCenter - aobj->getNode()->getPosition();
            if (!prevUp.isSmall()) {
                Vec2 delta = center - prevCenter;
                Vec2 upNormal = prevUp.getNormalized();
                Vec2 delta1 = delta.unrotate(upNormal);
                float rb = prevUp.length();
                float r = rb + delta1.x;
                float a = delta1.y / rb; // We assume that angle is small
                Vec2 newUp1 = r * Vec2::forAngle(a);
                Vec2 newUp = newUp1.rotate(upNormal);
                center = newUp + aobj->getNode()->getPosition();
            }
        }
        Vec2 up = center - aobj->getNode()->getPosition();
        if (up.isSmall()) {
            viewCenterAt(center);
        } else {
            float ratio = up.length() / (_worldCameraSize.y / 2.0);
            bool zoomRequired = ratio < 1.0;
            if (zoomRequired && zoomIfRequired) {
                float scaleBy = ratio / 2;
                viewZoom(scaleBy, center);
                zoomRequired = false;
            }
            if (!zoomRequired) {
                _viewRotation = up.getAngle();
                viewCenterAt(center);
                return;
            }
        }
    }
    _viewSurfaceId = 0;
}

void GameScene::viewFollow(Vec2 p)
{
    _pworld->queryPoint(
        CC_CALLBACK_3(GameScene::onViewFollowQueryPoint, this),
        p, nullptr
    );
    viewSurface(p, Vec2::ZERO, false, true);
}

void GameScene::viewFollow(Vec2 p, Obj* obj)
{
    if (obj) {
        if (obj->getObjType() == ObjType::AstroObj) {
            _viewSurfaceId = obj->getId();
            viewSurface(p, Vec2::ZERO, false, true);
        }
    } else {
        _viewSurfaceId = 0;
    }
}

bool GameScene::onViewFollowQueryPoint(PhysicsWorld& pworld, PhysicsShape& shape, void* userdata)
{
    UNUSED(pworld);
    UNUSED(userdata);
    ObjTag tag(shape.getBody()->getNode()->getTag());
    if (tag.type() == ObjType::AstroObj) {
        _viewSurfaceId = tag.id();
    }
    return true;
}

void GameScene::onViewPan(Vec2 screenLoc)
{
    if (!_viewPanEnabled) {
        _viewPanLastLoc = screenLoc;
        _viewPanEnabled = true;
    }
    Vec2 eye = _worldCamera->getPosition() - screen2world(screenLoc) + screen2world(_viewPanLastLoc);
    _viewPanLastLoc = screenLoc;
    viewLookAt(Vec2(eye.x, eye.y), true);
}

void GameScene::onViewPanStop()
{
    _viewPanEnabled = false;
}

void GameScene::onViewScroll(Vec2 screenDir)
{
    Vec2 eyePrev = _worldCamera->getPosition();
    Vec2 eye = eyePrev + screen2world(screenDir * _viewScrollFactor) - screen2world(Vec2::ZERO);
    viewLookAt(Vec2(eye.x, eye.y), true);
}

void GameScene::onViewZoom(float times, Vec2 center)
{
    float scaleBy = powf(_viewZoomFactor, times);
    viewZoom(scaleBy, center);
}

void GameScene::onViewRotate(float times, Vec2 center)
{
    if (_viewSurfaceId) {
        return; // Rotation in surface view is disabled
    }
    viewRotate(times * _viewRotationFactor, center);
}

void GameScene::onViewTimer(float dt)
{
    auto s = Director::getInstance()->getVisibleSize();
    float marginLeft = 5.0, marginRight = 5.0, marginTop = 50.0, marginBottom = 5.0;
    Vec2 screenDir;
    if (_mouseLastLoc.x <= marginLeft) {
        screenDir = Vec2(-1.0, 0.0);
    } else if (_mouseLastLoc.x >= s.width - marginRight) {
        screenDir = Vec2(1.0, 0.0);
    }
    if (_mouseLastLoc.y <= marginBottom) {
        screenDir = Vec2(0.0, -1.0);
    } else if (_mouseLastLoc.y >= s.height - marginTop) {
        screenDir = Vec2(0.0, 1.0);
    }
    if (screenDir != Vec2::ZERO) {
        onViewScroll(dt * screenDir);
    }
}

Vec2 GameScene::screen2world(Vec2 s)
{
    Vec3 w = _worldCamera->unprojectGL(Vec3(s.x, s.y, 0.0f));
    return Vec2(w.x, w.y);
}

void GameScene::initPlayers()
{
}

void GameScene::addPlayer(Player* player)
{
    _players.push_back(player);
    player->playerId = _players.size();
}

void GameScene::activatePlayer(Player* player)
{
    _activePlayer = player;
}

void GameScene::initCollisions()
{
    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = CC_CALLBACK_1(GameScene::onContactBegin, this);
    contactListener->onContactPreSolve = CC_CALLBACK_2(GameScene::onContactPreSolve, this);
    contactListener->onContactPostSolve = CC_CALLBACK_2(GameScene::onContactPostSolve, this);
    contactListener->onContactSeparate = CC_CALLBACK_1(GameScene::onContactSeparate, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);
}


bool GameScene::onContactBegin(PhysicsContact& contact)
{
//    CCLOG("onContactBegin: ");
    return dispatchContact(contact, nullptr, nullptr);
}

bool GameScene::onContactPreSolve(PhysicsContact& contact, PhysicsContactPreSolve& solve)
{
//    CCLOG("onContactPreSolve: fr# %f re# %f v# %f", solve.getFriction(), solve.getRestitution(), solve.getSurfaceVelocity().length());
    return dispatchContact(contact, &solve, nullptr);
}

void GameScene::onContactPostSolve(PhysicsContact& contact, const PhysicsContactPostSolve& solve)
{
    //CCLOG("onContactPostSolve: fr# %f re# %f v# %f", solve.getFriction(), solve.getRestitution(), solve.getSurfaceVelocity().length());
    dispatchContact(contact, nullptr, &solve);
}

void GameScene::onContactSeparate(PhysicsContact& contact)
{
//    CCLOG("onContactSeparate: ");
    dispatchContact(contact, nullptr, nullptr);
}

bool GameScene::dispatchContact(PhysicsContact& contact,
                                PhysicsContactPreSolve* preSolve,
                                const PhysicsContactPostSolve* postSolve)
{
    auto nodeA = contact.getShapeA()->getBody()->getNode();
    auto nodeB = contact.getShapeB()->getBody()->getNode();

    if (!nodeA || !nodeB) {
        return false; // avoid contact handling after node destruction
    }

    ContactInfo cinfo(this, contact, preSolve, postSolve);

    if (!cinfo.thisObj || !cinfo.thatObj) {
        return false; // avoid contact handling after obj destruction
    }

#define VG_CHECKCOLLISION(x, y) \
    if (cinfo.thisTag.type() == ObjType::x && cinfo.thatTag.type() == ObjType::y) { \
        return onContact ## x ## y (cinfo); \
    } \
    if (cinfo.thisTag.type() == ObjType::y && cinfo.thatTag.type() == ObjType::x) { \
        cinfo.swap(); \
        return onContact ## x ## y(cinfo); \
    } \
    /**/
    VG_CHECKCOLLISION(Unit, AstroObj);
    VG_CHECKCOLLISION(Projectile, AstroObj);
    VG_CHECKCOLLISION(Projectile, Unit);
#undef VG_CHECKCOLLISION

    return true;
}

bool GameScene::onContactUnitAstroObj(ContactInfo& cinfo)
{
    Unit* unit = static_cast<Unit*>(cinfo.thisObj);
    AstroObj* aobj = static_cast<AstroObj*>(cinfo.thatObj);
    switch (cinfo.contact.getEventCode()) {
    case PhysicsContact::EventCode::NONE:
        break;
    case PhysicsContact::EventCode::BEGIN:
        if (unit->surfaceId && unit->surfaceId != aobj->getId()) {
            // TODO[fate]: double contact with astro obj -- destroy unit
            return false;
        }
        unit->surfaceId = aobj->getId();
        cinfo.thisBody->setUpdateVelocityFunc(CC_CALLBACK_2(GameScene::updateUnitVelocityOnSurface, this));
        break;
    case PhysicsContact::EventCode::PRESOLVE:
        break;
    case PhysicsContact::EventCode::POSTSOLVE:
        break;
    case PhysicsContact::EventCode::SEPARATE:
        unit->surfaceId = 0;
        cinfo.thisBody->resetUpdateVelocityFunc();
        break;
    }

    if (unit->listenContactAstroObj) {
        unit->onContactAstroObj(cinfo);
    }
    return true;
}

bool GameScene::onContactProjectileAstroObj(ContactInfo& cinfo)
{
    Projectile* proj = static_cast<Projectile*>(cinfo.thisObj);
    return proj->onContactAstroObj(cinfo);
}

bool GameScene::onContactProjectileUnit(ContactInfo& cinfo)
{
    Projectile* proj = static_cast<Projectile*>(cinfo.thisObj);
    return proj->onContactUnit(cinfo);
}

void GameScene::updateUnitVelocityOnSurface(cpBody* body, float dt)
{
    cc::PhysicsBody* unitBody = static_cast<cc::PhysicsBody*>(body->userData);
    cc::Node* unitNode = unitBody->getNode();
    ObjTag tag(unitNode->getTag());
    Unit* unit = _objs->getByIdAs<Unit>(tag.id());
    CCASSERT(unit, "unit has been already destroyed");
    if (unit->surfaceId) {
        if (AstroObj* surface = _objs->getByIdAs<AstroObj>(unit->surfaceId)) {
            // TODO[fate]: calculate normal force using astroobj's gravity and apply
            // force of friction (rolling resistance) instead of this:
            float w_surf = surface->getNode()->getPhysicsBody()->getAngularVelocity();
            body->w -= (body->w - w_surf) * (1 - cpfpow(0.1, dt));
        } else {
            // surface aobj was destroyed
            unit->surfaceId = 0;
            unitBody->resetUpdateVelocityFunc();
        }
    }
}

void GameScene::initGalaxy()
{
    auto pl = Planet::create(this);
    pl->setPosition(Vec2::ZERO);

    // Starting location
    float startLng = random<float>(0.0, 360.0);
    auto seg = pl->segments().locateLng(startLng);
    float startAlt = seg->altitude;
    Vec2 start = pl->geogr2world(startLng, startAlt);
    viewFollow(start, pl);


//    auto pl = Planet::create(this);
//    pl->setPosition(Vec2(s.width/2 + o.x, s.height*1/4 + o.y));
//    pl->getNode()->getPhysicsBody()->applyImpulse(Vec2(4e7,0));
//    pl->getNode()->getPhysicsBody()->applyTorque(5e8);


//    auto pl2 = Planet::create(this);
//    pl2->setPosition(Vec2(s.width/2 + o.x, s.height*3/4 + o.y));
//    pl2->getNode()->getPhysicsBody()->applyImpulse(Vec2(-4e7,0));
//    pl2->getNode()->getPhysicsBody()->applyTorque(-5e8);

    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = [=](EventKeyboard::KeyCode keyCode, Event* event) {
        float t = 1e8;
        float j = 1e7;
        auto body = pl->getNode()->getPhysicsBody();
        switch (keyCode) {
        case EventKeyboard::KeyCode::KEY_A:
            body->applyTorque(t);
            break;
        case EventKeyboard::KeyCode::KEY_SPACE:
            for (auto& kv : *_objs) {
                if (auto tank = dynamic_cast<Tank*>(kv.second)) {
                    tank->shoot();
                }
            }
            break;
        case EventKeyboard::KeyCode::KEY_S:
            body->applyTorque(-t);
            break;
        case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
            body->applyImpulse(body->world2Local(Vec2(-j, 0)));
            break;
        case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
            body->applyImpulse(body->world2Local(Vec2(j, 0)));
            break;
        case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
            body->applyImpulse(body->world2Local(Vec2(0, -j)));
            break;
        case EventKeyboard::KeyCode::KEY_UP_ARROW:
            body->applyImpulse(body->world2Local(Vec2(0, j)));
            break;
        case EventKeyboard::KeyCode::KEY_Q:
            if (isKeyHeld(EventKeyboard::KeyCode::KEY_CTRL)) {
                menuCloseCallback(this);
            }
            break;
        default:
            break;
        }
    };
    _eventDispatcher->addEventListenerWithFixedPriority(keyboardListener, 1);
}
