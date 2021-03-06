#pragma once

#include "Defs.h"
#include "Obj.h"
#include "Resources.h"
#include "Units.h"

using PlayerId = int;

class IAIStrategy {
public:
    virtual ~IAIStrategy() {}
    virtual void update(float delta) = 0;
};

class Player : public Obj {
public:
    std::string name;
    PlayerId playerId;
    cc::Color4F color;
    std::unique_ptr<IAIStrategy> ai;

    ResVec res = {{0, 0}};
    i64 supply = 0; // Currently in use
    i64 supplyMax = 0; // Currently can be supported
    i64 supplyLimit = 200; // Game limit of supply for given player

    std::vector<Id> selected;
    std::vector<std::vector<Id>> groups;
public:
    OBJ_CREATE_FUNC(Player);
    bool isSelect(Id id);
    bool canSelect(Id id) const;
    void select(Id id);
    void selectAdd(Id id);
    void selectRemove(Id id);
    void clearSelection();
    void drawSelection(bool value = true);
    void selectGroup(size_t idx);
    void setSelectionToGroup(size_t idx);
    void addSelectionToGroup(size_t idx);

    void giveOrder(Unit::Order order, bool add);

    ObjType getObjType() override;
    void update(float delta) override;
protected:
    Player() {}
    bool init(GameScene* game) override;
private:
    cc::DrawNode* _selectionNode = nullptr;
    bool _drawSelection = false;
};

class MoronAI : public IAIStrategy {
private:
    class ITankAI {
    public:
        virtual ~ITankAI() {}
        virtual void update(float delta, Tank* tank) = 0;
    };

    struct TankState {
        Id id;
        std::unique_ptr<ITankAI> ai;
    };

    class Attacking : public ITankAI {
    private:
        GameScene* _game;
        float _dir;

        float _thinkElapsed = 0.0f;
        float _thinkDuration = 0.1f;

        Unit* _lastTarget = nullptr;
        float _lastError; // in degrees
        float _targetRandomAngle; // in degrees
        float _initialError = 8; // in degrees
        cc::Vec2 _targetRandomVector;
    public:
        explicit Attacking(GameScene* game, float dir);
        void update(float delta, Tank* tank) override;
        void randomizeAll();

    private:
        void think(Tank* tank);
        void randomizeAll(Unit* target);
        void decreaseAimError(Unit* target);
        cc::Vec2 randomizeTarget(cocos2d::Vec2 targetPos);
        float getAimError(Unit* target);
        bool aim(Tank* tank, cc::Vec2 target, float targetSizeSq, float& shootAngle);
    };

    class Defending : public ITankAI {
    private:
        GameScene* _game;
    public:
        explicit Defending(GameScene* game);
        void update(float delta, Tank* tank) override;
    };

private:
    GameScene* _game;
    Player* _player;
    float _thinkElapsed = 0.0f;
    float _thinkDuration;
    std::unordered_map<Id, TankState> _tanks;
    ui64 _tankCount = 0;
public:
    MoronAI(GameScene* game, Player* player, float thinkDuration);
    void update(float delta);
private:
    void think();
};

