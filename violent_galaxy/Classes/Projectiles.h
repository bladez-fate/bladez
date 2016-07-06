#pragma once

#include "Defs.h"
#include "Obj.h"
#include "Units.h"
#include "Physics.h"

class Projectile : public VisualObj {
public:
    Id ownerId = 0; // Unit that launched it
    bool listenContactAstroObj = false;
    virtual bool onContactAstroObj(ContactInfo&);
    virtual bool onContactUnit(ContactInfo&);
    ObjType getObjType() override;
    void destroy() override;
    virtual void hit(Unit* unit);
    virtual void setPlayer(Player* player);
    Player* getPlayer() { return _player; }
protected:
    Projectile() {}
    bool init(GameScene* game) override;
protected:
    Player* _player;
    cc::PhysicsBody* _body = nullptr;
};

class Shell : public Projectile {
public:
    OBJ_CREATE_FUNC(Shell);
    float getSize() override;
protected:
    Shell() {}
    virtual bool init(GameScene* game) override;
    cc::Node* createNodes() override;
    cc::PhysicsBody* createBody() override;
    void draw() override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    float _size;
};

