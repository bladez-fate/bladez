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

MoronAI::MoronAI(GameScene* game, Player* player, float thinkDuration)
    : _game(game)
    , _player(player)
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
            if (_tanks.find(id) == _tanks.end() && tank->getPlayer() == _player) {
                // Brand new tank has arrived
                TankState& ts = _tanks[id];
                ts.id = id;
                ts.ai.reset(_tankCount % 2 == 0?
                    (ITankAI*)(new Attacking(_game, 1.0f)):
                    (ITankAI*)(new Attacking(_game, -1.0f))
                );
                _tankCount++;
            }
        }
    }

    // TODO[fate]: Switch strategy if something goes wrong
}

MoronAI::Attacking::Attacking(GameScene* game, float dir)
    : _game(game)
    , _dir(dir)
{}

void MoronAI::Attacking::update(float delta, Tank* tank)
{
    _thinkElapsed += delta;
    if (_thinkElapsed > _thinkDuration) {
        _thinkElapsed = random<float>(0, _thinkDuration/10.0f);
        think(tank);
    }
}

void MoronAI::Attacking::think(Tank* tank)
{
    if (tank->surfaceId) {
        if (Planet* planet = _game->objs()->getByIdAs<Planet>(tank->surfaceId)) {
            Vec2 tankPos = tank->getNode()->getPhysicsBody()->getPosition();

            std::set<Unit*> enemyUnits;
            std::set<Building*> enemyBuildings;
            _game->physicsWorld()->queryPoint([=, &enemyUnits, &enemyBuildings]
                                              (PhysicsWorld&, PhysicsShape& shape, void*) -> bool {
                ObjTag tag(shape.getBody()->getNode()->getTag());
                Id id = tag.id();
                if (tag.type() == ObjType::Unit) {
                    if (Unit* unit = _game->objs()->getByIdAs<Unit>(id)) {
                        if (unit != tank && unit->getPlayer() != tank->getPlayer()) {
                            enemyUnits.insert(unit);
                        }
                    }
                }
                if (tag.type() == ObjType::Building) {
                    if (Building* building = _game->objs()->getByIdAs<Building>(id)) {
                        if (building->getPlayer() != tank->getPlayer()) {
                            enemyBuildings.insert(building);
                        }
                    }
                }
                return true; // Continue
            }, tankPos, nullptr, tank->getSize() * 100);

            // Find nearest enemy building position
            Vec2 enemyBuildingPos;
            float enemyBuildingDistSq = -1.0f;
            for (Building* building : enemyBuildings) {
                Vec2 pos = building->getNode()->getPhysicsBody()->getPosition();
                float distSq = (pos - tankPos).getLengthSq();
                if (enemyBuildingDistSq == -1.0f || distSq < enemyBuildingDistSq) {
                    enemyBuildingDistSq = distSq;
                    enemyBuildingPos = pos;
                }
            }

            // Find nearest enemy unit position
            Vec2 targetPos;
            Unit* targetUnit = nullptr;
            float enemyUnitDistSq = -1.0f;
            for (Unit* unit : enemyUnits) {
                Vec2 pos = unit->getNode()->getPhysicsBody()->getPosition();
                float distSq = (pos - tankPos).getLengthSq();
                if (enemyUnitDistSq == -1.0f || distSq < enemyUnitDistSq) {
                    enemyUnitDistSq = distSq;
                    targetPos = pos;
                    targetUnit = unit;
                }
            }

            // Analyze sutuation and give orders
            float shootAngle = 0.0f;
            if (targetUnit
                    && aim(tank,
                           randomizeTarget(targetPos),
                           targetUnit->getSize() / 10,
                           shootAngle)
                    && tank->isGunAnglePossible(shootAngle)
            ) {
                // Hit is possible -- wait for cooldown

                // Introduce an error
                float aimError = CC_DEGREES_TO_RADIANS(getAimError(targetUnit));
                shootAngle += aimError;

                Vec2 fromPoint;
                Vec2 gunDir;
                tank->getShootParams(fromPoint, gunDir);
                float gunAngle = gunDir.getAngle();
                if (fabs(angleDistance(gunAngle, shootAngle)) < CC_DEGREES_TO_RADIANS(0.5)) {
                    // Enough aim accuracy -- shoot
                    if (tank->shoot()) {
                        decreaseAimError(targetUnit);
                    }
                } else {
                    // Gun is not in position -- rotate gun
                    Vec2 aimPos = tank->getShootCenter() + 10000 * Vec2::forAngle(shootAngle);
                    tank->giveOrder(Unit::Order(Unit::OrderType::Aim, aimPos), false);
                }
            } else if (!enemyBuildings.empty()) {
                tank->giveOrder(Unit::Order(Unit::OrderType::Move, enemyBuildingPos), false);
            } else {
                Polar src = planet->world2polar(tankPos);
                Polar dst = src;
                float dir = _dir;
                if (random<float>(0.0f, 1.0f) < 0.03f) {
                    dir = -_dir; // Sometimes we need to go backwards to avoid tank hanging bug
                }
                dst.a += dir * CC_DEGREES_TO_RADIANS(10);
                Vec2 orderPos = planet->polar2world(dst.r, dst.a);
                tank->giveOrder(Unit::Order(Unit::OrderType::Move, orderPos), false);
            }
        }
    }
}

