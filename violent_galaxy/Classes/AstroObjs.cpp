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
        float height = random<float>(100.0, 1000.0);
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
    node()->drawSolidCircle(Vec2(_coreRadius*0.6, 0), _coreRadius*0.3, 0, 48, Color4F(1.0f, 1.0f, 0.0f, 1.0f));
    node()->drawSolidCircle(Vec2(0, _coreRadius*0.6), _coreRadius*0.3, 0, 48, Color4F(1.0f, 0.6f, 0.0f, 1.0f));
    node()->drawSolidCircle(Vec2(-_coreRadius*0.3, -_coreRadius*0.3), _coreRadius*0.4, 0, 48, Color4F(0.8f, 1.0f, 0.0f, 1.0f));
//    node()->drawSolidRect(
//        Vec2(-radius/10, 0),
//        Vec2(radius/10, 9*radius/10),
//        Color4F::YELLOW
//    );
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
