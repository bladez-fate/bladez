#include "Buildings.h"
#include "Projectiles.h"
#include "GameScene.h"
#include <chipmunk/chipmunk_private.h>

USING_NS_CC;

bool Building::init(GameScene* game)
{
    VisualObj::init(game);
    setZs(ZsBuildingDefault);
    return true;
}

void Building::destroy()
{
    VisualObj::destroy();
}

ObjType Building::getObjType()
{
    return ObjType::Building;
}

bool Factory::init(GameScene* game)
{
    _size = 70;
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

    Color4F darkGray(colorFilter(Color4F(0.3f, 0.3f, 0.3f, 1.0f)));
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
        colorFilter(Color4F::GRAY)
    );
    node()->drawSolidCircle(Vec2(0.6*r, 0.6*r), 0.1*r, 0, 12, darkGray);
    node()->drawSolidRect(
        Vec2(-0.9*r, -1.0*r),
        Vec2(-0.7*r, 0.1*r),
        colorFilter(Color4F::GRAY, 0.8f)
    );
    node()->drawSolidRect(
        Vec2(0.7*r, -1.0*r),
        Vec2(0.9*r, 0.1*r),
        colorFilter(Color4F::GRAY, 0.8f)
    );
    node()->drawSolidRect(
        Vec2(-0.6*r, -1.0*r),
        Vec2(-0.5*r, -0.5*r),
        colorFilter(Color4F::GRAY, 0.6f)
    );
    node()->drawSolidRect(
        Vec2(0.5*r, -1.0*r),
        Vec2(0.6*r, -0.5*r),
        colorFilter(Color4F::GRAY, 0.6f)
    );
}

float Factory::getSize()
{
    return _size;
}

void ResourceProducer::update(float delta, Player* player)
{
    if (player != _player) {
        _player = player;
        _elapsed = 0.0f; // Production cylce is reset on building capture
    } else if (_player) {
        _elapsed += delta;
        // Note that we assume that delta in much less than _period
        if (_elapsed > _period) {
            _elapsed -= _period;
            _player->res.add(_resAdd);
        }
    }
}

bool Mine::init(GameScene* game)
{
    _size = 60;
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

    Color4F darkColor(colorFilter(Color4F(0.5f, 0.6f, 0.2f, 1.0f)));
    Color4F mainColor(colorFilter(Color4F(0.6f, 0.8f, 0.3f, 1.0f)));
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
    node()->drawSolidCircle(Vec2(-0.4*r, 0.6*r), 0.1*r, 0, 12, 4.0, 1.0, darkColor);
    node()->drawSolidCircle(Vec2(0.1*r, 0.45*r), 0.1*r, 0, 12, 2.0, 0.7, darkColor);
    node()->drawSolidRect(
        Vec2(-0.9*r, -1.0*r),
        Vec2(-0.7*r, 0.1*r),
        colorFilter(Color4F::GRAY, 0.8f)
    );
    node()->drawSolidRect(
        Vec2(0.7*r, -1.0*r),
        Vec2(0.9*r, 0.1*r),
        colorFilter(Color4F::GRAY, 0.8f)
    );
    node()->drawSolidRect(
        Vec2(-0.6*r, -1.0*r),
        Vec2(-0.5*r, -0.5*r),
        colorFilter(Color4F::GRAY, 0.6f)
    );
    node()->drawSolidRect(
        Vec2(0.5*r, -1.0*r),
        Vec2(0.6*r, -0.5*r),
        colorFilter(Color4F::GRAY, 0.6f)
    );
}

void Mine::update(float delta)
{
    _resProd.update(delta, _player);
}

float Mine::getSize()
{
    return _size;
}

bool PumpJack::init(GameScene* game)
{
    _size = 100;
    Building::init(game);
    return true;
}

Node* PumpJack::createNodes()
{
    return DrawNode::create();
}

PhysicsBody* PumpJack::createBody()
{
    _body = PhysicsBody::create(50.0, 10000.0);
    auto shape = PhysicsShapeBox::create(
        Size(_size, _size),
        gBuildingMaterial
    );
    _body->addShape(shape, false);
    return _body;
}

void PumpJack::draw()
{
    node()->clear();
    float r = _size / 2;

    // Building under tower
    node()->drawSolidRect(
        r*Vec2(-0.5, -0.7), r*Vec2(1.0, -1.0),
        colorFilter(Color4F::WHITE)
    );

    // Mine
    node()->drawSolidRect(
        r*Vec2(-0.979, -0.95), r*Vec2(-0.579, -1.0),
        colorFilter(Color4F::WHITE)
    );

    Vec2 tower[] = {
        r*Vec2(0.0, 0.6), r*Vec2(0.0, 0.4), r*Vec2(-0.4, -0.7),
        r*Vec2(0.6, -0.7), r*Vec2(0.2, 0.4), r*Vec2(0.2, 0.6)
    };
    node()->drawSolidPoly(tower, sizeof(tower)/sizeof(*tower), colorFilter(Color4F::GRAY));

    Vec2 hammer[] = {
        r*Vec2(0.884, 0.313), r*Vec2(-0.614, 0.875),
        r*Vec2(-0.684, 0.687), r*Vec2(0.814, 0.125)
    };
    node()->drawSolidPoly(hammer, sizeof(hammer)/sizeof(*hammer), colorFilter(Color4F::WHITE, 0.8));

    Vec2 hammerHead[] = {
        r*Vec2(-0.520, 0.839), r*Vec2(-0.508, 1.155), r*Vec2(-0.708, 0.910),
        r*Vec2(-0.778, 0.722), r*Vec2(-0.789, 0.406), r*Vec2(-0.590, 0.652)
    };
    node()->drawSolidPoly(hammerHead, sizeof(hammerHead)/sizeof(*hammerHead), colorFilter(Color4F(1.0, 0.4, 0.2, 1.0)));

    // Thread from hammer head to mine
    node()->drawSolidRect(
        r*Vec2(-0.789, 0.406), r*Vec2(-0.770, -0.950),
        colorFilter(Color4F::BLACK)
    );

    node()->drawSolidRect(
        Vec2(0.0*r,  0.0*r),
        Vec2(0.2*r, -0.7*r),
        colorFilter(Color4F::GRAY, 0.8f)
    );
}

void PumpJack::update(float delta)
{
    _resProd.update(delta, _player);
}

float PumpJack::getSize()
{
    return _size;
}
