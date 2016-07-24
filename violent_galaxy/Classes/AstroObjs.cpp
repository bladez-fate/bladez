#include "AstroObjs.h"
#include "GameScene.h"
#include <base/ccRandom.h>

#include <unordered_set>

USING_NS_CC;

bool AstroObj::init(GameScene* game)
{
    VisualObj::init(game);
    auto body = _rootNode->getPhysicsBody();
    game->physicsWorld()->getForceField()->addGravitySource(body);
    body->setContactTestBitmask(gMatterBitmask);
    return true;
}

ObjType AstroObj::getObjType()
{
    return ObjType::AstroObj;
}

Planet::Planet()
    : _segments(360)
{
    // Generate mountains
    for (int i = 0; i < 100; i++) {
        float height = random<float>(100.0, 400.0);
        float slope = random<float>(1.0, 10.0);
        float width = std::min(180.0, slope * height / 100.0);
        float longitude = random<float>(0.0, 360.0);
        for (size_t i = 0; i < width; i++) {
            float x = 0;
            if (i < width/2) {
                x = i * 2 / width;
            } else {
                x = (width - i) * 2 / width;
            }
            Segment& s = *_segments.locateLng(longitude + i);
            GeoPoint& ms = s.pts.back();
            ms.altitude += x * height;
        }
    }

    // Generate deposits
    std::unordered_set<Segment*> occupied;
    ui64 fails = 0;
    StratumId sid = 1;
    for (int i = 0; i < 50; ) {
        float lng = random<float>(0.0, 360.0);
        Segment& seg = *_segments.locateLng(lng);
        if (occupied.find(&seg) != occupied.end()) {
            if (++fails > 1000) {
                break;
            } else {
                continue;
            }
        } else {
            occupied.insert(&seg);
        }

        constexpr size_t ptsCount = 6;
        seg.split(ptsCount);

        Res res(i % 2 == 0? Res::Ore: Res::Oil);
        _deposits.push_back(Deposit());
        Deposit& dep = _deposits.back();
        dep.res = res;
        dep.resLeft = 10000;

        if (res == Res::Ore) {
            for (size_t i = 0; i < ptsCount; i++) {
                float depth = 50;
                float z = sinf(M_PI * float(i) / (ptsCount - 1));
                GeoPoint& pt = seg.pts[i];
                pt.strata.push_back(Stratum());
                Stratum& st = pt.strata.back();
                st.id = sid;
                st.alt1 = pt.altitude - z * depth;
                st.alt2 = pt.altitude;
                st.col1 = st.col2 = Color4F(0.9f, 0.7f, 0.3f, 1.0f);
                st.deposit = &dep;
            }
        } if (res == Res::Oil) {
            for (size_t i = 0; i < ptsCount; i++) {
                float depth = 50;
                float altitude = std::min(seg.pts.back().altitude, seg.pts.front().altitude) - depth;
                float height = 30;
                float z = sinf(M_PI * float(i) / (ptsCount - 1));
                GeoPoint& pt = seg.pts[i];
                pt.strata.push_back(Stratum());
                Stratum& st = pt.strata.back();
                st.id = sid;
                st.alt1 = altitude - z * height/2;
                st.alt2 = altitude + z * height/2;
                st.col1 = st.col2 = Color4F::BLACK;
                st.deposit = &dep;
            }
        }

        i++;
        sid++;
    }
}

float Planet::getSize()
{
    return _coreRadius * 1.5;
}

Vec2 Planet::polar2local(float r, float a)
{
    return r * Vec2::forAngle(a);
}

Vec2 Planet::polar2world(float r, float a)
{
    return _body->local2World(polar2local(r, a));
}

Vec2 Planet::geogr2local(float lng, float alt)
{
    return (alt + _coreRadius) * Vec2::forAngle(CC_DEGREES_TO_RADIANS(lng));
}

Vec2 Planet::geogr2world(float lng, float alt)
{
    return _body->local2World(geogr2local(lng, alt));
}

bool Planet::init(GameScene* game)
{
    _coreRadius = 8000;
    _surfAltitude = 100;
    _atmoAltitude = 1500;
    _spacAltitude = 6000;
    fillCrust();
    AstroObj::init(game);
    return true;
}

Node* Planet::createNodes()
{
    return DrawNode::create();
}

PhysicsBody* Planet::createBody()
{
    _body = PhysicsBody::create();
    Vec2 vert[3];
    vert[2] = Vec2::ZERO;
    Vec2 c1 = _crust.back();
    for (Vec2& c2 : _crust) {
        vert[0] = c1;
        vert[1] = c2;
        _body->addShape(PhysicsShapePolygon::create(vert, 3, gPlanetMaterial), false);
        c1 = c2;
    }
//    PhysicsBody* body = PhysicsBody::createCircle(
//        _coreRadius,
//        gPlanetMaterial,
//        Vec2::ZERO
//    );
    _body->setMass(1e10);
    _body->setMoment(1e12);
    return _body;
}

