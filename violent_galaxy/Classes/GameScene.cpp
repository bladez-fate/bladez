#include "GameScene.h"
#include <base/ccRandom.h>
#include <chipmunk/chipmunk_private.h>
#include <SimpleAudioEngine.h>
#include "Projectiles.h"
#include "Buildings.h"

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

void GameScene::addDeadObj(Obj* obj)
{
     _deadObjs.insert(obj);
}

bool GameScene::init()
{
    if (!Layer::init()) {
        return false;
    }
    
//    auto s = Director::getInstance()->getVisibleSize();
//    Vec2 o = Director::getInstance()->getVisibleOrigin();

//    auto closeItem = MenuItemImage::create(
//        "CloseNormal.png",
//        "CloseSelected.png",
//        CC_CALLBACK_1(GameScene::menuCloseCallback, this)
//    );
//    closeItem->setPosition(Vec2(o.x + s.width - closeItem->getContentSize().width/2 ,
//                                o.y + closeItem->getContentSize().height/2));

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
    // Remove dead objs
    for (Obj* obj : _deadObjs) {
        obj->destroy();
    }
    _deadObjs.clear();

    // Tile grid
    _unitGrid.clear();
    for (auto kv : *_objs) {
        Obj* obj = kv.second;
        if (Unit* unit = dynamic_cast<Unit*>(obj)) {
            Vec2 p = unit->getNode()->getPhysicsBody()->getPosition();
            _unitGrid.add(unit, p, unit->getSize() / 2);
        }
    }

    // Push overlapping units
    for (auto kv : *_objs) {
        Obj* obj = kv.second;
        if (Unit* u = dynamic_cast<Unit*>(obj)) {
            auto body = u->getNode()->getPhysicsBody();
            Vec2 p = body->getPosition();
            float radius = u->getSize() / 2;
            u->sepDir = Vec2::ZERO;
            _unitGrid.query(p, u->getSize() / 2, [=] (Unit* u2, Vec2 p2, float radius2, float distSq) -> bool {
                if (u == u2) {
                    return true;
                }
                auto body2 = u2->getNode()->getPhysicsBody();
                Vec2 rv = body->getVelocity() - body2->getVelocity(); // relative velocity
                if (rv.lengthSquared() < gMaxSeparationVelocitySq) {
                    Vec2 d = p - p2;
                    d.normalize();
                    float maxDist = radius + radius2;
                    d *= std::max(0.0, std::min(1.0, 1.0 - distSq / maxDist / maxDist));
                    if (d.isSmall()) {
                        Vec2 xdir = body->local2World(Vec2::UNIT_X) - body->local2World(Vec2::ZERO);
                        Vec2 xdir2 = body2->local2World(Vec2::UNIT_X) - body2->local2World(Vec2::ZERO);
                        Vec2 sepAxis = xdir + xdir2;
                        sepAxis.normalize();
                        int order = (u->getId() < u2->getId()? -1: 1);
                        d = order * sepAxis;
                    }
                    u->sepDir += d;
                }
                return true;
            });
        }
    }

    // Update
    Layer::update(delta);
    playerUpdate(delta);
    keyboardUpdate(delta);

    for (auto kv : *_objs) {
        Obj* obj = kv.second;
        obj->update(delta);
    }

    _view.update(delta);
    guiUpdate(delta);
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

    _view.init(this);
    initGalaxy();
    initCollisions();

    initPlayers();
    initMouse();
    initKeyboard();

    initGui();
}

