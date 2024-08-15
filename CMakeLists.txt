cmake_minimum_required(VERSION 3.10)
project(voxel-engine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source files
file(GLOB SOURCES "src/*.cpp")

# Add GLAD
add_library(glad libs/glad/src/glad.c)
target_include_directories(glad PUBLIC libs/glad/include)
include_directories(libs/)
include_directories(learnopengl libs/learnopengl)

# Add GLFW
add_subdirectory(libs/glfw)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
# GLM
include(FetchContent)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
)

FetchContent_MakeAvailable(glm)

# Add the executable
add_executable(voxel-engine ${SOURCES})

# Link libraries
target_link_libraries(voxel-engine glad glfw glm::glm${CMAKE_DL_LIBS})
target_include_directories(voxel-engine PRIVATE libs/glad/include)

# target_link_libraries(voxel-engine PRIVATE glm::glm)


# Define the shaders directory
set(SHADERS_SRC_DIR "${CMAKE_SOURCE_DIR}/src/shaders")
set(SHADERS_DST_DIR "${CMAKE_BINARY_DIR}/shaders")

# Add a custom command to copy shaders to the build directory
add_custom_command(
    TARGET voxel-engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SHADERS_SRC_DIR} ${SHADERS_DST_DIR}
)

# Add custom target to ensure shaders are copied before build
add_custom_target(copy_shaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SHADERS_SRC_DIR} ${SHADERS_DST_DIR}
    DEPENDS ${SOURCES}
)
add_dependencies(voxel-engine copy_shaders)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

