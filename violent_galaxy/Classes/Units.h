#pragma once

#include <unordered_map>

#include "Defs.h"
#include "Obj.h"
#include "Physics.h"

template <class T>
class TileGrid {
private:
    struct Item {
        T t;
        cc::Vec2 p;
        float radius;

        Item() {}
        Item(const T& t_, cc::Vec2 p_, float radius_) : t(t_), p(p_), radius(radius_) {}
    };

    struct Cell {
        std::vector<Item> items;
        ui64 generation = 0;
        Cell()
        {
            items.reserve(100);
        }
    };

public:
    TileGrid(float length, size_t order)
        : _gridLength(length)
        , _sizeOrder(order)
        , _size(1ull << order)
        , _sizeMask(_size - 1)
        , _cellLength(_gridLength / _size)
        , _cells(_size * _size)
    {}

    void clear()
    {
        _generation++;
    }

    void add(T t, cc::Vec2 p, float radius)
    {
        CCASSERT(2*radius < _cellLength, "too large object for TileGrid");
        i64 xi = (i64)floorf(p.x / _cellLength);
        i64 yi = (i64)floorf(p.y / _cellLength);
        if (xi < 0) {
            xi -= _size * (xi / _size - 1);
        }
        if (yi < 0) {
            yi -= _size * (yi / _size - 1);
        }
        add(t, p, radius, xi + 1        , yi + 1);
        add(t, p, radius, xi            , yi + 1);
        add(t, p, radius, xi + _sizeMask, yi + 1);
        add(t, p, radius, xi + 1        , yi);
        add(t, p, radius, xi            , yi);
        add(t, p, radius, xi + _sizeMask, yi);
        add(t, p, radius, xi + 1        , yi + _sizeMask);
        add(t, p, radius, xi            , yi + _sizeMask);
        add(t, p, radius, xi + _sizeMask, yi + _sizeMask);
    }

    template <class F>
    void query(cc::Vec2 p, float radius, F&& f) {
        i64 xi = (i64)floorf(p.x / _cellLength);
        i64 yi = (i64)floorf(p.y / _cellLength);
        if (xi < 0) {
            xi -= _size * (xi / _size - 1);
        }
        if (yi < 0) {
            yi -= _size * (yi / _size - 1);
        }
        xi = xi & _sizeMask;
        yi = yi & _sizeMask;
        Cell& cell = _cells[xi | (yi << _sizeOrder)];
        if (cell.generation == _generation) {
            for (Item& item : cell.items) {
                float distSq = (item.p - p).lengthSquared();
                float cutoffSq = item.radius + radius;
                cutoffSq *= cutoffSq;
                if (distSq < cutoffSq) { // Check overlap
                    if (!f(item.t, item.p, item.radius, distSq)) {
                        break;
                    }
                }
            }
        }
    }

private:
    void add(T t, cc::Vec2 p, float radius, i64 xi, i64 yi)
    {
        CCASSERT(xi >= 0, "TileGrid rounding internal error");
        CCASSERT(yi >= 0, "TileGrid rounding internal error");
        xi = xi & _sizeMask;
        yi = yi & _sizeMask;
        Cell& cell = _cells[xi | (yi << _sizeOrder)];
        if (cell.generation < _generation) {
            cell.items.clear();
            cell.generation = _generation;
        }
        cell.items.emplace_back(t, p, radius);
    }

private:
    float _gridLength;
    size_t _sizeOrder;
    size_t _size;
    ui64 _sizeMask;
    float _cellLength;
    std::vector<Cell> _cells;
    ui64 _generation = 0;
};

class Unit : public VisualObj {
public:
    enum class OrderType : ui8 {
        None,       // ARGUMENT TYPE:

        // Generic orders
        Move,       // point
        Aim,        // point
        Follow,     // id
        AttackMove, // point
        Attack,     // id
        Patrol,     // point
        Stop,       // none
        Hold,        // none

        // Special orders
        Repair,     // id

        MAX
    };

    struct Order {
        OrderType type;
        cc::Vec2 p;
        Id id;

