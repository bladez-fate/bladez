#include "Units.h"
#include "Projectiles.h"
#include "GameScene.h"
#include <chipmunk/chipmunk_private.h>

USING_NS_CC;

bool Unit::init(GameScene* game)
{
    VisualObj::init(game);
    setZs(ZsUnitDefault);
    return true;
}

void Unit::destroy()
{
	if (_player) {
		_player->supply -= supply;
	}
	VisualObj::destroy();
}

void Unit::replaceWith(Unit* unit)
{
    // Move node
    //unit->setPosition(_rootNode->getPosition());
    //unit->_rootNode->setRotation(_rootNode->getRotation());

    // Copy body state
    auto b = _rootNode->getPhysicsBody();
    auto ub = unit->_rootNode->getPhysicsBody();
    auto pos = b->getPosition();
    ub->setPosition(pos.x, pos.y);
    //ub->setRotation(b->getRotation());
    ub->setVelocity(b->getVelocity());
    ub->setAngularVelocity(b->getAngularVelocity());

    // Should not we reuse Id?
    die();
}

void Unit::setPlayer(Player* player)
{
    if (_player) {
        _player->supply -= supply;
    }
    VisualObj::setPlayer(player);
    if (_player) {
        _player->supply += supply;
    }
}

void Unit::damage(i32 value)
{
    hp -= value;
    if (hp <= 0) {
        Vec2 pos = _rootNode->getPhysicsBody()->getPosition();
        Color4F col = Color4F::WHITE;
        if (auto player = getPlayer()) {
            col = player->color;
        }

        // Create boom trash
        Vec2 up = -_game->physicsWorld()->getForceField()->getGravity(pos).getNormalized();
        for (int i = 0; i < 7; i++) {
            Shell* shell = Shell::create(_game);
            shell->setColor(col);
            shell->setDamage(10);
            shell->setPosition(pos);

            float angle = CC_DEGREES_TO_RADIANS(random<float>(-60, 60));
            Vec2 j = up * random<float>(10.0f, 25.0f);
            j = j.rotate(Vec2::forAngle(angle));
            shell->getNode()->getPhysicsBody()->applyTorque(random<float>(-100.0f, 100.0f));
            shell->getNode()->getPhysicsBody()->applyImpulse(j);
        }

        destroy();
    }
}

void Unit::goBack()
{
    setZs(ZsBackground);
}

void Unit::goFront()
{
    setZs(ZsForeground);
}

float Unit::separationVelocityAlong(Vec2 axis)
{
    axis.normalize();
    return sepDir.dot(axis) * gMaxSeparationVelocity * 0.4f;
}

void Unit::giveOrder(Order order, bool add)
{
    if (!add) {
        stopCurrentOrder();
        _orders.clear();
    }
    if (_orders.size() >= gMaxOrders) {
        return; // TODO[fate]: add some kind of notification on order ignore
    }
    _orders.emplace_back(order);
}

ObjType Unit::getObjType()
{
    return ObjType::Unit;
}

bool DropCapsid::init(GameScene* game)
{
    _size = 20;
    Unit::init(game);
    listenContactAstroObj = true;
    return true;
}

float DropCapsid::getSize()
{
    return _size;
}

Node* DropCapsid::createNodes()
{
    return DrawNode::create();
}

PhysicsBody* DropCapsid::createBody()
{
    _body = PhysicsBody::createCircle(
        _size / 2,
        gUnitMaterial,
        Vec2::ZERO
    );
    _body->setMass(1.1);
    _body->setMoment(210.0);
    return _body;
}

void DropCapsid::draw()
{
    node()->clear();
    float r = _size / 2;
    node()->drawSolidCircle(Vec2::ZERO, r, 0, 12, colorFilter(Color4F::WHITE));
    node()->drawSolidRect(
        Vec2(-r/10, -9*r/10),
        Vec2( r/10,  9*r/10),
        uniformColor()
    );
    node()->drawSolidRect(
        Vec2(-9*r/10, -r/10),
        Vec2( 9*r/10,  r/10),
        uniformColor()
    );
}

