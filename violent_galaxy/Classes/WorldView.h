#pragma once

#include "Defs.h"

class GameScene;

class WorldView {
public:
    void init(GameScene* game);
    void update(float delta);
    void createWorldCamera(cc::Vec2 eye);
    cc::Vec2 getCenter() const;
    void eyeAt(cc::Vec2 eye);
    void lookAt(cc::Vec2 eye, bool continuos);
    void centerAt(cc::Vec2 center);
    void zoom(float scaleBy, cc::Vec2 center);
    void rotate(float rotateBy, cc::Vec2 center);
    void surfaceView(cc::Vec2 center, cocos2d::Vec2 prevCenter, bool continuos, bool zoomIfRequired);
    void follow(cc::Vec2 p);
    void follow(cc::Vec2 p, Obj* obj);
    bool onFollowQueryPoint(cc::PhysicsWorld& pworld, cc::PhysicsShape& shape, void* userdata);
    void onPan(cc::Vec2 screenLoc);
    void onPanStop();
    void onScroll(cc::Vec2 screenDir);
    void onZoom(float times, cc::Vec2 center);
    void onRotate(float times, cc::Vec2 center);
    void onTimer(float dt);
public:
    cc::Vec2 screen2world(cc::Vec2 s);
    float getZoom() const { return _zoom; }
private:
    GameScene* _game;
    bool _panEnabled = false;
    cc::Vec2 _panLastLoc;
    float _zoom = 1.0f; // world length per screen pixel
    float _rotation = M_PI_2;
    cc::Camera* _camera = nullptr;
    cc::Vec2 _cameraSize;
    cc::Vec2 _eye;
    Id _surfaceId = 0;
    Id _followId = 0;
    static constexpr float _scrollFactor = 1000.0f; // worldlength per second per viewzoom
    static constexpr float _zoomFactor = 1.1f; // zoom per one wheel scroll
    static constexpr float _zoomMin = 1e-1f;
    static constexpr float _zoomMax = 1e+5f;
    static constexpr float _rotationFactor = 1e-1f; // radians per one wheel scroll
    static constexpr float _nearPlane = 1.0f;
    static constexpr float _farPlane = 1000.0f;
};
