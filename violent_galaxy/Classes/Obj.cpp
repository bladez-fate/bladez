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
    _game->objs()->release(this); // creates autorelease obj
}


bool VisualObj::init(GameScene* game)
{
    Obj::init(game);
    game->addChild(_rootNode);

    char buf[128];
    snprintf(buf, sizeof(buf), "%s#%d", typeid(this).name(), (int)_id);
    _rootNode->setName(buf);

    _rootNode->setTag(ObjTag(getObjType(), _id));

    _rootNode->setCameraMask((unsigned short)CameraFlag::USER1);

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
}

