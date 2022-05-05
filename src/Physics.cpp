#include "Physics.h"
#include "Collider.h"
#include <iterator>

namespace
{
    CollisionVolume& getCv(const WeakRef<CollisionVolume>& v)
    {
        return *v;
    }

    template<typename T>
    CollisionVolume& getCv(const std::pair<WeakRef<CollisionVolume>,T>& v)
    {
        return *v.first;
    }

    template<typename T>
    auto getBoundsFor(T& cont, Entity* eptr)
    {
        using std::begin, std::end;
        auto lower = std::lower_bound(begin(cont), end(cont), eptr, [](const auto& l, const Entity* r) { return std::less<const Entity*>{}(&getCv(l).owner(), r); });
        return std::make_pair(lower, std::upper_bound(lower, end(cont), eptr, [](const Entity* l, const auto& r) { return std::less<const Entity*>{}(l, &getCv(r).owner()); }));
    }

    template<typename T>
    bool handleCvRemove(T& cont, CollisionVolume& c)
    {
        auto [it, sentinel] = getBoundsFor(cont, &c.owner());
        auto total = sentinel - it;
        auto found = std::find_if(it, sentinel, [&c](const auto& v) { return &getCv(v) == &c; });
        if (found == sentinel) return false;
        cont.erase(found);
        return total == 1;
    }
}

Physics& Physics::instance()
{
    static Physics p;
    return p;
}

std::optional<SweepResult> Physics::sweep(const AnyCol& col, const glm::vec3& movement) const
{
    std::optional<SweepResult> result;
    float lowest_t = 2;
    for (auto& cv : _statics)
    {
        if (auto inter = intersectMoving(cv->buildCollisions(), col, movement))
        {
            if (inter->t < lowest_t)
            {
                lowest_t = inter->t;
                result = SweepResult{ *inter, cv.ptr(), nullptr };
            }
        }
    }
    for (auto& di : _dynamics)
    {
        if (auto inter = intersectMoving(di.first->buildCollisions(), col, movement))
        {
            if (inter->t < lowest_t)
            {
                lowest_t = inter->t;
                result = SweepResult{ *inter, di.first.ptr(), di.second.ptr() };
            }
        }
    }

    return result;
}

void Physics::add(CollisionVolume& c)
{
    Entity* eptr = &c.owner();
    Status& status = _status[eptr];
    if (status != Status::Dynamic)
    {
        status = Status::Static;
        _statics.emplace(getBoundsFor(_statics, eptr).second, c);
        return;
    }
    std::vector<DynamicInfo> to_add;
    auto [it, sentinel] = getBoundsFor(_dynamics, eptr);
    if (!it->first) //This should only happen at start, if the physicsmovement is started before the collisionvolume
    {
        it->first = c;
    }
    else
    {
        _dynamics.emplace(sentinel, c, it->second);
    }
}

void Physics::remove(CollisionVolume& c)
{
    Entity* eptr = &c.owner();
    auto status_it = _status.find(eptr);

    bool all_erased = status_it->second == Status::Static? handleCvRemove(_statics, c) : handleCvRemove(_dynamics, c);
    if (all_erased) _status.erase(status_it);
}

void Physics::add(PhysicsMovement& m)
{
    Entity* eptr = &m.owner();
    Status& status = _status[eptr];
    std::vector<DynamicInfo> to_add;
    if (status == Status::None)
    {
        to_add.emplace_back(nullptr, m);
    }
    else //if (status == Status::Static)
    {
        auto [it, sentinel] = getBoundsFor(_statics, eptr);
        to_add.reserve(sentinel - it);
        for (auto i = it; i != sentinel; ++i)
        {
            to_add.emplace_back(std::move(*i), m);
        }
        _statics.erase(it, sentinel);
    }

    status = Status::Dynamic;
    auto [it, sentinel] = getBoundsFor(_dynamics, eptr);
    _dynamics.insert(sentinel, std::make_move_iterator(to_add.begin()), std::make_move_iterator(to_add.end()));
}

void Physics::remove(PhysicsMovement& m)
{
    Entity* eptr = &m.owner();
    auto [it, sentinel] = getBoundsFor(_dynamics, eptr);
    if (it == sentinel) return;

    std::vector<WeakRef<CollisionVolume>> to_add;
    to_add.reserve(sentinel - it);
    for (auto i = it; i != sentinel; ++i)
    {
        to_add.emplace_back(std::move(i->first));
    }
    _dynamics.erase(it, sentinel);
    _status[eptr] = Status::Static;
    _statics.insert(getBoundsFor(_statics, eptr).second, std::make_move_iterator(to_add.begin()), std::make_move_iterator(to_add.end()));
}
