#include "Buildings.h"
#include "Projectiles.h"
#include "GameScene.h"
#include <chipmunk/chipmunk_private.h>

USING_NS_CC;

void CaptureChecker::update(float delta, Player* player, VisualObj* obj, GameScene* game)
{
    std::set<Id> ids;
    game->physicsWorld()->queryPoint(
        CC_CALLBACK_3(CaptureChecker::onQueryPoint, this),
        obj->getNode()->getPosition(),
        &ids,
        obj->getSize()
    );
    std::set<Player*> players;
    for (Id id : ids) {
        if (Unit* unit = game->objs()->getByIdAs<Unit>(id)) {
            players.insert(unit->getPlayer());
        }
    }
    if (players.size() == 1) {
        if (Player* capturer = *players.begin()) {
            if (capturer != obj->getPlayer()) {
                if (_capturer != capturer) { // Begin capture
                    _capturer = capturer;
                    _elapsed = 0.0f;
                }
                _elapsed += delta;
                if (_elapsed > _period) { // Successful capture
                    obj->setPlayer(_capturer);
                    _capturer = nullptr;
                }
                return; // Dont stop capture
            }
        }
    }

    // Stop capture
    _capturer = nullptr;
}

bool CaptureChecker::onQueryPoint(PhysicsWorld& pworld, PhysicsShape& shape, void* userdata)
{
    UNUSED(pworld);
    std::set<Id>& ids = *reinterpret_cast<std::set<Id>*>(userdata);
    ObjTag tag(shape.getBody()->getNode()->getTag());
    Id id = tag.id();
    if (tag.type() == ObjType::Unit) {
        ids.insert(id);
    }
    return true;
}


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

void Building::update(float delta)
{
    _captureChecker.update(delta, _player, this, _game);
}

ObjType Building::getObjType()
{
    return ObjType::Building;
}

void UnitProducer::update(float delta, Player* player, VisualObj* obj, GameScene* game)
{
    if (player != _player) {
        _player = player;
        _elapsed = 0.0f; // Production cylce is reset on building capture, money are lost
    }
    if (_player) {
        if (_elapsed == 0.0f) { // Start production if there is enough resources
            if (_player->res.enough(_unitCost)) {
                _player->res.sub(_unitCost);
                _elapsed += delta;
            }
        } else {
            _elapsed += delta;
            // Note that we assume that delta in much less than _period
            if (_elapsed > _period) {
                _elapsed = 0.0;
                auto dc = DropCapsid::create(game);
                dc->setPosition(obj->getNode()->getPosition());
                dc->onLandCreate = [](GameScene* game) {
                    return Tank::create(game);
                };
                dc->setPlayer(_player);
            }
        }
    }
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

void Factory::update(float delta)
{
    _unitProd.update(delta, _player, this, _game);
    Building::update(delta);
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

    Color4F darkColor(colorFilter(Color4F(0.5f, 0.3f, 0.2f, 1.0f)));
    Color4F mainColor(colorFilter(Color4F(0.6f, 0.4f, 0.3f, 1.0f)));
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
    Building::update(delta);
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
    Building::update(delta);
}

float PumpJack::getSize()
{
    return _size;
}
