#pragma once

#include "Defs.h"
#include "base/CCRefPtr.h"
#include "Units.h"
#include "AstroObjs.h"
#include "Player.h"

template <class Id, class T>
class Storage : public cc::Ref {
public:
    void add(T* t)
    {
        t->setId(++_lastId);
        CCASSERT(_storage.find(t->getId()) == _storage.end(), "storage id duplicate");
        _storage.insert(t->getId(), t);
    }

    T* getById(Id id)
    {
        auto iter = _storage.find(id);
        if (iter == _storage.end()) {
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
        auto iter = _storage.find(id);
        if (iter == _storage.end()) {
            return nullptr;
        }
        T* t = iter->second;
        t->retain();
        _storage.erase(iter);
        t->autorelease();
        return t;
    }

    void remove(T* t)
    {
        auto iter = _storage.find(t->getId());
        if (iter == _storage.end()) {
            return;
        }
        _storage.erase(iter);
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
    cc::Map<Id, T*> _storage;
};

using ObjStorage = Storage<Id, Obj>;

class GameScene : public cc::Layer
{
public:
    static cc::Scene* createScene();
    CREATE_FUNC(GameScene);

    ObjStorage* objs() { return _objs.get(); }
    cc::PhysicsWorld* physicsWorld() { return _pworld; }
    void addPlayer(Player* player);
public:
    void menuCloseCallback(cc::Ref* pSender);
private: // Initialization
    GameScene() {}
    virtual bool init() override;
    void createWorld(cocos2d::Scene* scene, cc::PhysicsWorld* pworld);
    void createKeyHoldHandler();
private: // Keyboard
    bool isKeyHeld(cc::EventKeyboard::KeyCode code);
    std::map<cc::EventKeyboard::KeyCode, std::chrono::high_resolution_clock::time_point> _keyHold;
private: // Mouse
    void onMouseMove(cc::Event *event);
    void onMouseDown(cc::Event *event);
    void onMouseUp(cc::Event *event);
    void onMouseScroll(cc::Event *event);
private: // View
    void createWorldCamera(cocos2d::Vec2 eye);
    void viewLookAt(cc::Vec2 eye);
    void onViewPan(cc::Vec2 loc);
    void onViewPanStop();
    void onViewZoom(float times, cocos2d::Vec2 center);
    void onViewRotate(float times, cocos2d::Vec2 center);
    cc::Vec2 screen2world(cc::Vec2 s);
    bool _viewPanEnabled = false;
    cc::Vec2 _viewPanLastLoc;
    float _viewZoom = 10.0f;
    float _viewRotation = M_PI_2;
    cc::Camera* _worldCamera = nullptr;
    static constexpr float _viewZoomFactor = 1.1f; // zoom per one scroll
    static constexpr float _viewRotationFactor = 1e-1f; // radians per one scroll
    static constexpr float _viewNearPlane = 1.0f;
    static constexpr float _viewFarPlane = 1000.0f;
public: // Collisions
    bool onContactBegin(cc::PhysicsContact& contact);
    bool onContactPreSolve(cc::PhysicsContact& contact, cc::PhysicsContactPreSolve& solve);
    void onContactPostSolve(cc::PhysicsContact& contact, const cc::PhysicsContactPostSolve& solve);
    void onContactSeparate(cc::PhysicsContact& contact);
    bool dispatchContact(cc::PhysicsContact& contact,
                         cc::PhysicsContactPreSolve* preSolve,
                         const cc::PhysicsContactPostSolve* postSolve);
    bool onContactUnitAstroObj(ContactInfo& cinfo);
    void updateUnitVelocityOnSurface(cpBody* body, float dt);
private:
    cc::PhysicsWorld* _pworld = nullptr;
    cc::RefPtr<ObjStorage> _objs;
    using Players = std::vector<Player*>;
    Players _players;
};
