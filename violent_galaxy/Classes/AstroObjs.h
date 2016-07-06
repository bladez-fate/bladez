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
    float getSize() override;
    const AngularVec<Segment>& segments() const { return _segments; }
    cc::Vec2 polar2local(float r, float a);
    cc::Vec2 polar2world(float r, float a);
    cc::Vec2 geogr2local(float lng, float alt);
    cc::Vec2 geogr2world(float lng, float alt);
protected:
    Planet();
    virtual bool init(GameScene* game) override;
    cc::Node* createNodes() override;
    cc::PhysicsBody* createBody() override;
    void draw() override;
    void drawAtmoCell(float r1, float r2, float a1, float a2, cc::Color4F r1col, cc::Color4F r2col);
    void fillCrust();
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;
    float _coreRadius;
    float _surfAltitude;
    float _atmoAltitude;
    float _spacAltitude;
    AngularVec<Segment> _segments;
    std::vector<cc::Vec2> _crust;
};
