cmake_minimum_required (VERSION 3.12)
project (Cgt)

set(PROJECT_SOURCES
	src/main.cpp
	src/SafeGl.h
	src/SafeGl.cpp
	src/stb_impl.cpp
	src/Model.h
	src/Model.cpp
	src/Mesh.h
	src/Mesh.cpp
	src/Component.h
	src/Component.cpp
	src/Entity.h
	src/Entity.cpp
	src/Transformation.h
	src/Transformation.cpp
	src/WeakRef.hpp
	src/World.h
	src/World.cpp
	src/Rotator.h
	src/Rotator.cpp
	src/Terrain.h
	src/Terrain.cpp
	src/SimpleMovement.h
	src/SimpleMovement.cpp
	src/ComponentManager.hpp
	src/Utility.h
	src/Event.hpp
	src/Geometry.h
	src/Geometry.cpp
)
set(PROJECT_SHADERS
	res/vertex_shader.glsl
	res/fragment_shader.glsl
	res/terrain_vertex_shader.glsl
	res/terrain_fragment_shader.glsl
)
set(PROJECT_RESOURCES
	res/heightmap-1024x1024.png
	res/Heightmap_Mountain.png
	res/Heightmap_Rocky.png
	res/grass.png
    res/rock.png
	res/snowrocks.png
	res/earth.jpg
	res/sun.jpg
	res/moon.jpg
)

include(FetchContent)
FetchContent_Declare(
	STB
	GIT_REPOSITORY https://github.com/nothings/stb.git
	GIT_TAG af1a5bc352164740c1cc1354942b1c6b72eacb8a
)
FetchContent_Declare(
	GLEW
	URL https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.zip
)
FetchContent_Declare(
	GLM
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e
)
FetchContent_Declare(
	GLFW
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG 7d5a16ce714f0b5f4efa3262de22e4d948851525
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(STB GLEW GLM GLFW)

#Manually setup glew because bad :(
add_subdirectory(${glew_SOURCE_DIR}/build/cmake)
target_include_directories(glew_s INTERFACE $<BUILD_INTERFACE:${glew_SOURCE_DIR}/include>)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_SOURCES} ${PROJECT_SHADERS})
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_SHADERS})
set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
target_link_libraries(${PROJECT_NAME} glew_s glm glfw)
target_include_directories(${PROJECT_NAME} PRIVATE ${stb_SOURCE_DIR} src)

#Copy ressources next to executable
foreach(RES ${PROJECT_RESOURCES} ${PROJECT_SHADERS})
	configure_file(${RES} ${RES} COPYONLY)
endforeach()
