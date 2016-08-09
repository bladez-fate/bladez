#pragma once

#include "Defs.h"
#include "Obj.h"

class ShapeTag {
public:
    ShapeTag(int tag)
        : _value(tag)
    {}

    template <class T>
    ShapeTag(T type, ui32 id)
    {
        CCASSERT((id & ~idMask) == 0, "id overflow");
        _value = ((ui8)type << typeShift) | id;
    }


    template <class T>
    T type() const
    {
        return T((_value & typeMask) >> typeShift);
    }

    template <class T>
    bool is(T t) const
    {
        return type<T>() == t;
    }

    ui32 id() const
    {
        return _value & idMask;
    }

    int value() const
    {
        return *reinterpret_cast<const int*>(&_value);
    }

    operator int() const
    {
        return value();
    }
private:
    ui32 _value;

    static constexpr ui32 idMask = 0xffffff; // 24 bits
    static constexpr ui32 typeMask = 0xf000000; // 4 bits
    static constexpr ui32 typeShift = 24;
};

struct ContactInfo {
    cc::PhysicsShape* thisShape = nullptr;
    cc::PhysicsShape* thatShape = nullptr;
    ShapeTag thisShapeTag;
    ShapeTag thatShapeTag;
    cc::PhysicsBody* thisBody = nullptr;
    cc::PhysicsBody* thatBody = nullptr;
    cc::Node* thisNode = nullptr;
    cc::Node* thatNode = nullptr;
    ObjTag thisObjTag;
    ObjTag thatObjTag;
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