bool DropCapsid::onContactAstroObj(ContactInfo& cinfo)
{
    if (cinfo.contact.getEventCode() == PhysicsContact::EventCode::POSTSOLVE) {
        if (cpArbiter* arb = static_cast<cpArbiter*>(cinfo.contact.getContactInfo())) {
            cpContactPointSet cps = cpArbiterGetContactPointSet(arb);
            bool moving = false;
            for (int i = 0; i < cps.count; ++i) {
                cpVect va = cpBodyGetVelocityAtWorldPoint(arb->body_a, cps.points[i].pointA);
                cpVect vb = cpBodyGetVelocityAtWorldPoint(arb->body_b, cps.points[i].pointB);
                cpVect rv = cpvsub(va, vb);
                cpFloat wa = cpBodyGetAngularVelocity(arb->body_a);
                cpFloat wb = cpBodyGetAngularVelocity(arb->body_b);
                cpFloat rw = wa - wb;
                //if (cpvlengthsq(rv) > 1e-6 || fabs(rw) > 0.5) {
                if (cpvlengthsq(rv) > 1.0 || fabs(rw) > 5) {
                    moving = true;
                    break;
                }
            }
            if (!moving) {
                Unit* created = onLandCreate(_game);
                Player* player = _player;
                replaceWith(created); // this is destroyed

                float up = (cinfo.thisBody->getPosition() - cinfo.thatBody->getPosition()).getAngle()  / (M_PI / 180.0);
                created->getNode()->getPhysicsBody()->setRotation(90 - up);
                created->setPlayer(player);
//                CCLOG("LANDING up# %f", up);
                return true;
            }
        }
    }
    return true;
}

float Tank::getSize()
{
    return _size;
}

Vec2 Tank::getShootCenter()
{
    return _body->local2World(_gunBegin);
}

void Tank::getShootParams(Vec2& fromPoint, Vec2& dir)
{
    Vec2 localBegin = _gunBegin;
    Vec2 localEnd = _gunBegin + _gunLength * Vec2::forAngle(CC_DEGREES_TO_RADIANS(_angle));
    Vec2 begin = _body->local2World(localBegin);
    fromPoint = _body->local2World(localEnd);
    dir = (fromPoint - begin).getNormalized();
}

bool Tank::isGunAnglePossible(float angle)
{
    Vec2 localAxis = _body->world2Local(Vec2::forAngle(angle)) - _body->world2Local(Vec2::ZERO);
    float localAngle = CC_RADIANS_TO_DEGREES(angleMain(localAxis.getAngle()));
    return _angleMin < localAngle && localAngle < _angleMax;
}

void Tank::gunRotationSpeed(float speed)
{
    _rotationSpeed = clampf(speed, -1.0f, 1.0f);
}

void Tank::moveLeft(bool go)
{
    _movingLeft = go;
}

void Tank::moveRight(bool go)
{
    _movingRight = go;
}

bool Tank::shoot()
{
    if (_cooldownLeft <= 0.0f) {
        _cooldownLeft = _cooldown;
        Vec2 fromPoint;
        Vec2 dir;
        getShootParams(fromPoint, dir);
        Projectile* proj = Shell::create(_game);
        proj->setPosition(fromPoint);
        Vec2 j = dir * _power;
        proj->getNode()->getPhysicsBody()->applyImpulse(j);
        _body->applyImpulse(_body->world2Local(Vec2::ZERO) - _body->world2Local(j));
        return true;
    } else {
        return false;
    }
}

float Tank::getInitialProjectileVelocity()
{
    return _power / Shell::bodyMass;
}

//void Tank::incAngle(float dt)
//{
//    _angle = clampf(_angle + _angleStep * dt, _angleMin, _angleMax);
//    draw();
//}

//void Tank::decAngle(float dt)
//{
//    _angle = clampf(_angle - _angleStep * dt, _angleMin, _angleMax);
//    draw();
//}

//void Tank::subPower()
//{
//    _power = clampf(_power - _powerStep, _powerMin, _powerMax);
//}

//void Tank::addPower()
//{
//    _power = clampf(_power + _powerStep, _powerMin, _powerMax);
//}

