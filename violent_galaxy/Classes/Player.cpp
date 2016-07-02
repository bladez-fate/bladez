#include "Player.h"
#include "GameScene.h"

bool Player::init(GameScene* game)
{
    Obj::init(game);
    _game->addPlayer(this);
    return true;
}

ObjType Player::getObjType()
{
    return ObjType::Player;
}

