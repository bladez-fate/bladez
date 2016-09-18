#include "Player.h"
#include "GameScene.h"
#include "Buildings.h"

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
                gSelectionColor
            );
        }
    }
    if (ai.get()) {
        ai->update(delta);
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

bool Player::canSelect(Id id) const
{
    if (Obj* obj = _game->objs()->getById(id)) {
        if (Building* building = dynamic_cast<Building*>(obj)) {
            if (building->getPlayer() == this) {
                return true;
            }
        }
        if (Unit* unit = dynamic_cast<Unit*>(obj)) {
            if (unit->getPlayer() == this) {
                return true;
            }
        }
    }
    return false;
}

void Player::select(Id id)
{
    if (canSelect(id)) {
        selected.clear();
        selected.push_back(id);
    }
}

void Player::selectAdd(Id id)
{
    if (canSelect(id)) {
        selected.push_back(id);
    }
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
            _game->addChild(_selectionNode, 1001);
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
}

void Player::addSelectionToGroup(size_t idx)
{
    CCASSERT(idx < groups.size(), "wrong group idx");
    std::set<Id> grp;
    for (Id id : groups[idx]) {
        grp.insert(id);
    }

    for (Id& id : selected) {
        if (id == 0) {
            continue; // Leave empty space for destroyed units
        }
        Obj* obj = _game->objs()->getById(id);
        if (!obj) {
            id = 0; // Mark as destroyed
            continue;
        }
        if (grp.find(id) == grp.end()) {
            grp.insert(id);
            groups[idx].push_back(id);
        }
    }
}

void Player::giveOrder(Unit::Order order, bool add)
{
    for (Id& id : selected) {
        if (id == 0) {
            continue; // Leave empty space for destroyed units
        }
        Obj* obj = _game->objs()->getById(id);
        if (!obj) {
            id = 0; // Mark as destroyed
            continue;
        }

        if (Unit* unit = dynamic_cast<Unit*>(obj)) {
            unit->giveOrder(order, add);
        }
    }
}

MoronAI::MoronAI(GameScene* game, float thinkDuration)
    : _game(game)
    , _thinkDuration(thinkDuration)
{}

void MoronAI::update(float delta)
{
    _thinkElapsed += delta;
    if (_thinkElapsed > _thinkDuration) {
        _thinkElapsed = 0.0f;
        think();
    }
    for (auto& kv : _tanks) {
        TankState& ts = kv.second;
        if (Tank* tank = _game->objs()->getByIdAs<Tank>(ts.id)) {
            ts.ai->update(delta, tank);
        }
    }
}

void MoronAI::think()
{
    // Clear states for destroyed tanks
    for (auto i = _tanks.begin(), e = _tanks.end(); i != e;) {
        auto& kv = *i;
        TankState& ts = kv.second;
        if (!_game->objs()->getByIdAs<Tank>(ts.id)) {
            // Tank was destroyed -- clear state
            _tanks.erase(i++);
        } else {
            ++i;
        }
    }

    // Assign strategy for newly created tanks
    for (auto kv : *_game->objs()) {
        Obj* obj = kv.second;
        if (Tank* tank = dynamic_cast<Tank*>(obj)) {
            Id id = tank->getId();
            if (_tanks.find(id) == _tanks.end()) {
                // Brand new tank has arrived
                TankState& ts = _tanks[id];
                ts.id = id;
                ts.ai.reset(_tankCount % 2 == 0?
                    (ITankAI*)(new ExploringTank(_game)):
                    (ITankAI*)(new AttackingTank(_game))
                );
                _tankCount++;
            }
        }
    }

    // Switch strategy if something goes wrong
    // Eventually all supply will be in exploring tanks!!!!!!!!!!
}

MoronAI::AttackingTank::AttackingTank(GameScene* game)
    : _game(game)
{}

void MoronAI::AttackingTank::update(float delta, Tank* tank)
{

}

MoronAI::ExploringTank::ExploringTank(GameScene* game)
    : _game(game)
{}

void MoronAI::ExploringTank::update(float delta, Tank* tank)
{

}