Tank::ExecResult Tank::executeMove(Vec2 p)
{
    if (surfaceId) {
        if (Planet* planet = _game->objs()->getByIdAs<Planet>(surfaceId)) {
            Polar src = planet->world2polar(_body->getPosition());
            Polar dst = planet->world2polar(p);
            float aDist = angleDistance(src.a, dst.a);
            if (fabsf(aDist) * src.r < getSize() / 10) {
                moveLeft(false);
                moveRight(false);
                return ExecResult::Done;
            } else if (aDist < 0) {
                moveRight(true);
                moveLeft(false);
                return ExecResult::InProgress;
            } else {
                moveRight(false);
                moveLeft(true);
                return ExecResult::InProgress;
            }
        } else {
            return ExecResult::Failed;
        }
    } else {
        return ExecResult::Delayed;
    }
}

struct AimInfo {
    cc::Vec2 source; // From where we want shoot
    cc::Vec2 target; // Where we try to aim
    float mass; // Projectile mass
    float velocity; // Projectile starting velocity
};

struct AimResult {
    bool hit; // true if we can hit, otherwise false
    cc::Vec2 axis; // axis to shoot along for hit
};

bool Aim(GameScene* game, const AimInfo& info, AimResult& res)
{
    // Simplest possible aiming in uniform gravitational field
    // Choose reference frame is which Oy axis is antiparallel to gravity
    // And origin is in the source point
    // r' = U*r + r0 (r' - world, r - new reference frame)
    // Solve problem in new reference frame
    //    x(t) = v0x*t
    //    +y(t) = v0y*t - g*t^2/2
    //    v0x = v0*cos(a);  v0y = v0*sin(a)   -- initial velocity
    //    x(t) = x;       y(t) = y            -- coordinates of a target
    //    x = v0*t*cos(a)           => t = x / (v0 * cos(a))
    // Energy conservation:
    //    E0 = K0                             -- initial energy
    //    E = K + m*g*y                       -- final energy
    //    E0 = E => K0 = K + m*g*y
    //    K = K0 - m*g*y = m*v0^2/2 - m*g*y
    //    v^2 = v0^2/2 - g*y (1)
    //    Height Condition: v0^2/2 > g*y (2)
    //    vx = v0x => vy^2 = v^2 - v0x^2
    //    vy = v0y - g*t                      -- law for velocity change
    //   | (v0y - g*t)^2 = v^2 - v0x^2
    //   | x = v0x*t   => v0x = x/t
    //    (v0y - g*x/v0x)^2 = v^2 - v0x^2  -- to hard try find t instead
    //   | v0y = g*t + vy
    //   | v0x = x/t
    // TODO





    //    y = v*t*sin(a) - g*t^2/2   <--'
    //
    //    y = x*tg(a) - g * x^2 / (v * cos(a))^2 / 2
    // Use: 1 / cos(a)^2 = 1 + tg(a)^2, and let c := tg(a)
    //    y = x*c - g*x^2/v^2* (1+c^2)/2
    //    c^2 * [g*x^2/v^2/2] - c*[x] + [y + g*x^2/v^2/2]
    //    D = x^2 - 4 [g*x^2/v^2/2]*[y + g*x^2/v^2/2]
    //      = x^2 - [g*x^2/v^2]*[2*y + g*x^2/v^2]
    //      = x^2 * [1 - g/v^2*[2*y + g*x^2/v^2]]
    return true;
}

//Tank::ExecResult Tank::executeAttack(Id targetId)
//{
//    if (surfaceId) {
//        if (Planet* planet = _game->objs()->getByIdAs<Planet>(surfaceId)) {
//            if (Unit* target = _game->objs()->getByIdAs<Unit>(targetId)) {
//                Polar src = planet->world2polar(_body->getPosition());
//                Polar dst = planet->world2polar(target->getNode()->getPosition());
//                float aDist = angleDistance(src.a, dst.a);
//                if (fabsf(aDist) * src.r < getSize()*10) {
//                    moveLeft(false);
//                    moveRight(false);
//                    return ExecResult::Done;
//                } else if (aDist < 0) {
//                    moveRight(true);
//                    moveLeft(false);
//                    return ExecResult::InProgress;
//                } else {
//                    moveRight(false);
//                    moveLeft(true);
//                    return ExecResult::InProgress;
//                }
//            }
//        }
//        return ExecResult::Failed;
//    } else {
//        return ExecResult::Delayed;
//    }
//}

