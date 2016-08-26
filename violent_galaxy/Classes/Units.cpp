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
    CCASSERT(fabsf(axis.getLengthSq() - 1) < 1e-3, "got unnormalized axis vector");
    return sepDir.dot(axis) * gMaxSeparationVelocity * 0.4f;
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

void Tank::shoot()
{
    if (_cooldownLeft <= 0.0f) {
        _cooldownLeft = _cooldown;
        Projectile* proj = Shell::create(_game);
        Vec2 localBegin = _gunBegin;
        Vec2 localEnd = _gunBegin + _gunLength * Vec2::forAngle(CC_DEGREES_TO_RADIANS(_angle));
        Vec2 begin = _body->local2World(localBegin);
        Vec2 end = _body->local2World(localEnd);
        proj->setPosition(end);
        Vec2 j = (end - begin).getNormalized() * _power;
        proj->getNode()->getPhysicsBody()->applyImpulse(j);
        _body->applyImpulse(_body->world2Local(Vec2::ZERO) - _body->world2Local(j));
    }
}

void Tank::incAngle(float dt)
{
    _angle = clampf(_angle + _angleStep * dt, _angleMin, _angleMax);
    draw();
}

void Tank::decAngle(float dt)
{
    _angle = clampf(_angle - _angleStep * dt, _angleMin, _angleMax);
    draw();
}

void Tank::subPower()
{
    _power = clampf(_power - _powerStep, _powerMin, _powerMax);
}

void Tank::addPower()
{
    _power = clampf(_power + _powerStep, _powerMin, _powerMax);
}

void Tank::moveLeft(bool go)
{
    _movingLeft = go;
}

void Tank::moveRight(bool go)
{
    _movingRight = go;
}

void Tank::move()
{
    float v = 0.0f;
    if (Tank::isMoving()) {
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

bool Tank::isMoving()
{
    return _movingLeft ^ _movingRight;
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
    _powerMax = 20;
    _powerStep = 1;

    _cooldown = 2;

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

void Tank::update(float delta)
{
    if (!getNode()->getPhysicsBody()) {
        return; // Happens just after creation
    }
    move();
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
