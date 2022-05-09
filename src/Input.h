#ifndef CGT_INPUT_H
#define CGT_INPUT_H

#include "Event.hpp"
#include <utility>
#include <stdexcept>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

template<typename T>
class Input
{
    friend class Inputs;
public:
    using ValueType = T;
    class Poller
    {
    public:
        virtual bool update(T& val) = 0; //Updates the value and returns true if the value changed
        virtual ~Poller() = default;
    };

    template<typename P>
    Input(P& p)
        :_poller{ std::make_unique<P>(p) }
    {
    }
    template<typename P>
    Input(P&& p)
        :_poller{ std::make_unique<P>(std::move(p)) }
    {
    }

    Input(std::unique_ptr<Poller> poller)
        :_poller{ std::move(poller) }
    {
        if (!_poller) throw std::invalid_argument("poller must not be null");
    }

    T value() const
    {
        return _value;
    }

    PrivateEvent<Input, T> valueChanged;
private:
    void update()
    {
        if (_poller->update(_value)) valueChanged(_value);
    }

    T _value{};
    std::unique_ptr<Poller> _poller;
};

class KeyPoller : public Input<bool>::Poller
{
public:
    explicit KeyPoller(int keycode);

    bool update(bool& val) override;
private:
    int _keycode;
};

class MousePoller : public Input<glm::vec2>::Poller
{
public:
    MousePoller();

    bool update(glm::vec2& val) override;
private:
};

class MouseButtonPoller : public Input<bool>::Poller
{
public:
    explicit MouseButtonPoller(int keycode);

    bool update(bool& val) override;
private:
    int _keycode;
};

class Inputs
{
    friend class World;
public:
    static Inputs& instance();

    Input<bool> forward{ KeyPoller{GLFW_KEY_W} };
    Input<bool> backward{ KeyPoller{GLFW_KEY_S} };
    Input<bool> left{ KeyPoller{GLFW_KEY_A} };
    Input<bool> right{ KeyPoller{GLFW_KEY_D} };
    Input<bool> attack{ MouseButtonPoller{GLFW_MOUSE_BUTTON_LEFT} };

    Input<glm::vec2> look{ MousePoller{} };
private:
    void update();
};

#endif
