#ifndef CGT_EVENT_HPP
#define CGT_EVENT_HPP

#include <vector>
#include <utility>
#include "WeakRef.hpp"
#include "Utility.h"

template<typename... Args>
struct Event
{
private:
    friend struct HasEvents;
    typedef void (WeakReferencable::*Fptr)(Args...);
public:
    template<typename T>
    void add(T& target, void (T::*mf)(Args...))
    {
        _callables.emplace_back(target, static_cast<Fptr>(mf));
    }

    template<typename T>
    void remove(T& target)
    {
        WeakRef<WeakReferencable> wr = target;
        std::erase_if(_callables, [&wr](auto& p) { return !p.first || p.first == wr; });
    }

    template<typename... CallArgs>
    void trigger(CallArgs&&... args)
    {
        _callables.erase(Utility::forEachRemovable(_callables, [&](auto& p) {
            if (!p.first) return true;
            (p.first->*p.second)(args...); //No forward as the elements may be shared by multiple functions and should not be moved
            return false;
            }), end(_callables));
    }

    template<typename... CallArgs>
    void operator()(CallArgs&&... args)
    {
        trigger(std::forward<CallArgs>(args)...);
    }

private:
    std::vector<std::pair<WeakRef<WeakReferencable>, Fptr>> _callables;
};

template<typename T, typename... Args>
struct PrivateEvent : private Event<Args...>
{
    friend T;
    using Event<Args...>::add;
    using Event<Args...>::remove;
};

#endif
