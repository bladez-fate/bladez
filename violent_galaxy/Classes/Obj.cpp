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

    // draw() is not called because you should call setZs() anyway
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
    draw();
}

void VisualObj::setPlayer(Player* player)
{
    _player = player;
    draw();
}

Color4F VisualObj::colorFilter(Color4F c, float uniform)
{
    // Apply player color
    Color4F p = (getPlayer()? getPlayer()->color: gNeutralPlayerColor);
    c = Color4F(
        ((1-uniform)*c.r + uniform*p.r),
        ((1-uniform)*c.g + uniform*p.g),
        ((1-uniform)*c.b + uniform*p.b),
        c.a
    );

    // Darken background units
    if (_zs == ZsBackground) {
        float k = 0.8;
        return Color4F(k * c.r, k * c.g, k * c.b, c.a);
    } else {
        return c;
    }
}

Color4F VisualObj::uniformColor()
{
    return colorFilter(Color4F::BLACK, 1.0f);
}

