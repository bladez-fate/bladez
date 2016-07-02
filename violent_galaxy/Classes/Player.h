#pragma once

#include "Defs.h"
#include "Obj.h"

using PlayerId = int;

class Player : public Obj {
public:
    std::string name;
    PlayerId playerId;
    cc::Color4F color;
    ObjType getObjType() override;
protected:
    Player() {}
    bool init(GameScene* game) override;
};

