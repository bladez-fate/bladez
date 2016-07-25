#pragma once

#include "Defs.h"
#include "Obj.h"
#include "Physics.h"

class Building : public VisualObj {
public:
    Id surfaceId = 0; // Astro obj that builing is placed on
    ObjType getObjType() override;
    void destroy() override;
    virtual void setPlayer(Player* player);
    Player* getPlayer() { return _player; }
protected:
    Building() {}
    bool init(GameScene* game) override;
protected:
    Player* _player;
};

class Factory : public Building {
public:
    OBJ_CREATE_FUNC(Factory);
    float getSize() override;
protected:
    Factory() {}
    virtual bool init(GameScene* game) override;
    cc::Node* createNodes() override;
    cc::PhysicsBody* createBody() override;
    void draw() override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;

    float _size;
};
