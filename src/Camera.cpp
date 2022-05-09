#include "Camera.h"
#include <GLFW/glfw3.h>

void Camera::start()
{
    _main = this;
}

Camera* Camera::main()
{
    return _main;
}

extern GLFWwindow* window;

glm::mat4 Camera::projectionMatrix() const
{
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    return glm::perspective(glm::radians(45.f), float(w)/h, 0.1f, 100.f);
}

WeakRef<Camera> Camera::_main;
