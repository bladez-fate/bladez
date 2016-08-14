#include "Player.h"
#include "GameScene.h"

USING_NS_CC;

bool Player::init(GameScene* game)
{
    Obj::init(game);
    _game->addPlayer(this);
    groups.resize(GRP_COUNT);
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
            float radius = _game->viewSelectionRadius(size);
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

bool Player::isSelect(Id id)
{
    for (Id sid : selected) {
        if (id == sid) {
            return true;
        }
    }
    return false;
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

void Player::selectRemove(Id id)
{
    for (Id& sid : selected) {
        if (sid == id) {
            sid = 0;
        }
    }
    selected.erase(std::remove(selected.begin(), selected.end(), 0), selected.end());
}

void Player::clearSelection()
{
    selected.clear();
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

void Player::selectGroup(size_t idx)
{
    CCASSERT(idx < groups.size(), "wrong group idx");
    if (groups[idx].empty()) {
        return; // Don't allow to select empty group
    }
    selected = groups[idx];
}

void Player::setSelectionToGroup(size_t idx)
{
    CCASSERT(idx < groups.size(), "wrong group idx");
    groups[idx] = selected;
    // TODO[fate]: do not allow enemies to be in group
}

void Player::addSelectionToGroup(size_t idx)
{
    CCASSERT(idx < groups.size(), "wrong group idx");
    std::set<Id> grp;
    for (Id id : groups[idx]) {
        grp.insert(id);
    }

    for (Id id : selected) {
        if (grp.find(id) == grp.end()) {
            grp.insert(id);
            groups[idx].push_back(id);
        }
    }
    // TODO[fate]: do not allow enemies to be in group
}
