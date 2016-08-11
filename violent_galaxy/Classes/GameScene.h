#pragma once

#include "Defs.h"
#include "base/CCRefPtr.h"
#include "Units.h"
#include "AstroObjs.h"
#include "Player.h"
#include "WorldView.h"

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
    void keyboardUpdate(float delta);
    void createKeyHoldHandler();
    bool isKeyHeld(cc::EventKeyboard::KeyCode code);
    std::map<cc::EventKeyboard::KeyCode, std::chrono::high_resolution_clock::time_point> _keyHold;
private: // Mouse
    void initMouse();
    void onMouseMove(cc::Event *event);
    void onMouseDown(cc::Event *event);
    void onMouseUp(cc::Event *event);
    void onMouseWheel(cc::Event *event);
    void onMouseTimer(float dt);
    void mousePan(cc::Vec2 screenLoc);
    void mousePanStop();
    void mouseSelectRect(cc::Vec2 screenLoc);
    void mouseSelectRectStop(cc::Vec2 screenLoc, bool apply);
    cc::Vec2 _mouseLastLoc;
    bool _mouseSelectRectEnabled = false;
    cc::Vec2 _mouseSelectRectStart; // screen coordinated of selection rectangle begining
    cc::Vec2 _mouseSelectRectEnd; // screen coordinated of selection rectangle ending
    cc::DrawNode* _mouseSelectRectNode = nullptr;
    bool _mousePanEnabled = false;
    cc::Vec2 _mousePanLastLoc;
    static constexpr float _mouseTimerIntervalSec = 0.05;
    static constexpr float _mouseScrollFactor = 1000.0f; // worldlength per second per viewzoom
    static constexpr float _mouseZoomFactor = 1.1f; // zoom per one wheel scroll
    static constexpr float _mouseRotationFactor = 1e-1f; // radians per one wheel scroll
    static constexpr float _mouseViewActionDuration = 0.16;
    static constexpr float _mouseFollowDuration = 1.0;
public: // View
    WorldView _view;
private: // Players
    void initPlayers();
    void playerUpdate(float delta);
public:
    void addPlayer(Player* player);
private:
    void playerActivate(Player* player);
    void playerSelectPoint(cc::Vec2 p, bool add, bool all);
    bool onSelectQueryPoint(cc::PhysicsWorld& pworld, cc::PhysicsShape& shape, void* userdata, bool add, bool all);
    void playerSelectRect(cc::Vec2 p1, cc::Vec2 p2, bool add);
    bool onSelectQueryRect(cc::PhysicsWorld& pworld, cc::PhysicsShape& shape, void* userdata);
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
    void initBuildings(Planet* planet);
    Planet* _planet = nullptr;
};