Tank::ExecResult Tank::executeAim(Vec2 p)
{
    if (surfaceId) {
        if (Planet* planet = _game->objs()->getByIdAs<Planet>(surfaceId)) {
            Vec2 fromPoint;
            Vec2 sourceDir;
            getShootParams(fromPoint, sourceDir);
            Vec2 targetDir = (p - getShootCenter()).getNormalized();
            if (targetDir.isSmall() || sourceDir.isSmall()) {
                gunRotationSpeed(0.0f);
                return ExecResult::Done; // We do not know where to aim, so we are done
            }
            float aDist = angleDistance(sourceDir.getAngle(), targetDir.getAngle());
            if (fabsf(aDist) < CC_DEGREES_TO_RADIANS(5)) {
                if (fabsf(aDist) < CC_DEGREES_TO_RADIANS(0.2)) {
                    gunRotationSpeed(0.0f);
                    return ExecResult::Done;
                } else if (aDist < 0) {
                    gunRotationSpeed(-0.1f);
                    return ExecResult::InProgress;
                } else {
                    gunRotationSpeed(0.1f);
                    return ExecResult::InProgress;
                }
            } else if (aDist < 0) {
                gunRotationSpeed(-1.0f);
                return ExecResult::InProgress;
            } else {
                gunRotationSpeed(1.0f);
                return ExecResult::InProgress;
            }
        } else {
            return ExecResult::Failed;
        }
    } else {
        return ExecResult::Delayed;
    }
}

void Tank::stopCurrentOrder()
{
    gunRotationSpeed(0.0f);
    moveRight(false);
    moveLeft(false);
}

void Tank::move()
{
    float v = 0.0f;

    if (_movingLeft ^ _movingRight) {
        if (_movingRight) {
            v += _targetV;
        } else {
            v -= _targetV;
        }
    }

    Vec2 xdir = _body->local2World(Vec2::UNIT_X) - _body->local2World(Vec2::ZERO);
    v += separationVelocityAlong(xdir);

    _track->setSurfaceVelocity(-v * xdir);
}

void Tank::rotateGun(float dt)
{
    if (_rotationSpeed != 0.0f) {
        _angle = clampf(_angle + _angleStep * _rotationSpeed * dt, _angleMin, _angleMax);
        draw();
    }
}

bool Tank::init(GameScene* game)
{
    _size = 20;

    Size bb(100,50);
    Vec2 offs(0,5);
    Vec2 cg_offs(0, 0);
    Vec2 base[8] = {
        Vec2(-40, -20), Vec2(-50, -10), Vec2(-50, 0), Vec2(-40, 10),
        Vec2(40, 10), Vec2(50, 0), Vec2(50, -10), Vec2(40, -20)
    };
    Vec2 head[6] = {
        Vec2(-30, 10), Vec2(-30, 20), Vec2(-20, 30), Vec2(20, 30),
        Vec2(30, 20), Vec2(30, 10)
    };
    Vec2 gunBegin(0, 20);
    float gunLength = 50;

    float scale = _size/bb.width;
    for (size_t i = 0; i < sizeof(_base)/sizeof(*_base); i++) _base[i] = (base[i] - cg_offs) * scale;
    for (size_t i = 0; i < sizeof(_head)/sizeof(*_head); i++) _head[i] = (head[i] - cg_offs) * scale;
    _gunBegin = (gunBegin - cg_offs) * scale;
    _bb = bb * scale;
    _offs = offs * scale;
    _cg_offs = cg_offs * scale;
    _gunLength = gunLength * scale;

    _angleMin = 0;
    _angleMax = 180 - _angleMin;
    _angleStep = 90;
    _powerMin = 1;
    _powerMax = 40;
    _powerStep = 1;

    _cooldown = 3;

    _angle = 60;
    _power = _powerMax;
    _targetV = 100;

    Unit::init(game);

    return true;
}

