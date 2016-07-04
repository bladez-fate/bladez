#include "Units.h"
#include "GameScene.h"
#include "chipmunk/chipmunk_private.h"

USING_NS_CC;

bool Unit::init(GameScene* game)
{
    VisualObj::init(game);
    _rootNode->getPhysicsBody()->setContactTestBitmask(gMatterBitmask);
    return true;
}

void Unit::destroy()
{
    VisualObj::destroy();
}

void Unit::replaceWith(Unit* unit)
{
    // Move node
    //unit->setPosition(_rootNode->getPosition());
    //unit->_rootNode->setRotation(_rootNode->getRotation());

    // Copy body state
    auto b = _rootNode->getPhysicsBody();
    auto ub = unit->_rootNode->getPhysicsBody();
    auto pos = b->getPosition();
    ub->setPosition(pos.x, pos.y);
    //ub->setRotation(b->getRotation());
    ub->setVelocity(b->getVelocity());
    ub->setAngularVelocity(b->getAngularVelocity());

    // Should not we reuse Id?
    destroy();
}

void Unit::setPlayer(Player* player)
{
    _player = player;
}

ObjType Unit::getObjType()
{
    return ObjType::Unit;
}

bool ColonyShip::init(GameScene* game)
{
    _radius = 10;
    Unit::init(game);
    listenContactAstroObj = true;
    return true;
}

Node* ColonyShip::createNodes()
{
    return DrawNode::create();
}

PhysicsBody* ColonyShip::createBody()
{
    _body = PhysicsBody::createCircle(
        _radius,
        gUnitMaterial,
        Vec2::ZERO
    );
//    _body = PhysicsBody::createBox(
//        Size(2*radius, 2*radius),
//        gUnitMaterial,
//        Vec2::ZERO
//    );
    _body->setMass(1.0);
    _body->setMoment(200.0);
    return _body;
}

void ColonyShip::draw()
{
    node()->drawSolidCircle(Vec2::ZERO, _radius, 0, 12, Color4F::WHITE);
    node()->drawSolidRect(
        Vec2(-_radius/10, -9*_radius/10),
        Vec2( _radius/10,  9*_radius/10),
        Color4F::BLUE
    );
    node()->drawSolidRect(
        Vec2(-9*_radius/10, -_radius/10),
        Vec2( 9*_radius/10,  _radius/10),
        Color4F::BLUE
    );
}

bool ColonyShip::onContactAstroObj(ContactInfo& cinfo)
{
    if (cinfo.contact.getEventCode() == PhysicsContact::EventCode::POSTSOLVE) {
        if (cpArbiter* arb = static_cast<cpArbiter*>(cinfo.contact.getContactInfo())) {
            cpContactPointSet cps = cpArbiterGetContactPointSet(arb);
            bool moving = false;
            for (int i = 0; i < cps.count; ++i) {
                cpVect va = cpBodyGetVelocityAtWorldPoint(arb->body_a, cps.points[i].pointA);
                cpVect vb = cpBodyGetVelocityAtWorldPoint(arb->body_b, cps.points[i].pointB);
                cpVect rv = cpvsub(va, vb);
                cpFloat wa = cpBodyGetAngularVelocity(arb->body_a);
                cpFloat wb = cpBodyGetAngularVelocity(arb->body_b);
                cpFloat rw = wa - wb;
                if (cpvlengthsq(rv) > 1e-6 || fabs(rw) > 0.5) {
                    moving = true;
                    break;
                }
            }
            if (!moving) {
                auto created = onLandCreate(_game);
                replaceWith(created);

                // FIXME: this is sometimes up side down
//                float normal = cpvtoangle(cinfo.swapped? cps.normal: cpvneg(cps.normal));
//                float rotation = normal - M_PI_2;
//                created->getNode()->getPhysicsBody()->setRotation(rotation / (M_PI / 180.0));
//                CCLOG("rotation# %f normal x# %f y# %f", rotation, cps.normal.x, cps.normal.y);

                float up = (cinfo.thisBody->getPosition() - cinfo.thatBody->getPosition()).getAngle()  / (M_PI / 180.0);
                created->getNode()->getPhysicsBody()->setRotation(90 - up);
                CCLOG("LANDING up# %f", up);
            }
        }
    }
    return true;
}

//bool Spaceport::init(GameScene* game)
//{
//    _rootNode = DrawNode::create();
//    cc::Size size(20, 20);
//    node()->drawSolidCircle(Vec2::ZERO, size.width/2, 0, 12, Color4F::WHITE);
//    node()->drawSolidRect(
//        Vec2(-size.width/2, -size.height/2),
//        Vec2( size.width/2,  0),
//        Color4F::WHITE
//    );

//    _body = PhysicsBody::createBox(
//        size,
//        gUnitMaterial,
//        Vec2::ZERO
//    );
//    _rootNode->setPhysicsBody(_body);

//    Unit::init(game);

//    return true;
//}

bool Tank::init(GameScene* game)
{
    _size = 20;

    Size bb(100,50);
    Vec2 offs(0,5);
    Vec2 base[8] = {
        Vec2(-40, -20), Vec2(-50, -10), Vec2(-50, 0), Vec2(-40, 10),
        Vec2(40, 10), Vec2(50, 0), Vec2(50, -10), Vec2(40, -20)
    };
    Vec2 head[6] = {
        Vec2(-30, 10), Vec2(-30, 20), Vec2(-20, 30), Vec2(20, 30),
        Vec2(30, 20), Vec2(30, 10)
    };
    Vec2 gunBegin(0, 20);
    float gunLength = 50;

    float scale = _size/bb.width;
    for (size_t i = 0; i < sizeof(_base)/sizeof(*_base); i++) _base[i] = base[i] * scale;
    for (size_t i = 0; i < sizeof(_head)/sizeof(*_head); i++) _head[i] = head[i] * scale;
    _gunBegin = gunBegin * scale;
    _bb = bb * scale;
    _offs = offs * scale;
    _gunLength = gunLength * scale;

    _angle = 60 * (M_PI/180);

    Unit::init(game);

    return true;
}

Node* Tank::createNodes()
{
    return DrawNode::create();
}

PhysicsBody* Tank::createBody()
{
    _body = PhysicsBody::createBox(
        _bb,
        gUnitMaterial,
        _offs
    );
    return _body;
}

void Tank::draw()
{
    node()->drawSegment(
        _gunBegin,
        _gunBegin + _gunLength * Vec2(cosf(_angle), sinf(_angle)),
        1, Color4F::BLUE
    );
    node()->drawSolidPoly(_base, sizeof(_base)/sizeof(*_base), Color4F::WHITE);
    node()->drawSolidPoly(_head, sizeof(_head)/sizeof(*_head), Color4F::BLUE);
}