void GameScene::initKeyboard()
{
    createKeyHoldHandler();
    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = [=](EventKeyboard::KeyCode keyCode, Event* event) {
        switch (keyCode) {
        case EventKeyboard::KeyCode::KEY_Q:
            if (isKeyHeld(EventKeyboard::KeyCode::KEY_CTRL) && isKeyHeld(EventKeyboard::KeyCode::KEY_SHIFT)) {
                menuCloseCallback(this);
            }
            break;
        default:
            break;
        }

        // Player control keyh handling
        if (_activePlayer) {
            // Select army
            if (keyCode == gHKSelectArmy) {
                std::vector<Id> army;
                for (auto kv : *_objs) {
                    if (Unit* unit = dynamic_cast<Unit*>(kv.second)) {
                        if (unit->getPlayer() == _activePlayer) {
                            army.push_back(unit->getId());
                        }
                    }
                }
                if (!army.empty()) {
                    _activePlayer->selected = army;
                }
            }

            // Group management
            for (size_t idx = 0; idx < GRP_COUNT; idx++) {
                if (keyCode == gHKGroup[idx]) {
                    if (isKeyHeld(EventKeyboard::KeyCode::KEY_CTRL)) {
                        _activePlayer->setSelectionToGroup(idx);
                    } else if (isKeyHeld(EventKeyboard::KeyCode::KEY_SHIFT)) {
                        _activePlayer->addSelectionToGroup(idx);
                    } else {
                        _activePlayer->selectGroup(idx);
                        if (!_keyHistory.empty()) {
                            const KeyHistory& kh = _keyHistory.back();
                            // TODO[fate]: add timeout
                            //auto now = std::chrono::high_resolution_clock::now();
                            //if (kh.time < now && (now - kh.time).count() <= 1)
                            if (kh.spec.key == gHKGroup[idx] && kh.spec.mod.empty()) {
                                playerCenterSelection();
                            }
                        }
                    }
                    break;
                }
            }

            // Unit control
            for (Id id : _activePlayer->selected) {
                if (auto obj = objs()->getById(id)) {
                    if (auto tank = dynamic_cast<Tank*>(obj)) {
                        int repeat = isKeyHeld(EventKeyboard::KeyCode::KEY_SHIFT)? 10: 1;
                        while (repeat--) {
                            if (keyCode == gHKShoot) {
                                tank->shoot();
                                repeat = 0;
//                            } else if (keyCode == gHKPowerInc) {
//                                tank->addPower();
//                            } else if (keyCode == gHKPowerDec) {
//                                tank->subPower();
//                            } else if (keyCode == gHKGoBack) {
//                                tank->goBack();
//                            } else if (keyCode == gHKGoFront) {
//                                tank->goFront();
                            }
                        }
                    }
                }
            }
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);
//    _eventDispatcher->addEventListenerWithFixedPriority(keyboardListener, 1);
    createKeyHistoryHandler();
}

void GameScene::keyboardUpdate(float delta)
{
//    if (_activePlayer) {
//        for (Id id : _activePlayer->selected) {
//            if (auto obj = objs()->getById(id)) {
//                if (auto tank = dynamic_cast<Tank*>(obj)) {
//                    if (isKeyHeld(gHKAngleInc)) {
//                        tank->incAngle(delta);
//                    }
//                    if (isKeyHeld(gHKAngleDec)) {
//                        tank->decAngle(delta);
//                    }
//                    tank->moveLeft(isKeyHeld(gHKMoveLeft));
//                    tank->moveRight(isKeyHeld(gHKMoveRight));
//                }
//            }
//        }
//    }
}

void GameScene::createKeyHoldHandler()
{
    auto eventListener = EventListenerKeyboard::create();
    eventListener->onKeyPressed = [=](EventKeyboard::KeyCode keyCode, Event* event) {
        if (_keyHold.find(keyCode) == _keyHold.end()) {
            auto now = std::chrono::high_resolution_clock::now();
            _keyHold[keyCode] = now;
        }
    };
    eventListener->onKeyReleased = [=](EventKeyboard::KeyCode keyCode, Event* event) {
        _keyHold.erase(keyCode);
    };
    this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, this);
}

