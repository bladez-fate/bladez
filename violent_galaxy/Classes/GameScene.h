#pragma once

#include "Defs.h"
#include "base/CCRefPtr.h"
#include "Units.h"
#include "AstroObjs.h"
#include "Player.h"

template <class Id, class T>
class Storage : public cc::Ref {
public:
    using Map = cc::Map<Id, T*>;
    using iterator = typename Map::iterator;
    using const_iterator = typename Map::const_iterator;
public:
    iterator begin() { return _map.begin(); }
    iterator end() { return _map.end(); }
    const_iterator begin() const { return _map.begin(); }
    const_iterator end() const { return _map.end(); }
    const_iterator cbegin() const { return _map.cbegin(); }
    const_iterator cend() const { return _map.cend(); }

    void add(T* t)
    {
        t->setId(++_lastId);
        CCASSERT(_map.find(t->getId()) == _map.end(), "storage id duplicate");
        _map.insert(t->getId(), t);
    }

    T* getById(Id id)
    {
        auto iter = _map.find(id);
        if (iter == _map.end()) {
            return nullptr;
        }
        T* unit = iter->second;
        return unit;
    }

    template <class U>
    U* getByIdAs(Id id)
    {
        return static_cast<U*>(getById(id));
    }

    T* releaseById(Id id)
    {
        auto iter = _map.find(id);
        if (iter == _map.end()) {
            return nullptr;
        }
        T* t = iter->second;
        t->retain();
        _map.erase(iter);
        t->autorelease();
        return t;
    }

    void remove(T* t)
    {
        auto iter = _map.find(t->getId());
        if (iter == _map.end()) {
            return;
        }
        _map.erase(iter);
    }

    void release(T* t)
    {
        releaseById(t->getId());
    }

    CREATE_FUNC(Storage);
private:
    Storage() {}
    bool init() { return true; }
private:
    Id _lastId = 0;
    Map _map;
};

using ObjStorage = Storage<Id, Obj>;

class GameScene : public cc::Layer
{
public:
    static cc::Scene* createScene();
    CREATE_FUNC(GameScene);

    ObjStorage* objs() { return _objs.get(); }
    cc::PhysicsWorld* physicsWorld() { return _pworld; }
public:
    void menuCloseCallback(cc::Ref* pSender);
private: // Scene
    GameScene() {}
    virtual bool init() override;
    void update(float delta) override;
private: // World
    void createWorld(cc::Scene* scene, cc::PhysicsWorld* pworld);
    cc::PhysicsWorld* _pworld = nullptr;
    cc::RefPtr<ObjStorage> _objs;
private: // Keyboard
    void initKeyboard();
    void createKeyHoldHandler();
    bool isKeyHeld(cc::EventKeyboard::KeyCode code);
    std::map<cc::EventKeyboard::KeyCode, std::chrono::high_resolution_clock::time_point> _keyHold;
private: // Mouse
    void initMouse();
    void onMouseMove(cc::Event *event);
    void onMouseDown(cc::Event *event);
    void onMouseUp(cc::Event *event);
    void onMouseWheel(cc::Event *event);
    cc::Vec2 _mouseLastLoc;
private: // View
    void initWorldView();
    void createWorldCamera(cc::Vec2 eye);
    cc::Vec2 viewCenter() const;
    void viewEyeAt(cc::Vec2 eye);
    void viewLookAt(cc::Vec2 eye, bool continuos);
    void viewCenterAt(cc::Vec2 center);
    void viewZoom(float scaleBy, cc::Vec2 center);
    void viewRotate(float rotateBy, cc::Vec2 center);
    void viewSurface(cc::Vec2 center, cocos2d::Vec2 prevCenter, bool continuos, bool zoomIfRequired);
    void viewFollow(cc::Vec2 p);
    void viewFollow(cc::Vec2 p, Obj* obj);
    bool onViewFollowQueryPoint(cc::PhysicsWorld& pworld, cc::PhysicsShape& shape, void* userdata);
    void onViewPan(cc::Vec2 screenLoc);
    void onViewPanStop();
    void onViewScroll(cc::Vec2 screenDir);
    void onViewZoom(float times, cc::Vec2 center);
    void onViewRotate(float times, cc::Vec2 center);
    void onViewTimer(float dt);
public:
    cc::Vec2 screen2world(cc::Vec2 s);
    float getViewZoom() const { return _viewZoom; }
private:
    bool _viewPanEnabled = false;
    cc::Vec2 _viewPanLastLoc;
    float _viewZoom = 1.0f; // world length per screen pixel
    float _viewRotation = M_PI_2;
    cc::Camera* _worldCamera = nullptr;
    cc::Vec2 _worldCameraSize;
    Id _viewSurfaceId = 0;
    static constexpr float _viewScrollFactor = 1000.0f; // worldlength per second per viewzoom
    static constexpr float _viewZoomFactor = 1.1f; // zoom per one wheel scroll
    static constexpr float _viewZoomMin = 1e-1f;
    static constexpr float _viewZoomMax = 1e+5f;
    static constexpr float _viewRotationFactor = 1e-1f; // radians per one wheel scroll
    static constexpr float _viewNearPlane = 1.0f;
    static constexpr float _viewFarPlane = 1000.0f;
    static constexpr float _viewTimerIntervalSec = 0.02;
private: // Players
    void initPlayers();
    void playerUpdate(float delta);
public:
    void addPlayer(Player* player);
private:
    void playerActivate(Player* player);
    void playerSelect(cc::Vec2 p, bool add, bool all);
    bool onSelectQueryPoint(cc::PhysicsWorld& pworld, cc::PhysicsShape& shape, void* userdata, bool add, bool all);
    Player* _activePlayer = nullptr;
    using Players = std::vector<Player*>;
    Players _players;
public: // Collisions
    void initCollisions();
    bool onContactBegin(cc::PhysicsContact& contact);
    bool onContactPreSolve(cc::PhysicsContact& contact, cc::PhysicsContactPreSolve& solve);
    void onContactPostSolve(cc::PhysicsContact& contact, const cc::PhysicsContactPostSolve& solve);
    void onContactSeparate(cc::PhysicsContact& contact);
    bool dispatchContact(cc::PhysicsContact& contact,
                         cc::PhysicsContactPreSolve* preSolve,
                         const cc::PhysicsContactPostSolve* postSolve);
    bool onContactUnitAstroObj(ContactInfo& cinfo);
    bool onContactProjectileAstroObj(ContactInfo& cinfo);
    bool onContactProjectileUnit(ContactInfo& cinfo);
    void updateUnitVelocityOnSurface(cpBody* body, float dt);
public: // Galaxy
    void initGalaxy();
};