Node* Tank::createNodes()
{
    return DrawNode::create();
}

PhysicsBody* Tank::createBody()
{
    _body = PhysicsBody::create(1.0, 200.0);
    float r = _bb.width * 0.1;
    auto hull = PhysicsShapeBox::create(
        Size(_bb.width - 2*r, _bb.height * 0.9 - 2*r),
        gUnitMaterial,
        _offs - _cg_offs + Vec2(0, _bb.height * 0.05),
        r
    );
    _track = PhysicsShapeBox::create(
        Size(_bb.width * 0.8, _bb.height * 0.1),
        gUnitMaterial,
        _offs - _cg_offs + Vec2(0, -_bb.height * 0.45)
     );
    _body->addShape(hull, false);
    _body->addShape(_track, false);
    return _body;
}

void Tank::draw()
{
    node()->clear();
    node()->drawSegment(
        _gunBegin,
        _gunBegin + _gunLength * Vec2::forAngle(CC_DEGREES_TO_RADIANS(_angle)),
        1, uniformColor()
    );
    node()->drawSolidPoly(_base, sizeof(_base)/sizeof(*_base), colorFilter(Color4F::WHITE));
    node()->drawSolidPoly(_head, sizeof(_head)/sizeof(*_head), uniformColor());
}

void Tank::handleOrders(float delta)
{
    while (!_orders.empty()) {
        Order order = _orders.front();
        ExecResult result = ExecResult::Done;
        switch (order.type) {
        case OrderType::Move: result = executeMove(order.p); break;
        case OrderType::Aim: result = executeAim(order.p); break;
        default: break;
        }

        if (result == ExecResult::Done) {
            _orderDelayElapsed = 0;
            _orders.pop_front();
            continue; // Handle next order
        } else if (result == ExecResult::InProgress) {
            _orderDelayElapsed = 0;
            break; // Stop handling
        } else if (result == ExecResult::Delayed) {
            _orderDelayElapsed += delta;
            if (_orderDelayElapsed > gOrderDelayTimeout) {
                _orderDelayElapsed = 0;
                // Fail order by timeout (see below)
            } else {
                break; // Continue trying
            }
        }

        // Failed order -- stop all orders in list
        _orders.clear();
    }
}

void Tank::update(float delta)
{
    if (!getNode()->getPhysicsBody()) {
        return; // Happens just after creation
    }
    handleOrders(delta);
    move();
    rotateGun(delta);
    if (_cooldownLeft > 0) {
        _cooldownLeft -= delta;
    }
}

bool SpaceStation::init(GameScene* game)
{
    _size = 100;
    Unit::init(game);
    return true;
}

Node* SpaceStation::createNodes()
{
    return DrawNode::create();
}

PhysicsBody* SpaceStation::createBody()
{
    _body = PhysicsBody::create(50.0, 10000.0);
    auto hull = PhysicsShapeCircle::create(
        _size / 2,
        gUnitMaterial
    );
    _body->addShape(hull, false);
    return _body;
}

void SpaceStation::draw()
{
    node()->clear();
    float r = _size / 2;
    Color4F darkGray = colorFilter(Color4F(0.3f, 0.3f, 0.3f, 1.0f));
    node()->drawSolidCircle(Vec2::ZERO, r*1.05, 0, 12, darkGray);
    node()->drawSolidCircle(Vec2::ZERO, r, 0, 12, colorFilter(Color4F::GRAY));
    node()->drawSolidRect(
        Vec2(-7*r/10, 4*r/10),
        Vec2( 7*r/10, 6*r/10),
        darkGray
    );
    node()->drawTriangleGradient(
        Vec2(-7*r/10, -6*r/10),
        Vec2( 7*r/10, -6*r/10),
        Vec2(      0,  r),
        colorFilter(Color4F::GRAY), colorFilter(Color4F::GRAY), uniformColor()
    );
}

float SpaceStation::getSize()
{
    return _size;
}
