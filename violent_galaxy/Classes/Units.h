#pragma once

#include "Defs.h"
#include "Obj.h"
#include "Physics.h"

class Unit : public VisualObj {
public:
    const i32 hpMax;
    const i32 supply; // Supply required by this unit
    i32 hp;
    Id surfaceId = 0; // Astro obj that unit is in contact with
public:
    bool listenContactAstroObj = false;
    virtual bool onContactAstroObj(ContactInfo&) { return true; }
    virtual bool onContactUnit(ContactInfo&) { return true; }
    ObjType getObjType() override;
    void destroy() override;
    void replaceWith(Unit* unit);
    virtual void setPlayer(Player* player);
    Player* getPlayer() { return _player; }
    void damage(i32 value);
protected:
    Unit(i32 hpMax_ = 1, i32 supply_ = 1)
        : supply(supply_)
        , hpMax(hpMax_)
        , hp(hpMax)
    {}
    bool init(GameScene* game) override;
protected:
    Player* _player = nullptr;
};

class DropCapsid : public Unit {
public:
    OBJ_CREATE_FUNC(DropCapsid);
    float getSize() override;
    virtual bool onContactAstroObj(ContactInfo& cinfo) override;
protected:
    DropCapsid()
        : Unit(100, 1)
    {}
    bool init(GameScene* game) override;
    cc::Node* createNodes() override;
    cc::PhysicsBody* createBody() override;
    void draw() override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;
public:
    std::function<Unit*(GameScene*)> onLandCreate;
    float _size;
};

class Tank : public Unit {
public:
    OBJ_CREATE_FUNC(Tank);
    float getSize() override;
    void shoot();
    void incAngle(float dt);
    void decAngle(float dt);
    void subPower();
    void addPower();
    void moveLeft(bool go);
    void moveRight(bool go);
    void move();
protected:
    Tank()
        : Unit(150, 1)
    {}
    virtual bool init(GameScene* game) override;
    cc::Node* createNodes() override;
    cc::PhysicsBody* createBody() override;
    void draw() override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;
    cc::PhysicsShape* _track = nullptr;

    float _size;

    cc::Size _bb;
    cc::Vec2 _offs;
    cc::Vec2 _cg_offs;
    cc::Vec2 _base[8];
    cc::Vec2 _head[6];
    cc::Vec2 _gunBegin;
    float _gunLength;
    float _angle;
    float _power;
    float _targetV;
    bool _movingLeft = false;
    bool _movingRight = false;

    float _angleMin;
    float _angleMax;
    float _angleStep;
    float _powerMin;
    float _powerMax;
    float _powerStep;
};

class SpaceStation : public Unit {
public:
    OBJ_CREATE_FUNC(SpaceStation);
    float getSize() override;
protected:
    SpaceStation()
        : Unit(1500, 5)
    {}
    virtual bool init(GameScene* game) override;
    cc::Node* createNodes() override;
    cc::PhysicsBody* createBody() override;
    void draw() override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;

    float _size;
};
