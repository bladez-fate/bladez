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

void WorldView::zoom(float scaleBy, Vec2 zoomTo)
{
    Vec2 offs = _state.center - zoomTo;
    offs *= 1.0f/_state.zoom;
    _state.zoom = clampf(_state.zoom * scaleBy, _zoomMin, _zoomMax);
    offs *= _state.zoom;
    Vec2 centerNew = zoomTo + offs;
    _camera->removeFromParent();
    createWorldCamera(centerNew);
}

void WorldView::rotate(float rotateBy, Vec2 axis)
{
    Vec2 offs = _state.center - axis;
    _state.rotation += rotateBy;
    offs = offs.rotate(Vec2::forAngle(rotateBy));
    Vec2 centerNew = axis + offs;
    lookAt(centerNew, false);
}

void WorldView::scroll(Vec2 screenDir)
{
    Vec2 center = _state.center + screen2world(screenDir) - screen2world(Vec2::ZERO);
    lookAt(center, true);
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

Vec2 WorldView::screen2world(Vec2 s)
{
    Vec3 w = _camera->unprojectGL(Vec3(s.x, s.y, 0.0f));
    return Vec2(w.x, w.y);
}

void WorldView::createWorldCamera(Vec2 center)
{
    auto s = Director::getInstance()->getVisibleSize();
    _state.size = Vec2(s.width * _state.zoom, s.height * _state.zoom);
    _camera = Camera::createOrthographic(
        _state.size.x, _state.size.y,
        _nearPlane, _farPlane
    );
    _camera->setCameraFlag(gWorldCameraFlag);
    _camera->setPositionZ(1.0f);

    lookAt(center, false);
    _game->addChild(_camera);
}

void WorldView::lookAt(Vec2 center, bool continuos)
{
    Vec2 prevCenter = _state.center;
    centerAt(center);
    if (_surfaceId) {
        surfaceView(_state.center, prevCenter, continuos, false);
    }
}

void WorldView::centerAt(Vec2 center)
{
    _state.center = center;
    Vec2 eye = _state.getEye();
    _camera->setPosition(eye);
    _camera->lookAt(Vec3(eye.x, eye.y, 0.0f), _state.getUp());
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
            float ratio = up.length() / (_state.size.y / 2.0);
            bool zoomRequired = ratio < 1.0;
            if (zoomRequired && zoomIfRequired) {
                float scaleBy = ratio / 2;
                zoom(scaleBy, center);
                zoomRequired = false;
            }
            if (!zoomRequired) {
                _state.rotation = up.getAngle();
                centerAt(center);
                return;
            }
        }
    }
    _surfaceId = 0;
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
