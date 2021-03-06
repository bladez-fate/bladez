#include "Projectiles.h"
#include "GameScene.h"
#include <chipmunk/chipmunk_private.h>

USING_NS_CC;

bool Projectile::init(GameScene* game)
{
    VisualObj::init(game);
    setZs(ZsProjectileDefault);
    return true;
}

void Projectile::destroy()
{
    VisualObj::destroy();
}

ObjType Projectile::getObjType()
{
    return ObjType::Projectile;
}


bool Projectile::onContactAstroObj(ContactInfo& cinfo)
{
//    CCLOG("PROJECTILE CONTACT ASTROOBJ id# %d aobjId# %d", (int)_id, (int)cinfo.thatObjTag.id());

    destroy();
    return false;
}

bool Projectile::onContactUnit(ContactInfo& cinfo)
{
//    CCLOG("PROJECTILE CONTACT UNIT id# %d unitId# %d", (int)_id, (int)cinfo.thatObjTag.id());
    hit(static_cast<Unit*>(cinfo.thatObj));
    destroy();
    return false;
}

void Projectile::hit(Unit* unit)
{
    if (auto node = unit->getNode()) {
        if (auto hitBody = node->getPhysicsBody()) {
            Vec2 j = _body->getVelocity() * _body->getMass();
            j *= 10; // Amplify impulse
            hitBody->applyImpulse(
                hitBody->world2Local(j) - hitBody->world2Local(Vec2::ZERO),
                hitBody->world2Local(_body->getPosition())
            );
            unit->damage(_damage);
        }
    }
}

void Projectile::setPlayer(Player* player)
{
    _player = player;
}

bool Shell::init(GameScene* game)
{
    _size = 2;
    _damage = 30;
    Projectile::init(game);
    return true;
}

Node* Shell::createNodes()
{
    return DrawNode::create();
}

PhysicsBody* Shell::createBody()
{
    _body = PhysicsBody::createCircle(_size, gProjectileMaterial);
    _body->setMass(bodyMass);
    _body->setMoment(2.0);
    return _body;
}

void Shell::draw()
{
    node()->drawSolidCircle(Vec2::ZERO, _size, 0, 6, _color);
}

float Shell::getSize()
{
    return _size;
}

void Shell::setColor(Color4F color)
{
    _color = color;
    draw();
}
