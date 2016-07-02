#pragma once

#include "Defs.h"
#include "Obj.h"
#include "Physics.h"

class AstroObj : public VisualObj {
public:
    ObjType getObjType() override;
protected:
    AstroObj() {}
    bool init(GameScene* game) override;
};

class Planet : public AstroObj {
public:
    OBJ_CREATE_FUNC(Planet);
protected:
    Planet() {}
    virtual bool init(GameScene* game) override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;
};
