#ifndef CGT_WEAKREF_HPP
#define CGT_WEAKREF_HPP

#include <memory>

class WeakReferencable
{
protected:
    WeakReferencable() = default;
    WeakReferencable(const WeakReferencable&) {}
    WeakReferencable(WeakReferencable&& other) noexcept
        : _self(std::move(other._self))
    {
        if(_self) *_self = this;
    }

    WeakReferencable& operator=(const WeakReferencable& other) { return *this; }
    WeakReferencable& operator=(WeakReferencable&& other) noexcept
    {
        if(_self) *_self = nullptr;
        _self = std::move(other._self);
        if(_self) *_self = this;

        return *this;
    }

    ~WeakReferencable() noexcept
    {
        if(_self) *_self = nullptr;
    }
private:
    std::shared_ptr<const WeakReferencable*> getSelfPtr() const
    {
        if (!_self) _self = std::make_shared<const WeakReferencable*>(this);
        return _self;
    }

    template<typename T> friend struct WeakRef;
    mutable std::shared_ptr<const WeakReferencable*> _self;
};



template<typename T>
struct WeakRef
{
    template<typename U> friend struct WeakRef;
private:
    std::shared_ptr<const WeakReferencable*> _ptr;
public:
    WeakRef() = default;
    WeakRef(T& ref) requires std::derived_from<std::remove_cv_t<T>, WeakReferencable> : _ptr(ref.getSelfPtr()) {}
    WeakRef(const WeakRef& other) = default;
    WeakRef(WeakRef&& other) noexcept
    {
        swap(_ptr, other._ptr);
    }
    template<typename U>
    WeakRef(const WeakRef<U>& other) requires std::convertible_to<U*, T*>
        : _ptr(other._ptr)
    {
    }
    template<typename U>
    WeakRef(WeakRef<U>&& other) requires std::convertible_to<U*, T*>
    {
        swap(_ptr, other._ptr);
    }
    WeakRef& operator=(const WeakRef& other) = default;
    WeakRef& operator=(WeakRef&& other) noexcept
    {
        swap(_ptr, other._ptr);
    }

    //Returns nullptr if invalid. The other accessors lead to undefined behaviour when invalid
    T* ptr() const noexcept
    {
        return _ptr? const_cast<T*>(static_cast<const T*>(*_ptr)) : nullptr;
    }

    T* operator->() const noexcept
    {
        return ptr();
    }

    T& operator*() const noexcept
    {
        return *ptr();
    }

    T& get() const noexcept
    {
        return *ptr();
    }

    explicit operator bool() const noexcept
    {
        return ptr() != nullptr;
    }
};

#endif
