#include "WorldView.h"
#include "GameScene.h"

USING_NS_CC;

void WorldView::init(GameScene* game)
{
    _game = game;
    createWorldCamera();
}

void WorldView::update(float delta)
{
    for (auto i = _actions.begin(); i != _actions.end(); ) {
        WorldViewAction* a = *i;
        if (!a->perform(this, delta)) {
            delete a;
            _actions.erase(i++);
        } else {
            ++i;
        }
    }
    applyState(getViewStateAtPoint(_state.center, false, _state.surfaceId), 0.0f);
}

WorldViewAction* WorldView::zoom(float scaleBy, float duration)
{
    float zoom2 = clampf(_state.zoom * scaleBy, _zoomMin, _zoomMax);
    if (duration == 0.0f) {
        _state.zoom = zoom2;
        removeWorldCamera();
        createWorldCamera();
        return nullptr;
    } else {
        WorldViewAction* action = WorldViewAction::createLinear(duration);
        action->zoom(zoom2 / _state.zoom);
        return action;
    }
}

WorldViewAction* WorldView::zoomTo(float scaleBy, Vec2 point, float duration)
{
    Vec2 offs = _state.center - point;
    offs *= 1.0f/_state.zoom;
    float zoom2 = clampf(_state.zoom * scaleBy, _zoomMin, _zoomMax);
    offs *= zoom2;
    Vec2 center2 = point + offs;
    if (duration == 0.0f) {
        _state.zoom = zoom2;
        _state.center = center2;
        removeWorldCamera();
        createWorldCamera();
        return nullptr;
    } else {
        WorldViewAction* action = WorldViewAction::createLinear(duration);
        action->zoomTo(zoom2 / _state.zoom, point);
        return action;
    }
}

WorldViewAction* WorldView::rotate(float rotateBy, float duration)
{
    if (duration == 0.0f) {
        _state.rotation += rotateBy;
        centerAt(_state.center, 0.0f); // Just to enforce redraw
        return nullptr;
    } else {
        WorldViewAction* action = WorldViewAction::createLinear(duration);
        action->rotateBy(rotateBy);
        return action;
    }
}

WorldViewAction* WorldView::rotateAround(float rotateBy, Vec2 axis, float duration)
{
    Vec2 offs = _state.center - axis;
    offs = offs.rotate(Vec2::forAngle(rotateBy));
    Vec2 center2 = axis + offs;
    if (duration == 0.0f) {
        _state.rotation += rotateBy;
        centerAt(center2, 0.0f);
        return nullptr;
    } else {
        WorldViewAction* action = WorldViewAction::createLinear(duration);
        action->moveBy(center2 - _state.center);
        action->rotateBy(rotateBy);
        return action;
    }
}

