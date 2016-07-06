#include "Physics.h"
#include <chipmunk/chipmunk_private.h>
#include "GameScene.h"

ContactInfo::ContactInfo(GameScene* game, cocos2d::PhysicsContact& contact_,
                         cc::PhysicsContactPreSolve* preSolve_,
                         const cc::PhysicsContactPostSolve* postSolve_)
    : thisShape(contact_.getShapeA())
    , thatShape(contact_.getShapeB())
    , thisBody(thisShape->getBody())
    , thatBody(thatShape->getBody())
    , thisNode(thisBody->getNode())
    , thatNode(thatBody->getNode())
    , thisTag(thisNode->getTag())
    , thatTag(thatNode->getTag())
    , thisObj(game->objs()->getById(thisTag.id()))
    , thatObj(game->objs()->getById(thatTag.id()))
    , contact(contact_)
    , preSolve(preSolve_)
    , postSolve(postSolve_)
{}

void ContactInfo::swap()
{
    swapped = !swapped;
    std::swap(thisShape, thatShape);
    std::swap(thisBody,  thatBody);
    std::swap(thisNode,  thatNode);
    std::swap(thisTag,   thatTag);
    std::swap(thisObj,   thatObj);
}