        Order() {}
        Order(OrderType type_, cc::Vec2 p_) : type(type_), p(p_), id(0) {}
        Order(OrderType type_, cc::Vec2 p_, Id id_) : type(type_), p(p_), id(id_) {}
    };

    using Orders = std::deque<Order>;
public:
    const i32 hpMax;
    const i32 supply; // Supply required by this unit
    i32 hp;
    Id surfaceId = 0; // Astro obj that unit is in contact with
    Id surfaceIdCount = 0; // Astro obj that unit is in contact with
    cc::Vec2 sepDir; // Direction for separation
    bool listenContactAstroObj = false;
public:
    virtual bool onContactAstroObj(ContactInfo&) { return true; }
    virtual bool onContactUnit(ContactInfo&) { return true; }
    ObjType getObjType() override;
    void destroy() override;
    void replaceWith(Unit* unit);
    void setPlayer(Player* player) override;
    void damage(i32 value);
    void goBack();
    void goFront();
    float separationVelocityAlong(cc::Vec2 axis);
    void giveOrder(Order order, bool add);
    virtual void stopCurrentOrder() {}
protected:
    Unit(i32 hpMax_ = 1, i32 supply_ = 1)
        : supply(supply_)
        , hpMax(hpMax_)
        , hp(hpMax)
    {}
    bool init(GameScene* game) override;
protected:
    Orders _orders;
};

class DropCapsid : public Unit {
public:
    OBJ_CREATE_FUNC(DropCapsid);
    float getSize() override;
    virtual bool onContactAstroObj(ContactInfo& cinfo) override;
protected:
    DropCapsid()
        : Unit(100, 1)
    {}
    bool init(GameScene* game) override;
    cc::Node* createNodes() override;
    cc::PhysicsBody* createBody() override;
    void draw() override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;
public:
    std::function<Unit*(GameScene*)> onLandCreate;
    float _size;
};

class Tank : public Unit {
public:
    OBJ_CREATE_FUNC(Tank);
    float getSize() override;
    void shoot();
//    void incAngle(float dt);
//    void decAngle(float dt);
//    void subPower();
//    void addPower();
protected:
    Tank()
        : Unit(150, 1)
    {}
    virtual bool init(GameScene* game) override;
    cc::Node* createNodes() override;
    cc::PhysicsBody* createBody() override;
    void draw() override;
    void update(float delta) override;
    void handleOrders(float delta);
    void move();
    void rotateGun(float dt);

    // Orders execution
    enum class ExecResult : ui8 {
        Done,
        InProgress,
        Delayed,
        Failed,
        MAX
    };

    void getShootParams(cc::Vec2& fromPoint, cc::Vec2& dir);

    void gunRotationSpeed(float speed);
    void moveLeft(bool go);
    void moveRight(bool go);

    ExecResult executeMove(cc::Vec2 p);
    ExecResult executeAim(cc::Vec2 p);
//    ExecResult executeAttack(Id targetId);

    void stopCurrentOrder() override;

protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;
    cc::PhysicsShape* _track = nullptr;

    float _size;

    cc::Size _bb;
    cc::Vec2 _offs;
    cc::Vec2 _cg_offs;
    cc::Vec2 _base[8];
    cc::Vec2 _head[6];
    cc::Vec2 _gunBegin;
    float _gunLength;
    float _angle;
    float _rotationSpeed = 0.0f;
    float _power;
    float _targetV;
    bool _movingLeft = false;
    bool _movingRight = false;

    float _angleMin;
    float _angleMax;
    float _angleStep;
    float _powerMin;
    float _powerMax;
    float _powerStep;

    float _cooldown;
    float _cooldownLeft = 0;

    float _orderDelayElapsed = 0;
};

class SpaceStation : public Unit {
public:
    OBJ_CREATE_FUNC(SpaceStation);
    float getSize() override;
protected:
    SpaceStation()
        : Unit(1500, 5)
    {}
    virtual bool init(GameScene* game) override;
    cc::Node* createNodes() override;
    cc::PhysicsBody* createBody() override;
    void draw() override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;

    float _size;
};
