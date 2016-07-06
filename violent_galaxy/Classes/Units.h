#pragma once

#include "Defs.h"
#include "Obj.h"
#include "Physics.h"

class Unit : public VisualObj {
public:
    Id surfaceId = 0; // Astro obj that unit is in contact with
    bool listenContactAstroObj = false;
    virtual bool onContactAstroObj(ContactInfo&) { return true; }
    virtual bool onContactUnit(ContactInfo&) { return true; }
    ObjType getObjType() override;
    void destroy() override;
    void replaceWith(Unit* unit);
    virtual void setPlayer(Player* player);
    Player* getPlayer() { return _player; }
protected:
    Unit() {}
    bool init(GameScene* game) override;
protected:
    Player* _player;
};

class ColonyShip : public Unit {
public:
    OBJ_CREATE_FUNC(ColonyShip);
    float getSize() override;
    virtual bool onContactAstroObj(ContactInfo& cinfo) override;
protected:
    ColonyShip() {}
    bool init(GameScene* game) override;
    cc::Node* createNodes() override;
    cc::PhysicsBody* createBody() override;
    void draw() override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;
public:
    std::function<Unit*(GameScene*)> onLandCreate;
    float _radius;
};

class Tank : public Unit {
public:
    OBJ_CREATE_FUNC(Tank);
    float getSize() override;
    void shoot();
protected:
    Tank() {}
    virtual bool init(GameScene* game) override;
    cc::Node* createNodes() override;
    cc::PhysicsBody* createBody() override;
    void draw() override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;

    float _size;

    cc::Size _bb;
    cc::Vec2 _offs;
    cc::Vec2 _base[8];
    cc::Vec2 _head[6];
    cc::Vec2 _gunBegin;
    float _gunLength;
    float _angle;
    float _power;
};
