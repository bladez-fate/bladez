#include "Projectiles.h"
#include "GameScene.h"
#include <chipmunk/chipmunk_private.h>

USING_NS_CC;

bool Projectile::init(GameScene* game)
{
    VisualObj::init(game);
    _rootNode->getPhysicsBody()->setContactTestBitmask(gMatterBitmask);
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
    CCLOG("PROJECTILE CONTACT ASTROOBJ id# %d aobjId# %d", (int)_id, (int)cinfo.thatTag.id());

    destroy();
    return false;
}

bool Projectile::onContactUnit(ContactInfo& cinfo)
{
    CCLOG("PROJECTILE CONTACT UNIT id# %d unitId# %d", (int)_id, (int)cinfo.thatTag.id());
    hit(static_cast<Unit*>(cinfo.thatObj));
    destroy();
    return false;
}

void Projectile::hit(Unit* unit)
{

}

void Projectile::setPlayer(Player* player)
{
    _player = player;
}

bool Shell::init(GameScene* game)
{
    _size = 1;
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
    _body->setMass(0.01);
    _body->setMoment(2.0);
    return _body;
}

void Shell::draw()
{
    node()->drawSolidCircle(Vec2::ZERO, _size, 0, 4, Color4F::WHITE);
}
