#pragma once

#include "Defs.h"
#include "Obj.h"
#include "Physics.h"
#include "Resources.h"

class CaptureChecker {
private:
    float _period = 2.0f;
    float _elapsed = 0.0f;
    Player* _capturer = nullptr;
public:
    void update(float delta, Player* player, VisualObj* obj, GameScene* game);
    bool onQueryPoint(cc::PhysicsWorld& pworld, cc::PhysicsShape& shape,
                      void* userdata, cc::Vec2 pw, float distance, cc::Vec2 up);
};

class Building : public VisualObj {
public:
    Id surfaceId = 0; // Astro obj that builing is placed on
    ObjType getObjType() override;
    void destroy() override;
    void update(float delta) override;
    virtual float getProductionProgress() { return 0.0f; }
    void setPlayer(Player *player) override;
private:
    CaptureChecker _captureChecker;
protected:
    Building() {}
    bool init(GameScene* game) override;
};

class UnitProducer {
private:
    ResVec _unitCost;
    float _period;
    float _elapsed = 0.0f;
    Player* _player = nullptr;
    ui32 _unitSupply;
    ui32 _supplyReserved = 0;
public:
    UnitProducer(const ResVec& unitCost, ui32 unitSupply, float period)
        : _unitCost(unitCost)
        , _unitSupply(unitSupply)
        , _period(period)
    {}
    void update(float delta, Player* player, VisualObj* obj, GameScene* game);
    float progress();
};

class Factory : public Building {
public:
    OBJ_CREATE_FUNC(Factory);
    float getSize() override;
protected:
    Factory() : _unitProd({{150, 0}}, 1, 12.0f /* time to build */) {}
    virtual bool init(GameScene* game) override;
    cc::Node* createNodes() override;
    cc::PhysicsBody* createBody() override;
    void draw() override;
    void update(float delta) override;
    float getProductionProgress() override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;
    cc::PhysicsShape* _foundation = nullptr; // shape on the astro obj
    float _size;
    UnitProducer _unitProd;
};

class ResourceProducer {
private:
    ResVec _resAdd;
    float _period;
    float _elapsed = 0.0f;
    Player* _player = nullptr;
    Deposit* _deposit = nullptr;
public:
    ResourceProducer(const ResVec& resAdd, float addPeriod)
        : _resAdd(resAdd)
        , _period(addPeriod)
    {}

    void setDeposit(Deposit* deposit)
    {
        _deposit = deposit;
    }

    void update(float delta, Player* player);
};

class Mine : public Building {
public:
    OBJ_CREATE_FUNC(Mine);
    float getSize() override;

    void setDeposit(Deposit* deposit)
    {
        _resProd.setDeposit(deposit);
    }

protected:
    Mine() : _resProd({{10, 0}}, 1) {}
    virtual bool init(GameScene* game) override;
    cc::Node* createNodes() override;
    cc::PhysicsBody* createBody() override;
    void draw() override;
    void update(float delta) override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;
    cc::PhysicsShape* _foundation = nullptr; // shape on the astro obj
    float _size;
    ResourceProducer _resProd;
};

class PumpJack : public Building {
public:
    OBJ_CREATE_FUNC(PumpJack);
    float getSize() override;

    void setDeposit(Deposit* deposit)
    {
        _resProd.setDeposit(deposit);
    }

protected:
    PumpJack() : _resProd({{0, 5}}, 1) {}
    virtual bool init(GameScene* game) override;
    cc::Node* createNodes() override;
    cc::PhysicsBody* createBody() override;
    void draw() override;
    void update(float delta) override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;
    cc::PhysicsShape* _foundation = nullptr; // shape on the astro obj
    float _size;
    ResourceProducer _resProd;
};
