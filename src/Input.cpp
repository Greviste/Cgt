#include "Input.h"

extern GLFWwindow* window;

KeyPoller::KeyPoller(int keycode)
    :_keycode{ keycode }
{
}

bool KeyPoller::update(bool& val)
{
    bool old = val;
    val = glfwGetKey(window, _keycode) == GLFW_PRESS;
    return val != old;
}

MousePoller::MousePoller()
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

bool MousePoller::update(glm::vec2& val)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec2 old = val;
    val.x = xpos;
    val.y = ypos;
    return val != old;
}

Inputs& Inputs::instance()
{
    static Inputs i;
    return i;
}

void Inputs::update()
{
    forward.update();
    backward.update();
    left.update();
    right.update();
    look.update();
}