void GameScene::createKeyHistoryHandler()
{
    auto eventListener = EventListenerKeyboard::create();
    eventListener->onKeyPressed = [=](EventKeyboard::KeyCode keyCode, Event* event) {
        if (!KeySpec::isMod(keyCode)) {
            auto now = std::chrono::high_resolution_clock::now();
            while (_keyHistory.size() >= _keyHistorySize) {
                _keyHistory.pop_front();
            }
            _keyHistory.push_back(KeyHistory());
            KeyHistory& kh = _keyHistory.back();
            kh.time = now;
            kh.spec.key = keyCode;
            for (EventKeyboard::KeyCode mod : KeySpec::getKeyModList()) {
                if (isKeyHeld(mod)) {
                    kh.spec.mod.insert(mod);
                }
            }
        }
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

    _mouseSelectRectNode = DrawNode::create();
    this->addChild(_mouseSelectRectNode, gZOrderMouseSelectionRect);
}

void GameScene::onMouseMove(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    Vec2 screenLoc = e->getLocationInView();
    if (e->getMouseButton() == 0) {
        mouseSelectRect(screenLoc);
    }
    if (e->getMouseButton() == 2) {
        mousePan(screenLoc);
    }
    _mouseLastLoc = screenLoc;
}

void GameScene::onMouseDown(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    if (e->getMouseButton() == 0) {
       mouseSelectRect(e->getLocationInView());
    }
    if (e->getMouseButton() == 2) {
       mousePan(e->getLocationInView());
    }
}

void GameScene::onMouseUp(Event *event)
{
    EventMouse* e = (EventMouse*)event;
    Vec2 p = e->getLocationInView();
    Vec2 pw = _view.screen2world(p);
    if (e->getMouseButton() == 0) {
        bool doSelection = false;
        if (isKeyHeld(EventKeyboard::KeyCode::KEY_X)) {
//            auto dc = DropCapsid::create(this);
//            dc->setPosition(pw);
//            dc->onLandCreate = [](GameScene* game) {
//                return Tank2::create(game);
//            };
//            dc->setPlayer(_activePlayer);
        } else if (isKeyHeld(EventKeyboard::KeyCode::KEY_C)) {
            auto dc = DropCapsid::create(this);
            dc->setPosition(pw);
            dc->onLandCreate = [](GameScene* game) {
                return Tank::create(game);
            };
            dc->setPlayer(_activePlayer);
//        } else if (isKeyHeld(EventKeyboard::KeyCode::KEY_V)) {
//            auto ss = SpaceStation::create(this);
//            ss->setPosition(pw);
//            for (auto kv : *_objs) {
//                if (AstroObj* ao = dynamic_cast<AstroObj*>(kv.second)) {
//                    PhysicsBody* gsource = ao->getNode()->getPhysicsBody();
//                    Vec2 p = ss->getNode()->getPhysicsBody()->getPosition();
//                    Vec2 o = gsource->getPosition();
//                    Vec2 g = _pworld->getForceField()->getBodyGravity(gsource, p);
//                    Vec2 r = p - o;
//                    // For orbital velocity: g = v*v/r  =>  v = sqrt(g*r)
//                    float v = sqrtf(g.length() * r.length());
//                    ss->getNode()->getPhysicsBody()->setVelocity(
//                        v * r.getNormalized().rotate(Vec2::forAngle(M_PI_2))
//                    );
//                    break; // TODO[fate]: find nearest planet
//                }
//            }
//            ss->setPlayer(_activePlayer);
//        } else if (isKeyHeld(EventKeyboard::KeyCode::KEY_B)) {
//            auto fact = Factory::create(this);
//            fact->setPosition(pw);
//            for (auto kv : *_objs) {
//                if (Planet* planet = dynamic_cast<Planet*>(kv.second)) {
//                    auto pb = planet->getNode()->getPhysicsBody();

//                    // Set factory orientation upwards
//                    Vec2 rw = pw - pb->getPosition(); // building coords in world frame
//                    float dirw = rw.getAngle();
//                    fact->getNode()->setRotation(
//                        90 - CC_RADIANS_TO_DEGREES(dirw)
//                    );

//                    // Create platform body
//                    Vec2 rl = pb->world2Local(pw); // building coords in planet local frame
//                    float dirl = rl.getAngle();
//                    float len = 0.6*fact->getSize(); // half of platform length
//                    Vec2 bldFloor = (rl.length() - fact->getSize()/2) * rl.getNormalized(); // coordinates of building floor in body frame
//                    Vec2 rightEdge(bldFloor + len*Vec2::forAngle(dirl - M_PI_2));
//                    Vec2 leftEdge(bldFloor - len*Vec2::forAngle(dirl - M_PI_2));
//                    float foundationHeight = 50;
//                    planet->addPlatform(Platform(
//                        rightEdge,
//                        leftEdge,
//                        planet->altAng2local(planet->getAltitudeAt(leftEdge.getAngle()) - foundationHeight, leftEdge.getAngle()),
//                        planet->altAng2local(planet->getAltitudeAt(rightEdge.getAngle()) - foundationHeight, rightEdge.getAngle())
//                    ));
//                    break; // TODO[fate]: find nearest planet
//                }
//            }
//            fact->setPlayer(_activePlayer);
        } else {
            doSelection = true;
        }
        mouseSelectRectStop(p, doSelection);
    } else if (e->getMouseButton() == 1) {
        bool add = isKeyHeld(EventKeyboard::KeyCode::KEY_SHIFT);
        playerOrderPoint(p, add);
    } else if (e->getMouseButton() == 2) {
        if (isKeyHeld(EventKeyboard::KeyCode::KEY_CTRL)) {
            _view.act(_view.follow(pw, _mouseFollowDuration));
        }
        mousePanStop();
    }
}

void GameScene::onMouseWheel(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    if (e->getScrollY() != 0.0) {
        _view.act(_view.zoomTo(
            powf(_mouseZoomFactor, e->getScrollY()),
            _view.screen2world(e->getLocationInView()),
            _mouseViewActionDuration
        ));
        _view.act(_view.defaultView(_view.getCenter(), 5*_mouseViewActionDuration));
    }
    if (e->getScrollX() != 0.0) {
        if (!_view.isSurfaceView()) {
            _view.act(_view.rotateAround(
                -e->getScrollX() * _mouseRotationFactor,
                _view.screen2world(e->getLocationInView()),
                _mouseViewActionDuration
            ));
        }
    }
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
        _view.act(_view.screenScroll(dt * _mouseScrollFactor * screenDir, true, dt));
    }
}

void GameScene::mousePan(Vec2 screenLoc)
{
    if (!_mousePanEnabled) {
        _mousePanLastLoc = screenLoc;
        _mousePanEnabled = true;
    }
    _view.screenScroll(_mousePanLastLoc - screenLoc, true, 0.0f);
    _mousePanLastLoc = screenLoc;
}

void GameScene::mousePanStop()
{
    _mousePanEnabled = false;
}

