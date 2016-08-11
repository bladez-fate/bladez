#pragma once

#include "Defs.h"
#include "Obj.h"

using PlayerId = int;

class Player : public Obj {
public:
    OBJ_CREATE_FUNC(Player);
    std::string name;
    PlayerId playerId;
    cc::Color4F color;
    std::vector<Id> selected;
    bool isSelect(Id id);
    void select(Id id);
    void selectAdd(Id id);
    void selectRemove(Id id);
    void clearSelection();
    void drawSelection(bool value = true);
    ObjType getObjType() override;
    void update(float delta) override;
protected:
    Player() {}
    bool init(GameScene* game) override;
private:
    cc::DrawNode* _selectionNode = nullptr;
    bool _drawSelection = false;
};

