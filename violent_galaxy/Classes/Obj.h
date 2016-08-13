#pragma once

#include "Defs.h"

#define OBJ_CREATE_FUNC(type) \
    static type* create(GameScene* game) \
    { \
        type *pRet = new(std::nothrow) type(); \
        if (pRet && pRet->init(game)) \
        { \
            pRet->autorelease(); \
            return pRet; \
        } \
        else \
        { \
            delete pRet; \
            pRet = nullptr; \
            return nullptr; \
        } \
    } \
    /**/


enum class ObjType : ui8 {
    Unknown = 0,
    Player = 1,
    AstroObj = 2,
    Unit = 3,
    Projectile = 4,
    Building = 5,
};

class ObjTag {
public:
    ObjTag(int tag)
        : _value(tag)
    {}

    ObjTag(ObjType type, ui32 id)
    {
        CCASSERT((id & ~idMask) == 0, "id overflow");
        _value = ((ui8)type << typeShift) | id;
    }

    ObjType type() const
    {
        return ObjType((_value & typeMask) >> typeShift);
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

class Obj : public cc::Ref {
public:
    Id getId() { return _id; }
    void setId(Id id) { _id = id; }
    virtual ObjType getObjType() = 0;
    virtual void update(float delta);
protected:
    Obj() {}
    virtual bool init(GameScene* game);
    virtual void destroy();
protected:
    Id _id;
    GameScene* _game;
};

class VisualObj : public Obj {
public:
    cc::Node* getNode() { return _rootNode; }
    void setPosition(const cc::Vec2& position);
    virtual float getSize() = 0;
protected:
    VisualObj() {}
    bool init(GameScene* game) override;
    void destroy() override;
    virtual cc::Node* createNodes() = 0;
    virtual cc::PhysicsBody* createBody() = 0;
    virtual void draw() = 0;
protected:
    cc::Node* _rootNode = nullptr;
};
