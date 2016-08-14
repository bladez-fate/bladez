#include "Obj.h"
#include "GameScene.h"

USING_NS_CC;

bool Obj::init(GameScene* game)
{
    _game = game;
    _game->objs()->add(this);
    return true;
}

void Obj::destroy()
{
//    _game->objs()->release(this); // creates autorelease obj
    _game->objs()->remove(this); // creates autorelease obj
//    CCLOG("OBJ DESTROY id# %d", (int)_id);
}

void Obj::update(float delta)
{
    // override me
}


bool VisualObj::init(GameScene* game)
{
    Obj::init(game);

    _rootNode = createNodes();
    game->addChild(_rootNode);
    char buf[128];
    snprintf(buf, sizeof(buf), "%s#%d", typeid(this).name(), (int)_id);
    _rootNode->setName(buf);
    _rootNode->setTag(ObjTag(getObjType(), _id));
    _rootNode->setCameraMask((unsigned short)gWorldCameraFlag);

    if (auto body = createBody()) {
        _rootNode->setPhysicsBody(body);
    }

    draw();
    return true;
}

void VisualObj::destroy()
{
    _rootNode->removeFromParent();
    Obj::destroy();
}

void VisualObj::setPosition(const Vec2& position)
{
    _rootNode->setPosition(position);
    if (auto body = _rootNode->getPhysicsBody()) {
        body->setPosition(position.x, position.y);
    }
}

void VisualObj::setZs(Zs zs)
{
    _zs = zs;
    if (_useZsForLocalZOrder) {
        _rootNode->setLocalZOrder(ZsOrder(_zs));
    }
    if (auto body = _rootNode->getPhysicsBody()) {
        body->setCategoryBitmask(_zs);
        body->setContactTestBitmask(_zs);
        body->setCollisionBitmask(_zs);
    }
}

