#include "SimpleMovement.h"
#include <cmath>
#include <numbers>

//TODO : Input manager instead of this
extern GLFWwindow* window;

void SimpleMovement::update(Seconds delta)
{
    Vector pos = get<Transformation>().translation();

    float dist = _speed * delta.count();

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        pos.z -= dist;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        pos.z += dist;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        pos.x += dist;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        pos.x -= dist;

    if (_terrain)
    {
        pos.y += _fixed_height - _terrain->getAltitudeOf(pos);
    }

    get<Transformation>().translation(pos);
}

void SimpleMovement::terrain(Terrain* t)
{
    _terrain = t;
}

void SimpleMovement::terrain(Terrain& t)
{
    _terrain = t;
}

Terrain* SimpleMovement::terrain() const
{
    return _terrain.ptr();
}
