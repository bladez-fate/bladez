#pragma once

#include "Defs.h"

class GameScene;

class WorldView {
public:
    void init(GameScene* game);
    void update(float delta);

    void zoom(float scaleBy, cc::Vec2 center);
    void rotate(float rotateBy, cc::Vec2 center);
    void scroll(cc::Vec2 screenDir);
    void follow(cc::Vec2 p);
    void follow(cc::Vec2 p, Obj* obj);

    cc::Vec2 getCenter() const;
    bool isSurfaceView() const { return _surfaceId; }
    cc::Vec2 screen2world(cc::Vec2 s);
    float getZoom() const { return _zoom; }
private:
    void createWorldCamera(cc::Vec2 eye);
    void eyeAt(cc::Vec2 eye);
    void lookAt(cc::Vec2 eye, bool continuos);
    void centerAt(cc::Vec2 center);
    void surfaceView(cc::Vec2 center, cocos2d::Vec2 prevCenter, bool continuos, bool zoomIfRequired);
    bool onFollowQueryPoint(cc::PhysicsWorld& pworld, cc::PhysicsShape& shape, void* userdata);
private:
    GameScene* _game;
    cc::Camera* _camera = nullptr;

    // Camera positioning
    cc::Vec2 _cameraSize;
    cc::Vec2 _eye;
    float _zoom = 1.0f; // world length per screen pixel
    float _rotation = M_PI_2;

    // Target objects
    Id _surfaceId = 0;
    Id _followId = 0;

    // Options
    static constexpr float _zoomMin = 1e-1f;
    static constexpr float _zoomMax = 1e+5f;
    static constexpr float _nearPlane = 1.0f;
    static constexpr float _farPlane = 1000.0f;
};
