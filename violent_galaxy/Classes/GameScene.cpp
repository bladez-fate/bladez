#include "GameScene.h"
#include "SimpleAudioEngine.h"
#include "chipmunk/chipmunk_private.h"

USING_NS_CC;

void GameScene::updateUnitVelocityOnSurface(cpBody* body, float dt)
{
    cc::PhysicsBody* unitBody = static_cast<cc::PhysicsBody*>(body->userData);
    cc::Node* unitNode = unitBody->getNode();
    ObjTag tag(unitNode->getTag());
    Unit* unit = _objs->getByIdAs<Unit>(tag.id());
    CCASSERT(unit, "unit has been already destroyed");
    if (unit->surfaceId) {
        if (AstroObj* surface = _objs->getByIdAs<AstroObj>(unit->surfaceId)) {
            float w_surf = surface->getNode()->getPhysicsBody()->getAngularVelocity();
            body->w -= (body->w - w_surf) * (1 - cpfpow(0.1, dt));
        } else {
            // surface aobj was destroyed
            unit->surfaceId = 0;
            unitBody->resetUpdateVelocityFunc();
        }
    }
}

Scene* GameScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::createWithPhysics();
//    scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
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

void GameScene::addPlayer(Player* player)
{
    _players.push_back(player);
    player->playerId = _players.size();
}

// on "init" you need to initialize your instance
bool GameScene::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(GameScene::menuCloseCallback, this));
    
    closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    // create and initialize a label
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
    auto s = Director::getInstance()->getVisibleSize();
    Vec2 o = Director::getInstance()->getVisibleOrigin();

    // Initialize world camera
    createWorldCamera(Vec2::ZERO);

    _pworld = pworld;
    pworld->setSpeed(1.0);

//    auto pl = Planet::create(this);
//    pl->setPosition(Vec2(s.width/2 + o.x, s.height/2 + o.y));

//    auto cs = ColonyShip::create(this);
//    cs->setPosition(Vec2(visibleSize.width/4 + origin.x, visibleSize.height/2 + origin.y));
//    cs->getNode()->getPhysicsBody()->applyImpulse(Vec2(0, 30));

    auto pl = Planet::create(this);
    pl->setPosition(Vec2(s.width/2 + o.x, s.height*1/4 + o.y));
    pl->getNode()->getPhysicsBody()->applyImpulse(Vec2(4e7,0));
    pl->getNode()->getPhysicsBody()->applyTorque(5e8);


    auto pl2 = Planet::create(this);
    pl2->setPosition(Vec2(s.width/2 + o.x, s.height*3/4 + o.y));
    pl2->getNode()->getPhysicsBody()->applyImpulse(Vec2(-4e7,0));
    pl2->getNode()->getPhysicsBody()->applyTorque(-5e8);

