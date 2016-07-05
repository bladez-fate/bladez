#pragma once

#include <vector>
#include <base/ccMacros.h>

struct RPoint {
    float r;
    float a;
};

template <class Cell>
class AngularVec : public std::vector<Cell> {
public:
    using Base = std::vector<Cell>;
    using Iterator = typename Base::iterator;
    using ConstIterator = typename Base::const_iterator;

    explicit AngularVec(size_t asize)
        : Base(asize)
        , _astep(2 * M_PI / asize)
    {}

    Iterator locate(float a)
    {
        if (a < 0) { // Get rid of negative angle (for mod to work right)
            a -= (2 * M_PI) * floorf(a / (2 * M_PI));
            CCASSERT(a >= 0, "angle is still negtive");
        }
        i64 ai = (i64)floorf(a / _astep) % this->size();
        CCASSERT(ai >= 0 && ai < (i64)this->size(), "angle rounding internal error");
        return this->begin() + ai;
    }

    float angle(Iterator i) const
    {
        return (i - this->begin()) * _astep;
    }

    float angle(ConstIterator i) const
    {
        return (i - this->begin()) * _astep;
    }

private:
    float _astep;
};

template <class Cell>
class RadialGrid {
public:
    friend class Iterator;
    class Iterator {
    public:
        Iterator()
            : _grid(nullptr)
            , _ri(-1)
            , _ai(-1)
        {}

        Iterator(RadialGrid* grid_, size_t ri_, size_t ai_)
            : _grid(grid_)
            , _ri(ri_)
            , _ai(ai_)
        {}

        bool isValid()
        {
            return _grid;
        }

        Cell& operator*()
        {
            return _grid->getCell(_ri, _ai);
        }

        Cell* operator->()
        {
            return &this->operator*();
        }
    private:
        RadialGrid* _grid;
        size_t _ri;
        size_t _ai;
    };

    RadialGrid(float r1, float r2, size_t rsize, size_t asize)
        : _r1(r1)
        , _r2(r2)
        , _rsize(rsize)
        , _asize(asize)
        , _rstep((_r2 - _r1) / _rsize)
        , _astep(2 * M_PI / _asize)
    {
        _cells.resize(_rsize * _asize);
    }

    Iterator locate(float r, float a)
    {
        i64 ri = (i64)floorf((r - _r1) / _rstep);
        if (a < 0) { // Get rid of negative angle (for mod to work right)
            a -= (2 * M_PI) * floorf(a / (2 * M_PI));
            CCASSERT(a >= 0, "angle is still negtive");
        }
        i64 ai = (i64)floorf(a / _astep) % _asize;
        CCASSERT(ai >= 0 && ai < _asize, "angle rounding internal error");
        if (ri < 0 || ri >= _rsize) {
            return Iterator(); // Out of grid range
        }
        return Iterator(this, ri, ai);
    }

public: // Accessors
    Cell* getCell(size_t ri, size_t ai)
    {
        return _cells[ri * _asize + ai];
    }
private:
    float _r1; // starting radius
    float _r2; // ending radius
    size_t _rsize; // number of radial steps
    size_t _asize; // number of angular steps
    float _rstep;
    float _astep;
    std::vector<Cell> _cells; // (ri * asize + ai) -> cell
};
