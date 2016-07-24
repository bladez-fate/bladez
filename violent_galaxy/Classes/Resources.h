#pragma once

#include "Defs.h"

using ResAmount = ui64;

enum class Res : ui8 {
    Ore = 0,
    Oil = 1,
};

static constexpr ui8 ResMax = 2;

struct Deposit {
    Res res;
    ResAmount resLeft;
};
