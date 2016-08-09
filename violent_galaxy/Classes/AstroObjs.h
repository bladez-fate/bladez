#pragma once

#include "Defs.h"
#include "Obj.h"
#include "Physics.h"
#include "RadialGrid.h"
#include "Resources.h"

class AstroObj : public VisualObj {
public:
    enum class ShapeType : ui8 {
        Crust = 0,
        BuildingPlatform = 1,
    };
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

    float getAltitudeAt(float a) const
    {
        a = mainAngle(a);
        CC_ASSERT(a1 <= a);
        CC_ASSERT(a <= a2);
        const GeoPoint* pt1 = &pts.front();
        const GeoPoint* pt2 = &next->pts.front();
        for (const GeoPoint& pt : pts) {
            if (pt.angle < a) { // TODO[fate]: use binary search in vector instead of plain iteration
                pt1 = &pt;
            } else {
                pt2 = &pt;
            }
        }
        float jump = 0;
        if (pt2->angle < pt1->angle) {
            jump = 2*M_PI;
        }
        CC_ASSERT(pt1->angle <= a);
        CC_ASSERT(a <= pt2->angle + jump);
        float alpha = (a - pt1->angle) / (pt2->angle + jump - pt1->angle);
        return pt1->altitude + alpha * (pt2->altitude - pt1->altitude);
    }
};

class Platform {
public:
    static constexpr size_t POINTS = 4;
    cc::PhysicsShape* shape;
    cc::Vec2 pts[POINTS];
    Platform(cc::Vec2 pt0, cc::Vec2 pt1, cc::Vec2 pt2, cc::Vec2 pt3);
};

class Planet : public AstroObj {
public:
    OBJ_CREATE_FUNC(Planet);
    float getSize() override;
    const AngularVec<Segment>& segments() const { return _segments; }
    cc::Vec2 polar2local(float r, float a);
    cc::Vec2 altAng2local(float alt, float a);
    cc::Vec2 polar2world(float r, float a);
    cc::Vec2 geogr2local(float lng, float alt);
    cc::Vec2 geogr2world(float lng, float alt);
    float getAltitudeAt(float a) const;
    void addPlatform(Platform&& platform);
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
    std::vector<Platform> _platforms;
};