void Planet::draw()
{
    // Draw atmosphere gradient
    Color4F coreCol = Color4F::RED;
    Color4F surfCol = Color4F(0.0, 0.5, 0.9, 1.0);
    Color4F atmoCol = Color4F(0.0, 0.2, 0.8, 1.0);
    Color4F spacCol = Color4F::BLACK; // TODO: transparent?
    auto prev = _segments.begin() + (_segments.size() - 1); // forward iter to last element
    for (auto i = _segments.begin(), e = _segments.end(); i != e; ++i) {
        float r1 = _coreRadius;
        float r2 = r1 + _surfAltitude;
        float r3 = r1 + _atmoAltitude;
        float r4 = r1 + _spacAltitude;
        float a1 = prev->a1;
        float a2 = i->a1;
        drawAtmoCell(r1, r2, a1, a2, coreCol, surfCol);
        drawAtmoCell(r2, r3, a1, a2, surfCol, atmoCol);
        drawAtmoCell(r3, r4, a1, a2, atmoCol, spacCol);
        prev = i;
    }

    // Draw crust
    Vec2 vert[3];
    vert[2] = Vec2::ZERO;
    Vec2 c1 = _crust.back();
    for (Vec2& c2 : _crust) {
        vert[0] = c1;
        vert[1] = c2;
        node()->drawSolidPoly(vert, 3, Color4F::GREEN);
        c1 = c2;
    }

    auto pi1 = (_segments.end() - 1)->pts.end() - 1;
    for (auto i2 = _segments.begin(), e2 = _segments.end(); i2 != e2; ++i2) {
        Segment& seg2 = *i2;
        for (auto pi2 = seg2.pts.begin(), pe2 = seg2.pts.end(); pi2 != pe2; ++pi2) {
            float a1 = pi1->angle;
            float a2 = pi2->angle;
            std::vector<Stratum>* st1 = &pi1->strata;
            std::vector<Stratum>* st2 = &pi2->strata;

            // Merge sort
            auto st1i = st1->begin();
            auto st1e = st1->end();
            auto st2i = st2->begin();
            auto st2e = st2->end();
            while (st2i != st2e && st1i != st1e) {
                if (st1i->id == st2i->id) {
                    Stratum s1 = *st1i;
                    Stratum s2 = *st2i;
                    drawStratumCell(a1, a2, s1, s2);
                    ++st1i;
                    ++st2i;
                } else if (st1i->id < st2i->id) {
                    ++st1i;
                } else {
                    ++st2i;
                }
            }
            pi1 = pi2;
        }
    }

    // Some big stuff inside for decoration and to see rotation
    node()->drawSolidCircle(Vec2(_coreRadius*0.6, 0), _coreRadius*0.3, 0, 48, Color4F(1.0f, 1.0f, 0.0f, 1.0f));
    node()->drawSolidCircle(Vec2(0, _coreRadius*0.6), _coreRadius*0.3, 0, 48, Color4F(1.0f, 0.6f, 0.0f, 1.0f));
    node()->drawSolidCircle(Vec2(-_coreRadius*0.3, -_coreRadius*0.3), _coreRadius*0.4, 0, 48, Color4F(0.8f, 1.0f, 0.0f, 1.0f));
}

void Planet::drawStratumCell(float a1, float a2, const Stratum& s1, const Stratum& s2)
{
    float r11 = _coreRadius + s1.alt1;
    float r12 = _coreRadius + s2.alt1;
    float r21 = _coreRadius + s1.alt2;
    float r22 = _coreRadius + s2.alt2;
    Vec2 v11(r11 * cosf(a1), r11 * sinf(a1));
    Vec2 v12(r12 * cosf(a2), r12 * sinf(a2));
    Vec2 v21(r21 * cosf(a1), r21 * sinf(a1));
    Vec2 v22(r22 * cosf(a2), r22 * sinf(a2));
    node()->drawTriangleGradient(v11, v21, v12, s1.col1, s2.col1, s1.col2);
    node()->drawTriangleGradient(v12, v21, v22, s1.col2, s2.col1, s2.col2);
}


void Planet::drawAtmoCell(float r1, float r2, float a1, float a2, Color4F r1col, Color4F r2col)
{
    Vec2 v11(r1 * cosf(a1), r1 * sinf(a1));
    Vec2 v12(r1 * cosf(a2), r1 * sinf(a2));
    Vec2 v21(r2 * cosf(a1), r2 * sinf(a1));
    Vec2 v22(r2 * cosf(a2), r2 * sinf(a2));
    node()->drawTriangleGradient(v11, v21, v12, r1col, r2col, r1col);
    node()->drawTriangleGradient(v12, v21, v22, r1col, r2col, r2col);
}

void Planet::fillCrust()
{
    _crust.reserve(_segments.size());
    for (auto i = _segments.begin(), e = _segments.end(); i != e; ++i) {
        Segment& seg = *i;
        for (auto pi = seg.pts.begin(), pe = seg.pts.end(); pi != pe; ++pi) {
            float r = _coreRadius + pi->altitude;
            float a = pi->angle;
            _crust.push_back(r * Vec2::forAngle(a));
        }
    }
}
