#pragma once

#include "Defs.h"
#include "Obj.h"
#include "Physics.h"
#include "RadialGrid.h"

class AstroObj : public VisualObj {
public:
    ObjType getObjType() override;
protected:
    AstroObj() {}
    bool init(GameScene* game) override;
};

class Planet : public AstroObj {
public:
    struct Segment {
        // TODO: std::vector<RPoint> _crust; // vector of (r, a) sorted by a
        float altitude = 100.0; // Crust altitude over planet core
    };
public:
    OBJ_CREATE_FUNC(Planet);
protected:
    Planet();
    virtual bool init(GameScene* game) override;
    cc::Node* createNodes() override;
    cc::PhysicsBody* createBody() override;
    void draw() override;
    void fillCrust();
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;
    float _coreRadius;
    AngularVec<Segment> _segments;
    std::vector<cc::Vec2> _crust;
};
