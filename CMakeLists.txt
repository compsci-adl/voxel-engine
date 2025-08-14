cmake_minimum_required(VERSION 3.10)
project(voxel-engine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)    # changed to off

# Add source files
file(GLOB SOURCES "src/main.cpp")

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker /errorlimit:0")
endif()

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
	GIT_TAG 	master
)

FetchContent_MakeAvailable(glm)

set(IMGUI_PATH ${CMAKE_CURRENT_LIST_DIR}/libs/imgui)
file(GLOB IMGUI_GLOB
    ${IMGUI_PATH}/imgui.h
    ${IMGUI_PATH}/imgui.cpp
    ${IMGUI_PATH}/imconfig.h
    ${IMGUI_PATH}/imgui_demo.cpp
    ${IMGUI_PATH}/imgui_draw.cpp
    ${IMGUI_PATH}/imgui_internal.h
    ${IMGUI_PATH}/imstb_rectpack.h
    ${IMGUI_PATH}/imstb_textedit.h
    ${IMGUI_PATH}/imstb_truetype.h
    ${IMGUI_PATH}/imgui_tables.cpp
    ${IMGUI_PATH}/imgui_widgets.cpp

    # specific bindings...
    ${IMGUI_PATH}/backends/imgui_impl_glfw.h
    ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
    ${IMGUI_PATH}/backends/imgui_impl_opengl3.h
    ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_PATH}/backends/imgui_impl_opengl3_loader.cpp
    )

add_library("imgui" STATIC ${IMGUI_GLOB})
target_include_directories("imgui" PUBLIC ${IMGUI_PATH} /usr/local/include)
target_link_libraries("imgui" PRIVATE glfw)

# Add the executable
add_executable(voxel-engine ${SOURCES})

# Link libraries
target_include_directories(voxel-engine PRIVATE libs/glad/include)
target_include_directories(voxel-engine PRIVATE ${IMGUI_PATH})
target_include_directories(voxel-engine PRIVATE ${IMGUI_PATH}/backends)
target_link_libraries(voxel-engine glad glfw glm::glm ${CMAKE_DL_LIBS} "imgui")

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

