#pragma once

#include "Defs.h"
#include "Obj.h"
#include "Physics.h"
#include "RadialGrid.h"
#include "Resources.h"

class AstroObj : public VisualObj {
public:
    ObjType getObjType() override;
protected:
    AstroObj() {}
    bool init(GameScene* game) override;
};

using StratumId = ui32;

struct Stratum {
    StratumId id = 0;
    Deposit* deposit = nullptr;
    float alt1 = 0; // lowest altitude
    float alt2 = 0; // highest altitude
    cc::Color4F col1;
    cc::Color4F col2;
};

struct Segment;

struct GeoPoint {
    Segment* segment = nullptr;
    float angle = 0.0f;
    float altitude = 100.0; // Crust altitude over planet core
    std::vector<Stratum> strata;
};

struct Segment {
    float a1 = 0.0f;
    float a2 = 0.0f;
    Segment* next = nullptr;
    Segment* prev = nullptr;

    std::vector<GeoPoint> pts;

    Segment() {
        pts.resize(1);
        GeoPoint& p = pts.back();
        p.segment = this;
    }

    void split(size_t size)
    {
        CCASSERT(pts.size() == 1, "split() must be called on unsplit segments");
        float alt1 = pts.back().altitude;
        float alt2 = next->pts.front().altitude;

        pts.resize(size);
        float da = (a2 - a1) / size;
        float dalt = (alt2 - alt1) / size;
        size_t count = 0;
        for (GeoPoint& p : pts) {
            // Note that linear interpolation of altitude is not equivalent to linear interpolation of (x,y) pairs
            p.segment = this;
            p.angle = a1 + da * count;
            p.altitude = alt1 + dalt * count;
            count++;
        }
    }

    void init(float a1_, float a2_, Segment* next_, Segment* prev_)
    {
        a1 = a1_;
        a2 = a2_;
        next = next_;
        prev = prev_;
        pts.back().angle = a1;
    }
};

class Planet : public AstroObj {
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
    void drawStratumCell(float a1, float a2, const Stratum& s1, const Stratum& s2);
    void fillCrust();
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;
    float _coreRadius;
    float _surfAltitude;
    float _atmoAltitude;
    float _spacAltitude;
    AngularVec<Segment> _segments;
    std::list<Deposit> _deposits;
    std::vector<cc::Vec2> _crust;
};
