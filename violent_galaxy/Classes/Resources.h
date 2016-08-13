#pragma once

#include "Defs.h"

using ResAmount = i64;

enum class Res : ui8 {
    Ore = 0,
    Oil = 1,
};

struct Deposit {
    Res res;
    ResAmount resLeft;
};

struct ResVec {
    ResAmount amount[RES_COUNT];

    ResAmount ore() const
    {
        return amount[(ui8)Res::Ore];
    }

    ResAmount oil() const
    {
        return amount[(ui8)Res::Oil];
    }

    void add(const ResVec& o)
    {
        for (ui8 i = 0; i < RES_COUNT; i++) {
            amount[i] += o.amount[i];
        }
    }

    void sub(const ResVec& o)
    {
        for (ui8 i = 0; i < RES_COUNT; i++) {
            amount[i] -= o.amount[i];
        }
    }
};
