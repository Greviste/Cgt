#include "SafeGl.hpp"
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

template<auto D>
struct RaiiCall
{
    ~RaiiCall()
    {
        D();
    }
};

using namespace SafeGl;

std::string loadTextFile(std::filesystem::path path)
{
    std::ifstream file(path);
    if (!file) throw std::invalid_argument("Unable to open file");

    std::ostringstream sstream;
    sstream << file.rdbuf();
    return std::move(sstream).str();
}

Shader compileShader(GLenum shader_type, std::string_view src)
{
    Shader shader(glCreateShader(shader_type));
    const char* src_pointer = src.data();
    glShaderSource(shader.get(), 1, &src_pointer, NULL);
    glCompileShader(shader.get());

    GLint result;
    GLint log_length;
    glGetShaderiv(shader.get(), GL_COMPILE_STATUS, &result);
    glGetShaderiv(shader.get(), GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
        std::string log_message(log_length + 1, '\0');
        glGetShaderInfoLog(shader.get(), log_length, NULL, log_message.data());
        std::clog << "[Shader compilation] : " << log_message << std::endl;
    }
    if (result != GL_TRUE) throw std::runtime_error("Shader compilation failed");

    return shader;
}

Program compileShaderProgram(std::string_view vertex_src, std::string_view fragment_src)
{
    Shader vertex_shader(compileShader(GL_VERTEX_SHADER, vertex_src));
    Shader fragment_shader(compileShader(GL_FRAGMENT_SHADER, fragment_src));
    Program program(glCreateProgram());
    glAttachShader(program.get(), vertex_shader.get());
    glAttachShader(program.get(), fragment_shader.get());
    glLinkProgram(program.get());

    GLint result;
    GLint log_length;
    glGetProgramiv(program.get(), GL_LINK_STATUS, &result);
    glGetProgramiv(program.get(), GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
        std::string log_message(log_length + 1, '\0');
        glGetProgramInfoLog(program.get(), log_length, NULL, log_message.data());
        std::clog << "[Shader linking] : " << log_message << std::endl;
    }
    if (result != GL_TRUE) throw std::runtime_error("Shader linking failed");

    glDetachShader(program.get(), vertex_shader.get());
    glDetachShader(program.get(), fragment_shader.get());

    return program;
}

Program loadAndCompileProgram(std::filesystem::path v, std::filesystem::path f)
{
    return compileShaderProgram(loadTextFile(v), loadTextFile(f));
}

Texture loadTexture(std::filesystem::path filename)
{
    int x, y, channels;
    SafeHandle<void*, stbi_image_free> data(stbi_load(filename.string().c_str(), &x, &y, &channels, 4));
    if (!data) throw std::invalid_argument("Unable to open texture file");
    std::cout << channels;
    Texture texture;
    glGenTextures(1, handleInit(texture));
    glBindTexture(GL_TEXTURE_2D, texture.get());

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.get());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
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

glm::vec3 camera_position = glm::vec3(0.0f, 0.5f, 3.0f);
glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
float orbital_angle = 0;
float orbital_dist = 9;
float rotation_speed = 1;

size_t plane_size = 16;

Buffer vertex_buffer;
Buffer element_buffer;
size_t element_count;

void rebuildPlane(size_t n)
{
    auto [plane_vertices, plane_indices] = generatePlane(n);
    glBufferData(GL_ARRAY_BUFFER, plane_vertices.size() * sizeof(glm::vec3), plane_vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, plane_indices.size() * sizeof(unsigned short), plane_indices.data(), GL_STATIC_DRAW);
    element_count = plane_indices.size();
}

void bindTextureAt(const Texture& texture, GLuint location, int id)
{
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_2D, texture.get());
    if (location != -1) glUniform1i(location, id);
}


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

    VertexArray vertex_array;
    glGenVertexArrays(1, handleInit(vertex_array));
    glBindVertexArray(vertex_array.get());

    Program program = loadAndCompileProgram("res/vertex_shader.glsl", "res/fragment_shader.glsl");
    GLuint mvpLocation = glGetUniformLocation(program.get(), "mvp");
    GLuint heightmap_location = glGetUniformLocation(program.get(), "heightmap");
    GLuint grass_location = glGetUniformLocation(program.get(), "grass_texture");
    GLuint rock_location = glGetUniformLocation(program.get(), "rock_texture");
    GLuint snow_location = glGetUniformLocation(program.get(), "snow_texture");

    glGenBuffers(1, handleInit(vertex_buffer));
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.get());
    glGenBuffers(1, handleInit(element_buffer));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer.get());

    rebuildPlane(plane_size);

    Texture heightmap = loadTexture("res/Heightmap_Mountain.png");
    Texture grass_texture = loadTexture("res/grass.png");
    Texture rock_texture = loadTexture("res/rock.png");
    Texture snow_texture = loadTexture("res/snowrocks.png");

    auto last_tick = std::chrono::high_resolution_clock::now();
    do
    {
        auto this_tick = std::chrono::high_resolution_clock::now();
        delta_time = this_tick - last_tick;
        last_tick = this_tick;

        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program.get());
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.get());
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer.get());

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

        glm::mat4 mvp = projection * view * glm::mat4(1.0);
        if (mvpLocation != -1) glUniformMatrix4fv(mvpLocation, 1, false, &mvp[0][0]);

        bindTextureAt(heightmap, heightmap_location, 0);
        bindTextureAt(grass_texture, grass_location, 1);
        bindTextureAt(rock_texture, rock_location, 2);
        bindTextureAt(snow_texture, snow_location, 3);

        glDrawElements(GL_TRIANGLES, element_count, GL_UNSIGNED_SHORT, nullptr);

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
        case GLFW_KEY_KP_ADD:
            if (plane_size < 256)
            {
                plane_size *= 2;
                rebuildPlane(plane_size);
            }
            break;
        case GLFW_KEY_KP_SUBTRACT:
            if (plane_size > 2)
            {
                plane_size /= 2;
                rebuildPlane(plane_size);
            }
            break;
        case GLFW_KEY_C:
            orbital_mode = !orbital_mode;
            break;
        }
    }
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

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
