#include "AstroObjs.h"
#include "GameScene.h"
#include <base/ccRandom.h>

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
    return ObjType::Astro;
}

Planet::Planet()
    : _segments(360)
{
    // Generate mountains
    for (int i = 0; i < 100; i++) {
        float height = random<float>(100.0, 400.0);
        float slope = random<float>(1.0, 10.0);
        float width = std::min(180.0, slope * height / 100.0);
        float latitude = random<float>(0.0, 360.0);
        for (size_t i = 0; i < width; i++) {
            Segment& s = *_segments.locate(CC_DEGREES_TO_RADIANS(latitude + i));
            float x = 0;
            if (i < width/2) {
                x = i * 2 / width;
            } else {
                x = (width - i) * 2 / width;
            }
            s.altitude += x * height;
        }
    }
}

bool Planet::init(GameScene* game)
{
    _coreRadius = 8000;
    _surfAltitude = 100;
    _atmoAltitude = 1500;
    _spacAltitude = 4000;
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
    PhysicsBody* body = PhysicsBody::create();
    Vec2 vert[3];
    vert[2] = Vec2::ZERO;
    Vec2 c1 = _crust.back();
    for (Vec2& c2 : _crust) {
        vert[0] = c1;
        vert[1] = c2;
        body->addShape(PhysicsShapePolygon::create(vert, 3, gPlanetMaterial), false);
        c1 = c2;
    }
//    PhysicsBody* body = PhysicsBody::createCircle(
//        _coreRadius,
//        gPlanetMaterial,
//        Vec2::ZERO
//    );
    body->setMass(1e10);
    body->setMoment(1e12);
    return body;
}



void Planet::draw()
{
    // Draw atmosphere gradient: core:red --> surf:cyan --> atmo:blue --> spac:transparent
    Color4F coreCol = Color4F::RED;
    Color4F surfCol = Color4F(0.0, 0.5, 0.9, 1.0);
    Color4F atmoCol = Color4F(0.0, 0.1, 0.9, 1.0);
    Color4F spacCol = Color4F::BLACK; // TODO: transparent?
    auto prev = _segments.begin() + (_segments.size() - 1); // forward iter to last element
    for (auto i = _segments.begin(), e = _segments.end(); i != e; ++i) {
        float r1 = _coreRadius;
        float r2 = r1 + _surfAltitude;
        float r3 = r1 + _atmoAltitude;
        float r4 = r1 + _spacAltitude;
        float a1 = _segments.angle(prev);
        float a2 = _segments.angle(i);
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

//    node()->drawSolidPoly(_crust.data(), _crust.size(), Color4F::GREEN);
//    node()->drawSolidCircle(Vec2::ZERO, _coreRadius, 0, 360, Color4F::GREEN);

    // Some big stuff inside for decoration and to see rotation
    node()->drawSolidCircle(Vec2(_coreRadius*0.6, 0), _coreRadius*0.3, 0, 48, Color4F(1.0f, 1.0f, 0.0f, 1.0f));
    node()->drawSolidCircle(Vec2(0, _coreRadius*0.6), _coreRadius*0.3, 0, 48, Color4F(1.0f, 0.6f, 0.0f, 1.0f));
    node()->drawSolidCircle(Vec2(-_coreRadius*0.3, -_coreRadius*0.3), _coreRadius*0.4, 0, 48, Color4F(0.8f, 1.0f, 0.0f, 1.0f));
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
    _crust.resize(_segments.size());
    size_t count = 0;
    for (auto i = _segments.begin(), e = _segments.end(); i != e; ++i, count++) {
        float r = _coreRadius + i->altitude;
        float a = _segments.angle(i);
        _crust[count].x = r * cosf(a);
        _crust[count].y = r * sinf(a);
    }
}
