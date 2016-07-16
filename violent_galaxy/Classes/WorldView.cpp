#include "WorldView.h"
#include "GameScene.h"

USING_NS_CC;

void WorldView::init(GameScene* game)
{
    _game = game;
    createWorldCamera(Vec2::ZERO);
}

void WorldView::update(float delta)
{

}

void WorldView::createWorldCamera(Vec2 eye)
{
    auto s = Director::getInstance()->getVisibleSize();
    _cameraSize = Vec2(s.width * _zoom, s.height * _zoom);
    _camera = Camera::createOrthographic(
        _cameraSize.x, _cameraSize.y,
        _nearPlane, _farPlane
    );
    _camera->setCameraFlag(gWorldCameraFlag);
    _camera->setPositionZ(1.0f);
    lookAt(eye, false);
    _game->addChild(_camera);
}

Vec2 WorldView::getCenter() const
{
    return _eye + (_cameraSize / 2.0).rotate(Vec2::forAngle(_rotation - M_PI_2));
}

void WorldView::eyeAt(Vec2 eye)
{
    _eye = eye;
    _camera->setPosition(_eye);
    _camera->lookAt(Vec3(_eye.x, _eye.y, 0.0f), Vec3(cosf(_rotation), sinf(_rotation), 0.0f));
}

void WorldView::lookAt(Vec2 eye, bool continuos)
{
    Vec2 prevCenter = getCenter();
    eyeAt(eye);
    if (_surfaceId) {
        surfaceView(getCenter(), prevCenter, continuos, false);
    }
}

void WorldView::centerAt(Vec2 center)
{
    eyeAt(center - (_cameraSize/2.0).rotate(Vec2::forAngle(_rotation - M_PI_2)));
}

void WorldView::zoom(float scaleBy, Vec2 center)
{
    Vec2 offs = _eye - center;
    offs *= 1.0f/_zoom;
    _zoom = clampf(_zoom * scaleBy, _zoomMin, _zoomMax);
    offs *= _zoom;
    Vec2 eyeNew = center + offs;
    _camera->removeFromParent();
    createWorldCamera(eyeNew);
}

void WorldView::rotate(float rotateBy, Vec2 center)
{
    Vec2 offs = _eye - center;
    _rotation += rotateBy;
    offs = offs.rotate(Vec2::forAngle(rotateBy));
    Vec2 eyeNew = center + offs;
    lookAt(eyeNew, false);
}

void WorldView::surfaceView(Vec2 center, Vec2 prevCenter, bool continuos, bool zoomIfRequired)
{
    if (!_surfaceId) {
        return;
    }
    if (AstroObj* aobj = _game->objs()->getByIdAs<AstroObj>(_surfaceId)) {
        if (continuos) {
            // For enforcing constant altitude during horizontal scrolling
            Vec2 prevUp = prevCenter - aobj->getNode()->getPosition();
            if (!prevUp.isSmall()) {
                Vec2 delta = center - prevCenter;
                Vec2 upNormal = prevUp.getNormalized();
                Vec2 delta1 = delta.unrotate(upNormal);
                float rb = prevUp.length();
                float r = rb + delta1.x;
                float a = delta1.y / rb; // We assume that angle is small
                Vec2 newUp1 = r * Vec2::forAngle(a);
                Vec2 newUp = newUp1.rotate(upNormal);
                center = newUp + aobj->getNode()->getPosition();
            }
        }
        Vec2 up = center - aobj->getNode()->getPosition();
        if (up.isSmall()) {
            centerAt(center);
        } else {
            float ratio = up.length() / (_cameraSize.y / 2.0);
            bool zoomRequired = ratio < 1.0;
            if (zoomRequired && zoomIfRequired) {
                float scaleBy = ratio / 2;
                zoom(scaleBy, center);
                zoomRequired = false;
            }
            if (!zoomRequired) {
                _rotation = up.getAngle();
                centerAt(center);
                return;
            }
        }
    }
    _surfaceId = 0;
}

void WorldView::follow(Vec2 p)
{
    _game->physicsWorld()->queryPoint(
        CC_CALLBACK_3(WorldView::onFollowQueryPoint, this),
        p, nullptr
    );
    surfaceView(p, Vec2::ZERO, false, true);
}

void WorldView::follow(Vec2 p, Obj* obj)
{
    if (obj) {
        if (obj->getObjType() == ObjType::AstroObj) {
            _surfaceId = obj->getId();
            surfaceView(p, Vec2::ZERO, false, true);
        }
    } else {
        _surfaceId = 0;
    }
}

bool WorldView::onFollowQueryPoint(PhysicsWorld& pworld, PhysicsShape& shape, void* userdata)
{
    UNUSED(pworld);
    UNUSED(userdata);
    ObjTag tag(shape.getBody()->getNode()->getTag());
    if (tag.type() == ObjType::AstroObj) {
        _surfaceId = tag.id();
    }
    if (tag.type() == ObjType::Unit || tag.type() == ObjType::AstroObj) {
        _followId = tag.id();
    }
    return true;
}

void WorldView::onScroll(Vec2 screenDir)
{
    Vec2 eye = _eye + screen2world(screenDir) - screen2world(Vec2::ZERO);
    lookAt(eye, true);
}

void WorldView::onZoom(float times, Vec2 center)
{
    float scaleBy = powf(_zoomFactor, times);
    zoom(scaleBy, center);
}

void WorldView::onRotate(float times, Vec2 center)
{
    if (_surfaceId) {
        return; // Rotation in surface view is disabled
    }
    rotate(times * _rotationFactor, center);
}

Vec2 WorldView::screen2world(Vec2 s)
{
    Vec3 w = _camera->unprojectGL(Vec3(s.x, s.y, 0.0f));
    return Vec2(w.x, w.y);
}
