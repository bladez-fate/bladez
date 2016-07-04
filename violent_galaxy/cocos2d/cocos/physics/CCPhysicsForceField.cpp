#include "physics/CCPhysicsForceField.h"
#if CC_USE_PHYSICS

#include "chipmunk/chipmunk_private.h"
#include "physics/CCPhysicsBody.h"
#include "physics/CCPhysicsWorld.h"

NS_CC_BEGIN

PhysicsForceField::PhysicsForceField()
    : _gravityConstant(1.0)
    , _minDistanceSq(1e-3)
{}

bool PhysicsForceField::init()
{
    return true;
}

void PhysicsForceField::addGravitySource(cocos2d::PhysicsBody* body)
{
    _gravitySources.pushBack(body);
}

cpVect PhysicsForceField::getGravity(cpVect p)
{
    cpVect ret = cpvzero;
    for (PhysicsBody* body : _gravitySources) {
        cpBody* cpBody = body->getCPBody();
        cpVect d = cpvsub(cpBodyGetPosition(cpBody), p);
        float dlensq = cpvlengthsq(d);
        if (dlensq >= _minDistanceSq) {
            ret = cpvadd(ret, cpvmult(d, cpBody->m / (dlensq*cpfsqrt(dlensq)) ));
        }
    }
    ret = cpvmult(ret, _gravityConstant);
}

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
