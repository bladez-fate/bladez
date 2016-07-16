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
//    scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL, (unsigned short)gWorldCameraFlag);
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
    scheduleUpdate();

    return true;
}

void GameScene::update(float delta)
{
    Layer::update(delta);
    playerUpdate(delta);
    keyboardUpdate(delta);
    view.update(delta);
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

    view.init(this);
    initGalaxy();
    initCollisions();

    initPlayers();
    initMouse();
    initKeyboard();
}

void GameScene::initKeyboard()
{
    createKeyHoldHandler();
    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = [=](EventKeyboard::KeyCode keyCode, Event* event) {
        switch (keyCode) {
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

void GameScene::keyboardUpdate(float delta)
{
    if (_activePlayer) {
        for (Id id : _activePlayer->selected) {
            if (auto obj = objs()->getById(id)) {
                if (auto tank = dynamic_cast<Tank*>(obj)) {
                    if (isKeyHeld(EventKeyboard::KeyCode::KEY_UP_ARROW)) {
                        tank->upAngle(delta);
                    }
                    if (isKeyHeld(EventKeyboard::KeyCode::KEY_DOWN_ARROW)) {
                        tank->downAngle(delta);
                    }
                    tank->moveLeft(isKeyHeld(EventKeyboard::KeyCode::KEY_LEFT_ARROW));
                    tank->moveRight(isKeyHeld(EventKeyboard::KeyCode::KEY_RIGHT_ARROW));
                }
            }
        }
    }
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
    this->schedule(schedule_selector(GameScene::onMouseTimer), _mouseTimerIntervalSec);
}

void GameScene::onMouseMove(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    Vec2 screenLoc = e->getLocationInView();
    if (e->getMouseButton() == 2) {
        mousePan(screenLoc);
    }

    _mouseLastLoc = screenLoc;
//    CCLOG("MOUSEMOVE button# %d scrX# %f scrY# %f", e->getMouseButton(), screenLoc.x, screenLoc.y);
}

void GameScene::onMouseDown(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    if (e->getMouseButton() == 2) {
       mousePan(e->getLocationInView());
    }
//    CCLOG("MOUSEDOWN button# %d", e->getMouseButton());
}

void GameScene::onMouseUp(Event *event)
{
    EventMouse* e = (EventMouse*)event;
    Vec2 p = e->getLocationInView();
    Vec2 pw = view.screen2world(p);
    if (e->getMouseButton() == 0) {
        if (isKeyHeld(EventKeyboard::KeyCode::KEY_C)) {
            auto dc = DropCapsid::create(this);
            dc->setPosition(pw);
            dc->onLandCreate = [](GameScene* game) {
                return Tank::create(game);
            };
        } else if (isKeyHeld(EventKeyboard::KeyCode::KEY_V)) {
            auto ss = SpaceStation::create(this);
            ss->setPosition(pw);
            for (auto kv : *_objs) {
                if (AstroObj* ao = dynamic_cast<AstroObj*>(kv.second)) {
                    PhysicsBody* gsource = ao->getNode()->getPhysicsBody();
                    Vec2 p = ss->getNode()->getPhysicsBody()->getPosition();
                    Vec2 o = gsource->getPosition();
                    Vec2 g = _pworld->getForceField()->getBodyGravity(gsource, p);
                    Vec2 r = p - o;
                    // For orbital velocity: g = v*v/r  =>  v = sqrt(g*r)
                    float v = sqrtf(g.length() * r.length());
                    ss->getNode()->getPhysicsBody()->setVelocity(
                        v * r.getNormalized().rotate(Vec2::forAngle(M_PI_2))
                    );
                    break; // TODO[fate]: find nearest planet
                }
            }
        } else {
            playerSelect(pw,
                isKeyHeld(EventKeyboard::KeyCode::KEY_SHIFT),
                isKeyHeld(EventKeyboard::KeyCode::KEY_CTRL)
            );
        }
    } else if (e->getMouseButton() == 1) {
        if (isKeyHeld(EventKeyboard::KeyCode::KEY_CTRL)) {
            view.follow(pw);
        }
    } else if (e->getMouseButton() == 2) {
        mousePanStop();
    }
//    CCLOG("MOUSEUP button# %d", e->getMouseButton());
}

void GameScene::onMouseWheel(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    if (e->getScrollY() != 0.0) {
        view.onZoom(e->getScrollY(), view.screen2world(e->getLocationInView()));
    }
    if (e->getScrollX() != 0.0) {
        view.onRotate(-e->getScrollX(), view.screen2world(e->getLocationInView()));
    }
    CCLOG("MOUSEWHEEL button# %d scrollX# %f scrollY# %f", e->getMouseButton(), e->getScrollX(), e->getScrollY());
}

void GameScene::onMouseTimer(float dt)
{
    auto s = Director::getInstance()->getVisibleSize();
    float marginLeft = 5.0, marginRight = 5.0, marginTop = 50.0, marginBottom = 5.0;
    Vec2 screenDir;
    if (_mouseLastLoc.x <= marginLeft) {
        screenDir += Vec2(-1.0, 0.0);
    } else if (_mouseLastLoc.x >= s.width - marginRight) {
        screenDir += Vec2(1.0, 0.0);
    }
    if (_mouseLastLoc.y <= marginBottom) {
        screenDir += Vec2(0.0, -1.0);
    } else if (_mouseLastLoc.y >= s.height - marginTop) {
        screenDir += Vec2(0.0, 1.0);
    }
    if (screenDir != Vec2::ZERO) {
        view.onScroll(dt * _mouseScrollFactor * screenDir);
    }
}

void GameScene::mousePan(Vec2 screenLoc)
{
    if (!_mousePanEnabled) {
        _mousePanLastLoc = screenLoc;
        _mousePanEnabled = true;
    }
    view.onScroll(_mousePanLastLoc - screenLoc);
    _mousePanLastLoc = screenLoc;
}

void GameScene::mousePanStop()
{
    _mousePanEnabled = false;
}

void GameScene::initPlayers()
{
}

void GameScene::playerUpdate(float delta)
{
    for (Player* player : _players) {
        player->update(delta);
    }
}

void GameScene::addPlayer(Player* player)
{
    _players.push_back(player);
    player->playerId = _players.size();
}

void GameScene::playerActivate(Player* player)
{
    if (player == _activePlayer) {
        return;
    }
    if (_activePlayer) {
        _activePlayer->drawSelection(false);
    }
    _activePlayer = player;
    if (_activePlayer) {
        _activePlayer->drawSelection(true);
    }
}

void GameScene::playerSelect(Vec2 p, bool add, bool all)
{
    if (!_activePlayer) {
        return;
    }
    _pworld->queryPoint(
        CC_CALLBACK_3(GameScene::onSelectQueryPoint, this, add, all),
        p, nullptr
    );
}

bool GameScene::onSelectQueryPoint(PhysicsWorld& pworld, PhysicsShape& shape, void* userdata, bool add, bool all)
{
    UNUSED(pworld);
    UNUSED(userdata);
    UNUSED(all);
    ObjTag tag(shape.getBody()->getNode()->getTag());
    if (tag.type() == ObjType::Unit) {
        if (add) {
            _activePlayer->selectAdd(tag.id());
        } else {
            _activePlayer->select(tag.id());
        }
    }
    return false;
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
    view.follow(start, pl);

    // Player
    auto player = Player::create(this);
    player->name = "Player1";
    player->color = Color4F::RED;
    playerActivate(player);

    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = [=](EventKeyboard::KeyCode keyCode, Event* event) {
//        float t = 1e8;
//        float j = 1e7;
//        auto body = pl->getNode()->getPhysicsBody();
//        switch (keyCode) {
//        case EventKeyboard::KeyCode::KEY_A:
//            body->applyTorque(t);
//            break;
//        case EventKeyboard::KeyCode::KEY_S:
//            body->applyTorque(-t);
//            break;
//        case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
//            body->applyImpulse(body->world2Local(Vec2(-j, 0)));
//            break;
//        case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
//            body->applyImpulse(body->world2Local(Vec2(j, 0)));
//            break;
//        case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
//            body->applyImpulse(body->world2Local(Vec2(0, -j)));
//            break;
//        case EventKeyboard::KeyCode::KEY_UP_ARROW:
//            body->applyImpulse(body->world2Local(Vec2(0, j)));
//            break;
//        case EventKeyboard::KeyCode::KEY_Q:
//            if (isKeyHeld(EventKeyboard::KeyCode::KEY_CTRL)) {
//                menuCloseCallback(this);
//            }
//            break;
//        default:
//            break;
//        }
        if (_activePlayer) {
            for (Id id : _activePlayer->selected) {
                if (auto obj = objs()->getById(id)) {
                    if (auto tank = dynamic_cast<Tank*>(obj)) {
                        int repeat = isKeyHeld(EventKeyboard::KeyCode::KEY_SHIFT)? 10: 1;
                        while (repeat--) {
                            switch (keyCode) {
                            case EventKeyboard::KeyCode::KEY_SPACE:
                                tank->shoot();
                                repeat = 0;
                                break;
                            case EventKeyboard::KeyCode::KEY_A:
                                tank->addPower();
                                break;
                            case EventKeyboard::KeyCode::KEY_Z:
                                tank->subPower();
                                break;
                            default:
                                break;
                            }
                        }
                    }
                }
            }
        }
    };
    _eventDispatcher->addEventListenerWithFixedPriority(keyboardListener, 1);
}
