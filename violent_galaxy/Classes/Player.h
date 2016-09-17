#pragma once

#include "Defs.h"
#include "Obj.h"
#include "Resources.h"
#include "Units.h"

using PlayerId = int;

class Player : public Obj {
public:
    std::string name;
    PlayerId playerId;
    cc::Color4F color;

    ResVec res = {{0, 0}};
    i64 supply = 0; // Currently in use
    i64 supplyMax = 0; // Currently can be supported
    i64 supplyLimit = 200; // Game limit of supply for given player

    std::vector<Id> selected;
    std::vector<std::vector<Id>> groups;
public:
    OBJ_CREATE_FUNC(Player);
    bool isSelect(Id id);
    void select(Id id);
    void selectAdd(Id id);
    void selectRemove(Id id);
    void clearSelection();
    void drawSelection(bool value = true);
    void selectGroup(size_t idx);
    void setSelectionToGroup(size_t idx);
    void addSelectionToGroup(size_t idx);

    void giveOrder(Unit::Order order, bool add);

    ObjType getObjType() override;
    void update(float delta) override;
protected:
    Player() {}
    bool init(GameScene* game) override;
private:
    cc::DrawNode* _selectionNode = nullptr;
    bool _drawSelection = false;
};

