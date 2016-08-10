#include "Buildings.h"
#include "Projectiles.h"
#include "GameScene.h"
#include <chipmunk/chipmunk_private.h>

USING_NS_CC;

bool Building::init(GameScene* game)
{
    VisualObj::init(game);
    _rootNode->getPhysicsBody()->setContactTestBitmask(gMatterBitmask);
    return true;
}

void Building::destroy()
{
    VisualObj::destroy();
}

void Building::setPlayer(Player* player)
{
    _player = player;
}

ObjType Building::getObjType()
{
    return ObjType::Building;
}

bool Factory::init(GameScene* game)
{
    _size = 100;
    Building::init(game);
    return true;
}

Node* Factory::createNodes()
{
    return DrawNode::create();
}

PhysicsBody* Factory::createBody()
{
    _body = PhysicsBody::create(50.0, 10000.0);
    auto shape = PhysicsShapeBox::create(
        Size(_size, _size),
        gBuildingMaterial
    );
    _body->addShape(shape, false);
    return _body;
}

void Factory::draw()
{
    node()->clear();
    float r = _size / 2;

    Color4F darkGray(0.3f, 0.3f, 0.3f, 1.0f);
    node()->drawSolidRect(
        Vec2(-0.7*r, 1.0*r),
        Vec2(-0.3*r, 1.8*r),
        darkGray
    );
    node()->drawSolidRect(
        Vec2(0.3*r, 1.0*r),
        Vec2(0.7*r, 1.8*r),
        darkGray
    );
    node()->drawSolidRect(
        Vec2(-r, -r),
        Vec2( r, r),
        Color4F::GRAY
    );
    node()->drawSolidCircle(Vec2(0.6*r, 0.6*r), 0.1*r, 0, 12, darkGray);
    node()->setLocalZOrder(-1);
}

float Factory::getSize()
{
    return _size;
}

bool Mine::init(GameScene* game)
{
    _size = 100;
    Building::init(game);
    return true;
}

Node* Mine::createNodes()
{
    return DrawNode::create();
}

PhysicsBody* Mine::createBody()
{
    _body = PhysicsBody::create(50.0, 10000.0);
    auto shape = PhysicsShapeBox::create(
        Size(_size, _size),
        gBuildingMaterial
    );
    _body->addShape(shape, false);
    return _body;
}

void Mine::draw()
{
    node()->clear();
    float r = _size / 2;

    Color4F darkColor(0.5f, 0.6f, 0.2f, 1.0f);
    Color4F mainColor(0.6f, 0.8f, 0.3f, 1.0f);
    node()->drawSolidRect(
        Vec2(-0.3*r, 1.0*r),
        Vec2(0.3*r, 2.4*r),
        darkColor
    );
    node()->drawSolidRect(
        Vec2(-r, -r),
        Vec2( r, r),
        mainColor
    );
    node()->drawSolidCircle(Vec2(0.6*r, 0.6*r), 0.1*r, 0, 12, darkColor);
    node()->setLocalZOrder(-1);
}

float Mine::getSize()
{
    return _size;
}
