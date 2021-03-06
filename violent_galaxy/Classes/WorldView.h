#pragma once

#include "Defs.h"

class GameScene;
class WorldViewAction;
class WorldView;

enum class EActionCategory : ui32 {
    Update,
    Scroll,
    Move,
    Zoom,
    Rotate,
    Follow
};

class WorldView {
private:
    // Camera positioning, size and orientation
    struct State {
        cc::Vec2 center;
        float zoom = 1.0f; // world length per screen pixel
        float rotation = M_PI_2;

        Id surfaceId;
        Id followId;

        cc::Vec2 getSize() const
        {
            cc::Size s = getScreenSize();
            return cc::Vec2(s.width * zoom, s.height * zoom);
        }

        cc::Vec2 getEye() const
        {
            return center - (getSize() / 2.0).rotate(cc::Vec2::forAngle(rotation - M_PI_2));
        }

        cc::Vec3 getUp() const
        {
            return cc::Vec3(cosf(rotation), sinf(rotation), 0.0f);
        }
    private:
        cc::Size getScreenSize() const
        {
            return cc::Director::getInstance()->getVisibleSize();
        }
    };
public:
    void init(GameScene* game);
    void update(float delta);

    // TODO[fate]: this should return new State, which can be transformed to action, which can be act()-ed
    WorldViewAction* zoom(float scaleBy, float duration);
    WorldViewAction* zoomTo(float scaleBy, cc::Vec2 point, float duration);
    WorldViewAction* rotate(float rotateBy, float duration);
    WorldViewAction* rotateAround(float rotateBy, cc::Vec2 axis, float duration);
    WorldViewAction* scroll(cc::Vec2 offset, bool polar, float duration);
    WorldViewAction* screenScroll(cc::Vec2 offset, bool polar, float duration);
    WorldViewAction* follow(cc::Vec2 p, float duration);
    WorldViewAction* follow(cc::Vec2 p, Obj* obj, float duration);
    WorldViewAction* moveTo(cc::Vec2 p, float duration);

    WorldViewAction* defaultView(cc::Vec2 p, float duration);

    void act(WorldViewAction* action);
    void clearActions();

    bool isSurfaceView() const { return _state.surfaceId; }
    void setSurfaceId(Id id) { _state.surfaceId = id; }
    cc::Vec2 screen2world(cc::Vec2 s);
    cc::Vec2 world2screen(cc::Vec2 w);
    float angleDiff(float phi, float psi);
    float getZoom() const { return _state.zoom; }
    cc::Vec2 getCenter() const { return _state.center; }
private:
    void removeWorldCamera();
    void createWorldCamera();
    WorldViewAction* applyState(const State& state, EActionCategory category, float duration);
    WorldViewAction* centerAt(cc::Vec2 center, float duration);
    State getViewStateAtPoint(cc::Vec2 center, bool zoomIfRequired, Id surfaceId) const;
    bool onFollowQueryPoint(cc::PhysicsWorld& pworld, cc::PhysicsShape& shape, void* userdata,
                            Id* surfaceId, Id* followId);
private:
    GameScene* _game;
    cc::Camera* _camera = nullptr;

    State _state;

    // TODO[fate]: use plain vector/array
    std::map<EActionCategory, ui32> _actionsInFly;
    std::map<EActionCategory, ui32> _actionsMaxInFly;
    std::list<WorldViewAction*> _actions;

    // Options
    static constexpr float _zoomMin = 1e-1f;
    static constexpr float _zoomMax = 1e+5f;
    static constexpr float _nearPlane = 1.0f;
    static constexpr float _farPlane = 1000.0f;
};

class WorldViewAction {
public:
    enum class EType : ui32 {
        Linear,
        Smooth
    };
public:
    WorldViewAction(EType type, EActionCategory category, float duration)
        : _type(type)
        , _category(category)
        , _duration(duration)
    {}

    static WorldViewAction* createLinear(EActionCategory category, float duration)
    {
        return new WorldViewAction(EType::Linear, category, duration);
    }

    static WorldViewAction* createSmooth(EActionCategory category, float duration)
    {
        return new WorldViewAction(EType::Smooth, category, duration);
    }

    static void spawnAdd(WorldViewAction*& action, WorldViewAction* addAction)
    {
        if (addAction) {
            if (action) {
                action->spawn(addAction);
            } else {
                action = addAction;
            }
        }
    }

    WorldViewAction* setLinear()
    {
        if (!this)
            return nullptr;
        _type = EType::Linear;
        return this;
    }

    WorldViewAction* setSmooth()
    {
        if (!this)
            return nullptr;
        _type = EType::Smooth;
        return this;
    }

    WorldViewAction* setCategory(EActionCategory category)
    {
        if (!this)
            return nullptr;
        _category = category;
        return this;
    }

    EActionCategory getCategory()
    {
        return _category;
    }

    WorldViewAction* setPolar(bool polar)
    {
        if (!this)
            return nullptr;
        _polar = polar;
        return this;
    }

    WorldViewAction* moveBy(cc::Vec2 value)
    {
        if (!this)
            return nullptr;
        _offset = value;
        return this;
    }

    WorldViewAction* zoom(float value)
    {
        if (!this)
            return nullptr;
        _zoom = value;
        return this;
    }

    WorldViewAction* zoomTo(float value, cc::Vec2 point)
    {
        if (!this)
            return nullptr;
        _zoomTo = value;
        _zoomToPoint = point;
        return this;
    }

    WorldViewAction* rotateBy(float value)
    {
        if (!this)
            return nullptr;
        _rotate = value;
        return this;
    }

    WorldViewAction* setSurfaceId(Id surfaceId)
    {
        if (!this)
            return nullptr;
        _surfaceId = surfaceId;
        return this;
    }

    WorldViewAction* spawn(WorldViewAction* action)
    {
        if (!this)
            return action;
        if (_nextSpawn) {
            _nextSpawn->spawn(action);
        } else {
            _nextSpawn.reset(action);
        }
        return this;
    }

    // Applies action to given view
    // Returns true if not finished yet or false if action is done
    bool perform(WorldView* view, float dt);
private:
    float smooth(float p);
private:
    EActionCategory _category;
    EType _type;
    float _duration;
    float _elapsed = 0.0f;
    cc::Vec2 _offset;
    bool _polar = false;
    float _zoom = 1.0f;
    float _zoomTo = 1.0f;
    cc::Vec2 _zoomToPoint;
    float _rotate = 0.0f;
    Id _surfaceId = 0;
    std::unique_ptr<WorldViewAction> _nextSpawn;
};