WorldViewAction* WorldView::scroll(Vec2 offset, bool polar, float duration)
{
    if (duration == 0.0f) {
        Vec2 prevCenter = _state.center;
        Vec2 center = _state.center + offset;
        centerAt(center, 0.0f);
        if (_state.surfaceId) {
            if (polar) {
                if (AstroObj* aobj = _game->objs()->getByIdAs<AstroObj>(_state.surfaceId)) {
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
            }
            centerAt(_state.center, 0.0f);
        }
        return nullptr;
    } else {
        WorldViewAction* action = WorldViewAction::createLinear(duration);
        action->setPolar(polar);
        action->moveBy(offset);
        return action;
    }
}

WorldViewAction* WorldView::screenScroll(Vec2 offset, bool polar, float duration)
{
    return scroll(screen2world(offset) - screen2world(Vec2::ZERO), polar, duration);
}

WorldViewAction* WorldView::follow(Vec2 p, float duration)
{
    _game->physicsWorld()->queryPoint(
        CC_CALLBACK_3(WorldView::onFollowQueryPoint, this),
        p, nullptr
    );
    return applyState(getViewStateAtPoint(p, true, _state.surfaceId), duration)->setSmooth();
}

WorldViewAction* WorldView::follow(Vec2 p, Obj* obj, float duration)
{
    if (obj) {
        if (obj->getObjType() == ObjType::AstroObj) {
            _state.surfaceId = obj->getId();
            return applyState(getViewStateAtPoint(p, true, _state.surfaceId), duration)->setSmooth();
        }
    } else {
        _state.surfaceId = 0;
    }
    return nullptr;
}

void WorldView::act(WorldViewAction* action)
{
    if (action) {
        _actions.push_back(action);
    }
}

Vec2 WorldView::screen2world(Vec2 s)
{
    Vec3 w = _camera->unprojectGL(Vec3(s.x, s.y, 0.0f));
    return Vec2(w.x, w.y);
}

Vec2 WorldView::world2screen(Vec2 w)
{
    Vec2 s = _camera->projectGL(Vec3(w.x, w.y, 0.0f));
    return Vec2(s.x, s.y);
}

float WorldView::angleDiff(float phi, float psi)
{
    return phi - psi; // TODO[fate]: normalize angles and find nearest rotation
}

void WorldView::removeWorldCamera()
{
    _camera->removeFromParent();
}

void WorldView::createWorldCamera()
{
    auto size = _state.getSize();
    _camera = Camera::createOrthographic(
        size.x, size.y,
        _nearPlane, _farPlane
    );
    _camera->setCameraFlag(gWorldCameraFlag);
    _camera->setPositionZ(1.0f);

    centerAt(_state.center, 0.0f);
    _game->addChild(_camera);
}

WorldViewAction* WorldView::applyState(const WorldView::State& state, float duration)
{
    if (duration == 0.0f) {
        if (state.zoom == state.zoom) {
            _state = state;
            centerAt(_state.center, 0.0f);
        } else {
            _state = state;
            removeWorldCamera();
            createWorldCamera();
        }
        return nullptr;
    } else {
        WorldViewAction* action = WorldViewAction::createLinear(duration);
        action->moveBy(state.center - _state.center);
        action->zoom(state.zoom / _state.zoom);
        action->rotateBy(angleDiff(state.rotation, _state.rotation));
        return action;
    }
}

WorldViewAction* WorldView::centerAt(Vec2 center, float duration)
{
    if (duration == 0.0f) {
        _state.center = center;
        Vec2 eye = _state.getEye();
        _camera->setPosition(eye);
        _camera->lookAt(Vec3(eye.x, eye.y, 0.0f), _state.getUp());
        return nullptr;
    } else {
        WorldViewAction* action = WorldViewAction::createLinear(duration);
        action->moveBy(center - _state.center);
        return action;
    }
}

WorldView::State WorldView::getViewStateAtPoint(Vec2 center, bool zoomIfRequired, Id surfaceId) const
{
    State result = _state;
    result.center = center;
    result.surfaceId = surfaceId;
    if (surfaceId) {
        if (AstroObj* aobj = _game->objs()->getByIdAs<AstroObj>(surfaceId)) {
            Vec2 up = center - aobj->getNode()->getPosition();
            if (!up.isSmall()) {
                float ratio = up.length() / (_state.getSize().y / 2.0);
                bool zoomRequired = ratio < 1.0;
                if (zoomRequired && zoomIfRequired) {
                    float scaleBy = ratio / 2;
                    result.zoom *= scaleBy;
                    zoomRequired = false;
                }
                if (!zoomRequired) {
                    result.rotation = up.getAngle();
                    return result;
                }
            }
        }
    }
    result.surfaceId = 0;
    return result;
}

bool WorldView::onFollowQueryPoint(PhysicsWorld& pworld, PhysicsShape& shape, void* userdata)
{
    UNUSED(pworld);
    UNUSED(userdata);
    ObjTag tag(shape.getBody()->getNode()->getTag());
    if (tag.type() == ObjType::AstroObj) {
        _state.surfaceId = tag.id();
    }
    if (tag.type() == ObjType::Unit || tag.type() == ObjType::AstroObj) {
        _state.followId = tag.id();
    }
    return true;
}

bool WorldViewAction::perform(WorldView* view, float dt)
{
    float t1 = _elapsed;
    _elapsed += dt;
    if (_elapsed > _duration) {
        _elapsed = _duration;
    }
    float t2 = _elapsed;
    if (t1 < t2) {
        CCASSERT(_duration != 0, "zero duration of WorldViewAction");
        float p1 = t1 / _duration;
        float p2 = t2 / _duration;
        float dp = 0;
        switch (_type) {
        case EType::Linear:
            dp = p2 - p1;
            break;
        case EType::Smooth:
            dp = smooth(p2) - smooth(p1);
            break;
        }
        if (_offset != Vec2::ZERO) {
            view->scroll(dp * _offset, _polar, 0.0f);
        }
        if (_zoom != 1.0f) {
            view->zoom(powf(_zoom, dp), 0.0f);
        }
        if (_zoomTo != 1.0f) {
            view->zoomTo(powf(_zoomTo, dp), _zoomToPoint, 0.0f);
        }
        if (_rotate != 0.0f) {
            view->rotate(dp * _rotate, 0.0f);
        }
    }
    bool res = _elapsed < _duration;
    if (_nextSpawn) {
        bool nextRes = _nextSpawn->perform(view, dt);
        res = res || nextRes;
    }
    return res;
}

float WorldViewAction::smooth(float p)
{
    if (p < 0.5) {
        return 2*p*p;
    } else {
        p = 1 - p;
        return 1 - 2*p*p;
    }
}