void MoronAI::Attacking::randomizeAll(Unit* target)
{
    _targetRandomAngle = random<float>(-_lastError, _lastError);
    float err = _lastError / _initialError * target->getSize();
    _targetRandomVector.x = random<float>(-err, err);
    _targetRandomVector.y = random<float>(-err, err);
}

void MoronAI::Attacking::decreaseAimError(Unit* target)
{
    if (_lastError > 0.8f) {
        _lastError /= 2.0f;
    }
    randomizeAll(target);
}

Vec2 MoronAI::Attacking::randomizeTarget(Vec2 targetPos)
{
    return targetPos + _targetRandomVector;
}

float MoronAI::Attacking::getAimError(Unit* target)
{
    if (target != _lastTarget) {
        _lastTarget = target;
        _lastError = _initialError;
        randomizeAll(target);
    }
    return _targetRandomAngle;
}

// Iterative procedure to find an angle at which tank should shoot to hit the target
// Returns true if hit is possible, else false
// `shootAngle' contains resulting angle if true was returned
bool MoronAI::Attacking::aim(Tank* tank, Vec2 target, float targetSize, float& shootAngle)
{
    float v0 = tank->getInitialProjectileVelocity();
    float targetSizeSq = targetSize * targetSize;

    Vec2 tankPos = tank->getShootCenter();
    Vec2 axis = (target - tankPos).getNormalized();
    int state = 0; // 0 - before the first overshoot (fast mode)
                   // 1 - binary search mode
    float maxAngle = axis.getAngle(); // overshoot angle
    float minAngle = axis.getAngle(); // undershoot angle
    for (int iteration = 0; iteration < 15; iteration++) {
        // Simulate projectile flight
        Vec2 r = tankPos;
        Vec2 v = v0 * axis;
        float minRange = (tankPos - target).length(); // Ox-distance
        float targetHeight = 0.0f; // Oy-height of projectile with min Ox-distance
        bool negativeRange = false;
        bool positiveRange = false;

        float dt = 1.0f/30.0f; // 30 fps
        for (float t = 0.0f; t < 1.5f; t += dt) {
            // Advance projectile simulation
            Vec2 g = _game->physicsWorld()->getForceField()->getGravity(r);
            r += v * dt; // + g * dt * dt / 2; second order correction should not be simulated
            v += g * dt;

            // Frame of referece with respect to gravity (antiparallel to Oy) and with target at origin
            Vec2 Oy = -g.getNormalized();
            Vec2 Ox(Oy.y, -Oy.x) ; // Rotate 90 degrees clockwise
            Vec2 rr = r - target;

            // Check hit
            if (rr.lengthSquared() < targetSizeSq) {
                shootAngle = axis.getAngle();
                return true;
            }

            // Measures to improve next iteration try
            float range = Vec2::dot(rr, Ox);
            if (range < 0) {
                negativeRange = true;
            } else {
                positiveRange = true;
            }
            if (fabs(range) < fabs(minRange)) {
                minRange = range;
                targetHeight = Vec2::dot(rr, Oy);
            }
        }

        // Improve accuracy
        Vec2 up = -_game->physicsWorld()->getForceField()->getGravity(tankPos);
        float axisAngle = axis.getAngle();
        float aDist = angleDistance(axisAngle, up.getAngle());
        if (negativeRange && positiveRange && targetHeight > 0) { // Overshoot
            if (state == 0) {
                state = 1;
            } else {
                maxAngle = axisAngle;
            }
            // Binary search to increase accuracy
            axis = Vec2::forAngle((maxAngle + minAngle) / 2);
        } else { // Undershoot
            if (state == 0) {
                // Fast search for min/max pair of angles
                minAngle = axisAngle;
                maxAngle = minAngle + aDist / 10;
                axis = Vec2::forAngle(maxAngle);
            } else { // Binary search to increase accuracy
                minAngle = axisAngle;
                axis = Vec2::forAngle((maxAngle + minAngle) / 2);
            }
        }
    }
    if (state == 0) {
        // We never got overshoot -- probably target is too far
        return false;
    } else {
        // Hit is possible, but we were unable to find it with required accuracy
        shootAngle = (minAngle + maxAngle) / 2;
        return true;
    }
}

MoronAI::Defending::Defending(GameScene* game)
    : _game(game)
{}

void MoronAI::Defending::update(float delta, Tank* tank)
{

}
