#include "AstroObjs.h"
#include "GameScene.h"

USING_NS_CC;

bool AstroObj::init(GameScene* game)
{
    VisualObj::init(game);
    game->physicsWorld()->getForceField()->addGravitySource(_rootNode->getPhysicsBody());
    _rootNode->getPhysicsBody()->setContactTestBitmask(gMatterBitmask);
    return true;
}

ObjType AstroObj::getObjType()
{
    return ObjType::Astro;
}

bool Planet::init(GameScene* game)
{
    _rootNode = DrawNode::create();

    float radius = 80;
    node()->drawSolidCircle(Vec2::ZERO, radius, 0, 48, Color4F::GREEN);
    node()->drawSolidCircle(Vec2(radius*0.6, 0), radius*0.3, 0, 48, Color4F(1.0f, 1.0f, 0.0f, 1.0f));
    node()->drawSolidCircle(Vec2(0, radius*0.6), radius*0.3, 0, 48, Color4F(1.0f, 0.6f, 0.0f, 1.0f));
    node()->drawSolidCircle(Vec2(-radius*0.3, -radius*0.3), radius*0.4, 0, 48, Color4F(0.8f, 1.0f, 0.0f, 1.0f));
//    node()->drawSolidRect(
//        Vec2(-radius/10, 0),
//        Vec2(radius/10, 9*radius/10),
//        Color4F::YELLOW
//    );

    _body = PhysicsBody::createCircle(
        radius,
        gPlanetMaterial,
        Vec2::ZERO
    );
    _body->setMass(1e6);
    _body->setMoment(1e8);
    _rootNode->setPhysicsBody(_body);

    AstroObj::init(game);

    return true;
}
