#pragma once

#include "Defs.h"

class GameScene;

class WorldView {
public:
    void init(GameScene* game);
    void update(float delta);

    void zoom(float scaleBy, cc::Vec2 zoomTo);
    void rotate(float rotateBy, cc::Vec2 axis);
    void scroll(cc::Vec2 screenDir);
    void follow(cc::Vec2 p);
    void follow(cc::Vec2 p, Obj* obj);

    bool isSurfaceView() const { return _surfaceId; }
    cc::Vec2 screen2world(cc::Vec2 s);
    float getZoom() const { return _state.zoom; }
private:
    void createWorldCamera(cc::Vec2 eye);
    void lookAt(cc::Vec2 center, bool continuos);
    void centerAt(cc::Vec2 center);
    void surfaceView(cc::Vec2 center, cocos2d::Vec2 prevCenter, bool continuos, bool zoomIfRequired);
    bool onFollowQueryPoint(cc::PhysicsWorld& pworld, cc::PhysicsShape& shape, void* userdata);
private:
    GameScene* _game;
    cc::Camera* _camera = nullptr;

    // Camera positioning and orientation
    struct State {
        cc::Vec2 center;
        cc::Vec2 size;
        float zoom = 1.0f; // world length per screen pixel
        float rotation = M_PI_2;

        cc::Vec2 getEye() const
        {
            return center - (size / 2.0).rotate(cc::Vec2::forAngle(rotation - M_PI_2));
        }

        cc::Vec3 getUp() const
        {
            return cc::Vec3(cosf(rotation), sinf(rotation), 0.0f);
        }
    };

    State _state;

    // Camera actions
    struct Action {
        enum class EType : ui32 {
            Linear,
            Smooth
        };
        EType type;
        float progress; // from zero (no progress) to 1 (full progress)
        State source;
        State target;
    };

    Action _action;

    // Target objects
    Id _surfaceId = 0;
    Id _followId = 0;

    // Options
    static constexpr float _zoomMin = 1e-1f;
    static constexpr float _zoomMax = 1e+5f;
    static constexpr float _nearPlane = 1.0f;
    static constexpr float _farPlane = 1000.0f;
};
