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

#include "World.h"
#include "Mesh.h"
#include "Model.h"

template<auto D>
struct RaiiCall
{
    ~RaiiCall()
    {
        D();
    }
};

using namespace SafeGl;

Image loadImage(std::filesystem::path filename)
{
    int x, y, channels;
    SafeHandle<void*, stbi_image_free> data(stbi_load(filename.string().c_str(), &x, &y, &channels, 4));
    if (!data) throw std::invalid_argument("Unable to open texture file");
    Image img;
    img.width = x;
    img.height = y;
    img.pixels.resize(x * y);
    memcpy(img.pixels.data(), data, img.pixels.size() * sizeof(Rgba));

    return img;
}

std::tuple<std::vector<glm::vec3>, std::vector<unsigned short>> generatePlane(size_t n)
{
    std::vector<glm::vec3> vertices(n * n);
    std::vector<unsigned short> indices((n - 1) * (n - 1) * 2 * 3);
    auto next_vertice = begin(vertices);
    auto next_indice = begin(indices);

    for (size_t i = 0; i < n; ++i)
    {
        float i_coord = (float)i / (n - 1) * 2 - 1;
        for (size_t j = 0; j < n; ++j)
        {
            float j_coord = (float)j / (n - 1) * 2 - 1;
            *(next_vertice++) = { i_coord,0,j_coord };
        }
    }

    for (size_t i = 0; i < n - 1; ++i)
    {
        for (size_t j = 0; j < n - 1; ++j)
        {
            *(next_indice++) = i + j * n;
            *(next_indice++) = i + (j + 1) * n;
            *(next_indice++) = i + 1 + j * n;

            *(next_indice++) = i + (j + 1) * n;
            *(next_indice++) = i + 1 + (j + 1) * n;
            *(next_indice++) = i + 1 + j * n;
        }
    }

    return { vertices, indices };
}

void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

glm::vec3 camera_position = glm::vec3(0.0f, -0.5f, 3.0f);
glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
float orbital_angle = 0;
float orbital_dist = 9;
float rotation_speed = 1;


std::chrono::duration<float> delta_time;
bool orbital_mode = false;

int main(void)
{
    if (!glfwInit()) throw std::runtime_error("Glfw initialization failure");
    RaiiCall<glfwTerminate> glfw_terminate;

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1024, 768, "TP2", NULL, NULL);
    if (window == NULL) throw std::runtime_error("Window creation failure");

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) throw std::runtime_error("Glew initialization failure");

    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);

    Mesh mesh;
    setUnitSphere(mesh);
    mesh.texture = loadImage("res/grass.png");
    World world;
    Entity& entity = world.createEntity();
    Model& model = entity.buildComponent<Model>(mesh);

    auto last_tick = std::chrono::high_resolution_clock::now();
    do
    {
        auto this_tick = std::chrono::high_resolution_clock::now();
        delta_time = this_tick - last_tick;
        last_tick = this_tick;

        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view;
        if (orbital_mode)
        {
            view = rotate(rotate(translate(glm::mat4(1.0), glm::vec3(0, 0, -orbital_dist)), glm::radians(45.f), glm::vec3(1,0,0)), orbital_angle, glm::vec3(0, 1, 0));
        }
        else
        {
            view = glm::lookAt(camera_position, camera_position + camera_target, camera_up);;
        }
        glm::mat4 projection = glm::perspective(glm::radians(45.f), 4.f / 3, 0.1f, 100.f);

        glm::mat4 vp = projection * view;

        model.draw(vp);

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (glfwWindowShouldClose(window) == 0);

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
        case GLFW_KEY_C:
            orbital_mode = !orbital_mode;
            break;
        }
    }
}

void processInput(GLFWwindow* window)
{
    //Camera zoom in and out
    float cameraSpeed = 2.5f * delta_time.count();
    glm::vec3 rightVector = normalize(cross(camera_target, camera_up));

    if (orbital_mode)
    {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            rotation_speed = std::clamp(rotation_speed + delta_time.count(), 0.1f, 10.f);
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            rotation_speed = std::clamp(rotation_speed - delta_time.count(), 0.1f, 10.f);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            orbital_dist -= cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            orbital_dist += cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            orbital_angle += rotation_speed * delta_time.count();
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            orbital_angle -= rotation_speed * delta_time.count();
    }
    else
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera_position += cameraSpeed * camera_target;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera_position -= cameraSpeed * camera_target;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera_position += cameraSpeed * rightVector;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera_position -= cameraSpeed * rightVector;
    }
}
