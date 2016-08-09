#include "Physics.h"
#include <chipmunk/chipmunk_private.h>
#include "GameScene.h"

ContactInfo::ContactInfo(GameScene* game, cocos2d::PhysicsContact& contact_,
                         cc::PhysicsContactPreSolve* preSolve_,
                         const cc::PhysicsContactPostSolve* postSolve_)
    : thisShape(contact_.getShapeA())
    , thatShape(contact_.getShapeB())
    , thisShapeTag(thisShape->getTag())
    , thatShapeTag(thatShape->getTag())
    , thisBody(thisShape->getBody())
    , thatBody(thatShape->getBody())
    , thisNode(thisBody->getNode())
    , thatNode(thatBody->getNode())
    , thisObjTag(thisNode->getTag())
    , thatObjTag(thatNode->getTag())
    , thisObj(game->objs()->getById(thisObjTag.id()))
    , thatObj(game->objs()->getById(thatObjTag.id()))
    , contact(contact_)
    , preSolve(preSolve_)
    , postSolve(postSolve_)
{}

void ContactInfo::swap()
{
    swapped = !swapped;
    std::swap(thisShape, thatShape);
    std::swap(thisShapeTag, thatShapeTag);
    std::swap(thisBody,  thatBody);
    std::swap(thisNode,  thatNode);
    std::swap(thisObjTag,   thatObjTag);
    std::swap(thisObj,   thatObj);
}
