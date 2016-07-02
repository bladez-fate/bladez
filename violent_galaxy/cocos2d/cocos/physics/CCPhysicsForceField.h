#ifndef __CCPHYSICS_FORCEFIELD_H__
#define __CCPHYSICS_FORCEFIELD_H__

#include "base/ccConfig.h"
#include "chipmunk/chipmunk_types.h"

#if CC_USE_PHYSICS

#include "base/CCRef.h"
#include "base/CCVector.h"

NS_CC_BEGIN

class PhysicsBody;
class PhysicsWorld;

class CC_DLL PhysicsForceField : public Ref
{
public:
    PhysicsForceField();
    virtual bool init();

    void addGravitySource(PhysicsBody* body);

    cpVect getGravity(cpVect p);

    float getGravityConstant() { return _gravityConstant; }
    void setGravityConstant(float value);

    CREATE_FUNC(PhysicsForceField);
private:
    Vector<PhysicsBody*> _gravitySources;
    float _gravityConstant;
    float _minDistanceSq;
};

NS_CC_END

#endif // CC_USE_PHYSICS
#endif // __CCPHYSICS_FORCEFIELD_H__
