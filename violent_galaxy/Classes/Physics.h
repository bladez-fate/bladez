#pragma once

#include "Defs.h"
#include "Obj.h"

struct ContactInfo {
    cc::PhysicsShape* thisShape = nullptr;
    cc::PhysicsShape* thatShape = nullptr;
    cc::PhysicsBody* thisBody = nullptr;
    cc::PhysicsBody* thatBody = nullptr;
    cc::Node* thisNode = nullptr;
    cc::Node* thatNode = nullptr;
    ObjTag thisTag;
    ObjTag thatTag;
    Obj* thisObj = nullptr;
    Obj* thatObj = nullptr;
    bool swapped = false; // false:this=A,thatB;  true:this=B,that=A;

    cc::PhysicsContact& contact;
    cc::PhysicsContactPreSolve* preSolve = nullptr;
    const cc::PhysicsContactPostSolve* postSolve = nullptr;

    ContactInfo(GameScene* game, cc::PhysicsContact& contact_,
                cc::PhysicsContactPreSolve* preSolve_,
                const cc::PhysicsContactPostSolve* postSolve_);

    void swap();
};
