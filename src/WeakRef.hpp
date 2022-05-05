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

    WeakReferencable& operator=(const WeakReferencable& other) = delete;
    WeakReferencable& operator=(WeakReferencable&& other) noexcept
    {
        if (&other == this) return *this;

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
    T* uncheckedPtr() const noexcept
    {
        return const_cast<T*>(static_cast<const T*>(*_ptr));
    }
public:
    WeakRef(std::nullptr_t = nullptr) {};
    WeakRef(T* ptr) : _ptr(ptr? ptr->getSelfPtr() : nullptr) {};
    WeakRef(T& ref) : _ptr(ref.getSelfPtr()) {}
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

        return *this;
    }

    //Returns nullptr if invalid. The other accessors lead to undefined behaviour when invalid
    T* ptr() const noexcept
    {
        return _ptr? uncheckedPtr() : nullptr;
    }

    operator T*() const noexcept
    {
        return ptr();
    }

    T* operator->() const noexcept
    {
        return uncheckedPtr();
    }

    T& get() const noexcept
    {
        return *uncheckedPtr();
    }

    T& operator*() const noexcept
    {
        return get();
    }

    explicit operator bool() const noexcept
    {
        return ptr() != nullptr;
    }

    friend bool operator==(const WeakRef& l, const WeakRef& r) = default;
    template<typename P>
    friend bool operator==(const WeakRef& l, P* r) { return l.ptr() == r; }
};

#endif
