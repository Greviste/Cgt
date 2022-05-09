#include "SafeGl.h"
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstddef>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstring>

#include "World.h"
#include "Mesh.h"
#include "Model.h"
#include "Rotator.h"
#include "Terrain.h"
#include "SimpleMovement.h"
#include "Light.h"
#include "Collider.h"
#include "Camera.h"
#include "Utility.h"

template<auto D>
struct RaiiCall
{
    ~RaiiCall()
    {
        D();
    }
};

using namespace SafeGl;
using namespace Utility;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
GLFWwindow* window = nullptr;

int main(void)
{
    try {
        if (!glfwInit()) throw std::runtime_error("Glfw initialization failure");
        RaiiCall<glfwTerminate> glfw_terminate;

        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(1024, 768, "TP2", NULL, NULL);
        if (window == NULL) throw std::runtime_error("Window creation failure");

        glfwMakeContextCurrent(window);
        glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
        glfwSetKeyCallback(window, key_callback);

        glewExperimental = true;
        if (glewInit() != GLEW_OK) throw std::runtime_error("Glew initialization failure");

        glClearColor(0.8f, 0.8f, 0.8f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        MeshData mesh, cube_mesh;
        setUnitSphere(mesh);
        setUnitCube(cube_mesh);
        World world;

        Entity& cube = world.createEntity();
        cube.buildComponent<Model>(loadImage("res/moon.jpg"), cube_mesh);
        cube.getOrBuildComponent<CollisionVolume>().volume = CollisionBox{ glm::vec3{ 0.1, 0.1, 0.1 } };
        cube.getOrBuildComponent<PhysicsMovement>().velocity({ -1,0,0 });
        cube.getOrBuildComponent<Transformation>().scale({ 0.1,0.1,0.1 });
        cube.getOrBuildComponent<Transformation>().translation({ 1,2,0 });
        cube.getOrBuildComponent<Transformation>().rotation(glm::vec3{ 0.1,1.12,0.5 });

        Entity& sphere = world.createEntity();
        sphere.buildComponent<Model>(loadImage("res/moon.jpg"), mesh);
        sphere.getOrBuildComponent<CollisionVolume>().volume = CollisionSphere{ 0.1 };
        sphere.getOrBuildComponent<PhysicsMovement>();
        sphere.getOrBuildComponent<Transformation>().scale({ 0.1,0.1,0.1 });
        sphere.getOrBuildComponent<Transformation>().translation({ -1,1.12,0.05 });

        Entity& terrain = world.createEntity();
        //Terrain& terrain_comp = terrain.buildComponent<Terrain>(loadImage("res/Heightmap_Rocky.png"), loadImage("res/grass.png"), loadImage("res/rock.png"), loadImage("res/snowrocks.png"));
        terrain.buildComponent<Model>(loadImage("res/grass.png"), cube_mesh);
        terrain.getOrBuildComponent<Transformation>().scale({5,1,5});
        terrain.getOrBuildComponent<CollisionVolume>().volume = CollisionBox{ { 5, 1, 5 } };

        Entity& earth = world.createEntity();
        earth.getOrBuildComponent<Transformation>().scale({ 0.05, 0.05, 0.05 });
        Model& earth_m = earth.buildComponent<Model>(loadImage("res/earth.jpg"), mesh);
        setUnitSphere(mesh, 10, 10);
        earth_m.addLod(mesh, 1);
        setUnitSphere(mesh, 3, 3);
        earth_m.addLod(mesh, 1); //total 2
        //earth.buildComponent<SimpleMovement>().terrain(terrain_comp);

        Entity& light = world.createEntity();
        light.buildComponent<Transformation>(glm::vec3(0.0f, 5.0f, 10.0f), glm::vec3(-0.1,0,0));
        light.buildComponent<Light>();

        Entity& player = world.createEntity();
        player.buildComponent<Transformation>().translation({ 0,2,0.5 });
        player.buildComponent<Camera>();
        player.buildComponent<SimpleMovement>();

        auto last_tick = std::chrono::high_resolution_clock::now();
        do
        {
            auto this_tick = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> delta_time = this_tick - last_tick;
            last_tick = this_tick;

            world.update(delta_time);
            world.draw();

            glfwSwapBuffers(window);
            glfwPollEvents();
        } while (glfwWindowShouldClose(window) == 0);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what();
        return -1;
    }

    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, true);
            break;
        }
    }
}

