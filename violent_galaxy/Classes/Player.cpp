#include "Player.h"
#include "GameScene.h"

USING_NS_CC;

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

void Player::update(float delta)
{
    UNUSED(delta);
    if (_selectionNode) {
        _selectionNode->clear();
        for (Id& id : selected) {
            if (id == 0) {
                continue; // Leave empty space for destroyed units
            }
            Obj* obj = _game->objs()->getById(id);
            if (!obj) {
                id = 0; // Mark as destroyed
                continue;
            }

            VisualObj* vobj = static_cast<VisualObj*>(obj);

            // NOTE: Consider rendering selection node on GUI camera
            float size = vobj->getSize();
            float radius = clampf(_game->getViewZoom() * 16, size * 1.1, 1000);
            _selectionNode->setCameraMask((unsigned short)gWorldCameraFlag);
            _selectionNode->drawCircle(
                vobj->getNode()->getPosition(),
                radius, 0, 36,
                false,
                color
            );
        }
    }
}


void Player::select(Id id)
{
    selected.clear();
    selectAdd(id);
}

void Player::selectAdd(Id id)
{
    selected.push_back(id);
}

void Player::drawSelection(bool value)
{
    _drawSelection = value;
    if (_drawSelection) {
        if (!_selectionNode) {
            _selectionNode = DrawNode::create();
            _selectionNode->setLocalZOrder(1);
            _game->addChild(_selectionNode);
        }
    } else {
        if (_selectionNode) {
            _selectionNode->removeFromParent();
            _selectionNode = nullptr;
        }
    }
}