//    auto cs = ColonyShip::create(this);
//    cs->setPosition(Vec2(visibleSize.width*0.3 + origin.x, visibleSize.height*0.8 + origin.y));
//    cs->getNode()->getPhysicsBody()->applyImpulse(Vec2(0, 30));
//    cs->onLandCreate = [](GameScene* game) {
//        return Tank::create(game);
//    };

    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = CC_CALLBACK_1(GameScene::onContactBegin, this);
    contactListener->onContactPreSolve = CC_CALLBACK_2(GameScene::onContactPreSolve, this);
    contactListener->onContactPostSolve = CC_CALLBACK_2(GameScene::onContactPostSolve, this);
    contactListener->onContactSeparate = CC_CALLBACK_1(GameScene::onContactSeparate, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);

    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseMove = CC_CALLBACK_1(GameScene::onMouseMove, this);
    mouseListener->onMouseUp = CC_CALLBACK_1(GameScene::onMouseUp, this);
    mouseListener->onMouseDown = CC_CALLBACK_1(GameScene::onMouseDown, this);
    mouseListener->onMouseScroll = CC_CALLBACK_1(GameScene::onMouseScroll, this);
    _eventDispatcher->addEventListenerWithFixedPriority(mouseListener, 1);

    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = [=](EventKeyboard::KeyCode keyCode, Event* event) {
        float t = 1e8;
        float j = 1e7;
        auto body = pl->getNode()->getPhysicsBody();
        switch (keyCode) {
        case EventKeyboard::KeyCode::KEY_A:
            body->applyTorque(t);
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

    createKeyHoldHandler();
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
        return false; // avoid contact handling after destruction
    }

    ContactInfo cinfo(this, contact, preSolve, postSolve);

    if (cinfo.thisTag.type() == ObjType::Unit && cinfo.thatTag.type() == ObjType::Astro) {
        return onContactUnitAstroObj(cinfo);
    }
    if (cinfo.thisTag.type() == ObjType::Astro && cinfo.thatTag.type() == ObjType::Unit) {
        cinfo.swap();
        return onContactUnitAstroObj(cinfo);
    }

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

void GameScene::onMouseMove(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    if (e->getMouseButton() == 1) {
        onViewPan(e->getLocationInView());
    }
//    CCLOG("MOUSEMOVE button# %d", e->getMouseButton());
}

void GameScene::onMouseDown(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    if (e->getMouseButton() == 1) {
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
    }
    if (e->getMouseButton() == 1) {
        onViewPanStop();
    }
//    CCLOG("MOUSEUP button# %d", e->getMouseButton());
}

void GameScene::onMouseScroll(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    if (e->getScrollY() != 0.0) {
        onViewZoom(-e->getScrollY(), screen2world(e->getLocationInView()));
    }
    if (e->getScrollX() != 0.0) {
        onViewRotate(-e->getScrollX(), screen2world(e->getLocationInView()));
    }
    CCLOG("MOUSESCROLL button# %d scrollX# %f scrollY# %f", e->getMouseButton(), e->getScrollX(), e->getScrollY());
}

void GameScene::viewLookAt(Vec2 eye)
{
    _worldCamera->setPosition(eye);
    _worldCamera->lookAt(Vec3(eye.x, eye.y, 0.0f), Vec3(cosf(_viewRotation), sinf(_viewRotation), 0.0f));
}

void GameScene::onViewPan(Vec2 loc)
{
    if (!_viewPanEnabled) {
        _viewPanLastLoc = loc;
        _viewPanEnabled = true;
    }
    Vec3 eye = _worldCamera->getPosition3D()
            - _worldCamera->unprojectGL(Vec3(loc.x, loc.y, 0.0f))
            + _worldCamera->unprojectGL(Vec3(_viewPanLastLoc.x, _viewPanLastLoc.y, 0.0f));
    _viewPanLastLoc = loc;
    viewLookAt(Vec2(eye.x, eye.y));
}

void GameScene::onViewPanStop()
{
    _viewPanEnabled = false;
}

void GameScene::createWorldCamera(Vec2 eye)
{
    auto s = Director::getInstance()->getVisibleSize();
    _worldCamera = Camera::createOrthographic(s.width * _viewZoom, s.height * _viewZoom, _viewNearPlane, _viewFarPlane);
    _worldCamera->setCameraFlag(CameraFlag::USER1);
    _worldCamera->setPositionZ(1.0f);
    viewLookAt(eye);
    addChild(_worldCamera);
}

void GameScene::onViewZoom(float times, Vec2 center)
{
    Vec2 eye = _worldCamera->getPosition();
    Vec2 offs = eye - center;
    float scaleBy = powf(_viewZoomFactor, times);
    _viewZoom *= scaleBy;
    offs *= scaleBy;
    Vec2 eyeNew = center + offs;
    _worldCamera->removeFromParent();
    createWorldCamera(eyeNew);
}

void GameScene::onViewRotate(float times, Vec2 center)
{
    Vec2 eye = _worldCamera->getPosition();
    Vec2 offs = eye - center;
    float rotateBy = times * _viewRotationFactor;
    _viewRotation += rotateBy;
    offs = offs.rotate(Vec2::forAngle(rotateBy));
    Vec2 eyeNew = center + offs;
    viewLookAt(eyeNew);
}

Vec2 GameScene::screen2world(Vec2 s)
{
    Vec3 w = _worldCamera->unprojectGL(Vec3(s.x, s.y, 0.0f));
    return Vec2(w.x, w.y);
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
