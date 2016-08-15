#pragma once

#include "Defs.h"
#include "Obj.h"
#include "Physics.h"
#include "Resources.h"

class Building : public VisualObj {
public:
    Id surfaceId = 0; // Astro obj that builing is placed on
    ObjType getObjType() override;
    void destroy() override;
protected:
    Building() {}
    bool init(GameScene* game) override;
};

class Factory : public Building {
public:
    OBJ_CREATE_FUNC(Factory);
    float getSize() override;
protected:
    Factory() {}
    virtual bool init(GameScene* game) override;
    cc::Node* createNodes() override;
    cc::PhysicsBody* createBody() override;
    void draw() override;
protected:
    cc::DrawNode* node() { return static_cast<cc::DrawNode*>(_rootNode); }
    cc::PhysicsBody* _body = nullptr;
    cc::PhysicsShape* _foundation = nullptr; // shape on the astro obj
    float _size;
};

class ResourceProducer {
private:
    ResVec _resAdd;
    float _period;
    float _elapsed = 0.0f;
    Player* _player = nullptr;
public:
    ResourceProducer(const ResVec& resAdd, float addPeriod)
        : _resAdd(resAdd)
        , _period(addPeriod)
    {}

    void update(float delta, Player* player);
};

class Mine : public Building {
public:
    OBJ_CREATE_FUNC(Mine);
    float getSize() override;
protected:
    Mine() : _resProd({{100, 0}}, 10) {}
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
protected:
    PumpJack() : _resProd({{0, 50}}, 10) {}
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