void GameScene::mouseSelectRect(Vec2 screenLoc)
{
    if (!_mouseSelectRectEnabled) {
        _mouseSelectRectStart = screenLoc;
        _mouseSelectRectEnabled = true;
    }
    _mouseSelectRectEnd = screenLoc;
    _mouseSelectRectNode->clear();
    _mouseSelectRectNode->drawRect(
        _mouseSelectRectStart,
        _mouseSelectRectEnd,
        gSelectionColor
    );
}

void GameScene::mouseSelectRectStop(Vec2 screenLoc, bool apply)
{
    if (_mouseSelectRectEnabled) {
        _mouseSelectRectEnabled = false;
        _mouseSelectRectNode->clear();
        if (apply) {
            if (_mouseSelectRectStart != _mouseSelectRectEnd) {
                playerSelectRect(
                    _mouseSelectRectStart,
                    _mouseSelectRectEnd,
                    isKeyHeld(EventKeyboard::KeyCode::KEY_SHIFT)
                );
            } else {
                playerSelectPoint(
                    screenLoc,
                    isKeyHeld(EventKeyboard::KeyCode::KEY_SHIFT),
                    isKeyHeld(EventKeyboard::KeyCode::KEY_CTRL)
                );
            }
        }
    }
}

float GameScene::viewSelectionRadius(float size)
{
    return clampf(_view.getZoom() * 16.0f, size * 1.1f, 1000.0f);
}

void GameScene::initGui()
{
    _selectionPanel.init(this);
}

void GameScene::guiUpdate(float delta)
{
    if (!_guiIndicators) {
        _guiIndicators = DrawNode::create();
        this->addChild(_guiIndicators, gZOrderIndicators);
    }
    _guiIndicators->clear();
    for (auto kv : *_objs) {
        if (Unit* unit = dynamic_cast<Unit*>(kv.second)) {
            Vec2 pw = unit->getNode()->getPosition();
            float screenSize = rintf(unit->getSize() / _view.getZoom());
            if (screenSize >= 20.0f) {
                Vec2 p = _view.world2screen(pw) - Vec2(0, screenSize * 0.6f);
                Vec2 r = Vec2(rintf(p.x), rintf(p.y));
                float lx = rintf(screenSize / 2);
                float rx = rintf(screenSize - lx);
                // TODO[fate]: do not draw indicator off screen
                Vec2 p1 = r - Vec2(lx, 2.0f);
                Vec2 p2 = r + Vec2(rx, 2.0f);
                float share = (float)unit->hp / unit->hpMax;
                float mx = rintf(p1.x + screenSize * share);
                Color4F hpColor(share < gHpRedLevel? gHpRedColor: (share < gHpYellowLevel? gHpYellowColor: gHpGreenColor));
                _guiIndicators->drawSolidRect(
                    p1 - Vec2(1.0f, 1.0f), p2 + Vec2(1.0f, 1.0f),
                    gIndicatorBorderColor
                );
                if (p1.x < mx) {
                    _guiIndicators->drawSolidRect(
                        p1, Vec2(mx, p2.y),
                        hpColor
                    );
                }
                if (mx + 1.0f < p2.x) {
                    _guiIndicators->drawSolidRect(
                        Vec2(mx + 1.0f, p1.y), p2,
                        gHpBgColor
                    );
                }
            }
        }
        if (Building* building = dynamic_cast<Building*>(kv.second)) {
            Vec2 pw = building->getNode()->getPosition();
            float screenSize = rintf(building->getSize() / _view.getZoom());
            if (screenSize >= 20.0f) {
                Vec2 p = _view.world2screen(pw) + Vec2(0, screenSize * 0.6f);
                Vec2 r = Vec2(rintf(p.x), rintf(p.y));
                float lx = rintf(screenSize / 2);
                float rx = rintf(screenSize - lx);
                // TODO[fate]: do not draw indicator off screen
                Vec2 p1 = r - Vec2(lx, 2.0f);
                Vec2 p2 = r + Vec2(rx, 2.0f);
                float share = building->getProductionProgress();
                if (share > 0.0f) {
                    float mx = rintf(p1.x + screenSize * share);
                    _guiIndicators->drawSolidRect(
                        p1 - Vec2(1.0f, 1.0f), p2 + Vec2(1.0f, 1.0f),
                        gIndicatorBorderColor
                    );
                    if (p1.x < mx) {
                        _guiIndicators->drawSolidRect(
                            p1, Vec2(mx, p2.y),
                            gProdColor
                        );
                    }
                    if (mx + 1.0f < p2.x) {
                        _guiIndicators->drawSolidRect(
                            Vec2(mx + 1.0f, p1.y), p2,
                            gProdBgColor
                        );
                    }
                }
            }
        }
    }

    if (!_resIcons) {
        auto s = Director::getInstance()->getVisibleSize();
        _resIcons = DrawNode::create();
        this->addChild(_resIcons, gZOrderResIcons);
        float width = 80;
        float size = 14;
        float txtwidth = width - size - 4;
        for (int i = -1; i < (int)RES_COUNT; i++) {
            Vec2 p(s.width - width * (i+2), s.height - 15);
            auto l = Label::createWithTTF("0", "fonts/arial.ttf", size, Size(txtwidth, size));
            l->setPosition(p + Vec2(txtwidth/2, 0));
            l->setHorizontalAlignment(TextHAlignment::LEFT);
            l->setVerticalAlignment(TextVAlignment::BOTTOM);
            this->addChild(l, gZOrderResLabels);
            _resIcons->drawSolidRect(
                p - Vec2(size + 3, size/2 + 1),
                p - Vec2(1, - size/2 - 1),
                Color4F::WHITE
            );
            if (i == -1) { // supply
                _supplyLabel = l;
                _resIcons->drawSolidRect(
                    p - Vec2(size + 2, size/2),
                    p - Vec2(2, -size/2),
                    gSupplyColor
                );
            } else { // resources
                _resLabels[i] = l;
                _resIcons->drawSolidRect(
                    p - Vec2(size + 2, size/2),
                    p - Vec2(2, -size/2),
                    gResColor[i]
                );
            }
        }
    }
    {
        std::stringstream ss;
        ss << (_activePlayer? _activePlayer->supply: 0)
           << "/"
           << (_activePlayer? _activePlayer->supplyMax: 0);
        _supplyLabel->setString(ss.str());
    }
    for (size_t i = 0; i < RES_COUNT; i++) {
        std::stringstream ss;
        ss << (_activePlayer? _activePlayer->res.amount[i]: 0);
        _resLabels[i]->setString(ss.str());
    }
}

