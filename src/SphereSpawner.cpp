#include "SphereSpawner.h"
#include "Input.h"
#include "Model.h"
#include "Collider.h"
#include "Utility.h"
#include "TimedDestroy.h"

using namespace std::chrono_literals;

namespace
{
    MeshData simpleSphere()
    {
        MeshData data;
        setUnitSphere(data);
        return data;
    }
}

void SphereSpawner::start()
{
    Inputs::instance().attack.valueChanged.add(*this, &SphereSpawner::spawnInput);
}

void SphereSpawner::spawnInput(bool val)
{
    static MeshData sphere_data = simpleSphere();

    if (!val) return;

    glm::vec3 dir = get<Transformation>().rotation() * glm::vec3{ 0,0,-1 };
    Entity& sphere = world().createEntity();
    sphere.buildComponent<Model>(Utility::loadImage("res/earth.jpg"), sphere_data);
    sphere.getOrBuildComponent<CollisionVolume>().volume = CollisionSphere{ 0.1 };
    sphere.getOrBuildComponent<PhysicsMovement>().velocity(dir * 2.f);
    sphere.getOrBuildComponent<PhysicsMovement>().mass = 1000;
    Transformation& sphere_t = sphere.getOrBuildComponent<Transformation>();
    sphere_t.translation(get<Transformation>().translation());
    sphere_t.scale({ 0.1,0.1,0.1 });
    sphere.buildComponent<TimedDestroy>().timer(2500ms);
}
