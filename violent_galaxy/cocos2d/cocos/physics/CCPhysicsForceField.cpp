#include "physics/CCPhysicsForceField.h"
#if CC_USE_PHYSICS

#include "chipmunk/chipmunk_private.h"
#include "physics/CCPhysicsBody.h"
#include "physics/CCPhysicsWorld.h"

NS_CC_BEGIN

PhysicsForceField::PhysicsForceField()
    : _gravityConstant(10.0)
    , _minDistanceSq(1e-3)
{}

PhysicsForceField::~PhysicsForceField()
{
    for (auto bm : _gravitySources) {
        PhysicsBody* body = bm.first;
        body->release();
    }
}

bool PhysicsForceField::init()
{
    return true;
}

void PhysicsForceField::addGravitySource(cocos2d::PhysicsBody* body, float mass)
{
    body->retain();
    _gravitySources.push_back({body, mass});
}

void PhysicsForceField::addBodyField(cpVect p, PhysicsBody* body, float mass, cpVect& ret)
{
    cpBody* cpBody = body->getCPBody();
    cpVect d = cpvsub(cpBodyGetPosition(cpBody), p);
    float dlensq = cpvlengthsq(d);
    if (dlensq >= _minDistanceSq) {
        ret = cpvadd(ret, cpvmult(d, mass / (dlensq*cpfsqrt(dlensq)) ));
    }
}

cpVect PhysicsForceField::getGravity(cpVect p)
{
    cpVect ret = cpvzero;
    for (auto bm : _gravitySources) {
        PhysicsBody* body = bm.first;
        float mass = bm.second;
        addBodyField(p, body, mass, ret);
    }
    ret = cpvmult(ret, _gravityConstant);
    return ret;
}

//cpVect PhysicsForceField::getBodyGravity(PhysicsBody* body, cpVect p)
//{
//    cpVect ret = cpvzero;
//    addBodyField(p, body, ret);
//    ret = cpvmult(ret, _gravityConstant);
//    return ret;
//}

//Vec2 PhysicsForceField::getBodyGravity(PhysicsBody* body, Vec2 p)
//{
//    cpVect g(getBodyGravity(body, cpVect{p.x, p.y}));
//    return Vec2(g.x, g.y);
//}

void PhysicsForceField::setGravityConstant(float value)
{
    _gravityConstant = value;
}

//void PhysicsForceField::addBody(cocos2d::PhysicsBody* body)
//{
//    cpBodySetVelocityUpdateFunc(body->getCPBody(), bodyUpdateVelocity);
//}

NS_CC_END

#endif // CC_USE_PHYSICS