void GameScene::initPlayers()
{
}

void GameScene::playerUpdate(float delta)
{
//    This is already do by calling update() on every object
//    for (Player* player : _players) {
//        player->update(delta);
//    }
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

void GameScene::playerSelectPoint(Vec2 p, bool add, bool all)
{
    if (!_activePlayer) {
        return;
    }
    _pworld->queryPoint(
        CC_CALLBACK_3(GameScene::onSelectQueryPoint, this, add, all),
        _view.screen2world(p),
        nullptr
    );
}

bool GameScene::onSelectQueryPoint(PhysicsWorld& pworld, PhysicsShape& shape, void* userdata, bool add, bool all)
{
    UNUSED(pworld);
    UNUSED(userdata);
    UNUSED(all);
    ObjTag tag(shape.getBody()->getNode()->getTag());
    Id id = tag.id();
    if (tag.type() == ObjType::Unit) {
        if (add) {
            if (_activePlayer->isSelect(id)) {
                _activePlayer->selectRemove(id);
            } else {
                _activePlayer->selectAdd(id);
            }
        } else {
            _activePlayer->select(id);
        }
    }
    if (tag.type() == ObjType::Building) {
        _activePlayer->select(id);
    }
    return false;
}

void GameScene::playerSelectRect(Vec2 p1, Vec2 p2, bool add)
{
    if (!_activePlayer) {
        return;
    }
    Vec2 p[] = {
        Vec2(std::min(p1.x, p2.x), std::min(p1.y, p2.y)),
        Vec2(std::max(p1.x, p2.x), std::min(p1.y, p2.y)),
        Vec2(std::max(p1.x, p2.x), std::max(p1.y, p2.y)),
        Vec2(std::min(p1.x, p2.x), std::max(p1.y, p2.y))
    };
    Vec2 pw[] = {
        _view.screen2world(p[0]),
        _view.screen2world(p[1]),
        _view.screen2world(p[2]),
        _view.screen2world(p[3])
    };
    Vec2 bbw[] = {
        Vec2(std::min(pw[0].x, std::min(pw[1].x, std::min(pw[2].x, pw[3].x))),
             std::min(pw[0].y, std::min(pw[1].y, std::min(pw[2].y, pw[3].y)))),
        Vec2(std::max(pw[0].x, std::max(pw[1].x, std::max(pw[2].x, pw[3].x))),
             std::max(pw[0].y, std::max(pw[1].y, std::max(pw[2].y, pw[3].y)))),
    };
    std::set<Id> ids;
    _pworld->queryRect(
        CC_CALLBACK_3(GameScene::onSelectQueryRect, this),
        Rect(bbw[0].x, bbw[0].y, bbw[1].x - bbw[0].x, bbw[1].y - bbw[0].y),
        &ids
    );

    bool selectionCleared = false;
    for (Id id : ids) {
        if (VisualObj* vobj = _objs->getByIdAs<VisualObj>(id)) {
            Vec2 uw = vobj->getNode()->getPosition();
            Vec2 u = _view.world2screen(uw);
            float r = vobj->getSize() / 2;
            if (u.x > p[0].x - r && u.x < p[2].x + r) {
                if (u.y > p[0].y - r && u.y < p[2].y + r) {
                    if (vobj->getObjType() == ObjType::Building || (!add && !selectionCleared)) {
                        selectionCleared = true;
                        _activePlayer->clearSelection();
                    }
                    _activePlayer->selectAdd(id);
                }
            }
        }
    }
}

bool GameScene::onSelectQueryRect(PhysicsWorld& pworld, PhysicsShape& shape, void* userdata)
{
    UNUSED(pworld);
    std::set<Id>& ids = *reinterpret_cast<std::set<Id>*>(userdata);
    ObjTag tag(shape.getBody()->getNode()->getTag());
    if (tag.type() == ObjType::Unit) {
        if (ids.size() == 1 && objs()->getById(*ids.begin())->getObjType() == ObjType::Building) {
            ids.clear();
        }
        ids.insert(tag.id());
    }
    if (tag.type() == ObjType::Building) {
        if (ids.empty()) {
            ids.insert(tag.id());
        }
    }
    return true;
}

void GameScene::playerCenterSelection()
{
    if (_activePlayer) {
        Vec2 center = Vec2::ZERO;
        size_t count = 0;
        for (Id id : _activePlayer->selected) {
            if (VisualObj* vobj = dynamic_cast<VisualObj*>(objs()->getById(id))) {
                center += vobj->getNode()->getPosition();
                count++;
            }
        }
        if (count > 0) {
            center = center / count;
            _view.act(_view.moveTo(center, 0.0f));
        }
    }
}

void GameScene::playerOrderPoint(Vec2 p, bool add)
{
    if (!_activePlayer) {
        return;
    }
    Vec2 pw = _view.screen2world(p);

    // Determine order type
    Unit::OrderType orderType = Unit::OrderType::Move;
    if (isKeyHeld(EventKeyboard::KeyCode::KEY_CTRL)) {
        orderType = Unit::OrderType::Aim;
    }

    // Check if it is an ID-order
    bool found = false;
    _pworld->queryPoint([=, &found] (PhysicsWorld&, PhysicsShape& shape, void*) -> bool {
        ObjTag tag(shape.getBody()->getNode()->getTag());
        Id id = tag.id();
        _activePlayer->giveOrder(Unit::Order(orderType, pw, id), add);
        return false;
    }, pw, nullptr);

    // It is a POINT-order
    if (!found) {
        _activePlayer->giveOrder(Unit::Order(orderType, pw), add);
    }
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
    return dispatchContact(contact, nullptr, nullptr);
}

bool GameScene::onContactPreSolve(PhysicsContact& contact, PhysicsContactPreSolve& solve)
{
    return dispatchContact(contact, &solve, nullptr);
}

void GameScene::onContactPostSolve(PhysicsContact& contact, const PhysicsContactPostSolve& solve)
{
    dispatchContact(contact, nullptr, &solve);
}

void GameScene::onContactSeparate(PhysicsContact& contact)
{
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

//    CCLOG("COLLISION thisObjType# %d thatObjType# %d thisShapeType# %d thatShapeType# %d",
//          (int)cinfo.thisObjTag.type(), (int)cinfo.thatObjTag.type(),
//          cinfo.thisShapeTag.type<int>(), cinfo.thatShapeTag.type<int>());

#define VG_CHECKCOLLISION_BOTH(x) \
    if (cinfo.thisObjTag.type() == ObjType::x && cinfo.thatObjTag.type() == ObjType::x) { \
        return onContact ## x ## x (cinfo); \
    } \
    /**/
#define VG_IGNORECOLLISION_BOTH(x) \
    if (cinfo.thisObjTag.type() == ObjType::x && cinfo.thatObjTag.type() == ObjType::x) { \
        return false; \
    } \
    /**/
#define VG_DEFAULTCOLLISION(x, y)
#define VG_IGNORECOLLISION(x, y) \
    if (cinfo.thisObjTag.type() == ObjType::x && cinfo.thatObjTag.type() == ObjType::y) { \
        return false; \
    } \
    if (cinfo.thisObjTag.type() == ObjType::y && cinfo.thatObjTag.type() == ObjType::x) { \
        return false; \
    } \
    /**/
#define VG_CHECKCOLLISION(x, y) \
    if (cinfo.thisObjTag.type() == ObjType::x && cinfo.thatObjTag.type() == ObjType::y) { \
        return onContact ## x ## y (cinfo); \
    } \
    if (cinfo.thisObjTag.type() == ObjType::y && cinfo.thatObjTag.type() == ObjType::x) { \
        cinfo.swap(); \
        return onContact ## x ## y(cinfo); \
    } \
    /**/
    VG_IGNORECOLLISION_BOTH(Unit);
    VG_IGNORECOLLISION_BOTH(Projectile);
    VG_CHECKCOLLISION  (Unit,       AstroObj);
    VG_CHECKCOLLISION  (Projectile, AstroObj);
    VG_CHECKCOLLISION  (Projectile, Unit);
    VG_DEFAULTCOLLISION(Building,   AstroObj);
    VG_IGNORECOLLISION (Building,   Unit);
    VG_IGNORECOLLISION (Building,   Projectile);
#undef VG_CHECKCOLLISION_BOTH
#undef VG_DEFAULTCOLLISION
#undef VG_IGNORECOLLISION
#undef VG_CHECKCOLLISION

    return true;
}

const char* ContactEventCodeName(PhysicsContact::EventCode ecode)
{
    switch (ecode) {
    case PhysicsContact::EventCode::NONE:
        return "NONE";
    case PhysicsContact::EventCode::BEGIN:
        return "BEGIN";
    case PhysicsContact::EventCode::PRESOLVE:
        return "PRESOLVE";
    case PhysicsContact::EventCode::POSTSOLVE:
        return "POSTSOLVE";
    case PhysicsContact::EventCode::SEPARATE:
        return "SEPARATE";
    default:
        return "UNKNOWN";
    }
}

bool GameScene::onContactUnitAstroObj(ContactInfo& cinfo)
{
    Unit* unit = static_cast<Unit*>(cinfo.thisObj);
    AstroObj* aobj = static_cast<AstroObj*>(cinfo.thatObj);

    if (cinfo.thatShapeTag.is(AstroObj::ShapeType::BuildingPlatform)) {
        return false;
    }

    switch (cinfo.contact.getEventCode()) {
    case PhysicsContact::EventCode::NONE:
        break;
    case PhysicsContact::EventCode::BEGIN:
        if (unit->surfaceId && unit->surfaceId != aobj->getId()) {
            // TODO[fate]: double contact with astro obj -- destroy unit
            return false;
        }
        if (unit->surfaceId == aobj->getId()) {
            unit->surfaceIdCount++;
        } else {
            unit->surfaceId = aobj->getId();
            unit->surfaceIdCount = 1;
        }
        cinfo.thisBody->setUpdateVelocityFunc(CC_CALLBACK_2(GameScene::updateUnitVelocityOnSurface, this));
        break;
    case PhysicsContact::EventCode::PRESOLVE:
        break;
    case PhysicsContact::EventCode::POSTSOLVE:
        break;
    case PhysicsContact::EventCode::SEPARATE:
        if (unit->surfaceId == aobj->getId()) {
            unit->surfaceIdCount--;
            if (unit->surfaceIdCount == 0) {
                cinfo.thisBody->resetUpdateVelocityFunc();
            }
        }
        break;
    }

    if (unit->listenContactAstroObj) {
        unit->onContactAstroObj(cinfo);
    }
    return true;
}

bool GameScene::onContactProjectileAstroObj(ContactInfo& cinfo)
{
    if (cinfo.thatShapeTag.is(AstroObj::ShapeType::BuildingPlatform)) {
        return false;
    }

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

float GameScene::initBuildings(Planet* planet, Player** players, size_t playersCount)
{
    auto pb = planet->getNode()->getPhysicsBody();
    auto segments = planet->segments();

    std::vector<Building*> factories;

    for (const Segment& seg : segments) {
        const GeoPoint& pt1 = seg.pts.front();
        const GeoPoint& pt2 = seg.next->pts.front();

        if (fabs(pt1.altitude - pt2.altitude) > 40.0) {
            continue;
        }

        Building* building = nullptr;
        if (seg.deposits.empty()) {
            if (random(0.0f, 1.0f) >= 40.0f/360.0f) {
                continue;
            }
            building = Factory::create(this);
            factories.push_back(building);
        } else if (seg.deposits.front()->res == Res::Ore) {
            if (random(0.0f, 1.0f) >= 0.8) {
                continue;
            }
            building = Mine::create(this);
        } else if (seg.deposits.front()->res == Res::Oil) {
            if (random(0.0f, 1.0f) >= 0.8) {
                continue;
            }
            building = PumpJack::create(this);
        }

        Vec2 pw = planet->altAng2world(
            std::max(pt1.altitude, pt2.altitude) + 10.0f + building->getSize()/2,
            (pt1.angle + pt2.angle) / 2
        );
        building->setPosition(pw);

        // Set factory orientation upwards
        Vec2 rw = pw - pb->getPosition(); // building coords in world frame
        float dirw = rw.getAngle();
        building->getNode()->setRotation(
            90 - CC_RADIANS_TO_DEGREES(dirw)
        );

        // Create platform body
        Vec2 rl = pb->world2Local(pw); // building coords in planet local frame
        float dirl = rl.getAngle();
        float len = 0.6*building->getSize(); // half of platform length
        Vec2 bldFloor = (rl.length() - building->getSize()/2) * rl.getNormalized(); // coordinates of building floor in body frame
        Vec2 rightEdge(bldFloor + len*Vec2::forAngle(dirl - M_PI_2));
        Vec2 leftEdge(bldFloor - len*Vec2::forAngle(dirl - M_PI_2));
        float foundationHeight = 50;
        planet->addPlatform(Platform(
            rightEdge,
            leftEdge,
            planet->altAng2local(planet->getAltitudeAt(leftEdge.getAngle()) - foundationHeight, leftEdge.getAngle()),
            planet->altAng2local(planet->getAltitudeAt(rightEdge.getAngle()) - foundationHeight, rightEdge.getAngle())
        ));
    }

    size_t factoriesPerPlayer = factories.size() / playersCount;
    size_t idx = 0;
    while (playersCount-- > 0 && idx < factories.size()) {
        Building* building = factories[idx];
        building->setPlayer(*(players++));
        if (factoriesPerPlayer > 0) {
            idx += factoriesPerPlayer;
        } else {
            idx++;
        }
    }
    Building* humanFact = factories[0]; // This must be the first player's factory
    return planet->world2polar(humanFact->getNode()->getPosition()).getLongitude();
}

void GameScene::initGalaxy()
{
    auto pl = Planet::create(this);
    _planet = pl;
    pl->setPosition(Vec2::ZERO);
    //pl->getNode()->getPhysicsBody()->applyTorque(1e11);
    //pl->getNode()->getPhysicsBody()->applyImpulse(Vec2(1e11,0.5e11));

    // Human Player
    auto human = Player::create(this);
    human->res = {{150, 0}};
    human->name = "Player1";
    human->color = gPlayerColor[0];
    playerActivate(human);

    // Computer1
    auto computer1 = Player::create(this);
    computer1->res = {{150, 0}};
    computer1->name = "Computer1";
    computer1->color = gPlayerColor[1];
    computer1->ai.reset(new MoronAI(this, 1.0f));

    // Computer2
    auto computer2 = Player::create(this);
    computer2->res = {{150, 0}};
    computer2->name = "Computer2";
    computer2->color = gPlayerColor[2];
    computer2->ai.reset(new MoronAI(this, 1.0f));

    Player* players[] = { human, computer1, computer2 };
    float startLng = initBuildings(pl, players, sizeof(players)/sizeof(*players));

    // Starting location
    auto seg = pl->segments().locateLng(startLng);
    float startAlt = seg->pts.front().altitude;
    Vec2 start = pl->geogr2world(startLng, startAlt);
    _view.act(_view.follow(start, pl, 0.0f));

//    auto keyboardListener = EventListenerKeyboard::create();
//    keyboardListener->onKeyPressed = [=](EventKeyboard::KeyCode keyCode, Event* event) {
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
//    };
//    _eventDispatcher->addEventListenerWithFixedPriority(keyboardListener, 1);
}

bool KeySpec::isMod(EventKeyboard::KeyCode key)
{
    auto ml = getKeyModList();
    return ml.find(key) != ml.end();
}

const std::set<EventKeyboard::KeyCode>& KeySpec::getKeyModList()
{
    static std::set<EventKeyboard::KeyCode> ret;
    if (ret.empty()) { // Not thread-safe
        ret.insert(EventKeyboard::KeyCode::KEY_SHIFT);
        ret.insert(EventKeyboard::KeyCode::KEY_ALT);
        ret.insert(EventKeyboard::KeyCode::KEY_CTRL);
    }
    return ret;
}

void Panels::init(GameScene* game)
{
//    _background = DrawNode::create();
//    game->addChild(_background, gZOrderSelectionPanel);

//    Size s = Director::getInstance()->getVisibleSize();
//    Vec2 o = Director::getInstance()->getVisibleOrigin();

//    // Selection panel
//    _background->drawSolidRect(
//        Vec2(gMiniMapPanelWidth, 0),
//        Vec2((s.width - gControlPanelWidth), gSelectionPanelHeight),
//        gPanelBgColor
//    );
//    _background->drawRect(
//        Vec2(gMiniMapPanelWidth, 0),
//        Vec2((s.width - gControlPanelWidth), gSelectionPanelHeight),
//        gPanelBorderColor
//    );

//    // MiniMap panel
//    _background->drawSolidRect(
//        Vec2(0, 0),
//        Vec2(gMiniMapPanelWidth, gMiniMapPanelHeight),
//        gPanelBgColor
//    );
//    _background->drawRect(
//        Vec2(0, 0),
//        Vec2(gMiniMapPanelWidth, gMiniMapPanelHeight),
//        gPanelBorderColor
//    );

//    // Control panel
//    _background->drawSolidRect(
//        Vec2(s.width - gControlPanelWidth, 0),
//        Vec2(s.width, gControlPanelHeight),
//        gPanelBgColor
//    );
//    _background->drawRect(
//        Vec2(s.width - gControlPanelWidth, 0),
//        Vec2(s.width, gControlPanelHeight),
//        gPanelBorderColor
//    );
}
