#include "World.h"
#include "Entity.h"
#include "Component.h"
#include "Light.h"
#include "Input.h"
#include "Camera.h"
#include <algorithm>

Entity& World::createEntity()
{
    return *_entities.emplace_back(new Entity(*this));
}

void World::update(Seconds s)
{
    Inputs::instance().update();

    _tick_remainder += s;
    while (_tick_remainder >= _tick_period)
    {
        tickOnce();
        _tick_remainder -= _tick_period;
    }

    forEachManager([&s](ComponentManagerBase& m) { m.update(s); });

    while(_should_cleanup)
    {
        _should_cleanup = false;
        auto it = _entities.begin();
        forEachManager([&s](ComponentManagerBase& m) { m.cleanup(s); });
    }
    erase_if(_entities, [](auto& ptr) { return ptr->_marked_for_destroy; });
}

extern GLFWwindow* window;

void World::draw() const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (Camera* cam = Camera::main())
    {
        glm::mat4 v = cam->get<Transformation>().invMatrix();
        glm::mat4 inv_v = cam->get<Transformation>().matrix();
        glm::mat4 p = cam->projectionMatrix();

        _lights.clear();
        for (const Light* light : getAll<Light>())
        {
            _lights.push_back(light->buildData());
            _lights.back().pos = v * glm::vec4(_lights.back().pos, 1);
            _lights.back().vp *= inv_v;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glCullFace(GL_BACK);

        for (auto& elem : _managers) elem.second->draw(v, p);
    }
}

void World::drawGeometry(const glm::mat4& v, const glm::mat4& p) const
{
    for (auto& elem : _managers) elem.second->drawGeometry(v, p);
}

void World::tickOnce()
{
    forEachManager([=](ComponentManagerBase& m) { m.tick(_tick_period); });
}

const std::vector<LightData>& World::lightData() const
{
    return _lights;
}

template<typename F>
void World::forEachManager(F&& func)
{
    for (auto& elem : _managers) func(*elem.second);
    while (!_additional_managers.empty())
    {
        auto list = std::move(_additional_managers);
        for (auto& elem : list)
        {
            func(*elem.second);
            _managers.emplace(std::move(elem));
        }
    }
}
