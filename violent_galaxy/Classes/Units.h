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
    virtual bool onContactAstroObj(ContactInfo& cinfo) override;
protected:
    ColonyShip() {}
    bool init(GameScene* game) override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;
public:
    std::function<Unit*(GameScene*)> onLandCreate;
};

class Spaceport : public Unit {
public:
    OBJ_CREATE_FUNC(Spaceport);
protected:
    Spaceport() {}
    virtual bool init(GameScene* game) override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;
};

class Tank : public Unit {
public:
    OBJ_CREATE_FUNC(Tank);
protected:
    Tank() {}
    virtual bool init(GameScene* game) override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;
};
